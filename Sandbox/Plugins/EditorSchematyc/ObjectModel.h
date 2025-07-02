// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "ComponentsModel.h"
#include "ComponentsDictionaryModel.h"

#include "AbstractObjectModel.h"

#include "VariablesModel.h"

#include <drx3D/CoreX/Sandbox/DrxSignal.h>
#include <Controls/DictionaryWidget.h>

namespace Schematyc {

struct IScriptView;
struct IScriptRegistry;

}

namespace DrxSchematycEditor {

class CComponentItem;
class CInterfaceImplItem;
class CGraphItems;
class CStateItem;
class CStateMachineItem;
class CInterfaceImplItem;
class CVariableItem;

enum EResourceType
{
	Unknown = 0,
	Object,
	Library,
};

class CObjectModel :
	public CAbstractComponentsModel,
	public CAbstractObjectStructureModel,
	public CAbstractVariablesModel
{
	typedef std::vector<CComponentItem*>     Components;
	typedef std::vector<CInterfaceImplItem*> Interfaces;
	typedef std::vector<CGraphItem*>         Graphs;
	typedef std::vector<CStateItem*>         StateTree;
	typedef std::vector<CVariableItem*>      Variables;

public:
	CObjectModel(Schematyc::IScriptView& scriptView, EResourceType resourceType);
	~CObjectModel();

	EResourceType GetType() const { return m_resourceType; }

	// CAbstractComponentsModel
	virtual u32                     GetComponentItemCount() const override { return m_components.size(); }
	virtual CComponentItem*            GetComponentItemByIndex(u32 index) const override;
	virtual CComponentItem*            CreateComponent(DrxGUID typeId, tukk szName) override;
	virtual bool                       RemoveComponent(CComponentItem& component) override;

	virtual CAbstractDictionary*       GetAvailableComponentsDictionary() { return static_cast<CAbstractDictionary*>(&m_componentsDictionary); };

	virtual Schematyc::IScriptElement* GetScriptElement() const override  { return &m_scriptElement; }
	// ~CAbstractComponentsModel

	// CAbstractObjectStructureModel
	virtual u32             GetGraphItemCount() const override { return m_graphs.size(); }
	virtual CGraphItem*        GetGraphItemByIndex(u32 index) const override;
	virtual CGraphItem*        CreateGraph() override;
	virtual bool               RemoveGraph(CGraphItem& functionItem) override;

	virtual u32             GetStateMachineItemCount() const override { return m_stateMachines.size(); }
	virtual CStateMachineItem* GetStateMachineItemByIndex(u32 index) const override;
	virtual CStateMachineItem* CreateStateMachine(Schematyc::EScriptStateMachineLifetime stateMachineLifetime) override;
	virtual bool               RemoveStateMachine(CStateMachineItem& stateItem) override;
	// ~CAbstractObjectStructureModel

	// CAbstractVariablesModel
	virtual u32                       GetNumVariables() const override { return m_variables.size(); }
	virtual CAbstractVariablesModelItem* GetVariableItemByIndex(u32 index) override;
	virtual u32                       GetVariableItemIndex(const CAbstractVariablesModelItem& variableItem) const override;
	virtual CAbstractVariablesModelItem* CreateVariable() override;
	virtual bool                         RemoveVariable(CAbstractVariablesModelItem& variableItem) override;
	// ~CAbstractVariablesModel

	// CAbstractVariableTypesModel
	// TODO
	// ~CAbstractVariableTypesModel

	// CAbstractSignalsModel
	// TODO
	// ~CAbstractSignalsModel

	u32 GetVariableIndex(const CVariableItem& variableItem) const;

protected:
	void LoadFromScriptElement();

private:
	const EResourceType             m_resourceType;
	Schematyc::IScriptView&         m_scriptView;
	Schematyc::IScriptRegistry&     m_scriptRegistry;
	Schematyc::IScriptElement&      m_scriptElement;

	CComponentsDictionary           m_componentsDictionary;
	Components                      m_components;

	Graphs                          m_graphs;
	Variables                       m_variables;
	std::vector<CStateMachineItem*> m_stateMachines;
};

}

