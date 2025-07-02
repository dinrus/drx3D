#ifndef DRX3D_TRIANGLE_MESH_H
#define DRX3D_TRIANGLE_MESH_H

#include <drx3D/Physics/Collision/Shapes/TriangleIndexVertexArray.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

///The TriangleMesh class is a convenience class derived from TriangleIndexVertexArray, that provides storage for a concave triangle mesh. It can be used as data for the BvhTriangleMeshShape.
///It allows either 32bit or 16bit indices, and 4 (x-y-z-w) or 3 (x-y-z) component vertices.
///If you want to share triangle/index data between graphics mesh and collision mesh (BvhTriangleMeshShape), you can directly use TriangleIndexVertexArray or derive your own class from StridingMeshInterface.
///Performance of TriangleMesh and TriangleIndexVertexArray used in a BvhTriangleMeshShape is the same.
class TriangleMesh : public TriangleIndexVertexArray
{
	AlignedObjectArray<Vec3> m_4componentVertices;
	AlignedObjectArray<Scalar> m_3componentVertices;

	AlignedObjectArray<u32> m_32bitIndices;
	AlignedObjectArray<u16> m_16bitIndices;
	bool m_use32bitIndices;
	bool m_use4componentVertices;

public:
	Scalar m_weldingThreshold;

	TriangleMesh(bool use32bitIndices = true, bool use4componentVertices = true);

	bool getUse32bitIndices() const
	{
		return m_use32bitIndices;
	}

	bool getUse4componentVertices() const
	{
		return m_use4componentVertices;
	}
	///By default addTriangle won't search for duplicate vertices, because the search is very slow for large triangle meshes.
	///In general it is better to directly use TriangleIndexVertexArray instead.
	void addTriangle(const Vec3& vertex0, const Vec3& vertex1, const Vec3& vertex2, bool removeDuplicateVertices = false);

	///Add a triangle using its indices. Make sure the indices are pointing within the vertices array, so add the vertices first (and to be sure, avoid removal of duplicate vertices)
	void addTriangleIndices(i32 index1, i32 index2, i32 index3);

	i32 getNumTriangles() const;

	virtual void preallocateVertices(i32 numverts);
	virtual void preallocateIndices(i32 numindices);

	///findOrAddVertex is an internal method, use addTriangle instead
	i32 findOrAddVertex(const Vec3& vertex, bool removeDuplicateVertices);
	///addIndex is an internal method, use addTriangle instead
	void addIndex(i32 index);
};

#endif  //DRX3D_TRIANGLE_MESH_H
