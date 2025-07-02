// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

namespace pfx2
{


ILINE void CParticleProfiler::AddEntry(const CParticleComponentRuntime& runtime, EProfileStat type, uint value)
{
#if !defined(_RELEASE)
	if (!IsEnabled())
		return;
	u32k threadId = JobUpr::GetWorkerThreadId();
	SEntry entry { &runtime, type, value };
	m_entries[threadId + 1].push_back(entry);
#endif
}

ILINE CTimeProfiler::CTimeProfiler(CParticleProfiler& profiler, const CParticleComponentRuntime& runtime, EProfileStat stat)
	: m_profiler(profiler)
	, m_runtime(runtime)
#if !defined(_RELEASE)
	, m_stat(stat)
	, m_startTicks(DrxGetTicks())
#endif
{
}

ILINE CTimeProfiler::~CTimeProfiler()
{
#if !defined(_RELEASE)
	const int64 endTicks = DrxGetTicks();
	const uint time = uint(gEnv->pTimer->TicksToSeconds(endTicks - m_startTicks) * 1000000.0f);
	m_profiler.AddEntry(m_runtime, m_stat, time);
	m_profiler.AddEntry(m_runtime, EPS_TotalTiming, time);
#endif
}
}
