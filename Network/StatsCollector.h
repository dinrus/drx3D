// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  helper class to write network statistics to a file
   -------------------------------------------------------------------------
   История:
   - 13/08/2004   09:29 : Created by Craig Tiller
*************************************************************************/
#ifndef __STATSCOLLECTOR_H__
#define __STATSCOLLECTOR_H__

#pragma once

#include <drx3D/Sys/TimeValue.h>
#include <drx3D/Network/Config.h>
#include <drx3D/Network/NetAddress.h>

#if STATS_COLLECTOR

class CStatsCollector
{
public:
	CStatsCollector(tukk filename = "netstats.log");
	~CStatsCollector();

	void BeginPacket(const TNetAddress& to, CTimeValue nTime, u32 id);
	void AddOverheadBits(tukk describe, float bits);
	void Message(tukk message, float bits);
	void BeginGroup(tukk name);
	void AddData(tukk name, float bits);
	void EndGroup();
	void EndPacket(u32 nSize);

	void LostPacket(u32 id);

	#if STATS_COLLECTOR_DEBUG_KIT
	void SetDebugKit(bool on) { m_debugKit = on; }
	#endif

	#if STATS_COLLECTOR_INTERACTIVE
	void InteractiveUpdate();
	#endif

private:
	#if STATS_COLLECTOR_FILE
	FILE * m_file;
	i32    m_fileId;
	string m_fileName;
	void WriteToLogFile(tukk format, ...);
	void CreateLogFile();
	#endif
	#if STATS_COLLECTOR_DEBUG_KIT
	bool m_debugKit;
	#endif

	#if STATS_COLLECTOR_INTERACTIVE
	std::vector<u32> m_components;
	string m_name;

	struct SStats
	{
		SStats() : last(0), total(0), mx(0), mn(1e12f), count(0), average(0) {}
		void Add(float);
		CTimeValue lastUpdate;
		float      total;
		float      mx;
		float      mn;
		float      last;
		float      average;
		i32        count;
	};
	std::map<string, SStats> m_stats;

	void DrawLine(i32 line, tukk fmt, ...);
	#endif
};

#else // !_STATS_COLLECTOR

class CStatsCollector
{
public:
	ILINE CStatsCollector(tukk filename = "") {}
	ILINE void BeginPacket(const TNetAddress& to, CTimeValue nTime, u32 id) {}
	ILINE void AddOverheadBits(tukk describe, float bits)               {}
	ILINE void Message(tukk message, float bits)                        {}
	ILINE void BeginGroup(tukk name)                                    {}
	ILINE void AddData(tukk name, float bits)                           {}
	ILINE void EndGroup()                                                      {}
	ILINE void EndPacket(u32 nSize)                                         {}
	ILINE void LostPacket(u32 id)                                           {}
	ILINE void SetDebugKit(bool)                                               {}
};

#endif // _STATS_COLLECTOR

#endif
