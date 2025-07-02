#define CLEAR_MANIFOLD 1

#include <drx3D/Physics/Collision/Dispatch/SphereSphereCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

SphereSphereCollisionAlgorithm::SphereSphereCollisionAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* col0Wrap, const CollisionObject2Wrapper* col1Wrap)
	: ActivatingCollisionAlgorithm(ci, col0Wrap, col1Wrap),
	  m_ownManifold(false),
	  m_manifoldPtr(mf)
{
	if (!m_manifoldPtr)
	{
		m_manifoldPtr = m_dispatcher->getNewManifold(col0Wrap->getCollisionObject(), col1Wrap->getCollisionObject());
		m_ownManifold = true;
	}
}

SphereSphereCollisionAlgorithm::~SphereSphereCollisionAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}

void SphereSphereCollisionAlgorithm::processCollision(const CollisionObject2Wrapper* col0Wrap, const CollisionObject2Wrapper* col1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	(void)dispatchInfo;

	if (!m_manifoldPtr)
		return;

	resultOut->setPersistentManifold(m_manifoldPtr);

	SphereShape* sphere0 = (SphereShape*)col0Wrap->getCollisionShape();
	SphereShape* sphere1 = (SphereShape*)col1Wrap->getCollisionShape();

	Vec3 diff = col0Wrap->getWorldTransform().getOrigin() - col1Wrap->getWorldTransform().getOrigin();
	Scalar len = diff.length();
	Scalar radius0 = sphere0->getRadius();
	Scalar radius1 = sphere1->getRadius();

#ifdef CLEAR_MANIFOLD
	m_manifoldPtr->clearManifold();  //don't do this, it disables warmstarting
#endif

	///iff distance positive, don't generate a new contact
	if (len > (radius0 + radius1 + resultOut->m_closestPointDistanceThreshold))
	{
#ifndef CLEAR_MANIFOLD
		resultOut->refreshContactPoints();
#endif  //CLEAR_MANIFOLD
		return;
	}
	///distance (negative means penetration)
	Scalar dist = len - (radius0 + radius1);

	Vec3 normalOnSurfaceB(1, 0, 0);
	if (len > SIMD_EPSILON)
	{
		normalOnSurfaceB = diff / len;
	}

	///point on A (worldspace)
	///Vec3 pos0 = col0->getWorldTransform().getOrigin() - radius0 * normalOnSurfaceB;
	///point on B (worldspace)
	Vec3 pos1 = col1Wrap->getWorldTransform().getOrigin() + radius1 * normalOnSurfaceB;

	/// report a contact. internally this will be kept persistent, and contact reduction is done

	resultOut->addContactPoint(normalOnSurfaceB, pos1, dist);

#ifndef CLEAR_MANIFOLD
	resultOut->refreshContactPoints();
#endif  //CLEAR_MANIFOLD
}

Scalar SphereSphereCollisionAlgorithm::calculateTimeOfImpact(CollisionObject2* col0, CollisionObject2* col1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	(void)col0;
	(void)col1;
	(void)dispatchInfo;
	(void)resultOut;

	//not yet
	return Scalar(1.);
}
