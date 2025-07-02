// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __NETDEBUGSERVERINFO_H__
#define __NETDEBUGSERVERINFO_H__

#pragma once

#include <drx3D/Network/Config.h>

#if ENABLE_NET_DEBUG_INFO

	#include <drx3D/Network/NetDebugInfo.h>

class CNetDebugServerInfo : public CNetDebugInfoSection, public INetDumpLogger
{
public:
	CNetDebugServerInfo();
	virtual ~CNetDebugServerInfo();

	// CNetDebugInfoSection
	virtual void Tick(const SNetworkProfilingStats* const pProfilingStats);
	virtual void Draw(const SNetworkProfilingStats* const pProfilingStats);
	// ~CNetDebugInfoSection

	// INetDumpLogger
	virtual void Log(tukk pFmt, ...);
	//~INetDumpLogger

private:

	u32 m_line;
};

#endif // ENABLE_NET_DEBUG_INFO

#endif
