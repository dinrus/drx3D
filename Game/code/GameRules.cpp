// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$

	-------------------------------------------------------------------------
	История:
		- 7:2:2006   15:38 : Created by Marcio Martins

*************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/CoreX/TypeInfo_impl.h>
#include <drx3D/Game/ScriptBind_GameRules.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/Game.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/Game/Actor.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/Utility/DrxWatch.h>
#include <drx3D/Game/SmokeUpr.h>
#include <drx3D/Game/ActorUpr.h>

#include <drx3D/Game/IPlayerInput.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Game/WeaponSystem.h>
#include <drx3D/Game/GameCache.h>

#include <drx3D/Game/UI/UIUpr.h>
#include <drx3D/Game/UI/UICVars.h>
#include <drx3D/Game/UI/HUD/HUDEventWrapper.h>
#include <drx3D/Game/UI/HUD/HUDEventDispatcher.h>
#include <drx3D/Game/UI/HUD/HUDUtils.h>

#include <drx3D/Game/HitDeathReactionsSystem.h>
#include <drx3D/Game/CustomReactionFunctions.h>

#include <drx3D/Game/StatsRecordingMgr.h>
#include <drx3D/Game/ITelemetryCollector.h>

#include <drx3D/Game/GameActions.h>
#include <drx3D/Game/Audio/GameAudio.h>
#include <drx3D/CoreX/Game/IGameStatistics.h>

#include <drx3D/Game/Effects/ExplosionGameEffect.h>
#include <drx3D/Game/Effects/GameEffectsSystem.h>
#include <drx3D/Game/PersistantStats.h>
#include <drx3D/CoreX/String/UnicodeFunctions.h>
#include <drx3D/Eng3D/ITimeOfDay.h>

#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include <algorithm>

#include <drx3D/Game/Network/Lobby/GameBrowser.h>

#include <drx3D/Game/GameRulesModules/GameRulesModulesUpr.h>
#include <drx3D/Game/GameRulesModules/IGameRulesTeamsModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesStateModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesPlayerSetupModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesScoringModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesSpawningModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesActorActionModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesAssistScoringModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesDamageHandlingModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesPlayerStatsModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesSpectatorModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesVictoryConditionsModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesObjectivesModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesStatsRecording.h>

#include <drx3D/Game/EquipmentLoadout.h>

#include <drx3D/Game/GameRulesModules/IGameRulesPickupListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesClientConnectionListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesTeamChangedListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRevivedListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesSurvivorCountListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesKillListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesPlayerStatsListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesClientScoreListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesActorActionListener.h>
#include <drx3D/Game/GameRulesModules/GameRulesObjective_Predator.h>

#if USE_PC_PREMATCH
#include <drx3D/Game/GameRulesModules/IGameRulesPrematchListener.h>
#endif // #USE_PC_PREMATCH

#include <drx3D/Game/PlayerVisTable.h>

#include <drx3D/Game/AI/GameAISystem.h>

#include <drx3D/Game/RecordingSystem.h>

#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageUpr.h>
#include <drx3D/Game/GodMode.h>

#include <drx3D/Game/GameMechanismUpr.h>

#include <drx3D/Game/TelemetryCollector.h>
#include <drx3D/Game/LagOMeter.h>
#include <drx3D/Game/PlayerProgression.h>
#include <drx3D/Game/PersistantStats.h>
#include <drx3D/Game/Battlechatter.h>
#include <drx3D/Sys/IPerfHud.h>
#include <drx3D/Game/Audio/AreaAnnouncer.h>
#include <drx3D/Game/Audio/MiscAnnouncer.h>
#include <drx3D/Game/Projectile.h>
#include <drx3D/Game/Network/Lobby/GameLobby.h>
#include <drx3D/Game/UI/ProfileOptions.h>
#include <drx3D/Game/SkillKill.h>
#include <drx3D/Game/CorpseUpr.h>
#include <drx3D/Game/BodyUpr.h>
#include <drx3D/Game/PickAndThrowWeapon.h>

#include <drx3D/Game/Environment/LedgeUpr.h>
#include <drx3D/Game/PlaylistUpr.h>

#include <drx3D/Game/Utility/DesignerWarning.h>
#include <drx3D/Game/Utility/DrxDebugLog.h>
#include <drx3D/Game/Network/Lobby/GameAchievements.h>

#include <drx3D/Game/EntityUtility/EntityEffectsCloak.h>
#include <drx3D/Game/ClientHitEffectsMP.h>

#include <drx3D/Phys/IDeferredCollisionEvent.h>

#include <drx3D/Game/MPTrackViewUpr.h>
#include <drx3D/Game/MPPathFollowingUpr.h>
#include <drx3D/Game/VTOLVehicleUpr/VTOLVehicleUpr.h>
#include <drx3D/Game/TeamVisualizationUpr.h>
#include <drx3D/Game/Audio/Announcer.h>

#include <drx3D/Game/MultiplayerEntities/CarryEntity.h>

#include <drx3D/Game/PlayerMovementController.h>
#include <drx3D/Game/PlayerPlugin_InteractiveEntityMonitor.h>

#include <drx3D/Game/Effects/ParameterGameEffect.h>

#include <drx3D/Game/EnvironmentalWeapon.h>
#include <drx3D/Game/ReplayActor.h>

#include <drx3D/Game/UI/UIMultiPlayer.h>

#if NUM_ASPECTS > 8
	#define GAMERULES_LIMITS_ASPECT				eEA_GameServerC
	#define GAMERULES_TEAMS_SCORE_ASPECT	eEA_GameServerA
#else
	#define GAMERULES_LIMITS_ASPECT				eEA_GameServerStatic
	#define GAMERULES_TEAMS_SCORE_ASPECT	eEA_GameServerStatic
#endif

#define GAMERULES_TIME_OF_DAY_DYNAMIC_ASPECT eEA_GameServerDynamic

#ifdef _DEBUG
bool CGameRules::s_dbgAssertOnFailureToFindHitType = true;
#endif


enum EPrecacheType
{
	Precache_DBA,
	Precache_AG,
	Precache_CDF,
	Precache_CGF,
	Precache_CGA,
	Precache_CHR,
	Precache_Particle,
	Precache_Sound,
	Precache_AudioHint,
	Precache_VehicleXML,
	Precache_FlashTexs,
	Precache_Ammo,
	Precache_BodyDamage,
	Precache_GFX,
	Precache_ADB,
	Precache_ADBTagDefs,
	Precache_Item,
	Precache_TOTAL
};

tukk PRECACHE_TYPES[Precache_TOTAL] =
{
	"DBAs",
	"AGs",
	"CDFs",
	"CGFs",
	"CGAs",
	"CHRs",
	"Particles",
	"Sounds",
	"AudioHints",
	"VehicleXMLs",
	"FlashTexs",
	"Ammos",
	"BodyDamages",
	"GFXs",
	"ADBs",
	"ADBTagDefs",
	"Items",
};

tukk PRECACHE_LIST_XML = "Scripts/GameRules/PrecacheLists.xml";

AUTOENUM_BUILDNAMEARRAY(CGameRules::s_reservedHitTypes, RESERVED_HIT_TYPES);
AUTOENUM_BUILDNAMEARRAY(CGameRules::s_hitTypeFlags, HIT_TYPES_FLAGS);

//------------------------------------------------------------------------
AUTOENUM_BUILDNAMEARRAY(CGameRules::s_gameModeNames, AEGameModeList);

//------------------------------------------------------------------------

IEntityClass* CGameRules::s_pC4Explosive = NULL;	//used in GameRulesClientServer.cpp
IEntityClass* CGameRules::s_pSmartMineClass = NULL;	//used in GameRulesClientServer.cpp
IEntityClass* CGameRules::s_pTurretClass = NULL;	//used in GameRulesClientServer.cpp

CGameRules::CGameRules()
: m_pGameFramework(0),
	m_pGameplayRecorder(0),
	m_pSystem(0),
	m_pActorSystem(0),
	m_pEntitySystem(0),
	m_pScriptSystem(0),
	m_pMaterialUpr(0),
	m_pClientNetChannel(0),
	m_teamIdGen(0),
	m_hitTypeIdGen(EHitType::Unreserved),
	m_gameStartedTime(0.0f),
	m_gameStartTime(0.0f),
	m_cachedServerTime(0.0f),
	m_gamePausedTime(0LL),
	m_pBattlechatter(0),
	m_pAreaAnnouncer(0),
	m_pMiscAnnouncer(NULL),
	m_pExplosionGameEffect(0),
	m_pVotingSystem(0),
	m_ignoreEntityNextCollision(0),
	m_timeOfDayInitialized(false),
	m_processingHit(0),
	m_pMigratingPlayerInfo(NULL),
	m_migratingPlayerMaxCount(0),
	m_pHostMigrationParams(NULL),
	m_pHostMigrationClientParams(NULL),
	m_bPendingLoadoutChange(false),
	m_pVTOLVehicleUpr(NULL),
	m_pTeamVisualizationUpr(NULL),
	m_pCorpseUpr(NULL),
	m_pAnnouncer(NULL),
	m_pClientHitEffectsMP(NULL),
	m_levelLoaded(false),
	m_idleTime(0),
	m_numLocalPlayerRevives(0),
	m_bHasCalledEnteredGame(false),
	m_bCanUpdateSkillRanking(true),
	m_sessionStatisticsSaved(false),
	m_bClientTeamInLead(false),
	m_bLevelNameCheckNeeded(false),
	m_bIsTeamGame(false),
	m_bClientKickVoteActive(false),
	m_ClientCooldownEndTime(-1.f),
	m_bClientKickVoteSent(false),
	m_bClientKickVotedFor(false),
	m_mpTrackViewUpr(NULL),
	m_mpPathFollowingUpr(NULL),
	m_isRestarting(false),
	m_bIntroSequenceRegistered(false),
	m_bIntroCurrentlyPlaying(false),
	m_bIntroSequenceCompletedPlaying(false),
	m_gameStarted( false )
#if USE_PC_PREMATCH
	, m_numRequiredPlayers(0)
	, m_previousNumRequiredPlayers(-1)
	, m_finishPrematchTime(0.f)
	, m_prematchState(ePS_None)
#endif
{
// Initialise module pointers
#define GAMERULES_MODULE_LIST_FUNC(type, name, lowerCase, useInEditor) m_##lowerCase##Module = 0;
	GAMERULES_MODULE_TYPES_LIST
#undef GAMERULES_MODULE_LIST_FUNC

	GAME_FX_SYSTEM.LoadData();

	m_timeLimit = g_pGameCVars->g_timelimit;
	m_scoreLimit = g_pGameCVars->g_scoreLimitOverride ? g_pGameCVars->g_scoreLimitOverride : g_pGameCVars->g_scoreLimit;
	m_scoreLimitOverride = g_pGameCVars->g_scoreLimitOverride;
	m_roundLimit = g_pGameCVars->g_roundlimit;
	m_votingEnabled = (g_pGameCVars->sv_votingEnable != 0);
	m_votingCooldown = g_pGameCVars->sv_votingCooldown;
	m_votingMinVotes = g_pGameCVars->sv_votingMinVotes;
	m_votingRatio = g_pGameCVars->sv_votingRatio;

	//Pre-seed the hit types with the 'Invalid' one
	m_hitTypes.push_back(HitTypeInfo("Invalid", 0));

	m_pendingActorsToBeKnockedDown.reserve( 10 ); // arbitrary number, unlikely to reach that

	for(i32 i = 0; i < MAX_CONCURRENT_EXPLOSIONS; i++)
	{
		m_explosionValidities[i]	= false;
	}

	if (gEnv->bMultiplayer)
	{
		m_migratingPlayerMaxCount = MAX_PLAYER_LIMIT;

		if (gEnv->pConsole)
		{
			ICVar* pMaxPlayers = gEnv->pConsole->GetCVar("sv_maxplayers");
			if (pMaxPlayers)
			{
				m_migratingPlayerMaxCount = pMaxPlayers->GetIVal();
			}
		}
		m_pMigratingPlayerInfo = new SMigratingPlayerInfo[m_migratingPlayerMaxCount];
		DRX_ASSERT(m_pMigratingPlayerInfo != NULL);

		m_explosionFeedback.SetSignal("ExplosionFeedback");

		m_hostMigrationCachedEntities.reserve(128);

		if(!gEnv->IsDedicated())
		{
			CUIMultiPlayer* pUIEvt = UIEvents::Get<CUIMultiPlayer>();
			RegisterKillListener(pUIEvt);
		}
	}
	m_explosionSoundmoodEnter.SetSignal("Player_Explosion");
	m_explosionSoundmoodExit.SetSignal("Player_StopExplosionMood");

	ClearAllMigratingPlayers();

	SkillKill::Reset();

	g_pGame->GetHitDeathReactionsSystem().GetCustomReactionFunctions().InitCustomReactionsData();

	if (gEnv->pInput)
	{
		gEnv->pInput->AddEventListener(this);
	}

	m_hasWinningKill = false;

	m_svLastTeamDiscoCause[0] = m_svLastTeamDiscoCause[1] = eDC_Unknown;

	m_uSecurity = drx_random_uint32();

	if(gEnv->bServer)
		m_bSecurityInitialized = true;
	else
		m_bSecurityInitialized = false;

#if USE_PC_PREMATCH
	if (gEnv->bMultiplayer)
	{
		m_prematchState = ePS_Prematch;

		m_prematchAudioSignalPlayer.SetSignal("PregameLoop");
	}
	else
	{
		m_prematchState = ePS_Match;
	}
#endif
	s_pC4Explosive = gEnv->pEntitySystem->GetClassRegistry()->FindClass("c4explosive");
	s_pSmartMineClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("SmartMine");
	s_pTurretClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Turret");

	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener( this );
}

//------------------------------------------------------------------------
CGameRules::~CGameRules()
{
	CCCPOINT(GameRules_Destroyed);

	DrxLog("[GameRules] destructing %p", this);

	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener( this );

	SaveSessionStatistics();

	CTelemetryCollector* pTelemetryCollector = (CTelemetryCollector*)g_pGame->GetITelemetryCollector();
	if (pTelemetryCollector)
	{
		pTelemetryCollector->CloseEventStream();

		if (!gEnv->bMultiplayer)
			pTelemetryCollector->CloseStatoscopeStream();
	}

	if(gEnv->bMultiplayer && !gEnv->IsDedicated())
	{
		CUIMultiPlayer* pUIEvt = UIEvents::Get<CUIMultiPlayer>();
		UnRegisterKillListener(pUIEvt);
	}

	SAFE_DELETE_ARRAY(m_pMigratingPlayerInfo);
	SAFE_DELETE(m_pHostMigrationParams);
	SAFE_DELETE(m_pHostMigrationClientParams);

	CPersistantStats* pPersistantStats = CPersistantStats::GetInstance();
	if ( pPersistantStats )
	{
		pPersistantStats->SetInGame(false);
	}

	if( gEnv->IsClient() )
	{
		// Stop force feedback
		IForceFeedbackSystem* pForceFeedbackSystem = gEnv->pGame->GetIGameFramework()->GetIForceFeedbackSystem();
		if(pForceFeedbackSystem)
		{
			pForceFeedbackSystem->StopAllEffects();
		}
	}

	g_pGame->GetWeaponSystem()->GetTracerUpr().Reset();

	if (m_pGameFramework)
	{
		m_pGameFramework->GetIGameRulesSystem()->SetCurrentGameRules(0);

		if(m_pGameFramework->GetIViewSystem())
			m_pGameFramework->GetIViewSystem()->RemoveListener(this);
	}

	SAFE_DELETE(m_pBattlechatter);
	SAFE_DELETE(m_pAreaAnnouncer);
	SAFE_DELETE(m_pMiscAnnouncer);

	SAFE_DELETE_GAME_EFFECT(m_pExplosionGameEffect);

// Delete any modules
#define GAMERULES_MODULE_LIST_FUNC(type, name, lowerCase, useInEditor) SAFE_DELETE(m_##lowerCase##Module);
	GAMERULES_MODULE_TYPES_LIST
#undef GAMERULES_MODULE_LIST_FUNC

	g_pGame->GetIGameFramework()->UnregisterListener(this);
	gEnv->pNetwork->RemoveHostMigrationEventListener(this);

	CGameMechanismUpr::GetInstance()->Inform(kGMEvent_GameRulesDestroyed);

	ClearEntityTeams();

	if (g_pGame->GetHostMigrationState() != CGame::eHMS_NotMigrating)
	{
		// Quitting game mid-migration (probably caused by a failed migration), re-enable timers so that the game isn't paused if we join a new one!
		g_pGame->AbortHostMigration();
	}

	if (g_pGame->GetRecordingSystem())
	{
		g_pGame->GetRecordingSystem()->Reset();
	}

	if (CEquipmentLoadout *pEquipmentLoadout = g_pGame->GetEquipmentLoadout())
	{
		pEquipmentLoadout->OnGameEnded();
	}

	SAFE_DELETE(m_pVTOLVehicleUpr);
	SAFE_DELETE(m_pTeamVisualizationUpr);
	SAFE_DELETE(m_pCorpseUpr);
	SAFE_DELETE(m_pAnnouncer);
	SAFE_DELETE(m_pClientHitEffectsMP);
	SAFE_DELETE(m_mpTrackViewUpr);
	SAFE_DELETE(m_mpPathFollowingUpr);
	SAFE_DELETE(m_pVotingSystem);

	FreezeInput(false);
	g_pGameActions->FilterMPPreGameFreeze()->Enable(false);

	if (gEnv->pInput)
		gEnv->pInput->RemoveEventListener(this);

	// GAME_FX_SYSTEM must release its data after everything else (especially recording system), to give a chance
	// for all game effects to turn off (they might rely on data which will be released)
	GAME_FX_SYSTEM.ReleaseData();
}

//------------------------------------------------------------------------
bool CGameRules::Init( IGameObject * pGameObject )
{
	CCCPOINT(GameRules_Init);

	//Will always be valid
	CSmokeUpr::GetSmokeUpr()->Reset();
	CActorUpr::GetActorUpr()->Reset();

	g_pGame->GetPlayerVisTable()->Reset();

	SetGameObject(pGameObject);

	if (!GetGameObject()->BindToNetwork())
		return false;

	if (gEnv->bMultiplayer)
	{
		static i32 GAMERULES__s_round = 1;
		if (g_pGame->GetGameBrowser())
		{
			tukk natString = g_pGame->GetGameBrowser()->GetNatTypeString();
			DrxLog( "GameRules::Init natString=%s", natString);
		}
		if (gEnv->bServer)
		{
			DrxLog( "GameRules::Init SERVER");
		}
		else
		{
			DrxLog( "GameRules::Init CLIENT");
		}
		DrxLog( "Round %d", GAMERULES__s_round);
		GAMERULES__s_round++;
	}

	i32  modei;
	const bool  modeOk = AutoEnum_GetEnumValFromString(GetEntity()->GetClass()->GetName(), S_GetGameModeNamesArray(), eGM_NUM_GAMEMODES, &modei);
	DRX_ASSERT(modeOk);
	m_gameMode = (EGameMode) modei;
	DRX_ASSERT((m_gameMode > eGM_INVALID_GAMEMODE) && (m_gameMode < eGM_NUM_GAMEMODES));

	GetGameObject()->EnablePostUpdates(this);

	m_pGameFramework = g_pGame->GetIGameFramework();
	m_pGameplayRecorder = m_pGameFramework->GetIGameplayRecorder();
	m_pSystem = m_pGameFramework->GetISystem();
	m_pActorSystem = m_pGameFramework->GetIActorSystem();
	m_pEntitySystem = gEnv->pEntitySystem;
	m_pScriptSystem = m_pSystem->GetIScriptSystem();
	m_pMaterialUpr = gEnv->p3DEngine->GetMaterialUpr();

	//Register as ViewSystem listener (for cut-scenes, ...)
	if(m_pGameFramework->GetIViewSystem())
		m_pGameFramework->GetIViewSystem()->AddListener(this);

	m_script = GetEntity()->GetScriptTable();
	m_script->GetValue("Client", m_clientScript);
	m_script->GetValue("Server", m_serverScript);

	m_clientStateScript = m_clientScript;
	m_serverStateScript = m_serverScript;

	m_scriptClientHitInfo.Create(gEnv->pScriptSystem);

	m_pGameFramework->GetIGameRulesSystem()->SetCurrentGameRules(this);
	g_pGame->GetGameRulesScriptBind()->AttachTo(this);

	CGameMechanismUpr::GetInstance()->Inform(kGMEvent_GameRulesInit);

	const bool isMultiplayer=gEnv->bMultiplayer;
	if (isMultiplayer)
	{
		m_forbiddenAreas.reserve(1);		// Only expecting 1 forbidden area
		m_forbiddenAreaHelpers.reserve(1);	// Again only expected 1 (but could be more)

		m_bLevelNameCheckNeeded = true;

		if (CEquipmentLoadout *pEquipmentLoadout = g_pGame->GetEquipmentLoadout())
		{
			RegisterClientConnectionListener(pEquipmentLoadout);
		}

		m_pBattlechatter = new CBattlechatter;
		m_pAreaAnnouncer = new CAreaAnnouncer;
		m_pMiscAnnouncer = new CMiscAnnouncer;
		m_pVTOLVehicleUpr = new CVTOLVehicleUpr();
		m_pTeamVisualizationUpr = new CTeamVisualizationUpr();
		m_pCorpseUpr = new CCorpseUpr();
		m_pAnnouncer = new CAnnouncer(m_gameMode);
		m_pClientHitEffectsMP = new CClientHitEffectsMP();
		m_mpTrackViewUpr = new CMPTrackViewUpr();
		m_mpPathFollowingUpr = new CMPPathFollowingUpr();

	  if (gEnv->bServer && gEnv->IsDedicated() && g_pGame->GetCVars()->sv_votingEnable)
		{
			m_pVotingSystem = new CVotingSystem;
		}

		CEquipmentLoadout *pEquipmentLoadout = g_pGame->GetEquipmentLoadout();
		if (pEquipmentLoadout)
		{
			pEquipmentLoadout->SetPackageGroup(CEquipmentLoadout::SDK);
		}
	}

	InitSessionStatistics();

	CTelemetryCollector* pTelemetryCollector = (CTelemetryCollector*)g_pGame->GetITelemetryCollector();
	if (pTelemetryCollector && gEnv->bServer)
	{
		// This needs to come after the session id has been set (which is currently done in InitSessionStatistics)
		// Clients need to wait until the session id has been serialized to them
		pTelemetryCollector->CreateEventStream();
	}

	// Create explosion game effect and set active
	if(m_pExplosionGameEffect == NULL)
	{
		m_pExplosionGameEffect = GAME_FX_SYSTEM.CreateEffect<CExplosionGameEffect>();
		m_pExplosionGameEffect->Initialise();
	}
	if(m_pExplosionGameEffect)
	{
		m_pExplosionGameEffect->SetActive(true);
	}

	// Create modules
	tukk gameRulesName = GetEntity()->GetClass()->GetName();
	tukk xmlPath = CGameRulesModulesUpr::GetInstance()->GetXmlPath(gameRulesName);
	DrxLog ("Loading game rules class='%s' xml='%s'", gameRulesName, xmlPath);
	INDENT_LOG_DURING_SCOPE();

	CGameRulesModulesUpr *pModulesUpr = CGameRulesModulesUpr::GetInstance();
	if (xmlPath)
	{
		XmlNodeRef root = gEnv->pSystem->LoadXmlFromFile( xmlPath );
		if (root)
		{
			i32 numModules = root->getChildCount();
			for (i32 i = 0; i < numModules; ++ i)
			{
				XmlNodeRef childXml = root->getChild(i);

				tukk childTag = childXml->getTag();
				tukk className;

				if (childXml->getAttr("class", &className))
				{
					bool ok = false;

// For each module type, check if the current node is of that type and create the appropriate one
#define GAMERULES_MODULE_LIST_FUNC(type, name, lowerCase, useInEditor) \
	if (!stricmp(childTag, #name))	\
	{	\
		if (!gEnv->IsEditor() || useInEditor) \
		{ \
			DRX_ASSERT_MESSAGE(!m_##lowerCase##Module, "Module already exists");	\
			m_##lowerCase##Module = pModulesUpr->Create##name##Module(className);	\
			if (m_##lowerCase##Module)	\
			{	\
				DrxComment("CGameRules::Init() created %s module", className);	\
				m_##lowerCase##Module->Init(childXml);	\
				ok = true;	\
			}	\
			else	\
			{	\
				DrxLogAlways("CGameRules::Init() ERROR: Failed to create %s module", className);	\
			}	\
		} \
		else \
		{ \
			DrxComment("CGameRules::Init() module '%s' not created because we're in the editor", className);	\
			ok = true; \
		} \
	}

					GAMERULES_MODULE_TYPES_LIST

					if (!ok)
					{
						DrxLogAlways("Failed to create module %s", className);
						DRX_ASSERT_MESSAGE(ok, "Failed to create gamerules module");
					}

#undef GAMERULES_MODULE_LIST_FUNC
				}
			}
		}
#ifndef _RELEASE
		else
		{
			DrxFatalError("Failed to load gamerules xml '%s'", xmlPath);
		}
#endif
	}

	if (!gEnv->IsEditor() && !gEnv->IsDedicated())
	{
		SHUDEvent initGameRules;
		initGameRules.eventType = eHUDEvent_OnInitGameRules;
		CHUDEventDispatcher::CallEvent(initGameRules);
	}

	g_pGame->GetIGameFramework()->RegisterListener(this, "gamerules", FRAMEWORKLISTENERPRIORITY_GAME);
	gEnv->pNetwork->AddHostMigrationEventListener(this, "CGameRules", ELPT_PostEngine);

	CPersistantStats *pPersistantStats = CPersistantStats::GetInstance();

	AddGameRulesListener(CPlayerProgression::GetInstance());
	AddGameRulesListener(pPersistantStats);

	CAfterMatchAwards *pAfterMatchAwards = pPersistantStats->GetAfterMatchAwards();
	DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CGameRules::Init() clearing out AfterMatchAwards as we start a new match");
	pAfterMatchAwards->Clear();

	CRecordingSystem* pRecordingSystem = g_pGame->GetRecordingSystem();
	if (pRecordingSystem)
	{
		pRecordingSystem->OnGameRulesInit();
	}

	if (g_pGame->GetEquipmentLoadout())
	{
		g_pGame->GetEquipmentLoadout()->SelectProfileLoadout();
	}

	m_bIsTeamGame = pModulesUpr->IsTeamGame(gameRulesName);

	if( m_pVTOLVehicleUpr )
	{
		m_pVTOLVehicleUpr->Init();
	}

	if(m_pTeamVisualizationUpr)
	{
		m_pTeamVisualizationUpr->Init();
	}

	GAME_FX_SYSTEM.GameRulesInitialise();


	m_timeLastShownUnbalancedTeamsWarning = gEnv->pTimer->GetFrameStartTime();

	return true;
}

void CGameRules::InitSessionStatistics()
{
	CStatsRecordingMgr		*sr=g_pGame->GetStatsRecorder();

	if (sr)
	{
		if (!gEnv->bMultiplayer)
		{
			CTelemetryCollector* pTelemetryCollector = (CTelemetryCollector*)g_pGame->GetITelemetryCollector();
			if (pTelemetryCollector)
				pTelemetryCollector->CloseStatoscopeStream();
		}

		sr->BeginSession();

		if (!gEnv->bMultiplayer)
		{
			CTelemetryCollector* pTelemetryCollector = (CTelemetryCollector*)g_pGame->GetITelemetryCollector();
			if (pTelemetryCollector)
				pTelemetryCollector->CreateStatoscopeStream();
		}

		if (sr->IsTrackingEnabled())
		{
			if( IStatsTracker* tr = sr->GetSessionTracker() )
			{
				string gamemode = GetEntity()->GetClass()->GetName();
				gamemode.MakeLower();
				tr->StateValue(eSS_Gamemode, gamemode.c_str());

				string strMapName = "NO_MAP_ASSIGNED";
				string strMapPath;

				if ( ICVar *sv_map = gEnv->pConsole->GetCVar("sv_map") )
				{
					if ( tukk mapName = sv_map->GetString() )
					{
						strMapName = mapName;
						strMapName.MakeLower();

						if ( ILevelInfo * pInfo = g_pGame->GetIGameFramework()->GetILevelSystem()->GetLevelInfo(mapName) )
							if ( tukk mapPath = pInfo->GetPath() )
								strMapPath = mapPath;
					}
				}

				// strip the path off the beginning of the map, the path is already output in the map path key, and having it in the map name
				// causes the path concatenation in the tool to get a doubled up directory components
				{
					i32		pathOffset=strMapName.rfind('/');
					if (pathOffset!=-1)
					{
						strMapName=strMapName.Right(strMapName.length()-pathOffset-1);
					}
				}

				tr->StateValue(eSS_Map,			strMapName.c_str());
				tr->StateValue(eGSS_MapPath,	strMapPath.c_str());
			}

			// if teams are available then add it
			if(gEnv->bMultiplayer)
			{
				TTeamIdMap::iterator it = m_teams.begin();
				while (it != m_teams.end())
				{
					sr->AddTeam(it->second,it->first);
					++it;
				}
			}
			else
			{
				sr->AddTeam(0, "AI");
				sr->AddTeam(1, "Players");
			}
		}
	}

	g_pGame->ClearSessionTelemetry();
}

void CGameRules::SaveSessionStatistics(float delay)
{
	if (m_sessionStatisticsSaved)
	{
		if (delay == 0.f)
		{
			// This is a failsafe if the user quits the game while an end session is queued
			CStatsRecordingMgr		*sr=g_pGame->GetStatsRecorder();
			if (sr)
			{
				sr->EndSessionNowIfQueued();
			}
		}
		// In multiplayer we only want to upload the session stats once per game
		// In singleplayer we upload once for each checkpoint
		g_pGame->ClearSessionTelemetry();
	}
	else
	{
#if ENABLE_GAME_CODE_COVERAGE
		CGameCodeCoverageUpr::GetInstance()->UploadHitCheckpointsToServer();
#endif

		g_pGame->UploadSessionTelemetry();

		CStatsRecordingMgr		*sr=g_pGame->GetStatsRecorder();
		if (sr)
		{
			sr->EndSession(delay);
		}

		if (gEnv->bMultiplayer)
		{
			CTelemetryCollector* pTelemetryCollector = (CTelemetryCollector*)g_pGame->GetITelemetryCollector();
			if (pTelemetryCollector)
			{
				pTelemetryCollector->CloseStatoscopeStream();
			}
		}

		if (gEnv->bMultiplayer)
		{
			CHANGED_NETWORK_STATE(this, eEA_GameServerStatic);	// Probably don't need this, but leaving it to be safe

			m_sessionStatisticsSaved = true;
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::PostInit( IGameObject * pGameObject )
{
	CCCPOINT(GameRules_PostInit);
	INDENT_LOG_DURING_SCOPE(true, "During CGameRules::PostInit");

	pGameObject->EnableUpdateSlot(this, 0);
	pGameObject->SetUpdateSlotEnableCondition(this, 0, eUEC_WithoutAI);
	pGameObject->EnablePostUpdates(this);

	IConsole *pConsole=gEnv->pConsole;
	RegisterConsoleCommands(pConsole);
	RegisterConsoleVars(pConsole);

	if (m_teamsModule)
	{
		m_teamsModule->PostInit();
	}
	if (m_stateModule)
	{
		m_stateModule->PostInit();
	}
	if (m_playerSetupModule)
	{
		m_playerSetupModule->PostInit();
	}
	if (m_damageHandlingModule)
	{
		m_damageHandlingModule->PostInit();
	}
	if (m_spawningModule)
	{
		m_spawningModule->PostInit();
	}
	if (m_actorActionModule)
	{
		m_actorActionModule->PostInit();
	}

	switch(m_gameMode)
	{
		case eGM_InstantAction:
		{
			tukk localisedMessage = CHUDUtils::LocalizeString("@ui_msg_ia_status");
			SHUDEventWrapper::OnGameStatusUpdate(eGBNFLP_Neutral, localisedMessage);
			break;
		}
		case eGM_TeamInstantAction:
		{
			tukk localisedMessage = CHUDUtils::LocalizeString("@ui_msg_tia_status");
			SHUDEventWrapper::OnGameStatusUpdate(eGBNFLP_Neutral, localisedMessage);
			break;
		}
	}

}

//------------------------------------------------------------------------
void CGameRules::InitClient(i32 channelId)
{
	if( m_pVTOLVehicleUpr )
	{
		m_pVTOLVehicleUpr->InitClient(channelId);
	}
}

//------------------------------------------------------------------------
void CGameRules::PostInitClient(i32 channelId)
{
	INDENT_LOG_DURING_SCOPE(true, "During CGameRules::PostInitClient");

	// update the time
	i32 timeSinceStarted = (i32)((m_cachedServerTime - m_gameStartedTime).GetMilliSecondsAsInt64() / 1000LL);
	GetGameObject()->InvokeRMI(ClPostInit(), PostInitParams(timeSinceStarted, SkillKill::GetFirstBlood(), m_uSecurity), eRMI_ToClientChannel, channelId);

	if (m_gameStartTime.GetMilliSeconds() > GetServerTime())
		GetGameObject()->InvokeRMI(ClSetGameStartTimer(), SetGameTimeParams(m_gameStartTime), eRMI_ToClientChannel, channelId);

	// update team status on the client
	for (TEntityTeamIdMap::const_iterator tit=m_entityteams.begin(); tit!=m_entityteams.end(); ++tit)
	{
		IEntity *pEntity = gEnv->pEntitySystem->GetEntity(tit->first);
		if((pEntity) && !(pEntity->GetFlags() & (ENTITY_FLAG_SERVER_ONLY|ENTITY_FLAG_CLIENT_ONLY)))
		{
			GetGameObject()->InvokeRMIWithDependentObject(ClSetTeam(), SetTeamParams(tit->first, tit->second), eRMI_ToClientChannel, tit->first, channelId);
		}
	}

#if USE_PC_PREMATCH
	if (m_prematchState == ePS_Countdown)
	{
		float timeLeft = (gEnv->pTimer->GetFrameStartTime().GetSeconds() - m_finishPrematchTime);
		if (timeLeft > 0.f)
		{
			GetGameObject()->InvokeRMI(ClStartingPrematchCountDown(),
				StartingPrematchCountDownParams(timeLeft),
				eRMI_ToClientChannel, channelId);
		}
	}
#endif

	if (g_pGame->GetHostMigrationState() != CGame::eHMS_NotMigrating)
	{
		// Tell this client who else has made it
		for (i32 i = 0; i < MAX_PLAYERS; ++ i)
		{
			TNetChannelID migratedChannelId = m_migratedPlayerChannels[i];
			if (migratedChannelId)
			{
				IActor *pActor = GetActorByChannelId(migratedChannelId);
				if (pActor)
				{
					EntityParams params;
					params.entityId = pActor->GetEntityId();
					GetGameObject()->InvokeRMIWithDependentObject(ClHostMigrationPlayerJoined(), params, eRMI_ToClientChannel|eRMI_NoLocalCalls, params.entityId, channelId);
				}
			}
			else
			{
				break;
			}
		}
	}

	i32k numRespawnParams = m_respawndata.size();
	for (i32 i = 0; i < numRespawnParams; ++ i)
	{
		SEntityRespawnData *pData = &m_respawndata[i];
		if (pData->m_bHasRespawned)
		{
			SRespawnUpdateParams params;
			params.m_respawnEntityId = pData->m_currentEntityId;
			params.m_respawnHashId = (i32) pData->m_nameHash.id;
			GetGameObject()->InvokeRMIWithDependentObject(ClUpdateRespawnData(), params, eRMI_ToClientChannel, params.m_respawnEntityId, channelId);
		}
	}

	//Synch trackview animations
	if(m_mpTrackViewUpr)
	{
		STrackViewParameters params;
		m_mpTrackViewUpr->Server_SynchAnimationTimes(params);
		GetGameObject()->InvokeRMIWithDependentObject(ClTrackViewSynchAnimations(), params, eRMI_ToClientChannel|eRMI_NoLocalCalls, 0, channelId);
	}

	if (GetObjectivesModule())
	{
		GetObjectivesModule()->PostInitClient(channelId);
	}

	IGameRulesStateModule *pStateModule = GetStateModule();
	if (pStateModule && (pStateModule->GetGameState() == IGameRulesStateModule::EGRS_PostGame))
	{
		DrxLog("CGameRules::PostInitClient() sending late victory message to channel %d", channelId);
		IGameRulesVictoryConditionsModule *pVictoryConditions = GetVictoryConditionsModule();
		pVictoryConditions->SendVictoryMessage(channelId);
	}
}

//------------------------------------------------------------------------
bool CGameRules::ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params )
{
	ResetGameObject();

	DRX_ASSERT_MESSAGE(false, "CGameRules::ReloadExtension not implemented");

	return false;
}

//------------------------------------------------------------------------
bool CGameRules::GetEntityPoolSignature( TSerialize signature )
{
	DRX_ASSERT_MESSAGE(false, "CGameRules::GetEntityPoolSignature not implemented");

	return true;
}

//------------------------------------------------------------------------
void CGameRules::Release()
{
	CCCPOINT(GameRules_Release);

	UnregisterConsoleCommands(gEnv->pConsole);
	UnregisterConsoleVars(gEnv->pConsole);
	delete this;
}

//------------------------------------------------------------------------
void CGameRules::FullSerialize( TSerialize ser )
{
}

//-----------------------------------------------------------------------------------------------------
void CGameRules::PostSerialize()
{
}

//------------------------------------------------------------------------
void CGameRules::Update( SEntityUpdateContext& ctx, i32 updateSlot )
{
	DrxWatch3DTick(ctx.fFrameTime);

	CTimeValue previousServerTime = m_cachedServerTime;
	m_cachedServerTime = g_pGame->GetIGameFramework()->GetServerTime();

	if (m_hostMigrationTimeSinceGameStarted.GetValue())
	{
		int64 initialValue = m_gameStartedTime.GetValue();
		m_gameStartedTime = (m_cachedServerTime - m_hostMigrationTimeSinceGameStarted);
		m_hostMigrationTimeSinceGameStarted.SetValue(0);

		IGameRulesRoundsModule *pRoundsModule = GetRoundsModule();
		if (pRoundsModule)
		{
			pRoundsModule->AdjustTimers(m_cachedServerTime - previousServerTime);
		}
	}

	//Will always be valid
	CSmokeUpr::GetSmokeUpr()->Update(ctx.fFrameTime);
	CActorUpr::GetActorUpr()->Update(ctx.fFrameTime);

	if (updateSlot!=0)
		return;

	ProcessQueuedExplosions();

	if (gEnv->bServer)
  {
		UpdateEntitySchedules(ctx.fFrameTime);
		KnockBackPendingActors();
  }
	else
	{
		UpdateIdleKick(ctx.fFrameTime);
	}

	if ((!gEnv->bMultiplayer) && m_cinematicInput.IsAnyCutSceneRunning())
	{
		m_cinematicInput.Update(ctx.fFrameTime);
	}

	if(m_pBattlechatter)
		m_pBattlechatter->Update(ctx.fFrameTime);

	if(m_pAreaAnnouncer)
		m_pAreaAnnouncer->Update(ctx.fFrameTime);

	if (m_pMiscAnnouncer)
	{
		m_pMiscAnnouncer->Update(ctx.fFrameTime);
	}

	if(m_pCorpseUpr)
	{
		m_pCorpseUpr->Update(ctx.fFrameTime);
	}

	DRX_TODO(06, 10, 2009, "[CG] Make these update calls into a list of listeners!");
	if (m_stateModule)
	{
		m_stateModule->Update(ctx.fFrameTime);
	}

	if (m_playerStatsModule)
	{
		m_playerStatsModule->Update(ctx.fFrameTime);
	}

	if (m_spawningModule)
	{
		m_spawningModule->Update(ctx.fFrameTime);
	}

	if (m_victoryConditionsModule)
	{
		m_victoryConditionsModule->Update(ctx.fFrameTime);
	}

	if (m_damageHandlingModule)
	{
		m_damageHandlingModule->Update(ctx.fFrameTime);
	}

	if (m_spectatorModule)
	{
		m_spectatorModule->Update(ctx.fFrameTime);
	}

	if (m_objectivesModule)
	{
		if (GetStateModule()->GetGameState() != IGameRulesStateModule::EGRS_PostGame)
		{
			m_objectivesModule->Update(ctx.fFrameTime);
		}
	}

	if (m_roundsModule)
	{
		m_roundsModule->Update(ctx.fFrameTime);
	}

	CGodMode::GetInstance().Update(ctx.fFrameTime);

	if (m_bPendingLoadoutChange)
	{
		ApplyLoadoutChange();
	}

	if (gEnv->bMultiplayer && !gEnv->IsEditor())
	{
		UpdateNetLimbo();
	}

	if(m_mpTrackViewUpr)
	{
		m_mpTrackViewUpr->Update();
	}

	if (gEnv->bMultiplayer && gEnv->bServer)
	{
		CTelemetryCollector* pTelemetryCollector = (CTelemetryCollector*)g_pGame->GetITelemetryCollector();
		if (pTelemetryCollector)
		{
			static i32 prevNetBoundObjects = 0;
			SNetworkProfilingStats profileStats;
			gEnv->pNetwork->GetProfilingStatistics(&profileStats);
			i32 currentNetBoundObjects = profileStats.m_numBoundObjects;
			if (prevNetBoundObjects != currentNetBoundObjects)
			{
				pTelemetryCollector->LogEvent("Net Bound Objects", (float)currentNetBoundObjects);
				prevNetBoundObjects = currentNetBoundObjects;
			}
		}
	}

	if (m_pVTOLVehicleUpr)
	{
		m_pVTOLVehicleUpr->Update(ctx.fFrameTime);
	}

	if (gEnv->bMultiplayer && gEnv->IsClient() && IsTeamGame() && g_pGameCVars->g_autoAssignTeams && g_pGameCVars->g_switchTeamAllowed)
	{
		CGameLobby *pGameLobby = g_pGame->GetGameLobby();
		if (pGameLobby && pGameLobby->UseLobbyTeamBalancing())
		{
			i32 localTeamId = GetTeam(g_pGame->GetClientActorId());
			if ((localTeamId == 1) || (localTeamId == 2))
			{
				i32 otherTeamId = 3 - localTeamId;
				i32 localTeamCount = GetTeamPlayerCount(localTeamId, false);
				i32 otherTeamCount = GetTeamPlayerCount(otherTeamId, false);

				i32 requiredDifference = max(g_pGameCVars->g_switchTeamRequiredPlayerDifference, g_pGameCVars->g_switchTeamUnbalancedWarningDifference);
				if ((localTeamCount - otherTeamCount) >= requiredDifference)
				{
					CTimeValue currentTime = gEnv->pTimer->GetFrameStartTime();
					if ((currentTime - m_timeLastShownUnbalancedTeamsWarning).GetSeconds() > g_pGameCVars->g_switchTeamUnbalancedWarningTimer)
					{
						SHUDEventWrapper::OnChatMessage( 0, -3, "@ui_menu_manual_switch_team" );
						m_timeLastShownUnbalancedTeamsWarning = currentTime;
					}
				}
			}
		}
	}

#ifndef _RELEASE
	if(m_mpPathFollowingUpr)
	{
		m_mpPathFollowingUpr->Update();
	}

	g_pGame->GetLedgeUpr()->DebugDraw();
#endif //_RELEASE


#if USE_PC_PREMATCH
	if (m_prematchState == ePS_Prematch)
	{
		// We're in this state if we haven't spawned yet or it's counting down the inital spawn
		if (gEnv->bServer)
		{
			if (IGameRulesStateModule* pStateModule = g_pGame->GetGameRules()->GetStateModule())
			{
				IGameRulesStateModule::EGR_GameState state = pStateModule->GetGameState();
				if (state == IGameRulesStateModule::EGRS_InGame)
				{
					ChangePrematchState(ePS_Match);
				}
			}
		}
	}
	else if (m_prematchState == ePS_PrematchWaitingForPlayers)
	{
		// We're in this state if we have spawned (so game state is ingame) but we haven't got the right amount of players yet.
		if (gEnv->bServer)
		{
			if (IGameRulesStateModule* pStateModule = g_pGame->GetGameRules()->GetStateModule())
			{
				IGameRulesStateModule::EGR_GameState state = pStateModule->GetGameState();
				if (state == IGameRulesStateModule::EGRS_InGame)
				{
					i32 numRequiredPlayers = g_pGameCVars->g_minPlayersForRankedGame - GetPlayerCount(true);
					CGameLobby *pGameLobby = g_pGame->GetGameLobby();
					bool bCanStart = (numRequiredPlayers <= 0);
					if (bCanStart && pGameLobby && pGameLobby->UseLobbyTeamBalancing())
					{
						bCanStart = false;
						if (pGameLobby->IsGameBalanced())
						{
							if (abs(GetTeamPlayerCount(1, true) - GetTeamPlayerCount(2, true)) < 2)
							{
								bCanStart = true;
							}
						}
						else
						{
							if (m_timeStartedWaitingForBalancedGame.GetValue() == 0LL)
							{
								m_timeStartedWaitingForBalancedGame = m_cachedServerTime;
								CHANGED_NETWORK_STATE(this, eEA_GameServerStatic);
							}
							else
							{
								float timeWaited = m_cachedServerTime.GetDifferenceInSeconds(m_timeStartedWaitingForBalancedGame);
								if (timeWaited > g_pGameCVars->gl_waitForBalancedGameTime)
								{
									ForceBalanceTeams();
								}
							}
						}
					}

					if (bCanStart)
					{
						if (g_pGameCVars->g_restartWhenPrematchFinishes == 1)
						{
							Restart();
						}
						else if (g_pGameCVars->g_restartWhenPrematchFinishes == 2)
						{
							PrematchRespawn();
						}
						ChangePrematchState(ePS_Countdown);
					}
					else
					{
						CPlaylistUpr *pPlaylistUpr = g_pGame->GetPlaylistUpr();
						if (pGameLobby && pPlaylistUpr)
						{
							if (pGameLobby->IsPrivateGame() || pPlaylistUpr->IsUsingCustomVariant())
							{
								ChangePrematchState(ePS_Countdown);
							}
						}
					}

					numRequiredPlayers = max(numRequiredPlayers, 0);
					if (m_numRequiredPlayers != numRequiredPlayers)
					{
						m_numRequiredPlayers = numRequiredPlayers;
						CHANGED_NETWORK_STATE(this, eEA_GameServerStatic);
					}
				}
			}
		}

		if ((m_prematchState == ePS_PrematchWaitingForPlayers)
			&& (gEnv->IsClient()))
		{
			// Don't need any hud messages in intro
			if(!IsIntroSequenceCurrentlyPlaying())
			{
				if ((m_previousNumRequiredPlayers != m_numRequiredPlayers) || (m_timeStartedWaitingForBalancedGame.GetValue() != 0LL))
				{
					m_previousNumRequiredPlayers = m_numRequiredPlayers;
					if (m_waitingForPlayerMessage1.empty())
					{
						m_waitingForPlayerMessage1.assign(CHUDUtils::LocalizeString("@mp_prematch_warning"));
					}

					if (m_numRequiredPlayers > 1)
					{
						DrxFixedStringT<4> numPlayersLeft;
						numPlayersLeft.Format("%d", m_numRequiredPlayers);
						m_waitingForPlayerMessage2.assign(CHUDUtils::LocalizeString("@mp_prematch_playercount_warning", numPlayersLeft.c_str()));
					}
					else if (m_numRequiredPlayers == 1)
					{
						m_waitingForPlayerMessage2.assign(CHUDUtils::LocalizeString("@mp_prematch_oneplayer_warning"));
					}
					else
					{
						if (m_timeStartedWaitingForBalancedGame.GetValue() != 0LL)
						{
							float timeWaited = m_cachedServerTime.GetDifferenceInSeconds(m_timeStartedWaitingForBalancedGame);
							float timeRemaining = g_pGameCVars->gl_waitForBalancedGameTime - timeWaited;
							m_waitingForPlayerMessage2.Format("%.0f", max(0.f, timeRemaining));
							m_waitingForPlayerMessage2 = CHUDUtils::LocalizeString("@ui_menu_gamelobby_waiting_for_opponents", m_waitingForPlayerMessage2.c_str());
						}
						else
						{
							m_waitingForPlayerMessage2.assign(CHUDUtils::LocalizeString("@mp_prematch_opponents"));
						}
					}

					SHUDEventWrapper::OnBigWarningMessage(m_waitingForPlayerMessage1.c_str(), m_waitingForPlayerMessage2.c_str());
				}
			}
		}
	}
	else if (m_prematchState == ePS_Countdown)
	{
		const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		if (gEnv->bServer)
		{
			bool bStartGame = currentTime > m_finishPrematchTime;

			if (bStartGame)
			{
				ChangePrematchState(ePS_Match);
				ResetGameTime();

				CHANGED_NETWORK_STATE(this, eEA_GameServerStatic);

				CStatsRecordingMgr *pStatsRecording = g_pGame->GetStatsRecorder();
				if (pStatsRecording)
				{
					pStatsRecording->RoundActuallyStarted();
				}

				m_previousNumRequiredPlayers = -1;
				m_waitingForPlayerMessage1.clear();
				m_waitingForPlayerMessage2.clear();

				if(!IsIntroSequenceCurrentlyPlaying())
				{
					SHUDEventWrapper::OnBigWarningMessage(m_waitingForPlayerMessage1.c_str(), m_waitingForPlayerMessage2.c_str());
				}

				g_pGame->GetUI()->ActivateDefaultState();
				CHUDEventDispatcher::CallEvent(eHUDEvent_OnPrematchFinished);
			}
		}

		if (gEnv->IsClient() )
		{
			if (g_pGameCVars->g_restartWhenPrematchFinishes == 1)
			{
				if (currentTime > m_finishPrematchTime)
				{
					g_pGame->GetUI()->ActivateState("no_hud");
				}
			}
		}
	}
#endif
}

#if USE_PC_PREMATCH
void CGameRules::ChangePrematchState(EPrematchState newState)
{
	if (newState != m_prematchState)
	{
		EPrematchState oldState = m_prematchState;
		m_prematchState = newState;

		if ((newState == ePS_Countdown)
			&& gEnv->bServer)
		{
			m_finishPrematchTime = (gEnv->pTimer->GetFrameStartTime() + g_pGame->GetCVars()->g_gameRules_startTimerLength).GetSeconds();

			g_pGameActions->FilterMPPreGameFreeze()->Enable(true);

			GetGameObject()->InvokeRMI(ClStartingPrematchCountDown(),
				StartingPrematchCountDownParams(g_pGame->GetCVars()->g_gameRules_startTimerLength),
				eRMI_ToAllClients);
		}

		if (newState == ePS_Match)
		{
			if (m_stateModule)
			{
				if (m_stateModule->GetGameState() == IGameRulesStateModule::EGRS_InGame)
				{
					g_pGameActions->FilterMPPreGameFreeze()->Enable(false);
				}
			}

			CPersistantStats *pStats = CPersistantStats::GetInstance();
			if (pStats)
			{
				pStats->OnGameActuallyStarting();
			}

			bool isSkipped = (oldState == ePS_Prematch && newState == ePS_Match);
			if(m_roundsModule)
			{
				m_roundsModule->OnPrematchStateEnded(isSkipped);
			}

			if (m_objectivesModule)
			{
				m_objectivesModule->OnPrematchStateEnded();
			}

			CParameterGameEffect * pParameterGameEffect = g_pGame->GetParameterGameEffect();
			if (pParameterGameEffect)
			{
				pParameterGameEffect->SetSaturationAmount(1.f, CParameterGameEffect::eSEU_PreMatch);
			}

			// Notify listeners
			OnPrematchEnd_NotifyListeners();

			//m_prematchAudioSignalPlayer.Stop();

			if (gEnv->bServer)
			{
				CHANGED_NETWORK_STATE(this, eEA_GameServerStatic);
			}
		}
	}
}

void CGameRules::PrematchRespawn()
{
	if (m_statsRecordingModule)
	{
		IActorSystem * pActorSystem = g_pGame->GetIGameFramework()->GetIActorSystem();
		IActorIteratorPtr iter = pActorSystem->CreateActorIterator();

		while (IActor *pActor = iter->Next())
		{
			m_statsRecordingModule->OnPrematchEnd(pActor);
		}
	}

	if (IGameRulesSpawningModule* pSpawningModule=GetSpawningModule())
	{
		pSpawningModule->ReviveAllPlayers(false, false);
	}
}

#endif

void CGameRules::UpdateGameRulesCvars()
{
	//TODO: callbacks for the game rules
	if (gEnv->bServer && ((m_timeLimit != g_pGameCVars->g_timelimit) ||
		((g_pGameCVars->g_scoreLimitOverride == 0)  && (m_scoreLimit != g_pGameCVars->g_scoreLimit)) ||
		(m_roundLimit != g_pGameCVars->g_roundlimit) ||
		(m_scoreLimitOverride != g_pGameCVars->g_scoreLimitOverride)))
	{
		m_timeLimit = g_pGameCVars->g_timelimit;
		m_scoreLimit = g_pGameCVars->g_scoreLimitOverride ? g_pGameCVars->g_scoreLimitOverride : g_pGameCVars->g_scoreLimit;
		m_roundLimit = g_pGameCVars->g_roundlimit;
		m_scoreLimitOverride = g_pGameCVars->g_scoreLimitOverride;

		CHANGED_NETWORK_STATE(this, GAMERULES_LIMITS_ASPECT );
	}
}


//------------------------------------------------------------------------
void CGameRules::ApplyLoadoutChange()
{
	IActor *pClientActor = g_pGame->GetIGameFramework()->GetClientActor();
	CEquipmentLoadout *pEquipmentLoadout = g_pGame->GetEquipmentLoadout();

	DRX_ASSERT(pClientActor);
	if (pClientActor != NULL && pEquipmentLoadout != NULL)
	{
		u16 gameChannelId = pClientActor->GetChannelId();
		DRX_ASSERT(gameChannelId);
		pEquipmentLoadout->ClSendCurrentEquipmentLoadout(gameChannelId);

		// If this is the first time we've set a loadout then request a revive automatically
		CActor * pActor = static_cast<CActor*>(pClientActor);
		bool forcedEquipmentChange = pActor->GetSpectatorState() == CActor::eASS_ForcedEquipmentChange;
		if ((pActor->GetSpectatorState() == CActor::eASS_None)  || forcedEquipmentChange)
		{
			CGameRules *pGameRules = g_pGame->GetGameRules();
			IGameRulesSpawningModule *pSpawningModule = pGameRules->GetSpawningModule();
			DRX_ASSERT(pSpawningModule);

			if(!m_objectivesModule || m_objectivesModule->RequestReviveOnLoadoutChange())
			{
				EntityId clientActorId = pClientActor->GetEntityId();
				pSpawningModule->ClRequestRevive(clientActorId);
			}

				// this call here will potentially bring the client and server out of sync
				//pActor->SetSpectatorState(CActor::eASS_Ingame);
		}

		pEquipmentLoadout->SetHasPreGameLoadoutSent(true);

		m_bPendingLoadoutChange = false;
	}
}

//------------------------------------------------------------------------
void CGameRules::KnockBackPendingActors()
{
	std::vector<EntityId>::const_iterator itEnd = m_pendingActorsToBeKnockedDown.end();
	for (std::vector<EntityId>::const_iterator it = m_pendingActorsToBeKnockedDown.begin(); it != itEnd; ++it)
		KnockActorDown(*it);

	m_pendingActorsToBeKnockedDown.clear();
}

//------------------------------------------------------------------------
void CGameRules::HandleEvent( const SGameObjectEvent& event)
{
}

//------------------------------------------------------------------------
void CGameRules::ProcessEvent( SEntityEvent& event)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_GAME);

	static ICVar* pTOD = gEnv->pConsole->GetCVar("sv_timeofdayenable");

	switch(event.event)
	{
	//This is called while loading a saved game
	case ENTITY_EVENT_PRE_SERIALIZE:
		{
			g_pGame->PreSerialize();
			ResetQueuedExplosionsAndHits();
		}
		break;

	case ENTITY_EVENT_RESET:
	{
		// done here rather than CGameRules::Restart so that it is called on clients as well as servers
		SaveSessionStatistics();
		InitSessionStatistics();

		m_timeOfDayInitialized = false;

		for(i32 i = 0; i < MAX_CONCURRENT_EXPLOSIONS; i++)
		{
			m_explosions[i].m_mfxInfo.Reset();
			m_explosionValidities[i]	= false;
		}

		while (!m_queuedExplosions.empty())
			m_queuedExplosions.pop();

		while (!m_queuedExplosionsAwaitingRaycasts.empty())
			m_queuedExplosionsAwaitingRaycasts.pop();

		while (!m_queuedHits.empty())
			m_queuedHits.pop();

		m_processingHit=0;

      // TODO: move this from here
		g_pGame->GetWeaponSystem()->GetTracerUpr().Reset();
		m_respawns.clear();
		m_removals.clear();
		m_gamePausedTime.SetValue(0LL);

		if (m_stateModule)
		{
			m_stateModule->OnGameReset();
		}

		i32 resetScore = 0;
		if (m_scoringModule)
		{
			resetScore = m_scoringModule->GetStartTeamScore();
		}

		for (TTeamScoresMap::iterator iter=m_teamscores.begin(); iter!=m_teamscores.end(); ++iter)
		{
			iter->second.m_teamScore = resetScore;
			iter->second.m_roundTeamScore = 0;
		}

		if (m_playerStatsModule)
			m_playerStatsModule->ClearAllPlayerStats();

		if (GetObjectivesModule())
		{
			GetObjectivesModule()->OnGameReset();
		}

		if (GetVictoryConditionsModule())
		{
			GetVictoryConditionsModule()->OnRestart();
		}

		g_pGame->GetGameAISystem()->Reset(false);

		if (gEnv->IsEditor() && event.nParam[0])
		{
			IActor* pActor = g_pGame->GetIGameFramework()->GetClientActor();
			EntityId playerId = 0;
			if(pActor)
				playerId = pActor->GetEntityId();

			if (GetPlayerSetupModule())
			{
				if(pActor)
					pActor->GetInventory()->Destroy();

				GetPlayerSetupModule()->OnPlayerRevived(playerId);
			}
			else
			{
				CallScript(m_script, "EquipPlayer", playerId, false);
			}
		}

		m_bHasCalledEnteredGame = false;

		break;
	}
  case ENTITY_EVENT_POST_SERIALIZE:
    {
      break;
    }
	case ENTITY_EVENT_START_GAME:
	{
		m_timeOfDayInitialized = false;
		g_pGame->GetWeaponSystem()->GetTracerUpr().Reset();
		g_pGame->GetGameAudio()->Reset();

		const bool bIsMultiplayer = gEnv->bMultiplayer;
		if (gEnv->bServer && bIsMultiplayer && pTOD && pTOD->GetIVal() && g_pGame->GetIGameFramework()->IsImmersiveMPEnabled())
		{
			static ICVar* pStart = gEnv->pConsole->GetCVar("sv_timeofdaystart");
			if (pStart)
				gEnv->p3DEngine->GetTimeOfDay()->SetTime(pStart->GetFVal(), true);
		}

		if(gEnv->IsClient())
		{
			// Enabling of game type specific action maps must be done here instead of the Init() as it needs to happen after the CCET_DisableActionMap context task
			IActionMapUpr *pActionMapMan = g_pGame->GetIGameFramework()->GetIActionMapUpr();
			IActionMap *pAM = NULL;
			pActionMapMan->EnableActionMap("multiplayer",bIsMultiplayer);
			pActionMapMan->EnableActionMap("singleplayer",!bIsMultiplayer);
			if(bIsMultiplayer)
			{
				pAM=pActionMapMan->GetActionMap("multiplayer");
			}
			else
			{
				pAM=pActionMapMan->GetActionMap("singleplayer");
			}

			if(pAM)
			{
				pAM->SetActionListener(GetEntity()->GetId());
			}
		}

		if(GetVictoryConditionsModule())
		{
			GetVictoryConditionsModule()->OnStartGame();
		}

		if (GetObjectivesModule())
		{
			GetObjectivesModule()->OnStartGame();
		}

		if (GetRoundsModule())
		{
			GetRoundsModule()->OnStartGame();
		}

		if ( CPersistantStats::GetInstance() )
		{
			CPersistantStats::GetInstance()->AddListeners();
		}

		if (g_pGame->GetRecordingSystem())
		{
			g_pGame->GetRecordingSystem()->OnStartGame();
		}

		SetupForbiddenAreaShapesHelpers();

		GetSpawningModule()->OnStartGame();


		if (GetObjectivesModule())
		{
			GetObjectivesModule()->OnStartGamePost();
		}

		m_gameStarted = true;

		break;
	}

	case ENTITY_EVENT_ENTER_SCRIPT_STATE:
		m_clientStateScript=0;
		m_serverStateScript=0;

		IEntityScriptProxy *pScriptProxy=static_cast<IEntityScriptProxy *>(GetEntity()->GetProxy(ENTITY_PROXY_SCRIPT));
		if (pScriptProxy)
		{
			tukk stateName=pScriptProxy->GetState();

			m_clientScript->GetValue(stateName, m_clientStateScript);
			m_serverScript->GetValue(stateName, m_serverStateScript);
		}
		break;
	}

}

//------------------------------------------------------------------------
void CGameRules::SetAuthority( bool auth )
{
}

//------------------------------------------------------------------------
void CGameRules::PostUpdate( float frameTime )
{
	INDENT_LOG_DURING_SCOPE(true, "During CGameRules::PostUpdate");

	if(m_pVotingSystem && m_pVotingSystem->IsInProgress())
	{
		bool bVoteSuccess = false;
		const bool bVoteFinished = KickVoteConditionsMet(bVoteSuccess);
		if(bVoteFinished) // If votes for has been reached, or there's not enough voters left to hit threshold
		{
			EndVoting(bVoteSuccess);
		}
		else if(m_pVotingSystem->GetVotingTime().GetSeconds() > g_pGame->GetCVars()->sv_votingTimeout)
		{
			EndVoting(false);
		}
	}
}

//------------------------------------------------------------------------
IActor *CGameRules::GetActorByChannelId(i32 channelId) const
{
	if (m_hostMigrationCachedEntities.empty())
	{
		return static_cast<IActor *>(m_pGameFramework->GetIActorSystem()->GetActorByChannelId(channelId));
	}
	else
	{
		DRX_ASSERT(g_pGame->GetHostMigrationState() != CGame::eHMS_NotMigrating);

		IActor *pCachedActor = NULL;
		IActor *pCurrentActor = NULL;

		IActorSystem *pActorSystem = g_pGame->GetIGameFramework()->GetIActorSystem();
		IActorIteratorPtr it = pActorSystem->CreateActorIterator();

		while (IActor *pActor = it->Next())
		{
			if (pActor->GetChannelId() == channelId)
			{
				const bool bInRemoveList = stl::find(m_hostMigrationCachedEntities, pActor->GetEntityId());
				if (bInRemoveList)
				{
					DRX_ASSERT(!pCachedActor);
					pCachedActor = pActor;
				}
				else
				{
					DRX_ASSERT(!pCurrentActor);
					pCurrentActor = pActor;
				}
			}
		}

		if (gEnv->bServer)
		{
			// Server: if we've got a cached one then we are a secondary server, this can give us a few frames of
			// having a duplicated actor (pCurrentActor), we need to use the one that will be kept (pCachedActor).
			if (pCachedActor)
			{
				return pCachedActor;
			}
			else
			{
				return pCurrentActor;
			}
		}
		else
		{
			// Client: Use actor given to us by the server, if we haven't been given one then use the cached actor.
			if (pCurrentActor)
			{
				return pCurrentActor;
			}
			else
			{
				return pCachedActor;
			}
		}
	}
}

//------------------------------------------------------------------------
bool CGameRules::IsRealActor(EntityId actorId) const
{
	if (g_pGame->GetHostMigrationState() == CGame::eHMS_NotMigrating)
	{
		return true;
	}
	else
	{
		// If we're host migrating, we may have 2 actors for the same person at this point.  Need to make sure we're the real one
		IActor *pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(actorId);
		if (pActor)
		{
			IActor *pChannelActor = g_pGame->GetGameRules()->GetActorByChannelId(pActor->GetChannelId());
			if (pChannelActor == pActor)
			{
				return true;
			}
		}
		return false;
	}
}

//------------------------------------------------------------------------
IActor *CGameRules::GetActorByEntityId(EntityId entityId) const
{
	return m_pGameFramework->GetIActorSystem()->GetActor(entityId);
}

//------------------------------------------------------------------------
tukk CGameRules::GetActorNameByEntityId(EntityId entityId) const
{
	IActor *pActor = GetActorByEntityId(entityId);
	if (pActor)
		return pActor->GetEntity()->GetName();
	return 0;
}

//------------------------------------------------------------------------
ILINE tukk CGameRules::GetActorName(IActor *pActor) const
{
	return pActor->GetEntity()->GetName();
};

//------------------------------------------------------------------------
i32 CGameRules::GetChannelId(EntityId entityId) const
{
	IActor *pActor = static_cast<IActor *>(m_pGameFramework->GetIActorSystem()->GetActor(entityId));
	if (pActor)
		return pActor->GetChannelId();

	return 0;
}


//------------------------------------------------------------------------
bool CGameRules::ShouldKeepClient(i32 channelId, EDisconnectionCause cause, tukk desc) const
{
	return (!strcmp("timeout", desc) || cause==eDC_Timeout);
}


//------------------------------------------------------------------------
void CGameRules::PrecacheList(XmlNodeRef precacheListNode)
{
	CItemGeometryCache&  rItemGeomCache = g_pGame->GetGameSharedParametersStorage()->GetItemResourceCache().GetItemGeometryCache();

	i32 totPrecacheTypes = precacheListNode->getChildCount();

	for (i32 p = 0; p < totPrecacheTypes; ++ p)
	{
		//Update loading screen and important tick functions
		SYNCHRONOUS_LOADING_TICK();

		XmlNodeRef precacheNode = precacheListNode->getChild(p);
		i32 totPrecacheItems = precacheNode->getChildCount();

		i32 type;
		tukk precacheType = precacheNode->getTag();
		for (type=0; type<Precache_TOTAL; type++)
		{
			if (strcmpi(precacheType, PRECACHE_TYPES[type]) == 0)
			{
				break;
			}
		}

		DesignerWarning((type != Precache_TOTAL), "Unknown type %s in %s", precacheType, PRECACHE_LIST_XML);
		if (type != Precache_TOTAL)
		{
			if (type == Precache_CDF)
			{
				m_cachedCharacterInstances.reserve(totPrecacheItems);
			}
			else if (type == Precache_Particle)
			{
				m_cachedParticleEffects.reserve(totPrecacheItems);
			}

			for (i32 k = 0; k < totPrecacheItems; ++k)
			{
				XmlNodeRef precacheItemNode = precacheNode->getChild(k);
				tukk precacheItem = precacheItemNode->getAttr("name");
				tukk precachePlatform = precacheItemNode->getAttr("platform");
				bool useCgfStreaming = true;
				precacheItemNode->getAttr("useCgfStreaming", useCgfStreaming);

				if (strlen(precachePlatform) > 0)
				{
					if (strcmpi(precachePlatform, "pc") != 0) // it's NOT pc
					{
						continue;
					}
				}

				switch(type)
				{
				case Precache_DBA:
					DrxLog("Preload DBA %s", precacheItem);

					assert(gEnv);
					PREFAST_ASSUME(gEnv);

					if (!gEnv->pCharacterUpr->DBA_LockStatus(precacheItem, 1, ICharacterUpr::eStreamingDBAPriority_Normal))
					{
						DrxLog("Failed preload for DBA %s", precacheItem);
					}
					break;
				case Precache_AG:
					DrxLog("Preload AG %s", precacheItem);
#ifndef _RELEASE
					DrxFatalError("Impossible to precache animationgraphs without animationgraph code");
#endif
					break;
				case Precache_CDF:
					{
						DrxLog("Preload CDF %s", precacheItem);
						TCharacterInstancePtr pCharacterInstance(gEnv->pCharacterUpr->CreateInstance(precacheItem));
						m_cachedCharacterInstances.push_back(pCharacterInstance);
						break;
					}
				case Precache_CGF:
					{
						DrxLog("Preload CGF %s", precacheItem);
						rItemGeomCache.CacheGeometry(precacheItem, useCgfStreaming);  // this gets the materials too :) and the Item Geometry Cache takes care of releasing references
						break;
					}
				case Precache_CGA:
					{
						DrxLog("Preload CGA %s", precacheItem);
						rItemGeomCache.CacheGeometry(precacheItem, true);  // this gets the materials too :) and the Item Geometry Cache takes care of releasing references
						break;
					}
				case Precache_CHR:
					{
						DrxLog("Preload CHR %s", precacheItem);
						rItemGeomCache.CacheGeometry(precacheItem, true);  // this gets the materials too :) and the Item Geometry Cache takes care of releasing references
						break;
					}
				case Precache_Particle:
					{
						DrxLog("Preload Particle %s", precacheItem);
						TParticleEffectPtr  pParticleEffect (gEnv->pParticleUpr->FindEffect(precacheItem, "CGameRules::PrecacheList"));
						m_cachedParticleEffects.push_back(pParticleEffect);
						break;
					}
				case Precache_Sound:
					{
						DrxLog("Preload Sound %s", precacheItem);
						REINST("needs verification!");
						//pSoundSystem->Precache(precacheItem, 0, FLAG_SOUND_PRECACHE_LOAD_PROJECT);
						break;
					}
				case Precache_AudioHint:
					{
						DrxLog("Preload AudioHint %s", precacheItem);
						REINST("needs verification!");
						//pSoundSystem->CacheAudioFile(precacheItem, eAFCT_GAME_HINT);
						break;
					}
				case Precache_VehicleXML:
					{
						DrxLog("Preload vehicle XML %s", precacheItem);
						DRX_ASSERT_MESSAGE(DrxStringUtils::stristr(precacheItem, "Vehicles/Implementations/Xml"), "Precaching XMLs is generally a bad idea and should only be used for vehicle implementation XMLs");
						if (m_cachedXmlNodesMap.find(precacheItem) == m_cachedXmlNodesMap.end())
						{
							XmlNodeRef ref=GetISystem()->LoadXmlFromFile(precacheItem);
							if (ref)
							{
								m_cachedXmlNodesMap[precacheItem] = ref;

								i32 loadObjects = 0;
								precacheItemNode->getAttr("loadObjects", loadObjects);
								if(loadObjects)
								{
									DrxLog("Preload vehicle XML Objects %s", precacheItem);
									rItemGeomCache.CacheGeometryFromXml(ref, true);
								}
							}
							else
							{
								DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Unable to load precache xml %s", precacheItem);
							}
						}
						break;
					}
				case Precache_FlashTexs:
					{
						DrxLog("Preload FlashTex %s", precacheItem);
						if (IRenderer* piRenderer=gEnv->pRenderer)
						{
							if (TTextureInstancePtr pTextureToCache=piRenderer->EF_LoadTexture(precacheItem, (FT_NOMIPS|FT_DONT_STREAM)))
							{
								piRenderer->EF_PrecacheResource(pTextureToCache, 0, 0, 0, -1);
								m_cachedFlashTextures.push_back(pTextureToCache);
								pTextureToCache->Release();
							}
						}
						break;
					}
				case Precache_Ammo:
					{
						DrxLog("Preload Ammo %s", precacheItem);

						IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(precacheItem);
						if(pClass)
						{
							CWeaponSystem* pWeaponSystem = g_pGame->GetWeaponSystem();

							const SAmmoParams* pAmmoParams = pWeaponSystem->GetAmmoParams(pClass);

							if(pAmmoParams)
							{
								pAmmoParams->CacheResources();
							}
							else
							{
								GameWarning("Unable to find ammoParams for IEntityClass %s", precacheItem);
							}
						}
						else
						{
							GameWarning("Unable to find IEntityClass class %s", precacheItem);
						}
						break;
					}
				case Precache_BodyDamage:
					{
						CBodyDamageUpr *pBodyDamageUpr = g_pGame->GetBodyDamageUpr();
						if (pBodyDamageUpr)
						{
							tukk pType = precacheItemNode->getTag();
							if (!stricmp(pType, "BodyDamage"))
							{
								tukk pBodyFile = precacheItemNode->getAttr("body");
								tukk pPartsFile = precacheItemNode->getAttr("parts");
								SBodyDamageDef def;
								CBodyDamageUpr::GetBodyDamageDef(pBodyFile, pPartsFile, def);
								pBodyDamageUpr->CacheBodyDamage(def);
							}
							else if (!stricmp(pType, "BodyDestruction"))
							{
								SBodyDestructibilityDef def;
								CBodyDamageUpr::GetBodyDestructibilityDef(precacheItem, def);
								pBodyDamageUpr->CacheBodyDestruction(def);
							}
						}
						break;
					}
				case Precache_GFX:
					{
						if (!gEnv->IsDedicated())
						{
							DrxLog("Preload GFX %s", precacheItem);
						}
						break;
					}
				case Precache_ADB:
					{
						IMannequin &mannequinSys = gEnv->pGame->GetIGameFramework()->GetMannequinInterface();
						if( mannequinSys.GetAnimationDatabaseUpr().Load(precacheItem) == NULL )
						{
							GameWarning("Unable to precache ADB %s", precacheItem);
						}
					}
					break;
				case Precache_ADBTagDefs:
					{
						IMannequin &mannequinSys = gEnv->pGame->GetIGameFramework()->GetMannequinInterface();
						if( mannequinSys.GetAnimationDatabaseUpr().LoadTagDefs(precacheItem, true) == NULL )
						{
							GameWarning("Unable to precache ADB tag defs %s", precacheItem);
						}
					}
					break;
				case Precache_Item:
					{
						DrxLog("Preload item %s", precacheItem);

						CGameSharedParametersStorage* pGameParamsStorage = g_pGame->GetGameSharedParametersStorage();

						IEntityClass* pItemClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(precacheItem);
						if (pItemClass)
						{
							CItemSharedParams *pSharedParams = pGameParamsStorage->GetItemSharedParameters(pItemClass->GetName(), false);
							if(!pSharedParams)
							{
								GameWarning("Uninitialised item params. Has the xml been setup correctly for item %s?", pItemClass->GetName());
							}
							else
							{
								pSharedParams->CacheResourcesForLevelStartMP(pGameParamsStorage->GetItemResourceCache(), pItemClass);
							}
						}
					}
					break;
				}
			}
		}

		SLICE_AND_SLEEP();
	}
}

//------------------------------------------------------------------------
void CGameRules::PrecacheLevel()
{
	LOADING_TIME_PROFILE_SECTION;

	CallScript(m_script, "PrecacheLevel");
	XmlNodeRef root = gEnv->pSystem->LoadXmlFromFile( PRECACHE_LIST_XML );
	if (root)
	{
		tukk gameRuleName = GetEntity()->GetClass()->GetName();
    tukk levelName = g_pGame->GetIGameFramework()->GetLevelName();

		i32 totalPrecacheSets = root->getChildCount();
		for (i32 i = 0; i < totalPrecacheSets; ++ i)
		{
			XmlNodeRef precacheNode = root->getChild(i);
			tukk precacheName = precacheNode->getAttr("name");

			if(strcmpi(precacheNode->getTag(), "GameMode") == 0)
			{
				bool precacheIt = false;
				if ( (strcmpi(precacheName, gameRuleName) == 0) || ((strcmpi(precacheName, "Multiplayer") == 0) && gEnv->bMultiplayer))
				{
					precacheIt = true;
				}
#ifndef _RELEASE
				static ICVar * pLoadAllLayersForResList = 0;
				if (!pLoadAllLayersForResList)
				{
					pLoadAllLayersForResList = gEnv->pConsole->GetCVar("sv_LoadAllLayersForResList");
				}
				if (gEnv->bMultiplayer && pLoadAllLayersForResList && (pLoadAllLayersForResList->GetIVal() != 0))
				{
					//Bypass the gamemode filtering. This ensures that an autoresourcelist for generating pak files will contain all the possible assets.
					precacheIt = true;
				}
#endif // #ifndef _RELEASE
				if (precacheIt)
				{
						PrecacheList(precacheNode);
				}
			}
			else if(strcmpi(precacheNode->getTag(), "Level") == 0)
			{
				if((strcmpi(precacheName, levelName) == 0))
				{
					PrecacheList(precacheNode);
				}
			}
			else if(strcmpi(precacheNode->getTag(), "MultiplayerOption" ) == 0)
			{
				if( gEnv->bMultiplayer )
				{
					bool precacheIt = true;

#ifndef _RELEASE
					static ICVar * pLoadAllLayersForResList = 0;
					if (!pLoadAllLayersForResList)
					{
						pLoadAllLayersForResList = gEnv->pConsole->GetCVar("sv_LoadAllLayersForResList");
					}
					if (pLoadAllLayersForResList && pLoadAllLayersForResList->GetIVal() != 0)
					{
						//Bypass any variant filtering. This ensures that an autoresourcelist for generating pak files will contain all the possible assets.
						precacheIt = true;
					}
#endif // #ifndef _RELEASE
					if (precacheIt)
					{
						PrecacheList(precacheNode);
					}
				}
			}
			else if(strcmpi(precacheNode->getTag(), "Always") == 0)
			{
				PrecacheList(precacheNode);
			}
		}
	}

	if(m_pClientHitEffectsMP)
	{
		m_pClientHitEffectsMP->Initialise();
	}

	if (g_pGame->GetEquipmentLoadout())
	{
		g_pGame->GetEquipmentLoadout()->PrecacheLevel();
	}

	XmlNodeRef xmlLevelData = LoadLevelXml();

	if(xmlLevelData)
	{
		XmlNodeRef disableSaveNode = xmlLevelData->findChild("DisableSave");
		if(disableSaveNode)
			g_pGame->GetIGameFramework()->AllowSave(false);
	}

	g_pGame->GetGameCache().PrecacheLevel();

	if (gEnv->bMultiplayer)
	{
		if (m_pAreaAnnouncer)
		{
			m_pAreaAnnouncer->Init();
		}

		if (m_pMiscAnnouncer)
		{
			m_pMiscAnnouncer->Init();
		}

		if(m_mpTrackViewUpr)
		{
			m_mpTrackViewUpr->Init();
		}

		CPlayerPlugin_InteractiveEntityMonitor::PrecacheLevel();
	}

	m_levelLoaded = true;
}

void CGameRules::PrecacheLevelResource(tukk resourceName, EGameResourceType resourceType)
{
	LOADING_TIME_PROFILE_SECTION;

	INDENT_LOG_DURING_SCOPE(true, "While %s is precaching level resource '%s' (resourceType=%d)...", GetEntity()->GetEntityTextDescription(), resourceName, resourceType);

	switch(resourceType)
	{
	case eGameResourceType_Loadout:
		{
			PreCacheEquipmentPack(resourceName);
		}
		break;

	case eGameResourceType_Item:
		{
			m_equipmentLoadOutPreCacheCallback.PreCacheItemResources(resourceName);
		}
		break;
	}
}

XmlNodeRef CGameRules::LoadLevelXml()
{
	IGameFramework* pGameFramework = g_pGame->GetIGameFramework();
	DRX_ASSERT(pGameFramework);

	string levelName = pGameFramework->GetLevelName();
	levelName = PathUtil::GetFileName(levelName);	// ensure we don't have anything like !testmap/ in the path

	INDENT_LOG_DURING_SCOPE(true, "While %s is loading level XML '%s'", GetEntity()->GetEntityTextDescription(), levelName.c_str());

	if(!IsValidName(levelName))
	{
		GameWarning("CGameRules::LoadLevelXml not level name found");
		return NULL;
	}

	ILevelInfo* pLevelInfo = pGameFramework->GetILevelSystem()->GetLevelInfo(levelName);
	if(!pLevelInfo)
	{
		GameWarning("CGameRules::LoadLevelXml not level info for level '%s' found", levelName.c_str());
		return NULL;
	}

	DrxFixedStringT<128> xmlPath;
	xmlPath.Format("%s/%s.xml", pLevelInfo->GetPath(), levelName.c_str());

	XmlNodeRef xmlLevelData = gEnv->pSystem->LoadXmlFromFile( xmlPath.c_str() );
	if(!xmlLevelData)
	{
		GameWarning("CGameRules::LoadLevelXml '%s' could not be loaded", xmlPath.c_str());
		return NULL;
	}

	return xmlLevelData;
}

//------------------------------------------------------------------------
XmlNodeRef CGameRules::FindPrecachedXmlFile(tukk sFilename)
{
	XmlNodeRef  ref = 0;

	TXmlFilename2NodeRefMap::const_iterator  it = m_cachedXmlNodesMap.find(sFilename);
	if (it != m_cachedXmlNodesMap.end())
	{
		ref = it->second;  // get the ref from the map if it's in it...
	}

	return ref;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CGameRules::SEquipmentLoadOutPreCacheCallback::PreCacheItemResources(tukk itemName)
{
	CGameSharedParametersStorage* pGameParamsStorage = g_pGame->GetGameSharedParametersStorage();
	IEntityClass* pItemClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(itemName);

	if (pItemClass)
	{
		CItemSharedParams *pSharedParams = pGameParamsStorage->GetItemSharedParameters(itemName, false);
		if(pSharedParams)
		{
			pSharedParams->CacheResources(pGameParamsStorage->GetItemResourceCache(), pItemClass);
		}
		else
		{
			GameWarning("Un-initialised item params. Has the xml been setup correctly for item %s?", pItemClass->GetName());
		}
	}
}

void CGameRules::PreCacheEquipmentPack(tukk szEquipmentPackName)
{
	if (IsValidName(szEquipmentPackName))
	{
		IGameFramework* pGameFramework = g_pGame->GetIGameFramework();
		DRX_ASSERT(pGameFramework);

		IEquipmentUpr* pEquipmentUpr = pGameFramework->GetIItemSystem()->GetIEquipmentUpr();
		DRX_ASSERT(pEquipmentUpr);

		pEquipmentUpr->PreCacheEquipmentPackResources(szEquipmentPackName, m_equipmentLoadOutPreCacheCallback);
	}
}

bool CGameRules::IsValidName( tukk name ) const
{
	return (name && name[0]);
}

//------------------------------------------------------------------------
void CGameRules::OnConnect(struct INetChannel *pNetChannel)
{
	m_pClientNetChannel=pNetChannel;

	CallScript(m_clientStateScript,"OnConnect");
}


//------------------------------------------------------------------------
void CGameRules::OnDisconnect(EDisconnectionCause cause, tukk desc)
{
	DrxLog("CGameRules::OnDisconnect(cause='%d', desc='%s')", cause, desc);
	INDENT_LOG_DURING_SCOPE();

	CRecordingSystem *crs = g_pGame->GetRecordingSystem();
	if (crs)
	{
		if (crs->IsPlayingBack() || crs->IsPlaybackQueued())
			crs->StopPlayback();
	}

	m_pClientNetChannel=0;
	i32 icause=(i32)cause;
	CallScript(m_clientStateScript, "OnDisconnect", icause, desc);

	// BecomeRemotePlayer() will put the player camera into 3rd person view, but
	// the player rig will still be first person (headless, not z sorted) so
	// don't do it during host migration events
	if (!g_pGame->IsGameSessionHostMigrating())
	{
		CActor *pLocalActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());
		if (pLocalActor)
		{
			pLocalActor->BecomeRemotePlayer();
		}
	}
}

//------------------------------------------------------------------------
bool CGameRules::OnClientConnect(i32 channelId, bool isReset)
{
	CCCPOINT_IF (isReset, GameRules_ClientConnect_Reset);
	CCCPOINT_IF (! isReset, GameRules_ClientConnect_NotReset);

	CGameLobby* pGameLobby = g_pGame->GetGameLobby();
	bool isSpectator = pGameLobby ? pGameLobby->GetSpectatorStatusFromChannelId(channelId) : false;

	if (!isReset)
	{
		m_channelIds.push_back(channelId);
	}

	bool hasSetupModule = (GetPlayerSetupModule() != NULL);

	IActor *pActor = NULL;

	string playerName;
	if (gEnv->bServer && gEnv->bMultiplayer)
	{
		if (INetChannel *pNetChannel=m_pGameFramework->GetNetChannel(channelId))
		{
			playerName=pNetChannel->GetNickname();
			if (!playerName.empty())
				playerName=VerifyName(playerName);
		}

		if (!hasSetupModule)
		{
			if(!playerName.empty())
				CallScript(m_serverStateScript, "OnClientConnect", channelId, isReset, playerName.c_str());
				else
				CallScript(m_serverStateScript, "OnClientConnect", channelId, isReset);
		}
	}
	else if (!hasSetupModule)
	{
		CallScript(m_serverStateScript, "OnClientConnect", channelId);
	}
	if (hasSetupModule)
	{
		GetPlayerSetupModule()->OnClientConnect(channelId, isReset, playerName.c_str(), isSpectator);
	}

	pActor=GetActorByChannelId(channelId);

	if (pActor)
	{
		// Hide spawned actors until the client *enters* the game
		pActor->GetEntity()->Hide(true);

		//we need to pass team somehow so it will be reported correctly
		i32 status[2];
		status[0] = GetTeam(pActor->GetEntityId());
		status[1] = pActor->GetSpectatorMode();

		m_pGameplayRecorder->Event(pActor->GetEntity(), GameplayEvent(eGE_Connected, 0, m_pGameFramework->IsChannelOnHold(channelId)?1.0f:0.0f, (uk )status));

		if (isReset)
		{
			SetTeam(GetChannelTeam(channelId), pActor->GetEntityId());

			// On reset try to spawn straight away.
			if (m_spawningModule)
			{
				m_spawningModule->SvRequestRevive(pActor->GetEntityId());
			}
		}

		//notify client he has entered the game
		GetGameObject()->InvokeRMIWithDependentObject(ClEnteredGame(), NoParams(), eRMI_ToClientChannel, pActor->GetEntityId(), channelId);

		i32 numListeners = m_clientConnectionListeners.size();
		for (i32 i = 0; i < numListeners; ++ i)
		{
			m_clientConnectionListeners[i]->OnClientConnect(channelId, isReset, pActor->GetEntityId());
		}
	}

	if (g_pGame->GetHostMigrationState() != CGame::eHMS_NotMigrating)
	{
		// This is a new client joining while we're migrating, need to tell them to pause game etc
		const float timeSinceStateChange = g_pGame->GetTimeSinceHostMigrationStateChanged();
		SMidMigrationJoinParams params(i32(g_pGame->GetHostMigrationState()), timeSinceStateChange);
		GetGameObject()->InvokeRMI(ClMidMigrationJoin(), params, eRMI_ToClientChannel, channelId);
	}

	if (pGameLobby)
	{
		DrxUserID userId = pGameLobby->GetUserIDFromChannelID(channelId);
		if (userId.IsValid())
		{
			m_participatingUsers.insert(userId);
		}
	}

	CTelemetryCollector* pTelemetryCollector = (CTelemetryCollector*)g_pGame->GetITelemetryCollector();
	if (pTelemetryCollector)
	{
		pTelemetryCollector->LogEvent("Num Players", (float)m_channelIds.size());
	}

	return pActor != 0;
}

//------------------------------------------------------------------------
void CGameRules::FinishMigrationForPlayer(i32 migratingIndex)
{
	// Remove the migrating player info (so we don't check again on a game restart!)
	m_pMigratingPlayerInfo[migratingIndex].Reset();
}

//------------------------------------------------------------------------
void CGameRules::OnClientDisconnect(i32 channelId, EDisconnectionCause cause, tukk desc, bool keepClient)
{
	IActor *pActor=GetActorByChannelId(channelId);

	CGameLobby *pGameLobby = g_pGame->GetGameLobby();
	if (pGameLobby != NULL && pGameLobby->IsMidGameLeaving())
	{
		return;
	}

	const bool  isTeamGame = (GetTeamCount() > 0);

	i32k  teamId = GetChannelTeam(channelId);
	const bool  teamValid = (isTeamGame ? ((teamId == 1) || (teamId == 2)) : (teamId == 0));

	i32  teamIndex = 0;
	if (teamValid)
	{
		teamIndex = (isTeamGame ? (teamId - 1) : 0);

		DRX_ASSERT(teamIndex >= 0);
		DRX_ASSERT(teamIndex < DRX_ARRAY_COUNT(m_svLastTeamDiscoCause));

		m_svLastTeamDiscoCause[teamIndex] = eDC_Timeout;
	}

	//assert(pActor);

	DrxLog("CGameRules::OnClientDisconnect(channelId=%d, cause=%d, desc='%s', keepClient=%d): actor='%s'", channelId, (i32)cause, desc, (keepClient?1:0), ((pActor&&pActor->GetEntity())?pActor->GetEntity()->GetName():"[null]"));
	INDENT_LOG_DURING_SCOPE();

	if (!pActor)
	{
		return;
	}

	const EntityId  eid = pActor->GetEntityId();

	if (m_pVotingSystem && m_pVotingSystem->IsInProgress())
	{
		const bool bNeedsUpdate = m_pVotingSystem->EntityLeftGame(eid);
		if (m_pVotingSystem->GetType() == eVS_kick)
		{
			if (eid == m_pVotingSystem->GetEntityId())
			{
				// Player being voted has left, end vote
				EndVoting(false);
			}
			else if (bNeedsUpdate)
			{
				// Send voting update as a player who's voted has left
				bool bVoteSuccess = false;
				const bool bVoteFinished = KickVoteConditionsMet(bVoteSuccess);
				if(!bVoteFinished)
				{
					UpdateKickVoteStatus(0);
				}
			}
		}
	}


	if (teamValid)
	{
		m_svLastTeamDiscoCause[teamIndex] = cause;
	}

	ClientDisconnect_NotifyListeners(eid);

	m_pGameplayRecorder->Event(pActor->GetEntity(), GameplayEvent(eGE_Disconnected,"",keepClient?1.0f:0.0f));

	if (keepClient)
	{
		pActor->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_NotPhysicalized);

		return;
	}

	if (IVehicle *pVehicle = pActor->GetLinkedVehicle())
	{
		if (IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(eid))
			pSeat->Reset();
	}

	stl::find_and_erase(m_channelIds, channelId);

	CallScript(m_serverStateScript, "OnClientDisconnect", channelId);

	i32 numListeners = m_clientConnectionListeners.size();
	for (i32 i = 0; i < numListeners; ++ i)
	{
		m_clientConnectionListeners[i]->OnClientDisconnect(channelId, eid);
	}

	SetTeam(0, eid);

	CTelemetryCollector* pTelemetryCollector = (CTelemetryCollector*)g_pGame->GetITelemetryCollector();
	if (pTelemetryCollector)
	{
		pTelemetryCollector->LogEvent("Num Players", (float)m_channelIds.size());
	}

	return;
}

//------------------------------------------------------------------------
bool CGameRules::OnClientEnteredGame(i32 channelId, bool isReset)
{
	CCCPOINT_IF(isReset, GameRules_ClientEnteredGame_Reset);
	CCCPOINT_IF(!isReset, GameRules_ClientEnteredGame_NotReset);

	IActor *pActor=GetActorByChannelId(channelId);
	if (!pActor)
		return false;

	// Ensure the actor is visible when entering the game (but not in the editor)
	if (!gEnv->IsEditing())
	{
		bool bUnHide = true;
		if(gEnv->bMultiplayer)
		{
			// On server, this entity may have been made invisible deliberately for an intro sequence. the call to Hide will make them visible again (even though m_bInvisible IS STILL 1...). Avoid the un-hide
			IGameRulesStateModule *pStateModule = g_pGame->GetGameRules()->GetStateModule();
			if (pStateModule && pStateModule->GetGameState() == IGameRulesStateModule::EGRS_Intro &&
				  g_pGame->GetGameRules()->IsIntroSequenceRegistered())
			{
				bUnHide = false;
			}
		}

		if(bUnHide)
		{
			IEntity* pEntity = pActor->GetEntity();
			pEntity->Hide(false);
		}
	}

	ClientEnteredGame_NotifyListeners( pActor->GetEntityId() );

	IScriptTable *pPlayer=pActor->GetEntity()->GetScriptTable();
	i32 loadingSaveGame=m_pGameFramework->IsLoadingSaveGame()?1:0;
	CallScript(m_serverStateScript, "OnClientEnteredGame", channelId, pPlayer, isReset, loadingSaveGame);

	// don't do this on reset - have already been added to correct team!
#ifndef OLD_VOICE_SYSTEM_DEPRECATED
	if(!isReset || GetTeamCount() < 2)
		ReconfigureVoiceGroups(pActor->GetEntityId(), -999, 0); /* -999 should never exist :) */
#endif

	i32 numListeners = m_clientConnectionListeners.size();
	for (i32 i = 0; i < numListeners; ++ i)
	{
		m_clientConnectionListeners[i]->OnClientEnteredGame(channelId, isReset, pActor->GetEntityId());
	}

	// in MP this is handled by GameRulesStandardState; in SP we need to trigger stats recording manually.
	if(!gEnv->bMultiplayer)
	{
		IGameRulesStatsRecording* pST = GetStatsRecordingModule();
		if (pST)
		{
			pST->OnInGameBegin();
		}
	}

	// Need to update the time of day serialisation chunk so that the new client can start at the right point
	// Note: Since we don't generally have a dynamic time of day, this will likely only effect clients
	// rejoining after a host migration since they won't be loading the value from the level
	CHANGED_NETWORK_STATE(this, GAMERULES_TIME_OF_DAY_DYNAMIC_ASPECT);

	return true;
}

//------------------------------------------------------------------------
void CGameRules::OnEntitySpawn(IEntity *pEntity)
{
}

//------------------------------------------------------------------------
void CGameRules::OnEntityRespawn(IEntity *pEntity)
{
	// Call entity script to allow custom logic
	EntityScripts::CallScriptFunction(pEntity, pEntity->GetScriptTable(), "OnEntityRespawn");
}

//------------------------------------------------------------------------
void CGameRules::OnEntityRemoved(IEntity *pEntity)
{
	if (gEnv->IsClient())
		SetTeam(0, pEntity->GetId());
}

//------------------------------------------------------------------------
void CGameRules::OnEntityReused(IEntity *pEntity, SEntitySpawnParams &params, EntityId prevId)
{
	if (gEnv->IsClient())
		SetTeam(0, prevId);
}

//------------------------------------------------------------------------
void CGameRules::OnItemDropped(EntityId itemId, EntityId actorId)
{
	i32 numListeners = m_pickupListeners.size();
	for (i32 i = 0; i < numListeners; ++ i)
	{
		m_pickupListeners[i]->OnItemDropped(itemId, actorId);
	}
}

//------------------------------------------------------------------------
void CGameRules::OnItemPickedUp(EntityId itemId, EntityId actorId)
{
	i32 numListeners = m_pickupListeners.size();
	for (i32 i = 0; i < numListeners; ++ i)
	{
		m_pickupListeners[i]->OnItemPickedUp(itemId, actorId);
	}

	if (actorId == g_pGame->GetIGameFramework()->GetClientActorId() && m_pHostMigrationClientParams)
	{
		if (!m_pHostMigrationClientParams->m_doneSetAmmo)
		{
			-- m_pHostMigrationClientParams->m_numExpectedItems;
			if (!m_pHostMigrationClientParams->m_numExpectedItems)
			{
				DrxLog("CGameRules::OnItemPickedUp, now received all expected items from host migration, setting ammo");
				IActor *pActor = g_pGame->GetIGameFramework()->GetClientActor();
				DRX_ASSERT(pActor);
				if (pActor)
				{
					IInventory *pInventory = pActor->GetInventory();
					for (i32 i = 0; i < m_pHostMigrationClientParams->m_numAmmoParams; ++ i)
					{
						DrxLog("    %s : %i", m_pHostMigrationClientParams->m_pAmmoParams[i].m_pAmmoClass->GetName(), m_pHostMigrationClientParams->m_pAmmoParams[i].m_count);
						// Set ammo locally so the HUD reports it correctly, we still have to tell the server though
						pInventory->SetAmmoCount(m_pHostMigrationClientParams->m_pAmmoParams[i].m_pAmmoClass, m_pHostMigrationClientParams->m_pAmmoParams[i].m_count);
						pInventory->RMIReqToServer_SetAmmoCount(m_pHostMigrationClientParams->m_pAmmoParams[i].m_pAmmoClass->GetName(), m_pHostMigrationClientParams->m_pAmmoParams[i].m_count);
					}

					EntityId currItemId = pInventory->GetCurrentItem();
					IItem *pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(currItemId);
					if (pItem && pItem->GetIWeapon())
					{
						SHUDEvent event;
						event.eventType = eHUDEvent_OnSetAmmoCount;
						event.eventPtrData = pItem->GetIWeapon();

						CHUDEventDispatcher::CallEvent(event);
					}

					if (m_pHostMigrationClientParams->m_pHolsteredItemClass)
					{
						DrxLog("  player had holstered item, class = '%s'", m_pHostMigrationClientParams->m_pHolsteredItemClass->GetName());
						EntityId holsteredItemId = pInventory->GetItemByClass(m_pHostMigrationClientParams->m_pHolsteredItemClass);
						if (holsteredItemId)
						{
							pInventory->SetHolsteredItem(holsteredItemId);
						}
						else
						{
							DrxLog("  ERROR: holstered item not in inventory");
						}
					}
				}

				m_pHostMigrationClientParams->m_doneSetAmmo = true;
				if (m_pHostMigrationClientParams->IsDone())
				{
					SAFE_DELETE(m_pHostMigrationClientParams);
				}
			}
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::OnPickupEntityDetached(EntityId entityId, EntityId actorId, bool isOnRemove, tukk pExtensionName)
{
	CCarryEntity *pCarryEntity = static_cast<CCarryEntity*>(g_pGame->GetIGameFramework()->QueryGameObjectExtension(entityId, pExtensionName));
	if (pCarryEntity)
	{
		pCarryEntity->AttachTo(0);
	}

	i32  numListeners = m_pickupListeners.size();
	for (i32 i=0; i<numListeners; i++)
	{
		m_pickupListeners[i]->OnPickupEntityDetached(entityId, actorId, isOnRemove);
	}
}

//------------------------------------------------------------------------
void CGameRules::OnPickupEntityAttached(EntityId entityId, EntityId actorId, tukk pExtensionName)
{
	CActor *pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(actorId));
	if(pActor && pActor->IsPlayer())
	{
		static_cast<CPlayer*>(pActor)->ExitPickAndThrow(true);
	}

	CCarryEntity *pCarryEntity = static_cast<CCarryEntity*>(g_pGame->GetIGameFramework()->QueryGameObjectExtension(entityId, pExtensionName));
	if (pCarryEntity)
	{
		pCarryEntity->AttachTo(actorId);
	}

	i32  numListeners = m_pickupListeners.size();
	for (i32 i=0; i<numListeners; i++)
	{
		m_pickupListeners[i]->OnPickupEntityAttached(entityId, actorId);
	}

	if (pActor)
	{
		if (pActor->IsClient())
		{
			CBattlechatter* pBattlechatter = GetBattlechatter();
			if(pBattlechatter)
			{
				pBattlechatter->LocalPlayerHasGotObjective();
			}
		}
		else
		{
			// someone else has the objective.. let them shout out about it
			BATTLECHATTER(BC_ObjectivePickup, actorId);
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::OnTextMessage(ETextMessageType type, tukk msg,
							   tukk p0, tukk p1, tukk p2, tukk p3)
{
	DrxLog("CGameRules::OnTextMessage(type=%d, msg='%s')", type, msg);
	INDENT_LOG_DURING_SCOPE();

	switch(type)
	{
		case eTextMessageServer:
		{
			SHUDEvent newServerMessage(eHUDEvent_OnServerMessage);
			newServerMessage.AddData(msg);
			CHUDEventDispatcher::CallEvent(newServerMessage);
		}
		break;

		default:
		DRX_ASSERT_MESSAGE( !"HUD MESSAGES", "Unhandled hud message." );
		break;
	}
}

//------------------------------------------------------------------------
void CGameRules::OnRevive(IActor *pActor)
{
	if (pActor->IsPlayer())
	{
		static_cast<CPlayer*>(pActor)->SpawnCorpse();
	}
}

//------------------------------------------------------------------------
void CGameRules::OnKill(IActor *pActor, const HitInfo &hitInfo, bool winningKill, bool firstKill, bool bulletTimeReplay)
{
	char weaponClassName[128];
	m_pGameFramework->GetNetworkSafeClassName(weaponClassName, sizeof(weaponClassName), hitInfo.weaponClassId);

  if ( gEnv->bServer && winningKill )
    GetVictoryConditionsModule()->SetWinningKillVictimShooter(hitInfo.targetId,hitInfo.shooterId);

	IActor *pShooterActor=gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(hitInfo.shooterId);

	if (pShooterActor != NULL && pShooterActor->IsPlayer())
	{
		CPlayer *pShooterPlayer=static_cast<CPlayer*>(pShooterActor);
		pShooterPlayer->RegisterKill(pActor, hitInfo.type);
	}

	const EntityId victimId = pActor->GetEntityId();

	if(!hitInfo.hitViaProxy)
	{
		if(!gEnv->bServer)
		{
			g_pGame->GetPersistantStats()->UpdateMultiKillStreak(hitInfo.shooterId, hitInfo.targetId);
		}

		bool isShooterClient = (hitInfo.shooterId == gEnv->pGame->GetIGameFramework()->GetClientActorId());
		if(isShooterClient)
		{
			CPlayerProgression::GetInstance()->SkillKillEvent(this, pActor, pShooterActor, hitInfo, firstKill);
		}
		else
		{
			CPlayerProgression::GetInstance()->SkillAssistEvent(this, pActor, pShooterActor, hitInfo);
		}

		if(gEnv->bMultiplayer && hitInfo.targetId != hitInfo.shooterId)
		{
			m_pClientHitEffectsMP->KillFeedback(static_cast<CActor*>(pActor), hitInfo);
		}
	}

	SHUDEvent battleLogEvent(eHUDEvent_OnNewBattleLogMessage);
	battleLogEvent.AddData( &hitInfo );
	battleLogEvent.AddData( static_cast<i32>(victimId) );
	battleLogEvent.AddData( weaponClassName );
	CHUDEventDispatcher::CallEvent(battleLogEvent);

	if (gEnv->bMultiplayer)
	{
		CRecordingSystem* pRecordingSystem = g_pGame->GetRecordingSystem();
		if (pRecordingSystem)
		{
			pRecordingSystem->OnKill(pActor, hitInfo, winningKill, bulletTimeReplay);
		}

		if (ActorShouldHideCurrentItemInsteadOfDroppingOnDeath(pActor))
		{
			bool  itemIsDroppable = false;
			CItem*  pCItem = static_cast<CItem*>(GetCurrentItemForActorWithStatus(pActor, NULL, &itemIsDroppable));
			if (pCItem != NULL && itemIsDroppable)
			{
				pCItem->Hide(true);
			}
		}
	}

	if (pActor->IsPlayer() && m_spawningModule)
	{
		m_spawningModule->OnPlayerKilled(hitInfo);
	}
}


//------------------------------------------------------------------------
void CGameRules::OnActorDeath( CActor* pActor )
{
	OnActorDeath_NotifyListeners( pActor );
}


//------------------------------------------------------------------------
void CGameRules::OnReviveInVehicle(IActor *pActor, EntityId vehicleId, i32 seatId, i32 teamId)
{
	SGameObjectEvent evt(eCGE_ActorRevive,eGOEF_ToAll, IGameObjectSystem::InvalidExtensionID, (uk )pActor);

	ScriptHandle handle(pActor->GetEntityId());
	ScriptHandle vhandle(pActor->GetEntityId());
	CallScript(m_clientScript, "OnReviveInVehicle", handle, vhandle, seatId, teamId);
}

//------------------------------------------------------------------------
void CGameRules::OnVehicleDestroyed(EntityId id)
{
	RemoveSpawnGroup(id);

	if (gEnv->bServer)
		CallScript(m_serverScript, "OnVehicleDestroyed", ScriptHandle(id));

	if (gEnv->IsClient())
		CallScript(m_clientScript, "OnVehicleDestroyed", ScriptHandle(id));
}

//------------------------------------------------------------------------
void CGameRules::OnVehicleSubmerged(EntityId id, float ratio)
{
	RemoveSpawnGroup(id);

	if (gEnv->bServer)
		CallScript(m_serverScript, "OnVehicleSubmerged", ScriptHandle(id), ratio);

	if (gEnv->IsClient())
		CallScript(m_clientScript, "OnVehicleSubmerged", ScriptHandle(id), ratio);
}

//------------------------------------------------------------------------
bool CGameRules::CanEnterVehicle( EntityId playerId )
{
	bool bResult = true;
	if (gEnv->bMultiplayer)
	{
		CPlayer *pPlayer = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(playerId));
		if (pPlayer)
		{
			if (pPlayer->IsDead() || pPlayer->GetActorStats()->bStealthKilled)
			{
				bResult = false;
			}
		}
	}
	return bResult;
}

//------------------------------------------------------------------------
bool CGameRules::IsGamemodeScoringEvent(EGameRulesScoreType pointsType) const
{
	switch(pointsType)
	{
	case EGRST_AON_Win:
	case EGRST_AON_Draw:
	case EGRST_CarryObjectiveTaken:
	case EGRST_CarryObjectiveRetrieved:
	case EGRST_CarryObjectiveCompleted:
	case EGRST_BombTheBaseCompleted:
	case EGRST_KingOfTheHillObjectiveHeld:
	case EGRST_CombiCapObj_Capturing_PerSec:
	case EGRST_PowerStruggle_CaptureSpear:
		return true;
	};

	return false;
}

//------------------------------------------------------------------------
void CGameRules::IncreasePoints(EntityId who, const SGameRulesScoreInfo & scoreInfo)
{
#ifndef _RELEASE
	if (g_pGameCVars->g_DisableScoring)
	{
		return;
	}
#endif

	DrxLog ("CGameRules::IncreasePoints [bServer=%d bClient=%d] rewarding %d points to player %s", gEnv->bServer, gEnv->IsClient(), scoreInfo.score, GetEntityName(who));
	INDENT_LOG_DURING_SCOPE();
	assert (gEnv->bServer);

	CCCPOINT(GameRules_SvModifyScore);

	IGameRulesStateModule *pStateModule = GetStateModule();

	// No scoring at game end
	if (pStateModule != NULL && pStateModule->GetGameState() != IGameRulesStateModule::EGRS_PostGame && scoreInfo.score != 0)
	{
		// Part 1: add to the magical and definitive server-side table of many scores! - Only if you have the module
		IGameRulesPlayerStatsModule *pPlayerStats = GetPlayerStatsModule();
		if (pPlayerStats)
		{
			pPlayerStats->IncreasePoints(who, scoreInfo.score);

			if (IsGamemodeScoringEvent(scoreInfo.type))
			{
				pPlayerStats->IncreaseGamemodePoints(who, scoreInfo.score);
			}
		}

		i32 teamScore = 0;
		i32k teamId = GetTeam(who);
		if (teamId)
		{
			teamScore = GetTeamsScore(teamId);
		}

		// Part 2: send an RMI to the machine belonging to the person who scored so they can display a message!
		IActor * whoActor =(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(who));
		DRX_ASSERT_MESSAGE(whoActor, string().Format("Can't give a score of %d to entity %d '%s' because it's not an actor", scoreInfo.score, who, GetEntityName(who)));
		if (whoActor)
		{
			CGameRules::ScoreChangeParams params(scoreInfo.data.PlayerKill.victim, scoreInfo.xp, scoreInfo.type, scoreInfo.xpRsn, teamScore);
			GetGameObject()->InvokeRMI(CGameRules::ClAddPoints(), params, eRMI_ToClientChannel, whoActor->GetChannelId());

			((CActor*)whoActor)->GetTelemetry()->OnIncreasePoints(scoreInfo.score, scoreInfo.type);
		}

		// Part 3: Tell listeners
		if(who == g_pGame->GetIGameFramework()->GetClientActorId())
		{
			ClientScoreEvent(scoreInfo.type, scoreInfo.xp, scoreInfo.xpRsn, teamScore);
		}
	}
#ifndef _RELEASE
	else
	{
		DrxLogAlways ("CGameRules::IncreasePoints NOT adding to score because game %s", pStateModule ? "has finished" : "has no 'state' module");
	}
#endif
}

//------------------------------------------------------------------------
void CGameRules::SvAddTaggedEntity(EntityId shooter, EntityId targetId, float time, ERadarTagReason reason)
{
	if(!gEnv->bServer) // server sends to all clients
		return;

	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(targetId);
	if( !pEntity )
	{
		return;
	}

	CActor* pTargetActor = (CActor*)m_pActorSystem->GetActor(targetId);
	if (pTargetActor)
	{
		pTargetActor->GetTelemetry()->OnTagged(shooter, time, reason);
	}

	const bool isMultiplayer = gEnv->bMultiplayer;

	CUICVars* pCvars = g_pGame->GetUI()->GetCVars();

	switch( reason )
	{
	// Tagging via visor/scan mode
	case eRTR_Tagging :
		{
			if( isMultiplayer )
			{
				if( !pCvars->hud_tagging_enabled )
					return;

				if( GetGameMode() == eGM_Assault )
				{
					i32 taggerTeam = GetTeam( shooter );

					if (!taggerTeam)		// If we haven't got a team (just joined or in spectator mode), default to 1 (since it's only used to determine which way round to show the scores)
						taggerTeam = 1;

					i32  attackingTeamId = GetRoundsModule()->GetPrimaryTeam();

					if( taggerTeam != attackingTeamId )
					{
						time = pCvars->hud_tagging_duration_assaultDefenders;
					}
				}
			}

			// OK
		}
		break;
	case eRTR_OnShot :
		{
			// OK
		}
		break;
	case eRTR_RadarOnly:
		{
			// OK
		}
		break;
	case eRTR_OnShoot:
		{
			DRX_ASSERT_MESSAGE(0, "ClTaggedEntity: Unhandled reason 'eRTR_OnShoot' in tagging RMI, ClTaggedEntity." ) ;
		}
		break;
	default :
		{
			DRX_ASSERT_MESSAGE(0, "ClTaggedEntity: Unhandled reason in tagging RMI, ClTaggedEntity." );
		}
		break;
	}

	TempRadarTaggingParams params(shooter, targetId, time, reason);

	if(GetTeamCount() > 1)
	{
		i32k shooterTeamId = GetTeam(shooter);
		i32k targetTeamId = GetTeam(targetId);
		if(shooterTeamId!=targetTeamId)
		{
			TPlayerTeamIdMap::const_iterator tit=m_playerteams.find(shooterTeamId);
			if (tit!=m_playerteams.end())
			{
				// send the tag information to all team mate
				for (TPlayers::const_iterator it=tit->second.begin(); it!=tit->second.end(); ++it)
				{
					GetGameObject()->InvokeRMI(ClTaggedEntity(), params, eRMI_ToClientChannel, GetChannelId(*it));
				}
			}
		}
	}
	else
	{
		// send the tag information to just the shooter.
		GetGameObject()->InvokeRMI(ClTaggedEntity(), params, eRMI_ToClientChannel, GetChannelId(shooter));
	}

	// Also send to the Target.
	GetGameObject()->InvokeRMI(ClTaggedEntity(), params, eRMI_ToClientChannel, GetChannelId(targetId));

	// add PP and CP for tagging this entity
	ScriptHandle shooterHandle(shooter);
	ScriptHandle targetHandle(targetId);
	CallScript(m_serverScript, "OnAddTaggedEntity", shooterHandle, targetHandle);
}

//------------------------------------------------------------------------
void CGameRules::RequestTagEntity(EntityId shooter, EntityId targetId, float time, ERadarTagReason reason )
{
	DrxLog("[tlh] @ CGameRules::RequestTagEntity()");

	g_pGame->GetPersistantStats()->HandleTaggingEntity(shooter, targetId);

	if (IGameRulesAssistScoringModule *assistScoringModule = GetAssistScoringModule())
	{
		if (reason == eRTR_Tagging)
		{
			HitInfo hitInfo;// empty hit info, the isTagEvent flag counts.
			hitInfo.shooterId = shooter;
			hitInfo.targetId = targetId;
			assistScoringModule->OnEntityHit(hitInfo, time);
		}
	}

	if (gEnv->bServer)
	{
		SvAddTaggedEntity(shooter, targetId, time, reason);  // this sends RMIs to the clients whose radars should be affected
	}
	else
	{
		i32k shooterTeamId = GetTeam(shooter);
		i32k targetTeamId = GetTeam(targetId);
		if(shooterTeamId!=targetTeamId || shooterTeamId == 0)
		{
			TempRadarTaggingParams params(shooter, targetId, time, reason);
			GetGameObject()->InvokeRMI(SvRequestTagEntity(), params, eRMI_ToServer);
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::OnKillMessage(EntityId targetId, EntityId shooterId)
{
#if ENABLE_PLAYER_KILL_RECORDING
	// check shooter and target exist.
	IActorSystem *pActorSystem = g_pGame->GetIGameFramework()->GetIActorSystem();
	assert(pActorSystem);
	IActor* target = pActorSystem->GetActor(targetId);
	IActor* shooter = pActorSystem->GetActor(shooterId);
	if( target && shooter )
	{
		IGameRulesPlayerStatsModule*  pPlayStatsMo = GetPlayerStatsModule();
		if( pPlayStatsMo )
		{
			//increment shooter's kill count on target
			pPlayStatsMo->IncreaseKillCount( shooterId, targetId );
		}
	}
#endif // ENABLE_PLAYER_KILL_RECORDING
}

//------------------------------------------------------------------------
IActor *CGameRules::SpawnPlayer(i32 channelId, tukk name, tukk className, const Vec3 &pos, const Ang3 &angles)
{
	if (!gEnv->bServer)
		return 0;

	IActor *pActor=GetActorByChannelId(channelId);
	if (!pActor)
		pActor = m_pActorSystem->CreateActor(channelId, VerifyName(name).c_str(), className, pos, Quat(angles), Vec3(1, 1, 1));

	return pActor;
}

void CGameRules::RevivePlayerMP(IActor *pActor, IEntity *pSpawnPoint, i32 teamId, bool clearInventory)
{
	if(!gEnv->bServer)
	{
		GameWarning("CGameRules::RevivePlayer() called on client");
		return;
	}
	assert(pSpawnPoint);

	u8 modelIndex = MP_MODEL_INDEX_DEFAULT;
	CEquipmentLoadout *pEquipmentLoadout = g_pGame->GetEquipmentLoadout();
	if (pEquipmentLoadout != NULL && pEquipmentLoadout->SvHasClientEquipmentLoadout(pActor->GetChannelId()))
	{
		modelIndex = pEquipmentLoadout->GetModelIndexOverride(pActor->GetChannelId());
	}

	const EntityId spawnPointId = pSpawnPoint->GetId();
	CActor *pCActor = static_cast<CActor*>(pActor);
	const Matrix34& rWorldTM = pSpawnPoint->GetWorldTM();

	RevivePlayer(pActor, rWorldTM.GetTranslation(), Ang3::GetAnglesXYZ(rWorldTM), teamId, modelIndex, clearInventory);
	GetSpawningModule()->SetLastSpawn(pActor->GetEntityId(), spawnPointId);
	i32 index = GetSpawningModule()->GetSpawnIndexForEntityId(spawnPointId);
	DrxLog("CGameRules::RevivePlayerMP() spawning at eid=%d, '%s' spawn index %d, position (%.3f, %.3f, %.3f)", spawnPointId, pSpawnPoint->GetName(), index, rWorldTM.GetTranslation().x, rWorldTM.GetTranslation().y, rWorldTM.GetTranslation().z);
	pActor->GetGameObject()->InvokeRMI(CActor::ClRevive(), CActor::ReviveParams(teamId, index, pCActor->GetNetPhysCounter(), modelIndex), eRMI_ToAllClients);

	if(strlen(g_pGameCVars->g_forceHeavyWeapon->GetString()) > 0)
	{
		SEntitySpawnParams params;
		params.sName = g_pGameCVars->g_forceHeavyWeapon->GetString();
		params.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(params.sName);
		params.nFlags |= (ENTITY_FLAG_NO_PROXIMITY|ENTITY_FLAG_NEVER_NETWORK_STATIC);

		if(IEntity *pHeavyWeaponEntity = gEnv->pEntitySystem->SpawnEntity(params))
		{
			if(CItem* pHeavyWeaponItem = pCActor->GetItem(pHeavyWeaponEntity->GetId()))
			{
				pHeavyWeaponItem->StartUse(pCActor->GetEntityId());
			}
		}
	}
}

void CGameRules::ClearInventory(IActor *pActor)
{
	IInventory *pInventory = pActor ? pActor->GetInventory() : NULL;

	if(pInventory)
	{
		if(!gEnv->bMultiplayer)
		{
			pInventory->Destroy(); // destroy calls clear for us
		}
		else
		{
			CPlayer * pPlayer = static_cast<CPlayer*>(pActor);
			pPlayer->DeselectWeapon();

			if(IItem * pCurrentItem = pPlayer->GetCurrentItem(true))
			{
				if(CWeapon* pCurrentWeapon = static_cast<CWeapon*>(pCurrentItem->GetIWeapon()))
				{
					if(pCurrentWeapon->IsHeavyWeapon())
					{
						pCurrentWeapon->StopUse( pPlayer->GetEntityId() );
					}
				}
				pCurrentItem->Select(false);
			}

			static IEntityClass *pNoWeaponClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("NoWeapon");
			static IEntityClass *pBinocularClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Binoculars");
			static IEntityClass *pPickAndThrowClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("PickAndThrowWeapon");

			EntityId noWeaponId = pInventory->GetItemByClass(pNoWeaponClass);
			EntityId binoId = pInventory->GetItemByClass(pBinocularClass);
			EntityId pickAndThrowId = pInventory->GetItemByClass(pPickAndThrowClass);

			if(noWeaponId)
				pInventory->RemoveItem(noWeaponId);

			if(binoId)
				pInventory->RemoveItem(binoId);

			if(pickAndThrowId)
				pInventory->RemoveItem(pickAndThrowId);

			if(gEnv->bServer)
				pInventory->Destroy(); // clients will receive the entity removed event from the server
			else
				pInventory->Clear(); // destroy calls clear on the server

			if(noWeaponId)
				pInventory->AddItem(noWeaponId);

			if(binoId)
				pInventory->AddItem(binoId);

			if(pickAndThrowId)
				pInventory->AddItem(pickAndThrowId);

			if(gEnv->bServer)
				pActor->GetGameObject()->InvokeRMI(CActor::ClClearInventory(), CActor::NoParams(), eRMI_ToRemoteClients);
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::RevivePlayer(IActor *pActor, const Vec3 &pos, const Ang3 &angles, i32 teamId, u8 modelIndex, bool clearInventory)
{
	DrxLog("CGameRules::RevivePlayer() actor=%s", pActor->GetEntity()->GetName());
	INDENT_LOG_DURING_SCOPE();

	if (!gEnv->bServer)
	{
		GameWarning("CGameRules::RevivePlayer() called on client");
		return;
	}

	const bool bIsClient = pActor->IsClient();
	CRecordingSystem* pRecordingSystem(NULL);
	if (bIsClient && (pRecordingSystem = g_pGame->GetRecordingSystem()))
	{
		pRecordingSystem->StartRecording();
	}

	if (clearInventory)
	{
		ClearInventory(pActor);
	}

	CActor* pCActor = static_cast<CActor*>(pActor);
	pCActor->NetReviveAt(pos, Quat(angles), teamId, modelIndex);

	if (gEnv->bMultiplayer && pCActor->GetSpectatorState()==CActor::eASS_None)
	{
		pCActor->SetSpectatorState(CActor::eASS_Ingame);
	}

	if(bIsClient)
	{
		g_pGame->GetUI()->ActivateDefaultState();
	}

	if (GetPlayerSetupModule())
	{
		GetPlayerSetupModule()->OnPlayerRevived(pActor->GetEntityId());
	}
	else
	{
		CallScript(m_script, "EquipPlayer", pActor->GetEntity()->GetScriptTable(), false);
	}

	m_pGameplayRecorder->Event(pActor->GetEntity(), GameplayEvent(eGE_Revive));

	if (m_statsRecordingModule)
	{
		DRX_ASSERT_MESSAGE(m_stateModule,"stats recording module requires an implementation of game state module to work. make sure there's one in this game modes XML");
		IGameRulesStateModule::EGR_GameState gameState = m_stateModule->GetGameState();
		if (gameState==IGameRulesStateModule::EGRS_InGame || ((g_pGameCVars->g_gameRules_preGame_StartSpawnedFrozen) && gameState==IGameRulesStateModule::EGRS_PreGame))
		{
			m_statsRecordingModule->OnPlayerRevived(pActor);
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::RevivePlayerInVehicle(IActor *pActor, EntityId vehicleId, i32 seatId, i32 teamId, bool clearInventory)
{
	if (!gEnv->bServer)
	{
		GameWarning("CGameRules::RevivePlayerInVehicle() called on client");
		return;
	}

	if (!pActor)
		return;

	CActor* pCActor = static_cast<CActor*>(pActor);

	// might get here with an invalid (-ve) seat id if all seats are currently occupied.
	// In that case we use the seat exit code to find a valid position to spawn at.
	if(seatId < 0)
	{
		IVehicle* pSpawnVehicle = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(vehicleId);
		Vec3 pos = ZERO;
		if(pSpawnVehicle != NULL && pSpawnVehicle->GetExitPositionForActor(pActor, pos, true))
		{
			Ang3 angles = pSpawnVehicle->GetEntity()->GetWorldAngles();	// face same direction as vehicle.
			RevivePlayer(pActor, pos, angles, teamId, MP_MODEL_INDEX_DEFAULT, clearInventory);
			return;
		}
	}

	if (IVehicle *pVehicle = pActor->GetLinkedVehicle())
	{
		if (IVehicleSeat *pSeat=pVehicle->GetSeatForPassenger(pActor->GetEntityId()))
			pSeat->Exit(false);
	}

	// stop using any mounted weapons before reviving
	if (CItem *pItem=static_cast<CItem *>(pActor->GetCurrentItem()))
	{
		if (pItem->IsMounted())
			pItem->StopUse(pActor->GetEntityId());
	}

	pCActor->SetHealth(100);
	pCActor->SetMaxHealth(100);

	if (!m_pGameFramework->IsChannelOnHold(pActor->GetChannelId()))
		pActor->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Alive);

	if (clearInventory && !gEnv->bMultiplayer)
	{
		IInventory *pInventory = pActor->GetInventory();
		pInventory->Destroy();
		pInventory->Clear();
	}

	pCActor->NetReviveInVehicle(vehicleId, seatId, teamId);

	m_pGameplayRecorder->Event(pActor->GetEntity(), GameplayEvent(eGE_Revive));
}

//------------------------------------------------------------------------
void CGameRules::RenamePlayer(IActor *pActor, tukk name)
{
	assert(pActor);
	IEntity* pActorEntity = pActor->GetEntity();
	string fixed=VerifyName(name, pActorEntity);
	const EntityId actorId = pActorEntity->GetId();
	RenameEntityParams params(actorId, fixed.c_str());
	if (!stricmp(fixed.c_str(), pActorEntity->GetName()))
		return;

	if (gEnv->bServer)
	{
		if (!gEnv->IsClient())
			pActor->GetEntity()->SetName(fixed.c_str());

		GetGameObject()->InvokeRMIWithDependentObject(ClRenameEntity(), params, eRMI_ToAllClients, params.entityId);

		if (INetChannel* pNetChannel = pActor->GetGameObject()->GetNetChannel())
			pNetChannel->SetNickname(fixed.c_str());

		m_pGameplayRecorder->Event(pActorEntity, GameplayEvent(eGE_Renamed, fixed));

		SHUDEventWrapper::PlayerRename(actorId);
	}
	else if (pActor->GetEntityId() == m_pGameFramework->GetClientActor()->GetEntityId())
	{
		GetGameObject()->InvokeRMIWithDependentObject(SvRequestRename(), params, eRMI_ToServer, params.entityId);
	}
}

//------------------------------------------------------------------------
string CGameRules::VerifyName(tukk name, IEntity *pEntity)
{
	const size_t cSizeLimit = 26;
	u32 nameBuffer32[cSizeLimit];
	Unicode::ConvertSafe<Unicode::eErrorRecovery_Discard>(nameBuffer32, name);

	// trim spaces and newlines at start/end
	u32* pName32 = nameBuffer32;
	while (*pName32 <= ' ')
	{
		++pName32;
	}
	u32* pEnd32 = pName32;
	while (*++pEnd32) {}
	while (pEnd32 != pName32 && pEnd32[-1] <= ' ')
	{
		*--pEnd32 = 0;
	}

	// no empty names
	if (pName32 == pEnd32)
	{
		static const char cEmpty[] = "empty";
		tukk pEmptyBegin = cEmpty;
		tukk pEmptyEnd = pEmptyBegin + sizeof(cEmpty); // Range intentionally includes null-terminator

		pName32 = nameBuffer32;
		pEnd32 = std::copy(pEmptyBegin, pEmptyEnd, pName32) - 1;
	}

	// no @ signs
	std::replace(pName32, pEnd32, u32('@'), u32('_'));

	// convert to UTF-8
	string nameFormatter;
	Unicode::Convert(nameFormatter, pName32, pEnd32);

	// search for duplicates
	if (IsNameTaken(nameFormatter.c_str(), pEntity))
	{
		i32 n=1;
		string appendix;
		do
		{
			appendix.Format("(%d)", n++);
		} while(IsNameTaken(nameFormatter+appendix));

		nameFormatter.append(appendix);
	}

	return nameFormatter;
}

//------------------------------------------------------------------------
bool CGameRules::IsNameTaken(tukk name, IEntity *pEntity)
{
	for (std::vector<i32>::const_iterator it=m_channelIds.begin(); it!=m_channelIds.end(); ++it)
	{
		IActor *pActor=GetActorByChannelId(*it);
		if (pActor != NULL && pActor->GetEntity()!=pEntity && !stricmp(name, pActor->GetEntity()->GetName()))
			return true;
	}

	return false;
}

//------------------------------------------------------------------------
void CGameRules::KillPlayer(IActor* pActor, const bool inDropItem, const bool inDoRagdoll, const HitInfo &inHitInfo)
{
	if(!gEnv->bServer)
		return;

	HitInfo hitInfo = inHitInfo;

	EntityId actorEid = pActor->GetEntityId();
	CActor* pCActor = static_cast<CActor*>(pActor);

	OnEntityKilledEarly(inHitInfo);

	EntityId itemIdToDrop = 0;

	bool  itemsDropped = false;
	bool  itemIsUsed = false;
	bool  itemIsDroppable = false;
	IItem*  pItem = GetCurrentItemForActorWithStatus(pActor, &itemIsUsed, &itemIsDroppable);
	if (pItem && itemIsUsed)
	{
		pItem->StopUse(actorEid);
	}
	else if (itemIsDroppable)
	{
		bool  dropItem = inDropItem;

		if (gEnv->bMultiplayer && dropItem)
		{
			dropItem = !ActorShouldHideCurrentItemInsteadOfDroppingOnDeath(pActor);
		}

		if (gEnv->IsEditor() && pActor->IsClient())
		{
			dropItem = false;
		}

		if (dropItem)
		{
			if (pItem)
			{
				EntityId itemId = pItem->GetEntityId();

				if(pActor->DropItem(itemId, 1.0f, false, true))
				{
					itemIdToDrop = itemId;
				}
			}
			if(!gEnv->bMultiplayer)
			{
				itemsDropped = true;
				pCActor->DropAttachedItems();
			}
		}
	}

	//Benito: Notify all remaining items which were not dropped
	//        Important for audio/animation to unload assets correctly
	if (!itemsDropped)
	{
		pCActor->NotifyInventoryAboutOwnerDeactivation();
	}


	char projectileClassName[128];
	bool foundProjectileClassName = m_pGameFramework->GetNetworkSafeClassName(projectileClassName, sizeof(projectileClassName), hitInfo.projectileClassId);
	if (!foundProjectileClassName)
	{
		drx_strcpy(projectileClassName, "unknown projectile");
	}

	IGameRulesAssistScoringModule *assistScoringModule = GetAssistScoringModule();
	if (assistScoringModule && actorEid == hitInfo.shooterId)
	{
		// If it was a suicide find the most recent attacker and use that as the shooterId if found
		EntityId recentShooter = assistScoringModule->SvGetMostRecentAttacker(actorEid);
		if (recentShooter)
		{
			hitInfo.shooterId = recentShooter;
		}
	}

	IGameRulesScoringModule *scoringModule = GetScoringModule();
	if (scoringModule)
	{
		scoringModule->DoScoringForDeath(pActor, hitInfo.shooterId, (i32)hitInfo.damage, hitInfo.partId, hitInfo.type);
	}

	IGameRulesStateModule *pStateModule = GetStateModule();
	IActor *pShooter = m_pGameFramework->GetIActorSystem()->GetActor(hitInfo.shooterId);

	CActor::KillParams params;
	params.shooterId = hitInfo.shooterId;
	params.targetId = hitInfo.targetId;
	params.weaponId = hitInfo.weaponId;
	params.projectileId = hitInfo.projectileId;
	params.itemIdToDrop = itemIdToDrop;
	params.weaponClassId = hitInfo.weaponClassId;
	params.damage = hitInfo.damage;
	params.material = -1;
	params.hit_type = hitInfo.type;
	params.hit_joint = hitInfo.partId;
	params.projectileClassId = hitInfo.projectileClassId;
	params.penetration = hitInfo.penetrationCount;
#if USE_LAGOMETER
	params.lagOMeterHitId = hitInfo.lagOMeterHitId;
#endif
	params.firstKill = false;
	params.killViaProxy = hitInfo.hitViaProxy;
	params.forceLocalKill = hitInfo.forceLocalKill;
	params.targetTeam = GetTeam(hitInfo.targetId);

	if(!hitInfo.hitViaProxy)
	{
		params.firstKill = SkillKill::IsFirstBlood(pShooter, pActor);

		g_pGame->GetPersistantStats()->UpdateMultiKillStreak(hitInfo.shooterId, hitInfo.targetId);

		if (foundProjectileClassName)
		{
			IEntityClass* pProjectileClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(projectileClassName);
			const SAmmoParams *pAmmoParams = g_pGame->GetWeaponSystem()->GetAmmoParams(pProjectileClass);
			if (pAmmoParams != NULL && pAmmoParams->pBulletTimeParams && !pAmmoParams->pBulletTimeParams->geometryName.empty())
			{
				// Server determines if it is a headshot and whether or not to play a bullet time replay in killcam
				// (this is important because the shooter and victim need to agree on this, actually the client could
				// predict this but it used to be checking if it was a skill kill or not which couldn't be accurately
				// predicted by the client)
				if (pShooter)
				{
					Vec3 rel = pShooter->GetEntity()->GetWorldPos() - pCActor->GetEntity()->GetWorldPos();
					const float minDist = 2 * g_pGameCVars->kc_bulletHoverDist;
					if (rel.len2() > sqr(minDist))
					{
						params.bulletTimeReplay = pCActor->IsHeadShot(hitInfo) || pAmmoParams->pBulletTimeParams->always;
					}
				}
			}
		}
	}

	// for Instant action scoreLimitReached() requires this kill's score to be attributed to the player
	IGameRulesPlayerStatsModule *statsModule = GetPlayerStatsModule();
	if (statsModule)
	{
		statsModule->OnPlayerKilled(hitInfo);
	}

	params.winningKill = ((m_victoryConditionsModule && m_victoryConditionsModule->ScoreLimitReached()) && (pStateModule != NULL && pStateModule->GetGameState() != IGameRulesStateModule::EGRS_PostGame) && (!m_hasWinningKill));
	if (!params.winningKill && m_objectivesModule)
	{
		params.winningKill = m_objectivesModule->IsWinningKill(hitInfo);
	}
	m_hasWinningKill |= params.winningKill;

	params.dir = hitInfo.dir;
	params.impulseScale = hitInfo.impulseScale;
	params.ragdoll = inDoRagdoll;

	params.penetration = hitInfo.penetrationCount;

	DrxLog("[CGameRules::KillPlayer] HitJoint: %d  PartID: %d", hitInfo.partId, params.hit_joint);

	if(hitInfo.type == EHitType::Frag && pShooter)
	{
		CWeaponSystem* pWeaponSystem = g_pGame->GetWeaponSystem();
		const CProjectile* pProjectileSrc = pWeaponSystem && hitInfo.projectileId ? pWeaponSystem->GetProjectile(hitInfo.projectileId) : 0;
		static const IEntityClass* pC4ExplosiveClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("c4explosive");
		if(pProjectileSrc && pProjectileSrc->GetEntity()->GetClass() == pC4ExplosiveClass)
		{
			const EntityId stuckToEntityId = pProjectileSrc->GetStuckToEntityId();
			if(stuckToEntityId && pShooter->IsFriendlyEntity(stuckToEntityId))
			{
				pShooter->GetGameObject()->InvokeRMI(CPlayer::ClIncrementIntStat(), CPlayer::SIntStatParams(EIPS_C4AttachedToTeamMateKills), eRMI_ToClientChannel, pShooter->GetChannelId());
			}
		}
	}

	pCActor->NetKill(params);

	pActor->GetGameObject()->InvokeRMI(CActor::ClKill(), params, eRMI_ToAllClients|eRMI_NoLocalCalls);

	if (gEnv->bMultiplayer && g_pGameCVars->g_useNetSyncToSpeedUpRMIs)
	{
		gEnv->pNetwork->SyncWithGame(eNGS_ForceChannelTick);
	}

	m_pGameplayRecorder->Event(pActor->GetEntity(), GameplayEvent(eGE_Death));


	CStatsRecordingMgr		*sr=g_pGame->GetStatsRecorder();
	if (sr)
	{
		char weaponClassName[128];
		if (!m_pGameFramework->GetNetworkSafeClassName(weaponClassName, sizeof(weaponClassName), hitInfo.weaponClassId))
		{
			drx_strcpy(weaponClassName, "unknown weapon");
		}

		if(sr->ShouldRecordEvent(eSE_Death, pActor))
		{
			if (IStatsTracker *tracker = sr->GetStatsTracker(pActor) )
			{
				tracker->Event(eSE_Death, new CDeathStats(hitInfo.projectileId, hitInfo.shooterId, GetHitType(hitInfo.type, "unknown hit type"), weaponClassName, projectileClassName));
			}
		}

		if (hitInfo.shooterId && hitInfo.shooterId != actorEid && pShooter && sr->ShouldRecordEvent(eSE_Kill, pShooter))
		{
			m_pGameplayRecorder->Event(pShooter->GetEntity(), GameplayEvent(eGE_Kill, 0, 0, (uk )&hitInfo.weaponId));
			if ( IStatsTracker *tracker = sr->GetStatsTracker(pShooter) )
			{
				tracker->Event( eSE_Kill, new CKillStats(hitInfo.projectileId, hitInfo.targetId, GetHitType(hitInfo.type, "unknown hit type"), weaponClassName, projectileClassName));
			}
		}
	}

	OnEntityKilled(hitInfo);
}

//------------------------------------------------------------------------
void CGameRules::PostHitKillCleanup(IActor *pActor)
{
	if (m_statsRecordingModule)
	{
		m_statsRecordingModule->OnPlayerKilled(pActor);
	}
}

//------------------------------------------------------------------------
void CGameRules::MovePlayer(IActor *pActor, const Vec3 &pos, const Quat &orientation)
{
	IVehicle *pVehicle = static_cast<CActor*>(pActor)->GetLinkedVehicle();
	if(pVehicle)
	{
		pVehicle->ExitVehicleAtPosition(pActor->GetEntityId(), pos);
	}

	CActor::MoveParams params(pos, orientation);
	//move player on client
	pActor->GetGameObject()->InvokeRMI(CActor::ClMoveTo(), params, eRMI_ToClientChannel|eRMI_NoLocalCalls, pActor->GetChannelId());

	//move player on server
	static_cast<CActor*>(pActor)->OnTeleported();
	pActor->GetEntity()->SetWorldTM(Matrix34::Create(Vec3(1,1,1), params.rot, params.pos));
}


//------------------------------------------------------------------------
void CGameRules::ChangeTeam(IActor *pActor, i32 teamId, bool onlyIfUnassigned)
{
	if (teamId!=0 && teamId==GetTeam(pActor->GetEntityId()))
		return;

	ChangeTeamParams params(pActor->GetEntityId(), teamId, onlyIfUnassigned);

	if (gEnv->bServer)
	{
		if (m_teamsModule)
		{
			m_teamsModule->RequestChangeTeam(pActor->GetEntityId(), teamId, onlyIfUnassigned);
		}
		else
		{
			ScriptHandle handle(params.entityId);
			CallScript(m_serverStateScript, "OnChangeTeam", handle, params.teamId);
		}
	}
	else if (pActor->GetEntityId() == m_pGameFramework->GetClientActor()->GetEntityId())
		GetGameObject()->InvokeRMIWithDependentObject(SvRequestChangeTeam(), params, eRMI_ToServer, params.entityId);
}

//------------------------------------------------------------------------
void CGameRules::ChangeTeam(IActor *pActor, tukk teamName, bool onlyIfUnassigned)
{
	if (!teamName)
		return;

	i32 teamId=GetTeamId(teamName);

	if (!teamId)
	{
		DrxLog("Invalid team: %s", teamName);
		return;
	}

	ChangeTeam(pActor, teamId, onlyIfUnassigned);
}

//------------------------------------------------------------------------
i32 CGameRules::GetPlayerCount(bool inGame, bool includeSpectators) const
{
	if (!inGame)
		return (i32)m_channelIds.size();

	i32 count=0;
	for (std::vector<i32>::const_iterator it=m_channelIds.begin(); it!=m_channelIds.end(); ++it)
	{
		if (IsChannelInGame(*it))
		{
			CActor *pActor = static_cast<CActor*>(GetActorByChannelId(*it));
			if (pActor && (includeSpectators || pActor->GetSpectatorState() != CActor::eASS_SpectatorMode))
			{
				++count;
			}
		}
	}

	return count;
}

//------------------------------------------------------------------------
i32 CGameRules::GetPlayerCountClient() const
{
	i32 playersCount= 0;
	if(m_playerStatsModule)
	{
		playersCount = m_playerStatsModule->GetNumPlayerStats();
	}
	return playersCount;
}

//------------------------------------------------------------------------
i32 CGameRules::GetSpectatorCount(bool inGame) const
{
	i32 count=0;
	for (std::vector<i32>::const_iterator it=m_channelIds.begin(); it!=m_channelIds.end(); ++it)
	{
		IActor *pActor = GetActorByChannelId(*it);
		if (pActor != NULL && (pActor->GetSpectatorMode() != 0))
		{
			if (!inGame || IsChannelInGame(*it))
				++count;
		}
	}

	return count;
}

//------------------------------------------------------------------------
EntityId CGameRules::GetPlayer(i32 idx)
{
	if (idx >= 0 && idx < (i32)m_channelIds.size())
	{
		IActor *pActor = GetActorByChannelId(m_channelIds[idx]);
		return pActor ? pActor->GetEntityId() : 0;
	}

	return 0;
}

//------------------------------------------------------------------------
void CGameRules::GetPlayers(TPlayers &players) const
{
	players.resize(0);
	players.reserve(m_channelIds.size());

	IActorSystem *pActorSystem = g_pGame->GetIGameFramework()->GetIActorSystem();
	IActorIteratorPtr it = pActorSystem->CreateActorIterator();
	while (IActor *pActor = it->Next())
	{
		if (pActor->GetChannelId())
		{
			players.push_back(pActor->GetEntityId());
		}
	}
}

void CGameRules::GetPlayersClient(TPlayers &players)
{
	players.resize(0);

	IGameRulesPlayerStatsModule* pPlayStatsMod = GetPlayerStatsModule();
	if(pPlayStatsMod)
	{
		i32k numStats = pPlayStatsMod->GetNumPlayerStats();
		players.reserve(numStats);
		for (i32 i=0; i<numStats; i++)
		{
			const SGameRulesPlayerStat* pPlayerStats = pPlayStatsMod->GetNthPlayerStats(i);
			players.push_back(pPlayerStats->playerId);
		}
	}
}


//------------------------------------------------------------------------
// returns true if the id is a player
//------------------------------------------------------------------------
bool CGameRules::IsPlayer(EntityId playerId) const
{
	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(playerId);

	return (pActor != NULL && pActor->IsPlayer());
}


//------------------------------------------------------------------------
bool CGameRules::IsPlayerInGame(EntityId playerId) const
{
	INetChannel *pNetChannel=g_pGame->GetIGameFramework()->GetNetChannel(GetChannelId(playerId));
	if (pNetChannel != NULL && pNetChannel->GetContextViewState()>=eCVS_InGame)
		return true;
	return false;
}

//------------------------------------------------------------------------
bool CGameRules::IsPlayerActivelyPlaying(EntityId playerId, bool mustBeAlive) const
{
	if(!gEnv->bMultiplayer)
		return true;

	// 'actively playing' means they have selected a team / joined the game.

	if(GetTeamCount() == 1)
	{
		IActor* pActor = reinterpret_cast<IActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(playerId));
		if(!pActor)
			return false;

		if (mustBeAlive)
			return (!pActor->IsDead() && pActor->GetSpectatorMode() == CActor::eASM_None);
		else
			return (!pActor->IsDead() || pActor->GetSpectatorMode() == CActor::eASM_None);
	}
	else
	{
		// in PS/TIA, out of the game if not yet on a team
		if(!mustBeAlive)
			return (GetTeam(playerId) != 0 );

		CActor* pActor = reinterpret_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(playerId));
		if(!pActor)
			return false;
		return (!pActor->IsDead() && GetTeam(playerId) != 0);

	}
}

//------------------------------------------------------------------------
bool CGameRules::IsChannelInGame(i32 channelId) const
{
	INetChannel *pNetChannel=g_pGame->GetIGameFramework()->GetNetChannel(channelId);
	if (pNetChannel != NULL && pNetChannel->GetContextViewState()>=eCVS_InGame)
		return true;
	return false;
}

//------------------------------------------------------------------------
void CGameRules::StartVoting(IActor *pActor, EVotingState t, EntityId id, tukk param)
{
  if(!pActor || !g_pGameCVars->sv_votingEnable)
    return;

  StartVotingParams params(t,id,param);
  EntityId entityId = pActor->GetEntityId();

  if (gEnv->bServer)
  {
    if(!m_pVotingSystem)
      return;
    CTimeValue st;
    CTimeValue curr_time = gEnv->pTimer->GetFrameStartTime();

    if(!m_pVotingSystem->GetCooldownTime(entityId,st) || (curr_time-st).GetSeconds()>g_pGame->GetCVars()->sv_votingCooldown)
    {
			if (t == eVS_kick)
			{
				if (entityId == id)
				{
					DrxLog("Player %s cannot vote for themselves",pActor->GetEntity()->GetName());
					return;
				}
				else
				{
					i32k totalPlayers = GetPlayerCount(false) - 1; // -1 has player being voted cannot vote
					i32k votersRequired = KickVotesTotalRequired();

					// Is there actually enough players able to vote (e.g. if there's only 2 players that's too little).
					const bool bValid = (totalPlayers >= votersRequired);
					if (!bValid)
					{
						DrxLog("Cannot start vote kick as there is not enough possible voters");
						return;
					}
				}
			}

      if(m_pVotingSystem->StartVoting(entityId,curr_time,t,id,param))
      {
        m_pVotingSystem->Vote(entityId,GetTeam(entityId), true);

				IEntity * pTargetEntity = gEnv->pEntitySystem->GetEntity(id);
				if(t == eVS_kick && pTargetEntity)
				{
					CGameLobby * pGameLobby = g_pGame->GetGameLobby();
					if(pGameLobby)
					{
						i32k totalVotesNeeded = i32(ceilf(GetPlayerCount(false)*g_pGame->GetCVars()->sv_votingRatio));
						i32k votesFor = m_pVotingSystem->GetNumVotesFor();
						i32k votesAgainst = m_pVotingSystem->GetNumVotesAgainst();

						// Need to convert frame time diff to game time so it is in sync on clients
						const float curFrameDiff = (gEnv->pTimer->GetFrameStartTime().GetSeconds() - m_pVotingSystem->GetVotingStartTime().GetSeconds());
						const float voteStartTimeGame = GetCurrentGameTime() - curFrameDiff;
						const float voteEndTimeGame = voteStartTimeGame + g_pGame->GetCVars()->sv_votingTimeout;

						DrxLog("Voting: Player '%s' starting kick vote against player '%s'", pActor->GetEntity()->GetName(), pTargetEntity->GetName());
						KickVoteParams _params(id, entityId, totalVotesNeeded, votesFor, votesAgainst, voteEndTimeGame, eKS_StartVote);
						GetGameObject()->InvokeRMI(ClKickVoteStatus(), _params, eRMI_ToAllClients);
					}
				}
				else
					SendChatMessage(eChatToAll, id, 0, "@mp_vote_initialized_nextmap");
      }
    }
    else
    {
      DrxLog("Player %s cannot start voting yet",pActor->GetEntity()->GetName());
    }
  }
  else if (pActor->GetEntityId() == m_pGameFramework->GetClientActor()->GetEntityId())
	{
		if(t == eVS_kick)
		{
			IEntity * pEntity = gEnv->pEntitySystem->GetEntity(id);
			if(pEntity)
				DrxLog("Voting: Requesting kick vote against player '%s'", pEntity->GetName());
		}

    GetGameObject()->InvokeRMIWithDependentObject(SvStartVoting(), params, eRMI_ToServer, entityId);
	}
}


//------------------------------------------------------------------------
i32 CGameRules::KickVotesTotalRequired()
{
	SCVars* pGameCvars = g_pGame->GetCVars();
	i32k totalPlayers = GetPlayerCount(false) - 1; // -1 has player being voted cannot vote
	i32k totalVotesNeeded = max(i32(ceilf(totalPlayers*pGameCvars->sv_votingRatio)), pGameCvars->sv_votingMinVotes);
	return totalVotesNeeded;
}

//------------------------------------------------------------------------
bool CGameRules::KickVoteConditionsMet(bool &bSuccess)
{
	i32k votesFor = m_pVotingSystem->GetNumVotesFor();
	i32k votesAgainst = m_pVotingSystem->GetNumVotesAgainst();

	i32k totalPlayers = GetPlayerCount(false) - 1; // -1 has player being voted cannot vote
	i32k votersRemaining = max(totalPlayers - votesFor - votesAgainst, 0);
	i32k votersRequired = max(KickVotesTotalRequired() - votesFor, 0);

	bSuccess = (votersRequired <= 0);

	return (bSuccess || (votersRemaining < votersRequired));
}

//------------------------------------------------------------------------
void CGameRules::UpdateKickVoteStatus(EntityId lastVoterId)
{
	SCVars* pGameCvars = g_pGame->GetCVars();
	EntityId kickTargetId = m_pVotingSystem->GetEntityId();

	i32k totalPlayers = GetPlayerCount(false) - 1; // -1 has player being voted cannot vote
	i32k totalVotesNeeded = max(i32(ceilf(totalPlayers*pGameCvars->sv_votingRatio)), pGameCvars->sv_votingMinVotes);

	i32k votesFor = m_pVotingSystem->GetNumVotesFor();
	i32k votesAgainst = m_pVotingSystem->GetNumVotesAgainst();

	// Need to convert frame time diff to game time so it is in sync on clients
	const float curFrameDiff = (gEnv->pTimer->GetFrameStartTime().GetSeconds() - m_pVotingSystem->GetVotingStartTime().GetSeconds());
	const float voteStartTimeGame = GetCurrentGameTime() - curFrameDiff;
	const float voteEndTimeGame = voteStartTimeGame + g_pGame->GetCVars()->sv_votingTimeout;

	KickVoteParams params(kickTargetId, lastVoterId, totalVotesNeeded, votesFor, votesAgainst, voteEndTimeGame, eKS_VoteProgress);
	GetGameObject()->InvokeRMI(ClKickVoteStatus(), params, eRMI_ToAllClients);
}

//------------------------------------------------------------------------
void CGameRules::Vote(IActor* pActor, bool yes)
{
  if(!pActor)
    return;
  EntityId id = pActor->GetEntityId();

  if (gEnv->bServer)
  {
    if(!m_pVotingSystem || !g_pGameCVars->sv_votingEnable)
      return;
    if(m_pVotingSystem->CanVote(id) && m_pVotingSystem->IsInProgress())
    {
      m_pVotingSystem->Vote(id,GetTeam(id), yes);

			if (id == g_pGame->GetClientActorId())
			{
				// For if the client voting is the server - non-dedicated
				m_bClientKickVotedFor = yes;
			}

			CGameLobby * pGameLobby = g_pGame->GetGameLobby();
			EntityId kickTargetId = m_pVotingSystem->GetEntityId();
			IEntity * pEntity = gEnv->pEntitySystem->GetEntity(kickTargetId);

			bool bVoteSuccess = false;
			const bool bVoteFinished = KickVoteConditionsMet(bVoteSuccess);
			if(pGameLobby && pEntity && !bVoteFinished)
			{
				DrxLog("Voting: voted %s for kicking player '%s'", yes?"yes":"no", pEntity->GetName());
				UpdateKickVoteStatus(id);
			}
    }
    else
    {
      DrxLog("Player %s cannot vote",pActor->GetEntity()->GetName());
    }
  }
  else if (id == m_pGameFramework->GetClientActor()->GetEntityId() && g_pGameCVars->sv_votingEnable)
	{
		DrxLog("Voting: Sending 'yes' vote to server");
		if(yes)
			GetGameObject()->InvokeRMIWithDependentObject(SvVote(), NoParams(), eRMI_ToServer, id);
		else
			GetGameObject()->InvokeRMIWithDependentObject(SvVoteNo(), NoParams(), eRMI_ToServer, id);

		m_bClientKickVotedFor = yes;
	}
}

//------------------------------------------------------------------------
void CGameRules::EndVoting(bool success)
{
  if(!m_pVotingSystem || !gEnv->bServer)
    return;

  if(success)
  {
    DrxLog("Voting \'%s\' succeeded.",m_pVotingSystem->GetSubject().c_str());
    switch(m_pVotingSystem->GetType())
    {
    case eVS_consoleCmd:
      gEnv->pConsole->ExecuteString(m_pVotingSystem->GetSubject());
      break;
    case eVS_kick:
      break;
    case eVS_nextMap:
      NextLevel();
      break;
    case eVS_changeMap:
      m_pGameFramework->ExecuteCommandNextFrame(string("map ")+m_pVotingSystem->GetSubject());
      break;
    case eVS_none:
      break;
    }
  }
  else
	{
		if(m_pVotingSystem->GetType() == eVS_kick)
		{
			CGameLobby * pGameLobby = g_pGame->GetGameLobby();
			EntityId kickTargetId = m_pVotingSystem->GetEntityId();
			IEntity * pEntity = gEnv->pEntitySystem->GetEntity(kickTargetId);
			if(pGameLobby && pEntity)
			{
				DrxLog("Voting: Ended kick vote against player '%s'. Failure.", pEntity->GetName());
				KickVoteParams params(kickTargetId, 0, 0, 0, 0, 0.f, eKS_VoteEnd_Failure);
				GetGameObject()->InvokeRMI(ClKickVoteStatus(), params, eRMI_ToAllClients);
			}
		}
		else
			DrxLog("Voting \'%s\' ended.",m_pVotingSystem->GetSubject().c_str());
	}

  m_pVotingSystem->EndVoting();
}

//------------------------------------------------------------------------
i32 CGameRules::CreateTeam(tukk name)
{
	TTeamIdMap::iterator it = m_teams.find(CONST_TEMP_STRING(name));
	if (it != m_teams.end())
		return it->second;

	m_teams.insert(TTeamIdMap::value_type(name, ++m_teamIdGen));
	m_playerteams.insert(TPlayerTeamIdMap::value_type(m_teamIdGen, TPlayers()));

	i32 startTeamScore = 0;
	if (m_scoringModule)
	{
		startTeamScore = m_scoringModule->GetStartTeamScore();
	}

	DRX_TODO(08, 09, 2009, "Team scores need moving into some kind of team info when there's a teams module. Hopefully will hold player, name etc. instead of all these maps.");
	m_teamscores.insert(TTeamScoresMap::value_type(m_teamIdGen, STeamScore(startTeamScore,0)));

	CStatsRecordingMgr		*sr=g_pGame->GetStatsRecorder();
	if(sr)
	{
		sr->AddTeam(m_teamIdGen, name);
	}

	return m_teamIdGen;
}

//------------------------------------------------------------------------
void CGameRules::CreateTeamAlias(tukk name, i32 teamId)
{
	m_teamAliases.insert(TTeamIdMap::value_type(name, teamId));
}

//------------------------------------------------------------------------
void CGameRules::RemoveTeam(i32 teamId)
{
	if(!stl::member_find_and_erase(m_teams, CONST_TEMP_STRING(GetTeamName(teamId))))
		return;

	for (TEntityTeamIdMap::iterator eit=m_entityteams.begin(); eit != m_entityteams.end(); ++eit)
	{
		if (eit->second == teamId)
			eit->second = 0; // 0 is no team
	}

	stl::member_find_and_erase(m_playerteams, teamId);
	stl::member_find_and_erase(m_teamscores, teamId);
}

//------------------------------------------------------------------------
tukk CGameRules::GetTeamName(i32 teamId) const
{
	for (TTeamIdMap::const_iterator it = m_teams.begin(); it!=m_teams.end(); ++it)
	{
		if (teamId == it->second)
			return it->first;
	}

	return 0;
}

//------------------------------------------------------------------------
i32 CGameRules::GetTeamId(tukk name) const
{
	TTeamIdMap::const_iterator it = m_teams.find(CONST_TEMP_STRING(name));
	if (it!=m_teams.end())
	{
		return it->second;
	}

	it = m_teamAliases.find(CONST_TEMP_STRING(name));
	if (it != m_teamAliases.end())
	{
		return it->second;
	}

	return 0;
}

//------------------------------------------------------------------------
// Doesn't seem to work on clients if inGame==true!
i32 CGameRules::GetTeamPlayerCount(i32 teamId, bool inGame) const
{
	if (!inGame)
	{
		TPlayerTeamIdMap::const_iterator it=m_playerteams.find(teamId);
		if (it!=m_playerteams.end())
			return (i32)it->second.size();
		return 0;
	}
	else
	{
		TPlayerTeamIdMap::const_iterator it=m_playerteams.find(teamId);
		if (it!=m_playerteams.end())
		{
			i32 count=0;

			const TPlayers &players=it->second;
			for (TPlayers::const_iterator pit=players.begin(); pit!=players.end(); ++pit)
				if (IsPlayerInGame(*pit))
					++count;

			return count;
		}
		return 0;
	}
}

//------------------------------------------------------------------------
// NOTE "flagsNeeded" are SGameRulesPlayerStat::EFlags flags
i32 CGameRules::GetTeamPlayerCountWithStatFlags(i32k teamId, i32k flagsNeeded, const bool needAllFlags)
{
	i32  count = 0;

	if (IGameRulesPlayerStatsModule* pPlayStatsMod=GetPlayerStatsModule())
	{
		i32k  numStats = pPlayStatsMod->GetNumPlayerStats();
		for (i32 i=0; i<numStats; i++)
		{
			const SGameRulesPlayerStat*  s = pPlayStatsMod->GetNthPlayerStats(i);

			i32k  maskedFlags = (s->flags & flagsNeeded);

			if ((needAllFlags && (maskedFlags == flagsNeeded)) || (!needAllFlags && (maskedFlags != 0)))
			{
				if (GetTeam(s->playerId) == teamId)
				{
					count++;
				}
			}
		}
	}

	return count;
}

//------------------------------------------------------------------------
i32 CGameRules::GetTeamPlayerCountWhoHaveSpawned(i32 teamId)
{
	return GetTeamPlayerCountWithStatFlags(teamId, SGameRulesPlayerStat::PLYSTATFL_HASSPAWNEDTHISROUND, true);
}

//------------------------------------------------------------------------
void CGameRules::SetTeamsScore(i32 teamId, i32 score)
{
	DRX_ASSERT(gEnv->bServer);
	if (g_pGameCVars->g_scoreLimit && (score > g_pGameCVars->g_scoreLimit))
	{
		score = g_pGameCVars->g_scoreLimit;
	}
	TTeamScoresMap::iterator it=m_teamscores.find(teamId);
	if (it!=m_teamscores.end())
	{
		if(gEnv->IsClient())
		{
			ClientTeamScoreFeedback(teamId, it->second.m_teamScore, score);
		}
		it->second.m_teamScore = score;
		CHANGED_NETWORK_STATE(this, GAMERULES_TEAMS_SCORE_ASPECT);
	}
}

//------------------------------------------------------------------------
i32 CGameRules::GetTeamsScore(i32 teamId) const
{
	TTeamScoresMap::const_iterator it=m_teamscores.find(teamId);
	if (it!=m_teamscores.end())
		return it->second.m_teamScore;

	return 0;
}

//------------------------------------------------------------------------
void CGameRules::SetTeamRoundScore(i32 teamId, i32 score)
{
	DRX_ASSERT(gEnv->bServer);
	TTeamScoresMap::iterator it=m_teamscores.find(teamId);
	if (it!=m_teamscores.end())
	{
		it->second.m_roundTeamScore = score;
		CHANGED_NETWORK_STATE(this, GAMERULES_TEAMS_SCORE_ASPECT);
	}
}

//------------------------------------------------------------------------
i32 CGameRules::SvGetTeamsScoreScoredThisRound(i32 teamId) const
{
	DRX_ASSERT(gEnv->bServer);

	TTeamScoresMap::const_iterator it=m_teamscores.find(teamId);
	if (it!=m_teamscores.end())
		return (it->second.m_teamScore - it->second.m_teamScoreRoundStart);

	return 0;
}

//------------------------------------------------------------------------
i32 CGameRules::GetTeamRoundScore(i32 teamId) const
{
	TTeamScoresMap::const_iterator it=m_teamscores.find(teamId);
	if (it!=m_teamscores.end())
		return it->second.m_roundTeamScore;

	return 0;
}

//------------------------------------------------------------------------
void CGameRules::SvCacheRoundStartTeamScores()
{
	DRX_ASSERT(gEnv->bServer);
	for (TTeamScoresMap::iterator iter=m_teamscores.begin(); iter!=m_teamscores.end(); ++iter)
	{
		iter->second.m_teamScoreRoundStart = iter->second.m_teamScore;
		CHANGED_NETWORK_STATE(this, GAMERULES_TEAMS_SCORE_ASPECT);
	}
}

//------------------------------------------------------------------------
void CGameRules::ClientTeamScoreFeedback(i32 teamId, i32 prevScore, i32 newScore)
{
	DRX_ASSERT(gEnv->IsClient());

	if(m_rulesListeners.empty() == false)
	{
		TGameRulesListenerVec::iterator  iter = m_rulesListeners.begin();
		while (iter != m_rulesListeners.end())
		{
			(*iter)->ClTeamScoreFeedback(teamId, prevScore, newScore);
			++iter;
		}
	}

	//Should this go into scoring module so the module determines if there should be an 'in the lead' announcement?
	i32 clientTeam = GetTeam(gEnv->pGame->GetIGameFramework()->GetClientActorId());

	if(teamId == clientTeam)
	{
		i32 otherTeamScore = GetTeamsScore(3 - clientTeam);
		if(prevScore <= otherTeamScore && newScore > otherTeamScore)
		{
			if(!m_bClientTeamInLead)	//Stops re-triggering if scores go 1, 0 then 1, 1 then 2, 1
			{
				CAnnouncer::GetInstance()->Announce("TakenTheLead", CAnnouncer::eAC_inGame);
			}
			m_bClientTeamInLead = true;
		}
	}
	else
	{
		i32 clientTeamScore = GetTeamsScore(clientTeam);
		if(prevScore <= clientTeamScore && newScore > clientTeamScore)
		{
			if(m_bClientTeamInLead)	//Must have been in the lead to lose it
			{
				CAnnouncer::GetInstance()->Announce("LostTheLead", CAnnouncer::eAC_inGame);
			}
			m_bClientTeamInLead = false;
		}
	}
}

bool CGameRules::HasVotingCooldownEnded (float &timeLeft) const
{
	if (m_ClientCooldownEndTime>=0.f)
	{
		const CTimeValue frameStart = gEnv->pTimer->GetFrameStartTime();
		if (frameStart < m_ClientCooldownEndTime)
		{
			timeLeft = (m_ClientCooldownEndTime - frameStart).GetSeconds();
			return false;
		}
	}
	return true;
}

bool CGameRules::IndividualScore () const
{
	if (!m_bIsTeamGame)
		return true;

	if (m_objectivesModule)
	{
		return m_objectivesModule->IndividualScore();
	}

	return false;
}

bool CGameRules::ShowRoundsAsDraw() const
{
	if (m_objectivesModule)
	{
		return m_objectivesModule->ShowRoundsAsDraw();
	}

	return false;
}

//------------------------------------------------------------------------
// skipPlayerId - newly spawned player, might have not health yet (if respawning in game), but must be considered alive
i32 CGameRules::GetTotalAlivePlayerCount( const EntityId skipPlayerId ) const
{
	i32 count=0;
	for(TPlayerTeamIdMap::const_iterator it=m_playerteams.begin(); it!=m_playerteams.end(); ++it)
	{
		const TPlayers &players=it->second;
		for (TPlayers::const_iterator pit=players.begin(); pit!=players.end(); ++pit)
			if ( skipPlayerId==(*pit) || IsPlayerActivelyPlaying(*pit, true))
				++count;
	}
	return count;
}

//------------------------------------------------------------------------
EntityId CGameRules::GetTeamActivePlayer(i32 teamId, i32 idx) const
{
	TPlayerTeamIdMap::const_iterator it=m_playerteams.find(teamId);
	if (it==m_playerteams.end())
		return 0;
	i32 count=0;
	const TPlayers &players=it->second;
	for (TPlayers::const_iterator pit=players.begin(); pit!=players.end(); ++pit)
		if ((IsPlayerActivelyPlaying(*pit, true)))
			if((count++)==idx)
				return (*pit);
	return 0;
}

//------------------------------------------------------------------------
EntityId CGameRules::GetTeamPlayer(i32 teamId, i32 idx)
{
	TPlayerTeamIdMap::const_iterator it=m_playerteams.find(teamId);
	if (it != m_playerteams.end())
	{
		if (idx >= 0 && idx < (i32)it->second.size())
			return it->second[idx];
	}

	return 0;
}

//------------------------------------------------------------------------
void CGameRules::GetTeamPlayers(i32 teamId, TPlayers &players)
{
	players.resize(0);
	TPlayerTeamIdMap::const_iterator it=m_playerteams.find(teamId);
	if (it!=m_playerteams.end())
		players=it->second;
}

//------------------------------------------------------------------------
bool CGameRules::SetTeam_Common(i32 teamId, EntityId entityId, bool& bIsPlayer)
{
	if (!entityId) // ignore these for now
		return false;

	i32 oldTeam = GetTeam(entityId);
	if (oldTeam==teamId)
		return false;

	stl::member_find_and_erase(m_entityteams, entityId);

	IActor *pActor=m_pActorSystem->GetActor(entityId);
	bool isplayer = (pActor != 0)  && (pActor->IsPlayer());
	bIsPlayer = isplayer;
	if (isplayer && oldTeam)
	{
		TPlayerTeamIdMap::iterator pit=m_playerteams.find(oldTeam);
		assert(pit!=m_playerteams.end());
		stl::find_and_erase(pit->second, entityId);
	}

	if (teamId)
	{
		m_entityteams.insert(TEntityTeamIdMap::value_type(entityId, teamId));

		if (isplayer)
		{
			TPlayerTeamIdMap::iterator pit=m_playerteams.find(teamId);
			assert(pit!=m_playerteams.end());
			pit->second.push_back(entityId);
		}
	}

	IEntity * pEntity = gEnv->pEntitySystem->GetEntity(entityId);
	if (!pEntity)
	{
		DrxLog("CGameRules::SetTeam_Common, tried to set team on NULL entity, id=%i", entityId);
		return false;
	}

	DrxLog("[RS] Entity '%s' has been set to be on team number %d", pEntity->GetName(), teamId);

	if(isplayer)
	{
#ifndef OLD_VOICE_SYSTEM_DEPRECATED
		ReconfigureVoiceGroups(entityId,oldTeam,teamId);
#endif
	}

	if(isplayer && m_spawningModule)
		m_spawningModule->OnSetTeam(entityId, teamId);

	IScriptTable *pEntityScript = pEntity->GetScriptTable();
	if (pEntityScript)
	{
		if (pEntityScript->GetValueType("OnSetTeam") == svtFunction)
		{
			m_pScriptSystem->BeginCall(pEntityScript, "OnSetTeam");
			m_pScriptSystem->PushFuncParam(pEntityScript);
			m_pScriptSystem->PushFuncParam(teamId);
			m_pScriptSystem->EndCall();
		}
	}

	const size_t numListeners = m_teamChangedListeners.size();
	for (size_t i = 0; i < numListeners; ++ i)
	{
		m_teamChangedListeners[i]->OnChangedTeam(entityId, oldTeam, teamId);
	}

	CRecordingSystem* pRecordingSystem = g_pGame->GetRecordingSystem();
	if (pRecordingSystem)
	{
		pRecordingSystem->OnSetTeam(entityId, teamId);
	}

	m_pGameplayRecorder->Event(pEntity, GameplayEvent(eGE_ChangedTeam, 0, (float)teamId));

	IGameObject* pGameObject = m_pGameFramework->GetGameObject(entityId);

	if(pGameObject)
	{
		STeamChangeInfo teamChangeInfo = {oldTeam, teamId};
		pGameObject->SendEvent(SGameObjectEvent(eCGE_SetTeam, eGOEF_ToAll, IGameObjectSystem::InvalidExtensionID, & teamChangeInfo));
	}

	if (teamId && !oldTeam)
	{
		DrxLog("[GameRules] AddEntityEventLister(%d(%s), ENTITY_EVENT_DONE, %p)", entityId, pEntity ? pEntity->GetName() : "null", this);
		// Entity is being put onto a team for the first time, add an event listener
		AddEntityEventDoneListener(entityId);
	}
	else if (!teamId && oldTeam)
	{
		DrxLog("[GameRules] ClDoSetTeam removing listener for %d(%s) %p", entityId, pEntity ? pEntity->GetName() : "null", this);
		RemoveEntityEventDoneListener(entityId);
	}

	return true;
}

//------------------------------------------------------------------------
void CGameRules::SetTeam(i32 teamId, EntityId entityId, bool clientOnly /*= false*/)
{
	if(!clientOnly)
	{
		assert(gEnv->bServer);
		if(!gEnv->bServer)
		{
			DrxLog("UNINTENTIONALLY SETTING TEAM ON CLIENT! This is wrong and can lead to confusion between server/client. Entity[%d] Team[%d]", entityId, teamId);
		}
	}

	bool bIsPlayer;
	if(SetTeam_Common(teamId, entityId, bIsPlayer))
	{
		if(bIsPlayer)
		{
			i32 channelId=GetChannelId(entityId);

			TChannelTeamIdMap::iterator it=m_channelteams.find(channelId);
			if (it!=m_channelteams.end())
			{
				if (!teamId)
					m_channelteams.erase(it);
				else
					it->second=teamId;
			}
			else if(teamId)
				m_channelteams.insert(TChannelTeamIdMap::value_type(channelId, teamId));

			if(m_spawningModule)
				m_spawningModule->OnSetTeam(entityId, teamId);

			CEquipmentLoadout *pEquipmentLoadout = g_pGame->GetEquipmentLoadout();
			if (pEquipmentLoadout)
			{
				pEquipmentLoadout->UpdateClassicModeModel((u16) channelId, teamId);
			}
		}
		// if this is a spawn group, update it's validity
		if (m_spawnGroups.find(entityId)!=m_spawnGroups.end())
			CheckSpawnGroupValidity(entityId);

		if(!clientOnly)
		{
			IEntity *pEntity = gEnv->pEntitySystem->GetEntity(entityId);
			if((pEntity) && !(pEntity->GetFlags() & (ENTITY_FLAG_SERVER_ONLY|ENTITY_FLAG_CLIENT_ONLY)))
			{
				GetGameObject()->InvokeRMIWithDependentObject(ClSetTeam(), SetTeamParams(entityId, teamId), eRMI_ToRemoteClients, entityId);
			}
		}
	}
}

//------------------------------------------------------------------------
i32 CGameRules::GetTeam(EntityId entityId) const
{
	TEntityTeamIdMap::const_iterator it = m_entityteams.find(entityId);
	if (it != m_entityteams.end())
		return it->second;

	return 0;
}

//------------------------------------------------------------------------
i32 CGameRules::GetChannelTeam(i32 channelId) const
{
	TChannelTeamIdMap::const_iterator it = m_channelteams.find(channelId);
	if (it != m_channelteams.end())
		return it->second;

	return 0;
}

//------------------------------------------------------------------------
CGameRules::eThreatRating CGameRules::GetThreatRating( const EntityId entityIdA, const EntityId entityIdB ) const
{
	if(entityIdA==entityIdB)
		return eFriendly;
	eThreatRating threat = eFriendly;
	if(GetTeamCount()<2 && GetThreatRatingWithoutTeams(entityIdA, entityIdB, threat))
		return threat;
	return GetThreatRatingByTeam(GetTeam(entityIdA), GetTeam(entityIdB));
}

CGameRules::eThreatRating CGameRules::GetThreatRatingByTeam( const int8 teamA, const int8 teamB ) const
{
	i32k teamCount = GetTeamCount();
	if(teamCount<2)
	{
		return eHostile;
	}
	else if(teamA!=0)
	{
		if(teamA==teamB || teamB==0)
			return eFriendly;
		else
			return eHostile;
	}
	else //If teamA is 0 in a team game then they are a Spectator.
	{
		if(teamB==1)
			return eFriendly;
		else
			return eHostile;
	}
}

bool CGameRules::GetThreatRatingWithoutTeams( const EntityId entityIdA, const EntityId entityIdB, eThreatRating& rThreat ) const
{
	IGameFramework* pGameFramework = g_pGame->GetIGameFramework();
	IActorSystem* pActorSys = pGameFramework->GetIActorSystem();

	{
		// A is linked to B (eg. a vehicle).
		if(IActor* pActorA = pActorSys->GetActor(entityIdA))
		{
			if(IEntity* pLinkedEntity = pActorA->GetLinkedEntity())
			{
				if(pLinkedEntity->GetId()==entityIdB)
				{
					rThreat = eFriendly;
					return true;
				}
			}
		}
		// B is a ReplayActor.
		if(CRecordingSystem* pRecordingSystem = g_pGame->GetRecordingSystem())
		{
			// If a ReplayActor is the Replay version of entityIdA.
			if(const CReplayActor* pReplayActor = pRecordingSystem->GetReplayActor(entityIdA, true))
			{
				if(pReplayActor->GetEntityId()==entityIdB)
				{
					rThreat = eFriendly;
					return true;
				}
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------
bool CGameRules::IsValidPlayerTeam(i32 teamId) const
{
	TPlayerTeamIdMap::const_iterator it=m_playerteams.find(teamId);
	return it!=m_playerteams.end();
}

void CGameRules::AddSpawnLocation(EntityId location, bool isInitialSpawn, bool doVisTest, tukk pGroupName)
{
	DRX_ASSERT_TRACE (m_spawningModule, ("No spawning module present while trying to add spawn location '%s'", GetEntityName(location)));
	m_spawningModule->AddSpawnLocation(location, isInitialSpawn, doVisTest, pGroupName);
}

void CGameRules::RemoveSpawnLocation(EntityId id, bool isInitialSpawn)
{
	DRX_ASSERT_TRACE (m_spawningModule, ("No spawning module present while trying to remove spawn location '%s'", GetEntityName(id)));
	m_spawningModule->RemoveSpawnLocation(id, isInitialSpawn);
}

void CGameRules::EnableSpawnLocation(EntityId location, bool isInitialSpawn, tukk pGroupName)
{
	DRX_ASSERT_TRACE (m_spawningModule, ("No spawning module present while trying to add spawn location '%s'", GetEntityName(location)));
	m_spawningModule->EnableSpawnLocation(location, isInitialSpawn, pGroupName);
}

void CGameRules::DisableSpawnLocation(EntityId id, bool isInitialSpawn)
{
	DRX_ASSERT_TRACE (m_spawningModule, ("No spawning module present while trying to remove spawn location '%s'", GetEntityName(id)));
	m_spawningModule->DisableSpawnLocation(id, isInitialSpawn);
}

i32 CGameRules::GetSpawnLocationCount() const
{
	DRX_ASSERT_TRACE (m_spawningModule, ("No spawning module present while trying to count spawn locations"));
	return m_spawningModule->GetSpawnLocationCount();
}

// EntityId CGameRules::GetSpawnLocation(i32 idx, bool initialSpawn) const
// {
// 	DRX_ASSERT_TRACE (m_spawningModule, ("No spawning module present while trying to look up spawn location %d", idx));
// 	return m_spawningModule->GetNthSpawnLocation(idx, initialSpawn);
// }

//------------------------------------------------------------------------
i32 CGameRules::GetEnemyTeamId(i32 myTeamId) const
{
	for(TPlayerTeamIdMap::const_iterator it=m_playerteams.begin(); it!=m_playerteams.end(); ++it)
	{
		if(it->first!=myTeamId)
			return it->first;
	}
	return -1;
}

//------------------------------------------------------------------------
EntityId CGameRules::GetFirstSpawnLocation(i32 teamId, EntityId groupId) const
{
	DRX_ASSERT_TRACE (m_spawningModule, ("No spawning module present while trying to get first spawn location for team=%d group='%s'", teamId, GetEntityName(groupId)));

	return m_spawningModule->GetFirstSpawnLocation(teamId);
}

//------------------------------------------------------------------------
void CGameRules::AddSpawnGroup(EntityId groupId)
{
	if (m_spawnGroups.find(groupId)==m_spawnGroups.end())
		m_spawnGroups.insert(TSpawnGroupMap::value_type(groupId, TSpawnLocations()));
}

//------------------------------------------------------------------------
void CGameRules::AddSpawnLocationToSpawnGroup(EntityId groupId, EntityId location)
{
	TSpawnGroupMap::iterator it=m_spawnGroups.find(groupId);
	if (it==m_spawnGroups.end())
		return;

	DrxLog("CGameRules::AddSpawnLocationToSpawnGroup() no longer resorting spawns. spawngroups unused at the moment");
	stl::push_back_unique(it->second, location);
}

//------------------------------------------------------------------------
void CGameRules::RemoveSpawnLocationFromSpawnGroup(EntityId groupId, EntityId location)
{
	TSpawnGroupMap::iterator it=m_spawnGroups.find(groupId);
	if (it==m_spawnGroups.end())
		return;

	DrxLog("CGameRules::RemoveSpawnLocationFromSpawnGroup() no longer resorting spawns. spawngroups unused at the moment");
	stl::find_and_erase(it->second, location);
}

//------------------------------------------------------------------------
void CGameRules::RemoveSpawnGroup(EntityId groupId)
{
	stl::member_find_and_erase(m_spawnGroups, groupId);

	DrxLog("CGameRules::RemoveSpawnGroup() no longer resorting spawns. spawngroups unused at the moment");

	CheckSpawnGroupValidity(groupId);
}

//------------------------------------------------------------------------
EntityId CGameRules::GetSpawnLocationGroup(EntityId spawnId) const
{
	for (TSpawnGroupMap::const_iterator it=m_spawnGroups.begin(); it!=m_spawnGroups.end(); ++it)
	{
		TSpawnLocations::const_iterator sit=std::find(it->second.begin(), it->second.end(), spawnId);
		if (sit!=it->second.end())
			return it->first;
	}

	return 0;
}

//------------------------------------------------------------------------
i32 CGameRules::GetSpawnGroupCount() const
{
	return (i32)m_spawnGroups.size();
}

//------------------------------------------------------------------------
EntityId CGameRules::GetSpawnGroup(i32 idx) const
{
	if (idx>=0 && idx<(i32)m_spawnGroups.size())
	{
		TSpawnGroupMap::const_iterator it=m_spawnGroups.begin();
		std::advance(it, idx);
		return it->first;
	}

	return 0;
}

//------------------------------------------------------------------------
void CGameRules::GetSpawnGroups(TSpawnLocations &groups) const
{
	groups.resize(0);
	groups.reserve(m_spawnGroups.size());
	for (TSpawnGroupMap::const_iterator it=m_spawnGroups.begin(); it!=m_spawnGroups.end(); ++it)
		groups.push_back(it->first);
}

//------------------------------------------------------------------------
bool CGameRules::IsSpawnGroup(EntityId id) const
{
	TSpawnGroupMap::const_iterator it=m_spawnGroups.find(id);
	return it!=m_spawnGroups.end();
}
//------------------------------------------------------------------------

bool CGameRules::AllowNullSpawnGroups() const
{
	bool allow = false;
	if (m_script->GetValueType("AllowNullSpawnGroups") == svtBool)
	{
		m_script->GetValue("AllowNullSpawnGroups", allow);
	}

	return allow;
}

//------------------------------------------------------------------------
void CGameRules::RequestSpawnGroup(EntityId spawnGroupId)
{
	CallScript(m_script, "RequestSpawnGroup", ScriptHandle(spawnGroupId));
}

//------------------------------------------------------------------------
void CGameRules::SetPlayerSpawnGroup(EntityId playerId, EntityId spawnGroupId)
{
	CallScript(m_script, "SetPlayerSpawnGroup", ScriptHandle(playerId), ScriptHandle(spawnGroupId));
}

//------------------------------------------------------------------------
EntityId CGameRules::GetPlayerSpawnGroup(IActor *pActor)
{
	if (m_script->GetValueType("GetPlayerSpawnGroup") != svtFunction)
		return 0;

	ScriptHandle ret(0);
	m_pScriptSystem->BeginCall(m_script, "GetPlayerSpawnGroup");
	m_pScriptSystem->PushFuncParam(m_script);
	m_pScriptSystem->PushFuncParam(pActor->GetEntity()->GetScriptTable());
	m_pScriptSystem->EndCall(ret);

	return (EntityId)ret.n;
}

//------------------------------------------------------------------------
void CGameRules::CheckSpawnGroupValidity(EntityId spawnGroupId)
{
	bool exists=spawnGroupId &&
		(m_spawnGroups.find(spawnGroupId)!=m_spawnGroups.end()) &&
		(gEnv->pEntitySystem->GetEntity(spawnGroupId)!=0);
	bool valid=exists && GetTeam(spawnGroupId)!=0;

	for (std::vector<i32>::const_iterator it=m_channelIds.begin(); it!=m_channelIds.end(); ++it)
	{
		IActor *pActor=GetActorByChannelId(*it);
		if (!pActor)
			continue;

		EntityId playerId=pActor->GetEntityId();
		if (GetPlayerSpawnGroup(pActor)==spawnGroupId)
		{
			if (!valid || GetTeam(spawnGroupId)!=GetTeam(playerId))
				CallScript(m_serverScript, "OnSpawnGroupInvalid", ScriptHandle(playerId), ScriptHandle(spawnGroupId));
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::AddHitListener(IHitListener* pHitListener)
{
	stl::push_back_unique(m_hitListeners, pHitListener);
}

//------------------------------------------------------------------------
void CGameRules::RemoveHitListener(IHitListener* pHitListener)
{
	stl::find_and_erase(m_hitListeners, pHitListener);
}

//------------------------------------------------------------------------
void CGameRules::AddGameRulesListener(SGameRulesListener* pRulesListener)
{
	stl::push_back_unique(m_rulesListeners, pRulesListener);
}

//------------------------------------------------------------------------
void CGameRules::RemoveGameRulesListener(SGameRulesListener* pRulesListener)
{
	stl::find_and_erase(m_rulesListeners, pRulesListener);
}

//------------------------------------------------------------------------
i32 CGameRules::RegisterHitType(tukk type, u32k flags)
{
#ifdef _DEBUG
	bool old = DbgSetAssertOnFailureToFindHitType(false);
#endif

	i32 id=GetHitTypeId(type);

#ifdef _DEBUG
	DbgSetAssertOnFailureToFindHitType(old);
#endif

	if (id)
		return id;

	for(i32 i=0;i<EHitType::Unreserved; i++)
	{
		if (stricmp(type, s_reservedHitTypes[i]) == 0)
		{
			id = i;
			break;
		}
	}
	if (id == 0)
	{
		id = m_hitTypeIdGen++;
	}

	m_hitTypes.push_back(HitTypeInfo( type, flags ));

	return id;
}

//------------------------------------------------------------------------
i32 CGameRules::GetHitTypesCount() const
{
	return m_hitTypeIdGen;
}

//------------------------------------------------------------------------
i32 CGameRules::GetHitTypeId( u32k crc ) const
{
	for ( i32 i = 0, size = m_hitTypes.size(); i < size; i++)
	{
		tukk pHitType = m_hitTypes[i].m_name.c_str();
		u32k crcHitType = CCrc32::ComputeLowercase( pHitType );

		if (crcHitType == crc)
			return i;
	}

#ifdef _DEBUG
	DRX_ASSERT_TRACE(!s_dbgAssertOnFailureToFindHitType, ("Unknown CRC \"%d\" is not one of the %d registered hit types! Please register it in Scripts/Entities/Items/HitTypes.xml file or GameRulesMPDamageHandling.cpp", crc, m_hitTypes.size()));
#endif

	return 0;
}

i32 CGameRules::GetHitTypeId(tukk type) const
{
	for ( i32 i = 0, size = m_hitTypes.size(); i < size; i++)
	{
		if (m_hitTypes[i].m_name.compareNoCase(type) == 0)
			return i;
	}

#ifdef _DEBUG
	DRX_ASSERT_TRACE(!s_dbgAssertOnFailureToFindHitType, ("\"%s\" is not one of the %d registered hit types! Please register it in Scripts/Entities/Items/HitTypes.xml file or GameRulesMPDamageHandling.cpp", type, m_hitTypes.size()));
#endif

	return 0;
}


//------------------------------------------------------------------------
tukk CGameRules::GetHitType(i32 id) const
{
	if(id > 0 && id < (i32)m_hitTypes.size())
	{
		return m_hitTypes[id].m_name.c_str();
	}
	else
	{
		return NULL;
	}
}

//------------------------------------------------------------------------
const HitTypeInfo* CGameRules::GetHitTypeInfo(i32 id) const
{
	if(id > 0 && id < (i32)m_hitTypes.size())
	{
		return &m_hitTypes[id];
	}
	else
	{
		return NULL;
	}
}

//------------------------------------------------------------------------
tukk CGameRules::GetHitType(i32 id, tukk defaultValue) const
{
	tukk res = GetHitType( id );
	return res ? res :  defaultValue;
}


// Query if we should feedback for a class of entities if it is _potentially_ hittable.
//
// In:		The entity class (NULL will abort!)
//
// Returns:	True if we should give feedback for potential hits; otherwise false.
//
bool CGameRules::ShouldGiveLocalPlayerHitableFeedbackForEntityClass(const IEntityClass* pEntityClass) const
{
	assert(pEntityClass != NULL);

	return
		(pEntityClass == s_pSmartMineClass) ||
		(pEntityClass == s_pTurretClass) ||
		(pEntityClass == s_pC4Explosive);
}

bool CGameRules::ShouldGiveLocalPlayerHitableFeedbackOnCrosshairHoverForEntityClass(const IEntityClass* pEntityClass) const
{
	assert(pEntityClass != NULL);

	return
		(pEntityClass == s_pSmartMineClass) ||
		(pEntityClass == s_pTurretClass) ||
		(pEntityClass == s_pC4Explosive);
}


// Query if we should give feedback for a hit in the player's HUD and such.
//
// Note: This function is also accessed via Lua scripting and thus cannot
// use the hit info data structure.
//
// In:		The feedback channel type.
// In:		The amount of damage that was inflicted (>= 0.0f).
//
// Returns:	True if we should give feedback for the hit; otherwise false.
//
bool CGameRules::ShouldGiveLocalPlayerHitFeedback(
	const ELocalPlayerHitFeedbackChannel feedbackChannel,
	const float damage) const
{
	switch (feedbackChannel)
	{
	case eLocalPlayerHitFeedbackChannel_Undefined:
	default:
		break;

	case eLocalPlayerHitFeedbackChannel_HUD:
		return (damage > 0.0f);

	case eLocalPlayerHitFeedbackChannel_2DSound:
		return true;
	}

	// We should never get here!
	assert(false);
	return false;
}


//------------------------------------------------------------------------
void CGameRules::SendTextMessage(ETextMessageType type, tukk msg, u32 to, i32 channelId,
																 tukk p0, tukk p1, tukk p2, tukk p3)
{
	if (!gEnv->bServer && (to & (eRMI_ToClientChannel | eRMI_ToOtherClients | eRMI_ToAllClients)))
	{
		GameWarning("CGameRules::SendTextMessage() called on client sending to other clients");
		return;
	}
	GetGameObject()->InvokeRMI(ClTextMessage(), TextMessageParams(type, msg, p0, p1, p2, p3), to, channelId);
}

//------------------------------------------------------------------------
void CGameRules::ChatLog(EChatMessageType type, EntityId sourceId, EntityId targetId, tukk msg)
{
	IEntity* pSource = gEnv->pEntitySystem->GetEntity(sourceId);
	IEntity* pTarget = gEnv->pEntitySystem->GetEntity(targetId);
	tukk  sourceName = pSource? pSource->GetName() : "<unknown>";
	tukk  targetName = pTarget? pTarget->GetName() : "<unknown>";
	i32 teamId = GetTeam(sourceId);

	char tempBuffer[64];

	switch (type)
	{
	case eChatToTeam:
		if (teamId)
		{
			targetName = tempBuffer;
			drx_sprintf(tempBuffer, "Team %s", GetTeamName(teamId));
		}
		else
		{
			targetName = "ALL";
		}
		break;
	case eChatToAll:
		targetName = "ALL";
		break;
	default:
		break;
	}

	DrxLog("CHAT %s to %s:", sourceName, targetName);
	DrxLog("   %s", msg);
}

//------------------------------------------------------------------------
void CGameRules::SendChatMessage(EChatMessageType type, EntityId sourceId, EntityId targetId, tukk msg)
{
}
void CGameRules::ForbiddenAreaWarning(bool active, i32 timer, EntityId targetId)
{
	if (active)
	{
		SHUDEvent event(eHUDEvent_LeavingBattleArea);
		CTimeValue currentTime = gEnv->pTimer->GetFrameStartTime();
		float fCurrentTime = currentTime.GetSeconds();
		float deathTime = fCurrentTime + timer;
		event.AddData(SHUDEventData(deathTime));
		CHUDEventDispatcher::CallEvent(event);
	}
	else
	{
		CHUDEventDispatcher::CallEvent(SHUDEvent(eHUDEvent_ReturningToBattleArea));
	}
}

//------------------------------------------------------------------------
void CGameRules::ResetGameTime()
{
	m_gameStartedTime = m_cachedServerTime;

	if (gEnv->bServer)
	{
		GetGameObject()->InvokeRMI(ClSetGameStartedTime(), SetGameTimeParams(m_gameStartedTime), eRMI_ToRemoteClients);
	}
}

//------------------------------------------------------------------------
void CGameRules::SetRemainingGameTime(float seconds)
{
	if (!g_pGame->IsGameSessionHostMigrating() || !gEnv->bServer)
	{
		// This function should only ever be called as part of the host migration
		// process, when the new server is being created
		GameWarning("CGameRules::SetRemainingGameTime() should only be called by the new server during host migration");
		return;
	}

	float currentTime = m_cachedServerTime.GetSeconds();
	float timeLimit = max(m_timeLimit * 60.0f, currentTime + seconds);

	// Set the start of the game at the appropriate point back in time...
	m_gameStartedTime.SetSeconds(currentTime + seconds - timeLimit);
}

//------------------------------------------------------------------------
float CGameRules::GetRemainingGameTimeNotZeroCapped() const
{
	float timeSinceGameHasStarted = 0.f;

#if USE_PC_PREMATCH
	if (m_prematchState == ePS_Match)
#endif
	{
		if (m_hostMigrationTimeSinceGameStarted.GetValue())
		{
			timeSinceGameHasStarted = m_hostMigrationTimeSinceGameStarted.GetSeconds();
		}
		else
		{
			if (m_gamePausedTime.GetValue() != 0LL) // Game is paused (probably ended), time stops
			{
				timeSinceGameHasStarted = (m_gamePausedTime - m_gameStartedTime).GetSeconds();
			}
			else
			{
				timeSinceGameHasStarted = (m_cachedServerTime - m_gameStartedTime).GetSeconds();
			}
		}
	}

	return (m_timeLimit * 60.f) - timeSinceGameHasStarted;
}

//------------------------------------------------------------------------
float CGameRules::GetRemainingGameTime() const
{
	return max(0.f, GetRemainingGameTimeNotZeroCapped());
}

//------------------------------------------------------------------------
float CGameRules::GetCurrentGameTime() const
{
	if (m_gamePausedTime.GetValue() != 0LL) // Game has ended, time stops
		return max(0.f, (m_gamePausedTime - m_gameStartedTime).GetSeconds());
	else
		return max(0.f, (m_cachedServerTime - m_gameStartedTime).GetSeconds());
}

//------------------------------------------------------------------------
bool CGameRules::IsTimeLimited() const
{
	return m_timeLimit>0.0f;
}

//------------------------------------------------------------------------
void CGameRules::ResetGameStartTimer(float time)
{
	if (!gEnv->bServer)
	{
		GameWarning("CGameRules::ResetGameStartTimer() called on client");
		return;
	}

	CCCPOINT_IF(time < 0.f, GameRules_SvGameStartCountdownAbort);
	CCCPOINT_IF(time >= 0.f, GameRules_SvGameStartCountdownBegin);

	m_gameStartTime = m_cachedServerTime + time;

	GetGameObject()->InvokeRMI(ClSetGameStartTimer(), SetGameTimeParams(m_gameStartTime), eRMI_ToRemoteClients);
}

//------------------------------------------------------------------------
float CGameRules::GetRemainingStartTimer() const
{
	return (m_gameStartTime - m_cachedServerTime).GetSeconds();
}

//------------------------------------------------------------------------
float CGameRules::GetServerTime() const
{
	return m_cachedServerTime.GetMilliSeconds();
}

//------------------------------------------------------------------------
bool CGameRules::OnCollision(const SGameCollision& event)
{
	FUNCTION_PROFILER(GetISystem(), PROFILE_GAME);

	CWeaponSystem* pWeaponSystem = g_pGame->GetWeaponSystem();
	CProjectile* pProjectileSrc = pWeaponSystem && event.pSrcEntity ? pWeaponSystem->GetProjectile(event.pSrcEntity->GetId()) : 0;
	bool bInvalid = (pProjectileSrc != NULL && !pProjectileSrc->IsAlive());

	if (gEnv->bMultiplayer)
	{
		if(!bInvalid)
		{
			// Check if the source entity is a projectile
			if (event.pSrcEntity && event.pTrgEntity && pProjectileSrc)
			{
				EntityId targetId = event.pTrgEntity->GetId();
				// Prevent friendly fire blood effects
				if (GetFriendlyFireRatio() <= 0.f && GetTeamCount() > 1)
				{
					EntityId shooterId = pProjectileSrc->GetOwnerId();
					i32 shooterTeamId = GetTeam(shooterId);
					i32 targetTeamId = GetTeam(targetId);

					if (shooterTeamId && (shooterTeamId == targetTeamId))
					{
						// If the players have a team and their team is the same then this
						// is friendly fire and we don't want the material effects (i.e. blood)
						// to be applied. So return false.
						return false;
					}
				}
				if (targetId == m_pGameFramework->GetClientActorId())
				{
					// Record blood effects on self for killcam replay purposes. These blood effects
					// are suppressed in DrxAction so that they can be replaced by post-processor effects.
					CRecordingSystem *pRecordingSystem = g_pGame->GetRecordingSystem();
					if (pRecordingSystem)
					{
						char effectName[] = "bullet.hit_flesh.a";
						// Randomly change the a into either a, b or c to get a different blood effect each time
						effectName[sizeof(effectName)-2] = drx_random('a', 'c');
						IParticleEffect *pParticle = gEnv->pParticleUpr->FindEffect(effectName);
						if (pParticle)
						{
							SRecording_SpawnCustomParticle customParticle;
							customParticle.location = Matrix34(IParticleEffect::ParticleLoc(event.pCollision->pt, event.pCollision->n));
							customParticle.pParticleEffect = pParticle;
							pRecordingSystem->AddPacket(customParticle);
						}
					}
				}
			}
		}

		// Check if this is an environmental weapon collision
		CEnvironmentalWeapon *pEnvWeapon = NULL;
		if(event.pSrcEntity)
		{
			pEnvWeapon = static_cast<CEnvironmentalWeapon*>(g_pGame->GetIGameFramework()->QueryGameObjectExtension(event.pSrcEntity->GetId(), "EnvironmentalWeapon"));
		}

		if(!pEnvWeapon && event.pTrgEntity)
		{
			pEnvWeapon = static_cast<CEnvironmentalWeapon*>(g_pGame->GetIGameFramework()->QueryGameObjectExtension(event.pTrgEntity->GetId(), "EnvironmentalWeapon"));
		}

		if(pEnvWeapon)
		{
			if(event.pCollision)
			{
				pEnvWeapon->OnCollision(*(event.pCollision));
			}
		}
	}

	// currently this function only calls server functions
	// prevent unnecessary script callbacks on the client
	if (!gEnv->bServer || IsDemoPlayback())
		return true;

	if (!m_damageHandlingModule)
		return true;

	if(!event.pCollision)
		return true;

	// filter out self-collisions
	if (event.pSrcEntity == event.pTrgEntity)
		return true;

	// collisions involving partId<-1 are to be ignored by game's damage calculations
	// usually created articially to make stuff break. See CMelee::Impulse
	if (event.pCollision->partid[0]<-1||event.pCollision->partid[1]<-1)
		return true;

	if(!gEnv->bMultiplayer)
	{
		IEntity *pTarget = event.pCollision->iForeignData[1]==PHYS_FOREIGN_ID_ENTITY ? (IEntity*)event.pCollision->pForeignData[1]:0;
		if (pTarget)
		{
			// Check for ignore
			if (pTarget->GetId() == m_ignoreEntityNextCollision)
			{
				m_ignoreEntityNextCollision = 0;
				return false;
			}

			// Check if source and target are AI and are friendly
			if (pProjectileSrc != NULL && !pProjectileSrc->ProcessCollisionEvent(pTarget))
			{
				return false;
			}
		}
	}

	// collisions with very low resulting impulse are ignored
	if (event.pCollision->normImpulse<=0.001f)
		return true;

	static IEntityClass* s_pBasicEntityClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("BasicEntity");
	static IEntityClass* s_pDefaultClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Default");
	bool srcClassFilter = false;
	bool trgClassFilter = false;

	if (event.pSrcEntity)
	{
		IEntityClass* pSrcClass = event.pSrcEntity->GetClass();
		// filter out any projectile collisions
		if (pProjectileSrc)
			return true;
		srcClassFilter = (pSrcClass == s_pBasicEntityClass || pSrcClass == s_pDefaultClass);
		if (srcClassFilter && !event.pTrgEntity)
			return true;
	}

	if (event.pTrgEntity)
	{
		// filter out any projectile collisions
		if (g_pGame->GetWeaponSystem()->GetProjectile(event.pTrgEntity->GetId()))
			return true;
		IEntityClass* pTrgClass = event.pTrgEntity->GetClass();
		trgClassFilter = (pTrgClass == s_pBasicEntityClass || pTrgClass == s_pDefaultClass);
		if (trgClassFilter && !event.pSrcEntity)
			return true;
	}

	if (srcClassFilter && trgClassFilter)
		return true;

	if (event.pSrcEntity && event.pSrcEntity->GetScriptTable())
	{
		SCollisionHitInfo colHitInfo;
		PrepCollision(0, 1, event, event.pTrgEntity, colHitInfo);

		m_damageHandlingModule->SvOnCollision(event.pSrcEntity, colHitInfo);
	}
	if (event.pTrgEntity && event.pTrgEntity->GetScriptTable())
	{
		SCollisionHitInfo colHitInfo;
		PrepCollision(1, 0, event, event.pSrcEntity, colHitInfo);

		m_damageHandlingModule->SvOnCollision(event.pTrgEntity, colHitInfo);
	}

	return true;
}

void CGameRules::OnCollision_NotifyAI( const EventPhys * pEvent )
{
	FUNCTION_PROFILER(GetISystem(), PROFILE_GAME);
	// Skip the collision handling if there is no AI system or when in multi-player.
	if (!gEnv->pAISystem || (gEnv->bMultiplayer && !gEnv->bServer)) // M�rcio: Enabling AI in Multiplayer!
		return;

	IActorSystem* pActorSystem = g_pGame->GetIGameFramework()->GetIActorSystem();
	IF_UNLIKELY (!pActorSystem)
		return;

	const EventPhysCollision* pCEvent = (const EventPhysCollision *) pEvent;
	IEntity* pColliderEntity = pCEvent->iForeignData[0] == PHYS_FOREIGN_ID_ENTITY ? (IEntity*) pCEvent->pForeignData[0] : NULL;
	IEntity* pTargetEntity = pCEvent->iForeignData[1] == PHYS_FOREIGN_ID_ENTITY ? (IEntity*) pCEvent->pForeignData[1] : NULL;

	if (pTargetEntity || pColliderEntity)
	{
		const EntityId colliderId = pColliderEntity ? pColliderEntity->GetId() : 0;
		const EntityId targetId = pTargetEntity ? pTargetEntity->GetId() : 0;

		if (colliderId)
		{
			const Vec3 impactVelocity = pCEvent->vloc[0] - pCEvent->vloc[1];
			const float impactSpeedSq = impactVelocity.GetLengthSquared();

			float minSpeedScale = 0.0f;
			float minVelocity = 3.0f;

			if (pColliderEntity && pColliderEntity->HasAI())
			{
				if (IActor* pColliderActor = pActorSystem->GetActor(colliderId))
				{
					bool colliderIsPlayer = false;
					bool noiseSuppressorIsActive = false;
					bool cloakIsActive = false;

					if (pColliderActor->IsClient())
					{
						colliderIsPlayer = true;
					}

					const bool targetIsControlledByAI = targetId && pTargetEntity->HasAI();

					if (targetIsControlledByAI)
					{
						IActor* pTargetActor = pActorSystem->GetActor(targetId);

						if (pTargetActor && (pColliderActor->IsClient() || pTargetActor->IsClient()))
						{
							if (colliderIsPlayer && (noiseSuppressorIsActive || cloakIsActive))
							{
								const float slightlyMoreThanMaximumPlayerWalkSpeed = 5.0f;
								minVelocity = slightlyMoreThanMaximumPlayerWalkSpeed;
							}
							else
							{
								// Make sure this collision gets handled
								minVelocity = 0.0f;
								minSpeedScale = 0.15f;
							}
						}
					}
					else if (colliderIsPlayer)
					{
						// The collider is the player and we're colliding with an entity
						// that is not an AI-controlled.
						if (noiseSuppressorIsActive)
						{
							return;
						}
					}
				}
			}

			if ((impactSpeedSq > sqr(minVelocity) && pCEvent->mass[0] > 0.3f && pCEvent->normImpulse > 0.01f))
			{
				float approxRadius = 0.2f;
				if (pColliderEntity)
				{
					AABB bounds;
					pColliderEntity->GetWorldBounds(bounds);
					approxRadius = bounds.GetRadius();
				}

				// Check that the object is big enough to be concerned with.
				if (pCEvent->mass[0] >= 0.3f && pCEvent->normImpulse >= 0.01f)
				{
					// Classify the object type based on mass and size.
					SAICollisionObjClassification type = AICOL_LARGE;
					if (approxRadius < 0.3f && pCEvent->mass[0] < 5.0f)
						type = AICOL_SMALL;
					else if (approxRadius < 2.0f && pCEvent->mass[0] < 100.0f)
						type = AICOL_MEDIUM;

					// Magic formula to calculate the reaction radius from the impact params.
					const float impactSpeed = impactVelocity.GetLength();
					const float reactionRadSize = clamp_tpl(pCEvent->mass[0] * (1.0f / 250.0f), 0.0f, 1.0f);
					const float speedScale = clamp_tpl(impactSpeed * (1.0f / 25.0f), minSpeedScale, 1.0f);

					const float reactionRadius = (5.0f + reactionRadSize * 30.0f) * speedScale;
					const float soundRadius = (10.0f + sqrtf(reactionRadSize) * 90.0f) * speedScale;

					assert(colliderId != 0);

					SAIStimulus stim(AISTIM_COLLISION, type, colliderId, targetId, pCEvent->pt, ZERO, reactionRadius);
					gEnv->pAISystem->RegisterStimulus(stim);

					SAIStimulus stimSound(AISTIM_SOUND, type == AICOL_SMALL ? AISOUND_COLLISION : AISOUND_COLLISION_LOUD, colliderId, 0,
						pCEvent->pt, ZERO, soundRadius, AISTIMPROC_FILTER_LINK_WITH_PREVIOUS);
					gEnv->pAISystem->RegisterStimulus(stimSound);
				}
			}
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::ClearAllMigratingPlayers(void)
{
	for (u32 index = 0; index < m_migratingPlayerMaxCount; ++index)
	{
		m_pMigratingPlayerInfo[index].Reset();
	}
}

//------------------------------------------------------------------------
EntityId CGameRules::SetChannelForMigratingPlayer(tukk name, u16 channelID)
{
	DrxLog("CGameRules::SetChannelForMigratingPlayer, channel=%i, name=%s", channelID, name);
	for (u32 index = 0; index < m_migratingPlayerMaxCount; ++index)
	{
		if (m_pMigratingPlayerInfo[index].InUse() && stricmp(m_pMigratingPlayerInfo[index].m_originalName, name) == 0)
		{
			EntityId playerId = m_pMigratingPlayerInfo[index].m_originalEntityId;
			FinishMigrationForPlayer(index);
			return playerId;
		}
	}
	return 0;
}

//------------------------------------------------------------------------
void CGameRules::StoreMigratingPlayer(IActor* pActor)
{
	if (pActor == NULL)
	{
		GameWarning("Invalid data for migrating player");
		return;
	}

	IEntity* pEntity = pActor->GetEntity();
	EntityId id = pEntity->GetId();
	bool registered = false;

	u16 channelId = pActor->GetChannelId();
	DRX_ASSERT(channelId);

	bool bShouldAdd = true;

	CGameLobby *pGameLobby = g_pGame->GetGameLobby();
	DRX_ASSERT(pGameLobby);
	if (pGameLobby)
	{
		SDrxMatchMakingConnectionUID conId = pGameLobby->GetConnectionUIDFromChannelID((i32) channelId);
		if (pGameLobby->GetSessionNames().Find(conId) == SSessionNames::k_unableToFind)
		{
			DrxLog("CGameRules::StoreMigratingPlayer() player %s (channelId=%u) has already left the game, not storing", pEntity->GetName(), channelId);
			bShouldAdd = false;
		}
	}

	if (bShouldAdd && (!m_hostMigrationCachedEntities.empty()))
	{
		if (!stl::find(m_hostMigrationCachedEntities, pActor->GetEntityId()))
		{
			bShouldAdd = false;
		}
	}

	if (bShouldAdd)
	{
		for (u32 index = 0; index < m_migratingPlayerMaxCount; ++index)
		{
			if (!m_pMigratingPlayerInfo[index].InUse())
			{
				m_pMigratingPlayerInfo[index].SetData(pEntity->GetName(), id, GetTeam(id), pEntity->GetWorldPos(), pEntity->GetWorldAngles(), pActor->GetHealth());
				m_pMigratingPlayerInfo[index].SetChannelID(channelId);
				registered = true;
				break;
			}
		}
	}

	if (!registered && bShouldAdd)
	{
		GameWarning("Too many migrating players!");
	}
}

//------------------------------------------------------------------------

i32 CGameRules::GetMigratingPlayerIndex(TNetChannelID channelID)
{
	i32 migratingPlayerIndex = -1;

	for (u32 index = 0; index < m_migratingPlayerMaxCount; ++index)
	{
		if (m_pMigratingPlayerInfo[index].InUse() && m_pMigratingPlayerInfo[index].m_channelID == channelID)
		{
			migratingPlayerIndex = index;
			break;
		}
	}

	return migratingPlayerIndex;
}

//------------------------------------------------------------------------
void CGameRules::RegisterConsoleCommands(IConsole *pConsole)
{
	// todo: move to power struggle implementation when there is one
	//REGISTER_COMMAND("buy",			"if (g_gameRules and g_gameRules.Buy) then g_gameRules:Buy(%1); end",VF_NULL,"");

	REGISTER_COMMAND("resetloadout", "if (g_gameRules and g_gameRules.ResetLoadout) then g_gameRules:ResetLoadout(); end",VF_INVISIBLE,"");
	REGISTER_COMMAND("additemtoloadout", "if (g_gameRules and g_gameRules.AddItemToLoadout) then g_gameRules:AddItemToLoadout(%1); end",VF_INVISIBLE,"");
	REGISTER_COMMAND("addammotoloadout", "if (g_gameRules and g_gameRules.AddAmmoToLoadout) then g_gameRules:AddAmmoToLoadout(%%); end",VF_INVISIBLE,"");

	REGISTER_COMMAND("g_debug_teams", CmdDebugTeams,VF_NULL,"");

	REGISTER_COMMAND("g_giveScore", CmdGiveScore, VF_NULL, "");
}

//------------------------------------------------------------------------
void CGameRules::UnregisterConsoleCommands(IConsole *pConsole)
{
	//pConsole->RemoveCommand("buy");

	pConsole->RemoveCommand("resetloadout");
	pConsole->RemoveCommand("additemtoloadout");
	pConsole->RemoveCommand("addammotoloadout");

	pConsole->RemoveCommand("g_debug_spawns");
	pConsole->RemoveCommand("g_debug_teams");
	pConsole->RemoveCommand("g_debug_objectives");

	pConsole->RemoveCommand("g_giveScore");
}

//------------------------------------------------------------------------
void CGameRules::RegisterConsoleVars(IConsole *pConsole)
{

}

//------------------------------------------------------------------------
void CGameRules::UnregisterConsoleVars(IConsole *pConsole)
{

}

//------------------------------------------------------------------------
void CGameRules::CmdDebugTeams(IConsoleCmdArgs *pArgs)
{
	CGameRules *pGameRules=g_pGame->GetGameRules();
	if (!pGameRules->m_entityteams.empty())
	{
		DrxLogAlways("// Teams //");
		for (TTeamIdMap::const_iterator tit=pGameRules->m_teams.begin(); tit!=pGameRules->m_teams.end(); ++tit)
		{
			DrxLogAlways("Team: %s  (id: %d)", tit->first.c_str(), tit->second);
			for (TEntityTeamIdMap::const_iterator eit=pGameRules->m_entityteams.begin(); eit!=pGameRules->m_entityteams.end(); ++eit)
			{
				if (eit->second==tit->second)
				{
					IEntity *pEntity=gEnv->pEntitySystem->GetEntity(eit->first);
					DrxLogAlways("    -> Entity: %s  class: %s  (eid: %d %08x)", pEntity?pEntity->GetName():"<null>", pEntity?pEntity->GetClass()->GetName():"<null>", eit->first, eit->first);
				}
			}
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::CmdGiveScore(IConsoleCmdArgs *pArgs)
{
	if (!gEnv->bServer)
	{
		DrxLog ("Server only command, sorry!");
	}
	else if (pArgs->GetArgCount() == 4)
	{
		tukk  findWithName = pArgs->GetArg(1);
		bool getLocalPlayer = (findWithName[0] == '.' && findWithName[1] == '\0');
		IEntity * theEntity = getLocalPlayer ? gEnv->pEntitySystem->GetEntity(g_pGame->GetIGameFramework()->GetClientActorId()) : gEnv->pEntitySystem->FindEntityByName(findWithName);

		if (theEntity)
		{
			i32k points = atoi(pArgs->GetArg(2));
			DRX_ASSERT_MESSAGE( points < SGameRulesScoreInfo::SCORE_MAX && points > SGameRulesScoreInfo::SCORE_MIN, string().Format("Adding score for player which is out of net-serialize bounds (%d is not within [%d .. %d])", points, SGameRulesScoreInfo::SCORE_MIN, SGameRulesScoreInfo::SCORE_MAX) );
			SGameRulesScoreInfo si( (EGameRulesScoreType)atoi(pArgs->GetArg(3)), static_cast<TGameRulesScoreInt>(points) );
			g_pGame->GetGameRules()->IncreasePoints(theEntity->GetId(), si);
		}
		else
		{
			DrxLogAlways ("Found no entity by the name of '%s'", findWithName);
		}
	}
	else
	{
		DrxLogAlways ("Command syntax: %s <entityName> <score> <score-type 0..9>", pArgs->GetArg(0));
	}
}

//------------------------------------------------------------------------
#if !defined(_RELEASE)

#define MESSAGE_FORMAT_STRING  "Damage being done by '%s' to '%s' (with weapon '%s' of class %s, hit type %d '%s') dir={%.2f %.2f %.2f} %s [%s]"
#define MESSAGE_PARAMETERS     GetEntityName(shooter), GetEntityName(target), GetEntityName(weapon), weaponEntity ? weaponEntity->GetClass()->GetName() : "N/A", hitType, GetHitType(hitType), dir.x, dir.y, dir.z
#define DO_WARNING(a)          DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, MESSAGE_FORMAT_STRING, MESSAGE_PARAMETERS, a, funcName)
#define DO_ASSERT(a)           DRX_ASSERT_TRACE(false, (MESSAGE_FORMAT_STRING, MESSAGE_PARAMETERS, a, funcName))

void CGameRules::SanityCheckHitData(const Vec3 & dir, EntityId shooter, EntityId target, EntityId weapon, u16 hitType, tukk  funcName)
{
	if (dir.x == 0.f && dir.y == 1.f && dir.z == 0.f && !g_pGameCVars->g_suppressHitSanityCheckWarnings)
	{
		IEntity * weaponEntity = gEnv->pEntitySystem->GetEntity(weapon);
		DO_WARNING("appears to have uninitialized direction vector!");
	}

	if (hitType == 0)
	{
		IEntity * weaponEntity = gEnv->pEntitySystem->GetEntity(weapon);
		DO_WARNING("has invalid hit type ID");
		DO_ASSERT("has invalid hit type ID");
	}
}

#undef MESSAGE_FORMAT_STRING
#undef MESSAGE_PARAMETERS
#undef DO_WARNING
#undef DO_ASSERT

#endif

//------------------------------------------------------------------------
void CGameRules::CreateScriptHitInfo(SmartScriptTable &scriptHitInfo, const HitInfo &hitInfo) const
{
	CScriptSetGetChain hit(scriptHitInfo);
	{
		hit.SetValue("normal", hitInfo.normal);
		hit.SetValue("pos", hitInfo.pos);
		hit.SetValue("dir", hitInfo.dir);
		hit.SetValue("partId", hitInfo.partId);
		hit.SetValue("backface", hitInfo.normal.Dot(hitInfo.dir)>=0.0f);

		hit.SetValue("targetId", ScriptHandle(hitInfo.targetId));
		hit.SetValue("shooterId", ScriptHandle(hitInfo.shooterId));
		hit.SetValue("weaponId", ScriptHandle(hitInfo.weaponId));
		hit.SetValue("projectileId", ScriptHandle(hitInfo.projectileId));

		IEntity *pTarget=m_pEntitySystem->GetEntity(hitInfo.targetId);
		IEntity *pShooter=m_pEntitySystem->GetEntity(hitInfo.shooterId);
		IEntity *pWeapon=m_pEntitySystem->GetEntity(hitInfo.weaponId);

		hit.SetValue("target", pTarget?pTarget->GetScriptTable():(IScriptTable *)0);
		hit.SetValue("shooter", pShooter?pShooter->GetScriptTable():(IScriptTable *)0);
		hit.SetValue("weapon", pWeapon?pWeapon->GetScriptTable():(IScriptTable *)0);

		hit.SetValue("materialId", hitInfo.material);

		ISurfaceType *pSurfaceType = gEnv->p3DEngine->GetMaterialUpr()->GetSurfaceType(hitInfo.material);
		if (pSurfaceType)
		{
			hit.SetValue("material", pSurfaceType->GetName());
			hit.SetValue("material_type", pSurfaceType->GetType());
		}
		else
		{
			hit.SetToNull("material");
			hit.SetToNull("material_type");
		}

		hit.SetValue("damage", hitInfo.damage);
		hit.SetValue("radius", hitInfo.radius);
		hit.SetValue("knocksDownLeg", hitInfo.knocksDownLeg);

		hit.SetValue("typeId", hitInfo.type);

		const HitTypeInfo *pHitInfo = GetHitTypeInfo(hitInfo.type);
    hit.SetValue("type", (pHitInfo != NULL) ? pHitInfo->m_name.c_str() : "");

		CActor* pTargetActor = (pTarget != NULL) ? static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor( hitInfo.targetId )) : NULL;
		if(pTargetActor != NULL)
		{
			i32k headShotType = pTargetActor->IsHelmetShot(hitInfo) ? eHeadShotType_Helmet : pTargetActor->IsHeadShot(hitInfo);
			hit.SetValue("headShotType", headShotType );
		}
		else
		{
			hit.SetValue("headShotType", eHeadShotType_None );
		}

		i32k meleeHit =  (pHitInfo != NULL) && (pHitInfo->m_flags & EHitTypeFlag::IsMeleeAttack);
		hit.SetValue("meleeHit", meleeHit);

		hit.SetValue("projectileClassId", hitInfo.projectileClassId);
		hit.SetValue("weaponClassId", hitInfo.weaponClassId);

		char projectileClassName[256];
		g_pGame->GetIGameFramework()->GetNetworkSafeClassName(projectileClassName, sizeof(projectileClassName), hitInfo.projectileClassId);
		hit.SetValue("projectileClass", projectileClassName);

		if (hitInfo.explosion)
			hit.SetValue("explosion", true);
		else
			hit.SetToNull("explosion");
	}
}

//------------------------------------------------------------------------
void CGameRules::CreateHitInfoFromScript(const SmartScriptTable &scriptHitInfo, HitInfo &hitInfo)
{
	DRX_ASSERT(scriptHitInfo.GetPtr());

	CScriptSetGetChain hit(scriptHitInfo);
	{
		hit.GetValue("normal", hitInfo.normal);
		hit.GetValue("pos", hitInfo.pos);
		hit.GetValue("dir", hitInfo.dir);
		hit.GetValue("partId", hitInfo.partId);

		ScriptHandle entId;

		hit.GetValue("targetId", entId);
		hitInfo.targetId = static_cast<EntityId>(entId.n);

		hit.GetValue("shooterId", entId);
		hitInfo.shooterId = static_cast<EntityId>(entId.n);

		hit.GetValue("weaponId", entId);
		hitInfo.weaponId = static_cast<EntityId>(entId.n);

		hit.GetValue("projectileId", entId);
		hitInfo.projectileId = static_cast<EntityId>(entId.n);

		u32 uProjectileClassId = 0;
		hit.GetValue("projectileClassId", uProjectileClassId);
		hitInfo.projectileClassId = uProjectileClassId;

		u32 uWeaponClassId = 0;
		hit.GetValue("weaponClassId", uWeaponClassId);
		hitInfo.weaponClassId = uWeaponClassId;

		hit.GetValue("materialId", hitInfo.material);

		hit.GetValue("damage", hitInfo.damage);
		hit.GetValue("radius", hitInfo.radius);

		hit.GetValue("typeId", hitInfo.type);
		hit.GetValue("remote", hitInfo.remote);
		hit.GetValue("bulletType", hitInfo.bulletType);
		hit.GetValue("explosion", hitInfo.explosion);
	}
}

//------------------------------------------------------------------------

void CGameRules::ShowScores(bool show)
{
	CallScript(m_script, "ShowScores", show);
}

//------------------------------------------------------------------------
void CGameRules::UpdateAffectedEntitiesSet(TExplosionAffectedEntities &affectedEnts, const pe_explosion &explosion)
{
	for (i32 i = 0; i < explosion.nAffectedEnts; ++i)
	{
		if (IEntity *pEntity = gEnv->pEntitySystem->GetEntityFromPhysics(explosion.pAffectedEnts[i]))
		{
			if (pEntity->IsHidden())
				continue;

			if (IScriptTable *pEntityTable = pEntity->GetScriptTable())
			{
				if (IPhysicalEntity* pPhys1 = pEntity->GetPhysics())
				{
					float affected = gEnv->pPhysicalWorld->IsAffectedByExplosion(pPhys1);
					if(ICharacterInstance* pChar = pEntity->GetCharacter(0))
					{
						if(ISkeletonPose* pSkel = pChar->GetISkeletonPose())
						{
							if(IPhysicalEntity* pPhys2 = pSkel->GetCharacterPhysics())
							{
								affected = max(affected, gEnv->pPhysicalWorld->IsAffectedByExplosion(pPhys2));
							}
						}
					}
					AddOrUpdateAffectedEntity(affectedEnts, pEntity, affected);
				}
			}
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::AddOrUpdateAffectedEntity(TExplosionAffectedEntities &affectedEnts, IEntity* pEntity, float affected)
{
	TExplosionAffectedEntities::iterator it=affectedEnts.find(pEntity);
	if (it!=affectedEnts.end())
	{
		if (it->second<affected)
			it->second=affected;
	}
	else
		affectedEnts.insert(TExplosionAffectedEntities::value_type(pEntity, affected));
}


//------------------------------------------------------------------------
void CGameRules::RemoveFriendlyAffectedEntities(const ExplosionInfo &explosionInfo, TExplosionAffectedEntities &affectedEntities)
{
	if(explosionInfo.friendlyfire == eFriendyFireSelf)
	{
		IEntity *pEntity=m_pEntitySystem->GetEntity(explosionInfo.shooterId);
		if(pEntity)
		{
			TExplosionAffectedEntities::iterator iter = affectedEntities.find(pEntity);
			if(iter != affectedEntities.end())
			{
				affectedEntities.erase(iter);
			}
		}
	}
	else if(explosionInfo.friendlyfire == eFriendyFireTeam)
	{
		CActor* shooterActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(explosionInfo.shooterId));
		if(shooterActor)
		{
			// MP: Friendly Team check for any shooter
			// SP: Friendly check only for client generated explosions & those will damage himself
			const bool isMultiplayer = gEnv->bMultiplayer;
			const bool shooterIsClient = shooterActor->IsClient();
			if (isMultiplayer || shooterIsClient)
			{
				TExplosionAffectedEntities::iterator iter = affectedEntities.begin();
				TExplosionAffectedEntities::iterator end = affectedEntities.end();
				for (; iter != end;)
				{
					const EntityId targetId = iter->first->GetId();
					const bool isClientStamp = (targetId == explosionInfo.shooterId) && (explosionInfo.type == CGameRules::EHitType::Stamp);
					i32k isFriendly = isMultiplayer ? shooterActor->IsFriendlyEntity(targetId, false) :
															(isClientStamp || ((targetId != explosionInfo.shooterId) && shooterActor->IsFriendlyEntity(targetId, false)));
					if(isFriendly)
					{
						TExplosionAffectedEntities::iterator nextIter = iter;
						++nextIter;
						affectedEntities.erase(iter);
						iter = nextIter;
					}
					else
					{
						++iter;
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::PrepCollision(i32 src, i32 trg, const SGameCollision& event, IEntity* pTarget, SCollisionHitInfo &result)
{
	const EventPhysCollision* pCollision = event.pCollision;
	result.pos = pCollision->pt;
	result.normal = pCollision->n;
	result.dir = Vec3(0.f, 0.f, 0.f);
	if (pCollision->vloc[src].GetLengthSquared() > 1e-6f)
	{
		result.dir = pCollision->vloc[src].GetNormalized();
	}
	result.velocity = pCollision->vloc[src];

	pe_status_living sl;
	if (pCollision->pEntity[src]->GetStatus(&sl) && sl.bSquashed)
	{
		result.target_velocity = pCollision->n*(200.0f*(1-src*2));
		result.target_mass = pCollision->mass[trg]>0 ? pCollision->mass[trg] : 10000.0f;
	}
	else
	{
		result.target_velocity = pCollision->vloc[trg];
		result.target_mass = pCollision->mass[trg];
	}

	result.backface = (pCollision->n.Dot(result.dir) >= 0);

	if (pTarget)
	{
		result.targetId = pTarget->GetId();

		if (pTarget->GetPhysics())
		{
			result.target_type = pTarget->GetPhysics()->GetType();
		}
	}

	result.materialId = pCollision->idmat[src];
	result.partId = pCollision->partid[src];
	result.mass = pCollision->mass[src];
}

//------------------------------------------------------------------------
void CGameRules::Restart()
{
	CCCPOINT(GameRules_Restart);
	CGameMechanismUpr::GetInstance()->Inform(kGMEvent_GameRulesRestart);

#if defined(USE_PERFHUD)
	CDebugAllowFileAccess afa;
	IDrxPerfHUD* pPerfHud = gEnv->pSystem->GetPerfHUD();
	if (pPerfHud)
	{
		static i32 roundCount = 0;
		string filename;
		filename.Format("PerfHudStatsRound%d.xml", ++roundCount);
		pPerfHud->SaveStats(filename);
		pPerfHud->ResetWidgets();
	}
#endif

	DrxWatch3DReset();

	g_pGame->GetPlayerVisTable()->Reset();

	if (gEnv->bServer)
	{
		if (m_stateModule)
		{
			m_stateModule->OnGameRestart();
		}

		CallScript(m_script, "RestartGame", true);
		ResetEntities();	// used to be done by lua Restart handler, but having it here means we don't need a Restart handler in lua (eg when using game state module)
							// lua RestartGame() now only needs to restart its internal state
	}
}

//------------------------------------------------------------------------
void CGameRules::NextLevel()
{
  if (!gEnv->bServer)
    return;

	ILevelRotation *pLevelRotation=g_pGame->GetPlaylistUpr()->GetLevelRotation();
	if (!pLevelRotation->GetLength())
		Restart();
	else
		pLevelRotation->ChangeLevel();
}

//------------------------------------------------------------------------
void CGameRules::ResetEntities()
{
	g_pGame->GetWeaponSystem()->GetTracerUpr().Reset();

	ResetQueuedExplosionsAndHits();

	// remove voice groups too. They'll be recreated when players are put back on their teams after reset.
#ifndef OLD_VOICE_SYSTEM_DEPRECATED
 	TTeamIdVoiceGroupMap::iterator it = m_teamVoiceGroups.begin();
 	TTeamIdVoiceGroupMap::iterator next;
 	for(; it != m_teamVoiceGroups.end(); it=next)
 	{
 		next = it; ++next;

		m_teamVoiceGroups.erase(it);
 	}
#endif

	m_respawns.clear();
	ClearEntityTeams();

	for (TPlayerTeamIdMap::iterator tit=m_playerteams.begin(); tit!=m_playerteams.end(); ++tit)
		tit->second.resize(0);

	g_pGame->GetIGameFramework()->Reset(gEnv->bServer);

	CEquipmentLoadout *pLoadout = g_pGame->GetEquipmentLoadout();
	if (pLoadout)
	{
		pLoadout->OnGameReset();
	}
}

//------------------------------------------------------------------------
void CGameRules::OnEndGame()
{
	bool isMultiplayer=gEnv->bMultiplayer ;

	if (gEnv->bServer)
	{
		// Set time the game ended on server, clients do this separately since they need to adjust
		// the time according to the reason the game ended (i.e. force it to 0 remaining time if the
		// game finished due to the time limit)
		m_gamePausedTime = m_cachedServerTime;
	}

#ifndef OLD_VOICE_SYSTEM_DEPRECATED
	if (isMultiplayer && gEnv->bServer)
		m_teamVoiceGroups.clear();
#endif

	CCCPOINT(GameRules_OnEndGame);

	if(gEnv->IsClient())
	{
		IGameFramework* pGameFramework = gEnv->pGame->GetIGameFramework();

		// Stop force feedback
		IForceFeedbackSystem* pForceFeedbackSystem = pGameFramework->GetIForceFeedbackSystem();
		if(pForceFeedbackSystem)
		{
			pForceFeedbackSystem->StopAllEffects();
		}

		IActionMapUpr *pActionMapMan = pGameFramework->GetIActionMapUpr();
		pActionMapMan->EnableActionMap("multiplayer", !isMultiplayer);
		pActionMapMan->EnableActionMap("singleplayer", isMultiplayer);

		IActionMap *am = NULL;
		if(isMultiplayer)
		{
			am = pActionMapMan->GetActionMap("multiplayer");
		}
		else
		{
			am = pActionMapMan->GetActionMap("singleplayer");
		}
		if(am)
		{
			am->SetActionListener(0);
		}
	}

	float delay = 0.f;
	if (isMultiplayer)
	{
		// Wait X seconds before uploading the statslog, this leaves enough time
		// for clients to send in their bonus xp values to the server
		delay = g_pGameCVars->g_telemetry_mp_upload_delay;
		if (gEnv->bServer && m_statsRecordingModule)
		{
			m_statsRecordingModule->OnGameEnd();
		}
	}
	SaveSessionStatistics(delay);
	CGameLobby *pGameLobby = g_pGame->GetGameLobby();
	if(pGameLobby)
	{
		pGameLobby->UpdatePreviousGameScores();
	}
}

//------------------------------------------------------------------------
void CGameRules::GameOver(EGameOverType localWinner)
{
	const CActor* pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());
	const bool isSpectator = pActor ? pActor->GetSpectatorState() == CActor::eASS_SpectatorMode : false;

	CCCPOINT_IF(localWinner == EGOT_Lose, GameRules_GameOverLose);
	CCCPOINT_IF(localWinner == EGOT_Draw, GameRules_GameOverDraw);
	CCCPOINT_IF(localWinner == EGOT_Win,  GameRules_GameOverWin);

	g_pGame->GetIGameFramework()->GetIGameplayRecorder()->Event(0, GameplayEvent(eGE_GameEnd, 0, 1.0f));
	if(m_rulesListeners.empty() == false)
	{
		TGameRulesListenerVec::iterator iter = m_rulesListeners.begin();
		while (iter != m_rulesListeners.end())
		{
			(*iter)->GameOver(localWinner, isSpectator);
			++iter;
		}
	}

	if(!gEnv->IsDedicated())
	{
		// Save profile before telling the game lobby, otherwise we send the profile as it was at the start of the game
		// rather than the one as it is now
		g_pGame->GetProfileOptions()->SaveProfile();
	}
}

//------------------------------------------------------------------------
void CGameRules::EnteredGame()
{
	if (m_bHasCalledEnteredGame)
	{
		return;
	}
	m_bHasCalledEnteredGame = true;

	if(m_rulesListeners.empty() == false)
	{
		TGameRulesListenerVec::iterator iter = m_rulesListeners.begin();
		while (iter != m_rulesListeners.end())
		{
			(*iter)->EnteredGame();
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::EndGameNear(EntityId id)
{
	if(m_rulesListeners.empty() == false)
	{
		TGameRulesListenerVec::iterator iter = m_rulesListeners.begin();
		while(iter != m_rulesListeners.end())
		{
			(*iter)->EndGameNear(id);
			++iter;
		}
	}
}


//------------------------------------------------------------------------
#if USE_PC_PREMATCH
void CGameRules::OnPrematchEnd_NotifyListeners()
{
	if(m_prematchListenersVec.empty() == false)
	{
		TPrematchListenersVec::iterator iter = m_prematchListenersVec.begin();
		while(iter != m_prematchListenersVec.end())
		{
			(*iter)->OnPrematchEnd();
			++iter;
		}
	}
}
#endif // USE_PC_PREMATCH

//------------------------------------------------------------------------
void CGameRules::ClientEnteredGame_NotifyListeners( EntityId clientId )
{
	if(m_rulesListeners.empty() == false)
	{
		TGameRulesListenerVec::iterator iter = m_rulesListeners.begin();
		while(iter != m_rulesListeners.end())
		{
			(*iter)->ClientEnteredGame( clientId );
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::ClientDisconnect_NotifyListeners( EntityId clientId )
{
	if(m_rulesListeners.empty() == false)
	{
		TGameRulesListenerVec::iterator iter = m_rulesListeners.begin();
		while(iter != m_rulesListeners.end())
		{
			(*iter)->ClientDisconnect( clientId );
			++iter;
		}
	}
}


//------------------------------------------------------------------------
void CGameRules::OnActorDeath_NotifyListeners( CActor* pActor )
{
	if(m_rulesListeners.empty() == false)
	{
		TGameRulesListenerVec::iterator iter = m_rulesListeners.begin();
		while(iter != m_rulesListeners.end())
		{
			(*iter)->OnActorDeath( pActor );
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::SvOnTimeLimitExpired_NotifyListeners()
{
	DRX_ASSERT(gEnv->bServer);
	if(m_rulesListeners.empty() == false)
	{
		TGameRulesListenerVec::iterator  iter = m_rulesListeners.begin();
		while (iter != m_rulesListeners.end())
		{
			(*iter)->SvOnTimeLimitExpired();
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::EntityRevived_NotifyListeners( EntityId entityId )
{
	if(m_revivedListenersVec.empty() == false)
	{
		TRevivedListenersVec::iterator  iter = m_revivedListenersVec.begin();
		while(iter != m_revivedListenersVec.end())
		{
			(*iter)->EntityRevived( entityId );
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::SvSurvivorCountRefresh_NotifyListeners(i32 count, const EntityId survivors[], i32 numKills)
{
	DRX_ASSERT(gEnv->bServer);
	if(m_survivorCountListenersVec.empty() == false)
	{
		TSurvivorCountListenersVec::iterator  iter = m_survivorCountListenersVec.begin();
		while(iter != m_survivorCountListenersVec.end())
		{
			(*iter)->SvSurvivorCountRefresh(count, survivors, numKills);
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::ClPlayerStatsNetSerializeReadDeath_NotifyListeners(const SGameRulesPlayerStat* s, u16 prevDeathsThisRound, u8 prevFlags)
{
	DRX_ASSERT(gEnv->IsClient());
	if (m_playerStatsListenersVec.empty() == false)
	{
		TPlayerStatsListenersVec::iterator  iter = m_playerStatsListenersVec.begin();
		while (iter != m_playerStatsListenersVec.end())
		{
			(*iter)->ClPlayerStatsNetSerializeReadDeath(s, prevDeathsThisRound, prevFlags);
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::OnRoundStart_NotifyListeners()
{
	if (m_roundsListenersVec.empty() == false)
	{
		TRoundsListenersVec::iterator  iter = m_roundsListenersVec.begin();
		while (iter != m_roundsListenersVec.end())
		{
			(*iter)->OnRoundStart();
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::OnRoundEnd_NotifyListeners()
{
	if (m_roundsListenersVec.empty() == false)
	{
		TRoundsListenersVec::iterator  iter = m_roundsListenersVec.begin();
		while (iter != m_roundsListenersVec.end())
		{
			(*iter)->OnRoundEnd();
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::OnRoundAboutToStart_NotifyListeners()
{
	if (m_roundsListenersVec.empty() == false)
	{
		TRoundsListenersVec::iterator  iter = m_roundsListenersVec.begin();
		while (iter != m_roundsListenersVec.end())
		{
			(*iter)->OnRoundAboutToStart();
			++iter;
		}
	}
	m_hasWinningKill = false;
}

//------------------------------------------------------------------------
void CGameRules::OnSuddenDeath_NotifyListeners()
{
	if (m_roundsListenersVec.empty() == false)
	{
		TRoundsListenersVec::iterator  iter = m_roundsListenersVec.begin();
		while (iter != m_roundsListenersVec.end())
		{
			(*iter)->OnSuddenDeath();
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::ClRoundsNetSerializeReadState_NotifyListeners(i32 newState, i32 curState)
{
	DRX_ASSERT(!gEnv->bServer);
	DRX_ASSERT(gEnv->IsClient());
	if (m_roundsListenersVec.empty() == false)
	{
		TRoundsListenersVec::iterator  iter = m_roundsListenersVec.begin();
		while (iter != m_roundsListenersVec.end())
		{
			(*iter)->ClRoundsNetSerializeReadState(newState, curState);
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::ClientScoreEvent(EGameRulesScoreType scoreType, i32 points, EXPReason inReason, i32 currentTeamScore)
{
	if(m_clientScoreListenersVec.empty() == false)
	{
		TClientScoreListenersVec::iterator iter = m_clientScoreListenersVec.begin();
		while (iter != m_clientScoreListenersVec.end())
		{
			(*iter)->ClientScoreEvent(scoreType, points, inReason, currentTeamScore);
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::ActorActionInformOnAction(const ActionId& actionId, i32 activationMode, float value)
{
	if (m_actorActionListenersVec.empty() == false)
	{
		TActorActionListenersVec::iterator  iter = m_actorActionListenersVec.begin();
		while (iter != m_actorActionListenersVec.end())
		{
			(*iter)->OnAction(actionId, activationMode, value);
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::CreateEntityRespawnData(EntityId entityId)
{
	if (m_pGameFramework->IsEditing())
		return;

	IEntity *pEntity=m_pEntitySystem->GetEntity(entityId);
	if (!pEntity)
		return;

	if (!HasEntityRespawnData(entityId))
	{
		SEntityRespawnData respawn;
		respawn.position = pEntity->GetWorldPos();
		respawn.rotation = pEntity->GetWorldRotation();
		respawn.scale = pEntity->GetScale();
		respawn.flags = pEntity->GetFlags();
		respawn.pClass = pEntity->GetClass();
		respawn.m_nameHash = pEntity->GetName();
		respawn.m_currentEntityId = pEntity->GetId();
		respawn.m_bHasRespawned = false;

		IScriptTable *pScriptTable = pEntity->GetScriptTable();

		if (pScriptTable)
			pScriptTable->GetValue("Properties", respawn.properties);

		m_respawndata.push_back(respawn);
	}
}

//------------------------------------------------------------------------
CGameRules::SEntityRespawnData *CGameRules::GetEntityRespawnData(EntityId entityId)
{
	i32k numRespawnData = m_respawndata.size();
	for (i32 i = 0; i < numRespawnData; ++ i)
	{
		SEntityRespawnData *pData = &m_respawndata[i];
		if (pData->m_currentEntityId == entityId)
		{
			return pData;
		}
	}
	return NULL;
}

//------------------------------------------------------------------------
CGameRules::SEntityRespawnData *CGameRules::GetEntityRespawnDataByHashId(DrxHashStringId nameHashId)
{
	i32k numRespawnData = m_respawndata.size();
	for (i32 i = 0; i < numRespawnData; ++ i)
	{
		SEntityRespawnData *pData = &m_respawndata[i];
		if (pData->m_nameHash == nameHashId)
		{
			return pData;
		}
	}
	return NULL;
}

//------------------------------------------------------------------------
bool CGameRules::HasEntityRespawnData(EntityId entityId) const
{
	i32k numRespawnData = m_respawndata.size();
	for (i32 i = 0; i < numRespawnData; ++ i)
	{
		const SEntityRespawnData *pData = &m_respawndata[i];
		if (pData->m_currentEntityId == entityId)
		{
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------
void CGameRules::ScheduleEntityRespawn(EntityId entityId, bool unique, float timer)
{
	if (m_pGameFramework->IsEditing())
		return;

	SEntityRespawn respawn;
	respawn.timer = timer;
	respawn.unique = unique;

	if (g_pGameCVars->g_forceItemRespawnTimer != 0.f)
	{
		respawn.timer = g_pGameCVars->g_forceItemRespawnTimer;
	}

	m_respawns.insert(TEntityRespawnMap::value_type(entityId, respawn));
}

//------------------------------------------------------------------------
void CGameRules::DoEntityRespawn(EntityId id)
{
	SEntityRespawnData *pData = GetEntityRespawnData(id);
	if (pData)
	{
		SEntitySpawnParams params;
		params.pClass=pData->pClass;
		params.qRotation=pData->rotation;
		params.vPosition=pData->position;
		params.vScale=pData->scale;
		params.nFlags=pData->flags | ENTITY_FLAG_NEVER_NETWORK_STATIC;

		string name;
#ifdef _DEBUG
		name = pData->m_nameHash.debugName;
		name.append("_repop");
#else
		name=pData->pClass->GetName();
#endif
		params.sName = name.c_str();

		IEntity *pEntity=m_pEntitySystem->SpawnEntity(params, false);

		if (pEntity)
		{
			pData->m_currentEntityId = pEntity->GetId();
			pData->m_bHasRespawned = true;

			if (pData->properties.GetPtr())
			{
				SmartScriptTable properties;
				IScriptTable *pScriptTable=pEntity->GetScriptTable();
				if (pScriptTable != NULL && pScriptTable->GetValue("Properties", properties))
				{
					if (properties.GetPtr())
					{
						// Clone table doesn't seem to always work with certain entities wheras set does:S
						pScriptTable->SetValue("Properties",pData->properties);
					}
				}
			}

			m_pEntitySystem->InitEntity(pEntity, params);

			SRespawnUpdateParams rmiParams;
			rmiParams.m_respawnEntityId = pData->m_currentEntityId;
			rmiParams.m_respawnHashId = (i32) pData->m_nameHash.id;
			GetGameObject()->InvokeRMIWithDependentObject(ClUpdateRespawnData(), rmiParams, eRMI_ToRemoteClients, rmiParams.m_respawnEntityId);
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::UpdateEntitySchedules(float frameTime)
{
	if (m_pGameFramework->IsEditing())
		return;

	TEntityRespawnMap::iterator next;
	for (TEntityRespawnMap::iterator it=m_respawns.begin(); it!=m_respawns.end(); it=next)
	{
		next=it; ++next;
		EntityId id=it->first;
		SEntityRespawn &respawn=it->second;

		if (respawn.unique)
		{
			IEntity *pEntity=m_pEntitySystem->GetEntity(id);
			if (pEntity)
				continue;
		}

		respawn.timer -= frameTime;
		if (respawn.timer<=0.0f)
		{
			DoEntityRespawn(id);
			m_respawns.erase(it);
		}
	}

	for (TEntityRemovalMap::iterator it=m_removals.begin(); it!=m_removals.end();)
	{
		EntityId id=it->first;
		SEntityRemovalData &removal=it->second;

		IEntity *pEntity=m_pEntitySystem->GetEntity(id);
		if (!pEntity)
		{
			m_removals.erase(it++);
			continue;
		}

		//TODO: Either fix this properly, so it takes proximity of all players into account, or remove it.
		if (removal.visibility)
		{
			AABB aabb;
			pEntity->GetWorldBounds(aabb);

			CCamera &camera=m_pSystem->GetViewCamera();
			if (camera.IsAABBVisible_F(aabb))
			{
				removal.timer=removal.time;
				++it;
				continue;
			}
		}

		removal.timer-=frameTime;
		if (removal.timer<=0.0f)
		{
			DrxLog ("[REMOVALS] Removing %s %s '%s' because %.2f seconds have elapsed since %s", pEntity->IsHidden() ? "hidden" : "visible", pEntity->GetClass()->GetName(), pEntity->GetName(), removal.time, removal.visibility ? "was on-screen" : "request");
			CCCPOINT_IF(removal.visibility,  GameRules_RemoveObjectAfterOffscreenForTime);
			CCCPOINT_IF(!removal.visibility, GameRules_RemoveObjectAfterTime);
			m_pEntitySystem->RemoveEntity(id);
			m_removals.erase(it++);
			continue;
		}

		++it;
	}
}

//------------------------------------------------------------------------
void CGameRules::FlushEntitySchedules()
{
	TEntityRemovalMap::iterator removalIt;
	for (removalIt = m_removals.begin(); removalIt != m_removals.end(); ++ removalIt)
	{
		EntityId id = removalIt->first;

		IEntity *pEntity=m_pEntitySystem->GetEntity(id);
		if (pEntity)
		{
			m_pEntitySystem->RemoveEntity(id);
		}
	}
	m_removals.clear();

	TEntityRespawnMap::iterator respawnIt;
	for (respawnIt = m_respawns.begin(); respawnIt != m_respawns.end(); ++ respawnIt)
	{
		DoEntityRespawn(respawnIt->first);
	}
	m_respawns.clear();
}

//------------------------------------------------------------------------
void CGameRules::FreezeInput(bool freeze)
{
#if !defined(DRX_USE_GCM_HUD)
	if (gEnv->pInput) gEnv->pInput->ClearKeyState();
#endif

	g_pGameActions->FilterFreezeTime()->Enable(freeze);

	if (freeze)
	{
		IVehicleClient *pVehicleClient = g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicleClient();
		if(pVehicleClient)
		{
			pVehicleClient->Reset();
		}

		CPlayer *pPlayer = static_cast<CPlayer *>(gEnv->pGame->GetIGameFramework()->GetClientActor());
		if(pPlayer)
		{
			IPlayerInput* pPlayerInput = pPlayer->GetPlayerInput();
			if(pPlayerInput)
			{
				pPlayerInput->Reset();
			}

			IVehicle *pVehicle = pPlayer->GetLinkedVehicle();
			if(pVehicle)
			{
				IVehicleMovement *pVehicleMovement = pVehicle->GetMovement();
				if(pVehicleMovement)
				{
					pVehicleMovement->ResetInput();
				}
			}
		}
	}
}

//------------------------------------------------------------------------
bool CGameRules::IsProjectile(EntityId id) const
{
	return g_pGame->GetWeaponSystem()->GetProjectile(id)!=0;
}

//------------------------------------------------------------------------
void CGameRules::AbortEntityRespawn(EntityId entityId, bool destroyData)
{
	stl::member_find_and_erase(m_respawns, entityId);

	if (destroyData)
	{
		i32k numRespawnData = m_respawndata.size();
		for (i32 i = 0; i < numRespawnData; ++ i)
		{
			SEntityRespawnData *pData = &m_respawndata[i];
			if (pData->m_currentEntityId == entityId)
			{
				m_respawndata.erase(m_respawndata.begin() + i);
				break;
			}
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::ScheduleEntityRemoval(EntityId entityId, float timer, bool visibility)
{
	if (!gEnv->bServer || gEnv->IsEditor())
		return;

	IEntity *pEntity=m_pEntitySystem->GetEntity(entityId);
	if (!pEntity)
		return;

	SEntityRemovalData removal;
	removal.time = timer;
	removal.timer = timer;
	removal.visibility = visibility && !gEnv->bMultiplayer;

	DrxLog ("[REMOVALS] Scheduling %s %s '%s' for removal %s %.2f seconds", pEntity->IsHidden() ? "hidden" : "visible", pEntity->GetClass()->GetName(), pEntity->GetName(), removal.visibility ? "once off-screen for" : "in", timer);
	CCCPOINT_IF(removal.visibility,  GameRules_SetRemoveObjectAfterOffscreenForTime);
	CCCPOINT_IF(!removal.visibility, GameRules_SetRemoveObjectAfterTime);

	m_removals.insert(TEntityRemovalMap::value_type(entityId, removal));
}

//------------------------------------------------------------------------
void CGameRules::AbortEntityRemoval(EntityId entityId)
{
	stl::member_find_and_erase(m_removals, entityId);
}

void CGameRules::ShowStatus()
{
	float timeRemaining = GetRemainingGameTime();
	i32 mins = (i32)(timeRemaining / 60.0f);
	i32 secs = (i32)(timeRemaining - mins*60);
	DrxLog("time remaining: %d:%02d", mins, secs);
}
#ifndef OLD_VOICE_SYSTEM_DEPRECATED
void CGameRules::ReconfigureVoiceGroups(EntityId id,i32 old_team,i32 new_team)
{
	INetContext *pNetContext = g_pGame->GetIGameFramework()->GetNetContext();
	if(!pNetContext)
		return;

	IVoiceContext *pVoiceContext = pNetContext->GetVoiceContext();
	if(!pVoiceContext)
		return; // voice context is now disabled in single player game. talk to me if there are any problems - Lin

	if(old_team==new_team)
		return;

	TTeamIdVoiceGroupMap::iterator iter=m_teamVoiceGroups.find(old_team);
	if(iter!=m_teamVoiceGroups.end())
	{
		iter->second->RemoveEntity(id);
		//DrxLog("<--Removing entity %d from team %d", id, old_team);
	}
	else
	{
		//DrxLog("<--Failed to remove entity %d from team %d", id, old_team);
	}

	iter=m_teamVoiceGroups.find(new_team);
	if(iter==m_teamVoiceGroups.end())
	{
		IVoiceGroup* pVoiceGroup=pVoiceContext->CreateVoiceGroup();
		iter=m_teamVoiceGroups.insert(std::make_pair(new_team,pVoiceGroup)).first;
	}
	iter->second->AddEntity(id);
	pVoiceContext->InvalidateRoutingTable();
	//DrxLog("-->Adding entity %d to team %d", id, new_team);
}
#endif

void CGameRules::PlayerPosForRespawn(CPlayer* pPlayer, bool save)
{
	static 	Matrix34	respawnPlayerTM(IDENTITY);
	if (save)
	{
		respawnPlayerTM = pPlayer->GetEntity()->GetWorldTM();
	}
	else
	{
		pPlayer->GetEntity()->SetWorldTM(respawnPlayerTM);
	}
}

void CGameRules::GetMemoryUsage(IDrxSizer *s) const
{
	s->Add(*this);
	s->AddContainer(m_channelIds);
	s->AddContainer(m_teams);
	s->AddContainer(m_entityteams);
	s->AddContainer(m_channelteams);
	s->AddContainer(m_playerteams);
	s->AddContainer(m_hitTypes);
	s->AddContainer(m_respawndata);
	s->AddContainer(m_respawns);
	s->AddContainer(m_removals);
	s->AddContainer(m_spawnGroups);
#ifndef OLD_VOICE_SYSTEM_DEPRECATED
	s->AddContainer(m_teamVoiceGroups);
#endif
	s->AddContainer(m_rulesListeners);
}

// sync up the telemetry session name so all clients in the game file their telemetry in the same place
// ITelemetryCollector can be NULL
bool CGameRules::NetSerializeTelemetry( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags )
{
	if (aspect == eEA_GameServerStatic)
	{
		// FIXME probably not the wisest use of our bandwidth, is the server name not stored elsewhere already?
		// FIXME session id changes when sessions are started and ended, we aren't marking this aspect as dirty in those cases. potential fix is to have a listener on the session id changing

		CTelemetryCollector		*tc=static_cast<CTelemetryCollector*>(g_pGame->GetITelemetryCollector());

		if (ser.IsWriting())
		{
			ser.Value("sessionid",tc ? tc->GetSessionId() : "");
		}
		else
		{
			string	session;
			ser.Value("sessionid",session);
			if (tc)
			{
				tc->SetSessionId(session);
				tc->CreateEventStream();
			}
		}
	}

	return true;
}

bool CGameRules::NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags )
{
	if (aspect == GAMERULES_TIME_OF_DAY_DYNAMIC_ASPECT)
	{
		u32 todFlags = 0;
		if (ser.IsReading())
		{
			todFlags |= ITimeOfDay::NETSER_COMPENSATELAG;
			if (!m_timeOfDayInitialized)
			{
				todFlags |= ITimeOfDay::NETSER_FORCESET;
				m_timeOfDayInitialized = true;
			}
		}
		gEnv->p3DEngine->GetTimeOfDay()->NetSerialize( ser, 0.0f, todFlags );
	}
	else
	{
		if (aspect == eEA_GameServerStatic)
		{
			gEnv->p3DEngine->GetTimeOfDay()->NetSerialize( ser, 0.0f, ITimeOfDay::NETSER_STATICPROPS );

#if USE_PC_PREMATCH
			const bool bClientOnly = (gEnv->IsClient() && !gEnv->bServer);
			const bool previousHasStarted = (m_prematchState == ePS_Match);

			EPrematchState prematchState = m_prematchState;
			ser.EnumValue("prematchState", prematchState, ePS_Prematch, ePS_Last);
			ser.Value("numRequiredPlayers", m_numRequiredPlayers, 'ui4');

			ser.Value("timeStartedWaiting", m_timeStartedWaitingForBalancedGame, 'tnet');

			if (bClientOnly)
			{
				ChangePrematchState((EPrematchState)prematchState);

				DrxLog("  m_timeStartedWaitingForBalancedGame=%" PRIi64, m_timeStartedWaitingForBalancedGame.GetValue());
			}

			bool bGameHasActuallyStarted = ((EPrematchState)prematchState == ePS_Match);
			if (bClientOnly && (bGameHasActuallyStarted != previousHasStarted))
			{
				//SHUDEventWrapper::SimpleBannerMessage("@ui_menu_gamelobby_starting_game", SHUDEventWrapper::kMsgAudioNULL);
				SHUDEvent newTeamMessage(eHUDEvent_OnServerMessage);
				newTeamMessage.AddData("@ui_menu_gamelobby_starting_game");
				CHUDEventDispatcher::CallEvent(newTeamMessage);

				CPersistantStats *pStats = CPersistantStats::GetInstance();
				if (pStats)
				{
					pStats->OnGameActuallyStarting();
				}

				m_previousNumRequiredPlayers = -1;

				if (prematchState != ePS_PrematchWaitingForPlayers)
				{
					m_waitingForPlayerMessage1.clear();
					m_waitingForPlayerMessage2.clear();
					SHUDEventWrapper::OnBigWarningMessage(m_waitingForPlayerMessage1.c_str(), m_waitingForPlayerMessage2.c_str());

					g_pGame->GetUI()->ActivateDefaultState();

					CHUDEventDispatcher::CallEvent(eHUDEvent_OnPrematchFinished);
				}
			}

			if(!bGameHasActuallyStarted && !gEnv->IsEditor())
			{
				CParameterGameEffect * pParameterGameEffect = g_pGame->GetParameterGameEffect();
				FX_ASSERT_MESSAGE (pParameterGameEffect, "Pointer to ParameterGameEffect is NULL");
				if (pParameterGameEffect)
				{
					pParameterGameEffect->SetSaturationAmount(-1.f, CParameterGameEffect::eSEU_PreMatch);
				}
			}
#endif
		}
		if (aspect == GAMERULES_TEAMS_SCORE_ASPECT)
		{
			i32 teamId = 0;
			u16 score = 0;
			u16 roundScore = 0;
			u16 scoreRoundStart = 0;
			i32 numTeams = m_teamscores.size();

			ser.Value("numTeams", numTeams, 'team');

			for (TTeamScoresMap::iterator iter=m_teamscores.begin(); iter!=m_teamscores.end(); ++iter)
			{
				if (ser.IsWriting())
				{
					teamId=iter->first;
					score=iter->second.m_teamScore;
					roundScore=iter->second.m_roundTeamScore;
					scoreRoundStart=iter->second.m_teamScoreRoundStart;
				}

				ser.Value("teamId", teamId, 'team');
				ser.Value("score", score, 'u16');
				ser.Value("roundScore", roundScore, 'u16');
				ser.Value("scoreRoundStart", scoreRoundStart, 'u16');

				if (ser.IsReading())
				{
					DRX_TODO(8,2,2010, "Providing the assert below never triggers, look into removing the serialiation of 'teamId' here - if needed it should be able to be got from the 'first' member of the iterator anyways");
					DRX_ASSERT(teamId == iter->first);
					ClientTeamScoreFeedback(teamId, iter->second.m_teamScore, score);
					iter->second.m_teamScore = score;
					iter->second.m_roundTeamScore = roundScore;
					iter->second.m_teamScoreRoundStart = scoreRoundStart;
				}
			}
		}
		if (aspect == GAMERULES_LIMITS_ASPECT)
		{
			float newTimeLimit = m_timeLimit;
			i32 newScoreLimit = m_scoreLimit;
			i32 newRoundLimit = m_roundLimit;
			bool votingEnabled = m_votingEnabled;
			i32	votingCooldown = m_votingCooldown;
			i32	votingMinVotes = m_votingMinVotes;
			float votingRatio = m_votingRatio;

			ser.Value("timeLimit", newTimeLimit, 'fsec');
			ser.Value("scoreLimit", newScoreLimit, 'u16');
			ser.Value("roundLimit", newRoundLimit, 'u16');
			ser.Value("votingEnabled", votingEnabled, 'bool');
			ser.Value("votingCooldown", votingCooldown, 'u16');
			ser.Value("votingMinVotes", votingMinVotes, 'ui5');
			ser.Value("votingRatio", votingRatio, 'unit');

			if (ser.IsReading())
			{
				// Ensure that the cvars are kept in sync as well as the limits that actually get used (otherwise host migration will grab incorrect values)
				if (m_timeLimit != newTimeLimit)
				{
					m_timeLimit = newTimeLimit;
					g_pGameCVars->g_timelimit = m_timeLimit;
				}
				if (m_scoreLimit != newScoreLimit)
				{
					m_scoreLimit = newScoreLimit;
					g_pGameCVars->g_scoreLimit = m_scoreLimit;
				}
				if (m_roundLimit != newRoundLimit)
				{
					m_roundLimit = newRoundLimit;
					g_pGameCVars->g_roundlimit = m_roundLimit;
				}
				if (m_votingEnabled != votingEnabled)
				{
					m_votingEnabled = votingEnabled;
					g_pGameCVars->sv_votingEnable = (m_votingEnabled ? 1 : 0);
				}
				if (m_votingCooldown != votingCooldown)
				{
					m_votingCooldown = votingCooldown;
					g_pGameCVars->sv_votingCooldown = m_votingCooldown;
				}
				if (m_votingMinVotes != votingMinVotes)
				{
					m_votingMinVotes = votingMinVotes;
					g_pGameCVars->sv_votingMinVotes = m_votingMinVotes;
				}
				if (m_votingRatio != votingRatio)
				{
					m_votingRatio = votingRatio;
					g_pGameCVars->sv_votingRatio = m_votingRatio;
				}

			}

			if (IGameRulesVictoryConditionsModule *pVictoryConditionsModule = g_pGame->GetGameRules()->GetVictoryConditionsModule())
			{
				pVictoryConditionsModule->ClUpdatedTimeLimit();
			}
		}
	}

	bool success = true;

	DRX_TODO(06, 10, 2009, "[CG] Make these serialise calls into a list of listeners!");
	if (m_stateModule)
	{
		success &= m_stateModule->NetSerialize(ser, aspect, profile, flags);
	}
	if (m_objectivesModule)
	{
		success &= m_objectivesModule->NetSerialize(ser, aspect, profile, flags);
	}
	if (m_spawningModule)
	{
		success &= m_spawningModule->NetSerialize(ser, aspect, profile, flags);
	}
	if (m_roundsModule)
	{
		success &= m_roundsModule->NetSerialize(ser, aspect, profile, flags);
	}
	if (m_scoringModule)
	{
		success &= m_scoringModule->NetSerialize(ser, aspect, profile, flags);
	}

	success&=NetSerializeTelemetry(ser,aspect,profile,flags);

	return success;
}

bool CGameRules::OnBeginCutScene(IAnimSequence* pSeq, bool bResetFX)
{
	if(!pSeq)
		return false;

	if(m_pExplosionGameEffect)
	{
		m_pExplosionGameEffect->SetCutSceneActive(true);
	}

	gEnv->SetCutsceneIsPlaying(true);

	m_cinematicInput.OnBeginCutScene(pSeq->GetFlags());

	return true;
}

bool CGameRules::OnEndCutScene(IAnimSequence* pSeq)
{
	if(!pSeq)
		return false;

	if(m_pExplosionGameEffect)
	{
		m_pExplosionGameEffect->SetCutSceneActive(false);
	}

	gEnv->SetCutsceneIsPlaying(false);

	m_cinematicInput.OnEndCutScene(pSeq->GetFlags());

	return true;
}

bool CGameRules::IsGameRulesClass(tukk cls)
{
	if(!cls || stricmp(cls, GetEntity()->GetClass()->GetName()))
		return false;
	return true;
}

bool CGameRules::CanPlayerSwitchItem( EntityId playerId )
{
	i32 canSwitch = 1;

	HSCRIPTFUNCTION pfnCanPlayerSwitchItem = 0;
	if (m_script->GetValue("CanPlayerSwitchItem", pfnCanPlayerSwitchItem))
	{
		ScriptHandle playerIdHandle(playerId);
		Script::CallReturn(gEnv->pScriptSystem, pfnCanPlayerSwitchItem, m_script, playerIdHandle, canSwitch);
		gEnv->pScriptSystem->ReleaseFunc(pfnCanPlayerSwitchItem);
	}

	return (canSwitch != 0);
}

bool CGameRules::RulesUseWeaponLoadouts()
{
	i32  use = 1;

	HSCRIPTFUNCTION  func = 0;
	if (m_script->GetValue("RulesUseWeaponLoadouts", func))
	{
		Script::CallReturn(gEnv->pScriptSystem, func, m_script, use);
		gEnv->pScriptSystem->ReleaseFunc(func);
	}

	return (use != 0);
}

void CGameRules::OnActorAction(IActor *pActor, const ActionId& actionId, i32 activationMode, float value)
{
	if (m_actorActionModule != NULL)
	{
		m_actorActionModule->OnActorAction(pActor, actionId, activationMode, value);
	}

	if (m_cinematicInput.IsPlayerNotActive())
	{
		m_cinematicInput.OnAction( (pActor != NULL) ? pActor->GetEntityId() : 0, actionId, activationMode, value );
	}
}

void CGameRules::SetAllPlayerVisibility( const bool bVisible, const bool bIncludeClientPlayer )
{
	IActorSystem *pActorSystem = g_pGame->GetIGameFramework()->GetIActorSystem();
	if(pActorSystem)
	{
		CGameRules::TPlayers players;
		GetPlayers(players);
		EntityId localPlayerId = gEnv->pGame->GetIGameFramework()->GetClientActorId();

		CGameRules::TPlayers::const_iterator iter = players.begin();
		CGameRules::TPlayers::const_iterator end = players.end();
		while(iter != end)
		{
			if(CPlayer* pPlayer = static_cast<CPlayer*>(pActorSystem->GetActor(*iter)))
			{
				if((pPlayer->GetEntityId() != localPlayerId) || bIncludeClientPlayer)
				{
					pPlayer->GetEntity()->Invisible(!bVisible);
				}
			}
			++iter;
		}
	}
}
void CGameRules::RegisterPickupListener( IGameRulesPickupListener *pListener )
{
	if (!stl::find(m_pickupListeners, pListener))
	{
		m_pickupListeners.push_back(pListener);
	}
}

void CGameRules::UnRegisterPickupListener( IGameRulesPickupListener *pListener )
{
	stl::find_and_erase(m_pickupListeners, pListener);
}

void CGameRules::RegisterClientConnectionListener( IGameRulesClientConnectionListener *pListener )
{
	if (!stl::find(m_clientConnectionListeners, pListener))
	{
		m_clientConnectionListeners.push_back(pListener);
	}
}

void CGameRules::UnRegisterClientConnectionListener( IGameRulesClientConnectionListener *pListener )
{
	stl::find_and_erase(m_clientConnectionListeners, pListener);
}

void CGameRules::RegisterTeamChangedListener( IGameRulesTeamChangedListener *pListener )
{
	if (!stl::find(m_teamChangedListeners, pListener))
	{
		m_teamChangedListeners.push_back(pListener);
	}
}

void CGameRules::UnRegisterTeamChangedListener( IGameRulesTeamChangedListener *pListener )
{
	stl::find_and_erase(m_teamChangedListeners, pListener);
}

void CGameRules::RegisterRevivedListener( IGameRulesRevivedListener *pListener )
{
	if (!stl::find(m_revivedListenersVec, pListener))
	{
		m_revivedListenersVec.push_back(pListener);
	}
}

void CGameRules::UnRegisterRevivedListener( IGameRulesRevivedListener *pListener )
{
	stl::find_and_erase(m_revivedListenersVec, pListener);
}

void CGameRules::RegisterSurvivorCountListener( IGameRulesSurvivorCountListener *pListener )
{
	if (!stl::find(m_survivorCountListenersVec, pListener))
	{
		m_survivorCountListenersVec.push_back(pListener);
	}
}

void CGameRules::UnRegisterSurvivorCountListener( IGameRulesSurvivorCountListener *pListener )
{
	stl::find_and_erase(m_survivorCountListenersVec, pListener);
}

void CGameRules::RegisterPlayerStatsListener( IGameRulesPlayerStatsListener *pListener )
{
	if (!stl::find(m_playerStatsListenersVec, pListener))
	{
		m_playerStatsListenersVec.push_back(pListener);
	}
}

void CGameRules::UnRegisterPlayerStatsListener( IGameRulesPlayerStatsListener *pListener )
{
	stl::find_and_erase(m_playerStatsListenersVec, pListener);
}

void CGameRules::RegisterRoundsListener( IGameRulesRoundsListener *pListener )
{
	if (!stl::find(m_roundsListenersVec, pListener))
	{
		m_roundsListenersVec.push_back(pListener);
	}
}

#if USE_PC_PREMATCH
void CGameRules::RegisterPrematchListener( IGameRulesPrematchListener *pListener )
{
	if (!stl::find(m_prematchListenersVec, pListener))
	{
		m_prematchListenersVec.push_back(pListener);
	}
}

void CGameRules::UnRegisterPrematchListener( IGameRulesPrematchListener *pListener )
{
	stl::find_and_erase(m_prematchListenersVec, pListener);
}
#endif // #if USE_PC_PREMATCH

void CGameRules::UnRegisterRoundsListener( IGameRulesRoundsListener *pListener )
{
	stl::find_and_erase(m_roundsListenersVec, pListener);
}

void CGameRules::RegisterClientScoreListener( IGameRulesClientScoreListener *pListener )
{
	stl::push_back_unique(m_clientScoreListenersVec, pListener);
}

void CGameRules::UnRegisterClientScoreListener( IGameRulesClientScoreListener *pListener )
{
	stl::find_and_erase(m_clientScoreListenersVec, pListener);
}

void CGameRules::RegisterActorActionListener( IGameRulesActorActionListener *pListener )
{
	stl::push_back_unique(m_actorActionListenersVec, pListener);
}

void CGameRules::UnRegisterActorActionListener( IGameRulesActorActionListener *pListener )
{
	stl::find_and_erase(m_actorActionListenersVec, pListener);
}

void CGameRules::RegisterKillListener( IGameRulesKillListener *pListener )
{
	if (!stl::find(m_killListeners, pListener))
	{
		m_killListeners.push_back(pListener);
	}
}

void CGameRules::UnRegisterKillListener( IGameRulesKillListener *pListener )
{
	stl::find_and_erase(m_killListeners, pListener);
}

void CGameRules::OnEntityKilledEarly( const HitInfo &hitInfo )
{
	i32 numListeners = m_killListeners.size();
	for (i32 i = 0; i < numListeners; ++ i)
	{
		IGameRulesKillListener *pKillListener = m_killListeners[i];
		pKillListener->OnEntityKilledEarly(hitInfo);
	}
}

void CGameRules::OnEntityKilled( const HitInfo &hitInfo )
{
	i32 numListeners = m_killListeners.size();
	for (i32 i = 0; i < numListeners; ++ i)
	{
		IGameRulesKillListener *pKillListener = m_killListeners[i];
		pKillListener->OnEntityKilled(hitInfo);
	}
}

i32 CGameRules::RegisterModuleRMIListener( IGameRulesModuleRMIListener *pRMIListener )
{
	i32 index = m_moduleRMIListenersVec.size();

	m_moduleRMIListenersVec.push_back(pRMIListener);

	return index;
}

void CGameRules::UnRegisterModuleRMIListener( i32 index )
{
	assert((i32)m_moduleRMIListenersVec.size() > index);
	m_moduleRMIListenersVec[index] = NULL;
}

void CGameRules::OnActionEvent( const SActionEvent& event )
{
	switch(event.m_event)
	{
	case eAE_resetBegin:
		{
			m_isRestarting=true;
			gEnv->pDrxPak->DisableRuntimeFileAccess(false);

			for (TPlayerTeamIdMap::iterator tit=m_playerteams.begin(); tit!=m_playerteams.end(); ++tit)
				tit->second.resize(0);
			ClearEntityTeams();
		}
		break;
	case eAE_resetEnd:
		{
			gEnv->pDrxPak->DisableRuntimeFileAccess(true);
			m_isRestarting=false;
		}
		break;
	}
}

void CGameRules::OnLoadGame(ILoadGame* pLoadGame)
{
	assert (! gEnv->bMultiplayer);
	SGameMechanismEventData data;
	data.m_data_LoadGame.m_interface = pLoadGame;
	CGameMechanismUpr::GetInstance()->Inform(kGMEvent_LoadGame, & data);
}

void CGameRules::OnSaveGame(ISaveGame* pSaveGame)
{
	assert (! gEnv->bMultiplayer);
	SGameMechanismEventData data;
	data.m_data_SaveGame.m_interface = pSaveGame;
	CGameMechanismUpr::GetInstance()->Inform(kGMEvent_SaveGame, & data);
}

//------------------------------------------------------------------------
#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
void CGameRules::SendNetConsoleCommand(tukk msg, u32 to, i32 channelId)
{
	GetGameObject()->InvokeRMI(ClNetConsoleCommand(), NetConsoleCommandParams(msg), to, channelId);
}
#endif

//------------------------------------------------------------------------
IHostMigrationEventListener::EHostMigrationReturn CGameRules::OnInitiate(SHostMigrationInfo& hostMigrationInfo, HMStateType& state)
{
	if (!hostMigrationInfo.ShouldMigrateNub())
	{
		return IHostMigrationEventListener::Listener_Done;
	}

	DrxLog("[Host Migration]: CGameRules::OnInitiate() Saving character for host migration started");

	HostMigrationRemoveNonchanneledPlayers();

	m_bCanUpdateSkillRanking = false;

	IGameRulesStateModule *pStateModule = GetStateModule();
	if (pStateModule != NULL && pStateModule->GetGameState() == IGameRulesStateModule::EGRS_InGame)
	{
		m_hostMigrationTimeSinceGameStarted = (m_cachedServerTime - m_gameStartedTime);
	}

	IGameRulesSpawningModule *pSpawningModule = GetSpawningModule();
	if (pSpawningModule)
	{
		pSpawningModule->HostMigrationStopAddingPlayers();
	}

	if (gEnv->IsClient())
	{
		if (!m_pHostMigrationParams)
		{
			m_pHostMigrationParams = new SHostMigrationClientRequestParams();
			m_pHostMigrationClientParams = new SHostMigrationClientControlledParams();
		}

		CPlayer *pPlayer = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
		if (pPlayer)
		{
			m_pHostMigrationClientParams->m_viewQuat = pPlayer->GetViewRotation();
			m_pHostMigrationClientParams->m_position = pPlayer->GetEntity()->GetPos();

			pe_status_living livStat;
			IPhysicalEntity *pPhysicalEntity = pPlayer->GetEntity()->GetPhysics();
			if (pPhysicalEntity != NULL && (pPhysicalEntity->GetType() == PE_LIVING) && (pPhysicalEntity->GetStatus(&livStat) > 0))
			{
				m_pHostMigrationClientParams->m_velocity = livStat.velUnconstrained;
				m_pHostMigrationClientParams->m_hasValidVelocity = true;
				DrxLog("    velocity={%f,%f,%f}", m_pHostMigrationClientParams->m_velocity.x, m_pHostMigrationClientParams->m_velocity.y, m_pHostMigrationClientParams->m_velocity.z);
			}

			IInventory *pInventory = pPlayer->GetInventory();

			m_pHostMigrationClientParams->m_numExpectedItems = pInventory->GetCount();

			i32 numAmmoTypes = pInventory->GetAmmoTypesCount();
			m_pHostMigrationClientParams->m_pAmmoParams = new SHostMigrationClientControlledParams::SAmmoParams[numAmmoTypes];
			m_pHostMigrationClientParams->m_numAmmoParams = numAmmoTypes;

			DrxLog("  player has %i different ammo types", numAmmoTypes);
			for (i32 i = 0; i < numAmmoTypes; ++ i)
			{
				IEntityClass *pAmmoType = pInventory->GetAmmoType(i);
				i32 ammoCount = pInventory->GetAmmoCount(pAmmoType);

				m_pHostMigrationClientParams->m_pAmmoParams[i].m_pAmmoClass = pAmmoType;
				m_pHostMigrationClientParams->m_pAmmoParams[i].m_count = ammoCount;

				DrxLog("    %s : %i", pAmmoType->GetName(), ammoCount);
			}

			EntityId holseredItemId = pInventory->GetHolsteredItem();
			if (holseredItemId)
			{
				IEntity *pHolsteredEntity = gEnv->pEntitySystem->GetEntity(holseredItemId);
				if (pHolsteredEntity)
				{
					m_pHostMigrationClientParams->m_pHolsteredItemClass = pHolsteredEntity->GetClass();
				}
			}

			IMovementController *pMovementController = pPlayer->GetMovementController();
			SMovementState movementState;
			pMovementController->GetMovementState(movementState);

			m_pHostMigrationClientParams->m_aimDirection = movementState.aimDirection;
			m_pHostMigrationParams->m_environmentalWeaponId = pPlayer->GetPickAndThrowEntity();

			if(!m_pHostMigrationParams->m_environmentalWeaponId) //We don't have a pick & throw weapon, but we may have just dropped/thrown one
			{
				const EntityId prevPickAndThrowEntity = pPlayer->GetPrevPickAndThrowEntity();
				IEntity* pEnvWeapon(NULL);
				if(prevPickAndThrowEntity && (pEnvWeapon = gEnv->pEntitySystem->GetEntity(prevPickAndThrowEntity)))
				{
					m_pHostMigrationParams->m_environmentalWeaponRot = pEnvWeapon->GetWorldRotation();
					m_pHostMigrationParams->m_environmentalWeaponPos = pEnvWeapon->GetWorldPos();
					if(IPhysicalEntity* pPhysics = pEnvWeapon->GetPhysics())
					{
						pe_status_dynamics status_dynamics;
						if(pPhysics->GetStatus(&status_dynamics))
						{
							m_pHostMigrationParams->m_environmentalWeaponVel = status_dynamics.v;
						}
					}
				}
			}
		}
		else
		{
			DRX_ASSERT_MESSAGE(false, "Failed to find client actor when initiating a host migration");
			return IHostMigrationEventListener::Listener_Terminate;
		}

		if (m_pBattlechatter && (!hostMigrationInfo.IsNewHost()))
		{
			m_pBattlechatter->SetLocalPlayer(NULL);
		}
	}

	g_pGame->SetHostMigrationState(CGame::eHMS_WaitingForPlayers);

	CCCPOINT(HostMigration_OnInitiate);
	return IHostMigrationEventListener::Listener_Done;
}

//------------------------------------------------------------------------
IHostMigrationEventListener::EHostMigrationReturn CGameRules::OnDemoteToClient(SHostMigrationInfo& hostMigrationInfo, HMStateType& state)
{
	if (!hostMigrationInfo.ShouldMigrateNub())
	{
		return IHostMigrationEventListener::Listener_Done;
	}

	DrxLogAlways("[Host Migration]: CGameRules::OnDemoteToClient() started");

	if (m_hostMigrationCachedEntities.empty())
	{
		HostMigrationFindDynamicEntities(m_hostMigrationCachedEntities);
	}
	else
	{
		HostMigrationRemoveDuplicateDynamicEntities();
	}

	if (GetObjectivesModule())
	{
		GetObjectivesModule()->OnHostMigration(hostMigrationInfo.IsNewHost());
	}

	DrxLogAlways("[Host Migration]: CGameRules::OnDemoteToClient() finished");

	CCCPOINT(HostMigration_OnDemoteToClient);
	return IHostMigrationEventListener::Listener_Done;
}

//------------------------------------------------------------------------
void CGameRules::HostMigrationFindDynamicEntities(TEntityIdVec &results)
{
	IItemSystem *pItemSystem = g_pGame->GetIGameFramework()->GetIItemSystem();
	IEntityItPtr pEntityIt = gEnv->pEntitySystem->GetEntityIterator();

	while (IEntity *pEntity = pEntityIt->Next())
	{
		if (pEntity->GetFlags() & ENTITY_FLAG_NEVER_NETWORK_STATIC)
		{
			results.push_back(pEntity->GetId());
			DrxLog("    found dynamic entity %i '%s'", pEntity->GetId(), pEntity->GetName());
		}
		else
		{
			CItem *pItem = static_cast<CItem*>(pItemSystem->GetItem(pEntity->GetId()));
			if (pItem)
			{
				// Need to reset owner on static items since they will be given to players again once we've rejoined
				pItem->SetOwnerId(0);
			}
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::HostMigrationRemoveDuplicateDynamicEntities()
{
	DRX_ASSERT(!m_hostMigrationCachedEntities.empty());

	TEntityIdVec dynamicEntities;
	HostMigrationFindDynamicEntities(dynamicEntities);

	DrxLog("CGameRules::HostMigrationRemoveDuplicateDynamicEntities(), found %" PRISIZE_T " entities, already know about %" PRISIZE_T, dynamicEntities.size(), m_hostMigrationCachedEntities.size());
	// Any entities in the dynamicEntities vector that aren't in the m_hostMigrationCachedEntities vector have been added during a previous migration attempt, need to remove them now
	// Note: entities in the m_hostMigrationCachedEntities vector are removed in OnFinalise
	i32k numEntities = dynamicEntities.size();
	for (i32 i = 0; i < numEntities; ++ i)
	{
		EntityId entityId = dynamicEntities[i];

		if (!stl::find(m_hostMigrationCachedEntities, entityId))
		{
			gEnv->pEntitySystem->RemoveEntity(entityId, true);
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::HostMigrationRemoveNonchanneledPlayers()
{
	CGameLobby *pGameLobby = g_pGame->GetGameLobby();
	DRX_ASSERT(pGameLobby);
	if (pGameLobby)
	{
		TPlayers playersToRemove;

		IActorSystem *pActorSystem = g_pGame->GetIGameFramework()->GetIActorSystem();

		playersToRemove.reserve(pActorSystem->GetActorCount());

		IActorIteratorPtr actorIt = pActorSystem->CreateActorIterator();
		IActor *pActor;
		while (pActor = actorIt->Next())
		{
			if (pActor->IsPlayer())
			{
				DRX_ASSERT(pActor->GetChannelId());
				SDrxMatchMakingConnectionUID conId = pGameLobby->GetConnectionUIDFromChannelID((i32) pActor->GetChannelId());
				if (pGameLobby->GetSessionNames().Find(conId) == SSessionNames::k_unableToFind)
				{
					DrxLog("  player '%s' has not got a corresponding CGameLobby entry, removing actor", pActor->GetEntity()->GetName());
					playersToRemove.push_back(pActor->GetEntityId());
				}
			}
		}

		i32k numPlayersToRemove = playersToRemove.size();
		for (i32 i = 0; i < numPlayersToRemove; ++ i)
		{
			FakeDisconnectPlayer(playersToRemove[i]);
		}
	}
}

//------------------------------------------------------------------------
IHostMigrationEventListener::EHostMigrationReturn CGameRules::OnPromoteToServer(SHostMigrationInfo& hostMigrationInfo, u32& state)
{
	if (!hostMigrationInfo.ShouldMigrateNub())
	{
		return IHostMigrationEventListener::Listener_Done;
	}

	DrxLogAlways("[Host Migration]: CGameRules::OnPromoteToServer() started");

	// Server time will change after we migrate (change from old server time to new server time)
	m_gameStartedTime.SetValue(m_gameStartedTime.GetValue() - m_cachedServerTime.GetValue());
	m_gameStartTime.SetValue(m_gameStartTime.GetValue() - m_cachedServerTime.GetValue());

	// If this migration has reset (we're not the original anticipated host, remove any entities from the first attempt
	if (!m_hostMigrationCachedEntities.empty())
	{
		HostMigrationRemoveDuplicateDynamicEntities();
		CPlayer *pPlayer = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(hostMigrationInfo.m_playerID));
		GetBattlechatter()->SetLocalPlayer(pPlayer);
	}

	// Need to do this before the FakeDisconnectPlayer calls below (otherwise CTF breaks since the server side structures
	// haven't been setup when the players are removed)
	if (GetObjectivesModule())
	{
		GetObjectivesModule()->OnHostMigration(hostMigrationInfo.IsNewHost());
	}

	// Now we know we're the server, remove the actors for anyone we know isn't going to migrate
	HostMigrationRemoveNonchanneledPlayers();

	for (u32 i = 0; i < MAX_PLAYERS; ++ i)
	{
		m_migratedPlayerChannels[i] = 0;
	}

	IItemSystem *pItemSystem = g_pGame->GetIGameFramework()->GetIItemSystem();

	IEntityItPtr it = gEnv->pEntitySystem->GetEntityIterator();
	it->MoveFirst();

	IEntity *pEntity = NULL;
	while (pEntity = it->Next())
	{
		IItem *pItem = pItemSystem->GetItem(pEntity->GetId());
		if (pItem)
		{
			if (pItem->GetOwnerId())
			{
				IEntity *pOwner = gEnv->pEntitySystem->GetEntity(pItem->GetOwnerId());
				if (pOwner)
				{
					EntityId currentItemId = 0;

					IActor *pOwnerActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pOwner->GetId());
					if (pOwnerActor)
					{
						IItem *pCurrentItem = pOwnerActor->GetCurrentItem();
						currentItemId = pCurrentItem ? pCurrentItem->GetEntityId() : 0;
					}

					DrxLog("[CG] Item '%s' is owned by '%s'", pEntity->GetName(), pOwner->GetName());
				}
			}
		}
		// Tell entities that we're host migrating
		// - Currently only used by ForbiddenArea but may well be needed for other entities later
		// - Currently only called on the new server, add to OnDemoteToClient if we need to use this on a client
		IScriptTable *pScript = pEntity->GetScriptTable();
		if (pScript != NULL && pScript->GetValueType("OnHostMigration") == svtFunction)
		{
			m_pScriptSystem->BeginCall(pScript, "OnHostMigration");
			m_pScriptSystem->PushFuncParam(pScript);
			m_pScriptSystem->PushFuncParam(true);
			m_pScriptSystem->EndCall();
		}
	}

	// the server does not listen for entity_event_done, clients do however, when we migrate
	// the new server needs to remove any of these events he may be listening for
	TEntityTeamIdMap::iterator entityTeamsIt = m_entityteams.begin();
	for (; entityTeamsIt != m_entityteams.end(); ++ entityTeamsIt)
	{
		EntityId entityId = entityTeamsIt->first;
		RemoveEntityEventDoneListener(entityId);

#if !defined(_RELEASE)
		IEntity* pTeamEntity = gEnv->pEntitySystem->GetEntity(entityId);
		DrxLog("[GameRules] OnPromoteToServer RemoveEntityEventLister(%d(%s), ENTITY_EVENT_DONE, %p)", entityId,
																																																				 pTeamEntity ? pTeamEntity->GetName() : "null",
																																																				 this);
#endif
	}

	ClearRemoveEntityEventListeners();

	i32k numRespawnParams = m_respawndata.size();
	for (i32 i = 0; i < numRespawnParams; ++ i)
	{
		SEntityRespawnData *pData = &m_respawndata[i];
		pEntity = gEnv->pEntitySystem->GetEntity(pData->m_currentEntityId);
		if (pEntity == NULL)
		{
			DrxLog("  detected respawn entity (id=%u) is not present, scheduling for respawn", pData->m_currentEntityId);
			ScheduleEntityRespawn(pData->m_currentEntityId, false, g_pGameCVars->g_defaultItemRespawnTimer);
		}
	}

	if (GetVictoryConditionsModule())
	{
		GetVictoryConditionsModule()->OnHostMigrationPromoteToServer();
	}

	if (GetRoundsModule())
	{
		GetRoundsModule()->OnPromoteToServer();
	}

	DrxLog("[Host Migration]: CGameRules::OnPromoteToServer() finished");

	CCCPOINT(HostMigration_OnPromoteToServer);
	return IHostMigrationEventListener::Listener_Done;
}

//------------------------------------------------------------------------
IHostMigrationEventListener::EHostMigrationReturn CGameRules::OnReconnectClient(SHostMigrationInfo& hostMigrationInfo, u32& state)
{
	if (!hostMigrationInfo.ShouldMigrateNub())
	{
		return IHostMigrationEventListener::Listener_Done;
	}

	DrxLogAlways("[Host Migration]: CGameRules::OnReconnectClient() started");
	if (hostMigrationInfo.IsNewHost())
	{
		// Can't use gamerules cached version of server time since this function will be called before the Update()
		m_gameStartedTime.SetValue(m_gameStartedTime.GetValue() + g_pGame->GetIGameFramework()->GetServerTime().GetValue());
		m_gameStartTime.SetValue(m_gameStartTime.GetValue() + g_pGame->GetIGameFramework()->GetServerTime().GetValue());
	}

	DrxLogAlways("[Host Migration]: CGameRules::OnReconnectClient() finished");
	CCCPOINT(HostMigration_OnReconnectClient);
	return IHostMigrationEventListener::Listener_Done;
}

static void FlushPhysicsQueues()
{
	// Flush the physics linetest and events queue
	if (gEnv->pPhysicalWorld)
	{
		gEnv->pPhysicalWorld->TracePendingRays(0);
		gEnv->pPhysicalWorld->ClearLoggedEvents();
	}
	if (gEnv->p3DEngine)
	{
		IDeferredPhysicsEventUpr* pPhysEventUpr = gEnv->p3DEngine->GetDeferredPhysicsEventUpr();
		if (pPhysEventUpr)
		{
			pPhysEventUpr->ClearDeferredEvents();
		}
	}
}


//------------------------------------------------------------------------
IHostMigrationEventListener::EHostMigrationReturn CGameRules::OnFinalise(SHostMigrationInfo& hostMigrationInfo, u32& state)
{
	if (!hostMigrationInfo.ShouldMigrateNub())
	{
		return IHostMigrationEventListener::Listener_Done;
	}

	DrxLogAlways("[Host Migration]: CGameRules::OnFinalise() started");
	CCCPOINT(HostMigration_OnFinalise);

	if (!hostMigrationInfo.IsNewHost())
	{
		FlushPhysicsQueues();
	}

	m_hostMigrationCachedEntities.clear();

	IGameRulesSpawningModule *pSpawningModule = GetSpawningModule();
	if (pSpawningModule)
	{
		pSpawningModule->HostMigrationResumeAddingPlayers();
	}

	g_pGame->PlayerIdSet(g_pGame->GetIGameFramework()->GetClientActorId());

	DrxLogAlways("[Host Migration]: CGameRules::OnFinalise() finished - success");
	return IHostMigrationEventListener::Listener_Done;
}

//------------------------------------------------------------------------
void CGameRules::OnComplete(SHostMigrationInfo& hostMigrationInfo)
{
	if (!hostMigrationInfo.ShouldMigrateNub() || (hostMigrationInfo.m_state == eHMS_Terminate))
	{
		return;
	}

	CPlayer *pClientActor = static_cast<CPlayer*>(m_pGameFramework->GetClientActor());
	if (pClientActor)
	{
		IEntity *pClientEntity = pClientActor->GetEntity();
		const EntityId clientEntityId = pClientEntity->GetId();

		DrxLog("CGameRules::OnComplete() We have our client actor ('%s'), send migration params", pClientEntity->GetName());

		pClientActor->OnHostMigrationCompleted();

		// Request various bits
		GetGameObject()->InvokeRMI(SvHostMigrationRequestSetup(), *m_pHostMigrationParams, eRMI_ToServer);
		SAFE_DELETE(m_pHostMigrationParams);

		pClientActor->GetEntity()->SetPos(m_pHostMigrationClientParams->m_position);
		pClientActor->SetViewRotation(m_pHostMigrationClientParams->m_viewQuat);

		if (m_pHostMigrationClientParams->m_hasValidVelocity)
		{
			pe_action_set_velocity actionVel;
			actionVel.v = m_pHostMigrationClientParams->m_velocity;
			actionVel.w.zero();
			IPhysicalEntity *pPhysicalEntity = pClientEntity->GetPhysics();
			if (pPhysicalEntity)
			{
				pPhysicalEntity->Action(&actionVel);
			}
		}

		CPlayerMovementController *pPMC = static_cast<CPlayerMovementController *>(pClientActor->GetMovementController());
		if (pPMC)
		{
			// Force an update through so that the aim direction gets set correctly
			pPMC->PostUpdate(0.f);
		}

		if (m_pHostMigrationClientParams->m_pSelectedItemClass)
		{
			CItem *pItem = pClientActor->GetItemByClass(m_pHostMigrationClientParams->m_pSelectedItemClass);
			if (pItem)
			{
				EntityId itemId = pItem->GetEntityId();
				if (pClientActor->GetCurrentItemId() != itemId)
				{
					pClientActor->SelectItem(itemId, false, true);
				}
			}
		}

		m_pHostMigrationClientParams->m_doneEnteredGame = true;
		if (m_pHostMigrationClientParams->IsDone())
		{
			SAFE_DELETE(m_pHostMigrationClientParams);
		}

		if (!gEnv->bServer)
		{
			SHUDEvent hostMigrationOnNewPlayer(eHUDEvent_HostMigrationOnNewPlayer);
			hostMigrationOnNewPlayer.AddData(SHUDEventData(i32(clientEntityId)));
			CHUDEventDispatcher::CallEvent(hostMigrationOnNewPlayer);
		}

		GetBattlechatter()->SetLocalPlayer(pClientActor);

		if (pClientActor->GetPendingDropEntityId())
		{
			pClientActor->DropItem(pClientActor->GetPendingDropEntityId());
		}

		IActorIteratorPtr it = g_pGame->GetIGameFramework()->GetIActorSystem()->CreateActorIterator();
		while (CActor *pActor = static_cast<CActor*>(it->Next()))
		{
			pActor->OnHostMigrationCompleted();
		}
	}

	SetPendingLoadoutChange();

	HostMigrationRemoveNonchanneledPlayers();

	if (hostMigrationInfo.IsNewHost())
	{
		IActorIteratorPtr it = g_pGame->GetIGameFramework()->GetIActorSystem()->CreateActorIterator();
		while (IActor *pActor = it->Next())
		{
				g_pGame->GetGameRules()->RestoreChannelTeamsFromMigration(pActor);
		}

		if(m_mpTrackViewUpr)
		{
			m_mpTrackViewUpr->Init();
		}

		g_pGame->SetHostMigrationState(CGame::eHMS_Resuming);
	}

	ClearAllMigratingPlayers();
}

//------------------------------------------------------------------------
void CGameRules::OnEntityEvent( IEntity *pEntity, SEntityEvent &event )
{
	if (event.event == ENTITY_EVENT_DONE)
	{
		if (!gEnv->bServer)
		{
			DrxLog("[GameRules] OnEntityEvent ENTITY_EVENT_DONE %d(%s) GameRules %p", pEntity->GetId(), pEntity->GetName(), this);
			ClDoSetTeam(0, pEntity->GetId());
		}
		else
		{
			SetTeam(0, pEntity->GetId());
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::OwnClientConnected_NotifyListeners()
{
	EnteredGame();
	i32 numListeners = m_clientConnectionListeners.size();
	for (i32 i = 0; i < numListeners; ++ i)
	{
		m_clientConnectionListeners[i]->OnOwnClientEnteredGame();
	}
}

//------------------------------------------------------------------------
void CGameRules::FakeDisconnectPlayer(EntityId playerId)
{
	// Pretend the player has disconnected
	DRX_TODO(09, 02, 2010, "Deprecate one of these listeners");
	ClientDisconnect_NotifyListeners(playerId);
	i32k numListeners = m_clientConnectionListeners.size();
	for (i32 i = 0; i < numListeners; ++ i)
	{
		m_clientConnectionListeners[i]->OnClientDisconnect(-1, playerId);
	}
	// Remove the actor
	gEnv->pEntitySystem->RemoveEntity(playerId);
}

//------------------------------------------------------------------------
void CGameRules::OnHostMigrationStateChanged()
{
	CGame::EHostMigrationState migrationState = g_pGame->GetHostMigrationState();
	if (migrationState == CGame::eHMS_Resuming)
	{
		if (gEnv->bServer)
		{
			GetGameObject()->InvokeRMI(ClHostMigrationFinished(), NoParams(), eRMI_ToRemoteClients);

			if(CPlayer *pPlayer = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor()))
			{
				//Env Weapon
				EntityId envWeaponId = pPlayer->GetPickAndThrowEntity();
				if(!envWeaponId)
				{
					DrxLog("CGameRules::OnHostMigrationStateChanged - No existing P&T weapon");
					envWeaponId = pPlayer->GetPrevPickAndThrowEntity();
				}

				if(envWeaponId)
				{
					DrxLog("CGameRules::OnHostMigrationStateChanged - But has previous one");
					CEnvironmentalWeapon* pEnvWeap = static_cast<CEnvironmentalWeapon*>(g_pGame->GetIGameFramework()->QueryGameObjectExtension(envWeaponId, "EnvironmentalWeapon"));
					if(pEnvWeap)
					{
						pEnvWeap->OnHostMigration(Quat(ZERO), Vec3(ZERO), Vec3(ZERO));
					}
				}
			}
		}
	}
	else if (migrationState == CGame::eHMS_NotMigrating)
	{
		CPlayer* pClientPlayer = NULL;
		if (gEnv->bServer)
		{
			TPlayers players;
			GetPlayers(players);

			i32k numPlayers = players.size();
			for (i32 i = 0; i < numPlayers; ++ i)
			{
				IActor *pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(players[i]);
				if (pActor)
				{
					pActor->SetMigrating(false);

					if (pActor->IsPlayer())
					{
						CPlayer *pPlayer = static_cast<CPlayer*>(pActor);
						// Check if any players were part way through a stealth kill when we started the migration
						EntityId shooterId = pPlayer->GetStealthKilledBy();
						if (shooterId)
						{
							DrxLog("  player '%s' should be dead from a stealth kill isDead=%s", pPlayer->GetEntity()->GetName(), pPlayer->IsDead() ? "true" : "false");
							if (pPlayer->IsDead() == false)
							{
								// Player was killed by a player who is no longer in the game
								HitInfo hitInfo;
								CStealthKill::ConstructHitInfo(shooterId, pPlayer->GetEntityId(), pPlayer->GetEntity()->GetForwardDir(), hitInfo);
								KillPlayer(pPlayer, true, true, hitInfo);
							}
							else if (pPlayer->GetActorStats()->isRagDoll == false)
							{
								DrxLog("  player is dead but not in ragdoll - player doing the stealth kill probably left in the migration");
								pPlayer->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Ragdoll);
							}
						}

						if(pPlayer->IsClient())
						{
							pClientPlayer = pPlayer;
						}
					}
				}
			}
		}
		else if (CPlayer *pPlayer = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor()))
		{
			pPlayer->SetMigrating(false);

			pClientPlayer = pPlayer;
		}

		if(pClientPlayer && pClientPlayer->GetGrabbedEntityId()) //If we are holding a pick & throw weapon then we need to reset the crosshair
		{
			IEntityClass* pPickAndThrowClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("PickAndThrowWeapon");
			CPickAndThrowWeapon* pPickAndThrowWeapon = static_cast<CPickAndThrowWeapon*>(g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(pClientPlayer->GetInventory()->GetItemByClass(pPickAndThrowClass)));
			if(pPickAndThrowWeapon)
			{
				pPickAndThrowWeapon->OnHostMigration();
			}
		}

		CWeaponSystem *pWeaponSystem = g_pGame->GetWeaponSystem();
		if (pWeaponSystem)
		{
			pWeaponSystem->OnResumeAfterHostMigration();
		}

		// Migration has finished, if we've still got client params then they won't be valid anymore
		SAFE_DELETE(m_pHostMigrationClientParams);
	}
}

//------------------------------------------------------------------------
i32 CGameRules::GetLivingPlayerCount() const
{
	TPlayers players;
	GetPlayers(players);

	i32 numLivingPlayers = 0;
	IActorSystem *pActorSystem = g_pGame->GetIGameFramework()->GetIActorSystem();
	i32k numPlayers = players.size();
	for (i32 i = 0; i < numPlayers; ++ i)
	{
		CPlayer *pPlayer = static_cast<CPlayer*>(pActorSystem->GetActor(players[i]));
		if (pPlayer)
		{
			if ((pPlayer->GetSpectatorState() == CActor::eASS_Ingame) && (pPlayer->GetSpectatorMode() == CActor::eASM_None) && !pPlayer->IsDead())
			{
				++ numLivingPlayers;
			}
		}
	}

	return numLivingPlayers;
}

//------------------------------------------------------------------------
float CGameRules::GetFriendlyFireRatio() const
{
	return g_pGameCVars->g_friendlyfireratio;
}

//------------------------------------------------------------------------
void CGameRules::SetPendingLoadoutChange()
{
	m_bPendingLoadoutChange = true;
}

//------------------------------------------------------------------------
void CGameRules::ClearEntityTeams()
{
	m_entityteams.clear();

	ClearRemoveEntityEventListeners();
}

//------------------------------------------------------------------------
void CGameRules::ClearRemoveEntityEventListeners()
{
	i32 numEntities = m_entityEventDoneListeners.size();
	for (i32 i = 0; i < numEntities; ++ i)
	{
		EntityId entityId = m_entityEventDoneListeners[i];
		gEnv->pEntitySystem->RemoveEntityEventListener(entityId, ENTITY_EVENT_DONE, this);

#if !defined(_RELEASE)
		IEntity *pEntity = gEnv->pEntitySystem->GetEntity(entityId);
		DrxLogAlways("[GameRules] ClearRemoveEntityEventListeners RemoveEntityEventLister(%d(%s), ENTITY_EVENT_DONE, %p)", entityId, pEntity ? pEntity->GetName() : "null", this);
#endif
	}
	m_entityEventDoneListeners.clear();
}

//------------------------------------------------------------------------
bool CGameRules::OnInputEvent(const SInputEvent &rInputEvent)
{
	u32 deviceIndex = g_pGame->GetExclusiveControllerDeviceIndex();

	if (deviceIndex == rInputEvent.deviceIndex && rInputEvent.deviceType != eIDT_Unknown)
	{
		m_idleTime = 0;
	}

	return false;
}

//------------------------------------------------------------------------
void CGameRules::OnUserLeftLobby( i32 channelId )
{
	if (g_pGame->GetHostMigrationState() == CGame::eHMS_WaitingForPlayers)
	{
		i32 migratingIndex = GetMigratingPlayerIndex(channelId);
		if (migratingIndex >= 0)
		{
			// Migrating player has left the lobby so they aren't going to make the migration, remove them

			FinishMigrationForPlayer(migratingIndex);

			IActor *pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActorByChannelId(channelId);
			if (pActor)
			{
				FakeDisconnectPlayer(pActor->GetEntityId());
			}
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::AddForbiddenArea(EntityId entityId)
{
	bool pushed = stl::push_back_unique(m_forbiddenAreas, entityId);

	if( pushed && m_gameStarted )
	{
		SetupForbiddenAreaShapesHelpers();
	}
}

//------------------------------------------------------------------------
void CGameRules::RemoveForbiddenArea(EntityId entityId)
{
	bool removed = stl::find_and_erase(m_forbiddenAreas, entityId);

	if( removed && m_gameStarted )
	{
		SetupForbiddenAreaShapesHelpers();
	}
}

//------------------------------------------------------------------------
void CGameRules::OnLocalPlayerRevived()
{
	// Tell the forbidden areas that the player has been revived
	CallOnForbiddenAreas("OnLocalPlayerRevived");
	m_numLocalPlayerRevives++;
}

//------------------------------------------------------------------------
void CGameRules::CallOnForbiddenAreas( tukk pFuncName )
{
	u32k numForbiddenAreas = m_forbiddenAreas.size();
	for (u32 i = 0; i < numForbiddenAreas; ++ i)
	{
		IEntity *pEntity = gEnv->pEntitySystem->GetEntity(m_forbiddenAreas[i]);
		DRX_ASSERT(pEntity);
		if (pEntity)
		{
			SmartScriptTable pEntityScript = pEntity->GetScriptTable();
			if (pEntityScript && (pEntityScript->GetValueType(pFuncName) == svtFunction))
			{
				m_pScriptSystem->BeginCall(pEntityScript, pFuncName);
				m_pScriptSystem->PushFuncParam(pEntityScript);
				m_pScriptSystem->EndCall();
			}
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::SetPausedGameTimer( bool bPaused, EGameOverReason reason )
{
	if (bPaused)
	{
		m_gamePausedTime = m_cachedServerTime;
		if (reason == EGOR_TimeLimitReached)
		{
			// If the game ran out of time, force the local timer to agree (this is necessary since
			// the timers are sometimes a little out of sync
			float remainingTime = GetRemainingGameTime();
			m_gamePausedTime += CTimeValue(remainingTime);
		}
	}
	else
	{
		m_gamePausedTime.SetValue(0LL);
	}
}

//------------------------------------------------------------------------
void CGameRules::SPlayerEndGameStatsParams::SerializeWith( TSerialize ser )
{
	if (ser.IsWriting())
	{
		IGameRulesPlayerStatsModule *pPlayerStatsModule = g_pGame->GetGameRules()->GetPlayerStatsModule();
		if (pPlayerStatsModule)
		{
			i32 numPlayerStats = pPlayerStatsModule->GetNumPlayerStats();
			numPlayerStats = MIN(numPlayerStats, k_maxPlayerStats);

			m_numPlayerStats = numPlayerStats;
			ser.Value("numStats", m_numPlayerStats, 'ui5');
			for (i32 i = 0; i < numPlayerStats; ++ i)
			{
				const SGameRulesPlayerStat *pPlayerStats = pPlayerStatsModule->GetNthPlayerStats(i);
				m_playerStats[i].m_playerId = pPlayerStats->playerId;
				m_playerStats[i].m_points = pPlayerStats->points;
				m_playerStats[i].m_kills = pPlayerStats->kills;
				m_playerStats[i].m_assists = pPlayerStats->assists;
				m_playerStats[i].m_deaths = pPlayerStats->deaths;
				m_playerStats[i].m_skillPoints = pPlayerStats->skillPoints;

				m_playerStats[i].SerializeWith(ser);
			}
		} // pPlayerStatsModule
	} // ser.IsWriting()
	else
	{
		ser.Value("numStats", m_numPlayerStats, 'ui5');
		i32k numPlayerStats = m_numPlayerStats;
		for (i32 i = 0; i < numPlayerStats; ++ i)
		{
			m_playerStats[i].SerializeWith(ser);
		}
	}
}

//------------------------------------------------------------------------
void CGameRules::SPlayerEndGameStatsParams::SPlayerEndGameStats::SerializeWith( TSerialize ser )
{
	ser.Value("playerId", m_playerId, 'eid');
	ser.Value("points", m_points, 'i32');
	ser.Value("kills", m_kills, 'u16');
	ser.Value("assists", m_assists, 'u16');
	ser.Value("deaths", m_deaths, 'u16');
	ser.Value("skillPoints", m_skillPoints, 'u16');
}

//------------------------------------------------------------------------
void CGameRules::PreCacheItemResources( tukk itemName )
{
	m_equipmentLoadOutPreCacheCallback.PreCacheItemResources(itemName);
}

//------------------------------------------------------------------------
void CGameRules::AddEntityEventDoneListener( EntityId id )
{
	DRX_ASSERT(!stl::find(m_entityEventDoneListeners, id));
	m_entityEventDoneListeners.push_back(id);
	gEnv->pEntitySystem->AddEntityEventListener(id, ENTITY_EVENT_DONE, this);
}

//------------------------------------------------------------------------
void CGameRules::RemoveEntityEventDoneListener( EntityId id )
{
	DRX_ASSERT(stl::find(m_entityEventDoneListeners, id));
	stl::find_and_erase(m_entityEventDoneListeners, id);
	gEnv->pEntitySystem->RemoveEntityEventListener(id, ENTITY_EVENT_DONE, this);
}

//------------------------------------------------------------------------
bool CGameRules::IsGameInProgress() const
{
	bool bInProgress = true;

	if (m_stateModule)
	{
		bInProgress = (m_stateModule->GetGameState() == IGameRulesStateModule::EGRS_InGame);
	}

	if (m_roundsModule)
	{
		bInProgress = (bInProgress && m_roundsModule->IsInProgress());
	}

	return bInProgress;
}

//------------------------------------------------------------------------
bool CGameRules::HUDScoreElementTimerShouldCountDown() const
{
	const bool  should = ((!m_stateModule || (m_stateModule->GetGameState() != IGameRulesStateModule::EGRS_PreGame)) &&
												(!m_roundsModule || m_roundsModule->IsInProgress()));
	return should;
}

//------------------------------------------------------------------------
EDisconnectionCause CGameRules::SvGetLastTeamDiscoCause(i32k teamId) const
{
	DRX_ASSERT_MESSAGE(GetTeamCount() > 0, "This team-game function is being called in a non-team based game mode");
	DRX_ASSERT_MESSAGE((teamId == 1) || (teamId == 2), "An invalid team id was passed to this team-game function");

	EDisconnectionCause cause = m_svLastTeamDiscoCause[teamId - 1];

	return cause;
}

//------------------------------------------------------------------------
EDisconnectionCause CGameRules::SvGetLastDiscoCause() const
{
	EDisconnectionCause  cause = eDC_Unknown;
	if (GetTeamCount() == 0)
	{
		cause = m_svLastTeamDiscoCause[0];
	}
	else
	{
		DRX_ASSERT_MESSAGE(0, "This non-team function is being called in a team-based game mode");
	}
	return cause;
}

//------------------------------------------------------------------------
IItem* CGameRules::GetCurrentItemForActorWithStatus(IActor* pActor, bool* outIsUsing, bool* outIsDroppable)
{
	IItem*  pItem = NULL;
	bool  isUsing = false;
	bool  isDroppable = false;

	CActor*  pCActor = (CActor*) pActor;

	IInventory*  pInventory = pActor->GetInventory();
	EntityId  itemId = (pInventory ? pInventory->GetCurrentItem() : 0);

	if (itemId && !pActor->GetLinkedVehicle())
	{
		pItem = pCActor->GetItem(itemId);

		if (pItem)
		{
			isUsing = pItem->IsUsed();
			isDroppable = (!isUsing && (!pActor->IsPlayer() || !CGodMode::GetInstance().IsDemiGod()));
		}
	}

	if (outIsUsing)
	{
		(*outIsUsing) = isUsing;
	}
	if (outIsDroppable)
	{
		(*outIsDroppable) = isDroppable;
	}
	return pItem;
}

//------------------------------------------------------------------------
bool CGameRules::ActorShouldHideCurrentItemInsteadOfDroppingOnDeath(IActor* pActor)
{
	bool  hideInstead = false;

	if (m_gameMode == eGM_Assault)
	{
		DRX_ASSERT(m_roundsModule);
		i32k  attackingTeam = m_roundsModule->GetPrimaryTeam();
		i32k  actorTeam = GetTeam(pActor->GetEntityId());
		DRX_ASSERT(attackingTeam != 0 && actorTeam != 0);

		hideInstead = (actorTeam != attackingTeam);
	}

	return hideInstead;
}

void CGameRules::ResetQueuedExplosionsAndHits()
{
	for(i32 i = 0; i < MAX_CONCURRENT_EXPLOSIONS; i++)
	{
		m_explosions[i].m_mfxInfo.Reset();
		m_explosionValidities[i]	= false;
	}

	while (!m_queuedExplosions.empty())
		m_queuedExplosions.pop();

	while (!m_queuedExplosionsAwaitingRaycasts.empty())
		m_queuedExplosionsAwaitingRaycasts.pop();

	while (!m_queuedHits.empty())
		m_queuedHits.pop();

	m_processingHit = 0;
}

void CGameRules::SetupForbiddenAreaShapesHelpers()
{
	m_forbiddenAreaHelpers.clear();

	IAreaUpr* pAreaUpr = gEnv->pEntitySystem->GetAreaUpr();
	if(pAreaUpr)
	{
		u32k numForbiddenAreas = m_forbiddenAreas.size();
		for (u32 i = 0; i < numForbiddenAreas; ++ i)
		{
			i32k k_shapeArraySize = 16;
			i32 shapeArrayCount = k_shapeArraySize;
			EntityId shapeArray[k_shapeArraySize];
			memset(shapeArray, 0, sizeof(shapeArray));

			EntityId forbiddenAreaId = m_forbiddenAreas[i];

			const bool success = pAreaUpr->GetLinkedAreas(forbiddenAreaId, &shapeArray[0], shapeArrayCount);
			DRX_ASSERT_MESSAGE(success, "increasing k_shapeArraySize will fix this, or linking less entities to the area");

			bool reversed = false;
			bool resetsObjects = true;
			IEntity *pFAEntity = gEnv->pEntitySystem->GetEntity(forbiddenAreaId);
			if (pFAEntity)
			{
				IScriptTable* pTable = pFAEntity->GetScriptTable();
				SmartScriptTable pEntityProperties;
				if(pTable && pTable->GetValue("Properties",pEntityProperties))
				{
					pEntityProperties->GetValue("bReversed",reversed);
					pEntityProperties->GetValue("bResetsObjects",resetsObjects);
				}
			}

			for(i32 a = 0; a < shapeArrayCount; a++)
			{
				EntityId shapeId = shapeArray[a];
				SForbiddenAreaHelper areaHelper(shapeId, reversed, resetsObjects, forbiddenAreaId);
				m_forbiddenAreaHelpers.push_back(areaHelper);
			}
		}
	}
}

bool CGameRules::IsInsideForbiddenArea(const Vec3& pos, bool doResetCheck, IEntity** ppArea )
{
	i32k helperCount = m_forbiddenAreaHelpers.size();
	for(i32 i = 0; i < helperCount; i++)
	{
		SForbiddenAreaHelper helper = m_forbiddenAreaHelpers[i];
		EntityId shapeId = helper.shapeId;
		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(shapeId);
		if(pEntity)
		{
			IEntityAreaProxy *pAreaProxy = (IEntityAreaProxy*)pEntity->GetProxy(ENTITY_PROXY_AREA);
			if(pAreaProxy)
			{
				bool inside = pAreaProxy->CalcPointWithin(INVALID_ENTITYID, pos);

				if(helper.reversed)
				{
					inside = !inside;
				}

				if(inside)
				{
					if( ppArea )
					{
						*ppArea = gEnv->pEntitySystem->GetEntity( helper.parentId );
					}
					//TODO: could cause a bug if areas overlap
					return doResetCheck ? helper.resetsObjects : true;
				}
			}
		}
	}

	return false;
}

void CGameRules::OnTimeOfDaySet()
{
	// If we are the server lets netserialise this to everyone
	if(gEnv->bServer && gEnv->bMultiplayer)
	{
		CHANGED_NETWORK_STATE(this, GAMERULES_TIME_OF_DAY_DYNAMIC_ASPECT);
	}
}

u8 CGameRules::GetRequiredPlayerTypesForGameMode()
{
	u8 requiredPlayerTypes = k_rptfgm_none;

	switch(m_gameMode)
	{
		case eGM_InstantAction:
		case eGM_TeamInstantAction:
		case eGM_CaptureTheFlag:
		case eGM_CrashSite:
		case eGM_PowerStruggle:
		case eGM_Extraction:
			requiredPlayerTypes |= k_rptfgm_marines;
			break;
		case eGM_Assault:
			requiredPlayerTypes = k_rptfgm_standard|k_rptfgm_marines;
			break;
		case eGM_Gladiator:
			requiredPlayerTypes = k_rptfgm_standard|k_rptfgm_hunter|k_rptfgm_hunter_marine;
			break;
		default:
			DRX_ASSERT_MESSAGE(0, string().Format("CGameRules::GetRequiredPlayerTypesForGameMode() found unhandled gameMode=%d", m_gameMode));
			break;
	}

	return requiredPlayerTypes;
}

bool CGameRules::GameModeRequiresDifferentCloakedChatter()
{
	return m_gameMode != eGM_Gladiator; // cloaked chatter uses normal chatter in gladiator/hunter
}


//TODO: This should be handled by a module of some kind
u8 CGameRules::GetRequiredPlayerTypeForConversation(i32 speakingActorTeamId, i32 listeningActorTeamId)
{
	u8 conversationPlayerType = k_rptfgm_none;

	bool speakerAndListenerAreFriends=false;	// we dont have entityIds to compare. When no teams the only friendly is yourself, well you can't hear yourself speak battlechatter so it doesn't matter

	if (GetTeamCount()>=2)
	{
		if (speakingActorTeamId == 0 || listeningActorTeamId == 0)
		{
			speakerAndListenerAreFriends=true;	// one of the actors is neutral, hence they're friends
		}
		else
		{
			speakerAndListenerAreFriends = (speakingActorTeamId == listeningActorTeamId);
		}
	}

	switch(m_gameMode)
	{
		case eGM_InstantAction:
		case eGM_TeamInstantAction:
		case eGM_CaptureTheFlag:
		case eGM_CrashSite:
		case eGM_PowerStruggle:
		case eGM_Extraction:
			if (speakerAndListenerAreFriends)
			{
				conversationPlayerType = k_rptfgm_standard;
			}
			else
			{
				conversationPlayerType = k_rptfgm_marines;
			}
			break;
		case eGM_Assault:
		{
			i32  attackingTeamId = m_roundsModule->GetPrimaryTeam();
			if (listeningActorTeamId == attackingTeamId)
			{
				if (speakerAndListenerAreFriends)
				{
					conversationPlayerType = k_rptfgm_standard;
				}
				else
				{
					conversationPlayerType = k_rptfgm_marines;
				}
			}
			else
			{
				conversationPlayerType = k_rptfgm_standard;
			}
			break;
		}
		case eGM_Gladiator:
			if (listeningActorTeamId == CGameRulesObjective_Predator::TEAM_SOLDIER)
			{
				if (speakerAndListenerAreFriends)
				{
					conversationPlayerType = k_rptfgm_hunter_marine;
				}
				else
				{
					conversationPlayerType = k_rptfgm_hunter;
				}
			}
			else
			{
				if (speakerAndListenerAreFriends)
				{
					conversationPlayerType = k_rptfgm_hunter;
				}
				else
				{
					conversationPlayerType = k_rptfgm_hunter_marine;
				}
			}
			break;
		default:
			DRX_ASSERT_MESSAGE(0, string().Format("CGameRules::GetRequiredPlayerTypeForConversation() found unhandled gameMode=%d", m_gameMode));
			break;
	}

	return conversationPlayerType;
}

void CGameRules::OnSystemEvent( ESystemEvent event,UINT_PTR wparam,UINT_PTR lparam )
{
	switch(event)
	{
		case	ESYSTEM_EVENT_LEVEL_LOAD_END:
			{
				if(IGameRulesSpectatorModule * pSpectatorModule = GetSpectatorModule())
				{
					EntityId spectatorPositionId = pSpectatorModule->GetSpectatorLocation(0);

					if(IEntity * pEntity = gEnv->pEntitySystem->GetEntity(spectatorPositionId))
					{
						gEnv->p3DEngine->OverrideCameraPrecachePoint(pEntity->GetWorldPos());
					}
				}
			}
			break;
		case	ESYSTEM_EVENT_LEVEL_PRECACHE_END:
			{
				CTelemetryCollector* pTelemetryCollector = (CTelemetryCollector*)g_pGame->GetITelemetryCollector();
				if (pTelemetryCollector)
					pTelemetryCollector->CreateStatoscopeStream();
			}
			break;
		case ESYSTEM_EVENT_LEVEL_UNLOAD:
			{
				if(CEquipmentLoadout* pEquipmentLoadout = g_pGame->GetEquipmentLoadout())
				{
					pEquipmentLoadout->ReleaseStreamedFPGeometry( CEquipmentLoadout::ePGE_All );
				}
			}
			break;
		case ESYSTEM_EVENT_SW_SHIFT_WORLD:
			{
				if(!gEnv->bMultiplayer && gEnv->bServer)
					break;

				IActorIteratorPtr actorIt = g_pGame->GetIGameFramework()->GetIActorSystem()->CreateActorIterator();
				IActor *pActor;
				while (pActor = actorIt->Next())
				{
					pActor->OnShiftWorld();
				}
			}
			break;
		default:
			break;
	}
}

#ifndef _RELEASE
bool HitInfo::IsPartIDInvalid()
{
	if(partId > 1023 || partId < -1)
	{
		if(IActor *pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(targetId))
		{
			return true;
		}
	}

	return false;
}
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void SDeferredMfxExplosion::Reset()
{
	if (m_rayId != 0)
	{
		g_pGame->GetRayCaster().Cancel(m_rayId);
		m_rayId = 0;
	}

	m_state = eDeferredMfxExplosionState_None;
	m_mfxTargetSurfaceId = 0;
	m_pMfxTargetPhysEnt = NULL;
}

void SDeferredMfxExplosion::OnRayCastDataReceived( const QueuedRayID& rayID, const RayCastResult& result )
{
	DRX_ASSERT(rayID == m_rayId);

	m_rayId = 0;

	if(result.hitCount > 0)
	{
		m_mfxTargetSurfaceId	= result.hits[0].surface_idx;
		m_pMfxTargetPhysEnt		= result.hits[0].pCollider;
		m_state = eDeferredMfxExplosionState_ResultImpact;
	}
	else
	{
		m_state = eDeferredMfxExplosionState_ResultNoImpact;
	}
}

#if defined(DEV_CHEAT_HANDLING)
void CGameRules::HandleDevCheat(u16 channelId, tukk  message)
{
	GetGameObject()->InvokeRMI(ClHandleCheatAccusation(), DevCheatHandlingParams(message), eRMI_ToClientChannel, channelId);
}
#endif

#if USE_PC_PREMATCH
void CGameRules::StartPrematch()
{
	ChangePrematchState(ePS_PrematchWaitingForPlayers);
}

void CGameRules::SkipPrematch()
{
	ForceBalanceTeams();

	ChangePrematchState(ePS_Match);
}

void CGameRules::ForceBalanceTeams()
{
	CGameLobby *pGameLobby = g_pGame->GetGameLobby();
	if (pGameLobby && pGameLobby->UseLobbyTeamBalancing() && !pGameLobby->IsGameBalanced())
	{
		pGameLobby->ForceBalanceTeams();
		i32 numChannelIds = m_channelIds.size();
		for (i32 i = 0; i < numChannelIds; ++ i)
		{
			i32 channelId = m_channelIds[i];
			IActor *pActor = GetActorByChannelId(channelId);
			if (pActor)
			{
				i32 newTeamId = pGameLobby->GetTeamByChannelId(channelId);
				i32 oldTeamId = GetTeam(pActor->GetEntityId());
				if (newTeamId != oldTeamId)
				{
					SetTeam(newTeamId, pActor->GetEntityId());
				}
			}
		}
	}
}
#endif
