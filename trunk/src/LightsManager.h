/* LightsManager - Control lights. */

#ifndef LightsManager_H
#define LightsManager_H

#include "PlayerNumber.h"
#include "GameInput.h"
#include "EnumHelper.h"

const float LIGHTS_FALLOFF_SECONDS = 0.1f;

enum GlobalLight
{
	GLOBAL_LIGHT_STEP  = 0 ,
	GLOBAL_LIGHT_DISCO,
	NUM_GLOBAL_LIGHTS
};
#define FOREACH_GlobalLight( i ) FOREACH_ENUM( GlobalLight, NUM_GLOBAL_LIGHTS, i )

enum PlayerLight 
{
	PLAYER_LIGHT_1 = 0,
	PLAYER_LIGHT_2,
	NUM_PLAYER_LIGHTS
};
#define FOREACH_PlayerLight( i ) FOREACH_ENUM( PlayerLight, NUM_PLAYER_LIGHTS, i )

enum LightsMode
{
	LIGHTSMODE_ATTRACT,
	LIGHTSMODE_JOINING,
	LIGHTSMODE_MENU,
	LIGHTSMODE_DEMONSTRATION,
	LIGHTSMODE_GAMEPLAY,
	LIGHTSMODE_STAGE,
	LIGHTSMODE_ALL_CLEARED,
	LIGHTSMODE_TEST,
	NUM_LIGHTS_MODES
};
const CString& LightsModeToString( LightsMode lm );


enum PlayerLightsMode
{
	PLM_MISS,
	PLM_HOLD,
	PLM_MULTI,    // more than one arrow to be pressed at a time
	PLM_COMBO5,
	PLM_COMBO10,
	PLM_COMBO25,
	PLM_COMBO50,
	PLM_COMBO100,
	NUM_PLAYER_LIGHTS_MODES
};
#define FOREACH_PlayerLightsMode( i ) FOREACH_ENUM( PlayerLightsMode, NUM_PLAYER_LIGHTS_MODES, i )

struct LightsState
{
	bool m_bGlobalLights[NUM_GLOBAL_LIGHTS];
	bool m_bPlayerLights[NUM_PLAYERS][NUM_PLAYER_LIGHTS];
};

class LightsDriver;

class LightsManager
{
public:
	LightsManager(CString sDriver);
	~LightsManager();
	
	void Update( float fDeltaTime );

	void BlinkGlobalLight( GlobalLight cl );
	void BlinkPlayerLight( PlayerNumber pn, PlayerLightsMode plm, float fSecs );

	void SetLightsMode( LightsMode lm );
	LightsMode GetLightsMode();

private:
	float m_fSecsLeftInGlobalLightBlink[NUM_GLOBAL_LIGHTS];
	float m_fSecsLeftInPlayerLightBlink[NUM_PLAYERS][NUM_PLAYER_LIGHTS_MODES];

	LightsDriver* m_pDriver;
	LightsMode m_LightsMode;
	LightsState m_LightsState;
};


extern LightsManager*	LIGHTSMAN;	// global and accessable from anywhere in our program

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
