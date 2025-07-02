// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "GraphItem.h"
#include "StateMachineItem.h"
#include "SignalItem.h"

#include <drx3D/CoreX/Sandbox/DrxSignal.h>

#include <QVariant>
#include <QString>

namespace Schematyc {

struct IScriptElement;

}

namespace DrxSchematycEditor {

class CGraphItem;
class CStateMachineItem;

class CAbstractObjectStructureModel
{
public:
	CAbstractObjectStructureModel() {}
	virtual ~CAbstractObjectStructureModel() {}

	virtual u32                     GetGraphItemCount() const                                                       { return 0; }
	virtual CGraphItem*                GetGraphItemByIndex(u32 index) const                                         { return nullptr; }
	virtual CGraphItem*                CreateGraph()                                                                   { return nullptr; }
	virtual bool                       RemoveGraph(CGraphItem& functionItem)                                           { return false; }

	virtual u32                     GetStateMachineItemCount() const                                                { return 0; }
	virtual CStateMachineItem*         GetStateMachineItemByIndex(u32 index) const                                  { return nullptr; }
	virtual CStateMachineItem*         CreateStateMachine(Schematyc::EScriptStateMachineLifetime stateMachineLifetime) { return nullptr; }
	virtual bool                       RemoveStateMachine(CStateMachineItem& stateItem)                                { return false; }

	virtual Schematyc::IScriptElement* GetScriptElement() const = 0;

	//
	u32                             GetNumItems() const;
	CAbstractObjectStructureModelItem* GetChildItemByIndex(u32 index) const;
	u32                             GetChildItemIndex(const CAbstractObjectStructureModelItem& item) const;
	bool                               RemoveItem(CAbstractObjectStructureModelItem& item);

public:
	CDrxSignal<void(CAbstractObjectStructureModelItem&)> SignalObjectStructureItemAdded;
	CDrxSignal<void(CAbstractObjectStructureModelItem&)> SignalObjectStructureItemRemoved;
	CDrxSignal<void(CAbstractObjectStructureModelItem&)> SignalObjectStructureItemInvalidated;
};

inline u32 CAbstractObjectStructureModel::GetNumItems() const
{
	return GetGraphItemCount() + GetStateMachineItemCount();
}

inline CAbstractObjectStructureModelItem* CAbstractObjectStructureModel::GetChildItemByIndex(u32 index) const
{
	u32 row = index;
	if (row < GetGraphItemCount())
	{
		return static_cast<CAbstractObjectStructureModelItem*>(GetGraphItemByIndex(row));
	}
	else if (row -= GetGraphItemCount(), row < GetStateMachineItemCount())
	{
		return static_cast<CAbstractObjectStructureModelItem*>(GetStateMachineItemByIndex(row));
	}

	return nullptr;
}

inline u32 CAbstractObjectStructureModel::GetChildItemIndex(const CAbstractObjectStructureModelItem& item) const
{
	switch (item.GetType())
	{
	case eObjectItemType_Graph:
		{
			u32k e = GetGraphItemCount();
			for (u32 i = 0; i < e; ++i)
			{
				const CGraphItem* pItem = GetGraphItemByIndex(i);
				if (pItem == &item)
					return i;
			}
		}
		break;
	case eObjectItemType_StateMachine:
		{
			u32k e = GetStateMachineItemCount();
			for (u32 i = 0; i < e; ++i)
			{
				const CStateMachineItem* pItem = GetStateMachineItemByIndex(i);
				if (pItem == &item)
					return i + GetGraphItemCount();
			}
		}
		break;
	default:
		break;
	}

	return 0xffffffff;
}

inline bool CAbstractObjectStructureModel::RemoveItem(CAbstractObjectStructureModelItem& item)
{
	switch (item.GetType())
	{
	case eObjectItemType_Graph:
		return RemoveGraph(static_cast<CGraphItem&>(item));
	case eObjectItemType_StateMachine:
		return RemoveStateMachine(static_cast<CStateMachineItem&>(item));
	default:
		break;
	}

	return false;
}

}

