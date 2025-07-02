#include <drx3D/Physics/Collision/Dispatch/GhostObject.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionWorld.h>
#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Maths/Linear/AabbUtil2.h>

GhostObject::GhostObject()
{
	m_internalType = CO_GHOST_OBJECT;
}

GhostObject::~GhostObject()
{
	///GhostObject should have been removed from the world, so no overlapping objects
	Assert(!m_overlappingObjects.size());
}

void GhostObject::addOverlappingObjectInternal(BroadphaseProxy* otherProxy, BroadphaseProxy* thisProxy)
{
	CollisionObject2* otherObject = (CollisionObject2*)otherProxy->m_clientObject;
	Assert(otherObject);
	///if this linearSearch becomes too slow (too many overlapping objects) we should add a more appropriate data structure
	i32 index = m_overlappingObjects.findLinearSearch(otherObject);
	if (index == m_overlappingObjects.size())
	{
		//not found
		m_overlappingObjects.push_back(otherObject);
	}
}

void GhostObject::removeOverlappingObjectInternal(BroadphaseProxy* otherProxy, Dispatcher* dispatcher, BroadphaseProxy* thisProxy)
{
	CollisionObject2* otherObject = (CollisionObject2*)otherProxy->m_clientObject;
	Assert(otherObject);
	i32 index = m_overlappingObjects.findLinearSearch(otherObject);
	if (index < m_overlappingObjects.size())
	{
		m_overlappingObjects[index] = m_overlappingObjects[m_overlappingObjects.size() - 1];
		m_overlappingObjects.pop_back();
	}
}

PairCachingGhostObject::PairCachingGhostObject()
{
	m_hashPairCache = new (AlignedAlloc(sizeof(HashedOverlappingPairCache), 16)) HashedOverlappingPairCache();
}

PairCachingGhostObject::~PairCachingGhostObject()
{
	m_hashPairCache->~HashedOverlappingPairCache();
	AlignedFree(m_hashPairCache);
}

void PairCachingGhostObject::addOverlappingObjectInternal(BroadphaseProxy* otherProxy, BroadphaseProxy* thisProxy)
{
	BroadphaseProxy* actualThisProxy = thisProxy ? thisProxy : getBroadphaseHandle();
	Assert(actualThisProxy);

	CollisionObject2* otherObject = (CollisionObject2*)otherProxy->m_clientObject;
	Assert(otherObject);
	i32 index = m_overlappingObjects.findLinearSearch(otherObject);
	if (index == m_overlappingObjects.size())
	{
		m_overlappingObjects.push_back(otherObject);
		m_hashPairCache->addOverlappingPair(actualThisProxy, otherProxy);
	}
}

void PairCachingGhostObject::removeOverlappingObjectInternal(BroadphaseProxy* otherProxy, Dispatcher* dispatcher, BroadphaseProxy* thisProxy1)
{
	CollisionObject2* otherObject = (CollisionObject2*)otherProxy->m_clientObject;
	BroadphaseProxy* actualThisProxy = thisProxy1 ? thisProxy1 : getBroadphaseHandle();
	Assert(actualThisProxy);

	Assert(otherObject);
	i32 index = m_overlappingObjects.findLinearSearch(otherObject);
	if (index < m_overlappingObjects.size())
	{
		m_overlappingObjects[index] = m_overlappingObjects[m_overlappingObjects.size() - 1];
		m_overlappingObjects.pop_back();
		m_hashPairCache->removeOverlappingPair(actualThisProxy, otherProxy, dispatcher);
	}
}

void GhostObject::convexSweepTest(const ConvexShape* castShape, const Transform2& convexFromWorld, const Transform2& convexToWorld, CollisionWorld::ConvexResultCallback& resultCallback, Scalar allowedCcdPenetration) const
{
	Transform2 convexFromTrans, convexToTrans;
	convexFromTrans = convexFromWorld;
	convexToTrans = convexToWorld;
	Vec3 castShapeAabbMin, castShapeAabbMax;
	/* Compute AABB that encompasses angular movement */
	{
		Vec3 linVel, angVel;
		Transform2Util::calculateVelocity(convexFromTrans, convexToTrans, 1.0, linVel, angVel);
		Transform2 R;
		R.setIdentity();
		R.setRotation(convexFromTrans.getRotation());
		castShape->calculateTemporalAabb(R, linVel, angVel, 1.0, castShapeAabbMin, castShapeAabbMax);
	}

	/// go over all objects, and if the ray intersects their aabb + cast shape aabb,
	// do a ray-shape query using convexCaster (CCD)
	i32 i;
	for (i = 0; i < m_overlappingObjects.size(); i++)
	{
		CollisionObject2* collisionObject = m_overlappingObjects[i];
		//only perform raycast if filterMask matches
		if (resultCallback.needsCollision(collisionObject->getBroadphaseHandle()))
		{
			//RigidcollisionObject* collisionObject = ctrl->GetRigidcollisionObject();
			Vec3 collisionObjectAabbMin, collisionObjectAabbMax;
			collisionObject->getCollisionShape()->getAabb(collisionObject->getWorldTransform(), collisionObjectAabbMin, collisionObjectAabbMax);
			AabbExpand(collisionObjectAabbMin, collisionObjectAabbMax, castShapeAabbMin, castShapeAabbMax);
			Scalar hitLambda = Scalar(1.);  //could use resultCallback.m_closestHitFraction, but needs testing
			Vec3 hitNormal;
			if (RayAabb(convexFromWorld.getOrigin(), convexToWorld.getOrigin(), collisionObjectAabbMin, collisionObjectAabbMax, hitLambda, hitNormal))
			{
				CollisionWorld::objectQuerySingle(castShape, convexFromTrans, convexToTrans,
													collisionObject,
													collisionObject->getCollisionShape(),
													collisionObject->getWorldTransform(),
													resultCallback,
													allowedCcdPenetration);
			}
		}
	}
}

void GhostObject::rayTest(const Vec3& rayFromWorld, const Vec3& rayToWorld, CollisionWorld::RayResultCallback& resultCallback) const
{
	Transform2 rayFromTrans;
	rayFromTrans.setIdentity();
	rayFromTrans.setOrigin(rayFromWorld);
	Transform2 rayToTrans;
	rayToTrans.setIdentity();
	rayToTrans.setOrigin(rayToWorld);

	i32 i;
	for (i = 0; i < m_overlappingObjects.size(); i++)
	{
		CollisionObject2* collisionObject = m_overlappingObjects[i];
		//only perform raycast if filterMask matches
		if (resultCallback.needsCollision(collisionObject->getBroadphaseHandle()))
		{
			CollisionWorld::rayTestSingle(rayFromTrans, rayToTrans,
											collisionObject,
											collisionObject->getCollisionShape(),
											collisionObject->getWorldTransform(),
											resultCallback);
		}
	}
}
