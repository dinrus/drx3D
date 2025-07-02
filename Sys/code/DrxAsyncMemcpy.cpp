// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/CoreX/Thread/IJobUpr_JobDelegator.h>

namespace
{
static void drxAsyncMemcpy_Int(
  uk dst
  , ukk src
  , size_t size
  , i32 nFlags
  ,  i32* sync)
{
	drxMemcpy(dst, src, size, nFlags);
	if (sync)
		DrxInterlockedDecrement(sync);
}
}

DECLARE_JOB("DrxAsyncMemcpy", TDrxAsyncMemcpy, drxAsyncMemcpy_Int);

#if !defined(DRX_ASYNC_MEMCPY_DELEGATE_TO_DRXSYSTEM)
DRX_ASYNC_MEMCPY_API void drxAsyncMemcpy(
#else
DRX_ASYNC_MEMCPY_API void _drxAsyncMemcpy(
#endif
  uk dst
  , ukk src
  , size_t size
  , i32 nFlags
  ,  i32* sync)
{
	TDrxAsyncMemcpy job(dst, src, size, nFlags, sync);
	job.Run();
}
