// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/Recoil.h>

#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/Animation/CharacterInstance.h>
#include <drx3D/Animation/Model.h>
#include <drx3D/Animation/CharacterUpr.h>
#include <drx3D/Animation/PoseModifierHelper.h>

namespace PoseModifier
{

/*
   CRecoil
 */

DRXREGISTER_CLASS(CRecoil)

//

CRecoil::CRecoil()
{
	m_state.Reset();
	m_stateExecute.Reset();
	m_bStateUpdate = false;
}

// IAnimationPoseModifier

bool CRecoil::Prepare(const SAnimationPoseModifierParams& params)
{
	if (!m_bStateUpdate)
		return true;

	m_stateExecute = m_state;
	m_bStateUpdate = false;
	return true;
}

bool CRecoil::Execute(const SAnimationPoseModifierParams& params)
{
	DEFINE_PROFILER_FUNCTION();

	if (m_stateExecute.time >= m_stateExecute.duration * 2.0f)
		return false;

	Skeleton::CPoseData* pPoseData = Skeleton::CPoseData::GetPoseData(params.pPoseData);
	if (!pPoseData)
		return false;

	const CDefaultSkeleton& defaultSkeleton = PoseModifierHelper::GetDefaultSkeleton(params);

	pPoseData->ComputeAbsolutePose(defaultSkeleton);

	QuatT* const __restrict pRelPose = pPoseData->GetJointsRelative();
	QuatT* const __restrict pAbsPose = pPoseData->GetJointsAbsolute();
	uint jointCount = pPoseData->GetJointCount();

	f32 tn = m_stateExecute.time / m_stateExecute.duration;
	m_stateExecute.time += params.timeDelta;

	//-------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------

	i32 RWeaponBoneIdx = defaultSkeleton.m_recoilDesc.m_weaponRightJointIndex;

	if (RWeaponBoneIdx < 0)
	{
		//	AnimFileWarning(PoseModifierHelper::GetCharInstance(params)->m_pDefaultSkeleton->GetModelFilePath(),"DinrusXAnimation: Invalid Bone Index");
		return false;
	}

	//	QuatT WeaponWorld			= QuatT(params.locationNextAnimation)*pAbsPose[WeaponBoneIdx];
	QuatT RWeaponWorld = pAbsPose[RWeaponBoneIdx];

	f32 fImpact = RecoilEffect(tn);
	//	float fColor[4] = {0,1,0,1};
	//	g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"rRecoil.m_fAnimTime: %f   fImpact: %f",m_state.time,fImpact);	g_YLine+=16.0f;
	//	g_YLine+=16.0f;

	Vec3 vWWeaponX = RWeaponWorld.GetColumn0();
	Vec3 vWWeaponY = RWeaponWorld.GetColumn1();
	Vec3 vWWeaponZ = RWeaponWorld.GetColumn2();
	Vec3 vWRecoilTrans = (-vWWeaponY * fImpact * m_stateExecute.strengh) + (vWWeaponZ * fImpact * m_stateExecute.strengh * 0.4f);

	//	g_pAuxGeom->SetRenderFlags( e_Def3DPublicRenderflags );
	//	g_pAuxGeom->DrawLine(WeaponWorld.t,RGBA8(0x3f,0x3f,0x3f,0x00), WeaponWorld.t+vWWeaponX,RGBA8(0xff,0x00,0x00,0x00) );
	//	g_pAuxGeom->DrawLine(WeaponWorld.t,RGBA8(0x3f,0x3f,0x3f,0x00), WeaponWorld.t+vWWeaponY,RGBA8(0x00,0xff,0x00,0x00) );
	//	g_pAuxGeom->DrawLine(WeaponWorld.t,RGBA8(0x3f,0x3f,0x3f,0x00), WeaponWorld.t+vWWeaponZ,RGBA8(0x00,0x00,0xff,0x00) );

	tukk strRIKSolver = defaultSkeleton.m_recoilDesc.m_ikHandleRight;
	Quat qRRealHandRot;
	u32 nREndEffector = 0;
	if (m_stateExecute.arms & 1)
	{
		LimbIKDefinitionHandle nHandle = CCrc32::ComputeLowercase(strRIKSolver);
		i32 idxDefinition = defaultSkeleton.GetLimbDefinitionIdx(nHandle);
		if (idxDefinition < 0)
			return 0;
		const IKLimbType& rIKLimbType = defaultSkeleton.m_IKLimbTypes[idxDefinition];
		u32 numLinks = rIKLimbType.m_arrJointChain.size();
		nREndEffector = rIKLimbType.m_arrJointChain[numLinks - 1].m_idxJoint;
		Vec3 vRealHandPos = pAbsPose[nREndEffector].t;
		qRRealHandRot = pAbsPose[nREndEffector].q;
		Vec3 LocalGoal = vRealHandPos + vWRecoilTrans;
		PoseModifierHelper::IK_Solver(defaultSkeleton, nHandle, LocalGoal, *pPoseData);
	}

	tukk strLIKSolver = defaultSkeleton.m_recoilDesc.m_ikHandleLeft;
	Quat qLRealHandRot;
	u32 nLEndEffector = 0;
	if (m_stateExecute.arms & 2)
	{
		if (m_stateExecute.arms == 2)
		{
			i32 LWeaponBoneIdx = defaultSkeleton.m_recoilDesc.m_weaponLeftJointIndex;
			if (LWeaponBoneIdx < 0)
				return false;
			QuatT LWeaponWorld = pAbsPose[LWeaponBoneIdx];
			vWWeaponX = LWeaponWorld.GetColumn0();
			vWWeaponY = LWeaponWorld.GetColumn1();
			vWWeaponZ = LWeaponWorld.GetColumn2();
			vWRecoilTrans = (-vWWeaponY * fImpact * m_stateExecute.strengh) + (vWWeaponZ * fImpact * m_stateExecute.strengh * 0.4f);
		}

		LimbIKDefinitionHandle nHandle = CCrc32::ComputeLowercase(strLIKSolver);
		i32 idxDefinition = defaultSkeleton.GetLimbDefinitionIdx(nHandle);
		if (idxDefinition < 0)
			return 0;
		const IKLimbType& rIKLimbType = defaultSkeleton.m_IKLimbTypes[idxDefinition];
		u32 numLinks = rIKLimbType.m_arrJointChain.size();
		nLEndEffector = rIKLimbType.m_arrJointChain[numLinks - 1].m_idxJoint;
		Vec3 vRealHandPos = pAbsPose[nLEndEffector].t;
		qLRealHandRot = pAbsPose[nLEndEffector].q;
		Vec3 LocalGoal = vRealHandPos + vWRecoilTrans;
		PoseModifierHelper::IK_Solver(defaultSkeleton, nHandle, LocalGoal, *pPoseData);
	}

	Matrix33 m33;
	m33.SetRotationAA(m_stateExecute.displacerad, vWWeaponY);
	u32 numJoints = defaultSkeleton.m_recoilDesc.m_joints.size();
	for (u32 i = 0; i < numJoints; i++)
	{
		i32 arm = defaultSkeleton.m_recoilDesc.m_joints[i].m_nArm;
		if (m_stateExecute.arms & arm)
		{
			i32 id = defaultSkeleton.m_recoilDesc.m_joints[i].m_nIdx;
			f32 delay = defaultSkeleton.m_recoilDesc.m_joints[i].m_fDelay;
			f32 weight = defaultSkeleton.m_recoilDesc.m_joints[i].m_fWeight;
			i32 _p0 = defaultSkeleton.m_arrModelJoints[id].m_idxParent;
			if (weight == 1.0f)
				pAbsPose[id].q = Quat::CreateRotationAA(fImpact * m_stateExecute.strengh * 0.5f, m33 * vWWeaponZ) * pAbsPose[id].q;
			pAbsPose[id].t += -vWWeaponY* RecoilEffect(tn - delay) * m_stateExecute.strengh * weight;  //pelvis
			pRelPose[id] = pAbsPose[_p0].GetInverted() * pAbsPose[id];
		}
	}

	if (m_stateExecute.arms & 1)
	{
		pAbsPose[nREndEffector].q = qRRealHandRot;
		i32 pr = defaultSkeleton.m_arrModelJoints[nREndEffector].m_idxParent;
		pRelPose[nREndEffector].q = !pAbsPose[pr].q * pAbsPose[nREndEffector].q;
	}

	if (m_stateExecute.arms & 2)
	{
		pAbsPose[nLEndEffector].q = qLRealHandRot;
		i32 pr = defaultSkeleton.m_arrModelJoints[nLEndEffector].m_idxParent;
		pRelPose[nLEndEffector].q = !pAbsPose[pr].q * pAbsPose[nLEndEffector].q;
	}

	for (u32 i = 1; i < jointCount; i++)
	{
		ANIM_ASSET_ASSERT(pRelPose[i].q.IsUnit());
		ANIM_ASSET_ASSERT(pRelPose[i].IsValid());
		i32 p = defaultSkeleton.m_arrModelJoints[i].m_idxParent;
		pRelPose[p].q.NormalizeSafe();
		pAbsPose[i] = pAbsPose[p] * pRelPose[i];
		pAbsPose[i].q.NormalizeSafe();
	}
#ifdef _DEBUG
	for (u32 j = 0; j < jointCount; j++)
	{
		ANIM_ASSET_ASSERT(pRelPose[j].q.IsUnit());
		ANIM_ASSET_ASSERT(pAbsPose[j].q.IsUnit());
		ANIM_ASSET_ASSERT(pRelPose[j].IsValid());
		ANIM_ASSET_ASSERT(pAbsPose[j].IsValid());
	}
#endif

	return true;
}

void CRecoil::Synchronize()
{
}

f32 CRecoil::RecoilEffect(f32 t)
{
	if (t < 0.0f) t = 0.0f;
	if (t > 1.0f) t = 1.0f;

	f32 sq2 = sqrtf(m_stateExecute.kickin);
	f32 scale = sq2 + gf_PI;

	f32 x = t * scale - sq2;
	if (x < 0.0f)
		return (-(x * x) + 2.0f) * 0.5f;
	return (cosf(x) + 1.0f) * 0.5f;
}

} //endns PoseModifier
