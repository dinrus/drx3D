// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/MultiBuffer.h>

class CDefaultSkeleton;
namespace Memory {
class CPool;
}

namespace Skeleton
{

class CPoseData : public IAnimationPoseData
{
public:

	CPoseData();

	static CPoseData* GetPoseData(IAnimationPoseData* pPoseData);

	void              SetMemoryPool(Memory::CPool* pMemoryPool);

	bool              Initialize(u32 jointCount);
	bool              Initialize(const CDefaultSkeleton& skeleton);
	bool              Initialize(const CPoseData& poseData);

	void              ComputeAbsolutePose(const CDefaultSkeleton& rDefaultSkeleton, bool singleRoot = false);
	void              ComputeRelativePose(const CDefaultSkeleton& rDefaultSkeleton);
	void              ResetToDefault(const CDefaultSkeleton& rDefaultSkeleton);

	i32               GetParentIndex(const CDefaultSkeleton& rDefaultSkeleton, u32k index) const;

	void              GetMemoryUsage(IDrxSizer* pSizer) const;

	/**
	 * Evaluates the total amount of memory dynamically allocated by this object.
	 */
	u32 GetAllocationLength() const;

	/**
	 * Retrieves model-space orientation, position and scale of a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @return Current position, orientation and scale values.
	 */
	const QuatTS GetJointAbsoluteOPS(u32k index) const;

	/**
	 * Retrieves parent-space orientation, position and scale of a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @return Current position, orientation and scale values.
	 */
	const QuatTS GetJointRelativeOPS(u32k index) const;

	//////////////////////////////////////////////////////////
	// IAnimationPoseData implementation
	//////////////////////////////////////////////////////////
	virtual u32       GetJointCount() const override;
	virtual const QuatT& GetJointRelative(u32k index) const override;
	virtual const Vec3&  GetJointRelativeP(u32k index) const override;
	virtual const Quat&  GetJointRelativeO(u32k index) const override;
	virtual const Diag33 GetJointRelativeS(u32k index) const override;
	virtual void         SetJointRelative(u32k index, const QuatT& transformation) override;
	virtual void         SetJointRelativeP(u32k index, const Vec3& position) override;
	virtual void         SetJointRelativeO(u32k index, const Quat& orientation) override;
	virtual void         SetJointRelativeS(u32k index, const float scaling) override;
	virtual const QuatT& GetJointAbsolute(u32k index) const override;
	virtual const Vec3&  GetJointAbsoluteP(u32k index) const override;
	virtual const Quat&  GetJointAbsoluteO(u32k index) const override;
	virtual const Diag33 GetJointAbsoluteS(u32k index) const override;
	virtual void         SetJointAbsolute(u32k index, const QuatT& transformation) override;
	virtual void         SetJointAbsoluteP(u32k index, const Vec3& position) override;
	virtual void         SetJointAbsoluteO(u32k index, const Quat& orientation) override;
	virtual void         SetJointAbsoluteS(u32k index, const float scaling) override;
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// OBSOLETE:
	// Use single joint Set/Get methods instead!
	ILINE QuatT*            GetJointsRelative();
	ILINE const QuatT*      GetJointsRelative() const;

	ILINE QuatT*            GetJointsAbsolute();
	ILINE const QuatT*      GetJointsAbsolute() const;

	ILINE float*            GetScalingRelative();
	ILINE const float*      GetScalingRelative() const;

	ILINE float*            GetScalingAbsolute();
	ILINE const float*      GetScalingAbsolute() const;

	ILINE JointState*       GetJointsStatus();
	ILINE const JointState* GetJointsStatus() const;

	ILINE const QuatT*      GetJointsRelativeMain() const { return GetJointsRelative(); } // TODO: Duplicated functionality, to be removed
	ILINE const QuatT*      GetJointsAbsoluteMain() const { return GetJointsAbsolute(); } // TODO: Duplicated functionality, to be removed
	ILINE const JointState* GetJointsStatusMain() const   { return GetJointsStatus(); }   // TODO: Duplicated functionality, to be removed
	//////////////////////////////////////////////////////////

	void ValidateRelative(const CDefaultSkeleton& rDefaultSkeleton) const;
	void ValidateAbsolute(const CDefaultSkeleton& rDefaultSkeleton) const;
	void Validate(const CDefaultSkeleton& rDefaultSkeleton) const;

private:

	bool AreScalingBuffersAllocated() const;
	bool IsScalingEnabled() const;
	bool EnableScaling();

	typedef Memory::MultiBuffer<NTypelist::CConstruct<QuatT, QuatT, JointState>::TType> JointsBuffer;
	typedef Memory::MultiBuffer<NTypelist::CConstruct<float, float>::TType>             ScalingBuffer;

	JointsBuffer  m_jointsBuffer;
	ScalingBuffer m_scalingBuffer;
	u32        m_jointCount;
};

} //endns Skeleton

namespace Skeleton
{

inline CPoseData* CPoseData::GetPoseData(IAnimationPoseData* pPoseData)
{
	return static_cast<CPoseData*>(pPoseData);
}

inline QuatT* CPoseData::GetJointsRelative()
{
	assert(m_jointsBuffer);
	return m_jointsBuffer.get<0>();
}

inline const QuatT* CPoseData::GetJointsRelative() const
{
	assert(m_jointsBuffer);
	return m_jointsBuffer.get<0>();
}

inline QuatT* CPoseData::GetJointsAbsolute()
{
	assert(m_jointsBuffer);
	return m_jointsBuffer.get<1>();
}

inline const QuatT* CPoseData::GetJointsAbsolute() const
{
	assert(m_jointsBuffer);
	return m_jointsBuffer.get<1>();
}

inline float* CPoseData::GetScalingRelative()
{
	return EnableScaling() ? m_scalingBuffer.get<0>() : nullptr;
}

inline const float* CPoseData::GetScalingRelative() const
{
	return IsScalingEnabled() ? m_scalingBuffer.get<0>() : nullptr;
}

inline float* CPoseData::GetScalingAbsolute()
{
	return EnableScaling() ? m_scalingBuffer.get<1>() : nullptr;
}

inline const float* CPoseData::GetScalingAbsolute() const
{
	return IsScalingEnabled() ? m_scalingBuffer.get<1>() : nullptr;
}

inline JointState* CPoseData::GetJointsStatus()
{
	assert(m_jointsBuffer);
	return m_jointsBuffer.get<2>();
}

inline const JointState* CPoseData::GetJointsStatus() const
{
	assert(m_jointsBuffer);
	return m_jointsBuffer.get<2>();
}

} //endns Skeleton
