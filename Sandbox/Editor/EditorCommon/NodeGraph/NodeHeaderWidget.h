// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <QGraphicsWidget>

class QGraphicsLinearLayout;

namespace DrxGraphEditor {

class CNodeWidget;
class CNodeGraphView;
class CNodeNameWidget;
class CNodeHeaderIcon;
class CNodeHeaderWidgetStyle;

class CNodeHeader : public QGraphicsWidget
{
public:
	enum class EIconSlot
	{
		Left,
		Right,
	};

public:
	CNodeHeader(CNodeWidget& nodeWidget);

	QString GetName() const;
	void    SetName(QString name);
	void    EditName();

	void    AddIcon(CNodeHeaderIcon* pHeaderIcon, EIconSlot slot);
	void    SetNameWidth(i32 width);
	void    SetNameColor(QColor color);

	void    OnSelectionChanged(bool isSelected);

protected:
	virtual void paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget) override;

private:
	const CNodeHeaderWidgetStyle* m_pStyle;
	CNodeWidget&                  m_nodeWidget;
	CNodeGraphView&               m_view;
	QGraphicsLinearLayout*        m_pLayout;
	CNodeNameWidget*              m_pName;
};

}

