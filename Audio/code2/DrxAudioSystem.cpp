// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Audio/StdAfx.h>
#include <drx3D/Audio/AudioCVars.h>
#include <drx3D/Audio/AudioSystem.h>
#include <drx3D/CoreX/Audio/IAudioSystem.h>
#include <drx3D/CoreX/Platform/platform_impl.inl>
#include <drx3D/Sys/IEngineModule.h>
#include <drx3D/CoreX/Memory/BucketAllocatorImpl.h>
#include <drx3D/CoreX/Extension/IDrxFactory.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	#include <drx3D/CoreX/Game/IGameFramework.h>
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

#if DRX_PLATFORM_DURANGO
	#include <apu.h>
#endif // DRX_PLATFORM_DURANGO

#define MAX_MODULE_NAME_LENGTH 256

namespace DrxAudio
{
// Define global objects.
CCVars g_cvars;

//////////////////////////////////////////////////////////////////////////
class CSystemEventListener_Sound : public ISystemEventListener
{
public:

	CSystemEventListener_Sound() = default;

	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override
	{
		if (gEnv->pAudioSystem != nullptr)
		{
			switch (event)
			{
			case ESYSTEM_EVENT_ACTIVATE:
				{
					// When Alt+Tabbing out of the application while it's in full-screen mode
					// ESYSTEM_EVENT_ACTIVATE is sent instead of ESYSTEM_EVENT_CHANGE_FOCUS.

					if (g_cvars.m_ignoreWindowFocus == 0)
					{
						// wparam != 0 is active, wparam == 0 is inactive
						// lparam != 0 is minimized, lparam == 0 is not minimized
						if (wparam == 0 || lparam != 0)
						{
							// lost focus
							gEnv->pAudioSystem->ExecuteTrigger(LoseFocusTriggerId);
						}
						else
						{
							// got focus
							gEnv->pAudioSystem->ExecuteTrigger(GetFocusTriggerId);
						}
					}
					break;
				}
			case ESYSTEM_EVENT_CHANGE_FOCUS:
				{
					if (g_cvars.m_ignoreWindowFocus == 0)
					{
						// wparam != 0 is focused, wparam == 0 is not focused
						if (wparam == 0)
						{
							// lost focus
							gEnv->pAudioSystem->ExecuteTrigger(LoseFocusTriggerId);
						}
						else
						{
							// got focus
							gEnv->pAudioSystem->ExecuteTrigger(GetFocusTriggerId);
						}
					}
					break;
				}
			}
		}
	}
};

static CSystemEventListener_Sound g_system_event_listener_sound;

///////////////////////////////////////////////////////////////////////////
bool CreateAudioSystem(SSysGlobEnv& env)
{
	bool bSuccess = false;
	CSystem* const pSystem = new CSystem;

	if (pSystem != nullptr)
	{
		if (env.pAudioSystem != nullptr)
		{
			env.pAudioSystem->Release();
			env.pAudioSystem = nullptr;
		}

		env.pAudioSystem = static_cast<IAudioSystem*>(pSystem);
		bSuccess = pSystem->Initialize();
	}

	return bSuccess;
}

///////////////////////////////////////////////////////////////////////////
void PrepareAudioSystem(CSystem* const pAudioSystem)
{
	DrxFixedStringT<MaxFilePathLength> const temp(pAudioSystem->GetConfigPath());

	// Must be blocking requests.
	SRequestUserData const data(ERequestFlags::ExecuteBlocking);
	pAudioSystem->ParseControlsData(temp.c_str(), EDataScope::Global, data);
	pAudioSystem->ParsePreloadsData(temp.c_str(), EDataScope::Global, data);
	pAudioSystem->PreloadSingleRequest(GlobalPreloadRequestId, false, data);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// dll interface

void AddPhysicalBlock(long size)
{
#ifndef _LIB
	_DrxMemoryUprPoolHelper::FakeAllocation(size);
#else
	GetISystem()->GetIMemoryUpr()->FakeAllocation(size);
#endif
}

//////////////////////////////////////////////////////////////////////////
class CEngineModule_DinrusXAudioSys : public ISystemModule
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(Drx::IDefaultModule)
	DRXINTERFACE_ADD(ISystemModule)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CEngineModule_DinrusXAudioSys, "EngineModule_DinrusXAudio", "ec73cf43-62ca-4a7f-8b45-1076dc6fdb8b"_drx_guid)

	CEngineModule_DinrusXAudioSys();
	virtual ~CEngineModule_DinrusXAudioSys() override;

	virtual tukk GetName() const override     { return "DinrusXAudioSys"; }
	virtual tukk GetCategory() const override { return "drx3D"; }

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize(SSysGlobEnv& env, const SSysInitParams& initParams) override
	{
		s_pInitParameters = &initParams;

		bool bSuccess = false;

		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "AudioSystem Memory Pool");

		if (CreateAudioSystem(env))
		{
#if DRX_PLATFORM_DURANGO
			// Do this before initializing the audio middleware!
			HRESULT const result = ApuCreateHeap(static_cast<UINT32>(g_cvars.m_fileCacheUprSize << 10));

			if (result != S_OK)
			{
				DrxFatalError("<Audio>: AudioSystem failed to allocate APU heap! (%d byte)", g_cvars.m_fileCacheUprSize << 10);
			}
#endif  // DRX_PLATFORM_DURANGO

			s_currentModuleName = m_pAudioImplNameCVar->GetString();

			// Get the first DrxAudio::IImplModule factory available in the module and create an instance of it
			auto pModule = env.pSystem->LoadModuleAndCreateFactoryInstance<IImplModule>(s_currentModuleName.c_str(), initParams);

			if (pModule != nullptr)
			{
				PrepareAudioSystem(static_cast<CSystem*>(env.pAudioSystem));
			}
			else
			{
				// In case of a failure set the null-implementation.
				SRequestUserData const data(ERequestFlags::ExecuteBlocking);
				static_cast<CSystem*>(env.pAudioSystem)->SetImpl(nullptr, data);
			}

			env.pSystem->GetISystemEventDispatcher()->RegisterListener(&g_system_event_listener_sound, "CSystemEventListener_Sound");

			// As soon as the audio system was created we consider this a success (even if the NULL implementation was used)
			bSuccess = true;
		}

		return bSuccess;
	}

	//////////////////////////////////////////////////////////////////////////
	static void OnAudioImplChanged(ICVar* pAudioImplNameCvar)
	{
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
		DrxFixedStringT<MAX_MODULE_NAME_LENGTH> const previousModuleName(s_currentModuleName);
		CSystem* const pAudioSystem = static_cast<CSystem*>(gEnv->pAudioSystem);
		SSysInitParams systemInitParams;
		s_currentModuleName = pAudioImplNameCvar->GetString();

		if (!previousModuleName.empty())
		{
			// Set the null impl
			SRequestUserData const data(ERequestFlags::ExecuteBlocking);
			pAudioSystem->SetImpl(nullptr, data);

			// Unload the previous module
			gEnv->pSystem->UnloadEngineModule(previousModuleName.c_str());
		}

		// Get the first DrxAudio::ISystemImplementationModule factory available in the module and create an instance of it
		auto pModule = gEnv->pSystem->LoadModuleAndCreateFactoryInstance<IImplModule>(s_currentModuleName.c_str(), *s_pInitParameters);

		// First try to load and initialize the new engine module.
		// This will release the currently running implementation but only if the library loaded successfully.
		if (pModule != nullptr)
		{
			SRequestUserData const data(ERequestFlags::ExecuteBlocking);

			// Then load global controls data and preloads.
			PrepareAudioSystem(pAudioSystem);

			// Then load level specific controls data and preloads.
			string const levelName = PathUtil::GetFileName(gEnv->pGameFramework->GetLevelName());

			if (!levelName.empty() && levelName.compareNoCase("Untitled") != 0)
			{
				string levelPath(gEnv->pAudioSystem->GetConfigPath());
				levelPath += s_szLevelsFolderName;
				levelPath += "/";
				levelPath += levelName;

				// Needs to be blocking so data is available for next preloading request!
				pAudioSystem->ParseControlsData(levelPath.c_str(), EDataScope::LevelSpecific, data);
				pAudioSystem->ParsePreloadsData(levelPath.c_str(), EDataScope::LevelSpecific, data);

				PreloadRequestId const preloadRequestId = DrxAudio::StringToId(levelName.c_str());
				pAudioSystem->PreloadSingleRequest(preloadRequestId, true, data);
			}

			// And finally re-trigger all active audio controls to restart previously playing sounds.
			pAudioSystem->RetriggerAudioControls(data);
		}
		else
		{
			// We could fail in two ways.
			// Either the module did not load in which case unloading of s_currentModuleName is redundant
			// or the module did not initialize in which case setting the null implementation is redundant.
			// As we currently do not know here how exactly the module failed we play safe and always set the null implementation and unload the modules.
			SRequestUserData const data(ERequestFlags::ExecuteBlocking);
			pAudioSystem->SetImpl(nullptr, data);

			// The module failed to initialize, unload both as we are running the null implementation now.
			gEnv->pSystem->UnloadEngineModule(s_currentModuleName.c_str());
			s_currentModuleName.clear();
		}

		// In any case send the event as we always loaded some implementation (either the proper or the NULL one).
		GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_AUDIO_IMPLEMENTATION_LOADED, 0, 0);
#endif  // INCLUDE_AUDIO_PRODUCTION_CODE
	}

private:

	ICVar*                                         m_pAudioImplNameCVar;
	static const SSysInitParams*                s_pInitParameters;
	static DrxFixedStringT<MAX_MODULE_NAME_LENGTH> s_currentModuleName;
};

const SSysInitParams* CEngineModule_DinrusXAudioSys::s_pInitParameters = nullptr;
DrxFixedStringT<MAX_MODULE_NAME_LENGTH> CEngineModule_DinrusXAudioSys::s_currentModuleName;
DRXREGISTER_SINGLETON_CLASS(CEngineModule_DinrusXAudioSys)

//////////////////////////////////////////////////////////////////////////
CEngineModule_DinrusXAudioSys::CEngineModule_DinrusXAudioSys()
{
	// Register audio cvars
	m_pAudioImplNameCVar = REGISTER_STRING_CB("s_AudioImplName", "DrxAudioImplSDLMixer", 0,
	                                          "Holds the name of the audio implementation library to be used.\n"
	                                          "Usage: s_AudioImplName <name of the library without extension>\n"
	                                          "Default: DrxAudioImplSDLMixer\n",
	                                          CEngineModule_DinrusXAudioSys::OnAudioImplChanged);
}

//////////////////////////////////////////////////////////////////////////
CEngineModule_DinrusXAudioSys::~CEngineModule_DinrusXAudioSys()
{
	SAFE_RELEASE(gEnv->pAudioSystem);

	if (gEnv->pSystem != nullptr)
	{
		gEnv->pSystem->UnloadEngineModule(s_currentModuleName.c_str());
	}
}
} //endns DrxAudio
#include <drx3D/CoreX/CrtDebugStats.h>
