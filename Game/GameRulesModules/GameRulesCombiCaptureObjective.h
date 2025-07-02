// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Implementation of a "combination capture" objective.
		One team's goal is to be in proximity of one or more of multiple "capture"
		points for a specified combined duration of time.
		The other team must prevent them.

	-------------------------------------------------------------------------
	История:
	- 16:12:2009  : Created by Thomas Houghton

*************************************************************************/

#ifndef _GAME_RULES_COMBI_CAPTURE_OBJECTIVE_H_
#define _GAME_RULES_COMBI_CAPTURE_OBJECTIVE_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/GameRulesModules/GameRulesHoldObjectiveBase.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesPlayerStatsListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRevivedListener.h>
#include <drx3D/Game/Audio/AudioSignalPlayer.h>
#include <drx3D/Game/GameRulesTypes.h>
#include <drx3D/Game/UI/UIUpr.h>
#include <drx3D/Game/UI/HUD/HUDEventDispatcher.h>

class CGameRulesCombiCaptureObjective :	public CGameRulesHoldObjectiveBase,
																				public IGameRulesRoundsListener,
																				public IGameRulesPlayerStatsListener,
																				public IGameRulesRevivedListener,
																				public SGameRulesListener
{
private:
	typedef CGameRulesHoldObjectiveBase BaseType;

	struct SSvCaptureScorer
	{
		EntityId  m_eid;
		i32  m_frame;
		float  m_fromTime;
		float  m_lastScoreBucketAddTime;
		float  m_scoreBucket;
		bool  m_primary;
		ILINE void Reset()
		{
			Set(0, 0, 0.f, false);
		}
		ILINE void Set(const EntityId eid, i32k frame, const float fromTime, const bool primary)
		{
			m_eid = eid;
			m_frame = frame;
			m_fromTime = fromTime;
			m_lastScoreBucketAddTime = fromTime;
			m_scoreBucket = 0.f;
			m_primary = primary;
		}
	};

	class CSvCaptureScorersList : public DrxFixedArray<SSvCaptureScorer, 8>
	{
	public:
		SSvCaptureScorer* FindByEntityId(const EntityId eid);
	};

	enum ECapFlashState
	{
		eCFS_NULL = 0,
		eCFS_Idle,
		eCFS_StartingCap,
		eCFS_CaptureInProgress,
		eCFS_StoppingCap,
		eCFS_CapComplete,
		eCFS_NUM
	};

	enum ECapFlashEvent
	{
		eCFE_Reset = 0,
		eCFE_Capturing,
		eCFE_StoppedCapturing,
		eCFE_ForcedComplete,
		eCFE_NUM
	};

public:
	CGameRulesCombiCaptureObjective();
	~CGameRulesCombiCaptureObjective();

	// IGameRulesSimpleEntityObjective
	virtual void Init(XmlNodeRef xml);
	virtual void Update(float frameTime);

	virtual void OnStartGame();

	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags );

	virtual bool IsComplete(i32 teamId);

	virtual void OnHostMigration(bool becomeServer) {}
	virtual bool IsPlayerEntityUsingObjective(EntityId playerId);
	// ~IGameRulesSimpleEntityObjective

	bool AllTeamPlayersDead(i32k teamId);

	// IGameRulesClientConnectionListener
	virtual void OnOwnClientEnteredGame();
	// ~IGameRulesClientConnectionListener

	// IGameRulesRoundsListener
	virtual void OnRoundStart();
	virtual void OnRoundEnd();
	virtual void OnSuddenDeath() {}
	virtual void ClRoundsNetSerializeReadState(i32 newState, i32 curState) {}
	virtual void OnRoundAboutToStart() {}
	// ~IGameRulesRoundsListener

	// IGameRulesPlayerStatsListener
	virtual void ClPlayerStatsNetSerializeReadDeath(const SGameRulesPlayerStat* s, u16 prevDeathsThisRound, u8 prevFlags);
	// ~IGameRulesPlayerStatsListener
	
	// IGameRulesRevivedListener
	virtual void EntityRevived(EntityId entityId);
	// ~IGameRulesRevivedListener

	// IGameRulesTeamChangedListener
	virtual void OnChangedTeam(EntityId entityId, i32 oldTeamId, i32 newTeamId);
	// ~IGameRulesTeamChangedListener

	// IGameRulesKillListener
	virtual void OnEntityKilledEarly(const HitInfo &hitInfo) {};
	virtual void OnEntityKilled(const HitInfo &hitInfo);
	// ~IGameRulesKillListener

	// SGameRulesListener (interface)
	virtual void SvOnTimeLimitExpired();
	virtual void ClTeamScoreFeedback(i32 teamId, i32 prevScore, i32 newScore);
	// ~SGameRulesListener

	// IGameRulesClientConnectionListener
	virtual void OnClientConnect(i32 channelId, bool isReset, EntityId playerId);
	// ~IGameRulesClientConnectionListener

protected:
	typedef std::vector<EntityId> TEntityIdVec;

	struct SCaptureEntity : public BaseType::IHoldEntityAdditionalDetails
	{
		void Reset(const SHoldEntityDetails* pDetails)
		{
			m_currentIcon = EGRMO_Unknown;

			m_iconTransitionAmount = 0.f;

			m_needIconUpdate = false;
			m_capturing = false;

			if (pDetails)
			{
				if (gEnv->IsClient())
				{
					/*if (m_alarmSignalPlayer.IsPlaying(pDetails->m_id))
					{
						m_alarmSignalPlayer.Stop(pDetails->m_id);
					}*/
					m_alarmSignalPlayer.Reset();
				}
				SetEnabled(true, false, pDetails);
			}
			else
			{
				// there should only be a NULL pDetails passed in on the initial Reset() call during the construction of the module
				m_alarmSignalPlayer.Reset();
				m_enabled = true;
			}
		}

		void Associate(const SHoldEntityDetails* pDetails, CGameRulesCombiCaptureObjective* pCombiCapObj);

		void SetEnabled(const bool enable, const bool updateIcon, const SHoldEntityDetails* pDetails);

		CAudioSignalPlayer m_alarmSignalPlayer;

		EGameRulesMissionObjectives m_currentIcon;

		float m_iconTransitionAmount;

		bool m_needIconUpdate;
		bool m_capturing;
		bool m_enabled;
	};

	enum ECombiCaptureVO
	{
		eCCVO_Initial = 0,
		eCCVO_25 = eCCVO_Initial,
		eCCVO_50 = 1,
		eCCVO_75 = 2,
		eCCVO_90 = 3,
		eCCVO_Size
	};

	struct SCombiVOData
	{
		SCombiVOData() : m_announcement(""), m_progress(0.0f) {}

		void Init(tukk announcement, const float progress)
		{
				m_announcement = announcement;
				m_progress = progress;
		}

		tukk m_announcement;
		float m_progress;
	};

	typedef DrxFixedStringT<32>  TFixedString;

	// CGameRulesHoldObjectiveBase
	virtual void OnInsideStateChanged(SHoldEntityDetails *pDetails);
	virtual void OnNewHoldEntity(SHoldEntityDetails *pDetails, i32 index);
	virtual void OnRemoveHoldEntity(SHoldEntityDetails *pDetails);
	virtual void OnNetSerializeHoldEntity(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags, SHoldEntityDetails *pDetails, i32 index) {};

	virtual void OnControllingTeamChanged(SHoldEntityDetails *pDetails, i32k oldControllingTeam);
	// ~CGameRulesHoldObjectiveBase

	void SvDoEndOfRoundPlayerScoring(i32k winningTeam);

	void ClUpdateSiteHUD(SHoldEntityDetails *pDetails, i32k currActiveIndex);

	void AwardPlayerPoints(TEntityIdVec *pEntityVec, EGRST scoreType);
	void UpdateIcon(SHoldEntityDetails * pDetails, bool force);
	void UpdateCaptureProgress(SHoldEntityDetails *pDetails, float frameTime);
	void ClSiteStartCapturing(SHoldEntityDetails *pDetails);
	EGameRulesMissionObjectives GetIcon(SHoldEntityDetails *pDetails);

	void UpdateCaptureVO();

	void SvUpdateCaptureScorers();

	virtual void DetermineControllingTeamId(SHoldEntityDetails *pDetails, i32k team1Count, i32k team2Count);

	void UpdateCaptureAudio(SHoldEntityDetails *pDetails);

	i32 GetNumDesiredEnabledCaptureEnts();
	void RefreshCaptureEntEnabledState(SHoldEntityDetails* pDetails, i32k numDesired);
	void RefreshAllCaptureEntsEnabledStates();
	void SetLoadoutPackageGroup(i32 teamId, bool bOnRoundEnd);

protected:
	static i32k  AMOUNT_OF_DESIRED_CAP_ENTS_MORE_THAN_PLAYERS;
	static i32k  MAX_DESIRED_CAP_ENTS;
	static i32k  MIN_DESIRED_CAP_ENTS;

	SCaptureEntity m_additionalInfo[HOLD_OBJECTIVE_MAX_ENTITIES];

	CSvCaptureScorersList  m_svCaptureScorers;

	TFixedString m_shouldShowIconFunc;

	EGameRulesMissionObjectives m_ourCapturePoint;
	EGameRulesMissionObjectives m_theirCapturePoint;
	EGameRulesMissionObjectives m_usCapturingPoint;
	EGameRulesMissionObjectives m_themCapturingPoint;

	i32 m_currentVO;
	SCombiVOData m_combiVOData[eCCVO_Size];

	i32 m_iconPriority;
	i32 m_numActiveEntities;
	i32 m_prevAttackingTeamId;
	i32 m_attackingTeamId;
	i32 m_clientTeamId;
	i32 m_highestNumDesiredCapEntsThisRound;

	float m_combiProgress;
	float m_combiProgressBanked;

	float m_goalCombiCaptureTime;

	float m_progressBankingThreshold;

	float  m_defWin_timeRemainBonus_minTime;
	float  m_defWin_timeRemainBonus_minPoints;

	float  m_captureScoringThreshold;
	float  m_captureScoringAssistThreshold;
	float  m_captureScoringAssistFrac;

	float  m_lastMinuteSkillAssessmentThreshold;

	bool m_doMidThresholdPartialCaptureScoring;

	bool m_contestable;
	bool m_useIcons;
	bool m_allowMultiPlayerCaptures;

	bool m_updatedCombiProgressThisFrame;
	bool m_bUpdatedBankedProgressThisFrame;
	bool m_bBetweenRounds;

	TAudioSignalID m_captureSignalId;
	TAudioSignalID m_interruptSignalId;
	TAudioSignalID m_inactiveSignalId;
	TAudioSignalID m_alarmSignalId;
};

#endif // _GAME_RULES_COMBI_CAPTURE_OBJECTIVE_H__

