// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IUpdateScheduler.h>

namespace sxema
{
class CUpdateScheduler : public IUpdateScheduler
{
private:

	struct SSlot
	{
		SSlot();
		SSlot(u32 _firstBucketIdx, UpdateFrequency _frequency, UpdatePriority _priority);

		CScopedConnection connection;
		u32            firstBucketIdx;
		UpdateFrequency   frequency;
		UpdatePriority    priority;
	};

	typedef std::vector<SSlot> SlotVector;

	struct SFindSlot // #SchematycTODO : Replace with lambda?
	{
		inline SFindSlot(const CConnectionScope* _pScope)
			: pScope(_pScope)
		{}

		inline bool operator()(const SSlot& lhs) const
		{
			return lhs.connection.GetScope() == pScope;
		}

		const CConnectionScope* pScope;
	};

	struct SObserver
	{
		SObserver(const CConnectionScope* _pScope, const UpdateCallback& _callback, UpdateFrequency _frequency, UpdatePriority _priority, const UpdateFilter& _filter);

		const CConnectionScope* pScope;
		UpdateCallback          callback;
		UpdateFrequency         frequency;
		UpdatePriority          currentPriority;
		UpdatePriority          newPriority;
		UpdateFilter            filter;
	};

	typedef std::vector<SObserver> ObserverVector;

	struct SSortObservers // #SchematycTODO : Replace with lambda?
	{
		inline bool operator()(const SObserver& lhs, const SObserver& rhs) const
		{
			//return (lhs.currentPriority > rhs.currentPriority) || ((lhs.currentPriority == rhs.currentPriority) && (lhs.callback < rhs.callback));
			return (lhs.currentPriority > rhs.currentPriority);
		}
	};

	struct SLowerBoundObserver // #SchematycTODO : Replace with lambda?
	{
		inline bool operator()(const SObserver& lhs, const UpdatePriority& rhs) const
		{
			return lhs.currentPriority > rhs;
		}
	};

	class CBucket
	{
	public:

		CBucket();

		void          Connect(CConnectionScope& scope, const UpdateCallback& callback, UpdateFrequency frequency, UpdatePriority priority, const UpdateFilter& filter);
		void          Disconnect(CConnectionScope& scope, UpdatePriority priority);
		void          BeginUpdate();
		void          Update(const SUpdateContext (&frequencyUpdateContexts)[EUpdateFrequency::Count], UpdatePriority beginPriority, UpdatePriority endPriority);
		void          EndUpdate();
		void          Reset();

		inline u32 GetSize() const
		{
			return m_observers.size() - m_garbageCount;
		}

	private:

		SObserver* FindObserver(CConnectionScope& scope, UpdatePriority priority);

		ObserverVector m_observers;
		u32         m_dirtyCount;
		u32         m_garbageCount;
		u32         m_pos;
	};

	enum : u32
	{
		BucketCount = 1u << (EUpdateFrequency::Count - 1)
	};

public:

	CUpdateScheduler();

	~CUpdateScheduler();

	// IUpdateScheduler
	virtual bool Connect(const SUpdateParams& params) override;
	virtual void Disconnect(CConnectionScope& scope) override;
	virtual bool InFrame() const override;
	virtual bool BeginFrame(float frameTime) override;
	virtual bool Update(UpdatePriority beginPriority = EUpdateStage::PrePhysics | EUpdateDistribution::Earliest, UpdatePriority endPriority = EUpdateStage::Post | EUpdateDistribution::End) override;
	virtual bool EndFrame() override;
	virtual void VerifyCleanup() override;
	virtual void Reset() override;
	// ~IUpdateScheduler

private:

	SlotVector m_slots;
	CBucket    m_buckets[BucketCount];
	float      m_frameTimes[BucketCount];
	u32     m_currentBucketIdx;
	bool       m_bInFrame;
};
} // sxema
