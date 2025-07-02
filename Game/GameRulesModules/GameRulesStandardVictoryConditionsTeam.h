// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Standard game rules module to handle victory conditions
	-------------------------------------------------------------------------
	История:
	- 08:09:2009  : Created by Ben Johnson

*************************************************************************/

#ifndef _GAME_RULES_STANDARD_VICTORY_CONDITIONS_TEAM_H_
#define _GAME_RULES_STANDARD_VICTORY_CONDITIONS_TEAM_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/IGameRulesVictoryConditionsModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesSurvivorCountListener.h>
#include <drx3D/Game/IGameRulesTeamChangedListener.h>
#include <drx3D/Game/Audio/AudioTypes.h>
#include <drx3D/Game/GameRules.h>

#define ESVC_SoundsList(f) \
	f(eSVC_StartGame)											/*0*/ \
	f(eSVC_3secsIn)												/*1*/	\
	f(eSVC_Halfway)												/*2*/ \
	f(eSVC_1min)													/*3*/ \
	f(eSVC_30secs)												/*4*/ \
	f(eSVC_15secs)												/*5*/ \
	f(eSVC_5secs)													/*6*/ \
	f(eSVC_3secs)													/*7*/ \
	f(eSVC_end)														/*8*/ 

enum ESV_Flags
{
	eSVCF_None																		= 0,
	eSVCF_CheckScore															= BIT(0),
	eSVCF_CheckTime																= BIT(1),
	eSVCF_CheckScoreAsTime												= BIT(2),
	eSVCF_WinOnPrimaryTeamDead										= BIT(3),
	eSVCF_WinOnSecondaryTeamDead									= BIT(4),
	eSVCF_DecideRoundWinnerUsingRoundPoints				= BIT(5),
	eSVCF_TreatScoreRoundVictoryAsGameVictory			= BIT(6),
	eSVCF_DecideGameWinnerWithTeamScore						= BIT(7),
	eSVCF_HasEverHadOpponents											= BIT(8),
	eSVCF_PlayedNearScoreLimit										= BIT(9),
	eSVCF_PlayHalfwaySound												= BIT(10),
	eSVCF_PlayHalfwaySoundAfterUpdatingTimeLimit	= BIT(11),
	eSVCF_HaveUpdatedTimeLimit										= BIT(12),
	eSVCF_PlayerSwitchTeams												= BIT(13),
	eSVCF_PlayTimeAnnouncements										= BIT(14),
};

typedef u16 TVictoryFlags;

class CGameRulesStandardVictoryConditionsTeam : public IGameRulesVictoryConditionsModule,
												public IGameRulesSurvivorCountListener,
												public IGameRulesTeamChangedListener
{
public:
	CGameRulesStandardVictoryConditionsTeam();
	virtual ~CGameRulesStandardVictoryConditionsTeam();

	// IGameRulesVictoryConditionsModule
	virtual void	Init(XmlNodeRef xml);
	virtual void	Update(float frameTime);

	virtual void	OnStartGame() {}
	virtual void	OnRestart();
	virtual void	SvOnEndGame();
	virtual void	SvForceEndGame(i32 winnerTeam, EGameOverReason eGameOverReason);

	virtual void	ClVictoryTeam(i32 teamId, EGameOverReason reason, ESVC_DrawResolution drawWinningResolution, const SDrawResolutionData& level1, const SDrawResolutionData& level2, const EntityId killedEntity, const EntityId shooterEntity);
	virtual void	ClVictoryPlayer(i32 playerId, EGameOverReason reason, const EntityId killedEntity, const EntityId shooterEntity) {};
	virtual void  TeamCompletedAnObjective(i32 teamId) {};
	virtual void	ClUpdatedTimeLimit();
	virtual bool  ScoreLimitReached();
  virtual void  SetWinningKillVictimShooter(EntityId victim, EntityId shooter) { m_winningVictim = victim; m_winningAttacker = shooter; }

	virtual void  OnHostMigrationPromoteToServer();

	const SDrawResolutionData* GetDrawResolutionData(ESVC_DrawResolution resolution) { return &m_drawResolutionData[resolution]; }

	virtual void SendVictoryMessage(i32 channelId);
	virtual void OnNewPlayerJoined(i32 channelId) {}
	// ~IGameRulesVictoryConditionsModule

	// IGameRulesSurvivorCountListener
	virtual void SvSurvivorCountRefresh(i32 count, const EntityId survivors[], i32 numKills);
	// ~IGameRulesSurvivorCountListener

	// IGameRulesTeamChangedListener
	virtual void OnChangedTeam(EntityId entityId, i32 oldTeamId, i32 newTeamId);
	// ~IGameRulesTeamChangedListener

	virtual void AddIntsToDrawResolutionData(ESVC_DrawResolution inResolutionLevel, i32k inTeam1Data, i32k inTeam2Data);
	virtual void SetFloatsInDrawResolutionData(ESVC_DrawResolution inResolutionLevel, const float inTeam1Data, const float inTeam2Data);

protected:
	// draw resolutions are considered in order, level 1 to level 3
	DrxFixedArray<SDrawResolutionData, ESVC_DrawResolution_num> m_drawResolutionData;

	ILINE bool AreAnyFlagsSet(TVictoryFlags flags)		{ return (m_VictoryConditionFlags & flags) != 0; }
	ILINE bool AreAllFlagsSet(TVictoryFlags flags)	{ return (m_VictoryConditionFlags & flags) == flags; }
	ILINE void SetFlags(TVictoryFlags flags)		{ m_VictoryConditionFlags |= flags;	}
	ILINE void ResetFlags(TVictoryFlags flags)		{ m_VictoryConditionFlags &= ~flags;	}

	struct STeamScoreResult
	{
		STeamScoreResult() : score(0), scoreTeamId(0) { };

		i32 score;
		i32 scoreTeamId;
	};

	void	GetMaxTeamScore(STeamScoreResult &result);
	void	GetMinTeamScore(STeamScoreResult &result);

	virtual bool SvGameStillHasOpponentPlayers();
	virtual void SvOpponentsCheckFailed();
	void  SvCheckOpponents();

	virtual void	CheckTimeLimit();
	virtual void  CheckScoreLimit();
	virtual void  CheckScoreAsTimeLimit();
	void	UpdateTimeLimitSounds();
	void  UpdateScoreLimitSounds();
	virtual void	OnEndGame(i32 teamId, EGameOverReason reason, ESVC_DrawResolution winningResolution=ESVC_DrawResolution_invalid, EntityId killedEntity=0, EntityId shooterEntity=0);
	virtual void TimeLimitExpired();
	i32k GetClientTeam(const EntityId clientId) const;
	const EGameOverType IsClientWinning(i32& highScore) const;
	const EGameOverType IsClientWinning(i32k clientTeam, i32& highScore) const;


	ESVC_DrawResolution GetDrawResolutionFromName(tukk inName);
	void InitDrawResolutions(XmlNodeRef xml);
	void AddToDrawResolutionData(ESVC_DrawResolution inResolutionLevel, const float inTeam1Data, const float inTeam2Data);

	void AddFloatsToDrawResolutionData(ESVC_DrawResolution inResolutionLevel, const float inTeam1Data, const float inTeam2Data);
	void MaxFloatsToDrawResolutionData(ESVC_DrawResolution inResolutionLevel, const float inTeam1Data, const float inTeam2Data);
	

	template <class T> bool IsTeam1Winning(const SDrawResolutionData::EWinningDataTest inDataTest, T team1Data, T team2Data);
	i32 TryToResolveDraw(ESVC_DrawResolution *outWinningResolution);
	void DebugDrawResolutions();

	void GetGameOverAnnouncement(const EGameOverType gameOverType, i32k clientScore, i32k opponentScore, EAnnouncementID& announcement);

	AUTOENUM_BUILDENUMWITHTYPE_WITHNUM(ESVC_Sounds, ESVC_SoundsList, eSVC_size);

	CGameRules *m_pGameRules;

	ESVC_Sounds m_currentSound;
	float m_currentSoundTime[eSVC_size];
	float m_fNextScoreLimitCheck;

	TVictoryFlags m_VictoryConditionFlags;
	uint	m_numUpdates;
	static i32k k_num_updates_actually_in_play=2;
  EntityId m_winningVictim;
  EntityId m_winningAttacker;

	CGameRules::VictoryTeamParams m_victoryParams;
};

#endif // _GAME_RULES_STANDARD_VICTORY_CONDITIONS_TEAM_H_
