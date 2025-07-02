#ifndef DRX3D_CONTACT_CONSTRAINT_H
#define DRX3D_CONTACT_CONSTRAINT_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/JacobianEntry.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>
#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>

//ContactConstraint can be automatically created to solve contact constraints using the unified TypedConstraint interface
ATTRIBUTE_ALIGNED16(class)
ContactConstraint : public TypedConstraint
{
protected:
	PersistentManifold m_contactManifold;

protected:
	ContactConstraint(PersistentManifold * contactManifold, RigidBody & rbA, RigidBody & rbB);

public:
	void setContactManifold(PersistentManifold * contactManifold);

	PersistentManifold* getContactManifold()
	{
		return &m_contactManifold;
	}

	const PersistentManifold* getContactManifold() const
	{
		return &m_contactManifold;
	}

	virtual ~ContactConstraint();

	virtual void getInfo1(ConstraintInfo1 * info);

	virtual void getInfo2(ConstraintInfo2 * info);

	///obsolete methods
	virtual void buildJacobian();
};

///very basic collision resolution without friction
Scalar resolveSingleCollision(RigidBody* body1, class CollisionObject2* colObj2, const Vec3& contactPositionWorld, const Vec3& contactNormalOnB, const struct ContactSolverInfo& solverInfo, Scalar distance);

///resolveSingleBilateral is an obsolete methods used for vehicle friction between two dynamic objects
void resolveSingleBilateral(RigidBody& body1, const Vec3& pos1,
							RigidBody& body2, const Vec3& pos2,
							Scalar distance, const Vec3& normal, Scalar& impulse, Scalar timeStep);

#endif  //DRX3D_CONTACT_CONSTRAINT_H
