// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$

 -------------------------------------------------------------------------
  История:
  - 3:8:2004   11:26 : Created by Márcio Martins
  - 17:8:2005        : Modified - NickH: Factory registration moved to GameFactory.cpp

*************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Game.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/Game/GameActions.h>

#include <drx3D/Game/UI/UIUpr.h>
#include <drx3D/Game/UI/ProfileOptions.h>
#include <drx3D/Game/UI/WarningsUpr.h>
#include <drx3D/Game/UI/Menu3dModels/FrontEndModelCache.h>
#include <drx3D/Game/UI/HUD/HUDEventDispatcher.h>
#include <drx3D/Game/UI/UICVars.h>
#include <drx3D/Game/UI/HUD/HUDMissionObjectiveSystem.h>
#include <drx3D/Game/UI/HUD/HUDEventWrapper.h>
#include <drx3D/Game/UI/Utils/ScreenLayoutUpr.h>
#include <drx3D/Game/UI/UIInput.h>
#include <drx3D/Game/ScreenResolution.h>

#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/Audio/GameAudio.h>
#include <drx3D/Game/ScreenEffects.h>
#include <drx3D/Game/WeaponSystem.h>
#include <drx3D/Game/Effects/GameEffectsSystem.h>
#include <drx3D/Game/GameInputActionHandlers.h>
#include <drx3D/Game/GameCache.h>
#include <drx3D/Game/ItemScheduler.h>
#include <drx3D/Game/Utility/DrxWatch.h>

#include <drx3D/Sys/File/IDrxPak.h>
#include <drx3D/CoreX/String/DrxPath.h>
#include <IActionMapUpr.h>
#include <drx3D/Act/IViewSystem.h>
#include <ILevelSystem.h>
#include <drx3D/Act/IItemSystem.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Movie/IMovieSystem.h>
#include <IPlayerProfiles.h>
#include <drx3D/CoreX/Lobby/IDrxLobbyUI.h>
#include <drx3D/Sys/ILocalizationUpr.h>
//#include <drx3D/Entity/IEntityPoolUpr.h>
#include <drx3D/Sys/File/IResourceUpr.h>
#include <drx3D/Act/ICustomActions.h>

#include <drx3D/Game/ScriptBind_Actor.h>
#include <drx3D/Game/ScriptBind_Item.h>
#include <drx3D/Game/ScriptBind_Weapon.h>
#include <drx3D/Game/ScriptBind_GameRules.h>
#include <drx3D/Game/ScriptBind_Game.h>
#include <drx3D/Game/ScriptBind_HitDeathReactions.h>
#include <drx3D/Game/Boids/ScriptBind_Boids.h>
#include <drx3D/Game/AI/ScriptBind_GameAI.h>
#include <drx3D/Game/UI/HUD/ScriptBind_HUD.h>
#include <drx3D/Game/Environment/ScriptBind_InteractiveObject.h>
#include <drx3D/Game/Network/Lobby/ScriptBind_MatchMaking.h>
#include <drx3D/Game/Turret/Turret/ScriptBind_Turret.h>
#include <drx3D/Game/ScriptBind_ProtectedBinds.h>
#include <drx3D/Game/Environment/ScriptBind_LightningArc.h>
#include <drx3D/Game/DLCUpr.h>
#include <drx3D/Game/CornerSmoother.h>
#include <drx3D/Input/IHardwareMouse.h>

#ifdef USE_LAPTOPUTIL
#include <drx3D/Game/LaptopUtil.h>
#endif

#include <drx3D/Game/GameFactory.h>

#include <drx3D/Game/Player.h>

#include <drx3D/Game/GameParameters.h>
#include <drx3D/Game/WeaponSharedParams.h>
#include <drx3D/Game/ItemResourceCache.h>
#include <drx3D/Game/ActorUpr.h>

#include <drx3D/Game/Nodes/G2FlowBaseNode.h>

#include <drx3D/Game/ISaveGame.h>
#include <drx3D/Game/ILoadGame.h>
#include <drx3D/CoreX/String/DrxPath.h>
#include <drx3D/Game/GameStateRecorder.h>
#include <drx3D/Game/GodMode.h>
#include <drx3D/Game/SmokeUpr.h>
#include <drx3D/Game/RecordingSystem.h>
#include <drx3D/Game/StatsRecordingMgr.h>
#include <drx3D/Game/PatchPakUpr.h>
#include <drx3D/Game/DataPatchDownloader.h>
#include <drx3D/Game/LagOMeter.h>
#include <drx3D/Game/TelemetryCollector.h>
#include <drx3D/Game/TacticalUpr.h>

#include <drx3D/Game/Environment/LedgeUpr.h>
#include <drx3D/Game/Environment/WaterPuddle.h>

#include <drx3D/Game/Graphics/ColorGradientUpr.h>
#include <drx3D/Game/VehicleClient.h>
#include <drx3D/Game/AI/TacticalPointLanguageExtender.h>
#include <drx3D/Game/AI/GameAISystem.h>
#include <drx3D/Game/AI/GameAIEnv.h>
#include <drx3D/Game/AI/AICorpse.h>

#include <drx3D/Game/Network/Lobby/GameUserPackets.h>
#include <drx3D/Game/Network/Lobby/GameAchievements.h>
#include <drx3D/Game/Network/Lobby/GameBrowser.h>
#include <drx3D/Game/Network/Lobby/GameLobbyUpr.h>
#include <drx3D/Game/Network/Lobby/DrxLobbySessionHandler.h>
#include <drx3D/Game/Network/Lobby/GameServerLists.h>
#include <drx3D/Game/Network/Squad/SquadUpr.h>
#include <drx3D/Game/Network/Lobby/PlaylistActivityTracker.h>

#include <drx3D/Game/Utility/ManualFrameStep.h>

#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageUpr.h>
#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageGUI.h>

#include <drx3D/Game/CodeCheckpointDebugMgr.h>

#include <drx3D/Game/AutoAimUpr.h>
#include <drx3D/Game/HitDeathReactionsSystem.h>
#include <drx3D/Game/MovementTransitionsSystem.h>
#include <drx3D/Game/CheckpointGame.h>
#include <drx3D/Game/BodyUpr.h>
#include <drx3D/Game/StealthKill.h>
#include <drx3D/Game/VehicleMovementBase.h>

#include <drx3D/Game/PlayerProgression.h>
#include <drx3D/Game/PersistantStats.h>
#include <drx3D/Game/Testing/FeatureTester.h>
#include <drx3D/Game/GamePhysicsSettings.h>

#include <drx3D/Game/EquipmentLoadout.h>
#include <drx3D/Game/UI/Menu3dModels/MenuRender3DModelMgr.h>

#include <drx3D/Game/MikeBullet.h>
#include <drx3D/Game/PlaylistUpr.h>
#include <drx3D/Game/DownloadMgr.h>
#include <drx3D/Game/Effects/HudInterferenceGameEffect.h>
#include <drx3D/Game/Effects/SceneBlurGameEffect.h>
#include <drx3D/Game/Effects/LightningGameEffect.h>
#include <drx3D/Game/Effects/ParameterGameEffect.h>

#include <drx3D/Game/Stereo3D/StereoFramework.h>

#include <drx3D/Game/GameMechanismUpr.h>
#include <drx3D/CoreX/Platform/IPlatformOS.h>

#if !defined(_RELEASE)
#include <drx3D/Game/Editor/GameRealtimeRemoteUpdate.h>
#endif

#include <drx3D/Game/GodMode.h>
#include <drx3D/Game/PlayerVisTable.h>
#include <drx3D/Game/Network/GameNetworkUtils.h>

#include <drx3D/Game/SlideController.h>

#include <drx3D/Sys/IStatoscope.h>

#include <drx3D/Game/GameLocalizationUpr.h>

#include <drx3D/Game/RevertibleConfigLoader.h>

#include <drx3D/Game/StatsEntityIdRegistry.h>

#include <drx3D/Game/MovingPlatforms/MovingPlatformMgr.h>

#include <drx3D/CoreX/Game/IGameVolumes.h>

#include <DrxLiveCreate/ILiveCreateHost.h>
#include <drx3D/CoreX/SFunctor.h>

#include <drx3D/Game/WorldBuilder.h>

#include <drx3D/DynRespSys/IDynamicResponseSystem.h>
#include <drx3D/Game/DynamicResponseSystem/ConditionDistanceToEntity.h>
#include <drx3D/Game/DynamicResponseSystem/GameTokenToDrsTranslator.h>
#include <drx3D/Game/DynamicResponseSystem/ActionExecuteAudioTrigger.h>
#include <drx3D/Game/DynamicResponseSystem/ActionSpeakLineBasedOnVariable.h>
#include <drx3D/CoreX/Serialization/ClassFactory.h>

#include <drx3D/Sys/FrameProfiler.h>
#include <drx3D/CoreX/Sandbox/IEditorGame.h>

//#define GAME_DEBUG_MEM  // debug memory usage
#undef  GAME_DEBUG_MEM

#define GAME_ITEMS_DATA_FOLDER "scripts/entities/items/XML"

#define DRXENGINE_SDK_GUID "{D860B77B-D6AB-4FB5-BBE3-84EE6B98D6DA}"

#define PRODUCT_VERSION_MAX_STRING_LENGTH (256)

#if DRX_PLATFORM_DURANGO
#include <drx3D/Game/XboxOneLive/XboxLiveGameEvents.h>
#include <drx3D/Game/Network/MatchMakingUtils.h>
#endif

#ifndef _LIB
	#include <drx3D/CoreX/Common_TypeInfo.h>
//STRUCT_INFO_T_INSTANTIATE(Vec3_tpl, <float>)
	STRUCT_INFO_T_INSTANTIATE(Quat_tpl, <float>)
	#if DRX_PLATFORM_ANDROID
	STRUCT_INFO_T_INSTANTIATE(QuatT_tpl, <float>)
	#endif
#endif

// Needed for the Game02 specific flow node
CG2AutoRegFlowNodeBase *CG2AutoRegFlowNodeBase::m_pFirst=0;
CG2AutoRegFlowNodeBase *CG2AutoRegFlowNodeBase::m_pLast=0;

#if DRX_PLATFORM_WINDOWS
	#define		DISABLE_FORCE_FEEDBACK_WHEN_USING_MOUSE_AND_KEYBOARD	1
#else
	#define		DISABLE_FORCE_FEEDBACK_WHEN_USING_MOUSE_AND_KEYBOARD	0
#endif

CGame *g_pGame = 0;
SCVars *g_pGameCVars = 0;
CGameActions *g_pGameActions = 0;
CTacticalPointLanguageExtender g_tacticalPointLanguageExtender;

static CRevertibleConfigLoader s_gameModeCVars(96, 5120);	// 5k - needs to hold enough room for patched cvars as well as multiplayer.cfg
static CRevertibleConfigLoader s_levelCVars(20, 1024);

static bool s_usingGlobalHeap = true;

static void OnChangedStereoRenderDevice(ICVar*	pStereoRenderDevice);

#ifndef NO_LIVECREATE
void LiveCreateUpdate()
{
	if (!gEnv)
	{
		return;
	}

	if (gEnv->IsEditor())
	{
		return;
	}

	if (gEnv->pLiveCreateHost)
	{
		gEnv->pLiveCreateHost->ExecuteCommands();
	}
}
#endif

CCountdownTimer::CCountdownTimer(FN_COMPLETION_CALLBACK fnComplete):
	m_fnCompleted(fnComplete),
	m_timeLeft(0.0f)
{
}

void CCountdownTimer::Start(float countdownDurationSeconds)
{
	m_timeLeft = countdownDurationSeconds;
}

void CCountdownTimer::Advance(float dt)
{
	if (m_timeLeft > 0.0f)
	{
		m_timeLeft -= dt;

		if (m_timeLeft <= 0.0f)
		{
			if (m_fnCompleted != NULL)
			{
				m_fnCompleted();
			}
		}
	}
}

bool CCountdownTimer::IsActive()
{
	return m_timeLeft > 0.0f;
}

void CCountdownTimer::Abort()
{
	m_timeLeft = 0.0f;
}

float CCountdownTimer::ToSeconds()
{
	return m_timeLeft;
}

#if ENABLE_VISUAL_DEBUG_PROTOTYPE

class CVisualDebugPrototype
{
	std::vector<ITexture*> m_Textures;

	struct DrawCallData
	{
		u32	lastDrawCallCount;
		float		redValue;
		float		lastAlpha;
		bool		up;

		DrawCallData() : lastDrawCallCount(0), redValue(0.0f), lastAlpha(0.0f), up(false)
		{}
	};

	i32						m_Enabled;
	DrawCallData	m_DrawCallData;

public:

	CVisualDebugPrototype()
	{
		REGISTER_CVAR2("g_VisualDebugEnable", &m_Enabled, 1, VF_CHEAT, "Set AI features to behave in earlier milestones - please use sparingly");

		ITexture *pTex = gEnv->pRenderer->EF_LoadTexture( "Textures/GVDS/TooMany.tif", FT_DONT_STREAM );
		if (pTex)
		{
			m_Textures.push_back(pTex);
		}

		pTex = gEnv->pRenderer->EF_LoadTexture( "Textures/GVDS/TooMany2.tif", FT_DONT_STREAM );
		if (pTex)
		{
			m_Textures.push_back(pTex);
		}

	}

	~CVisualDebugPrototype()
	{
		for (std::vector<ITexture*>::iterator it = m_Textures.begin(); m_Textures.end() != it; ++it)
		{
			if (*it)
			{
				(*(it))->Release();
			}
		}

		m_Textures.clear();
	}

	void Update(float deltaT)
	{
		if (m_Enabled != 4674)
		{
			if (gEnv->IsEditor())
			{
				CheckDrawCalls(deltaT);
			}
		}
	}


	void CheckDrawCalls(float deltaT)
	{
		if (m_Textures.size() < 2 || !m_Textures[0] || !m_Textures[1])
		{
			return;
		}

		i32 nDrawCalls = gEnv->pRenderer->GetCurrentNumberOfDrawCalls();

		if (3000 < nDrawCalls)
		{
			gEnv->pRenderer->SetColorOp(eCO_MODULATE,eCO_MODULATE,DEF_TEXARG0,DEF_TEXARG0);
			gEnv->pRenderer->SetState(GS_BLSRC_SRCALPHA|GS_BLDST_ONEMINUSSRCALPHA|GS_NODEPTHTEST);

			float alpha = max(0.0f, min(1.0f, (nDrawCalls - 3000) / 2000.0f));
			float reddy	= max(0.0f, min(1.0f, (nDrawCalls - 4000) / 3000.0f));

			m_DrawCallData.lastAlpha = (m_DrawCallData.lastAlpha + alpha) * 0.5f;
			//m_DrawCallData.lastAlpha = m_DrawCallData.lastAlpha * 0.9f;



			if (reddy > m_DrawCallData.redValue && m_DrawCallData.up)
			{
				m_DrawCallData.redValue += (reddy - m_DrawCallData.redValue) * deltaT;

				if (reddy < m_DrawCallData.redValue)
				{
					m_DrawCallData.up = false;
				}
				else if (m_DrawCallData.redValue > 0.3f)
				{
					m_DrawCallData.redValue = 0.3f;
					m_DrawCallData.up = false;
				}
			}
			else if (!m_DrawCallData.up)
			{
				m_DrawCallData.redValue -= 0.1f * deltaT;

				if (m_DrawCallData.redValue < 0.0f)
				{
					m_DrawCallData.redValue = 0.0f;
					m_DrawCallData.up = true;
				}
			}
			else
			{
				m_DrawCallData.up = false;
			}

			float w = static_cast<float>(gEnv->pRenderer->GetWidth());
			float h = static_cast<float>(gEnv->pRenderer->GetHeight());

			float scalef = .4f;
			//x = w * 0.25f - m_Textures[0]->GetWidth() * scalef * 0.5f;
			//y = h * 0.5f - m_Textures[0]->GetHeight() * scalef * 0.5f;

			gEnv->pRenderer->Draw2dImage(0.f, 200.f, m_Textures[0]->GetWidth() * scalef, m_Textures[0]->GetHeight() * scalef, m_Textures[0]->GetTextureID(), 0.0f, 0.0f, 1.0f, 1.0f, 180.0f, 1.0f, 1.0f - m_DrawCallData.redValue, 1.0f - m_DrawCallData.redValue, m_DrawCallData.lastAlpha, 0.9f);
			gEnv->pRenderer->Draw2dImage(0.f, 200.f, m_Textures[1]->GetWidth() * scalef, m_Textures[1]->GetHeight() * scalef, m_Textures[1]->GetTextureID(), 0.0f, 0.0f, 1.0f, 1.0f, 180.0f, 1.0f, 1.0f, 1.0f, m_DrawCallData.redValue, 1.0f);
		}
		else
		{
			m_DrawCallData.redValue = 0.0f;
		}
	}
};

#endif // ENABLE_VISUAL_DEBUG_PROTOTYPE

CGame::CGame()
: m_pFramework(0),
	m_pConsole(0),
	m_pWeaponSystem(0),
	m_pGamePhysicsSettings(0),
	m_pScriptBindActor(0),
	m_pScriptBindGame(0),
	m_pScriptBindHitDeathReactions(0),
	m_pScriptBindBoids(0),
	m_pScriptBindTurret(0),
	m_pPlayerProfileUpr(0),
	m_pGameAudio(0),
	m_pScreenEffects(0),
	m_clientActorId(-1),
	m_pPersistantStats(0),
#ifdef USE_LAPTOPUTIL
	m_pLaptopUtil(0),
#endif
	m_pLedgeUpr(0),
	m_pWaterPuddleUpr(0),
	m_colorGradientUpr(0),
	m_pRecordingSystem(0),
	m_pEquipmentLoadout(0),
	m_pPlayerProgression(0),
	m_statsRecorder(0),
	m_patchPakUpr(NULL),
	m_pMatchMakingTelemetry(NULL),
	m_pDataPatchDownloader(0),
	m_pGameLocalizationUpr(0),
#if USE_LAGOMETER
	m_pLagOMeter(0),
#endif
	m_telemetryCollector(0),
	m_pPlaylistActivityTracker(0),
#if USE_TELEMETRY_BUFFERS
	m_performanceBuffer(0),
	m_bandwidthBuffer(0),
	m_memoryTrackingBuffer(0),
	m_soundTrackingBuffer(0),
	m_secondTimePerformance((int64)0),
	m_secondTimeMemory((int64)0),
	m_secondTimeBandwidth((int64)0),
	m_secondTimeSound((int64)0),
#endif //#if USE_TELEMETRY_BUFFERS
	m_pGameBrowser(0),
	m_pGameLobbyUpr(0),
#if IMPLEMENT_PC_BLADES
	m_pGameServerLists(NULL),
#endif // IMPLEMENT_PC_BLADES
	m_pSquadUpr(0),
	m_pLobbySessionHandler(0),
	m_pUIUpr(NULL),
	m_pTacticalUpr(NULL),
	m_pAutoAimUpr(NULL),
	m_pHitDeathReactionsSystem(NULL),
	m_pBodyDamageUpr(NULL),
	m_pPlaylistUpr(NULL),
	m_pMovementTransitionsSystem(NULL),
	m_pScriptBindInteractiveObject(NULL),
	m_previousInputControllerDeviceIndex(0),
	m_currentXboxLivePartySize(0),
	m_hasExclusiveController(false),
	m_bExclusiveControllerConnected(false),
	m_bPausedForControllerDisconnect(false),
	m_bPausedForSystemMenu(false),
	m_bDeferredSystemMenuPause(false),
	m_previousPausedGameState(false),
	m_wasGamePausedBeforePLMForcedPause(false),
	m_hostMigrationState(eHMS_NotMigrating),
	m_hostMigrationTimeStateChanged(0.f),
	m_pDownloadMgr(NULL),
	m_pDLCUpr(NULL),
	m_pInputEventListenerOverride(NULL),
	m_randomGenerator(gEnv->bNoRandomSeed?0:(u32)gEnv->pTimer->GetAsyncTime().GetValue()),
	m_pGameAISystem(NULL),
	m_pRayCaster(NULL),
	m_pGameAchievements(NULL),
	m_pIntersectionTester(NULL),
	m_pGameActionHandlers(NULL),
	m_pGameCache(NULL),
	m_gameTypeMultiplayer(false),
	m_gameTypeInitialized(false),
	m_needsInitPatchables(false),
	m_settingRichPresence(false),
	m_bSignInOrOutEventOccured(false),
	m_iCachedGsmValue(0),
	m_fCachedGsmRangeValue(0.f),
	m_fCachedGsmRangeStepValue(0.f),
	m_pHudInterferenceGameEffect(NULL),
	m_pSceneBlurGameEffect(NULL),
	m_pLightningGameEffect(0),
	m_pParameterGameEffect(NULL),
	m_saveIconMode(eSIM_Off),
	m_saveIconTimer(0.0f),
	m_bUserHasPhysicalStorage(false),
	m_bCheckPointSave(false),
	m_pStatsEntityIdRegistry(NULL),
	m_pGameTokenSignalCreator(NULL),
#if DRX_PLATFORM_DURANGO
	m_userChangedDoSignOutAndIn(false),
#endif
	m_pMovingPlatformMgr(NULL)
#if ENABLE_MANUAL_FRAME_STEP
	, m_pManualFrameStep(new CManualFrameStepUpr())
#endif
{
	COMPILE_TIME_ASSERT( eCGE_Last <= 64 );

	m_pCVars = new SCVars();
	g_pGameCVars = m_pCVars;
	m_pGameActions = new CGameActions();
	g_pGameActions = m_pGameActions;
	m_pTacticalUpr = new CTacticalUpr();
	m_pBurnEffectUpr = new CBurnEffectUpr();
	g_pGame = this;
	m_bReload = false;
	m_inDevMode = false;
	m_userProfileChanged = true;
	m_bLastSaveDirty = true;
	m_editorDisplayHelpers = true;
	m_RenderingToHMD = false;

	ICVar*	pStereoOutput(gEnv->pConsole->GetCVar("r_StereoOutput"));
	if (pStereoOutput)
	{
		SetRenderingToHMD(pStereoOutput->GetIVal()==7);// 7 means HMD.
		SFunctor	oFunctor;
		oFunctor.Set(OnChangedStereoRenderDevice,pStereoOutput);
		pStereoOutput->AddOnChangeFunctor(oFunctor);
	}
	else
	{
		SetRenderingToHMD(false);
	}

	m_gameMechanismUpr = new CGameMechanismUpr();

#if ENABLE_GAME_CODE_COVERAGE
	new CGameCodeCoverageUpr("Scripts/gameCodeCoverage.xml");
	new CGameCodeCoverageGUI();
#endif

#if !defined(_RELEASE)
	m_pRemoteUpdateListener=&CGameRealtimeRemoteUpdateListener::GetGameRealtimeRemoteUpdateListener();
#endif

#if ENABLE_VISUAL_DEBUG_PROTOTYPE
	m_VisualDebugSys = new CVisualDebugPrototype();
#endif // ENABLE_VISUAL_DEBUG_PROTOTYPE

	SetInviteAcceptedState(eIAS_None);
	InvalidateInviteData();

	GetISystem()->SetIGame( this );

	CAnimationProxyDualCharacterBase::Load1P3PPairFile();

	m_bLoggedInFromInvite = false;
	m_updateRichPresenceTimer = 0.f;

	m_pendingRichPresenceSessionID = DrxSessionInvalidID;
	m_currentRichPresenceSessionID = DrxSessionInvalidID;

	m_bRefreshRichPresence = false;

	m_gameDataInstalled = true;
	m_postLocalisationBootChecksDone = true;

	m_cachedUserRegion = -1;
}

CGame::~CGame()
{
	m_pFramework->EndGameContext();
	m_pFramework->UnregisterListener(this);
	GetISystem()->GetPlatformOS()->RemoveListener(this);
	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);
	ReleaseScriptBinds();

	if (gEnv->pDynamicResponseSystem)
	{
		DRS::IResponseActor* pDrsActor = gEnv->pDynamicResponseSystem->GetResponseActor("GameTokenSignalSender");
		if (pDrsActor)
		{
			gEnv->pDynamicResponseSystem->ReleaseResponseActor(pDrsActor);
		}
	}

	CAnimationProxyDualCharacterBase::ReleaseBuffers();

	SAFE_DELETE_GAME_EFFECT(m_pHudInterferenceGameEffect);
	SAFE_DELETE_GAME_EFFECT(m_pSceneBlurGameEffect);
	SAFE_DELETE_GAME_EFFECT(m_pLightningGameEffect);
	SAFE_DELETE_GAME_EFFECT(m_pParameterGameEffect);

#ifdef USE_LAPTOPUTIL
	SAFE_DELETE(m_pLaptopUtil);
#endif
	SAFE_DELETE(m_pGameAudio);
	SAFE_DELETE(m_pScreenEffects);
	SAFE_DELETE(m_pPersistantStats);
	SAFE_DELETE(m_pEquipmentLoadout);
	SAFE_DELETE(m_pPlayerProgression);
	SAFE_DELETE(m_pGameAISystem);
	SAFE_DELETE(m_pPlayerVisTable);
	SAFE_DELETE(m_pRayCaster);
	SAFE_DELETE(m_pIntersectionTester);
	SAFE_DELETE(m_pGameActionHandlers);
	SAFE_DELETE(m_pGameCache);
	SAFE_RELEASE(m_pWeaponSystem);
	SAFE_DELETE(m_pGamePhysicsSettings);
	SAFE_DELETE(m_pItemStrings);
	SAFE_DELETE(m_pGameParametersStorage);
	SAFE_DELETE(m_pUIUpr);
	SAFE_DELETE(m_pLedgeUpr);
	SAFE_DELETE(m_pWaterPuddleUpr);
	SAFE_DELETE(m_pRecordingSystem);
	SAFE_DELETE(m_statsRecorder);
	SAFE_DELETE(m_patchPakUpr);
	SAFE_DELETE(m_pDataPatchDownloader);
	SAFE_DELETE(m_pGameLocalizationUpr);
	SAFE_DELETE(m_pGameTokenSignalCreator);
#if USE_LAGOMETER
	SAFE_DELETE(m_pLagOMeter);
#endif
	SAFE_DELETE(m_telemetryCollector);
	SAFE_DELETE(m_pPlaylistActivityTracker);
#if USE_TELEMETRY_BUFFERS
	SAFE_DELETE(m_performanceBuffer);
	SAFE_DELETE(m_bandwidthBuffer);
	SAFE_DELETE(m_memoryTrackingBuffer);
	SAFE_DELETE(m_soundTrackingBuffer);
#endif
	SAFE_DELETE(m_colorGradientUpr);
	SAFE_DELETE(m_pGameBrowser);
	SAFE_DELETE(m_pGameLobbyUpr);
#if IMPLEMENT_PC_BLADES
	SAFE_DELETE(m_pGameServerLists);
#endif //IMPLEMENT_PC_BLADES
	SAFE_DELETE(m_pSquadUpr);
	SAFE_DELETE(m_pTacticalUpr);
	SAFE_DELETE(m_pBurnEffectUpr);
	SAFE_DELETE(m_pAutoAimUpr);
	SAFE_DELETE(m_pHitDeathReactionsSystem);
	SAFE_DELETE(m_pBodyDamageUpr);
	SAFE_DELETE(m_pPlaylistUpr);
	SAFE_DELETE(m_pDLCUpr);
	SAFE_DELETE(m_pMovementTransitionsSystem);
	SAFE_DELETE(m_pStatsEntityIdRegistry);
	SAFE_DELETE(m_pMovingPlatformMgr);
	SAFE_DELETE(m_pMatchMakingTelemetry);
	SAFE_DELETE(m_pWorldBuilder);
#if ENABLE_MANUAL_FRAME_STEP
	SAFE_DELETE(m_pManualFrameStep);
#endif

	if (m_pLobbySessionHandler != NULL)
	{
		// Clear the pointer in DrxAction this will also call delete on m_pLobbySessionHandler and set it to NULL
		ClearGameSessionHandler();
	}

	if (gEnv->pNetwork && gEnv->pNetwork->GetLobby())
	{
		IDrxLobby* pLobby = gEnv->pNetwork->GetLobby();

		pLobby->UnregisterEventInterest(eCLSE_PartyMembers, CGame::PartyMembersCallback, this);
		pLobby->UnregisterEventInterest(eCLSE_UserProfileChanged, CGame::UserProfileChangedCallback, this);
		pLobby->UnregisterEventInterest(eCLSE_InviteAccepted, CGame::InviteAcceptedCallback, this);
		pLobby->UnregisterEventInterest(eCLSE_OnlineState, CGame::OnlineStateCallback, this);
		pLobby->UnregisterEventInterest(eCLSE_EthernetState, CGame::EthernetStateCallback, this);
		pLobby->UnregisterEventInterest(eCLSE_InviteAccepted, CGame::InviteAcceptedCallback, this);

		if (gEnv && gEnv->pNetwork)
		{
			gEnv->pNetwork->SetMultithreadingMode(INetwork::NETWORK_MT_OFF);		// Kill the network thread then its safe to kill the lobby
		}

		if (pLobby->GetLobbyService(eCLS_LAN))
		{
			pLobby->Terminate(eCLS_LAN, eCLSO_All, NULL, NULL);
		}

		if (pLobby->GetLobbyService(eCLS_Online))
		{
			pLobby->Terminate(eCLS_Online, eCLSO_All, NULL, NULL);
		}
	}

	GAME_FX_SYSTEM.Destroy();

	ScreenResolution::ReleaseScreenResolutions();

	if(gEnv && gEnv->pInput)
	{
		gEnv->pInput->RemoveEventListener(this);
		gEnv->pInput->SetExclusiveListener(NULL);
	}

	// Delete this after everything else...
	// some of the things deleted above could well be game mechanisms, so we need
	// the game mechanism manager to still exist when they're deleted [TF]
	SAFE_DELETE(m_gameMechanismUpr);
	SAFE_DELETE(m_pGameAchievements);

#if ENABLE_VISUAL_DEBUG_PROTOTYPE
	SAFE_DELETE(m_VisualDebugSys );
#endif // ENABLE_VISUAL_DEBUG_PROTOTYPE

	SAFE_DELETE(m_pCVars); // Do this last to avoid cached CVars being used and causing crashes

	g_pGame = 0;
	g_pGameCVars = 0;
	g_pGameActions = 0;
	GetISystem()->SetIGame( NULL );
}

#define EngineStartProfiler(x)
#define InitTerminationCheck(x)

static inline void InlineInitializationProcessing(tukk sDescription)
{
	EngineStartProfiler( sDescription );
	InitTerminationCheck( sDescription );
	gEnv->pLog->UpdateLoadingScreen(0);
}

bool CGame::Init(IGameFramework *pFramework)
{
  LOADING_TIME_PROFILE_SECTION(GetISystem());

	InlineInitializationProcessing("CGame::Init");
#ifdef GAME_DEBUG_MEM
	DumpMemInfo("CGame::Init start");
#endif

	m_pFramework = pFramework;
	assert(m_pFramework);

	m_pConsole = gEnv->pConsole;

	//This should be early so we can load the strings
	m_pGameLocalizationUpr = new CGameLocalizationUpr();

	RegisterConsoleVars();
	RegisterConsoleCommands();
	RegisterGameObjectEvents();


	if (!gEnv->IsDedicated())
	{
		//iterates the available screen resolutions and stores them, do that before creating the profile options
		ScreenResolution::InitialiseScreenResolutions();

		m_pUIUpr = new CUIUpr();
		m_pUIUpr->Init();
	}

	InitPlatformOS();

	{
		IProceduralClipFactory& proceduralClipFactory = m_pFramework->GetMannequinInterface().GetProceduralClipFactory();
		mannequin::RegisterProceduralClipsForModule(proceduralClipFactory);
	}

	// Initialize static item strings
	m_pItemStrings = new SItemStrings();

	m_pGameParametersStorage = new CGameSharedParametersStorage();
	m_pGamePhysicsSettings = new CGamePhysicsSettings();

	GetISystem()->GetPlatformOS()->AddListener(this, "CGame");
	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this);

	LoadActionMaps("libs/config/defaultProfile.xml");
	InlineInitializationProcessing("CGame::Init LoadActionMaps");
	InitScriptBinds();

	InlineInitializationProcessing("CGame::Init InitScriptBinds");

	//load user levelnames for ingame text and savegames
	LoadMappedLevelNames("Scripts/GameRules/LevelNames.xml");

	InitRichPresence();

	// Register all the games factory classes e.g. maps "Player" to CPlayer
	InitGameFactory(m_pFramework);

	InlineInitializationProcessing("CGame::Init InitGameFactory");

	IItemSystem* pItemSystem = pFramework->GetIItemSystem();
	pItemSystem->Scan(GAME_ITEMS_DATA_FOLDER);

 	m_pWeaponSystem = new CWeaponSystem(this, GetISystem());
	m_pWeaponSystem->Scan(GAME_ITEMS_DATA_FOLDER);

	string actorParamsFolder = "scripts/entities/actor/parameters";
	m_pFramework->GetIActorSystem()->Scan(actorParamsFolder);

	m_pAutoAimUpr = new CAutoAimUpr();

	InlineInitializationProcessing("CGame::Init HitDeathReactionsSystem");

	m_pHitDeathReactionsSystem = new CHitDeathReactionsSystem;
	DRX_ASSERT(m_pHitDeathReactionsSystem);

	m_pMovementTransitionsSystem = new CMovementTransitionsSystem;
	DRX_ASSERT(m_pMovementTransitionsSystem);

	gEnv->pConsole->CreateKeyBind("f12", "r_getscreenshot 2");

	//Ivo: initialites the Crysis conversion file.
	//this is a conversion solution for the Crysis game DLL. Other projects don't need it.
	// No need anymore
	//gEnv->pCharacterUpr->LoadCharacterConversionFile("Objects/CrysisCharacterConversion.ccc");

	// set game GUID
	m_pFramework->SetGameGUID(DRXENGINE_SDK_GUID);

	// TEMP
	// Load the action map beforehand (see above)
	// afterwards load the user's profile whose action maps get merged with default's action map
	m_pPlayerProfileUpr = m_pFramework->GetIPlayerProfileUpr();

	if (CProfileOptions* profileOptions = GetProfileOptions())
		profileOptions->Init();

	InlineInitializationProcessing("CGame::Init PlayerProfileUpr");

#if !defined(_RELEASE)
	if (!g_pGameCVars->g_skipStartupSignIn)
#endif
	{
		gEnv->pSystem->GetPlatformOS()->UserDoSignIn(0); // sign in the default user
	}

#if DRX_PLATFORM_DURANGO
	XboxLiveGameEvents::CreateGUID(m_playerSessionId);
#endif

	InlineInitializationProcessing("CGame::Init ProfileOptions");
	if (!gEnv->IsEditor())
	{
		DrxFixedStringT<128> configFileName;

		if(gEnv->IsDedicated())
		{
			tukk const DEDICATED_BASE_CONFIG_NAME = "dedicated";
			string path = gEnv->pSystem->GetRootFolder();

			configFileName.Format("%s%s.cfg", path.c_str(), DEDICATED_BASE_CONFIG_NAME);
			DrxLog("[dedicated] loading dedicated config %s", configFileName.c_str());

			SDedicatedConfigSink sink;
			gEnv->pSystem->LoadConfiguration(configFileName.c_str(), &sink, eLoadConfigInit);
		}

			DrxLogAlways("[Game Version]: "
#if defined(_RELEASE)
				"RELEASE "
#elif defined(_PROFILE)
				"PROFILE "
#else
				"DEBUG "
#endif

#if defined(PURE_CLIENT)
				"PURE CLIENT"
#elif (DEDICATED_SERVER)
				"DEDICATED SERVER"
#else
				"DEVELOPMENT BUILD"
#endif
				);

#if !defined(DEDICATED_SERVER) || defined(PURE_CLIENT)
			if (gEnv->IsDedicated())
			{
#if defined(_RELEASE)
				DrxFatalError("[Game]: Running wrong version of DedicatedServer.exe - aborting");
#endif // #if defined(_RELEASE)
			}
#endif

		EDrxLobbyServiceFeatures	features = EDrxLobbyServiceFeatures(eCLSO_All & ~(eCLSO_Voice));
		bool											isDedicated = gEnv->IsDedicated();
		EDrxLobbyError						error;

		if ( isDedicated )
		{
			features = EDrxLobbyServiceFeatures( features & ~( eCLSO_LobbyUI | eCLSO_Friends | eCLSO_Reward ) );
			m_pPlayerProfileUpr->SetExclusiveControllerDeviceIndex(0);
			if(gEnv->pInput)
			{
				gEnv->pInput->ForceFeedbackSetDeviceIndex(0);
			}
		}

		gEnv->pNetwork->SetMultithreadingMode(INetwork::NETWORK_MT_PRIORITY_NORMAL);

		gEnv->pNetwork->GetLobby()->SetUserPacketEnd(eGUPD_End);

#if DRX_PLATFORM_WINDOWS
		if (gEnv->IsDedicated())
		{
			ICmdLine *pCommandLine = gEnv->pSystem->GetICmdLine();
			if (pCommandLine->FindArg(eCLAT_Post, "connect") || pCommandLine->FindArg(eCLAT_Post, "client"))
			{
				gEnv->bServer = false;
				gEnv->SetIsClient(true);
			}
		}
#endif
#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD) || defined(IS_EAAS)
		if ( !(g_pGameCVars && (g_pGameCVars->g_useOnlineServiceForDedicated) && gEnv->IsDedicated()))
		{
			error = gEnv->pNetwork->GetLobby()->Initialise(eCLS_LAN, features, CGameBrowser::ConfigurationCallback, CGameBrowser::InitialiseCallback, this);
			DRX_ASSERT_MESSAGE( error == eCLE_Success, "Failed to initialize LAN lobby service" );
		}
#endif // #if !defined(_RELEASE) || defined(PERFORMANCE_BUILD) || defined(IS_EAAS)

		if(!gEnv->IsDedicated())
		{
			error = gEnv->pNetwork->GetLobby()->Initialise(eCLS_Online, features, CGameBrowser::ConfigurationCallback, CGameBrowser::InitialiseCallback, this);
		}
		else
		{
			DrxLog("Online lobby currently not supported for dedicated sever. Not initialized");
		}

		//DRX_ASSERT_MESSAGE( error == eCLE_Success, "Failed to initialize online lobby service" );
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_COMMENT, "Online Lobby not supported in the default SDK.");
		m_pSquadUpr = new CSquadUpr();		// MUST be done before game browser is constructed

		m_pGameBrowser = new CGameBrowser();

		//Set the matchmaking version based on the build version if g_matchmakingversion is a default value
		if (!gEnv->IsEditor() && g_pGameCVars->g_MatchmakingVersion <= 1)
		{
			i32k internalBuildVersion = gEnv->pSystem->GetBuildVersion().v[0];
			DrxLog("BuildVersion %d", internalBuildVersion);
			if (internalBuildVersion != 1)
			{
				g_pGameCVars->g_MatchmakingVersion = internalBuildVersion;
			}
		}

		CGameBrowser::InitLobbyServiceType();

		m_pGameLobbyUpr = new CGameLobbyUpr();
	}

	m_pGameAchievements = new CGameAchievements;	//Should be after GameLobbyUpr
	DRX_ASSERT(m_pGameAchievements);

#ifdef USE_LAPTOPUTIL
	// CLaptopUtil must be created before CFlashMenuObject as this one relies on it
	if(!m_pLaptopUtil)
		m_pLaptopUtil = new CLaptopUtil;
#endif

	if (!(gEnv->IsDedicated()) && !(gEnv->bMultiplayer))
	{
		i32k iCVarDifficulty = g_pGameCVars->g_difficultyLevel;
		EDifficulty difficulty = (iCVarDifficulty >= eDifficulty_Default && iCVarDifficulty < eDifficulty_COUNT ? (EDifficulty)iCVarDifficulty : eDifficulty_Default);
		SetDifficultyLevel(difficulty);
	}

	if (!m_pGameAudio)
	{
		m_pGameAudio = new CGameAudio();
	}

	InlineInitializationProcessing("CGame::Init GameAudio");

	m_pPlayerProgression = new CPlayerProgression();	//Needs to be before persistant stats

	if(!m_pPersistantStats)
	{
		m_pPersistantStats = new CPersistantStats();	//Needs to be before Skill Assessments
	}

#if !defined(_RELEASE)
	// START FIXME
	DRX_FIXME( 24, 02, 2010, "Temporarily added early cvar sets so that game init can use them to pick the correct sp/mp game. Needs to be removed when a cleaner MP/SP split is there.");
	// Abusing the pre cmd line args but necessary so that the current game type cvars are read to use for game init
	if (const ICmdLineArg *pCmdArg = gEnv->pSystem->GetICmdLine()->FindArg(eCLAT_Pre,"g_multiplayerDefault"))
	{
		g_pGameCVars->g_multiplayerDefault = pCmdArg->GetIValue();
	}
	// END FIXME
#endif // !defined(_RELEASE)

	GAME_FX_SYSTEM.Initialise();

	if (!gEnv->IsEditor())
	{
		if (g_pGameCVars->g_EPD == 3)
		{
			g_pGameCVars->g_multiplayerModeOnly = 1;
		}

		if (g_pGameCVars->g_multiplayerModeOnly)
		{
			g_pGameCVars->g_multiplayerDefault = 1;
		}
	}

	// DLC Upr needs to be loaded after the warnings manager because it may display warnings
	m_pDLCUpr = new CDLCUpr();

	ICVar* pMaxPlayers = gEnv->pConsole->GetCVar("sv_maxplayers");
	if(pMaxPlayers)
	{
		pMaxPlayers->SetOnChangeCallback(VerifyMaxPlayers);	// this needs to be set 1st, if MAX_PLAYER_LIMIT is greater than 32 we'll clamp it otherwise
		pMaxPlayers->Set(MAX_PLAYER_LIMIT);
	}

	if (!gEnv->IsEditor())
	{
		m_pDownloadMgr=new CDownloadMgr();
		m_patchPakUpr = new CPatchPakUpr;

		m_pDownloadMgr->Init("Scripts/Network/TCPServices.xml", "Scripts/Network/DownloadMgr.xml");	// after constructing CPatchPakUpr so the downloadmgr can add itself as a listener to the patchpakmanager
	}

	if (!gEnv->IsDedicated())
	{
			// This needs to be scheduled regardless of if we're showing the language selection screen or not, for autotest multiplayer (which we don't know about at this point)
			LoadPatchLocalizationData();

			//No localization is going to be shown. Perform post-loc checking immediately
			gEnv->pSystem->GetPlatformOS()->PostLocalizationBootChecks();
			InitGameType(g_pGameCVars->g_multiplayerDefault!=0, true);

			// we need this to setup the xmb buttons
			IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
			IDrxLobbyService *pLobbyService = pLobby ? pLobby->GetLobbyService(eCLS_Online) : NULL;
			IDrxLobbyUI *pLobbyUI = pLobbyService ? pLobbyService->GetLobbyUI() : NULL;
			if(pLobbyUI)
			{
				pLobbyUI->PostLocalizationChecks();
			}
	}
	else
	{
		InitGameType(true, true);
	}

	InlineInitializationProcessing("CGame::Init InitGameType");


	if (!m_pScreenEffects)
	{
		m_pScreenEffects = new CScreenEffects();
	}

	// Create hud interference game effect
	if(m_pHudInterferenceGameEffect == NULL)
	{
		m_pHudInterferenceGameEffect = GAME_FX_SYSTEM.CreateEffect<CHudInterferenceGameEffect>();
		if(m_pHudInterferenceGameEffect)
		{
			m_pHudInterferenceGameEffect->Initialise();
		}
	}

	// Create scene blur game effect
	if(m_pSceneBlurGameEffect == NULL)
	{
		m_pSceneBlurGameEffect = GAME_FX_SYSTEM.CreateEffect<CSceneBlurGameEffect>();
		if(m_pSceneBlurGameEffect)
		{
			m_pSceneBlurGameEffect->Initialise();
		}
	}

	// Create lightning game effect
	if(m_pLightningGameEffect == NULL)
	{
		m_pLightningGameEffect = GAME_FX_SYSTEM.CreateEffect<CLightningGameEffect>();
		if(m_pLightningGameEffect)
			m_pLightningGameEffect->Initialise();
	}

	// Create parameter game effect
	if(m_pParameterGameEffect == NULL)
	{
		m_pParameterGameEffect = GAME_FX_SYSTEM.CreateEffect<CParameterGameEffect>();
		if(m_pParameterGameEffect)
		{
			m_pParameterGameEffect->Initialise();
		}
	}

	if (!m_pLedgeUpr)
	{
		m_pLedgeUpr = new CLedgeUpr();
	}

	if (!m_pWaterPuddleUpr)
	{
		m_pWaterPuddleUpr = new CWaterPuddleUpr();
	}

	InlineInitializationProcessing("CGame::Init LedgeUpr");

	if (gEnv->pRenderer)
	{
		m_colorGradientUpr = new Graphics::CColorGradientUpr();
	}

	m_pFramework->RegisterListener(this, "Game", FRAMEWORKLISTENERPRIORITY_GAME);

	CVehicleClient *pVehicleClient = new CVehicleClient();
	pVehicleClient->Init();
	g_pGame->GetIGameFramework()->GetIVehicleSystem()->RegisterVehicleClient(pVehicleClient);

	InlineInitializationProcessing("CGame::Init RegisterVehicleClient");

#ifdef GAME_DEBUG_MEM
	DumpMemInfo("CGame::Init end");
#endif

	if (gEnv->IsEditor())
	{
		g_tacticalPointLanguageExtender.Initialize();
	}
	else
	{
		if ( !gEnv->IsDedicated() )
		{
#if IMPLEMENT_PC_BLADES
			m_pGameServerLists = new CGameServerLists();
#endif //IMPLEMENT_PC_BLADES

		}
	}

	// Initialise game handler for checkpoints
	CCheckpointGame::GetInstance()->Init();

	IDrxLobby *pLobby = gEnv->pNetwork ? gEnv->pNetwork->GetLobby() : NULL;
	if (pLobby)
	{
		pLobby->RegisterEventInterest(eCLSE_PartyMembers, CGame::PartyMembersCallback, this);
		pLobby->RegisterEventInterest(eCLSE_UserProfileChanged, CGame::UserProfileChangedCallback, this);
		pLobby->RegisterEventInterest(eCLSE_OnlineState, CGame::OnlineStateCallback, this);
		pLobby->RegisterEventInterest(eCLSE_EthernetState, CGame::EthernetStateCallback, this);
		pLobby->RegisterEventInterest(eCLSE_InviteAccepted, CGame::InviteAcceptedCallback, this);

		// online only stuff
		IDrxLobbyService *pOnlineLobbyService = pLobby->GetLobbyService(eCLS_Online);
		if(pOnlineLobbyService)
		{
			if(m_pPlayerProfileUpr)
			{
				DrxLog("[online] RegisterOnlineAttributes");
				INDENT_LOG_DURING_SCOPE();
				m_pPlayerProfileUpr->RegisterOnlineAttributes();
			}
		}
	}

#if ENABLE_FEATURE_TESTER
	new CFeatureTester();
#endif

#ifdef CODECHECKPOINT_DEBUG_ENABLED
	CCodeCheckpointDebugMgr::RetrieveCodeCheckpointDebugMgr();
#endif


	if(gEnv->pInput)
	{
		gEnv->pInput->SetExclusiveListener(this);
	}

	ICVar* pEnableAI = gEnv->pConsole->GetCVar("sv_AISystem");
	if(!gEnv->bMultiplayer || (pEnableAI && pEnableAI->GetIVal()))
	{
		m_pGameAISystem = new CGameAISystem();

		InlineInitializationProcessing("CGame::Init GameAISystem");
	}

	//if (gEnv->IsEditor())
	{
		m_pRayCaster = new GlobalRayCaster;
		m_pRayCaster->SetQuota(g_pGameCVars->g_gameRayCastQuota);

		m_pIntersectionTester = new GlobalIntersectionTester;
		m_pIntersectionTester->SetQuota(g_pGameCVars->g_gameIntersectionTestQuota);
	}

	m_pPlayerVisTable = new CPlayerVisTable();

	m_pGameCache = new CGameCache;
	m_pGameCache->Init();

	CBullet::StaticInit();

	InlineInitializationProcessing("CGame::Init IntersectionTester");

	m_pWorldBuilder = new CWorldBuilder();

	if(gEnv->pDynamicResponseSystem)
	{
		// register the custom DRS actions and conditions
		REGISTER_DRS_CUSTOM_ACTION(CActionExecuteAudioTrigger);
		REGISTER_DRS_CUSTOM_ACTION(CActionSpeakLineBasedOnVariable);
		REGISTER_DRS_CUSTOM_CONDITION(CConditionDistanceToEntity);
		// create a special DrsActor that sends out our automatic signals every time a gametoken changes its value
		DRS::IResponseActor* pDrsActor = gEnv->pDynamicResponseSystem->CreateResponseActor("GameTokenSignalSender");
		m_pGameTokenSignalCreator = new CGameTokenSignalCreator(pDrsActor);
	}
	return true;
}

bool CGame::CompleteInit()
{
	if (!gEnv->IsEditor())
	{
		//This will load the parameters in shared storage
		m_pWeaponSystem->Reload();
		CFrontEndModelCache::UpdateNeed3dFrontEndAssets();
	}
	//else look at OnEditorGameInitComplete()

#ifdef INCLUDE_GAME_AI_RECORDER
	assert(gEnv->bMultiplayer || m_pGameAISystem);
	if(m_pGameAISystem)
	{
		m_pGameAISystem->GetGameAIRecorder().Init();
	}
#endif //INCLUDE_GAME_AI_RECORDER

#ifdef GAME_DEBUG_MEM
	DumpMemInfo("CGame::CompleteInit");
#endif
	return true;
}

void CGame::OnEditorGameInitComplete()
{
	//This will load the parameters in shared storage
	m_pWeaponSystem->Reload();
}

void CGame::RegisterGameFlowNodes()
{
	if (IFlowSystem *pFlow = m_pFramework->GetIFlowSystem())
	{
		CG2AutoRegFlowNodeBase *pFactory = CG2AutoRegFlowNodeBase::m_pFirst;

		while (pFactory)
		{
			pFlow->RegisterType( pFactory->m_sClassName,pFactory );
			pFactory = pFactory->m_pNext;
		}
	}
}

CRevertibleConfigLoader &CGame::GetGameModeCVars()
{
	return s_gameModeCVars;
}

#define MODE_SWITCH_PAK_FILENAME "modes/gamemodeswitch.pak"
#define MODE_SWITCH_LIST_FILENAME "gamemodeswitch"

// Loads and Inits either singleplayer or multiplayer
// Called when selecting from the selection frontend
void CGame::InitGameType(bool multiplayer, bool fromInit /*= false*/)
{
	DrxLog("CGame::InitGameType() multiplayer=%s, fromInit=%s", multiplayer ? "true" : "false", fromInit ? "true" : "false");
#if defined(DEDICATED_SERVER)
	if (!multiplayer)
	{
		DrxFatalError("CGame::InitGameType(bool multiplayer, bool fromInit): multiplayer == false not supported for dedicated server");
		multiplayer = true;
	}
#endif

	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "CGame::InitGameType");

	SAFE_DELETE(m_pDataPatchDownloader);

	bool bWasMultiplayer = gEnv->bMultiplayer;
	tukk const hostName = gEnv->pSystem->GetPlatformOS()->GetHostName();
	DrxLog( "hostName = '%s'", hostName );

	if (g_pGameCVars->g_multiplayerModeOnly && multiplayer==false)
	{
		GameWarning("[InitGameType]: Cannot switch from multiplayer as g_multiplayerModeOnly is set.");
		return;
	}

	if( fromInit == false )
	{
		//Load the mode switch pak, this can considerably reduce the time spent switching especially from disc
		if( gEnv->pSystem->GetIResourceUpr()->LoadModeSwitchPak(MODE_SWITCH_PAK_FILENAME, multiplayer) == false )
		{
			DrxWarning( VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Could not load %s during game mode switch. This file can significantly reduce mode switching times.\n", MODE_SWITCH_PAK_FILENAME );
		}
	}

	gEnv->pGame->GetIGameFramework()->InitGameType(multiplayer, fromInit);

#if USE_TELEMETRY_BUFFERS
	SAFE_DELETE(m_performanceBuffer);
	SAFE_DELETE(m_bandwidthBuffer);
	SAFE_DELETE(m_memoryTrackingBuffer);
	SAFE_DELETE(m_soundTrackingBuffer);
#endif //#if USE_TELEMETRY_BUFFERS
	SAFE_DELETE(m_statsRecorder);
	SAFE_DELETE(m_pRecordingSystem);
	SAFE_DELETE(m_pEquipmentLoadout);
#if USE_LAGOMETER
	SAFE_DELETE(m_pLagOMeter);
#endif
	SAFE_DELETE(m_telemetryCollector);
	SAFE_DELETE(m_pPlaylistActivityTracker);
	SAFE_DELETE(m_pStatsEntityIdRegistry);
	SAFE_DELETE(m_pMatchMakingTelemetry);

	if (m_pLobbySessionHandler != NULL)
	{
		// Clear the pointer in DrxAction this will also call delete on m_pLobbySessionHandler and set it to NULL
		ClearGameSessionHandler();
	}
	if (m_pPlaylistUpr)
	{
		SAFE_DELETE(m_pPlaylistUpr);
	}

#if 0
	if (multiplayer)
	{
		CMPConfigSink cvarSink;
		gEnv->pSystem->LoadConfiguration("mp_release.cfg", &cvarSink);
	}
#endif // #if 0

	bool gameTypeChanged = (m_gameTypeMultiplayer != multiplayer) && !fromInit;

	tukk const GAMETYPE_CONFIG_NAMES[2] = {"multiplayer", "singleplayer"};
	i32k gameTypeID = multiplayer ? 0 : 1;
	tukk const gameTypeConfigName = GAMETYPE_CONFIG_NAMES[gameTypeID];

	s_gameModeCVars.RevertCVarChanges();

	// patch before loading multiplayer.cfg to allow it to be patchable
	if (multiplayer==true)
	{
#if 1
		if (!gEnv->IsEditor())
		{
			if (m_patchPakUpr)
			{
				// start & finish downloading and caching any paks in flight or timeout
				m_patchPakUpr->BlockingUpdateTillDone();
			}
		}
#endif
	}

	// Load config file
	s_gameModeCVars.LoadConfiguration(gameTypeConfigName);

	// For consoles load console & then platform specific mp config file
	// For PC just load platform specific mp config file
	if (multiplayer==true)
	{
		gEnv->pNetwork->SetMultithreadingMode(INetwork::NETWORK_MT_PRIORITY_HIGH);

		//Force enable statoscope
#if 0 //defined(_RELEASE) && defined(ENABLE_PROFILING_CODE)
		if(gEnv->pStatoscope)
		{
			gEnv->pStatoscope->SetupFPSCaptureCVars();
		}
#endif

		if (!gEnv->IsDedicated())
		{
			// Late initialise voice service if playing multiplayer (and not a dedicated server)
			gEnv->pNetwork->GetLobby()->Initialise(eCLS_Online, eCLSO_Voice, CGameBrowser::ConfigurationCallback, NULL, NULL);
		}

		tukk mpConfigName = NULL;

		EPlatform platform = GetPlatform();
		switch (platform)
		{
			case ePlatform_PC:
			{
				mpConfigName = "multiplayer_pc";
				break;
			}
			default:
			{
				mpConfigName = NULL;
				break;
			}
		}
		if (mpConfigName)
		{
			// Load mp platform specific config file
			s_gameModeCVars.LoadConfiguration(mpConfigName);
		}
	}
#if !defined(DEDICATED_SERVER)
	else
	{
		gEnv->pNetwork->SetMultithreadingMode(INetwork::NETWORK_MT_PRIORITY_NORMAL);
		// Early terminate voice service if playing single player (attempt to save memory)
		gEnv->pNetwork->GetLobby()->Terminate(eCLS_Online, eCLSO_Voice, NULL, NULL);
	}
#endif

	// Toggle entity pooling system, only used for singleplayer
	gEnv->pEntitySystem->GetIEntityPoolUpr()->Enable(!multiplayer);

	// Switch DrxNetwork to the correct threading mode
	gEnv->bMultiplayer=multiplayer;

	m_pGameLocalizationUpr->SetGameType(); //should be after gEnv->bMultiplayer is set

	REINST("needs verification!");
	/*ICVar* pFilename = gEnv->pConsole->GetCVar("s_AudioPreloadsFile");
	if(pFilename)
	{
		gEnv->pSoundSystem->ClearAudioDataCache();
		pFilename->Set(multiplayer ? "AudioPreloads_mp" : "AudioPreloads");
		gEnv->pSoundSystem->CacheAudioData(NULL, 1);
	}*/

	if (gameTypeChanged)
	{
		//Reload item/ammo parameters for game mode
		m_pWeaponSystem->Reload();
	}

	m_pPersistantStats->SetMultiplayer(multiplayer);

	if (m_pSquadUpr)
	{
		m_pSquadUpr->SetMultiplayer(multiplayer);
	}

	if(m_pGameLobbyUpr)
	{
		m_pGameLobbyUpr->SetMultiplayer(multiplayer);
		m_pGameLobbyUpr->LeaveGameSession(CGameLobbyUpr::eLSR_SwitchGameType);
	}

	ICVar* pAllowDisconnect = gEnv->pConsole->GetCVar("g_allowDisconnectIfUpdateFails");
	ICVar* pGsmLodsNum 			= gEnv->pConsole->GetCVar("e_GsmLodsNum");
	assert(pGsmLodsNum);
	ICVar* pGsmRange	 			= gEnv->pConsole->GetCVar("e_GsmRange");
	assert(pGsmRange);
	ICVar* pGsmRangeStep 		= gEnv->pConsole->GetCVar("e_GsmRangeStep");
	assert(pGsmRangeStep);

#if DRX_PLATFORM_WINDOWS && !defined(DEDICATED_SERVER)
	gEnv->pHardwareMouse->IncrementCounter();
#endif

	if(multiplayer)
	{
		if (!gEnv->IsEditor())
		{
			// Multiplayer Init
			DrxLog("CGame: Multiplayer game type initialized");
			g_pGameCVars->g_multiplayerDefault = 1;

			//The Squadmanager, gamebrowser and gamelobbymanager upon which this is dependent are initialised early, so that Rich Presence and certain PSN related functions can run on init.
			m_pLobbySessionHandler = new CDrxLobbySessionHandler();

			m_pDataPatchDownloader=new CDataPatchDownloader();
			m_pPlaylistUpr = new CPlaylistUpr;
			if (m_pPlaylistUpr)
			{
				m_pPlaylistUpr->Init("Scripts/Playlists");
			}
			//also setup any DLC Playlists
			if( m_pDLCUpr )
			{
				m_pDLCUpr->AddPlaylists();
			}

#if USE_LAGOMETER
			m_pLagOMeter = new CLagOMeter();
#endif

			DRX_ASSERT(!m_telemetryCollector);
			MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, EMemStatContextFlags::MSF_None, "CTelemetryCollector");
			m_telemetryCollector=new CTelemetryCollector;

			m_pPlaylistActivityTracker = new CPlaylistActivityTracker;
			m_pPlaylistActivityTracker->SetState( CPlaylistActivityTracker::eATS_GetActivity );

			if (g_pGameCVars->g_telemetry_gameplay_enabled)
			{
				MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, EMemStatContextFlags::MSF_None, "CStatsRecordingMgr");
				m_statsRecorder=new CStatsRecordingMgr;
			}


#if USE_TELEMETRY_BUFFERS
			{
				MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, EMemStatContextFlags::MSF_None, "Perf Telemetry Buffers");
				if(g_pGameCVars->g_telemetrySampleRatePerformance > 0.0f)
				{
					m_performanceBuffer = new CTelemetryBuffer(60*1024, m_telemetryCollector, sizeof(SPerformanceTelemetry));
				}

				if(g_pGameCVars->g_telemetrySampleRateBandwidth > 0.0f)
				{
					m_bandwidthBuffer = new CTelemetryBuffer(60*1024, m_telemetryCollector, sizeof(SBandwidthTelemetry));
				}

				if(g_pGameCVars->g_telemetrySampleRateMemory > 0.0f)
				{
					m_memoryTrackingBuffer = new CTelemetryBuffer(30*1024, m_telemetryCollector, sizeof(SMemoryTelemetry));
				}
			}
#endif //#if USE_TELEMETRY_BUFFERS

			m_pStatsEntityIdRegistry = new CStatsEntityIdRegistry();

			if (m_pUIUpr)
				m_pUIUpr->InitGameType(true, fromInit);

			// get the user's online attributes
			if(m_pPlayerProfileUpr)
			{
				m_pPlayerProfileUpr->EnableOnlineAttributes(true);
			}

			// Cache shadow cvars
			m_iCachedGsmValue = pGsmLodsNum->GetIVal();
			m_fCachedGsmRangeValue = pGsmRange->GetFVal();
			m_fCachedGsmRangeStepValue = pGsmRangeStep->GetFVal();

			if(pAllowDisconnect)
			{
				pAllowDisconnect->Set(0);
			}
		}

		bool		runMPInitNow=gEnv->IsDedicated();

#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
		if (g_pGameCVars->net_initLobbyServiceToLan)
		{
			runMPInitNow=true;
		}
#endif

		m_needsInitPatchables=true;

		// Michiel: have to initialize this on game-side too for now, the game doesnt have its asynchtasks anymore
		runMPInitNow = true;

		if (runMPInitNow)
		{
#if PLATFORM_SUPPORTS_RSS_PLAYLISTS
			CPlaylistUpr *pPlaylistUpr = g_pGame->GetPlaylistUpr();
			if(pPlaylistUpr)
			{
				pPlaylistUpr->RequestPlaylistFeedFromRSS();
				pPlaylistUpr->RequestVariantsFeedFromRSS();
			}
#endif
			if (gEnv->IsDedicated())
			{
				// If it's a dedicated server wait for the data patch & playlists to download before continuing
				// normally this would be done asynchronously in MenuDataMPLoaderTasks.cpp but that isn't
				// enabled in dedicated builds
				if (m_pDownloadMgr)
				{
#if PLATFORM_SUPPORTS_RSS_PLAYLISTS
					tukk resources[] = { "datapatch", "playlists", "variants" };
#else
					tukk resources[] = { "datapatch" };
#endif
					i32k numResources = sizeof(resources) / sizeof(tuk);
					m_pDownloadMgr->WaitForDownloadsToFinish(resources, numResources, g_pGameCVars->g_mpLoaderScreenMaxTimeSoftLimit);
				}
			}
			if (m_pDataPatchDownloader)
			{
				CDownloadableResourcePtr pRes = m_pDataPatchDownloader->GetDownloadableResource();
				if (pRes && pRes->GetState() == CDownloadableResource::k_dataAvailable)
				{
					m_pDataPatchDownloader->SetPatchingEnabled(true);
				}
				else
				{
					m_pDataPatchDownloader->CancelDownload();
				}
			}
			InitPatchableModules(true);
		}

		GetIGameFramework()->PauseGame(false, true);
	}
#if !defined(DEDICATED_SERVER)
	else
	{
		if (m_patchPakUpr)
		{
			// patch paks are currently for MP only - unload any loaded patch paks as we head into SP
			m_patchPakUpr->UnloadPatchPakFiles();
		}

		// Singleplayer Init
		DrxLog("CGame: Singleplayer game type initialised");
		g_pGameCVars->g_multiplayerDefault = 0;

		if (m_pUIUpr)
			m_pUIUpr->InitGameType(false, fromInit);

		if (!gEnv->IsEditor())
		{
			if (g_pGameCVars->g_telemetryEnabledSP!=0)			// (cvar values: 2 is record locally but not submit, 1 is record and submit)
			{
				m_statsRecorder=new CStatsRecordingMgr;				// if this it to be enabled on ship for SP it will need to ensure it is using a fixed memory buffer (like mp) and we will need to check that hand over of the storage buffer from mp/sp works in the case where the storage buffer has not been deleted due to a send still being in progress
			}

			if (g_pGameCVars->g_telemetryEnabledSP == 1 || g_pGameCVars->g_telemetrySampleRateSound > 0.0f || g_pGameCVars->g_telemetrySampleRatePerformance > 0.0f)
			{
				DRX_ASSERT(!m_telemetryCollector);
				m_telemetryCollector=new CTelemetryCollector;

#if USE_TELEMETRY_BUFFERS
				if(g_pGameCVars->g_telemetrySampleRateSound > 0.0f)
				{
					m_soundTrackingBuffer = new CTelemetryBuffer(30*1024, m_telemetryCollector, sizeof(SSoundTelemetry));
				}

				if(g_pGameCVars->g_telemetrySampleRatePerformance > 0.0f)
				{
					m_performanceBuffer = new CTelemetryBuffer(60*1024, m_telemetryCollector, sizeof(SPerformanceTelemetry));
				}

				if(g_pGameCVars->g_telemetrySampleRateMemory > 0.0f)
				{
					m_memoryTrackingBuffer = new CTelemetryBuffer(30*1024, m_telemetryCollector, sizeof(SMemoryTelemetry));
				}
#endif //#if USE_TELEMETRY_BUFFERS
			}

			DRX_ASSERT( m_pMatchMakingTelemetry == NULL );

			//LAN mode won't produce useful results for matchmaking telemetry
			if( gEnv->pNetwork->GetLobby()->GetLobbyServiceType() == eCLS_Online )
			{
				m_pMatchMakingTelemetry = new CMatchmakingTelemetry();
			}

			// online attributes not enabled for singleplayer
			if(m_pPlayerProfileUpr)
			{
				m_pPlayerProfileUpr->EnableOnlineAttributes(false);
			}

			if (bWasMultiplayer)
			{
				// Restore shadow cvars (Multiplayer game modified these to improve performance)
				pGsmLodsNum->Set( m_iCachedGsmValue  );
				pGsmRange->Set( m_fCachedGsmRangeValue );
				pGsmRangeStep->Set( m_fCachedGsmRangeStepValue );
			}

			if(pAllowDisconnect)
			{
				pAllowDisconnect->Set(1);
			}
		}

		// in MP, certain modules are initialised after the game data patch is downlaoaded
		// in SP, they are initialized immediately here
		m_needsInitPatchables=true;
		InitPatchableModules(false);
	}
#endif

	m_pHitDeathReactionsSystem->OnToggleGameMode();

	m_gameTypeMultiplayer = multiplayer;
	m_gameTypeInitialized = true;

#if !defined(DEDICATED_SERVER)
	CFrontEndModelCache::UpdateNeed3dFrontEndAssets();

#endif

	if(gameTypeChanged)
	{
		GetIGameFramework()->GetIMaterialEffects()->LoadFXLibraries(); //Reloading FX libraries will re-parse for GAME="SP/MP"
	}

#if DRX_PLATFORM_WINDOWS && !defined(DEDICATED_SERVER)
	gEnv->pHardwareMouse->DecrementCounter();
#endif

	if( fromInit == false )
	{
		//Unload the mode switch pak, we don't need it hanging around. Pass in the resourcelist name in case we are saving it for the build system.
		if( multiplayer == true )
		{
			gEnv->pSystem->GetIResourceUpr()->UnloadModeSwitchPak( MODE_SWITCH_PAK_FILENAME, MODE_SWITCH_LIST_FILENAME"_mp", multiplayer );
		}
#if !defined(DEDICATED_SERVER)
		else
		{
			gEnv->pSystem->GetIResourceUpr()->UnloadModeSwitchPak( MODE_SWITCH_PAK_FILENAME, MODE_SWITCH_LIST_FILENAME"_sp", multiplayer );
		}
#endif
	}

	SAFE_DELETE(m_pMovingPlatformMgr);
	if (multiplayer)
		m_pMovingPlatformMgr = new CMovingPlatformMgr();

	// Set some game-mode specific rendering parameters
	if (gEnv->pRenderer)
	{
		const float hudSilFillStr = multiplayer ? 0.25f : 0.15f;
		gEnv->pRenderer->EF_SetPostEffectParam("HudSilhouettes_FillStr", hudSilFillStr, true);
	}
}

// called after any game data patch is downloaded to finish initializing the parts of MP that are patchable
void CGame::InitPatchableModules(
	bool							inIsMultiplayer)
{
	if (m_needsInitPatchables)
	{
		//load the 'run once' script
		if( inIsMultiplayer && gEnv->pScriptSystem->ExecuteFile( "Scripts/DataPatcher/RunOnce.lua", true, true ) )
		{
			SmartScriptTable pRunOnceScript;
			if( gEnv->pScriptSystem->GetGlobalValue( "RunOnce", pRunOnceScript ) )
			{
				//enable a lot of extra script binds
				m_pScriptBindProtected->Enable();

				//run the 'run once' scripts that let us fix/adjust anything we want
				HSCRIPTFUNCTION scriptFunction;
				if( pRunOnceScript->GetValue( "Execute", scriptFunction ) )
				{
					Script::Call( gEnv->pScriptSystem, scriptFunction, pRunOnceScript );
					gEnv->pScriptSystem->ReleaseFunc( scriptFunction );
				}

				//remove the added script binds
				m_pScriptBindProtected->Disable();

#if !defined(_RELEASE)
				if( m_pCVars->g_debugTestProtectedScripts )
				{
					//test that they are inaccessible
					if( pRunOnceScript->GetValue( "Test", scriptFunction ) )
					{
						Script::Call( gEnv->pScriptSystem, scriptFunction, pRunOnceScript );
						gEnv->pScriptSystem->ReleaseFunc( scriptFunction );
					}
				}
#endif //!defined(_RELEASE)
			}
		}

		if (inIsMultiplayer)
		{
			m_pPlayerProgression->ResetUnlocks();

			DRX_ASSERT_MESSAGE(m_pEquipmentLoadout==NULL,"double initialisation of equipment loadout - logic flow error");
			m_pEquipmentLoadout = new CEquipmentLoadout();

			if (CDataPatchDownloader *pDP=GetDataPatchDownloader())
			{
				pDP->ApplyCVarPatch();
			}
		}

		m_pPlayerProgression->PostInit();	//Needs to be after Skill Assessments are unlocked

		ReloadPlayerParamFiles();
	}

	m_needsInitPatchables=false;
}

void CGame::ReloadPlayerParamFiles()
{
	// to make the player params xml file patchable, reload it here. in SP also reload it, to ensure values don't leak from a patched MP session -> SP
#define PLAYER_PARAM_FILE										"Scripts/Entities/Actor/Parameters/Player_Params.xml"
#define PLAYER_ENTITY_CLASS_PARAMS_FILE			"Scripts/Entities/Actor/Parameters/Player_EntityClassParams.xml"

	IActorSystem* pActorSystem = m_pFramework->GetIActorSystem();
	XmlNodeRef playerParams = GetISystem()->LoadXmlFromFile(PLAYER_PARAM_FILE);
	if (playerParams)
	{
		pActorSystem->ScanXML(playerParams,PLAYER_PARAM_FILE);
	}

	XmlNodeRef entityClassParams = GetISystem()->LoadXmlFromFile(PLAYER_ENTITY_CLASS_PARAMS_FILE);
	if (entityClassParams)
	{
		pActorSystem->ScanXML(entityClassParams,PLAYER_ENTITY_CLASS_PARAMS_FILE);
	}
}

void CGame::SetDifficultyLevel(EDifficulty difficulty)
{
	CDebugAllowFileAccess ignoreInvalidFileAccess;

	assert(!gEnv->bMultiplayer);

	static tukk szDifficultyConfigs[eDifficulty_COUNT] =
	{
		"difficulty/normal.cfg",	// Default '0' for normal

		"difficulty/easy.cfg",
		"difficulty/normal.cfg",
		"difficulty/hard.cfg",
		"difficulty/delta.cfg",
    "difficulty/posthuman.cfg",
	};

	if (!gEnv->bMultiplayer)
	{
		if (difficulty < eDifficulty_Default || difficulty >= eDifficulty_COUNT)
			difficulty = eDifficulty_Default;

		tukk szConfig = szDifficultyConfigs[difficulty];

		CDifficultyConfigSink cvarSink("CGame::SetDifficultyLevel");
		DrxLog("CGame: Loading singleplayer difficulty config \'%s\'", szConfig);
		gEnv->pSystem->LoadConfiguration(szConfig, &cvarSink, eLoadConfigGame);

		g_pGameCVars->g_difficultyLevel = difficulty;

		if(CProfileOptions* pOptions = g_pGame->GetProfileOptions())
		{
			i32 currProfileDif = pOptions->GetOptionValueAsInt( "SP/Difficulty" );
			if (difficulty!=currProfileDif)
			{
				pOptions->SetOptionValue("SP/Difficulty", (i32)difficulty);
				pOptions->SaveProfile(); // IAN: I don't think we should save here
			}
		}

		// Difficulty changed, so now record the easiest ever selected
		// Don't want player to get achievement for say hard difficulty if even played a little at easy
		EDifficulty difficultyExcludingDefault = difficulty;
		if (difficultyExcludingDefault == eDifficulty_Default)
		{
			difficultyExcludingDefault = eDifficulty_Normal; // Need to just be Easy,normal,hard,supersolider to determine lowest
		}

		if (g_pGameCVars->g_difficultyLevelLowestPlayed == -1) // Only way to reset, set -1 when changing difficulty in menu
		{
			g_pGameCVars->g_difficultyLevelLowestPlayed = difficultyExcludingDefault;
		}
		else
		{
			if (difficultyExcludingDefault < g_pGameCVars->g_difficultyLevelLowestPlayed)
			{
				g_pGameCVars->g_difficultyLevelLowestPlayed = difficultyExcludingDefault;
			}
		}
	}
}




#if ENABLE_VISUAL_DEBUG_PROTOTYPE
void CGame::UpdateVisualDebug(float deltaT)
{
	if (m_VisualDebugSys)
		{
		m_VisualDebugSys->Update(deltaT);
		}
}
#endif // ENABLE_VISUAL_DEBUG_PROTOTYPE


void CGame::SetInviteAcceptedState(EInviteAcceptedState state)
{
	DrxLog("[Invite] SetInviteAcceptedState %d to %d", m_inviteAcceptedState, state);

	m_inviteAcceptedState = state;
}

void CGame::SetInviteData(EDrxLobbyService service, u32 user, DrxInviteID id, EDrxLobbyError error, EDrxLobbyInviteType inviteType)
{

	m_inviteAcceptedData.m_service = service;
	m_inviteAcceptedData.m_user = user;
	m_inviteAcceptedData.m_id = id;
	m_inviteAcceptedData.m_error = error;
	m_inviteAcceptedData.m_bannedFromSession = false;
	m_inviteAcceptedData.m_type = inviteType;
	m_inviteAcceptedData.m_failedToAcceptInviteAsNotSignedIn=false;

	if(m_pSquadUpr && error == eCLE_Success && (inviteType == eCLIT_InviteToSquad))
	{
		m_pSquadUpr->SetInvitePending(true);
	}
}

void CGame::InvalidateInviteData()
{
	DrxLog("[Invite] InvalidateInviteData");

	SetInviteData(eCLS_Online, 0, DrxInvalidInvite, eCLE_Success, eCLIT_InviteToSquad);

	if(m_pSquadUpr)
	{
		m_pSquadUpr->SetInvitePending(false);
	}

	CWarningsUpr* pWarningsUpr = GetWarnings();
	if(pWarningsUpr)
	{
		pWarningsUpr->RemoveGameWarning("InviteSelectController");
		pWarningsUpr->RemoveGameWarning("ConfirmInvite");
	}

	m_bLoggedInFromInvite = false;
}

void CGame::SetInviteUserFromPreviousControllerIndex()
{
	DrxLog("[Invite] SetInviteUserFromPreviousControllerIndex %d", m_previousInputControllerDeviceIndex);

	m_inviteAcceptedData.m_user = m_previousInputControllerDeviceIndex;
}

void CGame::UpdateInviteAcceptedState()
{
	if(GetInviteAcceptedState() != eIAS_None && m_inviteAcceptedData.m_error == eCLE_Success && m_inviteAcceptedData.m_id == DrxInvalidInvite )
	{
		DrxLog("[Invite] Join invite in progress, but session id is invalid, bailing...");

		SetInviteAcceptedState(eIAS_None);
		InvalidateInviteData();	// for safety
	}

	CWarningsUpr* pWarningsUpr = GetWarnings();
	switch(GetInviteAcceptedState())
	{
		case eIAS_None:
		{
			break;
		}

		case eIAS_Init:
		{
			m_inviteAcceptedData.m_failedToAcceptInviteAsNotSignedIn=false;
			SetInviteAcceptedState(eIAS_StartAcceptInvite);
			break;
		}

		case eIAS_ConfirmInvite:
		{
			// this isn't used anymore, but just in case it comes back, leaving here for now
			if(pWarningsUpr && !g_pGame->GetUI()->IsLoading())	// don't show dialog while in the middle of loading, wait until ingame
			{
				pWarningsUpr->RemoveGameWarning("ConfirmInvite");
				pWarningsUpr->AddGameWarning("ConfirmInvite");
				SetInviteAcceptedState(eIAS_WaitForInviteConfirmation);
			}
			break;
		}

		case eIAS_WaitForInviteConfirmation:
		{
			// warning return will push this onto the next state
			break;
		}

		case eIAS_StartAcceptInvite:
		{
			const bool bChangeUser = false;

			if(pWarningsUpr)
			{
				// invites are destructive acts that have to happen
				// (according to TCRs), we need to clear any active
				// warnings here as they are no longer relevant
				pWarningsUpr->CancelWarnings();
			}

			// If we're in singleplayer or we need to change user, do existing behaviour
			if ((gEnv->bMultiplayer == false) || bChangeUser)
			{
				if(m_pUIUpr && m_pUIUpr->IsLoading())
				{
					DrxLog("[Invite] Waiting for loading to finish");

					SetInviteAcceptedState(eIAS_WaitForLoadToFinish);
				}
				else if(GetIGameFramework()->StartedGameContext())
				{
					DrxLog("[Invite] Accepting invite from in-game");

					SetInviteAcceptedState(eIAS_DisconnectGame);
				}
				else if (m_pGameLobbyUpr->HaveActiveLobby())
				{
					DrxLog("[Invite] Accepting invite from in-lobby");

					SetInviteAcceptedState(eIAS_DisconnectLobby);
				}
				else
				{
					DrxLog("[Invite] Accepting invite from the menus");

					SetInviteAcceptedState(eIAS_InitSinglePlayer);
				}
			}
			else
			{
				if(m_inviteAcceptedData.m_error == eCLE_Success)
				{
					if (IsGameTypeFullyInitialised())
					{
						// theres a period during mp initialisation where
						// the squad manager is not enabled yet, we need to cope with that here
						SetInviteAcceptedState(eIAS_WaitForSquadUprEnabled);
					}
					else
					{
						SetInviteAcceptedState(eIAS_WaitForInitOnline);
					}
				}
				else
				{
					SetInviteAcceptedState(eIAS_Error);
				}
			}

			break;
		}

		case eIAS_WaitForLoadToFinish:
		{
			if(m_pUIUpr && !m_pUIUpr->IsLoading())
			{
				SetInviteAcceptedState(eIAS_DisconnectGame);	// finished loading, kill the game off
			}
			break;
		}

		case eIAS_DisconnectGame:
		{
			if ((m_inviteAcceptedData.m_error == eCLE_Success) || (m_inviteAcceptedData.m_error == eCLE_UserNotSignedIn))
			{
				SetInviteAcceptedState(eIAS_WaitForSessionDelete);
			}
			else
			{
				DrxLog("[invite] trying to disconnect game for invite, but invite was retrieved with error %d", m_inviteAcceptedData.m_error);

				// single player doesn't want disconnecting if the invite was retreived with error, if mp
				// and signed out, then should of already been returned to sp main menu anyways
				SetInviteAcceptedState(eIAS_Error);
			}

			break;
		}

		case eIAS_DisconnectLobby:
		{
			m_pGameLobbyUpr->LeaveGameSession(CGameLobbyUpr::eLSR_AcceptingInvite);
			SetInviteAcceptedState(eIAS_WaitForSessionDelete);
			break;
		}

		case eIAS_WaitForSessionDelete:
		{
			if (!m_pGameLobbyUpr->HaveActiveLobby())
			{
				if(m_pUIUpr && m_pUIUpr->IsInMenu())
				{
					SetInviteAcceptedState(eIAS_InitSinglePlayer);
				}
			}
			break;
		}

		case eIAS_InitSinglePlayer:
		{
			if (m_inviteAcceptedData.m_error == eCLE_UserNotSignedIn)
			{
				DrxLog("CGame::UpdateInviteAcceptedData() has found a user isn't signed in. We need to continue and switch to multiplayer, which after signing in should let us try accepting the invite again");
				m_inviteAcceptedData.m_failedToAcceptInviteAsNotSignedIn=true;
				m_inviteAcceptedData.m_error = eCLE_Success;	// we need to progress to switch to MP and sign in
			}

			if(m_inviteAcceptedData.m_error == eCLE_Success)
			{
				EInviteAcceptedState nextState = eIAS_WaitForInitSinglePlayer;

				if(gEnv->bMultiplayer)
				{
					if(HasExclusiveControllerIndex())	// demo will likely need this guard :(
					{
						DrxLog("[invite] initialise single player as we are accepting an invite as a different user");
					}
				}
#if !defined(DEDICATED_SERVER)
				else if(HasExclusiveControllerIndex())
				{
						if(GetExclusiveControllerDeviceIndex() == m_inviteAcceptedData.m_user)
						{
							DrxLog("[invite] accepting an invite with the current user in sp, switching to mp");

							nextState = eIAS_InitMultiplayer;
						}
						else
						{
							DrxLog("[invite] in single player with a different user, heading back to splashscreen");

						}
				}
#endif

				SetInviteAcceptedState(nextState);
			}
			else
			{
				DrxLog("[invite] trying to init singleplayer from invite, but invite was retrieved with error %d", m_inviteAcceptedData.m_error);
				SetInviteAcceptedState(eIAS_Error);
			}
			break;
		}

		case eIAS_WaitForInitSinglePlayer:
		{
			break;
		}

		case eIAS_WaitForSplashScreen:
		{
			if(m_gameDataInstalled)
			{
				if(m_inviteAcceptedData.m_error == eCLE_Success)
				{
					SetInviteAcceptedState(eIAS_WaitForValidUser);
				}
				else
				{
					DrxLog("[invite] trying to init singleplayer from invit, but invite was retrieved with error %d", m_inviteAcceptedData.m_error);

					SetInviteAcceptedState(eIAS_Error);
				}
			}
			break;
		}

		case eIAS_WaitForValidUser:
		{
			if(m_inviteAcceptedData.m_user != INVALID_CONTROLLER_INDEX)
			{
				SetPreviousExclusiveControllerDeviceIndex(m_inviteAcceptedData.m_user);	// set the controller
				SetInviteAcceptedState(eIAS_WaitForInitProfile);	// wait
			}
			break;
		}

		case eIAS_WaitForInitProfile:
		{

			if(m_bLoggedInFromInvite)	// not convinced this is needed anymore, we now just wait for main and don't progress until file writing is done
			{
				// wait until we reach the single player main menu
				{
					SetInviteAcceptedState(eIAS_InitMultiplayer);
				}
			}
			break;
		}

		case eIAS_InitMultiplayer:
		{
			if(!g_pGameCVars->g_multiplayerModeOnly)
			{
			}

			SetInviteAcceptedState(eIAS_WaitForInitMultiplayer);
			CGameLobby::SetLobbyService(eCLS_Online);
			break;
		}

		case eIAS_WaitForInitMultiplayer:
		{
			// need to wait for the multiplayer menu screen to actually load
			{
				if(gEnv->bMultiplayer) // :(
				{
					SetInviteAcceptedState(eIAS_InitOnline);
					if (m_inviteAcceptedData.m_type == eCLIT_InviteToSquad)
					{
						m_pSquadUpr->SetInvitePending(true);
					}
				}
			}
			break;
		}

		case eIAS_InitOnline:
		{
			SetInviteAcceptedState(eIAS_WaitForInitOnline);
			break;
		}

		case eIAS_WaitForInitOnline:
		{
			// MP Loader sets accept invite now
			break;
		}

		case eIAS_WaitForSquadUprEnabled:
		{
			DRX_ASSERT(gEnv->bMultiplayer);

			if(m_pSquadUpr->IsEnabled())
			{
				// if we're loading, then need to wait, it does not end well otherwise
				SetInviteAcceptedState(eIAS_Accept);
			}
			break;
		}

		case eIAS_Accept:
		{
			bool failedAsNotSignedIn = m_inviteAcceptedData.m_failedToAcceptInviteAsNotSignedIn;

			if (failedAsNotSignedIn)
			{
				DrxLog("CGame::UpdateInviteAcceptedState() state accept. yet we failed to accept as not signed in. We need to now get our invite from the lobbyUI if we can.");

				SetInviteAcceptedState(eIAS_None);
				InvalidateInviteData();
			}
			else
			{
				m_pSquadUpr->InviteAccepted(m_inviteAcceptedData.m_id);

				SetInviteAcceptedState(eIAS_None);
				InvalidateInviteData();
			}

			break;
		}

		case eIAS_Error:
		{
			if(pWarningsUpr)
			{
				pWarningsUpr->RemoveGameWarning("InviteNotSignedIn");
				pWarningsUpr->RemoveGameWarning("InviteInvalidRequest");

				if (m_inviteAcceptedData.m_bannedFromSession)
				{
					pWarningsUpr->AddGameWarning("BannedFromSquad");
				}
				else
				{
					switch(m_inviteAcceptedData.m_error)
					{
						case eCLE_UserNotSignedIn:	// user accepted the invite while in a signed out state
							pWarningsUpr->AddGameWarning("InviteNotSignedIn");
							break;

						case eCLE_InvalidInviteFriendData:
						case eCLE_InvalidJoinFriendData:
							pWarningsUpr->AddGameWarning("InviteInvalidRequest");
							break;

						default:
							DrxLog("[invite] unexpected error %d passed into invite data", m_inviteAcceptedData.m_error);
							break;
					}
				}

				InvalidateInviteData();
			}
			break;
		}

		default:
		{
			DrxLog("[Invite] unknown invite accepted state");
			break;
		}
	}
}

void CGame::UpdateSaveIcon()
{
	if (gEnv->IsDedicated())
		return;

	static const float MAX_ICON_DISPLAY_TIME = 10.0f; // TODO: Failsafe. If required, fine tune this to the game.
	float currentTime = gEnv->pTimer->GetFrameStartTime(ITimer::ETIMER_UI).GetSeconds();
	bool bTooLong = currentTime - m_saveIconTimer >= MAX_ICON_DISPLAY_TIME;
	bool bSetIcon = false;
	bool bEnableIcon = false;

	if(m_saveIconMode != eSIM_Off)
	{
		if(m_saveIconMode == eSIM_SaveStart)
		{
			bSetIcon = true;
			bEnableIcon = true;
			bTooLong = false;
			m_saveIconMode = eSIM_Saving;
			m_saveIconTimer = currentTime;
		}

		if(bTooLong || m_saveIconMode == eSIM_Finished)
		{
			if(bTooLong || currentTime - m_saveIconTimer >= IPlatformOS::MINIMUM_SAVE_ICON_DISPLAY_TIME) // must be visible for this long regardless of how long the write takes
			{
				bSetIcon = true;
				bEnableIcon = false;
				bTooLong = false;
				m_saveIconMode = eSIM_Off;
				m_saveIconTimer = currentTime;
			}
		}
	}
	else
	{
		// In some instances the event isn't caught due to the flash object not existing. Send the message every MAX_ICON_DISPLAY_TIME secs to ensure they go away.
		if(bTooLong)
		{
			bSetIcon = true;
			bEnableIcon = false;
			m_saveIconMode = eSIM_Off;
			m_saveIconTimer = currentTime;
		}
	}

	if(bSetIcon)
	{
		if(bEnableIcon && m_bCheckPointSave)
		{
			m_bCheckPointSave = false;
		}

		if(m_bUserHasPhysicalStorage) // Only show icon if there is physical storage
		{
			// Always set the icon state, since at certain points the show/hide may not work depending on if the flash object exists
			SHUDEvent hudEvent;
			hudEvent.eventType = eHUDEvent_OnFileIO;
			hudEvent.AddData(bEnableIcon);
			CHUDEventDispatcher::CallEvent(hudEvent);
		}
	}
}

//warning 6262 needs an investigation
i32 CGame::Update(bool haveFocus, u32 updateFlags) PREFAST_SUPPRESS_WARNING (6262)
{
	DrxProfile::ProfilerFrameStart(gEnv->nMainFrameID);

#if ENABLE_MANUAL_FRAME_STEP
	const bool shouldBlock = m_pManualFrameStep->Update();
	if (shouldBlock)
	{
		return 1;
	}
#endif

#if defined(USER_timf)
	if (m_needMultiplayerFrontEndAssets)
	{
		assert (gEnv->bMultiplayer);
		assert (m_allowedToLoadMultiplayerFrontEndAssets);
		DrxWatch ("%s: $3Multiplayer front-end assets are currently loaded", gEnv->bMultiplayer ? "MULTIPLAYER" : "SINGLE PLAYER");
	}
	else if (m_allowedToLoadMultiplayerFrontEndAssets)
	{
		DrxWatch ("%s: $6Loading of multiplayer front-end assets is allowed but assets are not currently loaded", gEnv->bMultiplayer ? "MULTIPLAYER" : "SINGLE PLAYER");
	}
	else
	{
		DrxWatch ("%s: $7Loading of front-end assets is not currently allowed", gEnv->bMultiplayer ? "MULTIPLAYER" : "SINGLE PLAYER");
	}
#endif

#if !defined(DEDICATED_SERVER)
	if (!gEnv->bMultiplayer && m_bDeferredSystemMenuPause)
	{
		m_bDeferredSystemMenuPause = false;
		if(m_bPausedForSystemMenu)
		{
			gEnv->pGame->GetIGameFramework()->PauseGame(true, false);
		}
		else if(!m_previousPausedGameState)
		{
			gEnv->pGame->GetIGameFramework()->PauseGame(false, false);
		}
	}
#endif

	if (gEnv->bMultiplayer)
	{
		static float s_startTime = gEnv->pTimer->GetAsyncTime().GetSeconds();
		const float nowTime = gEnv->pTimer->GetAsyncTime().GetSeconds();
		const float elapsedTime = nowTime - s_startTime;
		tukk const elapsedTimeMsg = GetTimeString(elapsedTime,true);
		SetCrashDebugMessage(elapsedTimeMsg);
		CGameLobby *pGameLobby = g_pGame->GetGameLobby();
		if (pGameLobby)
		{
			pGameLobby->UpdateDebugString();
		}
	}

	if(m_pDLCUpr)
	{
		m_pDLCUpr->Update();
	}

#if USE_LAGOMETER
	if (m_pLagOMeter)
	{
		m_pLagOMeter->Update();
	}
#endif

	bool bRun = m_pFramework->PreUpdate( true, updateFlags );

	float frameTime = gEnv->pTimer->GetFrameTime();

	if (m_pMovingPlatformMgr)
		m_pMovingPlatformMgr->Update(frameTime);

	if (gEnv->pRenderer)
	{
		m_colorGradientUpr->UpdateForThisFrame(frameTime);
	}

	{
		if (m_pRayCaster)
		{
			FRAME_PROFILER("GlobalRayCaster", gEnv->pSystem, PROFILE_AI);

			m_pRayCaster->SetQuota(g_pGameCVars->g_gameRayCastQuota);
			m_pRayCaster->Update(frameTime);
		}

		if (m_pIntersectionTester)
		{
			FRAME_PROFILER("GlobalIntersectionTester", gEnv->pSystem, PROFILE_AI);

			m_pIntersectionTester->SetQuota(g_pGameCVars->g_gameIntersectionTestQuota);
			m_pIntersectionTester->Update(frameTime);
		}

#ifdef ENABLE_PROFILING_CODE		// will be off in release build, but on in debug, profile and perf builds
		if (g_pGameCVars->g_telemetryDisplaySessionId)
		{
			ColorF color = Col_White;
			const float x0 = 60.0f;
			const float y0 = 30.0f;
			const float scale = 1.50f;

			stack_string text;
			CTelemetryCollector* const tc = static_cast<CTelemetryCollector*>(GetITelemetryCollector());
			if (tc)
			{
				if (tc->AreTransfersInProgress() && i32(gEnv->pTimer->GetAsyncCurTime()*2.0f)&1)
				{
					text.Format("SessionID: %s ** UPLOADING DATA ** ", tc->GetSessionId().c_str());
				}
				else
				{
					text.Format("SessionID: %s", tc->GetSessionId().c_str());
				}
			}
			else
			{
				text.Format("SessionID: NULL");
			}

			gEnv->pRenderer->Draw2dLabel( x0, y0, scale, &color.r, false, "%s", text.c_str());
		}
#endif

		if (g_pGameCVars->g_DebugDrawPhysicsAccess)
		{
			stack_string text;

			if (m_pRayCaster)
			{
				GlobalRayCaster::ContentionStats rstats = m_pRayCaster->GetContentionStats();
				text.Format(
					"RayCaster\n"
					"---------\n"
					"Quota: %d\n"
					"Queue Size: %d / %d\n"
					"Immediate Count: %d / %d\n"
					"Immediate Average: %.1f\n"
					"Deferred Count: %d / %d\n"
					"Deferred Average: %.1f",
					rstats.quota,
					rstats.queueSize,
					rstats.peakQueueSize,
					rstats.immediateCount,
					rstats.peakImmediateCount,
					rstats.immediateAverage,
					rstats.deferredCount,
					rstats.peakDeferredCount,
					rstats.deferredAverage);

				bool warning = (rstats.immediateCount + rstats.deferredCount) > rstats.quota;
				warning = warning || (rstats.immediateAverage + rstats.deferredAverage) > rstats.quota;
				warning = warning || rstats.queueSize > (3 * rstats.quota);

				ColorF color = warning ? Col_Red : Col_DarkTurquoise;
				gEnv->pRenderer->Draw2dLabel(400.f, 40.f, 1.25f, &color.r, false, "%s", text.c_str());
			}

			if (m_pIntersectionTester)
			{
				GlobalIntersectionTester::ContentionStats istats = m_pIntersectionTester->GetContentionStats();
				text.Format(
					"IntersectionTester\n"
					"------------------\n"
					"Quota: %d\n"
					"Queue Size: %d / %d\n"
					"Immediate Count: %d / %d\n"
					"Immediate Average: %.1f\n"
					"Deferred Count: %d / %d\n"
					"Deferred Average: %.1f",
					istats.quota,
					istats.queueSize,
					istats.peakQueueSize,
					istats.immediateCount,
					istats.peakImmediateCount,
					istats.immediateAverage,
					istats.deferredCount,
					istats.peakDeferredCount,
					istats.deferredAverage);

				bool warning = (istats.immediateCount + istats.deferredCount) > istats.quota;
				warning = warning || (istats.immediateAverage + istats.deferredAverage) > istats.quota;
				warning = warning || istats.queueSize > (3 * istats.quota);

				ColorF color = warning ? Col_Red : Col_DarkTurquoise;
				gEnv->pRenderer->Draw2dLabel(600.0, 745.0f, 1.25f, &color.r, false, "%s", text.c_str());
			}
		}
	}

	UpdateInviteAcceptedState();

	float rpTime = m_updateRichPresenceTimer > 0.f ? m_updateRichPresenceTimer - frameTime : m_updateRichPresenceTimer;
	m_updateRichPresenceTimer = rpTime;

	if(m_desiredRichPresenceState != eRPS_none)
	{
		if(rpTime <= 0.f)
		{
			// at present the lobby ui is capable of running only
			// one task at a time, so it is possible for SetRichPresence
			// to fail. To counter this, we store the desired state and try again
			// each frame until we succeed
			if(SetRichPresence(m_desiredRichPresenceState))
			{
				m_desiredRichPresenceState = eRPS_none;
			}
		}
	}

	if(GetUserProfileChanged())
	{
#if !defined(DEDICATED_SERVER)
		if(!gEnv->bMultiplayer)
		{
			m_lastSaveGame.clear();
			SetUserProfileChanged(false);
		}
		else
#endif
		{
			IGameFramework *pFramework = GetIGameFramework();
			if(pFramework->StartedGameContext()) // use this to determine if we are still in a game
			{
				IActor *pActor = pFramework->GetClientActor();
				if(pActor) // the entity name can't be changed if we don't have it yet
				{
					IPlatformOS::TUserName tUserName;
					u32 userIndex = GetExclusiveControllerDeviceIndex();

					gEnv->pSystem->GetPlatformOS()->UserGetOnlineName(userIndex, tUserName);

					CGameRules *pGameRules = GetGameRules();
					if(pGameRules)
					{
						pGameRules->RenamePlayer(pActor, tUserName.c_str());
					}

					SetUserProfileChanged(false);
				}
			}
			else
			{
				SetUserProfileChanged(false);
			}
		}
	}

	if (m_pFramework->IsGamePaused() == false)
	{
		m_pWeaponSystem->Update(frameTime);

		m_pScreenEffects->Update(frameTime);

		m_pPlayerVisTable->Update(frameTime);

#if !defined(DEDICATED_SERVER)
		if (!gEnv->bMultiplayer)
		{
			if (m_pFramework->StartedGameContext())
				m_pGameAISystem->Update(frameTime);
		}
#endif

		m_pBurnEffectUpr->Update(frameTime);

		CItemAnimationDBAUpr& itemDbaUpr = m_pGameParametersStorage->GetItemResourceCache().Get1pDBAUpr();
		CItemPrefetchCHRUpr& itemPfUpr = m_pGameParametersStorage->GetItemResourceCache().GetPrefetchCHRUpr();

		float fCurrTime = gEnv->pTimer->GetCurrTime();
		itemDbaUpr.Update(fCurrTime);
		itemDbaUpr.Debug();

		itemPfUpr.Update(fCurrTime);

		m_pGameCache->Debug();
	}
	else
	{
		if (m_hostMigrationState == eHMS_WaitingForPlayers)
		{
			if (gEnv->bServer)
			{
				if (GetRemainingHostMigrationTimeoutTime() <= 0.f)
				{
					DrxLog("CGame: HostMigration timeout reached");
					SetHostMigrationState(eHMS_Resuming);
				}
			}
		}
		else if (m_hostMigrationState == eHMS_Resuming)
		{
			const float curTime = gEnv->pTimer->GetAsyncCurTime();
			const float timePassed = curTime - m_hostMigrationTimeStateChanged;
			const float timeRemaining = g_pGameCVars->g_hostMigrationResumeTime - timePassed;
			if (timeRemaining > 0.f)
			{
				SHUDEvent resumingEvent(eHUDEvent_OnUpdateGameResumeMessage);
				i32 time = MAX(i32(floor(timeRemaining + 0.5f)), 0);
				resumingEvent.AddData(time);
				CHUDEventDispatcher::CallEvent(resumingEvent);
			}
			else
			{
				SetHostMigrationState(eHMS_NotMigrating);
			}
		}
	}

	// Some effects need to be updated when game is paused, so use UI frame time to update Game Effects System
	float frameTimeNoPause = gEnv->pTimer->GetFrameTime(ITimer::ETIMER_UI);
	GAME_FX_SYSTEM.Update(frameTimeNoPause);

	UpdateSaveIcon();



	m_gameMechanismUpr->Update(frameTime);

	if (m_pRecordingSystem)
	{
		m_pRecordingSystem->Update(frameTime);
	}

	m_pPersistantStats->Update(frameTime);

	if( m_pPlaylistActivityTracker )
	{
		m_pPlaylistActivityTracker->Update(frameTime);
	}

#if !defined(_RELEASE)
	m_pRemoteUpdateListener->Update();
#endif

	if (gEnv->IsClient() && !gEnv->bServer && g_pGameCVars->sv_pacifist)
	{
		gEnv->pRenderer->Draw2dLabel(10, 10, 4, Col_White, false, "PACIFIST MODE ENABLED (Actors don't get damage)");
	}

	m_pFramework->PostUpdate( true, updateFlags );

	//--- Moved this from the earlier is not paused block to it stop querying joint positions mid-update.
	//--- Ideally this should not be a problem & then this can move back where it was.
	if (m_pFramework->IsGamePaused() == false)
	{
		m_pAutoAimUpr->Update(frameTime);
	}

	if (m_pGameBrowser)
	{
		m_pGameBrowser->Update(frameTime);
	}

	if (m_pRecordingSystem)
	{
		m_pRecordingSystem->PostUpdate();
	}

	{
		//Beni - For some reason, radial blur FX parameters have to be updated here (not during update)
		m_pScreenEffects->PostUpdate(frameTime);
	}

	if(m_inDevMode != gEnv->pSystem->IsDevMode())
	{
		m_inDevMode = gEnv->pSystem->IsDevMode();
	}

	IActionMapUpr *pAMM = m_pFramework->GetIActionMapUpr();
	DRX_ASSERT(pAMM);
	IActionMap *pAM = pAMM->GetActionMap("debug");
	if (pAM && pAM->Enabled() != m_inDevMode)
	{
		pAM->Enable(m_inDevMode);
	}

	CheckReloadLevel();

#ifndef _RELEASE
	if(gEnv->bMultiplayer)
	{
		CPlayerProgression::GetInstance()->UpdateDebug();
	}
#endif //#ifndef _RELEASE


#if NET_PROFILE_ENABLE
	if( gEnv->bMultiplayer && !gEnv->IsEditor() && (gEnv->pConsole->GetCVar("net_profile_logging")->GetIVal() != 0) && m_telemetryCollector)
	{
		static ICVar *pName = gEnv->pConsole->GetCVar("net_profile_logname");
		static CTimeValue s_timeValue = gEnv->pTimer->GetAsyncCurTime();
		CTimeValue timeValue = gEnv->pTimer->GetAsyncCurTime();

		if (timeValue - s_timeValue >= CTimeValue(10.f))
		{
			DrxFixedStringT<1024> filename;
			tukk fullFilename = pName->GetString();
			tukk forwardSlash = strrchr(fullFilename, '/');
			tukk backSlash = strrchr(fullFilename, '\\');
			tukk lastSlash = (forwardSlash > backSlash) ? forwardSlash : backSlash;
			lastSlash = (lastSlash) ? lastSlash : fullFilename;
			filename.Format("./%s", lastSlash);
			m_telemetryCollector->SubmitFile(filename.c_str(), filename.c_str());
			s_timeValue = timeValue;
		}
	}
#endif // #if NET_PROFILE_ENABLE

#if USE_TELEMETRY_BUFFERS
	if(!gEnv->IsEditor() && (m_performanceBuffer || m_bandwidthBuffer || m_memoryTrackingBuffer || m_soundTrackingBuffer))
	{
		CGameRules *cgr = GetGameRules();
		if (cgr)
		{
			const CTimeValue timeValue = gEnv->pTimer->GetAsyncTime();

			CTimeValue deltaTime;
			const float serverTimeInSeconds = cgr->GetServerTime() / 1000.0f;
			const bool updatePerfTelemetry = g_pGameCVars->g_telemetry_onlyInGame ? (cgr->IsGameInProgress() && serverTimeInSeconds >= 0.1f) : true;
			const bool updateBandwidthTelemetry = updatePerfTelemetry;

			// performance
			if(m_performanceBuffer && updatePerfTelemetry)
			{
				static float s_gpuTime = 0.f;
				static i32 s_gpuLimited = 0;
				static i32 framerateFrameCount = 0;
				static i32 s_drawcallCount = 0;
				static i32 s_drawcallOverbudget = 0;
				static float s_renderThreadTime = 0.0f;
				static float s_waitForRenderTime= 0.0f;
				static float s_waitForGPUTime= 0.0f;
				IF_UNLIKELY(m_secondTimePerformance.GetValue() == 0)
				{
					// Reset counters to zero if this is the first update
					s_gpuTime = 0.f;
					s_gpuLimited = 0;
					framerateFrameCount = 0;
					s_drawcallCount = 0;
					s_drawcallOverbudget = 0;
					s_renderThreadTime = 0.0f;
					s_waitForRenderTime = 0.0f;
					s_waitForGPUTime = 0.0f;
					m_secondTimePerformance = timeValue;
				}
				else
				{
					framerateFrameCount++;

					//Render stats must be updated each frame
					IRenderer* pRenderer = gEnv->pRenderer;
					if(pRenderer)
					{
						IRenderer::SRenderTimes renderTimes;

						pRenderer->GetRenderTimes(renderTimes);

						const float fRenderThreadTime = renderTimes.fTimeProcessedRT;
						const float fWaitForRender = renderTimes.fWaitForRender;

						if (fRenderThreadTime>0.0f)
						{
							s_renderThreadTime += fRenderThreadTime;
						}
						if (fWaitForRender >0.0f)
						{
							s_waitForRenderTime += fWaitForRender;
						}

						const float gpuFrameTime = pRenderer->GetGPUFrameTime();

						if(gpuFrameTime>0.f)
						{
							s_gpuTime += gpuFrameTime;
						}
						s_waitForGPUTime += renderTimes.fWaitForGPU;

						//wait for GPU is never zero, using small epsilon to determine if GPU Limited
						const float GPU_EPSILON = 0.001f;
						s_gpuLimited += (renderTimes.fWaitForGPU>GPU_EPSILON) ? 1 : 0;

						i32 nGeneralDrawcalls = 0;
						i32 nShadowDrawcalls = 0;
						pRenderer->GetCurrentNumberOfDrawCalls(nGeneralDrawcalls, nShadowDrawcalls);
						i32k totalDrawcalls = nGeneralDrawcalls+nShadowDrawcalls;
						s_drawcallCount += totalDrawcalls;
						if (totalDrawcalls>g_pGameCVars->g_telemetry_drawcall_budget)
						{
							s_drawcallOverbudget++;
						}
					}
				}

				const float sampleTimePerformance = g_pGameCVars->g_telemetrySampleRatePerformance;
				deltaTime = timeValue - m_secondTimePerformance;
				if (deltaTime >= sampleTimePerformance)
				{
					SPerformanceTelemetry spt;
					spt.m_timeInSeconds = serverTimeInSeconds;
					spt.m_numTicks = framerateFrameCount;
					spt.m_gpuTime = s_gpuTime;
					spt.m_gpuLimited = s_gpuLimited;
					spt.m_drawcalls = s_drawcallCount;
					spt.m_drawcallOverbudget = s_drawcallOverbudget;
					spt.m_deltaTime = deltaTime.GetSeconds();
					spt.m_renderThreadTime = s_renderThreadTime;
					spt.m_mainThreadTime = spt.m_deltaTime - s_waitForRenderTime;
					spt.m_waitForGPUTime = s_waitForGPUTime;

					m_performanceBuffer->AddData(&spt);

					m_secondTimePerformance = timeValue;
					framerateFrameCount = 0;
					s_gpuTime = 0.f;
					s_gpuLimited = 0;
					s_drawcallCount = 0;
					s_drawcallOverbudget = 0;
					s_renderThreadTime = 0.0f;
					s_waitForRenderTime = 0.0f;
					s_waitForGPUTime = 0.0f;
				}
			}

			// bandwidth
			if(m_bandwidthBuffer && updateBandwidthTelemetry)
			{
				IF_UNLIKELY(m_secondTimeBandwidth.GetValue() == 0)
				{
					m_secondTimeBandwidth = timeValue;
				}

				const float sampleTimeBandwidth = g_pGameCVars->g_telemetrySampleRateBandwidth;
				deltaTime = timeValue - m_secondTimeBandwidth;
				if (deltaTime >= sampleTimeBandwidth)
				{
					INetwork* pNetwork = gEnv->pNetwork;
					SBandwidthStats stats;
					pNetwork->GetBandwidthStatistics(&stats);

					SBandwidthTelemetry sbt;
					sbt.m_timeInSeconds = serverTimeInSeconds;
					sbt.m_bandwidthReceived = stats.m_total.m_totalBandwidthRecvd;
					sbt.m_bandwidthSent = stats.m_total.m_totalBandwidthSent;
					sbt.m_packetsSent = stats.m_total.m_totalPacketsSent;
					sbt.m_deltaTime = deltaTime.GetSeconds();
					m_bandwidthBuffer->AddData(&sbt);

					m_secondTimeBandwidth = timeValue;
				}
			}

			// memory
			if(m_memoryTrackingBuffer)
			{
				IF_UNLIKELY(m_secondTimeMemory.GetValue() == 0)
				{
					m_secondTimeMemory = timeValue;
				}

				const float sampleTimeMemory = g_pGameCVars->g_telemetrySampleRateMemory;
				deltaTime = timeValue - m_secondTimeMemory;
				if (deltaTime >= sampleTimeMemory)
				{
					IMemoryUpr::SProcessMemInfo processMemInfo;
					GetISystem()->GetIMemoryUpr()->GetProcessMemInfo(processMemInfo);
					const float cpuMemUsedInMB = (float)(processMemInfo.PagefileUsage)/(1024.0f*1024.0f);
					const float gpuMemUsedInMB = 0.0f;

					SMemoryTelemetry memTelem;
					memTelem.m_timeInSeconds = serverTimeInSeconds;
					memTelem.m_cpuMemUsedInMB = cpuMemUsedInMB;
					memTelem.m_gpuMemUsedInMB = gpuMemUsedInMB;
					m_memoryTrackingBuffer->AddData(&memTelem);

					m_secondTimeMemory = timeValue;
				}
			}

			// sound
			REINST("needs verification!");
			/*if(m_soundTrackingBuffer)
			{
				IF_UNLIKELY(m_secondTimeSound.GetValue() == 0)
				{
					m_secondTimeSound = timeValue;
				}

				deltaTime = timeValue - m_secondTimeSound;
				if(deltaTime >= g_pGameCVars->g_telemetrySampleRateSound)
				{
					ISoundSystem_Extended* pExt = gEnv->pSoundSystem->GetInterfaceExtended();
					if(pExt)
					{
						SSoundTelemetry soundTelem;
						pExt->GetMemoryInfo(&soundTelem.m_soundInfo);
						m_soundTrackingBuffer->AddData(&soundTelem);
					}

					m_secondTimeSound = timeValue;
				}
			}*/
		}
	}
#endif //#if USE_TELEMETRY_BUFFERS

	if (m_telemetryCollector)
	{
		m_telemetryCollector->Update();
	}
	if (m_statsRecorder)
	{
		m_statsRecorder->Update(frameTime);
	}
	if (m_patchPakUpr)
	{
		m_patchPakUpr->Update(frameTime);
	}

	if (gEnv->pRenderer)
	{
		Stereo3D::Update(frameTime);
	}

	CommitDeferredKills();

#ifndef NO_LIVECREATE
	LiveCreateUpdate();
#endif

	DrxProfile::ProfilerFrameEnd(gEnv->nMainFrameID);

	return bRun ? 1 : 0;
}

void CGame::EditorResetGame(bool bStart)
{
	DRX_ASSERT(gEnv->IsEditor());

	if (bStart)
	{
		IActionMapUpr* pAM = m_pFramework->GetIActionMapUpr();
		if (pAM)
		{
			pAM->InitActionMaps(pAM->GetLoadFromXMLPath());
			pAM->EnableActionMap("player", true); // enable default movement action map
			pAM->EnableFilter(0, false); // disable all filters

			const bool bMultiplayer = gEnv->bMultiplayer;
			pAM->EnableActionMap("multiplayer", bMultiplayer);
			pAM->EnableActionMap("singleplayer", !bMultiplayer);
		}

		// load & reset hud and related data.
		if (m_pUIUpr)
		{
			m_pUIUpr->ActivateDefaultState();
			SHUDEvent event;
			event.eventType = eHUDEvent_OnHUDReload;
			event.AddData( SHUDEventData(false));
			event.AddData( SHUDEventData(false) ); // Dynamically loaded
			CHUDEventDispatcher::CallEvent(event);
			m_pUIUpr->GetLayoutUpr()->UpdateHUDCanvasSize();
		}
	}
	else
	{
		gEnv->pConsole->ShowConsole(false);

		IActionMapUpr* pAM = m_pFramework->GetIActionMapUpr();
		if (pAM)
		{
			pAM->EnableActionMap(0, false); // disable all action maps
			pAM->EnableFilter(0, false); // disable all filters
		}
		GetMOSystem()->Reset();

		if (m_pTacticalUpr)
		{
			m_pTacticalUpr->ResetAllTacticalPoints();
			m_pTacticalUpr->ResetClassScanningData();
		}

		m_pBurnEffectUpr->Reset();

		m_pAutoAimUpr->OnEditorReset();

		if(CGameRules *gameRules = GetGameRules())
		{
			if(CCorpseUpr *corpseUpr = gameRules->GetCorpseUpr())
			{
				corpseUpr->ClearCorpses();
			}
		}

		if(CAICorpseUpr *aiCorpseUpr = CAICorpseUpr::GetInstance())
		{
			aiCorpseUpr->RemoveAllCorpses("CGame::EditorResetGame");
		}
	}

	if (m_pMovingPlatformMgr)
		m_pMovingPlatformMgr->Reset();
}

void CGame::PlayerIdSet(EntityId playerId)
{
	m_clientActorId = playerId;
}

void CGame::Shutdown()
{
#ifdef INCLUDE_GAME_AI_RECORDER
	assert(gEnv->bMultiplayer || m_pGameAISystem);
	if(m_pGameAISystem)
	{
		m_pGameAISystem->GetGameAIRecorder().Shutdown();
	}
#endif //INCLUDE_GAME_AI_RECORDER

	if (gEnv->IsEditor())
	{
		g_tacticalPointLanguageExtender.Deinitialize();
	}

	if (m_pPlayerProfileUpr)
	{
		m_pPlayerProfileUpr->LogoutUser(m_pPlayerProfileUpr->GetCurrentUser());
	}

	IVehicleSystem * pVehicleSystem = g_pGame->GetIGameFramework()->GetIVehicleSystem();
	IVehicleClient * pVehicleClient = pVehicleSystem->GetVehicleClient();
	pVehicleSystem->RegisterVehicleClient(NULL);

	//  manually update player vehicle client to NULL to avoid access violation in CPlayerInput::OnAction() when ALT+F4
	if (CPlayer * pPlayer = static_cast<CPlayer*>(m_pFramework->GetIActorSystem()->GetActor(m_pFramework->GetClientActorId())))
	{
		pPlayer->RegisterVehicleClient(NULL);
	}

	SAFE_DELETE(pVehicleClient);

	if (m_pPlaylistUpr)
	{
		SAFE_DELETE(m_pPlaylistUpr);
	}

	if (m_pUIUpr)
		m_pUIUpr->Shutdown();

	CBullet::StaticShutdown();

	CFrontEndModelCache::Allow3dFrontEndAssets(false, true);

	m_pFramework->ReleaseExtensions();

	this->~CGame();
}

tukk CGame::GetLongName()
{
	return GAME_LONGNAME;
}

tukk CGame::GetName()
{
	return GAME_NAME;
}

EPlatform CGame::GetPlatform() const
{
	EPlatform platform = ePlatform_Unknown;

#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO || DRX_PLATFORM_APPLE || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	platform = ePlatform_PC;
#elif DRX_PLATFORM_ORBIS
	platform = ePlatform_PS4;
#else
# error Unsupported Platform
#endif

	return platform;
}

void CGame::InitPlatformOS()
{
	static const char s_encryptionMagic[] = { 'C', 'R', 'Y', '3', 'S', 'D', 'K'  };    //2 for release, 1 for demo

	static const uint64 s_saveEncryptionKey[] = {
		0x000000000000000000, 0x000000000000000000, 0x000000000000000000, 0x000000000000000000,
		0x000000000000000000, 0x000000000000000000, 0x000000000000000000, 0x000000000000000000,
		0x000000000000000000, 0x000000000000000000, 0x000000000000000000, 0x000000000000000000,
		0x000000000000000000, 0x000000000000000000, 0x000000000000000000, 0x000000000000000000,
		0x000000000000000000, 0x000000000000000000, 0x000000000000000000, 0x000000000000000000,
		0x000000000000000000, 0x000000000000000000, 0x000000000000000000, 0x000000000000000000,
		0x000000000000000000, 0x000000000000000000, 0x000000000000000000, 0x000000000000000000,
		0x000000000000000000, 0x000000000000000000, 0x000000000000000000, 0x000000000000000000,
	};

	assert(GetISystem()->GetPlatformOS());
	GetISystem()->GetPlatformOS()->InitEncryptionKey(s_encryptionMagic, sizeof(s_encryptionMagic), (u8*)s_saveEncryptionKey, sizeof(s_saveEncryptionKey));
}

void CGame::OnPostUpdate(float fDeltaTime)
{
#if ENABLE_VISUAL_DEBUG_PROTOTYPE
	UpdateVisualDebug(fDeltaTime);
#endif // ENABLE_VISUAL_DEBUG_PROTOTYPE
}

void CGame::OnSaveGame(ISaveGame* pSaveGame)
{
	//ScopedSwitchToGlobalHeap useGlobalHeap;

	CPlayer *pPlayer = static_cast<CPlayer*>(GetIGameFramework()->GetClientActor());
	GetGameRules()->PlayerPosForRespawn(pPlayer, true);

	//save difficulty
	pSaveGame->AddMetadata("sp_difficulty", g_pGameCVars->g_difficultyLevel);
	pSaveGame->AddMetadata("sp_difficultylowestplayed", g_pGameCVars->g_difficultyLevelLowestPlayed);
	tukk levelName = g_pGame->GetIGameFramework()->GetLevelName();
	if(levelName && levelName[0])
	{
		pSaveGame->AddMetadata("sp_levelname", levelName);
	}

	//write file to profile
	if(m_pPlayerProfileUpr)
	{
		tukk saveGameFolder = m_pPlayerProfileUpr->GetSharedSaveGameFolder();
		const bool bSaveGameFolderShared = saveGameFolder && *saveGameFolder;
		tukk user = m_pPlayerProfileUpr->GetCurrentUser();
		if(IPlayerProfile *pProfile = m_pPlayerProfileUpr->GetCurrentProfile(user))
		{
			string filename(pSaveGame->GetFileName());
			DrxFixedStringT<128> profilename(pProfile->GetName());
			profilename+='_';
			filename = filename.substr(filename.rfind('/')+1);
			// strip profileName_ prefix
			if (bSaveGameFolderShared)
			{
				if(strnicmp(filename.c_str(), profilename.c_str(), profilename.length()) == 0)
					filename = filename.substr(profilename.length());
			}
			pProfile->SetAttribute("Singleplayer.LastSavedGame", filename);
		}

		m_bLastSaveDirty = true;
	}

	pSaveGame->AddMetadata("v_altitudeLimit", g_pGameCVars->pAltitudeLimitCVar->GetString());

	m_bLastSaveDirty = true;
}

void CGame::OnLoadGame(ILoadGame* pLoadGame)
{
	CHUDMissionObjectiveSystem* pMOSystem = GetMOSystem();
	if (pMOSystem)
	{
		pMOSystem->DeactivateObjectives( false );
	}

	// Ian: reload game side profile persistent stats to prevent exploits.
	IPlayerProfile *pProfile = m_pPlayerProfileUpr->GetCurrentProfile(m_pPlayerProfileUpr->GetCurrentUser());
	if(pProfile)
		m_pPlayerProfileUpr->ReloadProfile(pProfile, ePR_Game);

	bool isLastSaveGame = true;

#if DRX_PLATFORM_WINDOWS
	// Check to see if we're loading the last save game.
	string	saveGame = pLoadGame->GetFileName(), lastSaveGame = GetLastSaveGame();
	size_t	pos = saveGame.find_last_of("\\/");
	if(pos != string::npos)
	{
		++ pos;
		saveGame = saveGame.substr(pos, saveGame.length() - pos);
	}
	saveGame.MakeLower();
	lastSaveGame.MakeLower();
	isLastSaveGame = (saveGame == lastSaveGame);
#endif


	// dificulty level from the savegame now always overwrites the value in the user profile.
	{
		i32 difficulty = g_pGameCVars->g_difficultyLevel;
		pLoadGame->GetMetadata("sp_difficulty", difficulty);
		if (!isLastSaveGame) // to avoid exploits
			pLoadGame->GetMetadata("sp_difficultylowestplayed", g_pGameCVars->g_difficultyLevelLowestPlayed);
		SetDifficultyLevel((EDifficulty)difficulty);
		if(pProfile)
		{
			pProfile->SetAttribute("Singleplayer.LastSelectedDifficulty", difficulty);
			pProfile->SetAttribute("Option.g_difficultyLevel", difficulty);
			IPlayerProfileUpr::EProfileOperationResult result;
			m_pPlayerProfileUpr->SaveProfile(m_pPlayerProfileUpr->GetCurrentUser(), result, ePR_Options);
		}
	}

	// altitude limit
	tukk v_altitudeLimit =	pLoadGame->GetMetadata("v_altitudeLimit");
	if (v_altitudeLimit && *v_altitudeLimit)
	{
		g_pGameCVars->pAltitudeLimitCVar->ForceSet(v_altitudeLimit);
	}
	else
	{
		char buf[64];
		drx_sprintf(buf, "%g", g_pGameCVars->v_altitudeLimitDefault());
		g_pGameCVars->pAltitudeLimitCVar->ForceSet(buf);
	}
}


void CGame::OnSavegameFileLoadedInMemory( tukk pLevelName )
{
}

// All input is filtered through this function, if return true then other listeners will not recieve the input
bool CGame::OnInputEvent(const SInputEvent& inputEvent)
{
	bool isGamePadController = (inputEvent.deviceType==eIDT_Gamepad);

	if(isGamePadController)
	{
		// Store device index of controller providing input
		bool isConnectionChangeEvent = ((inputEvent.keyId == eKI_SYS_ConnectDevice) || (inputEvent.keyId == eKI_SYS_DisconnectDevice) ||
																		(inputEvent.keyId == eKI_XI_Connect) || (inputEvent.keyId == eKI_XI_Disconnect));
		if(isConnectionChangeEvent == false) // Only want to set device index when real input comes through, not device changes
		{
			m_previousInputControllerDeviceIndex = inputEvent.deviceIndex;
		}

		u32 myDeviceIndex = GetExclusiveControllerDeviceIndex();

		// If there is an exclusive controller, then ignore other controllers
		if(	m_hasExclusiveController && (inputEvent.deviceIndex != myDeviceIndex) )
		{
			return true; // Return true so that other listeners won't recieve this event
		}

#if DISABLE_FORCE_FEEDBACK_WHEN_USING_MOUSE_AND_KEYBOARD
		// On controller event re-enable force feedback for controller
		u32k forceFeedbackDeviceIndex = (gEnv->IsEditor()) ? inputEvent.deviceIndex : myDeviceIndex;
		gEnv->pInput->ForceFeedbackSetDeviceIndex(forceFeedbackDeviceIndex);
#endif

		if(m_hasExclusiveController)
		{
			// Add any game specific handling of controllers connecting/disconnecting here:-
			switch(inputEvent.keyId)
			{
				case eKI_SYS_ConnectDevice:
				{
					// Controller connected
					m_bExclusiveControllerConnected = true;
					break;
				}
				case eKI_SYS_DisconnectDevice:
				{
					// Controller disconnected
					m_bExclusiveControllerConnected = false;
					break;
				}
			}
		}
	}
#if DISABLE_FORCE_FEEDBACK_WHEN_USING_MOUSE_AND_KEYBOARD
	else if(inputEvent.deviceType==eIDT_Keyboard || inputEvent.deviceType==eIDT_Mouse)
	{
		// On keyboard/mouse event, disable Force Feedback for controller
		gEnv->pInput->ForceFeedbackSetDeviceIndex(EFF_INVALID_DEVICE_INDEX);
	}
#endif

	bool result = false;
	if (m_pInputEventListenerOverride)
	{
		result = m_pInputEventListenerOverride->OnInputEvent(inputEvent);
	}

	return result; // Return false for the other listeners to get this event
}


bool CGame::OnInputEventUI(const SUnicodeEvent& inputEvent)
{
	bool result = false;
	if (m_pInputEventListenerOverride)
	{
		result = m_pInputEventListenerOverride->OnInputEventUI(inputEvent);
	}

	return result; // Return false for the other listeners to get this event
}


void CGame::AddRenderSceneListener(IRenderSceneListener* pListener)
{
	stl::push_back_unique(m_renderSceneListeners, pListener);
}


void CGame::RemoveRenderSceneListener(IRenderSceneListener* pListener)
{
	stl::find_and_erase(m_renderSceneListeners, pListener);
}


void CGame::SetExclusiveControllerFromPreviousInput()
{
	DRX_ASSERT_MESSAGE(m_pPlayerProfileUpr, "No player profile manager, controller index will not be set, this will cause problems");

	if (m_hasExclusiveController)
	{
		DrxLog("CGame::SetExclusiveControllerFromPreviousInput() already have exclusive controller, ignoring");
		return;
	}

	const bool bChangeUser = m_pPlayerProfileUpr  ? (m_pPlayerProfileUpr->GetExclusiveControllerDeviceIndex() != m_previousInputControllerDeviceIndex) : false;

	if (bChangeUser)
	{
		LogoutCurrentUser(eLR_SetExclusiveController);
	}

	m_hasExclusiveController = true;
	m_bExclusiveControllerConnected = true;

	if (bChangeUser)
	{
		LoginUser(m_previousInputControllerDeviceIndex);

		m_pPlayerProfileUpr->SetExclusiveControllerDeviceIndex(m_previousInputControllerDeviceIndex);
		gEnv->pInput->ForceFeedbackSetDeviceIndex(m_previousInputControllerDeviceIndex);

		GetISystem()->GetPlatformOS()->UserSelectStorageDevice(m_previousInputControllerDeviceIndex);
	}
}

u32 CGame::GetExclusiveControllerDeviceIndex() const
{
	return m_pPlayerProfileUpr ? m_pPlayerProfileUpr->GetExclusiveControllerDeviceIndex() : 0	;
}

void CGame::RemoveExclusiveController()
{
	DrxLog("CGame::RemoveExclusiveController");

	bool hasExclusiveController = m_hasExclusiveController;
	m_hasExclusiveController = false;
	m_bExclusiveControllerConnected = false;

	m_pPlayerProfileUpr->SetExclusiveControllerDeviceIndex(INVALID_CONTROLLER_INDEX);
	gEnv->pInput->ForceFeedbackSetDeviceIndex(EFF_INVALID_DEVICE_INDEX);

	// Disable save icon, it may have been playing and can get stuck on
	{
		m_saveIconMode = eSIM_Off;
		m_saveIconTimer = 0.0f;

		SHUDEvent hudEvent;
		hudEvent.eventType = eHUDEvent_OnFileIO;
		hudEvent.AddData(false);
		CHUDEventDispatcher::CallEvent(hudEvent);
	}
}

bool CGame::SetControllerLayouts(tukk szButtonLayoutName, tukk szStickLayoutName, bool bUpdateProfileData)
{
	if (szButtonLayoutName == NULL || strcmp(szButtonLayoutName, "") == 0)
		return false;

	if (szStickLayoutName == NULL || strcmp(szStickLayoutName, "") == 0)
		return false;

	IActionMapUpr* pActionMapUpr = GetIGameFramework()->GetIActionMapUpr();
	DRX_ASSERT(pActionMapUpr != NULL);

	DrxFixedStringT<32> layoutKeyName;
	bool bResult;
	layoutKeyName.Format("buttonlayout_%s", szButtonLayoutName);

	bResult = pActionMapUpr->LoadControllerLayoutFile(layoutKeyName);
	if (!bResult)
	{
		GameWarning("CGame::SetControllerLayouts: Failed to load controller layout: %s", layoutKeyName.c_str());
		return false;
	}

	layoutKeyName.Format("sticklayout_%s", szStickLayoutName);

	bResult = pActionMapUpr->LoadControllerLayoutFile(layoutKeyName);
	if (!bResult)
	{
		GameWarning("CGame::SetControllerLayout: Failed to load controller layout file: %s", layoutKeyName.c_str());
		return false;
	}

	if (bUpdateProfileData && GetProfileOptions())
	{
		GetProfileOptions()->SetOptionValue("ControllerButtonLayout", szButtonLayoutName);
		GetProfileOptions()->SetOptionValue("ControllerStickLayout", szStickLayoutName);
	}

	return true;
}

tukk CGame::GetControllerLayout(const EControllerLayout layoutType) const
{
	CProfileOptions* profileOptions = GetProfileOptions();

	if (!profileOptions)
		return NULL;

	if (layoutType == eControllerLayout_Button)
	{
		return profileOptions->GetOptionValue("ControllerButtonLayout");
	}
	else // Must be stick
	{
		return profileOptions->GetOptionValue("ControllerStickLayout");
	}
}

void CGame::OnActionEvent(const SActionEvent& event)
{
	switch(event.m_event)
  {
	case eAE_connectFailed:
		CCCPOINT(Net_ConnectFailed);
		break;
  case  eAE_channelDestroyed:
		CCCPOINT(Net_ChannelDestroyed);
		m_pBurnEffectUpr->Reset();
    GameChannelDestroyed(event.m_value == 1);
    break;
	case eAE_serverIp:
		CCCPOINT(Net_GetServerIp);
		break;
	case eAE_serverName:
		CCCPOINT(Net_GetServerName);
		break;
	case eAE_earlyPreUpdate:
		break;
	case eAE_disconnected:
		break;
	case eAE_unloadLevel:
		{
			MEMSTAT_LABEL_SCOPED("CGame::OnActionEvent(eAE_unloadLevel)");
			m_pGameCache->Reset();

			m_pGameParametersStorage->GetItemResourceCache().FlushCaches();
			m_pGameParametersStorage->ReleaseLevelResources();
			m_pPlayerVisTable->Reset();

			m_pBurnEffectUpr->Reset();
			if (m_pBodyDamageUpr)
				m_pBodyDamageUpr->FlushLevelResourcesCache();

			g_tacticalPointLanguageExtender.Reset();
			m_pScreenEffects->Reset();
			CStealthKill::CleanUp();
			CSpectacularKill::CleanUp();
			m_pMovementTransitionsSystem->Flush();
			CSmokeUpr::GetSmokeUpr()->ReleaseObstructionObjects();

			if (gEnv->pRenderer)
			{
				m_colorGradientUpr->Reset();
			}

			if( m_pGameAISystem )
			{
				m_pGameAISystem->Reset(false); // Going to lie about the unload here, and reset it for unload later...
			}
			m_pGameAudio->Reset();
			if (GetMOSystem())
			{
				GetMOSystem()->Reset();
			}
			if (!gEnv->IsEditor())
			{
				g_tacticalPointLanguageExtender.Deinitialize();
			}
			if (m_renderSceneListeners.empty())
				stl::free_container(m_renderSceneListeners);
			if (m_pDownloadMgr)
				m_pDownloadMgr->Reset();
			if (m_pRecordingSystem)
				m_pRecordingSystem->Reset();

			m_pLedgeUpr->Reset();

			m_pLightningGameEffect->ClearSparks();

			m_pWaterPuddleUpr->Reset();

			m_clientActorId = 0;

			if (m_pMovingPlatformMgr)
				m_pMovingPlatformMgr->Reset();
		}
		break;
	case eAE_postUnloadLevel:
		{
			MEMSTAT_LABEL_SCOPED("CGame::OnActionEvent(eAE_postUnloadLevel)");
			if( m_pGameAISystem )
			{
				m_pGameAISystem->Reset(true);
			}
			CSchedulerActionPoolBase::ResetAll();
			CWeapon::StaticReset();
			if (m_statsRecorder)
			{
				m_statsRecorder->FlushData();
			}
		}
		break;
	case eAE_loadLevel:
		{
			MEMSTAT_LABEL_SCOPED("CGame::OnActionEvent(eAE_loadLevel)");
			if( m_pGameAISystem )
			{
				m_pGameAISystem->Reset(false);
				if (!gEnv->IsEditor())
				{
					g_tacticalPointLanguageExtender.Initialize();
				}
			}

			//Because of the code just right bellow (case eAE_inGame) we need to make sure to pre-cache the corresponding
			//resources for items tracked on persistant stats during level load
			for(i32 i = 0; i < s_numUnlockableAttachments; ++i)
			{
				i32k hasAttachment = m_pPersistantStats->GetStat(s_unlockableAttachmentNames[i], EMPS_AttachmentUnlocked);

#if !defined(RELEASE)
				bool bSavingResourceList = gEnv->pSystem->IsSavingResourceList();
#else
				bool bSavingResourceList = false;
#endif

				if (hasAttachment || bSavingResourceList)
				{
					CItemSharedParams* pItemParams = m_pGameParametersStorage->GetItemSharedParameters(s_unlockableAttachmentNames[i], false);
					IEntityClass* pItemClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(s_unlockableAttachmentNames[i]);
					if (pItemParams && pItemClass)
					{
						pItemParams->CacheResources(m_pGameParametersStorage->GetItemResourceCache(), pItemClass);
					}
				}
			}
		}
		break;
	case eAE_mapCmdIssued:
		if (!m_gameTypeInitialized || (g_pGameCVars->autotest_enabled && !!g_pGameCVars->g_multiplayerDefault != m_gameTypeMultiplayer))
		{
			InitGameType(g_pGameCVars->g_multiplayerDefault!=0, !m_gameTypeInitialized);
		}
		if (gEnv->bMultiplayer)
		{
			CGameLobby *pGameLobby = g_pGame->GetGameLobby();
			pGameLobby->OnMapCommandIssued();
		}
		break;
	case eAE_inGame:
		m_levelStartTime = gEnv->pTimer->GetFrameStartTime();
		AddPersistentAccessories();
		break;
	case eAE_preSaveGame:
		AddPersistentAccessories();
		break;
	case eAE_disconnectCommandFinished:
#if DRX_PLATFORM_DURANGO
		if (!g_pGame->GetGameLobby()->IsCurrentlyInSession())
		{
			EnsureSigninState();
		}
#endif
		break;
	}
}

void CGame::GameChannelDestroyed(bool isServer)
{
  if (!isServer)
  {
		if (!gEnv->pSystem->IsSerializingFile())
		{
			char buf[64];
			drx_sprintf(buf, "%g", g_pGameCVars->v_altitudeLimitDefault());
			g_pGameCVars->pAltitudeLimitCVar->ForceSet(buf);
		}
  }
}

CGameRules *CGame::GetGameRules() const
{
	return static_cast<CGameRules *>(m_pFramework->GetIGameRulesSystem()->GetCurrentGameRules());
}

bool CGame::IsLevelLoaded() const
{
	CGameRules* pGameRules = GetGameRules();
	return pGameRules ? pGameRules->IsLevelLoaded() : false;
}

#ifdef USE_LAPTOPUTIL
CLaptopUtil *CGame::GetLaptopUtil() const
{
	return m_pLaptopUtil;
}
#endif

CProfileOptions *CGame::GetProfileOptions() const
{
	return m_pUIUpr ? m_pUIUpr->GetOptions() : NULL;
}

CWarningsUpr *CGame::GetWarnings() const
{
	return m_pUIUpr ? m_pUIUpr->GetWarningUpr() : NULL;
}

CHUDMissionObjectiveSystem* CGame::GetMOSystem() const
{
	return m_pUIUpr ? m_pUIUpr->GetMOSystem() : NULL;
}


void CGame::LoadActionMaps(tukk filename)
{
	IActionMapUpr* pActionMapUpr = m_pFramework->GetIActionMapUpr();

	if (pActionMapUpr)
	{
		pActionMapUpr->RegisterActionMapEventListener(m_pGameActions);
		if (!pActionMapUpr->InitActionMaps(filename))
		{
			DrxFatalError("CGame::LoadActionMaps() Invalid action maps setup");
		}
	}
}

void CGame::InitScriptBinds()
{
	m_pScriptBindActor = new CScriptBind_Actor(m_pFramework->GetISystem());
	m_pScriptBindItem = new CScriptBind_Item(m_pFramework->GetISystem(), m_pFramework);
	m_pScriptBindWeapon = new CScriptBind_Weapon(m_pFramework->GetISystem(), m_pFramework);
	m_pScriptBindHUD = new CScriptBind_HUD(m_pFramework->GetISystem(), m_pFramework);
	m_pScriptBindGameRules = new CScriptBind_GameRules(m_pFramework->GetISystem(), m_pFramework);
	m_pScriptBindGame = new CScriptBind_Game(m_pFramework->GetISystem(), m_pFramework);
	m_pScriptBindHitDeathReactions = new CScriptBind_HitDeathReactions(m_pFramework->GetISystem(), m_pFramework);
	m_pScriptBindInteractiveObject = new CScriptBind_InteractiveObject(m_pFramework->GetISystem(), m_pFramework);
	m_pScriptBindBoids = new CScriptBind_Boids(m_pFramework->GetISystem());
	m_pScriptBindTurret = new CScriptBind_Turret(m_pFramework->GetISystem());
	m_pScriptBindProtected = new CScriptBind_ProtectedBinds( m_pFramework->GetISystem() );
	m_pScriptBindLightningArc = new CScriptBind_LightningArc(m_pFramework->GetISystem());

	ICVar* pEnableAI = gEnv->pConsole->GetCVar("sv_AISystem");
	if(!gEnv->bMultiplayer || (pEnableAI && pEnableAI->GetIVal()))
	{
		m_pScriptBindGameAI = new CScriptBind_GameAI(m_pFramework->GetISystem(), m_pFramework);
	}
	else
	{
		m_pScriptBindGameAI = NULL;
	}
}

void CGame::ReleaseScriptBinds()
{
	SAFE_DELETE(m_pScriptBindActor);
	SAFE_DELETE(m_pScriptBindItem);
	SAFE_DELETE(m_pScriptBindWeapon);
	SAFE_DELETE(m_pScriptBindHUD);
	SAFE_DELETE(m_pScriptBindGameRules);
	SAFE_DELETE(m_pScriptBindGame);
	SAFE_DELETE(m_pScriptBindInteractiveObject);
	SAFE_DELETE(m_pScriptBindHitDeathReactions);
	SAFE_DELETE(m_pScriptBindBoids);
	SAFE_DELETE(m_pScriptBindTurret);
	SAFE_DELETE(m_pScriptBindProtected);
}

void CGame::CheckReloadLevel()
{
	if(!m_bReload)
		return;

	m_bReload = false;

	if(gEnv->IsEditor() || gEnv->bMultiplayer)
	{
		return;
	}

	DrxFixedStringT<256> command;
	command.Format("map %s nb", m_pFramework->GetLevelName());
	gEnv->pConsole->ExecuteString(command.c_str());
}

void CGame::RegisterGameObjectEvents()
{
	IGameObjectSystem* pGOS = m_pFramework->GetIGameObjectSystem();

	pGOS->RegisterEvent(eCGE_OnShoot,"OnShoot");
	pGOS->RegisterEvent(eCGE_ActorRevive,"ActorRevive");
	pGOS->RegisterEvent(eCGE_VehicleDestroyed,"VehicleDestroyed");
	pGOS->RegisterEvent(eCGE_VehicleTransitionEnter,"VehicleTransitionEnter");
	pGOS->RegisterEvent(eCGE_VehicleTransitionExit,"VehicleTransitionExit");
	pGOS->RegisterEvent(eCGE_HUD_PDAMessage,"HUD_PDAMessage");
	pGOS->RegisterEvent(eCGE_HUD_TextMessage,"HUD_TextMessage");
	pGOS->RegisterEvent(eCGE_TextArea,"TextArea");
	pGOS->RegisterEvent(eCGE_HUD_Break,"HUD_Break");
	pGOS->RegisterEvent(eCGE_HUD_Reboot,"HUD_Reboot");
	pGOS->RegisterEvent(eCGE_InitiateAutoDestruction,"InitiateAutoDestruction");
	pGOS->RegisterEvent(eCGE_Event_Collapsing,"Event_Collapsing");
	pGOS->RegisterEvent(eCGE_Event_Collapsed,"Event_Collapsed");
	pGOS->RegisterEvent(eCGE_MultiplayerChatMessage,"MultiplayerChatMessage");
	pGOS->RegisterEvent(eCGE_ResetMovementController,"ResetMovementController");
	pGOS->RegisterEvent(eCGE_AnimateHands,"AnimateHands");
	pGOS->RegisterEvent(eCGE_EnablePhysicalCollider,"EnablePhysicalCollider");
	pGOS->RegisterEvent(eCGE_DisablePhysicalCollider,"DisablePhysicalCollider");
	pGOS->RegisterEvent(eCGE_SetTeam, "SetTeam");
	pGOS->RegisterEvent(eCGE_Launch, "Launch");
	// [*DavidR | 1/Sep/2009] CHECK: Can we put this on some HitDeathReaction
	// initialization code?
	pGOS->RegisterEvent(eCGE_ReactionEnd, "ReactionEnd");
	pGOS->RegisterEvent(eCGE_CoverTransitionEnter,"CoverTransitionEnter");
	pGOS->RegisterEvent(eCGE_CoverTransitionExit,"CoverTransitionExit");
	pGOS->RegisterEvent(eCGE_AllowStartTransitionEnter,"AllowStartTransitionEnter");
	pGOS->RegisterEvent(eCGE_AllowStartTransitionExit,"AllowStartTransitionExit");
	pGOS->RegisterEvent(eCGE_AllowStopTransitionEnter,"AllowStopTransitionEnter");
	pGOS->RegisterEvent(eCGE_AllowStopTransitionExit,"AllowStopTransitionExit");
	pGOS->RegisterEvent(eCGE_AllowDirectionChangeTransitionEnter,"AllowDirectionChangeTransitionEnter");
	pGOS->RegisterEvent(eCGE_AllowDirectionChangeTransitionExit,"AllowDirectionChangeTransitionExit");
	pGOS->RegisterEvent(eCGE_Ragdollize,"Ragdollize");
	pGOS->RegisterEvent(eCGE_ItemTakenFromCorpse, "ItemTakeFromCorpse");
}

void CGame::GetMemoryStatistics(IDrxSizer * s)
{
	s->AddObject( m_pGameAudio );
	s->AddObject( m_pUIUpr );
	s->AddObject( m_telemetryCollector );

	m_pWeaponSystem->GetMemoryStatistics(s);
	m_pScreenEffects->GetMemoryStatistics(s);

	s->Add(*m_pScriptBindActor);
	s->Add(*m_pScriptBindItem);
	s->Add(*m_pScriptBindWeapon);
	s->Add(*m_pScriptBindGameRules);
	s->Add(*m_pScriptBindGame);
	s->Add(*m_pScriptBindHUD);
	s->Add(*m_pScriptBindInteractiveObject);
	s->Add(*m_pScriptBindHitDeathReactions);
	s->Add(*m_pScriptBindBoids);
	s->Add(*m_pScriptBindTurret);
	s->Add(*m_pGameActions);

	m_pGameParametersStorage->GetMemoryStatistics(s);

	if (m_pPlayerProfileUpr)
	  m_pPlayerProfileUpr->GetMemoryStatistics(s);

	if (m_pHitDeathReactionsSystem)
		m_pHitDeathReactionsSystem->GetMemoryUsage(s);

	if (m_pBodyDamageUpr)
		m_pBodyDamageUpr->GetMemoryUsage(s);

	if (m_pMovementTransitionsSystem)
		m_pMovementTransitionsSystem->GetMemoryUsage(s);

	m_pGameCache->GetMemoryUsage(s);
}



void CGame::OnClearPlayerIds()
{
	// do nothing
}

void CGame::DumpMemInfo(tukk format, ...)
{
	DrxModuleMemoryInfo memInfo;
	DrxGetMemoryInfoForModule(&memInfo);

	va_list args;
	va_start(args,format);
	gEnv->pLog->LogV( ILog::eAlways,format,args );
	va_end(args);

	gEnv->pLog->LogWithType( ILog::eAlways, "Alloc=%" PRIu64 " kb  String=%" PRIu64 " kb  STL-alloc=%" PRIu64 " kb  STL-wasted=%" PRIu64 " kb", (memInfo.allocated - memInfo.freed) >> 10 , memInfo.DrxString_allocated >> 10, memInfo.STL_allocated >> 10 , memInfo.STL_wasted >> 10);
}

const string& CGame::GetLastSaveGame(string &levelName)
{
	if (m_pPlayerProfileUpr && (m_bLastSaveDirty || m_lastSaveGame.empty()))
	{
		IPlayerProfile* pProfile = m_pPlayerProfileUpr->GetCurrentProfile(m_pPlayerProfileUpr->GetCurrentUser());
		if (pProfile)
		{
			ISaveGameEnumeratorPtr pSGE = pProfile->CreateSaveGameEnumerator();
			ISaveGameEnumerator::SGameDescription desc;
			time_t curLatestTime = (time_t) 0;
			tukk lastSaveGame = "";
			i32k nSaveGames = pSGE->GetCount();

			for (i32 i=0; i<nSaveGames; ++i)
			{
				if (pSGE->GetDescription(i, desc))
				{
					if (desc.metaData.loadTime >= curLatestTime)
					{
						lastSaveGame = desc.name;
						curLatestTime = desc.metaData.loadTime;
						levelName = desc.metaData.levelName;
					}
				}
			}
			m_lastSaveGame = lastSaveGame;
		}

		m_bLastSaveDirty = false;
	}
	return m_lastSaveGame;
}

/*static */void CGame::ExpandTimeSeconds(i32 secs, i32& days, i32& hours, i32& minutes, i32& seconds)
{
	days  = secs / 86400;
	secs -= days * 86400;
	hours = secs / 3600;
	secs -= hours * 3600;
	minutes = secs / 60;
	seconds = secs - minutes * 60;
}

IGame::TSaveGameName CGame::CreateSaveGameName()
{
	//design wants to have different, more readable names for the savegames generated
	i32 id = 0;

	TSaveGameName saveGameName;
#if DRX_PLATFORM_DURANGO
	saveGameName = DRX_SAVEGAME_FILENAME;
#else
	//saves a running savegame id which is displayed with the savegame name
	if(IPlayerProfileUpr *m_pPlayerProfileUpr = gEnv->pGame->GetIGameFramework()->GetIPlayerProfileUpr())
	{
		tukk user = m_pPlayerProfileUpr->GetCurrentUser();
		if(IPlayerProfile *pProfile = m_pPlayerProfileUpr->GetCurrentProfile(user))
		{
			pProfile->GetAttribute("Singleplayer.SaveRunningID", id);
			pProfile->SetAttribute("Singleplayer.SaveRunningID", id+1);
		}
	}

	saveGameName = DRX_SAVEGAME_FILENAME;
	char buffer[16];
	itoa(id, buffer, 10);
	saveGameName.clear();
	if(id < 10)
		saveGameName += "0";
	saveGameName += buffer;
	saveGameName += "_";

	tukk levelName = GetIGameFramework()->GetLevelName();
	tukk mappedName = GetMappedLevelName(levelName);
	saveGameName += mappedName;

	saveGameName += "_";
	saveGameName += GetName();
	saveGameName += "_";
	string timeString;

	CTimeValue time = gEnv->pTimer->GetFrameStartTime() - m_levelStartTime;
	timeString.Format("%d", int_round(time.GetSeconds()));

	saveGameName += timeString;
#endif
	saveGameName += DRX_SAVEGAME_FILE_EXT;

	return saveGameName;
}

tukk CGame::GetMappedLevelName(tukk levelName) const
{
	TLevelMapMap::const_iterator iter = m_mapNames.find(CONST_TEMP_STRING(levelName));
	return (iter == m_mapNames.end()) ? levelName : iter->second.c_str();
}

void CGame::LoadMappedLevelNames( tukk xmlPath )
{
	//load user levelnames for ingame text and savegames
	XmlNodeRef lnames = GetISystem()->LoadXmlFromFile( xmlPath );
	if( lnames )
	{
		i32 num = lnames->getNumAttributes();
		tukk nameA, *nameB;
		for(i32 n = 0; n < num; ++n)
		{
			lnames->getAttributeByIndex(n, &nameA, &nameB);
			m_mapNames[string(nameA)] = string(nameB);
		}
	}
}

IGameStateRecorder* CGame::CreateGameStateRecorder(IGameplayListener* pL)
{
	//ScopedSwitchToGlobalHeap globalHeap;

	CGameStateRecorder* pGSP = new CGameStateRecorder();

	if(pGSP)
		pGSP->RegisterListener(pL);

	return (IGameStateRecorder*)pGSP;

}

Graphics::CColorGradientUpr& CGame::GetColorGradientUpr()
{
	return *m_colorGradientUpr;
}

CInteractiveObjectRegistry& CGame::GetInteractiveObjectsRegistry() const
{
	return m_pScriptBindInteractiveObject->GetObjectDataRegistry();
}

void CGame::ClearSessionTelemetry(void)
{
#if USE_TELEMETRY_BUFFERS
	m_secondTimePerformance = CTimeValue();
	m_secondTimeMemory = CTimeValue();
	m_secondTimeBandwidth = CTimeValue();
	m_secondTimeSound = CTimeValue();
	if (m_performanceBuffer)
	{
		m_performanceBuffer->Reset();
	}
	if (m_bandwidthBuffer)
	{
		m_bandwidthBuffer->Reset();
	}
	if (m_memoryTrackingBuffer)
	{
		m_memoryTrackingBuffer->Reset();
	}
	if(m_soundTrackingBuffer)
	{
		m_soundTrackingBuffer->Reset();
	}
#endif //#if USE_TELEMETRY_BUFFERS
}

//---------------------------------------
class BufferUtil
{
public:
	BufferUtil(i32 size)
		: m_pos(0)
		, m_size(size)
		, m_bufferOverflow(false)
	{
		m_pBuffer = new char[size];
	}

	~BufferUtil()
	{
		delete [] m_pBuffer;
	}

	template <class T>
	void Write(T &data)
	{
		SwapEndian(data, eBigEndian);		//swap to Big Endian

		if (m_pos + (i32)sizeof(T) <= m_size)
		{
			memcpy(m_pBuffer + m_pos, &data, sizeof(T));
			m_pos += sizeof(T);
		}
		else
		{
			m_bufferOverflow = true;
			DRX_ASSERT_MESSAGE(false, "Buffer size is not large enough");
		}

		SwapEndian(data, eBigEndian);	//swap back again
	}

	void WriteString(tukk string, i32 length)
	{
		// Write the length of the string followed by the string itself
		Write(length);
		if (m_pos + length <= m_size)
		{
			memcpy(m_pBuffer + m_pos, string, length);
			m_pos += length;
		}
		else
		{
			m_bufferOverflow = true;
			DRX_ASSERT_MESSAGE(false, "Buffer size is not large enough");
		}
	}

	template <class T, size_t S>
	void WriteString(DrxStackStringT<T, S> &string)
	{
		WriteString(string.c_str(), string.length());
	}

	tukk GetBuffer() { return m_pBuffer; }
	i32 GetUsedSize() { return m_pos; }
	bool Overflow() { return m_bufferOverflow; }

private:
	tuk m_pBuffer;
	i32 m_pos;
	i32 m_size;
	bool m_bufferOverflow;
};

//---------------------------------------
void CGame::UploadSessionTelemetry(void)
{
	if (m_telemetryCollector)
	{
		stack_string levelName;

		if (GetGameLobby() && gEnv->bMultiplayer)
		{
			levelName=GetGameLobby()->GetCurrentLevelName();
		}
		else if (m_pFramework)
		{
			levelName=m_pFramework->GetLevelName();
		}

		if (levelName.empty())
		{
			if (ILevelInfo* pLevelInfo = m_pFramework->GetILevelSystem()->GetCurrentLevel())
			{
				levelName = pLevelInfo->GetName();
			}
		}

		// strip the path off the beginning of the map for consistency with existing playtime.xml
		i32 pathOffset=levelName.rfind('/');
		if (pathOffset!=-1)
		{
			levelName=levelName.Right(levelName.length()-pathOffset-1);
		}

		if (levelName.empty())
		{
			levelName="unknown";
		}

		stack_string gameMode = "unknown";
		stack_string playerNames = "unknown";
		stack_string buildType = "unknown";
		i32 time = 0;
		CGameRules* pGameRules = GetGameRules();
		if (pGameRules)
		{
			gameMode = pGameRules->GetEntity()->GetClass()->GetName();
			gameMode.MakeLower();
			time = i32(pGameRules->GetCurrentGameTime() * 1000);
			const CGameRules::TDrxUserIdSet &users = pGameRules->GetParticipatingUsers();
			if (gEnv->bServer)
			{
				BufferUtil buffer(1024);
				i32 VERSION = 2;

				buffer.Write(VERSION);
				// Version 1 data
				buffer.WriteString(levelName);
				buffer.WriteString(gameMode);
				buffer.Write(time);

				i32 numUsers = users.size();
				buffer.Write(numUsers);
				CGameRules::TDrxUserIdSet::const_iterator itUser;
				for (itUser = users.begin(); itUser != users.end(); ++itUser)
				{
					DRX_ASSERT_MESSAGE(itUser->IsValid(), "DrxUserId is not valid");
					DrxFixedStringT<DRXLOBBY_USER_GUID_STRING_LENGTH> str = itUser->get()->GetGUIDAsString();
					buffer.WriteString(str);
				}
				// Version 2 additions
#if defined(PERFORMANCE_BUILD)
				buildType = "PERFORMANCE";
#elif defined(_RELEASE)
				buildType = "RELEASE";
#else
				buildType = "PROFILE";
#endif
				buffer.WriteString(buildType);

				if (!buffer.Overflow())
				{
					m_telemetryCollector->SubmitFromMemory("session_summary.bin",buffer.GetBuffer(),buffer.GetUsedSize(),CTelemetryCollector::k_tf_none);
				}
			}
		}

#if USE_TELEMETRY_BUFFERS
		if (pGameRules)
		{
			CGameRules::TPlayers players;
			pGameRules->GetPlayersClient(players);
			playerNames.clear();
			bool bFirst = true;
			for (CGameRules::TPlayers::const_iterator itPlayer=players.begin();itPlayer!=players.end(); ++itPlayer)
			{
				IEntity* pEntity = gEnv->pEntitySystem->GetEntity(*itPlayer);
				if (pEntity)
				{
					if (!bFirst)
					{
						playerNames.append(",");
					}
					playerNames.append(pEntity->GetName());
					bFirst = false;
				}
			}
		}

		string str;
		str.Format("<sheet><row map=\"%s\" gamemode=\"%s\" time=\"%d\" playerNames=\"%s\" /></sheet>",
			levelName.c_str(),gameMode.c_str(),time,playerNames.c_str());

		m_telemetryCollector->SubmitFromMemory("playtime.xml",str.c_str(),str.length(),CTelemetryCollector::k_tf_none);

		{
			//ScopedSwitchToGlobalHeap globalHeap;

			DrxFixedStringT<255> localFileName;

			if (m_performanceBuffer)
			{
				m_performanceBuffer->SubmitToServer("frametimes.log");
				localFileName.Format("%%USER%%/MiscTelemetry/%s_frametimes.log", m_telemetryCollector->GetSessionId().c_str() );
				m_performanceBuffer->DumpToFile(localFileName.c_str());
				m_performanceBuffer->Reset();
			}
			if (m_bandwidthBuffer)
			{
				m_bandwidthBuffer->SubmitToServer("bandwidth.log");
				localFileName.Format("%%USER%%/MiscTelemetry/%s_bandwidth.log", m_telemetryCollector->GetSessionId().c_str() );
				m_bandwidthBuffer->DumpToFile(localFileName.c_str());
				m_bandwidthBuffer->Reset();
			}
			if (m_memoryTrackingBuffer)
			{
				m_memoryTrackingBuffer->SubmitToServer("memory.log");
				localFileName.Format("%%USER%%/MiscTelemetry/%s_memory.log", m_telemetryCollector->GetSessionId().c_str() );
				m_memoryTrackingBuffer->DumpToFile(localFileName.c_str());
				m_memoryTrackingBuffer->Reset();
			}
			if(m_soundTrackingBuffer)
			{
				m_soundTrackingBuffer->SubmitToServer("sound.log");
				localFileName.Format("%%USER%%/MiscTelemetry/%s_sound.log", m_telemetryCollector->GetSessionId().c_str() );
				m_soundTrackingBuffer->DumpToFile(localFileName.c_str());
				m_soundTrackingBuffer->Reset();
			}
		}

		//Make summarystats.xml - one row for all the summary stats we output
		string row;
		str.clear();

		row.Format("<sheet>");
		str += row;
		row.Format("<row level=\"%s\"", levelName.c_str());
		str += row;

		if (gEnv->pRenderer)
		{
			//shader misses: global
			static i32		s_lastNumGlobalMisses = 0;

			SShaderCacheStatistics stats;
			gEnv->pRenderer->EF_Query(EFQ_GetShaderCacheInfo, stats);

			i32 globalMisses = stats.m_nGlobalShaderCacheMisses - s_lastNumGlobalMisses;

			row.Format(" numGlobalMisses=\"%d\"", globalMisses);
			str += row;

			s_lastNumGlobalMisses = stats.m_nGlobalShaderCacheMisses;
		}

		//maximum number of bound network objects
		INetwork* pNetwork = gEnv->pNetwork;
		SNetworkProfilingStats profileStats;
		pNetwork->GetProfilingStatistics(&profileStats);
		i32k maxNumNetBoundObjects = profileStats.m_maxBoundObjects;
		row.Format(" maxNumNetBoundObjects=\"%d\"",maxNumNetBoundObjects);
		str += row;

		row.Format(" />");
		str += row;
		row.Format("</sheet>");
		str += row;

		m_telemetryCollector->SubmitFromMemory("summarystats.xml",str.c_str(),str.length(),CTelemetryCollector::k_tf_none);
#endif // #if USE_TELEMETRY_BUFFERS
	}
}

void CGame::OnLevelEnd( tukk nextLevel )
{
	m_pBurnEffectUpr->Reset();
}

//------------------------------------------------------------------------
float CGame::GetTimeSinceHostMigrationStateChanged() const
{
	const float curTime = gEnv->pTimer->GetAsyncCurTime();
	const float timePassed = curTime - m_hostMigrationTimeStateChanged;
	return timePassed;
}

//------------------------------------------------------------------------
float CGame::GetRemainingHostMigrationTimeoutTime() const
{
	const float timePassed = GetTimeSinceHostMigrationStateChanged();
	const float timeRemaining = m_hostMigrationNetTimeoutLength - timePassed;
	return MAX(timeRemaining, 0.f);
}

//------------------------------------------------------------------------
float CGame::GetHostMigrationTimeTillResume() const
{
	float timeRemaining = 0.f;
	if (m_hostMigrationState == eHMS_WaitingForPlayers)
	{
		timeRemaining = GetRemainingHostMigrationTimeoutTime() + g_pGameCVars->g_hostMigrationResumeTime;
	}
	else if (m_hostMigrationState == eHMS_Resuming)
	{
		const float curTime = gEnv->pTimer->GetAsyncCurTime();
		const float timePassed = curTime - m_hostMigrationTimeStateChanged;
		timeRemaining = MAX(g_pGameCVars->g_hostMigrationResumeTime - timePassed, 0.f);
	}
	return timeRemaining;
}

//------------------------------------------------------------------------
void CGame::SetHostMigrationState(EHostMigrationState newState)
{
	float timeOfChange = gEnv->pTimer->GetAsyncCurTime();
	SetHostMigrationStateAndTime(newState, timeOfChange);
}

//------------------------------------------------------------------------
void CGame::SetHostMigrationStateAndTime( EHostMigrationState newState, float timeOfChange )
{
	DrxLog("CGame::SetHostMigrationState() state changing to '%i' (from '%i')", i32(newState), i32(m_hostMigrationState));

	if ((m_hostMigrationState == eHMS_NotMigrating) && (newState != eHMS_NotMigrating))
	{
		m_pFramework->PauseGame(true, false);
		g_pGameActions->FilterHostMigration()->Enable(true);

		ICVar *pTimeoutCVar = gEnv->pConsole->GetCVar("net_migrate_timeout");
		m_hostMigrationNetTimeoutLength = pTimeoutCVar->GetFVal();
		pTimeoutCVar->SetOnChangeCallback(OnHostMigrationNetTimeoutChanged);
	}

	m_hostMigrationState = newState;
	m_hostMigrationTimeStateChanged = timeOfChange;

	if (newState == eHMS_WaitingForPlayers)
	{
		SHUDEvent showHostMigration;
		showHostMigration.eventType = eHUDEvent_ShowHostMigrationScreen;
		CHUDEventDispatcher::CallEvent(showHostMigration);
	}
	else if (newState == eHMS_Resuming)
	{
		SHUDEvent hideHostMigration(eHUDEvent_HideHostMigrationScreen);
		CHUDEventDispatcher::CallEvent(hideHostMigration);
	}
	else if (newState == eHMS_NotMigrating)
	{
		AbortHostMigration();
	}

	// Notify the gamerules
	CGameRules *pGameRules = GetGameRules();
	pGameRules->OnHostMigrationStateChanged();
}

//------------------------------------------------------------------------
void CGame::AbortHostMigration()
{
	m_pFramework->PauseGame(false, false);
	m_hostMigrationState = eHMS_NotMigrating;
	m_hostMigrationTimeStateChanged = 0.f;
	ICVar *pTimeoutCVar = gEnv->pConsole->GetCVar("net_migrate_timeout");
	pTimeoutCVar->SetOnChangeCallback(NULL);
	g_pGameActions->FilterHostMigration()->Enable(false);
}

//------------------------------------------------------------------------
void CGame::OnHostMigrationNetTimeoutChanged(ICVar *pVar)
{
	g_pGame->m_hostMigrationNetTimeoutLength = pVar->GetFVal();
}

void CGame::LogoutCurrentUser(ELogoutReason reason)
{
#if DRX_PLATFORM_DURANGO
	// Everything falls to pieces due to startup flow requirements in FlashFrontEnd if we don't allow the language select screen in autotests
	//ICVar* pAutoTest = gEnv->pConsole->GetCVar("autotest_enabled");
	//if(pAutoTest && pAutoTest->GetIVal())
	//	return;

	string profileName = m_pPlayerProfileUpr->GetCurrentUser();

	IPlatformOS::TUserName userName;

	// For 360, we need to reactivate the profile in case the storage has changed,
	// or the user wants to play with a different save game on another storage device.
	// Ideally we should be able to call this for all platforms,
	// however it causes PC to fail since it assumes always signed in status.
	if(!GetISystem()->GetPlatformOS()->UserIsSignedIn(IPlatformOS::Unknown_User)) // there is an active user
	{
		m_pPlayerProfileUpr->LogoutUser(profileName);
	}

	/*
	if (reason == eLR_SetExclusiveController)
	{
		// Sign out this user from IPlatformOS to relinquish save game memory etc.
		u32 maxUsers = GetISystem()->GetPlatformOS()->UserGetMaximumSignedInUsers();
		for(u32 user = 0; user < maxUsers; ++user)
			if(GetISystem()->GetPlatformOS()->UserIsSignedIn(user))
				if(GetISystem()->GetPlatformOS()->UserGetName(user, userName))
				{
					if(strcmp(userName.c_str(), profileName.c_str()) == 0)
					{
						GetISystem()->GetPlatformOS()->UserSignOut(user);
					}
				}

		// Ensure default user is signed out if the controller index isn't actually signed in
		if(GetISystem()->GetPlatformOS()->UserIsSignedIn(IPlatformOS::Unknown_User))
			GetISystem()->GetPlatformOS()->UserSignOut(IPlatformOS::Unknown_User);
	}
	*/

	GetProfileOptions()->Init();
#endif // DRX_PLATFORM_DURANGO
}

void CGame::LoginUser(u32 user)
{
	bool bIsFirstTime = false;
	const bool bResetProfile = gEnv->pSystem->GetICmdLine()->FindArg(eCLAT_Pre,"ResetProfile") != 0;
	if (m_pPlayerProfileUpr)
	{
		IPlatformOS::TUserName tUserName;
		gEnv->pSystem->GetPlatformOS()->UserGetName(user, tUserName);
		tukk userName = tUserName.c_str();

		bool signedIn = false;
		bool ok = m_pPlayerProfileUpr->LoginUser(tUserName, bIsFirstTime);
		if (ok)
		{
			bool handled = false;

			// activate the always present profile "default"
			i32 profileCount = m_pPlayerProfileUpr->GetProfileCount(userName);
			if (profileCount > 0)
			{
				if(gEnv->IsDedicated())
				{
					for(i32 i = 0; i < profileCount; ++i )
					{
						IPlayerProfileUpr::SProfileDescription profDesc;
						ok = m_pPlayerProfileUpr->GetProfileInfo(userName, i, profDesc);
						if(ok)
						{
							const IPlayerProfile *preview = m_pPlayerProfileUpr->PreviewProfile(userName, profDesc.name);
							i32 iActive = 0;
							if(preview)
							{
								preview->GetAttribute("Activated",iActive);
							}
							if(iActive>0)
							{
								m_pPlayerProfileUpr->ActivateProfile(userName,profDesc.name);
								DrxLogAlways("[GameProfiles]: Successfully activated profile '%s' for user '%s'", profDesc.name, userName);
								m_pFramework->GetILevelSystem()->LoadRotation();
								handled = true;
								break;
							}
						}
					}
					m_pPlayerProfileUpr->PreviewProfile(userName,NULL);
				}

				if(!handled)
				{
					IPlayerProfileUpr::SProfileDescription desc;
					ok = m_pPlayerProfileUpr->GetProfileInfo(userName, 0, desc);
					if (ok)
					{
						time_t lastLoginTime;
						time(&lastLoginTime);
						m_pPlayerProfileUpr->SetProfileLastLoginTime(userName, 0, lastLoginTime);
						IPlayerProfile* pProfile = m_pPlayerProfileUpr->ActivateProfile(userName, desc.name);

						if (pProfile == 0)
						{
							GameWarning("[GameProfiles]: Cannot activate profile '%s' for user '%s'. Trying to re-create.", desc.name, userName);
							IPlayerProfileUpr::EProfileOperationResult profileResult;
							m_pPlayerProfileUpr->CreateProfile(userName, desc.name, true, profileResult); // override if present!
							pProfile = m_pPlayerProfileUpr->ActivateProfile(userName, desc.name);
							if (pProfile == 0)
							{
								GameWarning("[GameProfiles]: Cannot activate profile '%s' for user '%s'.", desc.name, userName);
							}
							else
							{
								GameWarning("[GameProfiles]: Successfully re-created profile '%s' for user '%s'.", desc.name, userName);
								signedIn = true;
							}
						}

						if (pProfile)
						{
							if (bResetProfile)
							{
								bIsFirstTime = true;
								pProfile->Reset();
								gEnv->pDrxPak->RemoveFile("%USER%/game.cfg");
								DrxLogAlways("[GameProfiles]: Successfully reset and activated profile '%s' for user '%s'", desc.name, userName);
							}

							// NOTE: temporary workaround, mark user device for force feedback, should get refactored at some point
							if (gEnv->pInput)
							{
								const IInputDevice* pDevice = gEnv->pInput->GetDevice(user, eIDT_Gamepad);
								if (pDevice)
								{
									m_pPlayerProfileUpr->SetExclusiveControllerDeviceIndex(user);
									gEnv->pInput->ForceFeedbackSetDeviceIndex(user);
								}
							}

							DrxLogAlways("[GameProfiles]: Successfully activated profile '%s' for user '%s'", desc.name, userName);
							signedIn = true;

							m_pFramework->GetILevelSystem()->LoadRotation();
						}
					}
					else
					{
						GameWarning("[GameProfiles]: Cannot get profile info for user '%s'", userName);
					}
				}
			}
			else
			{
				GameWarning("[GameProfiles]: User '%s' has no profiles", userName);
			}

			if(signedIn)
			{
				IPlayerProfileUpr *pPlayerProfileUpr = gEnv->pGame->GetIGameFramework()->GetIPlayerProfileUpr();
				if (pPlayerProfileUpr)
				{
					tukk currentUser = pPlayerProfileUpr->GetCurrentUser();
					IPlayerProfile* pProfile = pPlayerProfileUpr->GetCurrentProfile(currentUser);
					if (currentUser && pProfile)
					{
						tukk 	curUserName = pPlayerProfileUpr->GetCurrentProfile(currentUser)->GetUserId();
						tukk 	profileName = pPlayerProfileUpr->GetCurrentProfile(currentUser)->GetName();
						DrxLogAlways( "username %s signedIn %d userIndex %d", curUserName, signedIn, GetExclusiveControllerDeviceIndex() );

						DrxLogAlways( "ActivateProfile profileName %s", profileName );
						pPlayerProfileUpr->ActivateProfile(curUserName, profileName);
					}

					if(bIsFirstTime)
					{
						pProfile->LoadGamerProfileDefaults();

						if (m_pUIUpr)
						{
							m_pUIUpr->GetOptions()->InitializeFromCVar();
							m_pUIUpr->GetOptions()->SaveProfile();
							m_pPlayerProfileUpr->ReloadProfile(pProfile, ePR_Options);
						}
					}

					ICVar* pLanguageCVar = gEnv->pConsole->GetCVar("g_language");
					if(pLanguageCVar)
					{
						tukk currentLanguage = pLanguageCVar->GetString();
						//when starting the very first time, set subtitle option for czech and chineset to true, because they don't have localized audio
						if(m_pUIUpr && currentLanguage!=NULL && currentLanguage[0]!='\0' && (stricmp(currentLanguage, "chineset")==0 || stricmp(currentLanguage, "czech")==0))
						{
							i32 shouldCheck = m_pUIUpr->GetOptions()->GetOptionValueAsInt("NonLocalizedAudioSubtitleCheck");
							if(shouldCheck==0)
							{
								m_pUIUpr->GetOptions()->SetOptionValue("NonLocalizedAudioSubtitleCheck", 1);
								m_pUIUpr->GetOptions()->SetOptionValue("Subtitles", 1);
								m_pUIUpr->GetOptions()->SaveProfile();
							}
						}
					}


					m_bLoggedInFromInvite = (m_inviteAcceptedState == eIAS_WaitForInitProfile) ? true : false;
				}
			}

			//Cache the user region
			i32 userRegion = -1;
			if (IPlayerProfileUpr* pPlayerProfileUpr = gEnv->pGame->GetIGameFramework()->GetIPlayerProfileUpr())
			{
				tukk currentUserName = pPlayerProfileUpr->GetCurrentUser();

				IPlatformOS::SUserProfileVariant preference;
				IPlatformOS::TUserName curUserName = currentUserName;
				u32 curUser;
				if(gEnv->pSystem->GetPlatformOS()->UserIsSignedIn(curUserName, curUser) && curUser != IPlatformOS::Unknown_User)
				{
					if(gEnv->pSystem->GetPlatformOS()->GetUserProfilePreference(curUser, IPlatformOS::EUPP_REGION, preference))
					{
						userRegion = preference.GetInt();
					}
				}
			}
			SetUserRegion(userRegion);

			// Update controller layouts from profile
			tukk szControllerLayoutButton = GetControllerLayout(eControllerLayout_Button);
			tukk szControllerLayoutStick = GetControllerLayout(eControllerLayout_Stick);
			SetControllerLayouts(szControllerLayoutButton, szControllerLayoutStick, false);
		}
		else
			GameWarning("[GameProfiles]: Cannot login user '%s'", userName);
	}
	else
		GameWarning("[GameProfiles]: PlayerProfileUpr not available. Running without.");
}

void CGame::OnPlatformEvent(const IPlatformOS::SPlatformEvent& event)
{
	switch(event.m_eEventType)
	{
	case IPlatformOS::SPlatformEvent::eET_StorageMounted:
		{
			m_bUserHasPhysicalStorage = event.m_uParams.m_storageMounted.m_bPhysicalMedia;
			if(!event.m_uParams.m_storageMounted.m_bOnlyUpdateMediaState)
			{
				LoginUser(event.m_user);
			}
			break;
		}

	case IPlatformOS::SPlatformEvent::eET_SignIn:
		{
			{
				bool bIsExclusiveController = m_hasExclusiveController && GetExclusiveControllerDeviceIndex() == event.m_user;

				if(event.m_uParams.m_signIn.m_signedInState != IPlatformOS::SPlatformEvent::eSIS_NotSignedIn)
				{
				}
				else
				{
					// Handle sign-out
					if(event.m_uParams.m_signIn.m_previousSignedInState != IPlatformOS::SPlatformEvent::eSIS_NotSignedIn)
					{
						if(bIsExclusiveController && GetWarnings())
							GetWarnings()->CancelWarnings();

						assert(m_hasExclusiveController || event.m_user == IPlatformOS::Unknown_User);
						if(bIsExclusiveController || event.m_user == IPlatformOS::Unknown_User)
						{
							DrxLog("CGame::OnPlatformEvent signed out, isExclusive=%s, isUnknown=%s, bMultiplayer=%s", bIsExclusiveController ? "true" : "false", (event.m_user == IPlatformOS::Unknown_User) ? "true" : "false", gEnv->bMultiplayer ? "true" : "false");
#if !defined(DEDICATED_SERVER)
							if(bIsExclusiveController)
							{
								CErrorHandling *pErrorHandling = CErrorHandling::GetInstance();
								if (pErrorHandling)
								{
									pErrorHandling->OnFatalError(CErrorHandling::eFE_LocalSignedOut);
								}
								RemoveExclusiveController();
								m_bSignInOrOutEventOccured = true;
							}
#else
							// need to leave our game session too
							CGameLobbyUpr *pLobbyUpr = GetGameLobbyUpr();
							if(pLobbyUpr)
							{
								pLobbyUpr->LeaveGameSession(CGameLobbyUpr::eLSR_SignedOut);
							}
#endif
						}
					}
				}
			}

			// clear invite data if we have any
			if(event.m_uParams.m_signIn.m_signedInState == IPlatformOS::SPlatformEvent::eSIS_NotSignedIn)
			{
				// don't want to clear this if signing in
				if(m_inviteAcceptedData.m_id != DrxInvalidInvite)
				{
					// only clear the invite if the user signed out is the same
					// as the user accepting the invite
					bool isJoiningInvite = true;
					if(isJoiningInvite)
					{
						InvalidateInviteData();
					}
				}
			}
			break;
		}
	case IPlatformOS::SPlatformEvent::eET_StorageRemoved:
		DrxLog("CGame::OnPlatformEvent() eET_StorageRemoved deviceRemovedIsPrimary=%d", event.m_uParams.m_storageRemoved.m_bDeviceRemovedIsPrimary);
		if (event.m_uParams.m_storageRemoved.m_bDeviceRemovedIsPrimary && gEnv->bMultiplayer)
		{
			DrxLog("CGame::OnPlatformEvent() eET_StorageRemoved with the primary device being removed whilst in multiplayer.. we need to bail.");

			CErrorHandling *pErrorHandling = CErrorHandling::GetInstance();
			if (pErrorHandling)
			{
				pErrorHandling->OnFatalError(CErrorHandling::eFE_StorageRemoved);
			}
		}

		break;

	case IPlatformOS::SPlatformEvent::eET_FileError:
		{
			if(event.m_uParams.m_fileError.m_bRetry)
			{
				if(event.m_uParams.m_fileError.m_errorType & IPlatformOS::eFOC_ReadMask)
				{
					const string& file = GetLastSaveGame();
					if(!file.empty())
					{
						if(!GetIGameFramework()->LoadGame(file.c_str(), true))
							GetIGameFramework()->LoadGame(file.c_str(), false);
					}
				}
				else if(event.m_uParams.m_fileError.m_errorType & IPlatformOS::eFOC_WriteMask)
				{
					GetIGameFramework()->SaveGame(CreateSaveGameName().c_str());
				}
			}
			break;
		}

	case IPlatformOS::SPlatformEvent::eET_FileWrite:
		{
			if(event.m_uParams.m_fileWrite.m_type == IPlatformOS::SPlatformEvent::eFWT_CheckpointLevelStart
				|| event.m_uParams.m_fileWrite.m_type == IPlatformOS::SPlatformEvent::eFWT_Checkpoint)
			{
				m_bCheckPointSave = true;
			}

			// Don't display the icon immediately at level start since it happens during level load / precache and we'll end up breaking TCRs.
			// Instead wait for the SaveStart message to display the icon.
			if(event.m_uParams.m_fileWrite.m_type != IPlatformOS::SPlatformEvent::eFWT_CheckpointLevelStart)
			{
				if(event.m_uParams.m_fileWrite.m_type != IPlatformOS::SPlatformEvent::eFWT_SaveEnd)
				{
					if(m_saveIconMode != eSIM_Saving)
					{
						m_saveIconMode = eSIM_SaveStart;
						if(event.m_uParams.m_fileWrite.m_type == IPlatformOS::SPlatformEvent::eFWT_CreatingSave)
							m_bUserHasPhysicalStorage = true;
					}
				}
				else
				{
					m_saveIconMode = (event.m_uParams.m_fileWrite.m_type != IPlatformOS::SPlatformEvent::eFWT_SaveEnd) ? eSIM_SaveStart : eSIM_Finished;
				}
				if(DrxGetCurrentThreadId() == gEnv->mMainThreadId)
				{
					UpdateSaveIcon();
				}
			}
			break;
		}
	case IPlatformOS::SPlatformEvent::eET_SystemMenu:
		{
			const IPlatformOS::SPlatformEvent::UEventParams::SSysMenu& systemMenuEventData = event.m_uParams.m_systemMenu;
			if (systemMenuEventData.m_bOpened)
			{
#if !defined(DEDICATED_SERVER)
				if (!gEnv->bMultiplayer)
				{
					if(gEnv->pGame->GetIGameFramework()->StartedGameContext())
					{
						//check if we're opening the system menu while it's still closing and the eventual unpause hasn't happen yet
						if(!m_bDeferredSystemMenuPause)
						{
							m_previousPausedGameState = gEnv->pGame->GetIGameFramework()->IsGamePaused();
						}
						m_bPausedForSystemMenu = true;
						m_bDeferredSystemMenuPause = true;
					}
				}
#endif
				gEnv->pGame->GetIGameFramework()->GetIActionMapUpr()->Enable(false, true);
			}
			else if (systemMenuEventData.m_bClosed)
			{
#if !defined(DEDICATED_SERVER)
				if (!gEnv->bMultiplayer)
				{
					if(gEnv->pGame->GetIGameFramework()->IsGamePaused())
					{
						const bool bInIngameMenu = IsGameActive() && m_pUIUpr && m_pUIUpr->IsInMenu();
						if (m_bPausedForSystemMenu && !m_bPausedForControllerDisconnect && !bInIngameMenu) // Only unpause if all states are clear
						{
							m_bPausedForSystemMenu = false;
							m_bDeferredSystemMenuPause = true;
						}
					}
				}
#endif
				gEnv->pGame->GetIGameFramework()->GetIActionMapUpr()->Enable(true);
			}
		}
		break;
	}
}

void CGame::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
		case ESYSTEM_EVENT_SWITCHING_TO_LEVEL_HEAP:
		{
			assert (s_usingGlobalHeap);
			s_usingGlobalHeap = false;
			DrxLog ("Switched to level heap!");
			INDENT_LOG_DURING_SCOPE();
			CFrontEndModelCache::Allow3dFrontEndAssets(false,true);
		}
		break;

		case ESYSTEM_EVENT_LEVEL_LOAD_PREPARE:
		{
			DrxLog ("Preparing to load level!");
			INDENT_LOG_DURING_SCOPE();
			CFrontEndModelCache::Allow3dFrontEndAssets(false,true);
		}
		break;

		case ESYSTEM_EVENT_LEVEL_LOAD_START:
		{
			MEMSTAT_LABEL_SCOPED("CGame::OnSystemEvent(ESYSTEM_EVENT_LEVEL_LOAD_START)");
			DrxLog("CGame::OnSystemEvent(ESYSTEM_EVENT_LEVEL_LOAD_START) while in %s mode", m_gameTypeMultiplayer ? "multiplayer" : "single player");
			INDENT_LOG_DURING_SCOPE();

			m_pPersistantStats->RegisterLevelTimeListeners();
			if (!m_pRayCaster)
			{
				m_pRayCaster = new GlobalRayCaster;
				m_pRayCaster->SetQuota(g_pGameCVars->g_gameRayCastQuota);
			}
			if (!m_pIntersectionTester)
			{
				m_pIntersectionTester = new GlobalIntersectionTester;
				m_pIntersectionTester->SetQuota(g_pGameCVars->g_gameIntersectionTestQuota);
			}
			if (!m_pBodyDamageUpr)
			{
				m_pBodyDamageUpr = new CBodyDamageUpr();
			}
			if (!m_pGameActionHandlers)
			{
				m_pGameActionHandlers = new CGameInputActionHandlers;
			}

			m_pTacticalUpr->Init();

			if(gEnv->bMultiplayer && !m_pRecordingSystem)
			{
				MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, EMemStatContextFlags::MSF_None, "RecordingSystem");
				m_pRecordingSystem = new CRecordingSystem();
			}

			if( m_pScriptBindInteractiveObject )
			{
				m_pScriptBindInteractiveObject->GetObjectDataRegistry().Init();
			}
		}
		break;
	case ESYSTEM_EVENT_LEVEL_UNLOAD:
		{
			MEMSTAT_LABEL_SCOPED("CGame::OnSystemEvent(ESYSTEM_EVENT_LEVEL_UNLOAD)");
			m_pHitDeathReactionsSystem->Reset();
			m_pPlayerVisTable->Reset();

			SAFE_DELETE(m_pRayCaster);
			SAFE_DELETE(m_pIntersectionTester);
			SAFE_DELETE(m_pBodyDamageUpr);
			SAFE_DELETE(m_pGameActionHandlers);
			m_pWeaponSystem->FreePools();
			m_pTacticalUpr->Reset();

			CActorUpr::GetActorUpr()->Reset(false);
		  //	m_pPersistantStats->ResetCachedStats();
			CHUDEventDispatcher::FreeEventListeners();
			m_pPersistantStats->UnRegisterLevelTimeListeners();

			if( m_pScriptBindInteractiveObject )
			{
				m_pScriptBindInteractiveObject->GetObjectDataRegistry().Shutdown();
			}

			if (m_pMovingPlatformMgr)
				m_pMovingPlatformMgr->Reset();

			s_levelCVars.RevertCVarChanges();

			CornerSmoothing::OnLevelUnload();

			if (m_pGameCache)
			{
				m_pGameCache->Reset();
			}
		}
		break;

		case ESYSTEM_EVENT_LEVEL_POST_UNLOAD:
		{
			if (m_pParameterGameEffect)
			{
				m_pParameterGameEffect->Reset();
			}
			SAFE_DELETE(m_pRecordingSystem);
			if (s_usingGlobalHeap)
			{
				CFrontEndModelCache::Allow3dFrontEndAssets(true,false);
			}
		}
		break;

		case ESYSTEM_EVENT_SWITCHED_TO_GLOBAL_HEAP:
		{
			assert (!s_usingGlobalHeap);
			s_usingGlobalHeap = true;
			DrxLog ("Switched to global heap!");
			INDENT_LOG_DURING_SCOPE();
			CFrontEndModelCache::Allow3dFrontEndAssets(true,false);
		}
		break;
		case ESYSTEM_EVENT_TIME_OF_DAY_SET:
		{
			CGameRules* const pGameRules = GetGameRules();
			if (pGameRules)
			{
				pGameRules->OnTimeOfDaySet();
			}
			break;
		}
		case ESYSTEM_EVENT_EDITOR_GAME_MODE_CHANGED:
			{
				if (wparam != 0)
				{
					// Update audio environments on the player if dropping into game!
					IEntity* const pLocalPlayerEntity = gEnv->pEntitySystem->GetEntity(g_pGame->GetIGameFramework()->GetClientActorId());
/*
					if (pLocalPlayerEntity != NULL)
					{
						IEntityAudioProxy* const pIEntityAudioProxy = (IEntityAudioProxy*)pLocalPlayerEntity->GetProxy(ENTITY_PROXY_AUDIO);

						if (pIEntityAudioProxy != NULL)
						{
							pIEntityAudioProxy->SetCurrentEnvironments(INVALID_AUDIO_PROXY_ID);
						}
					}
					*/
				}

				break;
			}
#if DRX_PLATFORM_DURANGO
		case ESYSTEM_EVENT_CONTROLLER_REMOVED:
			{
				if(HasExclusiveControllerIndex())
				{
					i32 exclusiveDevIdx = GetExclusiveControllerDeviceIndex();
					if(exclusiveDevIdx == wparam)
					{
						CUIInput* pUIEvt = UIEvents::Get<CUIInput>();
						if (pUIEvt)
							pUIEvt->ExclusiveControllerDisconnected();
					}
				}
			}
			break;

#if defined(SUPPORT_DURANGO_LEGACY_MULTIPLAYER)
		case ESYSTEM_EVENT_ACTIVATION_EVENT:
			{
				// Always need to call this to ensure hookup of global even callbacks.
				MatchmakingUtils::OnActivated( 0 );
			}
			break;
#endif

		case ESYSTEM_EVENT_USER_CHANGED:

			{

				SUserXUID newXuid((wchar_t *)wparam);
				SUserXUID currentXuid;
				gEnv->pSystem->GetPlatformOS()->UserGetXUID(0, currentXuid);

				i32k newUserId = (u32)lparam;
				i32k currentUserId = gEnv->pSystem->GetPlatformOS()->UserGetId(0);

				const bool bSameUser = (newXuid == currentXuid);

				if (newUserId == 0)
				{
					// no user signed in
				}
				else if(!bSameUser)
				{
					m_userChangedDoSignOutAndIn = true;
					GetIGameFramework()->ExecuteCommandNextFrame("disconnect");
				}
			}
			break;

		case ESYSTEM_EVENT_DURANGO_CHANGE_VISIBILITY:
		case ESYSTEM_EVENT_CHANGE_FOCUS:
			{
				bool isVisible = wparam!=0;
				bool pause = !isVisible;

				if (pause)
				{
					m_wasGamePausedBeforePLMForcedPause = gEnv->pGame->GetIGameFramework()->IsGamePaused();
					if (!gEnv->pGame->GetIGameFramework()->IsGamePaused())
						gEnv->pGame->GetIGameFramework()->PauseGame(true, false);
				}
				else
				{
					if (!m_wasGamePausedBeforePLMForcedPause)
						gEnv->pGame->GetIGameFramework()->PauseGame(false, false);
					m_wasGamePausedBeforePLMForcedPause = false;
				}
			}
			break;

		case ESYSTEM_EVENT_PLM_ON_CONSTRAINED:
			{
				m_wasGamePausedBeforePLMForcedPause = gEnv->pGame->GetIGameFramework()->IsGamePaused();
				gEnv->pGame->GetIGameFramework()->PauseGame(true, false);
			}
			break;

		case ESYSTEM_EVENT_PLM_ON_RESUMING:
		case ESYSTEM_EVENT_PLM_ON_FULL:
			if (!m_wasGamePausedBeforePLMForcedPause)
			{
				gEnv->pGame->GetIGameFramework()->PauseGame(false, false);
			}
			m_wasGamePausedBeforePLMForcedPause = false;
			break;
#endif // DRX_PLATFORM_DURANGO
	}
}


/* static */
void CGame::SetRichPresenceCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg)
{
	DrxLog("[RichPresence] SetRichPresenceCallback - Rich presence %s with error code %d", error == eCLE_Success ? "succeeded" : "failed", error);

	bool setCurrentRichPresence = true;

	g_pGame->m_settingRichPresence = false;

	// so far, not supported on PC yet
	if(error != eCLE_Success)
	{
		if(error == eCLE_SystemIsBusy)
		{
			// if no new state pending, then try again with the previous state
			if(g_pGame->m_desiredRichPresenceState == eRPS_none)
			{
				DrxLog("  failed to set rich presence and no new state desired, retrying pending %d", g_pGame->m_pendingRichPresenceState);
				g_pGame->m_desiredRichPresenceState = g_pGame->m_pendingRichPresenceState;
			}

			g_pGame->m_updateRichPresenceTimer = g_pGameCVars->g_updateRichPresenceInterval;
			setCurrentRichPresence = false;
		}
		else
		{
			DrxLog("  cannot handle rich presence error, setting as complete for now");
		}
	}
	else
	{
		DrxLog("  successfully set rich presence");
	}

	if(setCurrentRichPresence)
	{
		DrxLog("  setting current rich presence");

		g_pGame->m_currentRichPresenceState = g_pGame->m_pendingRichPresenceState;
		g_pGame->m_currentRichPresenceSessionID = g_pGame->m_pendingRichPresenceSessionID;

		if(g_pGame->m_bRefreshRichPresence)
		{
			g_pGame->m_desiredRichPresenceState = (g_pGame->m_desiredRichPresenceState == eRPS_none) ? g_pGame->m_currentRichPresenceState : g_pGame->m_desiredRichPresenceState;
		}
	}

	g_pGame->m_pendingRichPresenceState = eRPS_none;
	g_pGame->m_pendingRichPresenceSessionID = DrxSessionInvalidID;
	g_pGame->m_bRefreshRichPresence = false;
}

bool CGame::SetRichPresence(ERichPresenceState state)
{
	// don't set rich presence if we don't have a controller yet
	if(!m_hasExclusiveController)
	{
		DrxLog("[RichPresence] not setting richpresence no player set yet");
		return true;
	}

	DrxSessionID sessionID = DrxSessionInvalidID;

	if((m_currentRichPresenceState == state) && GameNetworkUtils::CompareDrxSessionId(sessionID, m_currentRichPresenceSessionID))
	{
		if(state != eRPS_inGame || !gEnv->bMultiplayer)
		{
			DrxLog("[RichPresence] not setting richpresence state %d multiplayer %d", state, gEnv->bMultiplayer);
			return true;
		}
	}

	// we are already setting rich presence, so wait until that task
	// has finished
	if (m_settingRichPresence)
	{
		DrxLog("  current setting rich presence, setting desired state to %d", state);
		m_desiredRichPresenceState = state;
		return false;
	}

	IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
	IDrxLobbyService *pLobbyService = pLobby ? pLobby->GetLobbyService(eCLS_Online) : NULL;
	IDrxLobbyUI *pLobbyUI = pLobbyService ? pLobbyService->GetLobbyUI() : NULL;
	EDrxLobbyError error = eCLE_Success;

	m_pendingRichPresenceSessionID = sessionID;

	if(pLobbyUI)
	{
		u32 userIndex = GetExclusiveControllerDeviceIndex();

		DrxLog("[RichPresence] SetRichPresence %d userIndex %d", state, userIndex);

		switch(state)
		{
			case eRPS_idle:
			{
				SDrxLobbyUserData data;
				data.m_id = RICHPRESENCE_ID;
				data.m_type = eCLUDT_Int32;
				data.m_int32 = RICHPRESENCE_IDLE;
				error = pLobbyUI->SetRichPresence(userIndex, &data, 1, 0, CGame::SetRichPresenceCallback, this);
				break;
			}

			case eRPS_frontend:
			{
				SDrxLobbyUserData data;
				data.m_id = RICHPRESENCE_ID;
				data.m_type = eCLUDT_Int32;
				data.m_int32 = RICHPRESENCE_LOBBY; // TODO michiel: add richpresence frontend to spa?
				error = pLobbyUI->SetRichPresence(userIndex, &data, 1, 0, CGame::SetRichPresenceCallback, this);
				break;
			}

			case eRPS_lobby:
			{
				SDrxLobbyUserData data;
				data.m_id = RICHPRESENCE_ID;
				data.m_type = eCLUDT_Int32;
				data.m_int32 = RICHPRESENCE_LOBBY;
				error = pLobbyUI->SetRichPresence(userIndex, &data, 1, 0, CGame::SetRichPresenceCallback, this);
				break;
			}

			case eRPS_inGame:
			{
#if 0
				// Todo: MICHIEL add in-game presence to spa
				if(gEnv->bMultiplayer)
				{
					CGameRules *pGameRules = GetGameRules();
					tukk levelName = m_pFramework ? m_pFramework->GetLevelName() : NULL;
					tukk gameRulesName = pGameRules ? pGameRules->GetEntity()->GetClass()->GetName() : NULL;
					i32 gameMode = 0;
					i32 map = 0;

					if(levelName)
					{
						levelName = PathUtil::GetFileName(levelName);
						TRichPresenceMap::const_iterator iter = m_richPresence.find(levelName);
						map = (iter == m_richPresence.end()) ? 0 : iter->second;
					}

					if(gameRulesName)
					{
						TRichPresenceMap::const_iterator iter = m_richPresence.find(gameRulesName);
						gameMode = (iter == m_richPresence.end()) ? 0 : iter->second;
					}

					SDrxLobbyUserData data[eRPT_Max];
					data[eRPT_String].m_id = RICHPRESENCE_ID;
					data[eRPT_String].m_type = eCLUDT_Int32;
					data[eRPT_String].m_int32 = RICHPRESENCE_GAMEPLAY;
					data[eRPT_Param1].m_id = RICHPRESENCE_GAMEMODES;
					data[eRPT_Param1].m_type = eCLUDT_Int32;
					data[eRPT_Param1].m_int32 = gameMode;
					data[eRPT_Param2].m_id = RICHPRESENCE_MAPS;
					data[eRPT_Param2].m_type = eCLUDT_Int32;
					data[eRPT_Param2].m_int32 = map;

					error = pLobbyUI->SetRichPresence(userIndex, data, 3, 0, CGame::SetRichPresenceCallback, this);
#else
				if(gEnv->bMultiplayer)
				{
#endif
				}
#if !defined(DEDICATED_SERVER)
				else
				{
					SDrxLobbyUserData data;
					data.m_id = RICHPRESENCE_ID;
					data.m_type = eCLUDT_Int32;
					data.m_int32 = RICHPRESENCE_SINGLEPLAYER;

					error = pLobbyUI->SetRichPresence(userIndex, &data, 1, 0, CGame::SetRichPresenceCallback, this);
				}
#endif
				break;
			}
			default:
				DrxLog("[RichPresence] SetRichPresence - unknown rich presence %d", state);
				break;
		}

		if(error != eCLE_Success)
		{
			// failed to set rich presence, possibly because of too many lobby tasks,
			// store it and try again later
			m_desiredRichPresenceState = state;
			m_pendingRichPresenceSessionID = DrxSessionInvalidID;

			DrxLog("[RichPresence] SetRichPresence - Rich presence %s with error code %d", error == eCLE_Success ? "succeeded" : "failed", error);
		}
		else
		{
			m_settingRichPresence = true;
			m_pendingRichPresenceState = state;
			m_desiredRichPresenceState = eRPS_none;

			DrxLog("[RichPresence] SetRichPresence - Rich presence has been successfully started");
		}
	}
#if !defined(_RELEASE)
	else
	{
		error = eCLE_InternalError;
		DrxLog("[RichPresence] SetRichPresence called but we have no lobby, tried to set state to %d", state);
	}
#endif

	return (error == eCLE_Success);
}

void CGame::InitRichPresence()
{
	XmlNodeRef rpXML = gEnv->pGame->GetIGameFramework()->GetISystem()->LoadXmlFromFile("Scripts/Network/RichPresence.xml");
	if(rpXML)
	{
		i32 numElements = rpXML->getChildCount();
		for (i32 i = 0; i < numElements; ++ i)
		{
			XmlNodeRef childXml = rpXML->getChild(i);
			tukk levelName = NULL;
			i32 id = -1;

			if (childXml->getAttr("name", &levelName) && childXml->getAttr("id", id))
			{
				m_richPresence[levelName] = id;
			}
		}
	}

	// management
	m_desiredRichPresenceState = eRPS_none;
	m_pendingRichPresenceState = eRPS_none;
	m_currentRichPresenceState = eRPS_none;
}

void CGame::AddRichPresence( tukk path )
{
	XmlNodeRef rpXML = gEnv->pGame->GetIGameFramework()->GetISystem()->LoadXmlFromFile( path );

	if(rpXML)
	{
		i32 numElements = rpXML->getChildCount();
		for (i32 i = 0; i < numElements; ++ i)
		{
			XmlNodeRef childXml = rpXML->getChild(i);
			tukk levelName = NULL;
			i32 id = -1;

			if (childXml->getAttr("name", &levelName) && childXml->getAttr("id", id))
			{
				m_richPresence[levelName] = id;
			}
		}
	}
}

//static---------------------------------------
void CGame::PartyMembersCallback(UDrxLobbyEventData eventData, uk arg)
{
	DrxLogAlways("CGame::PartyMembersCallback(UDrxLobbyEventData eventData, uk arg)");

	CGame* pGame = (CGame*)arg;
	DRX_ASSERT(pGame);
	pGame->m_currentXboxLivePartySize = eventData.pPartyMembers->m_numMembers;
}

//static---------------------------------------
void CGame::UserProfileChangedCallback(UDrxLobbyEventData eventData, uk arg)
{
	DrxLog("[GameLobbyUpr] UserProfileChangedCallback user %d", eventData.pUserProfileChanged->m_user);

	CGame *pGame = (CGame*)arg;
	assert(pGame);

	IGameFramework *pFramework = pGame ? pGame->GetIGameFramework() : NULL;
	u32 userIndex = pGame ? pGame->GetExclusiveControllerDeviceIndex() : 0;

	if(eventData.pUserProfileChanged->m_user == userIndex)
	{
		if(pFramework && pFramework->StartedGameContext())
		{
			pGame->SetUserProfileChanged(true);
		}
	}
}

//static---------------------------------------
void CGame::InviteAcceptedCallback(UDrxLobbyEventData eventData, uk arg)
{
	DrxLog("[Invite] InviteAcceptedCallback");

	CGame *pGame = (CGame*)arg;

	SDrxLobbyInviteAcceptedData* inviteData = eventData.pInviteAcceptedData;

	bool acceptInvite = true;

	DRX_ASSERT_MESSAGE(pGame, "No game!");

	// we should always accept the invite if we have no exclusive controller
	if(pGame->m_hasExclusiveController)
	{
		// can't possibly be in a squad if we're not multiplayer, i hope
		if(gEnv->bMultiplayer && inviteData->m_error == eCLE_Success)
		{
			IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
			IDrxLobbyService *pLobbyService = pLobby ? pLobby->GetLobbyService(eCLS_Online) : NULL;
			IDrxMatchMaking *pMatchMaking = pLobbyService ? pLobbyService->GetMatchMaking(): NULL;

			bool alreadyInSession = false;

			// the session we are trying to join is the same as the session we are in
			if( alreadyInSession )
			{
				CWarningsUpr *pWarnings = pGame->GetWarnings();
				u32 exclusiveControllerIndex = pGame->GetExclusiveControllerDeviceIndex();
				if (exclusiveControllerIndex == inviteData->m_user)
				{
					DrxLog("[invite] we tried to accept an invite to a session we are already in");

					// the user is already in the session, tell them
					acceptInvite = false;

					pWarnings->RemoveGameWarning("InviteFailedIsHost");
					pWarnings->RemoveGameWarning("InviteFailedAlreadyInSession");
					pWarnings->AddGameWarning("InviteFailedAlreadyInSession");
				}
				else if(pGame->m_pSquadUpr->InCharge())
				{
					DrxLog("[invite] session is hosted on this system, yet someone on this system has tried to join it via invite");

					// someone else is trying to accept the session on the same system
					// as the host. only a problem on 360 which can have multiple users
					// signed in at a time
					acceptInvite = false;

					pWarnings->RemoveGameWarning("InviteFailedIsHost");
					pWarnings->RemoveGameWarning("InviteFailedAlreadyInSession");
					pWarnings->AddGameWarning("InviteFailedIsHost");
				}
			}
		}
	}

	if(acceptInvite)
	{
		DrxInviteID id = inviteData->m_id;
		EDrxLobbyInviteType inviteType = inviteData->m_id->IsFromInvite() ? eCLIT_InviteToSquad : eCLIT_JoinSessionInProgress;

		pGame->SetInviteData(inviteData->m_service, inviteData->m_user, id, inviteData->m_error, inviteType);
		pGame->SetInviteAcceptedState(eIAS_Init);
		pGame->m_bLoggedInFromInvite = true;
	}
}

//static---------------------------------------
void CGame::OnlineStateCallback(UDrxLobbyEventData eventData, uk arg)
{
	DrxLog("[Game] OnlineStateCallback");

	CGame *pGame = (CGame*)arg;
	DRX_ASSERT_MESSAGE(pGame, "No game!");

	SDrxLobbyOnlineStateData *pOnlineStateData  = eventData.pOnlineStateData;
	if(pOnlineStateData)
	{
		if(pOnlineStateData->m_curState == eOS_SignedOut)
		{
			bool isCurrentUser = true;

			if((pGame->m_inviteAcceptedData.m_id != DrxInvalidInvite) && (isCurrentUser))
			{
				if(pOnlineStateData->m_reason != eCLE_CyclingForInvite)
				{
					DrxLog("[Game] User %d signed out and was accepting an invite, invalidating the invite", pOnlineStateData->m_user);

					pGame->InvalidateInviteData();
				}
			}
		}

	}
}

//static---------------------------------------
void CGame::EthernetStateCallback(UDrxLobbyEventData eventData, uk arg)
{
	DrxLog("[Game] EthernetStateCallback");

	CGame *pGame = (CGame*)arg;
	DRX_ASSERT_MESSAGE(pGame, "No game!");

	SDrxLobbyEthernetStateData *pEthernetStateData = eventData.pEthernetStateData;

	if(pEthernetStateData)
	{
		ECableState cableState = pEthernetStateData->m_curState;
		if(cableState == eCS_Unplugged || cableState == eCS_Disconnected)
		{
			if(pGame->m_inviteAcceptedData.m_id != DrxInvalidInvite )
			{
				DrxLog("[Game] EthernetState has been %s, invalidating invite data", cableState == eCS_Unplugged ? "unplugged" : "disconnected");

				// cable has been pulled, invalidate the invite data
				pGame->InvalidateInviteData();
			}
		}
	}
}

THUDWarningId CGame::AddGameWarning( tukk stringId, tukk paramMessage, IGameWarningsListener* pListener /*= NULL*/ )
{
	if(GetWarnings())
		return GetWarnings()->AddGameWarning(stringId, paramMessage, pListener);
	return 0;
}

void CGame::RemoveGameWarning( tukk stringId )
{
	if(GetWarnings())
		GetWarnings()->RemoveGameWarning(stringId);
}

void CGame::RenderGameWarnings()
{
}

bool CGame::GameEndLevel(tukk nextLevel)
{
	// Ensure objectives cleared when chaining levels
	g_pGame->GetMOSystem()->DeactivateObjectives();
	return false;
}

void CGame::OnRenderScene(const SRenderingPassInfo &passInfo)
{
	TRenderSceneListeners::const_iterator it = m_renderSceneListeners.begin();
	TRenderSceneListeners::const_iterator end = m_renderSceneListeners.end();
	for(; it!=end; ++it)
	{
		(*it)->OnRenderScene(passInfo);
	}
}


u32 CGame::GetRandomNumber()
{
	return m_randomGenerator.GenerateUint32();
}

float CGame::GetRandomFloat()
{
	return m_randomGenerator.GenerateFloat();
}

bool CGame::LoadLastSave()
{
#if !defined(DEDICATED_SERVER)
	if (!gEnv->bMultiplayer)
	{
		bool bLoadSave = true;
		if (gEnv->IsEditor())
		{
			ICVar* pAllowSaveLoadInEditor = gEnv->pConsole->GetCVar("g_allowSaveLoadInEditor");
			if (pAllowSaveLoadInEditor)
			{
				bLoadSave = (pAllowSaveLoadInEditor->GetIVal() != 0);
			}
			else
			{
				bLoadSave = false;
			}

			if (!bLoadSave) // Wont go through normal path which reloads hud, reload here
			{
				g_pGame->PostSerialize();
			}
		}

		bool bSuccess = false;

		if (bLoadSave)
		{
			if(g_pGameCVars->g_enableSlimCheckpoints)
			{
				bSuccess = GetIGameFramework()->GetICheckpointSystem()->LoadLastCheckpoint();
			}
			else
			{
				string levelName("");
				m_bLastSaveDirty = true; // Just to be safe (Some code paths won't refresh level name, i.e. quicksave and possibly others)
				const string& fileName = g_pGame->GetLastSaveGame(levelName);

				if (!fileName.empty())
				{
					gEnv->pSystem->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_LEVEL_LOAD_RESUME_GAME, 0, 0);
					// Attempt a quick-load
					ELoadGameResult result = g_pGame->GetIGameFramework()->LoadGame(fileName.c_str(), true);

					if (result==eLGR_CantQuick_NeedFullLoad)
					{
						// Fall-back on a full-load
						result = g_pGame->GetIGameFramework()->LoadGame(fileName.c_str(), false);
					}

					bSuccess = result==eLGR_Ok;
				}
				else
				{
					GameWarning("[LoadLastSave]: No save game found to load.");
				}
			}
		}
		else
		{
			bSuccess = CGodMode::GetInstance().RespawnPlayerIfDead();
		}

		return bSuccess;
	}
#endif

	return false;
}

CGameLobby* CGame::GetGameLobby()
{
	return m_pGameLobbyUpr ? m_pGameLobbyUpr->GetGameLobby() : NULL;
}

void CGame::ClearGameSessionHandler()
{
	GetIGameFramework()->SetGameSessionHandler(NULL);
	m_pLobbySessionHandler = NULL;
}

void CGame::OnBeforeEditorLevelLoad()
{
	m_pGameParametersStorage->ReleaseLevelResources();
	if (m_pBodyDamageUpr)
		m_pBodyDamageUpr->FlushLevelResourcesCache();

	if (m_pTacticalUpr)
	{
		m_pTacticalUpr->ClearAllTacticalPoints();
	}

	m_pLedgeUpr->Reset();

	CSmokeUpr::GetSmokeUpr()->ReleaseObstructionObjects();
}

void CGame::OnExitGameSession()
{
}

void CGame::PreSerialize()
{
	//This is called while loading a saved game
	//Reset some game systems that might cause problems during loading
	m_pWeaponSystem->GetTracerUpr().Reset();
	m_pFramework->GetIItemSystem()->Reset();
	m_pGameParametersStorage->GetItemResourceCache().Get1pDBAUpr().Reset();

	g_tacticalPointLanguageExtender.Reset();
}


// this function is now called after all entities have been fullserialized'd.  (it was part of gamerules serialization before).
// This means that almost all modules could now do the whole serialization here, instead of doing part here and part in postserialize().
// however, there is no point in changing that for now.
void CGame::FullSerialize( TSerialize serializer )
{
	serializer.BeginGroup("IGame");
	if (GetMOSystem())
	{
		GetMOSystem()->Serialize(serializer);
	}

	if (m_pLedgeUpr)
	{
		m_pLedgeUpr->Serialize(serializer);
	}

	if( m_pGameAISystem )
	{
		m_pGameAISystem->Serialize(serializer);
		g_tacticalPointLanguageExtender.FullSerialize(serializer);
	}

	if(m_statsRecorder)
		m_statsRecorder->Serialize(serializer);

	if(m_colorGradientUpr)
		m_colorGradientUpr->Serialize(serializer);

	if(m_pTacticalUpr)
	{
		m_pTacticalUpr->Serialize(serializer);
	}

	if (m_pFramework->GetICustomActionUpr())
		m_pFramework->GetICustomActionUpr()->Serialize( serializer );

	serializer.EndGroup();
}

void CGame::PostSerialize()
{
	//reset HUD
	if (m_pUIUpr)
		m_pUIUpr->ActivateDefaultState();

	// Need to init player after dead or load saved
	SHUDEvent	hudEvent_initLocalPlayer(eHUDEvent_OnInitLocalPlayer);
	IActor	*pActor = gEnv->pGame->GetIGameFramework()->GetClientActor();
	if(pActor)
	{
		hudEvent_initLocalPlayer.AddData(static_cast<i32>(pActor->GetEntityId()));
	}
	else
	{
		hudEvent_initLocalPlayer.AddData(static_cast<i32>(0));
	}

	CHUDEventDispatcher::CallEvent(hudEvent_initLocalPlayer);

	// Ensure radar back to saved state
	if( m_pTacticalUpr )
	{
		m_pTacticalUpr->PostSerialize();
	}

	if( m_pGameAISystem )
	{
		m_pGameAISystem->PostSerialize();
	}

	if ( m_pUIUpr )
	{
		m_pUIUpr->PostSerialize();
	}

	// Want to set last save as dirty since the timestamp was updated, so thats the latest save now which should be resumed from
	m_bLastSaveDirty = true;
}

void CGame::OnDeathReloadComplete()
{
#if !defined(DEDICATED_SERVER)
	if (!gEnv->bMultiplayer)
	{
		if(!CGodMode::GetInstance().IsGodModeActive())
		{
			//load the last save-game if available
			gEnv->pConsole->ExecuteString("loadLastSave", false, true);
		}
	}
#endif
}

bool CGame::IsGameActive() const
{
	assert(g_pGame);
	IGameFramework* pGameFramework = g_pGame->GetIGameFramework();
	assert(pGameFramework);
	return (pGameFramework->StartingGameContext() || pGameFramework->StartedGameContext()) && (IsGameSessionHostMigrating() || pGameFramework->GetClientChannel());
}

void CGame::SetCrashDebugMessage(tukk const msg)
{
	drx_strcpy(gEnv->szDebugStatus, msg);
}

void CGame::AppendCrashDebugMessage(tukk const msg)
{
	drx_strcat(gEnv->szDebugStatus, msg);
}

void CGame::OnDedicatedConfigEntry( tukk szKey, tukk szValue )
{
	DrxLog("CGame::OnDedicatedConfigEntry() option=%s, value=%s, alreadyFound=%s", szKey, szValue, m_variantOptions.find(szKey) == m_variantOptions.end() ? "false" : "true");
	if (m_variantOptions.find(szKey) == m_variantOptions.end())
	{
		m_variantOptions[szKey] = szValue;
	}

#if defined(CVARS_WHITELIST)
	ICVarsWhitelist* pCVarsWhitelist = gEnv->pSystem->GetCVarsWhiteList();
	bool execute = (pCVarsWhitelist) ? pCVarsWhitelist->IsWhiteListed(szKey, false) : true;
	if (execute)
#endif // defined(CVARS_WHITELIST)
	{
		gEnv->pConsole->LoadConfigVar(szKey, szValue);
	}
}

void CGame::CMPConfigSink::OnLoadConfigurationEntry(tukk szKey, tukk szValue, tukk szGroup)
{
	if ((stricmp("g_matchmakingblock",szKey) != 0) && (stricmp("g_matchmakingversion",szKey) != 0))
	{
		DrxLog("nigel.cfg::'%s' only supports g_matchmakingblock & g_matchmakingversion",szKey);
		return;
	}
	ICVar *pCVar = gEnv->pConsole->GetCVar(szKey);
	if (pCVar)
	{
		pCVar->ForceSet(szValue);
	}
}

void CGame::LoadPatchLocalizationData()
{
	ILocalizationUpr *pLocMan = GetISystem()->GetLocalizationUpr();

	string sLocaFolderName = "Libs/Localization/";

	ICVar* pCVar = gEnv->pConsole->GetCVar("g_language");
	tukk g_languageIs = pCVar->GetString();

	//here we just load the patch localization file

	string sFileName;
	sFileName.Format("localization_%s.xml", g_languageIs);
	DrxLog("CGame::LoadPatchLocalizationData() is loading localization file=%s", sFileName.c_str());

	string locaFile = sLocaFolderName + sFileName.c_str();

	if (pLocMan->InitLocalizationData(locaFile.c_str()))
	{
		if (!gEnv->IsEditor())
		{
			// load only the init xml files
			pLocMan->LoadLocalizationDataByTag("patch");
		}
	}
}

void CGame::VerifyMaxPlayers(ICVar * pVar)
{
	i32 nPlayers = pVar->GetIVal();
	if (nPlayers < 2 || nPlayers > MAX_PLAYER_LIMIT)
		pVar->Set( CLAMP(nPlayers, 2, MAX_PLAYER_LIMIT) );
}

void CGame::SDedicatedConfigSink::OnLoadConfigurationEntry( tukk szKey, tukk szValue, tukk szGroup )
{
	g_pGame->OnDedicatedConfigEntry(szKey, szValue);
}

void CGame::QueueDeferredKill(const EntityId entityID)
{
	stl::push_back_unique(m_deferredKills, entityID);
}

void CGame::CommitDeferredKills()
{
	if (m_deferredKills.empty())
		return;

	DeferredKills::const_iterator it = m_deferredKills.begin();
	DeferredKills::const_iterator end = m_deferredKills.end();

	for (; it != end; ++it)
	{
		const EntityId entityID = *it;

		if (IEntity* entity = gEnv->pEntitySystem->GetEntity(entityID))
		{
			// Kill the agent by entityId a lot of damage.
			const HitInfo hitinfo(entityID, entityID, entityID,
				10000.0f, 0.0f, 0, -1, CGameRules::EHitType::Punish,
				ZERO, ZERO, ZERO);

			GetGameRules()->ClientHit(hitinfo);
		}
	}

	m_deferredKills.clear();
}

void CGame::AddPersistentAccessories()
{
#if !defined(DEDICATED_SERVER)
	if(!gEnv->bMultiplayer)
	{
		// Note: If this code changes, update the code right above which does the pre-cache during level load (case eAE_loadLevel)
		// Give the player his attachments
		if(IActor *pClientActor = GetIGameFramework()->GetClientActor())
		{
			IInventory* pInventory = pClientActor->GetInventory();
			if (pInventory)
			{
				pInventory->IgnoreNextClear();

				IItemSystem* pItemSystem = GetIGameFramework()->GetIItemSystem();

				for(i32 i = 0; i < s_numUnlockableAttachments; ++i)
				{
					i32 hasAttachment = m_pPersistantStats->GetStat(s_unlockableAttachmentNames[i], EMPS_AttachmentUnlocked);
					if(hasAttachment)
					{
						pItemSystem->GiveItem(pClientActor, s_unlockableAttachmentNames[i], false, false, true);
					}
				}
			}
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/// Export & load from level pak file
/// IMPORTANT: KEEP THE ORDER OF EXPORT-LOAD!

namespace
{
	tukk GenerateExportingBaseFileName( tukk levelName, tukk missionName )
	{
		static char baseFileName[512]; //This must be persistent memory

		drx_sprintf(baseFileName, "%s/datafile_%s", levelName, missionName);

		return baseFileName;
	}
}

IGame::ExportFilesInfo CGame::ExportLevelData( tukk levelName, tukk missionName ) const
{
	tukk baseFileName = GenerateExportingBaseFileName( levelName, missionName );
	char fileName[512];

	u32 exportedFileCount = 0; // Note: Increase the counter for every system which exports a file, regardless of success or not

	// Shape-volume data
	IGameVolumesEdit* pGameVolumesEdit = m_pFramework->GetIGameVolumesUpr() ? m_pFramework->GetIGameVolumesUpr()->GetEditorInterface() : NULL;
	if (pGameVolumesEdit != NULL)
	{
		IGame::ExportFilesInfo::GetNameForFile( baseFileName, exportedFileCount, fileName, sizeof(fileName) );
		pGameVolumesEdit->Export( fileName );
	}
	exportedFileCount++;

	// Ledge manager data
	CLedgeUprEdit* pLedgeUprEdit = m_pLedgeUpr ? m_pLedgeUpr->GetEditorUpr() : NULL;
	if (pLedgeUprEdit != NULL)
	{
		IGame::ExportFilesInfo::GetNameForFile( baseFileName, exportedFileCount, fileName, sizeof(fileName) );
		pLedgeUprEdit->Export( fileName );
	}
	exportedFileCount++;

	return IGame::ExportFilesInfo( baseFileName, exportedFileCount );
}

void CGame::LoadExportedLevelData( tukk levelName, tukk missionName )
{
	tukk baseFileName = GenerateExportingBaseFileName( levelName, missionName );

	char fileName[512];
	u32 loadedFileCount = 0; // Note: Increase the counter for every system which loads a file, regardless of success or not

	// Shape-volume data
	IGameVolumes* pGameVolumes = m_pFramework->GetIGameVolumesUpr();
	if (pGameVolumes != NULL)
	{
		IGame::ExportFilesInfo::GetNameForFile( baseFileName, loadedFileCount, fileName, sizeof(fileName) );
		pGameVolumes->Load( fileName );
	}
	loadedFileCount++;

	// Ledge manager data
	if (m_pLedgeUpr != NULL)
	{
		IGame::ExportFilesInfo::GetNameForFile( baseFileName, loadedFileCount, fileName, sizeof(fileName));
		m_pLedgeUpr->Load( fileName );
	}
	loadedFileCount++;
}


IGamePhysicsSettings* CGame::GetIGamePhysicsSettings()
{
	return m_pGamePhysicsSettings;
}

void CGame::GetTelemetryTimers( i32& careerTime, i32& gameTime, i32& sessionTime, uk pArg )
{
	CPersistantStats* pStats = g_pGame->GetPersistantStats();
	if( pStats )
	{
		careerTime = pStats->GetStat( EIPS_TimePlayed );
		//game time follows same rules as career time, just telemetry component starts counting from 0 at start of each session
		gameTime = careerTime;
	}

	sessionTime = (i32)gEnv->pTimer->GetFrameStartTime( ITimer::ETIMER_UI ).GetSeconds();
}

i32 CGame::GetDifficultyForTelemetry(i32 difficulty/*=-1*/) const
{
	//ICrysis3PartnerTelemetry::EGameDifficulty telemDiff = ICrysis3PartnerTelemetry::eGD_None;

	//if( gEnv->bMultiplayer == false )
	//{
	//	if( difficulty==-1 )
	//	{
	//		difficulty = g_pGameCVars->g_difficultyLevel;
	//	}
	//	switch( difficulty )
	//	{
	//	case eDifficulty_Easy:			telemDiff = ICrysis3PartnerTelemetry::eGD_Easy;
	//		break;
	//	case eDifficulty_Default:		//intentional fall through
	//	case eDifficulty_Normal:		telemDiff = ICrysis3PartnerTelemetry::eGD_Normal;
	//		break;
	//	case eDifficulty_Hard:			telemDiff = ICrysis3PartnerTelemetry::eGD_Hard;
	//		break;
	//	case eDifficulty_Delta:			telemDiff = ICrysis3PartnerTelemetry::eGD_VeryHard;
	//		break;
	//	case eDifficulty_PostHuman:	telemDiff = ICrysis3PartnerTelemetry::eGD_Expert;
	//		break;
	//	}
	//}

	//return telemDiff;
	return -1;
}

float CGame::GetFOV() const
{
	float fFov(g_pGameCVars->cl_fov);
	if (gEnv->bMultiplayer)
	{
		CGameRules * pGameRules = GetGameRules();
		if (pGameRules && !pGameRules->IsIntroSequenceCurrentlyPlaying())
		{
			return fFov * g_pGameCVars->cl_mp_fov_scalar;
		}
	}

	return fFov;
}

float CGame::GetPowerSprintTargetFov() const
{
	if (gEnv->bMultiplayer)
	{
		return g_pGameCVars->pl_movement.power_sprint_targetFov * g_pGameCVars->cl_mp_fov_scalar;
	}
	else

	return g_pGameCVars->pl_movement.power_sprint_targetFov;
}

static void OnChangedStereoRenderDevice(ICVar*	pStereoRenderDevice)
{
	if (!pStereoRenderDevice)
	{
		return;
	}
	g_pGame->SetRenderingToHMD(pStereoRenderDevice->GetIVal()==7);
}

static tukk g_checkpointOrder[] =
{
//	Jailbreak
	"CP1_Dock_A",
	"CP2_Dock_B",
	"CP3_Platform_Exit",
	"CP4_Bridge",
	"CP5_Dome",
	"CP6_Reactor",
	"CP7_Tower",
	"AB0_Minefield",
//	Fields
	"Fields_VVV_CP1",
	"Fields_AB1_ReachedSniper",
	"Fields_AB1_TowerDead",
	"Fields_AB1_Clearing",
	"Fields_AB1_TunnelStart",
	"Fields_AB1_AB2_Tunnel",
	"Fields_AB1_AB2_PreStalker",
	"Fields_Shed",
	"Fields_EndShed",
	"Fields_Hill",
//	Canyon
	"canyon_top_save_1",
	"canyon_lowervvv_save_2",
	"canyon_dam_top_save_3",
	"canyon_dam_1_save_4",
	"canyon_dam_2_save_5",
	"canyon_flood_save_6",
	"canyon_relay_save_7",
	"canyon_tower_save_8",
	"canyon_miniboss_save_9",
	"canyon_controlroom_save_10",
//	Swamp
	"Swamp1",
	"Swamp2",
	"Swamp3",
	"Swamp4",
	"Swamp5",
	"Swamp6",
	"Swamp6b",
	"Swamp7",
	"Swamp8",
	"Swamp9",
	"Swamp10",
	"Swamp11",
//	River
	"CP1_Hivemind",
	"CP2_Supercharge",
	"CP3_Stalkers",
	"CP4_CellIntel",
	"CP5_Buggy",
	"CP6_Bridge",
	"CP7_BridgeInterior",
	"CP8_RiverBed",
//	River2
//	"CP8_RiverBed",
	"CP9_Pinger",
	"CP10_SO_ICV",
	"CP11_Gate",
	"CP12_Approach",
	"CP13_Defences",
	"CP14_DefencesSO",
	"CP15_Pipe_Entrance",
//	"CP16_Rear_Entrance",
	"CP17_CELLHQDefences",
	"CP18_CellHQ",
//	Islands
	"islands_checkpoint_100",
	"islands_checkpoint_110",
	"islands_checkpoint_120",
	"islands_checkpoint_130",
	"islands_checkpoint_200",
	"islands_checkpoint_240",
	"islands_checkpoint_220",
	"islands_checkpoint_290",
	"islands_checkpoint_230",
	"islands_checkpoint_232",
	"islands_checkpoint_210",
	"islands_checkpoint_212",
	"islands_checkpoint_250",
	"islands_checkpoint_260",
	"islands_checkpoint_291",
	"islands_checkpoint_310",
	"islands_checkpoint_320",
	"islands_checkpoint_330",
	"islands_checkpoint_350",
	"360",
	"islands_checkpoint_201",
	"islands_checkpoint_202",
	"islands_checkpoint_203",
//	Cave
	"savegame05",
	"savegame10",
	"savegame11",
	"savegame12",
	"savegame13",
	"savegame14",
	"savegame20",
	"savegame21",
	"savegame22",
	"savegame23",
	"savegame24A",
	"savegame24B",
	"savegame25",
	"savegame26",
	"savegame30",
	"savegame31"
};

static const size_t g_checkpointOrderSize = (DRX_ARRAY_COUNT(g_checkpointOrder) );


i32 CGame::GetCheckpointIDForTelemetry(tukk checkpointName) const
{
	for (i32 i = 0; i < g_checkpointOrderSize; ++i)
	{
		if (stricmp(g_checkpointOrder[i], checkpointName) == 0)
			return i;
	}
	assert(!"invalid checkpoint name in CGame::GetCheckpointIDForTelemetry()");
	return 0;
}

#if DRX_PLATFORM_DURANGO
//////////////////////////////////////////////////////////////////////////
void CGame::EnsureSigninState()
{
	if (m_userChangedDoSignOutAndIn == true)
	{
		m_userChangedDoSignOutAndIn = false;
		LogoutCurrentUser(eLR_SetExclusiveController);
		gEnv->pSystem->GetPlatformOS()->UserDoSignIn(1);
	}
}
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


