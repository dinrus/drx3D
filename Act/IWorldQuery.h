// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __IWORLDQUERY_H__
#define __IWORLDQUERY_H__

#pragma once

#include "IGameObject.h"

typedef std::vector<EntityId> Entities;

struct IWorldQuery : IGameObjectExtension
{
	virtual IEntity*                GetEntityInFrontOf() = 0;
	virtual const EntityId*         ProximityQuery(i32& numberOfEntities) = 0;
	virtual const Vec3&             GetPos() const = 0;
	virtual const Vec3&             GetDir() const = 0;
	virtual const EntityId          GetLookAtEntityId(bool ignoreGlass = false) = 0;
	virtual const ray_hit*          GetLookAtPoint(const float fMaxDist = 0, bool ignoreGlass = false) = 0;
	virtual const ray_hit*          GetBehindPoint(const float fMaxDist = 0) = 0;
	virtual const EntityId*         GetEntitiesAround(i32& num) = 0;
	virtual IPhysicalEntity* const* GetPhysicalEntitiesAround(i32& num) = 0;
	virtual IPhysicalEntity*        GetPhysicalEntityInFrontOf() = 0;
};

#endif
