// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <DrxAnimation/IDrxAnimation.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

class CSkeletonPoseModifier : public IAnimationPoseModifier
{
public:
	struct SJoint
	{
		string m_name;
		i32 m_crc;
		QuatT m_transform;
		i32 m_id;

		SJoint();
		SJoint(const string& name, const QuatT& transform);
	};

public:
	virtual CSkeletonPoseModifier::~CSkeletonPoseModifier() {}

	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IAnimationPoseModifier)
	DRXINTERFACE_END()

	CRYGENERATE_CLASS_GUID(CSkeletonPoseModifier, "AnimationPoseModifier_SkeletonPoseModifier", "1558135d-7db2-42d1-a93b-228776a9d99b"_drx_guid);

public:
	void PoseJoint(const string& name, const QuatT& transform);

	// IAnimationPoseModifier implementation.
	virtual bool Prepare(const SAnimationPoseModifierParams& params) override;
	virtual bool Execute(const SAnimationPoseModifierParams& params) override;
	virtual void Synchronize() override                           {}
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const override {}

private:
	std::vector<SJoint> m_posedJoints;
};
