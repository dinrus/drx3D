// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

#include <DrxPhysics/primitives.h>
#include <DrxPhysics/physinterface.h>

#include <vector>
#include <memory.h>

struct SPhysSrcMesh
{
	std::unique_ptr<IGeometry>     pMesh;
	std::vector<Vec3_tpl<index_t>> faces;
	uint64                         closedIsles, restoredIsles;
	std::vector<i32>               isleTriCount;
	std::vector<i32>               isleTriClosed;
};

struct SPhysProxies
{
	std::vector<phys_geometry*>   proxyGeometries;

	std::shared_ptr<SPhysSrcMesh> pSrc;
	IGeometry::SProxifyParams     params;
	i32                           nMeshes;
	bool                          ready;

	void                          Serialize(Serialization::IArchive& ar);
};

