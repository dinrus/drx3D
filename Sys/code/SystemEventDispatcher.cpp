// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/SystemEventDispatcher.h>

CSystemEventDispatcher::CSystemEventDispatcher()
	:
	m_listeners(0)
{
}

bool CSystemEventDispatcher::RegisterListener(ISystemEventListener* pListener, tukk szName)
{
	if (!pListener || !szName || !strlen(szName) || szName[0] == '\0')
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR_DBGBRK, "CSystemEventDispatcher::RegisterListener(ISystemEventListener* pListener, tukk szName) => Was called with Null listener or invalid szName! The Listner was not registered!");
		return false;
	}

	m_listenerRegistrationLock.Lock();
	bool ret = m_listeners.Add(pListener, szName);
	m_listenerRegistrationLock.Unlock();
	return ret;
}

bool CSystemEventDispatcher::RemoveListener(ISystemEventListener* pListener)
{
	if (!pListener)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR_DBGBRK, "CSystemEventDispatcher::RemoveListener(ISystemEventListener* pListener) => Was called with Null listener!");
	}

	m_listenerRegistrationLock.Lock();
	m_listeners.Remove(pListener);
	m_listenerRegistrationLock.Unlock();
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CSystemEventDispatcher::OnSystemEventAnyThread(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	m_listenerRegistrationLock.Lock();
	for (TSystemEventListeners::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
	{
		notifier->OnSystemEventAnyThread(event, wparam, lparam);
	}
	m_listenerRegistrationLock.Unlock();
}

//////////////////////////////////////////////////////////////////////////
void CSystemEventDispatcher::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam, bool force_queue /*= false*/)
{
	if (!force_queue && gEnv && gEnv->mMainThreadId == DrxGetCurrentThreadId())
	{
		for (TSystemEventListeners::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
		{
			notifier->OnSystemEvent(event, wparam, lparam);
		}
	}
	else
	{
		SEventParams params;
		params.event = event;
		params.wparam = wparam;
		params.lparam = lparam;
		m_systemEventQueue.push(params);
	}

	// Also dispatch the event on this thread. This technically means the event
	//	will be sent twice (thru different OnSystemEventXX functions), therefore it is up to listeners which one they react to.
	OnSystemEventAnyThread(event, wparam, lparam);
}

//////////////////////////////////////////////////////////////////////////
void CSystemEventDispatcher::Update()
{
	assert(gEnv && gEnv->mMainThreadId == DrxGetCurrentThreadId());

	SEventParams params;
	while (m_systemEventQueue.try_pop(params))
	{
		OnSystemEvent(params.event, params.wparam, params.lparam);
	}
}
