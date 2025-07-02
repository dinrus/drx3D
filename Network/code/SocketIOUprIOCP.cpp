// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/SocketIOUprIOCP.h>

#if defined(HAS_SOCKETIOMANAGER_IOCP)

	#include  <drx3D/Network/Network.h>

	#if DRX_PLATFORM_WINDOWS
		#include <mswsock.h>
	#endif

static i32k COMFORTABLE_COMPLETED_BACKLOG = 64;
static i32k MAX_PACKETS_PER_INNER_LOOP = 32;
static i32k MAX_SMALL_WAITS = 1;
static i32k MAX_PACKETS_PER_OUTER_LOOP = MAX_SMALL_WAITS * MAX_PACKETS_PER_INNER_LOOP + 1;

	#if !USE_SYSTEM_ALLOCATOR
CSocketIOUprIOCP::TIORequestAllocator* CSocketIOUprIOCP::m_pAllocator = 0;
	#endif

CSocketIOUprIOCP::CSocketIOUprIOCP() : CSocketIOUpr(eSIOMC_SupportsBackoff | eSIOMC_NoBuffering)
	, m_recursionDepth(0)
	, m_cio({ 0 })
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

	m_iocp = NULL;
	m_pendingData = false;
	m_lastBackoffCheck = 0.0f;
}

CSocketIOUprIOCP::~CSocketIOUprIOCP()
{
	if (m_pWatchdog)
	{
		delete m_pWatchdog;
	}
}

bool CSocketIOUprIOCP::Init()
{
	m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (!m_iocp)
		return false;
	m_registeredSockets.push_back(SRegisteredSocket(222)); // 222 is not 123 (see below)
	m_registeredSockets[0].inUse = true;
	m_recursionDepth = 0;
	return true;
}

class CSocketIOUprIOCP::CAutoFreeIORequest
{
public:
	CAutoFreeIORequest(CSocketIOUprIOCP* pMgr, SIORequest* pReq) : m_pMgr(pMgr), m_pReq(pReq) {}
	~CAutoFreeIORequest() { if (m_pReq) m_pMgr->FreeIORequest(m_pReq); }

	void Release() { m_pReq = 0; }

private:
	CSocketIOUprIOCP* m_pMgr;
	SIORequest*           m_pReq;
};

bool CSocketIOUprIOCP::PollWait(u32 waitTime)
{
	if (m_pWatchdog)
	{
		m_pWatchdog->ClearStalls();
	}

	ULONG_PTR sockid_fromOS;

	g_systemBranchCounters.iocpReapSleep++;
	m_cio.bSuccess = GetQueuedCompletionStatus(m_iocp, &m_cio.nIOSize, &sockid_fromOS, &m_cio.pOverlapped, waitTime);
	m_cio.sockid = (i32)sockid_fromOS;
	m_cio.err = WAIT_TIMEOUT + 1; // a value that's != WAIT_TIMEOUT
	if (!m_cio.bSuccess)
	{
		m_cio.err = GetLastError();
	}

	return (m_cio.err != WAIT_TIMEOUT);
}

i32 CSocketIOUprIOCP::PollWork(bool& performedWork)
{
	ULONG_PTR sockid_fromOS;

	g_systemBranchCounters.iocpReapSleepCollect++;
	m_completedIO.push(m_cio); // m_cio populated in PollWait() above
	m_pendingData = true;
	CheckBackoff();

	while (true)
	{
		g_systemBranchCounters.iocpReapImmediate++;
		m_cio.bSuccess = GetQueuedCompletionStatus(m_iocp, &m_cio.nIOSize, &sockid_fromOS, &m_cio.pOverlapped, 0);
		m_cio.sockid = (i32)sockid_fromOS;
		if (!m_cio.bSuccess)
		{
			m_cio.err = GetLastError();
			if (m_cio.err == WAIT_TIMEOUT)
				break;
		}
		g_systemBranchCounters.iocpReapImmediateCollect++;
		m_completedIO.push(m_cio);
		m_pendingData = true;
		CheckBackoff();
	}

	if (m_completedIO.empty())
	{
		g_systemBranchCounters.iocpIdle++;
		m_pendingData = false;
		return eSM_COMPLETEDIO;
	}

	i32 retval = eEM_UNSPECIFIED;

	{
		SCOPED_GLOBAL_LOCK;
		performedWork = true;
		g_systemBranchCounters.iocpForceSync++;
		retval = PollInnerLoop(retval);
		NET_ASSERT(m_completedIO.empty());
		if (m_completedIO.empty())
			m_pendingData = false;
	}

	// check for anything we haven't sent to in a long long time
	// can't use g_time due to it not being threadsafe here, we hold no locks
	CTimeValue now = gEnv->pTimer->GetAsyncTime();
	if ((now - m_lastBackoffCheck).GetSeconds() > 0.5f)
	{
		g_systemBranchCounters.iocpBackoffCheck++;
		SCOPED_COMM_LOCK;
		for (TRegisteredSockets::iterator itSock = m_registeredSockets.begin(); itSock != m_registeredSockets.end(); ++itSock)
		{
			if (!itSock->inUse)
				continue;
			for (TBackoffTargets::iterator itBack = itSock->backoffTargets.begin(); itBack != itSock->backoffTargets.end(); ++itBack)
			{
				if ((now - itBack->second.lastSystemSend).GetSeconds() > 1.0f)
				{
					DoRequestSendTo(&*itSock, itBack->first, Frame_IDToHeader + eH_KeepAlive, 1, now);
				}
			}
		}
		m_lastBackoffCheck = now;
	}

	return retval;
}

void CSocketIOUprIOCP::CheckBackoff()
{
	if (m_completedIO.size() > COMFORTABLE_COMPLETED_BACKLOG && m_completedIO.back().bSuccess && m_completedIO.back().pOverlapped)
	{
		SIORequest* pReq = (SIORequest*) m_completedIO.back().pOverlapped;
		if (pReq->iort == eIORT_RecvFrom)
		{
			SCOPED_COMM_LOCK;
			SRegisteredSocket* pSock = GetRegisteredSocket(SSocketID::FromInt(m_completedIO.back().sockid - 1));
			if (pSock && !(m_rand.GenerateUint32() % 10))
			{
				NetLog("[CSocketIOUprIOCP::CheckBackoff] %s: Requesting Backoff. Backlog: %i", gEnv->IsDedicated() ? "Server" : "Client", (i32)m_completedIO.size());
				BackoffDueToRequest(pSock, pReq);
			}
		}
	}
}

void CSocketIOUprIOCP::BackoffDueToRequest(SRegisteredSocket* pSock, SIORequest* pReq)
{
	TNetAddress addr = ConvertAddr((DRXSOCKADDR*)pReq->addrBuffer, pReq->addrLength);
	TBackoffTargets::iterator it = pSock->backoffTargets.find(addr);
	if (it == pSock->backoffTargets.end())
		return;
	CTimeValue now = gEnv->pTimer->GetAsyncTime();
	if (now - it->second.lastBackoffSend > 1.0f)
	{
		DoRequestSendTo(pSock, addr, Frame_IDToHeader + eH_BackOff, 1, now);
	}
}

i32 CSocketIOUprIOCP::PollInnerLoop(i32& retval)
{
	while (!m_completedIO.empty())
	{
		i32 r = PollInner(m_completedIO.front());
		m_completedIO.pop();
		if (r < 0)
		{
			if (retval > 0 || r > retval)
				retval = r;
		}
		else if (r > 0)
		{
			if (retval < eEM_UNSPECIFIED || retval > r)
				retval = r;
		}
	}
	return retval;
}

i32 CSocketIOUprIOCP::PollInner(const SCompletedIO& io)
{
	ASSERT_GLOBAL_LOCK;

	SIORequest* pReq = (SIORequest*)io.pOverlapped;
	CRecursionGuard rg(this);
	CAutoFreeIORequest freeReq(this, pReq);
	g_time = gEnv->pTimer->GetAsyncTime();

	SRegisteredSocket* pSock = 0;

	if (!io.bSuccess)
	{
		NET_ASSERT(io.err != WAIT_TIMEOUT);
		{
			SCOPED_COMM_LOCK;
			if (!pReq)
			{
				NetWarning("GetQueuedCompletionStatus fails with no request: %d", io.err);
				return eEM_NO_REQUEST;
			}
			if (pReq->iort == eIORT_Dead)
			{
				return eEM_REQUEST_DEAD;
			}
			pSock = GetRegisteredSocket(SSocketID::FromInt(io.sockid - 1));
			if (!pSock)
			{
				//NetWarning("IO request on unbound socket id %d", io.sockid);
				return eEM_UNBOUND_SOCKET1;
			}
		}
		NET_ASSERT(pSock);
		return CompleteFailure(pSock, pReq, io.err);
	}

	if (!pReq)
	{
		NetWarning("GetQueuedCompletionStatus succeeds with no request");
		return eEM_NO_REQUEST;
	}
	if (pReq->iort == eIORT_Dead)
		return eEM_REQUEST_DEAD;

	{
		SCOPED_COMM_LOCK;
		pSock = GetRegisteredSocket(SSocketID::FromInt(io.sockid - 1));
		if (!pSock)
		{
			//NetWarning("IO request on unbound socket id %d", io.sockid);
			return eEM_UNBOUND_SOCKET2;
		}

		if (pSock == &m_registeredSockets[0])
		{
			// User messages are processed here, and since we don't use memory managed
			// overlapped structures (see PushUserMessage()) we need to prevent them
			// being freed by the CAutoFreeIORequest at the top of this function
			i32 retval = -i32(io.nIOSize);
	#if LOCK_NETWORK_FREQUENCY
			u32 frameID = *(u32*)(&pReq->dataBuffer);
			if (frameID != m_userMessageFrameID)
			{
				retval = eSM_COMPLETEDIO;
			}
	#endif // LOCK_NETWORK_FREQUENCY
			freeReq.Release();
			return retval;
		}
	}
	NET_ASSERT(pSock);
	if (io.nIOSize)
		return CompleteSuccess(pSock, pReq, io.nIOSize);
	else
		return CompleteEmptySuccess(pSock, pReq);
}

i32 CSocketIOUprIOCP::CompleteSuccess(SRegisteredSocket* pSock, SIORequest* pReq, DWORD nIOSize)
{
	DRX_PROFILE_FUNCTION(PROFILE_NETWORK);

	switch (pReq->iort)
	{
	case eIORT_RecvFromX:
		NET_ASSERT(false);
		break;
	case eIORT_SomewhatDone:
		break;
	case eIORT_RecvFrom:
		CNetwork::Get()->ReportGotPacket();
		if (!pSock->pRecvFromTarget)
			return eEM_NO_TARGET;
		pSock->pRecvFromTarget->OnRecvFromComplete(ConvertAddr((DRXSOCKADDR*)pReq->addrBuffer, pReq->addrLength), pReq->dataBuffer, nIOSize);
		break;
	#if !DRX_PLATFORM_DURANGO
	case eIORT_Recv:
		CNetwork::Get()->ReportGotPacket();
		if (!pSock->pRecvTarget)
			return eEM_NO_TARGET;
		pSock->pRecvTarget->OnRecvComplete(pReq->dataBuffer, nIOSize);
		break;
	case eIORT_Connect:
		CNetwork::Get()->ReportGotPacket();
		if (!pSock->pConnectTarget)
			return eEM_NO_TARGET;
		pSock->pConnectTarget->OnConnectComplete();
		break;
	case eIORT_Accept:
		CNetwork::Get()->ReportGotPacket();
		if (!pSock->pAcceptTarget)
			return eEM_NO_TARGET;
		else
		{
			DRXSOCKADDR* pLocalAddr;
			DRXSOCKADDR* pRemoteAddr;
			INT localAddrLen;
			INT remoteAddrLen;
			GetAcceptExSockaddrs(pReq->dataBuffer, 0, 256, 256, &pLocalAddr, &localAddrLen, &pRemoteAddr, &remoteAddrLen);
			pSock->pAcceptTarget->OnAccept(ConvertAddr(pRemoteAddr, remoteAddrLen), pReq->acceptSock);
		}
		break;
	#endif
	}
	return eSM_COMPLETE_SUCCESS_OK;
}

i32 CSocketIOUprIOCP::CompleteEmptySuccess(SRegisteredSocket* pSock, SIORequest* pReq)
{
	DRX_PROFILE_FUNCTION(PROFILE_NETWORK);

	switch (pReq->iort)
	{
	case eIORT_SomewhatDone:
		break;
	case eIORT_RecvFrom:
		if (!pSock->pRecvFromTarget)
			return eEM_NO_TARGET;
		pSock->pRecvFromTarget->OnRecvFromException(ConvertAddr((DRXSOCKADDR*)pReq->addrBuffer, pReq->addrLength), eSE_ZeroLengthPacket);
		break;
	case eIORT_RecvFromX:
		if (!pSock->pRecvFromTarget)
			return eEM_NO_TARGET;
		NetLog("CompleteEmptySuccess: pReq->bytesReceived==%d", pReq->bytesReceived);
		pSock->pRecvFromTarget->OnRecvFromException(ConvertAddr((DRXSOCKADDR*)pReq->addrBuffer, pReq->addrLength), OSErrorToSocketError(pReq->bytesReceived));
		break;
	case eIORT_SendTo:
		if (!pSock->pSendToTarget)
			return eEM_NO_TARGET;
		pSock->pSendToTarget->OnSendToException(ConvertAddr((DRXSOCKADDR*)pReq->addrBuffer, pReq->addrLength), eSE_ZeroLengthPacket);
		break;
	#if !DRX_PLATFORM_DURANGO
	case eIORT_Recv:
		if (!pSock->pRecvTarget)
			return eEM_NO_TARGET;
		pSock->pRecvTarget->OnRecvException(eSE_ZeroLengthPacket);
		break;
	case eIORT_Send:
		if (!pSock->pSendTarget)
			return eEM_NO_TARGET;
		pSock->pSendTarget->OnSendException(eSE_ZeroLengthPacket);
		break;
	case eIORT_Connect:
		if (!pSock->pConnectTarget)
			return eEM_NO_TARGET;
		pSock->pConnectTarget->OnConnectComplete();
		break;
	case eIORT_Accept:
		if (!pSock->pAcceptTarget)
			return eEM_NO_TARGET;
		else
		{
			DRXSOCKADDR* pLocalAddr;
			DRXSOCKADDR* pRemoteAddr;
			INT localAddrLen;
			INT remoteAddrLen;
			GetAcceptExSockaddrs(pReq->dataBuffer, 0, 256, 256, &pLocalAddr, &localAddrLen, &pRemoteAddr, &remoteAddrLen);
			pSock->pAcceptTarget->OnAccept(ConvertAddr(pRemoteAddr, remoteAddrLen), pReq->acceptSock);
		}
		break;
	#endif
	}

	return eSM_COMPLETE_EMPTY_SUCCESS_OK;
}

i32 CSocketIOUprIOCP::CompleteFailure(SRegisteredSocket* pSock, SIORequest* pReq, DWORD err)
{
	DRX_PROFILE_FUNCTION(PROFILE_NETWORK);

	ESocketError sockErr = OSErrorToSocketError(err);

	switch (pReq->iort)
	{
	case eIORT_SomewhatDone:
		break;
	case eIORT_RecvFromX:
		NET_ASSERT(false);
		break;
	case eIORT_RecvFrom:
		if (!pSock->pRecvFromTarget)
			return eEM_NO_TARGET;
		pSock->pRecvFromTarget->OnRecvFromException(ConvertAddr((DRXSOCKADDR*)pReq->addrBuffer, pReq->addrLength), sockErr);
		break;
	case eIORT_SendTo:
		if (!pSock->pSendToTarget)
			return eEM_NO_TARGET;
		pSock->pSendToTarget->OnSendToException(ConvertAddr((DRXSOCKADDR*)pReq->addrBuffer, pReq->addrLength), sockErr);
		break;
	#if !DRX_PLATFORM_DURANGO
	case eIORT_Recv:
		if (!pSock->pRecvTarget)
			return eEM_NO_TARGET;
		pSock->pRecvTarget->OnRecvException(sockErr);
		break;
	case eIORT_Send:
		if (!pSock->pSendTarget)
			return eEM_NO_TARGET;
		pSock->pSendTarget->OnSendException(sockErr);
		break;
	case eIORT_Connect:
		if (!pSock->pConnectTarget)
			return eEM_NO_TARGET;
		pSock->pConnectTarget->OnConnectException(sockErr);
		break;
	case eIORT_Accept:
		if (!pSock->pAcceptTarget)
			return eEM_NO_TARGET;
		pSock->pAcceptTarget->OnAcceptException(sockErr);
		closesocket(pReq->acceptSock);
		break;
	#endif
	}
	return eSM_COMPLETE_FAILURE_OK;
}

void CSocketIOUprIOCP::PushUserMessage(i32 msg)
{
	static SIORequest userMessageBuffer[MAX_USER_MESSAGES_PER_FRAME];
	static u32 userMessageIndex = 0;

	if (msg < eUM_LAST || msg > eUM_FIRST)
	{
		// N.B. range check above relies on user messages being -ve
		DrxFatalError("PushUserMessage(%d) invalid message", msg);
	}
	SIORequest* pReq = &userMessageBuffer[userMessageIndex++];
	userMessageIndex &= MAX_USER_MESSAGES_MASK;
	pReq->iort = eIORT_UserMessage;
	#if LOCK_NETWORK_FREQUENCY
	*(u32*)(&pReq->dataBuffer) = m_userMessageFrameID;
	#endif // LOCK_NETWORK_FREQUENCY
	PostQueuedCompletionStatus(m_iocp, -msg, SSocketID(0, m_registeredSockets[0].salt).AsInt() + 1, &(pReq->overlapped));
}

SSocketID CSocketIOUprIOCP::RegisterSocket(DRXSOCKET sock, i32 protocol)
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

	if (!CreateIoCompletionPort((HANDLE)sock, m_iocp, sockid.AsInt() + 1, 0))
	{
		NetWarning("CreateIoCompletionPort failed: %d", GetLastError());
		return SSocketID();
	}

	rs.sock = sock;
	rs.inUse = true;
	return sockid;
}

void CSocketIOUprIOCP::UnregisterSocket(SSocketID sockid)
{
	ASSERT_GLOBAL_LOCK;
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;
	CancelIo((HANDLE)pSock->sock);

	*pSock = SRegisteredSocket(pSock->salt + 1);
}

void CSocketIOUprIOCP::RegisterBackoffAddressForSocket(const TNetAddress& addr, SSocketID sockid)
{
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;

	pSock->backoffTargets.insert(std::make_pair(addr, SBackoffTarget()));

	if (m_pWatchdog)
	{
		m_pWatchdog->RegisterTarget(pSock->sock, addr);
	}
}

void CSocketIOUprIOCP::UnregisterBackoffAddressForSocket(const TNetAddress& addr, SSocketID sockid)
{
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;

	pSock->backoffTargets.erase(addr);

	if (m_pWatchdog)
	{
		m_pWatchdog->UnregisterTarget(pSock->sock, addr);
	}
}

CSocketIOUprIOCP::SRegisteredSocket* CSocketIOUprIOCP::GetRegisteredSocket(SSocketID sockid)
{
	ASSERT_COMM_LOCK;

	if (sockid.id < 0 || sockid.id >= m_registeredSockets.size())
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

void CSocketIOUprIOCP::SetRecvFromTarget(SSocketID sockid, IRecvFromTarget* pTarget)
{
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;
	pSock->pRecvFromTarget = pTarget;
}

void CSocketIOUprIOCP::SetSendToTarget(SSocketID sockid, ISendToTarget* pTarget)
{
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;
	pSock->pSendToTarget = pTarget;
}

void CSocketIOUprIOCP::SetConnectTarget(SSocketID sockid, IConnectTarget* pTarget)
{
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;
	pSock->pConnectTarget = pTarget;
}

void CSocketIOUprIOCP::SetAcceptTarget(SSocketID sockid, IAcceptTarget* pTarget)
{
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;
	pSock->pAcceptTarget = pTarget;
}

void CSocketIOUprIOCP::SetRecvTarget(SSocketID sockid, IRecvTarget* pTarget)
{
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;
	pSock->pRecvTarget = pTarget;
}

void CSocketIOUprIOCP::SetSendTarget(SSocketID sockid, ISendTarget* pTarget)
{
	SCOPED_COMM_LOCK;

	SRegisteredSocket* pSock = GetRegisteredSocket(sockid);
	if (!pSock)
		return;
	pSock->pSendTarget = pTarget;
}

bool CSocketIOUprIOCP::RequestRecvFrom(SSocketID sockid)
{
	SCOPED_COMM_LOCK;

	CRecursionGuard rg(this);

	SRegisteredSocket* rs = GetRegisteredSocket(sockid);
	if (!rs)
		return false;

	SIORequest* pReq = AllocIORequest();
	CAutoFreeIORequest freeReq(this, pReq);

	pReq->flags = 0;
	pReq->iort = eIORT_RecvFrom;
	i32 recvret = WSARecvFrom(rs->sock, pReq->wsaBuffer, 1, &pReq->bytesReceived, &pReq->flags, (DRXSOCKADDR*)pReq->addrBuffer, &pReq->addrLength, &(pReq->overlapped), NULL);
	switch (recvret)
	{
	case 0:
		if (rg.RecursionAllowed())
		{
			if (rs->pRecvFromTarget)
				rs->pRecvFromTarget->OnRecvFromComplete(ConvertAddr((DRXSOCKADDR*)pReq->addrBuffer, pReq->addrLength), pReq->dataBuffer, pReq->bytesReceived);
			pReq->iort = eIORT_SomewhatDone;
		}
		break;
	case DRX_SOCKET_ERROR:
		{
			i32 err = WSAGetLastError();
			if (ERROR_IO_PENDING != err)
			{
				NetWarning("WSARecvFrom failed: %d", err);
				pReq->iort = eIORT_RecvFromX;
				pReq->bytesReceived = err;
				if (FALSE == PostQueuedCompletionStatus(m_iocp, 0, sockid.AsInt() + 1, &pReq->overlapped))
					return false;
			}
		}
		break;
	default:
		NetLog("WSARecvFrom returns %d", recvret);
	}
	freeReq.Release();

	return true;
}

bool CSocketIOUprIOCP::RequestSendTo(SSocketID sockid, const TNetAddress& addr, u8k* pData, size_t len)
{
	SCOPED_COMM_LOCK;

	CRecursionGuard rg(this);

	if (len > sizeof(static_cast<SIORequest*>(NULL)->dataBuffer))
		return false;

	SRegisteredSocket* rs = GetRegisteredSocket(sockid);
	if (!rs)
		return false;

	return DoRequestSendTo(rs, addr, pData, len, 0.0f);
}

bool CSocketIOUprIOCP::RequestSendVoiceTo(SSocketID sockid, const TNetAddress& addr, u8k* pData, size_t len)
{
	return RequestSendTo(sockid, addr, pData, len);
}

bool CSocketIOUprIOCP::DoRequestSendTo(SRegisteredSocket* rs, const TNetAddress& addr, u8k* pData, size_t len, CTimeValue now)
{
	ASSERT_COMM_LOCK;

	SIORequest* pReq = AllocIORequest();
	CAutoFreeIORequest freeReq(this, pReq);

	TBackoffTargets::iterator it = rs->backoffTargets.find(addr);
	if (it != rs->backoffTargets.end())
	{
		if (now == 0.0f)
			now = gEnv->pTimer->GetAsyncTime();

		it->second.lastSystemSend = now;
		if (len == 1)
		{
			switch (Frame_HeaderToID[pData[0]])
			{
			case eH_BackOff:
				it->second.lastBackoffSend = now;
				break;
			case eH_KeepAlive:
				it->second.lastKeepAliveSend = now;
				break;
			}
		}
	}

	pReq->flags = 0;
	pReq->iort = eIORT_SendTo;
	memcpy(pReq->dataBuffer, pData, len);
	pReq->bytesReceived = len;
	pReq->wsaBuffer[0].len = len;
	if (!ConvertAddr(addr, (DRXSOCKADDR*)pReq->addrBuffer, &pReq->addrLength))
		return false;
	i32 sendret = WSASendTo(rs->sock, pReq->wsaBuffer, 1, &pReq->bytesReceived, 0, (DRXSOCKADDR*)pReq->addrBuffer, pReq->addrLength, &pReq->overlapped, NULL);
	if (sendret == DRX_SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()))
	{
		NetWarning("WSASendTo failed: %d", WSAGetLastError());
		return false;
	}
	freeReq.Release();

	#if NET_MINI_PROFILE || NET_PROFILE_ENABLE
	RecordPacketSendStatistics(pData, len);
	#endif

	return true;
}

bool CSocketIOUprIOCP::RequestRecv(SSocketID sockid)
{
	SCOPED_COMM_LOCK;

	CRecursionGuard rg(this);

	#if !DRX_PLATFORM_DURANGO
	SRegisteredSocket* rs = GetRegisteredSocket(sockid);
	if (!rs)
		return false;

	SIORequest* pReq = AllocIORequest();
	CAutoFreeIORequest freeReq(this, pReq);

	pReq->flags = 0;
	pReq->iort = eIORT_Recv;
	i32 recvret = WSARecv(rs->sock, pReq->wsaBuffer, 1, &pReq->bytesReceived, &pReq->flags, &pReq->overlapped, NULL);
	if (recvret == DRX_SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()))
	{
		NetWarning("WSARecv failed: %d", WSAGetLastError());
		return false;
	}
	freeReq.Release();

	return true;
	#else
	return false;
	#endif
}

bool CSocketIOUprIOCP::RequestSend(SSocketID sockid, u8k* pData, size_t len)
{
	SCOPED_COMM_LOCK;

	CRecursionGuard rg(this);

	#if !DRX_PLATFORM_DURANGO
	//if (len > sizeof(static_cast<SIORequest*>(NULL)->dataBuffer))
	//	return false;

	SRegisteredSocket* rs = GetRegisteredSocket(sockid);
	if (!rs)
		return false;

	while (len > 0)
	{
		SIORequest* pReq = AllocIORequest();
		CAutoFreeIORequest freeReq(this, pReq);

		pReq->flags = 0;
		pReq->iort = eIORT_Send;
		size_t ncp = std::min(len, size_t(MAX_UDP_PACKET_SIZE));
		memcpy(pReq->dataBuffer, pData, ncp);
		pReq->bytesReceived = ncp;
		pReq->wsaBuffer[0].len = ncp;
		i32 sendret = WSASend(rs->sock, pReq->wsaBuffer, 1, &pReq->bytesReceived, 0, &pReq->overlapped, NULL);
		if (sendret == DRX_SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()))
		{
			NetWarning("WSASendTo failed: %d", WSAGetLastError());
			return false;
		}
		freeReq.Release();

		pData += ncp;
		len -= ncp;
	}

	return true;
	#else
	return false;
	#endif
}

bool CSocketIOUprIOCP::RequestConnect(SSocketID sockid, const TNetAddress& addr)
{
	SCOPED_COMM_LOCK;

	CRecursionGuard rg(this);

	#if !DRX_PLATFORM_DURANGO
	SRegisteredSocket* rs = GetRegisteredSocket(sockid);
	if (!rs)
		return false;

	SIORequest* pReq = AllocIORequest();
	CAutoFreeIORequest freeReq(this, pReq);

	pReq->iort = eIORT_Connect;
	if (!ConvertAddr(addr, (DRXSOCKADDR*)pReq->addrBuffer, &pReq->addrLength))
		return false;
	LPFN_CONNECTEX lpfnConnectEx = NULL;
	GUID guidConnectEx = WSAID_CONNECTEX;
	DWORD dwBytes = 0;
	i32 ir = WSAIoctl(rs->sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidConnectEx, sizeof(guidConnectEx), &lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, NULL, NULL);
	if (DRX_SOCKET_ERROR == ir)
		return false;
	BOOL r = lpfnConnectEx(rs->sock, (DRXSOCKADDR*)pReq->addrBuffer, pReq->addrLength, NULL, 0, NULL, &pReq->overlapped);
	if (r)
	{
		freeReq.Release();
		return true;
	}
	else if (!r && ERROR_IO_PENDING == WSAGetLastError())
	{
		freeReq.Release();
		return true;
	}
	else
	{
		i32 err = WSAGetLastError();
		return false;
	}
	#else
	return false;
	#endif
}

bool CSocketIOUprIOCP::RequestAccept(SSocketID sockid)
{
	SCOPED_COMM_LOCK;

	CRecursionGuard rg(this);

	#if !DRX_PLATFORM_DURANGO
	SRegisteredSocket* rs = GetRegisteredSocket(sockid);
	if (!rs)
		return false;

	SIORequest* pReq = AllocIORequest();
	CAutoFreeIORequest freeReq(this, pReq);

	pReq->iort = eIORT_Accept;
	pReq->acceptSock = socket(AF_INET, SOCK_STREAM, 0);
	BOOL r = AcceptEx(rs->sock, pReq->acceptSock, pReq->dataBuffer, 0, 256, 256, &pReq->bytesReceived, &pReq->overlapped);
	if (r)
	{
		freeReq.Release();
		return true;
	}
	else if (!r && GetLastError() == ERROR_IO_PENDING)
	{
		freeReq.Release();
		return true;
	}
	else
		return false;
	#else
	return false;
	#endif
}

	#if USE_SYSTEM_ALLOCATOR
CSocketIOUprIOCP::SIORequest* CSocketIOUprIOCP::AllocIORequest()
{
	ASSERT_COMM_LOCK;

	return new SIORequest();
}

void CSocketIOUprIOCP::FreeIORequest(SIORequest* pReq)
{
	SCOPED_COMM_LOCK;

	delete pReq;
}
	#else
CSocketIOUprIOCP::SIORequest* CSocketIOUprIOCP::AllocIORequest()
{
	ASSERT_COMM_LOCK;

	if (!m_pAllocator)
		m_pAllocator = new TIORequestAllocator;

	return new(m_pAllocator->Allocate())SIORequest();
}

void CSocketIOUprIOCP::FreeIORequest(SIORequest* pReq)
{
	SCOPED_COMM_LOCK;

	pReq->~SIORequest();
	m_pAllocator->Deallocate(pReq);
}
	#endif

#endif
