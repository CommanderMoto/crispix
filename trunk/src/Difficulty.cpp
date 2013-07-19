#include "global.h"
#include "Difficulty.h"
#include "GameState.h"
#include "ThemeManager.h"


static const CString DifficultyNames[NUM_DIFFICULTIES] = {
	"Beginner",
	"Easy",
	"Medium",
	"Hard",
	"Challenge",
	"Edit",
};
XToString( Difficulty );
XToThemedString( Difficulty );

/* We prefer the above names; recognize a number of others, too.  (They'l
 * get normalized when written to SMs, etc.) */
Difficulty StringToDifficulty( const CString& sDC )
{
	CString s2 = sDC;
	s2.MakeLower();
	if( s2 == "beginner" )		return DIFFICULTY_BEGINNER;
	else if( s2 == "easy" )		return DIFFICULTY_EASY;
	else if( s2 == "basic" )	return DIFFICULTY_EASY;
	else if( s2 == "light" )	return DIFFICULTY_EASY;
	else if( s2 == "medium" )	return DIFFICULTY_MEDIUM;
	else if( s2 == "another" )	return DIFFICULTY_MEDIUM;
	else if( s2 == "trick" )	return DIFFICULTY_MEDIUM;
	else if( s2 == "standard" )	return DIFFICULTY_MEDIUM;
	else if( s2 == "difficult")	return DIFFICULTY_MEDIUM;
	else if( s2 == "hard" )		return DIFFICULTY_HARD;
	else if( s2 == "ssr" )		return DIFFICULTY_HARD;
	else if( s2 == "maniac" )	return DIFFICULTY_HARD;
	else if( s2 == "heavy" )	return DIFFICULTY_HARD;
	else if( s2 == "smaniac" )	return DIFFICULTY_CHALLENGE;
	else if( s2 == "challenge" )return DIFFICULTY_CHALLENGE;
	else if( s2 == "expert" )	return DIFFICULTY_CHALLENGE;
	else if( s2 == "oni" )		return DIFFICULTY_CHALLENGE;
	else if( s2 == "edit" )		return DIFFICULTY_EDIT;
	else						return DIFFICULTY_INVALID;
}


static const CString CourseDifficultyNames[NUM_DIFFICULTIES] =
{
	"(not used)",
	"Easy",
	"Regular",
	"Difficult",
	"(not used)",
};
XToString( CourseDifficulty );
XToThemedString( CourseDifficulty );
StringToX( CourseDifficulty );

CourseDifficulty GetNextShownCourseDifficulty( CourseDifficulty cd )
{
	for( CourseDifficulty d=(CourseDifficulty)(cd+1); d<NUM_DIFFICULTIES; ((int&)d)++ )
	{
		if( GAMESTATE->IsCourseDifficultyShown(d) )
			return d;
	}
	return DIFFICULTY_INVALID;
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
