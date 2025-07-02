// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QWidget>

class QVBoxLayout;
class CNotificationWidget;

//! Widget that lists all notification widgets that haven't been accepted by the user. Rather
//! than it being a QListView or QTreeView that ties into an AbstractItemModel, this actually
//! lists the actual notification pop up widgets

class CNotificationList : public QWidget
{
	Q_OBJECT
public:
	CNotificationList(QWidget* pParent = nullptr);
	~CNotificationList();

private:
	void AddNotification(i32 notificationId);
	bool CombineNotifications(i32 notificationId);
	void ClearAll();
	void OnContextMenu(const QPoint& point);

private:
	// If not -1, combine all incoming log notifications at the given index
	i32                           m_combineIdx;

	QVector<CNotificationWidget*> m_notifications;
	QVBoxLayout*                  m_pMainLayout;
	QVBoxLayout*                  m_pNotificationsLayout;
};

