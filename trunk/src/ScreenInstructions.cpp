#include "global.h"
#include "ScreenInstructions.h"
#include "RageUtil.h"
#include "GameSoundManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PrefsManager.h"


ScreenInstructions::ScreenInstructions( CString sName ) : ScreenWithMenuElements( sName )
{
	LOG->Trace( "ScreenInstructions::ScreenInstructions()" );

	//
	// Skip this screen unless someone chose easy or beginner
	//
	if( !PREFSMAN->m_bInstructions )
	{
		HandleScreenMessage( SM_GoToNextScreen );
		return;
	}
	if( GAMESTATE->m_PlayMode == PLAY_MODE_REGULAR )
	{
		Difficulty easiestDifficulty = (Difficulty)(NUM_DIFFICULTIES-1);
		FOREACH_HumanPlayer(p)
			easiestDifficulty = min( easiestDifficulty, GAMESTATE->m_PreferredDifficulty[p] );

		if( easiestDifficulty > DIFFICULTY_EASY )
		{
			HandleScreenMessage( SM_GoToNextScreen );
			return;
		}
	}

	if( GAMESTATE->m_PlayMode == PLAY_MODE_INVALID )
		RageException::Throw( "The PlayMode has not been set.  A theme must set the PlayMode before showing ScreenInstructions." );

	m_sprHowToPlay.Load( THEME->GetPathG(m_sName,PlayModeToString(GAMESTATE->m_PlayMode)) );
	m_sprHowToPlay.SetXY( CENTER_X, CENTER_Y );
	this->AddChild( &m_sprHowToPlay );

	m_sprHowToPlay.SetX( SCREEN_LEFT-SCREEN_WIDTH );
	m_sprHowToPlay.BeginTweening( 0.4f );		// sleep
	m_sprHowToPlay.BeginTweening( 0.6f, Actor::TWEEN_DECELERATE );
	m_sprHowToPlay.SetX( CENTER_X );

	this->SortByDrawOrder();

	SOUND->PlayMusic( THEME->GetPathS(m_sName,"music") );
}

void ScreenInstructions::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( IsTransitioning() )
		return;

	// default input handler
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenInstructions::MenuBack( PlayerNumber pn )
{
	Back( SM_GoToPrevScreen );
}

void ScreenInstructions::MenuStart( PlayerNumber pn )
{
	StartTransitioning( SM_GoToNextScreen );

	m_sprHowToPlay.StopTweening();
	m_sprHowToPlay.BeginTweening( 0.3f, Actor::TWEEN_ACCELERATE );
	m_sprHowToPlay.SetX( SCREEN_RIGHT+m_sprHowToPlay.GetUnzoomedWidth()/2 );
}

/*
 * (c) 2001-2004 Chris Danford
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
