#include<drx3D/Physics/Collision/Dispatch/ConvexPlaneCollisionAlgorithm.h>

#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Physics/Collision/Shapes/StaticPlaneShape.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

//#include <stdio.h>

ConvexPlaneCollisionAlgorithm::ConvexPlaneCollisionAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* col0Wrap, const CollisionObject2Wrapper* col1Wrap, bool isSwapped, i32 numPerturbationIterations, i32 minimumPointsPerturbationThreshold)
	: CollisionAlgorithm(ci),
	  m_ownManifold(false),
	  m_manifoldPtr(mf),
	  m_isSwapped(isSwapped),
	  m_numPerturbationIterations(numPerturbationIterations),
	  m_minimumPointsPerturbationThreshold(minimumPointsPerturbationThreshold)
{
	const CollisionObject2Wrapper* convexObjWrap = m_isSwapped ? col1Wrap : col0Wrap;
	const CollisionObject2Wrapper* planeObjWrap = m_isSwapped ? col0Wrap : col1Wrap;

	if (!m_manifoldPtr && m_dispatcher->needsCollision(convexObjWrap->getCollisionObject(), planeObjWrap->getCollisionObject()))
	{
		m_manifoldPtr = m_dispatcher->getNewManifold(convexObjWrap->getCollisionObject(), planeObjWrap->getCollisionObject());
		m_ownManifold = true;
	}
}

ConvexPlaneCollisionAlgorithm::~ConvexPlaneCollisionAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}

void ConvexPlaneCollisionAlgorithm::collideSingleContact(const Quat& perturbeRot, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	const CollisionObject2Wrapper* convexObjWrap = m_isSwapped ? body1Wrap : body0Wrap;
	const CollisionObject2Wrapper* planeObjWrap = m_isSwapped ? body0Wrap : body1Wrap;

	ConvexShape* convexShape = (ConvexShape*)convexObjWrap->getCollisionShape();
	StaticPlaneShape* planeShape = (StaticPlaneShape*)planeObjWrap->getCollisionShape();

	bool hasCollision = false;
	const Vec3& planeNormal = planeShape->getPlaneNormal();
	const Scalar& planeConstant = planeShape->getPlaneConstant();

	Transform2 convexWorldTransform = convexObjWrap->getWorldTransform();
	Transform2 convexInPlaneTrans;
	convexInPlaneTrans = planeObjWrap->getWorldTransform().inverse() * convexWorldTransform;
	//now perturbe the convex-world transform
	convexWorldTransform.getBasis() *= Matrix3x3(perturbeRot);
	Transform2 planeInConvex;
	planeInConvex = convexWorldTransform.inverse() * planeObjWrap->getWorldTransform();

	Vec3 vtx = convexShape->localGetSupportingVertex(planeInConvex.getBasis() * -planeNormal);

	Vec3 vtxInPlane = convexInPlaneTrans(vtx);
	Scalar distance = (planeNormal.dot(vtxInPlane) - planeConstant);

	Vec3 vtxInPlaneProjected = vtxInPlane - distance * planeNormal;
	Vec3 vtxInPlaneWorld = planeObjWrap->getWorldTransform() * vtxInPlaneProjected;

	hasCollision = distance < m_manifoldPtr->getContactBreakingThreshold();
	resultOut->setPersistentManifold(m_manifoldPtr);
	if (hasCollision)
	{
		/// report a contact. internally this will be kept persistent, and contact reduction is done
		Vec3 normalOnSurfaceB = planeObjWrap->getWorldTransform().getBasis() * planeNormal;
		Vec3 pOnB = vtxInPlaneWorld;
		resultOut->addContactPoint(normalOnSurfaceB, pOnB, distance);
	}
}

void ConvexPlaneCollisionAlgorithm::processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	(void)dispatchInfo;
	if (!m_manifoldPtr)
		return;

	const CollisionObject2Wrapper* convexObjWrap = m_isSwapped ? body1Wrap : body0Wrap;
	const CollisionObject2Wrapper* planeObjWrap = m_isSwapped ? body0Wrap : body1Wrap;

	ConvexShape* convexShape = (ConvexShape*)convexObjWrap->getCollisionShape();
	StaticPlaneShape* planeShape = (StaticPlaneShape*)planeObjWrap->getCollisionShape();

	bool hasCollision = false;
	const Vec3& planeNormal = planeShape->getPlaneNormal();
	const Scalar& planeConstant = planeShape->getPlaneConstant();
	Transform2 planeInConvex;
	planeInConvex = convexObjWrap->getWorldTransform().inverse() * planeObjWrap->getWorldTransform();
	Transform2 convexInPlaneTrans;
	convexInPlaneTrans = planeObjWrap->getWorldTransform().inverse() * convexObjWrap->getWorldTransform();

	Vec3 vtx = convexShape->localGetSupportingVertex(planeInConvex.getBasis() * -planeNormal);
	Vec3 vtxInPlane = convexInPlaneTrans(vtx);
	Scalar distance = (planeNormal.dot(vtxInPlane) - planeConstant);

	Vec3 vtxInPlaneProjected = vtxInPlane - distance * planeNormal;
	Vec3 vtxInPlaneWorld = planeObjWrap->getWorldTransform() * vtxInPlaneProjected;

	hasCollision = distance < m_manifoldPtr->getContactBreakingThreshold()+ resultOut->m_closestPointDistanceThreshold;
	resultOut->setPersistentManifold(m_manifoldPtr);
	if (hasCollision)
	{
		/// report a contact. internally this will be kept persistent, and contact reduction is done
		Vec3 normalOnSurfaceB = planeObjWrap->getWorldTransform().getBasis() * planeNormal;
		Vec3 pOnB = vtxInPlaneWorld;
		resultOut->addContactPoint(normalOnSurfaceB, pOnB, distance);
	}

	//the perturbation algorithm doesn't work well with implicit surfaces such as spheres, cylinder and cones:
	//they keep on rolling forever because of the additional off-center contact points
	//so only enable the feature for polyhedral shapes (BoxShape, ConvexHullShape etc)
	if (convexShape->isPolyhedral() && resultOut->getPersistentManifold()->getNumContacts() < m_minimumPointsPerturbationThreshold)
	{
		Vec3 v0, v1;
		PlaneSpace1(planeNormal, v0, v1);
		//now perform 'm_numPerturbationIterations' collision queries with the perturbated collision objects

		const Scalar angleLimit = 0.125f * SIMD_PI;
		Scalar perturbeAngle;
		Scalar radius = convexShape->getAngularMotionDisc();
		perturbeAngle = gContactBreakingThreshold / radius;
		if (perturbeAngle > angleLimit)
			perturbeAngle = angleLimit;

		Quat perturbeRot(v0, perturbeAngle);
		for (i32 i = 0; i < m_numPerturbationIterations; i++)
		{
			Scalar iterationAngle = i * (SIMD_2_PI / Scalar(m_numPerturbationIterations));
			Quat rotq(planeNormal, iterationAngle);
			collideSingleContact(rotq.inverse() * perturbeRot * rotq, body0Wrap, body1Wrap, dispatchInfo, resultOut);
		}
	}

	if (m_ownManifold)
	{
		if (m_manifoldPtr->getNumContacts())
		{
			resultOut->refreshContactPoints();
		}
	}
}

Scalar ConvexPlaneCollisionAlgorithm::calculateTimeOfImpact(CollisionObject2* col0, CollisionObject2* col1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	(void)resultOut;
	(void)dispatchInfo;
	(void)col0;
	(void)col1;

	//not yet
	return Scalar(1.);
}
