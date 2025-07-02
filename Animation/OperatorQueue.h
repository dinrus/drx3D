// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/CharacterUpr.h>
#include <drx3D/Animation/SkeletonPose.h>

namespace OperatorQueue
{
struct SValue
{
	f32    n[4];
	QuatT* p;
};

struct SOp
{
	u32 joint;
	u16 target;
	u16 op;
	SValue value;
};
} // OperatorQueue

class DRX_ALIGN(32) COperatorQueue:
	public IAnimationOperatorQueue
{
public:
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IAnimationPoseModifier)
	DRXINTERFACE_ADD(IAnimationOperatorQueue)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(COperatorQueue, "AnimationPoseModifier_OperatorQueue", "ac90f2bc-76a8-43ec-9970-463fb080a520"_drx_guid);

	COperatorQueue();
	virtual ~COperatorQueue() {}

private:
	enum EOpInternal
	{
		eOpInternal_OverrideAbsolute = eOp_Override,
		eOpInternal_OverrideRelative = eOp_OverrideRelative,
		eOpInternal_OverrideWorld    = eOp_OverrideWorld,
		eOpInternal_AdditiveAbsolute = eOp_Additive,
		eOpInternal_AdditiveRelative = eOp_AdditiveRelative,

		eOpInternal_StoreRelative,
		eOpInternal_StoreAbsolute,
		eOpInternal_StoreWorld,

		eOpInternal_ComputeAbsolute,
	};

	enum ETarget
	{
		eTarget_Position,
		eTarget_Orientation,
	};

public:
	virtual void PushPosition(u32 jointIndex, EOp eOp, const Vec3 &value) override;
	virtual void PushOrientation(u32 jointIndex, EOp eOp, const Quat &value) override;

	virtual void PushStoreRelative(u32 jointIndex, QuatT & output) override;
	virtual void PushStoreAbsolute(u32 jointIndex, QuatT & output) override;
	virtual void PushStoreWorld(u32 jointIndex, QuatT & output) override;

	virtual void PushComputeAbsolute() override;

	virtual void Clear() override;

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
	std::vector<OperatorQueue::SOp> m_ops[2];
	u32 m_current;
};
