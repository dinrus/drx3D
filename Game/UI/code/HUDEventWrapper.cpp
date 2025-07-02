// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/HUDEventWrapper.h>
#include <drx3D/Game/HUDEventDispatcher.h>

//////////////////////////////////////////////////////////////////////////

#include <drx3D/Game/HUDEventDispatcher.h>
#include <drx3D/Game/HUDUtils.h>
#include <drx3D/Game/Audio/GameAudio.h>

//////////////////////////////////////////////////////////////////////////

const SHUDEventWrapper::SMsgAudio  SHUDEventWrapper::kMsgAudioNULL;
CAudioSignalPlayer SHUDEventWrapper::s_unlockSound;
CAudioSignalPlayer SHUDEventWrapper::s_unlockSoundRare;

//////////////////////////////////////////////////////////////////////////

namespace SHUDEventWrapperUtils
{
	void InternalGenericMessageNotify(const EHUDEventType type, tukk msg, const bool loop, const SHUDEventWrapper::SMsgAudio& audio/*=kMsgAudioNULL*/)
	{
		if (!gEnv->IsDedicated())
		{
			SHUDEvent newRoundMessage(type);
			newRoundMessage.AddData(msg);
			newRoundMessage.AddData(loop);
			newRoundMessage.AddData(&audio);
			CHUDEventDispatcher::CallEvent(newRoundMessage);
		}
	}
}

void SHUDEventWrapper::RoundMessageNotify(tukk msg, const SHUDEventWrapper::SMsgAudio& audio/*=kMsgAudioNULL*/)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEventWrapperUtils::InternalGenericMessageNotify( eHUDEvent_OnRoundMessage, msg, false, audio);
	}
}

// Do not localise your message before passing into this
// Double localisation can cause corruption
void SHUDEventWrapper::GameStateNotify( tukk msg, const SHUDEventWrapper::SMsgAudio& audio, tukk p1 /*=NULL*/, tukk p2 /*=NULL*/, tukk p3 /*=NULL*/, tukk p4 /*=NULL*/ )
{
	if (!gEnv->IsDedicated())
	{
		string localisedString = CHUDUtils::LocalizeString(msg, p1, p2, p3, p4);	// cache the string to ensure no subsequent LocalizeString() calls could mess with our result
		SHUDEventWrapperUtils::InternalGenericMessageNotify( eHUDEvent_OnGameStateNotifyMessage, localisedString.c_str(), false, audio);
	}
}
void SHUDEventWrapper::GameStateNotify( tukk msg, tukk p1 /*=NULL*/, tukk p2 /*=NULL*/, tukk p3 /*=NULL*/, tukk p4 /*=NULL*/ )
{
	if (!gEnv->IsDedicated())
	{
    SHUDEventWrapper::GameStateNotify( msg, INVALID_AUDIOSIGNAL_ID, p1, p2, p3, p4);
	}
}

void SHUDEventWrapper::DisplayLateJoinMessage()
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent displayLateJoinMessage(eHUDEvent_DisplayLateJoinMessage);
		CHUDEventDispatcher::CallEvent(displayLateJoinMessage);
	}
}

void SHUDEventWrapper::TeamMessage( tukk msg, i32k team, const SHUDEventWrapper::SMsgAudio& audio, const bool bShowTeamName, const bool bShowTeamIcon, tukk  pCustomHeader, const float timeToHoldPauseState)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent newTeamMessage(eHUDEvent_OnTeamMessage);
		newTeamMessage.AddData(msg);
		newTeamMessage.AddData(team);
		newTeamMessage.AddData(&audio);
		newTeamMessage.AddData(bShowTeamName);
		newTeamMessage.AddData(bShowTeamIcon);
		newTeamMessage.AddData(pCustomHeader);
		newTeamMessage.AddData(timeToHoldPauseState);
		CHUDEventDispatcher::CallEvent(newTeamMessage);
	}
}

void SHUDEventWrapper::SimpleBannerMessage( tukk msg, const SHUDEventWrapper::SMsgAudio& audio, const float timeToHoldPauseState)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent newBannerMessage(eHUDEvent_OnSimpleBannerMessage);
		newBannerMessage.AddData(msg);
		newBannerMessage.AddData(&audio);
		newBannerMessage.AddData(timeToHoldPauseState);
		CHUDEventDispatcher::CallEvent(newBannerMessage);
	}
}

void SHUDEventWrapper::OnChatMessage( const EntityId entity, i32k teamId, tukk message)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent chatMessageEvent(eHUDEvent_OnChatMessage);
		chatMessageEvent.AddData( static_cast<i32>(entity) );
		chatMessageEvent.AddData( teamId );
		chatMessageEvent.AddData( message );
		CHUDEventDispatcher::CallEvent(chatMessageEvent);
	}
}

void SHUDEventWrapper::OnAssessmentCompleteMessage( tukk groupName, tukk assesmentName, tukk description, tukk descriptionParam, i32k xp )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent xpHudEvent(eHUDEvent_OnAssessmentComplete);
		xpHudEvent.AddData((ukk ) groupName);		//Group name, e.g 'Scar Kills'
		xpHudEvent.AddData((ukk ) assesmentName);	//Assessment name e.g 'Recruit'
		xpHudEvent.AddData((ukk ) description);
		xpHudEvent.AddData((ukk ) descriptionParam);
		xpHudEvent.AddData(xp);		//XP given
		CHUDEventDispatcher::CallEvent(xpHudEvent);
	}
}

void SHUDEventWrapper::OnNewMedalMessage( tukk name, tukk description, tukk descriptionParam )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent newMedalEvent(eHUDEvent_OnNewMedalAward);
		newMedalEvent.AddData((ukk ) name);		     // title
		newMedalEvent.AddData((ukk ) description); // description
		newMedalEvent.AddData((ukk ) descriptionParam);
		CHUDEventDispatcher::CallEvent(newMedalEvent);
	}
}

void SHUDEventWrapper::OnPromotionMessage( tukk rankName, i32k rank, i32k xpRequired )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent newPromotionEvent(eHUDEvent_OnPromotionMessage);
		newPromotionEvent.AddData((ukk ) rankName);		     // rank name, will be localised with @pp_promoted.
		newPromotionEvent.AddData(rank);                           // rank id for icon.
		newPromotionEvent.AddData(xpRequired);
		CHUDEventDispatcher::CallEvent(newPromotionEvent);
	}
}

void SHUDEventWrapper::OnSupportBonusXPMessage( i32k xpType /*EPPType*/, i32k xpGained )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent xpHudEvent(eHUDEvent_OnNewSkillKill);
		xpHudEvent.AddData(xpType);   // type info EPPType
		xpHudEvent.AddData(xpGained); // XP Points
		xpHudEvent.AddData(true);
		CHUDEventDispatcher::CallEvent(xpHudEvent);
	}
}

void SHUDEventWrapper::OnSkillKillMessage( i32k xpType /*EPPType*/, i32k xpGained )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent skillKillEvent(eHUDEvent_OnNewSkillKill);
		skillKillEvent.AddData(xpType);   // type info EPPType
		skillKillEvent.AddData(xpGained); // XP Points
		skillKillEvent.AddData(false);
		CHUDEventDispatcher::CallEvent(skillKillEvent);
	}
}

// Set Message to NULL to clear.
void SHUDEventWrapper::OnGameStatusUpdate( const EGoodBadNeutralForLocalPlayer good, tukk message )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent gameSatusMessageEvent(eHUDEvent_OnSetGameStateMessage);
		gameSatusMessageEvent.AddData((i32)good);         // for colours/anims
		gameSatusMessageEvent.AddData((uk )message);    // XP Points
		CHUDEventDispatcher::CallEvent(gameSatusMessageEvent);
	}
}

void SHUDEventWrapper::OnRoundEnd( i32k winner, const bool clientScoreIsTop, tukk victoryDescMessage, tukk victoryMessage, const EAnnouncementID announcement )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent endRoundMessage(eHUDEvent_OnRoundEnd);
		endRoundMessage.AddData( SHUDEventData(winner) );
		endRoundMessage.AddData( SHUDEventData(clientScoreIsTop) );
		endRoundMessage.AddData( SHUDEventData(victoryDescMessage) );
		endRoundMessage.AddData( SHUDEventData(victoryMessage) );
		endRoundMessage.AddData( SHUDEventData((i32) announcement) );
		CHUDEventDispatcher::CallEvent(endRoundMessage);
	}
}

void SHUDEventWrapper::OnSuddenDeath()
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent suddenDeathMessage(eHUDEvent_OnSuddenDeath);
		CHUDEventDispatcher::CallEvent(suddenDeathMessage);
	}
}

void SHUDEventWrapper::OnGameEnd(i32k teamOrPlayerId, const bool clientScoreIsTop, EGameOverReason reason, tukk message, ESVC_DrawResolution drawResolution, const EAnnouncementID announcement)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent endGameMessage(eHUDEvent_GameEnded);
		endGameMessage.AddData(SHUDEventData(teamOrPlayerId));
		endGameMessage.AddData(SHUDEventData(clientScoreIsTop));
		endGameMessage.AddData(SHUDEventData((i32)reason));
		endGameMessage.AddData(SHUDEventData(message));
		endGameMessage.AddData(SHUDEventData((i32) drawResolution));
		endGameMessage.AddData(SHUDEventData((i32) announcement));
		CHUDEventDispatcher::CallEvent(endGameMessage);
	}
}

void SHUDEventWrapper::UpdateGameStartCountdown(const EPreGameCountdownType countdownType, const float timeTillStartInSeconds)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent prematchEvent;
		prematchEvent.eventType = eHUDEvent_OnUpdateGameStartMessage;
		prematchEvent.AddData(timeTillStartInSeconds);
		prematchEvent.AddData((i32)countdownType);
		CHUDEventDispatcher::CallEvent(prematchEvent);
	}
}

void SHUDEventWrapper::CantFire( void )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent cantfire( eHUDEvent_CantFire );
		CHUDEventDispatcher::CallEvent(cantfire);
	}
}

void SHUDEventWrapper::FireModeChanged( IWeapon* pWeapon, i32k currentFireMode, bool bForceFireModeUpdate /* = false */ )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent event(eHUDEvent_OnFireModeChanged);
		event.AddData(SHUDEventData((uk )pWeapon));
		event.AddData(SHUDEventData(currentFireMode));
		event.AddData(SHUDEventData(bForceFireModeUpdate));

		CHUDEventDispatcher::CallEvent(event);
	}
}

void SHUDEventWrapper::ForceCrosshairType( IWeapon* pWeapon, const ECrosshairTypes desiredCrosshairType )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent event(eHUDEvent_ForceCrosshairType);
		event.AddData(SHUDEventData((uk )pWeapon));
		event.AddData(SHUDEventData(desiredCrosshairType));

		CHUDEventDispatcher::CallEvent(event);
	}
}

void SHUDEventWrapper::OnInteractionUseHoldTrack(const bool bTrackingUse)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent interactionEvent(eHUDEvent_OnInteractionUseHoldTrack);
		interactionEvent.AddData(SHUDEventData(bTrackingUse));
		CHUDEventDispatcher::CallEvent(interactionEvent);
	}
}

void SHUDEventWrapper::OnInteractionUseHoldActivated(const bool bSuccessful)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent interactionEvent(eHUDEvent_OnInteractionUseHoldActivated);
		interactionEvent.AddData(SHUDEventData(bSuccessful));
		CHUDEventDispatcher::CallEvent(interactionEvent);
	}
}

void SHUDEventWrapper::InteractionRequest( const bool activate, tukk msg, tukk action, tukk actionmap, const float duration, const bool bGameRulesRequest, const bool bIsFlowNodeReq, const bool bShouldSerialize )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent interactionEvent(eHUDEvent_OnInteractionRequest);
		interactionEvent.AddData(SHUDEventData(activate));
		interactionEvent.AddData(SHUDEventData(msg));
		interactionEvent.AddData(SHUDEventData(action));
		interactionEvent.AddData(SHUDEventData(actionmap));
		interactionEvent.AddData(SHUDEventData(duration));
		interactionEvent.AddData(SHUDEventData(bGameRulesRequest));
		interactionEvent.AddData(SHUDEventData(bIsFlowNodeReq));
		interactionEvent.AddData(SHUDEventData(bShouldSerialize));
		CHUDEventDispatcher::CallEvent(interactionEvent);
	}
}

void SHUDEventWrapper::ClearInteractionRequest( const bool bGameRulesRequest ) // TODO: Remove this and all uses of it, should use ClearInteractionMsg
{
	if (!gEnv->IsDedicated())
	{
		SHUDEventWrapper::InteractionRequest( false, NULL, NULL, NULL, -1.0f, bGameRulesRequest );
	}
}

void SHUDEventWrapper::SetInteractionMsg(const EHUDInteractionMsgType interactionType, 
																										tukk message, 
																										const float duration, 
																										const bool bShouldSerialize, 
																										tukk interactionAction/*= NULL*/,
																										tukk interactionActionmap/* = NULL*/,
																										const EntityId usableEntityId/* = 0*/,
																										const EntityId usableSwapEntityId/* = 0*/,
																										const EInteractionType usableInteractionType/* = eInteraction_None*/,
																										tukk szExtraParam1/* = NULL*/)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent event(eHUDEvent_OnSetInteractionMsg);
		event.AddData(SHUDEventData((i32k)interactionType));
		event.AddData(SHUDEventData(message));
		event.AddData(SHUDEventData(duration));
		event.AddData(SHUDEventData(bShouldSerialize));
		event.AddData(SHUDEventData(interactionAction));
		event.AddData(SHUDEventData(interactionActionmap));
		event.AddData(SHUDEventData((i32k)usableEntityId));
		event.AddData(SHUDEventData((i32k)usableSwapEntityId));
		event.AddData(SHUDEventData((i32k)usableInteractionType));
		event.AddData(SHUDEventData(szExtraParam1));
	
		CHUDEventDispatcher::CallEvent(event);	
	}
}

void SHUDEventWrapper::ClearInteractionMsg(EHUDInteractionMsgType interactionType, tukk onlyRemoveMsg ,const float fadeOutTime /*= -1.0f*/)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent event(eHUDEvent_OnClearInteractionMsg);
		event.AddData(SHUDEventData((i32)interactionType));
		event.AddData(SHUDEventData(onlyRemoveMsg));
		event.AddData(SHUDEventData(fadeOutTime));
		CHUDEventDispatcher::CallEvent(event);	
	}
}

void SHUDEventWrapper::DisplayInfo( i32k system, const float time, tukk msg, tukk param1/*=NULL*/, tukk param2/*=NULL*/, tukk param3/*=NULL*/, tukk param4/*=NULL*/, const EInfoSystemPriority priority/*=eInfoPriority_Low*/)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent event(eHUDEvent_InfoSystemsEvent);
		event.AddData(SHUDEventData(system));
		event.AddData(SHUDEventData(time));
		event.AddData(SHUDEventData(msg));
		event.AddData(SHUDEventData(param1));
		event.AddData(SHUDEventData(param2));
		event.AddData(SHUDEventData(param3));
		event.AddData(SHUDEventData(param4));
		event.AddData(SHUDEventData((i32)priority));
		CHUDEventDispatcher::CallEvent(event);
	}
}

void SHUDEventWrapper::ClearDisplayInfo( i32k system, const EInfoSystemPriority priority/*=eInfoPriority_Low*/)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEventWrapper::DisplayInfo( system, 0.0f, NULL, NULL, NULL, NULL, NULL, priority);
	}
}

void SHUDEventWrapper::GenericBattleLogMessage(const EntityId actorId, tukk message, tukk p1 /*=NULL*/, tukk p2 /*=NULL*/, tukk p3 /*=NULL*/, tukk p4 /*=NULL*/ )
{
	if (!gEnv->IsDedicated())
	{
		string localisedString = CHUDUtils::LocalizeString(message, p1, p2, p3, p4);	// cache the string to ensure no subsequent LocalizeString() calls could mess with our result
	
		SHUDEvent genericMessageEvent(eHUDEvent_OnGenericBattleLogMessage);
		genericMessageEvent.AddData(static_cast<i32>(actorId));
		genericMessageEvent.AddData(localisedString.c_str());
		CHUDEventDispatcher::CallEvent(genericMessageEvent);
	}
}

void SHUDEventWrapper::HitTarget( EGameRulesTargetType targetType, i32 bulletType, EntityId targetId )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent hudEvent(eHUDEvent_OnHitTarget);
		hudEvent.AddData( static_cast<i32>(targetType) );
		hudEvent.AddData( bulletType );
		hudEvent.AddData( static_cast<i32>(targetId) );
		CHUDEventDispatcher::CallEvent(hudEvent);
	}
}

void SHUDEventWrapper::OnGameStateMessage( const EntityId actorId, const bool bIsFriendly, tukk pMessage, EBL_IconFrames icon)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent hudEvent(eHUDEvent_OnNewGameStateMessage);
		hudEvent.AddData(static_cast<i32>(actorId));
		hudEvent.AddData(bIsFriendly);
		hudEvent.AddData(static_cast<ukk>(pMessage));
		hudEvent.AddData(static_cast<i32>(icon));
		CHUDEventDispatcher::CallEvent(hudEvent);
	}
}

void SHUDEventWrapper::OnNewObjective( const EntityId entityId, const EGameRulesMissionObjectives iconType )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent hudEvent(eHUDEvent_OnNewObjective);
		hudEvent.AddData( static_cast<i32>(entityId) );
		hudEvent.AddData( static_cast<i32>(iconType) );
		DRX_ASSERT(hudEvent.GetDataSize() != 7);	//The HUD_Radar will need updating if this changes
		CHUDEventDispatcher::CallEvent(hudEvent);
	}
}

void SHUDEventWrapper::OnNewObjective( const EntityId entityId, const EGameRulesMissionObjectives iconType, const float progress, i32k priority )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent hudEvent(eHUDEvent_OnNewObjective);
		hudEvent.AddData( static_cast<i32>(entityId) );
		hudEvent.AddData( static_cast<i32>(iconType) );
		hudEvent.AddData( progress ); 
		hudEvent.AddData( priority );
		DRX_ASSERT(hudEvent.GetDataSize() != 7);	//The HUD_Radar will need updating if this changes
		CHUDEventDispatcher::CallEvent(hudEvent);
	}
}

void SHUDEventWrapper::OnNewObjective( const EntityId entityId, const EGameRulesMissionObjectives iconType, const float progress, i32k priority, tukk pNameOverride, tukk pColourStr )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent hudEvent(eHUDEvent_OnNewObjective);
		hudEvent.AddData( static_cast<i32>(entityId) );
		hudEvent.AddData( static_cast<i32>(iconType) );
		hudEvent.AddData( progress ); 
		hudEvent.AddData( priority );
		hudEvent.AddData( pNameOverride );
		hudEvent.AddData( pColourStr );
		DRX_ASSERT(hudEvent.GetDataSize() != 7);	//The HUD_Radar will need updating if this changes
		CHUDEventDispatcher::CallEvent(hudEvent);
	}
}

void SHUDEventWrapper::OnNewObjectiveWithRadarEntity( const EntityId entityId, const EntityId radarEntityId, const EGameRulesMissionObjectives iconType, const float progress, i32k priority, tukk pNameOverride, tukk pColourStr )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent hudEvent(eHUDEvent_OnNewObjective);
		hudEvent.AddData( static_cast<i32>(entityId) );
		hudEvent.AddData( static_cast<i32>(iconType) );
		hudEvent.AddData( progress ); 
		hudEvent.AddData( priority );
		hudEvent.AddData( pNameOverride );
		hudEvent.AddData( pColourStr );
		hudEvent.AddData( static_cast<i32>(radarEntityId) );
		DRX_ASSERT(hudEvent.GetDataSize() == 7);	//The HUD_Radar will need updating if this changes
		CHUDEventDispatcher::CallEvent(hudEvent);
	}
}

void SHUDEventWrapper::OnRemoveObjective( const EntityId entityId )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent hudEvent(eHUDEvent_OnRemoveObjective);
		hudEvent.AddData( static_cast<i32>(entityId) );
		DRX_ASSERT(hudEvent.GetDataSize() != 3);	//The HUD_Radar will need updating if this changes
		CHUDEventDispatcher::CallEvent(hudEvent);
	}
}

void SHUDEventWrapper::OnRemoveObjective( const EntityId entityId, i32k priority )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent hudEvent(eHUDEvent_OnRemoveObjective);
		hudEvent.AddData( static_cast<i32>(entityId) );
		hudEvent.AddData( priority );
		DRX_ASSERT(hudEvent.GetDataSize() != 3);	//The HUD_Radar will need updating if this changes
		CHUDEventDispatcher::CallEvent(hudEvent);
	}
}

void SHUDEventWrapper::OnRemoveObjectiveWithRadarEntity( const EntityId entityId, const EntityId radarEntityId, i32k priority )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent hudEvent(eHUDEvent_OnRemoveObjective);
		hudEvent.AddData( static_cast<i32>(entityId) );
		hudEvent.AddData( priority );
		hudEvent.AddData( static_cast<i32>(radarEntityId) );
		DRX_ASSERT(hudEvent.GetDataSize() == 3);	//The HUD_Radar will need updating if this changes
		CHUDEventDispatcher::CallEvent(hudEvent);
	}
}

void SHUDEventWrapper::OnNewGameRulesObjective( const EntityId entityId )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent hudEvent(eHUDEvent_OnNewGameRulesObjective);
		hudEvent.AddData( static_cast<i32>(entityId) );
		CHUDEventDispatcher::CallEvent(hudEvent);
	}
}

void SHUDEventWrapper::OnBootAction(u32k bootAction, tukk elementName)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent hudEvent(eHUDEvent_HUDBoot);
		hudEvent.AddData(static_cast<i32>(bootAction));
		if (elementName && elementName[0])
		{
			hudEvent.AddData((uk )elementName);
		}
		CHUDEventDispatcher::CallEvent(hudEvent);
	}
}

void SHUDEventWrapper::ActivateOverlay(i32k type)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent event(eHUDEvent_ActivateOverlay);
		event.AddData(SHUDEventData(type));
		CHUDEventDispatcher::CallEvent(event);
	}
}

void SHUDEventWrapper::DeactivateOverlay(i32k type)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent event(eHUDEvent_DeactivateOverlay);
		event.AddData(SHUDEventData(type));
		CHUDEventDispatcher::CallEvent(event);
	}
}

void SHUDEventWrapper::OnPrepareItemSelected(const EntityId itemId, const EItemCategoryType category, const EWeaponSwitchSpecialParam specialParam )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent eventPrepareItemSelected(eHUDEvent_OnPrepareItemSelected);
		eventPrepareItemSelected.AddData(SHUDEventData((i32)itemId));
		eventPrepareItemSelected.AddData(SHUDEventData((i32)category));
		eventPrepareItemSelected.AddData(SHUDEventData((i32)specialParam));
		CHUDEventDispatcher::CallEvent(eventPrepareItemSelected);
	}
}

void SHUDEventWrapper::PlayerRename(const EntityId playerId, const bool isReplay /*= false*/)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent renameEvent(eHUDEvent_RenamePlayer);
		renameEvent.AddData(SHUDEventData((i32)playerId));
		renameEvent.AddData(SHUDEventData(isReplay));
		CHUDEventDispatcher::CallEvent(renameEvent);	
	}
}

void SHUDEventWrapper::OnPlayerLinkedToVehicle( const EntityId playerId, const EntityId vehicleId, const bool alwaysShowHealth )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent linkEvent(eHUDEvent_PlayerLinkedToVehicle);
		linkEvent.AddData(SHUDEventData((i32)playerId));
		linkEvent.AddData(SHUDEventData((i32)vehicleId));
		linkEvent.AddData(SHUDEventData(alwaysShowHealth));
		CHUDEventDispatcher::CallEvent(linkEvent);		
	}
}

void SHUDEventWrapper::OnPlayerUnlinkedFromVehicle( const EntityId playerId, const EntityId vehicleId, const bool keepHealthBar )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent linkEvent(eHUDEvent_PlayerUnlinkedFromVehicle);
		linkEvent.AddData(SHUDEventData((i32)playerId));
		linkEvent.AddData(SHUDEventData((i32)vehicleId));
		linkEvent.AddData(SHUDEventData(keepHealthBar));
		CHUDEventDispatcher::CallEvent(linkEvent);
	}
}

void SHUDEventWrapper::OnStartTrackFakePlayerTagname( const EntityId tagEntityId, i32k sessionNameIdxToTrack )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent linkEvent(eHUDEvent_OnStartTrackFakePlayerTagname);
		linkEvent.AddData(SHUDEventData((i32)tagEntityId));
		linkEvent.AddData(SHUDEventData((i32)sessionNameIdxToTrack));
		CHUDEventDispatcher::CallEvent(linkEvent);
	}
}

void SHUDEventWrapper::OnStopTrackFakePlayerTagname( const EntityId tagEntityId )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent linkEvent(eHUDEvent_OnEndTrackFakePlayerTagname);
		linkEvent.AddData(SHUDEventData((i32)tagEntityId));
		CHUDEventDispatcher::CallEvent(linkEvent);
	}
}

void SHUDEventWrapper::PowerStruggleNodeStateChange(i32k activeNodeIdentityId, i32k nodeHUDState)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent nodeStateChange(eHUDEvent_OnPowerStruggle_NodeStateChange);
		nodeStateChange.AddData(SHUDEventData(activeNodeIdentityId));
		nodeStateChange.AddData(SHUDEventData(nodeHUDState));
		CHUDEventDispatcher::CallEvent(nodeStateChange);
	}
}

void SHUDEventWrapper::PowerStruggleManageCaptureBar(EHUDPowerStruggleCaptureBarType inType, float inChargeAmount, bool inContention, tukk inBarString)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent manageCaptureBar(eHUDEvent_OnPowerStruggle_ManageCaptureBar);
		manageCaptureBar.AddData(SHUDEventData(static_cast<i32>(inType)));
		manageCaptureBar.AddData(SHUDEventData(inChargeAmount));
		manageCaptureBar.AddData(SHUDEventData(inContention));
		manageCaptureBar.AddData(SHUDEventData(inBarString));
		CHUDEventDispatcher::CallEvent(manageCaptureBar);
	}
}

void SHUDEventWrapper::OnBigMessage(tukk inSubTitle, tukk inTitle)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent bigMessage(eHUDEvent_OnBigMessage);
		bigMessage.AddData(inSubTitle);
		bigMessage.AddData(inTitle);
		CHUDEventDispatcher::CallEvent(bigMessage);
	}
}

void SHUDEventWrapper::OnBigWarningMessage(tukk line1, tukk line2, const float duration)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent bigWarningMessage(eHUDEvent_OnBigWarningMessage);
		bigWarningMessage.AddData(line1);
		bigWarningMessage.AddData(line2);
		bigWarningMessage.AddData(duration);
		CHUDEventDispatcher::CallEvent(bigWarningMessage);
	}
}

void SHUDEventWrapper::OnBigWarningMessageUnlocalized(tukk line1, tukk line2, const float duration)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent bigWarningMessage(eHUDEvent_OnBigWarningMessageUnlocalized);
		bigWarningMessage.AddData(line1);
		bigWarningMessage.AddData(line2);
		bigWarningMessage.AddData(duration);
		CHUDEventDispatcher::CallEvent(bigWarningMessage);
	}
}

void SHUDEventWrapper::UpdatedDirectionIndicator(const EntityId &id)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent updatedDirectionIndicator(eHUDEvent_OnShowHitIndicatorBothUpdated);
		updatedDirectionIndicator.AddData((i32)id);
		CHUDEventDispatcher::CallEvent(updatedDirectionIndicator);
	}
}

void SHUDEventWrapper::SetStaticTimeLimit(bool active, i32 time)
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent staticTimeLimitMessage(eHUDEvent_OnSetStaticTimeLimit);
		staticTimeLimitMessage.AddData(active);
		staticTimeLimitMessage.AddData(time);
		CHUDEventDispatcher::CallEvent(staticTimeLimitMessage);
	}
}

void SHUDEventWrapper::PlayUnlockSound()
{
	if (!gEnv->IsDedicated())
		{
		if(!s_unlockSound.HasValidSignal())
		{
			s_unlockSound.SetSignal("HUD_CollectiblePickUp");
		}
		REINST("needs verification!");
		//s_unlockSound.Play();
	}
}

void SHUDEventWrapper::PlayUnlockSoundRare()
{
	if (!gEnv->IsDedicated())
	{
		if(!s_unlockSoundRare.HasValidSignal())
		{
			s_unlockSoundRare.SetSignal("HUD_CollectiblePickUpRare");
		}
		REINST("needs verification!");
		//s_unlockSoundRare.Play();
	}
}

void SHUDEventWrapper::DisplayWeaponUnlockMsg(tukk szCollectibleId)
{
	SHUDEventWrapper::PlayUnlockSound();
}

void SHUDEventWrapper::RadarSweepActivated( i32 targetTeam, float activateDuration )
{
	if (!gEnv->IsDedicated())
	{
		SHUDEvent radarSweepMessage(eHUDEvent_OnRadarSweep);
		radarSweepMessage.AddData(targetTeam);
		radarSweepMessage.AddData(activateDuration);
		CHUDEventDispatcher::CallEvent(radarSweepMessage);
	}
}
