/* ActorScroller - ActorFrame that moves its children. */

#ifndef ActorScroller_H
#define ActorScroller_H

#include "ActorFrame.h"


class ActorScroller : public ActorFrame
{
public:
	ActorScroller();

	void Load( 
		float fScrollSecondsPerItem, 
		int iNumItemsToDraw, 
		const RageVector3	&vRotationDegrees,
		const RageVector3	&vTranslateTerm0,
		const RageVector3	&vTranslateTerm1,
		const RageVector3	&vTranslateTerm2 );

	virtual void Update( float fDelta );
	virtual void DrawPrimitives();	// DOES draw

	void SetDestinationItem( int iItem ) { m_fDestinationItem = float(iItem); }
	void SetCurrentAndDestinationItem( int iItem ) { m_fCurrentItem = m_fDestinationItem = float(iItem); }

protected:
	bool		m_bLoaded;
	float		m_fCurrentItem; // usually between 0 and m_SubActors.size()
	float		m_fDestinationItem;
	float		m_fSecondsPerItem;
	int			m_iNumItemsToDraw;
	
	// Note: Rotation is applied before translation.

	// rot = m_vRotationDegrees*itemOffset^1
	RageVector3	m_vRotationDegrees;

	// trans = m_vTranslateTerm0*itemOffset^0 + 
	//		   m_vTranslateTerm1*itemOffset^1 +
	//		   m_vTranslateTerm2*itemOffset^2
	RageVector3	m_vTranslateTerm0;
	RageVector3	m_vTranslateTerm1;
	RageVector3	m_vTranslateTerm2;
};

#endif

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
