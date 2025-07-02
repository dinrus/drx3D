// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __NETVIS_H__
#define __NETVIS_H__

#pragma once

#include <drx3D/Network/Config.h>

#if ENABLE_DEBUG_KIT

	#include <drx3D/Network/Network.h>

class CClientContextView;

class CNetVis
{
public:
	CNetVis(CClientContextView* pCtxView) : m_pCtxView(pCtxView) {};
	void AddSample(SNetObjectID obj, u16 prop, float value);
	void Update();

private:
	struct SSample
	{
		SSample(CTimeValue w, float v) : when(w), value(v) {}
		SSample(float v) : when(g_time), value(v) {}
		SSample() : when(g_time), value(0) {}
		CTimeValue when;
		float      value;
	};

	struct SStatistic
	{
		SStatistic() : total(0), average(0) {}
		std::queue<SSample> samples;
		float               total;
		float               average;
	};

	struct SIndex
	{
		SIndex() : prop(10000) {}
		SIndex(SNetObjectID o, u16 p) : obj(o), prop(p) {}
		SNetObjectID obj;
		u16       prop;

		bool operator<(const SIndex& rhs) const
		{
			return prop < rhs.prop || (prop == rhs.prop && obj < rhs.obj);
		}
	};

	typedef std::map<SIndex, SStatistic> TStatsMap;
	TStatsMap           m_stats;
	CClientContextView* m_pCtxView;

	struct SMinMax
	{
		SMinMax() : minVal(1e12f), maxVal(0) {}
		float minVal;
		float maxVal;
	};
	std::map<u16, SMinMax> m_minmax;
};

#endif // ENABLE_DEBUG_KIT

#endif
