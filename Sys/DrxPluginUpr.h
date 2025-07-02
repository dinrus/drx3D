// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/ProjectUpr.h>

#include <drx3D/Sys/IDrxPluginUpr.h>
#include <drx3D/Sys/IDrxPlugin.h>
#include <array>

#include <drx3D/CoreX/Platform/DrxLibrary.h>

struct SPluginContainer;

class CDrxPluginUpr final
	: public Drx::IPluginUpr
	, public ISystemEventListener
{
public:
	using TPluginListenerPair = std::pair<IEventListener*, std::vector<DrxClassID>>;

	CDrxPluginUpr(const SSysInitParams& initParams);
	virtual ~CDrxPluginUpr();

	// Drx::IPluginUpr
	virtual void RegisterEventListener(const DrxClassID& pluginClassId, IEventListener* pListener) override
	{
		// we have to simply add this listener now because the plugin can be loaded at any time
		// this should change in release builds since all the necessary plugins will be loaded upfront
		m_pluginListenerMap[pListener].push_back(pluginClassId);
	}

	virtual void RemoveEventListener(const DrxClassID& pluginClassId, IEventListener* pListener) override
	{
		auto it = m_pluginListenerMap.find(pListener);
		if (it != m_pluginListenerMap.end())
		{
			stl::find_and_erase(it->second, pluginClassId);
		}
	}

	virtual std::shared_ptr<Drx::IEnginePlugin> QueryPluginById(const DrxClassID& classID) const override;
	virtual void OnPluginUpdateFlagsChanged(Drx::IEnginePlugin& plugin, u8 newFlags, u8 changedStep) override;
	// ~Drx::IPluginUpr

	// Called by DinrusSystem during early init to initialize the manager and load plugins
	// Plugins that require later activation can do so by listening to system events such as ESYSTEM_EVENT_PRE_RENDERER_INIT
	void LoadProjectPlugins();

	void UpdateBeforeSystem() override;
	void UpdateBeforePhysics() override;
	void UpdateAfterSystem() override;
	void UpdateBeforeFinalizeCamera() override;
	void UpdateBeforeRender() override;
	void UpdateAfterRender() override;
	void UpdateAfterRenderSubmit() override;

	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;

	using TDefaultPluginPair = std::pair<u8 /* version with which the plug-in was made default */, SPluginDefinition>;
	// Gets the plug-ins, along with the version that they were made default in
	// This is called in order to update the default plug-ins for projects on upgrade
	// These are also built with the engine, thus will be statically linked in for monolithic builds.
	static std::array<TDefaultPluginPair, 4> GetDefaultPlugins()
	{
		return
		{
			{
				// Plug-ins made default with version 1
				{ 1, SPluginDefinition { EType::Native, "DrxDefaultEntities" } },
				{ 1, SPluginDefinition { EType::Native, "DrxSensorSystem" } },
				{ 1, SPluginDefinition { EType::Native, "DrxPerceptionSystem" } },
				// Plug-ins made default with version 3
				{ 3, SPluginDefinition{ EType::Native, "DrxGamePlatform",{ EPlatform::PS4 } } }
			}
		};
	}

protected:
#if DrxSharedLibrarySupported
	bool LoadPluginFromDisk(EType type, tukk path, bool notifyUserOnFailure = true);
#endif

	bool OnPluginLoaded(bool notifyUserOnFailure = true);
	void OnPluginUnloaded(Drx::IEnginePlugin* pPlugin);

	std::vector<Drx::IEnginePlugin*>& GetUpdatedPluginsForStep(Drx::IEnginePlugin::EUpdateStep step) { return m_updatedPlugins[IntegerLog2(static_cast<u8>(step))]; }

private:
	bool                    UnloadAllPlugins();
	void                    NotifyEventListeners(const DrxClassID& classID, IEventListener::EEvent event);

	std::vector<SPluginContainer> m_pluginContainer;
	std::map<IEventListener*, std::vector<DrxClassID>> m_pluginListenerMap;

	const SSysInitParams m_systemInitParams;
	bool                    m_bLoadedProjectPlugins;

	std::array<std::vector<Drx::IEnginePlugin*>, static_cast<size_t>(Drx::IEnginePlugin::EUpdateStep::Count)> m_updatedPlugins;
};
