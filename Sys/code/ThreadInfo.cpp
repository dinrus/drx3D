// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/ThreadInfo.h>
#include <drx3D/Sys/System.h>
#include <drx3D/CoreX/Thread/IThreadUpr.h>
#include <drx3D/CoreX/Platform/DrxWindows.h>

#if defined(ENABLE_PROFILING_CODE)

////////////////////////////////////////////////////////////////////////////
namespace
{
//////////////////////////////////////////////////////////////////////////
void GetThreadInfo(threadID nThreadId, uk pData)
{
	SThreadInfo::TThreadInfo* threadsOut = (SThreadInfo::TThreadInfo*)pData;
	(*threadsOut)[nThreadId] = gEnv->pThreadUpr->GetThreadName(nThreadId);
}
}

////////////////////////////////////////////////////////////////////////////
void SThreadInfo::GetCurrentThreads(TThreadInfo& threadsOut)
{
	GetThreadInfo(GetCurrentThreadId(), &threadsOut);
	gEnv->pThreadUpr->ForEachOtherThread(GetThreadInfo, &threadsOut);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
	#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO

////////////////////////////////////////////////////////////////////////////
void SThreadInfo::OpenThreadHandles(TThreads& threadsOut, const TThreadIds& threadIds /* = TThreadIds()*/, bool ignoreCurrThread /* = true*/)
{
	TThreadIds threadids = threadIds;
	if (threadids.empty())
	{
		TThreadInfo threads;
		GetCurrentThreads(threads);
		DWORD currThreadId = GetCurrentThreadId();

		for (TThreadInfo::iterator it = threads.begin(), end = threads.end(); it != end; ++it)
			if (!ignoreCurrThread || it->first != currThreadId)
				threadids.push_back(it->first);
	}
	for (TThreadIds::iterator it = threadids.begin(), end = threadids.end(); it != end; ++it)
	{
		SThreadHandle thread;
		thread.Id = *it;
		thread.Handle = OpenThread(THREAD_ALL_ACCESS, FALSE, *it);
		threadsOut.push_back(thread);
	}
}

////////////////////////////////////////////////////////////////////////////
void SThreadInfo::CloseThreadHandles(const TThreads& threads)
{
	for (TThreads::const_iterator it = threads.begin(), end = threads.end(); it != end; ++it)
		CloseHandle(it->Handle);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
	#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ORBIS || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
////////////////////////////////////////////////////////////////////////////
void SThreadInfo::OpenThreadHandles(TThreads& threadsOut, const TThreadIds& threadIds /* = TThreadIds()*/, bool ignoreCurrThread /* = true*/)
{
	assert(false); // not implemented!
}

////////////////////////////////////////////////////////////////////////////
void SThreadInfo::CloseThreadHandles(const TThreads& threads)
{
	assert(false); // not implemented!
}
	#endif
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#endif //#if defined(ENABLE_PROFILING_CODE)
