// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/PoseBlenderAim.h>

#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/Animation/CharacterInstance.h>
#include <drx3D/Animation/Model.h>
#include <drx3D/Animation/CharacterUpr.h>
#include <drx3D/Animation/PoseModifierHelper.h>
#include <drx3D/Animation/DirectionalBlender.h>
#include <drx3D/CoreX/Extension/DrxCreateClassInstance.h>

DRXREGISTER_CLASS(CPoseBlenderAim)

//
bool CPoseBlenderAim::Prepare(const SAnimationPoseModifierParams& params)
{
	if (!PrepareInternal(params))
	{
		Synchronize();  // Make sure to synchronize on failure, we dont want m_Get to keep the old values when m_Set is already set
		return false;
	}

	return true;
}

bool CPoseBlenderAim::PrepareInternal(const SAnimationPoseModifierParams& params)
{
	//float fTextColor[4] = {1,0,0,1};
	//g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.4f, fTextColor, false,"Prepare Aim-IK" );
	//g_YLine+=16.0f;

	bool reset = (m_blender.m_dataIn.bUseDirIK == 0) && (m_blender.m_dataOut.fDirIKBlend == 0.0f);

	m_blender.m_dataIn = m_blender.m_Set;
	if (reset)
	{
		// Assume we start aiming within the FOV
		m_blender.m_fFieldOfViewSmooth = 1;
		m_blender.m_fFieldOfViewRate = 0;

		m_blender.ClearSmoothingRates();
	}

	//	float fTextColor[4] = {1,0,0,1};
	//	g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.4f, fTextColor, false,"Update Aim-IK" );
	//	g_YLine+=16.0f;
	m_blender.m_numActiveDirPoses = 0;
	m_blender.m_dataOut.fDirIKInfluence = 0;

	if (Console::GetInst().ca_UseAimIK == 0)
		return false;
	CCharInstance* pInstance = PoseModifierHelper::GetCharInstance(params);
	if (pInstance->m_pDefaultSkeleton->m_ObjectType == CGA)
		return false;  //error-check: we shouldn't execute the PM any more

	u32k useAimIK = (m_blender.m_dataIn.bUseDirIK || m_blender.m_dataOut.fDirIKBlend);
	if (useAimIK == 0)
		return false;

	const CDefaultSkeleton& defaultSkeleton = PoseModifierHelper::GetDefaultSkeleton(params);
	u32 numDirBlends = defaultSkeleton.m_poseBlenderAimDesc.m_blends.size();
	if (numDirBlends == 0)
		return false;  //error-check: we shouldn't execute the PM any more
	for (u32 d = 0; d < numDirBlends; d++)
	{
		if (defaultSkeleton.m_poseBlenderAimDesc.m_blends[d].m_nParaJointIdx < 0 || defaultSkeleton.m_poseBlenderAimDesc.m_blends[d].m_nStartJointIdx < 0 || defaultSkeleton.m_poseBlenderAimDesc.m_blends[d].m_nReferenceJointIdx < 0)
		{
			tukk pModelName = pInstance->m_pDefaultSkeleton->GetModelFilePath();
			g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, pInstance->GetFilePath(), "DinrusXAnimation: No weapon-bone specified for model: %s", pModelName);
			return false;  //error-check: we shouldn't execute the PM any more
		}
	}
	u32 numRotJoints = defaultSkeleton.m_poseBlenderAimDesc.m_rotations.size();
	if (numRotJoints == 0)
	{
		tukk pModelName = pInstance->m_pDefaultSkeleton->GetModelFilePath();
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, pInstance->GetFilePath(), "DinrusXAnimation: No AimIK-Setup for model: %s", pModelName);
		return false;   //error-check: we shouldn't execute the PM any more
	}

	if (defaultSkeleton.m_poseBlenderAimDesc.m_blends.size())
	{
		i32 nParaJointIdx = defaultSkeleton.m_poseBlenderAimDesc.m_blends[0].m_nParaJointIdx;
		if (nParaJointIdx > 0)
		{
			if (m_blender.m_dataIn.bUseDirIK && Console::GetInst().ca_DrawAimPoses)
			{
				g_pAuxGeom->SetRenderFlags(e_Def3DPublicRenderflags);
				static Ang3 angle1(0, 0, 0);
				angle1 += Ang3(0.01f, 0.02f, 0.08f);
				AABB aabb = AABB(Vec3(-0.09f, -0.09f, -0.09f), Vec3(+0.09f, +0.09f, +0.09f));
				OBB obb1 = OBB::CreateOBBfromAABB(Matrix33::CreateRotationXYZ(angle1), aabb);
				g_pAuxGeom->DrawOBB(obb1, m_blender.m_dataIn.vDirIKTarget, 1, RGBA8(0x00, 0x00, 0xff, 0xff), eBBD_Extremes_Color_Encoded);
			}

			const f32 fFrameTime = max(0.0f, params.timeDelta);
			const bool bFadeOut = (m_blender.m_nDirIKDistanceFadeOut || m_blender.m_dataIn.bUseDirIK == 0);
			const f32 fIKBlendRate = bFadeOut ? -m_blender.m_dataIn.fDirIKFadeOutTime : m_blender.m_dataIn.fDirIKFadeInTime;
			const f32 fIkBlendDelta = fIKBlendRate * fFrameTime;
			m_blender.m_dataOut.fDirIKBlend = clamp_tpl(m_blender.m_dataOut.fDirIKBlend + fIkBlendDelta, 0.0f, 1.0f);
		}
	}

	if (m_blender.m_nDirIKDistanceFadeOut == 0 && m_blender.m_dataOut.fDirIKBlend < 0.001f)
		return false;  //IK not visible: we shouldn't execute the PM any more

	CAnimationSet* pAnimationSet = pInstance->m_pDefaultSkeleton->m_pAnimationSet;
	CSkeletonAnim* pSkeletonAnim = PoseModifierHelper::GetSkeletonAnim(params);
	u32 nDirIKLayer = m_blender.m_dataIn.nDirLayer;
	if (nDirIKLayer < 1 || nDirIKLayer >= numVIRTUALLAYERS)
		return false;
	f32 fDirIKLayerWeight = pSkeletonAnim->m_layers[nDirIKLayer].m_transitionQueue.m_fLayerTransitionWeight;
	f32 t0 = 1.0f - fDirIKLayerWeight;
	f32 t1 = fDirIKLayerWeight;
	for (u32 i = 0; i < MAX_EXEC_QUEUE * 2; i++)
	{
		m_blender.m_DirInfo[i].m_fWeight = 0;
		m_blender.m_DirInfo[i].m_nGlobalDirID0 = -1;
	}

	//----------------------------------------------------------------------------------------
	//-----  check if there are dir-poses in the same layer like the DIR-IK              -----
	//-----  this part will be replaced by a VEG (Parametric Sampler)                    -----
	//-----  an Aim-Pose or Look-Pose is simply a 2D-VEG that we can play in any layer   -----
	//----------------------------------------------------------------------------------------
	const DynArray<CAnimation>& rCurLayer = pSkeletonAnim->m_layers[nDirIKLayer].m_transitionQueue.m_animations;
	u32 numAnimsInLayer = rCurLayer.size();
	u32 numActiveAnims = 0;
	for (u32 a = 0; a < numAnimsInLayer; a++)
	{
		if (!rCurLayer[a].IsActivated())
			break;
		numActiveAnims++;
	}
	for (u32 i = 0; i < numActiveAnims; i++)
	{
		i32 nAnimID = rCurLayer[i].GetAnimationId();
		assert(rCurLayer[i].IsActivated());
		const ModelAnimationHeader* pAnim = pAnimationSet->GetModelAnimationHeader(nAnimID);
		assert(pAnim);
		assert(pAnim->m_nGlobalAnimId > -1);
		if (pAnim->m_nAssetType == AIM_File)
		{
			GlobalAnimationHeaderAIM& rGAH = g_AnimationUpr.m_arrGlobalAIM[pAnim->m_nGlobalAnimId];
			u32 numPoses = rGAH.m_arrAimIKPosesAIM.size();
			if (numPoses == 0)
				continue;
			if (rGAH.m_nExist == 0)
				continue;
			SDirInfo& rCurDirInfo = m_blender.m_DirInfo[m_blender.m_numActiveDirPoses];
			rCurDirInfo.m_fWeight = rCurLayer[i].GetTransitionWeight() * t1;
			rCurDirInfo.m_nGlobalDirID0 = pAnim->m_nGlobalAnimId;
			m_blender.m_numActiveDirPoses++;
		}
	}
	if (m_blender.m_numActiveDirPoses == 0)
		return false; // no animation in base-layer, no aim-pose

	return true;
}

bool CPoseBlenderAim::Execute(const SAnimationPoseModifierParams& params)
{
	DEFINE_PROFILER_FUNCTION();

	Skeleton::CPoseData* pPoseData = Skeleton::CPoseData::GetPoseData(params.pPoseData);
	if (!pPoseData)
		return false;

	//		float fTextColor[4] = {1,0,0,1};
	//		g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.4f, fTextColor, false,"Execute Aim-IK" );
	//		g_YLine+=16.0f;
	const CDefaultSkeleton& defaultSkeleton = PoseModifierHelper::GetDefaultSkeleton(params);
	u32 numDB = defaultSkeleton.m_poseBlenderAimDesc.m_blends.size();
	u32 numRotJoints = defaultSkeleton.m_poseBlenderAimDesc.m_rotations.size();
	u32 numPosJoints = defaultSkeleton.m_poseBlenderAimDesc.m_positions.size();

	const SJointsAimIK_Pos* pPos = 0;
	if (numPosJoints)
		pPos = &defaultSkeleton.m_poseBlenderAimDesc.m_positions[0];
	m_blender.ExecuteDirectionalIK(params, &defaultSkeleton.m_poseBlenderAimDesc.m_blends[0], numDB, &defaultSkeleton.m_poseBlenderAimDesc.m_rotations[0], numRotJoints, pPos, numPosJoints);

	//--------------------------------------------------------------------------
	//--- procedural adjustments
	//--------------------------------------------------------------------------
	const CDefaultSkeleton::SJoint* parrModelJoints = &defaultSkeleton.m_arrModelJoints[0];
	QuatT* const __restrict pRelPose = pPoseData->GetJointsRelative();
	QuatT* const __restrict pAbsPose = pPoseData->GetJointsAbsolute();
	const QuatTS& rAnimLocationNext = params.location;
	Vec3 vLocalAimIKTarget = (m_blender.m_dataIn.vDirIKTarget - rAnimLocationNext.t) * rAnimLocationNext.q;
	vLocalAimIKTarget /= params.pCharacterInstance->GetUniformScale();

	for (u32 g = 0; g < numDB; g++)
	{
		u32 numProc = defaultSkeleton.m_poseBlenderAimDesc.m_procAdjustments.size();
		for (u32 i = 0; i < numProc; i++)
		{
			if (defaultSkeleton.m_poseBlenderAimDesc.m_procAdjustments[i].m_nIdx < 0)
				numProc = 0;
		}
		i32 nDirJointIdx = m_blender.m_dbw[g].m_nParaJointIdx;

		f32 wR = m_blender.m_dbw[g].m_fWeight;
		f32 fInfluence = square(square(m_blender.m_dataOut.fDirIKInfluence)) * wR;
		if (nDirJointIdx > 0 && numProc && fInfluence)
		{
			f32 fDistribution = 1.0f / f32(numProc);
			for (u32 d = 0; d < 4; d++)
			{
				Vec3 realdir = pAbsPose[nDirJointIdx].q.GetColumn1();
				Vec3 wantdir = (vLocalAimIKTarget - pAbsPose[nDirJointIdx].t).GetNormalized();
				f32 angle = acos_tpl(realdir | wantdir);
				if (angle < 0.001f)
					continue;

				f32 blend = 1 - std::min(fabsf(angle), 1.0f);

				//	float fColDebug[4] = {1,1,0,1};
				//	g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.3f, fColDebug, false,"angle: %f   blend: %f  fDistribution: %f wR: %f",angle,blend,fDistribution,wR );
				// g_YLine+=16.0f;

				Quat rel;
				rel.SetRotationV0V1(realdir, wantdir);
				rel.SetNlerp(IDENTITY, rel, blend * fDistribution);

				for (u32 numSJ = 1; numSJ < numProc; numSJ++)
				{
					i32 nSpine2 = defaultSkeleton.m_poseBlenderAimDesc.m_procAdjustments[numSJ - 1].m_nIdx;
					i32 nSpine3 = defaultSkeleton.m_poseBlenderAimDesc.m_procAdjustments[numSJ].m_nIdx;
					pAbsPose[nSpine3].q.SetNlerp(pAbsPose[nSpine3].q, rel * pAbsPose[nSpine3].q, fInfluence);
					pRelPose[nSpine3].q = !pAbsPose[nSpine2].q * pAbsPose[nSpine3].q;
				}

				for (u32 r = 0; r < numRotJoints; r++)
				{
					i32 j = defaultSkeleton.m_poseBlenderAimDesc.m_rotations[r].m_nJointIdx;
					i32 p = parrModelJoints[j].m_idxParent;
					pAbsPose[j] = pAbsPose[p] * pRelPose[j];
					pAbsPose[j].q.Normalize();
				}
			}
		}
	}
	return 1;
}
