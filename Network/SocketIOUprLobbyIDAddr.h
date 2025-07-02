// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SOCKETIOMANAGERLOBBYIDADDR_H__
#define __SOCKETIOMANAGERLOBBYIDADDR_H__

#pragma once

#if USE_LOBBYIDADDR
	#define HAS_SOCKETIOMANAGER_LOBBYIDADDR
#endif // USE_LOBBYIDADDR

#if defined(HAS_SOCKETIOMANAGER_LOBBYIDADDR)

	#include <drx3D/Network/SocketError.h>
	#include <drx3D/CoreX/Memory/PoolAllocator.h>
	#include <drx3D/Network/ISocketIOUpr.h>
	#include <drx3D/Network/Network.h>

class CSocketIOUprLobbyIDAddr : public CSocketIOUpr
{
public:
	CSocketIOUprLobbyIDAddr();
	~CSocketIOUprLobbyIDAddr();
	bool        Init();
	tukk GetName() override { return "LobbyIDAddr"; }
	bool        PollWait(u32 waitTime) override;
	i32         PollWork(bool& performedWork) override;
	static void RecvPacket(uk privateRef, u8* recvBuffer, u32 recvSize, DRXSOCKET recvSocket, TNetAddress& recvAddr);

	SSocketID   RegisterSocket(DRXSOCKET sock, i32 protocol) override;
	void        SetRecvFromTarget(SSocketID sockid, IRecvFromTarget* pTarget) override;
	void        SetSendToTarget(SSocketID sockid, ISendToTarget* pTarget) override;
	void        SetConnectTarget(SSocketID sockid, IConnectTarget* pTarget) override;
	void        SetAcceptTarget(SSocketID sockid, IAcceptTarget* pTarget) override;
	void        SetRecvTarget(SSocketID sockid, IRecvTarget* pTarget) override;
	void        SetSendTarget(SSocketID sockid, ISendTarget* pTarget) override;
	void        RegisterBackoffAddressForSocket(TNetAddress addr, SSocketID sockid) override;
	void        UnregisterBackoffAddressForSocket(TNetAddress addr, SSocketID sockid) override;
	void        UnregisterSocket(SSocketID sockid) override;

	bool        RequestRecvFrom(SSocketID sockid) override;
	bool        RequestSendTo(SSocketID sockid, const TNetAddress& addr, u8k* pData, size_t len) override;
	bool        RequestSendVoiceTo(SSocketID sockid, const TNetAddress& addr, u8k* pData, size_t len) override;

	bool        RequestConnect(SSocketID sockid, const TNetAddress& addr) override;
	bool        RequestAccept(SSocketID sockid) override;
	bool        RequestSend(SSocketID sockid, u8k* pData, size_t len) override;
	bool        RequestRecv(SSocketID sockid) override;

	void        PushUserMessage(i32 msg) override {}

	bool        HasPendingData() override         { return false; }

	#if LOCK_NETWORK_FREQUENCY
	virtual void ForceNetworkStart() override {}
	virtual bool NetworkSleep() override      { return true; }
	#endif

private:
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
	};
	typedef std::vector<SRegisteredSocket> TRegisteredSockets;
	TRegisteredSockets m_registeredSockets;

	SRegisteredSocket* GetRegisteredSocket(SSocketID sockid);

	u8       m_recvBuffer[MAX_UDP_PACKET_SIZE];
	TNetAddress m_recvAddr;
	u32      m_recvSize;
	DRXSOCKET   m_recvSocket;
};

#endif // defined(HAS_SOCKETIOMANAGER_LOBBYIDADDR)

#endif // __SOCKETIOMANAGERLOBBYIDADDR_H__
