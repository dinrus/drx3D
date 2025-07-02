// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameStartup.h>
#include <drx3D/Game/Game.h>
#include <drx3D/Game/GameCVars.h>

#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/CoreX/String/DrxFixedString.h>
#include <drx3D/CoreX/Platform/DrxLibrary.h>
#include <drx3D/CoreX/Platform/platform_impl.inl>
#include <drx3D/Network/INetworkService.h>

#include <drx3D/Input/IHardwareMouse.h>
#include <drx3D/Sys/File/IDrxPak.h>
#include <drx3D/Sys/ILocalizationUpr.h>
#include <drx3D/Game/Editor/GameRealtimeRemoteUpdate.h>
#include <drx3D/Game/Utility/StringUtils.h>

#include <drx3D/Game/Testing/AutoTester.h>
#include <DrxThreading/IJobUpr.h>

#include <DrxThreading/IThreadUpr.h>
#include <DrxThreading/IThreadConfigUpr.h>

#if ENABLE_AUTO_TESTER 
static CAutoTester s_autoTesterSingleton;
#endif
 
#if defined(ENABLE_STATS_AGENT)
#include <drx3D/Game/StatsAgent.h>
#endif

#ifdef __LINK_GCOV__
extern "C" void __gcov_flush(void);
#define GCOV_FLUSH __gcov_flush()
namespace
{
	static void gcovFlushUpdate()
	{
		static unsigned sCounter = 0;
		static const sInterval = 1000;

		if (++sCounter == sInterval)
		{
			__gcov_flush();
			sCounter = 0;
		}
	}
}
#define GCOV_FLUSH_UPDATE gcovFlushUpdate()
#else
#define GCOV_FLUSH ((void)0)
#define GCOV_FLUSH_UPDATE ((void)0)
#endif

#if defined(_LIB) || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	extern "C" IGameFramework *CreateGameFramework();
#endif

#define DLL_INITFUNC_CREATEGAME "CreateGameFramework"

#if DRX_PLATFORM_WINDOWS
bool g_StickyKeysStatusSaved = false;
STICKYKEYS g_StartupStickyKeys = {sizeof(STICKYKEYS), 0};
TOGGLEKEYS g_StartupToggleKeys = {sizeof(TOGGLEKEYS), 0};
FILTERKEYS g_StartupFilterKeys = {sizeof(FILTERKEYS), 0};

#endif

#if defined(CVARS_WHITELIST)
CCVarsWhiteList g_CVarsWhiteList;

bool IsCommandLiteral(tukk pLiteral, tukk pCommand)
{ 
	// Compare the command with the literal, for the length of the literal
	while ((*pLiteral) && (tolower(*pLiteral++) == tolower(*pCommand++)));

	// If they're the same, ensure the whole command was tested
	return (!*(pLiteral) && (!(*pCommand) || isspace(*pCommand)));
}

#define WHITELIST(_stringliteral) if (IsCommandLiteral(_stringliteral, pCommandMod)) { return true; }
bool CCVarsWhiteList::IsWhiteListed(const string& command, bool silent)
{
	tukk  pCommandMod = command.c_str();
	if(pCommandMod[0] == '+')
	{
		pCommandMod++;
	}
	
	WHITELIST("sys_game_folder");
	
	WHITELIST("map");
	WHITELIST("i_mouse_smooth");
	WHITELIST("i_mouse_accel");
	WHITELIST("i_mouse_accel_max");
	WHITELIST("cl_sensitivity");
	WHITELIST("pl_movement.power_sprint_targetFov");
	WHITELIST("cl_fov");
	WHITELIST("hud_canvas_width_adjustment");
	WHITELIST("r_DrawNearFoV");
	WHITELIST("g_skipIntro");
	WHITELIST("hud_psychoPsycho");
	WHITELIST("hud_hide");
	WHITELIST("disconnect");

	WHITELIST("hud_bobHud");
  WHITELIST("e_CoverageBufferReproj");
  WHITELIST("e_LodRatio");
  WHITELIST("e_ViewDistRatio");
  WHITELIST("e_ViewDistRatioVegetation");
  WHITELIST("e_ViewDistRatioDetail");
  WHITELIST("e_MergedMeshesInstanceDist");
  WHITELIST("e_MergedMeshesViewDistRatio");
  WHITELIST("e_ParticlesObjectCollisions");
  WHITELIST("e_ParticlesMotionBlur");
  WHITELIST("e_ParticlesForceSoftParticles");
  WHITELIST("e_Tessellation");
  WHITELIST("e_TessellationMaxDistance");
  WHITELIST("r_TessellationTriangleSize");
  WHITELIST("r_SilhouettePOM");
	WHITELIST("e_GI");
	WHITELIST("e_GICache");
  WHITELIST("e_GIIterations");
	WHITELIST("e_ShadowsPoolSize");
	WHITELIST("e_ShadowsMaxTexRes");
	WHITELIST("r_FogShadows");
	WHITELIST("r_FogShadowsWater");
	WHITELIST("e_ParticlesShadows");
	WHITELIST("e_ShadowsTessellateCascades");
  WHITELIST("e_ShadowsResScale");
  WHITELIST("e_GsmCache");
  WHITELIST("r_WaterTessellationHW");
  WHITELIST("r_DepthOfField");
	WHITELIST("r_MotionBlur");
	WHITELIST("r_MotionBlurShutterSpeed");
	WHITELIST("g_radialBlur");
	WHITELIST("cl_zoomToggle");
	WHITELIST("r_TexMinAnisotropy");
	WHITELIST("r_TexMaxAnisotropy");
  WHITELIST("r_TexturesStreamPoolSize");
	WHITELIST("cl_crouchToggle");
	WHITELIST("r_ColorGrading");
	WHITELIST("r_SSAO");
	WHITELIST("r_SSDO");
	WHITELIST("r_SSReflections");
	WHITELIST("r_VSync");
	WHITELIST("r_DisplayInfo");
	WHITELIST("r_displayinfoTargetFPS");
  WHITELIST("r_ChromaticAberration");
	WHITELIST("r_HDRChromaShift");
	WHITELIST("r_HDRGrainAmount");
	WHITELIST("r_HDRBloomRatio");
	WHITELIST("r_HDRBrightLevel");
  WHITELIST("r_Sharpening");
	WHITELIST("r_Gamma");
	WHITELIST("r_GetScreenShot");
	WHITELIST("r_FullscreenWindow");
	WHITELIST("r_Fullscreen");
	WHITELIST("r_width");
	WHITELIST("r_height");
	WHITELIST("r_MultiGPU");
	WHITELIST("r_overrideDXGIOutput");
	WHITELIST("r_overrideDXGIAdapter");
	WHITELIST("r_FullscreenPreemption");
  WHITELIST("r_buffer_sli_workaround");
	WHITELIST("r_DeferredShadingAmbientSClear");
	WHITELIST("g_useHitSoundFeedback");
	WHITELIST("sys_MaxFps");
	WHITELIST("g_language");

	WHITELIST("sys_spec_ObjectDetail");
	WHITELIST("sys_spec_Shading");
	WHITELIST("sys_spec_VolumetricEffects");
	WHITELIST("sys_spec_Shadows");
	WHITELIST("sys_spec_Texture");
	WHITELIST("sys_spec_Physics");
	WHITELIST("sys_spec_PostProcessing");
	WHITELIST("sys_spec_Particles");
	WHITELIST("sys_spec_Sound");
	WHITELIST("sys_spec_Water");
	WHITELIST("sys_spec_GameEffects");
	WHITELIST("sys_spec_Light");

	WHITELIST("g_dedi_email");
	WHITELIST("g_dedi_password");

	WHITELIST("root");
	WHITELIST("logfile");
	WHITELIST("ResetProfile");
	WHITELIST("nodlc");

	WHITELIST("rcon_connect");
	WHITELIST("rcon_disconnect");
	WHITELIST("rcon_command");

	WHITELIST("quit");
	WHITELIST("votekick");
	WHITELIST("vote");

	WHITELIST("net_blaze_voip_enable");

	WHITELIST("sys_vr_support");

#if defined(DEDICATED_SERVER)
	WHITELIST("ban");
	WHITELIST("ban_remove");
	WHITELIST("ban_status");
	WHITELIST("ban_timeout");
	WHITELIST("kick"); 
	WHITELIST("startPlaylist");
	WHITELIST("status");
	WHITELIST("gl_map");
	WHITELIST("gl_gamerules");
	WHITELIST("sv_gamerules");
	WHITELIST("sv_password");
	WHITELIST("maxplayers");
	WHITELIST("sv_servername");

	WHITELIST("sv_bind");
	WHITELIST("g_scoreLimit");
	WHITELIST("g_timelimit");
	WHITELIST("g_minplayerlimit");
	WHITELIST("g_autoReviveTime");
	WHITELIST("g_numLives");
	WHITELIST("g_maxHealthMultiplier");
	WHITELIST("g_mpRegenerationRate");
	WHITELIST("g_friendlyfireratio");
	WHITELIST("g_mpHeadshotsOnly");
	WHITELIST("g_mpNoVTOL");
	WHITELIST("g_mpNoEnvironmentalWeapons");
	WHITELIST("g_allowCustomLoadouts");
	WHITELIST("g_allowFatalityBonus");
	WHITELIST("g_modevarivar_proHud");
	WHITELIST("g_modevarivar_disableKillCam");
	WHITELIST("g_modevarivar_disableSpectatorCam");
	WHITELIST("g_multiplayerDefault");
	WHITELIST("g_allowExplosives");
	WHITELIST("g_forceWeapon");
	WHITELIST("g_allowWeaponCustomisation");
	WHITELIST("g_infiniteCloak");
	WHITELIST("g_infiniteAmmo");
	WHITELIST("g_forceHeavyWeapon");
	WHITELIST("g_forceLoadoutPackage");


	WHITELIST("g_autoAssignTeams");
	WHITELIST("gl_initialTime");
	WHITELIST("gl_time");
	WHITELIST("g_gameRules_startTimerLength");
	WHITELIST("sv_maxPlayers");
	WHITELIST("g_switchTeamAllowed");
	WHITELIST("g_switchTeamRequiredPlayerDifference");
	WHITELIST("g_switchTeamUnbalancedWarningDifference");
	WHITELIST("g_switchTeamUnbalancedWarningTimer");

	WHITELIST("http_startserver");
	WHITELIST("http_stopserver");
	WHITELIST("http_password");

	WHITELIST("rcon_startserver");
	WHITELIST("rcon_stopserver");
	WHITELIST("rcon_password");

	WHITELIST("gl_StartGame");
	WHITELIST("g_messageOfTheDay");
	WHITELIST("g_serverImageUrl");

	WHITELIST("log_Verbosity");
	WHITELIST("log_WriteToFile");
	WHITELIST("log_WriteToFileVerbosity");
	WHITELIST("log_IncludeTime");
	WHITELIST("log_tick");
	WHITELIST("net_log");

	WHITELIST("g_pinglimit");
	WHITELIST("g_pingLimitTimer");

	WHITELIST("g_tk_punish");
	WHITELIST("g_tk_punish_limit");
	WHITELIST("g_idleKickTime");

  WHITELIST("net_reserved_slot_system");
	WHITELIST("net_add_reserved_slot");
	WHITELIST("net_remove_reserved_slot");
	WHITELIST("net_list_reserved_slot");
	
	WHITELIST("sv_votingCooldown");
	WHITELIST("sv_votingRatio");
	WHITELIST("sv_votingTimeout");
	WHITELIST("sv_votingEnable");
	WHITELIST("sv_votingBanTime");

	WHITELIST("g_dataRefreshFrequency");
	WHITELIST("g_quitOnNewDataFound");
	WHITELIST("g_quitNumRoundsWarning");
	WHITELIST("g_allowedDataPatchFailCount");
	WHITELIST("g_shutdownMessageRepeatTime");
	WHITELIST("g_shutdownMessage");
	WHITELIST("g_patchPakDediServerMustPatch");

	WHITELIST("g_server_region");

	WHITELIST("net_log_dirtysock");
#endif

	WHITELIST("sys_user_folder");
	WHITELIST("sys_screensaver_allowed");
	WHITELIST("sys_UncachedStreamReads");

	if (!silent)
	{
		string temp = command.Left(command.find(' '));
		if (temp.empty())
		{
			temp = command;
		}

#if defined(DEDICATED_SERVER)
		DrxLogAlways("[Warning] Unknown command: %s", temp.c_str());
#else
		DrxLog("[Warning] Unknown command: %s", temp.c_str());
#endif
	}

	return false;
}
#endif // defined(CVARS_WHITELIST)

static void RestoreStickyKeys();

static void AllowAccessibilityShortcutKeys(bool bAllowKeys)
{
// disabling sticky keys for now since they are not being restored, atexit is not being called because the game process is killed by the system
//#if DRX_PLATFORM_WINDOWS
#if 0
	if(!g_StickyKeysStatusSaved)
	{
		SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &g_StartupStickyKeys, 0);
		SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &g_StartupToggleKeys, 0);
		SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &g_StartupFilterKeys, 0);
		g_StickyKeysStatusSaved = true;
		atexit(RestoreStickyKeys);
	}

	if(bAllowKeys)
	{
		// Restore StickyKeys/etc to original state and enable Windows key      
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &g_StartupStickyKeys, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &g_StartupToggleKeys, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &g_StartupFilterKeys, 0);
	}
	else
	{
		STICKYKEYS skOff = g_StartupStickyKeys;
		skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
		skOff.dwFlags &= ~SKF_CONFIRMHOTKEY; 
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &skOff, 0);

		TOGGLEKEYS tkOff = g_StartupToggleKeys;
		tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
		tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tkOff, 0);

		FILTERKEYS fkOff = g_StartupFilterKeys;
		fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
		fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fkOff, 0);
	}
#endif
}
static void RestoreStickyKeys()
{
	AllowAccessibilityShortcutKeys(true);
}

#define EYEADAPTIONBASEDEFAULT		0.25f					// only needed for Crysis

#if DRX_PLATFORM_WINDOWS
void debugLogCallStack()
{
	// Print call stack for each find.
	tukk funcs[32];
	i32 nCount = 32;

	DrxLogAlways( "    ----- CallStack () -----");
	gEnv->pSystem->debug_GetCallStack( funcs, nCount);
	for (i32 i = 1; i < nCount; i++) // start from 1 to skip this function.
	{
		DrxLogAlways( "    %02d) %s",i,funcs[i] );
	}
}
#endif

void GameStartupErrorObserver::OnAssert(tukk condition, tukk message, tukk fileName, u32 fileLineNumber)
{
	if(!g_pGameCVars)
		return;

	if (g_pGameCVars->cl_logAsserts != 0)
		DrxLogAlways("---ASSERT: condition:%s; message:%s; fileName:%s; line %d", condition, message, fileName, fileLineNumber);

#if DRX_PLATFORM_WINDOWS
	if (g_pGameCVars->cl_logAsserts > 1)
	{
		debugLogCallStack();
		DrxLogAlways("----------------------------------------");
	}
#endif
}

void GameStartupErrorObserver::OnFatalError(tukk message)
{
	DrxLogAlways("---FATAL ERROR: message:%s", message);

#if DRX_PLATFORM_WINDOWS
	gEnv->pSystem->debug_LogCallStack();
	DrxLogAlways("----------------------------------------");
#endif
}

//////////////////////////////////////////////////////////////////////////

CGameStartup* CGameStartup::Create()
{
	static char buff[sizeof(CGameStartup)];
	return new (buff) CGameStartup();
}

CGameStartup::CGameStartup()
	:	m_pMod(NULL),
		m_modRef(&m_pMod),
		m_quit(false),
		m_reqModUnload(false),
		m_modDll(0), 
		m_frameworkDll(NULL),
		m_pFramework(NULL),
		m_fullScreenCVarSetup(false),
		m_nVOIPWasActive(-1)
{
	CGameStartupStatic::g_pGameStartup = this;
}

CGameStartup::~CGameStartup()
{
	if (m_pMod)
	{
		m_pMod->Shutdown();
		m_pMod = 0;
	}

	if (m_modDll)
	{
		DrxFreeLibrary(m_modDll);
		m_modDll = 0;
	}

	CGameStartupStatic::g_pGameStartup = NULL;

	ShutdownFramework();
}

#define EngineStartProfiler(x)
#define InitTerminationCheck(x)

static inline void InlineInitializationProcessing(tukk sDescription)
{
	EngineStartProfiler( sDescription );
	InitTerminationCheck( sDescription );
	gEnv->pLog->UpdateLoadingScreen(0);
}

IGameRef CGameStartup::Init(SSysInitParams &startupParams)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Game startup initialisation");

#if defined(CVARS_WHITELIST)
	startupParams.pCVarsWhitelist = &g_CVarsWhiteList;
#endif // defined(CVARS_WHITELIST)
	startupParams.pGameStartup = this;

	if (!InitFramework(startupParams))
	{
		return 0;
	}

	InlineInitializationProcessing("CGameStartup::Init");

  LOADING_TIME_PROFILE_SECTION(m_pFramework->GetISystem());

	// Load thread config
	gEnv->pThreadUpr->GetThreadConfigUpr()->LoadConfig("config/game.thread_config");

	ISystem* pSystem = m_pFramework->GetISystem();
	startupParams.pSystem = pSystem;

	const ICmdLineArg* pSvBind = gEnv->pSystem->GetICmdLine()->FindArg(eCLAT_Pre, "sv_bind"); 
	IConsole* pConsole = pSystem->GetIConsole();
	if ((pSvBind != NULL) && (pConsole != NULL))
	{
		string command = pSvBind->GetName() + string(" ") + pSvBind->GetValue();
		pConsole->ExecuteString(command.c_str(), true, false);
	}

#if defined(ENABLE_STATS_AGENT)
	const ICmdLineArg *pPipeArg = pSystem->GetICmdLine()->FindArg(eCLAT_Pre,"lt_pipename");
	CStatsAgent::CreatePipe( pPipeArg );
#endif

	REGISTER_COMMAND("g_loadMod", CGameStartupStatic::RequestLoadMod,VF_NULL,"");
	REGISTER_COMMAND("g_unloadMod", CGameStartupStatic::RequestUnloadMod, VF_NULL, "");

	// load the appropriate game/mod
#if !defined(_RELEASE)
	const ICmdLineArg *pModArg = pSystem->GetICmdLine()->FindArg(eCLAT_Pre,"MOD");
#else
	const ICmdLineArg *pModArg = NULL;
#endif // !defined(_RELEASE)

	InlineInitializationProcessing("CGameStartup::Init LoadLocalizationData");

	IGameRef pOut;
	if (pModArg && (*pModArg->GetValue() != 0) && (pSystem->IsMODValid(pModArg->GetValue())))
	{
		tukk pModName = pModArg->GetValue();
		assert(pModName);

		pOut = Reset(pModName);
	}
	else
	{
		pOut = Reset(GAME_NAME);
	}

	if (!m_pFramework->CompleteInit())
	{
		pOut->Shutdown();
		return 0;
	}

	InlineInitializationProcessing("CGameStartup::Init FrameworkCompleteInit");

	// should be after init game (should be executed even if there is no game)
	if(startupParams.bExecuteCommandLine)
		pSystem->ExecuteCommandLine();

	pSystem->GetISystemEventDispatcher()->RegisterListener(this);

	// Creates and starts the realtime update system listener.
	if (pSystem->IsDevMode())
	{
		CGameRealtimeRemoteUpdateListener::GetGameRealtimeRemoteUpdateListener().Enable(true);
	}


	GCOV_FLUSH;

	if (ISystem *pSystem = gEnv ? GetISystem() : NULL)
	{
		pSystem->RegisterErrorObserver(&m_errorObsever);
		pSystem->RegisterWindowMessageHandler(this);
	}
	else
	{
		DrxLogAlways("failed to find ISystem to register error observer");
		assert(0);
	}

	
	InlineInitializationProcessing("CGameStartup::Init End");

#if defined(DRX_UNIT_TESTING)
	// Register All unit tests of this module.
#if defined(_LIB)
	if(gEnv->pSystem)
	{
		DrxUnitTest::Test *pTest = DrxUnitTest::Test::m_pFirst; 
		for (; pTest != 0; pTest = pTest->m_pNext)
		{
			DrxUnitTest::IUnitTestUpr *pTestUpr = gEnv->pSystem->GetITestSystem()->GetIUnitTestUpr();
			if (pTestUpr)
			{
				pTest->m_unitTestInfo.module = "StaticBinary";
				pTestUpr->CreateTest( pTest->m_unitTestInfo );
			}
		}
	}
#endif

	// run unit tests
	DrxUnitTest::IUnitTestUpr *pTestUpr =  (gEnv && gEnv->pSystem) ? gEnv->pSystem->GetITestSystem()->GetIUnitTestUpr() : NULL;
	if (pTestUpr)
	{
		const ICmdLineArg* pSkipUnitTest = gEnv->pSystem->GetICmdLine()->FindArg(eCLAT_Pre, "skip_unit_tests"); 
		if(pSkipUnitTest == NULL)
		{
			const ICmdLineArg* pUseUnitTestExcelReporter = gEnv->pSystem->GetICmdLine()->FindArg(eCLAT_Pre, "use_unit_test_excel_reporter"); 
			if(pUseUnitTestExcelReporter)
			{
				gEnv->pSystem->GetITestSystem()->GetIUnitTestUpr()->RunAllTests(DrxUnitTest::ExcelReporter);
			}
			else // default is the minimal reporter
			{
				gEnv->pSystem->GetITestSystem()->GetIUnitTestUpr()->RunAllTests(DrxUnitTest::MinimalReporter);
			}
		}
	}
#endif // DRX_UNIT_TESTING
	
	assert(gEnv);
	PREFAST_ASSUME(gEnv);
	
	GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_RANDOM_SEED, (UINT_PTR)gEnv->pTimer->GetAsyncTime().GetMicroSecondsAsInt64(), 0);
	return pOut;
}

IGameRef CGameStartup::Reset(tukk pModName)
{
	LOADING_TIME_PROFILE_SECTION;
	if (m_pMod)
	{
		m_pMod->Shutdown();

		if (m_modDll)
		{
			DrxFreeLibrary(m_modDll);
			m_modDll = 0;
		}
	}

	m_modDll = 0;
	string modPath;
	
	if (stricmp(pModName, GAME_NAME) != 0)
	{
		modPath.append("Mods\\");
		modPath.append(pModName);
		modPath.append("\\");

		string filename;
		filename.append("..\\");
		filename.append(modPath);
		
#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
		filename.append("Bin64\\");
#else
		filename.append("Bin32\\");
#endif
		
		filename.append(pModName);
		filename.append(".dll");

#if !defined(_LIB) && !DRX_PLATFORM_LINUX && !DRX_PLATFORM_ANDROID
		m_modDll = DrxLoadLibrary(filename.c_str());
#endif
	}

	if (!m_modDll)
	{
		ModuleInitISystem(m_pFramework->GetISystem(),"DrxGame");
		i32k wordSizeCGame = (sizeof(CGame) + sizeof(i32) - 1) / sizeof(i32); // Round up to next word
		static i32 pGameBuffer[wordSizeCGame]; 
		m_pMod = new ((uk )pGameBuffer) CGame();
	}
	else
	{
		IGame::TEntryFunction CreateGame = (IGame::TEntryFunction)DrxGetProcAddress(m_modDll, "CreateGame");
		if (!CreateGame)
			return 0;

		m_pMod = CreateGame(m_pFramework);
	}

	if (m_pMod && m_pMod->Init(m_pFramework))
	{
		return m_modRef;
	}

	return 0;
}

void CGameStartup::Shutdown()
{
#if DRX_PLATFORM_WINDOWS
	AllowAccessibilityShortcutKeys(true);
#endif

#if defined(ENABLE_STATS_AGENT)
	CStatsAgent::ClosePipe();
#endif

	if (ISystem *pSystem = GetISystem())
	{
		pSystem->UnregisterErrorObserver(&m_errorObsever);
		pSystem->UnregisterWindowMessageHandler(this);
	}

	/*delete this;*/
	this->~CGameStartup();
}

i32 CGameStartup::Update(bool haveFocus, u32 updateFlags)
{
	// The frame profile system already creates an "overhead" profile label
	// in StartFrame(). Hence we have to set the FRAMESTART before.
	DRX_PROFILE_FRAMESTART("Main");

#if defined(JOBMANAGER_SUPPORT_PROFILING)
	gEnv->GetJobUpr()->SetFrameStartTime(gEnv->pTimer->GetAsyncTime());
#endif

	i32 returnCode = 0;

	if (gEnv && gEnv->pSystem && gEnv->pConsole)
	{
#if DRX_PLATFORM_WINDOWS
		if(gEnv && gEnv->pRenderer && gEnv->pRenderer->GetHWND())
		{
			bool focus = (::GetFocus() == gEnv->pRenderer->GetHWND());
			static bool focused = focus;
			if (focus != focused)
			{
				if(gEnv->pSystem->GetISystemEventDispatcher())
				{
					gEnv->pSystem->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_CHANGE_FOCUS, focus, 0);
				}
				focused = focus;
			}
		}
#endif
	}

	// update the game
	if (m_pMod)
	{
		returnCode = m_pMod->Update(haveFocus, updateFlags);
	}

#if defined(ENABLE_STATS_AGENT)
	CStatsAgent::Update();
#endif

#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)

	// Update Backend profilers
	u32 timeSample = JobUpr::IWorkerBackEndProfiler::GetTimeSample();

	assert(gEnv);
	PREFAST_ASSUME(gEnv);
	
	const JobUpr::IBackend * const __restrict pBackends[] = 
	{
		gEnv->GetJobUpr()->GetBackEnd(JobUpr::eBET_Thread),
		gEnv->GetJobUpr()->GetBackEnd(JobUpr::eBET_Blocking),
	};

	for(i32 i=0; i<DRX_ARRAY_COUNT(pBackends); ++i)
	{
		if(pBackends[i])
		{
			JobUpr::IWorkerBackEndProfiler* pWorkerProfiler = pBackends[i]->GetBackEndWorkerProfiler();
			pWorkerProfiler->Update(timeSample);
		}
	}
#endif

	// ghetto fullscreen detection, because renderer does not provide any kind of listener
	if (!m_fullScreenCVarSetup && gEnv && gEnv->pSystem && gEnv->pConsole)
	{
		ICVar *pVar = gEnv->pConsole->GetCVar("r_Fullscreen");
		if (pVar)
		{
			pVar->SetOnChangeCallback(FullScreenCVarChanged);
			m_fullScreenCVarSetup = true;
		}
	}
#if ENABLE_AUTO_TESTER 
	s_autoTesterSingleton.Update();
#endif
	GCOV_FLUSH_UPDATE;

	return returnCode;
}

void CGameStartup::FullScreenCVarChanged( ICVar *pVar )
{
	if(gEnv->pSystem->GetISystemEventDispatcher())
	{
		gEnv->pSystem->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_TOGGLE_FULLSCREEN, pVar->GetIVal(), 0);
	}
}

bool CGameStartup::GetRestartLevel(tuk* levelName)
{
	if(GetISystem()->IsRelaunch())
		*levelName = (tuk)(gEnv->pGame->GetIGameFramework()->GetLevelName());
	return GetISystem()->IsRelaunch();
}

bool CGameStartup::GetRestartMod(tuk pModNameBuffer, i32 modNameBufferSizeInBytes)
{
	if (m_reqModUnload)
	{
		if (modNameBufferSizeInBytes > 0)
			pModNameBuffer[0] = 0;
		return true;
	}

	if (m_reqModName.empty())
		return false;

	drx_strcpy(pModNameBuffer, modNameBufferSizeInBytes, m_reqModName.c_str());
	return true;
}

tukk CGameStartup::GetPatch() const
{
	// michiel - GS
	return NULL;	
}

#if defined(RELEASE_SERVER_SECURITY)

#define TMP_CONFIG_LINE_BUFFER			(2048)
#define FILE_CHECK_BUFFER_SIZE			(32768)

#include <drx3D/Sys/ZLib/IZLibCompressor.h>

bool ValidateFile(string path,string md5)
{
	bool bOk;
	// Try to open file on disk and hash it.
	FILE *file = fopen( path.c_str(),"rb" );
	if (file)
	{
		fseek( file,0,SEEK_END );
    u32 nFileSize = ftell(file);
		fseek( file,0,SEEK_SET );

		u8 *pBuf = (u8*)malloc( FILE_CHECK_BUFFER_SIZE );
		if (!pBuf)
		{
			fclose(file);
			return false;
		}
																
		IZLibCompressor			*pZLib=GetISystem()->GetIZLibCompressor();

		char digest[16];
		SMD5Context context;
		string digestAsString;
		
		pZLib->MD5Init(&context);

		while (nFileSize)
		{
			u32 fetchLength=min(nFileSize,(u32)FILE_CHECK_BUFFER_SIZE);

			if (fread( pBuf,fetchLength,1,file ) != 1)
			{
				free( pBuf );
				fclose(file);
				return false;
			}
			
			pZLib->MD5Update(&context,(tukk)pBuf,fetchLength);

			nFileSize-=fetchLength;
		}

		pZLib->MD5Final(&context,digest);

		digestAsString="";
		for (i32 a=0;a<16;a++)
		{
			string hex;
			hex.Format("%02x",(u8)digest[a]);
			digestAsString.append(hex);
		}

		bOk = (digestAsString.compare(md5) == 0 );

		free( pBuf );
		fclose(file);

		return bOk;
	}

	return false;
}

bool PerformDedicatedInstallationSanityCheck()
{
	char tmpLineBuffer[TMP_CONFIG_LINE_BUFFER];
	string checksumPath = PathUtil::GetGameFolder() + "/Scripts/DedicatedConfigs/" + "check.txt";
	bool bOk=true;
	
	FILE *checksumFile = fopen(checksumPath.c_str(),"r");

	if (!checksumFile)
	{
		DrxLogAlways("Failed to open validation configuration - Please check your install!");
		return false;
	}
	else
	{
		while (fgets(tmpLineBuffer,TMP_CONFIG_LINE_BUFFER-1,checksumFile))
		{
			string temp = tmpLineBuffer;
				
			string::size_type posEq = temp.find( "*", 0 );
			
			if (string::npos!=posEq)
			{
					string md5( temp, 0, posEq );
					string path( temp,posEq+1,temp.length());

					path.TrimRight(" \r\n");
					md5.TrimRight(" \r\n");

					if (!ValidateFile(path,md5))
					{
						DrxLogAlways(path + " failed validation check - Please check your install! ("+md5+")");
						bOk=false;
					}
			}
		}

		fclose(checksumFile);
	}

	return bOk;
}
#endif

i32 CGameStartup::Run( tukk  autoStartLevelName )
{
#if	defined(RELEASE_SERVER_SECURITY)
	DrxLogAlways("Performing Validation Checks");
	if (!PerformDedicatedInstallationSanityCheck())
	{
		DrxFatalError("Installation appears to be corrupt. Please check the log file for a list of problems.");
	}
#endif
	gEnv->pConsole->ExecuteString( "exec autoexec.cfg" );
	if (autoStartLevelName)
	{
		//load savegame
		if(DrxStringUtils::stristr(autoStartLevelName, DRX_SAVEGAME_FILE_EXT) != 0 )
		{
			DrxFixedStringT<256> fileName (autoStartLevelName);
			// NOTE! two step trimming is intended!
			fileName.Trim(" ");  // first:  remove enclosing spaces (outside ")
			fileName.Trim("\""); // second: remove potential enclosing "
			gEnv->pGame->GetIGameFramework()->LoadGame(fileName.c_str());
		}
		else	//start specified level
		{
			DrxFixedStringT<256> mapCmd ("map ");
			mapCmd+=autoStartLevelName;
			gEnv->pConsole->ExecuteString(mapCmd.c_str());
		}
	}

#if DRX_PLATFORM_WINDOWS
	if (!(gEnv && gEnv->pSystem) || (!gEnv->IsEditor() && !gEnv->IsDedicated()))
	{
		gEnv->pInput->ShowCursor(true); // Make the cursor visible again (it was hidden in InitFramework() )
		if (gEnv && gEnv->pSystem && gEnv->pSystem->GetIHardwareMouse())
			gEnv->pSystem->GetIHardwareMouse()->DecrementCounter();
	}

	AllowAccessibilityShortcutKeys(false);

	for(;;)
	{
		ISystem *pSystem = gEnv ? gEnv->pSystem : 0;
		if (!pSystem)
		{
			break;
		}
		
		if (pSystem->PumpWindowMessage(false) == -1)
		{
			break;
		}

		if (!Update(true, 0))
		{
			// need to clean the message loop (WM_QUIT might cause problems in the case of a restart)
			// another message loop might have WM_QUIT already so we cannot rely only on this
			pSystem->PumpWindowMessage(true);
			break;
		}
	}
#else
	// We should use bVisibleByDefault=false then...
	if (gEnv && gEnv->pHardwareMouse)
		gEnv->pHardwareMouse->DecrementCounter();

#if !DRX_PLATFORM_DURANGO
	for(;;)
	{
		if (!Update(true, 0))
		{
			break;
		}
	}
#endif

#endif // DRX_PLATFORM_WINDOWS

	return 0;
}

// If you have a valid RSA key set it here and set USE_RSA_KEY to 1
#ifndef IS_EAAS
#define USE_RSA_KEY 0
#if USE_RSA_KEY
static u8 g_rsa_public_key_data[] =
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
#endif
#else
// For EaaS, a key is required in release mode
// However, we set it in other configurations as well, so that signed PAK files can be loaded
// This is a default key that is used by DrxTek to sign the EaaS PAK files, you need to replace it with your own
#define USE_RSA_KEY 1
static u8 g_rsa_public_key_data[] =
{
	0x30,0x81,0x89,0x02,0x81,0x81,0x00,0xC5,0x66,0xA7,0xE5,0x21,0x0B,0x25,0x54,0xCC,0x29,0x53,0xD9,0x2F,
	0x87,0xE8,0x8D,0x1E,0x03,0xE7,0x03,0x29,0x08,0x89,0xDF,0xC3,0x88,0xFA,0xA1,0x20,0x69,0x1B,0xD0,0xE6,
	0x09,0xC0,0xB1,0x81,0x13,0xFD,0x9D,0x2C,0x1F,0xC2,0x3B,0x10,0xA3,0x19,0xF4,0xA5,0xB2,0xBE,0x63,0xAD,
	0x76,0xD7,0xEB,0x6D,0x32,0xAA,0x3D,0xC1,0xE3,0x00,0x88,0x2E,0x5A,0x11,0xE8,0xD6,0x88,0xF0,0xA3,0x35,
	0xF4,0xB8,0x89,0xFE,0xDB,0x3E,0x0A,0x75,0x75,0x00,0x3D,0x4A,0xA3,0xB2,0xC6,0x27,0x60,0x05,0x90,0x7A,
	0x25,0x0E,0x45,0x32,0x1E,0xD7,0xE3,0x3B,0x50,0x17,0xE1,0xC0,0xAC,0xA6,0x8F,0xA5,0x54,0x8B,0x63,0x4E,
	0x05,0x93,0x4F,0x64,0xB4,0x35,0x52,0xE5,0x8C,0xD5,0xC0,0x7C,0x9A,0x69,0x0B,0x02,0x03,0x01,0x00,0x01
};
#endif

u8k* CGameStartup::GetRSAKey(u32 *pKeySize) const
{
#if USE_RSA_KEY
	*pKeySize = sizeof(g_rsa_public_key_data);
	return g_rsa_public_key_data;
#else
	*pKeySize = 0;
	return NULL;
#endif
}

void CGameStartup::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_RANDOM_SEED:
		drx_random_seed(gEnv->bNoRandomSeed ? 0 : (u32)wparam);
		break;
	case ESYSTEM_EVENT_CHANGE_FOCUS:
		{
			#if DRX_PLATFORM_WINDOWS

			AllowAccessibilityShortcutKeys(wparam==0);
			
			#endif
			GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_RANDOM_SEED, (UINT_PTR)gEnv->pTimer->GetAsyncTime().GetMicroSecondsAsInt64(), 0);
		}
		break;
	case ESYSTEM_EVENT_LEVEL_LOAD_START:
		{			
			// For MP gamemodes set the correct sound parameter
			// Default to SP
			float sp_coop_mp = 0.0f;
			if ( gEnv->bMultiplayer )
			{
				sp_coop_mp = 2.0f;
			}
			//gEnv->pSoundSystem->SetGlobalParameter( "sp_coop_mp", sp_coop_mp );
			DrxLog("sp_coop_mp set to %f", sp_coop_mp);
		}
		break;

	case ESYSTEM_EVENT_FAST_SHUTDOWN:
		m_quit = true;
		break;
	}
}

bool CGameStartup::InitFramework(SSysInitParams &startupParams)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Init Game Framework" );

#if !defined(_LIB)
	m_frameworkDll = GetFrameworkDLL(startupParams.szBinariesDir);

	if (!m_frameworkDll)
	{
		// failed to open the framework dll
		DrxFatalError("Failed to open the GameFramework DLL!");
		
		return false;
	}

	IGameFramework::TEntryFunction CreateGameFramework = (IGameFramework::TEntryFunction)DrxGetProcAddress(m_frameworkDll, DLL_INITFUNC_CREATEGAME );

	if (!CreateGameFramework)
	{
		// the dll is not a framework dll
		DrxFatalError("Specified GameFramework DLL is not valid!");

		return false;
	}
#endif //_LIB

	m_pFramework = CreateGameFramework();

	if (!m_pFramework)
	{
		DrxFatalError("Failed to create the GameFramework Interface!");
		// failed to create the framework

		return false;
	}

#if DRX_PLATFORM_WINDOWS
	if (startupParams.pSystem == NULL || (!startupParams.bEditor && !gEnv->IsDedicated()))
	{
		// Hide the cursor during loading (it will be shown again in Run())
		// gEnv is no initialized yet, so we can't use gEnv->pInput->ShowCursor
		::ShowCursor(FALSE);
	}
#endif

	// initialize the engine
	if (!m_pFramework->Init(startupParams))
	{
		DrxFatalError("Failed to initialize DrxENGINE!");
		return false;
	}
	ModuleInitISystem(m_pFramework->GetISystem(),"DrxGame");

#if DRX_PLATFORM_WINDOWS
	if (gEnv->pRenderer)
	{
		SetWindowLongPtr(reinterpret_cast<HWND>(gEnv->pRenderer->GetHWND()), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}
#endif
	return true;
}

void CGameStartup::ShutdownFramework()
{
	if (m_pFramework)
	{
		m_pFramework->Shutdown();
		m_pFramework = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
#if DRX_PLATFORM_WINDOWS

bool CGameStartup::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	switch(msg)
	{
	case WM_SYSCHAR: // Prevent ALT + key combinations from creating 'ding' sounds
		{
			const bool bAlt = (lParam & (1 << 29)) != 0;
			if (bAlt && wParam == VK_F4)
			{
				return false; // Pass though ALT+F4
			}

			*pResult = 0;
			return true;
		}
		break;

	case WM_SIZE:
		HandleResizeForVOIP(wParam);
		break;

	case WM_SETFOCUS:
		if (g_pGameCVars)
		{
			g_pGameCVars->g_hasWindowFocus = true;
		}
		break;

	case WM_KILLFOCUS:
		if (g_pGameCVars)
		{
			g_pGameCVars->g_hasWindowFocus = false;
		}
		break;

	case WM_SETCURSOR:
		{
			// This is sample code to change the displayed cursor for Windows applications.
			// Note that this sample loads a texture (ie, .TIF or .DDS), not a .ICO or .CUR resource.
			IHardwareMouse* const pMouse = gEnv ? gEnv->pHardwareMouse : NULL;
			assert(pMouse && "HWMouse should be initialized before window is shown, check engine initialization order");
			tukk currentCursorPath = gEnv->pConsole->GetCVar("r_MouseCursorTexture")->GetString();
			const bool bResult = pMouse ? pMouse->SetCursor(currentCursorPath) : false;
			if (!bResult)
			{
				GameWarning("Unable to load cursor %s, does this file exist?", currentCursorPath);
			}
			*pResult = bResult ? TRUE : FALSE;
			return bResult;
		}
		break;
	}
	return false;
}


void CGameStartup::HandleResizeForVOIP(WPARAM wparam)
{
	if(gEnv->pConsole)
	{
		ICVar * pVOIPCvar = gEnv->pConsole->GetCVar("net_blaze_voip_enable");

		if(pVOIPCvar)
		{
			if(wparam == SIZE_RESTORED || wparam == SIZE_MAXIMIZED)
			{
				i32 currentVal = pVOIPCvar->GetIVal();
				if(m_nVOIPWasActive == -1)
				{
					m_nVOIPWasActive = currentVal;
				}
				if(m_nVOIPWasActive != currentVal)
				{
					pVOIPCvar->Set(m_nVOIPWasActive);
				}
				DrxLog("[VOIP] Game maximized or restored, VOIP was set to %d, saved value %d - now restored", currentVal, m_nVOIPWasActive);
			}
			else if(wparam == SIZE_MINIMIZED)
			{
				m_nVOIPWasActive = pVOIPCvar->GetIVal();
				pVOIPCvar->Set(0);
				DrxLog("[VOIP] Game minimized, VOIP was set to %d, setting to 0 while minimized", m_nVOIPWasActive);
			}
		}
	}	
}

//////////////////////////////////////////////////////////////////////////
#endif // DRX_PLATFORM_WINDOWS

CGameStartup* CGameStartupStatic::g_pGameStartup = NULL;

void CGameStartupStatic::RequestLoadMod(IConsoleCmdArgs* pCmdArgs)
{
	if (pCmdArgs->GetArgCount() == 2)
	{
		if (g_pGameStartup) 
		{	
			g_pGameStartup->m_reqModName = pCmdArgs->GetArg(1);
			ISystem* pSystem = g_pGameStartup->m_pFramework->GetISystem();
			pSystem->Quit();
		}
	}
	else
	{
		DrxLog("Error, correct syntax is: g_loadMod modname");
	}
}

void CGameStartupStatic::RequestUnloadMod(IConsoleCmdArgs* pCmdArgs)
{
	if (pCmdArgs->GetArgCount() == 1)
	{
		if (g_pGameStartup) 
		{	
			g_pGameStartup->m_reqModUnload = true;
			ISystem* pSystem = g_pGameStartup->m_pFramework->GetISystem();
			pSystem->Quit();
		}
	}
	else
	{
		DrxLog("Error, correct syntax is: g_unloadMod");
	}
}

void CGameStartupStatic::ForceCursorUpdate()
{
#if DRX_PLATFORM_WINDOWS
	if(gEnv && gEnv->pRenderer && gEnv->pRenderer->GetHWND())
	{
		SendMessage(HWND(gEnv->pRenderer->GetHWND()),WM_SETCURSOR,0,0);
	}
#endif
}
//--------------------------------------------------------------------------



