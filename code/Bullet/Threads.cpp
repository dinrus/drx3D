
#include <drx3D/Maths/Linear/Threads.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <algorithm>  // for min and max

#if DRX3D_USE_OPENMP && DRX3D_THREADSAFE

#include <drx/openmp/omp.h>

#endif  // #if DRX3D_USE_OPENMP && DRX3D_THREADSAFE

#if DRX3D_USE_PPL && DRX3D_THREADSAFE

// use Microsoft Parallel Patterns Library (installed with Visual Studio 2010 and later)
#include <ppl.h>  // if you get a compile error here, check whether your version of Visual Studio includes PPL
// Visual Studio 2010 and later should come with it
#include <concrtrm.h>  // for GetProcessorCount()

#endif  // #if DRX3D_USE_PPL && DRX3D_THREADSAFE

#if DRX3D_USE_TBB && DRX3D_THREADSAFE

// use Intel Threading Building Blocks for thread management
#define __TBB_NO_IMPLICIT_LINKAGE 1
#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

#endif  // #if DRX3D_USE_TBB && DRX3D_THREADSAFE

#if DRX3D_THREADSAFE
//
// Lightweight spin-mutex based on atomics
// Using ordinary system-provided mutexes like Windows critical sections was noticeably slower
// presumably because when it fails to lock at first it would sleep the thread and trigger costly
// context switching.
//

#if __cplusplus >= 201103L

// for anything claiming full C++11 compliance, use C++11 atomics
// on GCC or Clang you need to compile with -std=c++11
#define USE_CPP11_ATOMICS 1

#elif defined(_MSC_VER)

// on MSVC, use intrinsics instead
#define USE_MSVC_INTRINSICS 1

#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))

// available since GCC 4.7 and some versions of clang
// todo: check for clang
#define USE_GCC_BUILTIN_ATOMICS 1

#elif defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)

// available since GCC 4.1
#define USE_GCC_BUILTIN_ATOMICS_OLD 1

#endif

#if USE_CPP11_ATOMICS

#include <atomic>
#include <thread>

#define THREAD_LOCAL_STATIC thread_local static

bool SpinMutex::tryLock()
{
	std::atomic<i32>* aDest = reinterpret_cast<std::atomic<i32>*>(&mLock);
	i32 expected = 0;
	return std::atomic_compare_exchange_weak_explicit(aDest, &expected, i32(1), std::memory_order_acq_rel, std::memory_order_acquire);
}

void SpinMutex::lock()
{
	// note: this lock does not sleep the thread.
	while (!tryLock())
	{
		// spin
	}
}

void SpinMutex::unlock()
{
	std::atomic<i32>* aDest = reinterpret_cast<std::atomic<i32>*>(&mLock);
	std::atomic_store_explicit(aDest, i32(0), std::memory_order_release);
}

#elif USE_MSVC_INTRINSICS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <intrin.h>

#define THREAD_LOCAL_STATIC __declspec(thread) static

bool SpinMutex::tryLock()
{
	 long* aDest = reinterpret_cast<long*>(&mLock);
	return (0 == _InterlockedCompareExchange(aDest, 1, 0));
}

void SpinMutex::lock()
{
	// note: this lock does not sleep the thread
	while (!tryLock())
	{
		// spin
	}
}

void SpinMutex::unlock()
{
	 long* aDest = reinterpret_cast<long*>(&mLock);
	_InterlockedExchange(aDest, 0);
}

#elif USE_GCC_BUILTIN_ATOMICS

#define THREAD_LOCAL_STATIC static __thread

bool SpinMutex::tryLock()
{
	i32 expected = 0;
	bool weak = false;
	i32k memOrderSuccess = __ATOMIC_ACQ_REL;
	i32k memOrderFail = __ATOMIC_ACQUIRE;
	return __atomic_compare_exchange_n(&mLock, &expected, i32(1), weak, memOrderSuccess, memOrderFail);
}

void SpinMutex::lock()
{
	// note: this lock does not sleep the thread
	while (!tryLock())
	{
		// spin
	}
}

void SpinMutex::unlock()
{
	__atomic_store_n(&mLock, i32(0), __ATOMIC_RELEASE);
}

#elif USE_GCC_BUILTIN_ATOMICS_OLD

#define THREAD_LOCAL_STATIC static __thread

bool SpinMutex::tryLock()
{
	return __sync_bool_compare_and_swap(&mLock, i32(0), i32(1));
}

void SpinMutex::lock()
{
	// note: this lock does not sleep the thread
	while (!tryLock())
	{
		// spin
	}
}

void SpinMutex::unlock()
{
	// write 0
	__sync_fetch_and_and(&mLock, i32(0));
}

#else  //#elif USE_MSVC_INTRINSICS

#error "no threading primitives defined -- unknown platform"

#endif  //#else //#elif USE_MSVC_INTRINSICS

#else  //#if DRX3D_THREADSAFE

// These should not be called ever
void SpinMutex::lock()
{
	Assert(!"вызван нереализованный SpinMutex::lock()");
}

void SpinMutex::unlock()
{
	Assert(!"вызван нереализованный SpinMutex::unlock()");
}

bool SpinMutex::tryLock()
{
	Assert(!"вызван нереализованный SpinMutex::tryLock()");
	return true;
}

#define THREAD_LOCAL_STATIC static

#endif  // #else //#if DRX3D_THREADSAFE

struct ThreadsafeCounter
{
	u32 mCounter;
	SpinMutex mMutex;

	ThreadsafeCounter()
	{
		mCounter = 0;
		--mCounter;  // first count should come back 0
	}

	u32 getNext()
	{
		// no need to optimize this with atomics, it is only called ONCE per thread!
		mMutex.lock();
		mCounter++;
		if (mCounter >= DRX3D_MAX_THREAD_COUNT)
		{
			Assert(!"превышен счётчик потоков");
			// wrap back to the first worker index
			mCounter = 1;
		}
		u32 val = mCounter;
		mMutex.unlock();
		return val;
	}
};

static ITaskScheduler* gBtTaskScheduler=0;
static i32 gThreadsRunningCounter = 0;  // useful for detecting if we are trying to do nested parallel-for calls
static SpinMutex gThreadsRunningCounterMutex;
static ThreadsafeCounter gThreadCounter;

//
// DRX3D_DETECT_BAD_THREAD_INDEX tries to detect when there are multiple threads assigned the same thread index.
//
// DRX3D_DETECT_BAD_THREAD_INDEX is a developer option to test if
// certain assumptions about how the task scheduler manages its threads
// holds true.
// The main assumption is:
//   - when the threadpool is resized, the task scheduler either
//      1. destroys all worker threads and creates all new ones in the correct number, OR
//      2. never destroys a worker thread
//
// We make that assumption because we can't easily enumerate the worker threads of a task scheduler
// to assign nice sequential thread-indexes. We also do not get notified if a worker thread is destroyed,
// so we can't tell when a thread-index is no longer being used.
// We allocate thread-indexes as needed with a sequential global thread counter.
//
// Our simple thread-counting scheme falls apart if the task scheduler destroys some threads but
// continues to re-use other threads and the application repeatedly resizes the thread pool of the
// task scheduler.
// In order to prevent the thread-counter from exceeding the global max (DRX3D_MAX_THREAD_COUNT), we
// wrap the thread counter back to 1. This should only happen if the worker threads have all been
// destroyed and re-created.
//
// DRX3D_DETECT_BAD_THREAD_INDEX only works for Win32 right now,
// but could be adapted to work with pthreads
#define DRX3D_DETECT_BAD_THREAD_INDEX 0

#if DRX3D_DETECT_BAD_THREAD_INDEX

typedef DWORD ThreadId_t;
const static ThreadId_t kInvalidThreadId = 0;
ThreadId_t gDebugThreadIds[DRX3D_MAX_THREAD_COUNT];

static ThreadId_t getDebugThreadId()
{
	return GetCurrentThreadId();
}

#endif  // #if DRX3D_DETECT_BAD_THREAD_INDEX

// return a unique index per thread, main thread is 0, worker threads are in [1, DRX3D_MAX_THREAD_COUNT)
u32 GetCurrentThreadIndex()
{
	u32k kNullIndex = ~0U;
	THREAD_LOCAL_STATIC u32 sThreadIndex = kNullIndex;
	if (sThreadIndex == kNullIndex)
	{
		sThreadIndex = gThreadCounter.getNext();
		Assert(sThreadIndex < DRX3D_MAX_THREAD_COUNT);
	}
#if DRX3D_DETECT_BAD_THREAD_INDEX
	if (gBtTaskScheduler && sThreadIndex > 0)
	{
		ThreadId_t tid = getDebugThreadId();
		// if not set
		if (gDebugThreadIds[sThreadIndex] == kInvalidThreadId)
		{
			// set it
			gDebugThreadIds[sThreadIndex] = tid;
		}
		else
		{
			if (gDebugThreadIds[sThreadIndex] != tid)
			{
				// this could indicate the task scheduler is breaking our assumptions about
				// how threads are managed when threadpool is resized
				Assert(!"два или более потока имеют одинаковый thread-index!");
				__debugbreak();
			}
		}
	}
#endif  // #if DRX3D_DETECT_BAD_THREAD_INDEX
	return sThreadIndex;
}

bool IsMainThread()
{
	return GetCurrentThreadIndex() == 0;
}

void ResetThreadIndexCounter()
{
	// for when all current worker threads are destroyed
	Assert(IsMainThread());
	gThreadCounter.mCounter = 0;
}

ITaskScheduler::ITaskScheduler(tukk name)
{
	m_name = name;
	m_savedThreadCounter = 0;
	m_isActive = false;
}

void ITaskScheduler::activate()
{
	// gThreadCounter is used to assign a thread-index to each worker thread in a task scheduler.
	// The main thread is always thread-index 0, and worker threads are numbered from 1 to 63 (DRX3D_MAX_THREAD_COUNT-1)
	// The thread-indexes need to be unique amongst the threads that can be running simultaneously.
	// Since only one task scheduler can be used at a time, it is OK for a pair of threads that belong to different
	// task schedulers to share the same thread index because they can't be running at the same time.
	// So each task scheduler needs to keep its own thread counter value
	if (!m_isActive)
	{
		gThreadCounter.mCounter = m_savedThreadCounter;  // restore saved thread counter
		m_isActive = true;
	}
}

void ITaskScheduler::deactivate()
{
	if (m_isActive)
	{
		m_savedThreadCounter = gThreadCounter.mCounter;  // save thread counter
		m_isActive = false;
	}
}

void PushThreadsAreRunning()
{
	gThreadsRunningCounterMutex.lock();
	gThreadsRunningCounter++;
	gThreadsRunningCounterMutex.unlock();
}

void PopThreadsAreRunning()
{
	gThreadsRunningCounterMutex.lock();
	gThreadsRunningCounter--;
	gThreadsRunningCounterMutex.unlock();
}

bool ThreadsAreRunning()
{
	return gThreadsRunningCounter != 0;
}

void SetTaskScheduler(ITaskScheduler* ts)
{
	i32 threadId = GetCurrentThreadIndex();  // make sure we call this on main thread at least once before any workers run
	if (threadId != 0)
	{
		Assert(!"SetTaskScheduler должен вызываться из главного потока!");
		return;
	}
	if (gBtTaskScheduler)
	{
		// deactivate old task scheduler
		gBtTaskScheduler->deactivate();
	}
	gBtTaskScheduler = ts;
	if (ts)
	{
		// activate new task scheduler
		ts->activate();
	}
}

ITaskScheduler* GetTaskScheduler()
{
	return gBtTaskScheduler;
}

void ParallelFor(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelForBody& body)
{
#if DRX3D_THREADSAFE

#if DRX3D_DETECT_BAD_THREAD_INDEX
	if (!ThreadsAreRunning())
	{
		// clear out thread ids
		for (i32 i = 0; i < DRX3D_MAX_THREAD_COUNT; ++i)
		{
			gDebugThreadIds[i] = kInvalidThreadId;
		}
	}
#endif  // #if DRX3D_DETECT_BAD_THREAD_INDEX

	Assert(gBtTaskScheduler != NULL);  // call SetTaskScheduler() with a valid task scheduler first!
	gBtTaskScheduler->parallelFor(iBegin, iEnd, grainSize, body);

#else  // #if DRX3D_THREADSAFE

	// non-parallel version of ParallelFor
	Assert(!"called ParallelFor in non-threadsafe build. enable DRX3D_THREADSAFE");
	body.forLoop(iBegin, iEnd);

#endif  // #if DRX3D_THREADSAFE
}

Scalar ParallelSum(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelSumBody& body)
{
#if DRX3D_THREADSAFE

#if DRX3D_DETECT_BAD_THREAD_INDEX
	if (!ThreadsAreRunning())
	{
		// clear out thread ids
		for (i32 i = 0; i < DRX3D_MAX_THREAD_COUNT; ++i)
		{
			gDebugThreadIds[i] = kInvalidThreadId;
		}
	}
#endif  // #if DRX3D_DETECT_BAD_THREAD_INDEX

	Assert(gBtTaskScheduler != NULL);  // call SetTaskScheduler() with a valid task scheduler first!
	return gBtTaskScheduler->parallelSum(iBegin, iEnd, grainSize, body);

#else  // #if DRX3D_THREADSAFE

	// non-parallel version of ParallelSum
	Assert(!"called ParallelFor in non-threadsafe build. enable DRX3D_THREADSAFE");
	return body.sumLoop(iBegin, iEnd);

#endif  //#else // #if DRX3D_THREADSAFE
}

///
/// TaskSchedulerSequential -- non-threaded implementation of task scheduler
///                              (really just useful for testing performance of single threaded vs multi)
///
class TaskSchedulerSequential : public ITaskScheduler
{
public:
	TaskSchedulerSequential() : ITaskScheduler("Sequential") {}
	virtual i32 getMaxNumThreads() const DRX3D_OVERRIDE { return 1; }
	virtual i32 getNumThreads() const DRX3D_OVERRIDE { return 1; }
	virtual void setNumThreads(i32 numThreads) DRX3D_OVERRIDE {}
	virtual void parallelFor(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelForBody& body) DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("parallelFor_sequential");
		body.forLoop(iBegin, iEnd);
	}
	virtual Scalar parallelSum(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelSumBody& body) DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("parallelSum_sequential");
		return body.sumLoop(iBegin, iEnd);
	}
};

#if DRX3D_USE_OPENMP && DRX3D_THREADSAFE
///
/// TaskSchedulerOpenMP -- wrapper around OpenMP task scheduler
///
class TaskSchedulerOpenMP : public ITaskScheduler
{
	i32 m_numThreads;

public:
	TaskSchedulerOpenMP() : ITaskScheduler("OpenMP")
	{
		m_numThreads = 0;
	}
	virtual i32 getMaxNumThreads() const DRX3D_OVERRIDE
	{
		return omp_get_max_threads();
	}
	virtual i32 getNumThreads() const DRX3D_OVERRIDE
	{
		return m_numThreads;
	}
	virtual void setNumThreads(i32 numThreads) DRX3D_OVERRIDE
	{
		// With OpenMP, because it is a standard with various implementations, we can't
		// know for sure if every implementation has the same behavior of destroying all
		// previous threads when resizing the threadpool
		m_numThreads = (std::max)(1, (std::min)(i32(DRX3D_MAX_THREAD_COUNT), numThreads));
		omp_set_num_threads(1);  // hopefully, all previous threads get destroyed here
		omp_set_num_threads(m_numThreads);
		m_savedThreadCounter = 0;
		if (m_isActive)
		{
			ResetThreadIndexCounter();
		}
	}
	virtual void parallelFor(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelForBody& body) DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("parallelFor_OpenMP");
		PushThreadsAreRunning();
#pragma omp parallel for schedule(static, 1)
		for (i32 i = iBegin; i < iEnd; i += grainSize)
		{
			DRX3D_PROFILE("OpenMP_forJob");
			body.forLoop(i, (std::min)(i + grainSize, iEnd));
		}
		PopThreadsAreRunning();
	}
	virtual Scalar parallelSum(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelSumBody& body) DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("parallelFor_OpenMP");
		PushThreadsAreRunning();
		Scalar sum = Scalar(0);
#pragma omp parallel for schedule(static, 1) reduction(+ \
													   : sum)
		for (i32 i = iBegin; i < iEnd; i += grainSize)
		{
			DRX3D_PROFILE("OpenMP_sumJob");
			sum += body.sumLoop(i, (std::min)(i + grainSize, iEnd));
		}
		PopThreadsAreRunning();
		return sum;
	}
};
#endif  // #if DRX3D_USE_OPENMP && DRX3D_THREADSAFE

#if DRX3D_USE_TBB && DRX3D_THREADSAFE
///
/// TaskSchedulerTBB -- wrapper around Intel Threaded Building Blocks task scheduler
///
class TaskSchedulerTBB : public ITaskScheduler
{
	i32 m_numThreads;
	tbb::task_scheduler_init* m_tbbSchedulerInit;

public:
	TaskSchedulerTBB() : ITaskScheduler("IntelTBB")
	{
		m_numThreads = 0;
		m_tbbSchedulerInit = NULL;
	}
	~TaskSchedulerTBB()
	{
		if (m_tbbSchedulerInit)
		{
			delete m_tbbSchedulerInit;
			m_tbbSchedulerInit = NULL;
		}
	}

	virtual i32 getMaxNumThreads() const DRX3D_OVERRIDE
	{
		return tbb::task_scheduler_init::default_num_threads();
	}
	virtual i32 getNumThreads() const DRX3D_OVERRIDE
	{
		return m_numThreads;
	}
	virtual void setNumThreads(i32 numThreads) DRX3D_OVERRIDE
	{
		m_numThreads = (std::max)(1, (std::min)(i32(DRX3D_MAX_THREAD_COUNT), numThreads));
		if (m_tbbSchedulerInit)
		{
			// destroys all previous threads
			delete m_tbbSchedulerInit;
			m_tbbSchedulerInit = NULL;
		}
		m_tbbSchedulerInit = new tbb::task_scheduler_init(m_numThreads);
		m_savedThreadCounter = 0;
		if (m_isActive)
		{
			ResetThreadIndexCounter();
		}
	}
	struct ForBodyAdapter
	{
		const IParallelForBody* mBody;

		ForBodyAdapter(const IParallelForBody* body) : mBody(body) {}
		void operator()(const tbb::blocked_range<i32>& range) const
		{
			DRX3D_PROFILE("TBB_forJob");
			mBody->forLoop(range.begin(), range.end());
		}
	};
	virtual void parallelFor(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelForBody& body) DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("parallelFor_TBB");
		ForBodyAdapter tbbBody(&body);
		PushThreadsAreRunning();
		tbb::parallel_for(tbb::blocked_range<i32>(iBegin, iEnd, grainSize),
						  tbbBody,
						  tbb::simple_partitioner());
		PopThreadsAreRunning();
	}
	struct SumBodyAdapter
	{
		const IParallelSumBody* mBody;
		Scalar mSum;

		SumBodyAdapter(const IParallelSumBody* body) : mBody(body), mSum(Scalar(0)) {}
		SumBodyAdapter(const SumBodyAdapter& src, tbb::split) : mBody(src.mBody), mSum(Scalar(0)) {}
		void join(const SumBodyAdapter& src) { mSum += src.mSum; }
		void operator()(const tbb::blocked_range<i32>& range)
		{
			DRX3D_PROFILE("TBB_sumJob");
			mSum += mBody->sumLoop(range.begin(), range.end());
		}
	};
	virtual Scalar parallelSum(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelSumBody& body) DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("parallelSum_TBB");
		SumBodyAdapter tbbBody(&body);
		PushThreadsAreRunning();
		tbb::parallel_deterministic_reduce(tbb::blocked_range<i32>(iBegin, iEnd, grainSize), tbbBody);
		PopThreadsAreRunning();
		return tbbBody.mSum;
	}
};
#endif  // #if DRX3D_USE_TBB && DRX3D_THREADSAFE

#if DRX3D_USE_PPL && DRX3D_THREADSAFE
///
/// TaskSchedulerPPL -- wrapper around Microsoft Parallel Patterns Lib task scheduler
///
class TaskSchedulerPPL : public ITaskScheduler
{
	i32 m_numThreads;
	concurrency::combinable<Scalar> m_sum;  // for parallelSum
public:
	TaskSchedulerPPL() : ITaskScheduler("PPL")
	{
		m_numThreads = 0;
	}
	virtual i32 getMaxNumThreads() const DRX3D_OVERRIDE
	{
		return concurrency::GetProcessorCount();
	}
	virtual i32 getNumThreads() const DRX3D_OVERRIDE
	{
		return m_numThreads;
	}
	virtual void setNumThreads(i32 numThreads) DRX3D_OVERRIDE
	{
		// capping the thread count for PPL due to a thread-index issue
		i32k maxThreadCount = (std::min)(i32(DRX3D_MAX_THREAD_COUNT), 31);
		m_numThreads = (std::max)(1, (std::min)(maxThreadCount, numThreads));
		using namespace concurrency;
		if (CurrentScheduler::Id() != -1)
		{
			CurrentScheduler::Detach();
		}
		SchedulerPolicy policy;
		{
			// PPL seems to destroy threads when threadpool is shrunk, but keeps reusing old threads
			// force it to destroy old threads
			policy.SetConcurrencyLimits(1, 1);
			CurrentScheduler::Create(policy);
			CurrentScheduler::Detach();
		}
		policy.SetConcurrencyLimits(m_numThreads, m_numThreads);
		CurrentScheduler::Create(policy);
		m_savedThreadCounter = 0;
		if (m_isActive)
		{
			ResetThreadIndexCounter();
		}
	}
	struct ForBodyAdapter
	{
		const IParallelForBody* mBody;
		i32 mGrainSize;
		i32 mIndexEnd;

		ForBodyAdapter(const IParallelForBody* body, i32 grainSize, i32 end) : mBody(body), mGrainSize(grainSize), mIndexEnd(end) {}
		void operator()(i32 i) const
		{
			DRX3D_PROFILE("PPL_forJob");
			mBody->forLoop(i, (std::min)(i + mGrainSize, mIndexEnd));
		}
	};
	virtual void parallelFor(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelForBody& body) DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("parallelFor_PPL");
		// PPL dispatch
		ForBodyAdapter pplBody(&body, grainSize, iEnd);
		PushThreadsAreRunning();
		// note: MSVC 2010 doesn't support partitioner args, so avoid them
		concurrency::parallel_for(iBegin,
								  iEnd,
								  grainSize,
								  pplBody);
		PopThreadsAreRunning();
	}
	struct SumBodyAdapter
	{
		const IParallelSumBody* mBody;
		concurrency::combinable<Scalar>* mSum;
		i32 mGrainSize;
		i32 mIndexEnd;

		SumBodyAdapter(const IParallelSumBody* body, concurrency::combinable<Scalar>* sum, i32 grainSize, i32 end) : mBody(body), mSum(sum), mGrainSize(grainSize), mIndexEnd(end) {}
		void operator()(i32 i) const
		{
			DRX3D_PROFILE("PPL_sumJob");
			mSum->local() += mBody->sumLoop(i, (std::min)(i + mGrainSize, mIndexEnd));
		}
	};
	static Scalar sumFunc(Scalar a, Scalar b) { return a + b; }
	virtual Scalar parallelSum(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelSumBody& body) DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("parallelSum_PPL");
		m_sum.clear();
		SumBodyAdapter pplBody(&body, &m_sum, grainSize, iEnd);
		PushThreadsAreRunning();
		// note: MSVC 2010 doesn't support partitioner args, so avoid them
		concurrency::parallel_for(iBegin,
								  iEnd,
								  grainSize,
								  pplBody);
		PopThreadsAreRunning();
		return m_sum.combine(sumFunc);
	}
};
#endif  // #if DRX3D_USE_PPL && DRX3D_THREADSAFE

// create a non-threaded task scheduler (always available)
ITaskScheduler* GetSequentialTaskScheduler()
{
	static TaskSchedulerSequential sTaskScheduler;
	return &sTaskScheduler;
}

// create an OpenMP task scheduler (if available, otherwise returns null)
ITaskScheduler* GetOpenMPTaskScheduler()
{
#if DRX3D_USE_OPENMP && DRX3D_THREADSAFE
	static TaskSchedulerOpenMP sTaskScheduler;
	return &sTaskScheduler;
#else
	return NULL;
#endif
}

// create an Intel TBB task scheduler (if available, otherwise returns null)
ITaskScheduler* GetTBBTaskScheduler()
{
#if DRX3D_USE_TBB && DRX3D_THREADSAFE
	static TaskSchedulerTBB sTaskScheduler;
	return &sTaskScheduler;
#else
	return NULL;
#endif
}

// create a PPL task scheduler (if available, otherwise returns null)
ITaskScheduler* GetPPLTaskScheduler()
{
#if DRX3D_USE_PPL && DRX3D_THREADSAFE
	static TaskSchedulerPPL sTaskScheduler;
	return &sTaskScheduler;
#else
	return NULL;
#endif
}
