// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef LOADINGPROFILERSYSTEM_H
#define LOADINGPROFILERSYSTEM_H

#if defined(ENABLE_LOADING_PROFILER)

struct SLoadingTimeContainer;

struct SLoadingProfilerInfo
{
	string            name;
	double            selfTime;
	double            totalTime;
	u32            callsTotal;
	double            memorySize;

	DiskOperationInfo selfInfo;
	DiskOperationInfo totalInfo;
};

class CLoadingProfilerSystem
{
public:
	static void                   Init();
	static void                   ShutDown();
	static void                   CreateNoStackList(PodArray<SLoadingTimeContainer>&);
	static void                   OutputLoadingTimeStats(ILog* pLog, i32 nMode);
	static SLoadingTimeContainer* StartLoadingSectionProfiling(CLoadingTimeProfiler* pProfiler, tukk szFuncName);
	static void                   EndLoadingSectionProfiling(CLoadingTimeProfiler* pProfiler);
	static tukk            GetLoadingProfilerCallstack();
	static void                   FillProfilersList(std::vector<SLoadingProfilerInfo>& profilers);
	static void                   FlushTimeContainers();
	static void                   SaveTimeContainersToFile(tukk , double fMinTotalTime, bool bClean);
	static void                   WriteTimeContainerToFile(SLoadingTimeContainer* p, FILE* f, u32 depth, double fMinTotalTime);
	static void                   UpdateSelfStatistics(SLoadingTimeContainer* p);
	static void                   Clean();
protected:
	static void                   AddTimeContainerFunction(PodArray<SLoadingTimeContainer>&, SLoadingTimeContainer*);
protected:
	static i32                    nLoadingProfileMode;
	static i32                    nLoadingProfilerNotTrackedAllocations;
	static DrxCriticalSection     csLock;
	static i32                    m_iMaxArraySize;
	static SLoadingTimeContainer* m_pCurrentLoadingTimeContainer;
	static SLoadingTimeContainer* m_pRoot[2];
	static i32                    m_iActiveRoot;
	static ICVar*                 m_pEnableProfile;
};

#endif

#endif // LOADINGPROFILERSYSTEM_H
