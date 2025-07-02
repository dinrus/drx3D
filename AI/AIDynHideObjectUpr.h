// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   AIDynHideObjectUpr.h
   $Id$
   Описание: Provides to query hide points around entities which
   are flagged as AI hideable. The manage also caches the objects.

   -------------------------------------------------------------------------
   История:
   - 2007				: Created by Mikko Mononen
   - 2 Mar 2009	: Evgeny Adamenkov: Removed IRenderer

 *********************************************************************/

#ifndef _AIDYNHIDEOBJECTMANAGER_H_
#define _AIDYNHIDEOBJECTMANAGER_H_

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/CoreX/StlUtils.h>

struct SDynamicObjectHideSpot
{
	Vec3         pos, dir;
	EntityId     entityId;
	u32 nodeIndex;

	SDynamicObjectHideSpot(const Vec3& pos = ZERO, const Vec3& dir = ZERO, EntityId id = 0, u32 nodeIndex = 0) :
		pos(pos), dir(dir), entityId(id), nodeIndex(nodeIndex) {}
};

class CAIDynHideObjectUpr
{
public:
	CAIDynHideObjectUpr();

	void Reset();

	void GetHidePositionsWithinRange(std::vector<SDynamicObjectHideSpot>& hideSpots, const Vec3& pos, float radius,
	                                 IAISystem::tNavCapMask navCapMask, float passRadius, u32 lastNavNodeIndex = 0);

	bool ValidateHideSpotLocation(const Vec3& pos, const SAIBodyInfo& bi, EntityId objectEntId);

	void DebugDraw();

private:

	void         ResetCache();
	void         FreeCacheItem(i32 i);
	i32          GetNewCacheItem();
	u32 GetPositionHashFromEntity(IEntity* pEntity);
	void         InvalidateHideSpotLocation(const Vec3& pos, EntityId objectEntId);

	struct SCachedDynamicObject
	{
		EntityId                            id;
		u32                        positionHash;
		std::vector<SDynamicObjectHideSpot> spots;
		CTimeValue                          timeStamp;
	};

	typedef VectorMap<EntityId, u32> DynamicOHideObjectMap;

	DynamicOHideObjectMap             m_cachedObjects;
	std::vector<SCachedDynamicObject> m_cache;
	std::vector<i32>                  m_cacheFreeList;
};

#endif  // #ifndef _AIDYNHIDEOBJECTMANAGER_H_
