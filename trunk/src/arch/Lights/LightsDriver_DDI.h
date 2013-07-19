#ifndef LightsDriver_DDI_H
#define LightsDriver_DDI_H

#include "LightsDriver.h"

class LightsDriver_DDI : public LightsDriver
{
public:
	LightsDriver_DDI() {};
	virtual ~LightsDriver_DDI() {};
	
	void Set( const LightsState *ls );
};


#endif
