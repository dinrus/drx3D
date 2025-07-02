// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/MNMBaseTypes.h>
#include <drx3D/AI/MNMFixedAABB.h>
#include <drx3D/AI/MNMFixedVec2.h>
#include <drx3D/AI/MNMFixedVec3.h>

#include <drx3D/CoreX/Math/Drx_Geo.h>

namespace MNM
{
typedef fixed_t<i32, 16>   real_t;
typedef FixedVec2<i32, 16> vector2_t;
typedef FixedVec3<i32, 16> vector3_t;
typedef FixedAABB<i32, 16> aabb_t;

namespace Tile
{

typedef u16               VertexIndex;
typedef FixedVec3<u16, 5> Vertex;
typedef FixedAABB<u16, 5> AABB;

struct STriangle
{
	STriangle() {}

	VertexIndex    vertex[3];        //!< Vertex index into Tile::vertices array
	u16         linkCount : 4;    //!< Total link count connected to this triangle (begins at 'firstLink')
	u16         firstLink : 12;   //!< First link index into Tile::links array
	StaticIslandID islandID;         //!< Island for this triangle

	AreaAnnotation  areaAnnotation;

	AUTO_STRUCT_INFO;
};

struct SLink
{
	u16 side     : 4; //!< Side of the tile (cube) this link is associated with [0..13] or enumerated link type
	// See MNM::NavMesh::GetNeighbourTileOffset()
	u16 edge     : 2; //!< Local edge index of the triangle this link is on [0..2]
	// (NOTE: 'edge' is not used for off-mesh links)
	u16 triangle : 10;   //!< Index into Tile::triangles
	// (NOTE: This bit count makes our max triangle count 1024)
	// (NOTE: 'triangle' is re-purposed to be the index of the off-mesh index for off-mesh links)
	// #MNM_TODO pavloi 2016.07.26: gather all places, where max triangle count is implied and tie them to one variable, or
	// at least cover them with COMPILE_TIME_ASSERT's.

	// This enumeration can be assigned to the 'side' variable
	// which generally would index the 14 element array
	// NeighbourOffset_MeshGrid, found in MNM::NavMesh::GetNeighbourTileOffset().
	// If 'side' is found to be one of the values below,
	// it will not be used as a lookup into the table but instead
	// as a flag so we need to be sure these start after the final
	// index and do not exceed the 'side' variable's bit count.
	enum { OffMesh = 0xe, };    // A link to a nonadjacent triangle
	enum { Internal = 0xf, };   // A link to an adjacent triangle (not exposed to the tile edge)

	AUTO_STRUCT_INFO;
};

struct SBVNode
{
	u16     leaf   : 1;
	u16     offset : 15;

	Tile::AABB aabb;

	AUTO_STRUCT_INFO;
};

//! This structure is used to share a read-only access to the NavMesh tile data.
struct STileData
{
	const Tile::STriangle* pTriangles;
	const Tile::Vertex*    pVertices;
	const Tile::SLink*     pLinks;
	const Tile::SBVNode*   pNodes;

	u16                 trianglesCount;
	u16                 verticesCount;
	u16                 linksCount;
	u16                 bvNodesCount;

	u32                 hashValue;

	// #MNM_TODO pavloi 2016.08.15: there is code, which delivers tile coord as vector3_t (see NavMesh::GetTileContainerCoordinates)
	// but it's a bit wasteful - forces user to get values using as_int(), do bit shifting, hard to read without visualizers, etc.
	// Just introduce new structure, which holds u32 x,y,z and pass it everywhere. Or use Vec3i.
	vector3_t tileGridCoord;                     //!< Tile coordinates in the mesh grid (integer values).
	vector3_t tileOriginWorld;                   //!< Origin position of the tile in the world coordinate space.
};

//! Contains utility functions to read and manipulate tile data.
namespace Util
{
inline vector3_t GetVertexPosWorldFixed(const Tile::STriangle& triangle, const Tile::Vertex* pVertices, const size_t vertexIdx, const vector3_t& tileOrigin)
{
	const Tile::Vertex& vertex = pVertices[triangle.vertex[vertexIdx]];
	return tileOrigin + vector3_t(vertex);
}

inline vector3_t GetVertexPosWorldFixed(const STileData& tileData, const size_t triangleIdx, const size_t vertexIdx)
{
	return GetVertexPosWorldFixed(tileData.pTriangles[triangleIdx], tileData.pVertices, vertexIdx, tileData.tileOriginWorld);
}

inline Triangle GetTriangleWorld(const STileData& tileData, const size_t triangleIdx)
{
	Triangle t;
	t.v0 = GetVertexPosWorldFixed(tileData, triangleIdx, 0).GetVec3();
	t.v1 = GetVertexPosWorldFixed(tileData, triangleIdx, 1).GetVec3();
	t.v2 = GetVertexPosWorldFixed(tileData, triangleIdx, 2).GetVec3();
	return t;
}

}   // namespace Util

} //endns Tile
} //endns MNM
