#include "global.h"
#include "ScoreKeeperMAX2.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "Steps.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "ScreenGameplay.h"
#include "GameState.h"
#include "Course.h"
#include "SongManager.h"
#include "NoteDataUtil.h"
#include "RageLog.h"
#include "StageStats.h"
#include "ProfileManager.h"
#include "NetworkSyncManager.h"

ScoreKeeperMAX2::ScoreKeeperMAX2( const vector<Song*>& apSongs, const vector<Steps*>& apSteps_, const vector<AttackArray> &asModifiers, PlayerNumber pn_ ):
	ScoreKeeper(pn_), apSteps(apSteps_)
{
	ASSERT( apSongs.size() == apSteps_.size() );
	ASSERT( apSongs.size() == asModifiers.size() );
	//
	// Fill in g_CurStageStats, calculate multiplier
	//
	int iTotalPossibleDancePoints = 0;
	for( unsigned i=0; i<apSteps.size(); i++ )
	{
		Song* pSong = apSongs[i];
		Steps* pSteps = apSteps[i];
		const AttackArray &aa = asModifiers[i];
		NoteData ndTemp;
		pSteps->GetNoteData( &ndTemp );

		/* We might have been given lots of songs; don't keep them in memory uncompressed. */
		pSteps->Compress();

		const Style* pStyle = GAMESTATE->GetCurrentStyle();
		NoteData nd;
		pStyle->GetTransformedNoteDataForStyle( pn_, &ndTemp, &nd );

		/* Compute RadarValues before applying any user-selected mods.  Apply
		 * Course mods and count them in the "pre" RadarValues because they're
		 * forced and not chosen by the user.
		 */
		NoteDataUtil::TransformNoteData( nd, aa, pSteps->m_StepsType, pSong );
		RadarValues rvPre;
		NoteDataUtil::GetRadarValues( nd, pSong->m_fMusicLengthSeconds, rvPre );

		/* Apply user transforms to find out how the notes will really look. 
		 *
		 * XXX: This is brittle: if we end up combining mods for a song differently
		 * than ScreenGameplay, we'll end up with the wrong data.  We should probably
		 * have eg. GAMESTATE->GetOptionsForCourse(po,so,pn) to get options based on
		 * the last call to StoreSelectedOptions and the modifiers list, but that'd
		 * mean moving the queues in ScreenGameplay to GameState ... */
		NoteDataUtil::TransformNoteData( nd, GAMESTATE->m_PlayerOptions[pn_], pSteps->m_StepsType );
		RadarValues rvPost;
		NoteDataUtil::GetRadarValues( nd, pSong->m_fMusicLengthSeconds, rvPost );
		 
		iTotalPossibleDancePoints += this->GetPossibleDancePoints( rvPre, rvPost );
	}
	g_CurStageStats.iPossibleDancePoints[pn_] = iTotalPossibleDancePoints;


	m_iScoreRemainder = 0;
	m_iCurToastyCombo = 0; 
	m_iMaxScoreSoFar = 0;
	m_iPointBonus = 0;
	m_iNumTapsAndHolds = 0;
	m_bIsLastSongInCourse = false;

	memset( m_ComboBonusFactor, 0, sizeof(m_ComboBonusFactor) );
	switch( PREFSMAN->m_iScoringType )
	{
	case PrefsManager::SCORING_MAX2:
		m_iRoundTo = 1;
		break;
	case PrefsManager::SCORING_5TH:
		m_iRoundTo = 5;
		if (!GAMESTATE->IsCourseMode())
		{
			m_ComboBonusFactor[TNS_MARVELOUS] = 55;
			m_ComboBonusFactor[TNS_PERFECT] = 55;
			m_ComboBonusFactor[TNS_GREAT] = 33;
		}
		break;
	default:
		ASSERT(0);
	}

}

void ScoreKeeperMAX2::OnNextSong( int iSongInCourseIndex, const Steps* pSteps, const NoteData* pNoteData )
{
/*
  http://www.aaroninjapan.com/ddr2.html

  Note on NONSTOP Mode scoring

  Nonstop mode requires the player to play 4 songs in succession, with the total maximum possible score for the four song set being 100,000,000. This comes from the sum of the four stages' maximum possible scores, which, regardless of song or difficulty is: 

  10,000,000 for the first song
  20,000,000 for the second song
  30,000,000 for the third song
  40,000,000 for the fourth song

  We extend this to work with nonstop courses of any length.

  We also keep track of this scoring type in endless, with 100mil per iteration
  of all songs, though this score isn't actually seen anywhere right now.
*/
	//
	// Calculate the score multiplier
	//
	m_iMaxPossiblePoints = 0;
	if( GAMESTATE->IsCourseMode() )
	{
		const int numSongsInCourse = apSteps.size();
		ASSERT( numSongsInCourse != 0 );

		const int iIndex = iSongInCourseIndex % numSongsInCourse;
		m_bIsLastSongInCourse = (iIndex+1 == numSongsInCourse);

		if( numSongsInCourse < 10 )
		{
			const int courseMult = (numSongsInCourse * (numSongsInCourse + 1)) / 2;
			ASSERT(courseMult >= 0);

			m_iMaxPossiblePoints = (100000000 * (iIndex+1)) / courseMult;
		}
		else
		{
			/* When we have lots of songs, the scale above biases too much: in a
			 * course with 50 songs, the first song is worth 80k, the last 4mil, which
			 * is too much of a difference.
			 *
			 * With this, each song in a 50-song course will be worth 2mil. */
			m_iMaxPossiblePoints = 100000000 / numSongsInCourse;
		}
	}
	else
	{
		const int iMeter = clamp( pSteps->GetMeter(), 1, 10 );

		// long ver and marathon ver songs have higher max possible scores
		int iLengthMultiplier = SongManager::GetNumStagesForSong( GAMESTATE->m_pCurSong );
		switch( PREFSMAN->m_iScoringType )
		{
		case PrefsManager::SCORING_MAX2:
			m_iMaxPossiblePoints = iMeter * 10000000 * iLengthMultiplier;
			break;
		case PrefsManager::SCORING_5TH:
			m_iMaxPossiblePoints = (iMeter * iLengthMultiplier + 1) * 5000000;
			break;
		default:
			ASSERT(0);
		}
	}
	ASSERT( m_iMaxPossiblePoints >= 0 );
	m_iMaxScoreSoFar += m_iMaxPossiblePoints;

	m_iNumTapsAndHolds = pNoteData->GetNumRowsWithTapOrHoldHead() + pNoteData->GetNumHoldNotes();

	m_iPointBonus = m_iMaxPossiblePoints;

	ASSERT( m_iPointBonus >= 0 );

	m_iTapNotesHit = 0;
}

static int GetScore(int p, int B, int S, int n)
{
	/* There's a problem with the scoring system described below.  B/S is truncated
	 * to an int.  However, in some cases we can end up with very small base scores.
	 * Each song in a 50-song nonstop course will be worth 2mil, which is a base of
	 * 200k; B/S will end up being zero.
	 *
	 * If we rearrange the equation to (p*B*n) / S, this problem goes away.
	 * (To do that, we need to either use 64-bit ints or rearrange it a little
	 * more and use floats, since p*B*n won't fit a 32-bit int.)  However, this
	 * changes the scoring rules slightly.
	 */

#if 0
	/* This is the actual method described below. */
	return p * (B / S) * n;
#elif 1
	/* This doesn't round down B/S. */
	return int(int64_t(p) * n * B / S);
#else
	/* This also doesn't round down B/S. Use this if you don't have 64-bit ints. */
	return int(p * n * (float(B) / S));
#endif

}

void ScoreKeeperMAX2::AddScore( TapNoteScore score )
{
	int &iScore = g_CurStageStats.iScore[m_PlayerNumber];
/*
  http://www.aaroninjapan.com/ddr2.html

  Regular scoring:

  Let p = score multiplier (Perfect = 10, Great = 5, other = 0)
  
  Note on NONSTOP Mode scoring

  Let p = score multiplier (Marvelous = 10, Perfect = 9, Great = 5, other = 0)

  N = total number of steps and freeze steps
  S = The sum of all integers from 1 to N (the total number of steps/freeze steps) 
  n = number of the current step or freeze step (varies from 1 to N)
  B = Base value of the song (1,000,000 X the number of feet difficulty) - All edit data is rated as 5 feet
  So, the score for one step is: 
  one_step_score = p * (B/S) * n 
  
  *IMPORTANT* : Double steps (U+L, D+R, etc.) count as two steps instead of one *for your combo count only*, 
  so if you get a double L+R on the 112th step of a song, you score is calculated for only one step, not two, 
  as the combo counter might otherwise imply.  
	
  Now, through simple algebraic manipulation:
  S = 1+...+N = (1+N)*N/2 (1 through N added together) 

  Okay, time for an example.  Suppose we wanted to calculate the step score of a "Great" on the 57th step of 
  a 441 step, 8-foot difficulty song (I'm just making this one up): 
  
  S = (1 + 441)*441 / 2
  = 194,222 / 2
  = 97,461
  StepScore = p * (B/S) * n
  = 5 * (8,000,000 / 97,461) * 57
  = 5 * (82) * 57 (The 82 is rounded down from 82.08411...)
  = 23,370
  
  Remember this is just the score for the step, not the cumulative score up to the 57th step. Also, please note that 
  I am currently checking into rounding errors with the system and if there are any, how they are resolved in the system. 
  
  Note: if you got all Perfect on this song, you would get (p=10)*B, which is 80,000,000. In fact, the maximum possible 
  score for any song is the number of feet difficulty X 10,000,000. 
*/
	int p = 0;	// score multiplier 

	switch( score )
	{
	case TNS_MARVELOUS:	p = 10;		break;
	case TNS_PERFECT:	p = GAMESTATE->ShowMarvelous()? 9:10; break;
	case TNS_GREAT:		p = 5;		break;
	default:			p = 0;		break;
	}

	m_iTapNotesHit++;

	const int N = m_iNumTapsAndHolds;
	const int sum = (N * (N + 1)) / 2;
	const int B = m_iMaxPossiblePoints/10;

	// Don't use a multiplier if the player has failed
	if( g_CurStageStats.bFailedEarlier[m_PlayerNumber] )
	{
		iScore += p;
		// make score evenly divisible by 5
		// only update this on the next step, to make it less *obvious*
		/* Round to the nearest 5, instead of always rounding down, so a base score
		 * of 9 will round to 10, not 5. */
		if (p > 0)
			iScore = ((iScore+2) / 5) * 5;
	}
	else
	{
		iScore += GetScore(p, B, sum, m_iTapNotesHit);
		const int &iCurrentCombo = g_CurStageStats.iCurCombo[m_PlayerNumber];
		g_CurStageStats.iBonus[m_PlayerNumber] += m_ComboBonusFactor[score] * iCurrentCombo;
	}

	/* Subtract the maximum this step could have been worth from the bonus. */
	m_iPointBonus -= GetScore(10, B, sum, m_iTapNotesHit);

	if ( m_iTapNotesHit == m_iNumTapsAndHolds && score >= TNS_PERFECT )
	{
		if (!g_CurStageStats.bFailedEarlier[m_PlayerNumber])
			iScore += m_iPointBonus;
		if ( m_bIsLastSongInCourse )
		{
			iScore += 100000000 - m_iMaxScoreSoFar;

			/* If we're in Endless mode, we'll come around here again, so reset
			 * the bonus counter. */
			m_iMaxScoreSoFar = 0;
		}
	}

	ASSERT( iScore >= 0 );

	/* Undo rounding from the last tap, and re-round. */
	iScore += m_iScoreRemainder;
	m_iScoreRemainder = (iScore % m_iRoundTo);
	iScore = iScore - m_iScoreRemainder;
	
	ASSERT( iScore >= 0 );

	printf( "score: %i\n", iScore );
}

void ScoreKeeperMAX2::HandleTapScore( TapNoteScore score )
{
	if( score == TNS_HIT_MINE )
	{
		if( GAMESTATE->m_HealthState[m_PlayerNumber] != GameState::DEAD )
			g_CurStageStats.iActualDancePoints[m_PlayerNumber] += TapNoteScoreToDancePoints( TNS_HIT_MINE );
		g_CurStageStats.iTapNoteScores[m_PlayerNumber][TNS_HIT_MINE] += 1;
	}
}

void ScoreKeeperMAX2::HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow )
{
	ASSERT( iNumTapsInRow >= 1 );

	// Update dance points.
	if( GAMESTATE->m_HealthState[m_PlayerNumber] != GameState::DEAD )
		g_CurStageStats.iActualDancePoints[m_PlayerNumber] += TapNoteScoreToDancePoints( scoreOfLastTap );
	// update judged row totals
	g_CurStageStats.iTapNoteScores[m_PlayerNumber][scoreOfLastTap] += 1;

	//
	// Regular combo
	//
	
/*
  http://www.aaroninjapan.com/ddr2.html

  Note on ONI Mode scoring
  
  Your combo counter only increased with a "Marvelous/Perfect", and double Marvelous/Perfect steps (left and right, etc.)
  only add 1 to your combo instead of 2. The combo counter thus becomes a "Marvelous/Perfect" counter. 
*/
	/* True if a jump is one to combo, false if combo is purely based on tap count. */
	bool ComboIsPerRow = true;
	switch( PREFSMAN->m_iScoringType )
	{
	case PrefsManager::SCORING_MAX2:
		ComboIsPerRow = (GAMESTATE->m_PlayMode == PLAY_MODE_ONI);
		break;
	case PrefsManager::SCORING_5TH:
		ComboIsPerRow = true;
		break;
	default:
		ASSERT(0);
	}
	const int ComboCountIfHit = ComboIsPerRow? 1: iNumTapsInRow;
	TapNoteScore MinScoreToContinueCombo = GAMESTATE->m_PlayMode == PLAY_MODE_ONI? TNS_PERFECT:TNS_GREAT;

	if( scoreOfLastTap >= MinScoreToContinueCombo )
		g_CurStageStats.iCurCombo[m_PlayerNumber] += ComboCountIfHit;

	AddScore( scoreOfLastTap );		// only score once per row

	//
	// handle combo logic
	//
#ifndef DEBUG
	if( PREFSMAN->m_bAutoPlay && !GAMESTATE->m_bDemonstrationOrJukebox )	// cheaters never prosper
	{
		m_iCurToastyCombo = 0;
		return;
	}
#endif //DEBUG

	//
	// Toasty combo
	//
	switch( scoreOfLastTap )
	{
	case TNS_MARVELOUS:
	case TNS_PERFECT:
		m_iCurToastyCombo += iNumTapsInRow;

		if( m_iCurToastyCombo >= 250 &&
			m_iCurToastyCombo - iNumTapsInRow < 250 &&
			!GAMESTATE->m_bDemonstrationOrJukebox )
		{
			SCREENMAN->PostMessageToTopScreen( SM_PlayToasty, 0 );
			PROFILEMAN->IncrementToastiesCount( m_PlayerNumber );
		}
		break;
	default:
		m_iCurToastyCombo = 0;
		break;
	}

	
	NSMAN->ReportScore(m_PlayerNumber, scoreOfLastTap,
                       g_CurStageStats.iScore[m_PlayerNumber],
                       g_CurStageStats.iCurCombo[m_PlayerNumber]);
}


void ScoreKeeperMAX2::HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore )
{
	// update dance points totals
	if( GAMESTATE->m_HealthState[m_PlayerNumber] != GameState::DEAD )
		g_CurStageStats.iActualDancePoints[m_PlayerNumber] += HoldNoteScoreToDancePoints( holdScore );
	g_CurStageStats.iHoldNoteScores[m_PlayerNumber][holdScore] ++;

	if( holdScore == HNS_OK )
		AddScore( TNS_MARVELOUS );

	NSMAN->ReportScore(m_PlayerNumber, holdScore+7, 
                       g_CurStageStats.iScore[m_PlayerNumber],
                       g_CurStageStats.iCurCombo[m_PlayerNumber]);


}


int ScoreKeeperMAX2::GetPossibleDancePoints( const RadarValues& radars )
{
	/* Note that, if Marvelous timing is disabled or not active (not course mode),
	 * PERFECT will be used instead. */

	int NumTaps = int(radars[RADAR_NUM_TAPS_AND_HOLDS]);
	int NumHolds = int(radars[RADAR_NUM_HOLDS]); 
	return NumTaps*TapNoteScoreToDancePoints(TNS_MARVELOUS)+
	   NumHolds*HoldNoteScoreToDancePoints(HNS_OK);
}

int ScoreKeeperMAX2::GetPossibleDancePoints( const RadarValues& fOriginalRadars, const RadarValues& fPostRadars )
{
	/*
	 * The logic here is that if you use a modifier that adds notes, you should have to
	 * hit the new notes to get a high grade.  However, if you use one that removes notes,
	 * they should simply be counted as misses. */
	return max( 
		GetPossibleDancePoints(fOriginalRadars),
		GetPossibleDancePoints(fPostRadars) );
}


int ScoreKeeperMAX2::TapNoteScoreToDancePoints( TapNoteScore tns )
{
	if( !GAMESTATE->ShowMarvelous() && tns == TNS_MARVELOUS )
		tns = TNS_PERFECT;

/*
  http://www.aaroninjapan.com/ddr2.html

  Note on ONI Mode scoring

  The total number of Dance Points is calculated with Marvelous steps being worth 3 points, Perfects getting 
  2 points, OKs getting 3 points, Greats getting 1 point, and everything else is worth 0 points. (Note: The 

*/

	/* This is used for Oni percentage displays.  Grading values are currently in
	 * StageStats::GetGrade. */
	switch( tns )
	{
	case TNS_NONE:		return 0;
	case TNS_HIT_MINE:	return PREFSMAN->m_iPercentScoreWeightHitMine;
	case TNS_MISS:		return PREFSMAN->m_iPercentScoreWeightMiss;
	case TNS_BOO:		return PREFSMAN->m_iPercentScoreWeightBoo;
	case TNS_GOOD:		return PREFSMAN->m_iPercentScoreWeightGood;
	case TNS_GREAT:		return PREFSMAN->m_iPercentScoreWeightGreat;
	case TNS_PERFECT:	return PREFSMAN->m_iPercentScoreWeightPerfect;
	case TNS_MARVELOUS:	return PREFSMAN->m_iPercentScoreWeightMarvelous;
	default: FAIL_M( ssprintf("%i", tns) );
	}
}

int ScoreKeeperMAX2::HoldNoteScoreToDancePoints( HoldNoteScore hns )
{
	switch( hns )
	{
	case HNS_NONE: return 0;
	case HNS_NG: return PREFSMAN->m_iPercentScoreWeightNG;
	case HNS_OK: return PREFSMAN->m_iPercentScoreWeightOK;
	default: FAIL_M( ssprintf("%i", hns) );
	}
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
