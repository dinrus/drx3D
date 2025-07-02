#ifndef DRX3D_POINT2POINTCONSTRAINT_H
#define DRX3D_POINT2POINTCONSTRAINT_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/JacobianEntry.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>

class RigidBody;

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define Point2PointConstraintData2 Point2PointConstraintDoubleData2
#define Point2PointConstraintDataName "Point2PointConstraintDoubleData2"
#else
#define Point2PointConstraintData2 Point2PointConstraintFloatData
#define Point2PointConstraintDataName "Point2PointConstraintFloatData"
#endif  //DRX3D_USE_DOUBLE_PRECISION

struct ConstraintSetting
{
	ConstraintSetting() : m_tau(Scalar(0.3)),
							m_damping(Scalar(1.)),
							m_impulseClamp(Scalar(0.))
	{
	}
	Scalar m_tau;
	Scalar m_damping;
	Scalar m_impulseClamp;
};

enum Point2PointFlags
{
	DRX3D_P2P_FLAGS_ERP = 1,
	DRX3D_P2P_FLAGS_CFM = 2
};

/// point to point constraint between two rigidbodies each with a pivotpoint that descibes the 'ballsocket' location in local space
ATTRIBUTE_ALIGNED16(class)
Point2PointConstraint : public TypedConstraint
{
#ifdef IN_PARALLELL_SOLVER
public:
#endif
	JacobianEntry m_jac[3];  //3 orthogonal linear constraints

	Vec3 m_pivotInA;
	Vec3 m_pivotInB;

	i32 m_flags;
	Scalar m_erp;
	Scalar m_cfm;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	///for backwards compatibility during the transition to 'getInfo/getInfo2'
	bool m_useSolveConstraintObsolete;

	ConstraintSetting m_setting;

	Point2PointConstraint(RigidBody & rbA, RigidBody & rbB, const Vec3& pivotInA, const Vec3& pivotInB);

	Point2PointConstraint(RigidBody & rbA, const Vec3& pivotInA);

	virtual void buildJacobian();

	virtual void getInfo1(ConstraintInfo1 * info);

	void getInfo1NonVirtual(ConstraintInfo1 * info);

	virtual void getInfo2(ConstraintInfo2 * info);

	void getInfo2NonVirtual(ConstraintInfo2 * info, const Transform2& body0_trans, const Transform2& body1_trans);

	void updateRHS(Scalar timeStep);

	void setPivotA(const Vec3& pivotA)
	{
		m_pivotInA = pivotA;
	}

	void setPivotB(const Vec3& pivotB)
	{
		m_pivotInB = pivotB;
	}

	const Vec3& getPivotInA() const
	{
		return m_pivotInA;
	}

	const Vec3& getPivotInB() const
	{
		return m_pivotInB;
	}

	///override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5).
	///If no axis is provided, it uses the default axis for this constraint.
	virtual void setParam(i32 num, Scalar value, i32 axis = -1);
	///return the local value of parameter
	virtual Scalar getParam(i32 num, i32 axis = -1) const;

	virtual i32 getFlags() const
	{
		return m_flags;
	}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct Point2PointConstraintFloatData
{
	TypedConstraintData m_typeConstraintData;
	Vec3FloatData m_pivotInA;
	Vec3FloatData m_pivotInB;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct Point2PointConstraintDoubleData2
{
	TypedConstraintDoubleData m_typeConstraintData;
	Vec3DoubleData m_pivotInA;
	Vec3DoubleData m_pivotInB;
};

#ifdef DRX3D_BACKWARDS_COMPATIBLE_SERIALIZATION
///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
///this structure is not used, except for loading pre-2.82 .bullet files
///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct Point2PointConstraintDoubleData
{
	TypedConstraintData m_typeConstraintData;
	Vec3DoubleData m_pivotInA;
	Vec3DoubleData m_pivotInB;
};
#endif  //DRX3D_BACKWARDS_COMPATIBLE_SERIALIZATION

SIMD_FORCE_INLINE i32 Point2PointConstraint::calculateSerializeBufferSize() const
{
	return sizeof(Point2PointConstraintData2);
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE tukk Point2PointConstraint::serialize(uk dataBuffer, Serializer* serializer) const
{
	Point2PointConstraintData2* p2pData = (Point2PointConstraintData2*)dataBuffer;

	TypedConstraint::serialize(&p2pData->m_typeConstraintData, serializer);
	m_pivotInA.serialize(p2pData->m_pivotInA);
	m_pivotInB.serialize(p2pData->m_pivotInB);

	return Point2PointConstraintDataName;
}

#endif  //DRX3D_POINT2POINTCONSTRAINT_H
