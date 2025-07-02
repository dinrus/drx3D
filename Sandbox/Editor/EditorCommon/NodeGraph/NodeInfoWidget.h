// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <QGraphicsWidget>

class QGraphicsLinearLayout;

namespace DrxGraphEditor {

class CNodeWidget;
class CNodeInfoWidgetStyle;
class CNodeGraphView;

class CNodeInfoWidget : public QGraphicsWidget
{
public:
	CNodeInfoWidget(CNodeWidget& nodeWidget);

	void Update();

protected:
	virtual void paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget) override;

private:
	const CNodeInfoWidgetStyle* m_pStyle;
	CNodeWidget&                m_nodeWidget;
	CNodeGraphView&             m_view;

	bool                        m_showError;
	bool                        m_showWarning;
};

}

