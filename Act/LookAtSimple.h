// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef LookAtSimple_h
#define LookAtSimple_h

#include <drx3D/CoreX/Extension/ClassWeaver.h>

namespace AnimPoseModifier {

class DRX_ALIGN(32) CLookAtSimple:
	public IAnimationPoseModifier
{
private:
	struct State
	{
		i32 jointId;
		Vec3  jointOffsetRelative;
		Vec3  targetGlobal;
		f32   weight;
	};

public:
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IAnimationPoseModifier)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CLookAtSimple, "AnimationPoseModifier_LookAtSimple", "ba7e2a80-9970-435f-b667-9c08df616d74"_drx_guid);

	CLookAtSimple();
	virtual ~CLookAtSimple() {}

public:
	void SetJointId(u32 id)                      { m_state.jointId = id; }
	void SetJointOffsetRelative(const Vec3& offset) { m_state.jointOffsetRelative = offset; }

	void SetTargetGlobal(const Vec3& target)        { m_state.targetGlobal = target; }

	void SetWeight(f32 weight)                      { m_state.weight = weight; }

private:
	bool ValidateJointId(IDefaultSkeleton & pModelSkeleton);

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
};

} //endns AnimPoseModifier

#endif // LookAtSimple_h
