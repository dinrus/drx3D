// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Movie/StdAfx.h>
#include <drx3D/Movie/TimeRangesTrack.h>

void CTimeRangesTrack::SerializeKey(STimeRangeKey& key, XmlNodeRef& keyNode, bool bLoading)
{
	if (bLoading)
	{
		key.m_speed = 1;
		keyNode->getAttr("speed", key.m_speed);
		keyNode->getAttr("loop", key.m_bLoop);
		keyNode->getAttr("start", key.m_startTime);
		keyNode->getAttr("end", key.m_endTime);
	}
	else
	{
		if (key.m_speed != 1)
		{
			keyNode->setAttr("speed", key.m_speed);
		}

		if (key.m_bLoop)
		{
			keyNode->setAttr("loop", key.m_bLoop);
		}

		if (key.m_startTime != 0.0f)
		{
			keyNode->setAttr("start", key.m_startTime);
		}

		if (key.m_endTime != 0.0f)
		{
			keyNode->setAttr("end", key.m_endTime);
		}
	}
}

i32 CTimeRangesTrack::GetActiveKeyIndexForTime(const SAnimTime time)
{
	u32k numKeys = m_keys.size();

	if (numKeys == 0 || m_keys[0].m_time > time)
	{
		return -1;
	}

	i32 lastFound = 0;

	for (u32 i = 0; i < numKeys; ++i)
	{
		STimeRangeKey& key = m_keys[i];

		if (key.m_time > time)
		{
			break;
		}
		else if (key.m_time <= time)
		{
			lastFound = i;
		}
	}

	return lastFound;
}
