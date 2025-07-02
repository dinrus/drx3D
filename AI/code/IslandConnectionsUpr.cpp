// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/IslandConnectionsUpr.h>

IslandConnectionsUpr::IslandConnectionsUpr()
{
}

void IslandConnectionsUpr::Reset()
{
	m_globalIslandConnections.Reset();
}

MNM::IslandConnections& IslandConnectionsUpr::GetIslandConnections()
{
	return m_globalIslandConnections;
}

void IslandConnectionsUpr::SetOneWayConnectionBetweenIsland(const MNM::GlobalIslandID fromIsland, const MNM::IslandConnections::Link& link)
{
	m_globalIslandConnections.SetOneWayOffmeshConnectionBetweenIslands(fromIsland, link);
}

bool IslandConnectionsUpr::AreIslandsConnected(const IEntity* pEntityToTestOffGridLinks, const MNM::GlobalIslandID startIsland, const MNM::GlobalIslandID endIsland, const INavMeshQueryFilter* pFilter) const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	MNM::IslandConnections::TIslandsWay way;
	return m_globalIslandConnections.CanNavigateBetweenIslands(pEntityToTestOffGridLinks, startIsland, endIsland, pFilter, way);
}

#ifdef DRXAISYS_DEBUG

void IslandConnectionsUpr::DebugDraw() const
{
	m_globalIslandConnections.DebugDraw();
}

#endif
