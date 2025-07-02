// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __DeferredActionQueue_h__
#define __DeferredActionQueue_h__

#pragma once

#include "AgePriorityQueue.h"

#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/CoreX/functor.h>
#include <drx3D/CoreX/Memory/STLPoolAllocator.h>
#include <drx3D/CoreX/Memory/STLPoolAllocator_ManyElems.h>

struct NoContention
{
protected:
	NoContention(u32 quota) {}

	void PerformedSync()  {}
	void PerformedAsync() {}

	bool CanPerformAsync()
	{
		return true;
	}

	void UpdateStart()    {}
	void UpdateComplete() {}
};

struct DefaultContention
{
	void SetQuota(u32 quota)
	{
		m_quota = quota;
	}

	u32 GetQuota() const
	{
		return m_quota;
	}

	struct ContentionStats
	{
		u32 quota;
		u32 queueSize;
		u32 peakQueueSize;

		u32 immediateCount;
		u32 peakImmediateCount;
		u32 deferredCount;
		u32 peakDeferredCount;

		float  immediateAverage;
		float  deferredAverage;
	};

	ContentionStats GetContentionStats()
	{
		ContentionStats stats;
		stats.quota = m_quota;
		stats.immediateCount = m_immWindow[(m_averageHead - 1) % AverageWindowWidth];
		stats.peakImmediateCount = m_peakImmCount;
		stats.deferredCount = m_defWindow[(m_averageHead - 1) % AverageWindowWidth];
		stats.peakDeferredCount = m_peakDefCount;

		u32 immSum = 0;
		u32 defSum = 0;

		u32 count = std::min<u32>(AverageWindowWidth, m_averageHead - 5);
		for (u32 i = 0; i < count; ++i)
		{
			immSum += m_immWindow[i];
			defSum += m_defWindow[i];
		}

		stats.immediateAverage = immSum / (float)AverageWindowWidth;
		stats.deferredAverage = defSum / (float)AverageWindowWidth;

		stats.queueSize = m_queueSize;
		stats.peakQueueSize = m_peakQueueSize;

		return stats;
	}

	void ResetContentionStats()
	{
		m_immCount = 0;
		m_peakImmCount = 0;
		m_defCount = 0;
		m_peakDefCount = 0;
		m_queueSize = 0;
		m_peakQueueSize = 0;
		m_averageHead = 0;

		for (u32 i = 0; i < AverageWindowWidth; ++i)
		{
			m_immWindow[i] = 0;
			m_defWindow[i] = 0;
		}
	}

protected:
	DefaultContention(u32 quota = 64)
		: m_quota(quota)
		, m_immCount(0)
		, m_peakImmCount(0)
		, m_defCount(0)
		, m_peakDefCount(0)
		, m_queueSize(0)
		, m_peakQueueSize(0)
		, m_averageHead(0)
	{
		for (u32 i = 0; i < AverageWindowWidth; ++i)
		{
			m_immWindow[i] = 0;
			m_defWindow[i] = 0;
		}
	}

	inline void PerformedImmediate()
	{
		++m_immCount;
	}

	inline void PerformedDeferred()
	{
		++m_defCount;
	}

	inline bool CanPerformDeferred()
	{
		return m_defCount < m_quota;
	}

	void UpdateStart(u32 queueSize)
	{
		m_queueSize = queueSize;
		if (m_peakQueueSize < queueSize)
			m_peakQueueSize = queueSize;
	}

	void UpdateComplete(u32 queueSize)
	{
		m_immWindow[m_averageHead % AverageWindowWidth] = m_immCount;
		m_defWindow[m_averageHead % AverageWindowWidth] = m_defCount;

		++m_averageHead;

		if (m_immCount > m_peakImmCount)
			m_peakImmCount = m_immCount;

		if (m_defCount > m_peakDefCount)
			m_peakDefCount = m_defCount;

		m_immCount = 0;
		m_defCount = 0;
	}

protected:
	u32 m_quota;

	u32 m_immCount;
	u32 m_peakImmCount;
	u32 m_defCount;
	u32 m_peakDefCount;

	u32 m_queueSize;
	u32 m_peakQueueSize;

	enum
	{
		AverageWindowWidth = 10,
	};

	u32 m_averageHead;
	u32 m_immWindow[AverageWindowWidth];
	u32 m_defWindow[AverageWindowWidth];
};

struct ConservativeContention
	: public DefaultContention
{
	inline bool CanPerformDeferred()
	{
		u32 quota = m_quota;
		if (m_immCount >= quota)
			quota += 2;

		return (m_immCount + m_defCount < quota);
	}
};

template<typename CasterType, typename RequestType, typename ResultType, typename ContentionPolicyType = DefaultContention>
class DeferredActionQueue :
	public CasterType,
	public ContentionPolicyType
{
	typedef DeferredActionQueue<CasterType, RequestType, ResultType, ContentionPolicyType> Type;
public:
	typedef CasterType                                                                     Caster;
	typedef ContentionPolicyType                                                           ContentionPolicy;
	typedef typename RequestType::Priority                                                 PriorityType;
	typedef Functor2wRet<u32k&, RequestType&, bool>                                SubmitCallback;
	typedef Functor2<u32k&, const ResultType&>                                     ResultCallback;

	struct PriorityClass
	{
		PriorityClass()
			: basePriority(1.0f)
			, growthFactor(1.0f)
			, growthTime(1.0f)
		{
		}

		PriorityClass(float _basePriority, float _growthFactor, float _growthTime)
			: basePriority(_basePriority)
			, growthFactor(_growthFactor)
			, growthTime(_growthTime)
		{
		}

		float basePriority;
		float growthFactor;
		float growthTime;
	};

	DeferredActionQueue()
		: m_slotGenID(0)
	{
		Caster::SetCallback(functor(*this, &Type::CastComplete));

		m_priorityClasses.resize(RequestType::HighestPriority + 1);
		m_priorityClasses[RequestType::LowPriority] = PriorityClass(1.0f, 100.0f, 0.5f);
		m_priorityClasses[RequestType::MediumPriority] = PriorityClass(10.0f, 10.0f, 0.4f);
		m_priorityClasses[RequestType::HighPriority] = PriorityClass(25.0f, 5.0f, 0.3f);
		m_priorityClasses[RequestType::HighestPriority] = PriorityClass(50.0f, 2.5f, 0.2f);
	}

	inline void Reset()
	{
		m_priorityQueue.clear();
		m_slots.clear();
		m_submitted.clear();
	}

	inline const ResultType& Cast(const RequestType& request)
	{
		ContentionPolicy::PerformedImmediate();

		return Caster::Cast(request);
	}

	//! Puts a placeholder in queue - use submitCallback to fill in the request.
	inline u32 Queue(const PriorityType& priority, const ResultCallback& resultCallback,
	                    const SubmitCallback& submitCallback)
	{
		assert(resultCallback != 0);
		assert(submitCallback != 0);

		return m_priorityQueue.push_back(QueuedRequest(priority, resultCallback, submitCallback));
	}

	inline u32 Queue(const PriorityType& priority, const RequestType& request,
	                    const ResultCallback& resultCallback, const SubmitCallback& submitCallback = 0)
	{
		assert(resultCallback != 0);

		QueuedRequest queued(priority, request, resultCallback, submitCallback);

		Caster::Acquire(queued.request);

		return m_priorityQueue.push_back(queued);
	}

	inline void Cancel(u32k& queuedID)
	{
		typename Submitted::iterator it = m_submitted.find(queuedID);
		if (it == m_submitted.end())
		{
			if (m_priorityQueue.has(queuedID))
			{
				QueuedRequest& queued = m_priorityQueue[queuedID];

				Caster::Release(queued.request);

				m_priorityQueue.erase(queuedID);
			}
		}
		else
		{
			m_slots.erase(it->second);
			m_submitted.erase(it);
		}
	}

	inline void Update(float updateTime)
	{
		ContentionPolicy::UpdateStart(m_priorityQueue.size());

		if (!m_priorityQueue.empty() && ContentionPolicy::CanPerformDeferred())
		{
			PriorityClassUpdate doUpdate(m_priorityClasses);
			typename PriorityQueue::DefaultCompare doCompare;
			m_priorityQueue.partial_update(ContentionPolicy::GetQuota(), updateTime, doUpdate, doCompare);

			while (!m_priorityQueue.empty() && ContentionPolicy::CanPerformDeferred())
			{
				u32k& queuedID = m_priorityQueue.front_id();
				QueuedRequest& queued = m_priorityQueue.front();

				if (queued.submitCallback)
				{
					RequestType requestCopy(queued.request);

					if (!queued.submitCallback(queuedID, queued.request))
					{
						Caster::Release(requestCopy);

						m_priorityQueue.pop_front();
						continue;
					}

					Caster::Acquire(queued.request);
					Caster::Release(requestCopy);
				}

				Submit(queuedID, queued);

				Caster::Release(queued.request);

				m_priorityQueue.pop_front();
			}
		}

		ContentionPolicy::UpdateComplete(m_priorityQueue.size());
	}

	inline void SetPriorityClass(const PriorityType& priority, const PriorityClass& priorityClass)
	{
		m_priorityClasses[priority] = priorityClass;
	}

protected:
	struct Slot
	{
		Slot()
			: queuedID(0)
			, callback(0)
		{
		}
		Slot(u32k& _queuedID, const ResultCallback& _callback)
			: queuedID(_queuedID)
			, callback(_callback)
		{
		}

		u32         queuedID;
		ResultCallback callback;
	};

	u32 m_slotGenID;
	typedef std::map<u32, Slot> Slots;
	Slots  m_slots;

	typedef std::map<u32, u32> Submitted;
	Submitted m_submitted;

	struct QueuedRequest
	{
		QueuedRequest(const PriorityType& _priority, const ResultCallback& _callback, const SubmitCallback& _submitCallback)
			: priority(_priority)
			, resultCallback(_callback)
			, submitCallback(_submitCallback)
		{
		}

		QueuedRequest(const PriorityType& _priority, const RequestType& _request, const ResultCallback& _callback,
		              const SubmitCallback& _submitCallback)
			: priority(_priority)
			, request(_request)
			, resultCallback(_callback)
			, submitCallback(_submitCallback)
		{
		}

		PriorityType   priority;
		RequestType    request;
		ResultCallback resultCallback;
		SubmitCallback submitCallback;
	};

	typedef AgePriorityQueue<QueuedRequest> PriorityQueue;
	PriorityQueue m_priorityQueue;

	typedef std::vector<PriorityClass> PriorityClasses;
	PriorityClasses m_priorityClasses;

	struct PriorityClassUpdate
	{
		PriorityClassUpdate(const PriorityClasses& _priorityClasses)
			: priorityClasses(_priorityClasses)
		{
		}

		float operator()(const float& age, QueuedRequest& value)
		{
			const PriorityClass& priorityClass = priorityClasses[value.priority];
			return priorityClass.basePriority * pow_tpl(priorityClass.growthFactor, age / priorityClass.growthTime);
		}

		const PriorityClasses& priorityClasses;
	};

	inline void Submit(u32k& queuedID, const QueuedRequest& queued)
	{
		++m_slotGenID;
		while (!m_slotGenID)
			++m_slotGenID;

		m_slots.insert(typename Slots::value_type(m_slotGenID, Slot(queuedID, queued.resultCallback)));
		m_submitted.insert(typename Submitted::value_type(queuedID, m_slotGenID));

		SubmitQueuedCast(m_slotGenID, queuedID, queued);

		ContentionPolicy::PerformedDeferred();
	}

	inline void SubmitQueuedCast(u32 slotID, u32k& queuedID, const QueuedRequest& queued)
	{
		Caster::Queue(slotID, queued.request);
	}

	void CastComplete(u32 slotID, const ResultType& result)
	{
		typename Slots::iterator it = m_slots.find(slotID);
		if (it != m_slots.end())
		{
			const Slot& slot = it->second;

			slot.callback(slot.queuedID, result);
			m_submitted.erase(slot.queuedID);
			m_slots.erase(it);
		}
	}
};

#endif
