// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "AbstractCommentItem.h"

namespace DrxGraphEditor {

CAbstractCommentItem::CAbstractCommentItem(CNodeGraphViewModel& viewModel)
	: CAbstractNodeGraphViewModelItem(viewModel)
{
}

CAbstractCommentItem::~CAbstractCommentItem()
{
	SetAccpetsMoves(true);
	SetAcceptsSelection(true);
	SetAcceptsHighlightning(true);
}

void CAbstractCommentItem::SetPosition(QPointF position)
{
	if (GetAcceptsMoves() && m_position != position)
	{
		m_position = position;
		SignalPositionChanged();
	}
}

}

