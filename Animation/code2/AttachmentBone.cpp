// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/AttachmentBone.h>

#include <drx3D/Animation/AttachmentUpr.h>
#include <drx3D/Animation/CharacterInstance.h>
#include <drx3D/CoreX/Math/QTangent.h>
#include <drx3D/Animation/CharacterUpr.h>

u32 CAttachmentBONE::SetJointName(tukk szJointName)
{
	m_nJointID = -1;
	if (!szJointName)  { assert(0); return 0; }
	i32 nJointID = m_pAttachmentUpr->m_pSkelInstance->m_pDefaultSkeleton->GetJointIDByName(szJointName);
	if (nJointID < 0)
	{
		g_pILog->LogError("SetJointName is called for joint \"%s\", which does not exist in the model \"%s\".", szJointName, m_pAttachmentUpr->m_pSkelInstance->m_pDefaultSkeleton->GetModelFilePath());
		return 0;
	}
	m_strJointName = szJointName;
	ProjectAttachment();
	m_pAttachmentUpr->m_TypeSortingRequired++;
	return 1;
};

u32 CAttachmentBONE::ReName(tukk szSocketName, u32 crc)
{
	m_strSocketName.clear();
	m_strSocketName = szSocketName;
	m_nSocketCRC32 = crc;
	m_pAttachmentUpr->m_TypeSortingRequired++;
	return 1;
};

void CAttachmentBONE::AlignJointAttachment()
{
	CCharInstance* pSkelInstance = m_pAttachmentUpr->m_pSkelInstance;
	i32 nJointCount = pSkelInstance->m_pDefaultSkeleton->GetJointCount();
	if (m_nJointID < nJointCount)
	{
		m_AttAbsoluteDefault = pSkelInstance->m_pDefaultSkeleton->GetDefaultAbsJointByID(m_nJointID);
		m_AttRelativeDefault.SetIdentity();
	}
}

u32 CAttachmentBONE::ProjectAttachment(const Skeleton::CPoseData* pPoseData)
{
	m_nJointID = -1;
	i32 JointId = m_pAttachmentUpr->m_pSkelInstance->m_pDefaultSkeleton->GetJointIDByName(m_strJointName.c_str());
	if (JointId < 0)
		return 0;
	CCharInstance* pSkelInstance = m_pAttachmentUpr->m_pSkelInstance;
	i32 nJointCount = pSkelInstance->m_pDefaultSkeleton->GetJointCount();
	if (JointId >= nJointCount)
		return 0;

	m_nJointID = JointId;
	const CDefaultSkeleton* pDefaultSkeleton = pSkelInstance->m_pDefaultSkeleton;
	QuatT jointQT = pDefaultSkeleton->GetDefaultAbsJointByID(m_nJointID);
	m_AttRelativeDefault = jointQT.GetInverted() * m_AttAbsoluteDefault;
	m_AttFlags |= FLAGS_ATTACH_PROJECTED;

#ifndef _RELEASE
	if (pPoseData && Console::GetInst().ca_DrawAttachmentProjection && (pSkelInstance->m_CharEditMode & CA_DrawSocketLocation))
	{
		if (m_nJointID < 0)
			return 1;
		g_pAuxGeom->SetRenderFlags(e_Def3DPublicRenderflags);
		const QuatTS& rPhysLocation = pSkelInstance->m_location;
		const Vec3 pos = pPoseData->GetJointAbsolute(m_nJointID).t;
		g_pAuxGeom->DrawLine(rPhysLocation * pos, RGBA8(0xff, 0xff, 0xff, 0x00), rPhysLocation * m_AttModelRelative.t, RGBA8(0xff, 0xff, 0xff, 0x00), 10);
	}
#endif
	return 1;
}

void CAttachmentBONE::PostUpdateSimulationParams(bool bAttachmentSortingRequired, tukk pJointName)
{
	m_pAttachmentUpr->m_TypeSortingRequired += bAttachmentSortingRequired;
	m_Simulation.PostUpdate(m_pAttachmentUpr, pJointName);
};

void CAttachmentBONE::Update_Redirected(Skeleton::CPoseData& rPoseData)
{
	if (m_nJointID < 0)
		return;
	if ((m_AttFlags & FLAGS_ATTACH_PROJECTED) == 0)
		ProjectAttachment(&rPoseData);
	const CCharInstance* pSkelInstance = m_pAttachmentUpr->m_pSkelInstance;
	const CDefaultSkeleton& rDefaultSkeleton = *pSkelInstance->m_pDefaultSkeleton;
	m_AttRelativeDefault = rDefaultSkeleton.GetDefaultAbsJointByID(m_nJointID).GetInverted() * m_AttAbsoluteDefault;

	JointState* pJointState = rPoseData.GetJointsStatus();
	if ((pJointState[m_nJointID] & (eJS_Orientation | eJS_Position)) != (eJS_Orientation | eJS_Position))
	{
		u32k pid = rDefaultSkeleton.GetJointParentIDByID(m_nJointID);
		rPoseData.SetJointRelative(m_nJointID, rDefaultSkeleton.GetDefaultRelJointByID(m_nJointID));
		rPoseData.SetJointAbsolute(m_nJointID, rPoseData.GetJointAbsolute(pid) * rDefaultSkeleton.GetDefaultRelJointByID(m_nJointID));
		pJointState[m_nJointID] = eJS_Orientation | eJS_Position;
	}

	m_AttModelRelative = rPoseData.GetJointAbsoluteOPS(m_nJointID) * m_AttRelativeDefault;
	m_Simulation.UpdateSimulation(m_pAttachmentUpr, rPoseData, m_nJointID, QuatT(m_AttModelRelative.q, m_AttModelRelative.t), m_addTransformation, GetName());
	if (m_Simulation.m_crcProcFunction)
		m_pAttachmentUpr->ExecProcFunction(m_Simulation.m_crcProcFunction, &rPoseData);

#ifndef _RELEASE
	if (pSkelInstance->m_CharEditMode & CA_DrawSocketLocation)
	{
		g_pAuxGeom->SetRenderFlags(e_Def3DPublicRenderflags);
		const QuatTS& rPhysLocation = pSkelInstance->m_location;
		const Vec3 pos = rPoseData.GetJointAbsolute(m_nJointID).t;
		g_pAuxGeom->DrawLine(rPhysLocation * pos, RGBA8(0xff, 0x00, 0x00, 0xff), rPhysLocation * m_AttModelRelative.t, RGBA8(0x00, 0xff, 0x00, 0xff));
	}
#endif
}

void CAttachmentBONE::Update_Empty(Skeleton::CPoseData& rPoseData)
{
	if (m_nJointID < 0)
		return;
	if ((m_AttFlags & FLAGS_ATTACH_PROJECTED) == 0)
		ProjectAttachment(&rPoseData);
	m_AttModelRelative = rPoseData.GetJointAbsoluteOPS(m_nJointID) * m_AttRelativeDefault;

#ifndef _RELEASE
	CCharInstance* pSkelInstance = m_pAttachmentUpr->m_pSkelInstance;
	if (pSkelInstance->m_CharEditMode & CA_DrawSocketLocation)
	{
		g_pAuxGeom->SetRenderFlags(e_Def3DPublicRenderflags);
		const QuatTS& rPhysLocation = pSkelInstance->m_location;
		const Vec3 pos = rPoseData.GetJointAbsolute(m_nJointID).t;
		g_pAuxGeom->DrawLine(rPhysLocation * pos, RGBA8(0xff, 0x00, 0x00, 0xff), rPhysLocation * m_AttModelRelative.t, RGBA8(0x00, 0xff, 0x00, 0xff));
	}
#endif

}

void CAttachmentBONE::Update_Static(Skeleton::CPoseData& rPoseData)
{
	if (m_nJointID < 0)
		return;
	if ((m_AttFlags & FLAGS_ATTACH_PROJECTED) == 0)
		ProjectAttachment(&rPoseData);
	m_AttFlags &= FLAGS_ATTACH_VISIBLE ^ -1;
	const f32 fRadiusSqr = m_pIAttachmentObject->GetRadiusSqr();
	if (fRadiusSqr == 0.0f)
		return;  //if radius is zero, then the object is most probably not visible and we can continue
	if (m_pAttachmentUpr->m_fZoomDistanceSq > fRadiusSqr)
		return;  //too small to render. cancel the update

	m_AttModelRelative = rPoseData.GetJointAbsoluteOPS(m_nJointID) * m_AttRelativeDefault;
	if (m_Simulation.m_nClampType)
		m_Simulation.UpdateSimulation(m_pAttachmentUpr, rPoseData, -1, QuatT(m_AttModelRelative.q, m_AttModelRelative.t), m_addTransformation, GetName());
	else
		m_addTransformation = QuatT(IDENTITY);
	m_AttFlags |= FLAGS_ATTACH_VISIBLE;

#ifndef _RELEASE
	CCharInstance* pSkelInstance = m_pAttachmentUpr->m_pSkelInstance;
	if (pSkelInstance->m_CharEditMode & CA_DrawSocketLocation)
	{
		g_pAuxGeom->SetRenderFlags(e_Def3DPublicRenderflags);
		const QuatTS& rPhysLocation = pSkelInstance->m_location;
		const Vec3 pos = rPoseData.GetJointAbsolute(m_nJointID).t;
		g_pAuxGeom->DrawLine(rPhysLocation * pos, RGBA8(0xff, 0x00, 0x00, 0xff), rPhysLocation * m_AttModelRelative.t, RGBA8(0x00, 0xff, 0x00, 0xff));
	}
#endif
}

void CAttachmentBONE::Update_Execute(Skeleton::CPoseData& rPoseData)
{
	if (m_nJointID < 0)
		return;
	if ((m_AttFlags & FLAGS_ATTACH_PROJECTED) == 0)
		ProjectAttachment(&rPoseData);
	m_AttModelRelative = rPoseData.GetJointAbsoluteOPS(m_nJointID) * m_AttRelativeDefault;
	if (m_Simulation.m_nClampType)
		m_Simulation.UpdateSimulation(m_pAttachmentUpr, rPoseData, -1, QuatT(m_AttModelRelative.q, m_AttModelRelative.t), m_addTransformation, GetName());
	else
		m_addTransformation = QuatT(IDENTITY);
	m_pIAttachmentObject->ProcessAttachment(this);

	m_AttFlags &= FLAGS_ATTACH_VISIBLE ^ -1;
	const f32 fRadiusSqr = m_pIAttachmentObject->GetRadiusSqr();
	if (fRadiusSqr == 0.0f)
		return;     //if radius is zero, then the object is most probably not visible and we can continue
	if (m_pAttachmentUpr->m_fZoomDistanceSq > fRadiusSqr)
		return;  //too small to render. cancel the update
	m_AttFlags |= FLAGS_ATTACH_VISIBLE;

#ifndef _RELEASE
	CCharInstance* pSkelInstance = m_pAttachmentUpr->m_pSkelInstance;
	if (pSkelInstance->m_CharEditMode & CA_DrawSocketLocation)
	{
		g_pAuxGeom->SetRenderFlags(e_Def3DPublicRenderflags);
		const QuatTS& rPhysLocation = pSkelInstance->m_location;
		const Vec3 pos = rPoseData.GetJointAbsolute(m_nJointID).t;
		g_pAuxGeom->DrawLine(rPhysLocation * pos, RGBA8(0xff, 0x00, 0x00, 0xff), rPhysLocation * m_AttModelRelative.t, RGBA8(0x00, 0xff, 0x00, 0xff));
	}
#endif
}

const QuatT& CAttachmentBONE::GetAttModelRelative() const
{
	// This conversion technically breaks the strict aliasing rule, but it's unlikely for it to introduce any actual issues on any of our supported platforms.
	// Although both types are standard-layout and share a common initial sequence, we cannot solve this issue with a proper union approach at this point due to the presence of non-trivial constructors.
	// TODO: The Quat* family of types should probably provide a facitility to support these kind of conversions properly.
	return reinterpret_cast<const QuatT&>(m_AttModelRelative);
}

const QuatTS CAttachmentBONE::GetAttWorldAbsolute() const
{
	const QuatTS& rPhysLocation = m_pAttachmentUpr->m_pSkelInstance->m_location;
	return rPhysLocation * m_AttModelRelative;
}

void CAttachmentBONE::UpdateAttModelRelative()
{
	if (m_nJointID < 0)
		return;

	const Skeleton::CPoseData& poseData = m_pAttachmentUpr->m_pSkelInstance->m_SkeletonPose.GetPoseData();
	m_AttModelRelative = poseData.GetJointAbsoluteOPS(m_nJointID) * m_AttRelativeDefault;
}

u32 CAttachmentBONE::Immediate_AddBinding(IAttachmentObject* pIAttachmentObject, ISkin* pISkin, u32 nLoadingFlags)
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

void CAttachmentBONE::Immediate_ClearBinding(u32 nLoadingFlags)
{
	ClearBinding_Internal(true);
};

void CAttachmentBONE::ClearBinding_Internal(bool release)
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

u32 CAttachmentBONE::Immediate_SwapBinding(IAttachment* pNewAttachment)
{
	if (pNewAttachment == NULL || pNewAttachment->GetType() != GetType())
	{
		DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, "CAttachmentBONE::SwapBinding attempting to swap bindings of non-matching attachment types");
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

size_t CAttachmentBONE::SizeOfThis() const
{
	size_t nSize = sizeof(CAttachmentBONE) + sizeofVector(m_strSocketName);
	return nSize;
}

void CAttachmentBONE::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_strSocketName);
}

void CAttachmentBONE::Serialize(TSerialize ser)
{
	if (ser.GetSerializationTarget() != eST_Network)
	{
		ser.BeginGroup("CAttachmentBONE");

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

void CAttachmentBONE::HideAttachment(u32 x)
{
	m_pAttachmentUpr->OnHideAttachment(this, FLAGS_ATTACH_HIDE_MAIN_PASS | FLAGS_ATTACH_HIDE_SHADOW_PASS | FLAGS_ATTACH_HIDE_RECURSION, x != 0);

	if (x)
		m_AttFlags |= (FLAGS_ATTACH_HIDE_MAIN_PASS | FLAGS_ATTACH_HIDE_SHADOW_PASS | FLAGS_ATTACH_HIDE_RECURSION);
	else
		m_AttFlags &= ~(FLAGS_ATTACH_HIDE_MAIN_PASS | FLAGS_ATTACH_HIDE_SHADOW_PASS | FLAGS_ATTACH_HIDE_RECURSION);
}

void CAttachmentBONE::HideInRecursion(u32 x)
{
	m_pAttachmentUpr->OnHideAttachment(this, FLAGS_ATTACH_HIDE_RECURSION, x != 0);

	if (x)
		m_AttFlags |= FLAGS_ATTACH_HIDE_RECURSION;
	else
		m_AttFlags &= ~FLAGS_ATTACH_HIDE_RECURSION;
}

void CAttachmentBONE::HideInShadow(u32 x)
{
	m_pAttachmentUpr->OnHideAttachment(this, FLAGS_ATTACH_HIDE_SHADOW_PASS, x != 0);

	if (x)
		m_AttFlags |= FLAGS_ATTACH_HIDE_SHADOW_PASS;
	else
		m_AttFlags &= ~FLAGS_ATTACH_HIDE_SHADOW_PASS;
}
