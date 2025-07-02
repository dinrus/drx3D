// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ParticleSubEmitter.h
//  Version:     v1.00
//  Created:     20/04/2010 by Corey.
//  Описание: Split out from ParticleEmitter.h
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __particlesubemitter_h__
#define __particlesubemitter_h__
#pragma once

#include <drx3D/Eng3D/ParticleEffect.h>
#include <drx3D/Eng3D/ParticleContainer.h>
#include <drx3D/Eng3D/ParticleEnviron.h>
#include <drx3D/Eng3D/ParticleUtils.h>
#include <drx3D/CoreX/BitFiddling.h>

class CParticleEmitter;
class CParticleSource;
struct SParticleUpdateContext;
struct IAudioProxy;

//////////////////////////////////////////////////////////////////////////
class DRX_ALIGN(16) CParticleSubEmitter: public DinrusX3dEngBase, public _plain_reference_target<i32>
	// Maintains an emitter source state, emits particles to a container
	// Ref count increased only by emitted particles
{
public:

	CParticleSubEmitter(CParticleSource * pSource, CParticleContainer * pCont);
	~CParticleSubEmitter();

	ResourceParticleParams const& GetParams() const;
	
	CParticleContainer&           GetContainer() const
	                                                    { return *m_pContainer; }
	CParticleSource&              GetSource() const
	                                                    { assert(m_pSource && m_pSource->NumRefs()); return *m_pSource; }
	
	CParticleEmitter&             GetMain() const;

	// State.
	bool IsActive() const
	                            { return GetAge() >= m_fActivateAge; }
	void Deactivate();

	// Timing.
	float GetAge() const
	                            { return GetSource().GetAge(); }
	float GetStartAge() const
	                            { return m_fStartAge; }
	float GetRepeatAge() const
	                            { return m_fRepeatAge; }
	float GetStopAge() const
	                            { return m_fStopAge; }
	float GetParticleStopAge() const
	                                { return GetStopAge() + GetParams().GetMaxParticleLife(); }
	float GetRelativeAge(float fAgeAdjust = 0.f) const
	                                                  { return GetAgeRelativeTo(m_fStopAge, fAgeAdjust); }

	float GetStopAge(ParticleParams::ESoundControlTime eControl) const;
	float GetRelativeAge(ParticleParams::ESoundControlTime eControl, float fAgeAdjust = 0.f) const
	                                      { return GetAgeRelativeTo(min(GetStopAge(eControl), m_fRepeatAge), fAgeAdjust); }

	float GetStrength(float fAgeAdjust = 0.f, ParticleParams::ESoundControlTime eControl = ParticleParams::ESoundControlTime::EmitterLifeTime) const;

	Vec3 GetEmitFocusDir(const QuatTS &loc, float fStrength, Quat * pRot = 0) const;

	// Actions.
	void UpdateState(float fAgeAdjust = 0.f);
	void UpdateAudio();
	void ResetLoc()
	                                { m_LastLoc.s = -1.f; }
	void SetLastLoc()
	                                { m_LastLoc = GetSource().GetLocation(); }
	float GetParticleScale() const;
	bool GetMoveRelative(Vec3 & vPreTrans, QuatTS & qtMove) const;
	void UpdateForce();

	bool HasForce() const
	                                { return (m_pForce != 0); }

	i32 EmitParticle(SParticleUpdateContext & context, const EmitParticleData &data, float fAge = 0.f, QuatTS * plocPreTransform = NULL);
	void EmitParticles(SParticleUpdateContext & context);

	u32    GetEmitIndex() const
	                                 { return m_nEmitIndex; }
	u16    GetSequence() const
	                                 { return m_nSequenceIndex; }
	CChaosKey GetChaosKey() const
	                                 { return m_ChaosKey; }

	void OffsetPosition(const Vec3& delta) { m_LastLoc.t += delta; }

	void GetMemoryUsage(IDrxSizer* pSizer) const {}

private:
	// Associated structures.
	CParticleContainer* m_pContainer;         // Direct or shared container to emit particles into.
	_smart_ptr<CParticleSource> m_pSource;

	DrxAudio::ControlId m_startAudioTriggerId;
	DrxAudio::ControlId m_stopAudioTriggerId;
	DrxAudio::ControlId m_audioParameterId;
	DrxAudio::IObject* m_pIAudioObject;
	DrxAudio::EOcclusionType m_currentAudioOcclusionType;
	bool m_bExecuteAudioTrigger;

	// State.
	float m_fStartAge;              // Relative age when scheduled to start (default 0).
	float m_fStopAge;               // Relative age when scheduled to end (fHUGE if never).
	float m_fRepeatAge;             // Relative age when scheduled to repeat (fHUGE if never).
	float m_fLastEmitAge;           // Age of emission of last particle.
	float m_fActivateAge;           // Cached age for last activation mode.

	CChaosKey m_ChaosKey;           // Seed for randomising; inited every pulse.
	QuatTS m_LastLoc;               // Location at time of last update.

	u32 m_nEmitIndex;
	u16 m_nSequenceIndex;

	// External objects.
	IPhysicalEntity* m_pForce;

	// Methods.
	void Initialize(float fAge);
	float GetAgeRelativeTo(float fStopAge, float fAgeAdjust = 0.f) const;
	void DeactivateAudio();
	float ComputeDensityIncrease(float fStrength, float fParticleLife, const QuatTS &locA, const QuatTS * plocB) const;
	Matrix34 GetEmitTM() const;

};

#endif // __particlesubemitter_h__
