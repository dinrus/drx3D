// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Описание: Definitions for Lua-caching helpers used by Actors to avoid
				Lua accessing at game time
  
 -------------------------------------------------------------------------
  История:
  - 01:07:2010: Created by Kevin Kirst

*************************************************************************/

#ifndef __ACTORLUACACHE_H__
#define __ACTORLUACACHE_H__

#include <drx3D/Game/ActorDefinitions.h>
#include <drx3D/Game/AutoAimUpr.h>

#define MAKE_SHARED_PTR(cls) typedef _smart_ptr<cls> cls ## Ptr;

// Cached version of 'physicsParams' Lua table (per class instance)
struct SLuaCache_ActorPhysicsParams : public _reference_target_t
{
	SEntityPhysicalizeParams params;
	pe_player_dimensions playerDim;
	pe_player_dynamics playerDyn;
	bool bIsCached;

	SLuaCache_ActorPhysicsParams() : bIsCached(false) {}
	void GetMemoryUsage(IDrxSizer *s) const;
	bool CacheFromTable(SmartScriptTable pEntityTable, tukk szEntityClassName);
};
MAKE_SHARED_PTR(SLuaCache_ActorPhysicsParams);

// Cached version of 'gameParams' Lua table (per class instance)
struct SLuaCache_ActorGameParams : public _reference_target_t
{
	SActorGameParams gameParams;
	SAutoaimTargetRegisterParams autoAimParams;
	bool bIsCached;

	SLuaCache_ActorGameParams() : bIsCached(false) {}
	void GetMemoryUsage(IDrxSizer *s) const;
	bool CacheFromTable(SmartScriptTable pEntityTable);
};
MAKE_SHARED_PTR(SLuaCache_ActorGameParams);

// Cached version of Actor properties (per single instance)
struct SLuaCache_ActorProperties : public _reference_target_t
{
	SActorFileModelInfo fileModelInfo;
	float fPhysicMassMult;
	bool bIsCached;

	SLuaCache_ActorProperties() : bIsCached(false), fPhysicMassMult(1.0f) {}
	void GetMemoryUsage(IDrxSizer *s) const;
	bool CacheFromTable(SmartScriptTable pEntityTable, SmartScriptTable pProperties);
};
MAKE_SHARED_PTR(SLuaCache_ActorProperties);

#endif //__ACTORLUACACHE_H__
