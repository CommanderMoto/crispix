/* DDILifeMeter 
   Updates and draws the DDI "pressure gauge" element.
   Kindof overloaded: also decides when to fire flamethrowers!
 */

#ifndef DDILIFEMETERBAR_H
#define DDILIFEMETERBAR_H

#include "LifeMeter.h"
#include "Sprite.h"
#include "DDIManager.h"
#include <map>

class DDILifeMeterStream;

class DDILifeMeter : public LifeMeter
{
public:
	DDILifeMeter();
	~DDILifeMeter();
	
	virtual void Load( PlayerNumber pn );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void ChangeLife( TapNoteScore score );
	virtual void ChangeLife( HoldNoteScore score, TapNoteScore tscore  );
	virtual void ChangeLife( float fDeltaLifePercent );
	virtual void ChangeLifeMine();
	virtual void AfterLifeChanged() {};  // this life meter doesn't care
	virtual void OnDancePointsChange() {};	// ditto
	virtual bool IsInDanger() const;
	virtual bool IsHot() const;
	virtual bool IsFailing() const;
	virtual float GetLife() const { return m_fPressure; } // meaningless?

	void UpdateNonstopLifebar(int cleared, int total, int ProgressiveLifebarDifficulty);
	void FillForHowToPlay(int NumPerfects, int NumMisses);
	// this function is solely for HowToPlay

	// Call to adjust internal difficulty rating (initially taken off difficulty set in options)
	void MoreFire();
	void LessFire();

private:

	void LoadSequences();
	void ChangePressure(float fPressure); // different from ChangeLife, see comments in DDILifeMeter.cpp
	const DDISequence &RandomBurninateSequence();
	void TriggerCombos();

	DDILifeMeterStream*	m_pStream;

	float		m_fPressure;
	float		m_fTrailingPressure;	// this approaches m_fPressurePercentage
	float		m_fPressureVelocity;
	bool		m_bReturningToZero;		// true when pressure going back down
	float		m_fDifficulty;			// starts out at difficulty set in options
	int			m_iCombo;				// current combo count for this player
	float		m_fTimeSinceLastMiss;	// hit 'em harder when they're doing well
	int			m_iBurnCombo;			// combo of last unburned miss

	// Sequence tables, loaded from ini
	typedef		map<int, DDISequence>	sequenceMap;
	
	static		int				m_iInstanceCount;	// initial instance loads sequences
	static		sequenceMap		m_comboSequences;
	static		sequenceMap		m_burnSequences;
};

#endif
