#include <drx3D/Physics/Collision/Shapes/TriangleMesh.h>

TriangleMesh::TriangleMesh(bool use32bitIndices, bool use4componentVertices)
	: m_use32bitIndices(use32bitIndices),
	  m_use4componentVertices(use4componentVertices),
	  m_weldingThreshold(0.0)
{
	IndexedMesh meshIndex;
	meshIndex.m_numTriangles = 0;
	meshIndex.m_numVertices = 0;
	meshIndex.m_indexType = PHY_INTEGER;
	meshIndex.m_triangleIndexBase = 0;
	meshIndex.m_triangleIndexStride = 3 * sizeof(i32);
	meshIndex.m_vertexBase = 0;
	meshIndex.m_vertexStride = sizeof(Vec3);
	m_indexedMeshes.push_back(meshIndex);

	if (m_use32bitIndices)
	{
		m_indexedMeshes[0].m_numTriangles = m_32bitIndices.size() / 3;
		m_indexedMeshes[0].m_triangleIndexBase = 0;
		m_indexedMeshes[0].m_indexType = PHY_INTEGER;
		m_indexedMeshes[0].m_triangleIndexStride = 3 * sizeof(i32);
	}
	else
	{
		m_indexedMeshes[0].m_numTriangles = m_16bitIndices.size() / 3;
		m_indexedMeshes[0].m_triangleIndexBase = 0;
		m_indexedMeshes[0].m_indexType = PHY_SHORT;
		m_indexedMeshes[0].m_triangleIndexStride = 3 * sizeof(i16);
	}

	if (m_use4componentVertices)
	{
		m_indexedMeshes[0].m_numVertices = m_4componentVertices.size();
		m_indexedMeshes[0].m_vertexBase = 0;
		m_indexedMeshes[0].m_vertexStride = sizeof(Vec3);
	}
	else
	{
		m_indexedMeshes[0].m_numVertices = m_3componentVertices.size() / 3;
		m_indexedMeshes[0].m_vertexBase = 0;
		m_indexedMeshes[0].m_vertexStride = 3 * sizeof(Scalar);
	}
}

void TriangleMesh::addIndex(i32 index)
{
	if (m_use32bitIndices)
	{
		m_32bitIndices.push_back(index);
		m_indexedMeshes[0].m_triangleIndexBase = (u8*)&m_32bitIndices[0];
	}
	else
	{
		m_16bitIndices.push_back(index);
		m_indexedMeshes[0].m_triangleIndexBase = (u8*)&m_16bitIndices[0];
	}
}

void TriangleMesh::addTriangleIndices(i32 index1, i32 index2, i32 index3)
{
	m_indexedMeshes[0].m_numTriangles++;
	addIndex(index1);
	addIndex(index2);
	addIndex(index3);
}

i32 TriangleMesh::findOrAddVertex(const Vec3& vertex, bool removeDuplicateVertices)
{
	//return index of new/existing vertex
	///@todo: could use acceleration structure for this
	if (m_use4componentVertices)
	{
		if (removeDuplicateVertices)
		{
			for (i32 i = 0; i < m_4componentVertices.size(); i++)
			{
				if ((m_4componentVertices[i] - vertex).length2() <= m_weldingThreshold)
				{
					return i;
				}
			}
		}
		m_indexedMeshes[0].m_numVertices++;
		m_4componentVertices.push_back(vertex);
		m_indexedMeshes[0].m_vertexBase = (u8*)&m_4componentVertices[0];

		return m_4componentVertices.size() - 1;
	}
	else
	{
		if (removeDuplicateVertices)
		{
			for (i32 i = 0; i < m_3componentVertices.size(); i += 3)
			{
				Vec3 vtx(m_3componentVertices[i], m_3componentVertices[i + 1], m_3componentVertices[i + 2]);
				if ((vtx - vertex).length2() <= m_weldingThreshold)
				{
					return i / 3;
				}
			}
		}
		m_3componentVertices.push_back(vertex.getX());
		m_3componentVertices.push_back(vertex.getY());
		m_3componentVertices.push_back(vertex.getZ());
		m_indexedMeshes[0].m_numVertices++;
		m_indexedMeshes[0].m_vertexBase = (u8*)&m_3componentVertices[0];
		return (m_3componentVertices.size() / 3) - 1;
	}
}

void TriangleMesh::addTriangle(const Vec3& vertex0, const Vec3& vertex1, const Vec3& vertex2, bool removeDuplicateVertices)
{
	m_indexedMeshes[0].m_numTriangles++;
	addIndex(findOrAddVertex(vertex0, removeDuplicateVertices));
	addIndex(findOrAddVertex(vertex1, removeDuplicateVertices));
	addIndex(findOrAddVertex(vertex2, removeDuplicateVertices));
}

i32 TriangleMesh::getNumTriangles() const
{
	if (m_use32bitIndices)
	{
		return m_32bitIndices.size() / 3;
	}
	return m_16bitIndices.size() / 3;
}

void TriangleMesh::preallocateVertices(i32 numverts)
{
	if (m_use4componentVertices)
	{
		m_4componentVertices.reserve(numverts);
	}
	else
	{
		m_3componentVertices.reserve(numverts);
	}
}

void TriangleMesh::preallocateIndices(i32 numindices)
{
	if (m_use32bitIndices)
	{
		m_32bitIndices.reserve(numindices);
	}
	else
	{
		m_16bitIndices.reserve(numindices);
	}
}
