#include <drx3D/SharedMemory/IKTrajectoryHelper.h>
#include <X/BussIK/Node.h>
#include <X/BussIK/Tree.h>
#include <X/BussIK/Jacobian.h>
#include <X/BussIK/VectorRn.h>
#include <X/BussIK/MatrixRmn.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h>

#define RADIAN(X) ((X)*RadiansToDegrees)

//use BussIK and Reflexxes to convert from Cartesian endeffector future target to
//joint space positions at each real-time (simulation) step
struct IKTrajectoryHelperInternalData
{
	VectorR3 m_endEffectorTargetPosition;
	VectorRn m_nullSpaceVelocity;
	VectorRn m_dampingCoeff;

	b3AlignedObjectArray<Node*> m_ikNodes;

	IKTrajectoryHelperInternalData()
	{
		m_endEffectorTargetPosition.SetZero();
		m_nullSpaceVelocity.SetZero();
		m_dampingCoeff.SetZero();
	}
};

IKTrajectoryHelper::IKTrajectoryHelper()
{
	m_data = new IKTrajectoryHelperInternalData;
}

IKTrajectoryHelper::~IKTrajectoryHelper()
{
	delete m_data;
}

bool IKTrajectoryHelper::computeIK(const double endEffectorTargetPosition[3],
								   const double endEffectorTargetOrientation[4],
								   const double endEffectorWorldPosition[3],
								   const double endEffectorWorldOrientation[4],
								   const double* q_current, i32 numQ, i32 endEffectorIndex,
								   double* q_new, i32 ikMethod, const double* linear_jacobian, const double* angular_jacobian, i32 jacobian_size, const double dampIk[6])
{
	MatrixRmn AugMat;
	bool useAngularPart = (ikMethod == IK2_VEL_DLS_WITH_ORIENTATION || ikMethod == IK2_VEL_DLS_WITH_ORIENTATION_NULLSPACE || ikMethod == IK2_VEL_SDLS_WITH_ORIENTATION) ? true : false;

	Jacobian ikJacobian(useAngularPart, numQ, 1);

	ikJacobian.Reset();

	bool UseJacobianTargets1 = false;

	if (UseJacobianTargets1)
	{
		ikJacobian.SetJtargetActive();
	}
	else
	{
		ikJacobian.SetJendActive();
	}
	VectorR3 targets;
	targets.Set(endEffectorTargetPosition[0], endEffectorTargetPosition[1], endEffectorTargetPosition[2]);
	ikJacobian.ComputeJacobian(&targets);  // Set up Jacobian and deltaS vectors

	// Set one end effector world position from drx3D
	VectorRn deltaS(3);
	for (i32 i = 0; i < 3; ++i)
	{
		deltaS.Set(i, dampIk[i] * (endEffectorTargetPosition[i] - endEffectorWorldPosition[i]));
	}

	// Set one end effector world orientation from drx3D
	VectorRn deltaR(3);
	if (useAngularPart)
	{
		Quat startQ(endEffectorWorldOrientation[0], endEffectorWorldOrientation[1], endEffectorWorldOrientation[2], endEffectorWorldOrientation[3]);
		Quat endQ(endEffectorTargetOrientation[0], endEffectorTargetOrientation[1], endEffectorTargetOrientation[2], endEffectorTargetOrientation[3]);
		Quat deltaQ = endQ * startQ.inverse();
		float angle = deltaQ.getAngle();
		Vec3 axis = deltaQ.getAxis();
		if (angle > PI)
		{
			angle -= 2.0 * PI;
		}
		else if (angle < -PI)
		{
			angle += 2.0 * PI;
		}
		float angleDot = angle;
		Vec3 angularVel = angleDot * axis.normalize();
		for (i32 i = 0; i < 3; ++i)
		{
			deltaR.Set(i, dampIk[i + 3] * angularVel[i]);
		}
	}

	{
		if (useAngularPart)
		{
			VectorRn deltaC(6);
			MatrixRmn completeJacobian(6, numQ);
			for (i32 i = 0; i < 3; ++i)
			{
				deltaC.Set(i, deltaS[i]);
				deltaC.Set(i + 3, deltaR[i]);
				for (i32 j = 0; j < numQ; ++j)
				{
					completeJacobian.Set(i, j, linear_jacobian[i * numQ + j]);
					completeJacobian.Set(i + 3, j, angular_jacobian[i * numQ + j]);
				}
			}
			ikJacobian.SetDeltaS(deltaC);
			ikJacobian.SetJendTrans(completeJacobian);
		}
		else
		{
			VectorRn deltaC(3);
			MatrixRmn completeJacobian(3, numQ);
			for (i32 i = 0; i < 3; ++i)
			{
				deltaC.Set(i, deltaS[i]);
				for (i32 j = 0; j < numQ; ++j)
				{
					completeJacobian.Set(i, j, linear_jacobian[i * numQ + j]);
				}
			}
			ikJacobian.SetDeltaS(deltaC);
			ikJacobian.SetJendTrans(completeJacobian);
		}
	}

	// Calculate the change in theta values
	switch (ikMethod)
	{
		case IK2_JACOB_TRANS:
			ikJacobian.CalcDeltaThetasTranspose();  // Jacobian transpose method
			break;
		case IK2_DLS:
		case IK2_VEL_DLS_WITH_ORIENTATION:
		case IK2_VEL_DLS:
			//ikJacobian.CalcDeltaThetasDLS();			// Damped least squares method
			assert(m_data->m_dampingCoeff.GetLength() == numQ);
			ikJacobian.CalcDeltaThetasDLS2(m_data->m_dampingCoeff, AugMat);
			break;
		case IK2_VEL_DLS_WITH_NULLSPACE:
		case IK2_VEL_DLS_WITH_ORIENTATION_NULLSPACE:
			assert(m_data->m_nullSpaceVelocity.GetLength() == numQ);
			ikJacobian.CalcDeltaThetasDLSwithNullspace(m_data->m_nullSpaceVelocity, AugMat);
			break;
		case IK2_DLS_SVD:
			ikJacobian.CalcDeltaThetasDLSwithSVD();
			break;
		case IK2_PURE_PSEUDO:
			ikJacobian.CalcDeltaThetasPseudoinverse();  // Pure pseudoinverse method
			break;
		case IK2_SDLS:
		case IK2_VEL_SDLS:
		case IK2_VEL_SDLS_WITH_ORIENTATION:
			ikJacobian.CalcDeltaThetasSDLS();  // Selectively damped least squares method
			break;
		default:
			ikJacobian.ZeroDeltaThetas();
			break;
	}

	// Use for velocity IK, update theta dot
	//ikJacobian.UpdateThetaDot();

	// Use for position IK, incrementally update theta
	//ikJacobian.UpdateThetas();

	// Apply the change in the theta values
	//ikJacobian.UpdatedSClampValue(&targets);

	for (i32 i = 0; i < numQ; i++)
	{
		// Use for velocity IK
		q_new[i] = ikJacobian.dTheta[i] + q_current[i];

		// Use for position IK
		//q_new[i] = m_data->m_ikNodes[i]->GetTheta();
	}
	return true;
}


bool IKTrajectoryHelper::computeIK2(
	const double* endEffectorTargetPositions,
	const double* endEffectorCurrentPositions,
	i32 numEndEffectors,
	const double* q_current, i32 numQ,
	double* q_new, i32 ikMethod, const double* linear_jacobians, const double dampIk[6])
{
	MatrixRmn AugMat;
	bool useAngularPart = false;//for now (ikMethod == IK2_VEL_DLS_WITH_ORIENTATION || ikMethod == IK2_VEL_DLS_WITH_ORIENTATION_NULLSPACE || ikMethod == IK2_VEL_SDLS_WITH_ORIENTATION) ? true : false;

	Jacobian ikJacobian(useAngularPart, numQ, numEndEffectors);

	ikJacobian.Reset();

	bool UseJacobianTargets1 = false;

	if (UseJacobianTargets1)
	{
		ikJacobian.SetJtargetActive();
	}
	else
	{
		ikJacobian.SetJendActive();
	}

	VectorRn deltaC(numEndEffectors *3);
	MatrixRmn completeJacobian(numEndEffectors*3, numQ);

	for (i32 ne = 0; ne < numEndEffectors; ne++)
	{
		VectorR3 targets;
		targets.Set(endEffectorTargetPositions[ne*3+0], endEffectorTargetPositions[ne * 3 + 1], endEffectorTargetPositions[ne * 3 + 2]);

		// Set one end effector world position from drx3D
		VectorRn deltaS(3);
		for (i32 i = 0; i < 3; ++i)
		{
			deltaS.Set(i, dampIk[i] * (endEffectorTargetPositions[ne*3+i] - endEffectorCurrentPositions[ne*3+i]));
		}
		{
			
			for (i32 i = 0; i < 3; ++i)
			{
				deltaC.Set(ne*3+i, deltaS[i]);
				for (i32 j = 0; j < numQ; ++j)
				{
					completeJacobian.Set(ne * 3 + i, j, linear_jacobians[((ne*3+i) * numQ) + j]);
				}
			}
		}
	}
	ikJacobian.SetDeltaS(deltaC);
	ikJacobian.SetJendTrans(completeJacobian);

	// Calculate the change in theta values
	switch (ikMethod)
	{
	case IK2_JACOB_TRANS:
		ikJacobian.CalcDeltaThetasTranspose();  // Jacobian transpose method
		break;
	case IK2_DLS:
	case IK2_VEL_DLS_WITH_ORIENTATION:
	case IK2_VEL_DLS:
		//ikJacobian.CalcDeltaThetasDLS();			// Damped least squares method
		assert(m_data->m_dampingCoeff.GetLength() == numQ);
		ikJacobian.CalcDeltaThetasDLS2(m_data->m_dampingCoeff, AugMat);
		break;
	case IK2_VEL_DLS_WITH_NULLSPACE:
	case IK2_VEL_DLS_WITH_ORIENTATION_NULLSPACE:
		assert(m_data->m_nullSpaceVelocity.GetLength() == numQ);
		ikJacobian.CalcDeltaThetasDLSwithNullspace(m_data->m_nullSpaceVelocity, AugMat);
		break;
	case IK2_DLS_SVD:
		ikJacobian.CalcDeltaThetasDLSwithSVD();
		break;
	case IK2_PURE_PSEUDO:
		ikJacobian.CalcDeltaThetasPseudoinverse();  // Pure pseudoinverse method
		break;
	case IK2_SDLS:
	case IK2_VEL_SDLS:
	case IK2_VEL_SDLS_WITH_ORIENTATION:
		ikJacobian.CalcDeltaThetasSDLS();  // Selectively damped least squares method
		break;
	default:
		ikJacobian.ZeroDeltaThetas();
		break;
	}

	// Use for velocity IK, update theta dot
	//ikJacobian.UpdateThetaDot();

	// Use for position IK, incrementally update theta
	//ikJacobian.UpdateThetas();

	// Apply the change in the theta values
	//ikJacobian.UpdatedSClampValue(&targets);

	for (i32 i = 0; i < numQ; i++)
	{
		// Use for velocity IK
		q_new[i] = ikJacobian.dTheta[i] + q_current[i];

		// Use for position IK
		//q_new[i] = m_data->m_ikNodes[i]->GetTheta();
	}
	return true;
}


bool IKTrajectoryHelper::computeNullspaceVel(i32 numQ, const double* q_current, const double* lower_limit, const double* upper_limit, const double* joint_range, const double* rest_pose)
{
	m_data->m_nullSpaceVelocity.SetLength(numQ);
	m_data->m_nullSpaceVelocity.SetZero();
	// TODO: Expose the coefficents of the null space term so that the user can choose to balance the null space task and the IK target task.
	// Can also adaptively adjust the coefficients based on the residual of the null space velocity in the IK target task space.
	double stayCloseToZeroGain = 0.001;
	double stayAwayFromLimitsGain = 10.0;

	// Stay close to zero
	for (i32 i = 0; i < numQ; ++i)
	{
		m_data->m_nullSpaceVelocity[i] = stayCloseToZeroGain * (rest_pose[i] - q_current[i]);
	}

	// Stay away from joint limits
	for (i32 i = 0; i < numQ; ++i)
	{
		if (q_current[i] > upper_limit[i])
		{
			m_data->m_nullSpaceVelocity[i] += stayAwayFromLimitsGain * (upper_limit[i] - q_current[i]) / joint_range[i];
		}
		if (q_current[i] < lower_limit[i])
		{
			m_data->m_nullSpaceVelocity[i] += stayAwayFromLimitsGain * (lower_limit[i] - q_current[i]) / joint_range[i];
		}
	}
	return true;
}

bool IKTrajectoryHelper::setDampingCoeff(i32 numQ, const double* coeff)
{
	m_data->m_dampingCoeff.SetLength(numQ);
	m_data->m_dampingCoeff.SetZero();
	for (i32 i = 0; i < numQ; ++i)
	{
		m_data->m_dampingCoeff[i] = coeff[i];
	}
	return true;
}
