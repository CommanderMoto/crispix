#include "global.h"
#include "LightsManager.h"
#include "GameState.h"
#include "RageTimer.h"
#include "arch/Lights/LightsDriver.h"
#include "RageUtil.h"
#include "GameInput.h"	// for GameController
#include "InputMapper.h"
#include "Game.h"
#include "PrefsManager.h"

LightsManager*	LIGHTSMAN = NULL;	// global and accessable from anywhere in our program

LightsManager::LightsManager(CString sDriver)
{
	m_LightsMode = LIGHTSMODE_JOINING;

	m_pDriver = MakeLightsDriver(sDriver);

	ZERO( m_fSecsLeftInGlobalLightBlink);
	ZERO( m_fSecsLeftInPlayerLightBlink);
}

LightsManager::~LightsManager()
{
	SAFE_DELETE( m_pDriver );
}

void LightsManager::Update( float fDeltaTime )
{
	int iBlink;

	// update lights falloff
	{
		FOREACH_GlobalLight( gl )
			fapproach( m_fSecsLeftInGlobalLightBlink[gl], 0, fDeltaTime );
		FOREACH_PlayerNumber( pn )
			FOREACH_PlayerLightsMode( plm )
				fapproach( m_fSecsLeftInPlayerLightBlink[pn][plm], 0, fDeltaTime );
	}

	//
	// Set new lights state cabinet lights
	//
	ZERO( m_LightsState.m_bGlobalLights );
	ZERO( m_LightsState.m_bPlayerLights );

	switch( m_LightsMode )
	{
		case LIGHTSMODE_JOINING:
			{
	//			int iBeat = (int)(GAMESTATE->m_fSongBeat);
	//			bool bBlinkOn = (iBeat%2)==0;
				int iSec = (int)RageTimer::GetTimeSinceStart();
				int iTopIndex = iSec % 4;
				switch( iTopIndex )
				{
				case 0:	m_LightsState.m_bPlayerLights[PLAYER_1][PLAYER_LIGHT_1]  = true; break;
				case 1:	m_LightsState.m_bPlayerLights[PLAYER_1][PLAYER_LIGHT_2]  = true; break;
				case 2:	m_LightsState.m_bPlayerLights[PLAYER_2][PLAYER_LIGHT_1]  = true; break;
				case 3:	m_LightsState.m_bPlayerLights[PLAYER_2][PLAYER_LIGHT_2]  = true; break;
				}

				FOREACH_PlayerNumber( pn )
				{
					if( GAMESTATE->m_bSideIsJoined[pn] ) {
						m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_1] = true;
						m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_2] = true;
					}
				}
			}
			break;

		case LIGHTSMODE_ATTRACT:
			{
				int iSec = (int)RageTimer::GetTimeSinceStart();
				int iTopIndex = iSec % 4;
				switch( iTopIndex )
				{
				case 0:	m_LightsState.m_bPlayerLights[PLAYER_1][PLAYER_LIGHT_1]  = true; break;
				case 1:	m_LightsState.m_bPlayerLights[PLAYER_1][PLAYER_LIGHT_2]  = true; break;
				case 2:	m_LightsState.m_bPlayerLights[PLAYER_2][PLAYER_LIGHT_1]  = true; break;
				case 3:	m_LightsState.m_bPlayerLights[PLAYER_2][PLAYER_LIGHT_2]  = true; break;
				}
			}
			break;

		case LIGHTSMODE_MENU:
			{
				int iBeat = (int)(GAMESTATE->m_fSongBeat);
				int idx = iBeat/2; // run global lights slower

				// global lights go: 1, 2, 12, {}
				m_LightsState.m_bGlobalLights[GLOBAL_LIGHT_STEP] = (idx & 0x1) != 0;
				m_LightsState.m_bGlobalLights[GLOBAL_LIGHT_DISCO] = (idx & 0x2) != 0;
				
				bool bBlinkOn = ((iBeat/4)%2)==0;
				FOREACH_PlayerNumber( pn )
				{
					if( GAMESTATE->m_bSideIsJoined[pn] )
					{
						if (bBlinkOn) 
							m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_1] = true;
						else
							m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_2] = true;
					}
				}
			}
			break;

		case LIGHTSMODE_DEMONSTRATION:
		case LIGHTSMODE_GAMEPLAY:		
		{
			FOREACH_PlayerNumber( pn )
			{
				// Pick highest-numbered mode that has time left (so multi over-rides hold etc.)
				int plm;
				for (plm = NUM_PLAYER_LIGHTS_MODES-1; plm >= 0; plm--)
					if (m_fSecsLeftInPlayerLightBlink[pn][plm] > 0)
						break;

				switch (plm) 
				{
					case -1:
						// no lights mode currently
						break;

					case PLM_MISS:
						// Miss: illuminate the yellow light
						m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_2] = true; 
						break;
						
					case PLM_HOLD:
						// Hold: illuminate the green light
						m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_1] = true; 
						if (m_fSecsLeftInPlayerLightBlink[pn][PLM_MISS])
							m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_2] = true; //also show misses while holding
						break;

					case PLM_MULTI:
					{
						// Multi-step: flash both lights at 5hz
						iBlink = (int)(RageTimer::GetTimeSinceStart()*5);  // flash at 3hz
						if (iBlink & 1)
						{
							m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_1] = true;
							m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_2] = true;
						}
						break;
					}

					case PLM_COMBO5:
					{
						// Combo5: alternate at 3 hz
						iBlink = (int)(RageTimer::GetTimeSinceStart()*6);  // flash at 3hz
						if (iBlink & 1)
							m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_1] = true;
						else
							m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_2] = true;
						break;
					}

					case PLM_COMBO10:
					{
						// Combo10: pulse twice on each light, alternating
						int iSeq = (int)(RageTimer::GetTimeSinceStart()*6) % 8; 
						bool b1=false,b2=false;
						switch (iSeq)
						{
							case 0: b1=true; break;
							case 1: break;
							case 2: b1=true; break;
							case 3: break;
							case 4: b2=true; break;
							case 5: break;
							case 6: b2=true; break;
							case 7: break;
						}
						if (b1) m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_1] = true;
						if (b2) m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_2] = true;
						break;
					}

					case PLM_COMBO25:
					{
						// Combo25: pulse twice on each light, alternating
						int iSeq = (int)(RageTimer::GetTimeSinceStart()*6) % 8; 
						bool b1=false,b2=false;
						switch (iSeq)
						{
							case 0: b1=true; break;
							case 1: b2=true; break;
							case 2: b1=true; break;
							case 3: b2=true; break;
							case 4: b1=true; b2=true;
							case 5: break;
							case 6: b1=true; b2=true;
							case 7: break;
						}
						if (b1) m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_1] = true;
						if (b2) m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_2] = true;
						break;
					}

					case PLM_COMBO50:
					case PLM_COMBO100:
					{
						// Combo50: altnerate, then together
						int iSeq = (int)(RageTimer::GetTimeSinceStart()*6) % 8; 
						bool b1=false,b2=false;
						switch (iSeq)
						{
							case 0: b1=true; break;
							case 1: b2=true; break;
							case 2: b1=true; break;
							case 3: b2=true; break;
							case 4: b1=true; b2=true;
							case 5: break;
							case 6: b1=true; b2=true;
							case 7: break;
						}
						if (b1) m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_1] = true;
						if (b2) m_LightsState.m_bPlayerLights[pn][PLAYER_LIGHT_2] = true;
						break;
					}
				}
			}
			break;
		}

		case LIGHTSMODE_STAGE:
		case LIGHTSMODE_ALL_CLEARED:
		{
			// turn everything on
			FOREACH_GlobalLight( gl )
				m_LightsState.m_bGlobalLights[gl] = true;
			FOREACH_PlayerNumber( pn )
				FOREACH_PlayerLight(pl)
					m_LightsState.m_bPlayerLights[pn][pl] = true;
			break;
		}

		case LIGHTSMODE_TEST:
		{
			int iSec = (int)RageTimer::GetTimeSinceStart();
			int iTopIndex = iSec % 6;
			switch( iTopIndex )
			{
			case 0:	m_LightsState.m_bPlayerLights[PLAYER_1][PLAYER_LIGHT_1]  = true; break;
			case 1:	m_LightsState.m_bPlayerLights[PLAYER_1][PLAYER_LIGHT_2]  = true; break;
			case 2:	m_LightsState.m_bPlayerLights[PLAYER_2][PLAYER_LIGHT_1]  = true; break;
			case 3:	m_LightsState.m_bPlayerLights[PLAYER_2][PLAYER_LIGHT_2]  = true; break;
			case 4: m_LightsState.m_bGlobalLights[GLOBAL_LIGHT_STEP] = true; break;
			case 5: m_LightsState.m_bGlobalLights[GLOBAL_LIGHT_DISCO] = true; break;
			}
			break;
		}

		default:
			ASSERT(0);
	}

	// process global light blink regardless of light state
	if (m_fSecsLeftInGlobalLightBlink[GLOBAL_LIGHT_STEP]>0)
		m_LightsState.m_bGlobalLights[GLOBAL_LIGHT_STEP] = true;
	if (m_fSecsLeftInGlobalLightBlink[GLOBAL_LIGHT_DISCO]>0)
		m_LightsState.m_bGlobalLights[GLOBAL_LIGHT_DISCO] = true;

/*
	// If not joined, has enough credits, and not too late to join, then 
	// blink the menu buttons rapidly so they'll press Start
	{
		int iBeat = (int)(GAMESTATE->m_fSongBeat*4);
		bool bBlinkOn = (iBeat%2)==0;
		FOREACH_PlayerNumber( pn )
		{
			if( !GAMESTATE->m_bSideIsJoined[pn] && GAMESTATE->PlayersCanJoin() && GAMESTATE->EnoughCreditsToJoin() )
			{
				m_LightsState.m_bCabinetLights[LIGHT_BUTTONS_LEFT+pn] = bBlinkOn;
			}
		}
	}


	//
	// Update game controller lights
	//
	// FIXME:  Works only with Game==dance
	// FIXME:  lights pads for players who aren't playing
	switch( m_LightsMode )
	{
	case LIGHTSMODE_ATTRACT:
	case LIGHTSMODE_DEMONSTRATION:
		{
			ZERO( m_LightsState.m_bGameButtonLights );
		}
		break;
	case LIGHTSMODE_ALL_CLEARED:
	case LIGHTSMODE_STAGE:
	case LIGHTSMODE_JOINING:
		{
			FOREACH_GameController( gc )
			{
				bool bOn = GAMESTATE->m_bSideIsJoined[gc];

				FOREACH_GameButton( gb )
					m_LightsState.m_bGameButtonLights[gc][gb] = bOn;
			}
		}
		break;
	case LIGHTSMODE_MENU:
	case LIGHTSMODE_GAMEPLAY:
		{
			if( m_LightsMode == LIGHTSMODE_GAMEPLAY  &&  PREFSMAN->m_bBlinkGameplayButtonLightsOnNote )
			{
				//
				// Blink on notes.
				//
				FOREACH_GameController( gc )
				{
					FOREACH_GameButton( gb )
					{
						m_LightsState.m_bGameButtonLights[gc][gb] = m_fSecsLeftInGameButtonBlink[gc][gb] > 0 ;
					}
				}
			}
			else
			{
				//
				// Blink on button pressess.
				//
				FOREACH_GameController( gc )
				{
					// don't blink unjoined sides
					if( !GAMESTATE->m_bSideIsJoined[gc] )
						continue;

					FOREACH_GameButton( gb )
					{
						bool bOn = INPUTMAPPER->IsButtonDown( GameInput(gc,gb) );
						m_LightsState.m_bGameButtonLights[gc][gb] = bOn;
					}
				}
			}
		}
		break;
	case LIGHTSMODE_TEST:
		{
			int iSec = (int)RageTimer::GetTimeSinceStart();

			int iNumGameButtonsToShow = GAMESTATE->GetCurrentGame()->GetNumGameplayButtons();

			FOREACH_GameController( gc )
			{
				FOREACH_GameButton( gb )
				{
					bool bOn = gb==(iSec%iNumGameButtonsToShow);
					m_LightsState.m_bGameButtonLights[gc][gb] = bOn;
				}
			}
		}
		break;
	default:
		ASSERT(0);
	}
*/
	// apply new light values we set above
	m_pDriver->Set( &m_LightsState );
}

void LightsManager::BlinkGlobalLight( GlobalLight gl )
{
	m_fSecsLeftInGlobalLightBlink[gl] = LIGHTS_FALLOFF_SECONDS;
}


void LightsManager::BlinkPlayerLight( PlayerNumber pn, PlayerLightsMode plm, float fSecs )
{
	m_fSecsLeftInPlayerLightBlink[pn][plm] = fSecs;
}

void LightsManager::SetLightsMode( LightsMode lm )
{
	m_LightsMode = lm;
}

LightsMode LightsManager::GetLightsMode()
{
	return m_LightsMode;
}

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
