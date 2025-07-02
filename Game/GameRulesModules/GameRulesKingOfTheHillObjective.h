// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Implementation of a king of the hill objective (take and hold)
	-------------------------------------------------------------------------
	История:
	- 15:02:2010  : Created by Colin Gulliver

*************************************************************************/

#ifndef _GAME_RULES_KING_OF_THE_HILL_OBJECTIVE_H_
#define _GAME_RULES_KING_OF_THE_HILL_OBJECTIVE_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/GameRulesModules/GameRulesHoldObjectiveBase.h>
#include <drx3D/Game/GameRulesModules/IGameRulesTeamChangedListener.h>
#include <drx3D/Game/Audio/AudioSignalPlayer.h>
#include <drx3D/Game/GameRulesTypes.h>

class CGameRulesKingOfTheHillObjective :	public CGameRulesHoldObjectiveBase
{
private:
	typedef CGameRulesHoldObjectiveBase BaseType;
public:
	CGameRulesKingOfTheHillObjective();
	~CGameRulesKingOfTheHillObjective();

	// IGameRulesEntityObjective
	virtual void Init(XmlNodeRef xml);
	virtual void Update(float frameTime);

	virtual void OnStartGame();

	virtual bool IsComplete(i32 teamId) { return false; }

	virtual bool IsEntityFinished(i32 type, i32 index)	{ return false; }
	virtual bool CanRemoveEntity(i32 type, i32 index)		{ return true; }
	virtual void SetWaveNumber(i32 num, i32 waveCount) {}

	virtual void OnHostMigration(bool becomeServer) {}

	virtual void OnTimeTillRandomChangeUpdated(i32 type, float fPercLiveSpan);
	// ~IGameRulesEntityObjective

	// IGameRulesClientConnectionListener
	virtual void OnOwnClientEnteredGame();
	// ~IGameRulesClientConnectionListener

	// IGameRulesTeamChangedListener
	virtual void OnChangedTeam(EntityId entityId, i32 oldTeamId, i32 newTeamId);
	// ~IGameRulesTeamChangedListener

protected:
	struct SKotHEntity : public BaseType::IHoldEntityAdditionalDetails
	{
		SKotHEntity()
		{
			Reset();
		}

		void Reset()
		{
			m_currentIcon = EGRMO_Unknown;

			m_scoringTeamId = 0;
			m_fScoringSFX = 0.0f;

			m_timeSinceLastScore = 0.f;
			m_scoreTimerLength = 0.f;
			m_radiusEffectScale = 1.f;
			m_pulseTime = 0.f;

			m_isOnRadar = false;
			m_needsIconUpdate = false;
			m_bPulseEnabled = false;
		}

		EGameRulesMissionObjectives m_currentIcon;

		i32 m_scoringTeamId;
		float m_fScoringSFX;

		float m_timeSinceLastScore;
		float m_scoreTimerLength;
		float m_radiusEffectScale;
		float m_pulseTime;

		bool m_isOnRadar;
		bool m_needsIconUpdate;
		bool m_bPulseEnabled;
	};

	typedef DrxFixedStringT<32> TFixedString;

	// CGameRulesHoldObjectiveBase
	virtual bool AreObjectivesStatic() { return true; }
	virtual void OnInsideStateChanged(SHoldEntityDetails *pDetails);
	virtual void OnNewHoldEntity(SHoldEntityDetails *pDetails, i32 index);
	virtual void OnRemoveHoldEntity(SHoldEntityDetails *pDetails);
	// ~CGameRulesHoldObjectiveBase

	void SvSiteChangedOwner(SHoldEntityDetails *pDetails);
	void ClSiteChangedOwner(SHoldEntityDetails *pDetails, i32 oldTeamId);
	void ClSiteChangedOwnerAnnouncement(SHoldEntityDetails *pDetails, EntityId clientActorId, i32 ownerTeamId, i32 localTeam);

	void UpdateIcon(SHoldEntityDetails *pDetails);
	EGameRulesMissionObjectives GetIcon(SHoldEntityDetails *pDetails, tukk* ppOutName, tukk* ppOutColour);

	float CalculateScoreTimer(i32 playerCount);

	void ClUpdateHUD(SHoldEntityDetails *pDetails);

	void InitEntityAudio(SHoldEntityDetails *pDetails);
	void UpdateEntityAudio(SHoldEntityDetails *pDetails);
	void ClearEntityAudio(SHoldEntityDetails *pDetails);

	virtual void Announce(tukk announcement, TAnnounceType inType, const bool shouldPlayAudio = true) const;

	SKotHEntity m_additionalInfo[HOLD_OBJECTIVE_MAX_ENTITIES];

	TFixedString m_friendlyCaptureString;
	TFixedString m_enemyCaptureString;
	TFixedString m_friendlyLostString;
	TFixedString m_enemyLostString;
	TFixedString m_newEntityString;
	TFixedString m_shouldShowIconFunc;
	TFixedString m_shouldDoPulseEffectFunc;
	TFixedString m_gameStateNeutralString;
	TFixedString m_gameStateFriendlyString;
	TFixedString m_gameStateEnemyString;
	TFixedString m_gameStateDestructingString;
	TFixedString m_gameStateIncomingString;
	TFixedString m_iconTextDefend;
	TFixedString m_iconTextClear;
	TFixedString m_iconTextCapture;

	EGameRulesMissionObjectives m_neutralIcon;
	EGameRulesMissionObjectives m_friendlyIcon;
	EGameRulesMissionObjectives m_hostileIcon;
	EGameRulesMissionObjectives m_contestedIcon;

	float m_scoreTimerMaxLength;
	float m_scoreTimerAdditionalPlayerMultiplier;
	float m_pulseTimerLength;

	bool m_useIcons;

	TAudioSignalID m_captureSignalId;
};

#endif // _GAME_RULES_KING_OF_THE_HILL_OBJECTIVE_H_

