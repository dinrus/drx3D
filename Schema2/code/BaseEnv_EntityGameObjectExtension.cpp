// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfxBaseEnv.h>
#include <drx3D/Schema2/BaseEnv_EntityGameObjectExtension.h>
#include <drx3D/Schema2/BaseEnv_EntityFoundationProperties.h>

#include <drx3D/Schema2/BaseEnv_BaseEnv.h>
#include <drx3D/Schema2/IEntityAttributesProxy.h>
#include <drx3D/Schema2/BaseEnv_EntityMap.h>
#include <drx3D/Schema2/BaseEnv_EntityUserData.h>

namespace SchematycBaseEnv
{
	tukk CEntityGameObjectExtension::s_szExtensionName = "SchematycGameEntityGameObjectExtension";

	CEntityGameObjectExtension::CEntityGameObjectExtension()
		: m_simulationMode(sxema2::ESimulationMode::NotSet)
		, m_objectState(EObjectState::Uninitialized)
		, m_pObject(nullptr)
	{}

	CEntityGameObjectExtension::~CEntityGameObjectExtension()
	{
		DestroyObject();
	}

	bool CEntityGameObjectExtension::RegisterNetworkSerializer(const sxema2::NetworkSerializeCallbackConnection& callbackConnection, i32 aspects)
	{
		return false;
	}

	void CEntityGameObjectExtension::MarkAspectsDirty(i32 aspects) {}

	void CEntityGameObjectExtension::SetAspectDelegation(i32 aspects, const EAspectDelegation delegation) {}

	bool CEntityGameObjectExtension::ClientAuthority() const
	{
		return false;
	}

	u16 CEntityGameObjectExtension::GetChannelId() const
	{
		return 0;
	}

	bool CEntityGameObjectExtension::ServerAuthority() const
	{
		return true;
	}

	bool CEntityGameObjectExtension::LocalAuthority() const
	{
		return gEnv->bServer;
	}

	bool CEntityGameObjectExtension::Init(IGameObject* pGameObject)
	{
		SetGameObject(pGameObject);
		
		IEntityAttributesComponent* pAttribComponent = GetEntity()->GetOrCreateComponent<IEntityAttributesComponent>();

		// NOTE pavloi 2016.11.25: CEntityClassRegistrar::RegisterEntityClass() sets a pointer to SEntityClass in the userProxyData
		const CEntityClassRegistrar::SEntityClass* pClassUserData = static_cast<CEntityClassRegistrar::SEntityClass*>(GetEntity()->GetClass()->GetUserProxyData());
		DRX_ASSERT(pClassUserData);
		if (pClassUserData)
		{
			m_classGUID = pClassUserData->libClassGUID;
			return true;
		}
		return false;
	}

	void CEntityGameObjectExtension::InitClient(i32 channelId) {}

	void CEntityGameObjectExtension::PostInit(IGameObject* pGameObject)
	{
		CreateObject(!gEnv->IsEditor() || gEnv->IsEditorGameMode());
	}

	void CEntityGameObjectExtension::PostInitClient(i32 channelId) {}

	bool CEntityGameObjectExtension::ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& spawnParams)
	{
		return false;
	}

	void CEntityGameObjectExtension::PostReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& spawnParams) {}

	void CEntityGameObjectExtension::FullSerialize(TSerialize serialize) {}

	bool CEntityGameObjectExtension::NetSerialize(TSerialize serialize, EEntityAspects aspects, u8 profile, i32 flags)
	{
		return true;
	}

	void CEntityGameObjectExtension::PostSerialize() {}

	void CEntityGameObjectExtension::SerializeSpawnInfo(TSerialize serialize) {}

	ISerializableInfoPtr CEntityGameObjectExtension::GetSpawnInfo()
	{
		return ISerializableInfoPtr();
	}

	void CEntityGameObjectExtension::Update(SEntityUpdateContext& context, i32 updateSlot) {}

	void CEntityGameObjectExtension::PostUpdate(float frameTime) {}

	void CEntityGameObjectExtension::PostRemoteSpawn() {}

	uint64 CEntityGameObjectExtension::GetEventMask() const
	{
		return ENTITY_EVENT_BIT(ENTITY_EVENT_RESET) | ENTITY_EVENT_BIT(ENTITY_EVENT_START_LEVEL) | ENTITY_EVENT_BIT(ENTITY_EVENT_DONE);
	}

	void CEntityGameObjectExtension::ProcessEvent(const SEntityEvent& event)
	{
		switch(event.event)
		{
		case ENTITY_EVENT_RESET:
			{
				// Recreate object when class is modified in editor.
				if(gEnv->IsEditor() && ((GetEntity()->GetFlags() & ENTITY_FLAG_SPAWNED) == 0))
				{
					CreateObject(event.nParam[0] == 1);
				}
				break;
			}
		case ENTITY_EVENT_START_LEVEL:
			{
				// Run object now if previously deferred.
				if(m_objectState == EObjectState::Initialized)
				{
					RunObject();
				}
				break;
			}
		case ENTITY_EVENT_DONE:
			{
				DestroyObject();
				break;
			}
		}
		if(!gEnv->IsEditing())
		{
			//g_eventSignal.Select(GetEntityId()).Send(event);
		}
	}

	void CEntityGameObjectExtension::HandleEvent(const SGameObjectEvent& event) {}

	void CEntityGameObjectExtension::SetChannelId(u16 channelId) {}

	void CEntityGameObjectExtension::GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->Add(*this);
	}

	void CEntityGameObjectExtension::CreateObject(bool bEnteringGame)
	{
		DestroyObject();

		sxema2::ILibClassConstPtr pLibClass = gEnv->pSchematyc2->GetLibRegistry().GetClass(m_classGUID);
		if(pLibClass)
		{
			IEntity*                       pEntity = GetEntity();
			const EntityId                 entityId = pEntity->GetId();
			sxema2::IPropertiesConstPtr pFoundationProperties = pLibClass->GetFoundationProperties();
			DRX_ASSERT(pFoundationProperties);
			if(pFoundationProperties)
			{
				const SEntityFoundationProperties* pFoundationPropertiesImpl = pFoundationProperties->ToPtr<SEntityFoundationProperties>();
				if(pFoundationPropertiesImpl->bTriggerAreas)
				{
					pEntity->AddFlags(ENTITY_FLAG_TRIGGER_AREAS);
				}
				else
				{
					pEntity->ClearFlags(ENTITY_FLAG_TRIGGER_AREAS);
				}
			}

			bool                    bIsPreview = false;
			sxema2::EObjectFlags objectFlags = sxema2::EObjectFlags::None;

			if(gEnv->IsEditor())
			{
				const SEntityUserData* pUserData = static_cast<const SEntityUserData*>(GetGameObject()->GetUserData());
				if(pUserData)
				{
					bIsPreview  = pUserData->bIsPreview;
					objectFlags = pUserData->objectFlags;
				}
			}

			m_simulationMode = bIsPreview ? sxema2::ESimulationMode::Preview : bEnteringGame ? sxema2::ESimulationMode::Game : sxema2::ESimulationMode::Editing;
			m_objectState    = EObjectState::Initialized;

			sxema2::SObjectParams objectParams;
			objectParams.pLibClass = pLibClass;

			IEntityAttributesComponent* pAttributesProxy = GetEntity()->GetComponent<IEntityAttributesComponent>();
			if(pAttributesProxy)
			{
				IEntityAttributePtr pPropertiesAttribute = EntityAttributeUtils::FindAttribute(pAttributesProxy->GetAttributes(), "SchematycGameEntityPropertiesAttribute");
				if(pPropertiesAttribute)
				{
					objectParams.pProperties = pPropertiesAttribute->GetProperties();
				}
			}

			objectParams.pNetworkObject = this;
			objectParams.entityId       = sxema2::ExplicitEntityId(entityId);
			objectParams.flags          = objectFlags;

			m_pObject = gEnv->pSchematyc2->GetObjectUpr().CreateObject(objectParams);

			SchematycBaseEnv::CBaseEnv::GetInstance().GetGameEntityMap().AddObject(entityId, m_pObject->GetObjectId());

			// Run object now or defer?
			if(gEnv->IsEditor()/* && !gEnv->pGame->IsLevelLoading()*/)
			{
				RunObject();
			}
		}
	}

	void CEntityGameObjectExtension::DestroyObject()
	{
		if(m_pObject)
		{
			SchematycBaseEnv::CBaseEnv::GetInstance().GetGameEntityMap().RemoveObject(GetEntityId());
			gEnv->pSchematyc2->GetObjectUpr().DestroyObject(m_pObject);
			m_simulationMode = sxema2::ESimulationMode::NotSet;
			m_objectState    = EObjectState::Uninitialized;
			m_pObject        = nullptr;
		}
	}

	void CEntityGameObjectExtension::RunObject()
	{
		DRX_ASSERT(m_objectState == EObjectState::Initialized);
		if(m_objectState == EObjectState::Initialized)
		{
			m_objectState = EObjectState::Running;
			m_pObject->Run(m_simulationMode);
		}
	}
}