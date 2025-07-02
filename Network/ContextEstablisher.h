// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CONTEXTESTABLISHER_H__
#define __CONTEXTESTABLISHER_H__

#pragma once

#include <drx3D/Network/INetwork.h>
#include <drx3D/Network/Config.h>
#include <drx3D/Network/NetChannel.h>

class CContextEstablisher : public IContextEstablisher, public _reference_target_t
{
public:
	CContextEstablisher();
	~CContextEstablisher();

	void OnFailDisconnect(CNetChannel* pChannel) { m_disconnectOnFail = pChannel; }

	void AddTask(EContextViewState state, IContextEstablishTask* pTask);
	void Start();
	bool StepTo(SContextEstablishState& state);
	void Fail(EDisconnectionCause cause, const string& reason);
	bool IsDone() { return m_done || m_offset < 0 || m_offset == m_tasks.size(); }
	void Done();

	void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CContextEstablisher");

		pSizer->Add(*this);
		pSizer->AddContainer(m_tasks);
	}

#ifndef _RELEASE
	void OutputTiming(void);
#endif
#if ENABLE_DEBUG_KIT
	void DebugDraw();
#endif

	const string& GetFailureReason() { return m_failureReason; }

private:
	EContextEstablishTaskResult PerformSteps(i32& offset, SContextEstablishState& state);

	struct STask
	{
		STask() : state(eCVS_Initial), pTask(0)
#ifndef _RELEASE
			, numRuns(0)
			, done(0.0f)
#endif
		{
		}
		STask(EContextViewState s, IContextEstablishTask* p) : state(s), pTask(p)
#ifndef _RELEASE
			, numRuns(0)
			, done(0.0f)
#endif
		{
		}

		EContextViewState      state;
		IContextEstablishTask* pTask;

#ifndef _RELEASE
		i32        numRuns;
		CTimeValue done;
#endif

		bool operator<(const STask& rhs) const
		{
			return state < rhs.state;
		}
	};
	// -1 if not started, otherwise how far down are we
	i32                     m_offset;
	std::vector<STask>      m_tasks;
	bool                    m_done;

	string                  m_failureReason;
	i32                     m_debuggedFrame;

	_smart_ptr<CNetChannel> m_disconnectOnFail;

	CTimeValue              m_begin;
};

typedef _smart_ptr<CContextEstablisher> CContextEstablisherPtr;

#endif
