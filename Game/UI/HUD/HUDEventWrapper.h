// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __HUDEVENTWRAPPER_H__
#define __HUDEVENTWRAPPER_H__


//////////////////////////////////////////////////////////////////////////
#include <drx3D/Game/Audio/AudioSignalPlayer.h>
#include <drx3D/Game/Audio/AudioTypes.h>
#include <drx3D/Game/Audio/AudioSignalPlayer.h>
#include <drx3D/Game/GameRulesTypes.h>
#include <drx3D/Game/ItemSharedParams.h>
#include <drx3D/Game/UI/UITypes.h>

//////////////////////////////////////////////////////////////////////////

struct IWeapon;

//////////////////////////////////////////////////////////////////////////

enum EBL_IconFrames{
	eBLIF_NoIcon = 0,
	eBLIF_Melee = 1,
	eBLIF_Suicide = 2,
	eBLIF_Weapon = 3,	// only frame used in pvp messages now
	eBLIF_Stamp,	// not a valid frame. Used as an ID instead
	eBLIF_FragGrenade,		// not a valid frame. Used as an ID instead
	eBLIF_FlashGrenade,		// not a valid frame. Used as an ID instead
	eBLIF_SmokeGrenade,		// not a valid frame. Used as an ID instead
	eBLIF_C4,							// not a valid frame. Used as an ID instead
	eBLIF_HeadShot,				// not a valid frame. Used as an ID instead
	eBLIF_CarDeath,				// not a valid frame. Used as an ID instead
	eBLIF_ExplodingBarrel, // not a valid frame. Used as an ID instead
	eBLIF_MicrowaveBeam,		// not a valid frame. Used as an ID instead
	eBLIF_SuitDisrupter,		// not a valid frame. Used as an ID instead
	eBLIF_CrashSiteLanding,	// not a valid frame. Used as an ID instead
	eBLIF_MeleeInWeaponsFlash, // not a valid frame. Used as an ID instead
	eBLIF_SlidingMelee,			// not a valid frame. Used as an ID instead
	eBLIF_FallDeath,				// not a valid frame. Used as an ID instead
	eBLIF_PhysicsImpact,		// not a valid frame. Used as an ID instead
	eBLIF_StealthKill,			// not a valid frame. Used as an ID instead
	eBLIF_AutoTurret,				// not a valid frame. Used as an ID instead
	eBLIF_FireDamage,				// not a valid frame. Used as an ID instead
	eBLIF_SuicideInWeaponsFlash, // not a valid frame. Used as an ID instead
	eBLIF_ShotgunAttachment,  // not a valid frame. Used as an ID instead
	eBLIF_GaussAttachment,		// not a valid frame. Used as an ID instead
	eBLIF_GrenadeAttachment,	// not a valid frame. Used as an ID instead
	eBLIF_Relay,							// not a valid frame. Used as an ID instead
	eBLIF_Tick,								// not a valid frame. Used as an ID instead
	eBLIF_Swarmer,						// not a valid frame. Used as an ID instead

	// TODO : Headshot, currently using melee
};


// Wrappers for sending Events to the HUD...
struct SHUDEventWrapper
{
	// Message data //////////

	struct SMsgAudio
	{
	public:
		enum EType
		{
			eT_None = 0,
			eT_Signal,
			eT_Announcement
		};
		union UData
		{
			struct SSignal
			{
				TAudioSignalID  id;
			}
			signal;
			struct SAnnouncment
			{
				i32  teamId;
				EAnnouncementID  id;
			}
			announcement;
		};

	public:
		UData  m_datau;
		EType  m_type;

	public:
		SMsgAudio()
		{
			m_type = eT_None;
		}
		SMsgAudio(const TAudioSignalID signalId)
		{
			m_datau.signal.id = signalId;
			m_type = eT_Signal;
		}
		SMsgAudio(i32k teamId, const EAnnouncementID announceId)
		{
			m_datau.announcement.teamId = teamId;
			m_datau.announcement.id = announceId;
			m_type = eT_Announcement;
		}
	};

	static const SMsgAudio  kMsgAudioNULL;

	static CAudioSignalPlayer s_unlockSound;
	static CAudioSignalPlayer s_unlockSoundRare;

	// Messages //////////

	static void GameStateNotify(tukk msg, const SMsgAudio& audio, tukk p1=NULL, tukk p2=NULL, tukk p3=NULL, tukk p4=NULL);
	static void GameStateNotify( tukk msg, tukk p1=NULL, tukk p2=NULL, tukk p3=NULL, tukk p4=NULL);
	static void RoundMessageNotify(tukk msg, const SMsgAudio& audio=kMsgAudioNULL);
	static void DisplayLateJoinMessage();
	static void TeamMessage( tukk msg, i32k team, const SHUDEventWrapper::SMsgAudio& audio, const bool bShowTeamName, const bool bShowTeamIcon, tukk  pCustomHeader=NULL, const float timeToHoldPauseState=0.f);
	static void SimpleBannerMessage( tukk msg, const SHUDEventWrapper::SMsgAudio& audio, const float timeToHoldPauseState=0.f);
	static void OnChatMessage( const EntityId entity, i32k teamId, tukk message);
	static void OnGameStateMessage( const EntityId actorId, const bool bIsFriendly, tukk pMessage, EBL_IconFrames icon=eBLIF_NoIcon);
	static void OnAssessmentCompleteMessage( tukk groupName, tukk assesmentName, tukk description, tukk descriptionParam, i32k xp );
	static void OnNewMedalMessage( tukk name, tukk description, tukk descriptionParam );
	static void OnPromotionMessage( tukk rankName, i32k rank, i32k xpRequired );
	static void OnSupportBonusXPMessage( i32k xpType /*EPPType*/, i32k xpGained );
	static void OnSkillKillMessage( i32k xpType /*EPPType*/, i32k xpGained );
	static void OnGameStatusUpdate( const EGoodBadNeutralForLocalPlayer good, tukk message );
	static void OnRoundEnd( i32k winner, const bool clientScoreIsTop, tukk victoryDescMessage, tukk victoryMessage, const EAnnouncementID announcement );
	static void OnSuddenDeath( void );
	static void OnGameEnd(i32k teamOrPlayerId, const bool clientScoreIsTop, const EGameOverReason reason, tukk message, const ESVC_DrawResolution drawResolution, const EAnnouncementID announcement);

	static void UpdateGameStartCountdown(const EPreGameCountdownType countdownType, const float timeTillStartInSeconds);

	static void CantFire(void);
	static void FireModeChanged(IWeapon* pWeapon, i32k currentFireMode, bool bForceFireModeUpdate = false);
	static void ForceCrosshairType(IWeapon* pWeapon, const ECrosshairTypes forcedCrosshairType );
	static void OnInteractionUseHoldTrack(const bool bTrackingUse);
	static void OnInteractionUseHoldActivated(const bool bSuccessful);
	static void InteractionRequest(const bool activate, tukk msg, tukk action, tukk actionmap, const float duration, const bool bGameRulesRequest = false, const bool bIsFlowNodeReq = false, const bool bShouldSerialize = true); // TODO: Remove this and all uses of it, should use SetInteractionMsg
	static void ClearInteractionRequest(const bool bGameRulesRequest = false); // TODO: Remove this and all uses of it, should use ClearInteractionMsg
	static void SetInteractionMsg(const EHUDInteractionMsgType interactionType, 
																tukk message, 
																const float duration, 
																const bool bShouldSerialize, 
																tukk interactionAction = NULL,																// Interaction types only (But don't need if using new method (i.e. message="Press [[ACTIONNAME,ACTIONMAPNAME]] to do action")
																tukk interactionActionmap = NULL,														// Interaction types only (But don't need if using new method (i.e. message="Press [[ACTIONNAME,ACTIONMAPNAME]] to do action")
																const EntityId usableEntityId = 0,																	// Usable types only
																const EntityId usableSwapEntityId = 0,															// Usable types only
																const EInteractionType usableInteractionType = eInteraction_None,   // Usable types only
																tukk szExtraParam1 = NULL);																	// Used as a parameter for %2 for non usable msgs
	static void ClearInteractionMsg(EHUDInteractionMsgType interactionType, tukk onlyRemoveMsg = NULL, const float fadeOutTime = -1.0f);
	static void DisplayInfo(i32k system, const float time, tukk infomsg, tukk param1=NULL, tukk param2=NULL, tukk param3=NULL, tukk param4=NULL, const EInfoSystemPriority priority=eInfoPriority_Low);
	static void ClearDisplayInfo(i32k system, const EInfoSystemPriority priority=eInfoPriority_Low);
	static void GenericBattleLogMessage(const EntityId actorId, tukk message, tukk p1=NULL, tukk p2=NULL, tukk p3=NULL, tukk p4=NULL);

	static void HitTarget( EGameRulesTargetType targetType, i32 bulletType, EntityId targetId );

	static void OnNewObjective( const EntityId entityId, const EGameRulesMissionObjectives iconType );
	static void OnNewObjective( const EntityId entityId, const EGameRulesMissionObjectives iconType, const float progress, i32k priority );
	static void OnNewObjective( const EntityId entityId, const EGameRulesMissionObjectives iconType, const float progress, i32k priority, tukk pNameOverride, tukk pColourStr );
	static void OnNewObjectiveWithRadarEntity( const EntityId entityId, const EntityId radarEntityId, const EGameRulesMissionObjectives iconType, const float progress, i32k priority, tukk pNameOverride, tukk pColourStr );
	static void OnRemoveObjective( const EntityId entityId );
	static void OnRemoveObjective( const EntityId entityId, i32k priority );
	static void OnRemoveObjectiveWithRadarEntity( const EntityId entityId, const EntityId radarEntityId, i32k priority );
	static void OnNewGameRulesObjective( const EntityId entityId );

	static void RadarSweepActivated(i32 targetTeam, float activateDuration);

	static void OnBootAction(u32k bootAction, tukk elementName);

	static void ActivateOverlay(i32k type);
	static void DeactivateOverlay(i32k type);

	// Items
	static void OnPrepareItemSelected(const EntityId itemId, const EItemCategoryType category, const EWeaponSwitchSpecialParam specialParam );

	// Players
	static void PlayerRename(const EntityId playerId, const bool isReplay = false);

	// Vehicles
	static void OnPlayerLinkedToVehicle(const EntityId playerId, const EntityId vehicleId, bool alwaysShowHealthBar = false);
	static void OnPlayerUnlinkedFromVehicle(const EntityId playerId, const EntityId vehicleId, const bool keepHealthBar = false);

	static void OnStartTrackFakePlayerTagname( const EntityId tagEntityId, i32k sessionNameIdxToTrack );
	static void OnStopTrackFakePlayerTagname( const EntityId tagEntityId );

	static void PowerStruggleNodeStateChange(i32k activeNodeIdentityId, i32k nodeHUDState);
	static void PowerStruggleManageCaptureBar(EHUDPowerStruggleCaptureBarType inType, float inChargeAmount, bool inContention, tukk inBarString);

	static void OnBigMessage(tukk inSubTitle, tukk inTitle);
	static void OnBigWarningMessage(tukk line1, tukk line2, const float duration = -1.f);
	static void OnBigWarningMessageUnlocalized(tukk line1, tukk line2, const float duration = -1.f);
	static void UpdatedDirectionIndicator(const EntityId &id);
	static void SetStaticTimeLimit(bool active, i32 time);

	static void PlayUnlockSound();
	static void PlayUnlockSoundRare();

	static void DisplayWeaponUnlockMsg(tukk szCollectibleId);
};

//////////////////////////////////////////////////////////////////////////

#endif // __HUDEVENTWRAPPER_H__
