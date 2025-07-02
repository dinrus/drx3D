// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>

#include <drx3D/Act/CustomEventsUpr.h>

///////////////////////////////////////////////////
// Upr implementation to associate custom events
///////////////////////////////////////////////////

//------------------------------------------------------------------------------------------------------------------------
CCustomEventUpr::CCustomEventUpr()
	: m_curHighestEventId(CUSTOMEVENTID_INVALID)
{
}

//------------------------------------------------------------------------------------------------------------------------
CCustomEventUpr::~CCustomEventUpr()
{
	Clear();
}

//------------------------------------------------------------------------------------------------------------------------
bool CCustomEventUpr::RegisterEventListener(ICustomEventListener* pListener, const TCustomEventId eventId)
{
	SCustomEventData& eventData = m_customEventsData[eventId];   // Creates new entry if doesn't exist
	TCustomEventListeners& listeners = eventData.m_listeners;
	return listeners.Add(pListener);
}

//------------------------------------------------------------------------------------------------------------------------
bool CCustomEventUpr::UnregisterEventListener(ICustomEventListener* pListener, const TCustomEventId eventId)
{
	TCustomEventsMap::iterator eventsDataIter = m_customEventsData.find(eventId);
	if (eventsDataIter != m_customEventsData.end())
	{
		SCustomEventData& eventData = eventsDataIter->second;
		TCustomEventListeners& listeners = eventData.m_listeners;

		if (listeners.Contains(pListener))
		{
			listeners.Remove(pListener);
			return true;
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CCustomEventUpr::UnregisterEventListener: Listener isn't registered for event id: %u", eventId);
		}
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CCustomEventUpr::UnregisterEventListener: No event data exists for event id: %u", eventId);
	}

	return false;
}

//------------------------------------------------------------------------------------------------------------------------
bool CCustomEventUpr::UnregisterEvent(TCustomEventId eventId)
{
	TCustomEventsMap::iterator eventsDataIter = m_customEventsData.find(eventId);
	if (eventsDataIter != m_customEventsData.end())
	{
		SCustomEventData& eventData = eventsDataIter->second;
		TCustomEventListeners& listeners = eventData.m_listeners;
		listeners.Clear(true);
		m_customEventsData.erase(eventsDataIter);
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------------------------------------------------
void CCustomEventUpr::Clear()
{
	m_customEventsData.clear();
	m_curHighestEventId = CUSTOMEVENTID_INVALID;
}

//------------------------------------------------------------------------------------------------------------------------
void CCustomEventUpr::FireEvent(const TCustomEventId eventId, const TFlowInputData& customData)
{
	TCustomEventsMap::iterator eventsDataIter = m_customEventsData.find(eventId);
	if (eventsDataIter != m_customEventsData.end())
	{
		SCustomEventData& eventData = eventsDataIter->second;
		TCustomEventListeners& listeners = eventData.m_listeners;
		listeners.ForEachListener([&](ICustomEventListener* pListener){ pListener->OnCustomEvent(eventId, customData); });
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CCustomEventUpr::FireEvent: No event data exists for event id: %u", eventId);
	}
}

//------------------------------------------------------------------------------------------------------------------------
TCustomEventId CCustomEventUpr::GetNextCustomEventId()
{
	const TCustomEventId curNextId = m_curHighestEventId + 1;
	m_curHighestEventId = curNextId;
	return curNextId;
}
