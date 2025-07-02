#ifndef DRX3D_CONVEX_2D_SHAPE_H
#define DRX3D_CONVEX_2D_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>  // for the types

///The Convex2dShape allows to use arbitrary convex shapes as 2d convex shapes, with the Z component assumed to be 0.
///For 2d boxes, the Box2dShape is recommended.
ATTRIBUTE_ALIGNED16(class)
Convex2dShape : public ConvexShape
{
	ConvexShape* m_childConvexShape;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	Convex2dShape(ConvexShape * convexChildShape);

	virtual ~Convex2dShape();

	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const;

	virtual Vec3 localGetSupportingVertex(const Vec3& vec) const;

	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const;

	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const;

	ConvexShape* getChildShape()
	{
		return m_childConvexShape;
	}

	const ConvexShape* getChildShape() const
	{
		return m_childConvexShape;
	}

	virtual tukk getName() const
	{
		return "Convex2dShape";
	}

	///////////////////////////

	///getAabb's default implementation is brute force, expected derived classes to implement a fast dedicated version
	void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	virtual void getAabbSlow(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	virtual void setLocalScaling(const Vec3& scaling);
	virtual const Vec3& getLocalScaling() const;

	virtual void setMargin(Scalar margin);
	virtual Scalar getMargin() const;

	virtual i32 getNumPreferredPenetrationDirections() const;

	virtual void getPreferredPenetrationDirection(i32 index, Vec3& penetrationVector) const;
};

#endif  //DRX3D_CONVEX_2D_SHAPE_H
