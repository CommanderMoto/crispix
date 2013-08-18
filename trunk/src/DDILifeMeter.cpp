#include "global.h"
#include "DDILifeMeter.h"
#include "PrefsManager.h"
#include "RageException.h"
#include "RageTimer.h"
#include "GameState.h"
#include "RageMath.h"
#include "ThemeManager.h"
#include "song.h"
#include "StageStats.h"
#include "DDIManager.h"
#include "LightsManager.h"

// Light sequence timing
#define MISS_LIGHT_BLINK		   0.3f
#define LIGHTS_COMBO_SECONDS_SHORT 1.5f
#define LIGHTS_COMBO_SECONDS_LONG  3.0f

int DDILifeMeter::m_iInstanceCount = 0;
DDILifeMeter::sequenceMap DDILifeMeter::m_comboSequences;
DDILifeMeter::sequenceMap DDILifeMeter::m_burnSequences;

//
// Important!!!!  Do not use these macros during gameplay.  They return very slowly.  Cache them in a member.
//
static CachedThemeMetricI START_ROTATION	("PressureGauge","NeedleStartRotation");
static CachedThemeMetricI END_ROTATION		("PressureGauge","NeedleEndRotation");
static CachedThemeMetricI OFFSET_X			("PressureGauge","NeedleOffsetX");
static CachedThemeMetricI OFFSET_Y			("PressureGauge","NeedleOffsetY");

class DDILifeMeterStream : public Actor
{
public:
	DDILifeMeterStream()
	{
		START_ROTATION.Refresh();
		END_ROTATION.Refresh();
		OFFSET_X.Refresh();
		OFFSET_Y.Refresh();

		m_iStartRot = START_ROTATION;
		m_iEndRot = END_ROTATION;
		m_iOffsetX = OFFSET_X;
		m_iOffsetY = OFFSET_Y;

		CString sGraphicPath;
		RageTextureID ID;
		ID.bStretch = true;

		ID.filename = THEME->GetPathToG("PressureGauge needle");
		m_sprNeedle.Load( ID );
		m_sprNeedle.SetUseZBuffer( true );

		ID.filename = THEME->GetPathToG("PressureGauge background");
		m_sprBG.Load( ID );

		ID.filename = THEME->GetPathToG("PressureGauge foreground");
		m_sprFG.Load( ID );
	}

	Sprite	m_sprNeedle;
	Sprite	m_sprBG;
	Sprite	m_sprFG;

	PlayerNumber m_PlayerNumber;
	float	m_fPercent;
	float	m_fHotAlpha;

	int		m_iStartRot, m_iEndRot, m_iOffsetX, m_iOffsetY;

	void DrawPrimitives()
	{
		// draw bg
		m_sprBG.Draw();

		if( GAMESTATE->IsPlayerEnabled(m_PlayerNumber) )
		{
			DrawNeedle( m_fPercent );
		}

		m_sprFG.Draw();
	}

	void DrawNeedle( float fPercent )
	{
		// set needle rotation based on percent
		float fRot = m_iStartRot + fPercent*(m_iEndRot - m_iStartRot);
		m_sprNeedle.SetBaseRotationZ(fRot);
		m_sprNeedle.SetXY((float)m_iOffsetX, (float)m_iOffsetY);
		m_sprNeedle.Draw();
	}
};

DDILifeMeter::DDILifeMeter()
{
	m_pStream = new DDILifeMeterStream;
	m_fPressure = 0;
	m_fTrailingPressure = 0;
	m_fPressureVelocity = 0;  // mass per cubic time? huh? (kg/ms2 *  m/s = kg/s3)
	m_iCombo = 0;
	m_iBurnCombo = 0;
	m_fTimeSinceLastMiss = 0;
	m_fDifficulty = 1.0f;
	m_numTapNotes = 100;  // start out by assuming 100 tap notes, for scoring purposes

	this->AddChild( m_pStream );

	if (m_iInstanceCount == 0) {
		LoadSequences();
	}
	m_iInstanceCount++;

}

DDILifeMeter::~DDILifeMeter()
{
	delete m_pStream;
	m_iInstanceCount--;
	if (!m_iInstanceCount) {
		m_comboSequences.clear(); // clear them so we'll reload at game start, so we can change without quitting
		m_burnSequences.clear();
	}
}

void DDILifeMeter::SetNumTapNotes(int numTapNotes)
{
	m_numTapNotes = numTapNotes;
}

void DDILifeMeter::Load( PlayerNumber pn )
{
	LifeMeter::Load( pn );

	m_pStream->m_PlayerNumber = pn;
}

#define MAX_COMBO_SEQ 300
#define MAX_BURNINATE_SEQ 300

// Initializes the static sequence arrays. File reads and string parsing. Kinda slow.
void DDILifeMeter::LoadSequences()
{
	// load up to 300 burninate sequences, keyed as "Seq1=..."
	CString key("Burninate");
	CString valuenamebase("Seq");
	for (int i=1; i<MAX_BURNINATE_SEQ; i++) {

		DDISequence seq = DDIMAN->LoadSequence(key, valuenamebase + ssprintf("%d", i));
		if (!seq.empty()) {
			m_burnSequences[i] = seq;
		}
	}

	// load combo sequences up to 300, store in map, keyed as "OnNN=..."
	// Each is triggered when the corresponding combo is achieved
	key = "Combo";
	valuenamebase = "On";
	for (int i=1; i<MAX_COMBO_SEQ; i++) {

		DDISequence seq = DDIMAN->LoadSequence(key, valuenamebase + ssprintf("%d", i));
		if (!seq.empty()) {
			m_comboSequences[i] = seq;
		}
	}
}

// Take a bigger hit when we haven't missed for a while
// Used to be based on combo number, but this got obscene for fast hard songs
static float ComboDifficultyMultiplier(float timeSinceLastMiss) 
{
	// multiplier goes up to 5 as time goes to 25 seconds
	return (timeSinceLastMiss / 25.0f) * 5;
}

// Old algorithm for computing needle movement. Based on time: the longer the time between misses, the more pressure
float DDILifeMeter::ComputePressureChangeOld() const
{
	float delta = -2*PREFSMAN->m_fLifeDeltaPercentChangeMiss;
	delta *= ComboDifficultyMultiplier(m_fTimeSinceLastMiss);
	delta *= m_fDifficulty;

	return delta;
}

// Jared's comment:
/* 
Dr. Compiler,
   Please remove this comment. 

Thank you
Jared
*/

// Note: comment removed by preprocessor

// New algorithm for computing needle movement. Fixed amount so that the gauge fills 
// a baseline of 20 times if every step is missed. Bad players are handled by having the
// flame sequence file be very conservative for low step counts (max of 0.5 beat for < 15)
float DDILifeMeter::ComputePressureChangeNew() const
{
	float delta;

	if (m_iCombo >= 50) 
	{
		delta = 1.0f;	// if they break a combo >=50, always shoot!
	}
	else
	{
		delta = 20.0f / m_numTapNotes;	// otherwise, baseline shoot 20 times over the course of the song
		delta /= PREFSMAN->m_fLifeDifficultyScale;
		delta *= m_fDifficulty;
	}

	return delta;
}

void DDILifeMeter::ChangeLife( TapNoteScore score )
{
	float fDeltaPressure = 0;

	switch( GAMESTATE->m_SongOptions.m_DrainType )
	{
	case SongOptions::DRAIN_NORMAL:
	case SongOptions::DRAIN_NO_RECOVER:  // what does this option mean in our case?

		switch( score )
		{
		case TNS_MARVELOUS:
		case TNS_PERFECT:
		case TNS_GREAT:
		case TNS_GOOD:
			m_iCombo++;
			break;

		case TNS_BOO:
		case TNS_MISS:
			if (!m_bReturningToZero)	// don't count misses until gauge hits zero
			{
				// Compute pressure increase
				fDeltaPressure = ComputePressureChangeNew();
				// fDeltaPressure = CompurePressureChangeOld();

				// Store combo for next update sequence, then set to zero
				if (!m_iBurnCombo)
					m_iBurnCombo = m_iCombo;
				m_iCombo=0;

				// fire miss light
				LIGHTSMAN->BlinkPlayerLight(m_PlayerNumber, PLM_MISS, MISS_LIGHT_BLINK);

				m_fTimeSinceLastMiss = 0;
			}
			break;
		default:
			fDeltaPressure = 0;
		}
		break;

	case SongOptions::DRAIN_SUDDEN_DEATH:
		fDeltaPressure = 1.0; // hah ha ha, blast 'em on every missed step!
	default:
		ASSERT(0);
	}


	ChangePressure( fDeltaPressure );

	TriggerCombos();
}

// If there is a sequence associated with this combo number, play it
void DDILifeMeter::TriggerCombos()
{
	int comboToPlay = m_iCombo;

	// if combo > MAX_COMBOS, repeat last hundred
	if (comboToPlay >= MAX_COMBO_SEQ) {
		comboToPlay = ((comboToPlay - MAX_COMBO_SEQ) % 100) + MAX_COMBO_SEQ-100;
	}

	sequenceMap::iterator combo = m_comboSequences.find(comboToPlay);
	if (combo != m_comboSequences.end()) {
		DDIMAN->FlamethrowerSequence(m_PlayerNumber, combo->second, DDIMAN->GetSixteenthNoteMultiplier());
	}

	// Trigger lighting combos too
	if (m_iCombo) 
	{
		if ((m_iCombo % 100) == 0)
			LIGHTSMAN->BlinkPlayerLight(m_PlayerNumber, PLM_COMBO100, LIGHTS_COMBO_SECONDS_LONG);
		else if ((m_iCombo % 50) == 0)
			LIGHTSMAN->BlinkPlayerLight(m_PlayerNumber, PLM_COMBO50, LIGHTS_COMBO_SECONDS_LONG);
		else if ((m_iCombo % 25) == 0)
			LIGHTSMAN->BlinkPlayerLight(m_PlayerNumber, PLM_COMBO25, LIGHTS_COMBO_SECONDS_LONG);
		else if ((m_iCombo % 10) == 0)
			LIGHTSMAN->BlinkPlayerLight(m_PlayerNumber, PLM_COMBO10, LIGHTS_COMBO_SECONDS_SHORT);
//		else if ((m_iCombo % 5) == 0)
//			LIGHTSMAN->BlinkPlayerLight(m_PlayerNumber, PLM_COMBO5, LIGHTS_COMBO_SECONDS_SHORT);
	}
}

// Tap notes: the idea would be to count NG as a miss, OK probably as nil: defined by prefs
void DDILifeMeter::ChangeLife( HoldNoteScore score, TapNoteScore tscore )
{
	float fDeltaPressure = 0.0f;

	switch( GAMESTATE->m_SongOptions.m_DrainType )
	{
	case SongOptions::DRAIN_NORMAL:
	case SongOptions::DRAIN_NO_RECOVER:  // what does this option mean in our case?
		switch( score )
		{
		case HNS_OK:	
			fDeltaPressure = -PREFSMAN->m_fLifeDeltaPercentChangeOK;	
			break;
		case HNS_NG:	
			fDeltaPressure = -PREFSMAN->m_fLifeDeltaPercentChangeNG;	
		}
		break;

	case SongOptions::DRAIN_SUDDEN_DEATH:
		fDeltaPressure = 1.0; // hah ha ha, blast 'em on every missed step!


	default:
		ASSERT(0);
	}
	ChangePressure( fDeltaPressure );
}

static DDISequence EmptyDDISequence;

// Choose a burninate sequence not quite at random. The way it works is we take m_iCombo and
// jitter it by some amount, then pick the closest sequence to that index
const DDISequence &DDILifeMeter::RandomBurninateSequence()
{
	if (m_burnSequences.empty())
		return EmptyDDISequence;		// no sequences programmed

	// pick random seq around combo number in an increasing range (20 at 1000 combo)
	int range = max(10, m_iBurnCombo / 5); 
	int choice = m_iBurnCombo + RandomInt(-range, range); 

	// get entries above, below and pick whichever is closer
	sequenceMap::iterator up  = m_burnSequences.upper_bound(choice);
	sequenceMap::iterator low  = m_burnSequences.lower_bound(choice);

	sequenceMap::iterator end = m_burnSequences.end();

	if (up == end)
		up--;
	if (low == end)
		low = m_burnSequences.begin();

	if (abs(up->first - choice) < abs(low->first - choice))
		return up->second;
	else
		return low->second;
}


#define DDI_WALL_OF_FLAME_BURST_LENGTH_LIFE_PERCECNTAGE_SCALE 1.0f#define DDI_MINE_BURST_LENGTH 0.2f

// ChangeLife does not actually moves the gauge, it is merely the public interface where 
// CombinedLifeMeterTug calls into. In other words, it gets called when one player does
// well enough to tug at the other. Sounds like a great time to fire the Wall Of Flame!
void DDILifeMeter::ChangeLife( float fDeltaLife )
{
	if (fDeltaLife < 0)	{
/*		DDIMAN->Burninate(	m_PlayerNumber,
								SF_MY_WALL,
								-fDeltaLife * DDI_WALL_OF_FLAME_BURST_LENGTH_LIFE_PERCECNTAGE_SCALE );
*/	}
}

//Again, doesn't actually change life (pressure) but rather, burninates: sides and wall
void DDILifeMeter::ChangeLifeMine()
{
//	DDIMAN->Burninate(m_PlayerNumber, SF_MY_SIDES, DDI_MINE_BURST_LENGTH);
}

// ChangePressure actually moves the pressure bar
void DDILifeMeter::ChangePressure( float fDeltaPressure )
{
	m_fPressure += fDeltaPressure;
	CLAMP( m_fPressure, 0, 1 );

	m_fPressureVelocity += 0.3f*fDeltaPressure;
}

// DDI life meter has no hot/danger/fail whatever states
bool DDILifeMeter::IsHot() const
{ 
	return false;  
}

bool DDILifeMeter::IsInDanger() const
{ 
	return false;  
}

bool DDILifeMeter::IsFailing() const
{ 
	return false;  
}


void DDILifeMeter::Update( float fDeltaTime )
{
	LifeMeter::Update( fDeltaTime );

	// HACK:  Tweaking these values is very difficulty.  Update the
	// "physics" many times so that the spring motion appears faster
	for( int i=0; i<10; i++ )
	{

		const float fDelta = m_fPressure - m_fTrailingPressure;

		const float fSpringForce = fDelta * 2.0f;
		m_fPressureVelocity += fSpringForce * fDeltaTime;

		const float fViscousForce = -m_fPressureVelocity * 0.6f;
		m_fPressureVelocity += fViscousForce * fDeltaTime;

		CLAMP( m_fPressureVelocity, -0.6f, +0.6f ); 

		m_fTrailingPressure += m_fPressureVelocity * fDeltaTime;
	}

	// clip needle hard at zero -- also reset returning flag
	if (m_fTrailingPressure <= 0) {
		m_fTrailingPressure = 0;
		m_fPressureVelocity = 0;
		m_bReturningToZero = false;
	}

	if (m_fTrailingPressure >= 1) {
		// fire fire fire fire fire fire fire fire fire fire fire fire fire fire fire!
			DDIMAN->FlamethrowerSequence(m_PlayerNumber, RandomBurninateSequence(), DDIMAN->GetSixteenthNoteMultiplier());
		m_iBurnCombo = 0; // we've played it, now accept another one
		m_fPressure = 0;
		m_fTrailingPressure = 1;
		m_fPressureVelocity = -0.5;
		m_bReturningToZero = true;
	} 

	m_fTimeSinceLastMiss += fDeltaTime;
}


void DDILifeMeter::DrawPrimitives()
{
	m_pStream->m_fPercent = m_fTrailingPressure;

	ActorFrame::DrawPrimitives();
}
void DDILifeMeter::UpdateNonstopLifebar(const int cleared, 
		const int total, int ProgressiveLifebarDifficulty)
{
	return;
}

void DDILifeMeter::FillForHowToPlay(int NumPerfects, int NumMisses)
{
	m_fPressure = 0;
}

void DDILifeMeter::MoreFire()
{
	m_fDifficulty *= 1.5;
}

void DDILifeMeter::LessFire()
{
	m_fDifficulty /= 1.5;
}
