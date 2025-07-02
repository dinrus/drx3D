// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/DrxCharMorphParams.h>

class DrxModelAnimationContainer;

class DrxModEffMorph
{
public:
	// advances the current time of the played animation and returns the blending factor by which this animation affects the bone pose
	void Tick(f32 fDeltaTime);

	// starts the morphing sequence
	void StartMorph(i32 nMorphTargetId, const DrxCharMorphParams& rParams);

	// returns false when this morph target is inactive
	bool isActive() const;

	// returns the blending factor for this morph target
	f32 getBlending() const;

	// returns the morph target
	i32  getMorphTargetId() const;

	void setTime(f32 fTime)   { m_fTime = fTime; }
	void setSpeed(f32 fSpeed) { m_Params.m_fSpeed = fSpeed; }
	void stop();

	f32  getTime() const { return m_fTime; }
	void freeze()        { m_nFlags |= m_Params.FLAGS_FREEZE; }

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(&m_Params, sizeof(m_Params));
		pSizer->AddObject(m_fTime);
		pSizer->AddObject(m_nMorphTargetId);
		pSizer->AddObject(m_nFlags);
	}
public:

	// the animation container that will answer all questions regarding the morph target
	//DrxModelAnimationContainer* m_pAnimations;

	// the blend time
	DrxCharMorphParams m_Params;
	// time of morphing
	f32                m_fTime;
	// morph target id
	i32                m_nMorphTargetId;
	unsigned           m_nFlags; // the copy of the flags from m_Params
};
