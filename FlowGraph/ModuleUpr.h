// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/FlowGraph/IFlowGraphModuleUpr.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>
#include <drx3D/CoreX/Containers/VectorMap.h>

struct SActivationInfo;
class CFlowGraphModule;
class CFlowModuleCallNode;

class CFlowGraphModuleUpr : public IFlowGraphModuleUpr, public ISystemEventListener, public IGameFrameworkListener
{
public:
	CFlowGraphModuleUpr();
	virtual ~CFlowGraphModuleUpr();
	CFlowGraphModuleUpr(CFlowGraphModuleUpr const&) = delete;
	CFlowGraphModuleUpr& operator=(CFlowGraphModuleUpr const&) = delete;
	CFlowGraphModuleUpr(CFlowGraphModuleUpr &&) = delete;
	CFlowGraphModuleUpr& operator=(CFlowGraphModuleUpr&& other) = delete;

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);
	// ~ISystemEventListener

	// IGameFrameworkListener
	virtual void OnPostUpdate(float fDeltaTime);
	virtual void OnSaveGame(ISaveGame* pSaveGame)         {}
	virtual void OnLoadGame(ILoadGame* pLoadGame)         {}
	virtual void OnLevelEnd(tukk nextLevel)        {}
	virtual void OnActionEvent(const SActionEvent& event) {}
	// ~IGameFrameworkListener

	// IFlowGraphModuleUpr
	virtual bool               RegisterListener(IFlowGraphModuleListener* pListener, tukk name);
	virtual void               UnregisterListener(IFlowGraphModuleListener* pListener);

	virtual bool               DeleteModuleXML(tukk moduleName);
	virtual bool               RenameModuleXML(tukk moduleName, tukk newName);

	virtual tukk        GetModulePath(tukk name);

	virtual bool               SaveModule(tukk moduleName, XmlNodeRef saveTo);
	IFlowGraphModule*          LoadModuleFile(tukk moduleName, tukk fileName, bool bGlobal);

	virtual IFlowGraphModule*  GetModule(IFlowGraphPtr pFlowgraph) const;
	virtual IFlowGraphModule*  GetModule(tukk moduleName) const;
	virtual IFlowGraphModule*  GetModule(TModuleId id) const;

	virtual tukk        GetStartNodeName(tukk moduleName) const;
	virtual tukk        GetReturnNodeName(tukk moduleName) const;
	virtual tukk        GetCallerNodeName(tukk moduleName) const;

	virtual void               ClearModules();
	virtual void               ClearLevelModules();
	virtual void               ScanAndReloadModules(bool bScanGlobalModules, bool bScanLevelModules, tukk szLoadedLevelName);


	virtual bool               CreateModuleNodes(tukk moduleName, TModuleId moduleId);

	virtual IModuleIteratorPtr CreateModuleIterator();
	// ~IFlowGraphModuleUpr

	void Shutdown();

	void DestroyActiveModuleInstances();
	void RemoveCompletedModuleInstances(); // cleanup at end of update
	void ClearModuleRequestedInstances();

	void BroadcastModuleInstanceStarted(IFlowGraphModule* module, TModuleInstanceId instanceID);
	void BroadcastModuleInstanceFinished(IFlowGraphModule* module, TModuleInstanceId instanceID);

	DeclareStaticConstIntCVar(CV_fg_debugmodules, 0);
	ICVar* fg_debugmodules_filter;

private:
	typedef VectorMap<TModuleId, CFlowGraphModule*> TModuleMap;

	void              DestroyModule(TModuleMap::iterator& itModule);

	void              RescanModuleNames(bool bGlobal, tukk szLoadedLevelName);
	void              ScanFolder(const string& folderName, bool bGlobal);

	CFlowGraphModule* PreLoadModuleFile(tukk moduleName, tukk fileName, bool bGlobal);
	void              LoadModuleGraph(tukk moduleName, tukk fileName, IFlowGraphModuleListener::ERootGraphChangeReason rootGraphChangeReason);

	// Loaded modules
	TModuleMap m_Modules;
	TModuleId  m_nextModuleId;

	// Module name-ID map
	typedef std::unordered_map<string, TModuleId, stl::hash_strcmp<string>> TModuleIdMap;
	TModuleIdMap m_ModuleIds;

	// Module name-path map
	typedef std::unordered_map<string, string, stl::hash_strcmp<string>> TModulesPathInfo;
	TModulesPathInfo m_ModulesPathInfo;

	// Listeners
	CListenerSet<IFlowGraphModuleListener*> m_listeners;

	// Module Iterator
	class CModuleIterator : public IFlowGraphModuleIterator
	{
	public:
		CModuleIterator(CFlowGraphModuleUpr* pMM)
		{
			m_pModuleUpr = pMM;
			m_cur = m_pModuleUpr->m_Modules.begin();
			m_nRefs = 0;
		}

		void AddRef()
		{
			++m_nRefs;
		}

		void Release()
		{
			if (--m_nRefs <= 0)
			{
				this->~CModuleIterator();
				m_pModuleUpr->m_iteratorPool.push_back(this);
			}
		}

		IFlowGraphModule* Next();

		size_t            Count()
		{
			return m_pModuleUpr->m_Modules.size();
		}

		CFlowGraphModuleUpr* m_pModuleUpr;
		TModuleMap::iterator     m_cur;
		i32                      m_nRefs;
	};

	std::vector<CModuleIterator*> m_iteratorPool;

};
