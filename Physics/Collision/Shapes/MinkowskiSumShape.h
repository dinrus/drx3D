#ifndef DRX3D_MINKOWSKI_SUM_SHAPE_H
#define DRX3D_MINKOWSKI_SUM_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/ConvexInternalShape.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>  // for the types

/// The MinkowskiSumShape is only for advanced users. This shape represents implicit based minkowski sum of two convex implicit shapes.
ATTRIBUTE_ALIGNED16(class)
MinkowskiSumShape : public ConvexInternalShape
{
	Transform2 m_transA;
	Transform2 m_transB;
	const ConvexShape* m_shapeA;
	const ConvexShape* m_shapeB;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	MinkowskiSumShape(const ConvexShape* shapeA, const ConvexShape* shapeB);

	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const;

	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const;

	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const;

	void setTransform2A(const Transform2& transA) { m_transA = transA; }
	void setTransform2B(const Transform2& transB) { m_transB = transB; }

	const Transform2& getTransform2A() const { return m_transA; }
	const Transform2& getTransform2B() const { return m_transB; }

	// keep this for backward compatibility
	const Transform2& GetTransform2B() const { return m_transB; }

	virtual Scalar getMargin() const;

	const ConvexShape* getShapeA() const { return m_shapeA; }
	const ConvexShape* getShapeB() const { return m_shapeB; }

	virtual tukk getName() const
	{
		return "MinkowskiSum";
	}
};

#endif  //DRX3D_MINKOWSKI_SUM_SHAPE_H
