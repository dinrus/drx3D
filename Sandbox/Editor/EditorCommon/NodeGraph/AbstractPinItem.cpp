// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "AbstractPinItem.h"

#include "AbstractNodeItem.h"
#include "AbstractNodeGraphViewModel.h"

namespace DrxGraphEditor {

CAbstractPinItem::CAbstractPinItem(CNodeGraphViewModel& viewModel)
	: CAbstractNodeGraphViewModelItem(viewModel)
	, m_isDeactivated(false)
{
	SetAcceptsSelection(true);
	SetAcceptsHighlightning(true);
	SetAcceptsDeactivation(true);
}

CAbstractPinItem::~CAbstractPinItem()
{
	// TODO: Disconnect on destruction.
}

void CAbstractPinItem::AddConnection(CAbstractConnectionItem& connection)
{
	m_connections.insert(&connection);
}

void CAbstractPinItem::RemoveConnection(CAbstractConnectionItem& connection)
{
	m_connections.erase(&connection);
}

void CAbstractPinItem::Disconnect()
{
	for (CAbstractConnectionItem* pConnection : m_connections)
	{
		GetViewModel().RemoveConnection(*pConnection);
	}
	m_connections.clear();
}

void CAbstractPinItem::SetDeactivated(bool isDeactivated)
{
	if (GetAcceptsDeactivation() && m_isDeactivated != isDeactivated)
	{
		m_isDeactivated = isDeactivated;
		SignalDeactivatedChanged();
	}
}

u32 CAbstractPinItem::GetIndex() const
{
	return GetNodeItem().GetPinItemIndex(*this);
}

}

