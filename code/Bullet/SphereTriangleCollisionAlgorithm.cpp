
#include <drx3D/Physics/Collision/Dispatch/SphereTriangleCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Dispatch/SphereTriangleDetector.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

SphereTriangleCollisionAlgorithm::SphereTriangleCollisionAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool swapped)
	: ActivatingCollisionAlgorithm(ci, body0Wrap, body1Wrap),
	  m_ownManifold(false),
	  m_manifoldPtr(mf),
	  m_swapped(swapped)
{
	if (!m_manifoldPtr)
	{
		m_manifoldPtr = m_dispatcher->getNewManifold(body0Wrap->getCollisionObject(), body1Wrap->getCollisionObject());
		m_ownManifold = true;
	}
}

SphereTriangleCollisionAlgorithm::~SphereTriangleCollisionAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}

void SphereTriangleCollisionAlgorithm::processCollision(const CollisionObject2Wrapper* col0Wrap, const CollisionObject2Wrapper* col1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	if (!m_manifoldPtr)
		return;

	const CollisionObject2Wrapper* sphereObjWrap = m_swapped ? col1Wrap : col0Wrap;
	const CollisionObject2Wrapper* triObjWrap = m_swapped ? col0Wrap : col1Wrap;

	SphereShape* sphere = (SphereShape*)sphereObjWrap->getCollisionShape();
	TriangleShape* triangle = (TriangleShape*)triObjWrap->getCollisionShape();

	/// report a contact. internally this will be kept persistent, and contact reduction is done
	resultOut->setPersistentManifold(m_manifoldPtr);
	SphereTriangleDetector detector(sphere, triangle, m_manifoldPtr->getContactBreakingThreshold() + resultOut->m_closestPointDistanceThreshold);

	DiscreteCollisionDetectorInterface::ClosestPointInput input;
	input.m_maximumDistanceSquared = Scalar(DRX3D_LARGE_FLOAT);  ///@todo: tighter bounds
	input.m_transformA = sphereObjWrap->getWorldTransform();
	input.m_transformB = triObjWrap->getWorldTransform();

	bool swapResults = m_swapped;

	detector.getClosestPoints(input, *resultOut, dispatchInfo.m_debugDraw, swapResults);

	if (m_ownManifold)
		resultOut->refreshContactPoints();
}

Scalar SphereTriangleCollisionAlgorithm::calculateTimeOfImpact(CollisionObject2* col0, CollisionObject2* col1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	(void)resultOut;
	(void)dispatchInfo;
	(void)col0;
	(void)col1;

	//not yet
	return Scalar(1.);
}
