#ifndef DRX3D_THREAD_SUPPORT_INTERFACE_H
#define DRX3D_THREAD_SUPPORT_INTERFACE_H

class CriticalSection
{
public:
	CriticalSection() {}
	virtual ~CriticalSection() {}

	virtual void lock() = 0;
	virtual void unlock() = 0;
};

class ThreadSupportInterface
{
public:
	virtual ~ThreadSupportInterface() {}

	virtual i32 getNumWorkerThreads() const = 0;            // number of worker threads (total number of logical processors - 1)
	virtual i32 getCacheFriendlyNumThreads() const = 0;     // the number of logical processors sharing a single L3 cache
	virtual i32 getLogicalToPhysicalCoreRatio() const = 0;  // the number of logical processors per physical processor (usually 1 or 2)
	virtual void runTask(i32 threadIndex, uk userData) = 0;
	virtual void waitForAllTasks() = 0;

	virtual CriticalSection* createCriticalSection() = 0;
	virtual void deleteCriticalSection(CriticalSection* criticalSection) = 0;

	typedef void (*ThreadFunc)(uk userPtr);

	struct ConstructionInfo
	{
		ConstructionInfo(tukk uniqueName,
						 ThreadFunc userThreadFunc,
						 i32 threadStackSize = 65535)
			: m_uniqueName(uniqueName),
			  m_userThreadFunc(userThreadFunc),
			  m_threadStackSize(threadStackSize)
		{
		}

		tukk m_uniqueName;
		ThreadFunc m_userThreadFunc;
		i32 m_threadStackSize;
	};

	static ThreadSupportInterface* create(const ConstructionInfo& info);
};

#endif  //DRX3D_THREAD_SUPPORT_INTERFACE_H
