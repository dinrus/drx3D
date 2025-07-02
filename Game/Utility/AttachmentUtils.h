// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ATTACHMENTUTILS_H__
#define __ATTACHMENTUTILS_H__

struct IAttachment;
struct SEntitySlotInfo;
struct IEntity;

namespace AttachmentUtils
{
	i32k kDefaultPlayerMainSlot = 0;
	i32k kDefaultPlayerShadowSlot = 5;

	enum EAttachmentFlags
	{
		eAF_SyncCloak					=BIT(0),
		eAF_SyncRenderNearest	=BIT(1),
		eAF_AllowShadowCharAtt=BIT(2),

		eAF_Default = eAF_SyncCloak|eAF_SyncRenderNearest|eAF_AllowShadowCharAtt,
	};
	typedef u8 TAttachmentFlags;

	i32 FindFirstRenderedSlot( const IEntity* pEntity );
	bool IsEntityRenderedInNearestMode( const IEntity* pEntity );
	void SetEntityRenderInNearestMode( IEntity* pEntity, const bool drawNear );
	void SyncRenderInNearestModeWithEntity( IEntity* pMasterEntity, IEntity* pSlaveEntity );
	void SyncCloakWithEntity( const EntityId masterEntityId, const IEntity* pSlaveEntity, const bool forceDecloak = false );
	void SetupShadowAttachmentBinding( IEntity* pPlayerEntity, IEntity* pObjectEntity, u32k attNameCRC32Lower, i32k shadowSlotIndex = kDefaultPlayerShadowSlot, const bool bAllowCharacters = true );
	void SetupEntityAttachmentBinding( IAttachment* pPlayerAttachment, const EntityId objectEntityId );
	IAttachment* GetAttachment( IEntity* pEntity, tukk pAttachmentName, i32k slotIndex );
	IAttachment* GetAttachment( IEntity* pEntity, u32k attNameCRC32Lower, i32k slotIndex );
	
	void AttachObject( const bool bFirstPerson, IEntity* pPlayerEntity, IEntity* pObjectEntity, tukk pAttachmentName, const TAttachmentFlags flags = eAF_Default, i32k playerMainSlot = kDefaultPlayerMainSlot, i32k playerShadowSlot = kDefaultPlayerShadowSlot );
	void AttachObject( const bool bFirstPerson, IEntity* pPlayerEntity, IEntity* pObjectEntity, u32k attNameCRC32Lower, const TAttachmentFlags flags = eAF_Default, i32k playerMainSlot = kDefaultPlayerMainSlot, i32k playerShadowSlot = kDefaultPlayerShadowSlot );
	void DetachObject( IEntity* pPlayerEntity, IEntity* pObjectEntity, tukk pAttachmentName, const TAttachmentFlags flags = eAF_Default, i32k playerMainSlot = kDefaultPlayerMainSlot, i32k playerShadowSlot = kDefaultPlayerShadowSlot );
	void DetachObject( IEntity* pPlayerEntity, IEntity* pObjectEntity, u32k attNameCRC32Lower, const TAttachmentFlags flags = eAF_Default, i32k playerMainSlot = kDefaultPlayerMainSlot, i32k playerShadowSlot = kDefaultPlayerShadowSlot );
}

#endif // __ATTACHMENTUTILS_H__
