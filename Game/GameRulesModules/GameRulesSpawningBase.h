// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: 

-------------------------------------------------------------------------
История:
- 30:09:2009 : Created by James Bamford

*************************************************************************/

#ifndef __GAMERULES_SPAWNING_BASE_H__
#define __GAMERULES_SPAWNING_BASE_H__

#include <drx3D/Game/GameRulesModules/IGameRulesSpawningModule.h>
#include <drx3D/Game/GameRulesTypes.h>

class CGameRules;


class CGameRulesSpawningBase : public IGameRulesSpawningModule
{
private:
	typedef IGameRulesSpawningModule inherited;

protected:
	typedef std::vector<EntityId>								TSpawnLocations;

	struct SSpawnGroup
	{
		TSpawnLocations m_locations;
		u32 m_id;
		u8 m_useCount;
	};
	typedef std::vector<SSpawnGroup>						TSpawnGroups;

	static u32k kInvalidInitialSpawnIndex = ~0;

	TSpawnLocations			m_allSpawnLocations;
	TSpawnLocations			m_spawnLocations;
	TSpawnGroups				m_initialSpawnLocations;
	TPlayerDataMap			m_playerValues;
	CGameRules*					m_pGameRules;
	u32							m_activeInitialSpawnGroupIndex;
	bool								m_bTeamAlwaysUsesInitialSpawns[2];

public:
	CGameRulesSpawningBase();
	virtual ~CGameRulesSpawningBase();

	virtual void SetLastSpawn(EntityId playerId, EntityId spawnId);

	virtual void Init(XmlNodeRef xml);
	virtual void PostInit();
	virtual void Update(float frameTime);

	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags);

	virtual void AddSpawnLocation(EntityId location, bool isInitialSpawn, bool doVisTest, tukk pGroupName);
	virtual void RemoveSpawnLocation(EntityId id, bool isInitialSpawn);
	virtual void EnableSpawnLocation(EntityId location, bool isInitialSpawn, tukk pGroupName);
	virtual void DisableSpawnLocation(EntityId id, bool isInitialSpawn);

	virtual void SetInitialSpawnGroup(tukk groupName);
	
	bool	HasInitialSpawns() const { return !m_initialSpawnLocations.empty() && m_activeInitialSpawnGroupIndex != kInvalidInitialSpawnIndex; }
	const TEntityIdVec& GetInitialSpawns() const { return m_initialSpawnLocations[m_activeInitialSpawnGroupIndex].m_locations; }

	
	virtual EntityId GetSpawnLocation(EntityId playerId);
	virtual EntityId GetFirstSpawnLocation(i32 teamId) const;
	virtual i32 GetSpawnLocationCount() const;
	virtual EntityId GetNthSpawnLocation(i32 idx) const;
	virtual i32 GetSpawnIndexForEntityId(EntityId spawnId) const;

	virtual void AddAvoidPOI(EntityId entityId, float avoidDistance, bool enabled, bool bStaticPOI);
	virtual void RemovePOI(EntityId entityId);
	virtual void EnablePOI(EntityId entityId);
	virtual void DisablePOI(EntityId entityId);

	virtual void PlayerJoined(EntityId playerId);
	virtual void PlayerLeft(EntityId playerId);
	virtual void OnPlayerKilled(const HitInfo &hitInfo);

	virtual void ClRequestRevive(EntityId playerId);
	virtual bool SvRequestRevive(EntityId playerId, EntityId preferredSpawnId = 0);
	virtual void PerformRevive(EntityId playerId, i32 teamId, EntityId preferredSpawnId);
	virtual void OnSetTeam(EntityId playerId, i32 teamId) {}

	virtual const TPlayerDataMap* GetPlayerValuesMap(void) const { return &m_playerValues; }

	virtual void ReviveAllPlayers(bool isReset, bool bOnlyIfDead);
	
	virtual i32  GetRemainingLives(EntityId playerId) { return 0; }
	virtual i32  GetNumLives()  { return 0; }

	virtual ILINE float GetTimeFromDeathTillAutoReviveForTeam(i32 teamId) const { return -1.f; }
	virtual float GetPlayerAutoReviveAdditionalTime(IActor* pActor) const { return 0.f; }; 
	virtual float GetAutoReviveTimeScaleForTeam(i32 teamId) const { return 1.f; }
	virtual void SetAutoReviveTimeScaleForTeam(i32 teamId, float newScale) { }

	virtual void HostMigrationInsertIntoReviveQueue(EntityId playerId, float timeTillRevive) {}

	virtual void OnInGameBegin() {}
	virtual void OnInitialEquipmentScreenShown() {}
	virtual void OnNewRoundEquipmentScreenShown() {}
	virtual float GetRemainingInitialAutoSpawnTimer() { return -1.f; }
	virtual bool SvIsMidRoundJoiningAllowed() const { return true; };
	virtual bool CanPlayerSpawnThisRound(const EntityId playerId) const;
	virtual bool IsInInitialChannelsList(u32 channelId) const { return true; }

	virtual void HostMigrationStopAddingPlayers() {};
	virtual void HostMigrationResumeAddingPlayers() {};

protected:
	ILINE float GetTime() const { return gEnv->pTimer->GetFrameStartTime().GetMilliSeconds(); }

	bool IsPlayerInitialSpawning(const EntityId playerId) const;
	TSpawnLocations& GetSpawnLocations(const EntityId playerId);
	static TSpawnLocations* GetSpawnLocationsFromGroup(TSpawnGroups &groups, tukk pGroupName);

	void SelectNewInitialSpawnGroup();

	i32 m_spawnGroupOverride; 
};



#endif // __GAMERULES_SPAWNING_BASE_H__
