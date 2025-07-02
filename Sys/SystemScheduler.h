// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SYSTEMSCHEDULER_H__
#define __SYSTEMSCHEDULER_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Sys/System.h>
#include <drx3D/Sys/ISystemScheduler.h>

class CSystemScheduler : public ISystemScheduler
{
public:
	CSystemScheduler(CSystem* pSystem);
	virtual ~CSystemScheduler(void);

	// ISystemScheduler
	virtual void SliceAndSleep(tukk sliceName, i32 line);
	virtual void SliceLoadingBegin();
	virtual void SliceLoadingEnd();

	virtual void SchedulingSleepIfNeeded(void);
	// ~ISystemScheduler

protected:
	void SchedulingModeUpdate(void);

private:
	CSystem*    m_pSystem;
	ICVar*      m_svSchedulingAffinity;
	ICVar*      m_svSchedulingClientTimeout;
	ICVar*      m_svSchedulingServerTimeout;
	ICVar*      m_svSchedulingBucket;
	ICVar*      m_svSchedulingMode;
	ICVar*      m_svSliceLoadEnable;
	ICVar*      m_svSliceLoadBudget;
	ICVar*      m_svSliceLoadLogging;

	CTimeValue  m_lastSliceCheckTime;

	i32         m_sliceLoadingRef;

	tukk m_pLastSliceName;
	i32         m_lastSliceLine;
};

// Summary:
//	 Creates the system scheduler interface.
void CreateSystemScheduler(CSystem* pSystem);

#endif //__SYSTEMSCHEDULER_H__
