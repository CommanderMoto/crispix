/* ScreenSelectMusic - Choose a Song and Steps. */

#ifndef SCREEN_SELECT_MUSIC_H
#define SCREEN_SELECT_MUSIC_H

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "Banner.h"
#include "FadingBanner.h"
#include "BPMDisplay.h"
#include "GrooveRadar.h"
#include "GrooveGraph.h"
#include "DifficultyIcon.h"
#include "DifficultyMeter.h"
#include "OptionIconRow.h"
#include "DifficultyDisplay.h"
#include "DifficultyList.h"
#include "CourseContentsList.h"
#include "HelpDisplay.h"
#include "PaneDisplay.h"
#include "Character.h"
#include "BGAnimation.h"
#include "RageUtil_BackgroundLoader.h"

class ScreenSelectMusic : public ScreenWithMenuElements
{
public:
	ScreenSelectMusic( CString sName );
	virtual ~ScreenSelectMusic();

	virtual void DrawPrimitives();

	virtual void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuBack( PlayerNumber pn );

protected:
	void TweenOnScreen();
	void TweenOffScreen();
	void TweenScoreOnAndOffAfterChangeSort();
	enum DisplayMode { DISPLAY_SONGS, DISPLAY_COURSES, DISPLAY_MODES } m_DisplayMode;
	void SwitchDisplayMode( DisplayMode dm );
	void TweenSongPartsOnScreen( bool Initial );
	void TweenSongPartsOffScreen( bool Final );
	void TweenCoursePartsOnScreen( bool Initial );
	void TweenCoursePartsOffScreen( bool Final );
	void SkipSongPartTweens();
	void SkipCoursePartTweens();

	void ChangeDifficulty( PlayerNumber pn, int dir );

	void AfterStepsChange( PlayerNumber pn );
	void AfterTrailChange( PlayerNumber pn );
	void SwitchToPreferredDifficulty();
	void AfterMusicChange();
	void SortOrderChanged();

	void UpdateOptionsDisplays();
	void CheckBackgroundRequests();

	vector<Steps*>		m_vpSteps;
	vector<Trail*>		m_vpTrails;
	int					m_iSelection[NUM_PLAYERS];

	Sprite				m_sprCharacterIcon[NUM_PLAYERS];
	Sprite				m_sprBannerMask;
	FadingBanner		m_Banner;
	AutoActor			m_sprBannerFrame;
	AutoActor			m_sprExplanation;
	BPMDisplay			m_BPMDisplay;
	Sprite				m_sprStage;
	Sprite				m_sprCDTitleFront, m_sprCDTitleBack;
	Sprite				m_sprDifficultyFrame[NUM_PLAYERS];
	DifficultyIcon		m_DifficultyIcon[NUM_PLAYERS];
	Sprite				m_AutoGenIcon[NUM_PLAYERS];
    GrooveRadar			m_GrooveRadar;
    GrooveGraph			m_GrooveGraph;
	BitmapText			m_textSongOptions;
	BitmapText			m_textNumSongs;
	BitmapText			m_textTotalTime;
	OptionIconRow		m_OptionIconRow[NUM_PLAYERS];
	Sprite				m_sprMeterFrame[NUM_PLAYERS];
	Sprite				m_sprNonPresence[NUM_PLAYERS];
	DifficultyMeter			m_DifficultyMeter[NUM_PLAYERS];
	MusicSortDisplay	m_MusicSortDisplay;
	Sprite				m_sprHighScoreFrame[NUM_PLAYERS];
	BitmapText			m_textHighScore[NUM_PLAYERS];
	MusicWheel			m_MusicWheel;
	AutoActor			m_MusicWheelUnder;
	Sprite				m_sprBalloon;
	AutoActor			m_sprCourseHasMods;
	DifficultyDisplay   m_DifficultyDisplay;
	DifficultyList		m_DifficultyList;
	CourseContentsList	m_CourseContentsFrame;
	HelpDisplay			m_Artist;
	BitmapText			m_MachineRank;
	PaneDisplay			m_PaneDisplay[NUM_PLAYERS];

	bool				m_bMadeChoice;
	bool				m_bGoToOptions;
	Sprite				m_sprOptionsMessage;
	CString				m_sSampleMusicToPlay;
	TimingData			*m_pSampleMusicTimingData;
	float				m_fSampleStartSeconds, m_fSampleLengthSeconds;
	bool				m_bAllowOptionsMenu, m_bAllowOptionsMenuRepeat;

	BGAnimation			m_Overlay;
	Transition			m_bgOptionsOut;
	Transition			m_bgNoOptionsOut;

	RageSound			m_soundDifficultyEasier;
	RageSound			m_soundDifficultyHarder;
	RageSound			m_soundOptionsChange;
	RageSound			m_soundLocked;

	BackgroundLoader	m_BackgroundLoader;
};

#endif

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
