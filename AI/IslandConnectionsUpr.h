// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __IslandConnectionsUpr_h__
#define __IslandConnectionsUpr_h__

#pragma once

#include <drx3D/AI/INavigationSystem.h>
#include <drx3D/AI/IslandConnections.h>

class IslandConnectionsUpr
{
public:
	IslandConnectionsUpr();
	void                    Reset();

	MNM::IslandConnections& GetIslandConnections();

	void                    SetOneWayConnectionBetweenIsland(const MNM::GlobalIslandID fromIsland, const MNM::IslandConnections::Link& link);

	bool                    AreIslandsConnected(const IEntity* pEntityToTestOffGridLinks, const MNM::GlobalIslandID startIsland, const MNM::GlobalIslandID endIsland, const INavMeshQueryFilter* pFilter) const;

#ifdef DRXAISYS_DEBUG
	void DebugDraw() const;
#endif

private:
	MNM::IslandConnections m_globalIslandConnections;
};

#endif // __IslandConnectionsUpr_h__
