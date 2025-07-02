// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Entity/stdafx.h>

#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Entity/EntitySystem.h>
#include <drx3D/Entity/EntityUnitTests.h>
#include <drx3D/Entity/EntityClassRegistry.h>

#include <drx3D/Entity/EntitySchematycActions.h>
#include <drx3D/Entity/EntitySchematycUtilFunctions.h>
#include <drx3D/Entity/EntityUtilsComponent.h>

// Included only once per DLL module.
#include <drx3D/CoreX/Platform/platform_impl.inl>

#include <drx3D/Sys/IEngineModule.h>
#include <drx3D/CoreX/Extension/IDrxFactory.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

#include  <drx3D/Schema/IEnvRegistry.h>
#include  <drx3D/Schema/EnvComponent.h>
#include  <drx3D/Schema/EnvPackage.h>
#include <drx3D/CoreX/StaticInstanceList.h>

CEntitySystem* g_pIEntitySystem = nullptr;
constexpr DrxGUID SchematyEntityComponentsPackageGUID = "A37D36D5-2AB1-4B48-9353-3DEC93A4236A"_drx_guid;

struct CSystemEventListener_Entity : public ISystemEventListener
{
public:
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
	{
		switch (event)
		{
		case ESYSTEM_EVENT_GAME_POST_INIT:
			{
				static_cast<CEntityClassRegistry*>(g_pIEntitySystem->GetClassRegistry())->OnGameFrameworkInitialized();
			}
			break;
		case ESYSTEM_EVENT_REGISTER_SXEMA_ENV:
			{

				auto entitySchematycRegistration = [](sxema::IEnvRegistrar& registrar)
				{
					sxema::CEntityTimerAction::Register(registrar);
					sxema::CEntityDebugTextAction::Register(registrar);
					sxema::Entity::RegisterUtilFunctions(registrar);
					sxema::CEntityUtilsComponent::Register(registrar);

					if (gEnv->bTesting)
					{
						RegisterUnitTestComponents(registrar);
					}
				};

				gEnv->pSchematyc->GetEnvRegistry().RegisterPackage(
				  stl::make_unique<sxema::CEnvPackage>(
				    SchematyEntityComponentsPackageGUID,
				    "EntityComponents",
				    "DinrusPro",
				    "DRXENGINE Default Entity Components",
				    entitySchematycRegistration
				    )
				  );
			}
			break;
		case ESYSTEM_EVENT_FULL_SHUTDOWN:
		case ESYSTEM_EVENT_FAST_SHUTDOWN:
			{
				// Deregister the sxema packages which were registered in the entity system.
				if (gEnv->pSchematyc)
				{
					gEnv->pSchematyc->GetEnvRegistry().DeregisterPackage(SchematyEntityComponentsPackageGUID);

					g_pIEntitySystem->GetClassRegistry()->UnregisterSchematycEntityClass();
				}
			}
			break;
		case ESYSTEM_EVENT_LEVEL_LOAD_END:
			{
				if (g_pIEntitySystem)
				{
					if (!gEnv->pSystem->IsSerializingFile())
					{
						// activate the default layers
						g_pIEntitySystem->EnableDefaultLayers();
					}

					g_pIEntitySystem->OnLevelLoaded();
				}
			}
			break;
		case ESYSTEM_EVENT_3D_POST_RENDERING_END:
			if (g_pIEntitySystem)
			{
				g_pIEntitySystem->Unload();
			}
			break;
		}
	}
};
static CSystemEventListener_Entity g_system_event_listener_entity;

//////////////////////////////////////////////////////////////////////////
class CEngineModule_EntitySystem : public IEntitySystemEngineModule
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(Drx::IDefaultModule)
	DRXINTERFACE_ADD(IEntitySystemEngineModule)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CEngineModule_EntitySystem, "EngineModule_DinrusXEntity", "88565507-2f01-4c03-820c-5a1a9b4d623b"_drx_guid)

	virtual ~CEngineModule_EntitySystem()
	{
		GetISystem()->GetISystemEventDispatcher()->RemoveListener(&g_system_event_listener_entity);
		SAFE_RELEASE(g_pIEntitySystem);
	}

	//////////////////////////////////////////////////////////////////////////
	virtual tukk GetName() const override     { return "DinrusXEntitySys"; };
	virtual tukk GetCategory() const override { return "drx3D"; };

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize(SSysGlobEnv& env, const SSysInitParams& initParams) override
	{
		ISystem* pSystem = env.pSystem;

		CEntitySystem* pEntitySystem = new CEntitySystem(pSystem);
		g_pIEntitySystem = pEntitySystem;
		if (!pEntitySystem->Init(pSystem))
		{
			pEntitySystem->Release();
			return false;
		}

		pSystem->GetISystemEventDispatcher()->RegisterListener(&g_system_event_listener_entity, "CSystemEventListener_Entity");

		env.pEntitySystem = pEntitySystem;
		return true;
	}
};

DRXREGISTER_SINGLETON_CLASS(CEngineModule_EntitySystem)

#include <drx3D/CoreX/CrtDebugStats.h>
