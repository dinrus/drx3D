// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 

	-------------------------------------------------------------------------
	История:
	- 26:10:2009  : Created by Colin Gulliver

*************************************************************************/

#ifndef _GameRulesRoundsModule_h_
#define _GameRulesRoundsModule_h_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Network/SerializeFwd.h>
#include <drx3D/Game/IGameObject.h>
#include <drx3D/Game/GameRulesTypes.h>

#ifndef _RELEASE
#define LOG_PRIMARY_ROUND(...)  { if (g_pGameCVars->g_logPrimaryRound == 1) DrxLog(__VA_ARGS__); }
#else
#define LOG_PRIMARY_ROUND(...)
#endif

class IGameRulesRoundsModule
{
public:
	enum ERoundEndHUDState
	{
		eREHS_Unknown,
		eREHS_HUDMessage,
		eREHS_Top3,
		eREHS_WinningKill,
	};

	virtual ~IGameRulesRoundsModule() {}

	virtual void Init(XmlNodeRef xml) = 0;
	virtual void PostInit() = 0;
	virtual void Update(float frameTime) = 0;

	virtual void OnStartGame() = 0;
	virtual void OnEnterSuddenDeath() = 0;

	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags ) = 0;

	virtual void OnLocalPlayerSpawned() = 0;
	virtual void OnEndGame(i32 teamId, EntityId playerId, EGameOverReason reason) = 0;
	virtual i32 GetRoundNumber() = 0;
	virtual i32 GetRoundsRemaining() const = 0;
	virtual void SetTreatCurrentRoundAsFinalRound(const bool treatAsFinal) = 0;

	virtual i32 GetPrimaryTeam() const = 0;

	virtual bool CanEnterSuddenDeath() const = 0;
	virtual bool IsInProgress() const	= 0;
	virtual bool IsInSuddenDeath() const = 0;
	virtual bool IsRestarting() const = 0;
	virtual bool IsGameOver() const = 0;
	virtual bool IsRestartingRound(i32 round) const = 0;
	virtual float GetTimeTillRoundStart() const = 0;

	virtual i32 GetPreviousRoundWinnerTeamId() const = 0;
	virtual i32k* GetPreviousRoundTeamScores(void) const = 0;
	virtual EGameOverReason GetPreviousRoundWinReason() const = 0;

	virtual ERoundEndHUDState GetRoundEndHUDState() const = 0;

	virtual void OnPromoteToServer() = 0;

#if USE_PC_PREMATCH
	virtual void OnPrematchStateEnded(bool isSkipped) = 0; 
#endif // #if USE_PC_PREMATCH

	virtual bool ShowKillcamAtEndOfRound() const = 0;

	virtual void AdjustTimers(CTimeValue adjustment) = 0;
};

#endif // _GameRulesRoundsModule_h_
