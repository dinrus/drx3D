#ifndef DRX3D_SPHERE_TRIANGLE_DETECTOR_H
#define DRX3D_SPHERE_TRIANGLE_DETECTOR_H

#include <drx3D/Physics/Collision/NarrowPhase/DiscreteCollisionDetectorInterface.h>

class SphereShape;
class TriangleShape;

/// sphere-triangle to match the DiscreteCollisionDetectorInterface
struct SphereTriangleDetector : public DiscreteCollisionDetectorInterface
{
	virtual void getClosestPoints(const ClosestPointInput& input, Result& output, class IDebugDraw* debugDraw, bool swapResults = false);

	SphereTriangleDetector(SphereShape* sphere, TriangleShape* triangle, Scalar contactBreakingThreshold);

	virtual ~SphereTriangleDetector(){};

	bool collide(const Vec3& sphereCenter, Vec3& point, Vec3& resultNormal, Scalar& depth, Scalar& timeOfImpact, Scalar contactBreakingThreshold);

private:
	bool pointInTriangle(const Vec3 vertices[], const Vec3& normal, Vec3* p);
	bool facecontains(const Vec3& p, const Vec3* vertices, Vec3& normal);

	SphereShape* m_sphere;
	TriangleShape* m_triangle;
	Scalar m_contactBreakingThreshold;
};
#endif  //DRX3D_SPHERE_TRIANGLE_DETECTOR_H
