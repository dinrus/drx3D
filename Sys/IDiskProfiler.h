// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id: IDiskProfiler.h,v 1.0 2008/03/28 11:11:13 AntonKaplanyan Exp wwwrun $
   $DateTime$
   Описание:  Routine for profiling disk IO
   -------------------------------------------------------------------------
   История:
   - 28:3:2008   11:11 : Created by Anton Kaplanyan
*************************************************************************/

#ifndef __diskprofile_h__
#define __diskprofile_h__

#ifdef USE_DISK_PROFILER

	#include <drx3D/CoreX/Platform/platform.h>
	#include <drx3D/Sys/ISystem.h> // <> required for Interfuscator
	#include <drx3D/Sys/ITimer.h>

class IDiskProfiler;

//! Disk operations type.
enum EDiskProfileOperationType
{
	edotRead       = 1 << 0,
	edotWrite      = 1 << 1,
	edotOpen       = 1 << 2,
	edotClose      = 1 << 3,
	edotSeek       = 1 << 4,
	edotCompress   = 1 << 5,
	edotDecompress = 1 << 6,
};

//! Disk Profiler statistics.
struct SDiskProfileStatistics
{
	float    m_beginIOTime;
	float    m_endIOTime;
	u32   m_nIOType;         //!< EDiskProfileOperationType flags.
	threadID m_threadId;
	size_t   m_size;            //!< Size of IO operation, in bytes.
	string   m_strFile;
	u32   m_nTaskType;
	inline const bool operator<(const SDiskProfileStatistics& s) const
	{
		return m_beginIOTime < s.m_beginIOTime;
	}
};

//! Disk Profile interface class.
class IDiskProfiler
{
	friend class CDiskProfileTimer;
public:
	virtual ~IDiskProfiler(){}
	virtual void              Update() = 0;
	virtual bool              IsEnabled() const = 0;

	virtual DiskOperationInfo GetStatistics() const = 0;
	virtual bool              RegisterStatistics(SDiskProfileStatistics* pStatistics) = 0;
	virtual void              SetTaskType(const threadID nThreadId, u32k nType = 0xffffffff) = 0;
};

//! Disk Profile timer.
class CDiskProfileTimer
{
public:
	CDiskProfileTimer(u32k nIOType, const size_t nIOSize, IDiskProfiler* pProfiler, tukk pFileName)
		: m_pStatistics(NULL)
	{
		if (pProfiler && pProfiler->IsEnabled())
		{
			m_pStatistics = new SDiskProfileStatistics;
			m_pStatistics->m_threadId = DrxGetCurrentThreadId();
			m_pStatistics->m_beginIOTime = gEnv->pTimer->GetAsyncCurTime();
			m_pStatistics->m_endIOTime = -1.f;  //!< Uninitialized.
			m_pStatistics->m_nIOType = nIOType;
			m_pStatistics->m_size = nIOSize;
			m_pStatistics->m_strFile = pFileName;
			if (!pProfiler->RegisterStatistics(m_pStatistics))
			{
				delete m_pStatistics;
				m_pStatistics = NULL;
			}
		}
	}

	~CDiskProfileTimer()
	{
		if (m_pStatistics)
			m_pStatistics->m_endIOTime = gEnv->pTimer->GetAsyncCurTime();
	}

protected:
	SDiskProfileStatistics* m_pStatistics;
};

//! Disk Profile timer.
class CDiskProfileTypeScope
{
public:
	CDiskProfileTypeScope(u32k eTaskType, IDiskProfiler* pProfiler) : m_eType(eTaskType), m_nThreadId(DrxGetCurrentThreadId()), m_pProfiler(pProfiler)
	{
		if (pProfiler && pProfiler->IsEnabled())
		{
			pProfiler->SetTaskType(m_nThreadId, eTaskType);
		}
	}

	~CDiskProfileTypeScope()
	{
		if (m_pProfiler && m_pProfiler->IsEnabled())
		{
			m_pProfiler->SetTaskType(m_nThreadId);
		}
	}

protected:
	u32k   m_eType;
	const threadID m_nThreadId;
	IDiskProfiler* m_pProfiler;
};

#endif

#ifdef USE_DISK_PROFILER
	#define PROFILE_DISK(type, size, name) CDiskProfileTimer _profile_disk_io_ ## type(type, size, gEnv->pSystem->GetIDiskProfiler(), name);
	#define PROFILE_DISK_TASK_TYPE(type)   CDiskProfileTypeScope _profile_disk_type_(type, gEnv->pSystem->GetIDiskProfiler());
#else
	#define PROFILE_DISK(type, size, name)
	#define PROFILE_DISK_TASK_TYPE(type)
#endif // USE_DISK_PROFILER

#define PROFILE_DISK_READ(size)          PROFILE_DISK(edotRead, size, 0)
#define PROFILE_DISK_SEEK_WITHNAME(name) PROFILE_DISK(edotSeek, 0, name)
#define PROFILE_DISK_SEEK       PROFILE_DISK(edotSeek, 0, 0)
#define PROFILE_DISK_DECOMPRESS PROFILE_DISK(edotDecompress, 0, 0)
#define PROFILE_DISK_WRITE      PROFILE_DISK(edotWrite, 0, 0)
#define PROFILE_DISK_COMPRESS   PROFILE_DISK(edotCompress, 0, 0)
#define PROFILE_DISK_OPEN       PROFILE_DISK(edotOpen, 0, 0)
#define PROFILE_DISK_CLOSE      PROFILE_DISK(edotClose, 0, 0)

#endif // __diskprofile_h__
