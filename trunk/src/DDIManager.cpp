#include "global.h"
#include "DDIManager.h"
#include "arch/DDI/DDIDriver.h"
#include "RageUtil.h"
#include "GameState.h"
#include "Song.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include <ctype.h>

DDIManager*	DDIMAN = NULL;	// global and accessable from anywhere in our program

#define SEQUENCE_INI_PATH "Data/Sequences.ini"
DDIManager::ActiveSequence::ActiveSequence(PlayerNumber pn, const DDISequence &seq, float timeMult) :
	m_pn(pn),
	m_seq(seq),
	m_fTimeMult(timeMult),
	m_iCurStep(0)
{
	m_bDone = m_seq.empty();
	if (!m_bDone) 
		m_fTimeLeftOnCurStep = m_seq[0].time * timeMult;
}

// Sets flamethrower state based on mask, relative to player number
void DDIManager::PlaySequenceFlame( PlayerNumber pn, 
									DDISequenceFlame sf,
									bool bPlayerFlameState[2][NUM_PLAYER_FLAMES],
									bool bDisplayFlameState[NUM_DISPLAY_FLAMES])
{
	int me = MyPlayer(pn);
	int you = OtherPlayer(pn);

	if (sf & SF_MY_CENTER)		bPlayerFlameState[me][FLAMES_PLAYER_CENTER]	= true;
	if (sf & SF_MY_SIDES)		bPlayerFlameState[me][FLAMES_PLAYER_SIDES]	= true;
	if (sf & SF_MY_WALL)		bPlayerFlameState[me][FLAMES_PLAYER_WALL]		= true;
	if (sf & SF_OTHER_CENTER)	bPlayerFlameState[you][FLAMES_PLAYER_CENTER]	= true;
	if (sf & SF_OTHER_SIDES)	bPlayerFlameState[you][FLAMES_PLAYER_SIDES]	= true;
	if (sf & SF_OTHER_WALL)		bPlayerFlameState[you][FLAMES_PLAYER_WALL]	= true;
	if (sf & SF_MY_DISPLAY)		bDisplayFlameState[me==0 ? FLAMES_DISPLAY_LEFT : FLAMES_DISPLAY_RIGHT] = true;
	if (sf & SF_OTHER_DISPLAY)  bDisplayFlameState[me==0 ? FLAMES_DISPLAY_RIGHT : FLAMES_DISPLAY_LEFT] = true;
	if (sf & SF_CENTER_DISPLAY) bDisplayFlameState[FLAMES_DISPLAY_CENTER]		= true;
	if (sf & SF_LEFT_DISPLAY)	bDisplayFlameState[FLAMES_DISPLAY_LEFT]		= true;
	if (sf & SF_RIGHT_DISPLAY)	bDisplayFlameState[FLAMES_DISPLAY_RIGHT]		= true;
}

void DDIManager::ActiveSequence::Update( float fDeltaTime )
{
	if (!m_bDone) {
		m_fTimeLeftOnCurStep -= fDeltaTime;

		while (m_fTimeLeftOnCurStep <= 0) {
			m_iCurStep++;

			if (m_iCurStep == (int)m_seq.size())  {
				m_bDone = true;
				break; // sequence over
			}

			m_fTimeLeftOnCurStep += m_seq[m_iCurStep].time * m_fTimeMult;
		}
	}
}

// ------------------------------------------ Public Interface --------------------------------------

DDIManager::DDIManager()
{
	ZERO(m_fFlamethrowerTemp);
	m_fMaxBurnTime = PREFSMAN->m_fMaxBurnTime;
	m_fCoolingRate = PREFSMAN->m_fCoolingRate;

	m_sequenceIni.ReadFile(SEQUENCE_INI_PATH);

	m_pDriver = MakeDDIDriver();
};

DDIManager::~DDIManager()
{
}

void DDIManager::RefreshSequences()
{
	m_sequenceIni.Reset();
	m_sequenceIni.ReadFile(SEQUENCE_INI_PATH);
	//SCREENMAN->SystemMessage( "Reloaded manual sequences" ); 
}

// Wrap BPM to 90..180 to prevent timing from going too far off
float DDIManager::GetQuarterNoteMultiplier()
{
	if (GAMESTATE->m_pCurSong) {
		float bpm = GAMESTATE->m_pCurSong->GetBPMAtBeat( GAMESTATE->m_fSongBeat );
		if (bpm > 180)
			bpm /= 2;
		else if (bpm < 90)
			bpm *= 2;
		return 60 / bpm;
	} else {
		return 0.5; // assume 120 bpm if no current song
	}
}

float DDIManager::GetEighthNoteMultiplier()
{
	return GetQuarterNoteMultiplier() / 2;
}

float DDIManager::GetSixteenthNoteMultiplier()
{
	return GetQuarterNoteMultiplier() / 4;
}

float DDIManager::GetThirtysecondNoteMultiplier()
{
	return GetQuarterNoteMultiplier() / 8;
}

// Maps lights and flames to relay numbers
int DDIManager::PlayerFlameToRelay(PlayerNumber pn, PlayerFlame pf)
{
	return DDI_PLAYER_CHANNEL_START + pn*NUM_PLAYER_FLAMES + pf;
}

int DDIManager::DisplayFlameToRelay(DisplayFlame df)
{
	return DDI_DISPLAY_CHANNEL_START + df;
}

int DDIManager::LightToRelay(DDILight dl)
{
	return DDI_LIGHT_CHANNEL_START + dl;
}

int DDIManager::ArmEnableRelay()
{
	return DDI_ARM_CHANNEL;
}

void DDIManager::FlamethrowerSequence(PlayerNumber pn, const DDISequence &seq, float timeMult, bool bFireEvenIfDemo)
{
	if (!seq.empty() && (bFireEvenIfDemo || !GAMESTATE->m_bDemonstrationOrJukebox)) 
		m_sequences.push_front(ActiveSequence(pn, seq, timeMult));
}

// Clear all sequences and turn everything off
void DDIManager::StopTheMadness()
{
	m_sequences.clear();

	for (int pn=0; pn<NUM_PLAYERS; pn++)
		for (int pf=0; pf<NUM_PLAYER_FLAMES; pf++)
			SetFlamethrowerRelayState(PlayerFlameToRelay((PlayerNumber)pn, (PlayerFlame)pf), false);

	for (int df=0; df<NUM_DISPLAY_FLAMES; df++)
		SetFlamethrowerRelayState(DisplayFlameToRelay((DisplayFlame)df), false);
}

void DDIManager::SetLightState(DDILight dl, bool state)
{
	m_pDriver->SetRelayState(LightToRelay(dl), state);
}

void DDIManager::FlushLights()
{
	m_pDriver->Flush();
}

void DDIManager::ArmEnable()
{
	m_pDriver->SetRelayStateAndFlush(ArmEnableRelay(), true);
}

void DDIManager::ArmDisable()
{
	m_pDriver->SetRelayStateAndFlush(ArmEnableRelay(), false);
}

// Funnel all state setting through here
// Used for throttling of player effects
void DDIManager::SetFlamethrowerRelayState(int relay, bool state)
{
	// throttle only effects pointed at the player
	bool bShouldThrottle =
		(relay == PlayerFlameToRelay(PLAYER_1, FLAMES_PLAYER_CENTER)) ||
		(relay == PlayerFlameToRelay(PLAYER_1, FLAMES_PLAYER_SIDES))  ||
		(relay == PlayerFlameToRelay(PLAYER_2, FLAMES_PLAYER_CENTER)) ||
		(relay == PlayerFlameToRelay(PLAYER_2, FLAMES_PLAYER_SIDES));

	if (state && bShouldThrottle) {

		if (m_fFlamethrowerTemp[relay - DDI_PLAYER_CHANNEL_START] < m_fMaxBurnTime) {
			m_pDriver->SetRelayState(relay,	true); // Fire!
		} else {
			//SCREENMAN->SystemMessageNoAnimate( ssprintf("Throttled Flamethrower %d", relay) ); 
			m_pDriver->SetRelayState(relay,	false); // Denied!
		}

	} else 	{

		// turning relay off, not a player effect
		m_pDriver->SetRelayState(relay,	state);
	}
}

void DDIManager::UpdateFlamethrowerTemp( float fDeltaTime )
{
	for (int i=0; i<NUM_PLAYER_FLAMES*2; i++) {

		float temp = m_fFlamethrowerTemp[i];

		if (m_pDriver->GetRelayState(DDI_PLAYER_CHANNEL_START + i)) {
			temp += fDeltaTime;

			// if we actually need to throttle, add a little more 'heat' to prevent firing for at least another sec 
			if (temp  >= m_fMaxBurnTime)
				temp = max(temp, m_fMaxBurnTime + 1/m_fCoolingRate);

		} else {

			temp = max(0, temp - fDeltaTime * m_fCoolingRate);
		}

		m_fFlamethrowerTemp[i] = temp;
	}
}

void DDIManager::Update( float fDeltaTime )
{
	// Start by assuming all effects closed
	bool bPlayerFlameState[2][NUM_PLAYER_FLAMES];
	bool bDisplayFlameState[NUM_DISPLAY_FLAMES];
	ZERO(bPlayerFlameState);
	ZERO(bDisplayFlameState);

	// Walk through all active sequences, updating and triggering
	list<ActiveSequence>::iterator it = m_sequences.begin();
	while (it != m_sequences.end()) {
		list<ActiveSequence>::iterator next = it;
		++next;

		// Update the sequence. If it is not finished, output the current step to the flamethrowers
		// Otherwise delete the completed sequence
		it->Update(fDeltaTime);
		if (!it->m_bDone) {
			PlaySequenceFlame(it->m_pn, it->m_seq[it->m_iCurStep].flame, bPlayerFlameState, bDisplayFlameState);
		} else {
			m_sequences.erase(it);
		}

		it = next;
	}

	// update "temperatures" before we send new state, to handle throttling correctly
	UpdateFlamethrowerTemp( fDeltaTime );

	// Output current state (relay driver will only send data on changes)
	for (int pn=0; pn<NUM_PLAYERS; pn++)
		for (int pf=0; pf<NUM_PLAYER_FLAMES; pf++)
			SetFlamethrowerRelayState(PlayerFlameToRelay((PlayerNumber)pn, (PlayerFlame)pf), bPlayerFlameState[pn][pf]);

	for (int df=0; df<NUM_DISPLAY_FLAMES; df++)
		SetFlamethrowerRelayState(DisplayFlameToRelay((DisplayFlame)df), bDisplayFlameState[df]);

	m_pDriver->Flush();
}


static void StripLeadingAndTrailingWhitespace(CString &str)
{
	while (str.size() && isspace(*str.begin()))
		str.erase(str.begin());	// delete first char
	while (str.size() && isspace(*(str.end()-1))) 
		str.erase(str.end()-1); // delete last char
}

struct NameFlame {
	char *name;
	DDISequenceFlame flame;
};

static NameFlame FlameNames[] = {
	{"NONE",					0},
	{"PLAYER_CENTER",		SF_MY_CENTER},
	{"PLAYER_SIDES",		SF_MY_SIDES},
	{"PLAYER_WALL",			SF_MY_WALL},
	{"OTHER_CENTER",		SF_OTHER_CENTER},
	{"OTHER_SIDES",			SF_OTHER_SIDES},
	{"OTHER_WALL",			SF_OTHER_WALL},
	{"PLAYER_DISPLAY",		SF_MY_DISPLAY},
	{"OTHER_DISPLAY",		SF_OTHER_DISPLAY},
	{"DISPLAY_CENTER",		SF_CENTER_DISPLAY},
	{"DISPLAY_LEFT",		SF_LEFT_DISPLAY},
	{"DISPLAY_RIGHT",		SF_RIGHT_DISPLAY}
};

// Splits sequence flame defn at + to turn into a bitmask
// Ignores leading and trailing whitespace, and case
DDISequenceFlame DDIManager::ParseSequenceFlame(const CString &str)
{
	DDISequenceFlame flame = 0;
	CStringArray names;

	split(str, "+", names);

	for (unsigned int i=0; i<names.size(); i++) {

		StripLeadingAndTrailingWhitespace(names[i]);
		
		for (int j=0; j<ARRAYSIZE(FlameNames); j++) {
			if (names[i].CompareNoCase(FlameNames[j].name) == 0) { 
				flame |= FlameNames[j].flame;
				break;
			}
		}
	}

	return flame;
}

// Parses a single sequence step specifier (of form FLAME[+FLAME]:TIME) and adds it to sequence
void DDIManager::ParseSequenceStep(const CString &str, DDISequence &seq_to_add_to)
{
	DDISequenceStep step;

	// First split string into step:duration
	size_t colonpos = str.find(':');
	if (colonpos != str.npos) {	// skip entry if no colon

		step.flame = ParseSequenceFlame(str.substr(0, colonpos));

		CString timestr = str.substr(colonpos+1);
		StripLeadingAndTrailingWhitespace(timestr);
		step.time = (float)atof(timestr.c_str());

		seq_to_add_to.push_back(step);
	}
}

// Look up a sequence in the sequence ini file. Returns empty sequence if not present.
// You should probably cache the result, this may not be fast (lots of string parsing)
// Returns empty sequence (size zero vector) if not found
// Sequence includes terminator if found
DDISequence DDIManager::LoadSequence(const CString &key, const CString &valuename)
{
	CString value;
	DDISequence seq;

	if (m_sequenceIni.GetValue(key, valuename, value)) {
		// We have a sequence for the specified key/valuename. Parse it into a DDISequence
		CStringArray steps;
		split(value, ",", steps);
		for (unsigned int i=0; i<steps.size(); i++) {
			ParseSequenceStep(steps[i], seq);
		}
	}

	return seq;
}
