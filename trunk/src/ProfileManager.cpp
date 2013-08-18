#include "global.h"
#include "ProfileManager.h"
#include "RageUtil.h"
#include "arch/arch.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "RageFile.h"
#include "RageFileManager.h"
#include "IniFile.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"
#include "GameState.h"
#include "song.h"
#include "Steps.h"
#include "Course.h"
#include "GameManager.h"
#include "ProductInfo.h"
#include "RageUtil.h"
#include "ThemeManager.h"
#include "MemoryCardManager.h"
#include "XmlFile.h"
#include "StepsUtil.h"
#include "Style.h"


ProfileManager*	PROFILEMAN = NULL;	// global and accessable from anywhere in our program

#define NEW_MEM_CARD_NAME		""
#define USER_PROFILES_DIR		"Data/LocalProfiles/"
#define MACHINE_PROFILE_DIR		"Data/MachineProfile/"


ProfileManager::ProfileManager()
{
}

ProfileManager::~ProfileManager()
{
}

void ProfileManager::Init()
{
	FOREACH_PlayerNumber( p )
		m_bWasLoadedFromMemoryCard[p] = false;

	LoadMachineProfile();
}

void ProfileManager::GetLocalProfileIDs( vector<CString> &asProfileIDsOut ) const
{
	GetDirListing( USER_PROFILES_DIR "*", asProfileIDsOut, true, false );
}

void ProfileManager::GetLocalProfileNames( vector<CString> &asNamesOut ) const
{
	CStringArray vsProfileIDs;
	GetLocalProfileIDs( vsProfileIDs );
	LOG->Trace("GetLocalProfileNames: %u", unsigned(vsProfileIDs.size()));
	for( unsigned i=0; i<vsProfileIDs.size(); i++ )
	{
		CString sProfileID = vsProfileIDs[i];
		CString sProfileDir = USER_PROFILES_DIR + sProfileID + "/";
		CString sDisplayName = Profile::GetProfileDisplayNameFromDir( sProfileDir );
		LOG->Trace(" '%s'", sDisplayName.c_str());
		asNamesOut.push_back( sDisplayName );
	}
}


bool ProfileManager::LoadProfile( PlayerNumber pn, CString sProfileDir, bool bIsMemCard )
{
  LOG->Trace( "LoadingProfile P%d, %s, %d", pn+1, sProfileDir.c_str(), bIsMemCard );

	ASSERT( !sProfileDir.empty() );
	ASSERT( sProfileDir.Right(1) == "/" );

	if( bIsMemCard )
		MEMCARDMAN->PauseMountingThread();

	m_sProfileDir[pn] = sProfileDir;
	m_bWasLoadedFromMemoryCard[pn] = bIsMemCard;

	m_Profile[pn].LoadAllFromDir( m_sProfileDir[pn], PREFSMAN->m_bSignProfileData );

	if( bIsMemCard )
		MEMCARDMAN->UnPauseMountingThread();

	LOG->Trace( "Done loading profile." );

	return true;
}

bool ProfileManager::CreateProfile( CString sProfileDir, CString sName )
{
	bool bResult;

	Profile pro;
	pro.m_sDisplayName = sName;
	bResult = pro.SaveAllToDir( sProfileDir, PREFSMAN->m_bSignProfileData );
	if( !bResult )
		return false;

	FlushDirCache();
	return true;	
}

bool ProfileManager::LoadLocalProfileFromMachine( PlayerNumber pn )
{
	CString sProfileID = PREFSMAN->m_sDefaultLocalProfileID[pn];
	if( sProfileID.empty() )
	{
		m_sProfileDir[pn] = "";
		return false;
	}

	CString sDir = USER_PROFILES_DIR + sProfileID + "/";

	return LoadProfile( pn, sDir, false );
}

bool ProfileManager::LoadProfileFromMemoryCard( PlayerNumber pn )
{
	UnloadProfile( pn );

	// mount slot
	if( MEMCARDMAN->GetCardState(pn) == MEMORY_CARD_STATE_READY )
	{
		CString sDir = MEM_CARD_MOUNT_POINT[pn];

		// tack on a subdirectory so that we don't write everything to the root
		sDir += PREFSMAN->m_sMemoryCardProfileSubdir;
		sDir += '/'; 

		bool bResult;
		bResult = LoadProfile( pn, sDir, true );
		if( bResult )
			return true;
	
		CreateMemoryCardProfile( pn );
		
		bResult = LoadProfile( pn, sDir, true );
		return bResult;
	}

	return false;
}
			
bool ProfileManager::CreateMemoryCardProfile( PlayerNumber pn )
{
//	CString sDir = MEM_CARD_DIR[pn];

	ASSERT( MEMCARDMAN->GetCardState(pn) == MEMORY_CARD_STATE_READY );

	CString sDir = MEM_CARD_MOUNT_POINT[pn];
	
	DEBUG_ASSERT( FILEMAN->IsMounted(sDir) );	// should be called only if we've already mounted

	// tack on a subdirectory so that we don't write everything to the root
	sDir += PREFSMAN->m_sMemoryCardProfileSubdir;
	sDir += '/'; 

	return CreateProfile( sDir, NEW_MEM_CARD_NAME );
}

bool ProfileManager::LoadFirstAvailableProfile( PlayerNumber pn )
{
	if( LoadProfileFromMemoryCard(pn) )
		return true;

	if( LoadLocalProfileFromMachine(pn) )
		return true;
	
	return false;
}

void ProfileManager::SaveAllProfiles() const
{
	this->SaveMachineProfile();

	FOREACH_HumanPlayer( pn )
	{
		if( !this->IsUsingProfile(pn) )
			continue;

		this->SaveProfile( pn );
	}
}

bool ProfileManager::SaveProfile( PlayerNumber pn ) const
{
	if( m_sProfileDir[pn].empty() )
		return false;

	if( ProfileWasLoadedFromMemoryCard(pn) )
		MEMCARDMAN->PauseMountingThread();

	bool b = m_Profile[pn].SaveAllToDir( m_sProfileDir[pn], PREFSMAN->m_bSignProfileData );

	if( ProfileWasLoadedFromMemoryCard(pn) )
		MEMCARDMAN->UnPauseMountingThread();
	
	return b;
}

void ProfileManager::UnloadProfile( PlayerNumber pn )
{
	m_sProfileDir[pn] = "";
	m_bWasLoadedFromMemoryCard[pn] = false;
	m_Profile[pn].InitAll();
}

const Profile* ProfileManager::GetProfile( PlayerNumber pn ) const
{
	ASSERT( pn >= 0 && pn<NUM_PLAYERS );

	if( m_sProfileDir[pn].empty() )
		return NULL;
	else
		return &m_Profile[pn];
}

CString ProfileManager::GetPlayerName( PlayerNumber pn ) const
{
	const Profile *prof = GetProfile( pn );
	return prof ? prof->GetDisplayName() : CString("");
}


bool ProfileManager::CreateLocalProfile( CString sName )
{
	ASSERT( !sName.empty() );

	//
	// Find a free directory name in the profiles directory
	//
	CString sProfileID, sProfileDir;
	const int MAX_TRIES = 1000;
    int i;
	for( i=0; i<MAX_TRIES; i++ )
	{
		sProfileID = ssprintf("%08d",i);
		sProfileDir = USER_PROFILES_DIR + sProfileID;
		if( !DoesFileExist(sProfileDir) )
			break;
	}
	if( i == MAX_TRIES )
		return false;
	sProfileDir += "/";

	return CreateProfile( sProfileDir, sName );
}

bool ProfileManager::RenameLocalProfile( CString sProfileID, CString sNewName )
{
	ASSERT( !sProfileID.empty() );

	CString sProfileDir = USER_PROFILES_DIR + sProfileID;

	Profile pro;
	bool bResult;
	bResult = pro.LoadAllFromDir( sProfileDir, PREFSMAN->m_bSignProfileData );
	if( !bResult )
		return false;
	pro.m_sDisplayName = sNewName;
	bResult = pro.SaveAllToDir( sProfileDir, PREFSMAN->m_bSignProfileData );
	if( !bResult )
		return false;

	return true;
}

bool ProfileManager::DeleteLocalProfile( CString sProfileID )
{
	// delete all files in profile dir
	CString sProfileDir = USER_PROFILES_DIR + sProfileID;
	CStringArray asFilesToDelete;
	GetDirListing( sProfileDir + "/*", asFilesToDelete, false, true );
	for( unsigned i=0; i<asFilesToDelete.size(); i++ )
		FILEMAN->Remove( asFilesToDelete[i] );

	// delete edits
	GetDirListing( sProfileDir + "/" + EDITS_SUBDIR + "*", asFilesToDelete, false, true );
	for( unsigned i=0; i<asFilesToDelete.size(); i++ )
		FILEMAN->Remove( asFilesToDelete[i] );

	// remove edits dir
	FILEMAN->Remove( sProfileDir + "/" + EDITS_SUBDIR );

	// remove profile dir
	return FILEMAN->Remove( sProfileDir );
}

void ProfileManager::SaveMachineProfile() const
{
	// If the machine name has changed, make sure we use the new name.
	// It's important that this name be applied before the Player profiles 
	// are saved, so that the Player's profiles show the right machine name.
	const_cast<ProfileManager *> (this)->m_MachineProfile.m_sDisplayName = PREFSMAN->m_sMachineName;

	m_MachineProfile.SaveAllToDir( MACHINE_PROFILE_DIR, false ); /* don't sign machine profiles */
}




void ProfileManager::LoadMachineProfile()
{
	if( !m_MachineProfile.LoadAllFromDir(MACHINE_PROFILE_DIR, false) )
	{
		CreateProfile(MACHINE_PROFILE_DIR, "Machine");
		m_MachineProfile.LoadAllFromDir( MACHINE_PROFILE_DIR, false );
	}

	// If the machine name has changed, make sure we use the new name
	m_MachineProfile.m_sDisplayName = PREFSMAN->m_sMachineName;
}

bool ProfileManager::ProfileWasLoadedFromMemoryCard( PlayerNumber pn ) const
{
	return GetProfile(pn) && m_bWasLoadedFromMemoryCard[pn];
}

CString ProfileManager::GetProfileDir( ProfileSlot slot ) const
{
	switch( slot )
	{
	case PROFILE_SLOT_PLAYER_1:
	case PROFILE_SLOT_PLAYER_2:
		return m_sProfileDir[slot];
	case PROFILE_SLOT_MACHINE:
		return MACHINE_PROFILE_DIR;
	default:
		ASSERT(0);
	}
}

const Profile* ProfileManager::GetProfile( ProfileSlot slot ) const
{
	switch( slot )
	{
	case PROFILE_SLOT_PLAYER_1:
	case PROFILE_SLOT_PLAYER_2:
		if( m_sProfileDir[slot].empty() )
			return NULL;
		else
			return &m_Profile[slot];
	case PROFILE_SLOT_MACHINE:
		return &m_MachineProfile;
	default:
		ASSERT(0);
	}
}

//
// General
//
void ProfileManager::IncrementToastiesCount( PlayerNumber pn )
{
	if( PROFILEMAN->IsUsingProfile(pn) )
		++PROFILEMAN->GetProfile(pn)->m_iNumToasties;
	++PROFILEMAN->GetMachineProfile()->m_iNumToasties;
}

void ProfileManager::AddStepTotals( PlayerNumber pn, int iNumTapsAndHolds, int iNumJumps, int iNumHolds, int iNumMines, int iNumHands )
{
	if( PROFILEMAN->IsUsingProfile(pn) )
		PROFILEMAN->GetProfile(pn)->AddStepTotals( iNumTapsAndHolds, iNumJumps, iNumHolds, iNumMines, iNumHands );
	PROFILEMAN->GetMachineProfile()->AddStepTotals( iNumTapsAndHolds, iNumJumps, iNumHolds, iNumMines, iNumHands );
}

//
// Song stats
//
int ProfileManager::GetSongNumTimesPlayed( const Song* pSong, ProfileSlot slot ) const
{
	return GetProfile(slot)->GetSongNumTimesPlayed( pSong );
}

void ProfileManager::AddStepsScore( const Song* pSong, const Steps* pSteps, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut )
{
	hs.fPercentDP = max( 0, hs.fPercentDP );	// bump up negative scores

	iPersonalIndexOut = -1;
	iMachineIndexOut = -1;

	// In event mode, set the score's name immediately to the Profile's last
	// used name.  If no profile last used name exists, use "EVNT".
	if( PREFSMAN->m_bEventMode )
	{
		Profile* pProfile = PROFILEMAN->GetProfile(pn);
		if( pProfile && !pProfile->m_sLastUsedHighScoreName.empty() )
			hs.sName = pProfile->m_sLastUsedHighScoreName;
		else
			hs.sName = "EVNT";
	}
	else
	{
		hs.sName = RANKING_TO_FILL_IN_MARKER[pn];
	}

	//
	// save high score	
	//
	if( hs.fPercentDP >= PREFSMAN->m_fMinPercentageForMachineSongHighScore )
	{
		if( PROFILEMAN->IsUsingProfile(pn) )
			PROFILEMAN->GetProfile(pn)->AddStepsHighScore( pSong, pSteps, hs, iPersonalIndexOut );

		// don't leave machine high scores for edits
		if( pSteps->GetDifficulty() != DIFFICULTY_EDIT )
			PROFILEMAN->GetMachineProfile()->AddStepsHighScore( pSong, pSteps, hs, iMachineIndexOut );
	}

	//
	// save recent score	
	//
	if( PROFILEMAN->IsUsingProfile(pn) )
		PROFILEMAN->GetProfile(pn)->AddStepsRecentScore( pSong, pSteps, hs );
	PROFILEMAN->GetMachineProfile()->AddStepsRecentScore( pSong, pSteps, hs );
}

void ProfileManager::IncrementStepsPlayCount( const Song* pSong, const Steps* pSteps, PlayerNumber pn )
{
	if( PROFILEMAN->IsUsingProfile(pn) )
		PROFILEMAN->GetProfile(pn)->IncrementStepsPlayCount( pSong, pSteps );
	PROFILEMAN->GetMachineProfile()->IncrementStepsPlayCount( pSong, pSteps );
}

HighScore ProfileManager::GetHighScoreForDifficulty( const Song *s, const Style *st, ProfileSlot slot, Difficulty dc ) const
{
	// return max grade of notes in difficulty class
	vector<Steps*> aNotes;
	s->GetSteps( aNotes, st->m_StepsType );
	StepsUtil::SortNotesArrayByDifficulty( aNotes );

	const Steps* pSteps = s->GetStepsByDifficulty( st->m_StepsType, dc );

	if( pSteps && PROFILEMAN->IsUsingProfile(slot) )
		return PROFILEMAN->GetProfile(slot)->GetStepsHighScoreList(s,pSteps).GetTopScore();
	else
		return HighScore();
}


//
// Course stats
//
void ProfileManager::AddCourseScore( const Course* pCourse, const Trail* pTrail, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut )
{
	hs.fPercentDP = max( 0, hs.fPercentDP );	// bump up negative scores

	iPersonalIndexOut = -1;
	iMachineIndexOut = -1;

	// In event mode, set the score's name immediately to the Profile's last
	// used name.  If no profile last used name exists, use "EVNT".
	if( PREFSMAN->m_bEventMode )
	{
		Profile* pProfile = PROFILEMAN->GetProfile(pn);
		if( pProfile && !pProfile->m_sLastUsedHighScoreName.empty() )
			hs.sName = pProfile->m_sLastUsedHighScoreName;
		else
			hs.sName = "EVNT";
	}
	else
	{
		hs.sName = RANKING_TO_FILL_IN_MARKER[pn];
	}


	//
	// save high score	
	//
	if( hs.fPercentDP >= PREFSMAN->m_fMinPercentageForMachineCourseHighScore )
	{
		if( PROFILEMAN->IsUsingProfile(pn) )
			PROFILEMAN->GetProfile(pn)->AddCourseHighScore( pCourse, pTrail, hs, iPersonalIndexOut );
		PROFILEMAN->GetMachineProfile()->AddCourseHighScore( pCourse, pTrail, hs, iMachineIndexOut );
	}

	//
	// save recent score	
	//
	if( PROFILEMAN->IsUsingProfile(pn) )
		PROFILEMAN->GetProfile(pn)->AddCourseRecentScore( pCourse, pTrail, hs );
	PROFILEMAN->GetMachineProfile()->AddCourseRecentScore( pCourse, pTrail, hs );
}

void ProfileManager::IncrementCoursePlayCount( const Course* pCourse, const Trail* pTrail, PlayerNumber pn )
{
	if( PROFILEMAN->IsUsingProfile(pn) )
		PROFILEMAN->GetProfile(pn)->IncrementCoursePlayCount( pCourse, pTrail );
	PROFILEMAN->GetMachineProfile()->IncrementCoursePlayCount( pCourse, pTrail );
}


//
// Category stats
//
void ProfileManager::AddCategoryScore( StepsType st, RankingCategory rc, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut )
{
	if( hs.fPercentDP > PREFSMAN->m_fMinPercentageForMachineSongHighScore )
	{
		hs.sName = RANKING_TO_FILL_IN_MARKER[pn];
		if( PROFILEMAN->IsUsingProfile(pn) )
			PROFILEMAN->GetProfile(pn)->AddCategoryHighScore( st, rc, hs, iPersonalIndexOut );
		else
			iPersonalIndexOut = -1;
		PROFILEMAN->GetMachineProfile()->AddCategoryHighScore( st, rc, hs, iMachineIndexOut );
	}
}

void ProfileManager::IncrementCategoryPlayCount( StepsType st, RankingCategory rc, PlayerNumber pn )
{
	if( PROFILEMAN->IsUsingProfile(pn) )
		PROFILEMAN->GetProfile(pn)->IncrementCategoryPlayCount( st, rc );
	PROFILEMAN->GetMachineProfile()->IncrementCategoryPlayCount( st, rc );
}

bool ProfileManager::IsUsingProfile( ProfileSlot slot ) const
{
	switch( slot )
	{
	case PROFILE_SLOT_PLAYER_1:
	case PROFILE_SLOT_PLAYER_2:
		return GAMESTATE->IsHumanPlayer((PlayerNumber)slot) && !m_sProfileDir[slot].empty(); 
	case PROFILE_SLOT_MACHINE:
		return true;
	default:
		ASSERT(0);
		return false;
	}
}

/*
 * (c) 2003-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
