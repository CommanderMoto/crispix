/* MusicWheelItem - An item on the music wheel. */

#ifndef MUSICWHEELITEM_H
#define MUSICWHEELITEM_H

#include "ActorFrame.h"
#include "GradeDisplay.h"
#include "BitmapText.h"
#include "WheelNotifyIcon.h"
#include "TextBanner.h"
#include "GameConstantsAndTypes.h"
#include "ModeChoice.h"
class Course;
class Song;

struct WheelItemData;

class MusicWheelItem : public ActorFrame
{
public:
	MusicWheelItem();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void SetZTestMode( ZTestMode mode );
	virtual void SetZWrite( bool b );
	
	void LoadFromWheelItemData( WheelItemData* pWID );
	void RefreshGrades();

	WheelItemData *data;
	float				m_fPercentGray;

	// for TYPE_SECTION and TYPE_ROULETTE
	Sprite				m_sprSectionBar;
	// for TYPE_SECTION
	BitmapText			m_textSectionName;
	// for TYPE_ROULETTE
	BitmapText			m_textRoulette;

	// for a TYPE_MUSIC
	Sprite				m_sprSongBar;
	WheelNotifyIcon		m_WheelNotifyIcon;
	TextBanner			m_TextBanner;
	GradeDisplay		m_GradeDisplay[NUM_PLAYERS];

	// for TYPE_COURSE
	BitmapText			m_textCourse;

	// for TYPE_SORT
	BitmapText			m_textSort;
	ActorFrame m_All;
};

enum WheelItemType 
{
	TYPE_SECTION, 
	TYPE_SONG, 
	TYPE_ROULETTE, 
	TYPE_RANDOM, 
	TYPE_PORTAL, 
	TYPE_COURSE, 
	TYPE_SORT 
};

struct WheelItemData
{
	WheelItemData() {}
	WheelItemData( WheelItemType wit, Song* pSong, CString sSectionName, Course* pCourse, RageColor color, SortOrder so );

	WheelItemType	m_Type;
	CString			m_sSectionName;
	Course*			m_pCourse;
	Song*			m_pSong;
	RageColor		m_color;	// either text color or section background color
	WheelNotifyIcon::Flags  m_Flags;

	// for TYPE_SORT
	CString			m_sLabel;
	ModeChoice		m_Action;
	SortOrder	m_SortOrder;
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Chris Gomez, Glenn Maynard
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
