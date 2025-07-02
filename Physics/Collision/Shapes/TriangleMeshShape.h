#ifndef DRX3D_TRIANGLE_MESH_SHAPE_H
#define DRX3D_TRIANGLE_MESH_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/ConcaveShape.h>
#include <drx3D/Physics/Collision/Shapes/StridingMeshInterface.h>

///The TriangleMeshShape is an internal concave triangle mesh interface. Don't use this class directly, use BvhTriangleMeshShape instead.
ATTRIBUTE_ALIGNED16(class)
TriangleMeshShape : public ConcaveShape
{
protected:
	Vec3 m_localAabbMin;
	Vec3 m_localAabbMax;
	StridingMeshInterface* m_meshInterface;

	//TriangleMeshShape constructor has been disabled/protected, so that users will not mistakenly use this class.
	///Don't use TriangleMeshShape but use BvhTriangleMeshShape instead!
	TriangleMeshShape(StridingMeshInterface * meshInterface);

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	virtual ~TriangleMeshShape();

	virtual Vec3 localGetSupportingVertex(const Vec3& vec) const;

	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const
	{
		Assert(0);
		return localGetSupportingVertex(vec);
	}

	void recalcLocalAabb();

	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	virtual void processAllTriangles(TriangleCallback * callback, const Vec3& aabbMin, const Vec3& aabbMax) const;

	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const;

	virtual void setLocalScaling(const Vec3& scaling);
	virtual const Vec3& getLocalScaling() const;

	StridingMeshInterface* getMeshInterface()
	{
		return m_meshInterface;
	}

	const StridingMeshInterface* getMeshInterface() const
	{
		return m_meshInterface;
	}

	const Vec3& getLocalAabbMin() const
	{
		return m_localAabbMin;
	}
	const Vec3& getLocalAabbMax() const
	{
		return m_localAabbMax;
	}

	//debugging
	virtual tukk getName() const { return "TRIANGLEMESH"; }
};

#endif  //DRX3D_TRIANGLE_MESH_SHAPE_H
