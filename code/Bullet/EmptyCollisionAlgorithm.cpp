#include <drx3D/Physics/Collision/Dispatch/EmptyCollisionAlgorithm.h>

EmptyAlgorithm::EmptyAlgorithm(const CollisionAlgorithmConstructionInfo& ci)
	: CollisionAlgorithm(ci)
{
}

void EmptyAlgorithm::processCollision(const CollisionObject2Wrapper*, const CollisionObject2Wrapper*, const DispatcherInfo&, ManifoldResult*)
{
}

Scalar EmptyAlgorithm::calculateTimeOfImpact(CollisionObject2*, CollisionObject2*, const DispatcherInfo&, ManifoldResult*)
{
	return Scalar(1.);
}
