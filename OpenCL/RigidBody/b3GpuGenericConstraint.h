#ifndef D3_GPU_GENERIC_CONSTRAINT_H
#define D3_GPU_GENERIC_CONSTRAINT_H

#include <drx3D/Common/b3Quat.h>
struct b3RigidBodyData;
enum D3_CONSTRAINT_FLAGS
{
	D3_CONSTRAINT_FLAG_ENABLED = 1,
};

enum b3GpuGenericConstraintType
{
	D3_GPU_POINT2POINT_CONSTRAINT_TYPE = 3,
	D3_GPU_FIXED_CONSTRAINT_TYPE = 4,
	//	D3_HINGE_CONSTRAINT_TYPE,
	//	D3_CONETWIST_CONSTRAINT_TYPE,
	//	D3_D6_CONSTRAINT_TYPE,
	//	D3_SLIDER_CONSTRAINT_TYPE,
	//	D3_CONTACT_CONSTRAINT_TYPE,
	//	D3_D6_SPRING_CONSTRAINT_TYPE,
	//	D3_GEAR_CONSTRAINT_TYPE,

	D3_GPU_MAX_CONSTRAINT_TYPE
};

struct b3GpuConstraintInfo2
{
	// integrator parameters: frames per second (1/stepsize), default error
	// reduction parameter (0..1).
	b3Scalar fps, erp;

	// for the first and second body, pointers to two (linear and angular)
	// n*3 jacobian sub matrices, stored by rows. these matrices will have
	// been initialized to 0 on entry. if the second body is zero then the
	// J2xx pointers may be 0.
	b3Scalar *m_J1linearAxis, *m_J1angularAxis, *m_J2linearAxis, *m_J2angularAxis;

	// elements to jump from one row to the next in J's
	i32 rowskip;

	// right hand sides of the equation J*v = c + cfm * lambda. cfm is the
	// "constraint force mixing" vector. c is set to zero on entry, cfm is
	// set to a constant value (typically very small or zero) value on entry.
	b3Scalar *m_constraintError, *cfm;

	// lo and hi limits for variables (set to -/+ infinity on entry).
	b3Scalar *m_lowerLimit, *m_upperLimit;

	// findex vector for variables. see the LCP solver interface for a
	// description of what this does. this is set to -1 on entry.
	// note that the returned indexes are relative to the first index of
	// the constraint.
	i32* findex;
	// number of solver iterations
	i32 m_numIterations;

	//damping of the velocity
	b3Scalar m_damping;
};

D3_ATTRIBUTE_ALIGNED16(struct)
b3GpuGenericConstraint
{
	i32 m_constraintType;
	i32 m_rbA;
	i32 m_rbB;
	float m_breakingImpulseThreshold;

	b3Vec3 m_pivotInA;
	b3Vec3 m_pivotInB;
	b3Quat m_relTargetAB;

	i32 m_flags;
	i32 m_uid;
	i32 m_padding[2];

	i32 getRigidBodyA() const
	{
		return m_rbA;
	}
	i32 getRigidBodyB() const
	{
		return m_rbB;
	}

	const b3Vec3& getPivotInA() const
	{
		return m_pivotInA;
	}

	const b3Vec3& getPivotInB() const
	{
		return m_pivotInB;
	}

	i32 isEnabled() const
	{
		return m_flags & D3_CONSTRAINT_FLAG_ENABLED;
	}

	float getBreakingImpulseThreshold() const
	{
		return m_breakingImpulseThreshold;
	}

	///internal method used by the constraint solver, don't use them directly
	void getInfo1(u32* info, const b3RigidBodyData* bodies);

	///internal method used by the constraint solver, don't use them directly
	void getInfo2(b3GpuConstraintInfo2 * info, const b3RigidBodyData* bodies);
};

#endif  //D3_GPU_GENERIC_CONSTRAINT_H