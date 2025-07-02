// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "AbstractNodeGraphViewModelItem.h"

#include "NodeGraphItemPropertiesWidget.h"

namespace DrxGraphEditor {

CAbstractNodeGraphViewModelItem::CAbstractNodeGraphViewModelItem(CNodeGraphViewModel& viewModel)
	: m_viewModel(viewModel)
	, m_propertiesPriority(0)
	, m_acceptsMoves(false)
	, m_acceptsSelection(false)
	, m_acceptsHighlightning(false)
	, m_acceptsDeactivation(false)
	, m_allowsMultiItemProperties(false)
	, m_acceptsDeletion(true)
	, m_acceptsCopy(true)
	, m_acceptsPaste(true)
{

}

CAbstractNodeGraphViewModelItem::~CAbstractNodeGraphViewModelItem()
{
	SignalDeletion(this);
}

}

