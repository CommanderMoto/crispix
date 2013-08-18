/* DDIDriver - Controls DDI relay board */

#ifndef DDIDriver_H
#define DDIDriver_H

class DDIDriver
{
public:
	DDIDriver();
	~DDIDriver();

	void SetRelayState( int relay, bool state );
	virtual void Flush() = 0; // platform specific
	
	void SetRelayStateAndFlush(int relay, bool state );

	bool GetRelayState( int relay );

protected:

	bool m_bRelayState[16];	// mirror of hardware state
};

DDIDriver *MakeDDIDriver();	// factory for platform-specific driver 

#endif;