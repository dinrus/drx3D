// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/DrxPluginUpr.h>

#include <drx3D/Sys/System.h>
#include <drx3D/Sys/ProjectUpr.h>

#include <drx3D/CoreX/Extension/IDrxFactory.h>
#include <drx3D/CoreX/Extension/IDrxFactoryRegistry.h>
#include <drx3D/CoreX/Extension/DrxCreateClassInstance.h>

#include <drx3D/CoreX/Mono/IMonoRuntime.h>

#include <drx3D/CoreX/Platform/DrxLibrary.h>

// Descriptor for the binary file of a plugin.
// This is separate since a plugin does not necessarily have to come from a binary, for example if static linking is used.
struct SNativePluginModule
{
	SNativePluginModule() : m_pFactory(nullptr) {}
	SNativePluginModule(tukk szPath, IDrxFactory* pFactory = nullptr)
		: m_engineModulePath(szPath)
		, m_pFactory(pFactory) {}
	SNativePluginModule(SNativePluginModule& other)
		: m_engineModulePath(other.m_engineModulePath)
		, m_pFactory(other.m_pFactory)
	{
		other.MarkUnloaded();
	}
	SNativePluginModule& operator=(const SNativePluginModule&) = default;
	SNativePluginModule(SNativePluginModule&& other)
		: m_engineModulePath(std::move(other.m_engineModulePath))
		, m_pFactory(other.m_pFactory)
	{
		other.MarkUnloaded();
	}
	SNativePluginModule& operator=(SNativePluginModule&&) = default;
	~SNativePluginModule()
	{
		Shutdown();
	}

	bool Shutdown()
	{
		bool bSuccess = false;
		if (m_engineModulePath.size() > 0)
		{
			bSuccess = static_cast<CSystem*>(gEnv->pSystem)->UnloadDynamicLibrary(m_engineModulePath);

			// Prevent Shutdown twice
			MarkUnloaded();
		}

		return bSuccess;
	}

	void MarkUnloaded()
	{
		m_engineModulePath.clear();
	}

	bool IsLoaded()
	{
		return m_engineModulePath.size() > 0;
	}

	string m_engineModulePath;
	IDrxFactory* m_pFactory;
};

struct SPluginContainer
{
	// Constructor for native plug-ins
	SPluginContainer(const std::shared_ptr<Drx::IEnginePlugin>& plugin, SNativePluginModule&& module)
		: m_pPlugin(plugin)
		, m_module(std::move(module))
		, m_pluginClassId(plugin->GetFactory()->GetClassID())
	{
	}

	// Constructor for managed (Mono) plug-ins, or statically linked ones
	SPluginContainer(const std::shared_ptr<Drx::IEnginePlugin>& plugin)
		: m_pPlugin(plugin) 
	{
		if (IDrxFactory* pFactory = plugin->GetFactory())
		{
			m_pluginClassId = pFactory->GetClassID();
		}
	}

	bool Initialize(SSysGlobEnv& env, const SSysInitParams& initParams)
	{
		if (m_pPlugin)
		{
			return m_pPlugin->Initialize(env, initParams);
		}

		return false;
	}

	bool Shutdown()
	{
		m_pPlugin->UnregisterFlowNodes();

		m_pPlugin.reset();

		return m_module.Shutdown();
	}

	Drx::IEnginePlugin* GetPluginPtr() const
	{
		return m_pPlugin ? m_pPlugin.get() : nullptr;
	}

	friend bool operator==(const SPluginContainer& left, const SPluginContainer& right)
	{
		return (left.GetPluginPtr() == right.GetPluginPtr());
	}

	DrxClassID                     m_pluginClassId;
	string                         m_pluginAssetDirectory;

	Drx::IPluginUpr::EType     m_pluginType;

	std::shared_ptr<Drx::IEnginePlugin>  m_pPlugin;

	SNativePluginModule            m_module;
};

CDrxPluginUpr::CDrxPluginUpr(const SSysInitParams& initParams)
	: m_systemInitParams(initParams)
	, m_bLoadedProjectPlugins(false)
{
	GetISystem()->GetISystemEventDispatcher()->RegisterListener(this, "CDrxPluginUpr");
}

CDrxPluginUpr::~CDrxPluginUpr()
{
	GetISystem()->GetISystemEventDispatcher()->RemoveListener(this);

	UnloadAllPlugins();

	DRX_ASSERT(m_pluginContainer.empty());

	if (gEnv->pConsole)
	{
		gEnv->pConsole->RemoveCommand("sys_reload_plugin");
		gEnv->pConsole->UnregisterVariable("sys_debug_plugin", true);
	}
}

void CDrxPluginUpr::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_REGISTER_FLOWNODES:
		{
			for (SPluginContainer& it : m_pluginContainer)
			{
				it.GetPluginPtr()->RegisterFlowNodes();
			}
		}
		break;
	}
}

void CDrxPluginUpr::LoadProjectPlugins()
{
	// Find out how many Drx::IEnginePlugin implementations are available
	size_t numFactories;
	gEnv->pSystem->GetDrxFactoryRegistry()->IterateFactories(drxiidof<Drx::IEnginePlugin>(), nullptr, numFactories);

	std::vector<IDrxFactory*> factories;
	factories.resize(numFactories);

	// Start by looking for any Drx::IEnginePlugin implementation in the current module, in case of static linking
	gEnv->pSystem->GetDrxFactoryRegistry()->IterateFactories(drxiidof<Drx::IEnginePlugin>(), factories.data(), numFactories);
	for (size_t i = 0; i < numFactories; ++i)
	{
		// Create an instance of the plug-in
		IDrxUnknownPtr pUnknown = factories[i]->CreateClassInstance();
		std::shared_ptr<Drx::IEnginePlugin> pPlugin = drxinterface_cast<Drx::IEnginePlugin>(pUnknown);

		m_pluginContainer.emplace_back(pPlugin);

		OnPluginLoaded();
	}

	m_bLoadedProjectPlugins = true;

#if DrxSharedLibrarySupported
	// Load plug-ins specified in the .drxproject file from disk
	CProjectUpr* pProjectUpr = static_cast<CProjectUpr*>(gEnv->pSystem->GetIProjectUpr());
	const std::vector<SPluginDefinition>& pluginDefinitions = pProjectUpr->GetPluginDefinitions();

	for (const SPluginDefinition& pluginDefinition : pluginDefinitions)
	{
		if (!pluginDefinition.platforms.empty() && !stl::find(pluginDefinition.platforms, EPlatform::Current))
		{
			continue;
		}

#if defined(DRX_IS_MONOLITHIC_BUILD)
		// Don't attempt to load plug-ins that were statically linked in
		string pluginName = PathUtil::GetFileName(pluginDefinition.path);
		bool bValid = true;

		for (const std::pair<u8, SPluginDefinition>& defaultPlugin : CDrxPluginUpr::GetDefaultPlugins())
		{
			if (!strcmp(defaultPlugin.second.path, pluginName.c_str()))
			{
				bValid = false;
				break;
			}
		}

		if (!bValid)
		{
			continue;
		}
#endif

		LoadPluginFromDisk(pluginDefinition.type, pluginDefinition.path);
	}

#if !defined(DRX_IS_MONOLITHIC_BUILD) && !defined(DRX_PLATFORM_CONSOLE)
	// Always load the DrxUserAnalytics plugin
	SPluginDefinition userAnalyticsPlugin{ EType::Native, "DrxUserAnalytics" };
	if (std::find(std::begin(pluginDefinitions), std::end(pluginDefinitions), userAnalyticsPlugin) == std::end(pluginDefinitions))
	{
		LoadPluginFromDisk(userAnalyticsPlugin.type, userAnalyticsPlugin.path, false);
	}
#endif
#endif // DrxSharedLibrarySupported
}

#if DrxSharedLibrarySupported
bool CDrxPluginUpr::LoadPluginFromDisk(EType type, tukk szPath, bool notifyUserOnFailure)
{
	DRX_ASSERT_MESSAGE(m_bLoadedProjectPlugins, "Plug-ins must not be loaded before LoadProjectPlugins!");

	DrxLogAlways("Loading plug-in %s", szPath);

	switch (type)
	{
	case EType::Native:
		{
			MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "LoadPlugin");
			MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Other, 0, "%s", szPath);

			WIN_HMODULE hModule = static_cast<CSystem*>(gEnv->pSystem)->LoadDynamicLibrary(szPath, false);
			if (hModule == nullptr)
			{
				string errorMessage = string().Format("Plugin load failed, could not find dynamic library %s!", szPath);
				DrxWarning(VALIDATOR_MODULE_SYSTEM, notifyUserOnFailure ? VALIDATOR_ERROR : VALIDATOR_COMMENT, errorMessage.c_str());
				if (notifyUserOnFailure)
				{
					DrxMessageBox(errorMessage.c_str(), "Plug-in load failed!", eMB_Error);
				}

				return false;
			}

			// Create RAII struct to ensure that module is unloaded on failure
			SNativePluginModule nativeModule(szPath);

			// Find the first Drx::IEnginePlugin instance inside the module
			PtrFunc_GetHeadToRegFactories getHeadToRegFactories = (PtrFunc_GetHeadToRegFactories)DrxGetProcAddress(hModule, "GetHeadToRegFactories");
			SRegFactoryNode* pFactoryNode = getHeadToRegFactories();

			while (pFactoryNode != nullptr)
			{
				if (pFactoryNode->m_pFactory->ClassSupports(drxiidof<Drx::IEnginePlugin>()))
				{
					nativeModule.m_pFactory = pFactoryNode->m_pFactory;
					break;
				}

				pFactoryNode = pFactoryNode->m_pNext;
			}

			if (nativeModule.m_pFactory == nullptr)
			{
				string errorMessage = string().Format("Plugin load failed - valid IDrxPlugin implementation was not found in plugin %s!", szPath);
				DrxWarning(VALIDATOR_MODULE_SYSTEM, notifyUserOnFailure ? VALIDATOR_ERROR : VALIDATOR_COMMENT, errorMessage.c_str());
				if (notifyUserOnFailure)
				{
					DrxMessageBox(errorMessage.c_str(), "Plug-in load failed!", eMB_Error);
				}

				return false;
			}

			IDrxUnknownPtr pUnk = nativeModule.m_pFactory->CreateClassInstance();
			std::shared_ptr<Drx::IEnginePlugin> pPlugin = drxinterface_cast<Drx::IEnginePlugin>(pUnk);
			if (pPlugin == nullptr)
			{
				string errorMessage = string().Format("Plugin load failed - failed to create plug-in class instance in %s!", szPath);
				DrxWarning(VALIDATOR_MODULE_SYSTEM, notifyUserOnFailure ? VALIDATOR_ERROR : VALIDATOR_COMMENT, errorMessage.c_str());
				if (notifyUserOnFailure)
				{
					DrxMessageBox(errorMessage.c_str(), "Plug-in load failed!", eMB_Error);
				}

				return false;
			}

			m_pluginContainer.emplace_back(pPlugin, std::move(nativeModule));

			break;
		}

	case EType::Managed:
		{
			if (gEnv->pMonoRuntime == nullptr)
			{
				string errorMessage = string().Format("Plugin load failed - Tried to load Mono plugin %s without having loaded the DrxMono module!", szPath);
				DrxWarning(VALIDATOR_MODULE_SYSTEM, notifyUserOnFailure ? VALIDATOR_ERROR : VALIDATOR_COMMENT, errorMessage.c_str());
				if (notifyUserOnFailure)
				{
					DrxMessageBox(errorMessage.c_str(), "Plug-in load failed!", eMB_Error);
				}

				return false;
			}

			std::shared_ptr<Drx::IEnginePlugin> pPlugin = gEnv->pMonoRuntime->LoadBinary(szPath);
			if (pPlugin == nullptr)
			{
				string errorMessage = string().Format("Plugin load failed - Plugin load failed - Could not load Mono binary %s!", szPath);
				DrxWarning(VALIDATOR_MODULE_SYSTEM, notifyUserOnFailure ? VALIDATOR_ERROR : VALIDATOR_COMMENT, errorMessage.c_str());
				if (notifyUserOnFailure)
				{
					DrxMessageBox(errorMessage.c_str(), "Plug-in load failed!", eMB_Error);
				}

				return false;
			}

			m_pluginContainer.emplace_back(pPlugin);
			break;
		}
	}

	return OnPluginLoaded(notifyUserOnFailure);
}
#endif

bool CDrxPluginUpr::OnPluginLoaded(bool notifyUserOnFailure)
{
	SPluginContainer& containedPlugin = m_pluginContainer.back();

	if (!containedPlugin.Initialize(*gEnv, m_systemInitParams))
	{
		string errorMessage = string().Format("Plugin load failed - Initialization failure, check log for possible errors");
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, errorMessage.c_str());
		if (notifyUserOnFailure)
		{
			DrxMessageBox(errorMessage.c_str(), "Plug-in load failed!", eMB_Error);
		}

		if (containedPlugin.m_pPlugin != nullptr)
		{
			OnPluginUnloaded(containedPlugin.m_pPlugin.get());
		}

		containedPlugin.Shutdown();
		m_pluginContainer.pop_back();

		return false;
	}

	// Notification to listeners, that plugin got initialized
	NotifyEventListeners(containedPlugin.m_pluginClassId, IEventListener::EEvent::Initialized);
	return true;
}

void CDrxPluginUpr::OnPluginUnloaded(Drx::IEnginePlugin* pPlugin)
{
	for (u8 i = 0; i < static_cast<u8>(Drx::IEnginePlugin::EUpdateStep::Count); ++i)
	{
		std::vector<Drx::IEnginePlugin*>& updatedPlugins = GetUpdatedPluginsForStep(static_cast<Drx::IEnginePlugin::EUpdateStep>(1 << i));

		stl::find_and_erase(updatedPlugins, pPlugin);
	}
}

bool CDrxPluginUpr::UnloadAllPlugins()
{
	bool bError = false;
	for(auto it = m_pluginContainer.rbegin(), end = m_pluginContainer.rend(); it != end; ++it)
	{
		if (it->m_pPlugin != nullptr)
		{
			OnPluginUnloaded(it->m_pPlugin.get());
		}

		if (!it->Shutdown())
		{
			bError = true;
		}

		// notification to listeners, that plugin got un-initialized
		NotifyEventListeners(it->m_pluginClassId, IEventListener::EEvent::Unloaded);
	}

	m_pluginContainer.clear();

	return !bError;
}

void CDrxPluginUpr::NotifyEventListeners(const DrxClassID& classID, IEventListener::EEvent event)
{
	for (const TPluginListenerPair& pluginPair : m_pluginListenerMap)
	{
		if (std::find(pluginPair.second.cbegin(), pluginPair.second.cend(), classID) != pluginPair.second.cend())
		{
			pluginPair.first->OnPluginEvent(classID, event);
		}
	}
}

void CDrxPluginUpr::UpdateBeforeSystem()
{
	const std::vector<Drx::IEnginePlugin*>& updatedPlugins = GetUpdatedPluginsForStep(Drx::IEnginePlugin::EUpdateStep::BeforeSystem);
	for (Drx::IEnginePlugin* pPlugin : updatedPlugins)
	{
		pPlugin->UpdateBeforeSystem();
	}
}

void CDrxPluginUpr::UpdateBeforePhysics()
{
	const std::vector<Drx::IEnginePlugin*>& updatedPlugins = GetUpdatedPluginsForStep(Drx::IEnginePlugin::EUpdateStep::BeforePhysics);
	for (Drx::IEnginePlugin* pPlugin : updatedPlugins)
	{
		pPlugin->UpdateBeforePhysics();
	}
}

void CDrxPluginUpr::UpdateAfterSystem()
{
	float frameTime = gEnv->pTimer->GetFrameTime();

	const std::vector<Drx::IEnginePlugin*>& updatedPlugins = GetUpdatedPluginsForStep(Drx::IEnginePlugin::EUpdateStep::MainUpdate);
	for (Drx::IEnginePlugin* pPlugin : updatedPlugins)
	{
		pPlugin->MainUpdate(frameTime);
	}
}

void CDrxPluginUpr::UpdateBeforeFinalizeCamera()
{
	const std::vector<Drx::IEnginePlugin*>& updatedPlugins = GetUpdatedPluginsForStep(Drx::IEnginePlugin::EUpdateStep::BeforeFinalizeCamera);
	for (Drx::IEnginePlugin* pPlugin : updatedPlugins)
	{
		pPlugin->UpdateBeforeFinalizeCamera();
	}
}

void CDrxPluginUpr::UpdateBeforeRender()
{
	const std::vector<Drx::IEnginePlugin*>& updatedPlugins = GetUpdatedPluginsForStep(Drx::IEnginePlugin::EUpdateStep::BeforeRender);
	for (Drx::IEnginePlugin* pPlugin : updatedPlugins)
	{
		pPlugin->UpdateBeforeRender();
	}
}

void CDrxPluginUpr::UpdateAfterRender()
{
	const std::vector<Drx::IEnginePlugin*>& updatedPlugins = GetUpdatedPluginsForStep(Drx::IEnginePlugin::EUpdateStep::AfterRender);
	for (Drx::IEnginePlugin* pPlugin : updatedPlugins)
	{
		pPlugin->UpdateAfterRender();
	}
}

void CDrxPluginUpr::UpdateAfterRenderSubmit()
{
	const std::vector<Drx::IEnginePlugin*>& updatedPlugins = GetUpdatedPluginsForStep(Drx::IEnginePlugin::EUpdateStep::AfterRenderSubmit);
	for (Drx::IEnginePlugin* pPlugin : updatedPlugins)
	{
		pPlugin->UpdateAfterRenderSubmit();
	}
}

std::shared_ptr<Drx::IEnginePlugin> CDrxPluginUpr::QueryPluginById(const DrxClassID& classID) const
{
	for (const SPluginContainer& it : m_pluginContainer)
	{
		IDrxFactory* pFactory = it.GetPluginPtr()->GetFactory();
		if (pFactory)
		{
			if (pFactory->GetClassID() == classID || pFactory->ClassSupports(classID))
			{
				return it.m_pPlugin;
			}
		}
	}

	return nullptr;
}

void CDrxPluginUpr::OnPluginUpdateFlagsChanged(Drx::IEnginePlugin& plugin, u8 newFlags, u8 changedStep)
{
	std::vector<Drx::IEnginePlugin*>& updatedPlugins = GetUpdatedPluginsForStep(static_cast<Drx::IEnginePlugin::EUpdateStep>(changedStep));
	if ((newFlags & changedStep) != 0)
	{
		updatedPlugins.push_back(&plugin);
	}
	else
	{
		stl::find_and_erase(updatedPlugins, &plugin);
	}
}