// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание:  Register the factory templates used to create classes from names
e.g. REGISTER_FACTORY(pFramework, "Player", CPlayer, false);
or   REGISTER_FACTORY(pFramework, "Player", CPlayerG4, false);

Since overriding this function creates template based linker errors,
it's been replaced by a standalone function in its own cpp file.

-------------------------------------------------------------------------
История:
- 17:8:2005   Created by Nick Hesketh - Refactor'd from Game.cpp/h

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Game.h>
#include <drx3D/Game/Player.h>

//
#include <drx3D/Game/Item.h>
#include <drx3D/Game/Weapon.h>
#include <drx3D/Game/VehicleWeapon.h>
#include <drx3D/Game/VehicleWeaponGuided.h>
#include <drx3D/Game/VehicleWeaponControlled.h>
#include <drx3D/Game/VehicleMountedWeapon.h>
#include <drx3D/Game/Binocular.h>
#include <drx3D/Game/C4.h>
#include <drx3D/Game/DebugGun.h>
#include <drx3D/Game/GunTurret.h>
#include <drx3D/Game/JAW.h>
#include <drx3D/Game/AIGrenade.h>
#include <drx3D/Game/Accessory.h>
#include <drx3D/Game/HandGrenades.h>
#include <drx3D/Game/EnvironmentalWeapon.h>
#include <drx3D/Game/Laser.h>
#include <drx3D/Game/flashlight.h>
#include <drx3D/Game/DoubleMagazine.h>
#include <drx3D/Game/LTAG.h>
#include <drx3D/Game/HeavyMountedWeapon.h>
#include <drx3D/Game/HeavyWeapon.h>
#include <drx3D/Game/PickAndThrowWeapon.h>
#include <drx3D/Game/NoWeapon.h>
#include <drx3D/Game/WeaponMelee.h>
#include <drx3D/Game/UseableTurret.h>
#include <drx3D/Game/CinematicWeapon.h>

#include <drx3D/Game/DummyPlayer.h>

#include <drx3D/Game/ReplayObject.h>
#include <drx3D/Game/ReplayActor.h>

#include <drx3D/Game/MultiplayerEntities/CarryEntity.h>

#include <drx3D/Game/VehicleMovementBase.h>
#include <drx3D/Game/Vehicle/VehicleMovementDummy.h>
#include <drx3D/Game/VehicleActionAutomaticDoor.h>
#include <drx3D/Game/VehicleActionDeployRope.h>
#include <drx3D/Game/VehicleActionEntityAttachment.h>
#include <drx3D/Game/VehicleActionLandingGears.h>
#include <drx3D/Game/VehicleActionAutoTarget.h>
#include <drx3D/Game/VehicleDamageBehaviorBurn.h>
#include <drx3D/Game/VehicleDamageBehaviorCameraShake.h>
#include <drx3D/Game/VehicleDamageBehaviorExplosion.h>
#include <drx3D/Game/VehicleDamageBehaviorTire.h>
#include <drx3D/Game/VehicleDamageBehaviorAudioFeedback.h>
#include <drx3D/Game/VehicleMovementStdWheeled.h>
#include <drx3D/Game/VehicleMovementStdTank.h>
#include <drx3D/Game/VehicleMovementArcadeWheeled.h>
//#include <drx3D/Game/VehicleMovementHelicopterArcade.h>
#include <drx3D/Game/VehicleMovementHelicopter.h>
#include <drx3D/Game/VehicleMovementStdBoat.h>
#include <drx3D/Game/VehicleMovementTank.h>
#include <drx3D/Game/VehicleMovementMPVTOL.h>

#include <drx3D/Game/ScriptControlledPhysics.h>

#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/GameRulesModules/GameRulesModulesUpr.h>
#include <drx3D/Game/GameRulesModules/IGameRulesTeamsModule.h>
#include <drx3D/Game/GameRulesModules/GameRulesStandardTwoTeams.h>
#include <drx3D/Game/GameRulesModules/GameRulesGladiatorTeams.h>
#include <drx3D/Game/GameRulesModules/IGameRulesStateModule.h>
#include <drx3D/Game/GameRulesModules/GameRulesStandardState.h>
#include <drx3D/Game/GameRulesModules/GameRulesStandardVictoryConditionsTeam.h>
#include <drx3D/Game/GameRulesModules/GameRulesStandardVictoryConditionsPlayer.h>
#include <drx3D/Game/GameRulesModules/GameRulesObjectiveVictoryConditionsTeam.h>
#include <drx3D/Game/GameRulesModules/GameRulesObjectiveVictoryConditionsIndividualScore.h>
#include <drx3D/Game/GameRulesModules/GameRulesExtractionVictoryConditions.h>
#include <drx3D/Game/GameRulesModules/GameRulesSurvivorOneVictoryConditions.h>
#include <drx3D/Game/GameRulesModules/GameRulesStandardSetup.h>
#include <drx3D/Game/GameRulesModules/GameRulesStandardScoring.h>
#include <drx3D/Game/GameRulesModules/GameRulesAssistScoring.h>
#include <drx3D/Game/GameRulesModules/GameRulesStandardPlayerStats.h>
#include <drx3D/Game/GameRulesModules/IGameRulesSpawningModule.h>
#include <drx3D/Game/GameRulesModules/GameRulesSpawningBase.h>
#include <drx3D/Game/GameRulesModules/GameRulesMPSpawning.h>
#include <drx3D/Game/GameRulesModules/GameRulesMPSpawningWithLives.h>
#include <drx3D/Game/GameRulesModules/GameRulesMPWaveSpawning.h>
#include <drx3D/Game/GameRulesModules/GameRulesMPDamageHandling.h>
#include <drx3D/Game/GameRulesModules/GameRulesMPActorAction.h>
#include <drx3D/Game/GameRulesModules/GameRulesMPSpectator.h>
#include <drx3D/Game/GameRulesModules/GameRulesSPDamageHandling.h>
#include <drx3D/Game/GameRulesModules/GameRulesObjective_Predator.h>
#include <drx3D/Game/GameRulesModules/GameRulesStandardRounds.h>
#include <drx3D/Game/GameRulesModules/GameRulesStatsRecording.h>
#include <drx3D/Game/GameRulesModules/GameRulesObjective_PowerStruggle.h>
#include <drx3D/Game/GameRulesModules/GameRulesObjective_Extraction.h>
#include <drx3D/Game/GameRulesModules/GameRulesSimpleEntityBasedObjective.h>
#include <drx3D/Game/GameRulesModules/GameRulesObjective_CTF.h>

#include <drx3D/Game/Environment/Tornado.h>
#include <drx3D/Game/Environment/Shake.h>
#include <drx3D/Game/Environment/Rain.h>
#include <drx3D/Game/Environment/Snow.h>
#include <drx3D/Game/Environment/InteractiveObject.h>
#include <drx3D/Game/Environment/DeflectorShield.h>
#include <drx3D/Game/Environment/DangerousRigidBody.h>
#include <drx3D/Game/Environment/Ledge.h>
#include <drx3D/Game/Environment/WaterPuddle.h>
#include <drx3D/Game/Environment/SmartMine.h>
#include <drx3D/Game/Environment/MineField.h>
#include <drx3D/Game/Environment/DoorPanel.h>
#include <drx3D/Game/Environment/VicinityDependentObjectMover.h>
#include <drx3D/Game/Environment/WaterRipplesGenerator.h>
#include <drx3D/Game/Environment/LightningArc.h>

#include <drx3D/Game/AI/AICorpse.h>

#include <drx3D/Game/Turret/Turret/Turret.h>
#include <drx3D/Game/MPPath.h>

#include <drx3D/Act/IItemSystem.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/CoreX/Game/IGameVolumes.h>

#include <drx3D/Game/GameCVars.h>

#define HIDE_FROM_EDITOR(className)																																				\
  { IEntityClass *pItemClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(className);\
  pItemClass->SetFlags(pItemClass->GetFlags() | ECLF_INVISIBLE); }																				\

#define REGISTER_EDITOR_VOLUME_CLASS(frameWork, className)                                          \
{	                                                                                                  \
	IGameVolumes* pGameVolumes = frameWork->GetIGameVolumesUpr();                                 \
	IGameVolumesEdit* pGameVolumesEdit = pGameVolumes ? pGameVolumes->GetEditorInterface() : NULL;    \
	if (pGameVolumesEdit != NULL)                                                                     \
	{                                                                                                 \
		pGameVolumesEdit->RegisterEntityClass( className );                                             \
	}                                                                                                 \
} 

#define REGISTER_GAME_OBJECT(framework, name, script)\
	{\
	IEntityClassRegistry::SEntityClassDesc clsDesc;\
	clsDesc.sName = #name;\
	clsDesc.sScriptFile = script;\
struct C##name##Creator : public IGameObjectExtensionCreatorBase\
		{\
		IGameObjectExtensionPtr Create()\
			{\
			return ComponentCreate_DeleteWithRelease<C##name>();\
			}\
			void GetGameObjectExtensionRMIData( uk * ppRMI, size_t * nCount )\
			{\
			C##name::GetGameObjectExtensionRMIData( ppRMI, nCount );\
			}\
		};\
		static C##name##Creator _creator;\
		framework->GetIGameObjectSystem()->RegisterExtension(#name, &_creator, &clsDesc);\
	}

#define REGISTER_GAME_OBJECT_WITH_IMPL(framework, name, impl, script)\
	{\
	IEntityClassRegistry::SEntityClassDesc clsDesc;\
	clsDesc.sName = #name;\
	clsDesc.sScriptFile = script;\
struct C##name##Creator : public IGameObjectExtensionCreatorBase\
		{\
		IGameObjectExtensionPtr Create()\
			{\
			return ComponentCreate_DeleteWithRelease<C##impl>();\
			}\
			void GetGameObjectExtensionRMIData( uk * ppRMI, size_t * nCount )\
			{\
			C##impl::GetGameObjectExtensionRMIData( ppRMI, nCount );\
			}\
		};\
		static C##name##Creator _creator;\
		framework->GetIGameObjectSystem()->RegisterExtension(#name, &_creator, &clsDesc);\
	}

#define REGISTER_GAME_OBJECT_EXTENSION(framework, name)\
	{\
struct C##name##Creator : public IGameObjectExtensionCreatorBase\
		{\
		IGameObjectExtensionPtr Create()\
			{\
			return ComponentCreate_DeleteWithRelease<C##name>();\
			}\
			void GetGameObjectExtensionRMIData( uk * ppRMI, size_t * nCount )\
			{\
			C##name::GetGameObjectExtensionRMIData( ppRMI, nCount );\
			}\
		};\
		static C##name##Creator _creator;\
		framework->GetIGameObjectSystem()->RegisterExtension(#name, &_creator, NULL);\
	}

// Register the factory templates used to create classes from names. Called via CGame::Init()
void InitGameFactory(IGameFramework *pFramework)
{
	assert(pFramework);

	REGISTER_FACTORY(pFramework, "Player", CPlayer, false);
	REGISTER_FACTORY(pFramework, "PlayerHeavy", CPlayer, false);
	
	REGISTER_FACTORY(pFramework, "DamageTestEnt", CPlayer, true);

#if (USE_DEDICATED_INPUT)
	REGISTER_FACTORY(pFramework, "DummyPlayer", CDummyPlayer, true);
#endif

	//REGISTER_FACTORY(pFramework, "Civilian", CPlayer, true);

	// Null AI for AI pool
	REGISTER_FACTORY(pFramework, "NullAI", CPlayer, true);

	// Characters	
	REGISTER_FACTORY(pFramework, "Characters/Human", CPlayer, true);
	
	// Items
	REGISTER_FACTORY(pFramework, "Item", CItem, false);
	REGISTER_FACTORY(pFramework, "Accessory", CAccessory, false);
	REGISTER_FACTORY(pFramework, "Laser", CLaser, false);
	REGISTER_FACTORY(pFramework, "FlashLight", CFlashLight, false);
	REGISTER_FACTORY(pFramework, "DoubleMagazine", CDoubleMagazine, false);
	REGISTER_FACTORY(pFramework, "HandGrenades", CHandGrenades, false);

	// Weapons
	REGISTER_FACTORY(pFramework, "Weapon", CWeapon, false);
	REGISTER_FACTORY(pFramework, "VehicleWeapon", CVehicleWeapon, false);
	REGISTER_FACTORY(pFramework, "VehicleWeaponGuided", CVehicleWeaponGuided, false);
	REGISTER_FACTORY(pFramework, "VehicleWeaponControlled", CVehicleWeaponControlled, false);
	REGISTER_FACTORY(pFramework, "VehicleWeaponPulseC", CVehicleWeaponPulseC, false);
	REGISTER_FACTORY(pFramework, "VehicleMountedWeapon", CVehicleMountedWeapon, false);
	REGISTER_FACTORY(pFramework, "Binocular", CBinocular, false);
	REGISTER_FACTORY(pFramework, "C4", CC4, false);
	REGISTER_FACTORY(pFramework, "DebugGun", CDebugGun, false);
	REGISTER_FACTORY(pFramework, "GunTurret", CGunTurret, false);
	REGISTER_FACTORY(pFramework, "JAW", CJaw, false);
	REGISTER_FACTORY(pFramework, "AIGrenade", CAIGrenade, false);
	REGISTER_FACTORY(pFramework, "AISmokeGrenades", CAIGrenade, false);
	REGISTER_FACTORY(pFramework, "AIEMPGrenade", CAIGrenade, false);
	REGISTER_FACTORY(pFramework, "LTAG", CLTag, false);
	REGISTER_FACTORY(pFramework, "PickAndThrowWeapon", CPickAndThrowWeapon, false);
	REGISTER_FACTORY(pFramework, "NoWeapon", CNoWeapon, false);
	REGISTER_FACTORY(pFramework, "HeavyMountedWeapon", CHeavyMountedWeapon, false);
	REGISTER_FACTORY(pFramework, "HeavyWeapon", CHeavyWeapon, false);
	REGISTER_FACTORY(pFramework, "WeaponMelee", CWeaponMelee, false);
	REGISTER_FACTORY(pFramework, "UseableTurret", CUseableTurret, false);
	REGISTER_FACTORY(pFramework, "CinematicWeapon", CCinematicWeapon, false);
	
	// vehicle objects
	IVehicleSystem* pVehicleSystem = pFramework->GetIVehicleSystem();

#define REGISTER_VEHICLEOBJECT(name, obj) \
	REGISTER_FACTORY((IVehicleSystem*)pVehicleSystem, name, obj, false); \
	obj::m_objectId = pVehicleSystem->AssignVehicleObjectId(name);

	// damage behaviours
	REGISTER_VEHICLEOBJECT("Burn", CVehicleDamageBehaviorBurn);
	REGISTER_VEHICLEOBJECT("CameraShake", CVehicleDamageBehaviorCameraShake);
	REGISTER_VEHICLEOBJECT("Explosion", CVehicleDamageBehaviorExplosion);
	REGISTER_VEHICLEOBJECT("BlowTire", CVehicleDamageBehaviorBlowTire);
	REGISTER_VEHICLEOBJECT("AudioFeedback", CVehicleDamageBehaviorAudioFeedback);

	// actions
	REGISTER_VEHICLEOBJECT("AutomaticDoor", CVehicleActionAutomaticDoor);
	REGISTER_VEHICLEOBJECT("EntityAttachment", CVehicleActionEntityAttachment);
	REGISTER_VEHICLEOBJECT("LandingGears", CVehicleActionLandingGears);
	REGISTER_VEHICLEOBJECT("AutoAimTarget", CVehicleActionAutoTarget);

	//seat actions
	REGISTER_VEHICLEOBJECT("DeployRope", CVehicleActionDeployRope);

	// vehicle movements
	REGISTER_FACTORY(pVehicleSystem, "DummyMovement", CVehicleMovementDummy, false);
	//REGISTER_FACTORY(pVehicleSystem, "HelicopterArcade", CVehicleMovementHelicopterArcade, false);
	REGISTER_FACTORY(pVehicleSystem, "Helicopter", CVehicleMovementHelicopter, false);
	REGISTER_FACTORY(pVehicleSystem, "StdBoat", CVehicleMovementStdBoat, false);
	REGISTER_FACTORY(pVehicleSystem, "StdWheeled", CVehicleMovementStdWheeled, false);
	REGISTER_FACTORY(pVehicleSystem, "StdTank", CVehicleMovementStdTank, false);
	REGISTER_FACTORY(pVehicleSystem, "ArcadeWheeled", CVehicleMovementArcadeWheeled, false);
	REGISTER_FACTORY(pVehicleSystem, "Tank", CVehicleMovementTank, false);
	REGISTER_FACTORY(pVehicleSystem, "MPVTOL", CVehicleMovementMPVTOL, false);


	// Custom GameObjects
	REGISTER_GAME_OBJECT(pFramework, Tornado, "Scripts/Entities/Environment/Tornado.lua");
	REGISTER_GAME_OBJECT(pFramework, Shake, "Scripts/Entities/Environment/Shake.lua");
	REGISTER_GAME_OBJECT(pFramework, Rain, "Scripts/Entities/Environment/Rain.lua");
	REGISTER_GAME_OBJECT(pFramework, Snow, "Scripts/Entities/Environment/Snow.lua");
	REGISTER_GAME_OBJECT(pFramework, InteractiveObjectEx, "Scripts/Entities/PlayerInteractive/InteractiveObjectEx.lua");
	REGISTER_GAME_OBJECT(pFramework, DeployableBarrier, "Scripts/Entities/PlayerInteractive/DeployableBarrier.lua");
	REGISTER_GAME_OBJECT(pFramework, ReplayObject, "");
	REGISTER_GAME_OBJECT(pFramework, ReplayActor, "");
	REGISTER_GAME_OBJECT(pFramework, DeflectorShield, "Scripts/Entities/Others/DeflectorShield.lua");
	HIDE_FROM_EDITOR("DeflectorShield");
	REGISTER_GAME_OBJECT(pFramework, EnvironmentalWeapon, "Scripts/Entities/Multiplayer/EnvironmentWeapon_Rooted.lua");
	REGISTER_GAME_OBJECT(pFramework, DangerousRigidBody, "Scripts/Entities/Multiplayer/DangerousRigidBody.lua");
	REGISTER_GAME_OBJECT(pFramework, AICorpse, "");
	HIDE_FROM_EDITOR("ReplayObject");
	HIDE_FROM_EDITOR("ReplayActor");
	HIDE_FROM_EDITOR("AICorpse");
	HIDE_FROM_EDITOR("NullAI");

	//////////////////////////////////////////////////////////////////////////
	/// Shape/Volume objects
	REGISTER_GAME_OBJECT(pFramework, MPPath, "Scripts/Entities/Multiplayer/MPPath.lua");
	HIDE_FROM_EDITOR("MPPath");
	REGISTER_EDITOR_VOLUME_CLASS( pFramework, "MPPath" );

	REGISTER_GAME_OBJECT(pFramework, LedgeObject, "Scripts/Entities/ContextualNavigation/LedgeObject.lua");
	HIDE_FROM_EDITOR("LedgeObject");
	REGISTER_EDITOR_VOLUME_CLASS( pFramework, "LedgeObject" );
	REGISTER_GAME_OBJECT(pFramework, LedgeObjectStatic, "Scripts/Entities/ContextualNavigation/LedgeObjectStatic.lua");
	HIDE_FROM_EDITOR("LedgeObjectStatic");
	REGISTER_EDITOR_VOLUME_CLASS( pFramework, "LedgeObjectStatic" );

	REGISTER_GAME_OBJECT(pFramework, WaterPuddle, "Scripts/Entities/Environment/WaterPuddle.lua");
	HIDE_FROM_EDITOR("WaterPuddle");
	REGISTER_EDITOR_VOLUME_CLASS(pFramework, "WaterPuddle");
	//////////////////////////////////////////////////////////////////////////


	REGISTER_GAME_OBJECT(pFramework, SmartMine, "Scripts/Entities/Environment/SmartMine.lua");
	REGISTER_GAME_OBJECT(pFramework, MineField, "Scripts/Entities/Environment/MineField.lua");
	REGISTER_GAME_OBJECT(pFramework, DoorPanel, "Scripts/Entities/Environment/DoorPanel.lua");
	REGISTER_GAME_OBJECT(pFramework, VicinityDependentObjectMover, "Scripts/Entities/Environment/VicinityDependentObjectMover.lua");
	REGISTER_GAME_OBJECT(pFramework, WaterRipplesGenerator, "Scripts/Entities/Environment/WaterRipplesGenerator.lua");
	REGISTER_GAME_OBJECT(pFramework, LightningArc, "Scripts/Entities/Environment/LightningArc.lua");

	REGISTER_GAME_OBJECT_WITH_IMPL(pFramework, CTFFlag, CarryEntity, "Scripts/Entities/Multiplayer/CTFFlag.lua");
	
	REGISTER_GAME_OBJECT_WITH_IMPL(pFramework, Turret, Turret, "Scripts/Entities/Turret/Turret.lua");
	
	REGISTER_GAME_OBJECT_EXTENSION(pFramework, ScriptControlledPhysics);

	HIDE_FROM_EDITOR("CTFFlag");
	IEntityClassRegistry::SEntityClassDesc stdClass;
	stdClass.flags |= ECLF_INVISIBLE|ECLF_DEFAULT;
	stdClass.sName = "Corpse";
	gEnv->pEntitySystem->GetClassRegistry()->RegisterStdClass(stdClass);

	//GameRules
	REGISTER_FACTORY(pFramework, "GameRules", CGameRules, false);

	IGameRulesModulesUpr *pGameRulesModulesUpr = CGameRulesModulesUpr::GetInstance();

	REGISTER_FACTORY(pGameRulesModulesUpr, "StandardTwoTeams", CGameRulesStandardTwoTeams, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "GladiatorTeams", CGameRulesGladiatorTeams, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "StandardState", CGameRulesStandardState, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "StandardVictoryConditionsTeam", CGameRulesStandardVictoryConditionsTeam, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "StandardVictoryConditionsPlayer", CGameRulesStandardVictoryConditionsPlayer, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "ObjectiveVictoryConditionsTeam", CGameRulesObjectiveVictoryConditionsTeam, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "ObjectiveVictoryConditionsIndiv", CGameRulesObjectiveVictoryConditionsIndividualScore, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "ExtractionVictoryConditions", CGameRulesExtractionVictoryConditions, false);
#if SURVIVOR_ONE_ENABLED
	REGISTER_FACTORY(pGameRulesModulesUpr, "SurvivorOneVictoryConditions", CGameRulesSurvivorOneVictoryConditions, false);
#endif
	REGISTER_FACTORY(pGameRulesModulesUpr, "StandardSetup", CGameRulesStandardSetup, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "StandardScoring", CGameRulesStandardScoring, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "AssistScoring", CGameRulesAssistScoring, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "StandardPlayerStats", CGameRulesStandardPlayerStats, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "SpawningBase", CGameRulesSpawningBase, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "MPRSSpawning", CGameRulesRSSpawning, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "StandardStatsRecording", CGameRulesStatsRecording, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "MPSpawningWithLives", CGameRulesMPSpawningWithLives, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "MPWaveSpawning", CGameRulesMPWaveSpawning, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "MPDamageHandling", CGameRulesMPDamageHandling, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "MPActorAction", CGameRulesMPActorAction, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "MPSpectator", CGameRulesMPSpectator, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "SPDamageHandling", CGameRulesSPDamageHandling, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "Objective_Predator", CGameRulesObjective_Predator, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "StandardRounds", CGameRulesStandardRounds, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "Objective_PowerStruggle", CGameRulesObjective_PowerStruggle, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "Objective_Extraction", CGameRulesObjective_Extraction, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "Objective_SimpleEntityBased", CGameRulesSimpleEntityBasedObjective, false);
	REGISTER_FACTORY(pGameRulesModulesUpr, "Objective_CTF", CGameRulesObjective_CTF, false);

	pGameRulesModulesUpr->Init();
}
