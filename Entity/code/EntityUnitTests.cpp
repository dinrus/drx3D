// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Entity/stdafx.h>
#include <drx3D/Entity/Entity.h>
#include <drx3D/Entity/EntitySystem.h>

#include <drx3D/Sys/DrxUnitTest.h>
#include <drx3D/Schema/IEnvRegistrar.h>
#include <drx3D/Schema/EnvComponent.h>

#include <drx3D/Entity/SubstitutionProxy.h>

#include <drx3D/Schema/IEnvRegistrar.h>
#include <drx3D/Schema/EnvComponent.h>

DRX_UNIT_TEST_SUITE(EntityTestsSuit)
{
	struct IUnifiedEntityComponent : public IEntityComponent
	{
		static void ReflectType(sxema::CTypeDesc<IUnifiedEntityComponent>& desc)
		{
			desc.SetGUID("{C89DD0DD-9850-43BA-BF52-86EFB7C9D0A5}"_drx_guid);
		}
	};

	class CUnifiedEntityComponent : public IUnifiedEntityComponent
	{
	public:
		static void ReflectType(sxema::CTypeDesc<CUnifiedEntityComponent>& desc)
		{
			desc.AddBase<IUnifiedEntityComponent>();
			desc.SetGUID("{C89DD0DD-9850-43BA-BF52-86EFB7C9D0A5}"_drx_guid);
			desc.SetEditorCategory("Unit Tests");
			desc.SetLabel("Unified Entity Component");
			desc.SetDescription("Does stuff");
			desc.SetIcon("icons:ObjectTypes/light.ico");
			desc.SetComponentFlags({ IEntityComponent::EFlags::Transform, IEntityComponent::EFlags::Socket, IEntityComponent::EFlags::Attach });

			desc.AddMember(&CUnifiedEntityComponent::m_bMyBool, 'bool', "Bool", "Bool", "Bool desc", false);
			desc.AddMember(&CUnifiedEntityComponent::m_myFloat, 'floa', "Float", "Float", "Float desc", 0.f);
		}

		bool  m_bMyBool = false;
		float m_myFloat = 0.f;
	};

	struct SScopedSpawnEntity
	{
		SScopedSpawnEntity(tukk szName)
		{
			SEntitySpawnParams params;
			params.guid = DrxGUID::Create();
			params.sName = szName;
			params.nFlags = ENTITY_FLAG_CLIENT_ONLY;

			pEntity = static_cast<CEntity*>(g_pIEntitySystem->SpawnEntity(params));
			DRX_UNIT_TEST_ASSERT(pEntity != nullptr);
		}

		~SScopedSpawnEntity()
		{
			g_pIEntitySystem->RemoveEntity(pEntity->GetId());
		}

		CEntity* pEntity;
	};

	DRX_UNIT_TEST(SpawnTest)
	{
		SScopedSpawnEntity entity("TestEntity");
		EntityId id = entity.pEntity->GetId();

		DRX_UNIT_TEST_ASSERT(entity.pEntity->GetGuid() != DrxGUID::Null());

		DRX_UNIT_TEST_CHECK_EQUAL(id, g_pIEntitySystem->FindEntityByGuid(entity.pEntity->GetGuid()));
		DRX_UNIT_TEST_CHECK_EQUAL(entity.pEntity, g_pIEntitySystem->FindEntityByName("TestEntity"));

		// Test Entity components
		IEntitySubstitutionComponent* pComponent = entity.pEntity->GetOrCreateComponent<IEntitySubstitutionComponent>();
		DRX_UNIT_TEST_ASSERT(nullptr != pComponent);
		DRX_UNIT_TEST_ASSERT(1 == entity.pEntity->GetComponentsCount());
	}

	class CLegacyEntityComponent : public IEntityComponent
	{
	public:
		DRX_ENTITY_COMPONENT_INTERFACE_AND_CLASS_GUID(CLegacyEntityComponent, "LegacyEntityComponent", "b1febcee-be12-46a9-b69a-cc66d6c0d3e0"_drx_guid);
	};

	DRXREGISTER_CLASS(CLegacyEntityComponent);

	DRX_UNIT_TEST(CreateLegacyComponent)
	{
		SScopedSpawnEntity entity("LegacyComponentTestEntity");

		CLegacyEntityComponent* pComponent = entity.pEntity->GetOrCreateComponent<CLegacyEntityComponent>();
		DRX_UNIT_TEST_CHECK_DIFFERENT(pComponent, nullptr);
		DRX_UNIT_TEST_CHECK_EQUAL(entity.pEntity->GetComponentsCount(), 1);

		entity.pEntity->RemoveComponent(pComponent);
		DRX_UNIT_TEST_CHECK_EQUAL(entity.pEntity->GetComponentsCount(), 0);
	}

	struct ILegacyComponentInterface : public IEntityComponent
	{
		DRX_ENTITY_COMPONENT_INTERFACE_GUID(ILegacyComponentInterface, "d9a71012-8af1-4426-8835-99c4f4eb987d"_drx_guid)

		virtual bool IsValid() const { return false; }
	};

	class CLegacyEntityComponentWithInterface final : public ILegacyComponentInterface
	{
	public:
		DRX_ENTITY_COMPONENT_CLASS_GUID(CLegacyEntityComponentWithInterface, ILegacyComponentInterface, "LegacyEntityComponentWithInterface", "34ee1b92-2154-4bad-b69a-cc66d6c0d3e0"_drx_guid);

		virtual bool IsValid() const final { return true; }
	};

	DRXREGISTER_CLASS(CLegacyEntityComponentWithInterface);

	DRX_UNIT_TEST(CreateLegacyComponentWithInterface)
	{
		SScopedSpawnEntity entity("LegacyComponentTestInterfaceEntity");

		ILegacyComponentInterface* pComponent = entity.pEntity->GetOrCreateComponent<ILegacyComponentInterface>();
		DRX_UNIT_TEST_CHECK_DIFFERENT(pComponent, nullptr);
		DRX_UNIT_TEST_CHECK_EQUAL(entity.pEntity->GetComponentsCount(), 1);
		DRX_UNIT_TEST_ASSERT(pComponent->IsValid());
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent, entity.pEntity->GetComponent<CLegacyEntityComponentWithInterface>());
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent, entity.pEntity->GetComponent<ILegacyComponentInterface>());

		entity.pEntity->RemoveComponent(pComponent);
		DRX_UNIT_TEST_CHECK_EQUAL(entity.pEntity->GetComponentsCount(), 0);

		pComponent = entity.pEntity->GetOrCreateComponent<CLegacyEntityComponentWithInterface>();
		DRX_UNIT_TEST_CHECK_DIFFERENT(pComponent, nullptr);
		DRX_UNIT_TEST_CHECK_EQUAL(entity.pEntity->GetComponentsCount(), 1);
		DRX_UNIT_TEST_ASSERT(pComponent->IsValid());
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent, entity.pEntity->GetComponent<CLegacyEntityComponentWithInterface>());
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent, entity.pEntity->GetComponent<ILegacyComponentInterface>());

		DynArray<CLegacyEntityComponentWithInterface*> components;
		entity.pEntity->GetAllComponents<CLegacyEntityComponentWithInterface>(components);
		DRX_UNIT_TEST_CHECK_EQUAL(components.size(), 1);
		DRX_UNIT_TEST_CHECK_EQUAL(components.at(0), static_cast<CLegacyEntityComponentWithInterface*>(pComponent));

		DynArray<ILegacyComponentInterface*> componentsbyInterface;
		entity.pEntity->GetAllComponents<ILegacyComponentInterface>(componentsbyInterface);
		DRX_UNIT_TEST_CHECK_EQUAL(componentsbyInterface.size(), 1);
		DRX_UNIT_TEST_CHECK_EQUAL(componentsbyInterface.at(0), static_cast<CLegacyEntityComponentWithInterface*>(pComponent));
	}

	DRX_UNIT_TEST(CreateUnifiedComponent)
	{
		SScopedSpawnEntity entity("UnifiedComponentTestEntity");

		CUnifiedEntityComponent* pComponent = entity.pEntity->GetOrCreateComponent<CUnifiedEntityComponent>();
		DRX_UNIT_TEST_CHECK_DIFFERENT(pComponent, nullptr);
		DRX_UNIT_TEST_CHECK_EQUAL(entity.pEntity->GetComponentsCount(), 1);
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent->m_bMyBool, false);
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent->m_myFloat, 0.f);
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent, entity.pEntity->GetComponent<CUnifiedEntityComponent>());
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent, entity.pEntity->GetComponent<IUnifiedEntityComponent>());

		entity.pEntity->RemoveComponent(pComponent);
		DRX_UNIT_TEST_CHECK_EQUAL(entity.pEntity->GetComponentsCount(), 0);

		pComponent = static_cast<CUnifiedEntityComponent*>(entity.pEntity->GetOrCreateComponent<IUnifiedEntityComponent>());
		DRX_UNIT_TEST_CHECK_DIFFERENT(pComponent, nullptr);
		DRX_UNIT_TEST_CHECK_EQUAL(entity.pEntity->GetComponentsCount(), 1);
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent->m_bMyBool, false);
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent->m_myFloat, 0.f);
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent, entity.pEntity->GetComponent<CUnifiedEntityComponent>());
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent, entity.pEntity->GetComponent<IUnifiedEntityComponent>());

		DynArray<CUnifiedEntityComponent*> components;
		entity.pEntity->GetAllComponents<CUnifiedEntityComponent>(components);
		DRX_UNIT_TEST_CHECK_EQUAL(components.size(), 1);
		DRX_UNIT_TEST_CHECK_EQUAL(components.at(0), static_cast<CUnifiedEntityComponent*>(pComponent));

		DynArray<IUnifiedEntityComponent*> componentsbyInterface;
		entity.pEntity->GetAllComponents<IUnifiedEntityComponent>(componentsbyInterface);
		DRX_UNIT_TEST_CHECK_EQUAL(componentsbyInterface.size(), 1);
		DRX_UNIT_TEST_CHECK_EQUAL(componentsbyInterface.at(0), static_cast<CUnifiedEntityComponent*>(pComponent));
	}

	DRX_UNIT_TEST(UnifiedComponentSerialization)
	{
		XmlNodeRef node = gEnv->pSystem->CreateXmlNode();
		DrxGUID instanceGUID;

		{
			SScopedSpawnEntity entity("UnifiedComponentSerializationTestEntity");

			CUnifiedEntityComponent* pComponent = entity.pEntity->GetOrCreateComponent<CUnifiedEntityComponent>();
			DRX_UNIT_TEST_CHECK_DIFFERENT(pComponent, nullptr);
			// Check default values
			DRX_UNIT_TEST_CHECK_EQUAL(pComponent->m_bMyBool, false);
			DRX_UNIT_TEST_CHECK_EQUAL(pComponent->m_myFloat, 0.f);

			pComponent->m_bMyBool = true;
			pComponent->m_myFloat = 1337.f;

			instanceGUID = pComponent->GetGUID();

			// Save to XML
			entity.pEntity->SerializeXML(node, false);
		}

		// Create another entity for deserialization
		SScopedSpawnEntity entity("UnifiedComponentDeserializationTestEntity");
		// Deserialize
		entity.pEntity->SerializeXML(node, true);

		CUnifiedEntityComponent* pComponent = entity.pEntity->GetComponent<CUnifiedEntityComponent>();
		DRX_UNIT_TEST_CHECK_DIFFERENT(pComponent, nullptr);
		// Check deserialized values
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent->m_bMyBool, true);
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent->m_myFloat, 1337.f);
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent->GetGUID(), instanceGUID);
	}

	DRX_UNIT_TEST(QueryInvalidGUID)
	{
		SScopedSpawnEntity entity("UnifiedComponentTestEntity");
		CUnifiedEntityComponent* pComponent = entity.pEntity->GetOrCreateComponent<CUnifiedEntityComponent>();
		CLegacyEntityComponentWithInterface* pLegacyComponent = entity.pEntity->GetOrCreateComponent<CLegacyEntityComponentWithInterface>();
		DRX_UNIT_TEST_CHECK_EQUAL(entity.pEntity->GetComponentsCount(), 2);
		// Querying the lowest level GUIDs is disallowed
		DRX_UNIT_TEST_CHECK_EQUAL(entity.pEntity->GetComponent<IEntityComponent>(), nullptr);
	}

	class CComponent2 : public IEntityComponent
	{
	public:
		static void ReflectType(sxema::CTypeDesc<CComponent2>& desc)
		{
			desc.SetGUID("{8B80B6EA-3A85-48F0-89DC-EDE69E72BC08}"_drx_guid);
			desc.SetLabel("CComponent2");
		}

		virtual void Initialize() override
		{
			m_bInitialized = true;
		}

		bool m_bInitialized = false;
	};

	class CComponent1 : public IEntityComponent
	{
	public:
		static void ReflectType(sxema::CTypeDesc<CComponent1>& desc)
		{
			desc.SetGUID("{00235E09-55EC-46AD-A6C7-606EA4A40A6B}"_drx_guid);
			desc.SetLabel("CComponent1");
			desc.AddMember(&CComponent1::m_bLoadingFromDisk, 'load', "Loading", "Loading", "", false);
		}

		virtual void Initialize() override
		{
			if (m_bLoadingFromDisk)
			{
				// Make sure that CComponent2 is already available (but uninitialized)
				CComponent2* pOtherComponent = m_pEntity->GetComponent<CComponent2>();
				DRX_UNIT_TEST_CHECK_DIFFERENT(pOtherComponent, nullptr);
				if (pOtherComponent != nullptr)
				{
					DRX_UNIT_TEST_CHECK_EQUAL(pOtherComponent->m_bInitialized, false);
				}
			}
		}

		bool m_bLoadingFromDisk = false;
	};

	// Test whether two components will be deserialized into CEntity::m_components before being Initialized
	// Initialize must never be called during loading from disk if another component (in the same entity) has yet to be loaded
	DRX_UNIT_TEST(LoadMultipleComponents)
	{
		XmlNodeRef xmlNode;

		{
			SScopedSpawnEntity entity("SaveMultipleComponents");
			CComponent1* pComponent1 = entity.pEntity->GetOrCreateComponent<CComponent1>();
			CComponent2* pComponent2 = entity.pEntity->GetOrCreateComponent<CComponent2>();

			// Set boolean to true so we can assert existence of CComponent2 in CComponent1::Initialize
			pComponent1->m_bLoadingFromDisk = true;

			// Save
			xmlNode = gEnv->pSystem->CreateXmlNode("Entity");
			entity.pEntity->SerializeXML(xmlNode, false);
		}

		// Load
		SScopedSpawnEntity entity("LoadMultipleComponents");
		entity.pEntity->SerializeXML(xmlNode, true);

		DRX_UNIT_TEST_CHECK_DIFFERENT(entity.pEntity->GetComponent<CComponent1>(), nullptr);
		DRX_UNIT_TEST_CHECK_DIFFERENT(entity.pEntity->GetComponent<CComponent2>(), nullptr);
	}

	DRX_UNIT_TEST(TestComponentEvent)
	{
		class CComponentWithEvent : public IEntityComponent
		{
		public:
			virtual void ProcessEvent(const SEntityEvent& event) override
			{
				DRX_UNIT_TEST_CHECK_EQUAL(event.event, ENTITY_EVENT_PHYSICS_CHANGE_STATE);
				m_wasHit = true;
			}

			virtual uint64 GetEventMask() const override { return ENTITY_EVENT_BIT(ENTITY_EVENT_PHYSICS_CHANGE_STATE); }

			bool m_wasHit = false;
		};

		class CComponentWithoutEvent : public IEntityComponent
		{
		public:
			virtual void ProcessEvent(const SEntityEvent& event) override
			{
				DRX_UNIT_TEST_ASSERT(false);
			}
		};

		SScopedSpawnEntity entity("TestComponentEvent");
		CComponentWithEvent* pComponent = entity.pEntity->CreateComponentClass<CComponentWithEvent>();
		entity.pEntity->CreateComponentClass<CComponentWithoutEvent>();
		pComponent->m_wasHit = false;
		pComponent->SendEvent(SEntityEvent(ENTITY_EVENT_PHYSICS_CHANGE_STATE));
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent->m_wasHit, true);

		pComponent->m_wasHit = false;
		entity.pEntity->SendEvent(SEntityEvent(ENTITY_EVENT_PHYSICS_CHANGE_STATE));
		DRX_UNIT_TEST_CHECK_EQUAL(pComponent->m_wasHit, true);

		// Ensure that we can send the events without any issues
		entity.pEntity->SendEvent(SEntityEvent(ENTITY_EVENT_PHYSICS_CHANGE_STATE));
	}

	DRX_UNIT_TEST(TestNestedComponentEvent)
	{
		class CComponentWithEvent : public IEntityComponent
		{
		public:
			virtual void ProcessEvent(const SEntityEvent& event) override
			{
				DRX_UNIT_TEST_ASSERT(event.event == ENTITY_EVENT_PHYSICS_CHANGE_STATE || event.event == ENTITY_EVENT_SCRIPT_EVENT);

				m_receivedEvents++;

				if (event.event == ENTITY_EVENT_PHYSICS_CHANGE_STATE)
				{
					DRX_UNIT_TEST_CHECK_EQUAL(m_receivedEvents, 1);
					m_pEntity->SendEvent(SEntityEvent(ENTITY_EVENT_SCRIPT_EVENT));
					DRX_UNIT_TEST_CHECK_EQUAL(m_receivedEvents, 2);
				}
				else if (event.event == ENTITY_EVENT_SCRIPT_EVENT)
				{
					DRX_UNIT_TEST_CHECK_EQUAL(m_receivedEvents, 2);
				}
			}

			virtual uint64 GetEventMask() const override { return ENTITY_EVENT_BIT(ENTITY_EVENT_PHYSICS_CHANGE_STATE) | ENTITY_EVENT_BIT(ENTITY_EVENT_SCRIPT_EVENT); }

			u8 m_receivedEvents = 0;
		};

		SScopedSpawnEntity entity("TestComponentEvent");
		CComponentWithEvent* pComponent = entity.pEntity->CreateComponentClass<CComponentWithEvent>();
		entity.pEntity->SendEvent(SEntityEvent(ENTITY_EVENT_PHYSICS_CHANGE_STATE));
		entity.pEntity->RemoveComponent(pComponent);

		// Ensure that we can send the events without any issues
		entity.pEntity->SendEvent(SEntityEvent(ENTITY_EVENT_PHYSICS_CHANGE_STATE));
		entity.pEntity->SendEvent(SEntityEvent(ENTITY_EVENT_SCRIPT_EVENT));
	}

	DRX_UNIT_TEST(TestSpawnedTransformation)
	{
		SEntitySpawnParams params;
		params.guid = DrxGUID::Create();
		params.sName = __FUNCTION__;
		params.nFlags = ENTITY_FLAG_CLIENT_ONLY;

		params.vPosition = Vec3(1, 2, 3);
		const Ang3 angles = Ang3(gf_PI, 0, gf_PI * 0.5f);
		params.qRotation = Quat::CreateRotationXYZ(angles);
		params.vScale = Vec3(1.5f, 1.f, 0.8f);

		CEntity* pEntity = static_cast<CEntity*>(g_pIEntitySystem->SpawnEntity(params));
		DRX_UNIT_TEST_ASSERT(pEntity != nullptr);
		DRX_UNIT_TEST_ASSERT(pEntity->GetParent() == nullptr);

		// Check world-space position
		DRX_UNIT_TEST_CHECK_EQUAL(pEntity->GetWorldPos(), params.vPosition);
		DRX_UNIT_TEST_CHECK_EQUAL(pEntity->GetWorldTM().GetTranslation(), params.vPosition);

		// Check world-space rotation
		DRX_UNIT_TEST_CHECK_CLOSE(pEntity->GetWorldRotation(), params.qRotation, VEC_EPSILON);
		DRX_UNIT_TEST_CHECK_CLOSE(Quat(pEntity->GetWorldTM()), params.qRotation, VEC_EPSILON);
		DRX_UNIT_TEST_CHECK_CLOSE(pEntity->GetRightDir(), params.qRotation.GetColumn0(), VEC_EPSILON);
		DRX_UNIT_TEST_CHECK_CLOSE(pEntity->GetForwardDir(), params.qRotation.GetColumn1(), VEC_EPSILON);
		DRX_UNIT_TEST_CHECK_CLOSE(pEntity->GetUpDir(), params.qRotation.GetColumn2(), VEC_EPSILON);
		DRX_UNIT_TEST_CHECK_CLOSE(pEntity->GetWorldAngles(), Ang3(params.qRotation), VEC_EPSILON);

		// Check world-space scale
		DRX_UNIT_TEST_CHECK_CLOSE(pEntity->GetWorldScale(), params.vScale, VEC_EPSILON);
		DRX_UNIT_TEST_CHECK_CLOSE(pEntity->GetWorldTM().GetScale(), params.vScale, VEC_EPSILON);
		DRX_UNIT_TEST_CHECK_CLOSE(pEntity->GetWorldTM(), Matrix34::Create(params.vScale, params.qRotation, params.vPosition), VEC_EPSILON);

		// Entities with parents currently return the world space position for the local Get functions
		// Ensure that this still works, while it remains intended
		DRX_UNIT_TEST_CHECK_EQUAL(pEntity->GetPos(), params.vPosition);
		DRX_UNIT_TEST_CHECK_EQUAL(pEntity->GetRotation(), params.qRotation);
		DRX_UNIT_TEST_CHECK_EQUAL(pEntity->GetScale(), params.vScale);
		DRX_UNIT_TEST_CHECK_EQUAL(pEntity->GetLocalTM(), Matrix34::Create(params.vScale, params.qRotation, params.vPosition));

		g_pIEntitySystem->RemoveEntity(pEntity->GetId());
	}

	DRX_UNIT_TEST(TestSpawnedChildTransformation)
	{
		SEntitySpawnParams parentParams;
		parentParams.guid = DrxGUID::Create();
		parentParams.sName = __FUNCTION__;
		parentParams.nFlags = ENTITY_FLAG_CLIENT_ONLY;

		parentParams.vPosition = Vec3(1, 2, 3);
		const Ang3 angles = Ang3(gf_PI, 0, gf_PI * 0.5f);
		parentParams.qRotation = Quat::CreateRotationXYZ(angles);
		parentParams.vScale = Vec3(1.5f, 1.f, 0.8f);

		CEntity* const pParentEntity = static_cast<CEntity*>(g_pIEntitySystem->SpawnEntity(parentParams));
		DRX_UNIT_TEST_ASSERT(pParentEntity != nullptr);
		DRX_UNIT_TEST_ASSERT(pParentEntity->GetParent() == nullptr);

		SEntitySpawnParams childParams = parentParams;
		childParams.id = INVALID_ENTITYID;
		childParams.guid = DrxGUID::Create();
		childParams.vPosition = Vec3(2, 2, 2);
		childParams.qRotation = Quat::CreateRotationZ(0.2f);
		childParams.vScale = Vec3(1.f, 1.f, 1.2f);
		childParams.pParent = pParentEntity;

		const CEntity* const pChildEntity = static_cast<CEntity*>(g_pIEntitySystem->SpawnEntity(childParams));
		DRX_UNIT_TEST_ASSERT(pChildEntity != nullptr);
		DRX_UNIT_TEST_ASSERT(pChildEntity->GetParent() == pParentEntity);

		// Ensure that parent world transform is correct
		DRX_UNIT_TEST_CHECK_CLOSE(pParentEntity->GetWorldTM(), Matrix34::Create(parentParams.vScale, parentParams.qRotation, parentParams.vPosition), VEC_EPSILON);

		// Check world-space position
		DRX_UNIT_TEST_CHECK_EQUAL(pChildEntity->GetWorldPos(), pParentEntity->GetWorldTM().TransformPoint(childParams.vPosition));
		DRX_UNIT_TEST_CHECK_EQUAL(pChildEntity->GetWorldTM().GetTranslation(), pParentEntity->GetWorldTM().TransformPoint(childParams.vPosition));

		// Check world-space rotation
		DRX_UNIT_TEST_CHECK_CLOSE(pChildEntity->GetWorldRotation(), pParentEntity->GetWorldRotation() * childParams.qRotation, VEC_EPSILON);
		DRX_UNIT_TEST_CHECK_CLOSE(Quat(pChildEntity->GetWorldTM()), pParentEntity->GetWorldRotation() * childParams.qRotation, VEC_EPSILON);

		Matrix34 childWorldTransform = pParentEntity->GetWorldTM() * Matrix34::Create(childParams.vScale, childParams.qRotation, childParams.vPosition);
		childWorldTransform.OrthonormalizeFast();

		DRX_UNIT_TEST_CHECK_CLOSE(pChildEntity->GetRightDir(), childWorldTransform.GetColumn0(), VEC_EPSILON);
		DRX_UNIT_TEST_CHECK_CLOSE(pChildEntity->GetForwardDir(), childWorldTransform.GetColumn1(), VEC_EPSILON);
		DRX_UNIT_TEST_CHECK_CLOSE(pChildEntity->GetUpDir(), childWorldTransform.GetColumn2(), VEC_EPSILON);
		DRX_UNIT_TEST_CHECK_CLOSE(pChildEntity->GetWorldAngles(), Ang3(pParentEntity->GetWorldRotation() * childParams.qRotation), VEC_EPSILON);

		// Check world-space scale
		DRX_UNIT_TEST_CHECK_CLOSE(pChildEntity->GetWorldScale(), pParentEntity->GetWorldScale().CompMul(childParams.vScale), VEC_EPSILON);
		DRX_UNIT_TEST_CHECK_CLOSE(pChildEntity->GetWorldTM().GetScale(), pParentEntity->GetWorldScale().CompMul(childParams.vScale), VEC_EPSILON);
		DRX_UNIT_TEST_CHECK_CLOSE(pChildEntity->GetWorldTM(), pParentEntity->GetWorldTM() * Matrix34::Create(childParams.vScale, childParams.qRotation, childParams.vPosition), VEC_EPSILON);

		// Check local-space
		DRX_UNIT_TEST_CHECK_EQUAL(pChildEntity->GetPos(), childParams.vPosition);
		DRX_UNIT_TEST_CHECK_EQUAL(pChildEntity->GetRotation(), childParams.qRotation);
		DRX_UNIT_TEST_CHECK_EQUAL(pChildEntity->GetScale(), childParams.vScale);
		DRX_UNIT_TEST_CHECK_EQUAL(pChildEntity->GetLocalTM(), Matrix34::Create(childParams.vScale, childParams.qRotation, childParams.vPosition));

		g_pIEntitySystem->RemoveEntity(pParentEntity->GetId());
		g_pIEntitySystem->RemoveEntity(pChildEntity->GetId());
	}

	DRX_UNIT_TEST(TestDefaultEntitySpawnClass)
	{
		SEntitySpawnParams spawnParams;
		spawnParams.guid = DrxGUID::Create();
		spawnParams.sName = __FUNCTION__;
		spawnParams.nFlags = ENTITY_FLAG_CLIENT_ONLY;

		CEntity* const pEntity = static_cast<CEntity*>(g_pIEntitySystem->SpawnEntity(spawnParams));
		DRX_UNIT_TEST_ASSERT(pEntity != nullptr);
		DRX_UNIT_TEST_ASSERT(pEntity->GetClass() != nullptr);
		DRX_UNIT_TEST_ASSERT(pEntity->GetClass() == g_pIEntitySystem->GetClassRegistry()->GetDefaultClass());
	}

	DRX_UNIT_TEST(TestComponentShutdownOrder)
	{
		bool firstComponentDestroyed = false;
		bool secondComponentDestroyed = false;

		class CFirstComponent : public IEntityComponent
		{
			bool& wasDestroyed;
			const bool& wasOtherDestroyed;

		public:
			CFirstComponent(bool& destroyed, bool& otherDestroyed) : wasDestroyed(destroyed), wasOtherDestroyed(otherDestroyed) {}
			virtual ~CFirstComponent()
			{
				DRX_UNIT_TEST_ASSERT(!wasDestroyed);
				DRX_UNIT_TEST_ASSERT(wasOtherDestroyed);
				wasDestroyed = true;
			}
		};

		class CSecondComponent : public IEntityComponent
		{
			bool& wasDestroyed;
			const bool& wasOtherDestroyed;

		public:
			CSecondComponent(bool& destroyed, bool& otherDestroyed) : wasDestroyed(destroyed), wasOtherDestroyed(otherDestroyed) {}
			virtual ~CSecondComponent() 
			{
				DRX_UNIT_TEST_ASSERT(!wasDestroyed);
				DRX_UNIT_TEST_ASSERT(!wasOtherDestroyed);
				wasDestroyed = true;
			}
		};

		SEntitySpawnParams spawnParams;
		spawnParams.guid = DrxGUID::Create();
		spawnParams.sName = __FUNCTION__;
		spawnParams.nFlags = ENTITY_FLAG_CLIENT_ONLY;

		CEntity* const pEntity = static_cast<CEntity*>(g_pIEntitySystem->SpawnEntity(spawnParams));
		DRX_UNIT_TEST_ASSERT(pEntity != nullptr);

		pEntity->CreateComponentClass<CFirstComponent>(firstComponentDestroyed, secondComponentDestroyed);
		DRX_UNIT_TEST_ASSERT(!firstComponentDestroyed && !secondComponentDestroyed);
		pEntity->CreateComponentClass<CSecondComponent>(secondComponentDestroyed, firstComponentDestroyed);
		DRX_UNIT_TEST_ASSERT(!firstComponentDestroyed && !secondComponentDestroyed);

		g_pIEntitySystem->RemoveEntity(pEntity->GetId(), true);
		DRX_UNIT_TEST_ASSERT(firstComponentDestroyed && secondComponentDestroyed);
	}
}

void RegisterUnitTestComponents(sxema::IEnvRegistrar& registrar)
{
	sxema::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		scope.Register(SXEMA_MAKE_ENV_COMPONENT(EntityTestsSuit::CUnifiedEntityComponent));
		scope.Register(SXEMA_MAKE_ENV_COMPONENT(EntityTestsSuit::CComponent1));
		scope.Register(SXEMA_MAKE_ENV_COMPONENT(EntityTestsSuit::CComponent2));
	}
}
