// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/AICorpse.h>

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Act/IItemSystem.h>

#include <drx3D/Game/Game.h>
#include <drx3D/Game/WeaponSystem.h>
#include <drx3D/Game/Actor.h>

#include <drx3D/Game/GameCVars.h>

#define AI_CORPSES_MINIMUM 4

namespace
{
	bool AllowCloneAttachedItem( const IEntityClass* pItemClass )
	{
		static u32k kAllowedClassesCount = 5;
		static const IEntityClass* allowedClassTable[kAllowedClassesCount] = 
		{
			gEnv->pEntitySystem->GetClassRegistry()->FindClass("Heavy_minigun"),
			gEnv->pEntitySystem->GetClassRegistry()->FindClass("Heavy_mortar"),
			gEnv->pEntitySystem->GetClassRegistry()->FindClass("LightningGun"),
		};

		for(u32 i = 0; i < kAllowedClassesCount; ++i)
		{
			if(allowedClassTable[i] == pItemClass)
				return true;
		}

		return false;
	}

	bool ShouldDropOnCorpseRemoval( const IEntityClass* pItemClass )
	{
		static IEntityClass* pLightingGunClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("LightningGun");

		return (pLightingGunClass == pItemClass);
	}

	void RegisterEvents( IGameObjectExtension& goExt, IGameObject& gameObject )
	{
		i32k eventID = eCGE_ItemTakenFromCorpse;
		gameObject.UnRegisterExtForEvents( &goExt, NULL, 0 );
		gameObject.RegisterExtForEvents( &goExt, &eventID, 1 );
	}

	struct SCorpseRemovalScore
	{
		SCorpseRemovalScore( const EntityId _corpseId )
			: corpseId(_corpseId)
			, distance(10000000000.0f)
			, upClose(0)
			, farAway(0)
			, priority(0)
			, visible(0)
		{

		}

		EntityId corpseId;
		float    distance;
		u8    upClose;
		u8    farAway;
		u8	   priority;
		u8    visible;

		bool operator<(const SCorpseRemovalScore& otherCorpse) const
		{
			// 1- Far away go first
			if(farAway > otherCorpse.farAway)
				return true;
			
			if (farAway < otherCorpse.farAway)
				return false;

			// 2- In between up-close and far away next
			if(upClose < otherCorpse.upClose)
				return true;

			if(upClose > otherCorpse.upClose)
				return false;

			// 3- Within mid range non visible go first (unless priority high)
			if(visible < otherCorpse.visible)
				return true;

			if(visible > otherCorpse.visible)
				return false;

			// 4- If everything else fails short by priority and distance
			if(priority < otherCorpse.priority)
				return true;

			if(priority > otherCorpse.priority)
				return false;

			if (distance > otherCorpse.distance)
				return true;

			return false;
		}
	};
}

CAICorpse::CAICorpse()
{
}

CAICorpse::~CAICorpse()
{
	CAICorpseUpr* pAICorpseUpr = CAICorpseUpr::GetInstance();
	if(pAICorpseUpr != NULL)
	{
		pAICorpseUpr->UnregisterAICorpse( GetEntityId() );
	}

	for(u32 i = 0; i < m_attachedItemsInfo.size(); ++i)
	{
		if( m_attachedItemsInfo[i].id )
		{
			gEnv->pEntitySystem->RemoveEntity( m_attachedItemsInfo[i].id );
		}
	}
}

bool CAICorpse::Init( IGameObject * pGameObject )
{
	SetGameObject(pGameObject);

	CAICorpseUpr* pAICorpseUpr = CAICorpseUpr::GetInstance();
	assert(pAICorpseUpr != NULL);

	pAICorpseUpr->RegisterAICorpse( GetEntityId(), m_priority );

	return true;
}

void CAICorpse::PostInit( IGameObject * pGameObject )
{
	RegisterEvents( *this, *pGameObject );
}

bool CAICorpse::ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params )
{
	ResetGameObject();

	RegisterEvents( *this, *pGameObject );

	DRX_ASSERT_MESSAGE(false, "CAICorpse::ReloadExtension not implemented");

	return false;
}

bool CAICorpse::GetEntityPoolSignature( TSerialize signature )
{
	DRX_ASSERT_MESSAGE(false, "CAICorpse::GetEntityPoolSignature not implemented");

	return true;
}

void CAICorpse::Release()
{
	delete this;
}

void CAICorpse::FullSerialize( TSerialize ser )
{
#if AI_CORPSES_ENABLE_SERIALIZE
	ser.Value("modelName", m_modelName);
	ser.Value("priority", m_priority);

	u32 attachedWeaponsCount = m_attachedItemsInfo.size();
	ser.Value("attachedWeaponsCount", attachedWeaponsCount);

	stack_string weaponGroup;

	if(ser.IsReading())
	{
		// Restore the model/physics
		/*
		GetEntity()->LoadCharacter( 0, m_modelName.c_str() );
		
				SEntityPhysicalizeParams physicalizeParams;
				physicalizeParams.type = PE_ARTICULATED;
				physicalizeParams.nSlot = 0;
		
				GetEntity()->Physicalize(physicalizeParams);
		
				ICharacterInstance* pCharacterInstance = GetEntity()->GetCharacter(0);
				if (ser.BeginOptionalGroup("character", pCharacterInstance != NULL))
				{
					assert(pCharacterInstance != NULL);
					pCharacterInstance->Serialize(ser);
		
					ser.EndGroup();
				}
		
				IEntityPhysicalProxy* pPhysicsProxy = static_cast<IEntityPhysicalProxy*>(GetEntity()->GetProxy(ENTITY_PROXY_PHYSICS));
				if(ser.BeginOptionalGroup("characterPhysics", pPhysicsProxy != NULL))
				{
					assert(pPhysicsProxy != NULL);
					pPhysicsProxy->Serialize(ser);
		
					ser.EndGroup();
				}*/
		
		
		m_attachedItemsInfo.clear();

		for(u32 i = 0; i < attachedWeaponsCount; ++i)
		{
			weaponGroup.Format("Weapon_%d", i);
			ser.BeginGroup( weaponGroup.c_str() );
			{
				string className;
				string attachmentName;
				ser.Value( "weaponClass", className );
				ser.Value( "weaponAttachment", attachmentName );

				IEntityClass* pEntityClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass( className.c_str() );
				if(pEntityClass != NULL)
				{
					AttachedItem attachedWeaponInfo;
					attachedWeaponInfo.pClass = pEntityClass;
					attachedWeaponInfo.attachmentName = attachmentName.c_str();
					m_attachedItemsInfo.push_back(attachedWeaponInfo);
				}
			}
			ser.EndGroup();
		}
	}
	else
	{
	/*
		ICharacterInstance* pCharacterInstance = GetEntity()->GetCharacter(0);
			if (ser.BeginOptionalGroup("character", pCharacterInstance != NULL))
			{
				assert(pCharacterInstance != NULL);
				pCharacterInstance->Serialize(ser);
	
				ser.EndGroup();
			}
	
			IEntityPhysicalProxy* pPhysicsProxy = static_cast<IEntityPhysicalProxy*>(GetEntity()->GetProxy(ENTITY_PROXY_PHYSICS));
			if(ser.BeginOptionalGroup("characterPhysics", pPhysicsProxy != NULL))
			{
				assert(pPhysicsProxy != NULL);
				pPhysicsProxy->Serialize(ser);
	
				ser.EndGroup();
			}*/
	

		for(u32 i = 0; i < attachedWeaponsCount; ++i)
		{
			weaponGroup.Format("Weapon_%d", i);
			ser.BeginGroup( weaponGroup.c_str() );
			{
				string className = m_attachedItemsInfo[i].pClass->GetName();
				string attachmentName = m_attachedItemsInfo[i].attachmentName;
				ser.Value( "weaponClass", className );
				ser.Value( "weaponAttachment", attachmentName );
			}
			ser.EndGroup();
		}
	}
#endif //AI_CORPSES_ENABLE_SERIALIZE
}

void CAICorpse::PostSerialize()
{
#if AI_CORPSES_ENABLE_SERIALIZE
	ICharacterInstance* pCharacterInstance = GetEntity()->GetCharacter(0);
	if(pCharacterInstance != NULL)
	{
		IAttachmentUpr* pAttachmentUpr = pCharacterInstance->GetIAttachmentUpr();
		for(u32 i = 0; i < m_attachedItemsInfo.size(); ++i)
		{
			AttachedItem& weaponInfo = m_attachedItemsInfo[i];
			IAttachment* pAttachment = pAttachmentUpr->GetInterfaceByName( weaponInfo.attachmentName.c_str() );
			if(pAttachment != NULL)
			{
				weaponInfo.id = CloneAttachedItem( weaponInfo, pAttachment );
			}
		}
	}
#endif //AI_CORPSES_ENABLE_SERIALIZE
}

void CAICorpse::HandleEvent( const SGameObjectEvent& gameObjectEvent )
{
	if(gameObjectEvent.event == eCGE_ItemTakenFromCorpse)
	{
		assert(gameObjectEvent.param != NULL);

		bool matchFound = false;
		const EntityId itemId = *static_cast<const EntityId*>(gameObjectEvent.param);

		for (u32 i = 0; i < m_attachedItemsInfo.size(); ++i)
		{
			if(m_attachedItemsInfo[i].id == itemId)
			{
				ICharacterInstance* pCharacter = GetEntity()->GetCharacter(0);
				if(pCharacter != NULL)
				{
					IAttachment* pAttachment = pCharacter->GetIAttachmentUpr()->GetInterfaceByName( m_attachedItemsInfo[i].attachmentName.c_str() );
					if(pAttachment != NULL)
					{
						pAttachment->ClearBinding();
					}
				}

				m_attachedItemsInfo.removeAt(i);
				matchFound = true;
				break;
			}
		}

		if(matchFound)
		{
			if(m_attachedItemsInfo.empty())
			{
				m_priority = 0; //Lower the priority once all attached items have been removed
			}
		}
	}
}

void CAICorpse::GetMemoryUsage( IDrxSizer *pSizer ) const
{

}

void CAICorpse::SetupFromSource( IEntity& sourceEntity, ICharacterInstance& characterInstance, u32k priority)
{
	// 1.- Move resources from source entity, into AICorpse
	GetEntity()->SetFlags(GetEntity()->GetFlags() | (ENTITY_FLAG_CASTSHADOW));

	sourceEntity.MoveSlot(GetEntity(), 0);

	// Moving everything from one slot into another will also clear the render proxies in the source.
	// Thus, we need to invalidate the model so that it will be properly reloaded when a non-pooled
	// entity is restored from a save-game.
	CActor* sourceActor = static_cast<CActor*>(gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(sourceEntity.GetId()));
	if (sourceActor != NULL)
	{
		sourceActor->InvalidateCurrentModelName();
	}

	// 2.- After 'MoveSlot()', characterInstance is now stored inside CAICorpse
	// It needs to be now updated from the entity system
	characterInstance.SetFlags( characterInstance.GetFlags() | CS_FLAG_UPDATE );

#if AI_CORPSES_ENABLE_SERIALIZE
	m_modelName = characterInstance.GetFilePath();
#endif

	// 3.- Search for any attached weapon and clone them
	IItemSystem* pItemSystem = g_pGame->GetIGameFramework()->GetIItemSystem();

	IAttachmentUpr* pAttachmentUpr = characterInstance.GetIAttachmentUpr();
	u32k attachmentCount = (u32)pAttachmentUpr->GetAttachmentCount();
	for(u32 i = 0; i < attachmentCount ; ++i)
	{
		IAttachment* pAttachment = pAttachmentUpr->GetInterfaceByIndex(i);
		assert(pAttachment != NULL);

		IAttachmentObject* pAttachmentObject = pAttachment->GetIAttachmentObject();
		if((pAttachmentObject == NULL) || (pAttachmentObject->GetAttachmentType() != IAttachmentObject::eAttachment_Entity))
			continue;

		const EntityId attachedEntityId = static_cast<CEntityAttachment*>(pAttachmentObject)->GetEntityId();

		IItem* pItem = pItemSystem->GetItem(attachedEntityId);
		if(pItem != NULL)
		{
			if(AllowCloneAttachedItem( pItem->GetEntity()->GetClass() ))
			{
				if(m_attachedItemsInfo.size() < m_attachedItemsInfo.max_size())
				{
					AttachedItem attachedItemInfo;
					attachedItemInfo.pClass = pItem->GetEntity()->GetClass();
					attachedItemInfo.attachmentName = pAttachment->GetName();

					attachedItemInfo.id = CloneAttachedItem( attachedItemInfo, pAttachment );
					m_attachedItemsInfo.push_back(attachedItemInfo);
				}
			}
		}	
	}

	//Only accept requested priority if it has attached weapons
	m_priority = (m_attachedItemsInfo.size() > 0) ? priority : 0;

	//Force physics to sleep immediately (if not already)
	IPhysicalEntity* pCorpsePhysics = GetEntity()->GetPhysics();
	if(pCorpsePhysics != NULL)
	{
		pe_action_awake awakeAction;
		awakeAction.bAwake = 0;
		pCorpsePhysics->Action( &awakeAction );
	}
}

void CAICorpse::AboutToBeRemoved()
{
	IItemSystem* pItemSystem = g_pGame->GetIGameFramework()->GetIItemSystem();

	ICharacterInstance* pCharacter = GetEntity()->GetCharacter(0);
	IAttachmentUpr* pAttachmentUpr = (pCharacter != NULL) ? pCharacter->GetIAttachmentUpr() : NULL;

	for (u32 i = 0; i < (u32)m_attachedItemsInfo.size(); ++i)
	{
		AttachedItem& attachedItem = m_attachedItemsInfo[i];

		IItem* pItem = pItemSystem->GetItem( attachedItem.id );
		if((pItem != NULL) && ShouldDropOnCorpseRemoval( pItem->GetEntity()->GetClass() ))
		{
			IAttachment* pAttachment = (pAttachmentUpr != NULL) ? pAttachmentUpr->GetInterfaceByName( attachedItem.attachmentName.c_str() ) : NULL;
			if(pAttachment != NULL)
			{
				pAttachment->ClearBinding();
			}

			pItem->Drop( 1.0f, false, true );
			pItem->GetEntity()->SetFlags( pItem->GetEntity()->GetFlags() & ~ENTITY_FLAG_NO_SAVE );

			attachedItem.id = 0;
		}
	}
}

EntityId CAICorpse::CloneAttachedItem( const CAICorpse::AttachedItem& attachedItem, IAttachment* pAttachment  )
{
	stack_string clonedItemName;
	clonedItemName.Format("%s_%s", GetEntity()->GetName(), attachedItem.pClass->GetName() );
	SEntitySpawnParams params;
	params.sName = clonedItemName.c_str();
	params.pClass = attachedItem.pClass;

	// Flag as 'No Save' they will be recreated during serialization if needed
	params.nFlags |= (ENTITY_FLAG_NO_SAVE);				

	IEntity *pClonedItemEntity = gEnv->pEntitySystem->SpawnEntity(params);
	assert (pClonedItemEntity != NULL);

	IItem* pClonedItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pClonedItemEntity->GetId());
	assert(pClonedItem != NULL);

	pClonedItem->Physicalize(false, false);
	pClonedItem->SetOwnerId( GetEntityId() );

	// Set properties table to null, since they'll not be used
	IScriptTable* pClonedItemEntityScript = pClonedItemEntity->GetScriptTable();
	if (pClonedItemEntityScript != NULL)
	{
		pClonedItemEntity->GetScriptTable()->SetToNull("Properties");
	}

	// Swap attachments
	CEntityAttachment* pEntityAttachement = new CEntityAttachment();
	pEntityAttachement->SetEntityId( pClonedItemEntity->GetId() );

	pAttachment->AddBinding( pEntityAttachement );

	return pClonedItemEntity->GetId();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CAICorpseUpr* CAICorpseUpr::s_pThis = NULL;

CAICorpseUpr::CAICorpseUpr()
	: m_maxCorpses(AI_CORPSES_MINIMUM)
	, m_lastUpdatedCorpseIdx(0)
{
	assert(gEnv->bMultiplayer == false);
	assert(s_pThis == NULL);

	s_pThis = this;
}

CAICorpseUpr::~CAICorpseUpr()
{
	s_pThis = NULL;
}

void CAICorpseUpr::Reset()
{
	m_corpsesArray.clear();

	m_maxCorpses = max( g_pGameCVars->g_aiCorpses_MaxCorpses, AI_CORPSES_MINIMUM );

	m_corpsesArray.reserve( m_maxCorpses );

	m_lastUpdatedCorpseIdx = 0;
}

EntityId CAICorpseUpr::SpawnAICorpseFromEntity( IEntity& sourceEntity, const SCorpseParameters& corpseParams )
{
	assert(gEnv->bMultiplayer == false);

	if(g_pGameCVars->g_aiCorpses_Enable == 0)
		return 0;

	if(m_corpsesArray.size() == m_maxCorpses)
	{
		RemoveSomeCorpses();

		assert((u32)m_corpsesArray.size() < m_maxCorpses);
	}

	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, EMemStatContextFlags::MSF_None, "AICorpseUpr::SpawnCorpse");

	EntityId corpseId = 0;
	IPhysicalEntity* pSourcePhysics = sourceEntity.GetPhysics();
	if ((pSourcePhysics != NULL) && (pSourcePhysics->GetType() == PE_ARTICULATED))
	{
		ICharacterInstance *pSourceCharacterInstance = sourceEntity.GetCharacter(0);

		if (pSourceCharacterInstance != NULL)
		{
			IEntityClass *pCorpseClass =  gEnv->pEntitySystem->GetClassRegistry()->FindClass("AICorpse");
			assert(pCorpseClass);

			stack_string corpseName;
			corpseName.Format("%s_Corpse", sourceEntity.GetName());
			SEntitySpawnParams params;
			params.pClass = pCorpseClass;
			params.sName = corpseName.c_str();

#if !AI_CORPSES_ENABLE_SERIALIZE
			params.nFlags |= ENTITY_FLAG_NO_SAVE;
#endif

			params.vPosition = sourceEntity.GetWorldPos();
			params.qRotation = sourceEntity.GetWorldRotation();

			IEntity *pCorpseEntity = gEnv->pEntitySystem->SpawnEntity(params, true);
			if(pCorpseEntity != NULL)
			{
				corpseId = pCorpseEntity->GetId();
				
				CorpseInfo* pCorpseInfo = FindCorpseInfo( corpseId );
				assert(pCorpseInfo != NULL);

				CAICorpse* pCorpse = pCorpseInfo->GetCorpse();
				assert(pCorpse != NULL);
				pCorpse->SetupFromSource( sourceEntity, *pSourceCharacterInstance, (u32)corpseParams.priority);
			}
		}
	}

	return corpseId;
}

void CAICorpseUpr::RegisterAICorpse( const EntityId corpseId, u32k priority )
{
	assert(!HasCorpseInfo(corpseId));

	m_corpsesArray.push_back(CorpseInfo(corpseId, priority));
}

void CAICorpseUpr::UnregisterAICorpse( const EntityId corpseId )
{
	TCorpseArray::iterator corpsesEndIt = m_corpsesArray.end();
	for (TCorpseArray::iterator it = m_corpsesArray.begin(); it != corpsesEndIt; ++it)
	{
		if(it->corpseId != corpseId)
			continue;

		m_corpsesArray.erase(it);
		break;
	}
}

void CAICorpseUpr::RemoveSomeCorpses()
{
	u32k corspeCount = (u32)m_corpsesArray.size();
	assert(corspeCount > AI_CORPSES_MINIMUM);

	u32k maxCorpsesToRemove = MIN((corspeCount / AI_CORPSES_MINIMUM), 8);
	assert(maxCorpsesToRemove > 0);

	std::vector<SCorpseRemovalScore> corpseScoresInfo;
	corpseScoresInfo.reserve( corspeCount );

	const CCamera& viewCamera = gEnv->pSystem->GetViewCamera();
	const Vec3 cameraPosition = viewCamera.GetPosition();

	const float farAway = (g_pGameCVars->g_aiCorpses_ForceDeleteDistance * 0.85f);
	const float kUpCloseThreshold = (15.0f * 15.0f);   //Gives non-removal priority to corpses near the player
	const float kFarAwayThreshold = max((farAway * farAway), kUpCloseThreshold * 2.0f); //Gives removal priority to corpses far away from the player

	for(u32 i = 0; i < corspeCount; ++i)
	{
		CorpseInfo& corpseInfo = m_corpsesArray[i];
		SCorpseRemovalScore removalScore(corpseInfo.corpseId);
		
		CAICorpse* pCorpse = corpseInfo.GetCorpse();
		if(pCorpse != NULL)
		{
			AABB corpseBounds;
			pCorpse->GetEntity()->GetWorldBounds(corpseBounds);
			corpseBounds.Expand( Vec3(0.1f, 0.1f, 0.1f) );
			
			const float distanceSqr = (cameraPosition - corpseBounds.GetCenter()).len2();
			removalScore.distance = distanceSqr;
			removalScore.upClose = (distanceSqr < kUpCloseThreshold);
			removalScore.farAway = (distanceSqr > kFarAwayThreshold);
			removalScore.visible = viewCamera.IsAABBVisible_F(corpseBounds);
			removalScore.priority = pCorpse->GetPriority();
		}

		corpseScoresInfo.push_back(removalScore);
	}

	std::sort(corpseScoresInfo.begin(), corpseScoresInfo.end());

	assert(maxCorpsesToRemove < corpseScoresInfo.size());

	u32k corpseScoresCount = corpseScoresInfo.size();
	for(u32 i = 0; i < maxCorpsesToRemove; ++i)
	{
		RemoveCorpse(corpseScoresInfo[i].corpseId);
	}
}

void CAICorpseUpr::RemoveCorpse( const EntityId corpseId )
{
	TCorpseArray::iterator corpsesEndIt = m_corpsesArray.end();
	for (TCorpseArray::iterator it = m_corpsesArray.begin(); it != corpsesEndIt; ++it)
	{
		if(it->corpseId != corpseId)
			continue;

		CAICorpse* pAICorpse = it->GetCorpse();
		if(pAICorpse != NULL)
		{
			pAICorpse->AboutToBeRemoved();
		}

		gEnv->pEntitySystem->RemoveEntity( corpseId );
		m_corpsesArray.erase(it);
		break;
	}
}

CAICorpseUpr::CorpseInfo* CAICorpseUpr::FindCorpseInfo( const EntityId corpseId )
{
	const size_t corpseCount = m_corpsesArray.size();
	for (size_t i = 0; i < corpseCount; ++i)
	{
		if(m_corpsesArray[i].corpseId != corpseId)
			continue;

		return &(m_corpsesArray[i]);
	}

	return NULL;
}

bool CAICorpseUpr::HasCorpseInfo( const EntityId corpseId ) const
{
	const size_t corpseCount = m_corpsesArray.size();
	for (size_t i = 0; i < corpseCount; ++i)
	{
		if (m_corpsesArray[i].corpseId == corpseId)
			return true;
	}
	return false;
}

void CAICorpseUpr::Update( const float frameTime )
{
	u32k maxCorpsesToUpdateThisFrame = 4;

	const float cullPhysicsDistanceSqr = g_pGameCVars->g_aiCorpses_CullPhysicsDistance * g_pGameCVars->g_aiCorpses_CullPhysicsDistance;
	const float forceDeleteDistanceSqr = g_pGameCVars->g_aiCorpses_ForceDeleteDistance * g_pGameCVars->g_aiCorpses_ForceDeleteDistance;

	if (m_lastUpdatedCorpseIdx >= (u32)m_corpsesArray.size())
		m_lastUpdatedCorpseIdx = 0;

	const CCamera& viewCamera = gEnv->pSystem->GetViewCamera();

	DrxFixedArray<EntityId, maxCorpsesToUpdateThisFrame> corpsesToDelete;
	u32k corpsesEndIdx = min(m_lastUpdatedCorpseIdx + maxCorpsesToUpdateThisFrame, (u32)m_corpsesArray.size());

	for(u32 i = m_lastUpdatedCorpseIdx; i < corpsesEndIdx; ++i)
	{
		CorpseInfo& corpseInfo = m_corpsesArray[i];

		IEntity* pCorpseEntity = corpseInfo.GetCorpseEntity();
		if(pCorpseEntity != NULL)
		{
			AABB corpseBbox;
			pCorpseEntity->GetWorldBounds(corpseBbox);
			corpseBbox.Expand(Vec3(0.1f, 0.1f, 0.1f));

			const Vec3 corpsePosition = corpseBbox.GetCenter();
			const float distanceSqr = (corpsePosition - viewCamera.GetPosition()).len2();

			const bool attemptDeleteFarAway = (distanceSqr > forceDeleteDistanceSqr);
			const bool cullPhysics = (distanceSqr > cullPhysicsDistanceSqr);
			const bool isVisible = viewCamera.IsAABBVisible_F(corpseBbox);
			
			corpseInfo.flags.SetFlags( CorpseInfo::eFlag_FarAway, attemptDeleteFarAway );
			
			if(attemptDeleteFarAway && !isVisible)
			{
				corpsesToDelete.push_back(corpseInfo.corpseId);
			}
			else if(cullPhysics != corpseInfo.flags.AreAllFlagsActive( CorpseInfo::eFlag_PhysicsDisabled ))
			{
				IEntityPhysicalProxy* pCorpsePhysicsProxy = static_cast<IEntityPhysicalProxy*>(pCorpseEntity->GetProxy( ENTITY_PROXY_PHYSICS ));
				if (pCorpsePhysicsProxy != NULL)
				{
					//Simulate entity event to enable/disable physics
					SEntityEvent visibilityEvent;
					visibilityEvent.event = cullPhysics ? ENTITY_EVENT_HIDE : ENTITY_EVENT_UNHIDE;
					pCorpsePhysicsProxy->ProcessEvent( visibilityEvent );
					
					if(cullPhysics == false)
					{
						IPhysicalEntity* pCorpsePhysics = pCorpseEntity->GetPhysics();
						if(pCorpsePhysics != NULL)
						{
							pe_action_awake awakeAction;
							awakeAction.bAwake = 0;
							pCorpsePhysics->Action( &awakeAction );
						}
					}
				}

				corpseInfo.flags.SetFlags( CorpseInfo::eFlag_PhysicsDisabled, cullPhysics );
			}
		}
		else
		{
			//This should not happen, but in case remove the entity from the list
			GameWarning("AICorpseUpr - Detected corpse with no entity, removing from list");
			corpsesToDelete.push_back(corpseInfo.corpseId);
		}
	}

	m_lastUpdatedCorpseIdx = corpsesEndIdx;

	for(u32 i = 0; i < (u32)corpsesToDelete.size(); ++i)
	{
		RemoveCorpse(corpsesToDelete[i]);
	}

	DebugDraw();
}

void CAICorpseUpr::RemoveAllCorpses( tukk requester )
{
	while(m_corpsesArray.empty() == false)
	{
		RemoveCorpse( m_corpsesArray[0].corpseId );
	}

	m_lastUpdatedCorpseIdx = 0;

	DrxLog("AICorpseUpr: Removing all corpses as requested by '%s'", requester ? requester : "Unknown");
}

#if AI_CORPSES_DEBUG_ENABLED

void CAICorpseUpr::DebugDraw()
{
	if(g_pGameCVars->g_aiCorpses_DebugDraw == 0)
		return;

	IRenderAuxGeom* pRenderAux = gEnv->pRenderer->GetIRenderAuxGeom();

	gEnv->pRenderer->Draw2dLabel( 50.0f, 50.0f, 1.5f, Col_White, false, "Corpse count %" PRISIZE_T " - Max %d", m_corpsesArray.size(), m_maxCorpses );

	for(size_t i = 0; i < m_corpsesArray.size(); ++i)
	{
		CorpseInfo& corpse = m_corpsesArray[i];

		CAICorpse* pCorpse = corpse.GetCorpse();
		if(pCorpse != NULL)
		{
			AABB corpseBbox;
			pCorpse->GetEntity()->GetWorldBounds(corpseBbox);
			const Vec3 refPosition = corpseBbox.IsEmpty() ? pCorpse->GetEntity()->GetWorldPos() : corpseBbox.GetCenter();

			gEnv->pRenderer->DrawLabel( refPosition, 1.5f, "%s\nPriority %d\n%s\n%s", 
				pCorpse->GetEntity()->GetName(), pCorpse->GetPriority(),
				corpse.flags.AreAnyFlagsActive( CorpseInfo::eFlag_FarAway ) ? "Far away, remove when not visible" : "Not far away",
				corpse.flags.AreAllFlagsActive( CorpseInfo::eFlag_PhysicsDisabled) ? "Physics disabled" : "Physics enabled" );
			pRenderAux->DrawCone( refPosition + Vec3(0.0f, 0.0f, 1.5f), Vec3(0.0f, 0.0f, -1.0f), 0.3f, 0.8f, Col_Red );
		}
	}
}

#endif //AI_CORPSES_DEBUG_ENABLED

CAICorpseUpr::SCorpseParameters::Priority CAICorpseUpr::GetPriorityForClass( const IEntityClass* pEntityClass )
{
	static u32k kPriorityClassesCount = 3;
	static const IEntityClass* classesTable[kPriorityClassesCount] = 
	{
		gEnv->pEntitySystem->GetClassRegistry()->FindClass("Scorcher"),
	};

	static const SCorpseParameters::Priority prioritiesTable[kPriorityClassesCount] = 
	{
		SCorpseParameters::ePriority_VeryHight,
		SCorpseParameters::ePriority_VeryHight,
		SCorpseParameters::ePriority_High
	};

	for(u32 i = 0; i < kPriorityClassesCount; ++i)
	{
		if(classesTable[i] == pEntityClass)
			return prioritiesTable[i];
	}

	return SCorpseParameters::ePriority_Normal;
}
