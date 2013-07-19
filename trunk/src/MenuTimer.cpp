#include "global.h"
#include "MenuTimer.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "AnnouncerManager.h"
#include "ThemeManager.h"
#include "Font.h"
#include "GameSoundManager.h"

#define WARNING_START				THEME->GetMetricI("MenuTimer","WarningStart")
#define WARNING_BEEP_START			THEME->GetMetricI("MenuTimer","WarningBeepStart")
#define WARNING_COMMAND(i)			THEME->GetMetric ("MenuTimer", ssprintf("WarningCommand%i",i))
#define ON_COMMAND					THEME->GetMetric ("MenuTimer","OnCommand")

static const int TIMER_SECONDS = 99;
static const int MAX_STALL_SECONDS = 30;

MenuTimer::MenuTimer()
{
	m_fStallSeconds = 0;
	m_fStallSecondsLeft = MAX_STALL_SECONDS;
	m_bPaused = false;

	m_textDigit1.LoadFromFont( THEME->GetPathToF("MenuTimer") );
	m_textDigit2.LoadFromFont( THEME->GetPathToF("MenuTimer") );

	const float fCharWidth = (float) m_textDigit1.m_pFont->GetLineWidthInSourcePixels(L"0");
	m_textDigit1.SetX( -fCharWidth/2 );
	m_textDigit2.SetX( +fCharWidth/2 );

	this->AddChild( &m_textDigit1 );
	this->AddChild( &m_textDigit2 );

	SetSeconds( TIMER_SECONDS );

	m_soundBeep.Load( THEME->GetPathToS("MenuTimer tick") );
}

void MenuTimer::EnableStealth( bool bStealth )
{
	if( bStealth )
		m_soundBeep.Unload(); // unload the sound
	else
		m_soundBeep.Load( THEME->GetPathToS("MenuTimer tick") ); // reload the sound

	m_textDigit1.SetHidden( bStealth );	
	m_textDigit2.SetHidden( bStealth );
}

void MenuTimer::Update( float fDeltaTime ) 
{ 
	ActorFrame::Update( fDeltaTime );

	if( m_bPaused )
		return;

	// run down the stall time if any
	if( m_fStallSeconds > 0 )
		m_fStallSeconds = max( m_fStallSeconds - fDeltaTime, 0 );
	if( m_fStallSeconds > 0 )
		return;

	const float fOldSecondsLeft = m_fSecondsLeft;
	m_fSecondsLeft -= fDeltaTime;
	CLAMP( m_fSecondsLeft, 0, 99 );
	const float fNewSecondsLeft = m_fSecondsLeft;

	const int iOldDisplay = (int)(fOldSecondsLeft + 0.99f);
	const int iNewDisplay = (int)(fNewSecondsLeft + 0.99f);

	if( fOldSecondsLeft > 5.5  &&  fNewSecondsLeft < 5.5 )	// transition to below 5.5
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("hurry up") );

	if( iOldDisplay == iNewDisplay )
		return;

	SetText( iNewDisplay );

	if( iNewDisplay <= WARNING_START )
	{
		m_textDigit1.Command( WARNING_COMMAND(iNewDisplay) );
		m_textDigit2.Command( WARNING_COMMAND(iNewDisplay) );
	}
	
	if( iNewDisplay == 0 )
	{
		Stop();
		SCREENMAN->PostMessageToTopScreen( SM_MenuTimer, 0 );
		m_textDigit1.SetEffectNone();
		m_textDigit2.SetEffectNone();
	}

	if( iNewDisplay <= WARNING_BEEP_START )
		m_soundBeep.Play();
}


void MenuTimer::Pause()
{
	m_bPaused = true;
}

void MenuTimer::Stop()
{
	/* Don't call SetSeconds if we're already at 0: let the existing tweens finish. */
	if( m_fSecondsLeft >= 1 )
		SetSeconds( 0 );
	Pause();
}

void MenuTimer::Disable()
{
	SetSeconds( 99 );
	Pause();
}

void MenuTimer::Stall()
{
	/* Max amount of stall time we'll use: */
	const float Amt = min( 0.5f, m_fStallSecondsLeft );

	/* Amount of stall time to add: */
	const float ToAdd = Amt - m_fStallSeconds;

	m_fStallSeconds += ToAdd;
	m_fStallSecondsLeft -= ToAdd;
}

void MenuTimer::SetSeconds( int iSeconds )
{
	m_fSecondsLeft = (float)iSeconds;
	CLAMP( m_fSecondsLeft, 0, 99 );

	m_textDigit1.Command( ON_COMMAND );
	m_textDigit2.Command( ON_COMMAND );

	SetText( iSeconds );
}

void MenuTimer::Start()
{
	m_bPaused = false;
}

void MenuTimer::SetText( int iSeconds )
{
	const int iDigit1 = (int)(iSeconds)/10;
	const int iDigit2 = (int)(iSeconds)%10;

	m_textDigit1.SetText( ssprintf("%d",iDigit1) ); 
	m_textDigit2.SetText( ssprintf("%d",iDigit2) ); 
}

/*
 * (c) 2002-2004 Chris Danford
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
