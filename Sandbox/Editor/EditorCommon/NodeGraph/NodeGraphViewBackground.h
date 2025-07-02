// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QGraphicsWidget>

namespace DrxGraphEditor {

class CNodeGraphViewStyle;

// TODO: We should use background rendering of the QGraphicsView!
class CNodeGraphViewBackground : public QGraphicsWidget
{
	Q_OBJECT

public:
	CNodeGraphViewBackground(QGraphicsItem* pParent = nullptr);

	void SetStyle(const CNodeGraphViewStyle* pStyle) { m_pStyle = pStyle; }

Q_SIGNALS:
	void OnMouseRightReleasedEvent();

protected:
	virtual void paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget = nullptr) override;

	virtual void mousePressEvent(QGraphicsSceneMouseEvent* pEvent) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* pEvent) override;

private:
	void DrawGrid(QPainter* pPainter, QRectF rect, float gridSize) const;

	const CNodeGraphViewStyle* m_pStyle;

	i32                      m_offsetX;
	i32                      m_offsetY;
};
// ~TODO

}

