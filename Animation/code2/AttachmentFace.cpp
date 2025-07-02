// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/AttachmentFace.h>

#include <drx3D/Animation/AttachmentUpr.h>
#include <drx3D/Animation/CharacterInstance.h>
#include <drx3D/CoreX/Math/QTangent.h>
#include <drx3D/Animation/CharacterUpr.h>

u32 CAttachmentFACE::Immediate_AddBinding(IAttachmentObject* pIAttachmentObject, ISkin* pISkin, u32 nLoadingFlags)
{
	if (m_pIAttachmentObject)
	{
		u32 IsFastUpdateType = m_pAttachmentUpr->IsFastUpdateType(m_pIAttachmentObject->GetAttachmentType());
		if (IsFastUpdateType)
			m_pAttachmentUpr->RemoveEntityAttachment();
	}

	SAFE_RELEASE(m_pIAttachmentObject);
	m_pIAttachmentObject = pIAttachmentObject;

	if (pIAttachmentObject)
	{
		u32 IsFastUpdateType = m_pAttachmentUpr->IsFastUpdateType(pIAttachmentObject->GetAttachmentType());
		if (IsFastUpdateType)
			m_pAttachmentUpr->AddEntityAttachment();
	}
	m_pAttachmentUpr->m_TypeSortingRequired++;
	return 1;
}

void CAttachmentFACE::Immediate_ClearBinding(u32 nLoadingFlags)
{
	ClearBinding_Internal(true);
};

void CAttachmentFACE::ClearBinding_Internal(bool release)
{
	if (m_pIAttachmentObject)
	{
		if (m_pAttachmentUpr->m_pSkelInstance)
		{
			u32 IsFastUpdateType = m_pAttachmentUpr->IsFastUpdateType(m_pIAttachmentObject->GetAttachmentType());
			if (IsFastUpdateType)
				m_pAttachmentUpr->RemoveEntityAttachment();

			if (release)
			{
				m_pIAttachmentObject->Release();
			}

			m_pIAttachmentObject = 0;
		}
	}
	m_pAttachmentUpr->m_TypeSortingRequired++;
}

u32 CAttachmentFACE::Immediate_SwapBinding(IAttachment* pNewAttachment)
{
	if (pNewAttachment == NULL || pNewAttachment->GetType() != GetType())
	{
		DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, "CAttachmentFACE::SwapBinding attempting to swap bindings of non-matching attachment types");
		return 0;
	}

	if (m_pIAttachmentObject)
	{
		if (u32 retVal = static_cast<SAttachmentBase*>(pNewAttachment)->Immediate_AddBinding(m_pIAttachmentObject))
		{
			ClearBinding_Internal(false);
			return retVal;
		}
	}

	return 0;
}

u32 CAttachmentFACE::ProjectAttachment(const Skeleton::CPoseData* pPoseData)
{
	CCharInstance* pSkelInstance = m_pAttachmentUpr->m_pSkelInstance;
	CDefaultSkeleton* pDefaultSkeleton = pSkelInstance->m_pDefaultSkeleton;
	if (pDefaultSkeleton->m_ObjectType != CHR)
		return 0;

	Vec3 apos = GetAttAbsoluteDefault().t;
	ClosestTri cf;
	f32 fShortestDistance = 999999.0f;
	u32 nMeshFullyStreamdIn = 1;

	u32 numAttachments = m_pAttachmentUpr->GetAttachmentCount();
	for (u32 att = 0; att < numAttachments; att++)
	{
		u32 localtype = m_pAttachmentUpr->m_arrAttachments[att]->GetType();
		if (localtype != CA_SKIN)
			continue;
		u32 isHidden = m_pAttachmentUpr->m_arrAttachments[att]->IsAttachmentHidden();
		if (isHidden)
			continue;
		IAttachmentObject* pIAttachmentObject = m_pAttachmentUpr->m_arrAttachments[att]->GetIAttachmentObject();
		if (pIAttachmentObject == 0)
			continue;
		IAttachmentSkin* pIAttachmentSkin = pIAttachmentObject->GetIAttachmentSkin();
		if (pIAttachmentSkin == 0)
			continue;
		CSkin* pCModelSKIN = (CSkin*)pIAttachmentSkin->GetISkin();
		if (pCModelSKIN == 0)
			continue;
		CModelMesh* pCModelMeshSKIN = pCModelSKIN->GetModelMesh(0);
		u32 IsValid = pCModelMeshSKIN->IsVBufferValid();
		if (IsValid == 0)
		{
			nMeshFullyStreamdIn = 0;  //error
			break;
		}
	}

	//--------------------------------------------------------------------

	if (nMeshFullyStreamdIn)
	{
		for (u32 att = 0; att < numAttachments; att++)
		{
			u32 localtype = m_pAttachmentUpr->m_arrAttachments[att]->GetType();
			if (localtype != CA_SKIN)
				continue;
			u32 isHidden = m_pAttachmentUpr->m_arrAttachments[att]->IsAttachmentHidden();
			if (isHidden)
				continue;
			IAttachmentObject* pIAttachmentObject = m_pAttachmentUpr->m_arrAttachments[att]->GetIAttachmentObject();
			if (pIAttachmentObject == 0)
				continue;
			IAttachmentSkin* pIAttachmentSkin = pIAttachmentObject->GetIAttachmentSkin();
			if (pIAttachmentSkin == 0)
				continue;
			CSkin* pCModelSKIN = (CSkin*)pIAttachmentSkin->GetISkin();
			if (pCModelSKIN == 0)
				continue;
			IRenderMesh* pIRenderMeshSKIN = pCModelSKIN->GetIRenderMesh(0);
			if (pIRenderMeshSKIN == 0)
				continue;

			CModelMesh* pCModelMeshSKIN = pCModelSKIN->GetModelMesh(0);
			CAttachmentSKIN* pCAttachmentSkin = (CAttachmentSKIN*)pIAttachmentSkin;
			JointIdType* pRemapTable = &pCAttachmentSkin->m_arrRemapTable[0];
			ClosestTri scf = pCModelMeshSKIN->GetAttachmentTriangle(apos, pRemapTable);
			Vec3 vCenter = (scf.v[0].m_attTriPos + scf.v[1].m_attTriPos + scf.v[2].m_attTriPos) / 3;
			f32 fDistance = (apos - vCenter).GetLength();
			if (fShortestDistance > fDistance)
				fShortestDistance = fDistance, cf = scf;
		}

		if (CModelMesh* pModelMesh = pDefaultSkeleton->GetModelMesh())
		{
			if (pModelMesh->IsVBufferValid())
			{
				ClosestTri scf = pModelMesh->GetAttachmentTriangle(apos, 0);
				Vec3 vTriCenter = (scf.v[0].m_attTriPos + scf.v[1].m_attTriPos + scf.v[2].m_attTriPos) / 3;
				f32 fDistance = (apos - vTriCenter).GetLength();
				if (fShortestDistance > fDistance)
					fShortestDistance = fDistance, cf = scf;
			}
		}

		Vec3 vt0 = cf.v[0].m_attTriPos;
		Vec3 vt1 = cf.v[1].m_attTriPos;
		Vec3 vt2 = cf.v[2].m_attTriPos;
		Vec3 mid = (vt0 + vt1 + vt2) / 3.0f;
		Vec3 x = (vt1 - vt0).GetNormalized();
		Vec3 z = ((vt1 - vt0) % (vt0 - vt2)).GetNormalized();
		Vec3 y = z % x;
		QuatT TriMat = QuatT::CreateFromVectors(x, y, z, mid);
		m_AttRelativeDefault = TriMat.GetInverted() * m_AttAbsoluteDefault;
		m_Triangle = cf;

#ifndef _RELEASE
		if (Console::GetInst().ca_DrawAttachmentProjection && (pSkelInstance->m_CharEditMode & CA_DrawSocketLocation))
		{
			g_pAuxGeom->SetRenderFlags(e_Def3DPublicRenderflags);
			QuatTS& rPhysLocation = pSkelInstance->m_location;
			g_pAuxGeom->DrawLine(rPhysLocation * vt0, RGBA8(0xff, 0xff, 0xff, 0x00), rPhysLocation * vt1, RGBA8(0xff, 0xff, 0xff, 0x00), 5);
			g_pAuxGeom->DrawLine(rPhysLocation * vt1, RGBA8(0xff, 0xff, 0xff, 0x00), rPhysLocation * vt2, RGBA8(0xff, 0xff, 0xff, 0x00), 5);
			g_pAuxGeom->DrawLine(rPhysLocation * vt2, RGBA8(0xff, 0xff, 0xff, 0x00), rPhysLocation * vt0, RGBA8(0xff, 0xff, 0xff, 0x00), 5);
			g_pAuxGeom->DrawLine(rPhysLocation * mid, RGBA8(0xff, 0xff, 0xff, 0x00), rPhysLocation * m_AttAbsoluteDefault.t, RGBA8(0xff, 0xff, 0xff, 0x00), 10);
		}
#endif
		m_AttFlags |= FLAGS_ATTACH_PROJECTED; //success
		return 1;
	}

	return 0;
}

void CAttachmentFACE::ComputeTriMat()
{
	CCharInstance* pSkelInstance = m_pAttachmentUpr->m_pSkelInstance;
	const CDefaultSkeleton* pDefaultSkeleton = pSkelInstance->m_pDefaultSkeleton;
	const CDefaultSkeleton::SJoint* pJoints = &pDefaultSkeleton->m_arrModelJoints[0];
	const QuatT* pJointsAbsolute = &pSkelInstance->m_SkeletonPose.GetPoseData().GetJointAbsolute(0);
	const QuatT* pJointsAbsoluteDefault = pDefaultSkeleton->m_poseDefaultData.GetJointsAbsolute();

	Vec3 tv[3];
	for (u32 t = 0; t < 3; t++)
	{
		DualQuat wquat(IDENTITY);
		wquat.nq.w = 0.0f;
		for (u32 o = 0; o < 4; o++)
		{
			f32 w = m_Triangle.v[t].m_attWeights[o];
			if (w)
			{
				u32 b = m_Triangle.v[t].m_attJointIDs[o];
				i32 p = pJoints[b].m_idxParent;
				DualQuat dqp = pJointsAbsolute[p] * pJointsAbsoluteDefault[p].GetInverted();
				DualQuat dq = pJointsAbsolute[b] * pJointsAbsoluteDefault[b].GetInverted();
				f32 mul = f32(__fsel(dq.nq | dqp.nq, 1.0f, -1.0f));
				dq.nq *= mul;
				dq.dq *= mul;
				wquat += dq * w;
			}
		}
		f32 l = 1.0f / wquat.nq.GetLength();
		wquat.nq *= l;
		wquat.dq *= l;
		tv[t] = wquat * m_Triangle.v[t].m_attTriPos;
	}

	Vec3 tv0 = tv[0];
	Vec3 tv1 = tv[1];
	Vec3 tv2 = tv[2];
	Vec3 mid = (tv0 + tv1 + tv2) / 3.0f;
	Vec3 x = (tv1 - tv0).GetNormalized();
	Vec3 z = ((tv1 - tv0) % (tv0 - tv2)).GetNormalized();
	Vec3 y = z % x;
	QuatT TriMat = QuatT::CreateFromVectors(x, y, z, mid);
	m_AttModelRelative = TriMat * m_AttRelativeDefault;

#ifndef _RELEASE
	if (pSkelInstance->m_CharEditMode & CA_DrawSocketLocation)
	{
		g_pAuxGeom->SetRenderFlags(e_Def3DPublicRenderflags);
		QuatTS& rPhysLocation = pSkelInstance->m_location;
		g_pAuxGeom->DrawLine(rPhysLocation * tv0, RGBA8(0xff, 0xff, 0xff, 0xff), rPhysLocation * tv1, RGBA8(0xff, 0xff, 0xff, 0xff));
		g_pAuxGeom->DrawLine(rPhysLocation * tv1, RGBA8(0xff, 0xff, 0xff, 0xff), rPhysLocation * tv2, RGBA8(0xff, 0xff, 0xff, 0xff));
		g_pAuxGeom->DrawLine(rPhysLocation * tv2, RGBA8(0xff, 0xff, 0xff, 0xff), rPhysLocation * tv0, RGBA8(0xff, 0xff, 0xff, 0xff));
		g_pAuxGeom->DrawLine(rPhysLocation * mid, RGBA8(0xff, 0x00, 0x00, 0xff), rPhysLocation * m_AttModelRelative.t, RGBA8(0x00, 0xff, 0x00, 0xff));

		if (pSkelInstance->m_CharEditMode & CA_BindPose)
		{
			SAuxGeomRenderFlags renderFlags(e_Def3DPublicRenderflags);
			renderFlags.SetDepthTestFlag(e_DepthTestOff);
			g_pAuxGeom->SetRenderFlags(renderFlags);
			g_pAuxGeom->DrawSphere(rPhysLocation * m_AttModelRelative.t, 0.015f, RGBA8(0xff, 0x00, 0x00, 0));
		}
	}
#endif

}

void CAttachmentFACE::PostUpdateSimulationParams(bool bAttachmentSortingRequired, tukk pJointName)
{
	m_pAttachmentUpr->m_TypeSortingRequired += bAttachmentSortingRequired;
	m_Simulation.PostUpdate(m_pAttachmentUpr, pJointName);
};

void CAttachmentFACE::Update_Empty(Skeleton::CPoseData& rPoseData)
{
	if ((m_AttFlags & FLAGS_ATTACH_PROJECTED) == 0)
		ProjectAttachment(&rPoseData); //not projected, so do it now
	if ((m_AttFlags & FLAGS_ATTACH_PROJECTED) == 0)
		return; //Probably failed because mesh was not streamed in. Try again in next frame
	ComputeTriMat();
}

void CAttachmentFACE::Update_Static(Skeleton::CPoseData& rPoseData)
{
	if ((m_AttFlags & FLAGS_ATTACH_PROJECTED) == 0)
	{
		ProjectAttachment(&rPoseData); //not projected, so do it now
		if ((m_AttFlags & FLAGS_ATTACH_PROJECTED) == 0)
			return; //Probably failed because mesh was not streamed in. Try again in next frame
	}
	//This is a CGF. Update and simulate only when visible
	m_AttFlags &= FLAGS_ATTACH_VISIBLE ^ -1;
	const f32 fRadiusSqr = m_pIAttachmentObject->GetRadiusSqr();
	if (fRadiusSqr == 0.0f)
		return;     //if radius is zero, then the object is most probably not visible and we can continue
	if (m_pAttachmentUpr->m_fZoomDistanceSq > fRadiusSqr)
		return;  //too small to render. cancel the update
	m_AttFlags |= FLAGS_ATTACH_VISIBLE;
	ComputeTriMat();
	if (m_Simulation.m_nClampType)
		m_Simulation.UpdateSimulation(m_pAttachmentUpr, rPoseData, -1, m_AttModelRelative, m_addTransformation, GetName());
}

void CAttachmentFACE::Update_Execute(Skeleton::CPoseData& rPoseData)
{
	if ((m_AttFlags & FLAGS_ATTACH_PROJECTED) == 0)
	{
		ProjectAttachment(&rPoseData); //not projected, so do it now
		if ((m_AttFlags & FLAGS_ATTACH_PROJECTED) == 0)
			return; //Probably failed because mesh was not streamed in. Try again in next frame
	}
	ComputeTriMat();
	if (m_Simulation.m_nClampType)
		m_Simulation.UpdateSimulation(m_pAttachmentUpr, rPoseData, -1, m_AttModelRelative, m_addTransformation, GetName());
	m_pIAttachmentObject->ProcessAttachment(this);

	m_AttFlags &= FLAGS_ATTACH_VISIBLE ^ -1;
	const f32 fRadiusSqr = m_pIAttachmentObject->GetRadiusSqr();
	if (fRadiusSqr == 0.0f)
		return;     //if radius is zero, then the object is most probably not visible and we can continue
	if (m_pAttachmentUpr->m_fZoomDistanceSq > fRadiusSqr)
		return;  //too small to render. cancel the update
	m_AttFlags |= FLAGS_ATTACH_VISIBLE;
}

const QuatTS CAttachmentFACE::GetAttWorldAbsolute() const
{
	QuatTS rPhysLocation = m_pAttachmentUpr->m_pSkelInstance->m_location;
	return rPhysLocation * m_AttModelRelative;
};

void CAttachmentFACE::UpdateAttModelRelative()
{
	ComputeTriMat();
}

size_t CAttachmentFACE::SizeOfThis() const
{
	size_t nSize = sizeof(CAttachmentFACE) + sizeofVector(m_strSocketName);
	return nSize;
}

void CAttachmentFACE::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_strSocketName);
}

void CAttachmentFACE::Serialize(TSerialize ser)
{
	if (ser.GetSerializationTarget() != eST_Network)
	{
		ser.BeginGroup("CAttachmentFACE");

		bool bHideInMainPass;

		if (ser.IsWriting())
		{
			bHideInMainPass = ((m_AttFlags & FLAGS_ATTACH_HIDE_MAIN_PASS) == FLAGS_ATTACH_HIDE_MAIN_PASS);
		}

		ser.Value("HideInMainPass", bHideInMainPass);

		if (ser.IsReading())
		{
			HideAttachment(bHideInMainPass);
		}

		ser.EndGroup();
	}
}
void CAttachmentFACE::HideAttachment(u32 x)
{
	m_pAttachmentUpr->OnHideAttachment(this, FLAGS_ATTACH_HIDE_MAIN_PASS | FLAGS_ATTACH_HIDE_SHADOW_PASS | FLAGS_ATTACH_HIDE_RECURSION, x != 0);

	if (x)
		m_AttFlags |= (FLAGS_ATTACH_HIDE_MAIN_PASS | FLAGS_ATTACH_HIDE_SHADOW_PASS | FLAGS_ATTACH_HIDE_RECURSION);
	else
		m_AttFlags &= ~(FLAGS_ATTACH_HIDE_MAIN_PASS | FLAGS_ATTACH_HIDE_SHADOW_PASS | FLAGS_ATTACH_HIDE_RECURSION);
}

void CAttachmentFACE::HideInRecursion(u32 x)
{
	m_pAttachmentUpr->OnHideAttachment(this, FLAGS_ATTACH_HIDE_RECURSION, x != 0);

	if (x)
		m_AttFlags |= FLAGS_ATTACH_HIDE_RECURSION;
	else
		m_AttFlags &= ~FLAGS_ATTACH_HIDE_RECURSION;
}

void CAttachmentFACE::HideInShadow(u32 x)
{
	m_pAttachmentUpr->OnHideAttachment(this, FLAGS_ATTACH_HIDE_RECURSION, x != 0);

	if (x)
		m_AttFlags |= FLAGS_ATTACH_HIDE_SHADOW_PASS;
	else
		m_AttFlags &= ~FLAGS_ATTACH_HIDE_SHADOW_PASS;
}
