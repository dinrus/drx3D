// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __TCPSTREAMSOCKET_H__
#define __TCPSTREAMSOCKET_H__

#pragma once

#include <drx3D/Network/IStreamSocket.h>

class CTCPStreamSocket;

//struct SListenVisitor
//{
//	explicit SListenVisitor(CTCPStreamSocket* pStreamSocket) : m_pStreamSocket(pStreamSocket), m_result(false)
//	{
//		// do nothing
//	}
//
//	~SListenVisitor()
//	{
//		m_pStreamSocket = NULL;
//	}
//
//	template<typename T>
//	ILINE void Visit(const T&)
//	{
//		NetWarning("unsupported address type for CTCPStreamSocket::Listen");
//	}
//
//	void Visit(const SIPv4Addr& addr);
//
//	// add visit methods for other address types (e.g. SIPv6Addr)
//
//	CTCPStreamSocket* m_pStreamSocket;
//	bool m_result;
//};
//
//struct SConnectVisitor
//{
//	explicit SConnectVisitor(CTCPStreamSocket* pStreamSocket) : m_pStreamSocket(pStreamSocket), m_result(false)
//	{
//		// do nothing
//	}
//
//	~SConnectVisitor()
//	{
//		m_pStreamSocket = NULL;
//	}
//
//	template<typename T>
//	ILINE void Visit(const T&)
//	{
//		NetWarning("unsupported address type for CTCPStreamSocket::Connect");
//	}
//
//	void Visit(const SIPv4Addr& addr);
//
//	// add visit methods for other address types (e.g. SIPv6Addr)
//
//	CTCPStreamSocket* m_pStreamSocket;
//	bool m_result;
//};

class CTCPStreamSocket : public IStreamSocket, private IConnectTarget, private IAcceptTarget, private IRecvTarget, private ISendTarget
{
	friend struct SListenVisitor;
	friend struct SConnectVisitor;

public:
	CTCPStreamSocket();
	~CTCPStreamSocket();

	bool Init();

	void SetListener(IStreamListener* pListener, uk pUserData);
	bool Listen(const TNetAddress& addr);
	bool Connect(const TNetAddress& addr);
	bool Send(u8k* pBuffer, size_t nLength);
	void GetPeerAddr(TNetAddress& addr);
	void Shutdown();
	void Close();
	bool IsDead();

private:
#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	DRXSOCKET m_socket;
#endif

	SSocketID m_sockid;
	ISocketIOUpr* m_pSockIO;

	enum ESocketState {eSS_Closed, eSS_Listening, eSS_Connecting, eSS_Established};
	void SetSocketState(ESocketState state);
	ESocketState     m_socketState;

	IStreamListener* m_pListener;
	uk            m_pListenerUserData;

	// IConnectTarget
	virtual void OnConnectComplete();
	virtual void OnConnectException(ESocketError err);
	// ~IConnectTarget

	// IAcceptTarget
	virtual void OnAccept(const TNetAddress& from, DRXSOCKET sock);
	virtual void OnAcceptException(ESocketError err);
	// ~IAcceptTarget

	// IRecvTarget
	virtual void OnRecvComplete(u8k* pData, u32 len);
	virtual void OnRecvException(ESocketError err);
	// ~IRecvTarget

	// ISendTarget
	virtual void OnSendException(ESocketError err);
	// ~ISendTarget

	void Cleanup();
};

#endif
