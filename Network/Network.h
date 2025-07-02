// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  implementation of INetwork
   -------------------------------------------------------------------------
   История:
   - 26/07/2004   : Created by Craig Tiller
   !F RCON system - lluo
*************************************************************************/

#ifndef __NETWORK_H__
#define __NETWORK_H__

#pragma once

#define _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR 1

#include <vector>
#include <memory>
#include <drx3D/Network/Config.h>
#include <drx3D/Sys/TimeValue.h>
#include <drx3D/Network/StatsCollector.h>
#include <drx3D/Network/FrameTypes.h>
#include <drx3D/Network/Errors.h>
#include <drx3D/Network/NetResolver.h>
#include <drx3D/Network/INetworkPrivate.h>
#include <drx3D/Network/INetworkMember.h>
#include <drx3D/Network/NetHelpers.h>
#include <drx3D/Network/WorkQueue.h>
#include <drx3D/Network/NetTimer.h>
#include <drx3D/Network/MementoMemoryUpr.h>
#include <drx3D/Network/NetCVars.h>
#include <drx3D/Network/NetLog.h>
#include <drx3D/CoreX/Memory/STLGlobalAllocator.h>
#if NEW_BANDWIDTH_MANAGEMENT
	#include <drx3D/Network/NewMessageQueue.h>
#else
	#include <drx3D/Network/MessageQueue.h>
#endif // NEW_BANDWIDTH_MANAGEMENT
#include <drx3D/Network/IStreamSocket.h>
#include <drx3D/Network/IDatagramSocket.h>
#include <drx3D/Network/ISocketIOUpr.h>

#include <drx3D/Network/Whirlpool.h>
#include <drx3D/Network/ExponentialKeyExchange.h>
#include <drx3D/Network/CompressionUpr.h>

#include <drx3D/Network/WatchdogTimer.h>

struct ICVar;
class CNetNub;
class CNetContext;
struct INetworkMember;
struct IConsoleCmdArgs;
class CNetworkInspector;
class CServiceUpr;
#if ENABLE_DISTRIBUTED_LOGGER
class CDistributedLogger;
#endif

#if defined(ENABLE_LW_PROFILERS)
	#define NET_THREAD_TIMING
#endif

#ifdef NET_THREAD_TIMING
	#define NET_START_THREAD_TIMER() CNetwork::Get()->ThreadTimerStart()
	#define NET_STOP_THREAD_TIMER()  CNetwork::Get()->ThreadTimerStop()
	#define NET_TICK_THREAD_TIMER()  CNetwork::Get()->ThreadTimerTick()
#else
	#define NET_START_THREAD_TIMER()
	#define NET_STOP_THREAD_TIMER()
	#define NET_TICK_THREAD_TIMER()
#endif

struct PerformanceGuard
{
	SNetworkPerformance* m_pCounter;
	uint64               m_nOldVal;

	PerformanceGuard(SNetworkPerformance* p, uint64 oldVal)
	{
		m_pCounter = p;
		m_nOldVal = oldVal;
		p->m_nNetworkSync = DrxGetTicks();
	}

	~PerformanceGuard()
	{
		m_pCounter->m_nNetworkSync = DrxGetTicks() - m_pCounter->m_nNetworkSync + m_nOldVal;
	}

};

struct SSysBranchCounters
{
	SSysBranchCounters()
	{
		memset(this, 0, sizeof(*this));
	}
	u32 updateTickMain;
	u32 updateTickSkip;
	u32 iocpReapImmediate;
	u32 iocpReapImmediateCollect;
	u32 iocpReapSleep;
	u32 iocpReapSleepCollect;
	u32 iocpIdle;
	u32 iocpForceSync;
	u32 iocpBackoffCheck;
};

extern SSysBranchCounters g_systemBranchCounters;

enum EMementoMemoryType
{
	eMMT_Memento = 0,
	eMMT_PacketData,
	eMMT_ObjectData,
	eMMT_Alphabet,
	eMMT_NUM_TYPES
};

template<class T>
class NetMutexLock
{
public:
	NetMutexLock(T& mtx, bool lock) : m_mtx(mtx), m_lock(lock) { if (m_lock) m_mtx.Lock(); }
	~NetMutexLock() { if (m_lock) m_mtx.Unlock(); }

private:
	T&   m_mtx;
	bool m_lock;
};

typedef DrxLockT<DRXLOCK_RECURSIVE> NetFastMutex;

enum EDebugMemoryMode
{
	eDMM_Context  = 1,
	eDMM_Endpoint = 2,
	eDMM_Mementos = 4,
};

enum EDebugBandwidthMode
{
	eDBM_None          = 0,
	eDBM_AspectAgeList = 1,
	eDBM_AspectAgeMap  = 2,
	eDBM_PriorityMap   = 3,
};

tukk GetTimestampString();

extern CTimeValue g_time;

#if defined(_RELEASE)
	#define ASSERT_MUTEX_LOCKED(mtx)
	#define ASSERT_MUTEX_UNLOCKED(mtx)
#else
	#define ASSERT_MUTEX_LOCKED(mtx)   if (CNetwork::Get()->IsMultithreaded()) \
	  NET_ASSERT((mtx).IsLocked())
	#define ASSERT_MUTEX_UNLOCKED(mtx) if (CNetwork::Get()->IsMultithreaded()) \
	  NET_ASSERT(!(mtx).IsLocked())
#endif

#define ASSERT_GLOBAL_LOCK ASSERT_MUTEX_LOCKED(CNetwork::Get()->GetMutex())
#define ASSERT_COMM_LOCK   ASSERT_MUTEX_LOCKED(CNetwork::Get()->GetCommMutex())

class CNetwork : public INetworkPrivate
{
public:
	//! constructor
	CNetwork();
protected:
	//! destructor
	virtual ~CNetwork();

public:
	bool Init(i32 ncpu);

	// interface INetwork -------------------------------------------------------------

	virtual bool     HasNetworkConnectivity();
	virtual INetNub* CreateNub(tukk address,
	                           IGameNub* pGameNub,
	                           IGameSecurity* pSecurity,
	                           IGameQuery* pGameQuery);
	virtual ILanQueryListener*    CreateLanQueryListener(IGameQueryListener* pGameQueryListener);
	virtual INetContext*          CreateNetContext(IGameContext* pGameContext, u32 flags);
	virtual tukk           EnumerateError(NRESULT err);
	virtual void                  Release();
	virtual void                  GetMemoryStatistics(IDrxSizer* pSizer);
	virtual void                  GetPerformanceStatistics(SNetworkPerformance* pSizer);
	virtual void SyncWithGame(ENetworkGameSync);
	virtual tukk           GetHostName();
	virtual INetworkServicePtr    GetService(tukk name);
	virtual void                  FastShutdown();
	virtual void                  SetCDKey(tukk key);
	virtual bool                  PbConsoleCommand(tukk , i32 length);           // EvenBalance - M. Quinn
	virtual void                  PbCaptureConsoleLog(tukk output, i32 length); // EvenBalance - M. Quinn
	virtual void                  PbServerAutoComplete(tukk , i32 length);       // EvenBalance - M. Quinn
	virtual void                  PbClientAutoComplete(tukk , i32 length);       // EvenBalance - M. Quinn
	virtual void SetNetGameInfo(SNetGameInfo);
	virtual SNetGameInfo          GetNetGameInfo();
	virtual IDrxLobby*            GetLobby();

	virtual bool                  IsPbClEnabled();
	virtual bool                  IsPbSvEnabled();

	virtual void                  StartupPunkBuster(bool server);
	virtual void                  CleanupPunkBuster();

	virtual bool                  IsPbInstalled();

	virtual IRemoteControlSystem* GetRemoteControlSystemSingleton();

	virtual ISimpleHttpServer*    GetSimpleHttpServerSingleton();

	// nub helpers
	i32                           GetLogLevel();
	virtual CNetAddressResolver*  GetResolver()                        { return m_pResolver; }
	ILINE CWorkQueue&             GetToGameQueue()                     { return m_toGame; }
	ILINE CWorkQueue&             GetToGameLazyQueue()                 { return m_toGameLazyBuilding; }
	ILINE CWorkQueue&             GetFromGameQueue()                   { return m_fromGame; }
	ILINE CWorkQueue&             GetInternalQueue()                   { return m_intQueue; }
	ILINE CNetTimer&              GetTimer()                           { return m_timer; }
	ILINE CAccurateNetTimer&      GetAccurateTimer()                   { return m_accurateTimer; }
	virtual const CNetCVars&      GetCVars()                           { return m_cvars; }
	ILINE CMessageQueue::CConfig* GetMessageQueueConfig() const        { return m_pMessageQueueConfig; }
	ILINE CCompressionUpr&    GetCompressionUpr()              { return m_compressionUpr; }
	virtual ISocketIOUpr& GetExternalSocketIOUpr()         { return *m_pExternalSocketIOUpr; }
	virtual ISocketIOUpr& GetInternalSocketIOUpr()         { return *m_pInternalSocketIOUpr; }
	i32                       GetMessageQueueConfigVersion() const { return m_schedulerVersion; }

	CTimeValue                GetGameTime()                        { return m_gameTime; }

	void                      ReportGotPacket()                    { m_detection.ReportGotPacket(); }

	ILINE CStatsCollector*    GetStats()
	{
		static CStatsCollector m_stats("netstats.log");
		return &m_stats;
	}

#if ENABLE_DEBUG_KIT
	ILINE CNetworkInspector* GetNetInspector()
	{
		return m_pNetInspector;
	}
#endif

	void                   WakeThread();

	static ILINE CNetwork* Get()                   { return m_pThis; }
	ILINE NetFastMutex&    GetMutex()              { return m_mutex; }
	ILINE NetFastMutex&    GetCommMutex()          { return m_commMutex; }
	ILINE NetFastMutex&    GetLogMutex()           { return m_logMutex; }

	ILINE bool             IsMultithreaded() const { return (m_multithreadedMode != NETWORK_MT_OFF); }
	void                   SetMultithreadingMode(ENetwork_Multithreading_Mode threadingMode);

	CNetChannel*           FindFirstClientChannel();
	CNetChannel*           FindFirstRemoteChannel();

	string                 GetCDKey() const
	{
		return m_CDKey;
	}

	CServiceUpr* GetServiceUpr() { return m_pServiceUpr.get(); }

	// fast lookup of channels (mainly for punkbuster)
	i32          RegisterForFastLookup(CNetChannel* pChannel);
	void         UnregisterFastLookup(i32 id);
	CNetChannel* GetChannelByFastLookupId(i32 id);

	void         AddExecuteString(const string& toExec);

	virtual void BroadcastNetDump(ENetDumpType);

	static void LobbyTimerCallback(NetTimerId id, uk pUserData, CTimeValue time);

	void        LOCK_ON_STALL_TICKER()
	{
		if (IsMinimalUpdate())
		{
			m_mutex.Lock();
		}
	}

	void UNLOCK_ON_STALL_TICKER()
	{
		if (IsMinimalUpdate())
		{
			m_mutex.Unlock();
		}
	}

	// small hack to make adding things to the from game queue in general efficient
#define ADDTOFROMGAMEQUEUE_BODY(params) if (IsPrimaryThread()) { LOCK_ON_STALL_TICKER(); m_fromGame.Add params; UNLOCK_ON_STALL_TICKER(); } else { DrxAutoLock<NetFastMutex> lk(m_fromGame_otherThreadLock); m_fromGame_otherThreadQueue.Add params; }
	template<class A> void                                                       AddToFromGameQueue(const A& a)
	{ ADDTOFROMGAMEQUEUE_BODY((a)); }
	template<class A, class B> void                                              AddToFromGameQueue(const A& a, const B& b)
	{ ADDTOFROMGAMEQUEUE_BODY((a, b)); }
	template<class A, class B, class C> void                                     AddToFromGameQueue(const A& a, const B& b, const C& c)
	{ ADDTOFROMGAMEQUEUE_BODY((a, b, c)); }
	template<class A, class B, class C, class D> void                            AddToFromGameQueue(const A& a, const B& b, const C& c, const D& d)
	{ ADDTOFROMGAMEQUEUE_BODY((a, b, c, d)); }
	template<class A, class B, class C, class D, class E> void                   AddToFromGameQueue(const A& a, const B& b, const C& c, const D& d, const E& e)
	{ ADDTOFROMGAMEQUEUE_BODY((a, b, c, d, e)); }
	template<class A, class B, class C, class D, class E, class F> void          AddToFromGameQueue(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f)
	{ ADDTOFROMGAMEQUEUE_BODY((a, b, c, d, e, f)); }
	template<class A, class B, class C, class D, class E, class F, class G> void AddToFromGameQueue(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g)
	{ ADDTOFROMGAMEQUEUE_BODY((a, b, c, d, e, f, g)); }

#define ADDTOTOGAMEQUEUE_BODY(params) LOCK_ON_STALL_TICKER(); ASSERT_GLOBAL_LOCK; CNetwork::Get()->GetToGameQueue().Add params; UNLOCK_ON_STALL_TICKER();
	template<class A> void                                                       AddToToGameQueue(void (A::* ptr)(void), A* a)
	{ ADDTOTOGAMEQUEUE_BODY((ptr, a)); }
	template<class A> void                                                       AddToToGameQueue(const A& a)
	{ ADDTOTOGAMEQUEUE_BODY((a)); }
	template<class A, class B> void                                              AddToToGameQueue(const A& a, const B& b)
	{ ADDTOTOGAMEQUEUE_BODY((a, b)); }
	template<class A, class B, class C> void                                     AddToToGameQueue(const A& a, const B& b, const C& c)
	{ ADDTOTOGAMEQUEUE_BODY((a, b, c)); }
	template<class A, class B, class C, class D> void                            AddToToGameQueue(const A& a, const B& b, const C& c, const D& d)
	{ ADDTOTOGAMEQUEUE_BODY((a, b, c, d)); }
	template<class A, class B, class C, class D, class E> void                   AddToToGameQueue(const A& a, const B& b, const C& c, const D& d, const E& e)
	{ ADDTOTOGAMEQUEUE_BODY((a, b, c, d, e)); }
	template<class A, class B, class C, class D, class E, class F> void          AddToToGameQueue(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f)
	{ ADDTOTOGAMEQUEUE_BODY((a, b, c, d, e, f)); }
	template<class A, class B, class C, class D, class E, class F, class G> void AddToToGameQueue(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g)
	{ ADDTOTOGAMEQUEUE_BODY((a, b, c, d, e, f, g)); }

	virtual void                   NpCountReadBits(bool count);
	virtual bool                   NpGetChildFromCurrent(tukk name, SNetProfileStackEntry** entry, bool rmi);
	virtual void                   NpRegisterBeginCall(tukk name, SNetProfileStackEntry** entry, float budget, bool rmi);
	virtual void                   NpBeginFunction(SNetProfileStackEntry* entry, bool read);
	virtual void                   NpEndFunction();
	virtual bool                   NpIsInitialised();
	virtual SNetProfileStackEntry* NpGetNullProfile();

	/////////////////////////////////////////////////////////////////////////////
	// Host Migration
public:
	virtual void EnableHostMigration(bool bEnabled);
	virtual bool IsHostMigrationEnabled(void);

	virtual void TerminateHostMigration(DrxSessionHandle gh);
	virtual void AddHostMigrationEventListener(IHostMigrationEventListener* pListener, tukk pWho, EListenerPriorityType priority);
	virtual void RemoveHostMigrationEventListener(IHostMigrationEventListener* pListener);
	/////////////////////////////////////////////////////////////////////////////

	// Exposed block encryption
	virtual void   EncryptBuffer(u8* pOutput, u8k* pInput, u32 bufferLength, u8k* pKey, u32 keyLength);
	virtual void   DecryptBuffer(u8* pOutput, u8k* pInput, u32 bufferLength, u8k* pKey, u32 keyLength);

	virtual void   EncryptBuffer(TCipher cipher, u8* pOutput, u8k* pInput, u32 bufferLength);
	virtual void   DecryptBuffer(TCipher cipher, u8* pOutput, u8k* pInput, u32 bufferLength);

	virtual u32 RijndaelEncryptBuffer(u8* pOutput, u8k* pInput, u32 bufferLength, u8k* pKey, u32 keyLength);
	virtual u32 RijndaelDecryptBuffer(u8* pOutput, u8k* pInput, u32 bufferLength, u8k* pKey, u32 keyLength);

	// Exposed streamed encryption
	virtual TCipher BeginCipher(u8k* pKey, u32 keyLength);
	virtual void    Encrypt(TCipher cipher, u8* pOutput, u8k* pInput, u32 bufferLength);
	virtual void    Decrypt(TCipher cipher, u8* pOutput, u8k* pInput, u32 bufferLength);
	virtual void    EndCipher(TCipher cipher);

#ifdef DEDICATED_SERVER
	// Dedicated server scheduler
	virtual IDatagramSocketPtr GetDedicatedServerSchedulerSocket();
#endif

	virtual void             GetBandwidthStatistics(SBandwidthStats* const pStats);
	virtual void             GetProfilingStatistics(SNetworkProfilingStats* const pProfilingStats);

	virtual bool             ConvertAddr(const TNetAddress& addrIn, DRXSOCKADDR_IN* pSockAddr);
	virtual bool             ConvertAddr(const TNetAddress& addrIn, DRXSOCKADDR* pSockAddr, i32* addrLen);

	bool                     IsMinimalUpdate() { return m_allowMinimalUpdate; }

	virtual TMemHdl          MemAlloc(size_t sz);
	virtual void             MemFree(TMemHdl h);
	virtual uk            MemGetPtr(TMemHdl h);

	virtual u8            FrameHeaderToID(u8 header) { return Frame_HeaderToID[header]; }
	virtual u8            FrameIDToHeader(u8 id)     { return Frame_IDToHeader[id]; }

	virtual SObjectCounters& GetObjectCounters()           { return g_objcnt; }

#ifdef NET_THREAD_TIMING
	void ThreadTimerStart();
	void ThreadTimerStop();
	void ThreadTimerTick();
#endif

private:
	static CNetwork*    m_pThis;

	bool                m_allowMinimalUpdate;
	bool                m_bOverideChannelTickToGoNow;

	SNetworkPerformance m_networkPerformance;

	// NOTE: NetCVars is the most dependent component in DinrusXNetwork, it should be constructed before any other DinrusXNetwork components and
	// destructed after all other DinrusXNetwork components get destroyed (the order of member contructions is the order of declarations of
	// the class, and the order of member destruction is the reverse order of member constructions)
	CNetCVars m_cvars;

	// m_vFastChannelLookup must be constructed before any of the CWorkQueue members
	std::vector<CNetChannel*, stl::STLGlobalAllocator<CNetChannel*>> m_vFastChannelLookup;

	void LogNetworkInfo();
	void DoSyncWithGame(ENetworkGameSync);
	void DoSyncWithGameMinimal();
	void AddMember(INetworkMemberPtr pMember);
	// will all of our members will be dead soon?
	bool AllSuicidal();
	void UpdateLoop(ENetworkGameSync sync);
#if ENABLE_DISTRIBUTED_LOGGER
	std::unique_ptr<CDistributedLogger> m_pLogger;
#endif

	typedef std::map<NRESULT, string>                                                  TErrorMap;
	typedef std::vector<INetworkMemberPtr, stl::STLGlobalAllocator<INetworkMemberPtr>> VMembers;

	struct SNetError
	{
		NRESULT     nrErrorCode;
		tukk sErrorDescription;
	};

	TErrorMap              m_mapErrors;
	static SNetError       m_neNetErrors[];
	VMembers               m_vMembers;

	bool                   m_bQuitting;

	CNetAddressResolver*   m_pResolver;
	CMementoMemoryUpr* m_pMMM;
#if ENABLE_DEBUG_KIT
	CNetworkInspector*     m_pNetInspector;
#endif
	// m_vFastChannelLookup must be constructed before any of the CWorkQueue's
	CWorkQueue                     m_toGame;
	CWorkQueue                     m_fromGame;
	NetFastMutex                   m_fromGame_otherThreadLock;
	CWorkQueue                     m_fromGame_otherThreadQueue;
	CWorkQueue                     m_intQueue;
	CWorkQueue                     m_toGameLazyBuilding;
	CWorkQueue                     m_toGameLazyProcessing;
	CNetTimer                      m_timer;
	CAccurateNetTimer              m_accurateTimer;
	CCompressionUpr            m_compressionUpr;
	CMessageQueue::CConfig*        m_pMessageQueueConfig;
	NetTimerId                     m_lobbyTimer;
	CTimeValue                     m_lobbyTimerInterval;
	i32                            m_schedulerVersion;
	i32                            m_inSync[eNGS_NUM_ITEMS + 2]; // one extra for the continuous update loop, and the other for flushing the lazy queue

	std::unique_ptr<CServiceUpr> m_pServiceUpr;

	class CNetworkThread;
	std::unique_ptr<CNetworkThread> m_pThread;
	NetFastMutex                  m_mutex;
	NetFastMutex                  m_commMutex;
	NetFastMutex                  m_logMutex;

	CTimeValue                    m_gameTime;

	string                        m_CDKey;

	class CBufferWakeups;
	i32                          m_bufferWakeups;
	bool                         m_wokenUp;

	ENetwork_Multithreading_Mode m_multithreadedMode;
	i32                          m_occasionalCounter;
	i32                          m_cleanupMember;
	CTimeValue                   m_nextCleanup;
	std::vector<string>          m_consoleCommandsToExec;
	ISocketIOUpr*            m_pExternalSocketIOUpr;
	ISocketIOUpr*            m_pInternalSocketIOUpr;
	i32                          m_cpuCount;

	CTimeValue                   m_lastUpdateLock;
	bool                         m_forceLock;
	bool                         m_bDelayedExternalWork;

	SNetGameInfo                 m_gameInfo;

	bool UpdateTick(bool mt);

	enum eTickReturnState
	{
		eTRS_Quit,
		eTRS_Continue,
		eTRS_TimedOut
	};

	eTickReturnState DoMainTick(bool mt);

	bool m_isPbSvActive;
	bool m_isPbClActive;

	class CNetworkConnectivityDetection
	{
	public:
		CNetworkConnectivityDetection() : m_hasNetworkConnectivity(true), m_lastCheck(0.0f), m_lastPacketReceived(0.0f) {}

		bool HasNetworkConnectivity();
		void ReportGotPacket() { m_lastPacketReceived = std::max(g_time, m_lastPacketReceived); }

		void AddRef()          {}
		void Release()         {}
		bool IsDead()          { return false; }

	private:
		bool       m_hasNetworkConnectivity;
		CTimeValue m_lastCheck;
		CTimeValue m_lastPacketReceived;

		void DetectNetworkConnectivity();
	};

	CNetworkConnectivityDetection m_detection; // NOTE: this relies on NetTimer and WorkQueue objects to function correctly, so it must be declared/defined after them

#ifdef DEDICATED_SERVER
	IDatagramSocketPtr m_pDedicatedServerSchedulerSocket;
#endif

#ifdef NET_THREAD_TIMING
	CTimeValue m_threadTimeCur;
	i32        m_threadTimeDepth;
	float      m_threadTime;
#endif
};

extern i32 g_lockCount;
#define SCOPED_GLOBAL_LOCK_NO_LOG                                                                         \
  ASSERT_MUTEX_UNLOCKED(CNetwork::Get()->GetCommMutex());                                                 \
  NetMutexLock<NetFastMutex> globalLock(CNetwork::Get()->GetMutex(), CNetwork::Get()->IsMultithreaded()); \
  g_lockCount++;                                                                                          \
  CAutoUpdateWatchdogCounters updateWatchdogTimers
#if LOG_LOCKING
	#define SCOPED_GLOBAL_LOCK SCOPED_GLOBAL_LOCK_NO_LOG; NetLog("lock %s(%d)", __FUNCTION__, __LINE__); ENSURE_REALTIME
#else
	#define SCOPED_GLOBAL_LOCK SCOPED_GLOBAL_LOCK_NO_LOG; ENSURE_REALTIME
#endif

// should be used in the NetLog functions only
#define SCOPED_GLOBAL_LOG_LOCK DrxAutoLock<NetFastMutex> globalLogLock(CNetwork::Get()->GetLogMutex())
#define SCOPED_COMM_LOCK       NetMutexLock<NetFastMutex> commMutexLock(CNetwork::Get()->GetCommMutex(), CNetwork::Get()->IsMultithreaded())

// never place these macros in a block without braces...
// BAD: if(x) TO_GAME(blah);
// GOOD: if(x) {TO_GAME(blah);}

// pass a message callback from network system to game, to execute during SyncToGame
// must hold the global lock
// global lock will be held in the callback
#define TO_GAME CNetwork::Get()->AddToToGameQueue
// pass a message callback from network system to game, to execute in parallel with the network thread
// global lock will *NOT* be held in the callback
#define TO_GAME_LAZY ASSERT_GLOBAL_LOCK; CNetwork::Get()->GetToGameLazyQueue().Add
// pass a message from the game to the network;
// not thread-safe!
// the game must guarantee that its calls to the network system are synchronized
// the network code must guarantee that it never calls FROM_GAME in one of its threads
// global lock will be held in the callback
#define FROM_GAME CNetwork::Get()->AddToFromGameQueue
// pass a message from the network engine to the network engine to be executed later
// must hold the global lock
// global lock will be held in the callback
#define NET_TO_NET         ASSERT_GLOBAL_LOCK; CNetwork::Get()->WakeThread(); CNetwork::Get()->GetInternalQueue().Add

#define RESOLVER           (*CNetwork::Get()->GetResolver())
#if USE_ACCURATE_NET_TIMERS
	#define TIMER            CNetwork::Get()->GetAccurateTimer()
#else
	#define TIMER            CNetwork::Get()->GetTimer()
#endif // USE_ACCURATE_NET_TIMERS
#define ACCURATE_NET_TIMER CNetwork::Get()->GetAccurateTimer()
#if TIMER_DEBUG
	#define ADDTIMER(_when, _callback, _userdata, _name) AddTimer(_when, _callback, _userdata, __FILE__, __LINE__, _name)
#else
	#define ADDTIMER(_when, _callback, _userdata, _name) AddTimer(_when, _callback, _userdata)
#endif // TIMER_DEBUG
#define CVARS         CNetwork::Get()->GetCVars()
#define NET_INSPECTOR (*CNetwork::Get()->GetNetInspector())
#define STATS         (*CNetwork::Get()->GetStats())

class CMementoMemoryRegion
{
public:
	ILINE CMementoMemoryRegion(CMementoMemoryUpr* pMMM)
	{
		ASSERT_GLOBAL_LOCK;
		m_pPrev = m_pMMM;
		m_pMMM = pMMM;
	}

	ILINE ~CMementoMemoryRegion()
	{
		ASSERT_GLOBAL_LOCK;
		m_pMMM = m_pPrev;
	}

	static ILINE CMementoMemoryUpr& Get()
	{
		assert(m_pMMM);
		return *m_pMMM;
	}

private:
	CMementoMemoryRegion(const CMementoMemoryRegion&);
	CMementoMemoryRegion& operator=(const CMementoMemoryRegion&);

	CMementoMemoryUpr*        m_pPrev;
	static CMementoMemoryUpr* m_pMMM;
};

#define MMM_REGION(pMMM) CMementoMemoryRegion _mmmrgn(pMMM)

ILINE CMementoMemoryUpr& MMM()
{
	return CMementoMemoryRegion::Get();
}

#endif
