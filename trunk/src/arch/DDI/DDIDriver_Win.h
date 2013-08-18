/* DDIDriver_Win - sends serial data to relay board (windows) */

#ifndef DDIDRIVER_WIN_H
#define DDIDRIVER_WIN_H

#include "arch/DDI/DDIDriver.h"

class DDIDriver_Win : public DDIDriver 
{
public:
	DDIDriver_Win();
	~DDIDriver_Win();

	void ZeroRelays();

	virtual void Flush(); 
};

#endif
