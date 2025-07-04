// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

// Qt
#include <QObject>
#include <QMutex>

// DrxCommon
#include <drx3D/Sys/ILog.h>
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include "Internal/Notifications_Internal.h"
#include "EditorFramework/Preferences.h"
#include "NotificationCenter.h"

// Preferences
struct SNotificationPreferences : public SPreferencePage
{
	SNotificationPreferences()
		: SPreferencePage("Notifications", "Notifications/General")
		, m_maxNotificationCount(25)
		, m_combineNotifications(true)
		, m_allowPopUps(true)
	{
	}

	virtual bool Serialize(yasli::Archive& ar) override
	{
		ar.openBlock("general", "General");
		ar(m_allowPopUps, "allowPopUps", "Allow Pop-Ups");
		ar(m_combineNotifications, "combineNotifications", "Combine Notifications");
		ar(m_maxNotificationCount, "maxNotificationCount", m_combineNotifications ? "Max Notification Count" : "!Max Notification Count");
		ar.closeBlock();
		return true;
	}

	ADD_PREFERENCE_PAGE_PROPERTY(i32, maxNotificationCount, setMaxNotificationCount)
	ADD_PREFERENCE_PAGE_PROPERTY(bool, allowPopUps, setAllowPopUps)
	ADD_PREFERENCE_PAGE_PROPERTY(bool, combineNotifications, setCombineNotifications)
};

extern SNotificationPreferences gNotificationPreferences;

// Using Qt's signals and slots instead of Drxsignal because connections are, by default, executed on the thread
// where the receiver lives. This is done because we can only create UI in the main thread. For more info:
// http://doc.qt.io/qt-5/qt.html#ConnectionType-enum and http://doc.qt.io/qt-5/thread-basics.html

class EDITOR_COMMON_API CNotificationCenter : public QObject, public INotificationCenter, public IEditorNotifyListener, public ILogCallback
{
	Q_OBJECT
public:
	CNotificationCenter(QObject* pParent = nullptr);
	~CNotificationCenter();

	// Required to hook into the log after EditorCommon has been initialized
	virtual void                           Init() override;

	virtual void                           OnEditorNotifyEvent(EEditorNotifyEvent ev) override;

	virtual i32                            ShowInfo(const QString& title, const QString& message) override;
	virtual i32                            ShowWarning(const QString& title, const QString& message) override;
	virtual i32                            ShowCritical(const QString& title, const QString& message) override;
	virtual i32                            ShowProgress(const QString& title, const QString& message, bool bShowProgress = false) override;

	virtual void                           OnWriteToConsole(tukk szMessage, bool bNewLine) override;
	virtual void                           OnWriteToFile(tukk szMessage, bool bNewLine) override;

	std::vector<Internal::CNotification*>& GetNotifications()            { return m_notifications; }
	size_t                                 GetNotificationCount() const  { return m_notifications.size(); }
	Internal::CNotification*               GetNotification(i32 id) const { return id >= 0 && id < m_notifications.size() ? m_notifications[id] : nullptr; }

signals:
	void NotificationAdded(i32 id);

protected:
	i32 AddNotification(Internal::CNotification* notification);
	i32 AddNotification(Internal::CNotification::Type type, const QString& title, const QString& message);

protected:
	QMutex notificationsLock;
	std::vector<Internal::CNotification*> m_notifications;
};

