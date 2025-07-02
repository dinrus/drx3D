// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef AdvantagePointOccupancyControl_h
#define AdvantagePointOccupancyControl_h



#if defined(DRXAISYSTEM_DEBUG)
struct IRenderAuxGeom;
#endif

class CAdvantagePointOccupancyControl
{
	typedef std::map<EntityId, Vec3> OccupiedAdvantagePoints;

public:
	CAdvantagePointOccupancyControl();
	~CAdvantagePointOccupancyControl();

	void Reset();
	void OccupyAdvantagePoint(EntityId entityId, const Vec3& position);
	void ReleaseAdvantagePoint(EntityId entityId);
	bool IsAdvantagePointOccupied(const Vec3& position) const;
	bool IsAdvantagePointOccupied(const Vec3& position, const EntityId ignoreEntityId) const;

	void Update();

private:

	bool MatchAdvantagePointPosition(const Vec3& position, const Vec3& AdvantagePoint) const;

#if defined(DRXAISYSTEM_DEBUG)

	void DebugDraw() const;
	inline void DebugDrawAdvantagePoint(IRenderAuxGeom* debugRenderer, const EntityId entityID, const Vec3& occupiedPos) const;

#endif


	// TODO: Investigate if we can use tAIObjectID instead of position /Mario
	OccupiedAdvantagePoints m_occupiedAdvantagePoints;

#if defined(DRXAISYSTEM_DEBUG)
	// 0 if debug visualization is disabled, otherwise enabled.
	static i32 ai_AdvantagePointOccupancyDebug;
#endif
};

#endif //AdvantagePointOccupancyControl_h