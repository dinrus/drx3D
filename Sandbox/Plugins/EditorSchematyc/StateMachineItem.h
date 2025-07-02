// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AbstractObjectItem.h"

#include <DrxIcon.h>

#include <QString>

namespace Schematyc {

struct IScriptStateMachine;

}

namespace DrxSchematycEditor {

class CStateItem;

class CStateMachineItem : public CAbstractObjectStructureModelItem
{
public:
	CStateMachineItem(Schematyc::IScriptStateMachine& scriptStateMachine, CAbstractObjectStructureModel& model);
	virtual ~CStateMachineItem();

	// CAbstractObjectStructureModelItem
	virtual void                               SetName(QString name) override;
	virtual i32                              GetType() const override          { return eObjectItemType_StateMachine; }

	virtual const DrxIcon*                     GetIcon() const override;
	virtual CAbstractObjectStructureModelItem* GetParentItem() const override    { return static_cast<CAbstractObjectStructureModelItem*>(m_pParentItem); }

	virtual u32                             GetNumChildItems() const override { return GetNumStates(); }
	virtual CAbstractObjectStructureModelItem* GetChildItemByIndex(u32 index) const override;
	virtual u32                             GetChildItemIndex(const CAbstractObjectStructureModelItem& item) const override;

	virtual u32                             GetIndex() const;
	virtual void                               Serialize(Serialization::IArchive& archive) override;

	virtual bool                               AllowsRenaming() const override;
	// ~CAbstractObjectStructureModelItem

	void             SetParentItem(CAbstractObjectStructureModelItem* pParentItem) { m_pParentItem = pParentItem; }

	u32           GetNumStates() const                                          { return m_states.size(); }
	CStateItem*      GetStateItemByIndex(u32 index) const;
	CStateItem*      CreateState();
	bool             RemoveState();

	DrxGUID GetGUID() const;

protected:
	void LoadFromScriptElement();

private:
	CAbstractObjectStructureModelItem* m_pParentItem;
	Schematyc::IScriptStateMachine&    m_scriptStateMachine;
	std::vector<CStateItem*>           m_states;
};

}

