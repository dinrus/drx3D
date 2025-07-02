// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _CUSTOMEVENTMANAGER_H_
#define _CUSTOMEVENTMANAGER_H_

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Act/ICustomEvents.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>

///////////////////////////////////////////////////
// Upr implementation to associate custom events
///////////////////////////////////////////////////
class CCustomEventUpr : public ICustomEventUpr
{
public:
	CCustomEventUpr();
	virtual ~CCustomEventUpr();

public:
	// ICustomEventUpr
	virtual bool           RegisterEventListener(ICustomEventListener* pListener, const TCustomEventId eventId) override;
	virtual bool           UnregisterEventListener(ICustomEventListener* pListener, const TCustomEventId eventId) override;
	virtual bool           UnregisterEvent(TCustomEventId eventId) override;
	virtual void           Clear() override;
	virtual void           FireEvent(const TCustomEventId eventId, const TFlowInputData& customData) override;
	virtual TCustomEventId GetNextCustomEventId() override;
	// ~ICustomEventUpr

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		SIZER_COMPONENT_NAME(pSizer, "CustomEventUpr");

		pSizer->AddObject(this, sizeof(*this));
	}

private:
	typedef CListenerSet<ICustomEventListener*> TCustomEventListeners;

	struct SCustomEventData
	{
		SCustomEventData()
			: m_listeners(0)
		{}

		TCustomEventListeners m_listeners;
	};

	// Mapping of custom event id to listeners
	typedef std::map<TCustomEventId, SCustomEventData> TCustomEventsMap;
	TCustomEventsMap m_customEventsData;
	TCustomEventId   m_curHighestEventId;
};

#endif
