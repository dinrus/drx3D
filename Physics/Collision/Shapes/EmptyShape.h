#ifndef DRX3D_EMPTY_SHAPE_H
#define DRX3D_EMPTY_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/ConcaveShape.h>

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/Matrix3x3.h>
#include <drx3D/Physics/Collision/Shapes/CollisionMargin.h>

/// The EmptyShape is a collision shape without actual collision detection shape, so most users should ignore this class.
/// It can be replaced by another shape during runtime, but the inertia tensor should be recomputed.
ATTRIBUTE_ALIGNED16(class)
EmptyShape : public ConcaveShape
{
public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	EmptyShape();

	virtual ~EmptyShape();

	///getAabb's default implementation is brute force, expected derived classes to implement a fast dedicated version
	void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	virtual void setLocalScaling(const Vec3& scaling)
	{
		m_localScaling = scaling;
	}
	virtual const Vec3& getLocalScaling() const
	{
		return m_localScaling;
	}

	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const;

	virtual tukk getName() const
	{
		return "Empty";
	}

	virtual void processAllTriangles(TriangleCallback*, const Vec3&, const Vec3&) const
	{
	}

protected:
	Vec3 m_localScaling;
};

#endif  //DRX3D_EMPTY_SHAPE_H
