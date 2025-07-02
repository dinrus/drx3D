// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/SkeletonPose.h>

#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/Animation/CharacterInstance.h>
#include <drx3D/Animation/Model.h>
#include <drx3D/Animation/CharacterUpr.h>
#include <float.h>
#include<drx3D/Animation/FacialInstance.h>
#include <drx3D/Animation/PoseModifierHelper.h>
#include <drx3D/Animation/Recoil.h>
#include <drx3D/Animation/DrawHelper.h>

#define MIN_AABB 0.05f

void CSkeletonPose::SkeletonPostProcess(Skeleton::CPoseData& poseData)
{
	DEFINE_PROFILER_FUNCTION();

	CCharInstance* const __restrict pInstance = m_pInstance;
	QuatTS rAnimLocationNext = m_pInstance->m_location;
	CSkeletonAnim* const __restrict pSkeletonAnim = m_pSkeletonAnim;
	i32 nCurrentFrameID = g_pCharacterUpr->m_nUpdateCounter;
	if (pInstance->m_CharEditMode == 0)
	{
		if (pInstance->m_LastUpdateFrameID_Post == nCurrentFrameID)
			return;
	}

	//

	QuatT* const __restrict pRelativePose = poseData.GetJointsRelative();
	QuatT* const __restrict pAbsolutePose = poseData.GetJointsAbsolute();

	poseData.ValidateRelative(*m_pInstance->m_pDefaultSkeleton);

	// -------------------------------------------------------------------------

	m_pInstance->m_location = rAnimLocationNext;
	if (m_physics.m_bPhysicsRelinquished)
		m_pInstance->m_location.q.SetIdentity();
	//m_pInstance->m_fOriginalDeltaTime = g_pITimer->GetFrameTime();
	//m_pInstance->m_fDeltaTime = m_pInstance->m_fOriginalDeltaTime * m_pInstance->m_fPlaybackScale;

	u32 objectType = pInstance->m_pDefaultSkeleton->m_ObjectType;

	poseData.ValidateRelative(*m_pInstance->m_pDefaultSkeleton);

	if (pSkeletonAnim->m_facialDisplaceInfo.HasUsed() && (pSkeletonAnim->m_IsAnimPlaying || m_bFullSkeletonUpdate))
	{
		// if there are DispacementInfos resulting from the update, apply them now to the bones,
		// all the data that has to be accessed is assembled and a static function is called
		CFacialInstance* pFacialInstance = pInstance->m_pFacialInstance;
		CSkeletonPose* pSkeletonPose = &pFacialInstance->GetMasterCharacter()->m_SkeletonPose;

		const QuatT* const pJointsRelativeDefault = pSkeletonPose->GetPoseDataDefault().GetJointsRelative();

		CFacialModel::ApplyDisplaceInfoToJoints(pSkeletonAnim->m_facialDisplaceInfo, m_pInstance->m_pDefaultSkeleton, &poseData, pJointsRelativeDefault);

		poseData.ValidateRelative(*m_pInstance->m_pDefaultSkeleton);
		poseData.ComputeAbsolutePose(*m_pInstance->m_pDefaultSkeleton, m_pInstance->m_pDefaultSkeleton->m_ObjectType == CHR);
		poseData.ValidateAbsolute(*m_pInstance->m_pDefaultSkeleton);
	}

	poseData.ValidateRelative(*m_pInstance->m_pDefaultSkeleton);

	static ICVar* pUseSinglePositionVar = gEnv->pConsole->GetCVar("g_useSinglePosition");
	if (!pUseSinglePositionVar || ((pUseSinglePositionVar->GetIVal() & 4) == 0))
	{
		m_physics.SynchronizeWithPhysics(poseData);
	}
	m_pSkeletonAnim->PoseModifiersExecutePost(poseData, rAnimLocationNext);

	m_pSkeletonAnim->PoseModifiersSynchronize();

	m_physics.SynchronizeWithPhysicsPost();

	poseData.Validate(*m_pInstance->m_pDefaultSkeleton);

	// -------------------------------------------------------------------------
	if (Console::GetInst().ca_DebugAnimUpdates)
	{
		//just for debugging
		stack_string strModelPath = m_pInstance->m_pDefaultSkeleton->GetModelFilePath();
		g_pCharacterUpr->m_arrSkeletonUpdates.push_back(strModelPath);
		g_pCharacterUpr->m_arrAnimPlaying.push_back(pSkeletonAnim->m_IsAnimPlaying);
		g_pCharacterUpr->m_arrForceSkeletonUpdates.push_back(m_nForceSkeletonUpdate);
		g_pCharacterUpr->m_arrVisible.push_back(m_bInstanceVisible);
	}

	m_pInstance->m_AttachmentUpr.UpdateLocationsExecuteUnsafe(poseData);
	if (m_pInstance->IsCharacterVisible() || m_bFullSkeletonUpdate)
		UpdateBBox();

	pInstance->m_LastUpdateFrameID_Post = nCurrentFrameID;

	if (pSkeletonAnim->m_AnimationDrivenMotion && objectType == CHR)
		pRelativePose[0].SetIdentity();

	if ((m_nForceSkeletonUpdate& ISkeletonPose::kForceSkeletonUpdatesInfinitely) == 0)
	{
		m_nForceSkeletonUpdate--;
		if (m_nForceSkeletonUpdate < 0) m_nForceSkeletonUpdate = 0;
	}

	if (m_pPostProcessCallback)
		(*m_pPostProcessCallback)(m_pInstance, m_pPostProcessCallbackData);

	m_pPoseDataWriteable = NULL;

	if (Console::GetInst().DrawPose('1'))
		DrawHelper::Pose(*m_pInstance->m_pDefaultSkeleton, poseData, QuatT(m_pInstance->m_location), ColorB(0xff, 0xff, 0xff, 0xff));
}

#if defined(_RELEASE)
	#define BBOX_ERROR_CHECKING 0
#else
	#define BBOX_ERROR_CHECKING 1
#endif

void CSkeletonPose::UpdateBBox(u32 update)
{
	DEFINE_PROFILER_FUNCTION();

	const QuatT* const __restrict pAbsolutePose = GetPoseData().GetJointsAbsolute();
	const float* const __restrict pAbsoluteScaling = GetPoseData().GetScalingAbsolute();

	GetPoseData().Validate(*m_pInstance->m_pDefaultSkeleton);

	AABB rAABB;

	u32 nErrorCode = 0;
	const f32 fMaxBBox = 13000.0f;
	rAABB.min.Set(+99999.0f, +99999.0f, +99999.0f);
	rAABB.max.Set(-99999.0f, -99999.0f, -99999.0f);

	CDefaultSkeleton& rCModelSkeleton = *m_pInstance->m_pDefaultSkeleton;
	u32k numCGAJoints = m_arrCGAJoints.size();
	u32k numJoints = rCModelSkeleton.GetJointCount();
	u32k nIncludeList = rCModelSkeleton.m_BBoxIncludeList.size();

	if (numCGAJoints == 0)
	{
		// TODO: Support for bounding box scaling.

		if (rCModelSkeleton.m_usePhysProxyBBox)
		{
			for (u32 i = 0; i < numJoints; i++)
			{
				phys_geometry* pPhysGeom = rCModelSkeleton.m_arrModelJoints[i].m_PhysInfo.pPhysGeom;
				if (pPhysGeom == 0)
					continue; //joint is not physical geometry
				primitives::box bbox;
				pPhysGeom->pGeom->GetBBox(&bbox);
				OBB obb;
				obb.m33 = bbox.Basis.GetTransposed();
				obb.h = bbox.size;
				obb.c = bbox.center * obb.m33;
				AABB aabb = AABB::CreateAABBfromOBB(obb);
				AABB taabb = AABB::CreateTransformedAABB(Matrix34(pAbsolutePose[i]) * obb.m33, aabb);
				rAABB.Add(taabb.min);
				rAABB.Add(taabb.max);
			}
		}
		else
		{
			if (nIncludeList)
			{
				for (uint i = 0; i < nIncludeList; i++)
				{
					i32 nIncludeIdx = m_pInstance->m_pDefaultSkeleton->m_BBoxIncludeList[i];
					rAABB.Add(pAbsolutePose[nIncludeIdx].t);
				}
			}
			else
			{
				for (u32 i = 1; i < numJoints; i++)
				{
					const Vec3& absolutePoseLocation = pAbsolutePose[i].t;
					rAABB.Add(absolutePoseLocation);
				}
			}
		}

		if (numJoints == 1)
		{
			//spacial case: CHRs with one single joint cant have a proper Bbox
			rAABB.min = pAbsolutePose[0].t;
			rAABB.max = pAbsolutePose[0].t;
		}

#if BBOX_ERROR_CHECKING
		for (u32 i = 1; i < numJoints; i++)
		{
			const Vec3& absolutePoseLocation = pAbsolutePose[i].t;
			if (Console::GetInst().ca_AnimWarningLevel >= 2)
			{
				if (fabsf(absolutePoseLocation.x) > 3000.f || fabsf(absolutePoseLocation.y) > 3000.f || fabsf(absolutePoseLocation.z) > 3000.f)
				{
					tukk const jointName = m_pInstance->m_pDefaultSkeleton->GetJointNameByID(i);
					tukk const filePath  = m_pSkeletonAnim->m_pInstance->GetFilePath();

					gEnv->pLog->LogError("Absolute location <%.3f, %.3f, %.3f> for joint at index '%u' with name '%s' is too far away from the origin (model=%s).", absolutePoseLocation.x, absolutePoseLocation.y, absolutePoseLocation.z, i, jointName, filePath);
				}
			}
		}
#endif
	}
	else
	{
		//if we have a CGA, then us this code
		for (u32 i = 0; i < numCGAJoints; ++i)
		{
			if (m_arrCGAJoints[i].m_CGAObjectInstance)
			{
				const QuatTS bone(pAbsolutePose[i].q, pAbsolutePose[i].t, (pAbsoluteScaling ? pAbsoluteScaling[i] : 1.0f));
				const AABB aabb = AABB::CreateTransformedAABB(Matrix34(bone), m_arrCGAJoints[i].m_CGAObjectInstance->GetAABB());
				rAABB.Add(aabb);
			}
		}
	}

#if BBOX_ERROR_CHECKING
	u32 minValid = rAABB.min.IsValid();
	if (minValid == 0) nErrorCode |= 0x4000;
	u32 maxValid = rAABB.max.IsValid();
	if (maxValid == 0) nErrorCode |= 0x4000;
#endif

	CAttachmentUpr* pAttachmentUpr = static_cast<CAttachmentUpr*>(m_pInstance->GetIAttachmentUpr());
	if (pAttachmentUpr->m_TypeSortingRequired)
		pAttachmentUpr->SortByType();

	u32 nMinBoneAttachments = pAttachmentUpr->GetMinJointAttachments();
	u32 nMaxBoneAttachments = pAttachmentUpr->GetMaxJointAttachments();
	for (u32 i = nMinBoneAttachments; i < nMaxBoneAttachments; i++)
	{
		IAttachment* pIAttachment = pAttachmentUpr->GetInterfaceByIndex(i);
		u32 nType = pIAttachment->GetType();
		if (nType != CA_BONE)
			continue;
		CAttachmentBONE& rCAttachment = *((CAttachmentBONE*)pIAttachment);
		if (rCAttachment.m_AttFlags & (FLAGS_ATTACH_NO_BBOX_INFLUENCE | FLAGS_ATTACH_HIDE_MAIN_PASS))
			continue;
		IAttachmentObject* pIAttachmentObject = rCAttachment.m_pIAttachmentObject;
		if (pIAttachmentObject == 0)
			continue;
		IAttachmentObject::EType eAttachmentType = pIAttachmentObject->GetAttachmentType();
		u32 e = (eAttachmentType == IAttachmentObject::eAttachment_Entity);
		u32 c = (eAttachmentType == IAttachmentObject::eAttachment_Skeleton);
		u32 s = (eAttachmentType == IAttachmentObject::eAttachment_StatObj);
		u32 res = (e | c | s);
		if (res == 0)
			continue;

		AABB aabb = pIAttachmentObject->GetAABB();

#if BBOX_ERROR_CHECKING
		minValid = aabb.min.IsValid();
		maxValid = aabb.max.IsValid();
		if (minValid == 0 || maxValid == 0)
		{
			if (e) gEnv->pLog->LogError("DinrusXAnimation: Invalid BBox of eAttachment_Entity");
			if (c) gEnv->pLog->LogError("DinrusXAnimation: Invalid BBox of eAttachment_Skeleton");
			if (s) gEnv->pLog->LogError("DinrusXAnimation: Invalid BBox of eAttachment_StatObj");
			assert(0);
		}

		nErrorCode = 0;
		if (aabb.max.x < aabb.min.x) nErrorCode |= 0x0001;
		if (aabb.max.y < aabb.min.y) nErrorCode |= 0x0001;
		if (aabb.max.z < aabb.min.z) nErrorCode |= 0x0001;
		if (aabb.min.x < -fMaxBBox) nErrorCode |= 0x0002;
		if (aabb.min.y < -fMaxBBox) nErrorCode |= 0x0004;
		if (aabb.min.z < -fMaxBBox) nErrorCode |= 0x0008;
		if (aabb.max.x > +fMaxBBox) nErrorCode |= 0x0010;
		if (aabb.max.y > +fMaxBBox) nErrorCode |= 0x0020;
		if (aabb.max.z > +fMaxBBox) nErrorCode |= 0x0040;

		assert(nErrorCode == 0);
		if (nErrorCode)
		{
			if (e)
			{
				IStatObj* pIStatObj = pIAttachmentObject->GetIStatObj();
				if (pIStatObj)
				{
					tukk strFolderName = pIStatObj->GetFilePath();
					gEnv->pLog->LogError("DinrusXAnimation: Invalid BBox of eAttachment_Entity (%.3f,%.3f,%.3f)-(%.3f,%.3f,%.3f). ModenPath: '%s' ObjPathName '%s'", aabb.min.x, aabb.min.y, aabb.min.z, aabb.max.x, aabb.max.y, aabb.max.z, m_pInstance->m_pDefaultSkeleton->GetModelFilePath(), strFolderName);
				}
				else
				{
					gEnv->pLog->LogError("DinrusXAnimation: Invalid BBox of eAttachment_Entity (%.3f,%.3f,%.3f)-(%.3f,%.3f,%.3f). ModenPath: '%s' ", aabb.min.x, aabb.min.y, aabb.min.z, aabb.max.x, aabb.max.y, aabb.max.z, m_pInstance->m_pDefaultSkeleton->GetModelFilePath());
				}
				assert(0);
			}

			if (c) gEnv->pLog->LogError("DinrusXAnimation: Invalid BBox of eAttachment_Character (%.3f,%.3f,%.3f)-(%.3f,%.3f,%.3f). ModenPath: '%s'  ErrorCode: %08x", aabb.min.x, aabb.min.y, aabb.min.z, aabb.max.x, aabb.max.y, aabb.max.z, m_pInstance->m_pDefaultSkeleton->GetModelFilePath(), nErrorCode);
			if (s) gEnv->pLog->LogError("DinrusXAnimation: Invalid BBox of eAttachment_StatObj (%.3f,%.3f,%.3f)-(%.3f,%.3f,%.3f). ModenPath: '%s'  ErrorCode: %08x", aabb.min.x, aabb.min.y, aabb.min.z, aabb.max.x, aabb.max.y, aabb.max.z, m_pInstance->m_pDefaultSkeleton->GetModelFilePath(), nErrorCode);
		}
#endif

		assert(aabb.min.IsValid());
		assert(aabb.max.IsValid());
		AABB taabb;
		taabb.SetTransformedAABB(Matrix34(rCAttachment.m_AttModelRelative), aabb);
#ifdef _DEBUG
		assert(fabsf(taabb.min.x) < 3000.0f);
		assert(fabsf(taabb.min.y) < 3000.0f);
		assert(fabsf(taabb.min.z) < 3000.0f);
		assert(fabsf(taabb.max.x) < 3000.0f);
		assert(fabsf(taabb.max.y) < 3000.0f);
		assert(fabsf(taabb.max.z) < 3000.0f);
#endif
		assert(taabb.min.IsValid());
		rAABB.Add(taabb.min);
		assert(taabb.max.IsValid());
		rAABB.Add(taabb.max);
		//float fColor[4] = {0,1,0,1};
		//g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"BBox Update:  %s",pAttName),g_YLine+=16.0f;
	}

#if BBOX_ERROR_CHECKING
	if (rAABB.max.x < rAABB.min.x) nErrorCode = 0x0001;
	if (rAABB.max.y < rAABB.min.y) nErrorCode = 0x0001;
	if (rAABB.max.z < rAABB.min.z) nErrorCode = 0x0001;

	minValid = rAABB.min.IsValid();
	if (minValid == 0) nErrorCode |= 0x8000;
	maxValid = rAABB.max.IsValid();
	if (maxValid == 0) nErrorCode |= 0x8000;

	if (rAABB.min.x < -fMaxBBox) nErrorCode |= 0x0002;
	if (rAABB.min.y < -fMaxBBox) nErrorCode |= 0x0004;
	if (rAABB.min.z < -fMaxBBox) nErrorCode |= 0x0008;
	if (rAABB.max.x > +fMaxBBox) nErrorCode |= 0x0010;
	if (rAABB.max.y > +fMaxBBox) nErrorCode |= 0x0020;
	if (rAABB.max.z > +fMaxBBox) nErrorCode |= 0x0040;
	assert(nErrorCode == 0);
#endif

	rAABB.min -= rCModelSkeleton.m_AABBExtension.min;
	rAABB.max += rCModelSkeleton.m_AABBExtension.max;

	const float minAABB2 = MIN_AABB * 2;
	const float fSelectX = (rAABB.max.x - rAABB.min.x) - minAABB2;
	const float fSelectY = (rAABB.max.y - rAABB.min.y) - minAABB2;
	const float fSelectZ = (rAABB.max.z - rAABB.min.z) - minAABB2;

	rAABB.max.x = (float)__fsel(fSelectX, rAABB.max.x, rAABB.max.x + MIN_AABB);
	rAABB.min.x = (float)__fsel(fSelectX, rAABB.min.x, rAABB.min.x - MIN_AABB);
	rAABB.max.y = (float)__fsel(fSelectY, rAABB.max.y, rAABB.max.y + MIN_AABB);
	rAABB.min.y = (float)__fsel(fSelectY, rAABB.min.y, rAABB.min.y - MIN_AABB);
	rAABB.max.z = (float)__fsel(fSelectZ, rAABB.max.z, rAABB.max.z + MIN_AABB);
	rAABB.min.z = (float)__fsel(fSelectZ, rAABB.min.z, rAABB.min.z - MIN_AABB);

#if BBOX_ERROR_CHECKING
	if (nErrorCode)
	{
		gEnv->pLog->LogError("DinrusXAnimation: Invalid BBox (%.3f,%.3f,%.3f)-(%.3f,%.3f,%.3f). ModenPath: '%s'  ErrorCode: %08x", rAABB.min.x, rAABB.min.y, rAABB.min.z, rAABB.max.x, rAABB.max.y, rAABB.max.z, m_pInstance->m_pDefaultSkeleton->GetModelFilePath(), nErrorCode);
		assert(!"Invalid BBox");
		rAABB.min = Vec3(-2, -2, -2);
		rAABB.max = Vec3(2, 2, 2);

		if (Console::GetInst().ca_AnimWarningLevel >= 2)
		{
			if (m_pSkeletonAnim)
			{
				m_pSkeletonAnim->DebugLogTransitionQueueState();
			}
		}
	}
#endif

	m_AABB = rAABB;
}
