// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
История:
- 30:8:2005   12:30 : Created by Márcio Martins

*************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Item.h>
#include <drx3D/Game/ItemSharedParams.h>

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/CoreX/String/DrxPath.h>
#include <drx3D/Game/Actor.h>
#include <drx3D/Game/Game.h>
#include <drx3D/Game/PlayerAnimation.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/Game/ItemResourceCache.h>
#include <drx3D/Game/Weapon.h>

#include <drx3D/Game/ItemAnimation.h>

#if !defined(_RELEASE)
	#define DEBUG_ITEM_ACTIONS_ENABLED 1
#else
	#define DEBUG_ITEM_ACTIONS_ENABLED 0
#endif



namespace
{


	void OverrideAttachmentMaterial(IMaterial* pMaterial, CItem* pItem, i32 slot)
	{
		IEntity* pEntity = pItem->GetEntity();
		SEntitySlotInfo info;
		if (!pEntity->GetSlotInfo(slot, info))
			return;

		EntityId parentId = pItem->GetParentId();
		IEntity* pParent = gEnv->pEntitySystem->GetEntity(parentId);
		if (pParent && info.pStatObj)
		{
			ICharacterInstance* pCharacter = pParent->GetCharacter(slot);
			IAttachmentUpr* pAttachmentUpr = pCharacter ? pCharacter->GetIAttachmentUpr() : 0;

			if (pAttachmentUpr)
			{
				i32 attNum = pAttachmentUpr->GetAttachmentCount();
				for (i32 attSlot = 0; attSlot < attNum; ++attSlot)
				{
					IAttachment* pAttachment = pAttachmentUpr->GetInterfaceByIndex(attSlot);			
					IAttachmentObject* pAttObject = pAttachment ? pAttachment->GetIAttachmentObject() : 0;
					if (pAttObject && pAttObject->GetAttachmentType() == IAttachmentObject::eAttachment_StatObj)
					{
						CCGFAttachment* pCGFAtt = static_cast<CCGFAttachment*>(pAttObject);
						if (pCGFAtt->pObj == info.pStatObj)
						{
							pCGFAtt->SetReplacementMaterial(pMaterial,0);
						}
					}
				}
			}
		}
	}


}



//------------------------------------------------------------------------
struct CItem::SwitchHandAction
{
	SwitchHandAction(CItem *_item, i32 _hand): item(_item), hand(_hand) {};
	CItem *item; 
	i32 hand;

	void execute(CItem *_this)
	{
		item->SwitchToHand(hand);
	}
};

//------------------------------------------------------------------------
void CItem::RemoveEntity(bool force)
{
	if (gEnv->IsEditor() && !force)
		Hide(true);
	else if (IsServer() || force)
		gEnv->pEntitySystem->RemoveEntity(GetEntityId());
}

//------------------------------------------------------------------------
bool CItem::CreateCharacterAttachment(i32 slot, tukk name, i32 type, tukk bone)
{
	ICharacterInstance *pCharacter = GetEntity()->GetCharacter(slot);
	if (!pCharacter)
		return false;

	IAttachmentUpr *pAttachmentUpr = pCharacter->GetIAttachmentUpr();
	IAttachment *pAttachment = pAttachmentUpr->GetInterfaceByName(name);

	if (pAttachment)
	{
//		GameWarning("Item '%s' trying to create attachment '%s' which already exists!", GetEntity()->GetName(), name);
		return false;
	}

	pAttachment = pAttachmentUpr->CreateAttachment(name, type, bone);

	if (!pAttachment)
	{
		if (type == CA_BONE)
			GameWarning("Item '%s' failed to create attachment '%s' on bone '%s'!", GetEntity()->GetName(), name, bone);
		return false;
	}

	return true;
}

//------------------------------------------------------------------------
void CItem::DestroyCharacterAttachment(i32 slot, tukk name)
{
	ICharacterInstance *pCharacter = GetEntity()->GetCharacter(slot);
	if (!pCharacter)
		return;

	IAttachmentUpr *pAttachmentUpr = pCharacter->GetIAttachmentUpr();
	pAttachmentUpr->RemoveAttachmentByName(name);
}

ICharacterInstance* CItem::GetAppropriateCharacter(i32 slot, bool owner)
{
	ICharacterInstance* pCharacter = NULL;
	if(owner)
	{
		IEntity* pOwner = GetOwner();
		if(pOwner)
		{
			pCharacter = pOwner->GetCharacter(0);
		}
	}
	else
	{
		pCharacter = GetEntity()->GetCharacter(slot);
	}

	return pCharacter;
}

//------------------------------------------------------------------------
void CItem::ResetCharacterAttachment(i32 slot, tukk name, bool owner, EntityId attachedEntID)
{
	if(IAttachment * pAttachment = GetCharacterAttachment(GetAppropriateCharacter(slot, owner), name))
	{
		//--- Early out if this is not a matching entity. Very specific minimal fix for DT: 32803
		//--- where we know that the bug is going from entityAttachment to another entityAttachment
		if (attachedEntID && pAttachment->GetIAttachmentObject())
		{
			if (pAttachment->GetIAttachmentObject()->GetAttachmentType() == IAttachmentObject::eAttachment_Entity)
			{
				CEntityAttachment *entAttachment = (CEntityAttachment *)pAttachment->GetIAttachmentObject();

				if (attachedEntID != entAttachment->GetEntityId())
				{
					return;
				}				
			}
		}

		pAttachment->ClearBinding();
	}
}

//------------------------------------------------------------------------
IMaterial* CItem::GetCharacterAttachmentMaterial(i32 slot, tukk name, bool owner)
{
	IAttachmentObject* pAttachmentObject = NULL;
	if(IAttachment * pAttachment = GetCharacterAttachment(GetAppropriateCharacter(slot, owner), name))
	{
		pAttachmentObject = pAttachment->GetIAttachmentObject();
	}
	
	return pAttachmentObject ? (IMaterial*)pAttachmentObject->GetBaseMaterial() : 0;
}

//------------------------------------------------------------------------
tukk CItem::GetCharacterAttachmentBone(i32 slot, tukk name)
{
	ICharacterInstance *pCharacter = GetEntity()->GetCharacter(slot);
	if(IAttachment * pAttachment = GetCharacterAttachment(pCharacter, name))
	{
		return pCharacter->GetIDefaultSkeleton().GetJointNameByID(pAttachment->GetJointID());
	}
	
	return NULL;
}

//------------------------------------------------------------------------
void CItem::SetCharacterAttachment(i32 slot, tukk name, IEntity *pEntity, bool owner)
{
	if(IAttachment * pAttachment = GetCharacterAttachment(GetAppropriateCharacter(slot, owner), name))
	{
		CEntityAttachment *pEntityAttachment = new CEntityAttachment();
		pEntityAttachment->SetEntityId(pEntity->GetId());

		pAttachment->AddBinding(pEntityAttachment);
		pAttachment->HideAttachment(0);
	}
}

//------------------------------------------------------------------------
void CItem::SetCharacterAttachment(i32 slot, tukk name, IStatObj *pObj, bool owner)
{
	if(IAttachment * pAttachment = GetCharacterAttachment(GetAppropriateCharacter(slot, owner), name))
	{
		CCGFAttachment *pStatAttachment = new CCGFAttachment();
		pStatAttachment->pObj  = pObj;

		pAttachment->AddBinding(pStatAttachment);
	}
}

//------------------------------------------------------------------------
void CItem::SetCharacterAttachment(i32 slot, tukk name, ICharacterInstance *pAttachedCharacter, bool owner)
{
	if(IAttachment * pAttachment = GetCharacterAttachment(GetAppropriateCharacter(slot, owner), name))
	{
		//--- Stop attached characters from updating via the entity
		pAttachedCharacter->SetFlags(pAttachedCharacter->GetFlags() & ~CS_FLAG_UPDATE);

		CSKELAttachment *pCharacterAttachment = new CSKELAttachment();
		pCharacterAttachment->m_pCharInstance  = pAttachedCharacter;

		pAttachment->AddBinding(pCharacterAttachment);
	}
}

//------------------------------------------------------------------------
void CItem::SetCharacterAttachment(i32 slot, tukk name, IEntity *pEntity, i32 objSlot, bool owner)
{
	SEntitySlotInfo info;
	if (!pEntity->GetSlotInfo(objSlot, info))
		return;

	if (info.pCharacter)
		SetCharacterAttachment(slot, name, info.pCharacter, owner);
	else if (info.pStatObj)
		SetCharacterAttachment(slot, name, info.pStatObj, owner);
}

//------------------------------------------------------------------------
IAttachment * CItem::GetCharacterAttachment(ICharacterInstance * pCharacter, tukk name) const
{
	if(pCharacter)
	{
		IAttachmentUpr *pAttachmentUpr = pCharacter->GetIAttachmentUpr();
		if (IAttachment * pAttachment = pAttachmentUpr->GetInterfaceByName(name))
		{
			return pAttachment;
		}

		GameWarning("Item '%s' trying to get attachment '%s' which does not exist!", GetEntity()->GetName(), name);
	}

	return NULL;
}

//------------------------------------------------------------------------
void CItem::SetCharacterAttachmentLocalTM(i32 slot, tukk name, const Matrix34 &tm)
{
	if(IAttachment * pAttachment = GetCharacterAttachment(GetEntity()->GetCharacter(slot), name))
		pAttachment->SetAttRelativeDefault( QuatT(tm));
}

//------------------------------------------------------------------------
void CItem::SetCharacterAttachmentWorldTM(i32 slot, tukk name, const Matrix34 &tm)
{
	ICharacterInstance *pCharacter = GetEntity()->GetCharacter(slot);
	if(IAttachment * pAttachment = GetCharacterAttachment(pCharacter, name))
	{
		Matrix34 boneWorldMatrix = GetEntity()->GetSlotWorldTM(slot) *	Matrix34(pCharacter->GetISkeletonPose()->GetAbsJointByID(pAttachment->GetJointID()) );

		Matrix34 localAttachmentMatrix = (boneWorldMatrix.GetInverted()*tm);
		pAttachment->SetAttRelativeDefault(QuatT(localAttachmentMatrix));
	}
}

//------------------------------------------------------------------------
Matrix34 CItem::GetCharacterAttachmentLocalTM(i32 slot, tukk name)
{
	if(IAttachment * pAttachment = GetCharacterAttachment(GetEntity()->GetCharacter(slot), name))
	{
		return Matrix34(pAttachment->GetAttRelativeDefault());
	}
	else
	{
		return Matrix34::CreateIdentity();
	}
}

//------------------------------------------------------------------------
Matrix34 CItem::GetCharacterAttachmentWorldTM(i32 slot, tukk name)
{
	if(IAttachment * pAttachment = GetCharacterAttachment(GetEntity()->GetCharacter(slot), name))
	{
		return Matrix34(pAttachment->GetAttWorldAbsolute());
	}
	else
	{
		return Matrix34::CreateIdentity();
	}
}

//------------------------------------------------------------------------
void CItem::HideCharacterAttachment(i32 slot, tukk name, bool hide)
{
	if(IAttachment * pAttachment = GetCharacterAttachment(GetEntity()->GetCharacter(slot), name))
	{
		pAttachment->HideAttachment(hide?1:0);
	}
}

//------------------------------------------------------------------------
void CItem::HideCharacterAttachmentMaster(i32 slot, tukk name, bool hide)
{
	ICharacterInstance *pCharacter = GetEntity()->GetCharacter(slot);
	if (!pCharacter)
		return;

	pCharacter->HideMaster(hide?1:0);
}

//------------------------------------------------------------------------
void CItem::CreateAttachmentHelpers(i32 slot)
{
	for (THelperVector::const_iterator it = m_sharedparams->helpers.begin(); it != m_sharedparams->helpers.end(); ++it)
	{
		if (it->slot != slot)
			continue;

		CreateCharacterAttachment(slot, it->name.c_str(), CA_BONE, it->bone.c_str());
	}
}

//------------------------------------------------------------------------
void CItem::DestroyAttachmentHelpers(i32 slot)
{
	for (THelperVector::const_iterator it = m_sharedparams->helpers.begin(); it != m_sharedparams->helpers.end(); ++it)
	{
		if (it->slot != slot)
			continue;

		DestroyCharacterAttachment(slot, it->name.c_str());
	}
}

//------------------------------------------------------------------------
const THelperVector& CItem::GetAttachmentHelpers()
{
	return m_sharedparams->helpers;
}

//------------------------------------------------------------------------
bool CItem::SetGeometry(i32 slot, const ItemString& name, const ItemString& material, bool useParentMaterial, const Vec3& poffset, const Ang3& aoffset, float scale, bool forceReload)
{
	assert(slot >= 0 && slot < eIGS_Last);

	bool changedfp=false;
	switch(slot)
	{
	case eIGS_Owner:
		break;
	case eIGS_FirstPerson:
	case eIGS_ThirdPerson:
	default:
		{
			if (name.empty() || forceReload)
			{
				GetEntity()->FreeSlot(slot);
#ifndef ITEM_USE_SHAREDSTRING
				m_geometry[slot].resize(0);
#else
				m_geometry[slot].reset();
#endif
			}
	
			DestroyAttachmentHelpers(slot);

			if (!name.empty())
			{
				if (m_geometry[slot] != name)
				{
					tukk ext = PathUtil::GetExt(name.c_str());
					if ((stricmp(ext, "chr") == 0) || (stricmp(ext, "cdf") == 0) || (stricmp(ext, "cga") == 0) )
						GetEntity()->LoadCharacter(slot, name.c_str(), 0);
					else
						GetEntity()->LoadGeometry(slot, name.c_str(), 0, 0);

					changedfp=slot==eIGS_FirstPerson;
				}
				
				CreateAttachmentHelpers(slot);
			}

/*			if (slot == eIGS_FirstPerson)
			{
				ICharacterInstance *pCharacter = GetEntity()->GetCharacter(eIGS_FirstPerson);
				if (pCharacter)
				{
					pCharacter->SetFlags(pCharacter->GetFlags()&(~CS_FLAG_UPDATE));
				}
			}
      else */if (slot == eIGS_Destroyed)
        DrawSlot(eIGS_Destroyed, false);
		}
		break;
	}

	Matrix34 slotTM;
	slotTM = Matrix34::CreateRotationXYZ(aoffset);
	slotTM.ScaleColumn(Vec3(scale, scale, scale));
	slotTM.SetTranslation(poffset);
	GetEntity()->SetSlotLocalTM(slot, slotTM);

	if (changedfp && m_stats.mounted)
	{
		if (m_sharedparams->pMountParams && !m_sharedparams->pMountParams->pivot.empty())
		{
			Matrix34 tm=GetEntity()->GetSlotLocalTM(eIGS_FirstPerson, false);
			Vec3 pivot = GetSlotHelperPos(eIGS_FirstPerson, m_sharedparams->pMountParams->pivot.c_str(), false);
			tm.AddTranslation(pivot);

			GetEntity()->SetSlotLocalTM(eIGS_FirstPerson, tm);
		}

		GetEntity()->InvalidateTM();
	}

	m_geometry[slot] = name ? name : ItemString();

	ReAttachAccessories();
	
	IEntity* pParentEntity = gEnv->pEntitySystem->GetEntity(GetParentId());
	IMaterial* pOverrideMaterial = 0;
	if (!material.empty())
	{
		pOverrideMaterial = gEnv->p3DEngine->GetMaterialUpr()->LoadMaterial(material.c_str());
	}
	else if (useParentMaterial && pParentEntity)
	{
		ICharacterInstance* pParentCharacter = pParentEntity->GetCharacter(slot);
		IEntityRenderProxy* pParentRenderProxy = static_cast<IEntityRenderProxy*>(pParentEntity->GetProxy(ENTITY_PROXY_RENDER));
		if (pParentCharacter)
			pOverrideMaterial = pParentCharacter->GetIMaterial();
		else if (pParentRenderProxy)
			pOverrideMaterial = pParentRenderProxy->GetSlotMaterial(slot);
	}
	if (pOverrideMaterial)
	{
		ICharacterInstance* pCharacter = GetEntity()->GetCharacter(slot);
		IEntityRenderProxy* pRenderProxy = static_cast<IEntityRenderProxy*>(GetEntity()->GetProxy(ENTITY_PROXY_RENDER));
		OverrideAttachmentMaterial(pOverrideMaterial, this, slot);
		if (pCharacter)
			pCharacter->SetIMaterial_Instance(pOverrideMaterial);
		else if (pRenderProxy)
			pRenderProxy->SetSlotMaterial(slot, pOverrideMaterial);
	}

	if(slot == eIGS_FirstPerson && IsSelected())
	{
		CActor* pOwnerActor = GetOwnerActor();
		IActionController *pActionController = GetActionController();
		if(pActionController && pOwnerActor && pOwnerActor->IsClient())
		{
			UpdateScopeContexts(pActionController);
		}
	}

	return true;
}

//------------------------------------------------------------------------
bool CItem::PlayFragment(IAction* pAction, float speedOverride, float timeOverride, float animWeight, float ffeedbackWeight, bool concentratedFire)
{
	_smart_ptr<IAction> pActionPtr(pAction);

	DRX_ASSERT(pAction);
	if(!pAction)
	{
		return false;
	}

	CWeapon *pWeapon = static_cast<CWeapon*>(GetIWeapon());
	if (pWeapon && pWeapon->IsProxyWeapon())
	{
		return false;
	}

	bool success = false;

	float speed = (float)__fsel(-speedOverride, 1.0f, speedOverride);
	FragmentID fragID = pAction->GetFragmentID();
	pAction->SetSubContext(m_subContext);
	IActionController *pActionController = GetActionController();
	if ((fragID != FRAGMENT_ID_INVALID) && pActionController)
	{
		float fragmentDuration, transitionDuration;
		if (pActionController->QueryDuration(*pAction, fragmentDuration, transitionDuration))
		{					
			float duration = fragmentDuration+transitionDuration;
			if ((duration > 0.0f) && (timeOverride > 0.0f))
			{
				speed = (duration / timeOverride);
				DRX_ASSERT((speed > 0.0f) && (speed < 99999.0f));
			}

			if(duration > 0.f)
			{
				m_animationTime[eIGS_Owner] = (u32) MAX((duration*1000.0f/speed) - 20, 0.0f);
			}

			pAction->SetSpeedBias(speed);
			pAction->SetAnimWeight(animWeight);

			if(concentratedFire)
			{
				pAction->SetParam(CItem::sActionParamCRCs.concentratedFire, 1.f);
			}

			if(ffeedbackWeight != 1.f)
			{
				pAction->SetParam(CItem::sActionParamCRCs.ffeedbackScale, ffeedbackWeight);
			}

			pActionController->Queue(*pAction);

			success = true;
		}
	}

	return success;
}

//------------------------------------------------------------------------
i32 CItem::GetFragmentID(tukk actionName, const CTagDefinition** tagDef)
{
	i32 fragmentID = FRAGMENT_ID_INVALID;

	IActionController *pActionController = GetActionController();
	if (pActionController)
	{
		SAnimationContext &animContext = pActionController->GetContext();
		fragmentID = animContext.controllerDef.m_fragmentIDs.Find(actionName);

		if(tagDef && fragmentID != FRAGMENT_ID_INVALID)
		{
			*tagDef = animContext.controllerDef.GetFragmentTagDef(fragmentID);
		}
	}

	return fragmentID;
}

//------------------------------------------------------------------------
_smart_ptr<IAction> CItem::PlayAction(FragmentID action, i32 layer, bool loop, u32 flags, float speedOverride, float animWeigth, float ffeedbackWeight)
{
	_smart_ptr<IAction> pAction;

	const CWeapon* pWeapon = static_cast<CWeapon*>(GetIWeapon());
	if (pWeapon && pWeapon->IsProxyWeapon())
	{
		return pAction;
	}

	IActionController* pActionController = GetActionController();

	if (pActionController && action != FRAGMENT_ID_INVALID)
	{
		SAnimationContext& animContext = pActionController->GetContext();
		const CTagDefinition* pTagDefinition = animContext.controllerDef.GetFragmentTagDef(action);
		float timeOverride = -1.0f;

		bool concentratedFire = (flags&eIPAF_ConcentratedFire) != 0;

		TagState actionTags = TAG_STATE_EMPTY;

		if (pTagDefinition)
		{
			CTagState fragTags(*pTagDefinition);

			SetFragmentTags(fragTags);
			actionTags = fragTags.GetMask();
		}
			
		pAction = new CItemAction(PP_PlayerAction, action, actionTags);

		PlayFragment(pAction, speedOverride, timeOverride, animWeigth, ffeedbackWeight, concentratedFire);
	}

	return pAction;
}

//------------------------------------------------------------------------
u32 CItem::GetCurrentAnimationTime(i32 slot)
{
	return m_animationTime[slot];
}

//------------------------------------------------------------------------
void CItem::DrawSlot(i32 slot, bool bDraw, bool bNear)
{
	u32 flags = GetEntity()->GetSlotFlags(slot);
	if (bDraw)
		flags |= ENTITY_SLOT_RENDER;
	else
		flags &= ~ENTITY_SLOT_RENDER;

	if (bNear)
		flags |= ENTITY_SLOT_RENDER_NEAREST;
	else
		flags &= ~ENTITY_SLOT_RENDER_NEAREST;
	
	GetEntity()->SetSlotFlags(slot, flags);
}

//------------------------------------------------------------------------
Vec3 CItem::GetSlotHelperPos(i32 slot, tukk helper, bool worldSpace, bool relative) const
{
	Vec3 position(0,0,0);

	SEntitySlotInfo info;
	if (GetEntity()->GetSlotInfo(slot, info))
	{
		if (info.pStatObj)
		{
			IStatObj *pStatsObj = info.pStatObj;
			position = pStatsObj->GetHelperPos(helper);
			position = GetEntity()->GetSlotLocalTM(slot, false).TransformPoint(position);
		}
		else if (info.pCharacter)
		{
			ICharacterInstance *pCharacter = info.pCharacter;
			IAttachment* pAttachment = pCharacter->GetIAttachmentUpr()->GetInterfaceByName(helper);
			if (pAttachment)
			{
				position = worldSpace ? pAttachment->GetAttWorldAbsolute().t : pAttachment->GetAttModelRelative().t;
				return position;
			}
			else
			{
				IDefaultSkeleton& rIDefaultSkeleton = pCharacter->GetIDefaultSkeleton();
				ISkeletonPose* pSkeletonPose = pCharacter->GetISkeletonPose();
				i16 id = rIDefaultSkeleton.GetJointIDByName(helper);
				if (id > -1)
				{
					position = relative ? pSkeletonPose->GetRelJointByID(id).t : pSkeletonPose->GetAbsJointByID(id).t;
				}
			}

			if (!relative)
			{
				position = GetEntity()->GetSlotLocalTM(slot, false).TransformPoint(position);
			}
		}
	}

	if (worldSpace)
	{
		position = GetWorldTM().TransformPoint(position);
	}

	return position;
}

//------------------------------------------------------------------------
const Matrix33 &CItem::GetSlotHelperRotation(i32 slot, tukk helper, bool worldSpace, bool relative)
{
	static Matrix33 rotation;
	rotation.SetIdentity();

	IEntity* pEntity = GetEntity();
	if(!pEntity)
		return rotation;

	SEntitySlotInfo info;
	if (pEntity->GetSlotInfo(slot, info))
	{
    if (info.pStatObj)
    {
      IStatObj *pStatObj = info.pStatObj;
      rotation = Matrix33(pStatObj->GetHelperTM(helper));
      rotation.OrthonormalizeFast();
      rotation = Matrix33(GetEntity()->GetSlotLocalTM(slot, false))*rotation;        
    }
		else if (info.pCharacter)
		{
			ICharacterInstance *pCharacter = info.pCharacter;
			if(!pCharacter)
				return rotation;

			IAttachment* pAttachment = pCharacter->GetIAttachmentUpr()->GetInterfaceByName(helper);
			if(pAttachment)
			{
				rotation = Matrix33(worldSpace ? pAttachment->GetAttWorldAbsolute().q : pAttachment->GetAttModelRelative().q);
				return rotation;
			}
			else
			{
				IDefaultSkeleton& rIDefaultSkeleton = pCharacter->GetIDefaultSkeleton();
				ISkeletonPose* pSkeletonPose = pCharacter->GetISkeletonPose();
				i16 id = rIDefaultSkeleton.GetJointIDByName(helper);
				if (id > -1)
				{
					rotation = relative ? Matrix33(pSkeletonPose->GetRelJointByID(id).q) : Matrix33(pSkeletonPose->GetAbsJointByID(id).q);
				}
			}

			if (!relative)
			{
				rotation = Matrix33(pEntity->GetSlotLocalTM(slot, false)) * rotation;
			}
		}    
	}

	if (worldSpace)
	{
		rotation = Matrix33(pEntity->GetWorldTM()) * rotation;
	}

	return rotation;
}

//------------------------------------------------------------------------
//void CItem::StopSound(tSoundID id)
//{
//  if (id == INVALID_SOUNDID)
//    return;
//
//	bool synchSound = false;
//	IEntityAudioProxy *pIEntityAudioProxy = GetAudioProxy(false);
//	if (pIEntityAudioProxy)
//	{
//		if(synchSound)
//			pIEntityAudioProxy->StopSound(id, ESoundStopMode_OnSyncPoint);
//		else
//			pIEntityAudioProxy->StopSound(id);
//	}
//}

//------------------------------------------------------------------------
void CItem::Quiet()
{
	REINST("needs verification!");
	/*IEntityAudioProxy *pIEntityAudioProxy = GetAudioProxy(false);
	if (pIEntityAudioProxy)
	{
		pIEntityAudioProxy->StopAllSounds();
	}*/
}

//------------------------------------------------------------------------
//ISound *CItem::GetISound(tSoundID id)
//{
//	IEntityAudioProxy *pIEntityAudioProxy = GetAudioProxy(false);
//	if (pIEntityAudioProxy)
//		return pIEntityAudioProxy->GetSound(id);
//
//	return 0;
//}

//------------------------------------------------------------------------
/*
IEntityAudioProxy *CItem::GetAudioProxy(bool create)
{
	IEntityAudioProxy *pIEntityAudioProxy = (IEntityAudioProxy *)GetEntity()->GetProxy(ENTITY_PROXY_AUDIO);

	if (!pIEntityAudioProxy && create)
		pIEntityAudioProxy = crycomponent_cast<IEntityAudioProxyPtr> (GetEntity()->CreateProxy(ENTITY_PROXY_AUDIO)).get();

	return pIEntityAudioProxy;
}
*/

//------------------------------------------------------------------------
IEntityRenderProxy *CItem::GetRenderProxy(bool create)
{
	IEntityRenderProxy *pRenderProxy = (IEntityRenderProxy *)GetEntity()->GetProxy(ENTITY_PROXY_RENDER);
	if (!pRenderProxy && create)
		pRenderProxy = crycomponent_cast<IEntityRenderProxyPtr> (GetEntity()->CreateProxy(ENTITY_PROXY_RENDER)).get();

	return pRenderProxy;
}

//------------------------------------------------------------------------
IEntityPhysicalProxy *CItem::GetPhysicalProxy(bool create)
{
	IEntityPhysicalProxy *pPhysicalProxy = (IEntityPhysicalProxy *)GetEntity()->GetProxy(ENTITY_PROXY_PHYSICS);
	if (!pPhysicalProxy && create)
		pPhysicalProxy = crycomponent_cast<IEntityPhysicalProxyPtr> (GetEntity()->CreateProxy(ENTITY_PROXY_PHYSICS)).get();

	return pPhysicalProxy;
}

//------------------------------------------------------------------------
void CItem::DestroyedGeometry(bool use)
{
  if (!m_geometry[eIGS_Destroyed].empty())
  {    
    DrawSlot(eIGS_Destroyed, use);
		if (m_stats.viewmode&eIVM_FirstPerson)
			DrawSlot(eIGS_FirstPerson, !use);
		else
			DrawSlot(eIGS_ThirdPerson, !use);

    if (use)
      GetEntity()->SetSlotLocalTM(eIGS_Destroyed, GetEntity()->GetSlotLocalTM(eIGS_ThirdPerson, false));
  }  
}
