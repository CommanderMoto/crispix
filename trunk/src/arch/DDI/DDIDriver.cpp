/* DDIDriver - Controls DDI relay board */

#include "arch/DDI/DDIDriver.h"

DDIDriver::DDIDriver()
{	
}

DDIDriver::~DDIDriver()
{
}

void DDIDriver::SetRelayState( int relay, bool state ) 
{
	m_bRelayState[relay] = state;
}

bool DDIDriver::GetRelayState( int relay )
{
	return m_bRelayState[relay];
}

void DDIDriver::SetRelayStateAndFlush( int relay, bool state ) 
{
	SetRelayState(relay, state);
	Flush();
}

#ifdef WIN32
#include "arch/DDI/DDIDriver_Win.h"

DDIDriver *MakeDDIDriver()
{
	return new DDIDriver_Win;
}

#endif