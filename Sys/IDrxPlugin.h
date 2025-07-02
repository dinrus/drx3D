// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/IDrxPluginUpr.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/CoreX/Extension/IDrxUnknown.h>
#include <drx3D/CoreX/Extension/IDrxFactoryRegistryImpl.h>
#include <drx3D/FlowGraph/IFlowBaseNode.h>

namespace Drx
{
	//! Represents an engine plug-in that can be loaded as a dynamic library, or statically linked in.
	//! Plug-ins are loaded from JSON in your .drxproject file on engine startup.
	//! For a fully functional example, see Code\GameTemplates\cpp\Plugin.
	//! \par Example
	//! \include DinrusXSys/Examples/MinimalPlugin.cpp
	struct IEnginePlugin : public IDrxUnknown
	{
		DRXINTERFACE_DECLARE_GUID(IEnginePlugin, "f491a0db-3863-4fca-b6e6-bcfe2d98eea2"_drx_guid);

		//! Used to determine what type of updates a plug-in interested in, treated as bit flags
		//! This enumeration is ordered in the same order as the functions are called each frame
		enum class EUpdateStep : u8
		{
			BeforeSystem = 1 << 0,
			BeforePhysics = 1 << 1,
			MainUpdate = 1 << 2,
			BeforeFinalizeCamera = 1 << 3,
			BeforeRender = 1 << 4,
			AfterRender = 1 << 5,
			AfterRenderSubmit = 1 << 6,

			Count = 7
		};

		virtual ~IEnginePlugin() {}

		//! Used to check the name of the plug-in, by default returns the string specified with DRXGENERATE_CLASS_GUID or DRXGENERATE_SINGLETONCLASS_GUID
		virtual tukk GetName() const { return GetFactory()->GetName(); }

		//! Used to check what category this plug-in belongs to.
		virtual tukk GetCategory() const { return "Default"; }

		//! Called shortly after plug-in load when the plug-in is expected to initialize all its systems
		virtual bool Initialize(SSysGlobEnv& env, const SSysInitParams& initParams) = 0;

		//! Earliest point of update in a frame, before ISystem itself is updated
		//! Called on a plug-in if it has called EnableUpdate(EUpdateStep::BeforeSystem, true)
		virtual void UpdateBeforeSystem() {}
		//! Called before physics is updated for the new frame, best point for queuing physics jobs
		//! Called on a plug-in if it has called EnableUpdate(EUpdateStep::BeforePhysics, true)
		virtual void UpdateBeforePhysics() {}
		//! Called after ISystem has been updated, this is the main update where most game logic is expected to occur
		//! This is the default update that should be preferred if you don't need any special behavior
		//! Called on a plug-in if it has called EnableUpdate(EUpdateStep::MainUpdate, true)
		virtual void MainUpdate(float frameTime) {}
		//! Called immediately before the camera is considered finalized and passed on to occlusion culling
		//! Called on a plug-in if it has called EnableUpdate(EUpdateStep::BeforeFinalizeCamera, true)
		virtual void UpdateBeforeFinalizeCamera() {}
		//! Called before we begin rendering the scene
		//! Called on a plug-in if it has called EnableUpdate(EUpdateStep::BeforeRender, true)
		virtual void UpdateBeforeRender() {}
		//! Called after we have submitted the render request to the render thread
		//! Called on a plug-in if it has called EnableUpdate(EUpdateStep::AfterRender, true)
		virtual void UpdateAfterRender() {}
		//! Called after the renderer has submitted the frame to the output device
		//! Called on a plug-in if it has called EnableUpdate(EUpdateStep::AfterRenderSubmit, true)
		virtual void UpdateAfterRenderSubmit() {}

		//! Called when the plug-in should register all its flow nodes
		//! Automated version can be enabled using the PLUGIN_FLOWNODE_REGISTER macro
		virtual bool RegisterFlowNodes() { return false; }
		//! Called when the plug-in should unregister all its flow nodes
		//! Automated version can be enabled using the PLUGIN_FLOWNODE_UNREGISTER macro
		virtual bool UnregisterFlowNodes() { return false; }

	protected:
		//! Enables a particular update step to be called each frame on this plug-in
		//! \par Example
		//! \include DinrusXSys/Examples/MinimalPlugin.cpp
		void EnableUpdate(EUpdateStep step, bool enable)
		{
			if (enable)
			{
				if ((m_updateFlags & static_cast<u8>(step)) == 0)
				{
					m_updateFlags |= static_cast<u8>(step);

					gEnv->pSystem->GetIPluginUpr()->OnPluginUpdateFlagsChanged(*this, m_updateFlags, static_cast<u8>(step));
				}
			}
			else
			{
				if ((m_updateFlags & static_cast<u8>(step)) != 0)
				{
					m_updateFlags &= ~static_cast<u8>(step);

					gEnv->pSystem->GetIPluginUpr()->OnPluginUpdateFlagsChanged(*this, m_updateFlags, static_cast<u8>(step));
				}
			}
		}

	protected:
		std::vector<TFlowNodeTypeId> m_registeredFlowNodeIds;
		
	private:
		u8 m_updateFlags = 0;
	};

	#ifndef _LIB
		#define PLUGIN_FLOWNODE_REGISTER                                                                    \
		  virtual bool RegisterFlowNodes() override                                                         \
		  {                                                                                                 \
			m_registeredFlowNodeIds.clear();                                                                \
																											\
			if (auto pFlowSystem = gEnv->pFlowSystem)                                                       \
			{                                                                                               \
			  CAutoRegFlowNodeBase* pFactory = CAutoRegFlowNodeBase::s_pFirst;                              \
			  for (auto pFactory = CAutoRegFlowNodeBase::s_pFirst; pFactory; pFactory = pFactory->m_pNext)  \
			  {                                                                                             \
				TFlowNodeTypeId nodeId = pFlowSystem->RegisterType(pFactory->m_szClassName, pFactory);      \
				m_registeredFlowNodeIds.push_back(nodeId);                                                  \
				DrxLog("Successfully registered flownode '%s'", pFactory->m_szClassName);                   \
			  }                                                                                             \
			}                                                                                               \
																											\
			return m_registeredFlowNodeIds.empty();                                                         \
		  }
	#else
		#define PLUGIN_FLOWNODE_REGISTER
	#endif

	#ifndef _LIB
		#define PLUGIN_FLOWNODE_UNREGISTER                                         \
		  virtual bool UnregisterFlowNodes() override                              \
		  {                                                                        \
			bool bSuccess = true;                                                  \
			const size_t numFlownodes = m_registeredFlowNodeIds.size();            \
																				   \
			if (auto pFlowSystem = gEnv->pFlowSystem)                              \
			{                                                                      \
			  for (auto nodeId : m_registeredFlowNodeIds)                          \
			  {                                                                    \
				tukk szNameBuffer = gEnv->pFlowSystem->GetTypeName(nodeId); \
				DrxLog("Unregistering flownode '%s'", szNameBuffer);               \
				if (!pFlowSystem->UnregisterType(szNameBuffer))                    \
				{                                                                  \
				  bSuccess = false;                                                \
				  DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR,             \
							 "Error unregistering flownode '%s'", szNameBuffer);   \
				}                                                                  \
			  }                                                                    \
			}                                                                      \
			m_registeredFlowNodeIds.clear();                                       \
			return bSuccess;                                                       \
		  }
#else
#define PLUGIN_FLOWNODE_UNREGISTER
#endif
}