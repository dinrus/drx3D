// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/IAnimationPoseModifier.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/Animation/DirectionalBlender.h>

class CPoseBlenderAim : public IAnimationPoseBlenderDir
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IAnimationPoseModifier)
	DRXINTERFACE_ADD(IAnimationPoseBlenderDir)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CPoseBlenderAim, "AnimationPoseModifier_PoseBlenderAim", "058c3e18-b957-4faf-8989-b9cb2cff0d64"_drx_guid)

	virtual ~CPoseBlenderAim() {}

	// This interface
public:
	void SetState(bool state) override                                                    { m_blender.m_Set.bUseDirIK = state; }
	void SetTarget(const Vec3& target) override                                           { if (target.IsValid()) m_blender.m_Set.vDirIKTarget = target; }
	void SetLayer(u32 layer) override                                                  { m_blender.m_Set.nDirLayer = u8(std::max(layer, (u32)1)); }
	void SetFadeoutAngle(f32 angleRadians) override                                       { m_blender.m_Set.fDirIKFadeoutRadians = angleRadians; }
	void SetFadeOutSpeed(f32 time) override                                               { m_blender.m_Set.fDirIKFadeOutTime = (time > 0.0f) ? 1.0f / time : FLT_MAX; }
	void SetFadeInSpeed(f32 time) override                                                { m_blender.m_Set.fDirIKFadeInTime = (time > 0.0f) ? 1.0f / time : FLT_MAX; }
	void SetFadeOutMinDistance(f32 minDistance) override                                  { m_blender.m_Set.fDirIKMinDistanceSquared = minDistance * minDistance; }
	void SetPolarCoordinatesOffset(const Vec2& offset) override                           { m_blender.m_Set.vPolarCoordinatesOffset = offset; }
	void SetPolarCoordinatesSmoothTimeSeconds(f32 smoothTimeSeconds) override             { m_blender.m_Set.fPolarCoordinatesSmoothTimeSeconds = smoothTimeSeconds; }
	void SetPolarCoordinatesMaxRadiansPerSecond(const Vec2& maxRadiansPerSecond) override { m_blender.m_Set.vPolarCoordinatesMaxRadiansPerSecond = maxRadiansPerSecond; }
	f32  GetBlend() const override                                                        { return m_blender.m_Get.fDirIKInfluence; }

public:
	//high-level setup code. most of it it redundant when we switch to VEGs
	virtual bool Prepare(const SAnimationPoseModifierParams& params) override;
	//brute force pose-blending in Jobs
	virtual bool Execute(const SAnimationPoseModifierParams& params) override;
	//sometimes use for final adjustments
	virtual void Synchronize() override { m_blender.m_Get = m_blender.m_dataOut; };

	void         GetMemoryUsage(IDrxSizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}

private:
	bool PrepareInternal(const SAnimationPoseModifierParams& params);

public:
	SDirectionalBlender m_blender;
};
