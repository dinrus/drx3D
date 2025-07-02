#ifndef D3_TRIANGLE_INDEX_VERTEX_ARRAY_H
#define D3_TRIANGLE_INDEX_VERTEX_ARRAY_H

#include <drx3D/Physics/Collision/NarrowPhase/b3StridingMeshInterface.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/b3Scalar.h>

///The b3IndexedMesh indexes a single vertex and index array. Multiple b3IndexedMesh objects can be passed into a b3TriangleIndexVertexArray using addIndexedMesh.
///Instead of the number of indices, we pass the number of triangles.
D3_ATTRIBUTE_ALIGNED16(struct)
b3IndexedMesh
{
	D3_DECLARE_ALIGNED_ALLOCATOR();

	i32 m_numTriangles;
	u8k* m_triangleIndexBase;
	// Size in byte of the indices for one triangle (3*sizeof(index_type) if the indices are tightly packed)
	i32 m_triangleIndexStride;
	i32 m_numVertices;
	u8k* m_vertexBase;
	// Size of a vertex, in bytes
	i32 m_vertexStride;

	// The index type is set when adding an indexed mesh to the
	// b3TriangleIndexVertexArray, do not set it manually
	PHY_ScalarType m_indexType;

	// The vertex type has a default type similar to drx3D's precision mode (float or double)
	// but can be set manually if you for example run drx3D with double precision but have
	// mesh data in single precision..
	PHY_ScalarType m_vertexType;

	b3IndexedMesh()
		: m_indexType(PHY_INTEGER),
#ifdef D3_USE_DOUBLE_PRECISION
		  m_vertexType(PHY_DOUBLE)
#else   // D3_USE_DOUBLE_PRECISION
		  m_vertexType(PHY_FLOAT)
#endif  // D3_USE_DOUBLE_PRECISION
	{
	}
};

typedef b3AlignedObjectArray<b3IndexedMesh> IndexedMeshArray;

///The b3TriangleIndexVertexArray allows to access multiple triangle meshes, by indexing into existing triangle/index arrays.
///Additional meshes can be added using addIndexedMesh
///No duplcate is made of the vertex/index data, it only indexes into external vertex/index arrays.
///So keep those arrays around during the lifetime of this b3TriangleIndexVertexArray.
D3_ATTRIBUTE_ALIGNED16(class)
b3TriangleIndexVertexArray : public b3StridingMeshInterface
{
protected:
	IndexedMeshArray m_indexedMeshes;
	i32 m_pad[2];
	mutable i32 m_hasAabb;  // using i32 instead of bool to maintain alignment
	mutable b3Vec3 m_aabbMin;
	mutable b3Vec3 m_aabbMax;

public:
	D3_DECLARE_ALIGNED_ALLOCATOR();

	b3TriangleIndexVertexArray() : m_hasAabb(0)
	{
	}

	virtual ~b3TriangleIndexVertexArray();

	//just to be backwards compatible
	b3TriangleIndexVertexArray(i32 numTriangles, i32* triangleIndexBase, i32 triangleIndexStride, i32 numVertices, b3Scalar* vertexBase, i32 vertexStride);

	void addIndexedMesh(const b3IndexedMesh& mesh, PHY_ScalarType indexType = PHY_INTEGER)
	{
		m_indexedMeshes.push_back(mesh);
		m_indexedMeshes[m_indexedMeshes.size() - 1].m_indexType = indexType;
	}

	virtual void getLockedVertexIndexBase(u8** vertexbase, i32& numverts, PHY_ScalarType& type, i32& vertexStride, u8** indexbase, i32& indexstride, i32& numfaces, PHY_ScalarType& indicestype, i32 subpart = 0);

	virtual void getLockedReadOnlyVertexIndexBase(u8k** vertexbase, i32& numverts, PHY_ScalarType& type, i32& vertexStride, u8k** indexbase, i32& indexstride, i32& numfaces, PHY_ScalarType& indicestype, i32 subpart = 0) const;

	/// unLockVertexBase finishes the access to a subpart of the triangle mesh
	/// make a call to unLockVertexBase when the read and write access (using getLockedVertexIndexBase) is finished
	virtual void unLockVertexBase(i32 subpart) { (void)subpart; }

	virtual void unLockReadOnlyVertexBase(i32 subpart) const { (void)subpart; }

	/// getNumSubParts returns the number of separate subparts
	/// each subpart has a continuous array of vertices and indices
	virtual i32 getNumSubParts() const
	{
		return (i32)m_indexedMeshes.size();
	}

	IndexedMeshArray& getIndexedMeshArray()
	{
		return m_indexedMeshes;
	}

	const IndexedMeshArray& getIndexedMeshArray() const
	{
		return m_indexedMeshes;
	}

	virtual void preallocateVertices(i32 numverts) { (void)numverts; }
	virtual void preallocateIndices(i32 numindices) { (void)numindices; }

	virtual bool hasPremadeAabb() const;
	virtual void setPremadeAabb(const b3Vec3& aabbMin, const b3Vec3& aabbMax) const;
	virtual void getPremadeAabb(b3Vec3 * aabbMin, b3Vec3 * aabbMax) const;
};

#endif  //D3_TRIANGLE_INDEX_VERTEX_ARRAY_H
