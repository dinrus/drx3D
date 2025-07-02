// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QWidget>
#include <QPointer>
#include "QRollupBar.h"
#include "IToolWindowArea.h"

class QToolWindowManager;
class QLabel;

class QTOOLWINDOWMANAGER_EXPORT QToolWindowRollupBarArea : public QRollupBar, public IToolWindowArea
{
	Q_OBJECT;
	Q_INTERFACES(IToolWindowArea);

public:
	explicit QToolWindowRollupBarArea(QToolWindowManager* manager, QWidget *parent = 0);
	virtual ~QToolWindowRollupBarArea();

	void addToolWindow(QWidget* toolWindow, i32 index = -1) Q_DECL_OVERRIDE;
	void addToolWindows(const QList<QWidget*>& toolWindows, i32 index = -1) Q_DECL_OVERRIDE;

	void removeToolWindow(QWidget* toolWindow) Q_DECL_OVERRIDE;

	QList<QWidget*> toolWindows() Q_DECL_OVERRIDE;

	QVariantMap saveState() Q_DECL_OVERRIDE;
	void restoreState(const QVariantMap &data, i32 stateFormat) Q_DECL_OVERRIDE;
	void adjustDragVisuals() Q_DECL_OVERRIDE;

	QWidget* getWidget() Q_DECL_OVERRIDE { return this; }

	virtual bool switchAutoHide(bool newValue);

	const QPalette& palette() const Q_DECL_OVERRIDE { return QRollupBar::palette(); };
	void clear() Q_DECL_OVERRIDE { QRollupBar::clear(); };
	QRect rect() const Q_DECL_OVERRIDE { return QRollupBar::rect(); };
	QSize size() const Q_DECL_OVERRIDE { return QRollupBar::size(); };
	i32 count() const Q_DECL_OVERRIDE { return QRollupBar::count(); };
	QWidget* widget(i32 index) const Q_DECL_OVERRIDE { return QRollupBar::widget(index); };
	void deleteLater() Q_DECL_OVERRIDE { QRollupBar::deleteLater(); };
	i32 width() const Q_DECL_OVERRIDE { return QRollupBar::width(); };
	i32 height() const Q_DECL_OVERRIDE { return QRollupBar::height(); };
	const QRect geometry() const Q_DECL_OVERRIDE { return QRollupBar::geometry(); };
	void hide() Q_DECL_OVERRIDE { QRollupBar::hide(); };
	QObject* parent() const Q_DECL_OVERRIDE { return QRollupBar::parent(); };
	void setParent(QWidget* parent) Q_DECL_OVERRIDE { QRollupBar::setParent(parent); };
	i32 indexOf(QWidget* w) const Q_DECL_OVERRIDE;
	QWidget* parentWidget() const Q_DECL_OVERRIDE { return QRollupBar::parentWidget(); };
	QPoint mapFromGlobal(const QPoint & pos) const Q_DECL_OVERRIDE { return QRollupBar::mapFromGlobal(pos); };
	QPoint mapToGlobal(const QPoint & pos) const Q_DECL_OVERRIDE { return QRollupBar::mapToGlobal(pos); };
	void setCurrentWidget(QWidget* w) Q_DECL_OVERRIDE;

	QPoint mapCombineDropAreaFromGlobal(const QPoint & pos) const Q_DECL_OVERRIDE;
	QRect combineAreaRect() const Q_DECL_OVERRIDE;
	QRect combineSubWidgetRect(i32 index) const Q_DECL_OVERRIDE;
	i32 subWidgetAt(const QPoint& pos) const Q_DECL_OVERRIDE;
	virtual QTWMWrapperAreaType areaType() const { return QTWMWrapperAreaType::watRollups; };
protected:
	virtual bool eventFilter(QObject *o, QEvent *ev) Q_DECL_OVERRIDE;
protected Q_SLOTS:
	void closeRollup(i32 index);
	void mouseReleaseEvent(QMouseEvent * e);
	void swapToRollup();
private:
	void setDraggable(bool draggable);
	QPointer<QToolWindowManager> m_manager;
	QLabel* m_pTopWidget;
	QPoint m_areaDragStart;
	bool m_areaDraggable;
	bool m_tabDragCanStart;
	bool m_areaDragCanStart;
};

