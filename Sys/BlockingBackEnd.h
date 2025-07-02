// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ThreadBackEnd.h
//  Version:     v1.00
//  Created:     07/05/2011 by Christopher Bolte
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
// -------------------------------------------------------------------------
//  История:
////////////////////////////////////////////////////////////////////////////

#ifndef BLOCKING_BACKEND_H_
#define BLOCKING_BACKEND_H_

#include <drx3D/CoreX/Thread/IJobUpr.h>
#include <drx3D/Sys/JobStructs.h>

#include <drx3D/CoreX/Thread/IThreadUpr.h>

namespace JobUpr
{
class CJobUpr;
class CWorkerBackEndProfiler;
}

namespace JobUpr {
namespace BlockingBackEnd {
namespace detail {
// Размер стека для каждой нити-трудяги этого блокирующего бэкэнда.
enum {eStackSize = 32 * 1024 };

}   // namespace detail

// forward declarations
class CBlockingBackEnd;

// Класс, представляющий нить-трудягу для бэкэнда ПК.
class CBlockingBackEndWorkerThread : public IThread
{
public:
	CBlockingBackEndWorkerThread(CBlockingBackEnd* pBlockingBackend, DrxFastSemaphore& rSemaphore, JobUpr::SJobQueue_BlockingBackEnd& rJobQueue, JobUpr::SInfoBlock** pRegularWorkerFallbacks, u32 nRegularWorkerThreads, u32 nID);
	~CBlockingBackEndWorkerThread();

	// Стартует работу по приёму в потоке.
	virtual void ThreadEntry();

	// Сигнализирует ните-трудяге, чтобы больше не принимала работ и вышла.
	void SignalStopWork();

private:
	void DoWork();
	void DoWorkProducerConsumerQueue(SInfoBlock& rInfoBlock);

	u32                                 m_nId;                   // ид нити-трудяги
	 bool                          m_bStop;
	DrxFastSemaphore&                      m_rSemaphore;
	JobUpr::SJobQueue_BlockingBackEnd& m_rJobQueue;
	CBlockingBackEnd*                      m_pBlockingBackend;

	// Члены, используемые для специальной обработки blocking backend fallback.
	JobUpr::SInfoBlock** m_pRegularWorkerFallbacks;
	u32                   m_nRegularWorkerThreads;
};

// Реализация бэкэнда ПК.
// Имеется  n-нитей-трудяг, использующих атомные операции для "вытягивания" из очереди работ;
// используется семафор для сигналирования трудягам о требуемой работе.
class CBlockingBackEnd : public IBackend
{
public:
	CBlockingBackEnd(JobUpr::SInfoBlock** pRegularWorkerFallbacks, u32 nRegularWorkerThreads);
	virtual ~CBlockingBackEnd();

	bool           Init(u32 nSysMaxWorker);
	bool           ShutDown();
	void           Update() {}

	virtual void   AddJob(JobUpr::CJobDelegator& crJob, const JobUpr::TJobHandle cJobHandle, JobUpr::SInfoBlock& rInfoBlock);

	virtual u32 GetNumWorkerThreads() const { return m_nNumWorker; }

	void           AddBlockingFallbackJob(JobUpr::SInfoBlock* pInfoBlock, u32 nWorkerThreadID);

#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)
	JobUpr::IWorkerBackEndProfiler* GetBackEndWorkerProfiler() const { return m_pBackEndWorkerProfiler; }
#endif

private:
	friend class JobUpr::CJobUpr;

	JobUpr::SJobQueue_BlockingBackEnd m_JobQueue;                   // job queue node where jobs are pushed into and from
	DrxFastSemaphore                      m_Semaphore;                  // semaphore to count available jobs, to allow the workers to go sleeping instead of spinning when no work is requiered
	CBlockingBackEndWorkerThread**        m_pWorkerThreads;             // worker threads for blocking backend
	u8 m_nNumWorker;                                                 // number of allocated worker threads

	// members used for special blocking backend fallback handling
	JobUpr::SInfoBlock** m_pRegularWorkerFallbacks;
	u32                   m_nRegularWorkerThreads;

	// members required for profiling jobs in the frame profiler
#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)
	JobUpr::IWorkerBackEndProfiler* m_pBackEndWorkerProfiler;
#endif
};

} // namespace BlockingBackEnd
} // namespace JobUpr

#endif // BLOCKING_BACKEND_H_
