// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"

#include "NotificationCenterImpl.h"
#include "QtUtil.h"

#include <drx3D/Sys/IDrxLink.h>

SNotificationPreferences gNotificationPreferences;
REGISTER_PREFERENCES_PAGE_PTR(SNotificationPreferences, &gNotificationPreferences)

CNotificationCenter::CNotificationCenter(QObject* pParent /* = nullptr*/)
	: QObject(pParent)
{
}

CNotificationCenter::~CNotificationCenter()
{
	gEnv->pLog->RemoveCallback(this);

	for (auto i = 0; i < m_notifications.size(); ++i)
		delete m_notifications[i];
}

void CNotificationCenter::Init()
{
	// We just care about the init event to register a log callback once the editor is initialized
	GetIEditor()->RegisterNotifyListener(this);
}

void CNotificationCenter::OnEditorNotifyEvent(EEditorNotifyEvent ev)
{
	if (ev == eNotify_OnInit)
	{
		gEnv->pLog->AddCallback(this);
		// We can now unregister since all we cared about is being able to attach to the log
		GetIEditor()->UnregisterNotifyListener(this);
	}
}

i32 CNotificationCenter::ShowInfo(const QString& title, const QString& message)
{
	return AddNotification(Internal::CNotification::Info, title, message);
}

i32 CNotificationCenter::ShowWarning(const QString& title, const QString& message)
{
	return AddNotification(Internal::CNotification::Warning, title, message);
}

i32 CNotificationCenter::ShowCritical(const QString& title, const QString& message)
{
	return AddNotification(Internal::CNotification::Critical, title, message);
}

i32 CNotificationCenter::ShowProgress(const QString& title, const QString& message, bool bShowProgress /* = false*/)
{
	return AddNotification(new Internal::CProgressNotification(title, message, bShowProgress));
}

void CNotificationCenter::OnWriteToConsole(tukk szMessage, bool bNewLine)
{
	static tukk szWarning = "$6[Warning] ";
	static tukk szError = "$4[Error] ";
	static tukk szAssert = "$4[Assert]";

	QString message(szMessage);
	if (message.startsWith(szWarning, Qt::CaseInsensitive))
	{
		AddNotification(new Internal::CNotification(Internal::CNotification::Warning, "Warning", message.remove(szWarning), true));
	}
	else if (message.startsWith(szError, Qt::CaseInsensitive))
	{
		AddNotification(new Internal::CNotification(Internal::CNotification::Critical, "Error", message.remove(szError), true));
	}
	else if (message.startsWith(szAssert, Qt::CaseInsensitive))
	{
		AddNotification(new Internal::CNotification(Internal::CNotification::Critical, "Assert", message.remove(szAssert), true));
	}
}

void CNotificationCenter::OnWriteToFile(tukk szMessage, bool bNewLine)
{
	// do nothing
}

i32 CNotificationCenter::AddNotification(Internal::CNotification* pNotification)
{
	notificationsLock.lock();

	QString message(pNotification->GetMessage());
	QRegularExpressionMatch regExpMatch;
	QVector<QString> commands;
	if (message.contains(QRegularExpression("(crylink)[^\\s]+"), &regExpMatch))
	{
		message.remove(QRegularExpression("(crylink)[^\\s]+"));

		// for every command link
		for (const QString& commandLink : regExpMatch.capturedTexts())
		{
			DrxLinkService::CDrxLink link(commandLink.toLocal8Bit());
			for (auto query : link.GetQueries())
				commands.push_back(QtUtil::ToQString(query.second));
		}

		// Replace the message with our new message without the crylink
		pNotification->SetMessage(message);
	}

	pNotification->SetId(m_notifications.size());
	pNotification->SetDateTime(QDateTime::currentDateTime());

	// Add commands to notification
	for (const QString& command : commands)
		pNotification->AddCommand(command);

	// Add to notifications stack
	m_notifications.push_back(pNotification);
	NotificationAdded(pNotification->GetId());

	notificationsLock.unlock();

	return pNotification->GetId();
}

i32 CNotificationCenter::AddNotification(Internal::CNotification::Type type, const QString& title, const QString& message)
{
	return AddNotification(new Internal::CNotification(type, title, message));
}

