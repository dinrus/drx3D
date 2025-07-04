// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FLOWSYSTEM_H__
#define __FLOWSYSTEM_H__

#pragma once

#include <drx3D/FlowGraph/IFlowSystem.h>
#include "FlowSystemCVars.h"
#include <drx3D/CoreX/Containers/DrxListenerSet.h>

class CFlowGraphBase;
class CFlowGraphModuleUpr;
class CGameTokenSystem;

struct CFlowSystemContainer : public IFlowSystemContainer
{
	virtual void           AddItem(TFlowInputData item);
	virtual void           AddItemUnique(TFlowInputData item);

	virtual void           RemoveItem(TFlowInputData item);

	virtual TFlowInputData GetItem(i32 i);

	virtual void           RemoveItemAt(i32 i);
	virtual i32            GetItemCount() const;

	virtual void           Clear();

	virtual void           GetMemoryUsage(IDrxSizer* s) const;

	virtual void           Serialize(TSerialize ser);
private:
	DynArray<TFlowInputData> m_container;
};

class CFlowSystem : public IFlowSystem, public IEntitySystemSink, public ISystemEventListener, public IEntityClassRegistryListener
{
public:
	CFlowSystem();
	~CFlowSystem();

	// IFlowSystem
	virtual void                     Release();
	virtual void                     Update();
	virtual void                     Reset(bool unload);
	virtual void                     ReloadAllNodeTypes();
	virtual IFlowGraphPtr            CreateFlowGraph();
	virtual TFlowNodeTypeId          RegisterType(tukk type, IFlowNodeFactoryPtr factory);
	virtual bool                     UnregisterType(tukk typeName);
	virtual bool                     HasRegisteredDefaultFlowNodes() { return m_bRegisteredDefaultNodes; }
	virtual tukk              GetTypeName(TFlowNodeTypeId typeId);
	virtual TFlowNodeTypeId          GetTypeId(tukk typeName);
	virtual IFlowNodeTypeIteratorPtr CreateNodeTypeIterator();
	virtual void                     RegisterInspector(IFlowGraphInspectorPtr pInspector, IFlowGraphPtr pGraph = 0);
	virtual void                     UnregisterInspector(IFlowGraphInspectorPtr pInspector, IFlowGraphPtr pGraph = 0);
	virtual void                     EnableInspecting(bool bEnable) { m_bInspectingEnabled = bEnable; }
	virtual bool                     IsInspectingEnabled() const    { return m_bInspectingEnabled; }
	virtual IFlowGraphInspectorPtr   GetDefaultInspector() const    { return m_pDefaultInspector; }
	virtual IFlowGraph*              GetGraphById(TFlowGraphId graphId);
	virtual void                     OnEntityIdChanged(EntityId oldId, EntityId newId);
	virtual void                     GetMemoryUsage(IDrxSizer* s) const;

	virtual bool                     CreateContainer(TFlowSystemContainerId id);
	virtual void                     DeleteContainer(TFlowSystemContainerId id);
	virtual IFlowSystemContainerPtr  GetContainer(TFlowSystemContainerId id);

	virtual void                     Serialize(TSerialize ser);

	virtual IFlowNodePtr             CreateNodeOfType(IFlowNode::SActivationInfo*, TFlowNodeTypeId typeId);
	// ~IFlowSystem

	// TODO Make a single point of entry for this and the AIProxyUpr to share?
	// IEntitySystemSink
	virtual bool OnBeforeSpawn(SEntitySpawnParams& params)      { return true; }
	virtual void OnSpawn(IEntity* pEntity, SEntitySpawnParams& params);
	virtual bool OnRemove(IEntity* pEntity)                     { return true; }
	virtual void OnReused(IEntity* pEntity, SEntitySpawnParams& params);
	//~IEntitySystemSink

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);
	// ~ISystemEventListener

	// IEntityClassRegistryListener
	void OnEntityClassRegistryEvent(EEntityClassRegistryEvent event, const IEntityClass* pEntityClass);
	// ~IEntityClassRegistryListener

	void                           NotifyCriticalLoadingError() { m_criticalLoadingErrorHappened = true; }

	void                           PreInit();
	void                           Init();
	void                           Shutdown();
	void                           Enable(bool enable) { m_cVars.m_enableUpdates = enable; }

	TFlowGraphId                   RegisterGraph(CFlowGraphBase* pGraph, tukk debugName);
	void                           UnregisterGraph(CFlowGraphBase* pGraph);

	CFlowGraphModuleUpr*       GetModuleUpr();
	const CFlowGraphModuleUpr* GetModuleUpr() const;

	IFlowGraphModuleUpr*       GetIModuleUpr();

	IGameTokenSystem*              GetIGameTokenSystem();

	// resembles IFlowGraphInspector currently
	void NotifyFlow(CFlowGraphBase* pGraph, const SFlowAddress from, const SFlowAddress to);
	void NotifyProcessEvent(CFlowGraphBase* pGraph, IFlowNode::EFlowEvent event, IFlowNode::SActivationInfo* pActInfo, IFlowNode* pImpl);

	struct STypeInfo
	{
		STypeInfo() : name(""), factory(NULL) {}
		STypeInfo(const string& typeName, IFlowNodeFactoryPtr pFactory) : name(typeName), factory(pFactory) {}
		string              name;
		IFlowNodeFactoryPtr factory;

		void                GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(name);
		}
	};

#ifndef _RELEASE
	struct TSFGProfile
	{
		i32  graphsUpdated;
		i32  nodeActivations;
		i32  nodeUpdates;

		void Reset()
		{
			memset(this, 0, sizeof(*this));
		}
	};
	static TSFGProfile FGProfile;
#endif //_RELEASE

	const STypeInfo& GetTypeInfo(TFlowNodeTypeId typeId) const;

private:
	typedef std::queue<XmlNodeRef> TPendingComposites;

	void            LoadExtensions(string path);
	void            LoadExtensionFromXml(XmlNodeRef xml, TPendingComposites* pComposites);
	void            LoadComposites(TPendingComposites* pComposites);

	void            RegisterAllNodeTypes();
	void            RegisterAutoTypes();
	void            RegisterEntityTypes();
	TFlowNodeTypeId GenerateNodeTypeID();

	void            LoadBlacklistedFlownodeXML();
	bool            BlacklistedFlownode(tukk* editorName);

	void            UpdateGraphs();

private:
	// Inspecting enabled/disabled
	bool m_bInspectingEnabled;
	bool m_needToUpdateForwardings;
	bool m_criticalLoadingErrorHappened;
	bool m_bRegisteredDefaultNodes;

	class CNodeTypeIterator;
	typedef std::map<string, TFlowNodeTypeId> TTypeNameToIdMap;
	TTypeNameToIdMap                    m_typeNameToIdMap;
	std::vector<STypeInfo>              m_typeRegistryVec; // 0 is invalid
	typedef CListenerSet<CFlowGraphBase*> TGraphs;
	TGraphs                             m_graphs;
	std::vector<IFlowGraphInspectorPtr> m_systemInspectors; // only inspectors which watch all graphs

	std::vector<TFlowNodeTypeId>        m_freeNodeTypeIDs;
	TFlowNodeTypeId                     m_nextNodeTypeID;
	IFlowGraphInspectorPtr              m_pDefaultInspector;

	CFlowSystemCVars                    m_cVars;

	TFlowGraphId                        m_nextFlowGraphId;

	CFlowGraphModuleUpr*            m_pModuleUpr;

	XmlNodeRef                          m_blacklistNode;

	typedef std::map<TFlowSystemContainerId, IFlowSystemContainerPtr> TFlowSystemContainerMap;
	TFlowSystemContainerMap m_FlowSystemContainers;

	CGameTokenSystem* m_pGameTokenSystem = nullptr;
};

inline CFlowGraphModuleUpr* CFlowSystem::GetModuleUpr()
{
	return m_pModuleUpr;
}

////////////////////////////////////////////////////
inline const CFlowGraphModuleUpr* CFlowSystem::GetModuleUpr() const
{
	return m_pModuleUpr;
}

#endif
