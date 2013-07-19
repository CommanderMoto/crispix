/* DDIManager - Interface with DDI lights and flamethrowers. */

#ifndef DDIMANAGER_H
#define DDIMANAGER_H

#include "PlayerNumber.h"
//#include "GameInput.h"
#include "EnumHelper.h"
#include "IniFile.h"
#include <list>
#include <vector>

// ----------------------------- Sequence Definition ----------------------------

enum DDILight
{
	DDI_LIGHT_1,
	DDI_LIGHT_2,
	DDI_LIGHT_3,
	DDI_LIGHT_4,
	DDI_LIGHT_5,
	DDI_LIGHT_6,
	NUM_DDI_LIGHTS
};

#define FOREACH_DDILight( i ) FOREACH_ENUM( DDILight, NUM_DDI_LIGHTS, i )

// Firing Sequences 
// Flame firing, relative to player number (also absolute display effect references)

enum {
	SF_NONE				= 0,
	SF_MY_CENTER		= 0x1,
	SF_MY_SIDES			= 0x2,
	SF_MY_WALL			= 0x4,
	SF_OTHER_CENTER		= 0x8,
	SF_OTHER_SIDES		= 0x10,
	SF_OTHER_WALL		= 0x20,
	SF_MY_DISPLAY		= 0x40,
	SF_OTHER_DISPLAY	= 0x80,
	SF_CENTER_DISPLAY	= 0x100,
	SF_LEFT_DISPLAY		= 0x200,
	SF_RIGHT_DISPLAY	= 0x400
};
typedef long DDISequenceFlame;

struct DDISequenceStep {
	DDISequenceFlame	flame;
	float				time;  // time == 0 means sequence end
};

typedef vector<DDISequenceStep> DDISequence;

// ----------------------------- Hardware Definitions ----------------------------

enum PlayerFlame
{
	FIRST_PLAYER_FLAME   = 0,
	FLAMES_PLAYER_CENTER = 0,
	FLAMES_PLAYER_SIDES,
	FLAMES_PLAYER_WALL,
	NUM_PLAYER_FLAMES
};

#define FOREACH_PlayerFlame( i ) FOREACH_ENUM( PlayerFlame, NUM_PLAYER_FLAMES, i )

enum DisplayFlame
{
	FIRST_DISPLAY_FLAME	= 0,
	FLAMES_DISPLAY_LEFT	= 0,
	FLAMES_DISPLAY_RIGHT,
	FLAMES_DISPLAY_CENTER,
	NUM_DISPLAY_FLAMES
};

#define FOREACH_DisplayFlame( i ) FOREACH_ENUM( DisplayFlame, NUM_DISPLAY_FLAMES, i )

// these constants define the mapping from the enums above to relay board channels
// these are hard coded, to drive home the point that changes here must be accompanied by hardware changes
#define DDI_ARM_CHANNEL				0
#define DDI_PLAYER_CHANNEL_START	1
#define DDI_DISPLAY_CHANNEL_START	7
#define DDI_LIGHT_CHANNEL_START		10

#define DDI_PLAYER_CHANNEL_END		(DDI_PLAYER_CHANNEL_START + NUM_PLAYER_FLAMES*2 - 1)

// ----------------------------- Class Defn ----------------------------

class DDIDriver;

class DDIManager
{
public:
	DDIManager::	DDIManager();
	~DDIManager();
	
	void Update( float fDeltaTime );

	void ArmEnable();
	void ArmDisable();

	void SetLightState(DDILight dl, bool state);
	void FlushLights();

	// Fire flamethrower for specified time
	void Burninate(PlayerNumber pn, DDISequenceFlame sf, float fTime);

	// Run a sequence. All sequences play concurrently
	void FlamethrowerSequence(PlayerNumber pn, const DDISequence &seq, float timeMult, bool bFireEvenIfDemo = false);

	// Stops all sequences immediately
	void StopTheMadness();
	
	// Utility: get time mults for 1.0 = quarter, eighth, sixteenth (based on global bpm)
	float GetQuarterNoteMultiplier();
	float GetEighthNoteMultiplier();
	float GetSixteenthNoteMultiplier();
	float GetThirtysecondNoteMultiplier();

	// public so DDI driver can use these to apply safety logic to flamethrowers
	int PlayerFlameToRelay(PlayerNumber pn, PlayerFlame pf);
	int DisplayFlameToRelay(DisplayFlame df);
	int LightToRelay(DDILight dl);
	int ArmEnableRelay();

	// Look up a sequence in the sequence ini file. Returns empty sequence if not present.
	// You should probably cache the result, this may not be fast (lots of string parsing)
	DDISequence LoadSequence(const CString &key, const CString &valuename);
	void RefreshSequences(); // reloads from disk

private:

	struct ActiveSequence {
		PlayerNumber	m_pn;
		int				m_iCurStep;
		float			m_fTimeMult;
		float			m_fTimeLeftOnCurStep;
		DDISequence		m_seq;
		bool			m_bDone;

		ActiveSequence() {};
		ActiveSequence(PlayerNumber pn, const DDISequence &seq, float timeMult);

		void Update( float fDeltaTime);
	};

	int MyPlayer(PlayerNumber pn) { return pn; }
	int OtherPlayer(PlayerNumber pn) { return 1-pn; }
	void PlaySequenceFlame( PlayerNumber pn, DDISequenceFlame sf, bool bPlayerFlameState[2][NUM_PLAYER_FLAMES], bool bDisplayFlameState[NUM_DISPLAY_FLAMES]);
	void UpdateSequence( ActiveSequence *seq, float fDeltaTime );
	DDISequenceFlame ParseSequenceFlame(const CString &str);
	void ParseSequenceStep(const CString &str, DDISequence &seq_to_add_to);
	void SetFlamethrowerRelayState(int relay, bool state);
	void UpdateFlamethrowerTemp( float fDeltaTime );


	list<ActiveSequence> m_sequences;		// currently playing sequences
	float				m_fMaxBurnTime;		// throttling settings
	float				m_fCoolingRate;
	float				m_fFlamethrowerTemp[NUM_PLAYER_FLAMES*2]; // throttling state
	IniFile				 m_sequenceIni;
	DDIDriver			*m_pDriver;
};

extern DDIManager*	DDIMAN;	// global and accessable from anywhere in our program

#endif
