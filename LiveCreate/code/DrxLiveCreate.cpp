// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//////////////////////////////////////////////////////////////////////////

#include <drx3D/LiveCreate/StdAfx.h>

#include <drx3D/LiveCreate/DrxLiveCreate.h>
#include <drx3D/LiveCreate/LiveCreateUpr.h>
#include <drx3D/LiveCreate/LiveCreateCommands.h>
#include <drx3D/LiveCreate/LiveCreateHost.h>
#include <drx3D/CoreX/CrtDebugStats.h>

// Included only once per DLL module.
#include <drx3D/CoreX/Platform/platform_impl.inl>

#include <drx3D/Sys/IEngineModule.h>
#include <drx3D/CoreX/Extension/IDrxFactory.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

#ifndef NO_LIVECREATE

	#if DRX_PLATFORM_WINDOWS
		#ifndef _LIB
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
		#endif
	#endif

namespace LiveCreate
{
void RegisterCommandClasses()
{
	// Please put a line for all of the remote commands you want to call on the LiveCreate Hosts
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_SetCameraPosition);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_SetCameraFOV);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_EnableLiveCreate);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_DisableLiveCreate);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_EnableCameraSync);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_DisableCameraSync);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_SetEntityTransform);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_EntityUpdate);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_EntityDelete);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_SetCVar);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_SetTimeOfDayValue);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_SetTimeOfDay);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_SetTimeOfDayFull);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_SetEnvironmentFull);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_ObjectAreaUpdate);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_SetParticlesFull);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_SetArchetypesFull);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_FileSynced);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_EntityPropertyChange);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_UpdateLightParams);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_SyncSelection);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_EntitySetMaterial);
	REGISTER_LIVECREATE_COMMAND(CLiveCreateCmd_SyncLayerVisibility);
}
}

//////////////////////////////////////////////////////////////////////////
class CEngineModule_DrxLiveCreate : public ILiveCreateEngineModule
{
	DRXINTERFACE_BEGIN()
		DRXINTERFACE_ADD(Drx::IDefaultModule)
		DRXINTERFACE_ADD(ILiveCreateEngineModule)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CEngineModule_DrxLiveCreate, "EngineModule_DinrusXLiveCreate", "dc126bee-bdc6-411f-a111-b42839f2dd1b"_drx_guid)

	virtual ~CEngineModule_DrxLiveCreate() {}

	//////////////////////////////////////////////////////////////////////////
	virtual tukk GetName() const override { return "DinrusXLiveCreate"; };
	virtual tukk GetCategory() const override { return "drx3D"; };

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize(SSysGlobEnv& env, const SSysInitParams& initParams)
	{
		ISystem* pSystem = env.pSystem;

		pSystem->GetISystemEventDispatcher()->RegisterListener(&g_system_event_listener_livecreate, "CEngineModule_DrxLiveCreate");

		// new implementation
	#if DRX_PLATFORM_WINDOWS
		// Create the manager only on PC editor configuration
		if (env.IsEditor())
		{
			env.pLiveCreateUpr = new LiveCreate::CUpr();
		}
	#endif

		// create the host implementation only if not in editor
	#if DRX_PLATFORM_WINDOWS
		if (!env.IsEditor())
		{
			env.pLiveCreateHost = new LiveCreate::CHost();
		}
	#else
		env.pLiveCreateHost = new LiveCreate::CHost();
	#endif

		// always register LiveCreate commands (weather on host or on manager side)
		LiveCreate::RegisterCommandClasses();
		return true;
	}
};

DRXREGISTER_SINGLETON_CLASS(CEngineModule_DrxLiveCreate)

LiveCreate::IUpr* CreateLiveCreate(ISystem* pSystem)
{
	return NULL;
}

void DeleteLiveCreate(LiveCreate::IUpr* pLC)
{
}

#endif
