// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QObject>
#include <QGraphicsPixmapItem>

namespace DrxGraphEditor {

class CGraphicsSceneIcon : public QObject, public QGraphicsPixmapItem
{
	Q_OBJECT

public:
	CGraphicsSceneIcon(const QPixmap& pixmap);

Q_SIGNALS:
	void SignalHoverEnterEvent(CGraphicsSceneIcon& sender, QGraphicsSceneHoverEvent* pEvent);
	void SignalHoverMoveEvent(CGraphicsSceneIcon& sender, QGraphicsSceneHoverEvent* pEvent);
	void SignalHoverLeaveEvent(CGraphicsSceneIcon& sender, QGraphicsSceneHoverEvent* pEvent);

	void SignalMousePressEvent(CGraphicsSceneIcon& sender, QGraphicsSceneMouseEvent* pEvent);
	void SignalMouseReleaseEvent(CGraphicsSceneIcon& sender, QGraphicsSceneMouseEvent* pEvent);

protected:
	virtual void paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget) override;

	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* pEvent) override;
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* pEvent) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* pEvent) override;

	virtual void mousePressEvent(QGraphicsSceneMouseEvent* pEvent) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* pEvent) override;
};

}

