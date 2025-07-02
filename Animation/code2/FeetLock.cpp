// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/FeetLock.h>

#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/CoreX/Extension/IDrxFactoryRegistryImpl.h>
#include <drx3D/CoreX/Extension/DrxCreateClassInstance.h>
#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/Animation/CharacterInstance.h>
#include <drx3D/Animation/Model.h>
#include <drx3D/Animation/CharacterUpr.h>
#include <drx3D/Animation/PoseModifierHelper.h>

/*

   CFeetPoseStore

 */

DRXREGISTER_CLASS(CFeetPoseStore)

// IAnimationPoseModifier
bool CFeetPoseStore::Execute(const SAnimationPoseModifierParams& params)
{
	DEFINE_PROFILER_FUNCTION();

	Skeleton::CPoseData* pPoseData = Skeleton::CPoseData::GetPoseData(params.pPoseData);
	if (!pPoseData)
		return false;

	const CDefaultSkeleton& rDefaultSkeleton = (const CDefaultSkeleton&)params.GetIDefaultSkeleton();
	QuatT* pRelPose = pPoseData->GetJointsRelative();
	QuatT* pAbsPose = pPoseData->GetJointsAbsolute();

	for (u32 h = 0; h < MAX_FEET_AMOUNT; h++)
	{
		m_pFeetData[h].m_IsEndEffector = 0;
		u32 hinit = rDefaultSkeleton.m_strFeetLockIKHandle[h].size();
		if (hinit == 0)
			continue;
		tukk strLIKSolver = rDefaultSkeleton.m_strFeetLockIKHandle[h].c_str();
		LimbIKDefinitionHandle nHandle = CCrc32::ComputeLowercase(strLIKSolver);
		i32 idxDefinition = rDefaultSkeleton.GetLimbDefinitionIdx(nHandle);
		if (idxDefinition < 0)
			continue;
		const IKLimbType& rIKLimbType = rDefaultSkeleton.m_IKLimbTypes[idxDefinition];
		u32 numLinks = rIKLimbType.m_arrRootToEndEffector.size();
		i32 nRootTdx = rIKLimbType.m_arrRootToEndEffector[0];
		QuatT qWorldEndEffector = pRelPose[nRootTdx];
		for (u32 i = 1; i < numLinks; i++)
		{
			i32 cid = rIKLimbType.m_arrRootToEndEffector[i];
			qWorldEndEffector = qWorldEndEffector * pRelPose[cid];
			qWorldEndEffector.q.Normalize();
		}
		m_pFeetData[h].m_WorldEndEffector = qWorldEndEffector;
		assert(m_pFeetData[h].m_WorldEndEffector.IsValid());
		m_pFeetData[h].m_IsEndEffector = 1;
	}

#ifdef _DEBUG
	u32 numJoints = rDefaultSkeleton.GetJointCount();
	for (u32 j = 0; j < numJoints; j++)
	{
		assert(pRelPose[j].q.IsUnit());
		assert(pAbsPose[j].q.IsUnit());
		assert(pRelPose[j].IsValid());
		assert(pAbsPose[j].IsValid());
	}
#endif

	return false;
}

/*

   CFeetPoseRestore

 */

DRXREGISTER_CLASS(CFeetPoseRestore)

// IAnimationPoseModifier
bool CFeetPoseRestore::Execute(const SAnimationPoseModifierParams& params)
{
	DEFINE_PROFILER_FUNCTION();

	Skeleton::CPoseData* pPoseData = Skeleton::CPoseData::GetPoseData(params.pPoseData);
	if (!pPoseData)
		return false;

	const CDefaultSkeleton& rDefaultSkeleton = (const CDefaultSkeleton&)params.GetIDefaultSkeleton();
	QuatT* const __restrict pAbsPose = pPoseData->GetJointsAbsolute();
	QuatT* const __restrict pRelPose = pPoseData->GetJointsRelative();

	for (u32 i = 0; i < MAX_FEET_AMOUNT; i++)
	{
		if (m_pFeetData[i].m_IsEndEffector == 0)
			continue;
		tukk strLIKSolver = rDefaultSkeleton.m_strFeetLockIKHandle[i].c_str();
		LimbIKDefinitionHandle nHandle = CCrc32::ComputeLowercase(strLIKSolver);
		i32 idxDefinition = rDefaultSkeleton.GetLimbDefinitionIdx(nHandle);
		if (idxDefinition < 0)
			continue;
		assert(m_pFeetData->m_WorldEndEffector.IsValid());
		const IKLimbType& rIKLimbType = rDefaultSkeleton.m_IKLimbTypes[idxDefinition];
		u32 numLinks = rIKLimbType.m_arrRootToEndEffector.size();
		i32 lFootParentIdx = rIKLimbType.m_arrRootToEndEffector[numLinks - 2];
		i32 lFootIdx = rIKLimbType.m_arrRootToEndEffector[numLinks - 1];
		PoseModifierHelper::IK_Solver(PoseModifierHelper::GetDefaultSkeleton(params), nHandle, m_pFeetData[i].m_WorldEndEffector.t, *pPoseData);
		pAbsPose[lFootIdx] = m_pFeetData[i].m_WorldEndEffector;
		pRelPose[lFootIdx] = pAbsPose[lFootParentIdx].GetInverted() * m_pFeetData[i].m_WorldEndEffector;
	}

#ifdef _DEBUG
	u32 numJoints = rDefaultSkeleton.GetJointCount();
	for (u32 j = 0; j < numJoints; j++)
	{
		assert(pRelPose[j].q.IsUnit());
		assert(pAbsPose[j].q.IsUnit());
		assert(pRelPose[j].IsValid());
		assert(pAbsPose[j].IsValid());
	}
#endif

	return true;
}

/*

   CFeetLock

 */

CFeetLock::CFeetLock()
{
	DrxCreateClassInstance(CFeetPoseStore::GetCID(), m_store);
	assert(m_store.get());

	CFeetPoseStore* pStore = static_cast<CFeetPoseStore*>(m_store.get());
	pStore->m_pFeetData = &m_FeetData[0];

	DrxCreateClassInstance(CFeetPoseRestore::GetCID(), m_restore);
	assert(m_restore.get());

	CFeetPoseRestore* pRestore = static_cast<CFeetPoseRestore*>(m_restore.get());
	pRestore->m_pFeetData = &m_FeetData[0];
}
