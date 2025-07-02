
#include <drx3D/Physics/Collision/Dispatch/SphereBoxCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>
#include <drx3D/Physics/Collision/Shapes/BoxShape.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>
//#include <stdio.h>

SphereBoxCollisionAlgorithm::SphereBoxCollisionAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* col0Wrap, const CollisionObject2Wrapper* col1Wrap, bool isSwapped)
	: ActivatingCollisionAlgorithm(ci, col0Wrap, col1Wrap),
	  m_ownManifold(false),
	  m_manifoldPtr(mf),
	  m_isSwapped(isSwapped)
{
	const CollisionObject2Wrapper* sphereObjWrap = m_isSwapped ? col1Wrap : col0Wrap;
	const CollisionObject2Wrapper* boxObjWrap = m_isSwapped ? col0Wrap : col1Wrap;

	if (!m_manifoldPtr && m_dispatcher->needsCollision(sphereObjWrap->getCollisionObject(), boxObjWrap->getCollisionObject()))
	{
		m_manifoldPtr = m_dispatcher->getNewManifold(sphereObjWrap->getCollisionObject(), boxObjWrap->getCollisionObject());
		m_ownManifold = true;
	}
}

SphereBoxCollisionAlgorithm::~SphereBoxCollisionAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}

void SphereBoxCollisionAlgorithm::processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	(void)dispatchInfo;
	(void)resultOut;
	if (!m_manifoldPtr)
		return;

	const CollisionObject2Wrapper* sphereObjWrap = m_isSwapped ? body1Wrap : body0Wrap;
	const CollisionObject2Wrapper* boxObjWrap = m_isSwapped ? body0Wrap : body1Wrap;

	Vec3 pOnBox;

	Vec3 normalOnSurfaceB;
	Scalar penetrationDepth;
	Vec3 sphereCenter = sphereObjWrap->getWorldTransform().getOrigin();
	const SphereShape* sphere0 = (const SphereShape*)sphereObjWrap->getCollisionShape();
	Scalar radius = sphere0->getRadius();
	Scalar maxContactDistance = m_manifoldPtr->getContactBreakingThreshold();

	resultOut->setPersistentManifold(m_manifoldPtr);

	if (getSphereDistance(boxObjWrap, pOnBox, normalOnSurfaceB, penetrationDepth, sphereCenter, radius, maxContactDistance))
	{
		/// report a contact. internally this will be kept persistent, and contact reduction is done
		resultOut->addContactPoint(normalOnSurfaceB, pOnBox, penetrationDepth);
	}

	if (m_ownManifold)
	{
		if (m_manifoldPtr->getNumContacts())
		{
			resultOut->refreshContactPoints();
		}
	}
}

Scalar SphereBoxCollisionAlgorithm::calculateTimeOfImpact(CollisionObject2* col0, CollisionObject2* col1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	(void)resultOut;
	(void)dispatchInfo;
	(void)col0;
	(void)col1;

	//not yet
	return Scalar(1.);
}

bool SphereBoxCollisionAlgorithm::getSphereDistance(const CollisionObject2Wrapper* boxObjWrap, Vec3& pointOnBox, Vec3& normal, Scalar& penetrationDepth, const Vec3& sphereCenter, Scalar fRadius, Scalar maxContactDistance)
{
	const BoxShape* boxShape = (const BoxShape*)boxObjWrap->getCollisionShape();
	Vec3 const& boxHalfExtent = boxShape->getHalfExtentsWithoutMargin();
	Scalar boxMargin = boxShape->getMargin();
	penetrationDepth = 1.0f;

	// convert the sphere position to the box's local space
	Transform2 const& m44T = boxObjWrap->getWorldTransform();
	Vec3 sphereRelPos = m44T.invXform(sphereCenter);

	// Determine the closest point to the sphere center in the box
	Vec3 closestPoint = sphereRelPos;
	closestPoint.setX(d3Min(boxHalfExtent.getX(), closestPoint.getX()));
	closestPoint.setX(d3Max(-boxHalfExtent.getX(), closestPoint.getX()));
	closestPoint.setY(d3Min(boxHalfExtent.getY(), closestPoint.getY()));
	closestPoint.setY(d3Max(-boxHalfExtent.getY(), closestPoint.getY()));
	closestPoint.setZ(d3Min(boxHalfExtent.getZ(), closestPoint.getZ()));
	closestPoint.setZ(d3Max(-boxHalfExtent.getZ(), closestPoint.getZ()));

	Scalar intersectionDist = fRadius + boxMargin;
	Scalar contactDist = intersectionDist + maxContactDistance;
	normal = sphereRelPos - closestPoint;

	//if there is no penetration, we are done
	Scalar dist2 = normal.length2();
	if (dist2 > contactDist * contactDist)
	{
		return false;
	}

	Scalar distance;

	//special case if the sphere center is inside the box
	if (dist2 <= SIMD_EPSILON)
	{
		distance = -getSpherePenetration(boxHalfExtent, sphereRelPos, closestPoint, normal);
	}
	else  //compute the penetration details
	{
		distance = normal.length();
		normal /= distance;
	}

	pointOnBox = closestPoint + normal * boxMargin;
	//	v3PointOnSphere = sphereRelPos - (normal * fRadius);
	penetrationDepth = distance - intersectionDist;

	// transform back in world space
	Vec3 tmp = m44T(pointOnBox);
	pointOnBox = tmp;
	//	tmp = m44T(v3PointOnSphere);
	//	v3PointOnSphere = tmp;
	tmp = m44T.getBasis() * normal;
	normal = tmp;

	return true;
}

Scalar SphereBoxCollisionAlgorithm::getSpherePenetration(Vec3 const& boxHalfExtent, Vec3 const& sphereRelPos, Vec3& closestPoint, Vec3& normal)
{
	//project the center of the sphere on the closest face of the box
	Scalar faceDist = boxHalfExtent.getX() - sphereRelPos.getX();
	Scalar minDist = faceDist;
	closestPoint.setX(boxHalfExtent.getX());
	normal.setVal(Scalar(1.0f), Scalar(0.0f), Scalar(0.0f));

	faceDist = boxHalfExtent.getX() + sphereRelPos.getX();
	if (faceDist < minDist)
	{
		minDist = faceDist;
		closestPoint = sphereRelPos;
		closestPoint.setX(-boxHalfExtent.getX());
		normal.setVal(Scalar(-1.0f), Scalar(0.0f), Scalar(0.0f));
	}

	faceDist = boxHalfExtent.getY() - sphereRelPos.getY();
	if (faceDist < minDist)
	{
		minDist = faceDist;
		closestPoint = sphereRelPos;
		closestPoint.setY(boxHalfExtent.getY());
		normal.setVal(Scalar(0.0f), Scalar(1.0f), Scalar(0.0f));
	}

	faceDist = boxHalfExtent.getY() + sphereRelPos.getY();
	if (faceDist < minDist)
	{
		minDist = faceDist;
		closestPoint = sphereRelPos;
		closestPoint.setY(-boxHalfExtent.getY());
		normal.setVal(Scalar(0.0f), Scalar(-1.0f), Scalar(0.0f));
	}

	faceDist = boxHalfExtent.getZ() - sphereRelPos.getZ();
	if (faceDist < minDist)
	{
		minDist = faceDist;
		closestPoint = sphereRelPos;
		closestPoint.setZ(boxHalfExtent.getZ());
		normal.setVal(Scalar(0.0f), Scalar(0.0f), Scalar(1.0f));
	}

	faceDist = boxHalfExtent.getZ() + sphereRelPos.getZ();
	if (faceDist < minDist)
	{
		minDist = faceDist;
		closestPoint = sphereRelPos;
		closestPoint.setZ(-boxHalfExtent.getZ());
		normal.setVal(Scalar(0.0f), Scalar(0.0f), Scalar(-1.0f));
	}

	return minDist;
}
