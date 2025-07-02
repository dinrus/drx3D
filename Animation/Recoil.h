// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/IAnimationPoseModifier.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

namespace PoseModifier
{

class DRX_ALIGN(32) CRecoil: public IAnimationPoseModifier
{
public:
	struct State
	{
		f32    time;
		f32    duration;
		f32    strengh;
		f32    kickin;
		f32    displacerad;
		u32 arms;

		State()
		{
			Reset();
		}

		void Reset()
		{
			time = 100.0f;
			duration = 0.0f;
			strengh = 0.0f;
			kickin = 0.8f;
			displacerad = 0.0f;
			arms = 3;
		}
	};

public:
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IAnimationPoseModifier)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CRecoil, "AnimationPoseModifier_Recoil", "d7900cb9-e7be-4825-99e1-cc1211f9c561"_drx_guid)

	CRecoil();
	virtual ~CRecoil() {}

public:
	void SetState(const State& state) { m_state = state; m_bStateUpdate = true; }

private:
	f32 RecoilEffect(f32 t);

	// IAnimationPoseModifier
public:
	virtual bool Prepare(const SAnimationPoseModifierParams &params) override;
	virtual bool Execute(const SAnimationPoseModifierParams &params) override;
	virtual void Synchronize() override;

	void GetMemoryUsage(IDrxSizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}

private:
	State m_state;
	State m_stateExecute;
	bool m_bStateUpdate;
};

} //endns PoseModifier
