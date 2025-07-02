// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __NETCVARS_H__
#define __NETCVARS_H__

#pragma once

#include <drx3D/Sys/IConsole.h>
#include <drx3D/Network/Config.h>
#include <drx3D/Sys/TimeValue.h>

class CNetCVars
{
public:
	i32   TokenId;
	i32   CPU;
	i32   LogLevel;
	i32   EnableVoiceChat;
	i32   ChannelStats;
	float BandwidthAggressiveness;
	float NetworkConnectivityDetectionInterval; // disabled if less than 1.0f
	float InactivityTimeout;
	float InactivityTimeoutDevmode;
	i32   BackoffTimeout;
	i32   MaxMemoryUsage;
	i32   EnableVoiceGroups;
#if !NEW_BANDWIDTH_MANAGEMENT
	i32   RTTConverge;
#endif // !NEW_BANDWIDTH_MANAGEMENT
	i32   EnableTFRC;
#if DRX_PLATFORM_DURANGO
	i32   networkThreadAffinity;
#endif

#if LOCK_NETWORK_FREQUENCY == 0
	float channelLocalSleepTime;
#endif // LOCK_NETWORK_FREQUENCY == 0
	i32   socketMaxTimeout;
	i32   socketBoostTimeout;
	i32   socketMaxTimeoutMultiplayer;

#if NEW_BANDWIDTH_MANAGEMENT
	float net_availableBandwidthServer;
	float net_availableBandwidthClient;
	i32   net_defaultBandwidthShares;
	i32   net_defaultPacketRate;
	i32   net_defaultPacketRateIdle;
#endif // NEW_BANDWIDTH_MANAGEMENT

#if NET_ASSERT_LOGGING
	i32 AssertLogging;
#endif

#if !NEW_BANDWIDTH_MANAGEMENT
	i32   PacketSendRate; // packets per second
#endif                  // !NEW_BANDWIDTH_MANAGEMENT
	i32   NewQueueBehaviour;
	i32   SafetySleeps;
	i32   MaxPacketSize;
	float KeepAliveTime;
	float PingTime;

#if LOG_MESSAGE_DROPS
	i32 LogDroppedMessagesVar;
	bool LogDroppedMessages()
	{
		return LogDroppedMessagesVar || LogLevel;
	}
#endif

#if ENABLE_NETWORK_MEM_INFO
	i32 MemInfo;
#endif
#if ENABLE_DEBUG_KIT
	i32 NetInspector;
	i32 UseCompression;
	i32 ShowObjLocks;
	i32 DebugObjUpdates;
	i32 LogComments;
	i32 EndpointPendingQueueLogging;
	i32 DisconnectOnUncollectedBreakage;
	i32 DebugConnectionState;
	i32 RandomPacketCorruption; // 0 - disabled; 1 - UDP level (final stage); 2 - CTPEndpoint (before signing and encryption)
	//float WMICheckInterval;
	i32 PerfCounters;
	i32 GSDebugOutput;
#endif

#if LOG_MESSAGE_QUEUE
	i32    net_logMessageQueue;
	ICVar* pnet_logMessageQueueInjectLabel;
#endif // LOG_SEND_QUEUE

#if ENABLE_NET_DEBUG_INFO
	i32 netDebugInfo;
	i32 netDebugChannelIndex;
#endif

#if ENABLE_NET_DEBUG_ENTITY_INFO
	i32    netDebugEntityInfo;
	ICVar* netDebugEntityInfoClassName;
#endif

	i32 net_lobbyUpdateFrequency;

#if !defined(OLD_VOICE_SYSTEM_DEPRECATED)
	i32   VoiceLeadPackets;
	i32   VoiceTrailPackets;
	float VoiceProximity;
#endif // !defined(OLD_VOICE_SYSTEM_DEPRECATED)
#if defined(USE_CD_KEYS)
	#if !ALWAYS_CHECK_CD_KEYS
	i32 CheckCDKeys;
	#endif
#endif
	ICVar* StatsLogin;
	ICVar* StatsPassword;

	i32    RankedServer;

	i32    LanScanPortFirst;
	i32    LanScanPortNum;
	float  MinTCPFriendlyBitRate;

#if !defined(OLD_VOICE_SYSTEM_DEPRECATED)
	i32 useDeprecatedVoiceSystem;
#endif // !defined(OLD_VOICE_SYSTEM_DEPRECATED)

	CTimeValue StallEndTime;

#if INTERNET_SIMULATOR

	#define PACKET_LOSS_MAX_CLAMP (100.0f)      //-- Packet loss clamped to maximum of 100%
	#define PACKET_LAG_MAX_CLAMP  (60.0f)       //-- Packet lag clamped to max 60 seconds (60 an arbitrary number to identify the value as 'seconds')

	float net_PacketLossRate;
	float net_PacketLagMin;
	float net_PacketLagMax;
	i32   net_SimUseProfile;

	static void OnPacketLossRateChange(ICVar* NewLossRate)
	{
		if (NewLossRate->GetFVal() < 0.0f)
			NewLossRate->Set(0.0f);
		else if (NewLossRate->GetFVal() > PACKET_LOSS_MAX_CLAMP)
			NewLossRate->Set(PACKET_LOSS_MAX_CLAMP);
	}

	static void OnPacketExtraLagChange(ICVar* NewExtraLag)
	{
		if (NewExtraLag->GetFVal() < 0.0f)
			NewExtraLag->Set(0.0f);
		else if (NewExtraLag->GetFVal() > PACKET_LAG_MAX_CLAMP)
			NewExtraLag->Set(PACKET_LAG_MAX_CLAMP);
	}

	static void OnInternetSimUseProfileChange(ICVar* val);
	static void OnInternetSimLoadProfiles(ICVar* val);
#endif

#ifdef ENABLE_UDP_PACKET_FRAGMENTATION
	i32   net_max_fragmented_packets_per_source;
	float net_fragment_expiration_time;
	float net_packetfragmentlossrate;
	static void OnPacketFragmentLossRateChange(ICVar* NewLossRate)
	{
		if (NewLossRate->GetFVal() < 0.0f)
			NewLossRate->Set(0.0f);
		else if (NewLossRate->GetFVal() > 1.0f)
			NewLossRate->Set(1.0f);
	}
#endif

	float HighLatencyThreshold; // disabled if less than 0.0f
	float HighLatencyTimeLimit;

#if ENABLE_DEBUG_KIT
	i32 VisWindow;
	i32 VisMode;
	i32 ShowPing;
#endif

#if !defined(OLD_VOICE_SYSTEM_DEPRECATED)
	ICVar* pVoiceCodec;
#endif // !defined(OLD_VOICE_SYSTEM_DEPRECATED)
	ICVar* pSchedulerDebug;
	i32    SchedulerDebugMode;
	float  DebugDrawScale;
	float  SchedulerSendExpirationPeriod;

#if LOG_INCOMING_MESSAGES || LOG_OUTGOING_MESSAGES
	i32 LogNetMessages;
#endif
#if LOG_BUFFER_UPDATES
	i32 LogBufferUpdates;
#endif
#if STATS_COLLECTOR_INTERACTIVE
	i32 ShowDataBits;
#endif

	i32 RemoteTimeEstimationWarning;

#if ENABLE_CORRUPT_PACKET_DUMP
	i32 packetReadDebugOutput;
	bool doingPacketReplay() { return packetReadDebugOutput ? true : false; }
#else
	bool doingPacketReplay() { return false; }
#endif

	// BREAKAGE
	i32 breakageSyncEntities;

	i32 enableWatchdogTimer;

	// Non arithstream NetID bits
	i32 net_numNetIDLowBitBits;
	i32 net_numNetIDLowBitIDs;
	i32 net_numNetIDMediumBitBits;
	i32 net_numNetIDMediumBitIDs;
	i32 net_netIDHighBitStart;
	i32 net_numNetIDHighBitBits;
	i32 net_numNetIDHighBitIDs;
	i32 net_numNetIDs;
	i32 net_invalidNetID;

	// Dedi server scheduler
	i32 net_dedi_scheduler_server_port;
	i32 net_dedi_scheduler_client_base_port;

	i32 net_profile_deep_bandwidth_logging;

	static ILINE CNetCVars& Get()
	{
		NET_ASSERT(s_pThis);
		return *s_pThis;
	}

private:
	friend class CNetwork; // Our only creator

	CNetCVars(); // singleton stuff
	~CNetCVars();
	CNetCVars(const CNetCVars&);
	CNetCVars& operator=(const CNetCVars&);

	static CNetCVars* s_pThis;

	static void DumpObjectState(IConsoleCmdArgs*);
	static void DumpBlockingRMIs(IConsoleCmdArgs*);
	static void Stall(IConsoleCmdArgs*);
	static void SetCDKey(IConsoleCmdArgs*);

#if NEW_BANDWIDTH_MANAGEMENT
	static void GetChannelPerformanceMetrics(IConsoleCmdArgs* pArguments);
	static void SetChannelPerformanceMetrics(IConsoleCmdArgs* pArguments);
#endif // NEW_BANDWIDTH_MANAGEMENT
};

#endif
