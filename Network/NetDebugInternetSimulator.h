// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __NETDEBUGINTERNETSIMULATOR_H__
#define __NETDEBUGINTERNETSIMULATOR_H__

#pragma once

#include <drx3D/Network/Config.h>

#if ENABLE_NET_DEBUG_INFO

	#include <drx3D/Network/NetDebugInfo.h>

class CNetDebugInternetSimulator : public CNetDebugInfoSection
{
public:
	CNetDebugInternetSimulator();
	virtual ~CNetDebugInternetSimulator();

	virtual void Tick(const SNetworkProfilingStats* const pProfilingStats);
	virtual void Draw(const SNetworkProfilingStats* const pProfilingStats);

private:

	SAccumulator<eBS_Lag> m_lagAverage;

	float                 m_packetLossRate;
	u32                m_packetSends;
	u32                m_packetDrops;
	u32                m_packetLagMin;
	u32                m_packetLagMax;
};

#endif // ENABLE_NET_DEBUG_INFO

#endif
