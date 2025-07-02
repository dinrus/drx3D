// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "LevelIndependentFileMan.h"
#include "HyperGraph/FlowGraph.h"
#include <DrxFlowGraph/IFlowGraphModuleManager.h>

class CEditorFlowGraphModuleManager : public ILevelIndependentFileModule, public IFlowGraphModuleListener
{
public:
	CEditorFlowGraphModuleManager();
	virtual ~CEditorFlowGraphModuleManager();

	//ILevelIndependentFileModule
	virtual bool PromptChanges();
	//~ILevelIndependentFileModule

	//IFlowGraphModuleListener
	virtual void OnModuleInstanceCreated(IFlowGraphModule* module, TModuleInstanceId instanceID)   {}
	virtual void OnModuleInstanceDestroyed(IFlowGraphModule* module, TModuleInstanceId instanceID) {}
	virtual void OnModuleDestroyed(IFlowGraphModule* module);
	virtual void OnRootGraphChanged(IFlowGraphModule* module, ERootGraphChangeReason reason);
	virtual void OnModulesScannedAndReloaded();
	//~IFlowGraphModuleListener

	void          Init();

	bool          NewModule(string& filename, bool bGlobal = true, CHyperGraph** pHyperGraph = nullptr);

	IFlowGraphPtr GetModuleFlowGraph(i32 index) const;
	IFlowGraphPtr GetModuleFlowGraph(tukk name) const;
	void          DeleteModuleFlowGraph(CHyperFlowGraph* pGraph);

	void          CreateModuleNodes(tukk moduleName);

	bool          SaveModuleGraph(CHyperFlowGraph* pFG);

private:

	bool HasModifications();
	void SaveChangedGraphs();
	void DiscardChangedGraphs();
	void CreateEditorFlowgraphs();

	typedef std::vector<CHyperFlowGraph*> TFlowGraphs;
	TFlowGraphs m_FlowGraphs;
};

