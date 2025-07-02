// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QToolButton>
#include "NotificationCenter.h"

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <drx3D/Sandbox/Editor/EditorCommon/QtViewPane.h>

class QVBoxLayout;
class CNotificationWidget;
class QPopupWidget;

//! Dockable for whole notification center to be used as a tool window

class EDITOR_COMMON_API CNotificationCenterDockable : public CDockableWidget
{
	Q_OBJECT
public:
	CNotificationCenterDockable(QWidget* pParent = nullptr);

	//////////////////////////////////////////////////////////
	// CDockableWidget implementation
	virtual tukk GetPaneTitle() const override { return "Notification Center"; }
	virtual QRect       GetPaneRect() override { return QRect(0, 0, 640, 400); }
	//////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////////

//! Notification Center widget that is registered in the tray area.

class EDITOR_COMMON_API CNotificationCenterTrayWidget : public QToolButton
{
	Q_PROPERTY(i32 notificationDistance READ GetNotificationDistance WRITE SetNotificationDistance DESIGNABLE true)
	Q_PROPERTY(i32 animationDuration READ GetAnimationDuration WRITE SetAnimationDuration DESIGNABLE true)
	Q_PROPERTY(i32 animationDistance READ GetAnimationDistance WRITE SetAnimationDistance DESIGNABLE true)
	Q_PROPERTY(i32 notificationMaxCount READ GetNotificationMaxCount WRITE SetNotificationMaxCount DESIGNABLE true)

	Q_OBJECT
public:

	CNotificationCenterTrayWidget(QWidget* pParent = nullptr);
	~CNotificationCenterTrayWidget();

protected:
	void OnNotificationAdded(i32 id);
	void OnClicked(bool bChecked);

	i32  GetNotificationDistance() const;
	i32  GetNotificationMaxCount() const;
	i32  GetAnimationDuration() const;
	i32  GetAnimationDistance() const;

	void SetNotificationDistance(i32 distance);
	void SetNotificationMaxCount(i32 maxCount);
	void SetAnimationDuration(i32 duration);
	void SetAnimationDistance(i32 distance);

	// QWidget
	virtual bool eventFilter(QObject* pWatched, QEvent* pEvent) override;

protected:
	QPopupWidget*                  m_pPopUpMenu;
	class CNotificationPopupManager* m_pPopUpManager;
	// Set to the maximum warning type emitted. Resets when acknowledged by user.
	i32                              m_notificationType;
};

