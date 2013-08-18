#include "global.h"
#include "PlayerNumber.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "CommonMetrics.h"


RageColor PlayerToColor( PlayerNumber pn ) 
{
	switch( pn )
	{
		case PLAYER_1:	return COLOR_P1;
		case PLAYER_2:	return COLOR_P2;
		default: ASSERT(0); return RageColor(0.5f,0.5f,0.5f,1);
	}
};

RageColor PlayerToColor( int p ) 
{ 
	return PlayerToColor( (PlayerNumber)p ); 
}

PlayerNumber GetNextHumanPlayer( PlayerNumber pn )
{
	for( PlayerNumber p=(PlayerNumber)(pn+1); p<NUM_PLAYERS; ((int&)p)++ )
	{
		if( GAMESTATE->IsHumanPlayer(p) )
			return p;
	}
	return PLAYER_INVALID;
}

PlayerNumber GetNextEnabledPlayer( PlayerNumber pn )
{
	for( PlayerNumber p=(PlayerNumber)(pn+1); p<NUM_PLAYERS; ((int&)p)++ )
	{
		if( GAMESTATE->IsPlayerEnabled(p) )
			return p;
	}
	return PLAYER_INVALID;
}

PlayerNumber GetNextCpuPlayer( PlayerNumber pn )
{
	for( PlayerNumber p=(PlayerNumber)(pn+1); p<NUM_PLAYERS; ((int&)p)++ )
	{
		if( GAMESTATE->IsCpuPlayer(p) )
			return p;
	}
	return PLAYER_INVALID;
}

PlayerNumber GetNextPotentialCpuPlayer( PlayerNumber pn )
{
	for( PlayerNumber p=(PlayerNumber)(pn+1); p<NUM_PLAYERS; ((int&)p)++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			return p;
	}
	return PLAYER_INVALID;
}

/*
 * (c) 2001-2004 Chris Danford, Chris Gomez
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
