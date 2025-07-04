// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __EVENT_DISTRIBUTOR_H__
#define __EVENT_DISTRIBUTOR_H__

template<class TEventReceiver, typename TEventType, typename TFlagType>
class CEventDistributor
{
public:
	CEventDistributor()
	{
#if !defined(_RELEASE)
		m_debugSendingEvents = false;
#endif //#if !defined(_RELEASE)
	}

	virtual ~CEventDistributor()
	{

	}

	void RegisterEvent(TEventReceiver* pReceiver, TEventType type)
	{
		TFlagType flag = EventTypeToFlag(type);
		RegisterEventFlags(pReceiver, flag);
	}

	void RegisterEvents(TEventReceiver* pReceiver, TEventType type, TEventType type1)
	{
		TFlagType flag = EventTypeToFlag(type);
		flag |= EventTypeToFlag(type1);
		RegisterEventFlags(pReceiver, flag);
	}

	void RegisterEvents(TEventReceiver* pReceiver, TEventType type, TEventType type1, TEventType type2)
	{
		TFlagType flag = EventTypeToFlag(type);
		flag |= EventTypeToFlag(type1);
		flag |= EventTypeToFlag(type2);
		RegisterEventFlags(pReceiver, flag);
	}

	void RegisterEvents(TEventReceiver* pReceiver, TEventType type, TEventType type1, TEventType type2, TEventType type3)
	{
		TFlagType flag = EventTypeToFlag(type);
		flag |= EventTypeToFlag(type1);
		flag |= EventTypeToFlag(type2);
		flag |= EventTypeToFlag(type3);
		RegisterEventFlags(pReceiver, flag);
	}

	void RegisterEvents(TEventReceiver* pReceiver, TEventType type, TEventType type1, TEventType type2, TEventType type3, TEventType type4)
	{
		TFlagType flag = EventTypeToFlag(type);
		flag |= EventTypeToFlag(type1);
		flag |= EventTypeToFlag(type2);
		flag |= EventTypeToFlag(type3);
		flag |= EventTypeToFlag(type4);
		RegisterEventFlags(pReceiver, flag);
	}

	void RegisterEvents(TEventReceiver* pReceiver, TEventType type, TEventType type1, TEventType type2, TEventType type3, TEventType type4, TEventType type5)
	{
		TFlagType flag = EventTypeToFlag(type);
		flag |= EventTypeToFlag(type1);
		flag |= EventTypeToFlag(type2);
		flag |= EventTypeToFlag(type3);
		flag |= EventTypeToFlag(type4);
		flag |= EventTypeToFlag(type5);
		RegisterEventFlags(pReceiver, flag);
	}

	void UnregisterEvent(TEventReceiver* pReceiver, TEventType type)
	{
		TFlagType flag = EventTypeToFlag(type);
		UnregisterEventFlags(pReceiver, flag);
	}

	void UnregisterEvents(TEventReceiver* pReceiver, TEventType type, TEventType type1)
	{
		TFlagType flag = EventTypeToFlag(type);
		flag |= EventTypeToFlag(type1);
		UnregisterEventFlags(pReceiver, flag);
	}

	void UnregisterEvents(TEventReceiver* pReceiver, TEventType type, TEventType type1, TEventType type2)
	{
		TFlagType flag = EventTypeToFlag(type);
		flag |= EventTypeToFlag(type1);
		flag |= EventTypeToFlag(type2);
		UnregisterEventFlags(pReceiver, flag);
	}

	void UnregisterEvents(TEventReceiver* pReceiver, TEventType type, TEventType type1, TEventType type2, TEventType type3)
	{
		TFlagType flag = EventTypeToFlag(type);
		flag |= EventTypeToFlag(type1);
		flag |= EventTypeToFlag(type2);
		flag |= EventTypeToFlag(type3);
		UnregisterEventFlags(pReceiver, flag);
	}

	void UnregisterEvents(TEventReceiver* pReceiver, TEventType type, TEventType type1, TEventType type2, TEventType type3, TEventType type4)
	{
		TFlagType flag = EventTypeToFlag(type);
		flag |= EventTypeToFlag(type1);
		flag |= EventTypeToFlag(type2);
		flag |= EventTypeToFlag(type3);
		flag |= EventTypeToFlag(type4);
		UnregisterEventFlags(pReceiver, flag);
	}

	//Removes all events for a receiver
	void UnregisterReceiver(TEventReceiver* pReceiver)
	{
		DRX_ASSERT(m_pReceivers.size() == m_flags.size());

		i32 index = FindIndex(pReceiver);
		if(index != -1)
		{
			UnregisterReceiverByIndex(index);
		}

		DRX_ASSERT(m_pReceivers.size() == m_flags.size());
	}

protected:
	void RegisterEventFlags(TEventReceiver* pReceiver, TFlagType flag)
	{
#if !defined(_RELEASE)
		DRX_ASSERT(m_debugSendingEvents == false);
#endif //#if !defined(_RELEASE)

		DRX_ASSERT(m_pReceivers.size() == m_flags.size());

		i32 index = FindOrCreateIndex(pReceiver);

		DRX_ASSERT(index >= 0 && index < (i32) m_pReceivers.size());
		m_flags[index] |= flag;

		DRX_ASSERT(m_pReceivers.size() == m_flags.size());
	}

	void UnregisterEventFlags(TEventReceiver* pReceiver, TFlagType flag)
	{
		DRX_ASSERT(m_pReceivers.size() == m_flags.size());

		i32 index = FindIndex(pReceiver);
		if(index != -1)
		{
			m_flags[index] &= ~flag;
			if(m_flags[index] == 0)
			{
				UnregisterReceiverByIndex(index);
			}
		}

		DRX_ASSERT(m_pReceivers.size() == m_flags.size());
	}

	void UnregisterReceiverByIndex(i32 index)
	{

#if !defined(_RELEASE)
		DRX_ASSERT(m_debugSendingEvents == false);
#endif //#if !defined(_RELEASE)

		DRX_ASSERT(index >=0 && index < (i32) m_flags.size());
		DRX_ASSERT(m_pReceivers.size() == m_flags.size());

		const size_t lastIndex = m_pReceivers.size() - 1;
		if(index != lastIndex)
		{
			DRX_ASSERT(index >= 0 && index < (i32) m_pReceivers.size());
			m_pReceivers[index] = m_pReceivers[lastIndex];
			m_flags[index] = m_flags[lastIndex];
		}

		m_pReceivers.pop_back();
		m_flags.pop_back();

		DRX_ASSERT(m_pReceivers.size() == m_flags.size());
	}

	ILINE TFlagType EventTypeToFlag(TEventType event) const
	{
		const TFlagType one = 1;
		DRX_ASSERT(event < sizeof(TFlagType) * 8);
		return (one << (event));
	}

	i32 FindOrCreateIndex(TEventReceiver* pReceiver)
	{
		i32 index = FindIndex(pReceiver);
		if(index == -1)
		{
			index = m_pReceivers.size();

			m_pReceivers.push_back(pReceiver);
			m_flags.push_back(0);

			DRX_ASSERT(m_pReceivers.size() == m_flags.size());
		}
		return index;
	}

	ILINE i32 FindIndex(TEventReceiver* pReceiver)
	{
		const size_t size = m_pReceivers.size();
		for(size_t i = 0; i < size; i++)
		{
			if(m_pReceivers[i] == pReceiver)
			{
				return i;
			}
		}

		return -1;
	}

	ILINE bool IsFlagSet(TFlagType flags, TEventType type) const
	{
		const TFlagType flag = EventTypeToFlag(type);
		return IsFlagSet(flags, flag);
	}

	ILINE bool IsFlagSet(TFlagType flags, TFlagType flag) const
	{
		return (flags & flag) == flag;
	}

	std::vector<TEventReceiver*> m_pReceivers;
	std::vector<TFlagType> m_flags;

#if !defined(_RELEASE)
	bool m_debugSendingEvents;
#endif
};

#endif //#ifndef __EVENT_DISTRIBUTOR_H__