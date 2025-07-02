// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/ISocketIOUpr.h>
#include  <drx3D/Network/SocketIOUprIOCP.h>
#include  <drx3D/Network/SocketIOUprNull.h>
#include  <drx3D/Network/SocketIOUprSelect.h>
#include  <drx3D/Network/SocketIOUprLobbyIDAddr.h>
#if DRX_PLATFORM_DURANGO
	#include  <drx3D/Network/SocketIOUprDurango.h>
#endif

IDatagramSocketPtr CSocketIOUpr::CreateDatagramSocket(const TNetAddress& addr, u32 flags)
{
	SCOPED_GLOBAL_LOCK;

	TDatagramSockets::iterator socketIter;
	IDatagramSocketPtr pSocket;
	const SIPv4Addr* pIPv4Addr = stl::get_if<SIPv4Addr>(&addr);

	for (socketIter = m_datagramSockets.begin(); socketIter != m_datagramSockets.end(); ++socketIter)
	{
		const SIPv4Addr* pTestIPv4Addr = stl::get_if<SIPv4Addr>(&socketIter->addr);

		if ((socketIter->addr == addr) ||
		    (pIPv4Addr && pTestIPv4Addr && ((flags & eSF_StrictAddress) == 0) && ((flags & eSF_Online) == (socketIter->flags & eSF_Online))))
		{
			socketIter->refCount++;

			return socketIter->pSocket;
		}
	}

	pSocket = OpenSocket(addr, flags);

	if (pSocket)
	{
		SDatagramSocketData data;

		data.addr = addr;
		data.pSocket = pSocket;
		data.flags = flags;
		data.refCount = 1;

		m_datagramSockets.push_back(data);
	}

	return pSocket;
}

void CSocketIOUpr::FreeDatagramSocket(IDatagramSocketPtr pSocket)
{
	SCOPED_GLOBAL_LOCK;

	if (pSocket)
	{
		TDatagramSockets::iterator socketIter;

		for (socketIter = m_datagramSockets.begin(); socketIter != m_datagramSockets.end(); ++socketIter)
		{
			if (socketIter->pSocket == pSocket)
			{
				socketIter->refCount--;

				if (socketIter->refCount == 0)
				{
					m_datagramSockets.erase(socketIter);
				}

				return;
			}
		}
	}
}

#if NET_MINI_PROFILE || NET_PROFILE_ENABLE

void CSocketIOUpr::RecordPacketSendStatistics(u8k* pData, size_t len)
{
	u32 thisSend = (len + UDP_HEADER_SIZE) * 8;

	g_socketBandwidth.periodStats.m_totalBandwidthSent += thisSend;
	g_socketBandwidth.periodStats.m_totalPacketsSent++;
	g_socketBandwidth.bandwidthStats.m_total.m_totalBandwidthSent += thisSend;
	g_socketBandwidth.bandwidthStats.m_total.m_totalPacketsSent++;

	u8 headerType = Frame_HeaderToID[pData[0]];
	if (headerType == eH_DrxLobby)
	{
		g_socketBandwidth.periodStats.m_lobbyBandwidthSent += thisSend;
		g_socketBandwidth.periodStats.m_lobbyPacketsSent++;
		g_socketBandwidth.bandwidthStats.m_total.m_lobbyBandwidthSent += thisSend;
		g_socketBandwidth.bandwidthStats.m_total.m_lobbyPacketsSent++;
	}
	else
	{
		if (headerType == eH_Fragmentation)
		{
			g_socketBandwidth.periodStats.m_fragmentBandwidthSent += thisSend;
			g_socketBandwidth.periodStats.m_fragmentPacketsSent++;
			g_socketBandwidth.bandwidthStats.m_total.m_fragmentBandwidthSent += thisSend;
			g_socketBandwidth.bandwidthStats.m_total.m_fragmentPacketsSent++;
		}
		else
		{
			if ((headerType >= eH_TransportSeq0) && (headerType <= eH_SyncTransportSeq_Last))
			{
				g_socketBandwidth.periodStats.m_seqBandwidthSent += thisSend;
				g_socketBandwidth.periodStats.m_seqPacketsSent++;
				g_socketBandwidth.bandwidthStats.m_total.m_seqBandwidthSent += thisSend;
				g_socketBandwidth.bandwidthStats.m_total.m_seqPacketsSent++;
			}
		}
	}
}

#endif

bool CreateSocketIOUpr(i32 ncpus, ISocketIOUpr** ppExternal, ISocketIOUpr** ppInternal)
{
	bool created = false;

#if defined(HAS_SOCKETIOMANAGER_IOCP)
	if (!created)
	{
		if (ncpus >= 2 || gEnv->IsDedicated())
		{
			CSocketIOUprIOCP* pMgrIOCP = new CSocketIOUprIOCP();
			if ((pMgrIOCP != NULL) && (pMgrIOCP->Init() == true))
			{
				*ppInternal = pMgrIOCP;
				created = true;
			}
			else
			{
				created = false;
			}
		}
	}
#endif // defined(HAS_SOCKETIOMANAGER_IOCP)
#if defined(HAS_SOCKETIOMANAGER_DURANGO)
	if (!created)
	{
		CSocketIOUprDurango* pMgrDurango = new CSocketIOUprDurango();
		if ((pMgrDurango != NULL) && (pMgrDurango->Init() == true))
		{
			*ppInternal = pMgrDurango;
			created = true;
		}
		else
		{
			created = false;
		}
	}
#endif // defined(HAS_SOCKETIOMANAGER_DURANGO)
#if defined(HAS_SOCKETIOMANAGER_SELECT)
	if (!created)
	{
		CSocketIOUprSelect* pMgrSelect = new CSocketIOUprSelect();
		if ((pMgrSelect != NULL) && (pMgrSelect->Init() == true))
		{
			*ppInternal = pMgrSelect;
			created = true;
		}
		else
		{
			created = false;
		}
	}
#endif // defined(HAS_SOCKETIOMANAGER_SELECT)
	if (!created)
	{
		*ppInternal = new CSocketIOUprNull();
		if (*ppInternal != NULL)
		{
			created = true;
		}
	}

#if defined(HAS_SOCKETIOMANAGER_LOBBYIDADDR)
	if (created)
	{
		// If we created the internal socket manager OK, then create the external one
		CSocketIOUprLobbyIDAddr* pMgrLobbyAddrID = new CSocketIOUprLobbyIDAddr();
		if ((pMgrLobbyAddrID != NULL) && (pMgrLobbyAddrID->Init() == true))
		{
			*ppExternal = pMgrLobbyAddrID;
			created = true;
		}
		else
		{
			created = false;
		}
	}
#else
	// Games not using LobbyAddrID don't need the distinction between internal and external socket managers
	*ppExternal = *ppInternal;
#endif // defined(HAS_SOCKETIOMANAGER_LOBBYIDADDR)

	if (!created)
	{
		if (*ppExternal != NULL)
		{
			delete *ppExternal;
			*ppExternal = NULL;
		}

		if (*ppInternal != NULL)
		{
			delete *ppInternal;
			*ppInternal = NULL;
		}
	}

	return created;
}
