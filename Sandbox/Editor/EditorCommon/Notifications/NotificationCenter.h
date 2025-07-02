// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QObject>

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

//! On creation, spawns a progress notification widget. the lifetime of the widget is tied to the lifetime
//! of this notification. Example usage:
//! class WorkerThread : public QThread
//! {
//! 	virtual void run() override
//! 	{
//! 		CProgressNotification notification("My Title", "My Message", true);
//! 		for (i32 i = 0; i < 101; ++i)
//! 		{
//! 			float progress = static_cast<float>(i) * 0.01f;
//! 			notification.SetProgress(progress);
//! 			msleep(20);
//! 		}
//! 	}
//! };
//! 
//! WorkerThread* pWorker = new WorkerThread();
//! pWorker->start();

class EDITOR_COMMON_API CProgressNotification : public QObject
{
public:
	CProgressNotification(const QString& title, const QString& message, bool bShowProgress = false);
	~CProgressNotification();

	// Set progress [0.0, 1.0]. Anything greater or equal to 1.0 will mark the task as complete
	void SetProgress(float value);

	void SetMessage(const QString& message);
public:
	i32 m_id;
};

//////////////////////////////////////////////////////////////////////////

//! NotificationCenter will log any warnings and errors when using DrxWarning.
//! It will also log any asserts (even if asserts are ignored)
//! User can also show any additional info with the provided functions

class EDITOR_COMMON_API INotificationCenter
{
public:
	virtual void Init() = 0;

	virtual i32  ShowInfo(const QString& title, const QString& message) = 0;
	virtual i32  ShowWarning(const QString& title, const QString& message) = 0;
	virtual i32  ShowCritical(const QString& title, const QString& message) = 0;
	virtual i32  ShowProgress(const QString& title, const QString& message, bool bShowProgress = false) = 0;
};

