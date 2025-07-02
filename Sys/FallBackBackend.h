// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FallBackBackEnd.h
//  Version:     v1.00
//  Created:     07/05/2011 by Christopher Bolte
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
// -------------------------------------------------------------------------
//  История:
////////////////////////////////////////////////////////////////////////////

#ifndef FallBackBackEnd_H_
#define FallBackBackEnd_H_

#include <drx3D/CoreX/Thread/IJobUpr.h>

namespace JobUpr {
namespace FallBackBackEnd {

class CFallBackBackEnd : public IBackend
{
public:
	CFallBackBackEnd();
	~CFallBackBackEnd();

	bool   Init(u32 nSysMaxWorker) { return true; }
	bool   ShutDown()                 { return true; }
	void   Update()                   {}

	void   AddJob(JobUpr::CJobDelegator& crJob, const JobUpr::TJobHandle cJobHandle, JobUpr::SInfoBlock& rInfoBlock);

	u32 GetNumWorkerThreads() const { return 0; }

#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)
	virtual IWorkerBackEndProfiler* GetBackEndWorkerProfiler() const { return 0; }
#endif
};

} // namespace FallBackBackEnd
} // namespace JobUpr

#endif // FallBackBackEnd_H_
