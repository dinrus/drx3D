#ifndef DRX3D_CONVEX_SHAPE_INTERFACE1
#define DRX3D_CONVEX_SHAPE_INTERFACE1

#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/Matrix3x3.h>
#include <drx3D/Physics/Collision/Shapes/CollisionMargin.h>
#include <drx3D/Maths/Linear/AlignedAllocator.h>

#define MAX_PREFERRED_PENETRATION_DIRECTIONS 10

/// The ConvexShape is an abstract shape interface, implemented by all convex shapes such as BoxShape, ConvexHullShape etc.
/// It describes general convex shapes using the localGetSupportingVertex interface, used by collision detectors such as GjkPairDetector.
ATTRIBUTE_ALIGNED16(class)
ConvexShape : public CollisionShape
{
public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	ConvexShape();

	virtual ~ConvexShape();

	virtual Vec3 localGetSupportingVertex(const Vec3& vec) const = 0;

////////
#ifndef __SPU__
	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const = 0;
#endif  //#ifndef __SPU__

	Vec3 localGetSupportVertexWithoutMarginNonVirtual(const Vec3& vec) const;
	Vec3 localGetSupportVertexNonVirtual(const Vec3& vec) const;
	Scalar getMarginNonVirtual() const;
	void getAabbNonVirtual(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	virtual void project(const Transform2& trans, const Vec3& dir, Scalar& minProj, Scalar& maxProj, Vec3& witnesPtMin, Vec3& witnesPtMax) const;

	//notice that the vectors should be unit length
	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const = 0;

	///getAabb's default implementation is brute force, expected derived classes to implement a fast dedicated version
	void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const = 0;

	virtual void getAabbSlow(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const = 0;

	virtual void setLocalScaling(const Vec3& scaling) = 0;
	virtual const Vec3& getLocalScaling() const = 0;

	virtual void setMargin(Scalar margin) = 0;

	virtual Scalar getMargin() const = 0;

	virtual i32 getNumPreferredPenetrationDirections() const = 0;

	virtual void getPreferredPenetrationDirection(i32 index, Vec3& penetrationVector) const = 0;
};

#endif  //DRX3D_CONVEX_SHAPE_INTERFACE1
