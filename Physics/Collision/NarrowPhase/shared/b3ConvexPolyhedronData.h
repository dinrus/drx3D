
#ifndef D3_CONVEX_POLYHEDRON_DATA_H
#define D3_CONVEX_POLYHEDRON_DATA_H

#include <drx3D/Common/shared/b3Float4.h>
#include <drx3D/Common/shared/b3Quat.h>

typedef struct b3GpuFace b3GpuFace_t;
struct b3GpuFace
{
	b3Float4 m_plane;
	i32 m_indexOffset;
	i32 m_numIndices;
	i32 m_unusedPadding1;
	i32 m_unusedPadding2;
};

typedef struct b3ConvexPolyhedronData b3ConvexPolyhedronData_t;

struct b3ConvexPolyhedronData
{
	b3Float4 m_localCenter;
	b3Float4 m_extents;
	b3Float4 mC;
	b3Float4 mE;

	float m_radius;
	i32 m_faceOffset;
	i32 m_numFaces;
	i32 m_numVertices;

	i32 m_vertexOffset;
	i32 m_uniqueEdgesOffset;
	i32 m_numUniqueEdges;
	i32 m_unused;
};

#endif  //D3_CONVEX_POLYHEDRON_DATA_H
