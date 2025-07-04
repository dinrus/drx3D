// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Containers/DrxListenerSet.h>

class CGameEngine;

namespace Drx
{
	struct IEnginePlugin;

	//! Main source of plug-in management, from loading to querying of existing plug-ins
	struct IPluginUpr
	{
		//! Determines the type of a plug-in
		enum class EType
		{
			// C++ plug-in
			Native = 0,
			// Mono / C# plug-in
			Managed
		};

		//! Alias for backwards compatibility
		using EPluginType = EType;

		struct IEventListener
		{
			enum class EEvent
			{
				Initialized,
				Unloaded,
			};

			virtual ~IEventListener() {}
			virtual void OnPluginEvent(const DrxClassID& pluginClassId, EEvent event) = 0;
		};

		virtual ~IPluginUpr() = default;

		//! Registers a listener that is notified when a specific plug-in is loaded and unloaded
		template<typename T>
		void RegisterEventListener(IEventListener* pListener) { RegisterEventListener(drxiidof<T>(), pListener); }
	
		//! Removes a listener registered with RegisterEventListener
		template<typename T>
		void RemoveEventListener(IEventListener* pListener) { RemoveEventListener(drxiidof<T>(), pListener); }

		//! Queries a plug-in by implementation (T has to implement Drx::IEnginePlugin)
		//! This call can only succeed if the plug-in was specified in the running project's .drxproject file
		template<typename T>
		T* QueryPlugin() const
		{
			if (IDrxUnknownPtr pExtension = QueryPluginById(drxiidof<T>()))
			{
				return drxinterface_cast<T>(pExtension.get());
			}

			return nullptr;
		}

	protected:
		friend IEnginePlugin;
		virtual void OnPluginUpdateFlagsChanged(IEnginePlugin& plugin, u8 newFlags, u8 changedStep) = 0;

		virtual std::shared_ptr<Drx::IEnginePlugin> QueryPluginById(const DrxClassID& classID) const = 0;

		virtual void RegisterEventListener(const DrxClassID& pluginClassId, IEventListener* pListener) = 0;
		virtual void RemoveEventListener(const DrxClassID& pluginClassId, IEventListener* pListener) = 0;


		friend CGameEngine;
		virtual void UpdateBeforeSystem() = 0;
		virtual void UpdateBeforePhysics() = 0;
		virtual void UpdateAfterSystem() = 0;
		virtual void UpdateBeforeFinalizeCamera() = 0;
		virtual void UpdateBeforeRender() = 0;
		virtual void UpdateAfterRender() = 0;
		virtual void UpdateAfterRenderSubmit() = 0;
	};
}