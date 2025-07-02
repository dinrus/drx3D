// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Script Binding for Game

-------------------------------------------------------------------------
История:
- 14:08:2006   11:30 : Created by AlexL
*************************************************************************/
#ifndef __SCRIPTBIND_GAME_H__
#define __SCRIPTBIND_GAME_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>

struct IGameFramework;
struct ISystem;

class CScriptBind_Game :
	public CScriptableBase
{
	enum EGameCacheResourceType
	{
		eGCRT_Texture = 0,
		eGCRT_TextureDeferredCubemap = 1,
		eGCRT_StaticObject = 2,
		eGCRT_Material = 3,
	};

public:
	CScriptBind_Game(ISystem *pSystem, IGameFramework *pGameFramework);
	virtual ~CScriptBind_Game();

	virtual void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

protected:
	i32 ShowMainMenu(IFunctionHandler *pH);
	i32 PauseGame(IFunctionHandler *pH, bool pause);
	i32 IsMountedWeaponUsableWithTarget(IFunctionHandler *pH);

	i32 IsPlayer(IFunctionHandler *pH, ScriptHandle entityId);
	i32 RegisterVTOL(IFunctionHandler *pH, ScriptHandle entityId);
	
	i32 AddTacticalEntity(IFunctionHandler *pH, ScriptHandle entityId, i32 type);
	i32 RemoveTacticalEntity(IFunctionHandler *pH, ScriptHandle entityId, i32 type);

	i32 RegisterWithAutoAimUpr(IFunctionHandler *pH, ScriptHandle entityId, float innerRadiusFactor, float outerRadiusFactor, float snapRadiusFactor);
	i32 UnregisterFromAutoAimUpr(IFunctionHandler *pH, ScriptHandle entityId);

	i32 OnAmmoCrateSpawned(IFunctionHandler *pH, bool providesFragGrenades);

	i32 CacheResource(IFunctionHandler *pH, tukk whoIsRequesting, tukk resourceName, i32 resourceType, i32 resourceFlags);
	i32 CacheActorClassResources(IFunctionHandler *pH, tukk actorEntityClassName);
	i32 CacheEntityArchetype(IFunctionHandler *pH, tukk archetypeName);
	i32 CacheBodyDamageProfile(IFunctionHandler *pH, tukk bodyDamageFileName, tukk bodyDamagePartsFileName);

	//Checkpoint System
	i32 SaveCheckpoint(IFunctionHandler *pH, ScriptHandle checkpointId, tukk fileName);
	i32	LoadCheckpoint(IFunctionHandler *pH, tukk fileName);
	i32 QuickLoad(IFunctionHandler *pH);

	i32 QueueDeferredKill(IFunctionHandler* pH, ScriptHandle entityId);

#ifndef _RELEASE
	i32 DebugDrawCylinder(IFunctionHandler *pH, float x, float y, float z, float radius, float height, i32 r, i32 g, i32 b, i32 a);
	i32 DebugDrawCone( IFunctionHandler *pH, float x, float y, float z, float radius, float height, i32 r, i32 g, i32 b, i32 a );
	i32 DebugDrawAABB(IFunctionHandler *pH, float x, float y, float z, float x2, float y2, float z2, i32 r, i32 g, i32 b, i32 a);

	i32 DebugDrawPersistanceDirection(IFunctionHandler *pH, float startX, float startY, float startZ, float dirX, float dirY, float dirZ, i32 r, i32 g, i32 b, float duration);
#endif

	//Environmental weapons
	i32 OnEnvironmentalWeaponHealthChanged( IFunctionHandler *pH, ScriptHandle entityId );

	i32 ResetEntity( IFunctionHandler *pH, ScriptHandle entityId );
	i32 SetDangerousRigidBodyDangerStatus(IFunctionHandler *pH, ScriptHandle entityId, bool isDangerous, ScriptHandle triggerPlayerId);

	i32 SendEventToGameObject( IFunctionHandler* pH, ScriptHandle entityId, tuk event );

	i32 LoadPrefabLibrary( IFunctionHandler* pH,tukk filename);
	i32 SpawnPrefab( IFunctionHandler* pH,ScriptHandle entityId, tukk libname, tukk prefabname,u32 seed,u32 nMaxSpawn);
	i32 MovePrefab( IFunctionHandler* pH,ScriptHandle entityId);
	i32 DeletePrefab( IFunctionHandler* pH,ScriptHandle entityId);
	i32 HidePrefab( IFunctionHandler* pH,ScriptHandle entityId,bool bHide);

	// Resource caching:
	i32 CacheEquipmentPack(IFunctionHandler* pH, tukk equipmentPackName);	

private:
	void RegisterGlobals();
	void RegisterMethods();

	ISystem						*m_pSystem;
	IGameFramework		*m_pGameFW;
};

#endif //__SCRIPTBIND_GAME_H__
