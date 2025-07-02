// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/NodeGraphView.h>

namespace DrxSchematycEditor
{
class CMainWindow;

class CGraphViewWidget : public DrxGraphEditor::CNodeGraphView
{
public:
	CGraphViewWidget(CMainWindow& editor);
	~CGraphViewWidget();

	// DrxGraphEditor::CNodeGraphView
	virtual QWidget* CreatePropertiesWidget(DrxGraphEditor::GraphItemSet& selectedItems) override;
	// ~DrxGraphEditor::CNodeGraphView

private:
	CMainWindow& m_editor;

};

}

