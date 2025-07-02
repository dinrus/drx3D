// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PMTUDISCOVERY_H__
#define __PMTUDISCOVERY_H__

#pragma once

#include <drx3D/Sys/TimeValue.h>
#include <drx3D/Network/Network.h>

class CPMTUDiscovery
{
public:
	CPMTUDiscovery();

	u16 GetMaxPacketSize(CTimeValue curTime);
	void   SentPacket(CTimeValue curTime, u32 seq, u16 sz);
	void   AckedPacket(CTimeValue curTime, u32 seq, bool ack);
	void   FragmentedPacket(CTimeValue curTime);
	void   GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false)
	{
		SIZER_COMPONENT_NAME(pSizer, "CPMTUDiscovery");

		if (countingThis)
			pSizer->Add(*this);
		pSizer->AddContainer(m_experiments);
	}

private:
	u16     m_pmtu;
	u16     m_peak;
	u16     m_last;
	CTimeValue m_lastExperiment;
	CTimeValue m_lastExperimentRequest;
	bool       m_bInExperiment;

	struct SExperiment
	{
		u16     sz;
		CTimeValue when;

		SExperiment(u16 s = 0) : sz(s), when(g_time) {}
	};
	typedef std::map<u32, SExperiment> TExperimentMap;
	TExperimentMap m_experiments;
};

#endif
