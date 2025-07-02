#ifndef DRX3D_CONCAVE_SHAPE_H
#define DRX3D_CONCAVE_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>  // for the types
#include <drx3D/Physics/Collision/Shapes/TriangleCallback.h>

/// PHY_ScalarType enumerates possible scalar types.
/// See the StridingMeshInterface or HeightfieldTerrainShape for its use
typedef enum PHY_ScalarType
{
	PHY_FLOAT,
	PHY_DOUBLE,
	PHY_INTEGER,
	PHY_SHORT,
	PHY_FIXEDPOINT88,
	PHY_UCHAR
} PHY_ScalarType;

///The ConcaveShape class provides an interface for non-moving (static) concave shapes.
///It has been implemented by the StaticPlaneShape, BvhTriangleMeshShape and HeightfieldTerrainShape.
ATTRIBUTE_ALIGNED16(class)
ConcaveShape : public CollisionShape
{
protected:
	Scalar m_collisionMargin;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	ConcaveShape();

	virtual ~ConcaveShape();

	virtual void processAllTriangles(TriangleCallback * callback, const Vec3& aabbMin, const Vec3& aabbMax) const = 0;

	virtual Scalar getMargin() const
	{
		return m_collisionMargin;
	}
	virtual void setMargin(Scalar collisionMargin)
	{
		m_collisionMargin = collisionMargin;
	}
};

#endif  //DRX3D_CONCAVE_SHAPE_H
