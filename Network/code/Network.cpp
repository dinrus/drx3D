// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/Network.h>
#include  <drx3D/Network/Utils.h>

//#include  <drx3D/Network/platform.h>
#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	#include <drx3D/Network/INetwork.h>
#endif

#include <drx3D/Sys/IConsole.h>

#include  <drx3D/Network/NetworkInspector.h>

#include  <drx3D/Network/NetNub.h>
#include  <drx3D/Network/NetChannel.h>
#include  <drx3D/Network/Serialize.h>

#include  <drx3D/Network/ContextView.h>
#include  <drx3D/Network/NetContext.h>

#include  <drx3D/Network/CompressionUpr.h>

#include <drx3D/Network/ServiceUpr.h>
#include <drx3D/Network/LanQueryListener.h>

#include  <drx3D/Network/VoiceUpr.h>
#include  <drx3D/Network/IVoiceEncoder.h>
#include  <drx3D/Network/IVoiceDecoder.h>

#include  <drx3D/Network/DistributedLogger.h>
#include  <drx3D/Network/DebugKit.h>
#include  <drx3D/Network/NetDebugInfo.h>

#include  <drx3D/Network/RemoteControl.h>
#include  <drx3D/Network/SimpleHttpServer.h>

#include  <drx3D/Network/ServerProfiler.h>

#include <drx3D/Sys/ThreadProfilerSupport.h>
#include <drx3D/Sys/ITextModeConsole.h>
#include  <drx3D/Network/AutoConfigDownloader.h>

#include  <drx3D/Network/NetProfile.h>

#include  <drx3D/Network/StreamCipher.h>
#include  <drx3D/Network/rijndael.h>

#include  <drx3D/Network/PacketRateCalculator.h>
#if USE_GFWL
	#include <Winxnet.h>
#endif

#if ENABLE_OBJECT_COUNTING
	#include  <drx3D/Network/PsApi.h>
#endif

#include <drx3D/CoreX/Lobby/CommonIDrxMatchMaking.h>
#include <drx3D/CoreX/Thread/IThreadUpr.h>

#include <drx3D/Sys/FrameProfiler_JobSystem.h>

static i32k MIN_LOBBY_TICK_FREQUENCY = 4;

static i32k OCCASIONAL_TICKS = 50;

SSysBranchCounters g_systemBranchCounters;
SObjectCounters g_objcnt;
SObjectCounters* g_pObjcnt = &g_objcnt;
static SSysBranchCounters g_lastSystemBranchCounters;

static CAutoConfigDownloader* g_pFileDownloader;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CNetwork* CNetwork::m_pThis = 0;

i32 g_QuickLogY;
CTimeValue g_QuickLogTime;
CTimeValue g_time;
i32 g_lockCount = 0;

struct CNetwork::SNetError CNetwork::m_neNetErrors[] =
{
	{ NET_OK,                 "No Error"                                                   },
	{ NET_FAIL,               "Generic Error"                                              },
	// SOCKET
	{ NET_EINTR,              "WSAEINTR - interrupted function call"                       },
	{ NET_EBADF,              "WSAEBADF - Bad file number"                                 },
	{ NET_EACCES,             "WSAEACCES - error in accessing socket"                      },
	{ NET_EFAULT,             "WSAEFAULT - bad address"                                    },
	{ NET_EINVAL,             "WSAEINVAL - invalid argument"                               },
	{ NET_EMFILE,             "WSAEMFILE - too many open files"                            },
	{ NET_EWOULDBLOCK,        "WSAEWOULDBLOCK - resource temporarily unavailable"          },
	{ NET_EINPROGRESS,        "WSAEINPROGRESS - operation now in progress"                 },
	{ NET_EALREADY,           "WSAEALREADY - operation already in progress"                },
	{ NET_ENOTSOCK,           "WSAENOTSOCK - socket operation on non-socket"               },
	{ NET_EDESTADDRREQ,       "WSAEDESTADDRREQ - destination address required"             },
	{ NET_EMSGSIZE,           "WSAEMSGSIZE - message to long"                              },
	{ NET_EPROTOTYPE,         "WSAEPROTOTYPE - protocol wrong type for socket"             },
	{ NET_ENOPROTOOPT,        "WSAENOPROTOOPT - bad protocol option"                       },
	{ NET_EPROTONOSUPPORT,    "WSAEPROTONOSUPPORT - protocol not supported"                },
	{ NET_ESOCKTNOSUPPORT,    "WSAESOCKTNOSUPPORT - socket type not supported"             },
	{ NET_EOPNOTSUPP,         "WSAEOPNOTSUPP - operation not supported"                    },
	{ NET_EPFNOSUPPORT,       "WSAEPFNOSUPPORT - protocol family not supported"            },
	{ NET_EAFNOSUPPORT,       "WSAEAFNOSUPPORT - address family not supported by protocol" },
	{ NET_EADDRINUSE,         "WSAEADDRINUSE - address is in use"                          },
	{ NET_EADDRNOTAVAIL,      "WSAEADDRNOTAVAIL - address is not valid in context"         },
	{ NET_ENETDOWN,           "WSAENETDOWN - network is down"                              },
	{ NET_ENETUNREACH,        "WSAENETUNREACH - network is unreachable"                    },
	{ NET_ENETRESET,          "WSAENETRESET - network dropped connection on reset"         },
	{ NET_ECONNABORTED,       "WSACONNABORTED - software caused connection aborted"        },
	{ NET_ECONNRESET,         "WSAECONNRESET - connection reset by peer"                   },
	{ NET_ENOBUFS,            "WSAENOBUFS - no buffer space available"                     },
	{ NET_EISCONN,            "WSAEISCONN - socket is already connected"                   },
	{ NET_ENOTCONN,           "WSAENOTCONN - socket is not connected"                      },
	{ NET_ESHUTDOWN,          "WSAESHUTDOWN - cannot send after socket shutdown"           },
	{ NET_ETOOMANYREFS,       "WSAETOOMANYREFS - Too many references: cannot splice"       },
	{ NET_ETIMEDOUT,          "WSAETIMEDOUT - connection timed out"                        },
	{ NET_ECONNREFUSED,       "WSAECONNREFUSED - connection refused"                       },
	{ NET_ELOOP,              "WSAELOOP - Too many levels of symbolic links"               },
	{ NET_ENAMETOOLONG,       "WSAENAMETOOLONG - File name too long"                       },
	{ NET_EHOSTDOWN,          "WSAEHOSTDOWN - host is down"                                },
	{ NET_EHOSTUNREACH,       "WSAEHOSTUNREACH - no route to host"                         },
	{ NET_ENOTEMPTY,          "WSAENOTEMPTY - Cannot remove a directory that is not empty" },
	{ NET_EUSERS,             "WSAEUSERS - Ran out of quota"                               },
	{ NET_EDQUOT,             "WSAEDQUOT - Ran out of disk quota"                          },
	{ NET_ESTALE,             "WSAESTALE - File handle reference is no longer available"   },
	{ NET_EREMOTE,            "WSAEREMOTE - Item is not available locally"                 },

	// extended winsock errors(not BSD compliant)
#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO
	{ NET_EPROCLIM,           "WSAEPROCLIM - too many processes"                           },
	{ NET_SYSNOTREADY,        "WSASYSNOTREADY - network subsystem is unavailable"          },
	{ NET_VERNOTSUPPORTED,    "WSAVERNOTSUPPORTED - winsock.dll verison out of range"      },
	{ NET_NOTINITIALISED,     "WSANOTINITIALISED - WSAStartup not yet performed"           },
	{ NET_NO_DATA,            "WSANO_DATA - valid name, no data record of requested type"  },
	{ NET_EDISCON,            "WSAEDISCON - graceful shutdown in progress"                 },
#endif

	// extended winsock errors (corresponding to BSD h_errno)
	{ NET_HOST_NOT_FOUND,     "WSAHOST_NOT_FOUND - host not found"                         },
	{ NET_TRY_AGAIN,          "WSATRY_AGAIN - non-authoritative host not found"            },
	{ NET_NO_RECOVERY,        "WSANO_RECOVERY - non-recoverable error"                     },
	{ NET_NO_DATA,            "WSANO_DATA - valid name, no data record of requested type"  },
	{ NET_NO_ADDRESS,         "WSANO_ADDRESS - no address, look for MX record"             },

	// XNetwork specific
	{ NET_NOIMPL,             "XNetwork - Function not implemented"                        },
	{ NET_SOCKET_NOT_CREATED, "XNetwork - socket not yet created"                          },
	{ 0,                      0                                                            } // sentinel
};

class CNetwork::CNetworkThread : public IThread
{
public:
	// Start accepting work on thread
	virtual void ThreadEntry()
	{
		while (Get()->UpdateTick(true) && gEnv && gEnv->pSystem && !gEnv->pSystem->IsQuitting())
		{
		}
	}

	// Signals the thread that it should not accept anymore work and exit
	void SignalStopWork()
	{
		Get()->m_pInternalSocketIOUpr->PushUserMessage(ISocketIOUpr::eUM_QUIT);
	}
};

class CNetwork::CBufferWakeups
{
public:
	CBufferWakeups(bool release, CNetwork* pNet)
		: m_release(false)
		, m_pNet(pNet)
	{
		m_pNet->m_bufferWakeups++;
	}
	~CBufferWakeups()
	{
		if (0 == --m_pNet->m_bufferWakeups)
			if (m_release)
				if (m_pNet->m_wokenUp)
				{
					m_pNet->WakeThread();
					m_pNet->m_wokenUp = false;
				}
	}

private:
	bool      m_release;
	CNetwork* m_pNet;
};

CNetwork::CNetwork()
	: m_bQuitting(false)
	, m_pResolver(0)
	, m_pMMM(NULL)
	, m_bufferWakeups(0)
	, m_wokenUp(false)
	, m_nextCleanup(0.0f)
	, m_cleanupMember(0)
	, m_multithreadedMode(NETWORK_MT_OFF)
	, m_occasionalCounter(OCCASIONAL_TICKS)
	, m_cpuCount(1)
	, m_forceLock(true)
	, m_isPbSvActive(false)
	, m_isPbClActive(false)
	, m_bDelayedExternalWork(false)
#ifdef DEDICATED_SERVER
	, m_pDedicatedServerSchedulerSocket(NULL)
#endif
#ifdef NET_THREAD_TIMING
	, m_threadTimeDepth(0)
#endif
{
	NET_ASSERT(!m_pThis);
	m_pThis = this;
	g_time = gEnv->pTimer->GetAsyncTime();

	m_allowMinimalUpdate = false;
	m_bOverideChannelTickToGoNow = false;

	for (i32 i = 0; i < sizeof(m_inSync) / sizeof(*m_inSync); i++)
		m_inSync[i] = 0;

	SCOPED_GLOBAL_LOCK;

	if (gEnv->IsDedicated())
		CServerProfiler::Init();

	//Timur, Turn off testing.
	/*
	   if (!CWhirlpoolHash::Test())
	   {
	   DrxFatalError("Implementation error in Whirlpool hash");
	   }
	 */

#if ENABLE_DEBUG_KIT
	m_pNetInspector = CNetworkInspector::CreateInstance();
#endif
	m_pServiceUpr.reset(new CServiceUpr);

	m_vFastChannelLookup.reserve(16);
	m_vMembers.reserve(16);

	//#if DRX_PLATFORM_WINDOWS
	//	m_needSleeping = false;
	//	m_sink.m_pNetwork = this;
	//#endif
	i32 frequency = CNetCVars::Get().net_lobbyUpdateFrequency;
	if (frequency < MIN_LOBBY_TICK_FREQUENCY)
	{
		CNetCVars::Get().net_lobbyUpdateFrequency = MIN_LOBBY_TICK_FREQUENCY;
		frequency = MIN_LOBBY_TICK_FREQUENCY;
	}

	m_lobbyTimerInterval = CTimeValue(1.0f / static_cast<float>(frequency));
	m_lobbyTimer = TIMER.ADDTIMER(g_time + m_lobbyTimerInterval, LobbyTimerCallback, this, "LobbyTimer");
}

CNetwork::~CNetwork()
{
	NET_ASSERT(m_pThis == this);

	m_bQuitting = true;

	SyncWithGame(eNGS_WakeNetwork);
	SyncWithGame(eNGS_Shutdown);
	NET_ASSERT(m_vMembers.empty());

	m_compressionUpr.RequestTerminate();

	if (m_multithreadedMode != NETWORK_MT_OFF)
	{
		m_pThread->SignalStopWork();
		gEnv->pThreadUpr->JoinThread(m_pThread.get(), eJM_Join);
		m_pThread.reset();
	}

	// Stop HTTP server now, so we can shutdown cleanly
	// Otherwise, a socket reference is maintained and an assertion will be hit when this DLL is unloaded
	CSimpleHttpServer::GetSingleton().Quit();

	// Make sure all the queues are empty otherwise they will flush and execute when destroyed
	// which can crash if something gets used that has already been destroyed.
	m_toGame.FlushEmpty();
	m_fromGame.FlushEmpty();
	m_fromGame_otherThreadQueue.FlushEmpty();
	m_intQueue.FlushEmpty();
	m_toGameLazyBuilding.FlushEmpty();
	m_toGameLazyProcessing.FlushEmpty();

#if ENABLE_DEBUG_KIT
	CDebugKit::Shutdown();
#endif

	m_pServiceUpr.reset();

	// can safely delete singletons after this point
	// as all threads have been completed

#if ENABLE_DISTRIBUTED_LOGGER
	m_pLogger.reset();
#endif

#if ENABLE_DEBUG_KIT
	if (m_pNetInspector)
		CNetworkInspector::ReleaseInstance();
#endif

#ifdef DEDICATED_SERVER
	m_pDedicatedServerSchedulerSocket = NULL;
#endif

	if (m_pExternalSocketIOUpr == m_pInternalSocketIOUpr)
	{
		SAFE_DELETE(m_pExternalSocketIOUpr);
		m_pInternalSocketIOUpr = NULL;
	}
	else
	{
		SAFE_DELETE(m_pExternalSocketIOUpr);
		SAFE_DELETE(m_pInternalSocketIOUpr);
	}

#if DRX_PLATFORM_WINDOWS
	WSACleanup();
	#if USE_LIVE
	XNetCleanup();
	#endif
#else
	//CSocketUpr::releaseImpl();
	//delete &ISocketUprImplementation::getInstance();
#endif

	SAFE_DELETE(m_pMMM);
	SAFE_DELETE(m_pResolver);

	if (gEnv->IsDedicated())
		CServerProfiler::Cleanup();
}

bool CNetwork::AllSuicidal()
{
	for (VMembers::const_iterator iter = m_vMembers.begin(); iter != m_vMembers.end(); ++iter)
		if (!(*iter)->IsSuicidal())
			return false;
	return true;
}

void CNetwork::FastShutdown()
{
	if (m_bQuitting)
		return;

	SetMultithreadingMode(NETWORK_MT_OFF);    // Kill the network thread then its safe to kill the lobby

	// Enforce global lock after killing of the network thread otherwise we will (recursive) deadlock
	SCOPED_GLOBAL_LOCK;
	m_bQuitting = true;
	FlushNetLog(true);
	m_pServiceUpr.reset();

	IDrxLobby* pLobby = GetLobby();

	if (pLobby)
	{
		for (u32 i = 0; i < eCLS_NumServices; i++)
		{
			pLobby->Terminate((EDrxLobbyService)i, eCLSO_All, NULL, NULL);
		}
	}
}

void CNetwork::SetCDKey(tukk key)
{
	m_CDKey = key;
}

void CNetwork::AddExecuteString(const string& toExec)
{
	if (IsPrimaryThread())
		gEnv->pConsole->ExecuteString(toExec.c_str());
	else
		m_consoleCommandsToExec.push_back(toExec);
}

// EvenBalance - M. Quinn
bool CNetwork::PbConsoleCommand(tukk command, i32 length)
{
#ifdef __WITH_PB__
	SCOPED_GLOBAL_LOCK;

	if (!strnicmp(command, "pb_sv", 5))
	{
		PbSvAddEvent(PB_EV_CMD, -1, length, (tuk)command);
		return true;
	}
	else if (!strnicmp(command, "pb_", 3))
	{
		PbClAddEvent(PB_EV_CMD, length, (tuk)command);
		return true;
	}
	else
		return false;
#else
	return true;
#endif
}

// EvenBalance - M. Quinn
void CNetwork::PbCaptureConsoleLog(tukk output, i32 length)
{

#ifdef __WITH_PB__
	SCOPED_GLOBAL_LOCK_NO_LOG;

	PbCaptureConsoleOutput((tuk)output, length);
#endif

}

// EvenBalance - M. Quinn
void CNetwork::PbServerAutoComplete(tukk command, i32 length)
{
#ifdef __WITH_PB__
	SCOPED_GLOBAL_LOCK;

	PbServerCompleteCommand((tuk)command, length);
	return;
#endif
}

// EvenBalance - M. Quinn
void CNetwork::PbClientAutoComplete(tukk command, i32 length)
{
#ifdef __WITH_PB__
	SCOPED_GLOBAL_LOCK;

	PbClientCompleteCommand((tuk)command, length);
	return;
#endif
}

#if DRX_PLATFORM_WINDOWS
	#include <IPHlpApi.h>
//LINK_SYSTEM_LIBRARY("libIPHlpApi.a")
#endif

bool CNetwork::CNetworkConnectivityDetection::HasNetworkConnectivity()
{
	float detectionInterval = std::max(1.0f, CVARS.NetworkConnectivityDetectionInterval);
	// if we've received a packet recently, then assume that we still have connectivity
	if ((g_time - m_lastPacketReceived).GetSeconds() < detectionInterval)
		m_hasNetworkConnectivity = true;
	// otherwise, if we've not checked for some time, check again (also triggers the first time this routine is called normally)
	else if ((g_time - m_lastCheck).GetSeconds() > detectionInterval)
	{
		DetectNetworkConnectivity();
		m_lastCheck = g_time;
	}
	return m_hasNetworkConnectivity;
}

void CNetwork::CNetworkConnectivityDetection::DetectNetworkConnectivity()
{
	DRX_PROFILE_FUNCTION(PROFILE_NETWORK);

	std::vector<string> warnings;
	bool hadNetworkConnectivity = m_hasNetworkConnectivity;

	m_hasNetworkConnectivity = true;

#if DRX_PLATFORM_WINDOWS
	DWORD size = 0;
	GetInterfaceInfo(NULL, &size);

	std::vector<BYTE> buffer;
	buffer.resize(size);

	PIP_INTERFACE_INFO info = (PIP_INTERFACE_INFO)&buffer[0];
	if (NO_ERROR != GetInterfaceInfo(info, &size))
	{
		warnings.push_back(string().Format("[connectivity] Failed retrieving network interface table"));
		m_hasNetworkConnectivity = false;
	}
	else if (info->NumAdapters <= 0)
	{
		warnings.push_back(string().Format("[connectivity] No network adapters available"));
		m_hasNetworkConnectivity = false;
	}
	else
	{
		DWORD nconnected = info->NumAdapters;
		for (LONG i = 0; i < info->NumAdapters; ++i)
		{
			IP_ADAPTER_INDEX_MAP& adapter = info->Adapter[i];
			MIB_IFROW ifr;
			ZeroMemory(&ifr, sizeof(ifr));
			ifr.dwIndex = adapter.Index;
			
			if (NO_ERROR != GetIfEntry(&ifr)) //GetIfEntry потерялся(((
			{
				warnings.push_back(string().Format("[connectivity] Failed retrieving network interface at index %lu", ifr.dwIndex));
				//m_hasNetworkConnectivity = false;
				--nconnected;
			}
			else if (ifr.dwOperStatus != MIB_IF_OPER_STATUS_OPERATIONAL)
			{
				warnings.push_back(string().Format("[connectivity] %s is not operational", ifr.bDescr));
				//m_hasNetworkConnectivity = false;
				--nconnected;
			}
		}
		m_hasNetworkConnectivity = (nconnected != 0);
	}
#else
	// TODO:
#endif

	if (hadNetworkConnectivity && !m_hasNetworkConnectivity)
	{
		for (size_t i = 0; i < warnings.size(); ++i)
		{
			NetWarning("%s", warnings[i].c_str());
		}
	}
}

bool CNetwork::HasNetworkConnectivity()
{
	SCOPED_GLOBAL_LOCK;
	return m_detection.HasNetworkConnectivity();
}

bool CNetwork::Init(i32 ncpu)
{
	m_cpuCount = ncpu;

	m_gameTime = gEnv->pTimer->GetFrameStartTime();
	m_pMessageQueueConfig = CMessageQueue::LoadConfig("%engine%/Config/DefaultScripts/Scheduler.xml");
	DRX_ASSERT(m_pMessageQueueConfig != nullptr);

	tukk gameOverrideXml = "Scripts/Network/Scheduler.xml";
	if (gEnv->pDrxPak->IsFileExist(gameOverrideXml))
	{
		if (XmlNodeRef rootNode = gEnv->pSystem->LoadXmlFromFile(gameOverrideXml))
		{
			m_pMessageQueueConfig->Read(rootNode);
		}
	}

	m_schedulerVersion = 1;

	if (!m_pResolver)
		m_pResolver = new CNetAddressResolver();

	if (!m_pMMM)
	{
		m_pMMM = new CMementoMemoryUpr("Network");
	}

#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO

	#if USE_LIVE
	XNetStartupParams xnsp;
	memset(&xnsp, 0, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
	// If Live security ever needs turning off reinsert the line below.
	// Security doesn't appear to effect standard UDP data so cross platform LAN games still work with it enabled.
		#if !XBOX_RELEASE_USE_SECURE_SOCKETS || !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
		#endif
	if (XNetStartup(&xnsp) != 0)
	{
		NetWarning("XNetStartup() failed.");
		return false;
	}
	#endif // USE_LIVE

	u16 wVersionRequested;
	WSADATA wsaData;
	i32 err;
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		return false;
	}

	if (LOBYTE(wsaData.wVersion) != 2 ||
	    HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
	#if USE_LIVE
		XNetCleanup();
	#endif
		return false;
	}
#endif

	i32 n = 0;
	while (m_neNetErrors[n].sErrorDescription != NULL)
	{
		m_mapErrors[m_neNetErrors[n].nrErrorCode] = m_neNetErrors[n].sErrorDescription;
		n++;
	}

	if (!CreateSocketIOUpr(m_cpuCount, &m_pExternalSocketIOUpr, &m_pInternalSocketIOUpr))
		return false;
	DrxLogAlways("[net] Socket IO management: External [%s], Internal [%s]", GetExternalSocketIOUpr().GetName(), GetInternalSocketIOUpr().GetName());

	LogNetworkInfo();

	InitPrimaryThread();
	InitFrameTypes();

#if ENABLE_DISTRIBUTED_LOGGER
	m_pLogger.reset(new CDistributedLogger);
#endif

	g_pFileDownloader = new CAutoConfigDownloader;

#define REGISTER_POLICY(cls)                       \
  extern void RegisterCompressionPolicy_ ## cls(); \
  RegisterCompressionPolicy_ ## cls();             \

	REGISTER_POLICY(CAdaptiveBoolPolicy);
	REGISTER_POLICY(CAdaptiveFloatPolicy);
	REGISTER_POLICY(TAdaptiveOrientationPolicy);
	REGISTER_POLICY(TPredictiveOrientationPolicy);
	REGISTER_POLICY(CAdaptiveUnitVec3Policy);
	REGISTER_POLICY(TAdaptiveVec3Policy);
	REGISTER_POLICY(TPredictiveVec3Policy);
	REGISTER_POLICY(CInt64Policy);
	REGISTER_POLICY(CAdaptiveVelocityPolicy);
	REGISTER_POLICY(CBiggerOrSmallerPolicy);
	REGISTER_POLICY(CDefaultPolicy);
	REGISTER_POLICY(CEntityIdPolicy);
	REGISTER_POLICY(CSimpleEntityIdPolicy);
	REGISTER_POLICY(CFloatAsIntPolicy);
	REGISTER_POLICY(CJumpyPolicy);
	REGISTER_POLICY(CNoSendPolicy);
	REGISTER_POLICY(CQuantizedVec3Policy);
	REGISTER_POLICY(CRangedIntPolicy);
	REGISTER_POLICY(CRangedUnsignedIntPolicy);
	REGISTER_POLICY(CStringTablePolicy);
	REGISTER_POLICY(CTimePolicy);
	REGISTER_POLICY(CTimePolicyWithDistribution);
	REGISTER_POLICY(TTableDirVec3);

#undef REGISTER_POLICY
	return true;
}

void DownloadConfig()
{
	g_pFileDownloader->TriggerRefresh();
}

void CNetwork::SetMultithreadingMode(ENetwork_Multithreading_Mode threadingMode)
{
	m_mutex.Lock();

	if (gEnv->IsEditor() == true)
	{
		threadingMode = NETWORK_MT_OFF;
	}

	if (threadingMode != m_multithreadedMode)
	{
		//Threading is currently disabled - start and set prio
		if (m_multithreadedMode == NETWORK_MT_OFF)
		{
			m_pThread.reset(new CNetworkThread());

			if (!gEnv->pThreadUpr->SpawnThread(m_pThread.get(), "Network"))
			{
				DrxFatalError("Error spawning \"Network\" thread.");
			}
		}
		//disable thread
		else if (threadingMode == NETWORK_MT_OFF)
		{
			m_pThread->SignalStopWork();
			m_mutex.Unlock();
			gEnv->pThreadUpr->JoinThread(m_pThread.get(), eJM_Join);
			m_mutex.Lock();
			m_pThread.reset();
		}

		m_multithreadedMode = threadingMode;
	}
	m_mutex.Unlock();
}

void CNetwork::WakeThread()
{
#if !LOCK_NETWORK_FREQUENCY
	if (m_bufferWakeups)
		m_wokenUp = true;
	else
		m_pInternalSocketIOUpr->PushUserMessage(eUM_WAKE);
#endif
}

void CNetwork::BroadcastNetDump(ENetDumpType type)
{
	for (size_t i = 0; i < m_vMembers.size(); i++)
	{
		INetworkMemberPtr pMember = m_vMembers[i];
		pMember->NetDump(type);
	}
}

void CNetwork::LobbyTimerCallback(NetTimerId id, uk pUserData, CTimeValue time)
{
	CNetwork* pNetwork = static_cast<CNetwork*>(pUserData);

	if (gEnv->pLobby)
	{
		gEnv->pLobby->Tick(false);
	}

	pNetwork->m_lobbyTimer = TIMER.ADDTIMER(g_time + pNetwork->m_lobbyTimerInterval, LobbyTimerCallback, pNetwork, "LobbyTimer");
}

//////////////////////////////////////////////////////////////////////////
INetNub* CNetwork::CreateNub(tukk address, IGameNub* pGameNub,
                             IGameSecurity* pSecurity,
                             IGameQuery* pGameQuery)
{
	SCOPED_GLOBAL_LOCK;
	TNetAddressVec addrs;
	if (address)
	{
		CNameRequestPtr pReq = GetResolver()->RequestNameLookup(address);
		pReq->Wait();
		if (pReq->GetResult(addrs) != eNRR_Succeeded)
		{
			DrxLogAlways("Name resolution for '%s' failed", address);
			return NULL;
		}
		//		GetResolver()->FromString( address, addrs );
	}
	else
	{
		SIPv4Addr addr; // defaults to 0,0
		addrs.push_back(TNetAddress(addr));
	}
	if (addrs.empty())
	{
		SAFE_RELEASE(pGameNub);
		return NULL;
	}

	//DrxLog( "Resolved name as %s", GetResolver()->ToNumericString(addrs[0]).c_str() );
	CNetNub* pNub = new CNetNub(addrs[0], pGameNub,
	                            pSecurity, pGameQuery);
	if (!pNub->Init(this))
	{
		NetWarning("Failed creating network nub at %s", address);
		delete pNub;
		return NULL;
	}

	AddMember(pNub);
	CNetwork::Get()->WakeThread();
	return pNub;
}

ILanQueryListener* CNetwork::CreateLanQueryListener(IGameQueryListener* pGameQueryListener)
{
	SCOPED_GLOBAL_LOCK;
	CLanQueryListener* pLanQueryListener = new CLanQueryListener(pGameQueryListener);
	if (!pLanQueryListener->Init())
	{
		delete pLanQueryListener;
		return NULL;
	}

	AddMember(pLanQueryListener);
	CNetwork::Get()->WakeThread();
	return pLanQueryListener;
}

#if ENABLE_PACKET_PREDICTION
void ResetMessageApproximations();
#endif

INetContext* CNetwork::CreateNetContext(IGameContext* pGameContext, u32 flags)
{
	SCOPED_GLOBAL_LOCK;

#if ENABLE_PACKET_PREDICTION
	ResetMessageApproximations();
#endif

	CNetContext* pContext = new CNetContext(pGameContext, flags);
	AddMember(pContext);
	CNetwork::Get()->WakeThread();
	return pContext;
}

tukk CNetwork::EnumerateError(NRESULT err)
{
	TErrorMap::iterator i = m_mapErrors.find(err);
	if (i != m_mapErrors.end())
	{
		return i->second;
	}
	return "Unknown";
}

void CNetwork::Release()
{
	//	SCOPED_GLOBAL_LOCK;

	//	if (m_nNetworkInitialized)
	//		;

	delete this;
	m_pThis = 0;
}

void CNetwork::GetMemoryStatistics(IDrxSizer* pSizer)
{
	// dont lock if we want to compare the heap with all allocated data
	SCOPED_GLOBAL_LOCK;

#ifndef _LIB // Only when compiling as dynamic library
	{
		//SIZER_COMPONENT_NAME(pSizer,"Strings");
		//pSizer->AddObject( (this+1),string::_usedMemory(0) );
	}
	{
		SIZER_COMPONENT_NAME(pSizer, "STL Allocator Waste");
		DrxModuleMemoryInfo meminfo;
		ZeroStruct(meminfo);
		DrxGetMemoryInfoForModule(&meminfo);
		pSizer->AddObject((this + 2), (size_t)meminfo.STL_wasted);
	}
#endif

	SIZER_COMPONENT_NAME(pSizer, "CNetwork");

	if (!pSizer->Add(*this))
		return;

	pSizer->AddObject(m_neNetErrors, sizeof(m_neNetErrors));
	pSizer->AddContainer(m_vMembers);

	for (VMembers::iterator i = m_vMembers.begin(); i != m_vMembers.end(); ++i)
		(*i)->GetMemoryStatistics(pSizer);

	m_toGame.GetMemoryStatistics(pSizer);
	m_fromGame.GetMemoryStatistics(pSizer);
	m_intQueue.GetMemoryStatistics(pSizer);
	m_toGameLazyBuilding.GetMemoryStatistics(pSizer);
	m_toGameLazyProcessing.GetMemoryStatistics(pSizer);
	m_timer.GetMemoryStatistics(pSizer);
	m_accurateTimer.GetMemoryStatistics(pSizer);

	{
		DrxAutoLock<NetFastMutex> lk(m_fromGame_otherThreadLock);
		m_fromGame_otherThreadQueue.GetMemoryStatistics(pSizer);
	}

	//	for (i32 i=0; i<eMMT_NUM_TYPES; i++)
	//		m_mmm[i].GetMemoryStatistics(pSizer);

	m_compressionUpr.GetMemoryStatistics(pSizer);

	m_pServiceUpr->GetMemoryStatistics(pSizer);

	//m_pMessageQueueConfig->GetMemoryStatistics(pSizer);

	//#if DRX_PLATFORM_WINDOWS && ( defined(_DEBUG) || defined(DEBUG) )
	//	_CrtMemDumpAllObjectsSince(NULL);
	//	_CrtMemState ms;
	//	_CrtMemCheckpoint(&ms);
	//	_CrtMemDumpStatistics(&ms);
	//#endif
}

class CInSync
{
public:
	CInSync(i32& inSync) : m_inSync(inSync)
	{
		ASSERT_PRIMARY_THREAD;
		++m_inSync;
	}
	~CInSync()
	{
		ASSERT_PRIMARY_THREAD;
		--m_inSync;
	}
private:
	i32& m_inSync;
};

void CNetwork::DoSyncWithGameMinimal()
{
	if ((!gEnv) || (!gEnv->pSystem) || gEnv->pSystem->IsQuitting())
	{
		return;                                     // No point doing these if we are shutting down
	}

	m_toGame.Flush(true);           // Keep network and game talking - but don't try to process lazy queue - we are actually in the middle of its flush
	m_fromGame.Flush(true);

	IDrxLobby* pLobby = GetLobby();
	if (pLobby)
	{
		EDrxLobbyError error = pLobby->ProcessEvents();

		if (error != eCLE_Success)
		{
			NetLog("Minimal: Error running pLobby->ProcessEvents (%d)", error);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::SyncWithGame(ENetworkGameSync type)
{
	if (type == eNGS_AllowMinimalUpdate)
	{
		m_allowMinimalUpdate = true;
		return;
	}
	if (type == eNGS_DenyMinimalUpdate)
	{
		m_allowMinimalUpdate = false;
		return;
	}
	if (type == eNGS_MinimalUpdateForLoading)
	{
		if (m_allowMinimalUpdate)
		{
			SCOPED_GLOBAL_LOCK;
			DoSyncWithGameMinimal();
		}
		return;
	}
	if (type == eNGS_SleepNetwork)
	{
#if LOCK_NETWORK_FREQUENCY
		if (m_pInternalSocketIOUpr)
		{
			m_pInternalSocketIOUpr->PushUserMessage(ISocketIOUpr::eUM_SLEEP);
		}
#endif
		return;
	}
	if (type == eNGS_WakeNetwork)
	{
#if LOCK_NETWORK_FREQUENCY
		if (m_pInternalSocketIOUpr)
		{
			m_pInternalSocketIOUpr->ForceNetworkStart();
		}
#endif
		return;
	}
	if (type == eNGS_ForceChannelTick)
	{
		m_bOverideChannelTickToGoNow = true;
		return;
	}
	if (type == eNGS_DisplayDebugInfo)
	{
#if ENABLE_NET_DEBUG_INFO || NET_PROFILE_ENABLE || NET_MINI_PROFILE
		SCOPED_GLOBAL_LOCK;
#endif // #if ENABLE_NET_DEBUG_INFO || NET_PROFILE_ENABLE || NET_MINI_PROFILE

#if ENABLE_NET_DEBUG_INFO
		CNetDebugInfo::Get()->Tick();
#endif
#if NET_PROFILE_ENABLE || NET_MINI_PROFILE
		memcpy(&g_socketBandwidth.bandwidthStats.m_prev, &g_socketBandwidth.bandwidthStats.m_total, sizeof(SBandwidthStatsSubset));
#endif
		return;
	}

	//#if DRX_PLATFORM_WINDOWS
	//	// HACK: to make the WMI thread responsive on single core
	//	if (type == eNGS_FrameEnd && m_cpuCount < 2 && m_needSleeping)
	//		Sleep(1);
	//#endif

	static CThreadProfilerEvent evt_start("net_startframe");
	CThreadProfilerEvent::Instance show_evt_start(evt_start, type == eNGS_FrameStart);

	DRX_PROFILE_FUNCTION(PROFILE_NETWORK);

#if _DEBUG && defined(USER_craig)
	//SCOPED_ENABLE_FLOAT_EXCEPTIONS;
#endif

	if (m_bQuitting)
	{
		return;
	}

	i32 totSync = 0;
	for (i32 i = 0; i < sizeof(m_inSync) / sizeof(*m_inSync); i++)
	{
		totSync += m_inSync[i];
	}

	switch (type)
	{
	case eNGS_Shutdown:
		if (totSync)
		{
			DrxFatalError("CNetwork::SyncWithGame(eNGS_Shutdown) called recursively");
		}
		break;
	case eNGS_FrameStart:
	case eNGS_FrameEnd:
		if (totSync && totSync != (m_inSync[eNGS_Shutdown] + m_inSync[eNGS_NUM_ITEMS + 1]))
		{
			return;
		}
		break;
	}

	{
#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
		char profileLabel[32];
		drx_sprintf(profileLabel, "SyncWithGame() lock %d", type);
		DRX_PROFILE_REGION(PROFILE_NETWORK, "SyncWithGame() lock unknown");
		DRXPROFILE_SCOPE_PROFILE_MARKER(profileLabel);
		DRXPROFILE_SCOPE_PLATFORM_MARKER(profileLabel);
#endif
		CTimeValue startTime = gEnv->pTimer->GetAsyncTime();

		SCOPED_GLOBAL_LOCK;
		CBufferWakeups bufferWakeups(type == eNGS_FrameEnd, this);
		CInSync inSync(m_inSync[type]);
		DoSyncWithGame(type);

		CTimeValue middleTime = gEnv->pTimer->GetAsyncTime();

		EDrxLobbyError error = eCLE_Success;
		///// Moved here, need to ensure we have the global lock due to memory allocations..etc in the lobby
		IDrxLobby* pLobby = GetLobby();
		if (pLobby)
		{
			error = pLobby->ProcessEvents();

			if (error != eCLE_Success)
			{
				NetLog("Error running pLobby->ProcessEvents (%d)", error);
			}
		}

		CTimeValue endTime = gEnv->pTimer->GetAsyncTime();

		int64 lobbyTick = (endTime - middleTime).GetMilliSecondsAsInt64();
		int64 normalTick = (middleTime - startTime).GetMilliSecondsAsInt64();
		int64 total = lobbyTick + normalTick;

		if (total > 50LL)
		{
			NetLog("CNetwork::SyncWithGame(%d) took a long time %" PRId64 ", normal=%" PRId64 ", lobby=%" PRId64 ", gotLobbyLock=%s", type, total, normalTick, lobbyTick, error == eCLE_Success ? "true" : "false");
		}
	}

	if (!m_inSync[eNGS_NUM_ITEMS + 1])
	{
		CInSync inSyncFlush(m_inSync[eNGS_NUM_ITEMS + 1]);
		m_toGameLazyProcessing.Flush(false);

	}
}

#if ENABLE_OBJECT_COUNTING
struct SObjCntDisplay
{
	tukk name;
	i32         n;
	bool operator<(const SObjCntDisplay& r) const
	{
		return n > r.n;
	}
};
static void DumpObjCnt()
{
	SObjCntDisplay temp;
	std::vector<SObjCntDisplay> v;
	#define COUNTER(cnt) temp.n = g_objcnt.cnt.QuickPeek(); if (temp.n) { temp.name = # cnt; v.push_back(temp); }
	#include  <drx3D/Network/objcnt_defs.h>
	#undef COUNTER

	std::sort(v.begin(), v.end());

	if (ITextModeConsole* pTMC = gEnv->pSystem->GetITextModeConsole())
	{
		char buf[256];
		i32 y = 20;

		IMemoryUpr::SProcessMemInfo memInfo;
		GetISystem()->GetIMemoryUpr()->GetProcessMemInfo(memInfo);
		float kils = memInfo.WorkingSetSize / 1024.0f;
		drx_sprintf(buf, "MEM: %.2f kilobyte", kils);
		pTMC->PutText(0, y++, buf);

		for (std::vector<SObjCntDisplay>::iterator it = v.begin(); it != v.end(); ++it)
		{
			drx_sprintf(buf, "%d", it->n);
			pTMC->PutText(0, y, buf);
			pTMC->PutText(10, y++, it->name);
		}
	}
};
#endif
//////////////////////////////////////////////////////////////////////////
void CNetwork::GetPerformanceStatistics(SNetworkPerformance* pSizer)
{
	pSizer->m_nNetworkSync = m_networkPerformance.m_nNetworkSync;
#ifdef NET_THREAD_TIMING
	pSizer->m_threadTime = m_threadTime;
#else
	pSizer->m_threadTime = 0.0f;
#endif
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::DoSyncWithGame(ENetworkGameSync type)
{
	bool continuous = false;

	//

	switch (type)
	{
	case eNGS_FrameStart:
		{
			PerformanceGuard guard(&m_networkPerformance, 0);
			DRX_PROFILE_REGION(PROFILE_NETWORK, "Network:FrameStart");
			if (gEnv->IsDedicated())
				g_pFileDownloader->Update();
			if (!gEnv->IsEditor())
				m_gameTime = gEnv->pTimer->GetFrameStartTime();
			else
				m_gameTime = gEnv->pTimer->GetAsyncTime();
			//NetQuickLog(false, 0, "Local: %f", m_gameTime.GetSeconds());
			FlushNetLog(false);
			m_toGame.Flush(true);
			if (!m_inSync[eNGS_NUM_ITEMS + 1])
				m_toGameLazyBuilding.Swap(m_toGameLazyProcessing);

			NET_PROFILE_TICK();

#if ENABLE_DEBUG_KIT
			if (CNetCVars::Get().PerfCounters)
			{
				if (ITextModeConsole* pTMC = gEnv->pSystem->GetITextModeConsole())
				{
					i32 y = 0;
					char buf[256];
	#define DUMP_COUNTER(name) drx_sprintf(buf, # name ": %d (%d)", g_systemBranchCounters.name, g_systemBranchCounters.name - g_lastSystemBranchCounters.name); pTMC->PutText(0, y++, buf)
					DUMP_COUNTER(updateTickMain);
					DUMP_COUNTER(updateTickSkip);
					DUMP_COUNTER(iocpReapSleep);
					DUMP_COUNTER(iocpReapSleepCollect);
					DUMP_COUNTER(iocpReapImmediate);
					DUMP_COUNTER(iocpReapImmediateCollect);
					DUMP_COUNTER(iocpIdle);
					DUMP_COUNTER(iocpBackoffCheck);
					DUMP_COUNTER(iocpForceSync);
	#undef DUMP_COUNTER
					g_lastSystemBranchCounters = g_systemBranchCounters;
				}
			}
#endif

#if ENABLE_OBJECT_COUNTING
			DumpObjCnt();
#endif
		}
		break;
	case eNGS_FrameEnd:
		{
		DRX_PROFILE_REGION(PROFILE_NETWORK, "Network:FrameEnd");
			PerformanceGuard guard(&m_networkPerformance, m_networkPerformance.m_nNetworkSync);

			//NetQuickLog( gEnv->IsDedicated(), 0, "locks/frame: %d", g_lockCount );
			g_lockCount = 0;

			FlushNetLog(true);
			m_fromGame.Flush(true);
			{
				DrxAutoLock<NetFastMutex> lk(m_fromGame_otherThreadLock);
				m_fromGame_otherThreadQueue.Flush(true);
			}
			CMementoMemoryUpr::DebugDraw();

#if ENABLE_DEBUG_KIT
			CDebugKit::Get().Update();
#endif
#if STATS_COLLECTOR_INTERACTIVE
			GetStats()->InteractiveUpdate();
#endif

			if (CServerProfiler::ShouldSaveAndCrash())
			{
				gEnv->pConsole->ExecuteString("SaveLevelStats");
#if DRX_PLATFORM_ORBIS
				_Exit(0);
#else
				_exit(0);
#endif
			}
		}
		break;
	case eNGS_Shutdown:
		FlushNetLog(true);
		m_toGame.Flush(true);
		m_fromGame.Flush(true);
		{
			DrxAutoLock<NetFastMutex> lk(m_fromGame_otherThreadLock);
			m_fromGame_otherThreadQueue.Flush(true);
		}
		continuous = true;
		break;
	case eNGS_Shutdown_Clear:
		FlushNetLog(true);
		m_toGame.FlushEmpty();
		m_fromGame.FlushEmpty();
		{
			DrxAutoLock<NetFastMutex> lk(m_fromGame_otherThreadLock);
			m_fromGame_otherThreadQueue.FlushEmpty();
		}
		continuous = true;
		break;
	}

	if (!continuous)
	{
		UpdateLoop(type);
	}
	else
		while (true)
		{
			CInSync inSync(m_inSync[eNGS_NUM_ITEMS]);

			bool emptied = false;
			bool suicidal = false;
			{
				m_toGame.Flush(true);
				UpdateLoop(type);
				emptied = m_vMembers.empty();
				if (!emptied)
					suicidal = AllSuicidal();
				m_fromGame.Flush(true);
				m_toGame.Flush(true);
				{
					DrxAutoLock<NetFastMutex> lk(m_fromGame_otherThreadLock);
					m_fromGame_otherThreadQueue.Flush(true);
				}
			}
			if (continuous)
				if (!emptied)
					if (suicidal)
					{
						{
							CInSync inSyncFlush(m_inSync[eNGS_NUM_ITEMS + 1]);
							if (m_multithreadedMode != NETWORK_MT_OFF)
								m_mutex.Unlock();
							m_toGameLazyProcessing.Flush(false);
							if (m_multithreadedMode != NETWORK_MT_OFF)
								m_mutex.Lock();
							m_toGameLazyBuilding.Swap(m_toGameLazyProcessing);
						}
						DoSyncWithGame(eNGS_FrameStart);
						DoSyncWithGame(eNGS_FrameEnd);
#if LOCK_NETWORK_FREQUENCY
						m_pInternalSocketIOUpr->ForceNetworkStart();
#endif
						continue;
					}
			break;
		}
#if ENABLE_DISTRIBUTED_LOGGER
	if (m_pLogger.get())
		m_pLogger->Update(g_time);
#endif
#if ENABLE_DEBUG_KIT
	if (m_cvars.NetInspector)
		if (m_pNetInspector)
			m_pNetInspector->Update();
#endif
}

void CNetwork::UpdateLoop(ENetworkGameSync type)
{
	DRX_PROFILE_FUNCTION(PROFILE_NETWORK);

	if (type == eNGS_Shutdown)
	{
		return;
	}
	else if (type == eNGS_FrameEnd)
	{
		for (size_t i = 0; i < m_vMembers.size(); i++)
		{
			INetworkMemberPtr pMember = m_vMembers[i];
			if (pMember->IsDead())
			{
				m_vMembers.erase(m_vMembers.begin() + i);
				i--;
			}
			else
			{
				ENSURE_REALTIME;
				pMember->SyncWithGame(type);
			}
		}
	}
	else
	{
		for (i32 i = (i32)m_vMembers.size() - 1; i >= 0; i--)
		{
			ENSURE_REALTIME;
			// Prevent members that die during the frame from being sync'ed (can happen
			// during host migration as this can occur at any point in the frame)
			INetworkMemberPtr pMember = m_vMembers[i];
			if (!pMember->IsDead())
			{
				pMember->SyncWithGame(type);
			}
		}
	}

#ifdef __WITH_PB__
	if (type == eNGS_FrameEnd && !gEnv->IsEditor())
	{
		if (m_isPbSvActive)
		{
			//NET_ASSERT(isPbSvEnabled() != 0);
			PbServerProcessEvents();
		}

		if (m_isPbClActive)
		{
			//NET_ASSERT(isPbClEnabled() != 0);
			PbClientProcessEvents();
		}

		ASSERT_PRIMARY_THREAD;
		for (size_t i = 0; i < m_consoleCommandsToExec.size(); i++)
			gEnv->pConsole->ExecuteString(m_consoleCommandsToExec[i].c_str());
		m_consoleCommandsToExec.resize(0);
	}
#endif

	if (m_multithreadedMode == NETWORK_MT_OFF && type == eNGS_FrameEnd)
	{
		UpdateTick(false);
	}
}

bool CNetwork::UpdateTick(bool mt)
{
#if LOCK_NETWORK_FREQUENCY

	#if LOG_SOCKET_TIMEOUTS
	CTimeValue beforeSleep = gEnv->pTimer->GetAsyncCurTime();
	#endif // LOG_SOCKET_TIMEOUTS

	if (mt)
	{
		DRX_PROFILE_REGION_WAITING(PROFILE_NETWORK, "Wait - Network Wakeup");

		NET_STOP_THREAD_TIMER();
		bool inTime = m_pInternalSocketIOUpr->NetworkSleep();
		NET_TICK_THREAD_TIMER();

	#if LOG_SOCKET_TIMEOUTS
		CTimeValue afterSleep = gEnv->pTimer->GetAsyncCurTime();
		if (!inTime)
		{
			NetLog("CNetwork::UpdateTick(): stall detected, semaphore sleep was %" PRIi64, (afterSleep - beforeSleep).GetMilliSecondsAsInt64());
		}
	#endif // LOG_SOCKET_TIMEOUTS
	}

#endif // LOCK_NETWORK_FREQUENCY

	while (true)
	{
		DRX_PROFILE_REGION(PROFILE_NETWORK, "Network DoMainTick");

		switch (DoMainTick(mt))
		{
		case eTRS_Quit:
			return false;
		case eTRS_Continue:
			return true;
		case eTRS_TimedOut:
		//NetLog("CNetwork::UpdateTick(): Poll timed out - possible long game frame?");
		// fall-through for another tick
		default:
			break;
		}
	}
}

CNetwork::eTickReturnState CNetwork::DoMainTick(bool mt)
{
	eTickReturnState ret = eTRS_Continue;
	u32 waitTime = 0;

	ITimer* pTimer = gEnv->pTimer;
	bool mustLock = m_forceLock || m_bDelayedExternalWork;
	m_forceLock = false;
	CTimeValue now = pTimer->GetAsyncTime();
	mustLock |= ((now - m_lastUpdateLock).GetMilliSeconds() > 15);

	bool isLocked = false;
	{
		DRX_PROFILE_REGION_WAITING(PROFILE_NETWORK, "Wait - Network DoMainTick lock");

		if (mustLock)
		{
			m_mutex.Lock();
			isLocked = true;
			now = pTimer->GetAsyncTime();
		}
		else
		{
			isLocked = m_mutex.TryLock();
		}
	}

	bool didWorkInExternalPoll = false;

	if (isLocked)
	{
		DRX_PROFILE_FUNCTION(PROFILE_NETWORK);

		if (m_bDelayedExternalWork)
		{
			DRX_PROFILE_REGION_WAITING(PROFILE_NETWORK, "Wait - Network Poll Wait External");

			m_pExternalSocketIOUpr->PollWork(didWorkInExternalPoll);
			m_bDelayedExternalWork = false;
		}

		m_lastUpdateLock = now;
		g_systemBranchCounters.updateTickMain++;

#if USE_ACCURATE_NET_TIMERS
		if (mt)
		{
			waitTime = static_cast<u32>(max((TIMER.Update() - g_time).GetMilliSeconds(), 1.0f));
		}
		else
		{
			TIMER.Update();
		}
#else
		waitTime = static_cast<u32>((TIMER.Update() - g_time).GetSeconds());
		ACCURATE_NET_TIMER.Update();
#endif // USE_ACCURATE_NET_TIMERS

		//#if CHANNEL_TIED_TO_NETWORK_TICK
#if USE_CHANNEL_TIMERS
		if (gEnv->bMultiplayer && m_bOverideChannelTickToGoNow)
#endif
		{
			DRX_PROFILE_REGION(PROFILE_NETWORK, "Network Tick Channels");

			// Tick the channels
			u32k numMembers = m_vMembers.size();
			for (u32 i = 0; i < numMembers; ++i)
			{
				INetworkMemberPtr pMember = m_vMembers[i];
				if (pMember->UpdateOrder == eNMUO_Nub)
				{
					pMember->TickChannels(now, m_bOverideChannelTickToGoNow);
				}
			}
			m_bOverideChannelTickToGoNow = false;
		}
		//#endif

		m_pThis->m_intQueue.Flush(true);

		if (!m_vMembers.empty() && g_time >= m_nextCleanup)
		{
			INetworkMemberPtr pMember = m_vMembers[m_cleanupMember % m_vMembers.size()];
			if (!pMember->IsDead())
				pMember->PerformRegularCleanup();
			m_cleanupMember++;
		}

		CMementoMemoryUpr::Tick();
		CSimpleHttpServer::GetSingleton().Tick();

#if !USE_ACCURATE_NET_TIMERS
	#define WAIT_TIME_MIN (1) //in milliseconds

		waitTime *= 1000;

		if (waitTime > 100)
			waitTime = 100;
		else if (waitTime < WAIT_TIME_MIN)
			waitTime = WAIT_TIME_MIN;

		if (!mt)
			waitTime = 0;
#endif // !USE_ACCURATE_NET_TIMERS
		IDrxMatchMakingPrivate* pMMPrivate = gEnv->pLobby ? gEnv->pLobby->GetMatchMakingPrivate() : nullptr;
		if (pMMPrivate)
		{
			pMMPrivate->LobbyAddrIDTick();
		}
		m_mutex.Unlock();
	}
	else
	{
#if !USE_ACCURATE_NET_TIMERS
		waitTime = 1;
#endif // !USE_ACCURATE_NET_TIMERS
		g_systemBranchCounters.updateTickSkip++;
	}

	bool didWorkInInternalPoll = false;
	bool internalWorkToDo = false;
	bool externalWorkToDo = false;
	bool shouldPoll = true;
	i32 pollres = 0;
#if LOCK_NETWORK_FREQUENCY
	i32 timeout = 0;
	if (gEnv->bMultiplayer && !gEnv->IsEditor())
	{
		waitTime = CNetCVars::Get().socketMaxTimeoutMultiplayer;
		timeout = (CNetCVars::Get().socketMaxTimeout + CNetCVars::Get().socketMaxTimeoutMultiplayer - 1) / CNetCVars::Get().socketMaxTimeoutMultiplayer;
	}
	else
	{
		if (!gEnv->IsEditor())
		{
			// Normally (in single player) the network should use the maxtimeout to ensure it doesn't hog cpu, however during loading we actually don't mind, and by decreasing the timeout we can chew through
			//the context establishers much more effeciently
			bool isEstablishing = false;
			u32k numMembers = m_vMembers.size();
			for (u32 i = 0; i < numMembers; ++i)
			{
				INetworkMemberPtr pMember = m_vMembers[i];
				if ((pMember->UpdateOrder == eNMUO_Nub) && (!pMember->IsDead()) && (pMember->IsEstablishingContext()))
				{
					isEstablishing = true;
					break;
				}
			}

			if (isEstablishing)
			{
				waitTime = CNetCVars::Get().socketBoostTimeout;
			}
			else
			{
				waitTime = CNetCVars::Get().socketMaxTimeout;
			}
		}
		else
		{
			waitTime = CNetCVars::Get().socketMaxTimeout;
		}
	}
#endif
	if (!mt)
	{
		waitTime = 0;
	}

#if LOG_SOCKET_POLL_TIME
	float milliseconds = 0;
	float timeTaken = 0;
#endif // LOG_SOCKET_POLL_TIME

	while (shouldPoll)
	{
		// If we're *not* multiplayer, we'll only poll once
		shouldPoll &= (gEnv->bMultiplayer && !gEnv->IsEditor());
#if LOG_SOCKET_POLL_TIME
		timeTaken = -gEnv->pTimer->GetAsyncCurTime();
#endif // LOG_SOCKET_POLL_TIME

		{
			DRX_PROFILE_REGION_WAITING(PROFILE_NETWORK, "Wait - Network Poll Wait Internal");

			// Is there any work to do?
			NET_STOP_THREAD_TIMER();
			internalWorkToDo = m_pInternalSocketIOUpr->PollWait(waitTime);
			NET_START_THREAD_TIMER();
			if (internalWorkToDo == false)
			{
				// PollWait timed out
				if (shouldPoll)
				{
					// We're multiplayer so we should attempt to tick again unless we've been a very long time
					if (timeout == 0)
					{
						ret = eTRS_TimedOut;
						shouldPoll = false;
					}
					else
					{
						ret = eTRS_Continue;
						timeout--;
					}
				}
				else
				{
					// We're single player or editor, so we only want to tick once
					ret = eTRS_Continue;
					shouldPoll = false;
				}
			}
		}

		if (m_pInternalSocketIOUpr != m_pExternalSocketIOUpr)
		{
			DRX_PROFILE_REGION_WAITING(PROFILE_NETWORK, "Wait - Network Poll Wait External");

			externalWorkToDo = m_pExternalSocketIOUpr->PollWait(0);
		}

		if (internalWorkToDo || externalWorkToDo)
		{
			{
				DRX_PROFILE_REGION_WAITING(PROFILE_NETWORK, "Wait - Network DoMainTick PollWork lock");

				m_mutex.Lock();
			}

			if (internalWorkToDo)
			{
				DRX_PROFILE_REGION(PROFILE_NETWORK, "Network DoMainTick PollWork Internal");

				// The internal socket manager is the one that handles waits/sleeps
				pollres = m_pInternalSocketIOUpr->PollWork(didWorkInInternalPoll);
			}
			if (externalWorkToDo)
			{
				if (pollres != ISocketIOUpr::eUM_SLEEP || !(gEnv->bMultiplayer && !gEnv->IsEditor()))
				{
					DRX_PROFILE_REGION(PROFILE_NETWORK, "Network DoMainTick PollWork External");

					m_pExternalSocketIOUpr->PollWork(didWorkInExternalPoll);
				}
				else
				{
					m_bDelayedExternalWork = true;
				}
			}

			m_mutex.Unlock();

			if (pollres <= ISocketIOUpr::eEM_UNSPECIFIED)
				;
			else
				switch (pollres)
				{
				case ISocketIOUpr::eUM_QUIT:
					ret = eTRS_Quit;
				case ISocketIOUpr::eUM_SLEEP:
					shouldPoll = false;
					break;
				case 0: // Is 0 legitimate?  Need to investigate...
				case ISocketIOUpr::eUM_WAKE:
				case ISocketIOUpr::eSM_COMPLETEDIO:
				case ISocketIOUpr::eSM_COMPLETE_SUCCESS_OK:
				case ISocketIOUpr::eSM_COMPLETE_EMPTY_SUCCESS_OK:
				case ISocketIOUpr::eSM_COMPLETE_FAILURE_OK:
					break;
				default:
					NetWarning("Unhandled poll result %d", pollres);
					break;
				}

#if LOG_SOCKET_POLL_TIME
			timeTaken += gEnv->pTimer->GetAsyncCurTime();
			milliseconds = timeTaken * 1000.0f;
			NetLog("[POLL]: poll time %fms", milliseconds);
#endif // LOG_SOCKET_POLL_TIME
		}
	}

	m_forceLock = !(didWorkInExternalPoll | didWorkInInternalPoll);

	return ret;
}

namespace
{
struct CompareMembers
{
	bool operator()(INetworkMemberPtr p1, INetworkMemberPtr p2) const
	{
		return p1->UpdateOrder < p2->UpdateOrder;
	}
};
}

void CNetwork::AddMember(INetworkMemberPtr pMember)
{
	m_vMembers.push_back(pMember);
	std::stable_sort(m_vMembers.begin(), m_vMembers.end(), CompareMembers());
}

tukk CNetwork::GetHostName()
{
	static char buf[256];
#if DRX_PLATFORM_ORBIS
	drx_strcpy(buf, "Orbis");
#else
	DrxSock::gethostname(buf, 256);
#endif
	return buf;
}

void CNetwork::LogNetworkInfo()
{
#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE

	u32 i;
	char buf[256];
	DRXHOSTENT* hp;

	if (!gethostname(buf, sizeof(buf)))
	{
		PREFAST_SUPPRESS_WARNING(4996);
		hp = DrxSock::gethostbyname(buf);
		if (hp)
		{
			DrxLogAlways("network hostname: %s", hp->h_name);

			i = 0;

			while (hp->h_aliases[i])
			{
				DrxLogAlways("  alias: %s\n", hp->h_aliases[i]);
				i++;
			}

			i = 0;

			while (hp->h_addr_list[i])
			{
				DRXSOCKADDR_IN temp;

				memcpy(&(temp.sin_addr), hp->h_addr_list[i], hp->h_length);

	#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
				const in_addr_windows* pin_addr_win = reinterpret_cast<const in_addr_windows*>(&temp.sin_addr);
				DrxLog("  ip:%d.%d.%d.%d",    //  port:%d  family:%x",
				       (i32)(pin_addr_win->S_un.S_un_b.s_b1),
				       (i32)(pin_addr_win->S_un.S_un_b.s_b2),
				       (i32)(pin_addr_win->S_un.S_un_b.s_b3),
				       (i32)(pin_addr_win->S_un.S_un_b.s_b4));
	#else
				DrxLogAlways("  ip:%d.%d.%d.%d",    //  port:%d  family:%x",
				             (i32)(temp.sin_addr.S_un.S_un_b.s_b1),
				             (i32)(temp.sin_addr.S_un.S_un_b.s_b2),
				             (i32)(temp.sin_addr.S_un.S_un_b.s_b3),
				             (i32)(temp.sin_addr.S_un.S_un_b.s_b4));
				//		(i32)temp.sin_port,(u32)temp.sin_family);
	#endif
				i++;
			}
		}
	}
#endif
}

tukk GetTimestampString()
{

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	static char buffer[512];
	struct tm st;
	struct timeval now;
	gettimeofday(&now, NULL);
	if (gmtime_r(&now.tv_sec, &st) != NULL)
	{
		drx_sprintf(buffer, "%.2d:%.2d:%.2d.%.3d", st.tm_hour, st.tm_min, st.tm_sec, (i32)(now.tv_usec + 500) / 1000);
		return buffer;
	}
	else
	{
		return NULL;
	}
#elif !DRX_PLATFORM_WINDOWS
	return "";
#else
	SYSTEMTIME st;
	GetSystemTime(&st);
	static char buffer[512];
	drx_sprintf(buffer, "%.2d:%.2d:%.2d.%.3d", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	return buffer;
#endif

}

INetworkServicePtr CNetwork::GetService(tukk name)
{
	if (m_pServiceUpr.get())
		return m_pServiceUpr->GetService(name);
	return NULL;
}

IRemoteControlSystem* CNetwork::GetRemoteControlSystemSingleton()
{
	return &CRemoteControlSystem::GetSingleton();
}

ISimpleHttpServer* CNetwork::GetSimpleHttpServerSingleton()
{
	return &CSimpleHttpServer::GetSingleton();
}

IDrxLobby* CNetwork::GetLobby()
{
	return gEnv->pLobby;
}

void CNetwork::SetNetGameInfo(SNetGameInfo info)
{
	SCOPED_GLOBAL_LOCK;
	m_gameInfo = info;
}

SNetGameInfo CNetwork::GetNetGameInfo()
{
	ASSERT_GLOBAL_LOCK;
	return m_gameInfo;
}

i32 CNetwork::RegisterForFastLookup(CNetChannel* pChannel)
{
	size_t i;
	for (i = 0; i < m_vFastChannelLookup.size(); i++)
	{
		if (!m_vFastChannelLookup[i])
		{
			m_vFastChannelLookup[i] = pChannel;
			return i;
		}
	}
	m_vFastChannelLookup.push_back(pChannel);
	return i;
}

void CNetwork::UnregisterFastLookup(i32 id)
{
	NET_ASSERT(id >= 0);
	NET_ASSERT(id < (i32)m_vFastChannelLookup.size());
	NET_ASSERT(m_vFastChannelLookup[id]);
	m_vFastChannelLookup[id] = 0;
}

CNetChannel* CNetwork::GetChannelByFastLookupId(i32 id)
{
	NET_ASSERT(id >= 0);
	if (id < (i32)m_vFastChannelLookup.size())
		return m_vFastChannelLookup[id];
	else
		return 0;
}

void CEnsureRealtime::Failed()
{
	__debugbreak();
	NET_ASSERT(!"REALTIME");
}

CNetChannel* CNetwork::FindFirstClientChannel()
{
	for (VMembers::const_iterator iter = m_vMembers.begin(); iter != m_vMembers.end(); ++iter)
		if (CNetChannel* pChan = (*iter)->FindFirstClientChannel())
			return pChan;
	return 0;
}

CNetChannel* CNetwork::FindFirstRemoteChannel()
{
	for (VMembers::const_iterator iter = m_vMembers.begin(); iter != m_vMembers.end(); ++iter)
	{
		if (CNetChannel* pChan = (*iter)->FindFirstRemoteChannel())
		{
			return pChan;
		}
	}

	return NULL;
}

// NOTE: the following functions cannot call isPbClSvEnabled() function directly, since they tend to
// load PB dll's if not loaded, and that's not something we want (what we are doing is load PB dll
// only during PB enabled multiplayer sessions)

bool CNetwork::IsPbClEnabled()
{
	return m_isPbClActive;
}

bool CNetwork::IsPbSvEnabled()
{
	return m_isPbSvActive;
}

void CNetwork::StartupPunkBuster(bool server)
{
#ifdef __WITH_PB__
	PBsdk_SetPointers();

	if (server)
	{
		PbServerInitialize();
		EnablePbSv();
		m_isPbSvActive = true;
	}
	else
	{
		PbClientInitialize(NULL);
		EnablePbCl();
		m_isPbClActive = true;
	}
#endif
}

void CNetwork::CleanupPunkBuster()
{
#ifdef __WITH_PB__
	PbShutdown();
	m_isPbSvActive = false;
	m_isPbClActive = false;
#endif
}

bool CNetwork::IsPbInstalled()
{
#ifdef __WITH_PB__
	return isPbInstalled() != 0;
#else
	return false;
#endif
}

#if NET_PROFILE_ENABLE
void CNetwork::NpCountReadBits(bool count)
{
	return netProfileCountReadBits(count);
}

bool CNetwork::NpGetChildFromCurrent(tukk name, SNetProfileStackEntry** entry, bool rmi)
{
	return netProfileGetChildFromCurrent(name, entry, rmi);
}

void CNetwork::NpRegisterBeginCall(tukk name, SNetProfileStackEntry** entry, float budget, bool rmi)
{
	netProfileRegisterBeginCall(name, entry, budget, rmi);
}

void CNetwork::NpBeginFunction(SNetProfileStackEntry* entry, bool read)
{
	netProfileBeginFunction(entry, read);
}

void CNetwork::NpEndFunction()
{
	netProfileEndFunction();
}

bool CNetwork::NpIsInitialised()
{
	return netProfileIsInitialised();
}

SNetProfileStackEntry* CNetwork::NpGetNullProfile()
{
	return netProfileGetNullProfile();
}
#else
void                   CNetwork::NpCountReadBits(bool count)                                                                  {}
bool                   CNetwork::NpGetChildFromCurrent(tukk name, SNetProfileStackEntry** entry, bool rmi)             { return false; }
void                   CNetwork::NpRegisterBeginCall(tukk name, SNetProfileStackEntry** entry, float budget, bool rmi) {}
void                   CNetwork::NpBeginFunction(SNetProfileStackEntry* entry, bool read)                                     {}
void                   CNetwork::NpEndFunction()                                                                              {}
bool                   CNetwork::NpIsInitialised()                                                                            { return false; }
SNetProfileStackEntry* CNetwork::NpGetNullProfile()                                                                           { return NULL; }
#endif

void CNetwork::GetBandwidthStatistics(SBandwidthStats* const pStats)
{
	DRX_ASSERT(pStats);

#if NET_MINI_PROFILE || NET_PROFILE_ENABLE
	memcpy(pStats, &g_socketBandwidth.bandwidthStats, sizeof(SBandwidthStats));

	u32 channelIndex = 0;
	while (channelIndex < STATS_MAX_NUMBER_OF_CHANNELS)
	{
		SNetChannelStats& stats = pStats->m_channel[channelIndex];
		stats.m_inUse = false;

		++channelIndex;
	}
	pStats->m_numChannels = 0;

	for (size_t i = 0; i < m_vMembers.size(); i++)
	{
		INetworkMemberPtr pMember = m_vMembers[i];
		pMember->GetBandwidthStatistics(pStats);
	}
#endif
}

void CNetwork::GetProfilingStatistics(SNetworkProfilingStats* const pStats)
{

#ifdef ENABLE_PROFILING_CODE
	for (size_t i = 0; i < m_vMembers.size(); i++)
	{
		INetworkMemberPtr pMember = m_vMembers[i];
		pMember->GetProfilingStatistics(pStats);
	}
#endif // #ifdef ENABLE_PROFILING_CODE

#if INTERNET_SIMULATOR
	pStats->m_InternetSimulatorStats.m_packetSends = g_socketBandwidth.simPacketSends;
	pStats->m_InternetSimulatorStats.m_packetDrops = g_socketBandwidth.simPacketDrops;
	pStats->m_InternetSimulatorStats.m_lastPacketLag = g_socketBandwidth.simLastPacketLag;
#endif

#if NET_PROFILE_ENABLE
	pStats->m_ProfileInfoStats.clear();
	NET_PROFILE_LEAF_LIST(pStats->m_ProfileInfoStats);
#endif
}

bool CNetwork::ConvertAddr(const TNetAddress& addrIn, DRXSOCKADDR_IN* pSockAddr)
{
	return ::ConvertAddr(addrIn, pSockAddr);
}

bool CNetwork::ConvertAddr(const TNetAddress& addrIn, DRXSOCKADDR* pSockAddr, i32* addrLen)
{
	return ::ConvertAddr(addrIn, pSockAddr, addrLen);
}

/////////////////////////////////////////////////////////////////////////////
// Host Migration
void CNetwork::EnableHostMigration(bool bEnabled)
{
}

bool CNetwork::IsHostMigrationEnabled(void)
{
	return false;
}

void CNetwork::TerminateHostMigration(DrxSessionHandle gh)
{
}

void CNetwork::AddHostMigrationEventListener(IHostMigrationEventListener* pListener, tukk pWho, EListenerPriorityType priority)
{
	IDrxLobby* pLobby = GetLobby();

	if (pLobby)
	{
		pLobby->AddHostMigrationEventListener(pListener, pWho, priority);
	}
}

void CNetwork::RemoveHostMigrationEventListener(IHostMigrationEventListener* pListener)
{
	IDrxLobby* pLobby = GetLobby();

	if (pLobby)
	{
		pLobby->RemoveHostMigrationEventListener(pListener);
	}
}

/////////////////////////////////////////////////////////////////////////////

// Expose encryption method for game to use on files as necessary.

void CNetwork::EncryptBuffer(u8* pOutput, u8k* pInput, u32 bufferLength, u8k* pKey, u32 keyLength)
{
	if (pKey && (keyLength > 0))
	{
		CStreamCipher cipher;
		cipher.Init(pKey, keyLength);

		if (pInput && pOutput && (bufferLength > 0))
		{
			cipher.Encrypt(pInput, bufferLength, pOutput);
		}
	}
}

void CNetwork::DecryptBuffer(u8* pOutput, u8k* pInput, u32 bufferLength, u8k* pKey, u32 keyLength)
{
	if (pKey && (keyLength > 0))
	{
		CStreamCipher cipher;
		cipher.Init(pKey, keyLength);

		if (pInput && pOutput && (bufferLength > 0))
		{
			cipher.Decrypt(pInput, bufferLength, pOutput);
		}
	}
}

TCipher CNetwork::BeginCipher(u8k* pKey, u32 keyLength)
{
	CStreamCipher* cipher = new CStreamCipher;
	if (cipher)
		cipher->Init(pKey, keyLength);
	return cipher;
}

void CNetwork::Encrypt(TCipher cipher, u8* pOutput, u8k* pInput, u32 bufferLength)
{
	if (cipher && pInput && pOutput && (bufferLength > 0))
	{
		static_cast<CStreamCipher*>(cipher)->EncryptStream(pInput, bufferLength, pOutput);
	}
}

void CNetwork::Decrypt(TCipher cipher, u8* pOutput, u8k* pInput, u32 bufferLength)
{
	if (cipher && pInput && pOutput && (bufferLength > 0))
	{
		static_cast<CStreamCipher*>(cipher)->DecryptStream(pInput, bufferLength, pOutput);
	}
}

void CNetwork::EndCipher(TCipher cipher)
{
	delete static_cast<CStreamCipher*>(cipher);
}

void CNetwork::EncryptBuffer(TCipher cipher, u8* pOutput, u8k* pInput, u32 bufferLength)
{
	if (cipher && pInput && pOutput && (bufferLength > 0))
	{
		static_cast<CStreamCipher*>(cipher)->Encrypt(pInput, bufferLength, pOutput);
	}
}

void CNetwork::DecryptBuffer(TCipher cipher, u8* pOutput, u8k* pInput, u32 bufferLength)
{
	if (cipher && pInput && pOutput && (bufferLength > 0))
	{
		static_cast<CStreamCipher*>(cipher)->Decrypt(pInput, bufferLength, pOutput);
	}
}

u32 CNetwork::RijndaelEncryptBuffer(u8* pOutput, u8k* pInput, u32 bufferLength, u8k* pKey, u32 keyLength)
{
	Rijndael rin;
	Rijndael::KeyLength keyLen;

	if (keyLength >= 32)
	{
		keyLen = Rijndael::Key32Bytes;
	}
	else if (keyLength >= 24)
	{
		keyLen = Rijndael::Key24Bytes;
	}
	else if (keyLength >= 16)
	{
		keyLen = Rijndael::Key16Bytes;
	}
	else
	{
		return 0;
	}

	if (pKey && pInput && pOutput && (bufferLength > 0))
	{
		rin.init(Rijndael::CBC, Rijndael::Encrypt, pKey, keyLen);
		i32 bitsEncrypted = rin.blockEncrypt(pInput, bufferLength * 8, pOutput);

		if (bitsEncrypted > 0)
		{
			return bitsEncrypted / 8;
		}
	}

	return 0;
}

u32 CNetwork::RijndaelDecryptBuffer(u8* pOutput, u8k* pInput, u32 bufferLength, u8k* pKey, u32 keyLength)
{
	Rijndael rin;
	Rijndael::KeyLength keyLen;

	if (keyLength >= 32)
	{
		keyLen = Rijndael::Key32Bytes;
	}
	else if (keyLength >= 24)
	{
		keyLen = Rijndael::Key24Bytes;
	}
	else if (keyLength >= 16)
	{
		keyLen = Rijndael::Key16Bytes;
	}
	else
	{
		return 0;
	}

	if (pKey && pInput && pOutput && (bufferLength > 0))
	{
		rin.init(Rijndael::CBC, Rijndael::Decrypt, pKey, keyLen);
		i32 bitsDecrypted = rin.blockDecrypt(pInput, bufferLength * 8, pOutput);

		if (bitsDecrypted > 0)
		{
			return bitsDecrypted / 8;
		}
	}

	return 0;
}

TMemHdl CNetwork::MemAlloc(size_t sz)
{
	SCOPED_GLOBAL_LOCK;

	if (m_pMMM)
	{
		return m_pMMM->AllocHdl(sz);
	}

	return TMemInvalidHdl;
}

void CNetwork::MemFree(TMemHdl h)
{
	SCOPED_GLOBAL_LOCK;

	if (m_pMMM)
	{
		m_pMMM->FreeHdl(h);
	}
}

uk CNetwork::MemGetPtr(TMemHdl h)
{
	if (m_pMMM)
	{
		return m_pMMM->PinHdl(h);
	}

	return NULL;
}

#ifdef DEDICATED_SERVER

IDatagramSocketPtr CNetwork::GetDedicatedServerSchedulerSocket()
{
	u16 port = CNetCVars::Get().net_dedi_scheduler_client_base_port;

	while (!m_pDedicatedServerSchedulerSocket)
	{
		m_pDedicatedServerSchedulerSocket = GetExternalSocketIOUpr().CreateDatagramSocket(TNetAddress(SIPv4Addr(htonl(DrxSock::inet_addr("127.0.0.1")), port++)), eSF_StrictAddress);
	}

	return m_pDedicatedServerSchedulerSocket;
}

#endif

#ifdef NET_THREAD_TIMING

void CNetwork::ThreadTimerStart()
{
	assert(m_threadTimeDepth > 0);
	m_threadTimeDepth--;
	if (m_threadTimeDepth == 0)
		m_threadTimeCur -= gEnv->pTimer->GetAsyncTime();
}

void CNetwork::ThreadTimerStop()
{
	assert(m_threadTimeDepth >= 0);
	if (m_threadTimeDepth == 0)
		m_threadTimeCur += gEnv->pTimer->GetAsyncTime();
	m_threadTimeDepth++;
}

void CNetwork::ThreadTimerTick()
{
	assert(m_threadTimeDepth == 1); // 1 stop
	m_threadTime = m_threadTimeCur.GetSeconds();
	m_threadTimeCur = -gEnv->pTimer->GetAsyncTime();
	m_threadTimeDepth = 0;
}

#endif // NET_THREAD_TIMING
