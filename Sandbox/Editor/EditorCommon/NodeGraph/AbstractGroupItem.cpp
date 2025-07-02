// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "AbstractGroupItem.h"

namespace DrxGraphEditor {

CAbstractGroupItem::CAbstractGroupItem(CNodeGraphViewModel& viewModel)
	: CAbstractNodeGraphViewModelItem(viewModel)
{
	SetAcceptsSelection(true);
	SetAcceptsHighlightning(true);
}

CAbstractGroupItem::~CAbstractGroupItem()
{

}

void CAbstractGroupItem::SetPosition(QPointF position)
{
	if (GetAcceptsMoves() && m_position != position)
	{
		m_position = position;
		SignalPositionChanged();
	}
}

}

