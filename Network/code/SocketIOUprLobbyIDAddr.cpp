// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/SocketIOUprLobbyIDAddr.h>

#if defined(HAS_SOCKETIOMANAGER_LOBBYIDADDR)

	#include  <drx3D/Network/Network.h>

CSocketIOUprLobbyIDAddr::CSocketIOUprLobbyIDAddr() : CSocketIOUpr(eSIOMC_NoBuffering)
{
}

CSocketIOUprLobbyIDAddr::~CSocketIOUprLobbyIDAddr()
{
}

bool CSocketIOUprLobbyIDAddr::Init()
{
	m_registeredSockets.push_back(SRegisteredSocket(222)); // 222 is not 123 (see below)
	m_registeredSockets[0].inUse = true;
	return true;
}

bool CSocketIOUprLobbyIDAddr::PollWait(u32 waitTime)
{
	bool haveData = false;
	m_recvSize = 0;

	IDrxMatchMakingPrivate* pMMPrivate = gEnv->pLobby ? gEnv->pLobby->GetMatchMakingPrivate() : nullptr;
	if (pMMPrivate)
	{
		haveData = pMMPrivate->LobbyAddrIDHasPendingData();
	}
	return haveData;
}

void CSocketIOUprLobbyIDAddr::RecvPacket(uk privateRef, u8* recvBuffer, u32 recvSize, DRXSOCKET recvSocket, TNetAddress& recvAddr)
{
	CSocketIOUprLobbyIDAddr* _this = (CSocketIOUprLobbyIDAddr*)privateRef;
	CNetwork::Get()->ReportGotPacket();
	for (u32 a = 0; a < _this->m_registeredSockets.size(); a++)
	{
		if (_this->m_registeredSockets[a].inUse && _this->m_registeredSockets[a].sock == recvSocket)
		{
			if (!_this->m_registeredSockets[a].pRecvFromTarget)
			{
				NetLog("CSocketIOUprLobbyIDAddr::RecvPacket - socket matched for non registered recvFromTarget");
				return;
			}
			_this->m_registeredSockets[a].pRecvFromTarget->OnRecvFromComplete(recvAddr, recvBuffer, recvSize);
			return;
		}
	}

	NetLog("CSocketIOUprLobbyIDAddr::RecvPacket - Packet recieved but no matching socket");
}

i32 CSocketIOUprLobbyIDAddr::PollWork(bool& performedWork)
{
	IDrxMatchMakingPrivate* pMMPrivate = gEnv->pLobby ? gEnv->pLobby->GetMatchMakingPrivate() : nullptr;
	if (pMMPrivate)
	{
		pMMPrivate->LobbyAddrIDRecv(RecvPacket, this);
	}

	return eSM_COMPLETEDIO;
}

SSocketID CSocketIOUprLobbyIDAddr::RegisterSocket(DRXSOCKET sock, i32 protocol)
{
	ASSERT_GLOBAL_LOCK;
	SCOPED_COMM_LOCK;

	u32 id;
	for (id = 0; id < m_registeredSockets.size(); id++)
	{
		if (!m_registeredSockets[id].inUse)
			break;
	}
	if (id == m_registeredSockets.size())
		m_registeredSockets.push_back(SRegisteredSocket(123)); // 123 is an arbitrary number that's somewhat larger than zero

	SRegisteredSocket& rs = m_registeredSockets[id];
	rs = SRegisteredSocket(rs.salt + 1);
	SSocketID sockid(id, rs.salt);

	rs.sock = sock;
	rs.inUse = true;
	return sockid;
}

void CSocketIOUprLobbyIDAddr::UnregisterSocket(SSocketID sockid)
{
	ASSERT_GLOBAL_LOCK;
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;

	*pSock = SRegisteredSocket(pSock->salt + 1);
}

void CSocketIOUprLobbyIDAddr::RegisterBackoffAddressForSocket(TNetAddress addr, SSocketID sockid)
{
}

void CSocketIOUprLobbyIDAddr::UnregisterBackoffAddressForSocket(TNetAddress addr, SSocketID sockid)
{
}

CSocketIOUprLobbyIDAddr::SRegisteredSocket* CSocketIOUprLobbyIDAddr::GetRegisteredSocket(SSocketID sockid)
{
	ASSERT_COMM_LOCK;

	if (sockid.id >= m_registeredSockets.size())
	{
		NET_ASSERT(false);
		return 0;
	}
	if (!m_registeredSockets[sockid.id].inUse)
		return 0;
	if (m_registeredSockets[sockid.id].salt != sockid.salt)
		return 0;
	return &m_registeredSockets[sockid.id];
}

void CSocketIOUprLobbyIDAddr::SetRecvFromTarget(SSocketID sockid, IRecvFromTarget* pTarget)
{
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;
	pSock->pRecvFromTarget = pTarget;
}

void CSocketIOUprLobbyIDAddr::SetSendToTarget(SSocketID sockid, ISendToTarget* pTarget)
{
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;
	pSock->pSendToTarget = pTarget;
}

void CSocketIOUprLobbyIDAddr::SetConnectTarget(SSocketID sockid, IConnectTarget* pTarget)
{
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;
	pSock->pConnectTarget = pTarget;
}

void CSocketIOUprLobbyIDAddr::SetAcceptTarget(SSocketID sockid, IAcceptTarget* pTarget)
{
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;
	pSock->pAcceptTarget = pTarget;
}

void CSocketIOUprLobbyIDAddr::SetRecvTarget(SSocketID sockid, IRecvTarget* pTarget)
{
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;
	pSock->pRecvTarget = pTarget;
}

void CSocketIOUprLobbyIDAddr::SetSendTarget(SSocketID sockid, ISendTarget* pTarget)
{
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;
	pSock->pSendTarget = pTarget;
}

bool CSocketIOUprLobbyIDAddr::RequestRecvFrom(SSocketID sockid)
{
	return true;
}

bool CSocketIOUprLobbyIDAddr::RequestSendTo(SSocketID sockid, const TNetAddress& addr, u8k* pData, size_t len)
{
	return true;
}

bool CSocketIOUprLobbyIDAddr::RequestSendVoiceTo(SSocketID sockid, const TNetAddress& addr, u8k* pData, size_t len)
{
	return true;
}

bool CSocketIOUprLobbyIDAddr::RequestRecv(SSocketID sockid)
{
	return true;
}

bool CSocketIOUprLobbyIDAddr::RequestSend(SSocketID sockid, u8k* pData, size_t len)
{
	return true;
}

bool CSocketIOUprLobbyIDAddr::RequestConnect(SSocketID sockid, const TNetAddress& addr)
{
	return true;
}

bool CSocketIOUprLobbyIDAddr::RequestAccept(SSocketID sockid)
{
	return true;
}

#endif // defined(HAS_SOCKETIOMANAGER_LOBBYIDADDR)
