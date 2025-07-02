#include <drx3D/Physics/Dynamics/ConstraintSolver/Point2PointConstraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <new>

Point2PointConstraint::Point2PointConstraint(RigidBody& rbA, RigidBody& rbB, const Vec3& pivotInA, const Vec3& pivotInB)
	: TypedConstraint(POINT2POINT_CONSTRAINT_TYPE, rbA, rbB), m_pivotInA(pivotInA), m_pivotInB(pivotInB), m_flags(0), m_useSolveConstraintObsolete(false)
{
}

Point2PointConstraint::Point2PointConstraint(RigidBody& rbA, const Vec3& pivotInA)
	: TypedConstraint(POINT2POINT_CONSTRAINT_TYPE, rbA), m_pivotInA(pivotInA), m_pivotInB(rbA.getCenterOfMassTransform()(pivotInA)), m_flags(0), m_useSolveConstraintObsolete(false)
{
}

void Point2PointConstraint::buildJacobian()
{
	///we need it for both methods
	{
		m_appliedImpulse = Scalar(0.);

		Vec3 normal(0, 0, 0);

		for (i32 i = 0; i < 3; i++)
		{
			normal[i] = 1;
			new (&m_jac[i]) JacobianEntry(
				m_rbA.getCenterOfMassTransform().getBasis().transpose(),
				m_rbB.getCenterOfMassTransform().getBasis().transpose(),
				m_rbA.getCenterOfMassTransform() * m_pivotInA - m_rbA.getCenterOfMassPosition(),
				m_rbB.getCenterOfMassTransform() * m_pivotInB - m_rbB.getCenterOfMassPosition(),
				normal,
				m_rbA.getInvInertiaDiagLocal(),
				m_rbA.getInvMass(),
				m_rbB.getInvInertiaDiagLocal(),
				m_rbB.getInvMass());
			normal[i] = 0;
		}
	}
}

void Point2PointConstraint::getInfo1(ConstraintInfo1* info)
{
	getInfo1NonVirtual(info);
}

void Point2PointConstraint::getInfo1NonVirtual(ConstraintInfo1* info)
{
	if (m_useSolveConstraintObsolete)
	{
		info->m_numConstraintRows = 0;
		info->nub = 0;
	}
	else
	{
		info->m_numConstraintRows = 3;
		info->nub = 3;
	}
}

void Point2PointConstraint::getInfo2(ConstraintInfo2* info)
{
	getInfo2NonVirtual(info, m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform());
}

void Point2PointConstraint::getInfo2NonVirtual(ConstraintInfo2* info, const Transform2& body0_trans, const Transform2& body1_trans)
{
	Assert(!m_useSolveConstraintObsolete);

	//retrieve matrices

	// anchor points in global coordinates with respect to body PORs.

	// set jacobian
	info->m_J1linearAxis[0] = 1;
	info->m_J1linearAxis[info->rowskip + 1] = 1;
	info->m_J1linearAxis[2 * info->rowskip + 2] = 1;

	Vec3 a1 = body0_trans.getBasis() * getPivotInA();
	{
		Vec3* angular0 = (Vec3*)(info->m_J1angularAxis);
		Vec3* angular1 = (Vec3*)(info->m_J1angularAxis + info->rowskip);
		Vec3* angular2 = (Vec3*)(info->m_J1angularAxis + 2 * info->rowskip);
		Vec3 a1neg = -a1;
		a1neg.getSkewSymmetricMatrix(angular0, angular1, angular2);
	}

	info->m_J2linearAxis[0] = -1;
	info->m_J2linearAxis[info->rowskip + 1] = -1;
	info->m_J2linearAxis[2 * info->rowskip + 2] = -1;

	Vec3 a2 = body1_trans.getBasis() * getPivotInB();

	{
		//	Vec3 a2n = -a2;
		Vec3* angular0 = (Vec3*)(info->m_J2angularAxis);
		Vec3* angular1 = (Vec3*)(info->m_J2angularAxis + info->rowskip);
		Vec3* angular2 = (Vec3*)(info->m_J2angularAxis + 2 * info->rowskip);
		a2.getSkewSymmetricMatrix(angular0, angular1, angular2);
	}

	// set right hand side
	Scalar currERP = (m_flags & DRX3D_P2P_FLAGS_ERP) ? m_erp : info->erp;
	Scalar k = info->fps * currERP;
	i32 j;
	for (j = 0; j < 3; j++)
	{
		info->m_constraintError[j * info->rowskip] = k * (a2[j] + body1_trans.getOrigin()[j] - a1[j] - body0_trans.getOrigin()[j]);
		//printf("info->m_constraintError[%d]=%f\n",j,info->m_constraintError[j]);
	}
	if (m_flags & DRX3D_P2P_FLAGS_CFM)
	{
		for (j = 0; j < 3; j++)
		{
			info->cfm[j * info->rowskip] = m_cfm;
		}
	}

	Scalar impulseClamp = m_setting.m_impulseClamp;  //
	for (j = 0; j < 3; j++)
	{
		if (m_setting.m_impulseClamp > 0)
		{
			info->m_lowerLimit[j * info->rowskip] = -impulseClamp;
			info->m_upperLimit[j * info->rowskip] = impulseClamp;
		}
	}
	info->m_damping = m_setting.m_damping;
}

void Point2PointConstraint::updateRHS(Scalar timeStep)
{
	(void)timeStep;
}

///override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5).
///If no axis is provided, it uses the default axis for this constraint.
void Point2PointConstraint::setParam(i32 num, Scalar value, i32 axis)
{
	if (axis != -1)
	{
		AssertConstrParams(0);
	}
	else
	{
		switch (num)
		{
			case DRX3D_CONSTRAINT_ERP:
			case DRX3D_CONSTRAINT_STOP_ERP:
				m_erp = value;
				m_flags |= DRX3D_P2P_FLAGS_ERP;
				break;
			case DRX3D_CONSTRAINT_CFM:
			case DRX3D_CONSTRAINT_STOP_CFM:
				m_cfm = value;
				m_flags |= DRX3D_P2P_FLAGS_CFM;
				break;
			default:
				AssertConstrParams(0);
		}
	}
}

///return the local value of parameter
Scalar Point2PointConstraint::getParam(i32 num, i32 axis) const
{
	Scalar retVal(SIMD_INFINITY);
	if (axis != -1)
	{
		AssertConstrParams(0);
	}
	else
	{
		switch (num)
		{
			case DRX3D_CONSTRAINT_ERP:
			case DRX3D_CONSTRAINT_STOP_ERP:
				AssertConstrParams(m_flags & DRX3D_P2P_FLAGS_ERP);
				retVal = m_erp;
				break;
			case DRX3D_CONSTRAINT_CFM:
			case DRX3D_CONSTRAINT_STOP_CFM:
				AssertConstrParams(m_flags & DRX3D_P2P_FLAGS_CFM);
				retVal = m_cfm;
				break;
			default:
				AssertConstrParams(0);
		}
	}
	return retVal;
}
