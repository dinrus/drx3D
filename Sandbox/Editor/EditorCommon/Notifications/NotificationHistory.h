// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QWidget>
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

class QAdvancedTreeView;
class CNotificationModel;
class CNotificationFilterModel;

//! Shows a complete history of all notifications (except for progress/tasks)

class CNotificationHistory : public QWidget
{
public:
	CNotificationHistory(QWidget* pParent = nullptr);

protected:
	void OnNotificationsAdded();
	void OnContextMenu(const QPoint& pos) const;
	void OnDoubleClicked(const QModelIndex& index);

	void LoadState();
	void SaveState();

protected:
	QAdvancedTreeView*        m_pTreeView;
	CNotificationFilterModel* m_pFilterModel;
	CNotificationModel*       m_pNotificationModel;
	QToolButton*              m_pShowInfo;
	QToolButton*              m_pShowWarnings;
	QToolButton*              m_pShowErrors;
};

