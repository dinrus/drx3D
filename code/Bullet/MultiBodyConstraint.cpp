#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyPoint2Point.h>
 //for testing (BTMBP2PCONSTRAINT_BLOCK_ANGULAR_MOTION_TEST macro)

MultiBodyConstraint::MultiBodyConstraint(MultiBody* bodyA, MultiBody* bodyB, i32 linkA, i32 linkB, i32 numRows, bool isUnilateral, i32 type)
	: m_bodyA(bodyA),
	  m_bodyB(bodyB),
	  m_linkA(linkA),
	  m_linkB(linkB),
	  m_type(type),
	  m_numRows(numRows),
	  m_jacSizeA(0),
	  m_jacSizeBoth(0),
	  m_isUnilateral(isUnilateral),
	  m_numDofsFinalized(-1),
	  m_maxAppliedImpulse(100)
{
}

void MultiBodyConstraint::updateJacobianSizes()
{
	if (m_bodyA)
	{
		m_jacSizeA = (6 + m_bodyA->getNumDofs());
	}

	if (m_bodyB)
	{
		m_jacSizeBoth = m_jacSizeA + 6 + m_bodyB->getNumDofs();
	}
	else
		m_jacSizeBoth = m_jacSizeA;
}

void MultiBodyConstraint::allocateJacobiansMultiDof()
{
	updateJacobianSizes();

	m_posOffset = ((1 + m_jacSizeBoth) * m_numRows);
	m_data.resize((2 + m_jacSizeBoth) * m_numRows);
}

MultiBodyConstraint::~MultiBodyConstraint()
{
}

void MultiBodyConstraint::applyDeltaVee(MultiBodyJacobianData& data, Scalar* delta_vee, Scalar impulse, i32 velocityIndex, i32 ndof)
{
	for (i32 i = 0; i < ndof; ++i)
		data.m_deltaVelocities[velocityIndex + i] += delta_vee[i] * impulse;
}

Scalar MultiBodyConstraint::fillMultiBodyConstraint(MultiBodySolverConstraint& solverConstraint,
														MultiBodyJacobianData& data,
														Scalar* jacOrgA, Scalar* jacOrgB,
														const Vec3& constraintNormalAng,
														const Vec3& constraintNormalLin,
														const Vec3& posAworld, const Vec3& posBworld,
														Scalar posError,
														const ContactSolverInfo& infoGlobal,
														Scalar lowerLimit, Scalar upperLimit,
														bool angConstraint,
														Scalar relaxation,
														bool isFriction, Scalar desiredVelocity, Scalar cfmSlip,
														Scalar damping)
{
	solverConstraint.m_multiBodyA = m_bodyA;
	solverConstraint.m_multiBodyB = m_bodyB;
	solverConstraint.m_linkA = m_linkA;
	solverConstraint.m_linkB = m_linkB;

	MultiBody* multiBodyA = solverConstraint.m_multiBodyA;
	MultiBody* multiBodyB = solverConstraint.m_multiBodyB;

	SolverBody* bodyA = multiBodyA ? 0 : &data.m_solverBodyPool->at(solverConstraint.m_solverBodyIdA);
	SolverBody* bodyB = multiBodyB ? 0 : &data.m_solverBodyPool->at(solverConstraint.m_solverBodyIdB);

	RigidBody* rb0 = multiBodyA ? 0 : bodyA->m_originalBody;
	RigidBody* rb1 = multiBodyB ? 0 : bodyB->m_originalBody;

	Vec3 rel_pos1, rel_pos2;  //these two used to be inited to posAworld and posBworld (respectively) but it does not seem necessary
	if (bodyA)
		rel_pos1 = posAworld - bodyA->getWorldTransform().getOrigin();
	if (bodyB)
		rel_pos2 = posBworld - bodyB->getWorldTransform().getOrigin();

	if (multiBodyA)
	{
		if (solverConstraint.m_linkA < 0)
		{
			rel_pos1 = posAworld - multiBodyA->getBasePos();
		}
		else
		{
			rel_pos1 = posAworld - multiBodyA->getLink(solverConstraint.m_linkA).m_cachedWorldTransform.getOrigin();
		}

		i32k ndofA = multiBodyA->getNumDofs() + 6;

		solverConstraint.m_deltaVelAindex = multiBodyA->getCompanionId();

		if (solverConstraint.m_deltaVelAindex < 0)
		{
			solverConstraint.m_deltaVelAindex = data.m_deltaVelocities.size();
			multiBodyA->setCompanionId(solverConstraint.m_deltaVelAindex);
			data.m_deltaVelocities.resize(data.m_deltaVelocities.size() + ndofA);
		}
		else
		{
			Assert(data.m_deltaVelocities.size() >= solverConstraint.m_deltaVelAindex + ndofA);
		}

		//determine jacobian of this 1D constraint in terms of multibodyA's degrees of freedom
		//resize..
		solverConstraint.m_jacAindex = data.m_jacobians.size();
		data.m_jacobians.resize(data.m_jacobians.size() + ndofA);
		//copy/determine
		if (jacOrgA)
		{
			for (i32 i = 0; i < ndofA; i++)
				data.m_jacobians[solverConstraint.m_jacAindex + i] = jacOrgA[i];
		}
		else
		{
			Scalar* jac1 = &data.m_jacobians[solverConstraint.m_jacAindex];
			//multiBodyA->fillContactJacobianMultiDof(solverConstraint.m_linkA, posAworld, constraintNormalLin, jac1, data.scratch_r, data.scratch_v, data.scratch_m);
			multiBodyA->fillConstraintJacobianMultiDof(solverConstraint.m_linkA, posAworld, constraintNormalAng, constraintNormalLin, jac1, data.scratch_r, data.scratch_v, data.scratch_m);
		}

		//determine the velocity response of multibodyA to reaction impulses of this constraint (i.e. A[i,i] for i=1,...n_con: multibody's inverse inertia with respect to this 1D constraint)
		//resize..
		data.m_deltaVelocitiesUnitImpulse.resize(data.m_deltaVelocitiesUnitImpulse.size() + ndofA);  //=> each constraint row has the constrained tree dofs allocated in m_deltaVelocitiesUnitImpulse
		Assert(data.m_jacobians.size() == data.m_deltaVelocitiesUnitImpulse.size());
		Scalar* delta = &data.m_deltaVelocitiesUnitImpulse[solverConstraint.m_jacAindex];
		//determine..
		multiBodyA->calcAccelerationDeltasMultiDof(&data.m_jacobians[solverConstraint.m_jacAindex], delta, data.scratch_r, data.scratch_v);

		Vec3 torqueAxis0;
		if (angConstraint)
		{
			torqueAxis0 = constraintNormalAng;
		}
		else
		{
			torqueAxis0 = rel_pos1.cross(constraintNormalLin);
		}
		solverConstraint.m_relpos1CrossNormal = torqueAxis0;
		solverConstraint.m_contactNormal1 = constraintNormalLin;
	}
	else  //if(rb0)
	{
		Vec3 torqueAxis0;
		if (angConstraint)
		{
			torqueAxis0 = constraintNormalAng;
		}
		else
		{
			torqueAxis0 = rel_pos1.cross(constraintNormalLin);
		}
		solverConstraint.m_angularComponentA = rb0 ? rb0->getInvInertiaTensorWorld() * torqueAxis0 * rb0->getAngularFactor() : Vec3(0, 0, 0);
		solverConstraint.m_relpos1CrossNormal = torqueAxis0;
		solverConstraint.m_contactNormal1 = constraintNormalLin;
	}

	if (multiBodyB)
	{
		if (solverConstraint.m_linkB < 0)
		{
			rel_pos2 = posBworld - multiBodyB->getBasePos();
		}
		else
		{
			rel_pos2 = posBworld - multiBodyB->getLink(solverConstraint.m_linkB).m_cachedWorldTransform.getOrigin();
		}

		i32k ndofB = multiBodyB->getNumDofs() + 6;

		solverConstraint.m_deltaVelBindex = multiBodyB->getCompanionId();
		if (solverConstraint.m_deltaVelBindex < 0)
		{
			solverConstraint.m_deltaVelBindex = data.m_deltaVelocities.size();
			multiBodyB->setCompanionId(solverConstraint.m_deltaVelBindex);
			data.m_deltaVelocities.resize(data.m_deltaVelocities.size() + ndofB);
		}

		//determine jacobian of this 1D constraint in terms of multibodyB's degrees of freedom
		//resize..
		solverConstraint.m_jacBindex = data.m_jacobians.size();
		data.m_jacobians.resize(data.m_jacobians.size() + ndofB);
		//copy/determine..
		if (jacOrgB)
		{
			for (i32 i = 0; i < ndofB; i++)
				data.m_jacobians[solverConstraint.m_jacBindex + i] = jacOrgB[i];
		}
		else
		{
			//multiBodyB->fillContactJacobianMultiDof(solverConstraint.m_linkB, posBworld, -constraintNormalLin, &data.m_jacobians[solverConstraint.m_jacBindex], data.scratch_r, data.scratch_v, data.scratch_m);
			multiBodyB->fillConstraintJacobianMultiDof(solverConstraint.m_linkB, posBworld, -constraintNormalAng, -constraintNormalLin, &data.m_jacobians[solverConstraint.m_jacBindex], data.scratch_r, data.scratch_v, data.scratch_m);
		}

		//determine velocity response of multibodyB to reaction impulses of this constraint (i.e. A[i,i] for i=1,...n_con: multibody's inverse inertia with respect to this 1D constraint)
		//resize..
		data.m_deltaVelocitiesUnitImpulse.resize(data.m_deltaVelocitiesUnitImpulse.size() + ndofB);
		Assert(data.m_jacobians.size() == data.m_deltaVelocitiesUnitImpulse.size());
		Scalar* delta = &data.m_deltaVelocitiesUnitImpulse[solverConstraint.m_jacBindex];
		//determine..
		multiBodyB->calcAccelerationDeltasMultiDof(&data.m_jacobians[solverConstraint.m_jacBindex], delta, data.scratch_r, data.scratch_v);

		Vec3 torqueAxis1;
		if (angConstraint)
		{
			torqueAxis1 = constraintNormalAng;
		}
		else
		{
			torqueAxis1 = rel_pos2.cross(constraintNormalLin);
		}
		solverConstraint.m_relpos2CrossNormal = -torqueAxis1;
		solverConstraint.m_contactNormal2 = -constraintNormalLin;
	}
	else  //if(rb1)
	{
		Vec3 torqueAxis1;
		if (angConstraint)
		{
			torqueAxis1 = constraintNormalAng;
		}
		else
		{
			torqueAxis1 = rel_pos2.cross(constraintNormalLin);
		}
		solverConstraint.m_angularComponentB = rb1 ? rb1->getInvInertiaTensorWorld() * -torqueAxis1 * rb1->getAngularFactor() : Vec3(0, 0, 0);
		solverConstraint.m_relpos2CrossNormal = -torqueAxis1;
		solverConstraint.m_contactNormal2 = -constraintNormalLin;
	}
	{
		Vec3 vec;
		Scalar denom0 = 0.f;
		Scalar denom1 = 0.f;
		Scalar* jacB = 0;
		Scalar* jacA = 0;
		Scalar* deltaVelA = 0;
		Scalar* deltaVelB = 0;
		i32 ndofA = 0;
		//determine the "effective mass" of the constrained multibodyA with respect to this 1D constraint (i.e. 1/A[i,i])
		if (multiBodyA)
		{
			ndofA = multiBodyA->getNumDofs() + 6;
			jacA = &data.m_jacobians[solverConstraint.m_jacAindex];
			deltaVelA = &data.m_deltaVelocitiesUnitImpulse[solverConstraint.m_jacAindex];
			for (i32 i = 0; i < ndofA; ++i)
			{
				Scalar j = jacA[i];
				Scalar l = deltaVelA[i];
				denom0 += j * l;
			}
		}
		else if (rb0)
		{
			vec = (solverConstraint.m_angularComponentA).cross(rel_pos1);
			if (angConstraint)
			{
				denom0 = constraintNormalAng.dot(solverConstraint.m_angularComponentA);
			}
			else
			{
				denom0 = rb0->getInvMass() + constraintNormalLin.dot(vec);
			}
		}
		//
		if (multiBodyB)
		{
			i32k ndofB = multiBodyB->getNumDofs() + 6;
			jacB = &data.m_jacobians[solverConstraint.m_jacBindex];
			deltaVelB = &data.m_deltaVelocitiesUnitImpulse[solverConstraint.m_jacBindex];
			for (i32 i = 0; i < ndofB; ++i)
			{
				Scalar j = jacB[i];
				Scalar l = deltaVelB[i];
				denom1 += j * l;
			}
		}
		else if (rb1)
		{
			vec = (-solverConstraint.m_angularComponentB).cross(rel_pos2);
			if (angConstraint)
			{
				denom1 = constraintNormalAng.dot(-solverConstraint.m_angularComponentB);
			}
			else
			{
				denom1 = rb1->getInvMass() + constraintNormalLin.dot(vec);
			}
		}

		//
		Scalar d = denom0 + denom1;
		if (d > SIMD_EPSILON)
		{
			solverConstraint.m_jacDiagABInv = relaxation / (d);
		}
		else
		{
			//disable the constraint row to handle singularity/redundant constraint
			solverConstraint.m_jacDiagABInv = 0.f;
		}
	}

	//compute rhs and remaining solverConstraint fields
	Scalar penetration = isFriction ? 0 : posError;

	Scalar rel_vel = 0.f;
	i32 ndofA = 0;
	i32 ndofB = 0;
	{
		Vec3 vel1, vel2;
		if (multiBodyA)
		{
			ndofA = multiBodyA->getNumDofs() + 6;
			Scalar* jacA = &data.m_jacobians[solverConstraint.m_jacAindex];
			for (i32 i = 0; i < ndofA; ++i)
				rel_vel += multiBodyA->getVelocityVector()[i] * jacA[i];
		}
		else if (rb0)
		{
			rel_vel += rb0->getLinearVelocity().dot(solverConstraint.m_contactNormal1);
			rel_vel += rb0->getAngularVelocity().dot(solverConstraint.m_relpos1CrossNormal);
		}
		if (multiBodyB)
		{
			ndofB = multiBodyB->getNumDofs() + 6;
			Scalar* jacB = &data.m_jacobians[solverConstraint.m_jacBindex];
			for (i32 i = 0; i < ndofB; ++i)
				rel_vel += multiBodyB->getVelocityVector()[i] * jacB[i];
		}
		else if (rb1)
		{
			rel_vel += rb1->getLinearVelocity().dot(solverConstraint.m_contactNormal2);
			rel_vel += rb1->getAngularVelocity().dot(solverConstraint.m_relpos2CrossNormal);
		}

		solverConstraint.m_friction = 0.f;  //cp.m_combinedFriction;
	}

	solverConstraint.m_appliedImpulse = 0.f;
	solverConstraint.m_appliedPushImpulse = 0.f;

	{
		Scalar positionalError = 0.f;
		Scalar velocityError = (desiredVelocity - rel_vel) * damping;

		Scalar erp = infoGlobal.m_erp2;

		//split impulse is not implemented yet for MultiBody*
		//if (!infoGlobal.m_splitImpulse || (penetration > infoGlobal.m_splitImpulsePenetrationThreshold))
		{
			erp = infoGlobal.m_erp;
		}

		positionalError = -penetration * erp / infoGlobal.m_timeStep;

		Scalar penetrationImpulse = positionalError * solverConstraint.m_jacDiagABInv;
		Scalar velocityImpulse = velocityError * solverConstraint.m_jacDiagABInv;

		//split impulse is not implemented yet for MultiBody*

		//  if (!infoGlobal.m_splitImpulse || (penetration > infoGlobal.m_splitImpulsePenetrationThreshold))
		{
			//combine position and velocity into rhs
			solverConstraint.m_rhs = penetrationImpulse + velocityImpulse;
			solverConstraint.m_rhsPenetration = 0.f;
		}
		/*else
        {
            //split position and velocity into rhs and m_rhsPenetration
            solverConstraint.m_rhs = velocityImpulse;
            solverConstraint.m_rhsPenetration = penetrationImpulse;
        }
        */

		solverConstraint.m_cfm = 0.f;
		solverConstraint.m_lowerLimit = lowerLimit;
		solverConstraint.m_upperLimit = upperLimit;
	}

	return rel_vel;
}
