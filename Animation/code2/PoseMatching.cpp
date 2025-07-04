// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/PoseMatching.h>

#include <drx3D/Animation/PoseModifierHelper.h>
#include <drx3D/Animation/CharacterUpr.h>

namespace
{

ILINE void InitializePoseRelative(const CDefaultSkeleton& skeleton, QuatT* pResult, uint resultJointCount)
{
	const QuatT* pJointsRelative = skeleton.GetPoseData().GetJointsRelativeMain();
	u32k jointCount = min(skeleton.GetPoseData().GetJointCount(), resultJointCount);
	for (u32 i = 0; i < jointCount; ++i)
		pResult[i] = pJointsRelative[i];
}

ILINE void SamplePoseRelative(const CDefaultSkeleton& skeleton, const GlobalAnimationHeaderCAF& animation, float t, QuatT* pResult, uint resultJointCount)
{
	const Skeleton::CPoseData& poseData = skeleton.GetPoseData();
	const QuatT* pJointsRelativeDefault = poseData.GetJointsRelativeMain();
	const uint jointCount = min(poseData.GetJointCount(), resultJointCount);
	for (uint i = 0; i < jointCount; ++i)
	{
		pResult[i] = pJointsRelativeDefault[i];
		const CDefaultSkeleton::SJoint& joint = skeleton.m_arrModelJoints[i];
		IController* pController = animation.GetControllerByJointCRC32(joint.m_nJointCRC32);
		if (pController)
			pController->GetOP(0.0f, pResult[i].q, pResult[i].t);
	}
}

ILINE bool SamplePoseRelative(const CDefaultSkeleton& rDefaultSkeleton, uint animationId, float t, QuatT* pResult, i32 resultJointCount)
{
	const CAnimationSet& animationSet = *rDefaultSkeleton.m_pAnimationSet;

	const ModelAnimationHeader* pModelAnimation = animationSet.GetModelAnimationHeader(animationId);
	if (!pModelAnimation)
	{
		InitializePoseRelative(rDefaultSkeleton, pResult, resultJointCount);
		return false;
	}

	if (pModelAnimation->m_nAssetType != CAF_File)
	{
		InitializePoseRelative(rDefaultSkeleton, pResult, resultJointCount);
		return false;
	}

	i32k globalId = i32(pModelAnimation->m_nGlobalAnimId);
	if (globalId < 0)
	{
		InitializePoseRelative(rDefaultSkeleton, pResult, resultJointCount);
		return false;
	}

	SamplePoseRelative(rDefaultSkeleton, g_AnimationUpr.m_arrGlobalCAF[globalId], t, pResult, resultJointCount);
	return true;
}

} //endns

/*
   CPoseMatching
 */

DRXREGISTER_CLASS(CPoseMatching)

//

CPoseMatching::CPoseMatching() :
	m_jointStartIndex(0),
	m_pAnimationIds(NULL),
	m_animationCount(0),
	m_animationMatchId(-1)
{
}

//

void CPoseMatching::SetAnimations(const uint* pAnimationIds, uint count)
{
	m_pAnimationIds = pAnimationIds;
	m_animationCount = count;
}

bool CPoseMatching::GetMatchingAnimation(uint& animationId) const
{
	if (m_animationMatchId < 0)
		return false;

	animationId = m_animationMatchId;
	return true;
}

// IAnimationPoseModifier

bool CPoseMatching::Prepare(const SAnimationPoseModifierParams& params)
{
	CCharInstance* pCharacter = PoseModifierHelper::GetCharInstance(params);
	if (!m_jointStartIndex && pCharacter && pCharacter->m_pDefaultSkeleton)
	{
		const uint jointCount = uint(pCharacter->m_pDefaultSkeleton->m_arrModelJoints.size());
		for (uint i = 0; i < jointCount; ++i)
		{
			tukk name = pCharacter->m_pDefaultSkeleton->m_arrModelJoints[i].m_strJointName.c_str();
			if (strstr(name, "Spine"))
			{
				m_jointStartIndex = i;
				break;
			}
		}
	}

	m_animationMatchId = -1;
	return true;
}

bool CPoseMatching::Execute(const SAnimationPoseModifierParams& params)
{
	DEFINE_PROFILER_FUNCTION();

	CCharInstance* pCharInstance = PoseModifierHelper::GetCharInstance(params);
	if (!pCharInstance)
		return false;

	Skeleton::CPoseData* pPoseData = Skeleton::CPoseData::GetPoseData(params.pPoseData);
	if (!pPoseData)
		return false;

	const CDefaultSkeleton& rDefaultSkeleton = *pCharInstance->m_pDefaultSkeleton;
	uint jointStart = m_jointStartIndex;
	i32 jointStartParent = pPoseData->GetParentIndex(rDefaultSkeleton, jointStart);
	if (jointStartParent < 0)
		return false;

	const uint jointCount = min(pPoseData->GetJointCount(), jointStart + 1);

	PREFAST_SUPPRESS_WARNING(6255)
	QuatT * pJointsSampled = (QuatT*)alloca(jointCount * sizeof(QuatT));
	QuatT* pJointsPose = pPoseData->GetJointsAbsolute();

	float differenceMin = 1e10f;
	for (uint i = 0; i < m_animationCount; ++i)
	{
		uint animationId = m_pAnimationIds[i];
		SamplePoseRelative(rDefaultSkeleton, animationId, 0.0f, pJointsSampled, jointCount);

		for (uint j = 0; j < jointCount; ++j)
		{
			i32 parent = pPoseData->GetParentIndex(rDefaultSkeleton, j);
			if (parent > -1)
				pJointsSampled[j] = pJointsSampled[parent] * pJointsSampled[j];
		}

		float difference = 1.0f - (pJointsPose[jointStart].q.GetColumn1().Dot(pJointsSampled[jointStart].q.GetColumn1()));
		if (difference < differenceMin)
		{
			differenceMin = difference;
			m_animationMatchId = animationId;
		}
	}

	return true;
}

void CPoseMatching::Synchronize()
{
}
