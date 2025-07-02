// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/PhysX/StdAfx.h>

// Included only once per DLL module.
#include <drx3D/CoreX/Platform/platform_impl.inl>

#include <drx3D/PhysX/world.h>

#ifndef STANDALONE_PHYSICS
#include <drx3D/Sys/IEngineModule.h>
#include <drx3D/CoreX/Extension/IDrxFactory.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#endif
/* */

//////////////////////////////////////////////////////////////////////////
struct CSystemEventListner_Physics : public ISystemEventListener
{
public:
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
	{
		switch (event)
		{
		//case ESYSTEM_EVENT_RANDOM_SEED:
		//	drx_random_seed(gEnv->bNoRandomSeed ? 0 : wparam);
		//	break;
		case ESYSTEM_EVENT_LEVEL_LOAD_END:
			break;
		case ESYSTEM_EVENT_3D_POST_RENDERING_END:
			break;
		case ESYSTEM_EVENT_FAST_SHUTDOWN:
		case ESYSTEM_EVENT_FULL_SHUTDOWN:
			cpx::g_drxPhysX.DisconnectPhysicsDebugger();
			break;
		}
	}
};

static CSystemEventListner_Physics g_system_event_listener_physics;

//////////////////////////////////////////////////////////////////////////

DRXPHYSICS_API IPhysicalWorld *CreatePhysicalWorld(ISystem *pSystem)
{
	if (pSystem)
	{
		pSystem->GetISystemEventDispatcher()->RegisterListener(&g_system_event_listener_physics, "CSystemEventListner_Physics");
		return new PhysXWorld(pSystem->GetILog());
	}

	return new PhysXWorld(0);
}

//////////////////////////////////////////////////////////////////////////

#ifndef STANDALONE_PHYSICS
//////////////////////////////////////////////////////////////////////////
class CEngineModule_DinrusXPhys : public IPhysicsEngineModule
{
	DRXINTERFACE_BEGIN()
		DRXINTERFACE_ADD(Drx::IDefaultModule)
		DRXINTERFACE_ADD(IPhysicsEngineModule)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CEngineModule_DinrusXPhys, "EngineModule_DinrusXPhys", "526cabf3-d776-407f-aa23-38545bb6ae7f"_drx_guid)

		//////////////////////////////////////////////////////////////////////////
		virtual tukk GetName() const override { return "DinrusXPhys"; };
	virtual tukk GetCategory() const override { return "drx3D"; };

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize(SSysGlobEnv &env, const SSysInitParams &initParams) override
	{
		ISystem* pSystem = env.pSystem;

		if (pSystem)
			pSystem->GetISystemEventDispatcher()->RegisterListener(&g_system_event_listener_physics, "CSystemEventListner_Physics");

		env.pPhysicalWorld = new PhysXWorld(pSystem ? pSystem->GetILog() : 0);

		return true;
	}
};

DRXREGISTER_SINGLETON_CLASS(CEngineModule_DinrusXPhys)

#endif
