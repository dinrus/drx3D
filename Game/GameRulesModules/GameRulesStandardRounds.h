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

#ifndef _GameRulesStandardRounds_h_
#define _GameRulesStandardRounds_h_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/IGameRulesRoundsModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesTeamChangedListener.h>
#include <drx3D/CoreX/String/DrxFixedString.h>
#include <drx3D/Game/GameRulesTypes.h>
#include <drx3D/Game/IGameRulesStateModule.h>

class CGameRules;

class CGameRulesStandardRounds :	public IGameRulesRoundsModule,
									public IGameRulesTeamChangedListener,
									public IGameRulesStateListener
{
public:
	CGameRulesStandardRounds();
	virtual ~CGameRulesStandardRounds();

	// IGameRulesRoundsModule
	virtual void Init(XmlNodeRef xml);
	virtual void PostInit();
	virtual void Update(float frameTime);

	virtual void OnStartGame();
	virtual void OnEnterSuddenDeath();

	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags );

	virtual void OnLocalPlayerSpawned();
	virtual void OnEndGame(i32 teamId, EntityId playerId, EGameOverReason reason);
	virtual i32 GetRoundNumber();
	virtual i32 GetRoundsRemaining() const; // Returns 0 if currently on last round
	virtual void SetTreatCurrentRoundAsFinalRound(const bool treatAsFinal) { m_treatCurrentRoundAsFinalRound = treatAsFinal; }

	virtual i32 GetPrimaryTeam() const;

	virtual bool CanEnterSuddenDeath() const;
	virtual bool IsInProgress() const								{ return (m_roundState == eGRRS_InProgress || m_roundState == eGRRS_SuddenDeath); }
	virtual bool IsInSuddenDeath() const						{ return (m_roundState == eGRRS_SuddenDeath); }
	virtual bool IsRestarting() const								{ return (m_roundState == eGRRS_Restarting); }
	virtual bool IsRestartingRound(i32 round) const	{ return (round == eGRRS_Restarting); }
	virtual bool IsGameOver() const									{ return (m_roundState == eGRRS_GameOver); }

	virtual float GetTimeTillRoundStart() const;

	virtual i32 GetPreviousRoundWinnerTeamId() const { return m_previousRoundWinnerTeamId; }
	virtual i32k* GetPreviousRoundTeamScores(void) const { return m_previousRoundTeamScores; }
	virtual EGameOverReason GetPreviousRoundWinReason() const { return m_previousRoundWinReason; }

	virtual ERoundEndHUDState GetRoundEndHUDState() const { return m_roundEndHUDState; }

	virtual void OnPromoteToServer();
	
#if USE_PC_PREMATCH
	virtual void OnPrematchStateEnded(bool isSkipped); 
#endif // #if USE_PC_PREMATCH

	void ShowRoundStartingMessage(bool bPlayAudio);

	virtual bool ShowKillcamAtEndOfRound() const { return m_bShowKillcamAtEndOfRound; }
	virtual void AdjustTimers(CTimeValue adjustment);
	// ~IGameRulesRoundsModule

	// IGameRulesTeamChangedListener
	virtual void OnChangedTeam(EntityId entityId, i32 oldTeamId, i32 newTeamId);
	// ~IGameRulesTeamChangedListener

	// IGameRulesStateListener
	virtual void OnIntroStart() {}
	virtual void OnGameStart();
	virtual void OnGameEnd() {}
	virtual void OnStateEntered(IGameRulesStateModule::EGR_GameState newGameState)  {}
	// ~IGameRulesStateListener

	enum ERoundType
	{
		ERT_Any,
		ERT_Odd,
		ERT_Even,
	};

protected:
	typedef DrxFixedStringT<32> TFixedString;
	typedef std::vector<EntityId> TEntityIdVec;

	struct SEntityDetails
	{
		TEntityIdVec m_currentEntities;
		TFixedString m_activateFunc;
		TFixedString m_deactivateFunc;
		const IEntityClass *m_pEntityClass;
		i32 m_selectCount;
		bool m_doRandomSelection;
	};

	struct SOnEndRoundVictoryStrings
	{
		SOnEndRoundVictoryStrings()
		{
			m_reason = EGOR_Unknown;
			m_round = ERT_Any;
		}

		EGameOverReason m_reason;
		ERoundType m_round;

		TFixedString m_roundWinVictoryMessage;
		TFixedString m_roundLoseVictoryMessage;
	};

	struct SOnEndRoundStrings
	{
		SOnEndRoundStrings()
		{
			m_reason = EGOR_Unknown;
		}

		EGameOverReason m_reason;

		TFixedString m_roundWinMessage;
		TFixedString m_roundLoseMessage;
		TFixedString m_roundDrawMessage;
	};

	enum ERoundState
	{
		eGRRS_InProgress,		// Round is in progress
		eGRRS_Restarting,		// New round is about to start
		eGRRS_SuddenDeath,	// Sudden death mode
		eGRRS_Finished,			// All rounds finished (but still showing end of round scores)
		eGRRS_GameOver,			// Finished showing end of round scores
	};

	typedef std::vector<SEntityDetails> TEntityDetailsVec;

	static i32k MAX_END_OF_ROUND_STRINGS = 3;
	static i32k k_MaxEndOfRoundVictoryStrings = 4;

	void StartNewRound(bool isReset);
	void EndRound();
	void ReadEntityClasses(TEntityDetailsVec &classesVec, XmlNodeRef xml, TFixedString &startRoundString, bool &startRoundStringIsTeamMessage, TFixedString &startRoundStringExtra, bool &bShowTeamBanner, TFixedString &startRoundHeaderString, bool &bCustomHeader);
	void SetupEntities();
	void SetTeamEntities(TEntityDetailsVec &classesVec, i32 teamId);
	void GetTeamEntities(TEntityDetailsVec &classesVec, i32 teamId);
	void CallScript(EntityId entityId, tukk pFunction);
	void ActivateEntity(EntityId entityId, i32 teamId, tukk pActivateFunc, TEntityIdVec &pEntitiesVec);
	void DeactivateEntities(tukk pDeactivateFunc, TEntityIdVec &entitiesVec);
	void CheckForTeamEntity(IEntity *pEntity, i32 newTeamId, TEntityDetailsVec &entitiesVec);

	void DisplaySuddenDeathMessage();
	void ClDisplayEndOfRoundMessage();
	tukk ClGetVictoryMessage(i32k localTeam) const;
	const float GetVictoryMessageTime() const;
	const bool IsCurrentRoundType(const ERoundType roundType) const;

	bool IsFinished() const { return (m_roundState == eGRRS_Finished); }
	void SetRoundState(ERoundState inRoundState);
	void EnterRoundEndHUDState(ERoundEndHUDState state);
	void UpdateRoundEndHUD(float frameTime);

	void CalculateNewRoundTimes(float serverTime);
	void CalculateEndGameTime(float serverTime);

	void OnRoundEnd();
	void NewRoundTransition();

	static void CmdNextRound(IConsoleCmdArgs *pArgs);

	SOnEndRoundVictoryStrings m_endOfRoundVictoryStrings[k_MaxEndOfRoundVictoryStrings];
	SOnEndRoundStrings m_endOfRoundStrings[MAX_END_OF_ROUND_STRINGS];
	SOnEndRoundStrings m_endOfRoundStringsDefault;

	TEntityDetailsVec m_primaryTeamEntities;
	TEntityDetailsVec m_secondaryTeamEntities;

	// strings
	TFixedString m_primaryStartRoundString;
	TFixedString m_secondaryStartRoundString;
	TFixedString m_primaryStartRoundStringExtra;
	TFixedString m_secondaryStartRoundStringExtra;
	TFixedString m_primaryCustomHeader;
	TFixedString m_secondaryCustomHeader;


	TFixedString m_victoryMessage;
	TFixedString m_victoryDescMessage;

	i32 m_previousRoundTeamScores[2];
	
	i32 m_endOfRoundStringCount;
	i32 m_endOfRoundVictoryStringCount;
	i32 m_primaryTeamOverride; 

	i32 m_missedLoadout;
	
	i32 m_roundNumber;		// Start on round 0
	i32 m_previousRoundWinnerTeamId;
	i32 m_numLocalSpawnsThisRound;
	EntityId m_previousRoundWinnerEntityId;
	EGameOverReason m_previousRoundWinReason;
	
	float m_prevServerTime;
	float m_newRoundStartTime;
	float m_newRoundShowLoadoutTime;
	float m_victoryMessageTime;
	float m_suddenDeathTime;
	float m_timeSinceRoundEndStateChange;

	ERoundState m_roundState;
	ERoundEndHUDState m_roundEndHUDState;

	bool m_resetScores;			// Reset scores on round end
	bool m_allowBestOfVictory;
	bool m_treatCurrentRoundAsFinalRound;
	bool m_primaryStartRoundStringIsTeamMessage;
	bool m_secondaryStartRoundStringIsTeamMessage;
	bool m_bShownLoadout;
	bool m_bShowPrimaryTeamBanner;
	bool m_bShowSecondaryTeamBanner;
	bool m_bShowKillcamAtEndOfRound;
	bool m_bCustomHeaderPrimary;
	bool m_bCustomHeaderSecondary;
	bool m_bShowRoundStartingMessageEverySpawn;
};

#endif // _GameRulesStandardRounds_h_
