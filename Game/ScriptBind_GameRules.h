// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Script Binding for GameRules

-------------------------------------------------------------------------
История:
- 23:2:2006   18:30 : Created by Márcio Martins

*************************************************************************/
#ifndef __SCRIPTBIND_GAMERULES_H__
#define __SCRIPTBIND_GAMERULES_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>


class CGameRules;
class CActor;
struct IGameFramework;
struct ISystem;
struct HitInfo;


class CScriptBind_GameRules :
	public CScriptableBase
{
public:
	CScriptBind_GameRules(ISystem *pSystem, IGameFramework *pGameFramework);
	virtual ~CScriptBind_GameRules();

	virtual void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	void AttachTo(CGameRules *pGameRules);

	i32 IsServer(IFunctionHandler *pH);
	i32 IsClient(IFunctionHandler *pH);
	i32 IsMultiplayer(IFunctionHandler *pH);
	i32 CanCheat(IFunctionHandler *pH);

	i32 SpawnPlayer(IFunctionHandler *pH, i32 channelId, tukk name, tukk className, Vec3 pos, Vec3 angles);
	i32 Revive(IFunctionHandler *pH, ScriptHandle playerId);
	i32 RevivePlayer(IFunctionHandler *pH, ScriptHandle playerId, Vec3 pos, Vec3 angles, i32 teamId, bool clearInventory);
	i32 RevivePlayerInVehicle(IFunctionHandler *pH, ScriptHandle playerId, ScriptHandle vehicleId, i32 seatId, i32 teamId, bool clearInventory);
	i32 IsPlayer(IFunctionHandler *pH, ScriptHandle playerId);
	i32 IsProjectile(IFunctionHandler *pH, ScriptHandle entityId);

	i32 AddSpawnLocation(IFunctionHandler *pH, ScriptHandle entityId, bool isInitialSpawn, bool doVisTest, tukk pGroupName);
	i32 RemoveSpawnLocation(IFunctionHandler *pH, ScriptHandle id, bool isInitialSpawn);
	i32 EnableSpawnLocation(IFunctionHandler *pH, ScriptHandle entityId, bool isInitialSpawn, tukk pGroupName);
	i32 DisableSpawnLocation(IFunctionHandler *pH, ScriptHandle id, bool isInitialSpawn);
	i32 GetFirstSpawnLocation(IFunctionHandler *pH, i32 teamId);

	i32 AddSpawnGroup(IFunctionHandler *pH, ScriptHandle groupId);
	i32 AddSpawnLocationToSpawnGroup(IFunctionHandler *pH, ScriptHandle groupId, ScriptHandle location);
	i32 RemoveSpawnLocationFromSpawnGroup(IFunctionHandler *pH, ScriptHandle groupId, ScriptHandle location);
	i32 RemoveSpawnGroup(IFunctionHandler *pH, ScriptHandle groupId);
	i32 GetSpawnGroups(IFunctionHandler *pH);
	i32 IsSpawnGroup(IFunctionHandler *pH, ScriptHandle entityId);

	i32 SetPlayerSpawnGroup(IFunctionHandler *pH, ScriptHandle playerId, ScriptHandle groupId);

	i32 AddSpectatorLocation(IFunctionHandler *pH, ScriptHandle location);
	i32 RemoveSpectatorLocation(IFunctionHandler *pH, ScriptHandle id);
	
	i32 ServerExplosion(IFunctionHandler *pH, ScriptHandle shooterId, ScriptHandle weaponId, float dmg,
		Vec3 pos, Vec3 dir, float radius, float angle, float pressure, float holesize);
	i32 ServerHit(IFunctionHandler *pH, ScriptHandle targetId, ScriptHandle shooterId, ScriptHandle weaponId, float dmg, float radius, i32 materialId, i32 partId, i32 typeId);
	i32 ClientSelfHarm(IFunctionHandler *pH, float dmg, i32 materialId, i32 partId, i32 typeId, Vec3 dir);
	i32 ClientSelfHarmByEntity(IFunctionHandler *pH, ScriptHandle sourceEntity, float dmg, i32 materialId, i32 partId, i32 typeId, Vec3 dir);
	i32 ServerHarmVehicle(IFunctionHandler *pH, ScriptHandle vehicle, float dmg, i32 materialId, i32 typeId, Vec3 dir);

	i32 GetTeamName(IFunctionHandler *pH, i32 teamId);
	i32 GetTeamId(IFunctionHandler *pH, tukk teamName);

	i32 SetTeam(IFunctionHandler *pH, i32 teamId, ScriptHandle playerId);
	i32 ClientSetTeam(IFunctionHandler *pH, i32 teamId, ScriptHandle playerId);	// Specific usage case spawn points
	i32 GetTeam(IFunctionHandler *pH, ScriptHandle playerId);

	i32 ForbiddenAreaWarning(IFunctionHandler *pH, bool active, i32 timer, ScriptHandle targetId);

	i32 Announce(IFunctionHandler *pH, ScriptHandle playerId, tukk announcement, i32 context);

	i32 GetServerTime(IFunctionHandler *pH);

	i32	EndGame(IFunctionHandler *pH);
	i32 NextLevel(IFunctionHandler *pH);

	i32 GetHitMaterialId(IFunctionHandler *pH, tukk materialName);
	i32 GetHitTypeId(IFunctionHandler *pH, tukk type);
	i32 GetHitType(IFunctionHandler *pH, i32 id);
	i32 IsHitTypeIdMelee(IFunctionHandler *pH, i32 hitTypeId);

	i32 IsDemoMode(IFunctionHandler *pH);

  i32 DebugCollisionDamage(IFunctionHandler *pH);

	i32 SendDamageIndicator(IFunctionHandler* pH, ScriptHandle targetId, ScriptHandle shooterId, ScriptHandle weaponId, Vec3 dir, float damage, i32 projectileClassId, i32 hitTypeId);

	i32 EnteredGame(IFunctionHandler* pH);

	i32 Watch(IFunctionHandler *pH, tukk text);

	i32 DemiGodDeath(IFunctionHandler *pH);
	i32 GetPrimaryTeam(IFunctionHandler *pH);

	i32 AddForbiddenArea(IFunctionHandler *pH, ScriptHandle entityId);
	i32 RemoveForbiddenArea(IFunctionHandler *pH, ScriptHandle entityId);
	i32 MakeMovementVisibleToAI(IFunctionHandler *pH, tukk entityClass);
	i32 SendGameRulesObjectiveEntitySignal(IFunctionHandler *pH, ScriptHandle entityId, i32 signal);

	i32 ReRecordEntity(IFunctionHandler *pH, ScriptHandle entityId);
	
	i32 ShouldGiveLocalPlayerHitFeedback2DSound(IFunctionHandler *pH, float damage);
	i32 CanUsePowerStruggleNode(IFunctionHandler *pH, ScriptHandle userId, ScriptHandle entityId);

private:
	void RegisterGlobals();
	void RegisterMethods();

	CGameRules *GetGameRules(IFunctionHandler *pH);
	CActor *GetActor(EntityId id);

	void GetOptionalHitInfoParams(IFunctionHandler *pH, HitInfo &hitInfo);

	SmartScriptTable	m_players;
	SmartScriptTable	m_teamplayers;
	SmartScriptTable	m_spawnlocations;
	SmartScriptTable	m_spectatorlocations;
	SmartScriptTable	m_spawngroups;

	ISystem						*m_pSystem;
	IGameFramework		*m_pGameFW;
};


#endif //__SCRIPTBIND_GAMERULES_H__
