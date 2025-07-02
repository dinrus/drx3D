
#include <drx3D/Physics/Collision/Shapes/TriangleIndexVertexArray.h>

TriangleIndexVertexArray::TriangleIndexVertexArray(i32 numTriangles, i32* triangleIndexBase, i32 triangleIndexStride, i32 numVertices, Scalar* vertexBase, i32 vertexStride)
	: m_hasAabb(0)
{
	IndexedMesh mesh;

	mesh.m_numTriangles = numTriangles;
	mesh.m_triangleIndexBase = (u8k*)triangleIndexBase;
	mesh.m_triangleIndexStride = triangleIndexStride;
	mesh.m_numVertices = numVertices;
	mesh.m_vertexBase = (u8k*)vertexBase;
	mesh.m_vertexStride = vertexStride;

	addIndexedMesh(mesh);
}

TriangleIndexVertexArray::~TriangleIndexVertexArray()
{
}

void TriangleIndexVertexArray::getLockedVertexIndexBase(u8** vertexbase, i32& numverts, PHY_ScalarType& type, i32& vertexStride, u8** indexbase, i32& indexstride, i32& numfaces, PHY_ScalarType& indicestype, i32 subpart)
{
	Assert(subpart < getNumSubParts());

	IndexedMesh& mesh = m_indexedMeshes[subpart];

	numverts = mesh.m_numVertices;
	(*vertexbase) = (u8*)mesh.m_vertexBase;

	type = mesh.m_vertexType;

	vertexStride = mesh.m_vertexStride;

	numfaces = mesh.m_numTriangles;

	(*indexbase) = (u8*)mesh.m_triangleIndexBase;
	indexstride = mesh.m_triangleIndexStride;
	indicestype = mesh.m_indexType;
}

void TriangleIndexVertexArray::getLockedReadOnlyVertexIndexBase(u8k** vertexbase, i32& numverts, PHY_ScalarType& type, i32& vertexStride, u8k** indexbase, i32& indexstride, i32& numfaces, PHY_ScalarType& indicestype, i32 subpart) const
{
	const IndexedMesh& mesh = m_indexedMeshes[subpart];

	numverts = mesh.m_numVertices;
	(*vertexbase) = (u8k*)mesh.m_vertexBase;

	type = mesh.m_vertexType;

	vertexStride = mesh.m_vertexStride;

	numfaces = mesh.m_numTriangles;
	(*indexbase) = (u8k*)mesh.m_triangleIndexBase;
	indexstride = mesh.m_triangleIndexStride;
	indicestype = mesh.m_indexType;
}

bool TriangleIndexVertexArray::hasPremadeAabb() const
{
	return (m_hasAabb == 1);
}

void TriangleIndexVertexArray::setPremadeAabb(const Vec3& aabbMin, const Vec3& aabbMax) const
{
	m_aabbMin = aabbMin;
	m_aabbMax = aabbMax;
	m_hasAabb = 1;  // this is intentionally an i32 see notes in header
}

void TriangleIndexVertexArray::getPremadeAabb(Vec3* aabbMin, Vec3* aabbMax) const
{
	*aabbMin = m_aabbMin;
	*aabbMax = m_aabbMax;
}
