// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __NETDEBUGTRAFFICBANDWIDTH_H__
#define __NETDEBUGTRAFFICBANDWIDTH_H__

#pragma once

#include <drx3D/Network/Config.h>

#if ENABLE_NET_DEBUG_INFO

	#include <drx3D/Network/NetDebugInfo.h>

class CNetDebugTrafficBandwidth : public CNetDebugInfoSection
{
public:
	CNetDebugTrafficBandwidth();
	virtual ~CNetDebugTrafficBandwidth();

	virtual void Tick(const SNetworkProfilingStats* const pProfilingStats);
	virtual void Draw(const SNetworkProfilingStats* const pProfilingStats);

private:

	SBandwidthStats m_bandwidthStats;
};

#endif // ENABLE_NET_DEBUG_INFO

#endif
