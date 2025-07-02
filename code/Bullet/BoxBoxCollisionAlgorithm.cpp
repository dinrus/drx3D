#include <drx3D/Physics/Collision/Dispatch/BoxBoxCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/Shapes/BoxShape.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Dispatch/BoxBoxDetector.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>
#define USE_PERSISTENT_CONTACTS 1

BoxBoxCollisionAlgorithm::BoxBoxCollisionAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
	: ActivatingCollisionAlgorithm(ci, body0Wrap, body1Wrap),
	  m_ownManifold(false),
	  m_manifoldPtr(mf)
{
	if (!m_manifoldPtr && m_dispatcher->needsCollision(body0Wrap->getCollisionObject(), body1Wrap->getCollisionObject()))
	{
		m_manifoldPtr = m_dispatcher->getNewManifold(body0Wrap->getCollisionObject(), body1Wrap->getCollisionObject());
		m_ownManifold = true;
	}
}

BoxBoxCollisionAlgorithm::~BoxBoxCollisionAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}

void BoxBoxCollisionAlgorithm::processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	if (!m_manifoldPtr)
		return;

	const BoxShape* box0 = (BoxShape*)body0Wrap->getCollisionShape();
	const BoxShape* box1 = (BoxShape*)body1Wrap->getCollisionShape();

	/// report a contact. internally this will be kept persistent, and contact reduction is done
	resultOut->setPersistentManifold(m_manifoldPtr);
#ifndef USE_PERSISTENT_CONTACTS
	m_manifoldPtr->clearManifold();
#endif  //USE_PERSISTENT_CONTACTS

	DiscreteCollisionDetectorInterface::ClosestPointInput input;
	input.m_maximumDistanceSquared = DRX3D_LARGE_FLOAT;
	input.m_transformA = body0Wrap->getWorldTransform();
	input.m_transformB = body1Wrap->getWorldTransform();

	BoxBoxDetector detector(box0, box1);
	detector.getClosestPoints(input, *resultOut, dispatchInfo.m_debugDraw);

#ifdef USE_PERSISTENT_CONTACTS
	//  refreshContactPoints is only necessary when using persistent contact points. otherwise all points are newly added
	if (m_ownManifold)
	{
		resultOut->refreshContactPoints();
	}
#endif  //USE_PERSISTENT_CONTACTS
}

Scalar BoxBoxCollisionAlgorithm::calculateTimeOfImpact(CollisionObject2* /*body0*/, CollisionObject2* /*body1*/, const DispatcherInfo& /*dispatchInfo*/, ManifoldResult* /*resultOut*/)
{
	//not yet
	return 1.f;
}
