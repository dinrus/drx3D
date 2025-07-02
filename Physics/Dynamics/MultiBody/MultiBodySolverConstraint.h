#ifndef DRX3D_MULTIBODY_SOLVER_CONSTRAINT_H
#define DRX3D_MULTIBODY_SOLVER_CONSTRAINT_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

class MultiBody;
class MultiBodyConstraint;
#include <drx3D/Physics/Dynamics/ConstraintSolver/SolverBody.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/ContactSolverInfo.h>

///1D constraint along a normal axis between bodyA and bodyB. It can be combined to solve contact and friction constraints.
ATTRIBUTE_ALIGNED16(struct)
MultiBodySolverConstraint
{
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	MultiBodySolverConstraint() : m_solverBodyIdA(-1), m_multiBodyA(0), m_linkA(-1), m_solverBodyIdB(-1), m_multiBodyB(0), m_linkB(-1), m_orgConstraint(0), m_orgDofIndex(-1)
	{
	}

	i32 m_deltaVelAindex;  //more generic version of m_relpos1CrossNormal/m_contactNormal1
	i32 m_jacAindex;
	i32 m_deltaVelBindex;
	i32 m_jacBindex;

	Vec3 m_relpos1CrossNormal;
	Vec3 m_contactNormal1;
	Vec3 m_relpos2CrossNormal;
	Vec3 m_contactNormal2;  //usually m_contactNormal2 == -m_contactNormal1, but not always

	Vec3 m_angularComponentA;
	Vec3 m_angularComponentB;

	mutable SimdScalar m_appliedPushImpulse;
	mutable SimdScalar m_appliedImpulse;

	Scalar m_friction;
	Scalar m_jacDiagABInv;
	Scalar m_rhs;
	Scalar m_cfm;

	Scalar m_lowerLimit;
	Scalar m_upperLimit;
	Scalar m_rhsPenetration;
	union {
		uk m_originalContactPoint;
		Scalar m_unusedPadding4;
	};

	i32 m_overrideNumSolverIterations;
	i32 m_frictionIndex;

	i32 m_solverBodyIdA;
	MultiBody* m_multiBodyA;
	i32 m_linkA;

	i32 m_solverBodyIdB;
	MultiBody* m_multiBodyB;
	i32 m_linkB;

	//for writing back applied impulses
	MultiBodyConstraint* m_orgConstraint;
	i32 m_orgDofIndex;

	enum SolverConstraintType
	{
		DRX3D_SOLVER_CONTACT_1D = 0,
		DRX3D_SOLVER_FRICTION_1D
	};
};

typedef AlignedObjectArray<MultiBodySolverConstraint> MultiBodyConstraintArray;

#endif  //DRX3D_MULTIBODY_SOLVER_CONSTRAINT_H
