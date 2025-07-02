#ifndef DRX3D_CONVEX_TRIANGLEMESH_SHAPE_H
#define DRX3D_CONVEX_TRIANGLEMESH_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/PolyhedralConvexShape.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>  // for the types

/// The ConvexTriangleMeshShape is a convex hull of a triangle mesh, but the performance is not as good as ConvexHullShape.
/// A small benefit of this class is that it uses the StridingMeshInterface, so you can avoid the duplication of the triangle mesh data. Nevertheless, most users should use the much better performing ConvexHullShape instead.
ATTRIBUTE_ALIGNED16(class)
ConvexTriangleMeshShape : public PolyhedralConvexAabbCachingShape
{
	class StridingMeshInterface* m_stridingMesh;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	ConvexTriangleMeshShape(StridingMeshInterface * meshInterface, bool calcAabb = true);

	class StridingMeshInterface* getMeshInterface()
	{
		return m_stridingMesh;
	}
	const class StridingMeshInterface* getMeshInterface() const
	{
		return m_stridingMesh;
	}

	virtual Vec3 localGetSupportingVertex(const Vec3& vec) const;
	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const;
	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const;

	//debugging
	virtual tukk getName() const { return "ConvexTrimesh"; }

	virtual i32 getNumVertices() const;
	virtual i32 getNumEdges() const;
	virtual void getEdge(i32 i, Vec3& pa, Vec3& pb) const;
	virtual void getVertex(i32 i, Vec3& vtx) const;
	virtual i32 getNumPlanes() const;
	virtual void getPlane(Vec3 & planeNormal, Vec3 & planeSupport, i32 i) const;
	virtual bool isInside(const Vec3& pt, Scalar tolerance) const;

	virtual void setLocalScaling(const Vec3& scaling);
	virtual const Vec3& getLocalScaling() const;

	///computes the exact moment of inertia and the transform from the coordinate system defined by the principal axes of the moment of inertia
	///and the center of mass to the current coordinate system. A mass of 1 is assumed, for other masses just multiply the computed "inertia"
	///by the mass. The resulting transform "principal" has to be applied inversely to the mesh in order for the local coordinate system of the
	///shape to be centered at the center of mass and to coincide with the principal axes. This also necessitates a correction of the world transform
	///of the collision object by the principal transform. This method also computes the volume of the convex mesh.
	void calculatePrincipalAxisTransform(Transform2 & principal, Vec3 & inertia, Scalar & volume) const;
};

#endif  //DRX3D_CONVEX_TRIANGLEMESH_SHAPE_H
