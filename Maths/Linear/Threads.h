#ifndef DRX3D_THREADS_H
#define DRX3D_THREADS_H

//Используем OpenMP
#define DRX3D_USE_OPENMP 1
#define DRX3D_THREADSAFE 1

#include <drx3D/Maths/Linear/Scalar.h>  // содержит такие определения, как SIMD_FORCE_INLINE

#if defined(_MSC_VER) && _MSC_VER >= 1600
// даёт нам ошибку компиляции, если изменились сигнатуры у любых переопределённых методов.
#define DRX3D_OVERRIDE override
#endif

#ifndef DRX3D_OVERRIDE
#define DRX3D_OVERRIDE
#endif

// Не устанавливать более, чем в 64, не меняя ThreadSupportPosix
// и ThreadSupportWin32. Там используются битмаски UINT64.
u32k DRX3D_MAX_THREAD_COUNT = 64;  // только, если DRX3D_THREADSAFE равен 1

// только для внутреннего использования
bool IsMainThread();
bool ThreadsAreRunning();
u32 GetCurrentThreadIndex();
void ResetThreadIndexCounter();  // Уведомляет о разрушении всех рабочих потоков.

///
/// SpinMutex --  легковесный spin-mutex, реализованный с помощью атомных операций, никогда не
///               отправляет поток "спать", так как разработан под использование с планировщиком
///               задач, у которого один поток на ядро, и потоки не спят, пока задачи не исчерпаны.
///               Для общецелевого использования не очень подходит.
///
class SpinMutex
{
	i32 mLock;

public:
	SpinMutex()
	{
		mLock = 0;
	}
	void lock();
	void unlock();
	bool tryLock();
};

//
// NOTE: Mutex* is for internal drx3D use only
//
// If DRX3D_THREADSAFE is undefined or 0, should optimize away to nothing.
// This is good because for the single-threaded build of drx3D, any calls
// to these functions will be optimized out.
//
// However, for users of the multi-threaded build of drx3D this is kind
// of bad because if you call any of these functions from external code
// (where DRX3D_THREADSAFE is undefined) you will get unexpected race conditions.
//
SIMD_FORCE_INLINE void MutexLock(SpinMutex* mutex)
{
#if DRX3D_THREADSAFE
	mutex->lock();
#else
	(void)mutex;
#endif  // #if DRX3D_THREADSAFE
}

SIMD_FORCE_INLINE void MutexUnlock(SpinMutex* mutex)
{
#if DRX3D_THREADSAFE
	mutex->unlock();
#else
	(void)mutex;
#endif  // #if DRX3D_THREADSAFE
}

SIMD_FORCE_INLINE bool MutexTryLock(SpinMutex* mutex)
{
#if DRX3D_THREADSAFE
	return mutex->tryLock();
#else
	(void)mutex;
	return true;
#endif  // #if DRX3D_THREADSAFE
}

//
// IParallelForBody -- subclass this to express work that can be done in parallel
//
class IParallelForBody
{
public:
	virtual ~IParallelForBody() {}
	virtual void forLoop(i32 iBegin, i32 iEnd) const = 0;
};

//
// IParallelSumBody -- subclass this to express work that can be done in parallel
//                       and produces a sum over all loop elements
//
class IParallelSumBody
{
public:
	virtual ~IParallelSumBody() {}
	virtual Scalar sumLoop(i32 iBegin, i32 iEnd) const = 0;
};

//
// ITaskScheduler -- subclass this to implement a task scheduler that can dispatch work to
//                     worker threads
//
class ITaskScheduler
{
public:
	ITaskScheduler(tukk name);
	virtual ~ITaskScheduler() {}
	tukk getName() const { return m_name; }

	virtual i32 getMaxNumThreads() const = 0;
	virtual i32 getNumThreads() const = 0;
	virtual void setNumThreads(i32 numThreads) = 0;
	virtual void parallelFor(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelForBody& body) = 0;
	virtual Scalar parallelSum(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelSumBody& body) = 0;
	virtual void sleepWorkerThreadsHint() {}  // hint the task scheduler that we may not be using these threads for a little while

	// internal use only
	virtual void activate();
	virtual void deactivate();

protected:
	tukk m_name;
	u32 m_savedThreadCounter;
	bool m_isActive;
};

// set the task scheduler to use for all calls to ParallelFor()
// NOTE: you must set this prior to using any of the multi-threaded "Mt" classes
void SetTaskScheduler(ITaskScheduler* ts);

// get the current task scheduler
ITaskScheduler* GetTaskScheduler();

// get non-threaded task scheduler (always available)
ITaskScheduler* GetSequentialTaskScheduler();

// create a default task scheduler (Win32 or pthreads based)
ITaskScheduler* CreateDefaultTaskScheduler();

// get OpenMP task scheduler (if available, otherwise returns null)
ITaskScheduler* GetOpenMPTaskScheduler();

// get Intel TBB task scheduler (if available, otherwise returns null)
ITaskScheduler* GetTBBTaskScheduler();

// get PPL task scheduler (if available, otherwise returns null)
ITaskScheduler* GetPPLTaskScheduler();

// ParallelFor -- call this to dispatch work like a for-loop
//                 (iterations may be done out of order, so no dependencies are allowed)
void ParallelFor(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelForBody& body);

// ParallelSum -- call this to dispatch work like a for-loop, returns the sum of all iterations
//                 (iterations may be done out of order, so no dependencies are allowed)
Scalar ParallelSum(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelSumBody& body);

#endif
