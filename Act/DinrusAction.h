// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/ICmdLine.h>

#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Sys/IDrxPak.h>
#include "ISaveGame.h"

struct IFlowSystem;
struct IGameTokenSystem;
struct IEffectSystem;
struct IBreakableGlassSystem;
struct IForceFeedbackSystem;
struct IGameVolumes;

class CAIDebugRenderer;
class CAINetworkDebugRenderer;
class CGameRulesSystem;
class CScriptBind_Action;
class CScriptBind_ActorSystem;
class CScriptBind_ItemSystem;
class CScriptBind_ActionMapUpr;
class CScriptBind_Network;
class CScriptBind_VehicleSystem;
class CScriptBind_Vehicle;
class CScriptBind_VehicleSeat;
class CScriptBind_Inventory;
class CScriptBind_DialogSystem;
class CScriptBind_MaterialEffects;
class CScriptBind_UIAction;

class CFlowSystem;
class CDevMode;
class CTimeDemoRecorder;
class CGameQueryListener;
class CScriptRMI;
class CGameSerialize;
class CMaterialEffects;
class CMaterialEffectsCVars;
class CGameObjectSystem;
class CActionMapUpr;
class CActionGame;
class CActorSystem;
class CallbackTimer;
class CGameClientNub;
class CGameContext;
class CGameServerNub;
class CItemSystem;
class CLevelSystem;
class CUIDraw;
class CVehicleSystem;
class CViewSystem;
class CGameplayRecorder;
class CPersistantDebug;
class CPlayerProfileUpr;
class CDialogSystem;
class CSubtitleUpr;
class CGameplayAnalyst;
class CTimeOfDayScheduler;
class CNetworkCVars;
class CDrxActionCVars;
class CGameStatsConfig;
class CSignalTimer;
class CRangeSignaling;
class CAIProxy;
class CommunicationVoiceLibrary;
class CCustomActionUpr;
class CCustomEventUpr;
class CAIProxyUpr;
class CForceFeedBackSystem;
class CDrxActionPhysicQueues;
class CNetworkStallTickerThread;
class CSharedParamsUpr;
struct ICooperativeAnimationUpr;
struct IGameSessionHandler;
class CRuntimeAreaUpr;
class CColorGradientUpr;

struct CAnimationGraphCVars;
struct IRealtimeRemoteUpdate;
struct ISerializeHelper;
struct ITimeDemoRecorder;

class CNetMessageDistpatcher;
class CEntityContainerMgr;
class CEntityAttachmentExNodeRegistry;

class CDrxAction :
	public IGameFramework
{

public:
	CDrxAction(SSysInitParams& initParams);
	~CDrxAction();

	// IGameFramework
	virtual void                          ShutDown();

	virtual void                          PreSystemUpdate();
	virtual bool                          PostSystemUpdate(bool hasFocus, CEnumFlags<ESystemUpdateFlags> updateFlags = CEnumFlags<ESystemUpdateFlags>());
	virtual void                          PreFinalizeCamera(CEnumFlags<ESystemUpdateFlags> updateFlags);
	virtual void                          PreRender();
	virtual void                          PostRender(CEnumFlags<ESystemUpdateFlags> updateFlags);
	virtual void                          PostRenderSubmit();

	void                                  ClearTimers();
	virtual TimerID                       AddTimer(CTimeValue interval, bool repeat, TimerCallback callback, uk userdata);
	virtual uk                         RemoveTimer(TimerID timerID);

	virtual u32                        GetPreUpdateTicks();

	virtual void                          RegisterFactory(tukk name, IActorCreator* pCreator, bool isAI);
	virtual void                          RegisterFactory(tukk name, IItemCreator* pCreator, bool isAI);
	virtual void                          RegisterFactory(tukk name, IVehicleCreator* pCreator, bool isAI);
	virtual void                          RegisterFactory(tukk name, IGameObjectExtensionCreator* pCreator, bool isAI);
	virtual void                          RegisterFactory(tukk name, ISaveGame*(*func)(), bool);
	virtual void                          RegisterFactory(tukk name, ILoadGame*(*func)(), bool);

	virtual void                          InitGameType(bool multiplayer, bool fromInit);
	virtual bool                          CompleteInit();
	virtual void                          PrePhysicsUpdate() /*override*/;
	virtual void                          Reset(bool clients);
	virtual void                          GetMemoryUsage(IDrxSizer* pSizer) const;

	virtual void                          PauseGame(bool pause, bool force, u32 nFadeOutInMS = 0);
	virtual bool                          IsGamePaused();
	virtual bool                          IsGameStarted();
	virtual bool                          IsInLevelLoad();
	virtual bool                          IsLoadingSaveGame();
	virtual tukk                   GetLevelName();
	virtual void                          GetAbsLevelPath(tuk pPathBuffer, u32 pathBufferSize);
	virtual bool                          IsInTimeDemo();        // Check if time demo is in progress (either playing or recording);
	virtual bool                          IsTimeDemoRecording(); // Check if time demo is recording;

	virtual ISystem*                      GetISystem()           { return m_pSystem; }
	virtual ILanQueryListener*            GetILanQueryListener() { return m_pLanQueryListener; }
	virtual IUIDraw*                      GetIUIDraw();
	virtual IMannequin&                   GetMannequinInterface();
	virtual ILevelSystem*                 GetILevelSystem();
	virtual IActorSystem*                 GetIActorSystem();
	virtual IItemSystem*                  GetIItemSystem();
	virtual IBreakReplicator*             GetIBreakReplicator();
	virtual IVehicleSystem*               GetIVehicleSystem();
	virtual IActionMapUpr*            GetIActionMapUpr();
	virtual IViewSystem*                  GetIViewSystem();
	virtual IGameplayRecorder*            GetIGameplayRecorder();
	virtual IGameRulesSystem*             GetIGameRulesSystem();
	virtual IGameObjectSystem*            GetIGameObjectSystem();
	virtual IFlowSystem*                  GetIFlowSystem();
	virtual IGameTokenSystem*             GetIGameTokenSystem();
	virtual IEffectSystem*                GetIEffectSystem();
	virtual IMaterialEffects*             GetIMaterialEffects();
	virtual IBreakableGlassSystem*        GetIBreakableGlassSystem();
	virtual IPlayerProfileUpr*        GetIPlayerProfileUpr();
	virtual ISubtitleUpr*             GetISubtitleUpr();
	virtual IDialogSystem*                GetIDialogSystem();
	virtual ICooperativeAnimationUpr* GetICooperativeAnimationUpr();
	virtual ICheckpointSystem*            GetICheckpointSystem();
	virtual IForceFeedbackSystem*         GetIForceFeedbackSystem() const;
	virtual ICustomActionUpr*         GetICustomActionUpr() const;
	virtual ICustomEventUpr*          GetICustomEventUpr() const;
	virtual IRealtimeRemoteUpdate*        GetIRealTimeRemoteUpdate();
	virtual ITimeDemoRecorder*            GetITimeDemoRecorder() const;

	virtual bool                          StartGameContext(const SGameStartParams* pGameStartParams);
	virtual bool                          ChangeGameContext(const SGameContextParams* pGameContextParams);
	virtual void                          EndGameContext();
	virtual bool                          StartedGameContext() const;
	virtual bool                          StartingGameContext() const;
	virtual bool                          BlockingSpawnPlayer();

	virtual void                          ReleaseGameStats();

	virtual void                          ResetBrokenGameObjects();
	virtual void                          CloneBrokenObjectsAndRevertToStateAtTime(i32 iFirstBreakEventIndex, u16* pBreakEventIndices, i32& iNumBreakEvents, IRenderNode** outClonedNodes, i32& iNumClonedNodes, SRenderNodeCloneLookup& renderNodeLookup);
	virtual void                          ApplySingleProceduralBreakFromEventIndex(u16 uBreakEventIndex, const SRenderNodeCloneLookup& renderNodeLookup);
	virtual void                          UnhideBrokenObjectsByIndex(u16* ObjectIndicies, i32 iNumObjectIndices);

	void                                  Serialize(TSerialize ser); // defined in ActionGame.cpp
	virtual void                          FlushBreakableObjects();   // defined in ActionGame.cpp
	void                                  ClearBreakHistory();

	IGameToEditorInterface*               GetIGameToEditor() { return m_pGameToEditor; }
	virtual void                          InitEditor(IGameToEditorInterface* pGameToEditor);
	virtual void                          SetEditorLevel(tukk levelName, tukk levelFolder);
	virtual void                          GetEditorLevel(tuk* levelName, tuk* levelFolder);

	virtual void                          BeginLanQuery();
	virtual void                          EndCurrentQuery();

	virtual IActor*                       GetClientActor() const;
	virtual EntityId                      GetClientActorId() const;
	virtual IEntity*                      GetClientEntity() const;
	virtual EntityId                      GetClientEntityId() const;
	virtual INetChannel*                  GetClientChannel() const;
	virtual CTimeValue                    GetServerTime();
	virtual u16                        GetGameChannelId(INetChannel* pNetChannel);
	virtual INetChannel*                  GetNetChannel(u16 channelId);
	virtual void                          SetServerChannelPlayerId(u16 channelId, EntityId id);
	virtual const SEntitySchedulingProfiles* GetEntitySchedulerProfiles(IEntity* pEnt);
	virtual bool                          IsChannelOnHold(u16 channelId);
	virtual IGameObject*                  GetGameObject(EntityId id);
	virtual bool                          GetNetworkSafeClassId(u16& id, tukk className);
	virtual bool                          GetNetworkSafeClassName(tuk className, size_t classNameSizeInBytes, u16 id);
	virtual IGameObjectExtension*         QueryGameObjectExtension(EntityId id, tukk name);

	virtual INetContext*                  GetNetContext();

	virtual bool                          SaveGame(tukk path, bool bQuick = false, bool bForceImmediate = false, ESaveGameReason reason = eSGR_QuickSave, bool ignoreDelay = false, tukk checkpointName = NULL);
	virtual ELoadGameResult               LoadGame(tukk path, bool quick = false, bool ignoreDelay = false);
	virtual TSaveGameName                 CreateSaveGameName();

	virtual void                          ScheduleEndLevel(tukk nextLevel);
	virtual void                          ScheduleEndLevelNow(tukk nextLevel);

	virtual void                          OnEditorSetGameMode(i32 iMode);
	virtual bool                          IsEditing() { return m_isEditing; }

	virtual void                          OnBreakageSpawnedEntity(IEntity* pEntity, IPhysicalEntity* pPhysEntity, IPhysicalEntity* pSrcPhysEntity);

	bool                                  IsImmersiveMPEnabled();

	virtual void                          AllowSave(bool bAllow = true)
	{
		m_bAllowSave = bAllow;
	}

	virtual void AllowLoad(bool bAllow = true)
	{
		m_bAllowLoad = bAllow;
	}

	virtual bool                  CanSave();
	virtual bool                  CanLoad();

	virtual ISerializeHelper*     GetSerializeHelper() const;

	virtual bool                  CanCheat();

	virtual INetNub*              GetServerNetNub();
	virtual IGameServerNub*       GetIGameServerNub();
	virtual INetNub*              GetClientNetNub();
	virtual IGameClientNub*       GetIGameClientNub();

	void                          SetGameGUID(tukk gameGUID);
	tukk                   GetGameGUID()             { return m_gameGUID; }

	virtual bool                  IsVoiceRecordingEnabled() { return m_VoiceRecordingEnabled != 0; }

	virtual bool                  IsGameSession(DrxSessionHandle sessionHandle);
	virtual bool                  ShouldMigrateNub(DrxSessionHandle sessionHandle);

	virtual ISharedParamsUpr* GetISharedParamsUpr();

	virtual IGame*                GetIGame();
	virtual uk GetGameModuleHandle() const { return m_externalGameLibrary.dllHandle; }

	virtual float                 GetLoadSaveDelay() const { return m_lastSaveLoad; }

	virtual IGameVolumes*         GetIGameVolumesUpr() const;

	virtual void                  PreloadAnimatedCharacter(IScriptTable* pEntityScript);
	virtual void                  PrePhysicsTimeStep(float deltaTime);

	virtual void                  RegisterExtension(IDrxUnknownPtr pExtension);
	virtual void                  ReleaseExtensions();

	virtual void AddNetworkedClientListener(INetworkedClientListener& listener) { stl::push_back_unique(m_networkClientListeners, &listener); }
	virtual void RemoveNetworkedClientListener(INetworkedClientListener& listener) { stl::find_and_erase(m_networkClientListeners, &listener); }

	void DefineProtocolRMI(IProtocolBuilder* pBuilder);
	virtual void DoInvokeRMI(_smart_ptr<IRMIMessageBody> pBody, unsigned where, i32 channel, const bool isGameObjectRmi);

protected:
	virtual IDrxUnknownPtr        QueryExtensionInterfaceById(const DrxInterfaceID& interfaceID) const;
	// ~IGameFramework

public:

	static CDrxAction*          GetDrxAction() { return m_pThis; }

	virtual CGameServerNub*     GetGameServerNub();
	CGameClientNub*             GetGameClientNub();
	CGameContext*               GetGameContext();
	CScriptBind_Vehicle*        GetVehicleScriptBind()     { return m_pScriptBindVehicle; }
	CScriptBind_VehicleSeat*    GetVehicleSeatScriptBind() { return m_pScriptBindVehicleSeat; }
	CScriptBind_Inventory*      GetInventoryScriptBind()   { return m_pScriptInventory; }
	CPersistantDebug*           GetPersistantDebug()       { return m_pPersistantDebug; }
	CSignalTimer*               GetSignalTimer();
	CRangeSignaling*            GetRangeSignaling();
	virtual IPersistantDebug*   GetIPersistantDebug();
	virtual IGameStatsConfig*   GetIGameStatsConfig();
	CColorGradientUpr*      GetColorGradientUpr() const { return m_pColorGradientUpr; }

	virtual void                AddBreakEventListener(IBreakEventListener* pListener);
	virtual void                RemoveBreakEventListener(IBreakEventListener* pListener);

	void                        OnBreakEvent(u16 uBreakEventIndex);
	void                        OnPartRemoveEvent(i32 iPartRemoveEventIndex);

	virtual void                RegisterListener(IGameFrameworkListener* pGameFrameworkListener, tukk name, EFRAMEWORKLISTENERPRIORITY eFrameworkListenerPriority);
	virtual void                UnregisterListener(IGameFrameworkListener* pGameFrameworkListener);

	CDialogSystem*              GetDialogSystem()             { return m_pDialogSystem; }
	CTimeOfDayScheduler*        GetTimeOfDayScheduler() const { return m_pTimeOfDayScheduler; }

	CGameStatsConfig*           GetGameStatsConfig();
	IGameStatistics*            GetIGameStatistics();

	IGameSessionHandler*        GetIGameSessionHandler();
	void                        SetGameSessionHandler(IGameSessionHandler* pSessionHandler);

	CNetMessageDistpatcher*     GetNetMessageDispatcher()      { return m_pNetMsgDispatcher; }
	CEntityContainerMgr&         GetEntityContainerMgr()       { return *m_pEntityContainerMgr; }
	CEntityAttachmentExNodeRegistry& GetEntityAttachmentExNodeRegistry() { return *m_pEntityAttachmentExNodeRegistry; }

	//	INetQueryListener* GetLanQueryListener() {return m_pLanQueryListener;}
	bool                          LoadingScreenEnabled() const;

	i32                           NetworkExposeClass(IFunctionHandler* pFH);

	void                          NotifyGameFrameworkListeners(ISaveGame* pSaveGame);
	void                          NotifyGameFrameworkListeners(ILoadGame* pLoadGame);
	void                          NotifySavegameFileLoadedToListeners(tukk pLevelName);
	void                          NotifyForceFlashLoadingListeners();
	virtual void                  EnableVoiceRecording(const bool enable);
	virtual void                  MutePlayerById(EntityId mutePlayer);
	virtual IDebugHistoryUpr* CreateDebugHistoryUpr();
	virtual void                  ExecuteCommandNextFrame(tukk cmd);
	virtual tukk           GetNextFrameCommand() const;
	virtual void                  ClearNextFrameCommand();
	virtual void                  PrefetchLevelAssets(const bool bEnforceAll);

	virtual void                  ShowPageInBrowser(tukk URL);
	virtual bool                  StartProcess(tukk cmd_line);
	virtual bool                  SaveServerConfig(tukk path);

	void                          OnActionEvent(const SActionEvent& ev);

	bool                          IsPbSvEnabled() const { return m_pbSvEnabled; }
	bool                          IsPbClEnabled() const { return m_pbClEnabled; }

	void                          DumpMemInfo(tukk format, ...) PRINTF_PARAMS(2, 3);

	tukk                   GetStartLevelSaveGameName();

	virtual IAIActorProxy*        GetAIActorProxy(EntityId entityid) const;
	CAIProxyUpr*              GetAIProxyUpr()       { return m_pAIProxyUpr; }
	const CAIProxyUpr*        GetAIProxyUpr() const { return m_pAIProxyUpr; }

	void                          CreatePhysicsQueues();
	void                          ClearPhysicsQueues();
	CDrxActionPhysicQueues& GetPhysicQueues();
	bool                    IsGameSessionMigrating();

	void                    StartNetworkStallTicker(bool includeMinimalUpdate);
	void                    StopNetworkStallTicker();
	void                    GoToSegment(i32 x, i32 y);

	const std::vector<INetworkedClientListener*>& GetNetworkClientListeners() const { return m_networkClientListeners; }
	void FastShutdown();

private:
	bool Initialize(SSysInitParams& initParams);

	void InitScriptBinds();
	void ReleaseScriptBinds();

	bool InitGame(SSysInitParams& startupParams);
	bool ShutdownGame();

	void InitForceFeedbackSystem();
	void InitGameVolumesUpr();

	void InitCVars();
	void ReleaseCVars();

	void InitCommands();

	// console commands provided by DinrusAction
	static void DumpMapsCmd(IConsoleCmdArgs* args);
	static void MapCmd(IConsoleCmdArgs* args);
	static void ReloadReadabilityXML(IConsoleCmdArgs* args);
	static void UnloadCmd(IConsoleCmdArgs* args);
	static void PlayCmd(IConsoleCmdArgs* args);
	static void ConnectCmd(IConsoleCmdArgs* args);
	static void DisconnectCmd(IConsoleCmdArgs* args);
	static void DisconnectChannelCmd(IConsoleCmdArgs* args);
	static void StatusCmd(IConsoleCmdArgs* args);
	static void LegacyStatusCmd(IConsoleCmdArgs* args);
	static void VersionCmd(IConsoleCmdArgs* args);
	static void SaveTagCmd(IConsoleCmdArgs* args);
	static void LoadTagCmd(IConsoleCmdArgs* args);
	static void SaveGameCmd(IConsoleCmdArgs* args);
	static void GenStringsSaveGameCmd(IConsoleCmdArgs* args);
	static void LoadGameCmd(IConsoleCmdArgs* args);
	static void KickPlayerCmd(IConsoleCmdArgs* args);
	static void LegacyKickPlayerCmd(IConsoleCmdArgs* args);
	static void KickPlayerByIdCmd(IConsoleCmdArgs* args);
	static void LegacyKickPlayerByIdCmd(IConsoleCmdArgs* args);
	static void BanPlayerCmd(IConsoleCmdArgs* args);
	static void LegacyBanPlayerCmd(IConsoleCmdArgs* args);
	static void BanStatusCmd(IConsoleCmdArgs* args);
	static void LegacyBanStatusCmd(IConsoleCmdArgs* args);
	static void UnbanPlayerCmd(IConsoleCmdArgs* args);
	static void LegacyUnbanPlayerCmd(IConsoleCmdArgs* args);
	static void OpenURLCmd(IConsoleCmdArgs* args);
	static void TestResetCmd(IConsoleCmdArgs* args);

	static void DumpAnalysisStatsCmd(IConsoleCmdArgs* args);

#if !defined(_RELEASE)
	static void ConnectRepeatedlyCmd(IConsoleCmdArgs* args);
#endif

	static void TestTimeout(IConsoleCmdArgs* args);
	static void TestNSServerBrowser(IConsoleCmdArgs* args);
	static void TestNSServerReport(IConsoleCmdArgs* args);
	static void TestNSChat(IConsoleCmdArgs* args);
	static void TestNSStats(IConsoleCmdArgs* args);
	static void TestNSNat(IConsoleCmdArgs* args);
	static void TestPlayerBoundsCmd(IConsoleCmdArgs* args);
	static void DelegateCmd(IConsoleCmdArgs* args);
	static void DumpStatsCmd(IConsoleCmdArgs* args);

	// console commands for the remote control system
	//static void rcon_password(IConsoleCmdArgs* args);
	static void rcon_startserver(IConsoleCmdArgs* args);
	static void rcon_stopserver(IConsoleCmdArgs* args);
	static void rcon_connect(IConsoleCmdArgs* args);
	static void rcon_disconnect(IConsoleCmdArgs* args);
	static void rcon_command(IConsoleCmdArgs* args);

	static struct IRemoteControlServer* s_rcon_server;
	static struct IRemoteControlClient* s_rcon_client;

	static class CRConClientListener*   s_rcon_client_listener;

	//static string s_rcon_password;

	// console commands for the simple http server
	static void http_startserver(IConsoleCmdArgs* args);
	static void http_stopserver(IConsoleCmdArgs* args);

	static struct ISimpleHttpServer* s_http_server;

	// change the game query (better than setting it explicitly)
	void SetGameQueryListener(CGameQueryListener*);

	void CheckEndLevelSchedule();

#if !defined(_RELEASE)
	void CheckConnectRepeatedly();
#endif

	static void MutePlayer(IConsoleCmdArgs* pArgs);

	static void VerifyMaxPlayers(ICVar* pVar);
	static void ResetComments(ICVar* pVar);

	static void StaticSetPbSvEnabled(IConsoleCmdArgs* pArgs);
	static void StaticSetPbClEnabled(IConsoleCmdArgs* pArgs);

	// NOTE: anything owned by this class should be a pointer or a simple
	// type - nothing that will need its constructor called when DinrusAction's
	// constructor is called (we don't have access to malloc() at that stage)

	bool                          m_paused;
	bool                          m_forcedpause;

	static CDrxAction*            m_pThis;

	ISystem*                      m_pSystem;
	INetwork*                     m_pNetwork;
	I3DEngine*                    m_p3DEngine;
	IScriptSystem*                m_pScriptSystem;
	IEntitySystem*                m_pEntitySystem;
	ITimer*                       m_pTimer;
	ILog*                         m_pLog;
	IGameToEditorInterface*       m_pGameToEditor;

	_smart_ptr<CActionGame>       m_pGame;

	char                          m_editorLevelName[512]; // to avoid having to call string constructor, or allocating memory.
	char                          m_editorLevelFolder[512];
	char                          m_gameGUID[128];

	CLevelSystem*                 m_pLevelSystem;
	CActorSystem*                 m_pActorSystem;
	CItemSystem*                  m_pItemSystem;
	CVehicleSystem*               m_pVehicleSystem;
	CSharedParamsUpr*         m_pSharedParamsUpr;
	CActionMapUpr*            m_pActionMapUpr;
	CViewSystem*                  m_pViewSystem;
	CGameplayRecorder*            m_pGameplayRecorder;
	CGameRulesSystem*             m_pGameRulesSystem;

	CGameObjectSystem*            m_pGameObjectSystem;
	CUIDraw*                      m_pUIDraw;
	CScriptRMI*                   m_pScriptRMI;
	CAnimationGraphCVars*         m_pAnimationGraphCvars;
	IMannequin*                   m_pMannequin;
	CMaterialEffects*             m_pMaterialEffects;
	IBreakableGlassSystem*        m_pBreakableGlassSystem;
	CPlayerProfileUpr*        m_pPlayerProfileUpr;
	CDialogSystem*                m_pDialogSystem;
	CSubtitleUpr*             m_pSubtitleUpr;

	IEffectSystem*                m_pEffectSystem;
	CGameSerialize*               m_pGameSerialize;
	CallbackTimer*                m_pCallbackTimer;
	CGameplayAnalyst*             m_pGameplayAnalyst;
	CForceFeedBackSystem*         m_pForceFeedBackSystem;
	//	INetQueryListener *m_pLanQueryListener;
	ILanQueryListener*            m_pLanQueryListener;
	CCustomActionUpr*         m_pCustomActionUpr;
	CCustomEventUpr*          m_pCustomEventUpr;

	CGameStatsConfig*             m_pGameStatsConfig;

	IGameStatistics*              m_pGameStatistics;

	ICooperativeAnimationUpr* m_pCooperativeAnimationUpr;
	IGameSessionHandler*          m_pGameSessionHandler;

	CAIProxyUpr*              m_pAIProxyUpr;

	IGameVolumes*                 m_pGameVolumesUpr;

	// developer mode
	CDevMode* m_pDevMode;

	// TimeDemo recorder.
	CTimeDemoRecorder* m_pTimeDemoRecorder;

	// game queries
	CGameQueryListener* m_pGameQueryListener;

	// Currently handles the automatic creation of vegetation areas
	CRuntimeAreaUpr* m_pRuntimeAreaUpr;

	// script binds
	CScriptBind_Action*           m_pScriptA;
	CScriptBind_ItemSystem*       m_pScriptIS;
	CScriptBind_ActorSystem*      m_pScriptAS;
	CScriptBind_Network*          m_pScriptNet;
	CScriptBind_ActionMapUpr* m_pScriptAMM;
	CScriptBind_VehicleSystem*    m_pScriptVS;
	CScriptBind_Vehicle*          m_pScriptBindVehicle;
	CScriptBind_VehicleSeat*      m_pScriptBindVehicleSeat;
	CScriptBind_Inventory*        m_pScriptInventory;
	CScriptBind_DialogSystem*     m_pScriptBindDS;
	CScriptBind_MaterialEffects*  m_pScriptBindMFX;
	CScriptBind_UIAction*         m_pScriptBindUIAction;
	CTimeOfDayScheduler*          m_pTimeOfDayScheduler;
	CPersistantDebug*             m_pPersistantDebug;

	CAIDebugRenderer*             m_pAIDebugRenderer;
	CAINetworkDebugRenderer*      m_pAINetworkDebugRenderer;

	CNetworkCVars*                m_pNetworkCVars;
	CDrxActionCVars*              m_pDrxActionCVars;

	CColorGradientUpr*        m_pColorGradientUpr;

	//-- Network Stall ticker thread
#ifdef USE_NETWORK_STALL_TICKER_THREAD
	CNetworkStallTickerThread* m_pNetworkStallTickerThread;
	u32                     m_networkStallTickerReferences;
#endif // #ifdef USE_NETWORK_STALL_TICKER_THREAD

	// Console Variables with some DinrusAction as owner
	CMaterialEffectsCVars*  m_pMaterialEffectsCVars;

	CDrxActionPhysicQueues* m_pPhysicsQueues;

	typedef std::vector<IDrxUnknownPtr> TFrameworkExtensions;
	TFrameworkExtensions m_frameworkExtensions;

	// console variables
	ICVar* m_pEnableLoadingScreen;
	ICVar* m_pCheats;
	ICVar* m_pShowLanBrowserCVAR;
	ICVar* m_pDebugSignalTimers;
	ICVar* m_pDebugRangeSignaling;
	ICVar* m_pAsyncLevelLoad;

	bool   m_bShowLanBrowser;
	//
	bool   m_isEditing;
	bool   m_bScheduleLevelEnd;

	enum ESaveGameMethod
	{
		eSGM_NoSave = 0,
		eSGM_QuickSave,
		eSGM_Save
	};

	ESaveGameMethod m_delayedSaveGameMethod;     // 0 -> no save, 1=quick save, 2=save, not quick
	ESaveGameReason m_delayedSaveGameReason;
	i32             m_delayedSaveCountDown;

	struct SExternalGameLibrary
	{
		string        dllName;
		HMODULE       dllHandle;
		IGameStartup* pGameStartup;
		IGame*        pGame;

		SExternalGameLibrary() : dllName(""), dllHandle(0), pGameStartup(nullptr), pGame(nullptr) {}
		bool IsValid() const { return (pGameStartup != nullptr && pGame != nullptr); }
		void Reset()         { dllName = ""; dllHandle = 0; pGameStartup = nullptr; pGame = nullptr; }
	};

	struct SLocalAllocs
	{
		string m_delayedSaveGameName;
		string m_checkPointName;
		string m_nextLevelToLoad;
	};
	SLocalAllocs* m_pLocalAllocs;

	struct SGameFrameworkListener
	{
		IGameFrameworkListener*    pListener;
		DrxStackStringT<char, 64>  name;
		EFRAMEWORKLISTENERPRIORITY eFrameworkListenerPriority;
		SGameFrameworkListener() : pListener(0), eFrameworkListenerPriority(FRAMEWORKLISTENERPRIORITY_DEFAULT) {}
		void                       GetMemoryUsage(IDrxSizer* pSizer) const {}
	};

	typedef std::vector<SGameFrameworkListener> TGameFrameworkListeners;
	TGameFrameworkListeners* m_pGFListeners;
	IBreakEventListener*     m_pBreakEventListener;
	std::vector<bool>        m_validListeners;

	i32                      m_VoiceRecordingEnabled;

	bool                     m_bAllowSave;
	bool                     m_bAllowLoad;
	string*                  m_nextFrameCommand;
	string*                  m_connectServer;

#if !defined(_RELEASE)
	struct SConnectRepeatedly
	{
		bool  m_enabled;
		i32   m_numAttemptsLeft;
		float m_timeForNextAttempt;

		SConnectRepeatedly() : m_enabled(false), m_numAttemptsLeft(0), m_timeForNextAttempt(0.0f) {}
	} m_connectRepeatedly;
#endif

	float  m_lastSaveLoad;
	float  m_lastFrameTimeUI;

	bool   m_pbSvEnabled;
	bool   m_pbClEnabled;
	u32 m_PreUpdateTicks;


	SExternalGameLibrary                   m_externalGameLibrary;

	CNetMessageDistpatcher*                m_pNetMsgDispatcher;
	CEntityContainerMgr*                   m_pEntityContainerMgr;
	CEntityAttachmentExNodeRegistry*       m_pEntityAttachmentExNodeRegistry;

	CTimeValue                             m_levelStartTime;

	std::vector<INetworkedClientListener*> m_networkClientListeners;
};
