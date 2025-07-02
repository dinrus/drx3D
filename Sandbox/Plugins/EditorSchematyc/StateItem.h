// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AbstractObjectItem.h"
#include "VariablesModel.h"

#include "GraphItem.h"

#include <QString>
#include <QIcon>

namespace Schematyc {

struct IScriptState;

}

namespace DrxSchematycEditor {

class CVariableItem;

class CStateItem : public CAbstractObjectStructureModelItem
{
public:
	CStateItem(Schematyc::IScriptState& scriptState, CAbstractObjectStructureModel& model);
	virtual ~CStateItem();

	// CAbstractObjectStructureModelItem
	virtual void                               SetName(QString name) override;
	virtual i32                              GetType() const override       { return eObjectItemType_State; }
	virtual const DrxIcon*                     GetIcon() const override;
	virtual CAbstractObjectStructureModelItem* GetParentItem() const override { return m_pParentItem; }

	virtual u32                             GetNumChildItems() const override;
	virtual CAbstractObjectStructureModelItem* GetChildItemByIndex(u32 index) const override;
	virtual u32                             GetChildItemIndex(const CAbstractObjectStructureModelItem& item) const override;

	virtual u32                             GetIndex() const;
	virtual void                               Serialize(Serialization::IArchive& archive) override;

	virtual bool                               AllowsRenaming() const override;
	// ~CAbstractObjectStructureModelItem

	void        SetParentItem(CAbstractObjectStructureModelItem* pParent) { m_pParentItem = pParent; }

	CGraphItem* CreateGraph(CGraphItem::EGraphType type);
	bool        RemoveGraph(CGraphItem& functionItem);

	CStateItem* CreateState();
	bool        RemoveState(CStateItem& stateItem);

	//CSignalItem* CreateSignal();
	//bool         RemoveState(CSignalItem& stateItem);

	DrxGUID GetGUID() const;

protected:
	void LoadFromScriptElement();

private:
	CAbstractObjectStructureModelItem* m_pParentItem;
	Schematyc::IScriptState&           m_scriptState;

	std::vector<CGraphItem*>           m_graphs;
	std::vector<CStateItem*>           m_states;

	// CAbstractVariablesViewModel
	std::vector<CVariableItem*> m_variables;
};

}

