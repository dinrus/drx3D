// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/IAnimationPoseModifier.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

class CLimbIk :
	public IAnimationPoseModifier
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IAnimationPoseModifier)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CLimbIk, "AnimationPoseModifier_LimbIk", "3b00bbad-5b9c-4fa4-97e9-b720fcbc8839"_drx_guid)

	CLimbIk();
	virtual ~CLimbIk() {}

private:
	struct Setup
	{
		LimbIKDefinitionHandle setup;
		Vec3                   targetPositionLocal;
	};

private:
	Setup  m_setupsBuffer[2][16];

	Setup* m_pSetups;
	u32 m_setupCount;

	Setup* m_pSetupsExecute;
	u32 m_setupCountExecute;

public:
	void AddSetup(LimbIKDefinitionHandle setup, const Vec3& targetPositionLocal);

	// IAnimationPoseModifier
public:
	virtual bool Prepare(const SAnimationPoseModifierParams& params) override;
	virtual bool Execute(const SAnimationPoseModifierParams& params) override;
	virtual void Synchronize() override;

	void         GetMemoryUsage(IDrxSizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}
};
