// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __GAMECVARS_H__
#define __GAMECVARS_H__

#include <drx3D/Game/GameConstantCVars.h>
#include <drx3D/Game/Network/Lobby/GameLobbyCVars.h>

#if PC_CONSOLE_NET_COMPATIBLE
	#define DEFAULT_MINPLAYERLIMIT 6
#else
	#define DEFAULT_MINPLAYERLIMIT 1
#endif

#if !defined(_RELEASE)
#define TALOS 1
#else
#define TALOS 0
#endif

#if PC_CONSOLE_NET_COMPATIBLE
#define USE_PC_PREMATCH 0
#else
#define USE_PC_PREMATCH 1
#endif


struct SCaptureTheFlagParams
{
	float carryingFlag_SpeedScale;
};

struct SExtractionParams
{
		float carryingTick_SpeedScale;
		float carryingTick_EnergyCostPerHit;
		float carryingTick_DamageAbsorbDesperateEnergyCost;
};

struct SPredatorParams
{
	float hudTimerAlertWhenTimeRemaining;
	float hintMessagePauseTime;
};

struct SDeathCamSPParams
{
	i32 enable;
	i32 dof_enable;

	float updateFrequency;
	float dofRange;
	float dofRangeNoKiller;
	float dofRangeSpeed;
	float dofDistanceSpeed;
};

struct SPowerSprintParams
{
	float foward_angle;
};

struct SJumpAirControl
{
	float air_control_scale;
	float air_resistance_scale;
	float air_inertia_scale;
};

struct SPlayerHealth
{
	float normal_regeneration_rateSP;
	float critical_health_thresholdSP;
	float critical_health_updateTimeSP;
	float normal_threshold_time_to_regenerateSP;

	float normal_regeneration_rateMP;
	float critical_health_thresholdMP;
	float fast_regeneration_rateMP;
	float slow_regeneration_rateMP;
	float normal_threshold_time_to_regenerateMP;

	i32   enable_FallandPlay;
	i32		collision_health_threshold;

	float fallDamage_SpeedSafe;
	float fallDamage_SpeedFatal;
	float fallSpeed_HeavyLand;
	float fallDamage_SpeedFatalArmor;
	float fallSpeed_HeavyLandArmor;
	float fallDamage_SpeedSafeArmorMP;
	float fallDamage_SpeedFatalArmorMP;
	float fallSpeed_HeavyLandArmorMP;
	float fallDamage_CurveAttackMP; 
	float fallDamage_CurveAttack;
	i32		fallDamage_health_threshold;
	i32		debug_FallDamage;
	i32		enableNewHUDEffect;
	i32		minimalHudEffect;
};

struct SAltNormalization
{
	i32 enable;
	float hud_ctrl_Curve_Unified;
	float hud_ctrl_Coeff_Unified;
};

struct SPlayerMovement
{
	float nonCombat_heavy_weapon_speed_scale;
	float nonCombat_heavy_weapon_sprint_scale;
	float nonCombat_heavy_weapon_strafe_speed_scale;
	float nonCombat_heavy_weapon_crouch_speed_scale;

	float power_sprint_targetFov;

	float ground_timeInAirToFall;

	float speedScale;
	float strafe_SpeedScale;
	float sprint_SpeedScale;
	float crouch_SpeedScale;

	i32 sprintStamina_debug;

	float mp_slope_speed_multiplier_uphill;
	float mp_slope_speed_multiplier_downhill;
	float mp_slope_speed_multiplier_minHill;
};

struct SPlayerMelee
{
	enum EImpulsesState
	{
		ei_Disabled,
		ei_OnlyToAlive,
		ei_OnlyToDead,
		ei_FullyEnabled,
	};

	float	melee_snap_angle_limit;
	float melee_snap_blend_speed;
	float melee_snap_target_select_range;
	float melee_snap_end_position_range;
	float melee_snap_move_speed_multiplier;
	float damage_multiplier_from_behind;
	float damage_multiplier_mp;
	float angle_limit_from_behind;
	float mp_victim_screenfx_intensity;
	float mp_victim_screenfx_duration;
	float mp_victim_screenfx_blendout_duration;
	float mp_victim_screenfx_dbg_force_test_duration;
	i32		impulses_enable;
	i32		debug_gfx;
	i32		mp_melee_system;
	i32		mp_melee_system_camera_lock_and_turn;
	i32		mp_knockback_enabled;
	float	mp_melee_system_camera_lock_time;
	float mp_melee_system_camera_lock_crouch_height_offset;
	float mp_knockback_strength_vert;
	float mp_knockback_strength_hor;
	i32		mp_sliding_auto_melee_enabled;
};

struct SSpectacularKillCVars
{
	float maxDistanceError;
	float minTimeBetweenKills;
	float minTimeBetweenSameKills;
	float minKillerToTargetDotProduct;
	float maxHeightBetweenActors;
	float sqMaxDistanceFromPlayer;
	i32		debug;
};

struct SPlayerLedgeClamber
{
	float cameraBlendWeight;
	i32		debugDraw;
	i32		enableVaultFromStanding;
};

struct SPlayerLadder
{
	i32 ladder_renderPlayerLast;
	i32	ladder_logVerbosity;
};

struct SPlayerPickAndThrow
{
	i32		debugDraw;
	i32		useProxies;
	i32   cloakedEnvironmentalWeaponsAllowed; 

	float maxOrientationCorrectionTime;
	float orientationCorrectionTimeMult;

#ifndef _RELEASE
	float correctOrientationDistOverride; 
	float correctOrientationPosOverrideX;
	float correctOrientationPosOverrideY;
	i32		correctOrientationPosOverrideEnabled;
	i32		chargedThrowDebugOutputEnabled; 
	i32		environmentalWeaponHealthDebugEnabled;
	i32   environmentalWeaponImpactDebugEnabled; 
	i32   environmentalWeaponComboDebugEnabled; 
	i32   environmentalWeaponRenderStatsEnabled;
	i32   environmentalWeaponDebugSwing;
#endif // #ifndef _RELEASE

	float environmentalWeaponObjectImpulseScale; 
	float environmentalWeaponImpulseScale;	
	float environmentalWeaponHitConeInDegrees; 
	float minRequiredThrownEnvWeaponHitVelocity; 
	float awayFromPlayerImpulseRatio; 
	float environmentalWeaponDesiredRootedGrabAnimDuration; 
	float environmentalWeaponDesiredGrabAnimDuration; 
	float environmentalWeaponDesiredPrimaryAttackAnimDuration;
	float environmentalWeaponDesiredComboAttackAnimDuration;
	float environmentalWeaponUnrootedPickupTimeMult;
	float environmentalWeaponThrowAnimationSpeed;
	float environmentalWeaponFlippedImpulseOverrideMult;
	float environmentalWeaponFlipImpulseThreshold; 
	float environmentalWeaponLivingToArticulatedImpulseRatio; 

	i32		enviromentalWeaponUseThrowInitialFacingOveride;

	// View
	float environmentalWeaponMinViewClamp; 
	float environmentalWeaponViewLerpZOffset;
	float environmentalWeaponViewLerpSmoothTime;

	// Env weap melee Auto aim
	float complexMelee_snap_angle_limit;
	float complexMelee_lerp_target_speed;

	// Object impulse helpers
	float objectImpulseLowMassThreshold; 
	float objectImpulseLowerScaleLimit;

	// Combo settings
	float comboInputWindowSize;	  
	float minComboInputWindowDurationInSecs; 

	// Impact helpers
	float impactNormalGroundClassificationAngle; 
	float impactPtValidHeightDiffForImpactStick; 
	float reboundAnimPlaybackSpeedMultiplier;
	i32   environmentalWeaponSweepTestsEnabled;

	// Charged throw auto aim
	float chargedThrowAutoAimDistance;
	float chargedThrowAutoAimConeSize;
	float chargedThrowAutoAimDistanceHeuristicWeighting; 
	float chargedThrowAutoAimAngleHeuristicWeighting;
	float chargedThrowAimHeightOffset;
	i32   chargedthrowAutoAimEnabled;

	i32		intersectionAssistDebugEnabled;
	i32		intersectionAssistDeleteObjectOnEmbed; 
	float intersectionAssistCollisionsPerSecond;
	float intersectionAssistTimePeriod;
	float intersectionAssistTranslationThreshold;
	float intersectionAssistPenetrationThreshold;
};

struct SPlayerSlideControl
{
	float min_speed_threshold;
	float min_speed;

	float deceleration_speed;
	float min_downhill_threshold;
	float max_downhill_threshold;
	float max_downhill_acceleration;
};

// this block of vars is no longer used. The lua code that was using it, is no longer being called - and probably need to be removed too - 
struct SPlayerEnemyRamming
{
	float player_to_player;
	float ragdoll_to_player;
	float fall_damage_threashold;
	float safe_falling_speed;
	float fatal_falling_speed;
	float max_falling_damage;
	float min_momentum_to_fall;
};

struct SAICollisions
{
	float minSpeedForFallAndPlay;
	float minMassForFallAndPlay;
	float dmgFactorWhenCollidedByObject;
	i32 showInLog;
};

//////////////////////////////////////////////////////////////////////////

struct SPostEffect
{	// Use same naming convention as the post effects in the engine
	float FilterGrain_Amount;
	float FilterRadialBlurring_Amount;
	float FilterRadialBlurring_ScreenPosX;
	float FilterRadialBlurring_ScreenPosY;
	float FilterRadialBlurring_Radius;
	float Global_User_ColorC;
	float Global_User_ColorM;
	float Global_User_ColorY;
	float Global_User_ColorK;
	float Global_User_Brightness;
	float Global_User_Contrast;
	float Global_User_Saturation;
	float Global_User_ColorHue;
	float HUD3D_Interference;
	float HUD3D_FOV;
};

struct SSilhouette
{
	i32			enableUpdate;
	float		r;
	float		g;
	float		b;
	float		a;
	i32			enabled;
};

struct SAIPerceptionCVars
{
	i32			movement_useSurfaceType;
	float		movement_movingSurfaceDefault;
	float		movement_standingRadiusDefault;
	float		movement_crouchRadiusDefault;
	float		movement_standingMovingMultiplier;
	float		movement_crouchMovingMultiplier;

	float		landed_baseRadius;
	float		landed_speedMultiplier;
};

struct SAIThreatModifierCVars
{
	char const* DebugAgentName;

	float SOMIgnoreVisualRatio;
	float SOMDecayTime;

	float SOMMinimumRelaxed;
	float SOMMinimumCombat;
	float SOMCrouchModifierRelaxed;
	float SOMCrouchModifierCombat;
	float SOMMovementModifierRelaxed;
	float SOMMovementModifierCombat;
	float SOMWeaponFireModifierRelaxed;
	float SOMWeaponFireModifierCombat;

	float SOMCloakMaxTimeRelaxed;
	float SOMCloakMaxTimeCombat;
	float SOMUncloakMinTimeRelaxed;
	float SOMUncloakMinTimeCombat;
	float SOMUncloakMaxTimeRelaxed;
	float SOMUncloakMaxTimeCombat;
};

struct SCVars
{	
	static const float v_altitudeLimitDefault()
	{
		return 600.0f;
	}
	
	SGameReleaseConstantCVars m_releaseConstants;

	float cl_fov;
	float cl_mp_fov_scalar;
	i32   cl_tpvCameraCollision;
	float cl_tpvCameraCollisionOffset;
	i32   cl_tpvDebugAim;
	float cl_tpvDist;
	float cl_tpvDistHorizontal;
	float cl_tpvDistVertical;
	float cl_tpvInterpolationSpeed;
	float cl_tpvMaxAimDist;
	float cl_tpvMinDist;
	float cl_tpvYaw;
	float cl_sensitivity;
	float cl_sensitivityController;
	float cl_sensitivityControllerMP;
	i32		cl_invertMouse;
	i32		cl_invertController;
	i32		cl_invertLandVehicleInput;
	i32		cl_invertAirVehicleInput;
	i32		cl_crouchToggle;
	i32		cl_sprintToggle;
	i32		cl_debugSwimming;
	i32		cl_logAsserts;
	i32		cl_zoomToggle;
	float	cl_motionBlurVectorScale;
	float	cl_motionBlurVectorScaleSprint;
	i32		g_enableMPDoubleTapGrenadeSwitch;

	ICVar* 	ca_GameControlledStrafingPtr;

	float cl_shallowWaterSpeedMulPlayer;
	float cl_shallowWaterSpeedMulAI;
	float cl_shallowWaterDepthLo;
	float cl_shallowWaterDepthHi;

	float cl_speedToBobFactor;
	float cl_bobWidth;
	float cl_bobHeight;
	float cl_bobSprintMultiplier;
	float cl_bobVerticalMultiplier;
	float cl_bobMaxHeight;
	float cl_strafeHorzScale;
	float cl_controllerYawSnapEnable;
	float cl_controllerYawSnapAngle;
	float cl_controllerYawSnapTime;
	float cl_controllerYawSnapMax;
	float cl_controllerYawSnapMin;
	float cl_angleLimitTransitionTime;
	
#ifndef _RELEASE
	i32 g_interactiveObjectDebugRender; 
#endif //#ifndef _RELEASE

	//Interactive Entity Highlighting
	float g_highlightingMaxDistanceToHighlightSquared;
	float g_highlightingMovementDistanceToUpdateSquared;
	float g_highlightingTimeBetweenForcedRefresh;

	float g_ledgeGrabClearHeight; 
	float g_ledgeGrabMovingledgeExitVelocityMult;
	float g_vaultMinHeightDiff; 
	float g_vaultMinAnimationSpeed;

	i32 g_cloakFlickerOnRun;
	
#ifndef _RELEASE
	i32 kc_debugMannequin;
	i32 kc_debugPacketData;

	i32		g_mptrackview_debug;
	i32		g_hud_postgame_debug;
#endif	// #ifndef _RELEASE

	i32		g_VTOLVehicleUprEnabled; 
	float g_VTOLMaxGroundVelocity;
	float g_VTOLDeadStateRemovalTime;
	float g_VTOLDestroyedContinueVelocityTime;
	float g_VTOLRespawnTime;
	float g_VTOLGravity;
	float g_VTOLPathingLookAheadDistance;
	float g_VTOLOnDestructionPlayerDamage;
	float g_VTOLInsideBoundsScaleX;
	float g_VTOLInsideBoundsScaleY;
	float g_VTOLInsideBoundsScaleZ;
	float g_VTOLInsideBoundsOffsetZ;

#ifndef _RELEASE
	i32 g_VTOLVehicleUprDebugEnabled; 
	i32 g_VTOLVehicleUprDebugPathingEnabled;
	i32 g_VTOLVehicleUprDebugSimulatePath;
	float g_VTOLVehicleUprDebugSimulatePathTimestep;
	float g_VTOLVehicleUprDebugSimulatePathMinDotValueCheck;
	i32 g_vtolUprDebug;

	i32 g_mpPathFollowingRenderAllPaths;
	float g_mpPathFollowingNodeTextSize;
#endif

	i32		kc_useAimAdjustment;
	float kc_aimAdjustmentMinAngle;
	float	kc_precacheTimeStartPos;
	float	kc_precacheTimeWeaponModels;

	i32 g_useQuickGrenadeThrow; 
	i32 g_debugWeaponOffset;

#ifndef _RELEASE
	i32 g_PS_debug;
	i32 kc_debugStream;
#endif

	i32 g_MicrowaveBeamStaticObjectMaxChunkThreshold;

	float i_fastSelectMultiplier;

	float	cl_idleBreaksDelayTime;

	i32		cl_postUpdateCamera;

	i32 p_collclassdebug;

#if !defined(_RELEASE)
	tukk pl_viewFollow;
	float pl_viewFollowOffset;
	float pl_viewFollowMinRadius;
	float pl_viewFollowMaxRadius;
	i32		pl_watch_projectileAimHelper;
#endif

	float pl_cameraTransitionTime;
	float pl_cameraTransitionTimeLedgeGrabIn;
	float pl_cameraTransitionTimeLedgeGrabOut;

	float	pl_slideCameraFactor;

	float	cl_slidingBlurRadius;
	float	cl_slidingBlurAmount;
	float	cl_slidingBlurBlendSpeed;

	i32   sv_votingTimeout;
	i32   sv_votingCooldown;
	i32   sv_votingEnable;
	i32   sv_votingMinVotes;
	float sv_votingRatio;
	float sv_votingTeamRatio;
	float sv_votingBanTime;
	

	i32   sv_input_timeout;

	ICVar*sv_aiTeamName;

	ICVar *performance_profile_logname;

	i32		g_infiniteAmmoTutorialMode;

	i32		i_lighteffects;
	i32		i_particleeffects;
	i32		i_rejecteffects;
	i32		i_grenade_showTrajectory;
	float	i_grenade_trajectory_resolution;
	float	i_grenade_trajectory_dashes;
	float	i_grenade_trajectory_gaps;
	i32		i_grenade_trajectory_useGeometry;

	i32		i_ironsight_while_jumping_mp;
	i32		i_ironsight_while_falling_mp;
	float	i_ironsight_falling_unzoom_minAirTime;
	float	i_weapon_customisation_transition_time;
	i32		i_highlight_dropped_weapons;

	float i_laser_hitPosOffset;

	float pl_inputAccel;
	i32		pl_debug_energyConsumption;
	i32		pl_debug_pickable_items;
	float	pl_useItemHoldTime;
	i32		pl_autoPickupItemsWhenUseHeld;
	float	pl_autoPickupMinTimeBetweenPickups;
	i32		pl_debug_projectileAimHelper; 

	float pl_nanovision_timeToRecharge;
	float pl_nanovision_timeToDrain;
	float pl_nanovision_minFractionToUse;

	float pl_refillAmmoDelay;

	i32		pl_spawnCorpseOnDeath;

	i32		pl_doLocalHitImpulsesMP;

	i32 kc_enable;
	i32 kc_debug;
	i32 kc_debugStressTest;
	i32 kc_debugVictimPos;
	i32 kc_debugWinningKill;
	i32 kc_debugSkillKill;
	i32 kc_memStats;
	i32 kc_maxFramesToPlayAtOnce;
	i32 kc_cameraCollision;
	i32 kc_showHighlightsAtEndOfGame;
	i32 kc_enableWinningKill;
	i32 kc_canSkip;
	float kc_length;
	float kc_skillKillLength;
	float kc_bulletSpeed;
	float kc_bulletHoverDist;
	float kc_bulletHoverTime;
	float kc_bulletHoverTimeScale;
	float kc_bulletPostHoverTimeScale;
	float kc_bulletTravelTimeScale;
	float kc_bulletCamOffsetX;
	float kc_bulletCamOffsetY;
	float kc_bulletCamOffsetZ;
	float kc_bulletRiflingSpeed;
	float kc_bulletZoomDist;
	float kc_bulletZoomTime;
	float kc_bulletZoomOutRatio;
	float kc_kickInTime;
	float kc_projectileDistance;
	float kc_projectileHeightOffset;
	float kc_largeProjectileDistance;
	float kc_largeProjectileHeightOffset;
	float kc_projectileVictimHeightOffset;
	float kc_projectileMinimumVictimDist;
	float kc_smoothing;
	float kc_grenadeSmoothingDist;
	float kc_cameraRaiseHeight;
	float kc_resendThreshold;
	float kc_chunkStreamTime;

#if !defined(_RELEASE)
	i32 kc_copyKillCamIntoHighlightsBuffer;
#endif

	float g_multikillTimeBetweenKills;
	float g_flushed_timeBetweenGrenadeBounceAndSkillKill;
	float g_gotYourBackKill_targetDistFromFriendly;
	float g_gotYourBackKill_FOVRange;
	float g_guardian_maxTimeSinceLastDamage;
	float g_defiant_timeAtLowHealth;
	float g_defiant_lowHealthFraction;
	float g_intervention_timeBetweenZoomedAndKill;
	float g_blinding_timeBetweenFlashbangAndKill;
	float g_blinding_flashbangRecoveryDelayFrac;
	float g_neverFlagging_maxMatchTimeRemaining;
	float g_combinedFire_maxTimeBetweenWeapons;

	float g_fovToRotationSpeedInfluence;
	
	i32		dd_maxRMIsPerFrame;
	float dd_waitPeriodBetweenRMIBatches;

	i32 g_debugSpawnPointsRegistration;
	i32 g_debugSpawnPointValidity;
	float g_randomSpawnPointCacheTime;

	i32		g_detachCamera;
	i32		g_moveDetachedCamera;
	float g_detachedCameraMoveSpeed;
	float g_detachedCameraRotateSpeed;
	float g_detachedCameraTurboBoost;
	i32 g_detachedCameraDebug;
	i32   g_difficultyLevel;
	i32   g_difficultyLevelLowestPlayed;
	float g_flashBangMinSpeedMultiplier;
	float g_flashBangSpeedMultiplierFallOffEase;
	float g_flashBangNotInFOVRadiusFraction;
	float g_flashBangMinFOVMultiplier;
	i32		g_flashBangFriends;
	i32		g_flashBangSelf;
	float	g_friendlyLowerItemMaxDistance;

	i32		g_holdObjectiveDebug;

#if !defined(_RELEASE)
	i32		g_debugOffsetCamera;
	i32		g_debugLongTermAwardDays;
#endif //!defined(_RELEASE)

	i32		g_gameRayCastQuota;
	i32		g_gameIntersectionTestQuota;

	i32		g_STAPCameraAnimation;

	i32   g_debugaimlook;
	float g_playerLowHealthThreshold;
	float g_playerMidHealthThreshold;

	i32 g_SurvivorOneVictoryConditions_watchLvl;
	i32 g_SimpleEntityBasedObjective_watchLvl;
	i32 g_CTFScoreElement_watchLvl;
	i32 g_KingOfTheHillObjective_watchLvl;
	float g_HoldObjective_secondsBeforeStartForSpawn;

	i32 g_CombiCaptureObjective_watchLvl;
	i32 g_CombiCaptureObjective_watchTerminalSignalPlayers;

	i32 g_disable_OpponentsDisconnectedGameEnd;
	i32 g_victoryConditionsDebugDrawResolution;
#if USE_PC_PREMATCH
	i32 g_restartWhenPrematchFinishes;
#endif

#ifndef _RELEASE
	i32 g_predator_forceSpawnTeam;
	i32 g_predator_predatorHasSuperRadar;
	ICVar *g_predator_forcePredator1;
	ICVar *g_predator_forcePredator2;
	ICVar *g_predator_forcePredator3;
	ICVar *g_predator_forcePredator4;
#endif

	float g_predator_marineRedCrosshairDelay;

	i32 sv_pacifist;
	i32 g_devDemo;

	i32 g_bulletPenetrationDebug;
	float g_bulletPenetrationDebugTimeout;

	i32 g_fpDbaManagementEnable;
	i32 g_fpDbaManagementDebug;
	i32 g_charactersDbaManagementEnable;
	i32 g_charactersDbaManagementDebug;

	i32 g_thermalVisionDebug;

	float g_droppedItemVanishTimer;
	float g_droppedHeavyWeaponVanishTimer;

	i32		g_corpseUpr_maxNum;
	float g_corpseUpr_thermalHeatFadeDuration;
	float g_corpseUpr_thermalHeatMinValue;
	float g_corpseUpr_timeoutInSeconds;

	float g_explosion_materialFX_raycastLength;

	// explosion culling
	i32		g_ec_enable;
	float g_ec_radiusScale;
	float g_ec_volume;
	float g_ec_extent;
	i32		g_ec_removeThreshold;

	float g_radialBlur;

	float g_timelimit;
	float g_timelimitextratime;
	float g_roundScoreboardTime;
	float g_roundStartTime;
	i32		g_roundlimit;
	float g_friendlyfireratio;
	i32   g_revivetime; 
	i32   g_minplayerlimit;
	float g_hostMigrationResumeTime;
	i32   g_hostMigrationUseAutoLobbyMigrateInPrivateGames;
	i32   g_minPlayersForRankedGame;
	float g_gameStartingMessageTime;

	i32		g_mpRegenerationRate;
	i32		g_mpHeadshotsOnly;
	i32		g_mpNoVTOL;
	i32		g_mpNoEnvironmentalWeapons;
	i32		g_allowCustomLoadouts;
	i32		g_allowFatalityBonus;
	float g_spawnPrecacheTimeBeforeRevive;
	float g_autoReviveTime;
	float g_spawn_timeToRetrySpawnRequest;
	float g_spawn_recentSpawnTimer;
	float g_forcedReviveTime;
	i32		g_numLives;
	i32		g_autoAssignTeams;
	float g_maxHealthMultiplier;
	i32		g_mp_as_DefendersMaxHealth;
	float g_xpMultiplyer;
	i32   g_allowExplosives;
	i32   g_forceWeapon;
	i32		g_allowSpectators;
	i32		g_infiniteCloak;
	i32		g_allowWeaponCustomisation;
	ICVar*	g_forceHeavyWeapon;
	ICVar*  g_forceLoadoutPackage;

	i32 g_switchTeamAllowed;
	i32 g_switchTeamRequiredPlayerDifference;
	i32 g_switchTeamUnbalancedWarningDifference;
	float g_switchTeamUnbalancedWarningTimer;

	i32   g_tk_punish;
	i32		g_tk_punish_limit;

	float g_hmdFadeoutNearWallThreshold;

	i32   g_debugNetPlayerInput;
	i32   g_debugCollisionDamage;
	i32   g_debugHits;
	i32   g_suppressHitSanityCheckWarnings;

#ifndef _RELEASE
	i32		g_debugFakeHits;
	i32		g_FEMenuCacheSaveList;
#endif

	i32		g_enableLanguageSelectionMenu;
	i32		g_multiplayerDefault;
	i32		g_multiplayerModeOnly;
	i32		g_EPD;
	i32		g_frontEndRequiredEPD;
	i32		g_EnableDevMenuOptions;
	i32		g_frontEndUnicodeInput;
	i32		g_DisableMenuButtons;
	i32		g_EnablePersistantStatsDebugScreen;
	i32		g_craigNetworkDebugLevel;
	i32		g_presaleUnlock;
	i32		g_dlcPurchaseOverwrite;
	i32		g_MatchmakingVersion;
	i32		g_MatchmakingBlock;
	i32		g_enableInitialLoadoutScreen;
	i32		g_post3DRendererDebug;
	i32		g_post3DRendererDebugGridSegmentCount;

	i32		g_ProcessOnlineCallbacks;
#if !defined(_RELEASE)
	i32		g_debugTestProtectedScripts;
	i32		g_preloadUIAssets;

	i32   g_gameRules_skipStartTimer;
	
	i32		g_skipStartupSignIn;	
#endif

#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
	#define g_gameRules_startCmd		g_gameRules_startCmdCVar->GetString()
	ICVar*	g_gameRules_startCmdCVar;
#endif
	float g_gameRules_startTimerLength;

	float g_gameRules_postGame_HUDMessageTime;
	float g_gameRules_postGame_Top3Time;
	float g_gameRules_postGame_ScoreboardTime;

	i32		g_gameRules_startTimerMinPlayers;
	i32		g_gameRules_startTimerMinPlayersPerTeam;
	float	g_gameRules_startTimerPlayersRatio;
	float	g_gameRules_startTimerOverrideWait;
	i32		g_gameRules_preGame_StartSpawnedFrozen;

	i32		g_debug_fscommand;
	i32		g_skipIntro;
	i32		g_skipAfterLoadingScreen;
	i32		g_goToCampaignAfterTutorial;

	i32   g_aiCorpses_Enable;
	i32   g_aiCorpses_DebugDraw;
	i32   g_aiCorpses_MaxCorpses;
	float	g_aiCorpses_DelayTimeToSwap;

	float g_aiCorpses_CullPhysicsDistance;
	float g_aiCorpses_ForceDeleteDistance;

	i32		g_scoreLimit;
	i32		g_scoreLimitOverride;

	float g_spawn_explosiveSafeDist;

	i32 g_logPrimaryRound;

#if USE_REGION_FILTER
	i32 g_server_region;
#endif

	i32 g_enableInitialLoginSilent;
	float g_dataRefreshFrequency;
	i32		g_maxGameBrowserResults;


	//Inventory control
	i32		g_inventoryNoLimits;
	i32   g_inventoryWeaponCapacity;
	i32		g_inventoryExplosivesCapacity;
	i32		g_inventoryGrenadesCapacity;
	i32		g_inventorySpecialCapacity;

#if !defined(_RELEASE)
	i32		g_keepMPAudioSignalsPermanently;
#endif

	i32 g_loadoutServerControlled;
#if !defined(_RELEASE)
	i32 g_debugShowGainedAchievementsOnHUD;
#endif

	ICVar*pl_debug_filter;
	i32		pl_debug_vistable;
	i32		pl_debug_movement;
	i32		pl_debug_jumping;
	i32		pl_debug_aiming;
	i32		pl_debug_aiming_input;
	i32		pl_debug_view;
	i32		pl_debug_hit_recoil;
	i32		pl_debug_look_poses;

	i32		pl_renderInNearest;

#if !defined(_RELEASE)
	i32     pl_debug_vistableIgnoreEntities;
	i32     pl_debug_vistableAddEntityId;
	i32     pl_debug_vistableRemoveEntityId;
	i32		pl_debug_watch_camera_mode;
	i32		pl_debug_log_camera_mode_changes;
	i32     pl_debug_log_player_plugins;
	i32 pl_shotDebug;
#endif

	i32 pl_aim_assistance_enabled;
	i32 pl_aim_assistance_disabled_atDifficultyLevel;
	i32 pl_aim_acceleration_enabled;
	float pl_aim_cloaked_multiplier;
	float pl_aim_near_lookat_target_distance;
	i32 pl_targeting_debug;

	i32 pl_switchTPOnKill;
	i32 pl_stealthKill_allowInMP;
	i32 pl_stealthKill_uncloakInMP;
	i32 pl_stealthKill_debug;
	float pl_stealthKill_aimVsSpineLerp;
	float pl_stealthKill_maxVelocitySquared;
	i32 pl_slealth_cloakinterference_onactionMP;
	i32		pl_stealthKill_usePhysicsCheck; 
	i32		pl_stealthKill_useExtendedRange;

	float pl_stealth_shotgunDamageCap;
	float pl_shotgunDamageCap;

	float pl_freeFallDeath_cameraAngle;
	float pl_freeFallDeath_fadeTimer;

	float pl_fall_intensity_multiplier;
	float pl_fall_intensity_max;
	float pl_fall_time_multiplier;
	float pl_fall_time_max;
	float pl_fall_intensity_hit_multiplier;
	
	float pl_TacticalScanDuration;
	float pl_TacticalScanDurationMP;
	float pl_TacticalTaggingDuration;
	float pl_TacticalTaggingDurationMP;

	float controller_power_curve;
	float controller_multiplier_z;
	float controller_multiplier_x;
	float controller_full_turn_multiplier_x;
	float controller_full_turn_multiplier_z;

	i32 ctrlr_corner_smoother;
	i32 ctrlr_corner_smoother_debug;

	i32 ctrlr_OUTPUTDEBUGINFO;

	float pl_stampTimeout;
	i32		pl_stampTier;

	float pl_jump_maxTimerValue;
	float pl_jump_baseTimeAddedPerJump;
	float pl_jump_currentTimeMultiplierOnJump;

	i32		pl_boostedMelee_allowInMP;

	float pl_velocityInterpAirControlScale;
	i32 pl_velocityInterpSynchJump;
	i32 pl_debugInterpolation;
	float pl_velocityInterpAirDeltaFactor;
	float pl_velocityInterpPathCorrection;
	i32 pl_velocityInterpAlwaysSnap;
	i32 pl_adjustJumpAngleWithFloorNormal;

	float pl_netAimLerpFactor;
	float pl_netSerialiseMaxSpeed;

#ifndef _RELEASE
	i32 pl_watchPlayerState;
#endif

	i32 pl_serialisePhysVel;
	float pl_clientInertia;
	float pl_fallHeight;

	float pl_legs_colliders_dist;
	float pl_legs_colliders_scale;

	float g_manualFrameStepFrequency;

	SPowerSprintParams	 pl_power_sprint;
	SJumpAirControl pl_jump_control;
	SPlayerHealth pl_health;
	SPlayerMovement pl_movement;
	SPlayerLedgeClamber pl_ledgeClamber;
	SPlayerLadder pl_ladderControl;
	SPlayerPickAndThrow pl_pickAndThrow;
	SPlayerSlideControl	 pl_sliding_control;
	SPlayerSlideControl	 pl_sliding_control_mp;
	SPlayerEnemyRamming pl_enemy_ramming;
	SAICollisions AICollisions;	
	SPlayerMelee		pl_melee;
	SAltNormalization aim_altNormalization;
	SCaptureTheFlagParams mp_ctfParams;
	SExtractionParams mp_extractionParams;
	SPredatorParams mp_predatorParams;

	// animation triggered footsteps
	i32   g_FootstepSoundsFollowEntity;
	i32   g_FootstepSoundsDebug;
	float g_footstepSoundMaxDistanceSq;

#if !defined(_RELEASE)
	i32   v_debugMovement;
	float v_debugMovementMoveVertically;
	float v_debugMovementX;
	float v_debugMovementY;
	float v_debugMovementZ;
	float v_debugMovementSensitivity;
#endif

	i32   v_tankReverseInvertYaw;
	i32   v_profileMovement;  
	tukk v_profile_graph;
	i32   v_draw_suspension;
	i32   v_draw_slip;
	i32   v_pa_surface;    
	i32   v_invertPitchControl;  
	float v_wind_minspeed; 
	float v_sprintSpeed;
	i32   v_rockBoats;
	i32   v_debugSounds;
	float v_altitudeLimit;
	ICVar* pAltitudeLimitCVar;
	float v_altitudeLimitLowerOffset;
	float v_MPVTOLNetworkSyncFreq;
	float v_MPVTOLNetworkSnapThreshold;
	float v_MPVTOLNetworkCatchupSpeedLimit;
	float v_stabilizeVTOL;
	ICVar* pVehicleQuality;
	float v_mouseRotScaleSP;
	float v_mouseRotLimitSP;
	float v_mouseRotScaleMP;
	float v_mouseRotLimitMP;

	float pl_swimBackSpeedMul;
	float pl_swimSideSpeedMul;
	float pl_swimVertSpeedMul;
	float pl_swimNormalSprintSpeedMul;
	i32   pl_swimAlignArmsToSurface;

	i32   pl_drownDamage;
	float pl_drownTime;
	float pl_drownRecoveryTime;
	float pl_drownDamageInterval;

	i32 pl_mike_debug;
	i32 pl_mike_maxBurnPoints;

	//--- Procedural impluse
	i32		pl_impulseEnabled;
	float pl_impulseDuration;
	i32   pl_impulseLayer;
	float pl_impulseFullRecoilFactor;
	float pl_impulseMaxPitch;
	float pl_impulseMaxTwist;
	float pl_impulseCounterFactor;
#ifndef _RELEASE
	i32		pl_impulseDebug;
#endif //_RELEASE

	i32   g_assertWhenVisTableNotUpdatedForNumFrames;

	float gl_time;
	float gl_waitForBalancedGameTime;

	i32		hud_ContextualHealthIndicator;            // TODO : Move to CHUDVars
	float hud_objectiveIcons_flashTime;             // TODO : Move to CHUDVars
	i32   hud_faderDebug;                           // TODO : Move to CHUDVars
	i32		hud_ctrlZoomMode;                         // TODO : Move to CHUDVars
	i32		hud_aspectCorrection;                     // TODO : Move to CHUDVars
	float hud_canvas_width_adjustment;

	i32		hud_colorLine;                            // Menus only c-var
	i32		hud_colorOver;                            // Menus only c-var
	i32		hud_colorText;                            // Menus only c-var
	i32		hud_subtitles;                            // Menus only c-var
	i32		hud_startPaused;                          // Menus only c-var
	i32	  hud_allowMouseInput;											// Menus only c-var

	i32		hud_psychoPsycho;

	i32 menu3D_enabled;
#ifndef _RELEASE
	i32 menu_forceShowPii;
#endif

	i32 g_flashrenderingduringloading;	
	i32 g_levelfadein_levelload;	
	i32 g_levelfadein_quickload;	

	float aim_assistMinDistance;
	float aim_assistMaxDistance;
	float aim_assistMaxDistanceTagged;
	float aim_assistFalloffDistance;
	float aim_assistInputForFullFollow_Ironsight;
	float aim_assistMinTurnScale;
	float aim_assistSlowFalloffStartDistance;
	float aim_assistSlowDisableDistance;
	float aim_assistSlowThresholdOuter;
	float aim_assistSlowDistanceModifier;
	float aim_assistSlowStartFadeinDistance;
	float aim_assistSlowStopFadeinDistance;
	float aim_assistStrength;
	float aim_assistSnapRadiusScale;
	float aim_assistSnapRadiusTaggedScale;

	float aim_assistStrength_IronSight;
	float aim_assistMaxDistance_IronSight;
	float aim_assistMinTurnScale_IronSight;

	float aim_assistStrength_SniperScope;
	float aim_assistMaxDistance_SniperScope;
	float aim_assistMinTurnScale_SniperScope;

	ICVar*i_debuggun_1;
	ICVar*i_debuggun_2;

	float	slide_spread;
	i32		i_debug_projectiles;
	i32		i_debug_weaponActions;
	i32		i_debug_spread;
	i32		i_debug_recoil;
	i32		i_auto_turret_target;
	i32		i_auto_turret_target_tacshells;
	i32		i_debug_zoom_mods;
	i32   i_debug_turrets;
	i32   i_debug_sounds;
	i32		i_debug_mp_flowgraph;
	i32		i_flashlight_has_shadows;
	i32		i_flashlight_has_fog_volume;

	i32		i_debug_itemparams_memusage;
	i32		i_debug_weaponparams_memusage;

	float i_failedDetonation_speedMultiplier;
	float i_failedDetonation_lifetime;

	float i_hmg_detachWeaponAnimFraction;
	float i_hmg_impulseLocalDirection_x;
	float i_hmg_impulseLocalDirection_y;
	float i_hmg_impulseLocalDirection_z;

	i32     g_displayIgnoreList;
	i32     g_buddyMessagesIngame;

	i32			g_enableFriendlyFallAndPlay;
	i32			g_enableFriendlyAIHits;
	i32			g_enableFriendlyPlayerHits;

	i32			g_mpAllSeeingRadar;
	i32			g_mpAllSeeingRadarSv;
	i32			g_mpDisableRadar;
	i32			g_mpNoEnemiesOnRadar;
	i32			g_mpHatsBootsOnRadar;

#if !defined(_RELEASE)
	i32			g_spectate_Debug;
	i32			g_spectate_follow_orbitEnable;
#endif //!defined(_RELEASE)
	i32			g_spectate_TeamOnly;
	i32			g_spectate_DisableManual;
	i32			g_spectate_DisableDead;
	i32			g_spectate_DisableFree;
	i32			g_spectate_DisableFollow;
	float   g_spectate_skipInvalidTargetAfterTime; 
	float		g_spectate_follow_orbitYawSpeedDegrees;
	i32			g_spectate_follow_orbitAlsoRotateWithTarget;
	float		g_spectate_follow_orbitMouseSpeedMultiplier;
	float		g_spectate_follow_orbitMinPitchRadians;
	float		g_spectate_follow_orbitMaxPitchRadians;

	i32			g_deathCam;

	i32			g_spectatorOnly;
	float		g_spectatorOnlySwitchCooldown;

	i32			g_forceIntroSequence;
	i32			g_IntroSequencesEnabled;

	SDeathCamSPParams	g_deathCamSP;

#ifndef _RELEASE
	float		g_tpdeathcam_dbg_gfxTimeout;
	i32			g_tpdeathcam_dbg_alwaysOn;
#endif
	float		g_tpdeathcam_timeOutKilled;
	float		g_tpdeathcam_timeOutSuicide;
	float		g_tpdeathcam_lookDistWhenNoKiller;
	float		g_tpdeathcam_camDistFromPlayerStart;
	float		g_tpdeathcam_camDistFromPlayerEnd;
	float		g_tpdeathcam_camDistFromPlayerMin;
	float		g_tpdeathcam_camHeightTweak;
	float		g_tpdeathcam_camCollisionRadius;
	float		g_tpdeathcam_maxBumpCamUpOnCollide;
	float		g_tpdeathcam_zVerticalLimit;
	float		g_tpdeathcam_testLenIncreaseRestriction;
	float		g_tpdeathcam_collisionEpsilon;
	float		g_tpdeathcam_directionalFocusGroundTestLen;
	float		g_tpdeathcam_camSmoothSpeed;
	float		g_tpdeathcam_maxTurn;

	i32			g_killercam_disable;
	float		g_killercam_displayDuration;
	float		g_killercam_dofBlurAmount;
	float		g_killercam_dofFocusRange;
	i32			g_killercam_canSkip;

	float		g_postkill_minTimeForDeathCamAndKillerCam;
	float		g_postkill_splitScaleDeathCam;

	i32			g_useHitSoundFeedback;
	i32			g_useSkillKillSoundEffects;
	i32			g_hasWindowFocus;

	i32			g_displayPlayerDamageTaken;
	i32			g_displayDbgText_hud;
	i32			g_displayDbgText_silhouettes;
	i32			g_displayDbgText_plugins;
	i32			g_displayDbgText_pmv;
	i32			g_displayDbgText_actorState;

	float		vehicle_steering_curve_scale;
	float		vehicle_steering_curve;
	float		vehicle_acceleration_curve_scale;
	float		vehicle_acceleration_curve;
	float		vehicle_deceleration_curve_scale;
	float		vehicle_deceleration_curve;

	i32			g_spawn_vistable_numLineTestsPerFrame;
	i32			g_spawn_vistable_numAreaTestsPerFrame;
	i32			g_showShadowChar;
	i32			g_infiniteAmmo;			// Used by MP private matches, is NOT a cheat variable

	float		g_persistantStats_gamesCompletedFractionNeeded;

	i32		  g_animatorDebug;
	i32			g_hideArms;
	i32			g_debugSmokeGrenades;
	float		g_smokeGrenadeRadius;
	float		g_empOverTimeGrenadeLife;

	i32			g_kickCarDetachesEntities;
	float		g_kickCarDetachStartTime;
	float		g_kickCarDetachEndTime;
	
#if !defined(_RELEASE)
	i32			g_DisableScoring;
	i32			g_DisableCollisionDamage;
	i32         g_LogDamage;
	i32         g_ProjectilePathDebugGfx;
#endif
	
#if (USE_DEDICATED_INPUT)
	i32			g_playerUsesDedicatedInput;
#endif

	i32 watch_enabled;
	float watch_text_render_start_pos_x;
	float watch_text_render_start_pos_y;
	float watch_text_render_size;
	float watch_text_render_lineSpacing;
	float watch_text_render_fxscale;

	i32 autotest_enabled;
	ICVar* autotest_state_setup;
	i32 autotest_quit_when_done;
	i32 autotest_verbose;
	i32 designer_warning_enabled;
	i32 designer_warning_level_resources;

#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
	ICVar* net_onlyListGameServersContainingText;
	ICVar* net_nat_type;
	i32    net_initLobbyServiceToLan;
#endif

	i32 g_teamDifferentiation;

	SPostEffect g_postEffect;
	i32 g_gameFXSystemDebug;
	i32 g_gameFXLightningProfile;
	i32 g_DebugDrawPhysicsAccess;

	i32 ai_DebugVisualScriptErrors;
	i32 ai_EnablePressureSystem;
	i32 ai_DebugPressureSystem;
	i32 ai_DebugAggressionSystem;
	i32 ai_DebugBattleFront;
	i32 ai_DebugSearch;
	i32 ai_DebugDeferredDeath;
	
	float ai_CloakingDelay;
	float ai_CompleteCloakDelay;
	float ai_UnCloakingDelay;

	i32 ai_HazardsDebug;

	i32 ai_SquadUpr_DebugDraw;
	float ai_SquadUpr_MaxDistanceFromSquadCenter;
	float ai_SquadUpr_UpdateTick;

	float ai_ProximityToHostileAlertnessIncrementThresholdDistance;

	i32 g_actorViewDistRatio;
	i32 g_playerLodRatio;
	float g_itemsLodRatioScale;
	float g_itemsViewDistanceRatioScale;

	// Hit Death Reactions CVars
	i32			g_hitDeathReactions_enable;
	i32			g_hitDeathReactions_useLuaDefaultFunctions;
	i32			g_hitDeathReactions_disable_ai;
	i32			g_hitDeathReactions_debug;
	i32			g_hitDeathReactions_disableRagdoll;
	i32			g_hitDeathReactions_usePrecaching;

	enum EHitDeathReactionsLogReactionAnimsType
	{
		eHDRLRAT_DontLog = 0,
		eHDRLRAT_LogAnimNames,
		eHDRLRAT_LogFilePaths,
	};
	i32			g_hitDeathReactions_logReactionAnimsOnLoading;

	enum EHitDeathReactionsStreamingPolicy
	{
		eHDRSP_Disabled = 0,
		eHDRSP_ActorsAliveAndNotInPool,		// the assets are locked if at least one of the actors using the profile is alive and not in the pool
		eHDRSP_EntityLifespanBased,				// the assets are requested/released whenever the entities using them are spawned/removed
	};
	i32			g_hitDeathReactions_streaming;
	// ~Hit Death Reactions CVars

	SSpectacularKillCVars g_spectacularKill;

	i32			g_movementTransitions_enable;
	i32			g_movementTransitions_log;
	i32			g_movementTransitions_debug;

	float		g_maximumDamage;
	float		g_instantKillDamageThreshold;

	i32 g_flyCamLoop;
#if (USE_DEDICATED_INPUT)
	i32 g_dummyPlayersFire;
	i32 g_dummyPlayersMove;
	i32 g_dummyPlayersChangeWeapon;
	float g_dummyPlayersJump;
	i32 g_dummyPlayersRespawnAtDeathPosition;
	i32 g_dummyPlayersCommitSuicide;
	i32 g_dummyPlayersShowDebugText;
	float g_dummyPlayersMinInTime;
	float g_dummyPlayersMaxInTime;
	float g_dummyPlayersMinOutTime;
	float g_dummyPlayersMaxOutTime;
	ICVar* g_dummyPlayersGameRules;
	i32 g_dummyPlayersRanked;
#endif // #if (USE_DEDICATED_INPUT)

	i32			g_muzzleFlashCull;
	float		g_muzzleFlashCullDistance;
	i32			g_rejectEffectVisibilityCull;
	float		g_rejectEffectCullDistance;
	i32 		g_mpCullShootProbablyHits;

	float		g_cloakRefractionScale;
	float		g_cloakBlendSpeedScale;

#if TALOS
	ICVar *pl_talos;
#endif

	i32 g_telemetry_onlyInGame;
	i32 g_telemetry_drawcall_budget;
	i32 g_telemetry_memory_display;
	i32 g_telemetry_memory_size_sp;
	i32 g_telemetry_memory_size_mp;
	i32 g_telemetry_gameplay_enabled;
	i32 g_telemetry_gameplay_save_to_disk;
	i32 g_telemetry_gameplay_gzip;
	i32 g_telemetry_gameplay_copy_to_global_heap;
	i32 g_telemetryEnabledSP;
	float g_telemetrySampleRatePerformance;
	float g_telemetrySampleRateBandwidth;
	float g_telemetrySampleRateMemory;
	float g_telemetrySampleRateSound;
	float g_telemetry_xp_event_send_interval;
	float g_telemetry_mp_upload_delay;
	tukk g_telemetryTags;
	tukk g_telemetryConfig;
	i32 g_telemetry_serialize_method;
	i32 g_telemetryDisplaySessionId;
	tukk g_telemetryEntityClassesToExport;

	i32  g_modevarivar_proHud;
	i32  g_modevarivar_disableKillCam;
	i32  g_modevarivar_disableSpectatorCam;
	
#if !defined(_RELEASE)
	#define g_dataCentreConfig		g_dataCentreConfigCVar->GetString()
	ICVar	*g_dataCentreConfigCVar;
	#define g_downloadMgrDataCentreConfig		g_downloadMgrDataCentreConfigCVar->GetString()
	ICVar *g_downloadMgrDataCentreConfigCVar;
#else
	#define g_dataCentreConfig		g_dataCentreConfigStr
	tukk g_dataCentreConfigStr;
	#define g_downloadMgrDataCentreConfig g_downloadMgrDataCentreConfigStr
	tukk g_downloadMgrDataCentreConfigStr;
#endif

	i32 g_ignoreDLCRequirements;
	float sv_netLimboTimeout;
	float g_idleKickTime;

	float g_stereoIronsightWeaponDistance;
  i32   g_stereoFrameworkEnable;

	i32	g_useOnlineServiceForDedicated;

#if USE_LAGOMETER
	i32 net_graph;
#endif
	i32 g_enablePoolCache;
	i32 g_setActorModelFromLua;
	i32 g_loadPlayerModelOnLoad;
	i32 g_enableActorLuaCache;

	i32 g_enableSlimCheckpoints;

	float g_mpLoaderScreenMaxTime;
	float g_mpLoaderScreenMaxTimeSoftLimit;

	i32 g_mpKickableCars;

	float g_forceItemRespawnTimer;
	float g_defaultItemRespawnTimer;

	float g_updateRichPresenceInterval;

#if defined(DEDICATED_SERVER)
	i32 g_quitOnNewDataFound;
	i32 g_quitNumRoundsWarning;
	i32 g_allowedDataPatchFailCount;
	float g_shutdownMessageRepeatTime;
	ICVar *g_shutdownMessage;
#endif

	i32 g_useNetSyncToSpeedUpRMIs;

#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
  i32 g_testStatus;
#endif

#if ENABLE_RMI_BENCHMARK

	i32 g_RMIBenchmarkInterval;
	i32 g_RMIBenchmarkTwoTrips;

#endif

	ICVar *g_presaleURL;
	ICVar *g_messageOfTheDay;
	ICVar *g_serverImageUrl;

#ifndef _RELEASE
	i32    g_disableSwitchVariantOnSettingChanges; 
	i32    g_startOnQuorum;
#endif //#ifndef _RELEASE

	SAIPerceptionCVars ai_perception;
	SAIThreatModifierCVars ai_threatModifiers;

	CGameLobbyCVars *m_pGameLobbyCVars;

	// 3d hud
	float g_hud3d_cameraDistance;
	float g_hud3d_cameraOffsetZ;
	i32 g_hud3D_cameraOverride;

	SCVars()
	{
		memset(this,0,sizeof(SCVars));
	}

	~SCVars() { ReleaseCVars(); }

	void InitCVars(IConsole *pConsole);
	void ReleaseCVars();
	
	void InitAIPerceptionCVars(IConsole *pConsole);
	void ReleaseAIPerceptionCVars(IConsole* pConsole);
};

#endif //__GAMECVARS_H__
