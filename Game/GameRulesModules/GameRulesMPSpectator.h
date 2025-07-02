// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: 

-------------------------------------------------------------------------
История:
	- 11:09:2009  : Created by Thomas Houghton

*************************************************************************/

#ifndef __GAMERULESMPSPECTATOR_H__
#define __GAMERULESMPSPECTATOR_H__

#include <drx3D/Game/GameRulesModules/IGameRulesSpectatorModule.h>

class CGameRulesMPSpectator : public IGameRulesSpectatorModule
{
private:

	TChannelSpectatorModeMap  m_channelSpectatorModes;

	TSpawnLocations  m_spectatorLocations;

	CGameRules*  m_pGameRules;
	IGameFramework*  m_pGameFramework;
	IGameplayRecorder*  m_pGameplayRecorder;
	IActorSystem*  m_pActorSys; 

	u8  m_eatsActorActions	:1;
	u8  m_enableFree				:1;
	u8  m_enableFollow			:1;
	u8  m_enableFollowWhenNoLivesLeft : 1;
	u8  m_enableKiller			:1;
	u8  m_serverAlwaysAllowsSpectatorModeChange : 1;

	float  m_timeFromDeathTillAutoSpectate;

	struct SClientRequestInfo
	{
		SClientRequestInfo()	{	Reset();	}
		void Reset()
		{
			m_fRequestTimer = 0.0f;
			m_targetId			= 0;
			m_mode					= 0xFF;
		}

		float			m_fRequestTimer;
		EntityId	m_targetId;
		u8			m_mode;
	} m_ClientInfo;

public:

	CGameRulesMPSpectator();
	virtual ~CGameRulesMPSpectator();

	virtual void Init(XmlNodeRef xml);
	virtual void PostInit();
	virtual void Update(float frameTime);

	virtual bool ModeIsEnabledForPlayer(i32k mode, const EntityId playerId) const;
	virtual bool ModeIsAvailable(const EntityId playerId, i32k mode, EntityId* outOthEnt);
	virtual i32 GetBestAvailableMode(const EntityId playerId, EntityId* outOthEnt);
	virtual i32 GetNextMode(EntityId playerId, i32 inc, EntityId* outOthEid);

	virtual void SvOnChangeSpectatorMode(EntityId playerId, i32 mode, EntityId targetId, bool resetAll);
	virtual void SvRequestSpectatorTarget( EntityId playerId, i32 change );

	virtual void ClOnChangeSpectatorMode(EntityId playerId, i32 mode, EntityId targetId, bool resetAll);

	virtual bool GetModeFromChannelSpectatorMap(i32 channelId, i32* outMode) const;
	virtual void FindAndRemoveFromChannelSpectatorMap(i32 channelId);

	virtual EntityId GetNextSpectatorTarget(EntityId playerId, i32 change );
	virtual bool CanChangeSpectatorMode(EntityId playerId) const;
	virtual tukk GetActorSpectatorModeName(u8 mode);

	virtual void ChangeSpectatorMode(const IActor* pActor, u8 mode, EntityId targetId, bool resetAll, bool force = false);
	virtual void ChangeSpectatorModeBestAvailable(const IActor *pActor, bool resetAll);

	virtual void AddSpectatorLocation(EntityId location);
	virtual void RemoveSpectatorLocation(EntityId id);
	virtual i32 GetSpectatorLocationCount() const;
	virtual EntityId GetSpectatorLocation(i32 idx) const;
	virtual void GetSpectatorLocations(TSpawnLocations &locations) const;
	virtual EntityId GetRandomSpectatorLocation() const;
	virtual EntityId GetInterestingSpectatorLocation() const;
	virtual EntityId GetNextSpectatorLocation(const CActor* pPlayer) const;
	virtual EntityId GetPrevSpectatorLocation(const CActor* pPlayer) const;

	virtual float GetTimeFromDeathTillAutoSpectate() const { return m_timeFromDeathTillAutoSpectate; }
	virtual bool GetServerAlwaysAllowsSpectatorModeChange() const { return m_serverAlwaysAllowsSpectatorModeChange; }

	virtual void GetPostDeathDisplayDurations( EntityId playerId, i32 teamWhenKilled, bool isSuicide, bool isBulletTimeKill, float& rDeathCam, float& rKillerCam, float& rKillCam ) const;
};


#endif // __GAMERULESMPSPECTATOR_H__
