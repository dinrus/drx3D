// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "AbstractConnectionItem.h"

#include "NodeGraphViewStyle.h"

#include <QColor>

namespace DrxGraphEditor {

CAbstractConnectionItem::CAbstractConnectionItem(CNodeGraphViewModel& viewModel)
	: CAbstractNodeGraphViewModelItem(viewModel)
	, m_isDeactivated(false)
{
	SetAcceptsSelection(true);
	SetAcceptsHighlightning(true);
	SetAcceptsDeactivation(true);
}

CAbstractConnectionItem::~CAbstractConnectionItem()
{

}

void CAbstractConnectionItem::SetDeactivated(bool isDeactivated)
{
	if (GetAcceptsDeactivation() && m_isDeactivated != isDeactivated)
	{
		m_isDeactivated = isDeactivated;
		SignalDeactivatedChanged();
	}
}

}

