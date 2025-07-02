// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/Phys/IPhysics.h>
#include <drx3D/Sys/IWindowMessageHandler.h>

#include <drx3D/CoreX/Math/Random.h>
#include <drx3D/CoreX/Platform/DrxLibrary.h>

#include <drx3D/Sys/Timer.h>
#include <drx3D/Sys/DrxVersion.h>
#include <drx3D/Sys/CmdLine.h>
#include <drx3D/CoreX/String/DrxName.h>
#include <drx3D/CoreX/Math/Drx_Camera.h>

#include <drx3D/Sys/FrameProfileSystem.h>
#include <drx3D/Sys/MTSafeAllocator.h>
#include <drx3D/Sys/CPUDetect.h>
#include <drx3D/Sys/PakVars.h>
#include <drx3D/Sys/MemoryFragmentationProfiler.h>  // CMemoryFragmentationProfiler

#include <drx3D/Sys/DrxPluginUpr.h>
#include <drx3D/Sys/UserAnalyticsSystem.h>

#include <bitset>

struct IConsoleCmdArgs;
class CServerThrottle;
struct IDrxFactoryRegistryImpl;
struct IZLibCompressor;
class CLoadingProfilerSystem;
struct SThreadMetaData;
class CResourceUpr;
class CThreadUpr;
class IPhysicsThreadTask;
class IDrxSizer;
class CStreamEngine;
class CXmlUtils;
class CJSONUtils;
class DrxSizerStats;
class DrxSizerImpl;
class CThreadProfiler;
class IDiskProfiler;
class CLocalizedStringsUpr;
class CDownloadUpr;
struct IDrxPerfHUD;
class CNULLRenderAuxGeom;
class CManualFrameStepController;
class CProjectUpr;
struct DisplayContext;

namespace minigui
{
struct IMiniGUI;
}

namespace LiveCreate
{
struct IUpr;
struct IHost;
}

#if DRX_PLATFORM_ANDROID
	#define USE_ANDROIDCONSOLE
#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_MAC
	#define USE_UNIXCONSOLE
#elif DRX_PLATFORM_IOS
	#define USE_IOSCONSOLE
#elif DRX_PLATFORM_WINDOWS
	#define USE_WINDOWSCONSOLE
#endif

#if defined(USE_UNIXCONSOLE) || defined(USE_ANDROIDCONSOLE) || defined(USE_WINDOWSCONSOLE) || defined(USE_IOSCONSOLE)
	#define USE_DEDICATED_SERVER_CONSOLE
#endif

#if !DRX_PLATFORM_LINUX && !DRX_PLATFORM_ANDROID && !DRX_PLATFORM_DURANGO && !DRX_PLATFORM_APPLE && !DRX_PLATFORM_ORBIS
	#define DOWNLOAD_MANAGER
#endif

#ifdef DOWNLOAD_MANAGER
	#include <drx3D/Sys/DownloadUpr.h>
#endif //DOWNLOAD_MANAGER

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	#include <drx3D/CoreX/Platform/DrxLibrary.h>
#endif

#define NUM_UPDATE_TIMES (128U)

typedef uk WIN_HMODULE;

#if !defined(DRX_ASYNC_MEMCPY_DELEGATE_TO_DRXSYSTEM)
DRX_ASYNC_MEMCPY_API void drxAsyncMemcpy(uk dst, ukk src, size_t size, i32 nFlags,  i32* sync);
#else
DRX_ASYNC_MEMCPY_API void _drxAsyncMemcpy(uk dst, ukk src, size_t size, i32 nFlags,  i32* sync);
#endif

//forward declarations
class CScriptSink;
class CLUADbg;
struct SDefaultValidator;
class CPhysRenderer;
class CVisRegTest;
class CThreadProfiler;
class CImeUpr;

#define PHSYICS_OBJECT_ENTITY    0

#define MAX_STREAMING_POOL_INDEX 6
#define MAX_THREAD_POOL_INDEX    6

struct SSysCVars
{
	i32    sys_streaming_requests_grouping_time_period;
	i32    sys_streaming_sleep;
	i32    sys_streaming_memory_budget;
	i32    sys_streaming_max_finalize_per_frame;
	float  sys_streaming_max_bandwidth;
	i32    sys_streaming_debug;
	i32    sys_streaming_resetstats;
	i32    sys_streaming_debug_filter;
	float  sys_streaming_debug_filter_min_time;
	i32    sys_streaming_use_optical_drive_thread;
	ICVar* sys_streaming_debug_filter_file_name;
	ICVar* sys_localization_folder;
	ICVar* sys_build_folder;
	i32    sys_streaming_in_blocks;

	i32    sys_float_exceptions;
	i32    sys_no_crash_dialog;
	i32    sys_dump_aux_threads;
	i32    sys_keyboard_break;
	i32    sys_WER;
	i32    sys_dump_type;
	i32    sys_ai;
	i32    sys_physics;
	i32    sys_entitysystem;
	i32    sys_trackview;
	i32    sys_livecreate;
	i32    sys_vtune;
	float  sys_update_profile_time;
	i32    sys_limit_phys_thread_count;
	i32    sys_usePlatformSavingAPI;
#ifndef _RELEASE
	i32    sys_usePlatformSavingAPIEncryption;
#endif
	i32    sys_MaxFPS;
	float  sys_maxTimeStepForMovieSystem;
	i32    sys_force_installtohdd_mode;

#ifdef USE_HTTP_WEBSOCKETS
	i32 sys_simple_http_base_port;
#endif

	i32 sys_log_asserts;
	i32 sys_error_debugbreak;

	i32 sys_enable_crash_handler;

	i32 sys_intromoviesduringinit;
	ICVar* sys_splashscreen;

	i32 sys_deferAudioUpdateOptim;
	i32     sys_filesystemCaseSensitivity;

	PakVars pakVars;

#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO
	i32 sys_display_threads;
#endif

#if DRX_PLATFORM_WINDOWS
	i32 sys_highrestimer;
#endif

	i32 sys_vr_support;
};
extern SSysCVars g_cvars;

class CSystem;

struct SmallModuleInfo
{
	string              name;
	DrxModuleMemoryInfo memInfo;
};

struct SDinrusXStatsModuleInfo
{
	string              name;
	DrxModuleMemoryInfo memInfo;
	u32              moduleStaticSize;
	u32              usedInModule;
	u32              SizeOfCode;
	u32              SizeOfInitializedData;
	u32              SizeOfUninitializedData;
};

struct SDinrusXStatsGlobalMemInfo
{
	i32    totalUsedInModules;
	i32    totalCodeAndStatic;
	i32    countedMemoryModules;
	uint64 totalAllocatedInModules;
	i32    totalNumAllocsInModules;
	std::vector<SDinrusXStatsModuleInfo> modules;
};

struct CProfilingSystem : public IProfilingSystem
{
	//////////////////////////////////////////////////////////////////////////
	// VTune Profiling interface.

	// Summary:
	//	 Resumes vtune data collection.
	virtual void VTuneResume();
	// Summary:
	//	 Pauses vtune data collection.
	virtual void VTunePause();
	//////////////////////////////////////////////////////////////////////////
};

/*
   ===========================================
   The System interface Class
   ===========================================
 */
class CXConsole;

class CSystem final : public ISystem, public ILoadConfigurationEntrySink, public ISystemEventListener, public IWindowMessageHandler
{
public:
	CSystem(const SSysInitParams& startupParams);
	~CSystem();
	bool        IsUIFrameworkMode() override { return m_bUIFrameworkMode; }

	static void OnLanguageCVarChanged(ICVar* const pLanguage);
	static void OnLanguageAudioCVarChanged(ICVar* const pLanguageAudio);
	static void OnLocalizationFolderCVarChanged(ICVar* const pLocalizationFolder);

	// interface ILoadConfigurationEntrySink ----------------------------------

	virtual void OnLoadConfigurationEntry(tukk szKey, tukk szValue, tukk szGroup) override;

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;

	///////////////////////////////////////////////////////////////////////////
	//! @name ISystem implementation
	//@{
	virtual SSysGlobEnv* GetGlobalEnvironment() override { return &m_env; }

	tukk                       GetRootFolder() const override  { return m_root.c_str(); }

	virtual bool                        DoFrame(const SDisplayContextKey& displayContextKey = SDisplayContextKey{}, CEnumFlags<ESystemUpdateFlags> updateFlags = CEnumFlags<ESystemUpdateFlags>()) override;
	virtual IManualFrameStepController* GetManualFrameStepController() const override;

	virtual bool                      UpdateLoadtime() override;

	//! Begin rendering frame.
	virtual void RenderBegin(const SDisplayContextKey& displayContextKey) override;
	//! Render subsystems.
	void Render();
	//! End rendering frame and swap back buffer.
	virtual void RenderEnd(bool bRenderStats = true) override;

	virtual bool Update(CEnumFlags<ESystemUpdateFlags> updateFlags = CEnumFlags<ESystemUpdateFlags>(), i32 nPauseMode = 0) override;

	virtual void DoWorkDuringOcclusionChecks() override;
	virtual bool NeedDoWorkDuringOcclusionChecks() override { return m_bNeedDoWorkDuringOcclusionChecks; }

	virtual void RenderPhysicsHelpers() override;

	//! Update screen during loading.
	void UpdateLoadingScreen();

	//! Update screen and call some important tick functions during loading.
	void SynchronousLoadingTick(tukk pFunc, i32 line) override;

	//! Renders the statistics; this is called from RenderEnd, but if the
	//! Host application (Editor) doesn't employ the Render cycle in ISystem,
	//! it may call this method to render the essencial statistics
	void         RenderStatistics() override;
	void         RenderPhysicsStatistics(IPhysicalWorld* pWorld) override;

	u32       GetUsedMemory() override;

	virtual void DumpMemoryUsageStatistics(bool bUseKB) override;
	virtual void DumpMemoryCoverage() override;
	void         CollectMemInfo(SDinrusXStatsGlobalMemInfo&);

#ifndef _RELEASE
	virtual void GetCheckpointData(ICheckpointData& data) override;
	virtual void IncreaseCheckpointLoadCount() override;
	virtual void SetLoadOrigin(LevelLoadOrigin origin) override;
#endif

	void                         Relaunch(bool bRelaunch) override;
	bool                         IsRelaunch() const override        { return m_bRelaunch; };
	void                         SerializingFile(i32 mode) override { m_iLoadingMode = mode; }
	i32                          IsSerializingFile() const override { return m_iLoadingMode; }
	void                         Quit() override;
	bool                         IsQuitting() const override;
	bool                         IsShaderCacheGenMode() const override { return m_bShaderCacheGenMode; }
	virtual tukk          GetUserName() override;
	virtual i32                  GetApplicationInstance() override;
	virtual sUpdateTimes&        GetCurrentUpdateTimeStats() override;
	virtual const sUpdateTimes*  GetUpdateTimeStats(u32&, u32&) override;
	virtual void                 FillRandomMT(u32* pOutWords, u32 numWords) override;

	virtual CRndGen&             GetRandomGenerator() override   { return m_randomGenerator; }

	INetwork*                    GetINetwork() override          { return m_env.pNetwork; }
	IRenderer*                   GetIRenderer() override         { return m_env.pRenderer; }
	IInput*                      GetIInput() override            { return m_env.pInput; }
	ITimer*                      GetITimer() override            { return m_env.pTimer; }
	IDrxPak*                     GetIPak() override              { return m_env.pDrxPak; };
	IConsole*                    GetIConsole() override          { return m_env.pConsole; };
	IRemoteConsole*              GetIRemoteConsole() override;
	IScriptSystem*               GetIScriptSystem() override     { return m_env.pScriptSystem; }
	I3DEngine*                   GetI3DEngine() override         { return m_env.p3DEngine; }
	ICharacterUpr*           GetIAnimationSystem() override  { return m_env.pCharacterUpr; }
	DrxAudio::IAudioSystem*      GetIAudioSystem() override      { return m_env.pAudioSystem; }
	IPhysicalWorld*              GetIPhysicalWorld() override    { return m_env.pPhysicalWorld; }
	IMovieSystem*                GetIMovieSystem() override      { return m_env.pMovieSystem; };
	IAISystem*                   GetAISystem() override          { return m_env.pAISystem; }
	IMemoryUpr*              GetIMemoryUpr() override    { return m_pMemoryUpr; }
	IEntitySystem*               GetIEntitySystem() override     { return m_env.pEntitySystem; }
	LiveCreate::IHost*           GetLiveCreateHost()             { return m_env.pLiveCreateHost; }
	LiveCreate::IUpr*        GetLiveCreateUpr()          { return m_env.pLiveCreateUpr; }
	IThreadUpr*              GetIThreadUpr() override    { return m_env.pThreadUpr; }
	IMonoEngineModule*           GetIMonoEngineModule() override { return m_env.pMonoRuntime; }
	IDrxFont*                    GetIDrxFont() override          { return m_env.pDrxFont; }
	ILog*                        GetILog() override              { return m_env.pLog; }
	ICmdLine*                    GetICmdLine() override          { return m_pCmdLine; }
	IStreamEngine*               GetStreamEngine() override;
	IValidator*                  GetIValidator() override        { return m_pValidator; };
	IPhysicsDebugRenderer*       GetIPhysicsDebugRenderer() override;
	IPhysRenderer*               GetIPhysRenderer() override;
	IFrameProfileSystem*         GetIProfileSystem() override         { return &m_FrameProfileSystem; }
	virtual IDiskProfiler*       GetIDiskProfiler() override          { return m_pDiskProfiler; }
	CThreadProfiler*             GetThreadProfiler()                  { return m_pThreadProfiler; }
	INameTable*                  GetINameTable() override             { return m_env.pNameTable; };
	IBudgetingSystem*            GetIBudgetingSystem() override       { return(m_pIBudgetingSystem); }
	IFlowSystem*                 GetIFlowSystem() override            { return m_env.pFlowSystem; }
	IDialogSystem*               GetIDialogSystem() override          { return m_env.pDialogSystem; }
	DRS::IDynamicResponseSystem* GetIDynamicResponseSystem()          { return m_env.pDynamicResponseSystem; }
	IHardwareMouse*              GetIHardwareMouse() override         { return m_env.pHardwareMouse; }
	ISystemEventDispatcher*      GetISystemEventDispatcher() override { return m_pSystemEventDispatcher; }
	ITestSystem*                 GetITestSystem() override            { return m_pTestSystem.get(); }
	IUserAnalyticsSystem*        GetIUserAnalyticsSystem() override   { return m_pUserAnalyticsSystem; }
	Drx::IPluginUpr*         GetIPluginUpr() override         { return m_pPluginUpr; }
	IProjectUpr*             GetIProjectUpr() override;

	IResourceUpr*            GetIResourceUpr() override;
	ITextModeConsole*            GetITextModeConsole() override;
	IFileChangeMonitor*          GetIFileChangeMonitor() override   { return m_env.pFileChangeMonitor; }
	INotificationNetwork*        GetINotificationNetwork() override { return m_pNotificationNetwork; }
	IProfilingSystem*            GetIProfilingSystem() override     { return &m_ProfilingSystem; }
	IPlatformOS*                 GetPlatformOS() override           { return m_pPlatformOS.get(); }
	IDrxPerfHUD*                 GetPerfHUD() override              { return m_pPerfHUD; }
	IZLibCompressor*             GetIZLibCompressor() override      { return m_pIZLibCompressor; }
	IZLibDecompressor*           GetIZLibDecompressor() override    { return m_pIZLibDecompressor; }
	ILZ4Decompressor*            GetLZ4Decompressor() override      { return m_pILZ4Decompressor; }
	WIN_HWND                     GetHWND() override                 { return m_hWnd; }
	//////////////////////////////////////////////////////////////////////////
	// retrieves the perlin noise singleton instance
	CPNoise3*      GetNoiseGen() override;
	virtual uint64 GetUpdateCounter() override { return m_nUpdateCounter; }

	virtual void   SetLoadingProgressListener(ILoadingProgressListener* pLoadingProgressListener) override
	{
		m_pProgressListener = pLoadingProgressListener;
	};

	virtual ILoadingProgressListener* GetLoadingProgressListener() const override
	{
		return m_pProgressListener;
	};

	void         SetIFlowSystem(IFlowSystem* pFlowSystem) override                              { m_env.pFlowSystem = pFlowSystem; }
	void         SetIDialogSystem(IDialogSystem* pDialogSystem) override                        { m_env.pDialogSystem = pDialogSystem; }
	void         SetIDynamicResponseSystem(DRS::IDynamicResponseSystem* pDynamicResponseSystem) { m_env.pDynamicResponseSystem = pDynamicResponseSystem; }
	void         SetIMaterialEffects(IMaterialEffects* pMaterialEffects) override               { m_env.pMaterialEffects = pMaterialEffects; }
	void         SetIParticleUpr(IParticleUpr* pParticleUpr) override               { m_env.pParticleUpr = pParticleUpr; }
	void         SetIOpticsUpr(IOpticsUpr* pOpticsUpr) override                     { m_env.pOpticsUpr = pOpticsUpr; }
	void         SetIFileChangeMonitor(IFileChangeMonitor* pFileChangeMonitor) override         { m_env.pFileChangeMonitor = pFileChangeMonitor; }
	void         SetIFlashUI(IFlashUI* pFlashUI) override                                       { m_env.pFlashUI = pFlashUI; }
	void         SetIUIFramework(UIFramework::IUIFramework* pUIFramework) override              { m_env.pUIFramework = pUIFramework; }
	void         ChangeUserPath(tukk sUserPath) override;
	void         DetectGameFolderAccessRights();

	virtual void ExecuteCommandLine() override;

	virtual void GetUpdateStats(SSysUpdateStats& stats) override;

	//////////////////////////////////////////////////////////////////////////
	virtual XmlNodeRef CreateXmlNode(tukk sNodeName = "", bool bReuseStrings = false) override;
	virtual XmlNodeRef LoadXmlFromFile(tukk sFilename, bool bReuseStrings = false) override;
	virtual XmlNodeRef LoadXmlFromBuffer(tukk buffer, size_t size, bool bReuseStrings = false) override;
	virtual IXmlUtils* GetXmlUtils() override;
	//////////////////////////////////////////////////////////////////////////

	virtual Serialization::IArchiveHost* GetArchiveHost() const override         { return m_pArchiveHost; }

	void                                 SetViewCamera( CCamera& Camera) override;
	const CCamera&                       GetViewCamera() const override          { return m_ViewCamera; }

	virtual u32                       GetCPUFlags() override                  { return m_pCpu ? m_pCpu->GetFeatures() : 0; }
	virtual i32                          GetLogicalCPUCount() override           { return m_pCpu ? m_pCpu->GetLogicalCPUCount() : 0; }

	void                                 IgnoreUpdates(bool bIgnore) override    { m_bIgnoreUpdates = bIgnore; }
	void                                 SetGCFrequency(const float fRate);

	void                                 SetIProcess(IProcess* process) override;
	IProcess*                            GetIProcess() override { return m_pProcess; }
	//@}

	void                    SleepIfNeeded();

	virtual void            DisplayErrorMessage(tukk acMessage, float fTime, const float* pfColor = 0, bool bHardError = true) override;

	virtual void            FatalError(tukk format, ...) override PRINTF_PARAMS(2, 3);
	virtual void            ReportBug(tukk format, ...) override  PRINTF_PARAMS(2, 3);
	// Validator Warning.
	void                    WarningV(EValidatorModule module, EValidatorSeverity severity, i32 flags, tukk file, tukk format, va_list args) override;
	void                    Warning(EValidatorModule module, EValidatorSeverity severity, i32 flags, tukk file, tukk format, ...) override;
	void                    WarningOnce(EValidatorModule module, EValidatorSeverity severity, i32 flags, tukk file, tukk format, ...) override;
	bool                    CheckLogVerbosity(i32 verbosity) override;

	virtual void            DebugStats(bool checkpoint, bool leaks) override;
	void                    DumpWinHeaps() override;

	// tries to log the call stack . for DEBUG purposes only
	void        LogCallStack();

	virtual i32 DumpMMStats(bool log) override;

#if defined(CVARS_WHITELIST)
	virtual ICVarsWhitelist*             GetCVarsWhiteList() const                    { return m_pCVarsWhitelist; };
	virtual ILoadConfigurationEntrySink* GetCVarsWhiteListConfigSink() const override { return m_pCVarsWhitelistConfigSink; }
#else
	virtual ILoadConfigurationEntrySink* GetCVarsWhiteListConfigSink() const override { return NULL; }
#endif // defined(CVARS_WHITELIST)

	virtual ISystemUserCallback* GetUserCallback() const override { return m_pUserCallback; }

	//////////////////////////////////////////////////////////////////////////
	virtual void              SaveConfiguration() override;
	virtual void              LoadConfiguration(tukk sFilename, ILoadConfigurationEntrySink* pSink = 0, ELoadConfigurationType configType = eLoadConfigDefault) override;
	virtual ESystemConfigSpec GetConfigSpec(bool bClient = true) override;
	virtual void              SetConfigSpec(ESystemConfigSpec spec, bool bClient) override;
	virtual ESystemConfigSpec GetMaxConfigSpec() const override;
	//////////////////////////////////////////////////////////////////////////

	virtual i32        SetThreadState(ESubsystem subsys, bool bActive) override;
	virtual IDrxSizer* CreateSizer() override;
	virtual bool       IsPaused() const override { return m_bPaused; }

	//////////////////////////////////////////////////////////////////////////
	virtual IHmdUpr* GetHmdUpr() override { return m_pHmdUpr; }

	//////////////////////////////////////////////////////////////////////////
	// Creates an instance of the AVI Reader class.
	virtual IAVI_Reader*          CreateAVIReader() override;
	// Release the AVI reader
	virtual void                  ReleaseAVIReader(IAVI_Reader* pAVIReader) override;

	virtual ILocalizationUpr* GetLocalizationUpr() override;
	virtual void                  debug_GetCallStack(tukk* pFunctions, i32& nCount) override;
	virtual void                  debug_LogCallStack(i32 nMaxFuncs = 32, i32 nFlags = 0) override;
	// Get the current callstack in raw address form (more lightweight than the above functions)
	// static as memReplay needs it before CSystem has been setup - expose a ISystem interface to this function if you need it outside DinrusSystem
	static void                  debug_GetCallStackRaw(uk * callstack, u32& callstackLength);
	virtual void                 ApplicationTest(tukk szParam) override;

	virtual IDrxFactoryRegistry* GetDrxFactoryRegistry() const override;

public:
	bool Initialize(SSysInitParams& initParams);
	void RunMainLoop();

	// this enumeration describes the purpose for which the statistics is gathered.
	// if it's gathered to be dumped, then some different rules may be applied
	enum MemStatsPurposeEnum {nMSP_ForDisplay, nMSP_ForDump, nMSP_ForCrashLog, nMSP_ForBudget};
	// collects the whole memory statistics into the given sizer object
	void         CollectMemStats(IDrxSizer* pSizer, MemStatsPurposeEnum nPurpose = nMSP_ForDisplay, std::vector<SmallModuleInfo>* pStats = 0);
	void         GetExeSizes(IDrxSizer* pSizer, MemStatsPurposeEnum nPurpose = nMSP_ForDisplay);
	//! refreshes the m_pMemStats if necessary; creates it if it's not created
	void         TickMemStats(MemStatsPurposeEnum nPurpose = nMSP_ForDisplay, IResourceCollector* pResourceCollector = 0);

	void         SetVersionInfo(tukk const szVersion);

	virtual IDrxFactory* LoadModuleWithFactory(tukk dllName, const DrxInterfaceID& moduleInterfaceId) override;
	virtual bool UnloadEngineModule(tukk dllName) override;

#if DRX_PLATFORM_WINDOWS
	//friend
	static LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
	virtual uk         GetRootWindowMessageHandler() override;
	virtual void          RegisterWindowMessageHandler(IWindowMessageHandler* pHandler) override;
	virtual void          UnregisterWindowMessageHandler(IWindowMessageHandler* pHandler) override;
	virtual i32           PumpWindowMessage(bool bAll, WIN_HWND hWnd) override;
	virtual bool          IsImeSupported() const override;
	virtual IImeUpr*  GetImeUpr() const override { return m_pImeUpr; }

	// IWindowMessageHandler
#if DRX_PLATFORM_WINDOWS
	virtual bool HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
#endif
	// ~IWindowMessageHandler

	WIN_HMODULE LoadDynamicLibrary(tukk dllName, bool bQuitIfNotFound = true, bool bLogLoadingInfo = false);
	bool        UnloadDynamicLibrary(tukk dllName);
	void        GetLoadedDynamicLibraries(std::vector<string>& moduleNames) const;

private:

	bool InitializeEngineModule(const SSysInitParams& startupParams, tukk dllName, const DrxInterfaceID& moduleInterfaceId, bool bQuitIfNotFound);

	// Release all resources.
	void ShutDown();

	void SleepIfInactive();

	//! @name Initialization routines
	//@{

	bool InitNetwork(const SSysInitParams& startupParams);
	bool InitInput(const SSysInitParams& startupParams);

	bool InitConsole();
	bool InitRenderer(SSysInitParams& startupParams);
	bool InitPhysics(const SSysInitParams& startupParams);
	bool InitPhysicsRenderer(const SSysInitParams& startupParams);

	bool InitFont(const SSysInitParams& startupParams);
	bool InitFlash();
	bool InitAISystem(const SSysInitParams& startupParams);
	bool InitScriptSystem(const SSysInitParams& startupParams);
	bool InitFileSystem(const SSysInitParams& startupParams);
	void InitLog(const SSysInitParams& startupParams);
	void LoadPatchPaks();
	bool InitFileSystem_LoadEngineFolders();
	bool InitStreamEngine();
	bool Init3DEngine(const SSysInitParams& startupParams);
	bool InitAnimationSystem(const SSysInitParams& startupParams);
	bool InitMovieSystem(const SSysInitParams& startupParams);
	bool InitReflectionSystem(const SSysInitParams& startupParams);
	bool InitSchematyc(const SSysInitParams& startupParams);
	bool InitEntitySystem(const SSysInitParams& startupParams);
	bool InitDynamicResponseSystem(const SSysInitParams& startupParams);
	bool InitLiveCreate(const SSysInitParams& startupParams);
	bool InitMonoBridge(const SSysInitParams& startupParams);
	void InitGameFramework(SSysInitParams& startupParams);
	bool OpenRenderLibrary(const SSysInitParams& startupParams, i32 type);
	bool OpenRenderLibrary(const SSysInitParams& startupParams, tukk t_rend);
	bool CloseRenderLibrary(tukk t_rend);

	//@}

	//! @name Unload routines
	//@{
	void UnloadSchematycModule();
	//@}

	void Strange();
	bool ParseSystemConfig(string& sFileName);

	//////////////////////////////////////////////////////////////////////////
	// Threading functions.
	//////////////////////////////////////////////////////////////////////////
	void InitThreadSystem();
	void ShutDownThreadSystem();

	//////////////////////////////////////////////////////////////////////////
	// Helper functions.
	//////////////////////////////////////////////////////////////////////////
	void        CreateRendererVars(const SSysInitParams& startupParams);
	void        CreateSystemVars();
	void        CreateAudioVars();
	void        RenderStats();
	void        RenderOverscanBorders();
	void        RenderJobStats();
	void        RenderMemStats();
	void        RenderThreadInfo();
	void        FreeLib(WIN_HMODULE hLibModule);
	void        QueryVersionInfo();
	void        LogVersion();
	void        SetDevMode(bool bEnable);
	void        InitScriptDebugger();

	void        CreatePhysicsThread();
	void        KillPhysicsThread();

	static void SystemVersionChanged(ICVar* pCVar);

	bool        ReLaunchMediaCenter();
	void        LogSystemInfo();
	void        UpdateAudioSystems();
	void        PrePhysicsUpdate();

	// recursive
	// Arguments:
	//   sPath - e.g. "Game/Config/CVarGroups"
	void AddCVarGroupDirectory(const string& sPath);

#if DRX_PLATFORM_WINDOWS
	bool GetWinGameFolder(tuk szMyDocumentsPath, i32 maxPathSize);
#endif
public:
	// interface ISystem -------------------------------------------
	virtual void SetForceNonDevMode(const bool bValue) override;
	virtual bool GetForceNonDevMode() const override;
	virtual bool WasInDevMode() const override { return m_bWasInDevMode; }
	virtual bool IsDevMode() const override    { return m_bInDevMode && !GetForceNonDevMode(); }
	virtual bool IsMODValid(tukk szMODName) const override
	{
		if (!szMODName || strstr(szMODName, ".") || strstr(szMODName, "\\") || stricmp(szMODName, PathUtil::GetGameFolder().c_str()) == 0)
			return (false);
		return (true);
	}
	virtual void AutoDetectSpec(const bool detectResolution) override;

	virtual void AsyncMemcpy(uk dst, ukk src, size_t size, i32 nFlags,  i32* sync) override
	{
#if !defined(DRX_ASYNC_MEMCPY_DELEGATE_TO_DRXSYSTEM)
		drxAsyncMemcpy(dst, src, size, nFlags, sync);
#else
		_drxAsyncMemcpy(dst, src, size, nFlags, sync);
#endif
	}

#if DRX_PLATFORM_DURANGO
	virtual void OnPLMEvent(EPLM_Event event);
#endif

	// -------------------------------------------------------------

	//! attaches the given variable to the given container;
	//! recreates the variable if necessary
	ICVar*            attachVariable(tukk szVarName, i32* pContainer, tukk szComment, i32 dwFlags = 0);

	CCpuFeatures*     GetCPUFeatures()                { return m_pCpu; };

	string&           GetDelayedScreeenshot()         { return m_sDelayedScreeenshot; }

	CVisRegTest*&     GetVisRegTestPtrRef()           { return m_pVisRegTest; }

	const CTimeValue& GetLastTickTime(void) const     { return m_lastTickTime; }
	const ICVar*      GetDedicatedMaxRate(void) const { return m_svDedicatedMaxRate; }

//protected://ivate: // ------------------------------------------------------

	// System environment.
#if defined(SYS_ENV_AS_STRUCT)
	//since gEnv is a global var, this should just be a reference for code consistency
	SSysGlobEnv & m_env;
#else
	SSysGlobEnv m_env;
#endif

	CTimer             m_Time;                  //!<
	CCamera            m_ViewCamera;            //!<
	 bool      m_bQuit;                 //!< if is true the system is quitting.  as it may be polled from multiple threads.
	bool               m_bShaderCacheGenMode;   //!< true if the application runs in shader cache generation mode
	bool               m_bRelaunch;             //!< relaunching the app or not (true beforerelaunch)
	i32                m_iLoadingMode;          //!< Game is loading w/o changing context (0 not, 1 quickloading, 2 full loading)
	bool               m_bEditor;               //!< If running in Editor.
	bool               m_bNoCrashDialog;
	bool               m_bPreviewMode;          //!< If running in Preview mode.
	bool               m_bUIFrameworkMode;
	bool               m_bIgnoreUpdates;        //!< When set to true will ignore Update and Render calls,
	IValidator*        m_pValidator;            //!< Pointer to validator interface.
	bool               m_bForceNonDevMode;      //!< true when running on a cheat protected server or a client that is connected to it (not used in singlplayer)
	bool               m_bWasInDevMode;         //!< Set to true if was in dev mode.
	bool               m_bInDevMode;            //!< Set to true if was in dev mode.
	bool               m_bGameFolderWritable;   //!< True when verified that current game folder have write access.
	SDefaultValidator* m_pDefaultValidator;     //!<
	i32                m_nStrangeRatio;         //!<
	string             m_sDelayedScreeenshot;   //!< to delay a screenshot call for a frame
	CCpuFeatures*      m_pCpu;                  //!< CPU features
	i32                m_ttMemStatSS;           //!< Time to memstat screenshot
	string             m_szCmdLine;

	i32                m_iTraceAllocations;

#ifndef _RELEASE
	i32             m_checkpointLoadCount;      // Total times game has loaded from a checkpoint
	LevelLoadOrigin m_loadOrigin;               // Where the load was initiated from
	bool            m_hasJustResumed;           // Has resume game just been called
	bool            m_expectingMapCommand;
#endif

	IHmdUpr* m_pHmdUpr;

	//! DLLs handles.
	struct SDllHandles
	{
		WIN_HMODULE hRenderer;
		WIN_HMODULE hInput;
		WIN_HMODULE hFlash;
		WIN_HMODULE hSound;
		WIN_HMODULE hEntitySystem;
		WIN_HMODULE hNetwork;
		WIN_HMODULE hAI;
		WIN_HMODULE hMovie;
		WIN_HMODULE hPhysics;
		WIN_HMODULE hFont;
		WIN_HMODULE hScript;
		WIN_HMODULE h3DEngine;
		WIN_HMODULE hAnimation;
		WIN_HMODULE hIndoor;
		WIN_HMODULE hGame;
		WIN_HMODULE hLiveCreate;
		WIN_HMODULE hInterface;
	};
	SDllHandles                        m_dll;

	std::unordered_map<string, WIN_HMODULE, stl::hash_strcmp<string>> m_moduleDLLHandles;

	//! THe streaming engine
	CStreamEngine* m_pStreamEngine;

	//! current active process
	IProcess*       m_pProcess;

	IMemoryUpr* m_pMemoryUpr;

	CPhysRenderer*  m_pPhysRenderer;
	CCamera         m_PhysRendererCamera;
	ICVar*          m_p_draw_helpers_str;
	i32             m_iJumpToPhysProfileEnt;

	CTimeValue      m_lastTickTime;

	//! system event dispatcher
	ISystemEventDispatcher* m_pSystemEventDispatcher;

	//! The default font
	IFFont* m_pIFont;

	//! System to monitor given budget.
	IBudgetingSystem* m_pIBudgetingSystem;

	//! System to access zlib compressor
	IZLibCompressor* m_pIZLibCompressor;

	//! System to access zlib decompressor
	IZLibDecompressor* m_pIZLibDecompressor;

	//! System to access lz4 hc decompressor
	ILZ4Decompressor*   m_pILZ4Decompressor;

	CNULLRenderAuxGeom* m_pNULLRenderAuxGeom;

	// XML Utils interface.
	CXmlUtils*                   m_pXMLUtils;

	Serialization::IArchiveHost* m_pArchiveHost;

	//! global root folder
	string m_root;
	i32    m_iApplicationInstance;

	//! to hold the values stored in system.cfg
	//! because editor uses it's own values,
	//! and then saves them to file, overwriting the user's resolution.
	i32 m_iHeight = 0;
	i32 m_iWidth = 0;
	i32 m_iColorBits = 0;

	// System console variables.
	//////////////////////////////////////////////////////////////////////////

	// DLL names
	ICVar* m_sys_dll_ai;
	ICVar* m_sys_dll_response_system;
	ICVar* m_sys_user_folder;

#if !defined(_RELEASE)
	ICVar* m_sys_resource_cache_folder;
#endif

	ICVar* m_sys_initpreloadpacks;
	ICVar* m_sys_menupreloadpacks;

	ICVar* m_cvAIUpdate;
	ICVar* m_rWidth;
	ICVar* m_rHeight;
	ICVar* m_rColorBits;
	ICVar* m_rDepthBits;
	ICVar* m_rStencilBits;
	ICVar* m_rFullscreen;
	ICVar* m_rFullsceenNativeRes;
	ICVar* m_rWindowState;
	ICVar* m_rDriver;
	ICVar* m_pPhysicsLibrary;
	ICVar* m_rDisplayInfo;
	ICVar* m_rDisplayInfoTargetFPS;
	ICVar* m_rOverscanBordersDrawDebugView;
	ICVar* m_sysNoUpdate;
	ICVar* m_cvEntitySuppressionLevel;
	ICVar* m_pCVarQuit;
	ICVar* m_cvMemStats;
	ICVar* m_cvMemStatsThreshold;
	ICVar* m_cvMemStatsMaxDepth;
	ICVar* m_sysKeyboard;
	ICVar* m_sysWarnings;                   //!< might be 0, "sys_warnings" - Treat warning as errors.
	ICVar* m_cvSSInfo;                      //!< might be 0, "sys_SSInfo" 0/1 - get file sourcesafe info
	ICVar* m_svDedicatedMaxRate;
	ICVar* m_svAISystem;
	ICVar* m_clAISystem;
	ICVar* m_sys_profile_watchdog_timeout;
	ICVar* m_sys_job_system_filter;
	ICVar* m_sys_job_system_enable;
	ICVar* m_sys_job_system_profiler;
	ICVar* m_sys_job_system_max_worker;
	ICVar* m_sys_spec;
	ICVar* m_sys_firstlaunch;

	ICVar* m_sys_physics_enable_MT;
	ICVar* m_sys_audio_disable;

	ICVar* m_sys_SimulateTask;

	ICVar* m_sys_min_step;
	ICVar* m_sys_max_step;
	ICVar* m_sys_enable_budgetmonitoring;
	ICVar* m_sys_memory_debug;
	ICVar* m_sys_preload;
	ICVar* m_sys_use_Mono;

	//	ICVar *m_sys_filecache;
	ICVar* m_gpu_particle_physics;

	ICVar* m_sys_vr_support;  // depends on the m_pHmdUpr

	string m_sSavedRDriver;                 //!< to restore the driver when quitting the dedicated server

	//////////////////////////////////////////////////////////////////////////
	//! User define callback for system events.
	ISystemUserCallback* m_pUserCallback;

#if defined(CVARS_WHITELIST)
	//////////////////////////////////////////////////////////////////////////
	//! User define callback for whitelisting cvars
	ICVarsWhitelist*             m_pCVarsWhitelist;
	ILoadConfigurationEntrySink* m_pCVarsWhitelistConfigSink;
#endif // defined(CVARS_WHITELIST)

	WIN_HWND m_hWnd = nullptr;

	// this is the memory statistics that is retained in memory between frames
	// in which it's not gathered
	DrxSizerStats*               m_pMemStats;
	DrxSizerImpl*                m_pSizer;

	CFrameProfileSystem          m_FrameProfileSystem;

	CThreadProfiler*             m_pThreadProfiler;
	IDiskProfiler*               m_pDiskProfiler;

	std::unique_ptr<IPlatformOS> m_pPlatformOS;
	IDrxPerfHUD*                 m_pPerfHUD;
	minigui::IMiniGUI*           m_pMiniGUI;

	//i32 m_nCurrentLogVerbosity;

	SFileVersion              m_fileVersion;
	SFileVersion              m_productVersion;
	SFileVersion              m_buildVersion;

	CLocalizedStringsUpr* m_pLocalizationUpr;

	// Name table.
	CNameTable                       m_nameTable;

	IPhysicsThreadTask*              m_PhysThread;

	ESystemConfigSpec                m_nServerConfigSpec;
	ESystemConfigSpec                m_nMaxConfigSpec;

	std::unique_ptr<CServerThrottle> m_pServerThrottle;

	CProfilingSystem                 m_ProfilingSystem;
	sUpdateTimes                     m_UpdateTimes[NUM_UPDATE_TIMES];
	u32                           m_UpdateTimesIdx;

	// Pause mode.
	bool   m_bPaused;
	u8  m_PlatformOSCreateFlags;
	bool   m_bNoUpdate;

	uint64 m_nUpdateCounter;

	i32    sys_ProfileLevelLoading, sys_ProfileLevelLoadingDump;

	// MT random generator
	DrxCriticalSection m_mtLock;
	CMTRand_int32*     m_pMtState;

	// The random generator used by the engine and all its modules
	CRndGen m_randomGenerator;

public:
	//! Pointer to the download manager
	CDownloadUpr* m_pDownloadUpr;

#ifdef USE_FRAME_PROFILER
	void SetFrameProfiler(bool on, bool display, tuk prefix) override { m_FrameProfileSystem.SetProfiling(on, display, prefix, this); }
#else
	void SetFrameProfiler(bool on, bool display, tuk prefix) override {}
#endif

	//////////////////////////////////////////////////////////////////////////
	// File version.
	//////////////////////////////////////////////////////////////////////////
	virtual const SFileVersion& GetFileVersion() override;
	virtual const SFileVersion& GetProductVersion() override;
	virtual const SFileVersion& GetBuildVersion() override;

	bool                        CompressDataBlock(ukk input, size_t inputSize, uk output, size_t& outputSize, i32 level) override;
	bool                        DecompressDataBlock(ukk input, size_t inputSize, uk output, size_t& outputSize) override;

	void                        OpenBasicPaks(bool bLoadGamePaks);
	void                        OpenLanguagePak(char const* const szLanguage);
	void                        OpenLanguageAudioPak(char const* const szLanguage);
	void                        GetLocalizedPath(char const* const szLanguage, string& szLocalizedPath);
	void                        GetLocalizedAudioPath(char const* const szLanguage, string& szLocalizedPath);
	void                        CloseLanguagePak(char const* const szLanguage);
	void                        CloseLanguageAudioPak(char const* const szLanguage);

	void                        Deltree(tukk szFolder, bool bRecurse);
	void                        UpdateMovieSystem(i32k updateFlags, const float fFrameTime, const bool bPreUpdate);

	// level loading profiling
	virtual void                          OutputLoadingTimeStats() override;
	virtual struct SLoadingTimeContainer* StartLoadingSectionProfiling(CLoadingTimeProfiler* pProfiler, tukk szFuncName) override;
	virtual void                          EndLoadingSectionProfiling(CLoadingTimeProfiler* pProfiler) override;
	virtual tukk                   GetLoadingProfilerCallstack() override;

	//////////////////////////////////////////////////////////////////////////
	virtual CBootProfilerRecord* StartBootSectionProfiler(tukk name, tukk args,EProfileDescription type) override;
	virtual void                 StopBootSectionProfiler(CBootProfilerRecord* record) override;
	virtual void                 StartBootProfilerSession(tukk szName) override;
	virtual void                 StopBootProfilerSession(tukk szName) override;
	virtual void                 OnFrameStart(tukk szName) override;
	virtual void                 OnFrameEnd() override;
	//////////////////////////////////////////////////////////////////////////
	// DrxAssert and error related.
	virtual bool RegisterErrorObserver(IErrorObserver* errorObserver) override;
	bool         UnregisterErrorObserver(IErrorObserver* errorObserver) override;

	void         OnFatalError(tukk message);

#if defined(USE_DRX_ASSERT)
	virtual void OnAssert(tukk condition, tukk message, tukk fileName, u32 fileLineNumber) override;

	virtual bool IsAssertDialogVisible() const override;
	virtual bool AreAssertsEnabledForModule(u32 moduleId) override;
	virtual void DisableAssertionsForModule(u32 moduleId) override;

	virtual void         SetAssertVisible(bool bAssertVisble) override;
#endif

	virtual void ClearErrorMessages() override
	{
		m_ErrorMessages.clear();
	}

	virtual void AddPlatformOSCreateFlag(u8k createFlag) override { m_PlatformOSCreateFlags |= createFlag; }

	virtual bool IsLoading() override
	{
		return (m_systemGlobalState < ESYSTEM_GLOBAL_STATE_LEVEL_LOAD_END);
	}

	virtual ESystemGlobalState GetSystemGlobalState(void) override;
	virtual void               SetSystemGlobalState(const ESystemGlobalState systemGlobalState) override;

#if !defined(_RELEASE)
	virtual bool IsSavingResourceList() const override { return (g_cvars.pakVars.nSaveLevelResourceList != 0); }
#endif

private:
	std::vector<IErrorObserver*> m_errorObservers;
	ESystemGlobalState           m_systemGlobalState = ESYSTEM_GLOBAL_STATE_INIT;
	static tukk GetSystemGlobalStateName(const ESystemGlobalState systemGlobalState);

public:
	void InitLocalization();
	void UpdateUpdateTimes();

protected: // -------------------------------------------------------------
	ILoadingProgressListener*                 m_pProgressListener;
	CCmdLine*                                 m_pCmdLine;
	std::unique_ptr<ITestSystem>              m_pTestSystem; // needed for external test application (0 if not activated yet)
	CVisRegTest*                              m_pVisRegTest;
	CThreadUpr*                           m_pThreadUpr;
	CResourceUpr*                         m_pResourceUpr;
	ITextModeConsole*                         m_pTextModeConsole;
	INotificationNetwork*                     m_pNotificationNetwork;
	CDrxPluginUpr*                        m_pPluginUpr;
	CUserAnalyticsSystem*                     m_pUserAnalyticsSystem;
	CProjectUpr*                          m_pProjectUpr;
	CManualFrameStepController*               m_pManualFrameStepController = nullptr;

	bool                                      m_hasWindowFocus = true;

	string                                    m_binariesDir;
	string                                    m_currentLanguageAudio;

	std::vector<std::pair<CTimeValue, float>> m_updateTimes;

	CMemoryFragmentationProfiler              m_MemoryFragmentationProfiler;

#if !defined(DRX_IS_MONOLITHIC_BUILD)
	CDrxLibrary m_gameLibrary;
#endif

	struct SErrorMessage
	{
		string m_Message;
		float  m_fTimeToShow;
		float  m_Color[4];
		bool   m_HardFailure;
	};
	typedef std::list<SErrorMessage> TErrorMessages;
	TErrorMessages m_ErrorMessages;
	bool           m_bHasRenderedErrorMessage;
	bool           m_bNeedDoWorkDuringOcclusionChecks;


	std::unordered_map<u32, bool> m_mapWarningOnceAlreadyPrinted;
	DrxMutex						 m_mapWarningOnceMutex;

#if defined(USE_DRX_ASSERT)
	bool m_isAsserting = false;
	// Used to check if DrxAssert is enabled for a specific module
	std::bitset<eDrxM_Num> m_disabledAssertModules;
#endif

	friend struct SDefaultValidator;
	friend struct SDinrusXFoldersLoader;
	//	friend void ScreenshotCmd( IConsoleCmdArgs *pParams );

	public: std::vector<IWindowMessageHandler*> m_windowMessageHandlers;
	IImeUpr*                        m_pImeUpr;

	class CWatchdogThread*  m_pWatchdog = nullptr;
	static void WatchDogTimeOutChanged(ICVar* cvar);
};

/*extern static */ bool QueryModuleMemoryInfo(SDinrusXStatsModuleInfo& moduleInfo, i32 index);
