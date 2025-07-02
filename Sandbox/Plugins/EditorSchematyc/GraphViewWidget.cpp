// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "GraphViewWidget.h"

#include "PropertiesWidget.h"
#include "GraphViewModel.h"

#include <QtUtil.h>

namespace DrxSchematycEditor {

CGraphViewWidget::CGraphViewWidget(CMainWindow& editor)
	: DrxGraphEditor::CNodeGraphView()
	, m_editor(editor)
{

}

CGraphViewWidget::~CGraphViewWidget()
{

}

QWidget* CGraphViewWidget::CreatePropertiesWidget(DrxGraphEditor::GraphItemSet& selectedItems)
{
	CPropertiesWidget* pPropertiesWidget = new CPropertiesWidget(selectedItems, &m_editor);

	if (CNodeGraphViewModel* pModel = static_cast<CNodeGraphViewModel*>(GetModel()))
	{
		QObject::connect(pPropertiesWidget, &CPropertiesWidget::SignalPropertyChanged, pModel, &CNodeGraphViewModel::Refresh);
	}
	return pPropertiesWidget;
}

}

