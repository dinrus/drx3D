// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/SocketIOUprSelect.h>
#include  <drx3D/Network/Network.h>
#include  <drx3D/Network/UDPDatagramSocket.h>

#if defined(HAS_SOCKETIOMANAGER_SELECT)

CSocketIOUprSelect::CSocketIOUprSelect() : CSocketIOUpr(eSIOMC_SupportsBackoff)
	#if LOCK_NETWORK_FREQUENCY
	, m_userMessageFrameID(0)
	#endif // LOCK_NETWORK_FREQUENCY
{
	if (CNetCVars::Get().enableWatchdogTimer)
	{
		m_pWatchdog = new CWatchdogTimer;
	}
	else
	{
		m_pWatchdog = NULL;
	}

	m_wakeupSocket = DRX_INVALID_SOCKET;
	m_wakeupSender = DRX_INVALID_SOCKET;
}

CSocketIOUprSelect::~CSocketIOUprSelect()
{
	if (m_wakeupSocket != DRX_INVALID_SOCKET)
	{
		DrxSock::closesocket(m_wakeupSocket);
	}
	if (m_wakeupSender != DRX_INVALID_SOCKET)
	{
		DrxSock::closesocket(m_wakeupSender);
	}

	for (size_t i = 0; i < m_socketInfo.size(); i++)
		delete m_socketInfo[i];

	if (m_pWatchdog)
	{
		delete m_pWatchdog;
	}
}

bool CSocketIOUprSelect::Init()
{
	class CAutoCloseSocket
	{
	public:
		CAutoCloseSocket(DRXSOCKET sock) : m_sock(sock) {}

		~CAutoCloseSocket()
		{
			if (m_sock != DRX_INVALID_SOCKET)
				DrxSock::closesocket(m_sock);
		}

		void Release()
		{
			m_sock = DRX_INVALID_SOCKET;
		}

	private:
		DRXSOCKET m_sock;
	};

	i32 i;
	for (i = 1025; i < 65536; i++)
	{
		if (i == 0xed17 || i == 0xfa57)
			continue;
		m_wakeupSocket = DrxSock::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (m_wakeupSocket == DRX_INVALID_SOCKET)
			return false;

		CAutoCloseSocket closer(m_wakeupSocket);
		memset(&m_wakeupAddr, 0, sizeof(m_wakeupAddr));

		m_wakeupAddr.sin_family = AF_INET;
		m_wakeupAddr.sin_port = htons(i);
		m_wakeupAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		if (DrxSock::bind(m_wakeupSocket, (const DRXSOCKADDR*)&m_wakeupAddr, sizeof(DRXSOCKADDR_IN)) != DRX_SOCKET_ERROR)
		{
			closer.Release();
			break;
		}
		else
		{
			tukk msg = CNetwork::Get()->EnumerateError(MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, GetLastError()));
			NetWarning("[net] socket error: %s", msg);
		}
	}

	if (i == 65536)
		return false;

	m_wakeupSender = DrxSock::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_wakeupSender == DRX_INVALID_SOCKET)
		return false;

	DRXSOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = 0;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (DrxSock::bind(m_wakeupSender, (const DRXSOCKADDR*)&saddr, sizeof(DRXSOCKADDR_IN)) == DRX_SOCKET_ERROR)
	{
		tukk msg = ((CNetwork*)(GetISystem()->GetINetwork()))->EnumerateError(MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, GetLastError()));
		NetWarning("[net] socket error: %s", msg);

		return false;
	}

	if (!MakeSocketNonBlocking(m_wakeupSender))
		return false;
	if (!MakeSocketNonBlocking(m_wakeupSocket))
		return false;

	return true;
}

static DRXSOCKET smax(DRXSOCKET a, DRXSOCKET b)
{
	if (a == DRX_INVALID_SOCKET)
		return b;
	if (b == DRX_INVALID_SOCKET)
		return a;
	if (a < b)
		return b;
	else
		return a;
}

bool CSocketIOUprSelect::PollWait(u32 waitTime)
{
	bool haveData = false;

	if (m_pWatchdog)
	{
		m_pWatchdog->ClearStalls();
	}

	m_fdmax = DRX_INVALID_SOCKET;

	FD_ZERO(&m_read);
	FD_ZERO(&m_write);

	FD_SET(m_wakeupSocket, &m_read);
	m_fdmax = smax(m_wakeupSocket, m_fdmax);

	for (size_t i = 0; i < m_socketInfo.size(); i++)
	{
		SSocketInfo& si = *m_socketInfo[i];
		if (!si.isActive)
			continue;
		if (si.NeedRead())
		{
			FD_SET(si.sock, &m_read);
			m_fdmax = smax(si.sock, m_fdmax);
		}
		if (si.NeedWrite())
		{
			FD_SET(si.sock, &m_write);
			m_fdmax = smax(si.sock, m_fdmax);
		}
	}

	if (m_fdmax >= 0)
	{
		timeval tv;
		tv.tv_sec = waitTime / 1000;
		tv.tv_usec = (waitTime - (tv.tv_sec * 1000)) * 1000;
		i32 r = (i32)select((i32)m_fdmax + 1, &m_read, &m_write, NULL, &tv);
		switch (r)
		{
		case 0:
		case DRX_SOCKET_ERROR:
			break;
		default:
			haveData = true;
			break;
		}
	}

	return haveData;
}

i32 CSocketIOUprSelect::PollWork(bool& performedWork)
{
	i32 r = 0;
	i32 ret = eSM_COMPLETEDIO;
	performedWork = false;
	char buffer[MAX_UDP_PACKET_SIZE];
	char address[_SS_MAXSIZE];
	DRXSOCKLEN_T addrlen = _SS_MAXSIZE;

	if (FD_ISSET(m_wakeupSocket, &m_read))
	{
		if (DrxSock::recvfrom(m_wakeupSocket, buffer, MAX_UDP_PACKET_SIZE, 0, (DRXSOCKADDR*)address, &addrlen))
		{
			SUserMessage* pMessage = reinterpret_cast<SUserMessage*>(&buffer);
	#if LOCK_NETWORK_FREQUENCY
			if (pMessage->m_frameID == m_userMessageFrameID)
	#endif // LOCK_NETWORK_FREQUENCY
			{
				ret = pMessage->m_message;
			}
		}
	}

	size_t numSockets = m_socketInfo.size();
	for (size_t i = 0; i < numSockets; i++)
	{
		SSocketInfo& si = *m_socketInfo[i];
		if (!si.isActive)
			continue;
		if (si.NeedRead() && FD_ISSET(si.sock, &m_read))
		{
			if (si.nRecvFrom)
			{
	#if USE_PSN
				if (si.protocol == IPROTO_UDPP2P_SAFE)
				{
					// Note Orbis sockets p2p address is same size as normal DRXSOCKADDR_IN however two bytes of the padding are now used
					sockaddr_in_p2p* psnSock = (sockaddr_in_p2p*)address;
					memset(psnSock, 0, sizeof(*psnSock));
					psnSock->sin_family = AF_INET;
				}
	#endif
				r = DrxSock::recvfrom(si.sock, buffer, MAX_UDP_PACKET_SIZE, 0, (DRXSOCKADDR*)address, &addrlen);
				switch (r)
				{
				case 0:
					si.pRecvFromTarget->OnRecvFromException(ConvertAddr((DRXSOCKADDR*)address, addrlen), eSE_ZeroLengthPacket);
					si.nRecvFrom--;
					break;
				case DRX_SOCKET_ERROR:
					{
						DrxSock::eDrxSockError sockErr = DrxSock::TranslateLastSocketError();
						if (sockErr != DrxSock::eCSE_EWOULDBLOCK)
						{
							si.pRecvFromTarget->OnRecvFromException(ConvertAddr((DRXSOCKADDR*)address, addrlen), OSErrorToSocketError(DrxSock::TranslateToSocketError(sockErr)));
							si.nRecvFrom--;
						}
					}
					break;
				default:
					CNetwork::Get()->ReportGotPacket();
					si.pRecvFromTarget->OnRecvFromComplete(ConvertAddr((DRXSOCKADDR*)address, addrlen), (u8*)buffer, r);
					si.nRecvFrom--;
					break;
				}
			}
			if (si.nRecv)
			{
				r = recv(si.sock, buffer, MAX_UDP_PACKET_SIZE, 0);
				switch (r)
				{
				case 0:
					si.pRecvTarget->OnRecvException(eSE_ZeroLengthPacket);
					si.nRecv--;
					break;
				case DRX_SOCKET_ERROR:
					{
						DrxSock::eDrxSockError sockErr = DrxSock::TranslateLastSocketError();
						if (DrxSock::TranslateLastSocketError() != DrxSock::eCSE_EWOULDBLOCK)
						{
							si.pRecvTarget->OnRecvException(OSErrorToSocketError(DrxSock::TranslateToSocketError(sockErr)));
							si.nRecv--;
						}
					}
					break;
				default:
					CNetwork::Get()->ReportGotPacket();
					si.pRecvTarget->OnRecvComplete((u8*)buffer, r);
					si.nRecv--;
					break;
				}
			}
			if (si.nListen)
			{
				addrlen = _SS_MAXSIZE;
				DRXSOCKET sock = DrxSock::accept(si.sock, (DRXSOCKADDR*)address, &addrlen);
				if (sock != DRX_INVALID_SOCKET)
				{
					si.pAcceptTarget->OnAccept(ConvertAddr((DRXSOCKADDR*)address, addrlen), sock);
					si.nListen--;
				}
				else
				{
					DrxSock::eDrxSockError sockErr = DrxSock::TranslateLastSocketError();
					if (sockErr != DrxSock::eCSE_EWOULDBLOCK)
					{
						si.pAcceptTarget->OnAcceptException(OSErrorToSocketError(DrxSock::TranslateToSocketError(sockErr)));
						si.nListen--;
					}
				}
			}
		}
		if (si.NeedWrite() && FD_ISSET(si.sock, &m_write))
		{
			bool done = false;
			while (!done && !si.outgoing.empty())
			{
				r = DrxSock::send(si.sock, (tuk)si.outgoing.front().data, si.outgoing.front().nLength, 0);
				switch (r)
				{
				case 0:
					si.pSendTarget->OnSendException(eSE_ZeroLengthPacket);
					break;
				case DRX_SOCKET_ERROR:
					{
						DrxSock::eDrxSockError sockErr = DrxSock::TranslateLastSocketError();
						if (sockErr != DrxSock::eCSE_EWOULDBLOCK)
						{
							si.pSendTarget->OnSendException(OSErrorToSocketError(DrxSock::TranslateToSocketError(sockErr)));
						}
						else
						{
							DrxLogAlways("SocketIOUprSelect: WSAEWOULDBLOCK");
							done = true;
						}
						break;
					}
				}
				if (!done)
					si.outgoing.pop_front();
			}
			done = false;
			while (!done && !si.outgoingAddressed.empty())
			{
				i32 _addrlen = _SS_MAXSIZE;
				if (ConvertAddr(si.outgoingAddressed.front().addr, (DRXSOCKADDR*)address, &_addrlen))
				{
					r = DrxSock::sendto(si.sock, (tuk)si.outgoingAddressed.front().data, si.outgoingAddressed.front().nLength, 0, (DRXSOCKADDR*)address, _addrlen);
					switch (r)
					{
					case 0:
						si.pSendToTarget->OnSendToException(si.outgoingAddressed.front().addr, eSE_ZeroLengthPacket);
						break;
					case DRX_SOCKET_ERROR:
						{
							DrxSock::eDrxSockError sockErr = DrxSock::TranslateLastSocketError();
							if (sockErr != DrxSock::eCSE_EWOULDBLOCK)
							{
								si.pSendToTarget->OnSendToException(si.outgoingAddressed.front().addr, OSErrorToSocketError(DrxSock::TranslateToSocketError(sockErr)));
							}
							else
							{
								DrxLogAlways("SocketIOUprSelect: WSAEWOULDBLOCK");
								done = true;
							}
							break;
						}
					}
				}
				if (!done)
					si.outgoingAddressed.pop_front();
			}
		}
	}

	return ret;
}

void CSocketIOUprSelect::PushUserMessage(i32 msg)
{
	if (msg < eUM_LAST || msg > eUM_FIRST)
	{
		// N.B. range check above relies on user messages being -ve
		DrxFatalError("PushUserMessage(%d) invalid message", msg);
	}

	SUserMessage message;
	message.m_message = msg;
	#if LOCK_NETWORK_FREQUENCY
	message.m_frameID = m_userMessageFrameID;
	#endif // LOCK_NETWORK_FREQUENCY
	DrxSock::sendto(m_wakeupSocket, reinterpret_cast<tuk>(&message), sizeof(message), 0, (DRXSOCKADDR*)&m_wakeupAddr, sizeof(m_wakeupAddr));
}

void CSocketIOUprSelect::WakeUp()
{
	char buf[1] = { 0 };
	DrxSock::sendto(m_wakeupSender, buf, 0, 0, (DRXSOCKADDR*)&m_wakeupAddr, sizeof(m_wakeupAddr));
}

SSocketID CSocketIOUprSelect::RegisterSocket(DRXSOCKET sock, i32 protocol)
{
	u32 id;
	for (id = 0; id < m_socketInfo.size(); id++)
		if (!m_socketInfo[id]->isActive)
			break;
	if (id == m_socketInfo.size())
		m_socketInfo.push_back(new SSocketInfo());

	m_socketInfo[id]->isActive = true;
	do
		m_socketInfo[id]->salt++;
	while (!m_socketInfo[id]->salt);
	m_socketInfo[id]->sock = sock;
	m_socketInfo[id]->protocol = protocol;

	// Need to reset contents of m_sockInfo[id]. The UnregisterSocket should have done this by assigning SSocketInfo, but
	// somehow m_socketInfo[id].nRecv has gone negative somewhere between UnregisterSocket and RegisterSocket
	// Safer to reinitialise everything except the salt.
	m_socketInfo[id]->nRecvFrom = m_socketInfo[id]->nRecv = m_socketInfo[id]->nListen = 0;
	m_socketInfo[id]->pRecvFromTarget = NULL;
	m_socketInfo[id]->pSendToTarget = NULL;
	m_socketInfo[id]->pConnectTarget = NULL;
	m_socketInfo[id]->pAcceptTarget = NULL;
	m_socketInfo[id]->pRecvTarget = NULL;
	m_socketInfo[id]->pSendTarget = NULL;

	return SSocketID((i32)id, m_socketInfo[id]->salt);
}

void CSocketIOUprSelect::UnregisterSocket(SSocketID sockid)
{
	if (SSocketInfo* pSI = GetSocketInfo(sockid))
	{
		u16 salt = pSI->salt;
		*pSI = SSocketInfo();
		pSI->salt = salt;
		do
			pSI->salt++;
		while (!pSI->salt);
	}
}

void CSocketIOUprSelect::RegisterBackoffAddressForSocket(const TNetAddress& addr, SSocketID sockid)
{
	if (m_pWatchdog)
	{
		if (SSocketInfo* pSI = GetSocketInfo(sockid))
		{
			m_pWatchdog->RegisterTarget(pSI->sock, addr);
		}
	}
}

void CSocketIOUprSelect::UnregisterBackoffAddressForSocket(const TNetAddress& addr, SSocketID sockid)
{
	if (m_pWatchdog)
	{
		if (SSocketInfo* pSI = GetSocketInfo(sockid))
		{
			m_pWatchdog->UnregisterTarget(pSI->sock, addr);
		}
	}
}

CSocketIOUprSelect::SSocketInfo* CSocketIOUprSelect::GetSocketInfo(SSocketID id)
{
	if (id.id >= m_socketInfo.size())
		return 0;
	if (m_socketInfo[id.id]->salt != id.salt)
		return 0;
	if (!m_socketInfo[id.id]->isActive)
		return 0;
	return m_socketInfo[id.id];
}

void CSocketIOUprSelect::SetRecvFromTarget(SSocketID sockid, IRecvFromTarget* pTarget)
{
	if (SSocketInfo* pSI = GetSocketInfo(sockid))
	{
		pSI->pRecvFromTarget = pTarget;
		pSI->nRecvFrom *= (pTarget != NULL);
	}
}

void CSocketIOUprSelect::SetConnectTarget(SSocketID sockid, IConnectTarget* pTarget)
{
	if (SSocketInfo* pSI = GetSocketInfo(sockid))
	{
		pSI->pConnectTarget = pTarget;
	}
}

void CSocketIOUprSelect::SetSendToTarget(SSocketID sockid, ISendToTarget* pTarget)
{
	if (SSocketInfo* pSI = GetSocketInfo(sockid))
	{
		pSI->pSendToTarget = pTarget;
		if (!pTarget)
			pSI->outgoingAddressed.clear();
	}
}

void CSocketIOUprSelect::SetAcceptTarget(SSocketID sockid, IAcceptTarget* pTarget)
{
	if (SSocketInfo* pSI = GetSocketInfo(sockid))
	{
		pSI->pAcceptTarget = pTarget;
		pSI->nListen *= (pTarget != NULL);
	}
}

void CSocketIOUprSelect::SetRecvTarget(SSocketID sockid, IRecvTarget* pTarget)
{
	if (SSocketInfo* pSI = GetSocketInfo(sockid))
	{
		pSI->pRecvTarget = pTarget;
		pSI->nRecv *= (pTarget != NULL);
	}
}

void CSocketIOUprSelect::SetSendTarget(SSocketID sockid, ISendTarget* pTarget)
{
	if (SSocketInfo* pSI = GetSocketInfo(sockid))
	{
		pSI->pSendTarget = pTarget;
		if (!pTarget)
			pSI->outgoing.clear();
	}
}

bool CSocketIOUprSelect::RequestRecvFrom(SSocketID sockid)
{
	if (SSocketInfo* pSI = GetSocketInfo(sockid))
	{
		if (pSI->pRecvFromTarget)
		{
			pSI->nRecvFrom++;
			return true;
		}
	}
	return false;
}

bool CSocketIOUprSelect::RequestSendTo(SSocketID sockid, const TNetAddress& addr, u8k* pData, size_t len)
{
	if (len > MAX_UDP_PACKET_SIZE)
		return false;

	if (SSocketInfo* pSI = GetSocketInfo(sockid))
	{
		if (pSI->pSendToTarget)
		{
	#if NET_MINI_PROFILE || NET_PROFILE_ENABLE
			RecordPacketSendStatistics(pData, len);
	#endif

			if (!pSI->outgoingAddressed.empty())
			{
delaysend:
				pSI->outgoingAddressed.push_back(SOutgoingAddressedData());
				pSI->outgoingAddressed.back().nLength = len;
				memcpy(pSI->outgoingAddressed.back().data, pData, len);
				WakeUp();
			}
			else
			{
				char address[_SS_MAXSIZE];
				i32 addrlen = _SS_MAXSIZE;
				if (ConvertAddr(addr, (DRXSOCKADDR*)address, &addrlen))
				{
	#if USE_PSN
					if (pSI->protocol == IPROTO_UDPP2P_SAFE)
					{
						// Note Orbis sockets p2p address is same size as normal DRXSOCKADDR_IN however two bytes of the padding are now used
						sockaddr_in_p2p* inP2PSock = (sockaddr_in_p2p*)address;
						inP2PSock->sin_vport = htons(UDPP2P_VPORT);
					}
	#endif
					i32 r = DrxSock::sendto(pSI->sock, (tuk)pData, len, 0, (DRXSOCKADDR*)address, addrlen);
					switch (r)
					{
					case 0:
						pSI->pSendToTarget->OnSendToException(addr, eSE_ZeroLengthPacket);
						break;
					case DRX_SOCKET_ERROR:
						{
							DrxSock::eDrxSockError sockErr = DrxSock::TranslateLastSocketError();
							if (sockErr != DrxSock::eCSE_EWOULDBLOCK)
							{
								pSI->pSendToTarget->OnSendToException(addr, OSErrorToSocketError(DrxSock::TranslateToSocketError(sockErr)));
							}
							else
							{
								goto delaysend;
							}
							break;
						}
					}
				}
			}
			return true;
		}
	}
	return false;
}

bool CSocketIOUprSelect::RequestSendVoiceTo(SSocketID sockid, const TNetAddress& addr, u8k* pData, size_t len)
{
	return RequestSendTo(sockid, addr, pData, len);
}

bool CSocketIOUprSelect::RequestConnect(SSocketID sockid, const TNetAddress& addr)
{
	if (SSocketInfo* pSI = GetSocketInfo(sockid))
	{
		if (pSI->pConnectTarget)
		{
			char address[_SS_MAXSIZE];
			i32 addrlen = _SS_MAXSIZE;
			if (ConvertAddr(addr, (DRXSOCKADDR*)address, &addrlen))
			{
				if (DrxSock::connect(pSI->sock, (DRXSOCKADDR*)address, addrlen))
				{
					pSI->pConnectTarget->OnConnectException(OSErrorToSocketError(DrxSock::GetLastSocketError()));
				}
				else
					pSI->pConnectTarget->OnConnectComplete();
			}
			return true;
		}
	}
	return false;
}

bool CSocketIOUprSelect::RequestAccept(SSocketID sockid)
{
	if (SSocketInfo* pSI = GetSocketInfo(sockid))
	{
		if (pSI->pAcceptTarget)
		{
			pSI->nListen++;
			return true;
		}
	}
	return false;
}

bool CSocketIOUprSelect::RequestSend(SSocketID sockid, u8k* pData, size_t len)
{
	if (SSocketInfo* pSI = GetSocketInfo(sockid))
	{
		if (pSI->pSendTarget)
		{
			while (len)
			{
				pSI->outgoing.push_back(SOutgoingData());
				size_t ncp = std::min(len, size_t(MAX_UDP_PACKET_SIZE));
				pSI->outgoing.back().nLength = ncp;
				memcpy(pSI->outgoing.back().data, pData, ncp);
				pData += ncp;
				len -= ncp;
			}
			return true;
		}
	}
	return false;
}

bool CSocketIOUprSelect::RequestRecv(SSocketID sockid)
{
	if (SSocketInfo* pSI = GetSocketInfo(sockid))
	{
		if (pSI->pRecvTarget)
		{
			pSI->nRecv++;
			return true;
		}
	}
	return false;
}

#endif
