// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/EventDistributor.h>
#include <drx3D/Game/PlayerPlugin.h>

#ifndef __PLAYER_PLUGIN_EVENT_DISTRIBUTOR_H__
#define __PLAYER_PLUGIN_EVENT_DISTRIBUTOR_H__

class CPlayerPluginEventDistributor: public CEventDistributor<CPlayerPlugin, EPlayerPlugInEvent, uint64>
{
public:
	void HandleEvent(EPlayerPlugInEvent theEvent, uk  data)
	{
#if !defined(_RELEASE)
		const size_t previousSize = m_pReceivers.size();
		DRX_ASSERT(m_pReceivers.size() == m_flags.size());
		m_debugSendingEvents = true;	//base call will assert if register or unregister happens during handleEvent
#endif //#if !defined(_RELEASE)
		
		if( !m_flags.empty() )
		{
			PrefetchLine(&m_flags[0], 0);
			PrefetchLine(&m_pReceivers[0], 0);

			const size_t size = m_flags.size();

			PrefetchLine(&m_flags[size-1], 0);
			PrefetchLine(&m_pReceivers[size-1], 0);

			uint64 flag = EventTypeToFlag(theEvent);

			for(size_t i = 0; i < size; i++)
			{
				if(IsFlagSet(m_flags[i], flag))
				{
					m_pReceivers[i]->HandleEvent(theEvent, data);
				}
			}
		}

		//Assert if unregister (or register) happens during HandleEvent
#if !defined(_RELEASE)
		DRX_ASSERT(previousSize == m_pReceivers.size());
		DRX_ASSERT(m_pReceivers.size() == m_flags.size());
		m_debugSendingEvents = false;
#endif //#if !defined(_RELEASE)
	}
};

#endif //#ifndef __PLAYER_PLUGIN_EVENT_DISTRIBUTOR_H__