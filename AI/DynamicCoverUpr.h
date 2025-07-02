// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __DynamicCoverUpr_h__
#define __DynamicCoverUpr_h__
#pragma once

#include <drx3D/AI/Cover.h>
#include <drx3D/AI/EntityCoverSampler.h>

#include <drx3D/AI/HashGrid.h>

class DynamicCoverUpr :
	public IEntityEventListener
{
	struct EntityCoverState;
public:
	struct ValidationSegment
	{
		ValidationSegment()
			: center(ZERO)
			, normal(ZERO)
			, height(.0f)
			, length(.0f)
			, surfaceID(0)
			, segmentIdx(0)
			, flags(0)
		{
		}

		enum Flags
		{
			Disabled   = 1 << 0,
			Validating = 1 << 1,
		};

		Vec3           center;
		Vec3           normal;

		float          height;
		float          length;

		CoverSurfaceID surfaceID;
		u16         segmentIdx;
		u16         flags;
	};

	DynamicCoverUpr();

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity* entity, const SEntityEvent& event);
	//~IEntityEventListener

	void AddValidationSegment(const ValidationSegment& segment);
	void RemoveSurfaceValidationSegments(const CoverSurfaceID& surfaceID);
	void AddEntity(EntityId entityID);
	void RemoveEntity(EntityId entityID);

	void Reset();
	void Clear();
	void ClearValidationSegments();
	void Update(float updateTime);

	void BreakEvent(const Vec3& position, float radius);
	void MoveEvent(EntityId entityID, const Matrix34& worldTM);

private:
	void RemoveEntity(EntityId entityID, EntityCoverState& state);
	void EntityCoverSampled(EntityId entityID, EntityCoverSampler::ESide side, const ICoverSystem::SurfaceInfo& surfaceInfo);
	void RemoveEntityCoverSurfaces(EntityCoverState& state);

	void QueueValidation(i32 index);
	void ValidateOne();
	void IntersectionTestComplete(const QueuedIntersectionID& intID, const IntersectionTestResult& result);

	typedef std::vector<ValidationSegment> Segments;
	Segments m_segments;

	typedef std::vector<u16> FreeSegments;
	FreeSegments m_freeSegments;

	struct segment_position
	{
		segment_position(const Segments& segments)
			: m_segments(segments)
		{
		}

		Vec3 operator()(i32 segmentIdx) const
		{
			return m_segments[segmentIdx].center;
		}
	private:
		const Segments& m_segments;
	};

	typedef hash_grid<256, u32, hash_grid_2d<Vec3, Vec3i>, segment_position> SegmentsGrid;
	SegmentsGrid m_segmentsGrid;

	typedef std::vector<i32> DirtySegments;
	DirtySegments m_dirtyBuf;

	struct QueuedValidation
	{
		QueuedValidation()
			: validationSegmentIdx(-1)
			, waitingCount(0)
			, negativeCount(0)
		{
		}

		QueuedValidation(i32 index)
			: validationSegmentIdx(index)
			, waitingCount(0)
			, negativeCount(0)
		{
		}

		i32   validationSegmentIdx;

		u8 waitingCount;
		u8 negativeCount;
	};

	typedef std::deque<QueuedValidation> ValidationQueue;
	ValidationQueue m_validationQueue;

	struct EntityCoverState
	{
		enum EState
		{
			Moving,
			Sampling,
			Sampled,
		};

		EntityCoverState()
			: lastMovement((int64)0ll)
			, state(Moving)
		{
			surfaceID[EntityCoverSampler::Left] = CoverSurfaceID(CoverSurfaceID(0));
			surfaceID[EntityCoverSampler::Right] = CoverSurfaceID(CoverSurfaceID(0));
			surfaceID[EntityCoverSampler::Front] = CoverSurfaceID(CoverSurfaceID(0));
			surfaceID[EntityCoverSampler::Back] = CoverSurfaceID(CoverSurfaceID(0));

			memset(&lastWorldTM, 0, sizeof(Matrix34));
		}

		EntityCoverState(const CTimeValue& now)
			: lastMovement(now)
			, state(Moving)
		{
			surfaceID[EntityCoverSampler::Left] = CoverSurfaceID(CoverSurfaceID(0));
			surfaceID[EntityCoverSampler::Right] = CoverSurfaceID(CoverSurfaceID(0));
			surfaceID[EntityCoverSampler::Front] = CoverSurfaceID(CoverSurfaceID(0));
			surfaceID[EntityCoverSampler::Back] = CoverSurfaceID(CoverSurfaceID(0));

			memset(&lastWorldTM, 0, sizeof(Matrix34));
		}

		CTimeValue     lastMovement;
		CoverSurfaceID surfaceID[EntityCoverSampler::LastSide + 1];
		Matrix34       lastWorldTM;
		EState         state;
	};

	typedef VectorMap<EntityId, EntityCoverState> EntityCover;
	EntityCover        m_entityCover;

	EntityCoverSampler m_entityCoverSampler;
};

#endif
