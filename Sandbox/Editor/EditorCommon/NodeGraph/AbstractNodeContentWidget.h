// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

// TODO: Replace when CConnectionPoint has it's own header.
#include "IDrxGraphEditor.h"
// ~TODO

#include <QGraphicsWidget>

class QGraphicsGridLayout;
class QGraphicsWidget;

namespace DrxGraphEditor {

class CNodeWidget;
class CNodeGraphView;

class CPinWidget;
typedef std::vector<CPinWidget*> PinWidgetArray;

class EDITOR_COMMON_API CAbstractNodeContentWidget : public QGraphicsWidget
{
	Q_OBJECT

	// TODO: Type system!

public:
	CAbstractNodeContentWidget(CNodeWidget& node, CNodeGraphView& view);
	virtual ~CAbstractNodeContentWidget();

	CNodeGraphView&       GetView() const                  { return m_view; }
	CNodeWidget&          GetNode() const                  { return m_node; }

	bool                  IsCollapsible() const            { return m_isCollapsible; }
	void                  SetCollapsible(bool collapsible) { m_isCollapsible = collapsible; }

	CPinWidget*           GetPinWidget(const CAbstractPinItem& pin) const;
	CConnectionPoint*     GetConnectionPoint(const CAbstractPinItem& pin) const;
	const PinWidgetArray& GetPinWidgets() const { return m_pins; }

	virtual void          DeleteLater();
	virtual void          OnLayoutChanged()                                              {}
	virtual void          OnItemInvalidated()                                            {}

	virtual void          OnInputEvent(CNodeWidget* pSender, SMouseInputEventArgs& args) {}

protected:
	void         OnNodeMoved();
	void         UpdateLayout(QGraphicsLayout* pLayout);

	virtual void updateGeometry() override;

protected:
	PinWidgetArray m_pins;

private:
	CNodeWidget&    m_node;
	CNodeGraphView& m_view;
	bool            m_isCollapsible;
};

}

