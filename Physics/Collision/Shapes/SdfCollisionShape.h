#ifndef DRX3D_SDF_COLLISION_SHAPE_H
#define DRX3D_SDF_COLLISION_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/ConcaveShape.h>

class SdfCollisionShape : public ConcaveShape
{
	struct SdfCollisionShapeInternalData* m_data;

public:
	SdfCollisionShape();
	virtual ~SdfCollisionShape();

	bool initializeSDF(tukk sdfData, i32 sizeInBytes);

	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;
	virtual void setLocalScaling(const Vec3& scaling);
	virtual const Vec3& getLocalScaling() const;
	virtual void calculateLocalInertia(Scalar mass, Vec3& inertia) const;
	virtual tukk getName() const;
	virtual void setMargin(Scalar margin);
	virtual Scalar getMargin() const;

	virtual void processAllTriangles(TriangleCallback* callback, const Vec3& aabbMin, const Vec3& aabbMax) const;

	bool queryPoint(const Vec3& ptInSDF, Scalar& distOut, Vec3& normal);
};

#endif  //DRX3D_SDF_COLLISION_SHAPE_H
