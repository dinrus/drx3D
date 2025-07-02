// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Objectives module for CTF
	-------------------------------------------------------------------------
	История:
	- 23:02:2011  : Created by Colin Gulliver

*************************************************************************/

#ifndef _GAME_RULES_OBJECTIVE_CTF_H
#define _GAME_RULES_OBJECTIVE_CTF_H

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/IGameRulesObjectivesModule.h>
#include <drx3D/Game/IGameRulesModuleRMIListener.h>
#include <drx3D/Game/IGameRulesClientConnectionListener.h>
#include <drx3D/Game/IGameRulesKillListener.h>
#include <drx3D/Game/IGameRulesTeamChangedListener.h>
#include <drx3D/Game/IGameRulesRoundsListener.h>
#include <drx3D/Game/IGameRulesActorActionListener.h>
#include <drx3D/Entity/IEntitySystem.h>

class CGameRulesObjective_CTF : public IGameRulesObjectivesModule,
																public IGameRulesModuleRMIListener,
																public IGameRulesClientConnectionListener,
																public IGameRulesKillListener,
																public IGameRulesTeamChangedListener,
																public IGameRulesRoundsListener,
																public IGameRulesActorActionListener,
																public IEntityEventListener
{
public:
	CGameRulesObjective_CTF();
	virtual ~CGameRulesObjective_CTF();

	// IGameRulesObjectivesModule
	virtual void Init(XmlNodeRef xml);
	virtual void Update(float frameTime);

	virtual void OnStartGame();
	virtual void OnStartGamePost() {}
	virtual void OnGameReset() {}

	virtual void PostInitClient(i32 channelId);
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags ) { return true; }

	virtual bool HasCompleted(i32 teamId) { return false; }

	virtual void OnHostMigration(bool becomeServer);

	virtual bool AreSuddenDeathConditionsValid() const { return false; }
	virtual void CheckForInteractionWithEntity(EntityId interactId, EntityId playerId, SInteractionInfo &interactionInfo);
	virtual bool CheckIsPlayerEntityUsingObjective(EntityId playerId);

	virtual ESpawnEntityState GetObjectiveEntityState(EntityId entityId);

	virtual i32 GetAutoAssignTeamId(i32 channelId) { return 0; }
	virtual void OnEntitySignal(EntityId entityId, i32 signal) { } 
	// ~IGameRulesObjectivesModule

	// IGameRulesModuleRMIListener
	virtual void OnSingleEntityRMI(CGameRules::SModuleRMIEntityParams params);
	virtual void OnDoubleEntityRMI(CGameRules::SModuleRMITwoEntityParams params);
	virtual void OnEntityWithTimeRMI(CGameRules::SModuleRMIEntityTimeParams params) {}
	virtual void OnSvClientActionRMI(CGameRules::SModuleRMISvClientActionParams params, EntityId fromEid);
	// ~IGameRulesModuleRMIListener

	// IGameRulesClientConnectionListener
	virtual void OnClientConnect(i32 channelId, bool isReset, EntityId playerId) {}
	virtual void OnClientDisconnect(i32 channelId, EntityId playerId) { OnPlayerKilledOrLeft(playerId, 0); }
	virtual void OnClientEnteredGame(i32 channelId, bool isReset, EntityId playerId) {}
	virtual void OnOwnClientEnteredGame();
	// ~IGameRulesClientConnectionListener

	// IGameRulesKillListener
	virtual void OnEntityKilledEarly(const HitInfo &hitInfo) {}
	virtual void OnEntityKilled(const HitInfo &hitInfo) { OnPlayerKilledOrLeft(hitInfo.targetId, hitInfo.shooterId); }
	// ~IGameRulesKillListener

	// IGameRulesTeamChangedListener
	virtual void OnChangedTeam(EntityId entityId, i32 oldTeamId, i32 newTeamId);
	// ~IGameRulesTeamChangedListener

	// IGameRulesRoundsListener
	virtual void OnRoundStart() {};
	virtual void OnRoundEnd() {}
	virtual void OnSuddenDeath() {};
	virtual void ClRoundsNetSerializeReadState(i32 newState, i32 curState) {};
	virtual void OnRoundAboutToStart();
	// ~IGameRulesRoundsListener

	// IGameRulesActorActionListener
	virtual void OnAction(const ActionId& action, i32 activationMode, float value);
	// ~IGameRulesActorActionListener

	// IEntityEventListener
	virtual void OnEntityEvent( IEntity *pEntity, SEntityEvent &event );
	// ~IEntityEventListener

private:
	enum ERMITypes
	{
		// Single entity RMIs
		eRMI_SetupBase,
		eRMI_Pickup,
		eRMI_Drop,
		eRMI_SetupFlag,
		eRMI_SetupDroppedFlag,
		eRMI_ResetAll,
		eRMI_Scored,

		// Double entity RMIs
		eRMI_ResetFlag,
		eRMI_SetupFallbackWeapon,
	};

	struct STeamInfo
	{
		EntityId m_flagId;
		EntityId m_baseId;
		EntityId m_carrierId;
		EntityId m_fallbackWeaponId;

		ESpawnEntityState m_state;

		i32 m_droppedIconFrame;

		float m_resetTimer;
	};

	typedef DrxFixedStringT<32> TFixedString;

	void OnPlayerKilledOrLeft(EntityId targetId, EntityId shooterId);

	void AttachFlagToBase(STeamInfo *pTeamInfo);
	void DetachFlagFromBase(STeamInfo *pTeamInfo);

	STeamInfo *FindInfoByFlagId(EntityId flagId);
	i32 FindTeamIndexByCarrierId(EntityId playerId) const;
	STeamInfo *FindInfoByCarrierId(EntityId playerId);

	// Pick up records
	bool IsPlayerInPickedUpRecord(EntityId playerId) const; 
	void SetPlayerHasPickedUp(EntityId playerId, const bool bHasPickedUp); 
	void ClearPlayerPickedUpRecords(); 

	// Client to server RMIs
	void Handle_RequestPickup(EntityId playerId);
	void Handle_RequestDrop(EntityId playerId);

	// Server to client RMIs
	void Handle_ResetFlag(EntityId flagId, EntityId playerId);
	void Handle_SetupBase(EntityId baseId);
	void Handle_Pickup(EntityId playerId);
	void Handle_Drop(EntityId playerId);
	void Handle_SetupFlag(EntityId flagId);
	void Handle_SetupDroppedFlag(EntityId flagId);
	void Handle_ResetAll();
	void Handle_SetupFallbackWeapon(EntityId weaponId, EntityId flagId);
	void Handle_Scored(EntityId playerId);

	void Server_ResetAll();
	void Server_Drop(EntityId playerId, STeamInfo *pTeamInfo, EntityId shooterId);
	void Server_ResetFlag(STeamInfo *pTeamInfo, EntityId playerId);
	void Server_SwapSides();
	void Server_CheckForFlagRetrieval();
	void Server_PhysicalizeFlag(EntityId flagId, bool bEnable);

	void Common_Pickup(EntityId playerId);
	void Common_Drop(EntityId playerId);
	void Common_ResetFlag(EntityId flagId, EntityId playerId);
	void Common_Scored(EntityId playerId);
	void Common_ResetAll();

	void Client_RemoveAllIcons();
	void Client_SetIconForBase(i32 teamIndex);
	void Client_SetIconForFlag(i32 teamIndex);
	void Client_SetIconForCarrier(i32 teamIndex, EntityId carrierId, bool bRemoveIcon);
	void Client_SetAllIcons();
	void Client_UpdatePickupAndDrop();

	DrxFixedArray<EntityId, MAX_PLAYER_LIMIT> m_playersThatHavePickedUp;

	CAudioSignalPlayer m_baseAlarmSound[2];
	STeamInfo m_teamInfo[2];

	TFixedString m_stringComplete;
	TFixedString m_stringHasTaken;
	TFixedString m_stringHasCaptured;
	TFixedString m_stringHasDropped;
	TFixedString m_stringHasReturned;

	TFixedString m_iconStringDefend;
	TFixedString m_iconStringReturn;
	TFixedString m_iconStringEscort;
	TFixedString m_iconStringSeize;
	TFixedString m_iconStringKill;
	TFixedString m_iconStringBase;

	EGameRulesMissionObjectives m_iconFriendlyFlagCarried;
	EGameRulesMissionObjectives m_iconHostileFlagCarried;
	EGameRulesMissionObjectives m_iconFriendlyFlagDropped;
	EGameRulesMissionObjectives m_iconHostileFlagDropped;
	EGameRulesMissionObjectives m_iconFriendlyBaseWithFlag;
	EGameRulesMissionObjectives m_iconHostileBaseWithFlag;
	EGameRulesMissionObjectives m_iconFriendlyBaseWithNoFlag;
	EGameRulesMissionObjectives m_iconHostileBaseWithNoFlag;
	
	TAudioSignalID m_soundFriendlyComplete;
	TAudioSignalID m_soundHostileComplete;
	TAudioSignalID m_soundFriendlyPickup;
	TAudioSignalID m_soundHostilePickup;
	TAudioSignalID m_soundFriendlyReturn;
	TAudioSignalID m_soundHostileReturn;
	TAudioSignalID m_soundFriendlyDropped;
	TAudioSignalID m_soundHostileDropped;
	TAudioSignalID m_soundBaseAlarmOff;

	CGameRules *m_pGameRules;
	IEntityClass *m_pFallbackWeaponClass;

	i32 m_moduleRMIIndex;

	float m_resetTimerLength;
	float m_retrieveFlagDistSqr;
	float m_pickUpVisCheckHeight;

	bool m_bUseButtonHeld;
};

#endif // _GAME_RULES_OBJECTIVE_CTF_H

