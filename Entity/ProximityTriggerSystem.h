// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/RadixSort.h>

class CEntityComponentTriggerBounds;

//////////////////////////////////////////////////////////////////////////
struct SProximityElement
{
	//////////////////////////////////////////////////////////////////////////
	EntityId                        id;
	AABB                            aabb;
	u32                          bActivated : 1;
	std::vector<SProximityElement*> inside;

	SProximityElement()
	{
		id = 0;
		bActivated = 0;
	}
	~SProximityElement()
	{
	}
	bool AddInside(SProximityElement* elem)
	{
		// Sorted add.
		return stl::binary_insert_unique(inside, elem);
	}
	bool RemoveInside(SProximityElement* elem)
	{
		// sorted remove.
		return stl::binary_erase(inside, elem);
	}
	bool IsInside(SProximityElement* elem)
	{
		return std::binary_search(inside.begin(), inside.end(), elem);
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const {}
	//////////////////////////////////////////////////////////////////////////
	// Custom new/delete for pool allocator.
	//////////////////////////////////////////////////////////////////////////
	ILINE uk operator new(size_t nSize);
	ILINE void  operator delete(uk ptr);

};

//////////////////////////////////////////////////////////////////////////
struct SProximityEvent
{
	bool               bEnter; // Enter/Leave.
	EntityId           entity;
	SProximityElement* pTrigger;
	void               GetMemoryUsage(IDrxSizer* pSizer) const {}
};

//////////////////////////////////////////////////////////////////////////
class CProximityTriggerSystem
{
public:
	CProximityTriggerSystem();
	~CProximityTriggerSystem();

	SProximityElement* CreateTrigger();
	void               RemoveTrigger(SProximityElement* pTrigger);
	void               MoveTrigger(SProximityElement* pTrigger, const AABB& aabb, bool invalidateCachedAABB = false);

	SProximityElement* CreateEntity(EntityId id);
	void               MoveEntity(SProximityElement* pEntity, const Vec3& pos);
	void               RemoveEntity(SProximityElement* pEntity, bool instantEvent = false);

	void               Update();
	void               Reset();
	void               BeginReset();

	void               GetMemoryUsage(IDrxSizer* pSizer) const;

private:
	void CheckIfLeavingTrigger(SProximityElement* pEntity);
	void ProcessOverlap(SProximityElement* pEntity, SProximityElement* pTrigger);
	void RemoveFromTriggers(SProximityElement* pEntity, bool instantEvent = false);
	void PurgeRemovedTriggers();
	void SortTriggers();
	void SendEvents();
	void SendEvent(EEntityEvent eventId, EntityId triggerId, EntityId entityId);

private:
	typedef std::vector<SProximityElement*> Elements;

	Elements                        m_triggers;
	Elements                        m_triggersToRemove;
	Elements                        m_entitiesToRemove;
	bool                            m_bTriggerMoved;
	bool                            m_bResetting;

	std::vector<SProximityElement*> m_entities;
	std::vector<AABB>               m_triggersAABB;

	std::vector<SProximityEvent>    m_events;

	RadixSort                       m_triggerSorter;
	RadixSort                       m_entitySorter;

	std::vector<float>              m_minPosList0;
	std::vector<float>              m_minPosList1;
	u32k*                   m_Sorted0;
	u32k*                   m_Sorted1;

public:
	typedef stl::PoolAllocatorNoMT<sizeof(SProximityElement)> ProximityElement_PoolAlloc;
	static ProximityElement_PoolAlloc* g_pProximityElement_PoolAlloc;
};

//////////////////////////////////////////////////////////////////////////
inline uk SProximityElement::operator new(size_t nSize)
{
	uk ptr = CProximityTriggerSystem::g_pProximityElement_PoolAlloc->Allocate();
	if (ptr)
		memset(ptr, 0, nSize); // Clear objects memory.
	return ptr;
}
//////////////////////////////////////////////////////////////////////////
inline void SProximityElement::operator delete(uk ptr)
{
	if (ptr)
		CProximityTriggerSystem::g_pProximityElement_PoolAlloc->Deallocate(ptr);
}
