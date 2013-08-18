#include "global.h"
#include "LightsDriver_DDI.h"
#include "DDIManager.h"

// Global lights on 1 and 2, player on on 3 and 5, player 2 on 4 and 6
// In other words, player 1 on the top row of outlets, player 2 on the bottom
// Since we have manual control of lights 1-4, this gives us both globals and one player on manual
void LightsDriver_DDI::Set( const LightsState *ls )
{
	DDIMAN->SetLightState( DDI_LIGHT_1, ls->m_bGlobalLights[GLOBAL_LIGHT_STEP]);
	DDIMAN->SetLightState( DDI_LIGHT_2, ls->m_bGlobalLights[GLOBAL_LIGHT_DISCO]);
	DDIMAN->SetLightState( DDI_LIGHT_3, ls->m_bPlayerLights[PLAYER_1][PLAYER_LIGHT_1]);
	DDIMAN->SetLightState( DDI_LIGHT_4, ls->m_bPlayerLights[PLAYER_2][PLAYER_LIGHT_1]);
	DDIMAN->SetLightState( DDI_LIGHT_5, ls->m_bPlayerLights[PLAYER_1][PLAYER_LIGHT_2]);
	DDIMAN->SetLightState( DDI_LIGHT_6, ls->m_bPlayerLights[PLAYER_2][PLAYER_LIGHT_2]);
	DDIMAN->FlushLights();
}

