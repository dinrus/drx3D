#ifndef DRX3D_SPHERE_MINKOWSKI_H
#define DRX3D_SPHERE_MINKOWSKI_H

#include <drx3D/Physics/Collision/Shapes/ConvexInternalShape.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>  // for the types

///The SphereShape implements an implicit sphere, centered around a local origin with radius.
ATTRIBUTE_ALIGNED16(class)
SphereShape : public ConvexInternalShape

{
public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	SphereShape(Scalar radius) : ConvexInternalShape()
	{
		m_shapeType = SPHERE_SHAPE_PROXYTYPE;
		m_localScaling.setVal(1.0, 1.0, 1.0);
		m_implicitShapeDimensions.setZero();
		m_implicitShapeDimensions.setX(radius);
		m_collisionMargin = radius;
		m_padding = 0;
	}

	virtual Vec3 localGetSupportingVertex(const Vec3& vec) const;
	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const;
	//notice that the vectors should be unit length
	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const;

	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const;

	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	Scalar getRadius() const { return m_implicitShapeDimensions.getX() * m_localScaling.getX(); }

	void setUnscaledRadius(Scalar radius)
	{
		m_implicitShapeDimensions.setX(radius);
		ConvexInternalShape::setMargin(radius);
	}

	//debugging
	virtual tukk getName() const { return "SPHERE"; }

	virtual void setMargin(Scalar margin)
	{
		ConvexInternalShape::setMargin(margin);
	}
	virtual Scalar getMargin() const
	{
		//to improve gjk behaviour, use radius+margin as the full margin, so never get into the penetration case
		//this means, non-uniform scaling is not supported anymore
		return getRadius();
	}
};

#endif  //DRX3D_SPHERE_MINKOWSKI_H
