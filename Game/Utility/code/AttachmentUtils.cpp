// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Utility/AttachmentUtils.h>
#include <drx3D/Entity/IEntity.h>
//#include <drx3D/Entity/IEntityProxy.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Animation/IAttachment.h>
#include <drx3D/Game/RecordingSystem.h>
#include <drx3D/Game/EntityUtility/EntityEffectsCloak.h>

namespace AttachmentUtils
{
	i32 FindFirstRenderedSlot( const IEntity* pEntity )
	{
		IF_UNLIKELY(pEntity == NULL)
			return -1;

		i32 slot = -1;
		i32k objectSlots = pEntity->GetSlotCount();
		for (i32 i = 0 ; i < objectSlots; ++i)
		{
			if (pEntity->GetSlotFlags(i)&ENTITY_SLOT_RENDER)
			{
				slot = i;
				break;
			}
		}

		return slot;
	}

	bool IsEntityRenderedInNearestMode( const IEntity* pEntity )
	{
		if(pEntity)
		{
			i32k slotIndex = 0;
			u32k slotFlags = pEntity->GetSlotFlags(slotIndex);

			return ((slotFlags & ENTITY_SLOT_RENDER_NEAREST) != 0);
		}
		return false;
	}

	void SetEntityRenderInNearestMode( IEntity* pEntity, const bool drawNear )
	{
		IF_UNLIKELY (pEntity == NULL)
			return;

		i32k nslots = pEntity->GetSlotCount();
		for (i32 i = 0; i < nslots; ++i)
		{
			u32k slotFlags = pEntity->GetSlotFlags(i);
			if(slotFlags & ENTITY_SLOT_RENDER)
			{
				u32k newSlotFlags = drawNear ? (slotFlags|ENTITY_SLOT_RENDER_NEAREST) : slotFlags&(~ENTITY_SLOT_RENDER_NEAREST);
				pEntity->SetSlotFlags(i, newSlotFlags);
			}
		}

		if(IEntityRenderProxy* pProxy = (IEntityRenderProxy*)pEntity->GetProxy(ENTITY_PROXY_RENDER))
		{
			if(IRenderNode* pRenderNode = pProxy->GetRenderNode())
			{
				pRenderNode->SetRndFlags(ERF_REGISTER_BY_POSITION, drawNear);
			}
		}
	}

	void SyncRenderInNearestModeWithEntity( IEntity* pMasterEntity, IEntity* pSlaveEntity )
	{
		SetEntityRenderInNearestMode( pSlaveEntity, IsEntityRenderedInNearestMode(pMasterEntity) );
	}

	void SyncCloakWithEntity( const EntityId masterEntityId, const IEntity* pSlaveEntity, const bool forceDecloak /*= false*/ )
	{
		const EntityEffects::Cloak::CloakSyncParams cloakParams( masterEntityId, pSlaveEntity->GetId(), true, forceDecloak );
		const bool bObjCloaked = EntityEffects::Cloak::CloakSyncEntities( cloakParams ); 

		if(CRecordingSystem *pRecordingSystem = g_pGame->GetRecordingSystem())
		{
			pRecordingSystem->OnObjectCloakSync( cloakParams.cloakSlaveId, cloakParams.cloakMasterId, bObjCloaked, true);
		}

		i32k children = pSlaveEntity->GetChildCount();
		for(i32 i = 0; i < children; ++i)
		{
			SyncCloakWithEntity( masterEntityId, pSlaveEntity->GetChild(i), forceDecloak );
		}
	}

	void SetupShadowAttachmentBinding( IEntity* pPlayerEntity, IEntity* pObjectEntity, u32k attNameCRC32Lower, i32k shadowSlotIndex /*= kDefaultPlayerShadowSlot*/, const bool bAllowCharacters /*= true*/ )
	{
		IAttachment* pShadowAttachment = GetAttachment(pPlayerEntity, attNameCRC32Lower, shadowSlotIndex);
		if(!pShadowAttachment)
			return;

		i32k firstRenderedSlot = FindFirstRenderedSlot(pObjectEntity);
		if(firstRenderedSlot<0)
			return;

		SEntitySlotInfo objectSlotInfo;
		if(pObjectEntity->GetSlotInfo(firstRenderedSlot, objectSlotInfo))
		{
			// Use a CSKELAttachment/CCGFAttachment for Shadow Attachments.
			if(bAllowCharacters && objectSlotInfo.pCharacter)
			{
				CSKELAttachment* pSkelAttachment = new CSKELAttachment();
				pSkelAttachment->m_pCharInstance = objectSlotInfo.pCharacter;
				pShadowAttachment->AddBinding( pSkelAttachment );
			}
			else if(objectSlotInfo.pStatObj)
			{
				CCGFAttachment* pStatObjAttachment = new CCGFAttachment();
				pStatObjAttachment->pObj = objectSlotInfo.pStatObj;
				pShadowAttachment->AddBinding( pStatObjAttachment );
			}

			pShadowAttachment->HideAttachment(1);
			pShadowAttachment->HideInShadow(0);
		}
	}

	void SetupEntityAttachmentBinding( IAttachment* pPlayerAttachment, const EntityId objectEntityId )
	{
		CEntityAttachment* pEntityAttachment = new CEntityAttachment();
		pEntityAttachment->SetEntityId( objectEntityId );
		pPlayerAttachment->AddBinding( pEntityAttachment );
	}

	IAttachment* GetAttachment( IEntity* pEntity, tukk pAttachmentName, i32k slotIndex )
	{
		u32k crc = CCrc32::ComputeLowercase(pAttachmentName);
		return GetAttachment( pEntity, crc, slotIndex );
	}

	IAttachment* GetAttachment( IEntity* pEntity, u32k attNameCRC32Lower, i32k slotIndex )
	{
		SEntitySlotInfo info;
		if( pEntity && pEntity->GetSlotInfo(slotIndex, info) && info.pCharacter )
		{
			return info.pCharacter->GetIAttachmentUpr()->GetInterfaceByNameCRC( attNameCRC32Lower );
		}
		return NULL;
	}

	void AttachObject( const bool bFirstPerson, IEntity* pPlayerEntity, IEntity* pObjectEntity, tukk pAttachmentName, const TAttachmentFlags flags /*= eAF_Default*/, i32k playerMainSlot /*= kDefaultPlayerMainSlot*/, i32k playerShadowSlot /*= kDefaultPlayerShadowSlot*/ )
	{
		u32k crc = CCrc32::ComputeLowercase(pAttachmentName);
		AttachObject( bFirstPerson, pPlayerEntity, pObjectEntity, crc, flags, playerMainSlot, playerShadowSlot );
	}

	void AttachObject( const bool bFirstPerson, IEntity* pPlayerEntity, IEntity* pObjectEntity, u32k attNameCRC32Lower, const TAttachmentFlags flags /*= eAF_Default*/, i32k playerMainSlot /*= kDefaultPlayerMainSlot*/, i32k playerShadowSlot /*= kDefaultPlayerShadowSlot*/ )
	{
		IAttachment* pPlayerMainAttachment = GetAttachment(pPlayerEntity, attNameCRC32Lower, playerMainSlot);
		if(!pPlayerMainAttachment)
			return;

		// Sync the RenderNearest flag.
		if(flags&eAF_SyncRenderNearest)
		{
			SyncRenderInNearestModeWithEntity( pPlayerEntity, pObjectEntity );
		}

		// Sync the cloaking.
		if(flags&eAF_SyncCloak)
		{
			SyncCloakWithEntity( pPlayerEntity->GetId(), pObjectEntity );
		}

		// Setup the EntityAttachment.
		SetupEntityAttachmentBinding( pPlayerMainAttachment, pObjectEntity->GetId() );

		if(bFirstPerson)
		{
			// Setup Shadow Attachment.
			SetupShadowAttachmentBinding( pPlayerEntity, pObjectEntity, attNameCRC32Lower, playerShadowSlot, (flags&eAF_AllowShadowCharAtt)!=0 );
		}

		// Show newly attached attachment.
		pPlayerMainAttachment->HideAttachment(0);
	}

	void DetachObject( IEntity* pPlayerEntity, IEntity* pObjectEntity, tukk pAttachmentName, const TAttachmentFlags flags /*= eAF_Default*/, i32k playerMainSlot /*= kDefaultPlayerMainSlot*/, i32k playerShadowSlot /*= kDefaultPlayerShadowSlot*/ )
	{
		u32k crc = CCrc32::ComputeLowercase(pAttachmentName);
		DetachObject( pPlayerEntity, pObjectEntity, crc, flags, playerMainSlot, playerShadowSlot );
	}

	void DetachObject( IEntity* pPlayerEntity, IEntity* pObjectEntity, u32k attNameCRC32Lower, const TAttachmentFlags flags /*= eAF_Default*/, i32k playerMainSlot /*= kDefaultPlayerMainSlot*/, i32k playerShadowSlot /*= kDefaultPlayerShadowSlot*/ )
	{
		if(!pPlayerEntity)
			return;

		// Set the RenderNearest flag to false.
		if(flags&eAF_SyncRenderNearest)
		{
			SetEntityRenderInNearestMode( pObjectEntity, false );
		}

		// Force de-cloak.
		if(pObjectEntity && (flags&eAF_SyncCloak))
		{
			SyncCloakWithEntity( pPlayerEntity->GetId(), pObjectEntity, true );
		}

		// Clear the Main attachment binding.
		if(IAttachment* pPlayerMainAttachment = GetAttachment(pPlayerEntity, attNameCRC32Lower, playerMainSlot))
		{
			pPlayerMainAttachment->ClearBinding();
			pPlayerMainAttachment->HideInShadow(0);
		}

		// Clear the Shadow attachment binding.
		if(IAttachment* pPlayerShadowAttachment = GetAttachment(pPlayerEntity, attNameCRC32Lower, playerShadowSlot))
		{
			pPlayerShadowAttachment->ClearBinding();
		}
	}


}
