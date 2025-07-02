#ifndef DRX3D_MANIFOLD_RESULT_H
#define DRX3D_MANIFOLD_RESULT_H

class CollisionObject2;
struct CollisionObject2Wrapper;

#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>
class ManifoldPoint;

#include <drx3D/Physics/Collision/NarrowPhase/DiscreteCollisionDetectorInterface.h>

#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>

typedef bool (*ContactAddedCallback)(ManifoldPoint& cp, const CollisionObject2Wrapper* colObj0Wrap, i32 partId0, i32 index0, const CollisionObject2Wrapper* colObj1Wrap, i32 partId1, i32 index1);
extern ContactAddedCallback gContactAddedCallback;

//#define DEBUG_PART_INDEX 1

/// These callbacks are used to customize the algorith that combine restitution, friction, damping, Stiffness
typedef Scalar (*CalculateCombinedCallback)(const CollisionObject2* body0, const CollisionObject2* body1);

extern CalculateCombinedCallback gCalculateCombinedRestitutionCallback;
extern CalculateCombinedCallback gCalculateCombinedFrictionCallback;
extern CalculateCombinedCallback gCalculateCombinedRollingFrictionCallback;
extern CalculateCombinedCallback gCalculateCombinedSpinningFrictionCallback;
extern CalculateCombinedCallback gCalculateCombinedContactDampingCallback;
extern CalculateCombinedCallback gCalculateCombinedContactStiffnessCallback;

//ManifoldResult is a helper class to manage  contact results.
class ManifoldResult : public DiscreteCollisionDetectorInterface::Result
{
protected:
	PersistentManifold* m_manifoldPtr;

	const CollisionObject2Wrapper* m_body0Wrap;
	const CollisionObject2Wrapper* m_body1Wrap;
	i32 m_partId0;
	i32 m_partId1;
	i32 m_index0;
	i32 m_index1;

public:
	ManifoldResult()
		:
#ifdef DEBUG_PART_INDEX

		  m_partId0(-1),
		  m_partId1(-1),
		  m_index0(-1),
		  m_index1(-1)
#endif  //DEBUG_PART_INDEX
			  m_closestPointDistanceThreshold(0)
	{
	}

	ManifoldResult(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap);

	virtual ~ManifoldResult(){};

	void setPersistentManifold(PersistentManifold* manifoldPtr)
	{
		m_manifoldPtr = manifoldPtr;
	}

	const PersistentManifold* getPersistentManifold() const
	{
		return m_manifoldPtr;
	}
	PersistentManifold* getPersistentManifold()
	{
		return m_manifoldPtr;
	}

	virtual void setShapeIdentifiersA(i32 partId0, i32 index0)
	{
		m_partId0 = partId0;
		m_index0 = index0;
	}

	virtual void setShapeIdentifiersB(i32 partId1, i32 index1)
	{
		m_partId1 = partId1;
		m_index1 = index1;
	}

	virtual void addContactPoint(const Vec3& normalOnBInWorld, const Vec3& pointInWorld, Scalar depth);

	SIMD_FORCE_INLINE void refreshContactPoints()
	{
		Assert(m_manifoldPtr);
		if (!m_manifoldPtr->getNumContacts())
			return;

		bool isSwapped = m_manifoldPtr->getBody0() != m_body0Wrap->getCollisionObject();

		if (isSwapped)
		{
			m_manifoldPtr->refreshContactPoints(m_body1Wrap->getCollisionObject()->getWorldTransform(), m_body0Wrap->getCollisionObject()->getWorldTransform());
		}
		else
		{
			m_manifoldPtr->refreshContactPoints(m_body0Wrap->getCollisionObject()->getWorldTransform(), m_body1Wrap->getCollisionObject()->getWorldTransform());
		}
	}

	const CollisionObject2Wrapper* getBody0Wrap() const
	{
		return m_body0Wrap;
	}
	const CollisionObject2Wrapper* getBody1Wrap() const
	{
		return m_body1Wrap;
	}

	void setBody0Wrap(const CollisionObject2Wrapper* obj0Wrap)
	{
		m_body0Wrap = obj0Wrap;
	}

	void setBody1Wrap(const CollisionObject2Wrapper* obj1Wrap)
	{
		m_body1Wrap = obj1Wrap;
	}

	const CollisionObject2* getBody0Internal() const
	{
		return m_body0Wrap->getCollisionObject();
	}

	const CollisionObject2* getBody1Internal() const
	{
		return m_body1Wrap->getCollisionObject();
	}

	Scalar m_closestPointDistanceThreshold;

	/// in the future we can let the user override the methods to combine restitution and friction
	static Scalar calculateCombinedRestitution(const CollisionObject2* body0, const CollisionObject2* body1);
	static Scalar calculateCombinedFriction(const CollisionObject2* body0, const CollisionObject2* body1);
	static Scalar calculateCombinedRollingFriction(const CollisionObject2* body0, const CollisionObject2* body1);
	static Scalar calculateCombinedSpinningFriction(const CollisionObject2* body0, const CollisionObject2* body1);
	static Scalar calculateCombinedContactDamping(const CollisionObject2* body0, const CollisionObject2* body1);
	static Scalar calculateCombinedContactStiffness(const CollisionObject2* body0, const CollisionObject2* body1);
};

#endif  //DRX3D_MANIFOLD_RESULT_H
