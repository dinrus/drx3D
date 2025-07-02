// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __INUBMEMBER_H__
#define __INUBMEMBER_H__

#pragma once

#include <drx3D/Network/MementoMemoryUpr.h>
#include <drx3D/Network/FrameTypes.h>

class CNetChannel;

struct INetDumpLogger
{
	virtual ~INetDumpLogger() {}
	virtual void Log(tukk pFmt, ...) = 0;
};

struct INubMember : public CMultiThreadRefCount
{
	// conversion to a channel (may return NULL!)
	virtual CNetChannel* GetNetChannel() = 0;
	// is this item dead?
	virtual bool         IsDead() = 0;
	// is this item wanting to die?
	virtual bool         IsSuicidal() = 0;
	// should this item die?
	virtual void         Die(EDisconnectionCause cause, string msg) = 0;
	virtual void         RemovedFromNub() = 0;
	// process a packet
	virtual void         ProcessPacket(bool inSync, u8k* pData, u32 nLength) = 0;
	virtual void         ProcessSimplePacket(EHeaders hdr) = 0;
	// house keeping
	virtual void         PerformRegularCleanup() = 0;
	virtual void         SyncWithGame(ENetworkGameSync type) = 0;
	virtual void         NetDump(ENetDumpType type, INetDumpLogger& logger) = 0;

	virtual void         GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false) = 0;
};
typedef _smart_ptr<INubMember> INubMemberPtr;

#endif
