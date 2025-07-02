// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <QObject>
#include <QWidget>
#include <QPoint>
#include <QMap>

#include "DrxQtAPI.h"
#include "DrxQtCompatibility.h"

class QScrollableBox;
class QCollapsibleFrame;

class DRXQT_API QRollupBar : public QWidget
{
	Q_OBJECT

public:
	QRollupBar(QWidget *parent);
	~QRollupBar();
	i32 addWidget(QWidget* widget);
	void insertWidget(QWidget* widget, i32 index);
	void removeWidget(QWidget* widget);
	void removeAt(i32 index);
	virtual i32 indexOf(QWidget* widget) const;
	i32 count() const;
	void clear();
	bool isDragHandle(QWidget*);
	QWidget* getDragHandleAt(i32 index);
	QWidget* getDragHandle(QWidget*);
	QWidget* widget(i32 index) const;
	void SetRollupsClosable(bool);
	void SetRollupsReorderable(bool);
	bool RollupsReorderable() { return m_canReorder; }
protected:
	QWidget* getDropTarget() const;
	bool eventFilter(QObject * o, QEvent *e);
	QCollapsibleFrame* rollupAt(i32 index) const;
	QCollapsibleFrame* rollupAt(QPoint pos) const;
	i32 rollupIndexAt(QPoint pos) const;
Q_SIGNALS:
	void RollupCloseRequested(i32);

private Q_SLOTS:
	void OnRollupCloseRequested(QCollapsibleFrame*);

private:
	i32 attachNewWidget(QWidget* widget, i32 index=-1);

	QScrollableBox* m_pScrollBox;
	QPoint m_dragStartPosition;
	QWidget* m_pDragWidget;
	QMap<QWidget*, QCollapsibleFrame*> m_childRollups;
	QList<QCollapsibleFrame*> m_subFrames;
	QWidget* m_pDelimeter;
	i32 m_draggedId;
	bool m_DragInProgress;
	bool m_canReorder;
	bool m_rollupsClosable;
};

