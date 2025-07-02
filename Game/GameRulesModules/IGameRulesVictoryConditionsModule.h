// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Interface for the game rule module to handle victory conditions
	-------------------------------------------------------------------------
	История:
	- 08:09:2009  : Created by Ben Johnson

*************************************************************************/

#ifndef _IGAME_RULES_VICTORY_CONDITIONS_MODULE_H_
#define _IGAME_RULES_VICTORY_CONDITIONS_MODULE_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/Game/GameRulesTypes.h>

class IGameRulesVictoryConditionsModule
{
public:
	virtual ~IGameRulesVictoryConditionsModule() {}

	virtual void	Init(XmlNodeRef xml) = 0;
	virtual void	Update(float frameTime) = 0;

	virtual void OnStartGame() = 0;
	virtual void OnRestart() = 0;
	virtual void SvOnEndGame() = 0;
	virtual void SvForceEndGame(i32 winnerTeam, EGameOverReason eGameOverReason) = 0;

	// TODO: Need a way to let the victory have different parameters.
	virtual void	ClVictoryTeam(i32 teamId, EGameOverReason reason, ESVC_DrawResolution drawWinningResolution, const SDrawResolutionData& level1, const SDrawResolutionData& level2, const EntityId killedEntity, const EntityId shooterEntity) = 0;
	virtual void	ClVictoryPlayer(i32 playerId, EGameOverReason reason, const EntityId killedEntity, const EntityId shooterEntity) = 0;

	// needed for extraction to tell its victory module whenever a tick is extracted
	virtual void  TeamCompletedAnObjective(i32 teamId) = 0;

	//For sound system when time limit has been updated mid game
	virtual void ClUpdatedTimeLimit() = 0;

	// Check if the score limit has been reached
	virtual bool  ScoreLimitReached() = 0;

	virtual void  OnHostMigrationPromoteToServer() = 0;

	virtual const SDrawResolutionData* GetDrawResolutionData(ESVC_DrawResolution resolution) = 0;

  virtual void  SetWinningKillVictimShooter(EntityId victim, EntityId shooter) = 0;

	virtual void AddIntsToDrawResolutionData(ESVC_DrawResolution inResolutionLevel, i32k inTeam1Data, i32k inTeam2Data) = 0;
	virtual void SetFloatsInDrawResolutionData(ESVC_DrawResolution inResolutionLevel, const float inTeam1Data, const float inTeam2Data) = 0;

	virtual void SendVictoryMessage(i32 channelId) = 0;
	virtual void OnNewPlayerJoined(i32 channelId) = 0;
};

#endif // _IGAME_RULES_VICTORY_CONDITIONS_MODULE_H_
