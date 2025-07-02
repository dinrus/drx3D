#ifndef D3_GPU_SOLVER_CONSTRAINT_H
#define D3_GPU_SOLVER_CONSTRAINT_H

#include <drx3D/Common/b3Vec3.h>
#include <drx3D/Common/b3Matrix3x3.h>
//#include "b3JacobianEntry.h>
#include <drx3D/Common/b3AlignedObjectArray.h>

//#define NO_FRICTION_TANGENTIALS 1

///1D constraint along a normal axis between bodyA and bodyB. It can be combined to solve contact and friction constraints.
D3_ATTRIBUTE_ALIGNED16(struct)
b3GpuSolverConstraint
{
	D3_DECLARE_ALIGNED_ALLOCATOR();

	b3Vec3 m_relpos1CrossNormal;
	b3Vec3 m_contactNormal;

	b3Vec3 m_relpos2CrossNormal;
	//b3Vec3		m_contactNormal2;//usually m_contactNormal2 == -m_contactNormal

	b3Vec3 m_angularComponentA;
	b3Vec3 m_angularComponentB;

	mutable b3Scalar m_appliedPushImpulse;
	mutable b3Scalar m_appliedImpulse;
	i32 m_padding1;
	i32 m_padding2;
	b3Scalar m_friction;
	b3Scalar m_jacDiagABInv;
	b3Scalar m_rhs;
	b3Scalar m_cfm;

	b3Scalar m_lowerLimit;
	b3Scalar m_upperLimit;
	b3Scalar m_rhsPenetration;
	union {
		uk m_originalContactPoint;
		i32 m_originalConstraintIndex;
		b3Scalar m_unusedPadding4;
	};

	i32 m_overrideNumSolverIterations;
	i32 m_frictionIndex;
	i32 m_solverBodyIdA;
	i32 m_solverBodyIdB;

	enum b3SolverConstraintType
	{
		D3_SOLVER_CONTACT_1D = 0,
		D3_SOLVER_FRICTION_1D
	};
};

typedef b3AlignedObjectArray<b3GpuSolverConstraint> b3GpuConstraintArray;

#endif  //D3_GPU_SOLVER_CONSTRAINT_H
