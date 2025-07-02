// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/NodeGraphView.h>
#include "NodeGraph/NodeWidget.h"

class CAssetNodeBase;
class QPopupWidget;
class CDictionaryWidget;

namespace DrxGraphEditor
{

class CNodeGraphViewStyle;
class CNodeGraphViewBackground;
class CNodeGraphView;

}

class CGraphView : public DrxGraphEditor::CNodeGraphView
{
public:
	CGraphView(DrxGraphEditor::CNodeGraphViewModel* pViewModel);
protected:
	virtual bool PopulateNodeContextMenu(DrxGraphEditor::CAbstractNodeItem& node, QMenu& menu) override;
	virtual void ShowGraphContextMenu(QPointF screenPos) override;
	void         OnContextMenuEntryClicked(CAbstractDictionaryEntry& entry);
private:
	std::unique_ptr<QPopupWidget> m_pSearchPopup;
	CDictionaryWidget*            m_pSearchPopupContent;
};

class CAssetWidget : public DrxGraphEditor::CNodeWidget
{
public:
	CAssetWidget(CAssetNodeBase& item, DrxGraphEditor::CNodeGraphView& view);
protected:
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* pEvent) override;
};

