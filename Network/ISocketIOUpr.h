// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ISOCKETIOMANAGER_H__
#define __ISOCKETIOMANAGER_H__

#pragma once

#include <drx3D/Network/NetAddress.h>
#include <drx3D/CoreX/Memory/STLGlobalAllocator.h>
#include <drx3D/Network/Config.h>
#include <drx3D/Network/SocketError.h>
#include <drx3D/Network/IDatagramSocket.h>
#include <drx3D/Network/DrxSocks.h>

// N.B. MAX_USER_MESSAGES_PER_FRAME *MUST* BE A POWER OF 2
#define MAX_USER_MESSAGES_PER_FRAME (4)
#define MAX_USER_MESSAGES_MASK      (MAX_USER_MESSAGES_PER_FRAME - 1)

struct SSocketID
{
	static u16k InvalidId = ~u16(0);

	SSocketID() : id(InvalidId), salt(0) {}
	SSocketID(u16 i, u16 s) : id(i), salt(s) {}

	u16 id;
	u16 salt;

	ILINE bool operator!() const
	{
		return id == InvalidId;
	}
	typedef u16 SSocketID::* safe_bool_idiom_type;
	ILINE operator safe_bool_idiom_type() const
	{
		return !!(*this) ? &SSocketID::id : NULL;
	}

	ILINE bool operator!=(const SSocketID& rhs) const
	{
		return !(*this == rhs);
	}
	ILINE bool operator==(const SSocketID& rhs) const
	{
		return id == rhs.id && salt == rhs.salt;
	}
	ILINE bool operator<(const SSocketID& rhs) const
	{
		return id < rhs.id || (id == rhs.id && salt < rhs.salt);
	}
	ILINE bool operator>(const SSocketID& rhs) const
	{
		return id > rhs.id || (id == rhs.id && salt > rhs.salt);
	}

	u32 AsInt() const
	{
		u32 x = id;
		x <<= 16;
		x |= salt;
		return x;
	}
	static SSocketID FromInt(u32 x)
	{
		return SSocketID(x >> 16, x & 0xffffu);
	}

	bool IsLegal() const
	{
		return salt != 0;
	}

	tukk GetText(tuk tmpBuf = 0) const
	{
		static char singlebuf[64];
		if (!tmpBuf)
		{
			tmpBuf = singlebuf;
		}
		if (id == InvalidId)
		{
			sprintf(tmpBuf, "<nil>");
		}
		else if (!salt)
		{
			sprintf(tmpBuf, "illegal:%u:%u", id, salt);
		}
		else
		{
			sprintf(tmpBuf, "%u:%u", id, salt);
		}
		return tmpBuf;
	}

	u32 GetAsUint32() const
	{
		return (u32(salt) << 16) | id;
	}
};

struct IRecvFromTarget
{
	virtual ~IRecvFromTarget(){}
	virtual void OnRecvFromComplete(const TNetAddress& from, u8k* pData, u32 len) = 0;
	virtual void OnRecvFromException(const TNetAddress& from, ESocketError err) = 0;
};

struct ISendToTarget
{
	virtual ~ISendToTarget(){}
	virtual void OnSendToException(const TNetAddress& to, ESocketError err) = 0;
};

struct IConnectTarget
{
	virtual ~IConnectTarget(){}
	virtual void OnConnectComplete() = 0;
	virtual void OnConnectException(ESocketError err) = 0;
};

struct IAcceptTarget
{
	virtual ~IAcceptTarget(){}
	virtual void OnAccept(const TNetAddress& from, DRXSOCKET sock) = 0;
	virtual void OnAcceptException(ESocketError err) = 0;
};

struct IRecvTarget
{
	virtual ~IRecvTarget(){}
	virtual void OnRecvComplete(u8k* pData, u32 len) = 0;
	virtual void OnRecvException(ESocketError err) = 0;
};

struct ISendTarget
{
	virtual ~ISendTarget(){}
	virtual void OnSendException(ESocketError err) = 0;
};

enum ESocketIOUprCaps
{
	eSIOMC_SupportsBackoff = BIT(0),
	eSIOMC_NoBuffering     = BIT(1)
};

struct ISocketIOUpr
{
	enum EMessages
	{
		// User messages
		eUM_FIRST = -1, // used to range check user messages

		eUM_QUIT  = eUM_FIRST,
		eUM_WAKE  = -2,
		eUM_SLEEP = -3,

		eUM_LAST  = -3, // used to range check user messages

		// System messages
		eSM_COMPLETEDIO               = 1,
		eSM_COMPLETE_SUCCESS_OK       = 2,
		eSM_COMPLETE_EMPTY_SUCCESS_OK = 3,
		eSM_COMPLETE_FAILURE_OK       = 4,

		// Error messages
		eEM_UNSPECIFIED     = -666,
		eEM_UNBOUND_SOCKET1 = -667,
		eEM_UNBOUND_SOCKET2 = -668,
		eEM_NO_TARGET       = -670,
		eEM_NO_REQUEST      = -671,
		eEM_REQUEST_DEAD    = -680
	};

	u32k caps;

	ISocketIOUpr(u32 c) : caps(c) {}
	virtual ~ISocketIOUpr() {}

	virtual tukk GetName() = 0;

	virtual bool        PollWait(u32 waitTime) = 0;
	virtual i32         PollWork(bool& performedWork) = 0;

	virtual SSocketID   RegisterSocket(DRXSOCKET sock, i32 protocol) = 0;
	virtual void        SetRecvFromTarget(SSocketID sockid, IRecvFromTarget* pTarget) = 0;
	virtual void        SetConnectTarget(SSocketID sockid, IConnectTarget* pTarget) = 0;
	virtual void        SetSendToTarget(SSocketID sockid, ISendToTarget* pTarget) = 0;
	virtual void        SetAcceptTarget(SSocketID sockid, IAcceptTarget* pTarget) = 0;
	virtual void        SetRecvTarget(SSocketID sockid, IRecvTarget* pTarget) = 0;
	virtual void        SetSendTarget(SSocketID sockid, ISendTarget* pTarget) = 0;
	virtual void        RegisterBackoffAddressForSocket(const TNetAddress& addr, SSocketID sockid) = 0;
	virtual void        UnregisterBackoffAddressForSocket(const TNetAddress& addr, SSocketID sockid) = 0;
	virtual void        UnregisterSocket(SSocketID sockid) = 0;

	virtual bool        RequestRecvFrom(SSocketID sockid) = 0;
	virtual bool        RequestSendTo(SSocketID sockid, const TNetAddress& addr, u8k* pData, size_t len) = 0;
	virtual bool        RequestSendVoiceTo(SSocketID sockid, const TNetAddress& addr, u8k* pData, size_t len) = 0;

	virtual bool        RequestConnect(SSocketID sockid, const TNetAddress& addr) = 0;
	virtual bool        RequestAccept(SSocketID sock) = 0;
	virtual bool        RequestSend(SSocketID sockid, u8k* pData, size_t len) = 0;
	virtual bool        RequestRecv(SSocketID sockid) = 0;

	virtual void        PushUserMessage(i32 msg) = 0;

	virtual bool        HasPendingData() = 0;

#if LOCK_NETWORK_FREQUENCY
	virtual void ForceNetworkStart() = 0;
	virtual bool NetworkSleep() = 0;
#endif

	virtual IDatagramSocketPtr CreateDatagramSocket(const TNetAddress& addr, u32 flags) = 0;
	virtual void               FreeDatagramSocket(IDatagramSocketPtr pSocket) = 0;
};

class CSocketIOUpr : public ISocketIOUpr
{
public:
	CSocketIOUpr(u32 c) : ISocketIOUpr(c)  {}

	virtual IDatagramSocketPtr CreateDatagramSocket(const TNetAddress& addr, u32 flags) override;
	virtual void               FreeDatagramSocket(IDatagramSocketPtr pSocket) override;

#if NET_MINI_PROFILE || NET_PROFILE_ENABLE
	void RecordPacketSendStatistics(u8k* pData, size_t len);
#endif

private:
	struct SDatagramSocketData
	{
		TNetAddress        addr;
		IDatagramSocketPtr pSocket;
		u32             flags;
		u32             refCount;
	};

	typedef std::vector<SDatagramSocketData, stl::STLGlobalAllocator<SDatagramSocketData>> TDatagramSockets;

	TDatagramSockets m_datagramSockets;
};

bool CreateSocketIOUpr(i32 ncpus, ISocketIOUpr** ppExternal, ISocketIOUpr** ppInternal);

#endif
