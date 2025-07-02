// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Movie/StdAfx.h>
#include <drx3D/Movie/DrxMovie.h>
#include <drx3D/Movie/Movie.h>
#include <drx3D/CoreX/CrtDebugStats.h>

// Included only once per DLL module.
#include <drx3D/CoreX/Platform/platform_impl.inl>
#include <drx3D/Sys/IEngineModule.h>
#include <drx3D/CoreX/Extension/IDrxFactory.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

#undef GetClassName

struct CSystemEventListener_Movie : public ISystemEventListener
{
public:
	virtual ~CSystemEventListener_Movie()
	{
		if (gEnv->pSystem)
		{
			gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);
		}
	}

	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
	{
		switch (event)
		{
		case ESYSTEM_EVENT_LEVEL_POST_UNLOAD:
			{
				CLightAnimWrapper::ReconstructCache();
				break;
			}
		}
	}
};

static CSystemEventListener_Movie g_system_event_listener_movie;

class CEngineModule_drx3DMovie : public IMovieEngineModule
{
	DRXINTERFACE_BEGIN()
		DRXINTERFACE_ADD(Drx::IDefaultModule)
		DRXINTERFACE_ADD(IMovieEngineModule)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CEngineModule_drx3DMovie, "EngineModule_drx3DMovie", "dce26bee-bdc6-400f-a0e9-b42839f2dd5b"_drx_guid)

	virtual ~CEngineModule_drx3DMovie()
	{
		gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(&g_system_event_listener_movie);
		SAFE_RELEASE(gEnv->pMovieSystem);
	}

	virtual tukk GetName() const override { return "drx3DMovie"; };
	virtual tukk GetCategory() const override { return "drx3D"; };

	virtual bool        Initialize(SSysGlobEnv& env, const SSysInitParams& initParams) override
	{
		ISystem* pSystem = env.pSystem;
		pSystem->GetISystemEventDispatcher()->RegisterListener(&g_system_event_listener_movie, "CEngineModule_drx3DMovie");

		env.pMovieSystem = new CMovieSystem(pSystem);
		return true;
	}
};

DRXREGISTER_SINGLETON_CLASS(CEngineModule_drx3DMovie)