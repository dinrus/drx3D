// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "BroadcastManager.h"
#include "Events.h"

#include <QApplication>
#include <QEvent>
#include <QWidget>

CBroadcastManager::CBroadcastManager()
	: QObject(nullptr)
{
}

CBroadcastManager::~CBroadcastManager()
{
	DisconnectAll();
}

void CBroadcastManager::DisconnectAll()
{
	m_listeners.clear();
}

void CBroadcastManager::DisconnectById(uintptr_t id)
{
	for (auto& signal : m_listeners)
	{
		signal.second.DisconnectById(id);
	}
}

void CBroadcastManager::DisconnectById(const BroadcastEvent::EventType type, uintptr_t id)
{
	m_listeners[type].DisconnectById(id);
}

CBroadcastManager* CBroadcastManager::Get(QWidget* const pContextWidget)
{
	if (!pContextWidget)
		return nullptr;

	GetBroadcastManagerEvent getBroadcastManagerEvent;
	QApplication::sendEvent(pContextWidget, &getBroadcastManagerEvent);
	return getBroadcastManagerEvent.GetManager();
}

void CBroadcastManager::Broadcast(BroadcastEvent& event)
{
	const BroadcastEvent::EventType eventType = static_cast<BroadcastEvent::EventType>(event.type());
	m_listeners[eventType](event);
	m_listeners[BroadcastEvent::All](event);
}
