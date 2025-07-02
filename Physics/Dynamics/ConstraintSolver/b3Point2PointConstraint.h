#ifndef D3_POINT2POINTCONSTRAINT_H
#define D3_POINT2POINTCONSTRAINT_H

#include <drx3D/Common/b3Vec3.h>
//#include <drx3D/Physics/Dynamics/ConstraintSolver/b3JacobianEntry.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/b3TypedConstraint.h>

class b3RigidBody;

#ifdef D3_USE_DOUBLE_PRECISION
#define b3Point2PointConstraintData b3Point2PointConstraintDoubleData
#define b3Point2PointConstraintDataName "b3Point2PointConstraintDoubleData"
#else
#define b3Point2PointConstraintData b3Point2PointConstraintFloatData
#define b3Point2PointConstraintDataName "b3Point2PointConstraintFloatData"
#endif  //D3_USE_DOUBLE_PRECISION

struct b3ConstraintSetting
{
	b3ConstraintSetting() : m_tau(b3Scalar(0.3)),
							m_damping(b3Scalar(1.)),
							m_impulseClamp(b3Scalar(0.))
	{
	}
	b3Scalar m_tau;
	b3Scalar m_damping;
	b3Scalar m_impulseClamp;
};

enum b3Point2PointFlags
{
	D3_P2P_FLAGS_ERP = 1,
	D3_P2P_FLAGS_CFM = 2
};

/// point to point constraint between two rigidbodies each with a pivotpoint that descibes the 'ballsocket' location in local space
D3_ATTRIBUTE_ALIGNED16(class)
b3Point2PointConstraint : public b3TypedConstraint
{
#ifdef IN_PARALLELL_SOLVER
public:
#endif

	b3Vec3 m_pivotInA;
	b3Vec3 m_pivotInB;

	i32 m_flags;
	b3Scalar m_erp;
	b3Scalar m_cfm;

public:
	D3_DECLARE_ALIGNED_ALLOCATOR();

	b3ConstraintSetting m_setting;

	b3Point2PointConstraint(i32 rbA, i32 rbB, const b3Vec3& pivotInA, const b3Vec3& pivotInB);

	//b3Point2PointConstraint(i32  rbA,const b3Vec3& pivotInA);

	virtual void getInfo1(b3ConstraintInfo1 * info, const b3RigidBodyData* bodies);

	void getInfo1NonVirtual(b3ConstraintInfo1 * info, const b3RigidBodyData* bodies);

	virtual void getInfo2(b3ConstraintInfo2 * info, const b3RigidBodyData* bodies);

	void getInfo2NonVirtual(b3ConstraintInfo2 * info, const b3Transform& body0_trans, const b3Transform& body1_trans);

	void updateRHS(b3Scalar timeStep);

	void setPivotA(const b3Vec3& pivotA)
	{
		m_pivotInA = pivotA;
	}

	void setPivotB(const b3Vec3& pivotB)
	{
		m_pivotInB = pivotB;
	}

	const b3Vec3& getPivotInA() const
	{
		return m_pivotInA;
	}

	const b3Vec3& getPivotInB() const
	{
		return m_pivotInB;
	}

	///override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5).
	///If no axis is provided, it uses the default axis for this constraint.
	virtual void setParam(i32 num, b3Scalar value, i32 axis = -1);
	///return the local value of parameter
	virtual b3Scalar getParam(i32 num, i32 axis = -1) const;

	//	virtual	i32	calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	//	virtual	tukk 	serialize(uk dataBuffer, b3Serializer* serializer) const;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct b3Point2PointConstraintFloatData
{
	b3TypedConstraintData m_typeConstraintData;
	b3Vec3FloatData m_pivotInA;
	b3Vec3FloatData m_pivotInB;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct b3Point2PointConstraintDoubleData
{
	b3TypedConstraintData m_typeConstraintData;
	b3Vec3DoubleData m_pivotInA;
	b3Vec3DoubleData m_pivotInB;
};

/*
D3_FORCE_INLINE	i32	b3Point2PointConstraint::calculateSerializeBufferSize() const
{
	return sizeof(b3Point2PointConstraintData);

}

	///fills the dataBuffer and returns the struct name (and 0 on failure)
D3_FORCE_INLINE	tukk 	b3Point2PointConstraint::serialize(uk dataBuffer, b3Serializer* serializer) const
{
	b3Point2PointConstraintData* p2pData = (b3Point2PointConstraintData*)dataBuffer;

	b3TypedConstraint::serialize(&p2pData->m_typeConstraintData,serializer);
	m_pivotInA.serialize(p2pData->m_pivotInA);
	m_pivotInB.serialize(p2pData->m_pivotInB);

	return b3Point2PointConstraintDataName;
}
*/

#endif  //D3_POINT2POINTCONSTRAINT_H
