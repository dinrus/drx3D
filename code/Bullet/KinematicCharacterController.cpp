#include <stdio.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <drx3D/Physics/Collision/Dispatch/GhostObject.h>
#include <drx3D/Physics/Collision/Shapes/MultiSphereShape.h>
#include <drx3D/Physics/Collision/BroadPhase/OverlappingPairCache.h>
#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionWorld.h>
#include <drx3D/Maths/Linear/DefaultMotionState.h>
#include <drx3D/Physics/Dynamics/Character/KinematicCharacterController.h>

// static helper method
static Vec3
getNormalizedVector(const Vec3& v)
{
	Vec3 n(0, 0, 0);

	if (v.length() > SIMD_EPSILON)
	{
		n = v.normalized();
	}
	return n;
}

///@todo Interact with dynamic objects,
///Ride kinematicly animated platforms properly
///More realistic (or maybe just a config option) falling
/// -> Should integrate falling velocity manually and use that in stepDown()
///Support jumping
///Support ducking
class KinematicClosestNotMeRayResultCallback : public CollisionWorld::ClosestRayResultCallback
{
public:
	KinematicClosestNotMeRayResultCallback(CollisionObject2* me) : CollisionWorld::ClosestRayResultCallback(Vec3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 0.0))
	{
		m_me = me;
	}

	virtual Scalar addSingleResult(CollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		if (rayResult.m_collisionObject == m_me)
			return 1.0;

		return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
	}

protected:
	CollisionObject2* m_me;
};

class KinematicClosestNotMeConvexResultCallback : public CollisionWorld::ClosestConvexResultCallback
{
public:
	KinematicClosestNotMeConvexResultCallback(CollisionObject2* me, const Vec3& up, Scalar minSlopeDot)
		: CollisionWorld::ClosestConvexResultCallback(Vec3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 0.0)), m_me(me), m_up(up), m_minSlopeDot(minSlopeDot)
	{
	}

	virtual Scalar addSingleResult(CollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
	{
		if (convexResult.m_hitCollisionObject2 == m_me)
			return Scalar(1.0);

		if (!convexResult.m_hitCollisionObject2->hasContactResponse())
			return Scalar(1.0);

		Vec3 hitNormalWorld;
		if (normalInWorldSpace)
		{
			hitNormalWorld = convexResult.m_hitNormalLocal;
		}
		else
		{
			///need to transform normal into worldspace
			hitNormalWorld = convexResult.m_hitCollisionObject2->getWorldTransform().getBasis() * convexResult.m_hitNormalLocal;
		}

		Scalar dotUp = m_up.dot(hitNormalWorld);
		if (dotUp < m_minSlopeDot)
		{
			return Scalar(1.0);
		}

		return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
	}

protected:
	CollisionObject2* m_me;
	const Vec3 m_up;
	Scalar m_minSlopeDot;
};

/*
 * Returns the reflection direction of a ray going 'direction' hitting a surface with normal 'normal'
 *
 * from: http://www-cs-students.stanford.edu/~adityagp/final/node3.html
 */
Vec3 KinematicCharacterController::computeReflectionDirection(const Vec3& direction, const Vec3& normal)
{
	return direction - (Scalar(2.0) * direction.dot(normal)) * normal;
}

/*
 * Returns the portion of 'direction' that is parallel to 'normal'
 */
Vec3 KinematicCharacterController::parallelComponent(const Vec3& direction, const Vec3& normal)
{
	Scalar magnitude = direction.dot(normal);
	return normal * magnitude;
}

/*
 * Returns the portion of 'direction' that is perpindicular to 'normal'
 */
Vec3 KinematicCharacterController::perpindicularComponent(const Vec3& direction, const Vec3& normal)
{
	return direction - parallelComponent(direction, normal);
}

KinematicCharacterController::KinematicCharacterController(PairCachingGhostObject* ghostObject, ConvexShape* convexShape, Scalar stepHeight, const Vec3& up)
{
	m_ghostObject = ghostObject;
	m_up.setVal(0.0f, 0.0f, 1.0f);
	m_jumpAxis.setVal(0.0f, 0.0f, 1.0f);
	m_addedMargin = 0.02;
	m_walkDirection.setVal(0.0, 0.0, 0.0);
	m_AngVel.setVal(0.0, 0.0, 0.0);
	m_useGhostObjectSweepTest = true;
	m_turnAngle = Scalar(0.0);
	m_convexShape = convexShape;
	m_useWalkDirection = true;  // use walk direction by default, legacy behavior
	m_velocityTimeInterval = 0.0;
	m_verticalVelocity = 0.0;
	m_verticalOffset = 0.0;
	m_gravity = 9.8 * 3.0;  // 3G acceleration.
	m_fallSpeed = 55.0;     // Terminal velocity of a sky diver in m/s.
	m_jumpSpeed = 10.0;     // ?
	m_SetjumpSpeed = m_jumpSpeed;
	m_wasOnGround = false;
	m_wasJumping = false;
	m_interpolateUp = true;
	m_currentStepOffset = 0.0;
	m_maxPenetrationDepth = 0.2;
	full_drop = false;
	bounce_fix = false;
	m_linearDamping = Scalar(0.0);
	m_angularDamping = Scalar(0.0);

	setUp(up);
	setStepHeight(stepHeight);
	setMaxSlope(Radians(45.0));
}

KinematicCharacterController::~KinematicCharacterController()
{
}

PairCachingGhostObject* KinematicCharacterController::getGhostObject()
{
	return m_ghostObject;
}

bool KinematicCharacterController::recoverFromPenetration(CollisionWorld* collisionWorld)
{
	// Here we must refresh the overlapping paircache as the penetrating movement itself or the
	// previous recovery iteration might have used setWorldTransform and pushed us into an object
	// that is not in the previous cache contents from the last timestep, as will happen if we
	// are pushed into a new AABB overlap. Unhandled this means the next convex sweep gets stuck.
	//
	// Do this by calling the broadphase's setAabb with the moved AABB, this will update the broadphase
	// paircache and the ghostobject's internal paircache at the same time.    /BW

	Vec3 minAabb, maxAabb;
	m_convexShape->getAabb(m_ghostObject->getWorldTransform(), minAabb, maxAabb);
	collisionWorld->getBroadphase()->setAabb(m_ghostObject->getBroadphaseHandle(),
											 minAabb,
											 maxAabb,
											 collisionWorld->getDispatcher());

	bool penetration = false;

	collisionWorld->getDispatcher()->dispatchAllCollisionPairs(m_ghostObject->getOverlappingPairCache(), collisionWorld->getDispatchInfo(), collisionWorld->getDispatcher());

	m_currentPosition = m_ghostObject->getWorldTransform().getOrigin();

	//	Scalar maxPen = Scalar(0.0);
	for (i32 i = 0; i < m_ghostObject->getOverlappingPairCache()->getNumOverlappingPairs(); i++)
	{
		m_manifoldArray.resize(0);

		BroadphasePair* collisionPair = &m_ghostObject->getOverlappingPairCache()->getOverlappingPairArray()[i];

		CollisionObject2* obj0 = static_cast<CollisionObject2*>(collisionPair->m_pProxy0->m_clientObject);
		CollisionObject2* obj1 = static_cast<CollisionObject2*>(collisionPair->m_pProxy1->m_clientObject);

		if ((obj0 && !obj0->hasContactResponse()) || (obj1 && !obj1->hasContactResponse()))
			continue;

		if (!needsCollision(obj0, obj1))
			continue;

		if (collisionPair->m_algorithm)
			collisionPair->m_algorithm->getAllContactManifolds(m_manifoldArray);

		for (i32 j = 0; j < m_manifoldArray.size(); j++)
		{
			PersistentManifold* manifold = m_manifoldArray[j];
			Scalar directionSign = manifold->getBody0() == m_ghostObject ? Scalar(-1.0) : Scalar(1.0);
			for (i32 p = 0; p < manifold->getNumContacts(); p++)
			{
				const ManifoldPoint& pt = manifold->getContactPoint(p);

				Scalar dist = pt.getDistance();

				if (dist < -m_maxPenetrationDepth)
				{
					// TODO: cause problems on slopes, not sure if it is needed
					//if (dist < maxPen)
					//{
					//	maxPen = dist;
					//	m_touchingNormal = pt.m_normalWorldOnB * directionSign;//??

					//}
					m_currentPosition += pt.m_normalWorldOnB * directionSign * dist * Scalar(0.2);
					penetration = true;
				}
				else
				{
					//printf("touching %f\n", dist);
				}
			}

			//manifold->clearManifold();
		}
	}
	Transform2 newTrans = m_ghostObject->getWorldTransform();
	newTrans.setOrigin(m_currentPosition);
	m_ghostObject->setWorldTransform(newTrans);
	//	printf("m_touchingNormal = %f,%f,%f\n",m_touchingNormal[0],m_touchingNormal[1],m_touchingNormal[2]);
	return penetration;
}

void KinematicCharacterController::stepUp(CollisionWorld* world)
{
	Scalar stepHeight = 0.0f;
	if (m_verticalVelocity < 0.0)
		stepHeight = m_stepHeight;

	// phase 1: up
	Transform2 start, end;

	start.setIdentity();
	end.setIdentity();

	/* FIXME: Handle penetration properly */
	start.setOrigin(m_currentPosition);

	m_targetPosition = m_currentPosition + m_up * (stepHeight) + m_jumpAxis * ((m_verticalOffset > 0.f ? m_verticalOffset : 0.f));
	m_currentPosition = m_targetPosition;

	end.setOrigin(m_targetPosition);

	start.setRotation(m_currentOrientation);
	end.setRotation(m_targetOrientation);

	KinematicClosestNotMeConvexResultCallback callback(m_ghostObject, -m_up, m_maxSlopeCosine);
	callback.m_collisionFilterGroup = getGhostObject()->getBroadphaseHandle()->m_collisionFilterGroup;
	callback.m_collisionFilterMask = getGhostObject()->getBroadphaseHandle()->m_collisionFilterMask;

	if (m_useGhostObjectSweepTest)
	{
		m_ghostObject->convexSweepTest(m_convexShape, start, end, callback, world->getDispatchInfo().m_allowedCcdPenetration);
	}
	else
	{
		world->convexSweepTest(m_convexShape, start, end, callback, world->getDispatchInfo().m_allowedCcdPenetration);
	}

	if (callback.hasHit() && m_ghostObject->hasContactResponse() && needsCollision(m_ghostObject, callback.m_hitCollisionObject2))
	{
		// Only modify the position if the hit was a slope and not a wall or ceiling.
		if (callback.m_hitNormalWorld.dot(m_up) > 0.0)
		{
			// we moved up only a fraction of the step height
			m_currentStepOffset = stepHeight * callback.m_closestHitFraction;
			if (m_interpolateUp == true)
				m_currentPosition.setInterpolate3(m_currentPosition, m_targetPosition, callback.m_closestHitFraction);
			else
				m_currentPosition = m_targetPosition;
		}

		Transform2& xform = m_ghostObject->getWorldTransform();
		xform.setOrigin(m_currentPosition);
		m_ghostObject->setWorldTransform(xform);

		// fix penetration if we hit a ceiling for example
		i32 numPenetrationLoops = 0;
		m_touchingContact = false;
		while (recoverFromPenetration(world))
		{
			numPenetrationLoops++;
			m_touchingContact = true;
			if (numPenetrationLoops > 4)
			{
				//printf("character could not recover from penetration = %d\n", numPenetrationLoops);
				break;
			}
		}
		m_targetPosition = m_ghostObject->getWorldTransform().getOrigin();
		m_currentPosition = m_targetPosition;

		if (m_verticalOffset > 0)
		{
			m_verticalOffset = 0.0;
			m_verticalVelocity = 0.0;
			m_currentStepOffset = m_stepHeight;
		}
	}
	else
	{
		m_currentStepOffset = stepHeight;
		m_currentPosition = m_targetPosition;
	}
}

bool KinematicCharacterController::needsCollision(const CollisionObject2* body0, const CollisionObject2* body1)
{
	bool collides = (body0->getBroadphaseHandle()->m_collisionFilterGroup & body1->getBroadphaseHandle()->m_collisionFilterMask) != 0;
	collides = collides && (body1->getBroadphaseHandle()->m_collisionFilterGroup & body0->getBroadphaseHandle()->m_collisionFilterMask);
	return collides;
}

void KinematicCharacterController::updateTargetPositionBasedOnCollision(const Vec3& hitNormal, Scalar tangentMag, Scalar normalMag)
{
	Vec3 movementDirection = m_targetPosition - m_currentPosition;
	Scalar movementLength = movementDirection.length();
	if (movementLength > SIMD_EPSILON)
	{
		movementDirection.normalize();

		Vec3 reflectDir = computeReflectionDirection(movementDirection, hitNormal);
		reflectDir.normalize();

		Vec3 parallelDir, perpindicularDir;

		parallelDir = parallelComponent(reflectDir, hitNormal);
		perpindicularDir = perpindicularComponent(reflectDir, hitNormal);

		m_targetPosition = m_currentPosition;
		if (0)  //tangentMag != 0.0)
		{
			Vec3 parComponent = parallelDir * Scalar(tangentMag * movementLength);
			//			printf("parComponent=%f,%f,%f\n",parComponent[0],parComponent[1],parComponent[2]);
			m_targetPosition += parComponent;
		}

		if (normalMag != 0.0)
		{
			Vec3 perpComponent = perpindicularDir * Scalar(normalMag * movementLength);
			//			printf("perpComponent=%f,%f,%f\n",perpComponent[0],perpComponent[1],perpComponent[2]);
			m_targetPosition += perpComponent;
		}
	}
	else
	{
		//		printf("movementLength don't normalize a zero vector\n");
	}
}

void KinematicCharacterController::stepForwardAndStrafe(CollisionWorld* collisionWorld, const Vec3& walkMove)
{
	// printf("m_normalizedDirection=%f,%f,%f\n",
	// 	m_normalizedDirection[0],m_normalizedDirection[1],m_normalizedDirection[2]);
	// phase 2: forward and strafe
	Transform2 start, end;

	m_targetPosition = m_currentPosition + walkMove;

	start.setIdentity();
	end.setIdentity();

	Scalar fraction = 1.0;
	Scalar distance2 = (m_currentPosition - m_targetPosition).length2();
	//	printf("distance2=%f\n",distance2);

	i32 maxIter = 10;

	while (fraction > Scalar(0.01) && maxIter-- > 0)
	{
		start.setOrigin(m_currentPosition);
		end.setOrigin(m_targetPosition);
		Vec3 sweepDirNegative(m_currentPosition - m_targetPosition);

		start.setRotation(m_currentOrientation);
		end.setRotation(m_targetOrientation);

		KinematicClosestNotMeConvexResultCallback callback(m_ghostObject, sweepDirNegative, Scalar(0.0));
		callback.m_collisionFilterGroup = getGhostObject()->getBroadphaseHandle()->m_collisionFilterGroup;
		callback.m_collisionFilterMask = getGhostObject()->getBroadphaseHandle()->m_collisionFilterMask;

		Scalar margin = m_convexShape->getMargin();
		m_convexShape->setMargin(margin + m_addedMargin);

		if (!(start == end))
		{
			if (m_useGhostObjectSweepTest)
			{
				m_ghostObject->convexSweepTest(m_convexShape, start, end, callback, collisionWorld->getDispatchInfo().m_allowedCcdPenetration);
			}
			else
			{
				collisionWorld->convexSweepTest(m_convexShape, start, end, callback, collisionWorld->getDispatchInfo().m_allowedCcdPenetration);
			}
		}
		m_convexShape->setMargin(margin);

		fraction -= callback.m_closestHitFraction;

		if (callback.hasHit() && m_ghostObject->hasContactResponse() && needsCollision(m_ghostObject, callback.m_hitCollisionObject2))
		{
			// we moved only a fraction
			//Scalar hitDistance;
			//hitDistance = (callback.m_hitPointWorld - m_currentPosition).length();

			//			m_currentPosition.setInterpolate3 (m_currentPosition, m_targetPosition, callback.m_closestHitFraction);

			updateTargetPositionBasedOnCollision(callback.m_hitNormalWorld);
			Vec3 currentDir = m_targetPosition - m_currentPosition;
			distance2 = currentDir.length2();
			if (distance2 > SIMD_EPSILON)
			{
				currentDir.normalize();
				/* See Quake2: "If velocity is against original velocity, stop ead to avoid tiny oscilations in sloping corners." */
				if (currentDir.dot(m_normalizedDirection) <= Scalar(0.0))
				{
					break;
				}
			}
			else
			{
				//				printf("currentDir: don't normalize a zero vector\n");
				break;
			}
		}
		else
		{
			m_currentPosition = m_targetPosition;
		}
	}
}

void KinematicCharacterController::stepDown(CollisionWorld* collisionWorld, Scalar dt)
{
	Transform2 start, end, end_double;
	bool runonce = false;

	// phase 3: down
	/*Scalar additionalDownStep = (m_wasOnGround && !onGround()) ? m_stepHeight : 0.0;
	Vec3 step_drop = m_up * (m_currentStepOffset + additionalDownStep);
	Scalar downVelocity = (additionalDownStep == 0.0 && m_verticalVelocity<0.0?-m_verticalVelocity:0.0) * dt;
	Vec3 gravity_drop = m_up * downVelocity; 
	m_targetPosition -= (step_drop + gravity_drop);*/

	Vec3 orig_position = m_targetPosition;

	Scalar downVelocity = (m_verticalVelocity < 0.f ? -m_verticalVelocity : 0.f) * dt;

	if (m_verticalVelocity > 0.0)
		return;

	if (downVelocity > 0.0 && downVelocity > m_fallSpeed && (m_wasOnGround || !m_wasJumping))
		downVelocity = m_fallSpeed;

	Vec3 step_drop = m_up * (m_currentStepOffset + downVelocity);
	m_targetPosition -= step_drop;

	KinematicClosestNotMeConvexResultCallback callback(m_ghostObject, m_up, m_maxSlopeCosine);
	callback.m_collisionFilterGroup = getGhostObject()->getBroadphaseHandle()->m_collisionFilterGroup;
	callback.m_collisionFilterMask = getGhostObject()->getBroadphaseHandle()->m_collisionFilterMask;

	KinematicClosestNotMeConvexResultCallback callback2(m_ghostObject, m_up, m_maxSlopeCosine);
	callback2.m_collisionFilterGroup = getGhostObject()->getBroadphaseHandle()->m_collisionFilterGroup;
	callback2.m_collisionFilterMask = getGhostObject()->getBroadphaseHandle()->m_collisionFilterMask;

	while (1)
	{
		start.setIdentity();
		end.setIdentity();

		end_double.setIdentity();

		start.setOrigin(m_currentPosition);
		end.setOrigin(m_targetPosition);

		start.setRotation(m_currentOrientation);
		end.setRotation(m_targetOrientation);

		//set double test for 2x the step drop, to check for a large drop vs small drop
		end_double.setOrigin(m_targetPosition - step_drop);

		if (m_useGhostObjectSweepTest)
		{
			m_ghostObject->convexSweepTest(m_convexShape, start, end, callback, collisionWorld->getDispatchInfo().m_allowedCcdPenetration);

			if (!callback.hasHit() && m_ghostObject->hasContactResponse())
			{
				//test a double fall height, to see if the character should interpolate it's fall (full) or not (partial)
				m_ghostObject->convexSweepTest(m_convexShape, start, end_double, callback2, collisionWorld->getDispatchInfo().m_allowedCcdPenetration);
			}
		}
		else
		{
			collisionWorld->convexSweepTest(m_convexShape, start, end, callback, collisionWorld->getDispatchInfo().m_allowedCcdPenetration);

			if (!callback.hasHit() && m_ghostObject->hasContactResponse())
			{
				//test a double fall height, to see if the character should interpolate it's fall (large) or not (small)
				collisionWorld->convexSweepTest(m_convexShape, start, end_double, callback2, collisionWorld->getDispatchInfo().m_allowedCcdPenetration);
			}
		}

		Scalar downVelocity2 = (m_verticalVelocity < 0.f ? -m_verticalVelocity : 0.f) * dt;
		bool has_hit;
		if (bounce_fix == true)
			has_hit = (callback.hasHit() || callback2.hasHit()) && m_ghostObject->hasContactResponse() && needsCollision(m_ghostObject, callback.m_hitCollisionObject2);
		else
			has_hit = callback2.hasHit() && m_ghostObject->hasContactResponse() && needsCollision(m_ghostObject, callback2.m_hitCollisionObject2);

		Scalar stepHeight = 0.0f;
		if (m_verticalVelocity < 0.0)
			stepHeight = m_stepHeight;

		if (downVelocity2 > 0.0 && downVelocity2 < stepHeight && has_hit == true && runonce == false && (m_wasOnGround || !m_wasJumping))
		{
			//redo the velocity calculation when falling a small amount, for fast stairs motion
			//for larger falls, use the smoother/slower interpolated movement by not touching the target position

			m_targetPosition = orig_position;
			downVelocity = stepHeight;

			step_drop = m_up * (m_currentStepOffset + downVelocity);
			m_targetPosition -= step_drop;
			runonce = true;
			continue;  //re-run previous tests
		}
		break;
	}

	if ((m_ghostObject->hasContactResponse() && (callback.hasHit() && needsCollision(m_ghostObject, callback.m_hitCollisionObject2))) || runonce == true)
	{
		// we dropped a fraction of the height -> hit floor
		Scalar fraction = (m_currentPosition.getY() - callback.m_hitPointWorld.getY()) / 2;

		//printf("hitpoint: %g - pos %g\n", callback.m_hitPointWorld.getY(), m_currentPosition.getY());

		if (bounce_fix == true)
		{
			if (full_drop == true)
				m_currentPosition.setInterpolate3(m_currentPosition, m_targetPosition, callback.m_closestHitFraction);
			else
				//due to errors in the closestHitFraction variable when used with large polygons, calculate the hit fraction manually
				m_currentPosition.setInterpolate3(m_currentPosition, m_targetPosition, fraction);
		}
		else
			m_currentPosition.setInterpolate3(m_currentPosition, m_targetPosition, callback.m_closestHitFraction);

		full_drop = false;

		m_verticalVelocity = 0.0;
		m_verticalOffset = 0.0;
		m_wasJumping = false;
	}
	else
	{
		// we dropped the full height

		full_drop = true;

		if (bounce_fix == true)
		{
			downVelocity = (m_verticalVelocity < 0.f ? -m_verticalVelocity : 0.f) * dt;
			if (downVelocity > m_fallSpeed && (m_wasOnGround || !m_wasJumping))
			{
				m_targetPosition += step_drop;  //undo previous target change
				downVelocity = m_fallSpeed;
				step_drop = m_up * (m_currentStepOffset + downVelocity);
				m_targetPosition -= step_drop;
			}
		}
		//printf("full drop - %g, %g\n", m_currentPosition.getY(), m_targetPosition.getY());

		m_currentPosition = m_targetPosition;
	}
}

void KinematicCharacterController::setWalkDirection(
	const Vec3& walkDirection)
{
	m_useWalkDirection = true;
	m_walkDirection = walkDirection;
	m_normalizedDirection = getNormalizedVector(m_walkDirection);
}

void KinematicCharacterController::setVelocityForTimeInterval(
	const Vec3& velocity,
	Scalar timeInterval)
{
	//	printf("setVelocity!\n");
	//	printf("  interval: %f\n", timeInterval);
	//	printf("  velocity: (%f, %f, %f)\n",
	//		 velocity.x(), velocity.y(), velocity.z());

	m_useWalkDirection = false;
	m_walkDirection = velocity;
	m_normalizedDirection = getNormalizedVector(m_walkDirection);
	m_velocityTimeInterval += timeInterval;
}

void KinematicCharacterController::setAngularVelocity(const Vec3& velocity)
{
	m_AngVel = velocity;
}

const Vec3& KinematicCharacterController::getAngularVelocity() const
{
	return m_AngVel;
}

void KinematicCharacterController::setLinearVelocity(const Vec3& velocity)
{
	m_walkDirection = velocity;

	// HACK: if we are moving in the direction of the up, treat it as a jump :(
	if (m_walkDirection.length2() > 0)
	{
		Vec3 w = velocity.normalized();
		Scalar c = w.dot(m_up);
		if (c != 0)
		{
			//there is a component in walkdirection for vertical velocity
			Vec3 upComponent = m_up * (Sin(SIMD_HALF_PI - Acos(c)) * m_walkDirection.length());
			m_walkDirection -= upComponent;
			m_verticalVelocity = (c < 0.0f ? -1 : 1) * upComponent.length();

			if (c > 0.0f)
			{
				m_wasJumping = true;
				m_jumpPosition = m_ghostObject->getWorldTransform().getOrigin();
			}
		}
	}
	else
		m_verticalVelocity = 0.0f;
}

Vec3 KinematicCharacterController::getLinearVelocity() const
{
	return m_walkDirection + (m_verticalVelocity * m_up);
}

void KinematicCharacterController::reset(CollisionWorld* collisionWorld)
{
	m_verticalVelocity = 0.0;
	m_verticalOffset = 0.0;
	m_wasOnGround = false;
	m_wasJumping = false;
	m_walkDirection.setVal(0, 0, 0);
	m_velocityTimeInterval = 0.0;

	//clear pair cache
	HashedOverlappingPairCache* cache = m_ghostObject->getOverlappingPairCache();
	while (cache->getOverlappingPairArray().size() > 0)
	{
		cache->removeOverlappingPair(cache->getOverlappingPairArray()[0].m_pProxy0, cache->getOverlappingPairArray()[0].m_pProxy1, collisionWorld->getDispatcher());
	}
}

void KinematicCharacterController::warp(const Vec3& origin)
{
	Transform2 xform;
	xform.setIdentity();
	xform.setOrigin(origin);
	m_ghostObject->setWorldTransform(xform);
}

void KinematicCharacterController::preStep(CollisionWorld* collisionWorld)
{
	m_currentPosition = m_ghostObject->getWorldTransform().getOrigin();
	m_targetPosition = m_currentPosition;

	m_currentOrientation = m_ghostObject->getWorldTransform().getRotation();
	m_targetOrientation = m_currentOrientation;
	//	printf("m_targetPosition=%f,%f,%f\n",m_targetPosition[0],m_targetPosition[1],m_targetPosition[2]);
}

void KinematicCharacterController::playerStep(CollisionWorld* collisionWorld, Scalar dt)
{
	//	printf("playerStep(): ");
	//	printf("  dt = %f", dt);

	if (m_AngVel.length2() > 0.0f)
	{
		m_AngVel *= Pow(Scalar(1) - m_angularDamping, dt);
	}

	// integrate for angular velocity
	if (m_AngVel.length2() > 0.0f)
	{
		Transform2 xform;
		xform = m_ghostObject->getWorldTransform();

		Quat rot(m_AngVel.normalized(), m_AngVel.length() * dt);

		Quat orn = rot * xform.getRotation();

		xform.setRotation(orn);
		m_ghostObject->setWorldTransform(xform);

		m_currentPosition = m_ghostObject->getWorldTransform().getOrigin();
		m_targetPosition = m_currentPosition;
		m_currentOrientation = m_ghostObject->getWorldTransform().getRotation();
		m_targetOrientation = m_currentOrientation;
	}

	// quick check...
	if (!m_useWalkDirection && (m_velocityTimeInterval <= 0.0 || m_walkDirection.fuzzyZero())) 
	{
		//		printf("\n");
		return;  // no motion
	}

	m_wasOnGround = onGround();

	//Vec3 lvel = m_walkDirection;
	//Scalar c = 0.0f;

	if (m_walkDirection.length2() > 0)
	{
		// apply damping
		m_walkDirection *= Pow(Scalar(1) - m_linearDamping, dt);
	}

	m_verticalVelocity *= Pow(Scalar(1) - m_linearDamping, dt);

	// Update fall velocity.
	m_verticalVelocity -= m_gravity * dt;
	if (m_verticalVelocity > 0.0 && m_verticalVelocity > m_jumpSpeed)
	{
		m_verticalVelocity = m_jumpSpeed;
	}
	if (m_verticalVelocity < 0.0 && Fabs(m_verticalVelocity) > Fabs(m_fallSpeed))
	{
		m_verticalVelocity = -Fabs(m_fallSpeed);
	}
	m_verticalOffset = m_verticalVelocity * dt;

	Transform2 xform;
	xform = m_ghostObject->getWorldTransform();

	//	printf("walkDirection(%f,%f,%f)\n",walkDirection[0],walkDirection[1],walkDirection[2]);
	//	printf("walkSpeed=%f\n",walkSpeed);

	stepUp(collisionWorld);
	//todo: Experimenting with behavior of controller when it hits a ceiling..
	//bool hitUp = stepUp (collisionWorld);
	//if (hitUp)
	//{
	//	m_verticalVelocity -= m_gravity * dt;
	//	if (m_verticalVelocity > 0.0 && m_verticalVelocity > m_jumpSpeed)
	//	{
	//		m_verticalVelocity = m_jumpSpeed;
	//	}
	//	if (m_verticalVelocity < 0.0 && Fabs(m_verticalVelocity) > Fabs(m_fallSpeed))
	//	{
	//		m_verticalVelocity = -Fabs(m_fallSpeed);
	//	}
	//	m_verticalOffset = m_verticalVelocity * dt;

	//	xform = m_ghostObject->getWorldTransform();
	//}

	if (m_useWalkDirection)
	{
		stepForwardAndStrafe(collisionWorld, m_walkDirection);
	}
	else
	{
		//printf("  time: %f", m_velocityTimeInterval);
		// still have some time left for moving!
		Scalar dtMoving =
			(dt < m_velocityTimeInterval) ? dt : m_velocityTimeInterval;
		m_velocityTimeInterval -= dt;

		// how far will we move while we are moving?
		Vec3 move = m_walkDirection * dtMoving;

		//printf("  dtMoving: %f", dtMoving);

		// okay, step
		stepForwardAndStrafe(collisionWorld, move);
	}
	stepDown(collisionWorld, dt);

	//todo: Experimenting with max jump height
	//if (m_wasJumping)
	//{
	//	Scalar ds = m_currentPosition[m_upAxis] - m_jumpPosition[m_upAxis];
	//	if (ds > m_maxJumpHeight)
	//	{
	//		// substract the overshoot
	//		m_currentPosition[m_upAxis] -= ds - m_maxJumpHeight;

	//		// max height was reached, so potential energy is at max
	//		// and kinematic energy is 0, thus velocity is 0.
	//		if (m_verticalVelocity > 0.0)
	//			m_verticalVelocity = 0.0;
	//	}
	//}
	// printf("\n");

	xform.setOrigin(m_currentPosition);
	m_ghostObject->setWorldTransform(xform);

	i32 numPenetrationLoops = 0;
	m_touchingContact = false;
	while (recoverFromPenetration(collisionWorld))
	{
		numPenetrationLoops++;
		m_touchingContact = true;
		if (numPenetrationLoops > 4)
		{
			//printf("character could not recover from penetration = %d\n", numPenetrationLoops);
			break;
		}
	}
}

void KinematicCharacterController::setFallSpeed(Scalar fallSpeed)
{
	m_fallSpeed = fallSpeed;
}

void KinematicCharacterController::setJumpSpeed(Scalar jumpSpeed)
{
	m_jumpSpeed = jumpSpeed;
	m_SetjumpSpeed = m_jumpSpeed;
}

void KinematicCharacterController::setMaxJumpHeight(Scalar maxJumpHeight)
{
	m_maxJumpHeight = maxJumpHeight;
}

bool KinematicCharacterController::canJump() const
{
	return onGround();
}

void KinematicCharacterController::jump(const Vec3& v)
{
	m_jumpSpeed = v.length2() == 0 ? m_SetjumpSpeed : v.length();
	m_verticalVelocity = m_jumpSpeed;
	m_wasJumping = true;

	m_jumpAxis = v.length2() == 0 ? m_up : v.normalized();

	m_jumpPosition = m_ghostObject->getWorldTransform().getOrigin();

#if 0
	currently no jumping.
	Transform2 xform;
	m_rigidBody->getMotionState()->getWorldTransform (xform);
	Vec3 up = xform.getBasis()[1];
	up.normalize ();
	Scalar magnitude = (Scalar(1.0)/m_rigidBody->getInvMass()) * Scalar(8.0);
	m_rigidBody->applyCentralImpulse (up * magnitude);
#endif
}

void KinematicCharacterController::setGravity(const Vec3& gravity)
{
	if (gravity.length2() > 0) setUpVector(-gravity);

	m_gravity = gravity.length();
}

Vec3 KinematicCharacterController::getGravity() const
{
	return -m_gravity * m_up;
}

void KinematicCharacterController::setMaxSlope(Scalar slopeRadians)
{
	m_maxSlopeRadians = slopeRadians;
	m_maxSlopeCosine = Cos(slopeRadians);
}

Scalar KinematicCharacterController::getMaxSlope() const
{
	return m_maxSlopeRadians;
}

void KinematicCharacterController::setMaxPenetrationDepth(Scalar d)
{
	m_maxPenetrationDepth = d;
}

Scalar KinematicCharacterController::getMaxPenetrationDepth() const
{
	return m_maxPenetrationDepth;
}

bool KinematicCharacterController::onGround() const
{
	return (fabs(m_verticalVelocity) < SIMD_EPSILON) && (fabs(m_verticalOffset) < SIMD_EPSILON);
}

void KinematicCharacterController::setStepHeight(Scalar h)
{
	m_stepHeight = h;
}

Vec3* KinematicCharacterController::getUpAxisDirections()
{
	static Vec3 sUpAxisDirection[3] = {Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)};

	return sUpAxisDirection;
}

void KinematicCharacterController::debugDraw(IDebugDraw* debugDrawer)
{
}

void KinematicCharacterController::setUpInterpolate(bool value)
{
	m_interpolateUp = value;
}

void KinematicCharacterController::setUp(const Vec3& up)
{
	if (up.length2() > 0 && m_gravity > 0.0f)
	{
		setGravity(-m_gravity * up.normalized());
		return;
	}

	setUpVector(up);
}

void KinematicCharacterController::setUpVector(const Vec3& up)
{
	if (m_up == up)
		return;

	Vec3 u = m_up;

	if (up.length2() > 0)
		m_up = up.normalized();
	else
		m_up = Vec3(0.0, 0.0, 0.0);

	if (!m_ghostObject) return;
	Quat rot = getRotation(m_up, u);

	//set orientation with new up
	Transform2 xform;
	xform = m_ghostObject->getWorldTransform();
	Quat orn = rot.inverse() * xform.getRotation();
	xform.setRotation(orn);
	m_ghostObject->setWorldTransform(xform);
}

Quat KinematicCharacterController::getRotation(Vec3& v0, Vec3& v1) const
{
	if (v0.length2() == 0.0f || v1.length2() == 0.0f)
	{
		Quat q;
		return q;
	}

	return shortestArcQuatNormalize2(v0, v1);
}
