#include <drx3D/Physics/Collision/NarrowPhase/b3TriangleIndexVertexArray.h>

b3TriangleIndexVertexArray::b3TriangleIndexVertexArray(i32 numTriangles, i32* triangleIndexBase, i32 triangleIndexStride, i32 numVertices, b3Scalar* vertexBase, i32 vertexStride)
	: m_hasAabb(0)
{
	b3IndexedMesh mesh;

	mesh.m_numTriangles = numTriangles;
	mesh.m_triangleIndexBase = (u8k*)triangleIndexBase;
	mesh.m_triangleIndexStride = triangleIndexStride;
	mesh.m_numVertices = numVertices;
	mesh.m_vertexBase = (u8k*)vertexBase;
	mesh.m_vertexStride = vertexStride;

	addIndexedMesh(mesh);
}

b3TriangleIndexVertexArray::~b3TriangleIndexVertexArray()
{
}

void b3TriangleIndexVertexArray::getLockedVertexIndexBase(u8** vertexbase, i32& numverts, PHY_ScalarType& type, i32& vertexStride, u8** indexbase, i32& indexstride, i32& numfaces, PHY_ScalarType& indicestype, i32 subpart)
{
	drx3DAssert(subpart < getNumSubParts());

	b3IndexedMesh& mesh = m_indexedMeshes[subpart];

	numverts = mesh.m_numVertices;
	(*vertexbase) = (u8*)mesh.m_vertexBase;

	type = mesh.m_vertexType;

	vertexStride = mesh.m_vertexStride;

	numfaces = mesh.m_numTriangles;

	(*indexbase) = (u8*)mesh.m_triangleIndexBase;
	indexstride = mesh.m_triangleIndexStride;
	indicestype = mesh.m_indexType;
}

void b3TriangleIndexVertexArray::getLockedReadOnlyVertexIndexBase(u8k** vertexbase, i32& numverts, PHY_ScalarType& type, i32& vertexStride, u8k** indexbase, i32& indexstride, i32& numfaces, PHY_ScalarType& indicestype, i32 subpart) const
{
	const b3IndexedMesh& mesh = m_indexedMeshes[subpart];

	numverts = mesh.m_numVertices;
	(*vertexbase) = (u8k*)mesh.m_vertexBase;

	type = mesh.m_vertexType;

	vertexStride = mesh.m_vertexStride;

	numfaces = mesh.m_numTriangles;
	(*indexbase) = (u8k*)mesh.m_triangleIndexBase;
	indexstride = mesh.m_triangleIndexStride;
	indicestype = mesh.m_indexType;
}

bool b3TriangleIndexVertexArray::hasPremadeAabb() const
{
	return (m_hasAabb == 1);
}

void b3TriangleIndexVertexArray::setPremadeAabb(const b3Vec3& aabbMin, const b3Vec3& aabbMax) const
{
	m_aabbMin = aabbMin;
	m_aabbMax = aabbMax;
	m_hasAabb = 1;  // this is intentionally an i32 see notes in header
}

void b3TriangleIndexVertexArray::getPremadeAabb(b3Vec3* aabbMin, b3Vec3* aabbMax) const
{
	*aabbMin = m_aabbMin;
	*aabbMax = m_aabbMax;
}
