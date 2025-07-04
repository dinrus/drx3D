// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SOCKETIOMANAGERIOCP_H__
#define __SOCKETIOMANAGERIOCP_H__

#pragma once

#if DRX_PLATFORM_WINDOWS
	#define HAS_SOCKETIOMANAGER_IOCP
#endif

#if defined(HAS_SOCKETIOMANAGER_IOCP)

	#include <drx3D/Network/SocketError.h>
	#include <drx3D/CoreX/Memory/PoolAllocator.h>
	#include <drx3D/Network/ISocketIOUpr.h>
	#include <drx3D/Network/Network.h>
	#include <drx3D/CoreX/Math/Random.h>

class CSocketIOUprIOCP : public CSocketIOUpr
{
public:
	CSocketIOUprIOCP();
	~CSocketIOUprIOCP();
	bool        Init();
	tukk GetName() override { return "iocp"; }
	bool        PollWait(u32 waitTime) override;
	i32         PollWork(bool& performedWork) override;

	SSocketID   RegisterSocket(DRXSOCKET sock, i32 protocol) override;
	void        SetRecvFromTarget(SSocketID sockid, IRecvFromTarget* pTarget) override;
	void        SetSendToTarget(SSocketID sockid, ISendToTarget* pTarget) override;
	void        SetConnectTarget(SSocketID sockid, IConnectTarget* pTarget) override;
	void        SetAcceptTarget(SSocketID sockid, IAcceptTarget* pTarget) override;
	void        SetRecvTarget(SSocketID sockid, IRecvTarget* pTarget) override;
	void        SetSendTarget(SSocketID sockid, ISendTarget* pTarget) override;
	void        RegisterBackoffAddressForSocket(const TNetAddress& addr, SSocketID sockid) override;
	void        UnregisterBackoffAddressForSocket(const TNetAddress& addr, SSocketID sockid) override;
	void        UnregisterSocket(SSocketID sockid) override;

	bool        RequestRecvFrom(SSocketID sockid) override;
	bool        RequestSendTo(SSocketID sockid, const TNetAddress& addr, u8k* pData, size_t len) override;
	bool        RequestSendVoiceTo(SSocketID sockid, const TNetAddress& addr, u8k* pData, size_t len) override;

	bool        RequestConnect(SSocketID sockid, const TNetAddress& addr) override;
	bool        RequestAccept(SSocketID sockid) override;
	bool        RequestSend(SSocketID sockid, u8k* pData, size_t len) override;
	bool        RequestRecv(SSocketID sockid) override;

	void        PushUserMessage(i32 msg) override;

	bool        HasPendingData() override { return m_pendingData || (m_pWatchdog && m_pWatchdog->HasStalled()); }

	#if LOCK_NETWORK_FREQUENCY
	virtual void ForceNetworkStart() override { ++m_userMessageFrameID; m_wakeUpSema.Set(); }
	virtual bool NetworkSleep() override      { m_wakeUpSema.Reset(); return m_wakeUpSema.Wait(MAX_WAIT_TIME_FOR_SEMAPHORE); }
	#endif

private:
	HANDLE           m_iocp;
	i32              m_recursionDepth;
	bool             m_pendingData;
	static i32k MAX_RECURSION_DEPTH = 8;
	#if LOCK_NETWORK_FREQUENCY
	 u32  m_userMessageFrameID;
	#endif // LOCK_NETWORK_FREQUENCY

	CWatchdogTimer* m_pWatchdog;

	#if LOCK_NETWORK_FREQUENCY
	DrxEvent m_wakeUpSema;
	#endif

	struct SCompletedIO
	{
		BOOL           bSuccess;
		DWORD          nIOSize;
		i32            sockid;
		WSAOVERLAPPED* pOverlapped;
		DWORD          err;
	}                 m_cio;
	typedef std::queue<SCompletedIO> TCompletedIOQueue;
	TCompletedIOQueue m_completedIO;

	i32 PollInnerLoop(i32& retval);
	i32 PollInner(const SCompletedIO& io);

	class CRecursionGuard
	{
	public:
		CRecursionGuard(CSocketIOUprIOCP* pMgr) : m_pMgr(pMgr)
		{
			NET_ASSERT(m_pMgr->m_recursionDepth >= 0);
			NET_ASSERT(m_pMgr->m_recursionDepth < MAX_RECURSION_DEPTH);
			m_pMgr->m_recursionDepth++;
		}
		~CRecursionGuard()
		{
			m_pMgr->m_recursionDepth--;
			NET_ASSERT(m_pMgr->m_recursionDepth >= 0);
			NET_ASSERT(m_pMgr->m_recursionDepth < MAX_RECURSION_DEPTH);
		}

		bool RecursionAllowed() const
		{
			return m_pMgr->m_recursionDepth < MAX_RECURSION_DEPTH;
		}

	private:
		CSocketIOUprIOCP* m_pMgr;
	};

	enum EIORequestType
	{
		eIORT_Dead,
		eIORT_RecvFrom,
		eIORT_RecvFromX,
		eIORT_SendTo,
		eIORT_Accept,
		eIORT_Connect,
		eIORT_Recv,
		eIORT_Send,
		eIORT_UserMessage,
		eIORT_SomewhatDone
	};

	struct SBackoffTarget
	{
		SBackoffTarget() : lastBackoffSend(0.0f), lastSystemSend(0.0f) {}
		CTimeValue lastBackoffSend;
		CTimeValue lastSystemSend;
		CTimeValue lastKeepAliveSend;
	};

	typedef VectorMap<TNetAddress, SBackoffTarget> TBackoffTargets;

	struct SRegisteredSocket
	{
		SRegisteredSocket(u16 saltValue) :
			sock(DRX_INVALID_SOCKET),
			pRecvFromTarget(0),
			pSendToTarget(0),
			pConnectTarget(0),
			pAcceptTarget(0),
			pRecvTarget(0),
			pSendTarget(0),
			salt(saltValue),
			inUse(false)
		{
		}
		bool             inUse;
		DRXSOCKET        sock;
		u16           salt;
		IRecvFromTarget* pRecvFromTarget;
		ISendToTarget*   pSendToTarget;
		IConnectTarget*  pConnectTarget;
		IAcceptTarget*   pAcceptTarget;
		IRecvTarget*     pRecvTarget;
		ISendTarget*     pSendTarget;

		TBackoffTargets  backoffTargets;
	};
	typedef std::vector<SRegisteredSocket> TRegisteredSockets;
	TRegisteredSockets m_registeredSockets;

	struct SIORequest
	{
		SIORequest()
		{
			wsaBuffer[0].buf = (tuk)dataBuffer;
			wsaBuffer[0].len = sizeof(dataBuffer);
			overlapped.Internal = 0;
			overlapped.InternalHigh = 0;
			overlapped.Offset = 0;
			overlapped.OffsetHigh = 0;
			overlapped.hEvent = NULL;
			addrLength = sizeof(addrBuffer);
			acceptSock = DRX_INVALID_SOCKET;
			bytesReceived = 0;
			flags = 0;
			iort = eIORT_Dead;
			++g_objcnt.ioreq;
		}
		~SIORequest()
		{
			--g_objcnt.ioreq;
		}
		// overlapped *must* be the first element of this struct, and this struct cannot be derived from anything
		// code that uses it assumes that pointers to SIORequest::overlapped and the SIORequest itself are exactly the same
		WSAOVERLAPPED  overlapped;
		EIORequestType iort;
		u8          addrBuffer[_SS_MAXSIZE];
		u8          dataBuffer[MAX_UDP_PACKET_SIZE];
		WSABUF         wsaBuffer[1];
		DWORD          bytesReceived;
		DWORD          flags;
		INT            addrLength;
		DRXSOCKET      acceptSock;
	};

	#if !USE_SYSTEM_ALLOCATOR
	typedef stl::PoolAllocator<sizeof(SIORequest)> TIORequestAllocator;
	static TIORequestAllocator* m_pAllocator;
	#endif

	SRegisteredSocket* GetRegisteredSocket(SSocketID sockid);

	SIORequest*        AllocIORequest();
	void               FreeIORequest(SIORequest*);
	class CAutoFreeIORequest;

	CMTRand_int32 m_rand;
	CTimeValue    m_lastBackoffCheck;

	i32  CompleteSuccess(SRegisteredSocket* pSock, SIORequest* pReq, DWORD nIOSize);
	i32  CompleteEmptySuccess(SRegisteredSocket* pSock, SIORequest* pReq);
	i32  CompleteFailure(SRegisteredSocket* pSock, SIORequest* pReq, DWORD err);

	void CheckBackoff();
	void BackoffDueToRequest(SRegisteredSocket* pSock, SIORequest* pReq);
	void BackoffAll();
	bool DoRequestSendTo(SRegisteredSocket* pSock, const TNetAddress& addr, u8k* pData, size_t len, CTimeValue now);
};

#endif

#endif
