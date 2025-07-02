// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RayCastQueue_h__
#define __RayCastQueue_h__

#pragma once

#include "DeferredActionQueue.h"
#include <drx3D/Phys/physinterface.h>

struct RayCastResult
{
	enum
	{
		MaxHitCount = 4, //!< Allow up to 4 hits.
	};

	operator bool() const
	{
		return hitCount != 0;
	}

	const ray_hit& operator[](u32 index) const
	{
		assert(index < (u32)hitCount);
		assert(index < MaxHitCount);
		return hits[index];
	}

	ray_hit& operator[](u32 index)
	{
		assert(index < (u32)hitCount);
		assert(index < MaxHitCount);
		return hits[index];
	}

	const ray_hit* operator->() const
	{
		assert(hitCount > 0);
		return &hits[0];
	}

	ray_hit hits[MaxHitCount];
	i32     hitCount;
};

struct RayCastRequest
{
	enum
	{
		MaxSkipListCount = 64,
	};

	enum Priority
	{
		LowPriority = 0,
		MediumPriority,
		HighPriority,
		HighestPriority,

		TotalNumberOfPriorities
	};

	RayCastRequest()
		: skipListCount(0)
		, maxHitCount(1)
		, pLastHit(nullptr)
	{
	}

	RayCastRequest(const Vec3& _pos, const Vec3& _dir, i32 _objTypes, i32 _flags = 0, IPhysicalEntity** _skipList = 0,
			u8 _skipListCount = 0, u8 _maxHitCount = 1, ray_hit_cached* _pLastHit = nullptr)
		: pos(_pos)
		, dir(_dir)
		, objTypes(_objTypes)
		, flags(_flags)
		, skipListCount(_skipListCount)
		, maxHitCount(_maxHitCount)
		, pLastHit(_pLastHit)
	{
		assert(maxHitCount <= RayCastResult::MaxHitCount);
		assert(skipListCount <= MaxSkipListCount);

		skipListCount = std::min<u32>(skipListCount, MaxSkipListCount);

		u32 k = 0;
		for (u32 i = 0; i < skipListCount; ++i)
		{
			assert(_skipList[i]);
			if (_skipList[i])
				skipList[k++] = _skipList[i];
		}

		skipListCount = k;
	}

	RayCastRequest& operator=(const RayCastRequest& src) { memcpy(this, &src, sizeof(*this)); return *this; }

	Vec3             pos;
	Vec3             dir;

	i32              objTypes;
	i32              flags;

	u8            skipListCount;
	IPhysicalEntity* skipList[MaxSkipListCount];
	u8            maxHitCount;
	ray_hit_cached*  pLastHit;
};

template<i32 RayCasterID>
struct DefaultRayCaster
{
protected:
	typedef DefaultRayCaster<RayCasterID>          Type;
	typedef Functor2<u32, const RayCastResult&> Callback;

	DefaultRayCaster()
		: callback(0)
	{
	}

	ILINE void Acquire(RayCastRequest& request)
	{
		for (size_t i = 0; i < request.skipListCount; ++i)
			request.skipList[i]->AddRef();
	}

	ILINE void Release(RayCastRequest& request)
	{
		for (size_t i = 0; i < request.skipListCount; ++i)
			request.skipList[i]->Release();
	}

	ILINE IPhysicalWorld::SRWIParams GetRWIParams(const RayCastRequest& request)
	{
		IPhysicalWorld::SRWIParams params;
		params.org = request.pos;
		params.dir = request.dir;
		params.objtypes = request.objTypes;
		params.flags = request.flags;
		params.nMaxHits = request.maxHitCount;
		params.pSkipEnts = request.skipListCount ? const_cast<IPhysicalEntity**>(&request.skipList[0]) : 0;
		params.nSkipEnts = request.skipListCount;
		params.phitLast = request.pLastHit;

		return params;
	}

	inline const RayCastResult& Cast(const RayCastRequest& request)
	{
		assert(request.maxHitCount <= RayCastResult::MaxHitCount);
		assert(request.skipListCount <= RayCastRequest::MaxSkipListCount);
		assert((request.flags & rwi_queue) == 0);

		IPhysicalWorld::SRWIParams params = GetRWIParams(request);
		params.hits = &m_resultBuf.hits[0];

		m_resultBuf.hitCount = (u32)gEnv->pPhysicalWorld->RayWorldIntersection(params);

		return m_resultBuf;
	}

	inline void Queue(u32 rayID, const RayCastRequest& request)
	{
		assert(request.maxHitCount <= RayCastResult::MaxHitCount);
		assert(request.skipListCount <= RayCastRequest::MaxSkipListCount);

		IPhysicalWorld::SRWIParams params = GetRWIParams(request);
		params.flags |= rwi_queue;
		params.pForeignData = this;
		params.iForeignData = rayID;
		params.OnEvent = OnRWIResult;

		if (gEnv->pPhysicalWorld->RayWorldIntersection(params) == 0)
		{
			// The ray was unsuccessfully submitted.
			// It can happen when direction is zero.
			// Consider it a miss.
			m_resultBuf.hitCount = (u32)0;
			callback(rayID, m_resultBuf);
		}
	}

	inline void SetCallback(const Callback& _callback)
	{
		callback = _callback;
	}

	static i32 OnRWIResult(const EventPhysRWIResult* result)
	{
		Type* _this = static_cast<Type*>(result->pForeignData);
		i32 rayID = result->iForeignData;

		assert(result->nHits <= RayCastResult::MaxHitCount);

		_this->m_resultBuf.hitCount = (u32)result->nHits;

		if (result->nHits > 0)
		{
			i32 j = result->pHits[0].dist < 0.0f ? 1 : 0;
			for (i32 i = 0; i < _this->m_resultBuf.hitCount; ++i, ++j)
				_this->m_resultBuf.hits[i] = result->pHits[j];
		}

		_this->callback(rayID, _this->m_resultBuf);

		return 1;
	}

private:
	Callback      callback;
	RayCastResult m_resultBuf;
};

typedef u32 QueuedRayID;

template<i32 RayCasterID>
class RayCastQueue :
	public DeferredActionQueue<DefaultRayCaster<RayCasterID>, RayCastRequest, RayCastResult>
{
public:
	typedef DeferredActionQueue<DefaultRayCaster<RayCasterID>, RayCastRequest, RayCastResult> BaseType;
};

#endif
