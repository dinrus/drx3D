// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
	
	-------------------------------------------------------------------------
	История:
	- 07:09:2009  : Created by Ben Johnson

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameRulesMPActorAction.h>
#include <drx3D/Sys/XML/IXml.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/Actor.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/GameActions.h>
#include <drx3D/Game/IGameRulesSpawningModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesSpectatorModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesStateModule.h>
#include <drx3D/Game/GameRulesModules/GameRulesMPSpawning.h>
#include <drx3D/Game/UI/UIUpr.h>
#include <drx3D/Game/RecordingSystem.h>

CGameRulesMPActorAction::CGameRulesMPActorAction()
{
}

CGameRulesMPActorAction::~CGameRulesMPActorAction()
{
}

void CGameRulesMPActorAction::Init( XmlNodeRef xml )
{
}

void CGameRulesMPActorAction::PostInit()
{
}

void CGameRulesMPActorAction::OnActorAction(IActor *pActor, const ActionId& actionId, i32 activationMode, float value)
{
	CActor *pActorImpl = static_cast<CActor *>(pActor);
	if (pActorImpl)
	{
		EntityId  pid = pActor->GetEntityId();  // player id

		CGameRules *pGameRules = g_pGame->GetGameRules();
		IGameRulesSpectatorModule *specmod = pGameRules->GetSpectatorModule();

		if (!specmod || (pActorImpl->GetSpectatorMode() <= 0))
		{
			// Not in spectator mode

			if (pActorImpl->IsDead())
			{
				// if dead 

				CRecordingSystem *crs = g_pGame->GetRecordingSystem();

				if (crs != NULL && crs->IsPlayingBack())
				{
					// Recording system playing back 
					if (actionId == g_pGame->Actions().spectate_gen_skipdeathcam && g_pGameCVars->kc_canSkip )
						crs->StopPlayback();
				}
				else if ((actionId == g_pGame->Actions().spectate_gen_spawn || actionId == g_pGame->Actions().hud_mouseclick) && activationMode == eAAM_OnPress && pActorImpl->GetSpectatorState() != CActor::eASS_SpectatorMode)
				{
					// Revive requested.
					//	This may happen immediately or not at all.

					if (IGameRulesSpawningModule* pSpawningModule=pGameRules->GetSpawningModule())
					{
						IGameRulesStateModule*  stateModule = pGameRules->GetStateModule();
						if (!stateModule || (stateModule->GetGameState() != IGameRulesStateModule::EGRS_PostGame))
						{
							DrxLog("CGameRulesMPActorAction::OnActorAction() Requesting revive");
							pSpawningModule->ClRequestRevive(pActor->GetEntityId());
						}
					}
				}
				else if ((specmod != NULL && specmod->CanChangeSpectatorMode(pid)) && (((actionId == g_pGame->Actions().spectate_gen_nextmode) || (actionId == g_pGame->Actions().spectate_gen_prevmode)) && (activationMode == eAAM_OnPress)))  
				{
					// get into spectate mode
					if (!crs || !crs->IsPlayingOrQueued())
					{
						specmod->ChangeSpectatorModeBestAvailable(pActor, false);
					}
				}
			}
		}
		else
		{
			// is spectating

			i32  curspecmode = pActorImpl->GetSpectatorMode();
			i32  curspecstate = pActorImpl->GetSpectatorState();

			const CRecordingSystem* pRecordingSystem = g_pGame->GetRecordingSystem();
			// actions general across almost all spectator modes
			if( (curspecmode == CActor::eASM_Killer && !g_pGameCVars->g_killercam_canSkip) || (pRecordingSystem && pRecordingSystem->IsPlayingBack()) )
			{
				// Can't change mode or respawn-request, when in KillerCam mode or watching Killcam.
			}
			else if ((actionId == g_pGame->Actions().spectate_gen_spawn) && (activationMode == eAAM_OnPress) && pActorImpl->GetSpectatorState() != CActor::eASS_SpectatorMode)
			{
				IGameRulesSpawningModule *pSpawningModule = pGameRules->GetSpawningModule();
				if (pSpawningModule)
				{
					IGameRulesStateModule*  stateModule = pGameRules->GetStateModule();
					if (!stateModule || (stateModule->GetGameState() != IGameRulesStateModule::EGRS_PostGame))
					{
						DrxLog("CGameRulesMPActorAction::OnActorAction() Spectating, received spectate_gen_spawn action, requesting revive");
						pSpawningModule->ClRequestRevive(pActor->GetEntityId());
					}			
				}			
			}
			else if (((actionId == g_pGame->Actions().spectate_gen_nextmode) || (actionId == g_pGame->Actions().spectate_gen_prevmode)) && (activationMode == eAAM_OnPress))
			{
				DrxLog("[tlh] > changemode button pressed");
				if (specmod->CanChangeSpectatorMode(pid))
				{
					DrxLog("[tlh] > can change");
					i32  mode;
					EntityId  othEntId;
					mode = specmod->GetNextMode(pid, ((actionId == g_pGame->Actions().spectate_gen_nextmode) ? 1 : -1), &othEntId);
					if (mode != curspecmode)
					{
						DrxLog("[tlh] > changing to mode %d with othEnt %d",mode,othEntId);
						specmod->ChangeSpectatorMode(pActor, mode, othEntId, false);
					}
				}
			}
			else
			{
				// actions specific to individual spectator modes

				if (specmod->CanChangeSpectatorMode(pid))  // "CanChangeSpectatorMode?" is essentially "CanInteractWithSpectatorMode?" - ie. we don't want to be able to do any of this on the Join Game screen
				{
					if (curspecmode == CActor::eASM_Fixed)
					{
						i32  changeCam = 0;

						if (((actionId == g_pGame->Actions().spectate_cctv_nextcam) || (actionId == g_pGame->Actions().spectate_cctv_prevcam)) && (activationMode == eAAM_OnPress))
						{
							changeCam = (actionId == g_pGame->Actions().spectate_cctv_nextcam ? 1 : -1);
						}
						else if (actionId == g_pGame->Actions().spectate_cctv_changecam_xi)
						{
							if (value >= 1.f)
							{
								changeCam = 1;
							}
							else if (value <= -1.f)
							{
								changeCam = -1;
							}
						}

						if (changeCam != 0)
						{
							EntityId  locationId;

							if (changeCam > 0)
							{
								locationId = specmod->GetNextSpectatorLocation(pActorImpl);
							}
							else
							{
								locationId = specmod->GetPrevSpectatorLocation(pActorImpl);
							}

							pActorImpl->SetSpectatorFixedLocation(locationId);
						}
					}
					else if (curspecmode == CActor::eASM_Free)
					{
						;  // none
					}
					else if (curspecmode == CActor::eASM_Follow)
					{
						i32  change = 0;

						const CGameActions& actions = g_pGame->Actions();

						if (((actionId == actions.spectate_3rdperson_nextteammate) || (actionId == actions.spectate_3rdperson_prevteammate)) && (activationMode == eAAM_OnPress))
						{
							change = ((actionId == actions.spectate_3rdperson_nextteammate) ? 1 : -1);
						}
						else if (actionId == actions.spectate_3rdperson_changeteammate_xi)
						{
							if (value >= 1.f)
							{
								change = 1;
							}
							else if (value <= -1.f)
							{
								change = -1;
							}
						}
						else if(actionId == actions.xi_rotateyaw && pActorImpl->CanSpectatorOrbitYaw())
						{
							pActorImpl->SetSpectatorOrbitYawSpeed(fabs(value) > 0.2f ? value : 0.f, false);
						}
						else if(actionId == actions.rotateyaw && pActorImpl->CanSpectatorOrbitYaw())
						{
							pActorImpl->SetSpectatorOrbitYawSpeed(value * g_pGameCVars->g_spectate_follow_orbitMouseSpeedMultiplier, true);
						}
						else if(actionId == actions.xi_rotatepitch && pActorImpl->CanSpectatorOrbitPitch())
						{
							float val = fabs(value) > 0.2f ? value : 0.f;
							if(val && g_pGameCVars->cl_invertController)
							{
								val = -value;
							}
							pActorImpl->SetSpectatorOrbitPitchSpeed(val, false);
						}
						else if(actionId == actions.rotatepitch && pActorImpl->CanSpectatorOrbitPitch())
						{
							float val = fabs(value) > 0.2f ? value : 0.f;
							if(val && g_pGameCVars->cl_invertMouse)
							{
								val = -val;
							}
							pActorImpl->SetSpectatorOrbitPitchSpeed(val * g_pGameCVars->g_spectate_follow_orbitMouseSpeedMultiplier, true);
						}
						else if(actionId == actions.spectate_gen_nextcamera)
						{
							pActorImpl->ChangeCurrentFollowCameraSettings(true);
						}

						if (change != 0)
						{
							if (EntityId newTargetId=specmod->GetNextSpectatorTarget(pid, change))
							{
								pActorImpl->SetSpectatorTarget(newTargetId);
							}
						}
					}
					else if (curspecmode == CActor::eASM_Killer)
					{
						;  // none
					}
				}
			}
		}

		pGameRules->ActorActionInformOnAction(actionId, activationMode, value);
	}
}

