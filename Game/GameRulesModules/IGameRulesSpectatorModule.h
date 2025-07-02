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

#ifndef __IGAMERULESSPECTATORMODULE_H__
#define __IGAMERULESSPECTATORMODULE_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Act/IGameRulesSystem.h>

class CGameRules;
class CDrxAction;
class CGameplayRecorder;
struct IActor;
class CActor;
class CPlayer;

class IGameRulesSpectatorModule
{
	public:

	protected:

	typedef std::map<i32, i32>			TChannelSpectatorModeMap;
	typedef std::vector<EntityId>		TSpawnLocations;  // FIXME this is a duplicate of what's in GameRules, look at renaming to TSpecLocations or something?

	public:

	virtual ~IGameRulesSpectatorModule() {}

	virtual void Init(XmlNodeRef xml) = 0;
	virtual void PostInit() = 0;
	virtual void Update(float frameTime) = 0;

	virtual bool ModeIsEnabledForPlayer(i32k mode, const EntityId playerId) const = 0;
	virtual bool ModeIsAvailable(const EntityId playerId, i32k mode, EntityId* outOthEnt) = 0;
	virtual i32 GetBestAvailableMode(const EntityId playerId, EntityId* outOthEnt) = 0;
	virtual i32 GetNextMode(EntityId playerId, i32 inc, EntityId* outOthEid) = 0;

	virtual void SvOnChangeSpectatorMode(EntityId playerId, i32 mode, EntityId targetId, bool resetAll) = 0;
	virtual void SvRequestSpectatorTarget(EntityId playerId, i32 change) = 0;

	virtual void ClOnChangeSpectatorMode(EntityId playerId, i32 mode, EntityId targetId, bool resetAll) = 0;

	virtual bool GetModeFromChannelSpectatorMap(i32 channelId, i32* outMode) const = 0;
	virtual void FindAndRemoveFromChannelSpectatorMap(i32 channelId) = 0;

	virtual EntityId GetNextSpectatorTarget(EntityId playerId, i32 change ) = 0;
	virtual bool CanChangeSpectatorMode(EntityId playerId) const = 0;
	virtual tukk GetActorSpectatorModeName(u8 mode) = 0;
	virtual void ChangeSpectatorMode(const IActor* pActor, u8 mode, EntityId targetId, bool resetAll, bool force = false) = 0;
	virtual void ChangeSpectatorModeBestAvailable(const IActor *pActor, bool resetAll) = 0;

	virtual void AddSpectatorLocation(EntityId location) = 0;
	virtual void RemoveSpectatorLocation(EntityId id) = 0;
	virtual i32 GetSpectatorLocationCount() const = 0;
	virtual EntityId GetSpectatorLocation(i32 idx) const = 0;
	virtual void GetSpectatorLocations(TSpawnLocations &locations) const = 0;
	virtual EntityId GetRandomSpectatorLocation() const = 0;
	virtual EntityId GetInterestingSpectatorLocation() const = 0;
	virtual EntityId GetNextSpectatorLocation(const CActor* pPlayer) const = 0;
	virtual EntityId GetPrevSpectatorLocation(const CActor* pPlayer) const = 0;

	virtual float GetTimeFromDeathTillAutoSpectate() const = 0;
	virtual bool GetServerAlwaysAllowsSpectatorModeChange() const = 0; 

	virtual void GetPostDeathDisplayDurations( EntityId playerId, i32 teamWhenKilled, bool isSuicide, bool isBulletTimeKill, float& rDeathCam, float& rKillerCam, float& rKillCam ) const = 0;
};


#endif  // __IGAMERULESSPECTATORMODULE_H__
