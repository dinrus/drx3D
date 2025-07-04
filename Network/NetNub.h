// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  implements a point to which clients can be connected
   -------------------------------------------------------------------------
   История:
   - 26/07/2004   : Created by Craig Tiller
*************************************************************************/
#ifndef __NETNUB_H__
#define __NETNUB_H__

#pragma once

#include <drx3D/Network/INetwork.h>
#include <drx3D/Network/Network.h>
#include <drx3D/Network/IDatagramSocket.h>
#include <drx3D/Network/INetworkMember.h>
#include <drx3D/Network/INubMember.h>
#include <drx3D/Network/NetTimer.h>
#include <vector>
#include <drx3D/Network/Config.h>
#include <drx3D/CoreX/Lobby/CommonIDrxLobby.h>
#include <drx3D/CoreX/Lobby/CommonIDrxMatchMaking.h>

class CNetChannel;
class CNetwork;
class CDrxLobby;
struct INatNeg;
struct IServerReport;

enum EChannelLookupFlags
{
	eCLF_HandleBrokenWindowsICMP = 1,
};

class CNetNub : public INetNubPrivate, public INetworkMember, IDatagramListener
{
public:
	class CNubConnectingLock;

	CNetNub(
	  const TNetAddress& addr,
	  IGameNub* pGameNub,
	  IGameSecurity* pNetSecurity,
	  IGameQuery* pGameQuery);
	~CNetNub();

	// INetNub
	virtual void                        DeleteNub();
	virtual bool                        ConnectTo(tukk address, tukk connectionString);
	virtual const INetNub::SStatistics& GetStatistics();
	virtual bool                        IsConnecting();
	virtual void                        DisconnectPlayer(EDisconnectionCause cause, EntityId plr_id, tukk reason);
	virtual i32                         GetNumChannels() { return i32(m_channels.size()); }
	// ~INetNub

	virtual bool HasPendingConnections();

	// INetworkMember
	//virtual void Update( CTimeValue blocking );
	virtual bool         IsDead() { return m_bDead && 0 == m_keepAliveLocks && m_channels.empty(); }
	virtual bool         IsSuicidal();
	virtual void         GetMemoryStatistics(IDrxSizer* pSizer);
	virtual void         NetDump(ENetDumpType type);
	void                 NetDump(ENetDumpType type, INetDumpLogger& logger);
	virtual void         PerformRegularCleanup();
	virtual void         SyncWithGame(ENetworkGameSync type);
	virtual void         GetBandwidthStatistics(SBandwidthStats* const pStats);
	virtual CNetChannel* FindFirstClientChannel();
	virtual CNetChannel* FindFirstRemoteChannel();
	//#if LOCK_NETWORK_FREQUENCY
	virtual void         TickChannels(CTimeValue& now, bool force);
	//#endif
	virtual bool         IsEstablishingContext() const;
	// ~INetworkMember

	// IDatagramListener
	virtual void OnPacket(const TNetAddress& addr, u8k* pData, u32 nLength);
	virtual void OnError(const TNetAddress& addr, ESocketError error);
	// ~IDatagramListener

	// DinrusXNetwork-only code
	bool                 Init(CNetwork* pNetwork);
	bool                 SendTo(u8k* pData, size_t nSize, const TNetAddress& to);
	CNetwork*            GetNetwork()  { return m_pNetwork; }
	IGameSecurity*       GetSecurity() { return m_pSecurity; }
	void                 DisconnectChannel(EDisconnectionCause cause, const TNetAddress* pFrom, CNetChannel* pChannel, tukk reason);
	virtual void         DisconnectChannel(EDisconnectionCause cause, DrxSessionHandle session, tukk reason);
	virtual INetChannel* GetChannelFromSessionHandle(DrxSessionHandle session);

	void                 RegisterBackoffAddress(const TNetAddress& addr)
	{
		m_pSocketMain->RegisterBackoffAddress(addr);
	}
	void UnregisterBackoffAddress(const TNetAddress& addr)
	{
		m_pSocketMain->UnregisterBackoffAddress(addr);
	}

#if NEW_BANDWIDTH_MANAGEMENT
	void LogChannelPerformanceMetrics(u32 id) const;
	void SetChannelPerformanceMetrics(u32 id, INetChannel::SPerformanceMetrics& metrics);
#endif // NEW_BANDWIDTH_MANAGEMENT

	void GetLocalIPs(TNetAddressVec& vIPs)
	{
		m_pSocketMain->GetSocketAddresses(vIPs);
	}

	i32 GetSysSocket()
	{
		return (i32)m_pSocketMain->GetSysSocket();
	}

	void      DoConnectTo(const TNetAddressVec& ips, DrxSessionHandle session, string connectionString, CNubConnectingLock conlk);

	void      GN_Lazy_ContinueConnectTo(CNameRequestPtr pReq, DrxSessionHandle session, string connectionString, CNubConnectingLock conlk);
	void      GN_Loop_ContinueConnectTo(CNameRequestPtr pReq, DrxSessionHandle session, string connectionString, CNubConnectingLock conlk);

	IGameNub* GetGameNub() { return m_pGameNub; }
#if NEW_BANDWIDTH_MANAGEMENT
	u32    GetTotalBandwidthShares(void) const;
#endif // NEW_BANDWIDTH_MANAGEMENT

private:
	void         ProcessPacketFrom(const TNetAddress& from, u8k* pData, u32 nLength);
	void         ProcessErrorFrom(ESocketError err, const TNetAddress& from);
	bool         ProcessQueryPacketFrom(const TNetAddress& from, u8k* pData, u32 nLength);
	bool         ProcessConnectionPacketFrom(const TNetAddress& from, u8k* pData, u32 nLength);
	bool         ProcessTransportPacketFrom(const TNetAddress& from, u8k* pData, u32 nLength);

	void         ProcessSetup(const TNetAddress& from, u8k* pData, u32 nLength);

	void         ProcessLanQuery(const TNetAddress& from);
	bool         ProcessPingQuery(const TNetAddress& from, u8k* pData, u32 nLength);
	void         ProcessDisconnect(const TNetAddress& from, u8k* pData, u32 nLength);
	void         ProcessDisconnectAck(const TNetAddress& from);
	void         ProcessAlreadyConnecting(const TNetAddress& from, u8k* pData, u32 nLength);
	void         ProcessSimplePacket(const TNetAddress& from, u8k* pData, u32 nLength);

	void         ProcessKeyExchange0(const TNetAddress& from, u8k* pData, u32 nLength);
	void         ProcessKeyExchange1(const TNetAddress& from, u8k* pData, u32 nLength);

	CNetChannel* GetChannelForIP(const TNetAddress& ip, u32 flags);
	TNetAddress  GetIPForChannel(const INetChannel*);
	static void TimerCallback(NetTimerId, uk , CTimeValue);

	void CreateChannel(const TNetAddress& ip, const CExponentialKeyExchange::KEY_TYPE& key, const string& connectionString, u32 remoteSocketCaps, u32 proifle, DrxSessionHandle session, CNubConnectingLock conlk); // first half, network thread

	void GC_CreateChannel(CNetChannel* pNetChannel, string connectionString, CNubConnectingLock conlk);   // game thread

	typedef VectorMap<TNetAddress, INubMemberPtr> TChannelMap;

	// someone that we're disconnecting
	struct SDisconnect
	{
		CTimeValue          when;
		CTimeValue          lastNotify;
		EDisconnectionCause cause;
		char                info[256];
		size_t              infoLength;
		char                backlog[1024]; // NOTE: the disconnect message will be sent within one UDP packet, our network system disallows IP fragmentation, so packets larger than the local interface MTU get discoarded
		size_t              backlogLen;
	};

	enum EKeyExchangeState
	{
		eKES_NotStarted,      // a NULL state
		eKES_SetupInitiated,  // result of sending eH_ConnectionSetup, waiting for eH_ConnectionSetup_KeyExchange_0
		eKES_SentKeyExchange, // result of sending eH_ConnectionSetup_KeyExchange_0, waiting for eH_ConnectionSetup_KeyExchange_1
		eKES_KeyEstablished   // result of received eH_ConnectionSetup_KeyExchange_0, and sent eH_ConnectionSetup_KeyExchange_1 (eKES_SetupInitiated)
		                      // or received eH_ConnectionSetup_KeyExchange_1 (eKES_SentKeyExchange)
	};

	struct SKeyExchangeStuff
	{
		SKeyExchangeStuff() : kes(eKES_NotStarted), kesStart(0.0f) {}
		EKeyExchangeState                 kes;
#if ALLOW_ENCRYPTION
		CExponentialKeyExchange           exchange;
		CExponentialKeyExchange::KEY_TYPE B, g, p, A;
#endif

		CTimeValue kesStart; // key exchange state start time, if staying in the state too long, triggers a state timeout
	};

public:
	class CNubConnectingLock
	{
	public:
		CNubConnectingLock() : m_pNub(0) {}
		CNubConnectingLock(CNetNub* pNub) : m_pNub(pNub)
		{
			if (m_pNub)
				m_pNub->LockConnecting();
		}
		CNubConnectingLock(const CNubConnectingLock& lk) : m_pNub(lk.m_pNub)
		{
			if (m_pNub)
				m_pNub->LockConnecting();
		}
		~CNubConnectingLock()
		{
			if (m_pNub)
				m_pNub->UnlockConnecting();
		}
		void Swap(CNubConnectingLock& lk)
		{
			std::swap(m_pNub, lk.m_pNub);
		}
		CNubConnectingLock& operator=(CNubConnectingLock lk)
		{
			Swap(lk);
			return *this;
		}

	private:
		CNetNub* m_pNub;
	};

	class CNubKeepAliveLock
	{
	public:
		CNubKeepAliveLock() : m_pNub(0) {}
		CNubKeepAliveLock(CNetNub* pNub) : m_pNub(pNub)
		{
			if (m_pNub)
				m_pNub->m_keepAliveLocks++;
		}
		CNubKeepAliveLock(const CNubKeepAliveLock& lk) : m_pNub(lk.m_pNub)
		{
			if (m_pNub)
				m_pNub->m_keepAliveLocks++;
		}
		~CNubKeepAliveLock()
		{
			if (m_pNub)
				m_pNub->m_keepAliveLocks--;
		}
		void Swap(CNubKeepAliveLock& lk)
		{
			std::swap(m_pNub, lk.m_pNub);
		}
		CNubKeepAliveLock& operator=(CNubKeepAliveLock lk)
		{
			Swap(lk);
			return *this;
		}

	private:
		CNetNub* m_pNub;
	};

	void GC_FailedActiveConnect(EDisconnectionCause cause, string description, CNubKeepAliveLock); // game thread

private:
	// someone we're trying to connect to
	struct SPendingConnection : public SKeyExchangeStuff
	{
		SPendingConnection() : pChannel(0), lastSend(0.0f), connectCounter(0), profile(0) {}

		string                  connectionString;
		TNetAddress             to;
		TNetAddressVec          tos;
		_smart_ptr<CNetChannel> pChannel;
		CTimeValue              lastSend;
		i32                     connectCounter;
		u32                  profile;
		CNubConnectingLock      conlk;
		u32                  socketCaps;
		DrxSessionHandle        session;
	};

	// someone we're currently connecting with (waiting for game)
	struct SConnecting : public SKeyExchangeStuff
	{
		SConnecting() : lastNotify(0.0f) {}

		string           connectionString; // save the connection string for later use
		u32           socketCaps;
		u32           profile; //connecting user profile
		CTimeValue       lastNotify;
		DrxSessionHandle session;
	};

	void SendDisconnect(const TNetAddress& to, SDisconnect& dis);
	void AckDisconnect(const TNetAddress& to);

	void LobbySafeDisconnect(CNetChannel* pChannel, EDisconnectionCause cause, tukk reason);

	typedef std::map<TNetAddress, SDisconnect> TDisconnectMap;
	typedef std::set<TNetAddress>              TAckDisconnectSet;
	typedef std::map<TNetAddress, SConnecting> TConnectingMap;
	typedef std::vector<SPendingConnection>    TPendingConnectionSet;

	CNetwork*             m_pNetwork;
	IDrxLobby*            m_pLobby;
	SStatistics           m_statistics;
	string                m_hostName;
	TChannelMap           m_channels;
	IDatagramSocketPtr    m_pSocketMain;
	IGameSecurity*        m_pSecurity;
	IGameQuery*           m_pGameQuery;
	IGameNub*             m_pGameNub;
	bool                  m_bDead;
	TNetAddress           m_addr;
	TPendingConnectionSet m_pendingConnections;
	size_t                m_cleanupChannel;
	TDisconnectMap        m_disconnectMap;
	TAckDisconnectSet     m_ackDisconnectSet;
	TConnectingMap        m_connectingMap;
	NetTimerId            m_statsTimer;
	IServerReport*        m_serverReport;
	i32                   m_connectingLocks;
	i32                   m_keepAliveLocks;
	DrxCriticalSection    m_lockChannels;

	bool SendPendingConnect(SPendingConnection& pc);
	void SendConnecting(const TNetAddress& to, SConnecting& con);
	void AddDisconnectEntry(const TNetAddress& ip, DrxSessionHandle session, EDisconnectionCause cause, tukk reason);

	bool SendKeyExchange0(const TNetAddress& to, SConnecting& con, bool doNotRegenerate = false);
	bool SendKeyExchange1(SPendingConnection& pc, bool doNotRegenerate = false);

	void LockConnecting()
	{
		SCOPED_GLOBAL_LOCK;
		++m_connectingLocks;
	}
	void UnlockConnecting()
	{
		SCOPED_GLOBAL_LOCK;
		--m_connectingLocks;
	}
};

#endif
