// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __INETWORKPRIVATE_H__
#define __INETWORKPRIVATE_H__

#pragma once

#include <drx3D/Network/Config.h>
#include <drx3D/Network/INetwork.h>
#include <drx3D/Network/INetworkMember.h>
#include <drx3D/Network/objcnt.h>
#include <drx3D/Network/NetCVars.h>
#include <drx3D/Network/ISocketIOUpr.h>
#include <drx3D/Network/NetResolver.h>

struct INetworkPrivate : public INetwork
{
public:
	virtual SObjectCounters&     GetObjectCounters() = 0;
	virtual ISocketIOUpr&    GetExternalSocketIOUpr() = 0;
	virtual ISocketIOUpr&    GetInternalSocketIOUpr() = 0;
	virtual CNetAddressResolver* GetResolver() = 0;
	virtual const CNetCVars&     GetCVars() = 0;
	virtual bool                 ConvertAddr(const TNetAddress& addrIn, DRXSOCKADDR_IN* pSockAddr) = 0;
	virtual bool                 ConvertAddr(const TNetAddress& addrIn, DRXSOCKADDR* pSockAddr, i32* addrLen) = 0;
	virtual u8                FrameIDToHeader(u8 id) = 0;
	virtual void BroadcastNetDump(ENetDumpType) = 0;
};

struct INetNubPrivate : public INetNub
{
public:
	virtual void         DisconnectChannel(EDisconnectionCause cause, DrxSessionHandle session, tukk reason) = 0;
	virtual INetChannel* GetChannelFromSessionHandle(DrxSessionHandle session) = 0;
};

#endif // __INETWORKPRIVATE_H__
