#include "global.h"
#include "ScreenCredits.h"
#include "GameSoundManager.h"
#include "RageLog.h"
#include "SongManager.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "Sprite.h"
#include "song.h"
#include "BitmapText.h"
#include "ActorUtil.h"
#include "SongUtil.h"


#define BACKGROUNDS_SPACING_X				THEME->GetMetricF("ScreenCredits","BackgroundsSpacingX")
#define BACKGROUNDS_SPACING_Y				THEME->GetMetricF("ScreenCredits","BackgroundsSpacingY")
#define BACKGROUNDS_SCROLL_SECONDS_PER_ITEM	THEME->GetMetricF("ScreenCredits","BackgroundsScrollSecondsPerItem")
#define BACKGROUNDS_WIDTH					THEME->GetMetricF("ScreenCredits","BackgroundsWidth")
#define BACKGROUNDS_HEIGHT					THEME->GetMetricF("ScreenCredits","BackgroundsHeight")
#define TEXTS_COLOR_INTRO					THEME->GetMetricC("ScreenCredits","TextsColorIntro")
#define TEXTS_COLOR_HEADER					THEME->GetMetricC("ScreenCredits","TextsColorHeader")
#define TEXTS_COLOR_NORMAL					THEME->GetMetricC("ScreenCredits","TextsColorNormal")
#define TEXTS_ZOOM							THEME->GetMetricF("ScreenCredits","TextsZoom")
#define TEXTS_SPACING_X						THEME->GetMetricF("ScreenCredits","TextsSpacingX")
#define TEXTS_SPACING_Y						THEME->GetMetricF("ScreenCredits","TextsSpacingY")
#define TEXTS_SCROLL_SECONDS_PER_ITEM		THEME->GetMetricF("ScreenCredits","TextsScrollSecondsPerItem")

const int NUM_BACKGROUNDS = 20;

struct CreditLine
{
	int colorIndex;
	const char* text;
};

static const CreditLine CREDIT_LINES[] = 
{
	{1, "DANCE DANCE IMMOLATION 2nd MIX" },
	{0, ""},
	{2, "☆☆by INTERPRETIVE ARSON☆☆"},
	{0, ""},
	{2, "☆☆MACHINE☆☆"},
	{0, "Matt Blackwell - IGNITION & VENTILATION"},
	{0, "Krispy Mahoney - BAD-ASS PYRO"},
	{0, "Ian Baker - PYRO ALUMNUS"},
	{0, "Sam Gawthrop - PYRO & LIGHTS"},
	{0, "Jonathan Shekter - WIRES & BITS"},
	{0, "Rubin Abdi - STEP MANIAC"},
	{0, "Steve Goodman - SCREEN"},
	{0,""},
	{2,"☆☆OPERATIONS☆☆"},
	{0,""},
	{0,"Rubin Abdi"},
	{0,"Kristen Anciewicz"},
	{0,"Amada"},
	{0,"Amanda B"},
	{0,"Matt Blackwell"},
	{0,"Brendan Coffey"},
	{0,"Graham Phaedrus Eleusis"},
	{0,"Bowen Dwelle"},
	{0,"Mary Franck"},
	{0,"David Fine"},
	{0,"Skippy Fox"},
	{0,"Sam Gawhtrop"},
	{0,"Steven Goodman"},
	{0,"Jared Grippe"},
	{0,"Robyn Herr"},
	{0,"Jenafir House"},
	{0,"Mojgan Khodadoust"},
	{0,"Dr. Lizard"},
	{0,"Krispy Mahoney"},
	{0,"Corey March"},
	{0,"Don McCasland"},
	{0,"Kateri McRae"},
	{0,"Oreo Luan"},
	{0,"Orkan"},
	{0,"Chris Ory"},
	{0,"Penny Pattison"},
	{0,"Jonathan Shekter"},
	{0,"Nina Ramos"},
	{0,"Leevil Uba T"},
	{0,""},
	{2, "☆☆1st MIX☆☆"},
	{0, "Morley John - CONCEPT & WRANGLING"},
	{0, "Ian Baker - PADS & PYRO"},
	{0,"Matt Blackwell - VENTILLATION & A/V"},
	{0,"Jonathan Shekter - SOFTWARE & ELECTRONICS"},
	{0,"Nicole Aptekar - GRAPHICS & STEPFILES"},
	{0,"Jared Grippe - WELDING & WEBSITE"},
	{0,"Reed Kennedy - CLEVER MONKEY"},
	{0,""},
	{0,""},	
	{2,"☆☆VOLUNTEERS☆☆"},
	{0,"Colin Fahrion"},
	{0,"FKO"},
	{0,"GXAOUI"},
	{0,"Heath"},
	{0,"Meg Leland"},
	{0,"Mella Piercey"},
	{0,"Platypus Man"},
	{0,""},
	{2,"☆☆BROUGHT TO YOU BY☆☆"},
	{0,"BORG2"},
	{0,"TASTEE FLAME"},
	{0,""},
	{2,"☆☆SPECIAL THANKS TO☆☆"},
	{0,"Jeff Waldeck"},
	{0,"Patrick Ames"},
	{0,"J9"},
	{0,"NIMBY"},
	{0,"The Crucible"},
	{0,"The StepMania Team"}
};
const unsigned NUM_CREDIT_LINES = sizeof(CREDIT_LINES) / sizeof(CString);

ScreenCredits::ScreenCredits( CString sName ) : ScreenAttract( sName )
{
	vector<Song*> arraySongs;
	SONGMAN->GetSongs( arraySongs );
	SongUtil::SortSongPointerArrayByTitle( arraySongs );

	// FIXME:  Redo this screen with a BGA
	m_ScrollerBackgrounds.SetName( "Backgrounds" );
	m_ScrollerBackgrounds.Load(
		BACKGROUNDS_SCROLL_SECONDS_PER_ITEM,
		4,
		RageVector3(0, 0, 0),
		RageVector3(0, 0, 0),
		RageVector3(BACKGROUNDS_SPACING_X, BACKGROUNDS_SPACING_Y, 0),
		RageVector3(0, 0, 0) );
	SET_XY( m_ScrollerBackgrounds );
	this->AddChild( &m_ScrollerBackgrounds );

	m_ScrollerFrames.SetName( "Backgrounds" );
	m_ScrollerFrames.Load(
		BACKGROUNDS_SCROLL_SECONDS_PER_ITEM,
		4,
		RageVector3(0, 0, 0),
		RageVector3(0, 0, 0),
		RageVector3(BACKGROUNDS_SPACING_X, BACKGROUNDS_SPACING_Y, 0),
		RageVector3(0, 0, 0) );
	SET_XY( m_ScrollerFrames );
	this->AddChild( &m_ScrollerFrames );

	float fTime = 0;
	{
		for( int i=0; i<NUM_BACKGROUNDS; i++ )
		{
			Song* pSong = NULL;
			for( int j=0; j<50; j++ )
			{
				pSong = arraySongs[ rand()%arraySongs.size() ];
				if( pSong->HasBackground() )
					break;
			}

			Sprite* pBackground = new Sprite;
			pBackground->LoadBG( pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathToG("Common fallback background") );
			pBackground->ScaleToClipped( BACKGROUNDS_WIDTH, BACKGROUNDS_HEIGHT );
			m_ScrollerBackgrounds.AddChild( pBackground );

			Sprite* pFrame = new Sprite;
			pFrame->Load( THEME->GetPathToG("ScreenCredits background frame") );
			m_ScrollerFrames.AddChild( pFrame );
		}
		const int iFirst = -2, iLast = NUM_BACKGROUNDS+2;
		m_ScrollerBackgrounds.SetCurrentAndDestinationItem( iFirst );
		m_ScrollerBackgrounds.SetDestinationItem( iLast );

		m_ScrollerFrames.SetCurrentAndDestinationItem( iFirst );
		m_ScrollerFrames.SetDestinationItem( iLast );

		fTime = max( fTime, BACKGROUNDS_SCROLL_SECONDS_PER_ITEM*(iLast-iFirst) );
	}
	
	m_ScrollerTexts.SetName( "Texts" );
	m_ScrollerTexts.Load(
		TEXTS_SCROLL_SECONDS_PER_ITEM,
		40,
		RageVector3(0, 0, 0),
		RageVector3(0, 0, 0),
		RageVector3(TEXTS_SPACING_X, TEXTS_SPACING_Y, 0),
		RageVector3(0, 0, 0) );
	SET_XY( m_ScrollerTexts );
	this->AddChild( &m_ScrollerTexts );
	
	{
		for( unsigned i=0; i<ARRAYSIZE(CREDIT_LINES); i++ )
		{
			BitmapText* pText = new BitmapText;
			pText->LoadFromFont( THEME->GetPathToF("ScreenCredits titles") );
			pText->SetText( CREDIT_LINES[i].text );
			switch( CREDIT_LINES[i].colorIndex )
			{
			case 1:	pText->SetDiffuse( TEXTS_COLOR_INTRO );		break;
			case 2:	pText->SetDiffuse( TEXTS_COLOR_HEADER );	break;
			case 0:	pText->SetDiffuse( TEXTS_COLOR_NORMAL );	break;
			default:	ASSERT(0);
			}
			pText->SetZoom( TEXTS_ZOOM );
			m_ScrollerTexts.AddChild( pText );
		}

		const int iFirst = -10, iLast = ARRAYSIZE(CREDIT_LINES)+10;
		m_ScrollerTexts.SetCurrentAndDestinationItem( iFirst );
		m_ScrollerTexts.SetDestinationItem( iLast );
		fTime = max( fTime, TEXTS_SCROLL_SECONDS_PER_ITEM*(iLast-iFirst) );
	}

	m_Overlay.LoadFromAniDir( THEME->GetPathToB("ScreenCredits overlay") );
	this->AddChild( &m_Overlay );

	this->MoveToTail( &m_In );		// put it in the back so it covers up the stuff we just added
	this->MoveToTail( &m_Out );		// put it in the back so it covers up the stuff we just added

	this->ClearMessageQueue( SM_BeginFadingOut );	// ignore ScreenAttract's SecsToShow
	LOG->Trace("XXXXXXXXX %f", fTime);
	this->PostScreenMessage( SM_BeginFadingOut, fTime );
//	this->PostScreenMessage( SM_BeginFadingOut, m_Background.GetLengthSeconds() );
	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("credits") );
}

ScreenCredits::~ScreenCredits()
{
	m_ScrollerBackgrounds.DeleteAllChildren();
	m_ScrollerFrames.DeleteAllChildren();
	m_ScrollerTexts.DeleteAllChildren();
}


void ScreenCredits::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenAttract::HandleScreenMessage( SM );
}

/*
 * (c) 2003-2004 Chris Danford, Glenn Maynard
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
