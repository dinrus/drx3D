// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ISTREAMSOCKET_H__
#define __ISTREAMSOCKET_H__

#pragma once

#include <drx3D/Network/NetAddress.h>
#include <drx3D/Sys/TimeValue.h>
#include <drx3D/Network/ISocketIOUpr.h>

#include <drx3D/Network/SocketError.h>

struct IStreamSocket;

TYPEDEF_AUTOPTR(IStreamSocket);
typedef IStreamSocket_AutoPtr IStreamSocketPtr;

struct IStreamListener
{
	virtual ~IStreamListener(){}
	virtual void OnConnectionAccepted(IStreamSocketPtr pStreamSocket, uk pUserData) = 0;
	virtual void OnConnectCompleted(bool succeeded, uk pUserData) = 0;
	virtual void OnConnectionClosed(bool graceful, uk pUserData) = 0;
	virtual void OnIncomingData(u8k* pData, size_t nSize, uk pUserData) = 0;
};

struct IStreamSocket : public CMultiThreadRefCount
{
	virtual void SetListener(IStreamListener* pListener, uk pUserData = NULL) = 0;
	virtual bool Listen(const TNetAddress& addr) = 0;
	virtual bool Connect(const TNetAddress& addr) = 0;
	virtual bool Send(u8k* pBuffer, size_t nLength) = 0;
	virtual void GetPeerAddr(TNetAddress& addr) = 0;
	virtual void Shutdown() = 0;
	virtual void Close() = 0;
	virtual bool IsDead() = 0;
};

extern IStreamSocketPtr CreateStreamSocket(const TNetAddress& addr);

#endif
