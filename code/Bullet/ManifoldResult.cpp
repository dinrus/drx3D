
#include <drx3D/Physics/Collision/Dispatch/ManifoldResult.h>
#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

///This is to allow MaterialCombiner/Custom Friction/Restitution values
ContactAddedCallback gContactAddedCallback = 0;

CalculateCombinedCallback gCalculateCombinedRestitutionCallback = &ManifoldResult::calculateCombinedRestitution;
CalculateCombinedCallback gCalculateCombinedFrictionCallback = &ManifoldResult::calculateCombinedFriction;
CalculateCombinedCallback gCalculateCombinedRollingFrictionCallback = &ManifoldResult::calculateCombinedRollingFriction;
CalculateCombinedCallback gCalculateCombinedSpinningFrictionCallback = &ManifoldResult::calculateCombinedSpinningFriction;
CalculateCombinedCallback gCalculateCombinedContactDampingCallback = &ManifoldResult::calculateCombinedContactDamping;
CalculateCombinedCallback gCalculateCombinedContactStiffnessCallback = &ManifoldResult::calculateCombinedContactStiffness;

Scalar ManifoldResult::calculateCombinedRollingFriction(const CollisionObject2* body0, const CollisionObject2* body1)
{
	Scalar friction = body0->getRollingFriction() * body1->getFriction() + body1->getRollingFriction() * body0->getFriction();

	const Scalar MAX_FRICTION = Scalar(10.);
	if (friction < -MAX_FRICTION)
		friction = -MAX_FRICTION;
	if (friction > MAX_FRICTION)
		friction = MAX_FRICTION;
	return friction;
}

Scalar ManifoldResult::calculateCombinedSpinningFriction(const CollisionObject2* body0, const CollisionObject2* body1)
{
	Scalar friction = body0->getSpinningFriction() * body1->getFriction() + body1->getSpinningFriction() * body0->getFriction();

	const Scalar MAX_FRICTION = Scalar(10.);
	if (friction < -MAX_FRICTION)
		friction = -MAX_FRICTION;
	if (friction > MAX_FRICTION)
		friction = MAX_FRICTION;
	return friction;
}

///User can override this material combiner by implementing gContactAddedCallback and setting body0->m_collisionFlags |= CollisionObject2::customMaterialCallback;
Scalar ManifoldResult::calculateCombinedFriction(const CollisionObject2* body0, const CollisionObject2* body1)
{
	Scalar friction = body0->getFriction() * body1->getFriction();

	const Scalar MAX_FRICTION = Scalar(10.);
	if (friction < -MAX_FRICTION)
		friction = -MAX_FRICTION;
	if (friction > MAX_FRICTION)
		friction = MAX_FRICTION;
	return friction;
}

Scalar ManifoldResult::calculateCombinedRestitution(const CollisionObject2* body0, const CollisionObject2* body1)
{
	return body0->getRestitution() * body1->getRestitution();
}

Scalar ManifoldResult::calculateCombinedContactDamping(const CollisionObject2* body0, const CollisionObject2* body1)
{
	return body0->getContactDamping() + body1->getContactDamping();
}

Scalar ManifoldResult::calculateCombinedContactStiffness(const CollisionObject2* body0, const CollisionObject2* body1)
{
	Scalar s0 = body0->getContactStiffness();
	Scalar s1 = body1->getContactStiffness();

	Scalar tmp0 = Scalar(1) / s0;
	Scalar tmp1 = Scalar(1) / s1;
	Scalar combinedStiffness = Scalar(1) / (tmp0 + tmp1);
	return combinedStiffness;
}

ManifoldResult::ManifoldResult(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
	: m_manifoldPtr(0),
	  m_body0Wrap(body0Wrap),
	  m_body1Wrap(body1Wrap)
	  ,
	  m_partId0(-1),
	  m_partId1(-1),
	  m_index0(-1),
	  m_index1(-1)
	  ,
	  m_closestPointDistanceThreshold(0)
{
}

void ManifoldResult::addContactPoint(const Vec3& normalOnBInWorld, const Vec3& pointInWorld, Scalar depth)
{
	Assert(m_manifoldPtr);
	//order in manifold needs to match

	if (depth > m_manifoldPtr->getContactBreakingThreshold())
		//	if (depth > m_manifoldPtr->getContactProcessingThreshold())
		return;

	bool isSwapped = m_manifoldPtr->getBody0() != m_body0Wrap->getCollisionObject();
	bool isNewCollision = m_manifoldPtr->getNumContacts() == 0;

	Vec3 pointA = pointInWorld + normalOnBInWorld * depth;

	Vec3 localA;
	Vec3 localB;

	if (isSwapped)
	{
		localA = m_body1Wrap->getCollisionObject()->getWorldTransform().invXform(pointA);
		localB = m_body0Wrap->getCollisionObject()->getWorldTransform().invXform(pointInWorld);
	}
	else
	{
		localA = m_body0Wrap->getCollisionObject()->getWorldTransform().invXform(pointA);
		localB = m_body1Wrap->getCollisionObject()->getWorldTransform().invXform(pointInWorld);
	}

	ManifoldPoint newPt(localA, localB, normalOnBInWorld, depth);
	newPt.m_positionWorldOnA = pointA;
	newPt.m_positionWorldOnB = pointInWorld;

	i32 insertIndex = m_manifoldPtr->getCacheEntry(newPt);

	newPt.m_combinedFriction = gCalculateCombinedFrictionCallback(m_body0Wrap->getCollisionObject(), m_body1Wrap->getCollisionObject());
	newPt.m_combinedRestitution = gCalculateCombinedRestitutionCallback(m_body0Wrap->getCollisionObject(), m_body1Wrap->getCollisionObject());
	newPt.m_combinedRollingFriction = gCalculateCombinedRollingFrictionCallback(m_body0Wrap->getCollisionObject(), m_body1Wrap->getCollisionObject());
	newPt.m_combinedSpinningFriction = gCalculateCombinedSpinningFrictionCallback(m_body0Wrap->getCollisionObject(), m_body1Wrap->getCollisionObject());

	if ((m_body0Wrap->getCollisionObject()->getCollisionFlags() & CollisionObject2::CF_HAS_CONTACT_STIFFNESS_DAMPING) ||
		(m_body1Wrap->getCollisionObject()->getCollisionFlags() & CollisionObject2::CF_HAS_CONTACT_STIFFNESS_DAMPING))
	{
		newPt.m_combinedContactDamping1 = gCalculateCombinedContactDampingCallback(m_body0Wrap->getCollisionObject(), m_body1Wrap->getCollisionObject());
		newPt.m_combinedContactStiffness1 = gCalculateCombinedContactStiffnessCallback(m_body0Wrap->getCollisionObject(), m_body1Wrap->getCollisionObject());
		newPt.m_contactPointFlags |= DRX3D_CONTACT_FLAG_CONTACT_STIFFNESS_DAMPING;
	}

	if ((m_body0Wrap->getCollisionObject()->getCollisionFlags() & CollisionObject2::CF_HAS_FRICTION_ANCHOR) ||
		(m_body1Wrap->getCollisionObject()->getCollisionFlags() & CollisionObject2::CF_HAS_FRICTION_ANCHOR))
	{
		newPt.m_contactPointFlags |= DRX3D_CONTACT_FLAG_FRICTION_ANCHOR;
	}

	PlaneSpace1(newPt.m_normalWorldOnB, newPt.m_lateralFrictionDir1, newPt.m_lateralFrictionDir2);

	//BP mod, store contact triangles.
	if (isSwapped)
	{
		newPt.m_partId0 = m_partId1;
		newPt.m_partId1 = m_partId0;
		newPt.m_index0 = m_index1;
		newPt.m_index1 = m_index0;
	}
	else
	{
		newPt.m_partId0 = m_partId0;
		newPt.m_partId1 = m_partId1;
		newPt.m_index0 = m_index0;
		newPt.m_index1 = m_index1;
	}
	//printf("depth=%f\n",depth);
	///@todo, check this for any side effects
	if (insertIndex >= 0)
	{
		//const ManifoldPoint& oldPoint = m_manifoldPtr->getContactPoint(insertIndex);
		m_manifoldPtr->replaceContactPoint(newPt, insertIndex);
	}
	else
	{
		insertIndex = m_manifoldPtr->addManifoldPoint(newPt);
	}

	//User can override friction and/or restitution
	if (gContactAddedCallback &&
		//and if either of the two bodies requires custom material
		((m_body0Wrap->getCollisionObject()->getCollisionFlags() & CollisionObject2::CF_CUSTOM_MATERIAL_CALLBACK) ||
		 (m_body1Wrap->getCollisionObject()->getCollisionFlags() & CollisionObject2::CF_CUSTOM_MATERIAL_CALLBACK)))
	{
		//experimental feature info, for per-triangle material etc.
		const CollisionObject2Wrapper* obj0Wrap = isSwapped ? m_body1Wrap : m_body0Wrap;
		const CollisionObject2Wrapper* obj1Wrap = isSwapped ? m_body0Wrap : m_body1Wrap;
		(*gContactAddedCallback)(m_manifoldPtr->getContactPoint(insertIndex), obj0Wrap, newPt.m_partId0, newPt.m_index0, obj1Wrap, newPt.m_partId1, newPt.m_index1);
	}

	if (gContactStartedCallback && isNewCollision)
	{
		gContactStartedCallback(m_manifoldPtr);
	}
}
