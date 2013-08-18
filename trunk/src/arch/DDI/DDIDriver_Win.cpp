/* DDIDriver_Win - sends serial data to relay board (windows) */

#include "arch/DDI/DDIDriver_Win.h"
#include "global.h"
#include "RageLog.h"
#include "RageUtil.h" // for ZERO
#include "PrefsManager.h"

#include <windows.h>
static HANDLE S_DDIPort;

// opens serial port
// currently hardcoded to COM4, 9600 bps, no parity, 2 stop bits (who the hell uses 2 stop bits? man!)
DDIDriver_Win::DDIDriver_Win()
{
	char comFileName[] = "\\\\.\\COMx";
	comFileName[7] = (char)PREFSMAN->m_iDDIComPort + '0';

	S_DDIPort = CreateFile(comFileName, 
							GENERIC_WRITE, 
							0, // not shared
							NULL, // no inhertible security access
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL, // flags blah blah blah
							NULL); // template blah blah blah
	
	DCB dcb;
	ZERO(dcb);
	BuildCommDCB("baud=9600 parity=N data=8 stop=1", &dcb);
	dcb.DCBlength = sizeof(dcb);
	BOOL retval = SetCommState(S_DDIPort, &dcb);

	ZeroRelays();
}

DDIDriver_Win::~DDIDriver_Win()
{
	ZeroRelays();
	CloseHandle(S_DDIPort); // close serial port
}

void DDIDriver_Win::ZeroRelays()
{
	ZERO(m_bRelayState);
	Flush();
}

// Send appropriate bytes out serial port
void DDIDriver_Win::Flush() 
{
	unsigned char data[16];

	for (int i=0; i<16; i++)
		data[i] = (char)((i<< 1) | (int)m_bRelayState[i]);	// AR-16 uses a very simple protocol...

	OVERLAPPED overlapped;	// use asynchronous write
	ZERO(overlapped);		// default options (no handle to query completion, etc.) 
	WriteFile(S_DDIPort, (void*)&data, 16, NULL, &overlapped);

//	LOG->Trace("DDI: Turning relay %d %s", relay, state ? "ON" : "OFF"); 
};


