#include <drx3D/Physics/SoftBody/SoftSoftCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/Shapes/BoxShape.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/SoftBody/SoftBodySolvers.h>
#include <drx3D/Physics/SoftBody/SoftBody.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

#define USE_PERSISTENT_CONTACTS 1

SoftSoftCollisionAlgorithm::SoftSoftCollisionAlgorithm(PersistentManifold* /*mf*/, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* /*obj0*/, const CollisionObject2Wrapper* /*obj1*/)
	: CollisionAlgorithm(ci)
//m_ownManifold(false),
//m_manifoldPtr(mf)
{
}

SoftSoftCollisionAlgorithm::~SoftSoftCollisionAlgorithm()
{
}

void SoftSoftCollisionAlgorithm::processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& /*dispatchInfo*/, ManifoldResult* /*resultOut*/)
{
	SoftBody* soft0 = (SoftBody*)body0Wrap->getCollisionObject();
	SoftBody* soft1 = (SoftBody*)body1Wrap->getCollisionObject();
	soft0->getSoftBodySolver()->processCollision(soft0, soft1);
}

Scalar SoftSoftCollisionAlgorithm::calculateTimeOfImpact(CollisionObject2* /*body0*/, CollisionObject2* /*body1*/, const DispatcherInfo& /*dispatchInfo*/, ManifoldResult* /*resultOut*/)
{
	//not yet
	return 1.f;
}
