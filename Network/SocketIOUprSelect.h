// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SOCKETIOMANAGERSELECT_H__
#define __SOCKETIOMANAGERSELECT_H__

#pragma once

#include <drx3D/Network/Config.h>

#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_ORBIS || DRX_PLATFORM_APPLE
	#define HAS_SOCKETIOMANAGER_SELECT
#endif

#if defined(HAS_SOCKETIOMANAGER_SELECT)

	#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE || DRX_PLATFORM_ORBIS
		#define IPROTO_UDPP2P_SAFE 0      // clashes with IPROTO_IP
		#define UDPP2P_VPORT       1
	#endif

	#include <drx3D/Network/ISocketIOUpr.h>
	#include <drx3D/CoreX/Memory/STLPoolAllocator.h>
	#include <drx3D/Network/WatchdogTimer.h>
	#include <drx3D/CoreX/Memory/STLGlobalAllocator.h>
	#include <drx3D/Network/DrxSocks.h>

class CSocketIOUprSelect : public CSocketIOUpr
{
public:
	virtual tukk GetName() override { return "Select"; }

	bool                Init();
	CSocketIOUprSelect();
	~CSocketIOUprSelect();

	virtual bool      PollWait(u32 waitTime) override;
	virtual i32       PollWork(bool& performedWork) override;

	virtual SSocketID RegisterSocket(DRXSOCKET sock, i32 protocol) override;
	virtual void      SetRecvFromTarget(SSocketID sockid, IRecvFromTarget* pTarget) override;
	virtual void      SetConnectTarget(SSocketID sockid, IConnectTarget* pTarget) override;
	virtual void      SetSendToTarget(SSocketID sockid, ISendToTarget* pTarget) override;
	virtual void      SetAcceptTarget(SSocketID sockid, IAcceptTarget* pTarget) override;
	virtual void      SetRecvTarget(SSocketID sockid, IRecvTarget* pTarget) override;
	virtual void      SetSendTarget(SSocketID sockid, ISendTarget* pTarget) override;
	virtual void      RegisterBackoffAddressForSocket(const TNetAddress& addr, SSocketID sockid) override;
	virtual void      UnregisterBackoffAddressForSocket(const TNetAddress& addr, SSocketID sockid) override;
	virtual void      UnregisterSocket(SSocketID sockid) override;

	virtual bool      RequestRecvFrom(SSocketID sockid) override;
	virtual bool      RequestSendTo(SSocketID sockid, const TNetAddress& addr, u8k* pData, size_t len) override;
	virtual bool      RequestSendVoiceTo(SSocketID sockid, const TNetAddress& addr, u8k* pData, size_t len) override;

	virtual bool      RequestConnect(SSocketID sockid, const TNetAddress& addr) override;
	virtual bool      RequestAccept(SSocketID sock) override;
	virtual bool      RequestSend(SSocketID sockid, u8k* pData, size_t len) override;
	virtual bool      RequestRecv(SSocketID sockid) override;

	virtual void      PushUserMessage(i32 msg) override;

	virtual bool      HasPendingData() override { return (m_pWatchdog && m_pWatchdog->HasStalled()); }

	#if LOCK_NETWORK_FREQUENCY
	virtual void ForceNetworkStart() override { ++m_userMessageFrameID; }
	virtual bool NetworkSleep() override      { return true; }
	#endif

private:
	CWatchdogTimer* m_pWatchdog;

	struct SOutgoingData
	{
		i32   nLength;
		u8 data[MAX_UDP_PACKET_SIZE];
	};
	#if USE_SYSTEM_ALLOCATOR
	typedef std::list<SOutgoingData>                                                                                        TOutgoingDataList;
	#else
	typedef std::list<SOutgoingData, stl::STLPoolAllocator<SOutgoingData, stl::PoolAllocatorSynchronizationSinglethreaded>> TOutgoingDataList;
	#endif
	struct SOutgoingAddressedData : public SOutgoingData
	{
		TNetAddress addr;
	};
	#if USE_SYSTEM_ALLOCATOR
	typedef std::list<SOutgoingAddressedData>                                                                                                 TOutgoingAddressedDataList;
	#else
	typedef std::list<SOutgoingAddressedData, stl::STLPoolAllocator<SOutgoingAddressedData, stl::PoolAllocatorSynchronizationSinglethreaded>> TOutgoingAddressedDataList;
	#endif

	struct SSocketInfo
	{
		SSocketInfo()
		{
			salt = 1;
			isActive = false;
			sock = DRX_INVALID_SOCKET;
			nRecvFrom = nRecv = nListen = 0;
			pRecvFromTarget = NULL;
			pSendToTarget = NULL;
			pConnectTarget = NULL;
			pAcceptTarget = NULL;
			pRecvTarget = NULL;
			pSendTarget = NULL;
		}

		u16                     salt;
		bool                       isActive;
		DRXSOCKET                  sock;
		i32                        nRecvFrom;
		i32                        nRecv;
		i32                        nListen;
		TOutgoingDataList          outgoing;
		TOutgoingAddressedDataList outgoingAddressed;

		IRecvFromTarget*           pRecvFromTarget;
		ISendToTarget*             pSendToTarget;
		IConnectTarget*            pConnectTarget;
		IAcceptTarget*             pAcceptTarget;
		IRecvTarget*               pRecvTarget;
		ISendTarget*               pSendTarget;

		i32                      protocol;

		bool NeedRead() const  { return nRecv || nRecvFrom || nListen; }
		bool NeedWrite() const { return !outgoing.empty() || !outgoingAddressed.empty(); }
	};
	std::vector<SSocketInfo*, stl::STLGlobalAllocator<SSocketInfo*>> m_socketInfo;

	SSocketInfo* GetSocketInfo(SSocketID id);
	void         WakeUp();

	DRXSOCKADDR_IN m_wakeupAddr;
	DRXSOCKET      m_wakeupSocket;
	DRXSOCKET      m_wakeupSender;

	#if LOCK_NETWORK_FREQUENCY
	 u32 m_userMessageFrameID;
	#endif // LOCK_NETWORK_FREQUENCY
	struct SUserMessage
	{
		i32    m_message;
	#if LOCK_NETWORK_FREQUENCY
		u32 m_frameID;
	#endif // LOCK_NETWORK_FREQUENCY
	};

	fd_set    m_read;
	fd_set    m_write;
	DRXSOCKET m_fdmax;
};

#endif

#endif
