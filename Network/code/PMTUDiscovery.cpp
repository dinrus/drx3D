// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/PMTUDiscovery.h>
#include  <drx3D/Network/Config.h>
#include  <drx3D/Network/Network.h>

// this list of values MUST be kept in ascending sorted order
// originally from RFC 1191
static u16k LikelyPMTUs[] = {
	68,  // minimum mtu - spec
	296,
	508,
	576,
	1006,
	1492,
	2002,
	4352,
	8166,
	17914,
	//	32000,
	//	65535, // maximum mtu - spec
};

static i32k NumLikelyPMTUs = sizeof(LikelyPMTUs) / sizeof(*LikelyPMTUs);

static const CTimeValue SHORT_TIMEOUT = 15.0f;
static const CTimeValue LONG_TIMEOUT = (10.0f * 60.0f); // ten minutes

static i32 MTUIndexForSize(u16 size)
{
	//return std::lower_bound( LikelyPMTUs, LikelyPMTUs+NumLikelyPMTUs, size ) - LikelyPMTUs;
	i32 index = (i32)(std::lower_bound(LikelyPMTUs, LikelyPMTUs + NumLikelyPMTUs, size) - LikelyPMTUs);
	return index - (size < LikelyPMTUs[index]); // size <= LikelyPMTUs[index]
}

CPMTUDiscovery::CPMTUDiscovery() : m_pmtu(1300), m_peak(0), m_last(0), m_lastExperiment(0.0f), m_lastExperimentRequest(0.0f), m_bInExperiment(true)
{
}

u16 CPMTUDiscovery::GetMaxPacketSize(CTimeValue curTime)
{
	//return 1300;

	if (CNetCVars::Get().MaxPacketSize != 0)
		return CNetCVars::Get().MaxPacketSize;

	m_bInExperiment |= (curTime - m_lastExperiment) > LONG_TIMEOUT;

	u16 report;

	if (m_bInExperiment && (curTime - m_lastExperimentRequest) > SHORT_TIMEOUT)
	{
		i32 index = MTUIndexForSize(m_pmtu);
		index += (index < NumLikelyPMTUs);
		assert(index >= 0 && index < NumLikelyPMTUs);
		PREFAST_ASSUME(index >= 0 && index < NumLikelyPMTUs);
		report = LikelyPMTUs[index];
		m_lastExperimentRequest = curTime;
		// NetLog("[pmtu] next experiment new_pmtu[%d] - pmtu[%d], peak[%d], last[%d]", report, m_pmtu, m_peak, m_last);
	}
	else
	{
		report = m_pmtu;
	}

	return report - UDP_HEADER_SIZE;
}

void CPMTUDiscovery::SentPacket(CTimeValue curTime, u32 seq, u16 sz)
{
	if (sz < (65535 - UDP_HEADER_SIZE))
		sz += UDP_HEADER_SIZE;
	else
		sz = 65535;
	m_last = sz;
	if (sz > m_peak)
	{
		m_peak = sz;
		if (CVARS.LogLevel > 1)
			NetLog("[pmtu] peak sent packet size[%d]", sz);
	}
	if (sz > m_pmtu)
	{
		if (CVARS.LogLevel > 1)
			NetLog("[pmtu] experiment packet[seq=%d] sz[%d] - pmtu[%d], peak[%d], last[%d]", seq, sz, m_pmtu, m_peak, m_last);
		m_bInExperiment = false;
		m_experiments[seq] = sz;
		m_lastExperiment = curTime;
	}

	while (!m_experiments.empty() && (g_time - m_experiments.begin()->second.when).GetSeconds() > 1.0f)
	{
		AckedPacket(g_time, m_experiments.begin()->first, false);
	}
}

void CPMTUDiscovery::AckedPacket(CTimeValue curTime, u32 seq, bool ack)
{
	if (ack)
	{
		TExperimentMap::const_iterator iter = m_experiments.find(seq);
		if (iter != m_experiments.end())
		{
			if (iter->second.sz > m_pmtu)
			{
				u16 last = m_pmtu;
				m_pmtu = iter->second.sz;
				if (CVARS.LogLevel > 1)
					NetLog("[pmtu] INCREASED to [%d] from [%d]", m_pmtu, last);
			}
			m_bInExperiment = true; // immediately call for a new experiment!
		}
	}

	m_experiments.erase(m_experiments.begin(), m_experiments.upper_bound(seq));
}

void CPMTUDiscovery::FragmentedPacket(CTimeValue curTime)
{
	if (m_experiments.empty())
	{
		u16 last = m_pmtu;
		i32 index = MTUIndexForSize(m_pmtu);
		//index -= !!index;
		assert(index >= 0 && index < DRX_ARRAY_COUNT(LikelyPMTUs));
		index -= (index && m_pmtu == LikelyPMTUs[index]); // m_pmtu >= LikelyPMTUs[index]
		assert(index >= 0 && index < NumLikelyPMTUs);
		m_pmtu = LikelyPMTUs[index];
		m_bInExperiment = false;
		if (CVARS.LogLevel > 1)
			NetLog("[pmtu] DROPPED to [%d] from [%d]", m_pmtu, last);
	}
}
