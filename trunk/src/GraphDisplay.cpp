#include "global.h"
#include "GraphDisplay.h"
#include "ThemeManager.h"
#include "RageTextureManager.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "StageStats.h"

//#define DIVIDE_LINE_WIDTH			THEME->GetMetricI(m_sName,"TexturedBottomHalf")

GraphDisplay::GraphDisplay()
{
	m_pTexture = NULL;
}


void GraphDisplay::Load( CString TexturePath, float height )
{
	m_Position = 1;
	memset( m_CurValues, 0, sizeof(m_CurValues) );
	memset( m_DestValues, 0, sizeof(m_DestValues) );
	memset( m_LastValues, 0, sizeof(m_LastValues) );

	Unload();
	m_pTexture = TEXTUREMAN->LoadTexture( TexturePath );
	m_size.x = (float) m_pTexture->GetSourceWidth();
	m_size.y = (float) m_pTexture->GetSourceHeight();

	for( int i = 0; i < VALUE_RESOLUTION; ++i )
		m_CurValues[i] = height;
	UpdateVerts();
}

void GraphDisplay::Unload()
{
	if( m_pTexture != NULL )
		TEXTUREMAN->UnloadTexture( m_pTexture );

	m_pTexture = NULL;
}

void GraphDisplay::LoadFromStageStats( const StageStats &s, PlayerNumber pn )
{
	memcpy( m_LastValues, m_CurValues, sizeof(m_CurValues) );
	m_Position = 0;
	s.GetLifeRecord( pn, m_DestValues, VALUE_RESOLUTION );
	for( unsigned i=0; i<ARRAYSIZE(m_DestValues); i++ )
		CLAMP( m_DestValues[i], 0.f, 1.f );
	UpdateVerts();
}

void GraphDisplay::UpdateVerts()
{
	RectF quadVertices;

	switch( m_HorizAlign )
	{
	case align_left:	quadVertices.left = 0;				quadVertices.right = m_size.x;		break;
	case align_center:	quadVertices.left = -m_size.x/2;	quadVertices.right = m_size.x/2;	break;
	case align_right:	quadVertices.left = -m_size.x;		quadVertices.right = 0;			break;
	default:		ASSERT( false );
	}

	switch( m_VertAlign )
	{
	case align_top:		quadVertices.top = 0;				quadVertices.bottom = m_size.y;	break;
	case align_middle:	quadVertices.top = -m_size.y/2;		quadVertices.bottom = m_size.y/2;	break;
	case align_bottom:	quadVertices.top = -m_size.y;		quadVertices.bottom = 0;			break;
	default:		ASSERT(0);
	}

	int NumSlices = VALUE_RESOLUTION-1;

	int i;
	for( i = 0; i < 4*NumSlices; ++i )
		Slices[i].c = RageColor(1,1,1,1);

	for( i = 0; i < NumSlices; ++i )
	{
		const float Left = SCALE( float(i), 0.0f, float(NumSlices), quadVertices.left, quadVertices.right );
		const float Right = SCALE( float(i+1), 0.0f, float(NumSlices), quadVertices.left, quadVertices.right );
		const float LeftTop = SCALE( float(m_CurValues[i]), 0.0f, 1.0f, quadVertices.bottom, quadVertices.top );
		const float RightTop = SCALE( float(m_CurValues[i+1]), 0.0f, 1.0f, quadVertices.bottom, quadVertices.top );

		Slices[i*4+0].p = RageVector3( Left,		LeftTop,	0 );	// top left
		Slices[i*4+1].p = RageVector3( Left,		quadVertices.bottom,	0 );	// bottom left
		Slices[i*4+2].p = RageVector3( Right,		quadVertices.bottom,	0 );	// bottom right
		Slices[i*4+3].p = RageVector3( Right,		RightTop,	0 );	// top right

	//	Slices[i*4+0].c = RageColor(.2,.2,.2,1);
	//	Slices[i*4+1].c = RageColor(1,1,1,1);
	//	Slices[i*4+2].c = RageColor(1,1,1,1);
	//	Slices[i*4+3].c = RageColor(.2,.2,.2,1);
	}

	const RectF *tex = m_pTexture->GetTextureCoordRect( 0 );
	for( i = 0; i < NumSlices; ++i )
	{
		const float Left = SCALE( float(i), 0.0f, float(NumSlices), tex->left, tex->right );
		const float Right = SCALE( float(i+1), 0.0f, float(NumSlices), tex->left, tex->right );
		const float LeftTop = SCALE( float(m_CurValues[i]), 0.0f, 1.0f, tex->bottom, tex->top );
		const float RightTop = SCALE( float(m_CurValues[i+1]), 0.0f, 1.0f, tex->bottom, tex->top );
		
		Slices[i*4+0].t = RageVector2( Left,		LeftTop );	// top left
		Slices[i*4+1].t = RageVector2( Left,		tex->bottom );	// bottom left
		Slices[i*4+2].t = RageVector2( Right,		tex->bottom );	// bottom right
		Slices[i*4+3].t = RageVector2( Right,		RightTop );	// top right
	}
}

void GraphDisplay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( m_Position == 1 )
		return;

	m_Position = clamp( m_Position+fDeltaTime, 0, 1 );
	for( int i = 0; i < VALUE_RESOLUTION; ++i )
		m_CurValues[i] = m_DestValues[i]*m_Position + m_LastValues[i]*(1-m_Position);

	UpdateVerts();
}

void GraphDisplay::DrawPrimitives()
{
	DISPLAY->ClearAllTextures();
	DISPLAY->SetTexture( 0, m_pTexture );
	Actor::SetRenderStates();	// set Actor-specified render states

	DISPLAY->DrawQuads( Slices, ARRAYSIZE(Slices) );
	DISPLAY->SetTexture( 0, NULL );
}

/*
 * (c) 2003 Glenn Maynard
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
