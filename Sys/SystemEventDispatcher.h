// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SYSTEMEVENTDISPATCHER_H__
#define __SYSTEMEVENTDISPATCHER_H__

#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>

class CSystemEventDispatcher : public ISystemEventDispatcher
{
public:
	CSystemEventDispatcher();
	virtual ~CSystemEventDispatcher(){}

	// ISystemEventDispatcher
	virtual bool RegisterListener(ISystemEventListener* pListener, tukk szName);
	virtual bool RemoveListener(ISystemEventListener* pListener);

	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam, bool force_queue = false);
	virtual void Update();

	// ~ISystemEventDispatcher
private:
	void OnSystemEventAnyThread(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);

	typedef CListenerSet<ISystemEventListener*> TSystemEventListeners;
	TSystemEventListeners m_listeners;

	// for the events coming from other threads
	struct SEventParams
	{
		ESystemEvent event;
		UINT_PTR     wparam;
		UINT_PTR     lparam;
	};

	typedef DrxMT::queue<SEventParams> TSystemEventQueue;
	TSystemEventQueue  m_systemEventQueue;
	DrxCriticalSection m_listenerRegistrationLock;
};

#endif //__SYSTEMEVENTDISPATCHER_H__
