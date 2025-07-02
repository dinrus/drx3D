#ifndef DRX3D_MULTIBODY_CONSTRAINT_H
#define DRX3D_MULTIBODY_CONSTRAINT_H

#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h>


//Don't change any of the existing enum values, so add enum types at the end for serialization compatibility
enum TypedMultiBodyConstraintType
{
	MULTIBODY_CONSTRAINT_LIMIT=3,
	MULTIBODY_CONSTRAINT_1DOF_JOINT_MOTOR,
	MULTIBODY_CONSTRAINT_GEAR,
	MULTIBODY_CONSTRAINT_POINT_TO_POINT,
	MULTIBODY_CONSTRAINT_SLIDER,
	MULTIBODY_CONSTRAINT_SPHERICAL_MOTOR,
	MULTIBODY_CONSTRAINT_FIXED,
	MULTIBODY_CONSTRAINT_SPHERICAL_LIMIT,
	MAX_MULTIBODY_CONSTRAINT_TYPE,
};

class MultiBody;
struct SolverInfo;

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodySolverConstraint.h>

struct MultiBodyJacobianData
{
	AlignedObjectArray<Scalar> m_jacobians;
	AlignedObjectArray<Scalar> m_deltaVelocitiesUnitImpulse;  //holds the joint-space response of the corresp. tree to the test impulse in each constraint space dimension
	AlignedObjectArray<Scalar> m_deltaVelocities;             //holds joint-space vectors of all the constrained trees accumulating the effect of corrective impulses applied in SI
	AlignedObjectArray<Scalar> scratch_r;
	AlignedObjectArray<Vec3> scratch_v;
	AlignedObjectArray<Matrix3x3> scratch_m;
	AlignedObjectArray<SolverBody>* m_solverBodyPool;
	i32 m_fixedBodyId;
};

ATTRIBUTE_ALIGNED16(class)
MultiBodyConstraint
{
protected:
	MultiBody* m_bodyA;
	MultiBody* m_bodyB;
	i32 m_linkA;
	i32 m_linkB;

	i32 m_type; //TypedMultiBodyConstraintType

	i32 m_numRows;
	i32 m_jacSizeA;
	i32 m_jacSizeBoth;
	i32 m_posOffset;

	bool m_isUnilateral;
	i32 m_numDofsFinalized;
	Scalar m_maxAppliedImpulse;

	// warning: the data block lay out is not consistent for all constraints
	// data block laid out as follows:
	// cached impulses. (one per row.)
	// jacobians. (interleaved, row1 body1 then row1 body2 then row2 body 1 etc)
	// positions. (one per row.)
	AlignedObjectArray<Scalar> m_data;

	void applyDeltaVee(MultiBodyJacobianData & data, Scalar * delta_vee, Scalar impulse, i32 velocityIndex, i32 ndof);

	Scalar fillMultiBodyConstraint(MultiBodySolverConstraint & solverConstraint,
									 MultiBodyJacobianData & data,
									 Scalar * jacOrgA, Scalar * jacOrgB,
									 const Vec3& constraintNormalAng,

									 const Vec3& constraintNormalLin,
									 const Vec3& posAworld, const Vec3& posBworld,
									 Scalar posError,
									 const ContactSolverInfo& infoGlobal,
									 Scalar lowerLimit, Scalar upperLimit,
									 bool angConstraint = false,

									 Scalar relaxation = 1.f,
									 bool isFriction = false, Scalar desiredVelocity = 0, Scalar cfmSlip = 0, Scalar damping = 1.0);

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	MultiBodyConstraint(MultiBody * bodyA, MultiBody * bodyB, i32 linkA, i32 linkB, i32 numRows, bool isUnilateral, i32 type);
	virtual ~MultiBodyConstraint();

	void updateJacobianSizes();
	void allocateJacobiansMultiDof();

	i32 getConstraintType() const
	{
		return m_type;
	}
	//many constraints have setFrameInB/setPivotInB. Will use 'getConstraintType' later.
	virtual void setFrameInB(const Matrix3x3& frameInB) {}
	virtual void setPivotInB(const Vec3& pivotInB) {}

	virtual void finalizeMultiDof() = 0;

	virtual i32 getIslandIdA() const = 0;
	virtual i32 getIslandIdB() const = 0;

	virtual void createConstraintRows(MultiBodyConstraintArray & constraintRows,
									  MultiBodyJacobianData & data,
									  const ContactSolverInfo& infoGlobal) = 0;

	i32 getNumRows() const
	{
		return m_numRows;
	}

	MultiBody* getMultiBodyA()
	{
		return m_bodyA;
	}
	MultiBody* getMultiBodyB()
	{
		return m_bodyB;
	}

	i32 getLinkA() const
	{
		return m_linkA;
	}
	i32 getLinkB() const
	{
		return m_linkB;
	}
	void internalSetAppliedImpulse(i32 dof, Scalar appliedImpulse)
	{
		Assert(dof >= 0);
		Assert(dof < getNumRows());
		m_data[dof] = appliedImpulse;
	}

	Scalar getAppliedImpulse(i32 dof)
	{
		Assert(dof >= 0);
		Assert(dof < getNumRows());
		return m_data[dof];
	}
	// current constraint position
	// constraint is pos >= 0 for unilateral, or pos = 0 for bilateral
	// NOTE: ignored position for friction rows.
	Scalar getPosition(i32 row) const
	{
		return m_data[m_posOffset + row];
	}

	void setPosition(i32 row, Scalar pos)
	{
		m_data[m_posOffset + row] = pos;
	}

	bool isUnilateral() const
	{
		return m_isUnilateral;
	}

	// jacobian blocks.
	// each of size 6 + num_links. (jacobian2 is null if no body2.)
	// format: 3 'omega' coefficients, 3 'v' coefficients, then the 'qdot' coefficients.
	Scalar* jacobianA(i32 row)
	{
		return &m_data[m_numRows + row * m_jacSizeBoth];
	}
	const Scalar* jacobianA(i32 row) const
	{
		return &m_data[m_numRows + (row * m_jacSizeBoth)];
	}
	Scalar* jacobianB(i32 row)
	{
		return &m_data[m_numRows + (row * m_jacSizeBoth) + m_jacSizeA];
	}
	const Scalar* jacobianB(i32 row) const
	{
		return &m_data[m_numRows + (row * m_jacSizeBoth) + m_jacSizeA];
	}

	Scalar getMaxAppliedImpulse() const
	{
		return m_maxAppliedImpulse;
	}
	void setMaxAppliedImpulse(Scalar maxImp)
	{
		m_maxAppliedImpulse = maxImp;
	}

	virtual void debugDraw(class IDebugDraw * drawer) = 0;

	virtual void setGearRatio(Scalar ratio) {}
	virtual void setGearAuxLink(i32 gearAuxLink) {}
	virtual void setRelativePositionTarget(Scalar relPosTarget) {}
	virtual void setErp(Scalar erp) {}
};

#endif  //DRX3D_MULTIBODY_CONSTRAINT_H
