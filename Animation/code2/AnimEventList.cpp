// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/AnimEventList.h>

CAnimEventList::CAnimEventList()
{
}

CAnimEventList::~CAnimEventList()
{
}

u32 CAnimEventList::GetCount() const
{
	return m_animEvents.size();
}

const CAnimEventData& CAnimEventList::GetByIndex(u32 animEventIndex) const
{
	return m_animEvents[animEventIndex];
}

CAnimEventData& CAnimEventList::GetByIndex(u32 animEventIndex)
{
	return m_animEvents[animEventIndex];
}

void CAnimEventList::Append(const CAnimEventData& animEvent)
{
	m_animEvents.push_back(animEvent);
}

void CAnimEventList::Remove(u32 animEventIndex)
{
	m_animEvents.erase(animEventIndex);
}

void CAnimEventList::Clear()
{
	m_animEvents.clear();
}

size_t CAnimEventList::GetAllocSize() const
{
	size_t allocSize = 0;
	allocSize += m_animEvents.get_alloc_size();

	u32k animEventCount = m_animEvents.size();
	for (u32 i = 0; i < animEventCount; ++i)
	{
		allocSize += m_animEvents[i].GetAllocSize();
	}

	return allocSize;
}

void CAnimEventList::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(m_animEvents);
}
