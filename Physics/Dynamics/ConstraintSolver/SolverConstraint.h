#ifndef DRX3D_SOLVER_CONSTRAINT_H
#define DRX3D_SOLVER_CONSTRAINT_H

class RigidBody;
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Matrix3x3.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/JacobianEntry.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

//#define NO_FRICTION_TANGENTIALS 1
#include <drx3D/Physics/Dynamics/ConstraintSolver/SolverBody.h>

///1D constraint along a normal axis between bodyA and bodyB. It can be combined to solve contact and friction constraints.
ATTRIBUTE_ALIGNED16(struct)
SolverConstraint
{
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

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
		i32 m_numRowsForNonContactConstraint;
	};

	i32 m_overrideNumSolverIterations;
	i32 m_frictionIndex;
	i32 m_solverBodyIdA;
	i32 m_solverBodyIdB;

	enum SolverConstraintType
	{
		DRX3D_SOLVER_CONTACT_1D = 0,
		DRX3D_SOLVER_FRICTION_1D
	};
};

typedef AlignedObjectArray<SolverConstraint> ConstraintArray;

#endif  //DRX3D_SOLVER_CONSTRAINT_H
