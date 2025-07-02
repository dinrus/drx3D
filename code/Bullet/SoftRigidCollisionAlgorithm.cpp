
#include <drx3D/Physics/SoftBody/SoftRigidCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>
#include <drx3D/Physics/Collision/Shapes/BoxShape.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/SoftBody/SoftBody.h>
#include <drx3D/Physics/SoftBody/SoftBodySolvers.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

///TODO: include all the shapes that the softbody can collide with
///alternatively, implement special case collision algorithms (just like for rigid collision shapes)

//#include <stdio.h>

SoftRigidCollisionAlgorithm::SoftRigidCollisionAlgorithm(PersistentManifold* /*mf*/, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper*, const CollisionObject2Wrapper*, bool isSwapped)
	: CollisionAlgorithm(ci),
	  //m_ownManifold(false),
	  //m_manifoldPtr(mf),
	  m_isSwapped(isSwapped)
{
}

SoftRigidCollisionAlgorithm::~SoftRigidCollisionAlgorithm()
{
	//m_softBody->m_overlappingRigidBodies.remove(m_rigidCollisionObject2);

	/*if (m_ownManifold)
	{
	if (m_manifoldPtr)
	m_dispatcher->releaseManifold(m_manifoldPtr);
	}
	*/
}

#include <stdio.h>
#include <drx3D/Maths/Linear/Quickprof.h>
void SoftRigidCollisionAlgorithm::processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	DRX3D_PROFILE("SoftRigidCollisionAlgorithm::processCollision");
	(void)dispatchInfo;
	(void)resultOut;
	//printf("SoftRigidCollisionAlgorithm\n");
	//	const CollisionObject2Wrapper* softWrap = m_isSwapped?body1Wrap:body0Wrap;
	//	const CollisionObject2Wrapper* rigidWrap = m_isSwapped?body0Wrap:body1Wrap;
	SoftBody* softBody = m_isSwapped ? (SoftBody*)body1Wrap->getCollisionObject() : (SoftBody*)body0Wrap->getCollisionObject();
	const CollisionObject2Wrapper* rigidCollisionObject2Wrap = m_isSwapped ? body0Wrap : body1Wrap;

	if (softBody->m_collisionDisabledObjects.findLinearSearch(rigidCollisionObject2Wrap->getCollisionObject()) == softBody->m_collisionDisabledObjects.size())
	{
		softBody->getSoftBodySolver()->processCollision(softBody, rigidCollisionObject2Wrap);
	}
}

Scalar SoftRigidCollisionAlgorithm::calculateTimeOfImpact(CollisionObject2* col0, CollisionObject2* col1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	(void)resultOut;
	(void)dispatchInfo;
	(void)col0;
	(void)col1;

	//not yet
	return Scalar(1.);
}
