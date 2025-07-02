#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyPoint2Point.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>

#ifndef BTMBP2PCONSTRAINT_BLOCK_ANGULAR_MOTION_TEST
#define BTMBP2PCONSTRAINT_DIM 3
#else
#define BTMBP2PCONSTRAINT_DIM 6
#endif

MultiBodyPoint2Point::MultiBodyPoint2Point(MultiBody* body, i32 link, RigidBody* bodyB, const Vec3& pivotInA, const Vec3& pivotInB)
	: MultiBodyConstraint(body, 0, link, -1, BTMBP2PCONSTRAINT_DIM, false, MULTIBODY_CONSTRAINT_POINT_TO_POINT),
	  m_rigidBodyA(0),
	  m_rigidBodyB(bodyB),
	  m_pivotInA(pivotInA),
	  m_pivotInB(pivotInB)
{
	m_data.resize(BTMBP2PCONSTRAINT_DIM);  //at least store the applied impulses
}

MultiBodyPoint2Point::MultiBodyPoint2Point(MultiBody* bodyA, i32 linkA, MultiBody* bodyB, i32 linkB, const Vec3& pivotInA, const Vec3& pivotInB)
	: MultiBodyConstraint(bodyA, bodyB, linkA, linkB, BTMBP2PCONSTRAINT_DIM, false, MULTIBODY_CONSTRAINT_POINT_TO_POINT),
	  m_rigidBodyA(0),
	  m_rigidBodyB(0),
	  m_pivotInA(pivotInA),
	  m_pivotInB(pivotInB)
{
	m_data.resize(BTMBP2PCONSTRAINT_DIM);  //at least store the applied impulses
}

void MultiBodyPoint2Point::finalizeMultiDof()
{
	//not implemented yet
	Assert(0);
}

MultiBodyPoint2Point::~MultiBodyPoint2Point()
{
}

i32 MultiBodyPoint2Point::getIslandIdA() const
{
	if (m_rigidBodyA)
		return m_rigidBodyA->getIslandTag();

	if (m_bodyA)
	{
		if (m_linkA < 0)
		{
			MultiBodyLinkCollider* col = m_bodyA->getBaseCollider();
			if (col)
				return col->getIslandTag();
		}
		else
		{
			if (m_bodyA->getLink(m_linkA).m_collider)
				return m_bodyA->getLink(m_linkA).m_collider->getIslandTag();
		}
	}
	return -1;
}

i32 MultiBodyPoint2Point::getIslandIdB() const
{
	if (m_rigidBodyB)
		return m_rigidBodyB->getIslandTag();
	if (m_bodyB)
	{
		if (m_linkB < 0)
		{
			MultiBodyLinkCollider* col = m_bodyB->getBaseCollider();
			if (col)
				return col->getIslandTag();
		}
		else
		{
			if (m_bodyB->getLink(m_linkB).m_collider)
				return m_bodyB->getLink(m_linkB).m_collider->getIslandTag();
		}
	}
	return -1;
}

void MultiBodyPoint2Point::createConstraintRows(MultiBodyConstraintArray& constraintRows,
												  MultiBodyJacobianData& data,
												  const ContactSolverInfo& infoGlobal)
{
	//	i32 i=1;
	i32 numDim = BTMBP2PCONSTRAINT_DIM;
	for (i32 i = 0; i < numDim; i++)
	{
		MultiBodySolverConstraint& constraintRow = constraintRows.expandNonInitializing();
		//memset(&constraintRow,0xffffffff,sizeof(MultiBodySolverConstraint));
		constraintRow.m_orgConstraint = this;
		constraintRow.m_orgDofIndex = i;
		constraintRow.m_relpos1CrossNormal.setVal(0, 0, 0);
		constraintRow.m_contactNormal1.setVal(0, 0, 0);
		constraintRow.m_relpos2CrossNormal.setVal(0, 0, 0);
		constraintRow.m_contactNormal2.setVal(0, 0, 0);
		constraintRow.m_angularComponentA.setVal(0, 0, 0);
		constraintRow.m_angularComponentB.setVal(0, 0, 0);

		constraintRow.m_solverBodyIdA = data.m_fixedBodyId;
		constraintRow.m_solverBodyIdB = data.m_fixedBodyId;

		Vec3 contactNormalOnB(0, 0, 0);
#ifndef BTMBP2PCONSTRAINT_BLOCK_ANGULAR_MOTION_TEST
		contactNormalOnB[i] = -1;
#else
		contactNormalOnB[i % 3] = -1;
#endif

		// Convert local points back to world
		Vec3 pivotAworld = m_pivotInA;
		if (m_rigidBodyA)
		{
			constraintRow.m_solverBodyIdA = m_rigidBodyA->getCompanionId();
			pivotAworld = m_rigidBodyA->getCenterOfMassTransform() * m_pivotInA;
		}
		else
		{
			if (m_bodyA)
				pivotAworld = m_bodyA->localPosToWorld(m_linkA, m_pivotInA);
		}
		Vec3 pivotBworld = m_pivotInB;
		if (m_rigidBodyB)
		{
			constraintRow.m_solverBodyIdB = m_rigidBodyB->getCompanionId();
			pivotBworld = m_rigidBodyB->getCenterOfMassTransform() * m_pivotInB;
		}
		else
		{
			if (m_bodyB)
				pivotBworld = m_bodyB->localPosToWorld(m_linkB, m_pivotInB);
		}

		Scalar posError = i < 3 ? (pivotAworld - pivotBworld).dot(contactNormalOnB) : 0;

#ifndef BTMBP2PCONSTRAINT_BLOCK_ANGULAR_MOTION_TEST

		fillMultiBodyConstraint(constraintRow, data, 0, 0, Vec3(0, 0, 0),
								contactNormalOnB, pivotAworld, pivotBworld,  //sucks but let it be this way "for the time being"
								posError,
								infoGlobal,
								-m_maxAppliedImpulse, m_maxAppliedImpulse);
		//@todo: support the case of MultiBody versus RigidBody,
		//see Point2PointConstraint::getInfo2NonVirtual
#else
		const Vec3 dummy(0, 0, 0);

		Assert(m_bodyA->isMultiDof());

		Scalar* jac1 = jacobianA(i);
		const Vec3& normalAng = i >= 3 ? contactNormalOnB : dummy;
		const Vec3& normalLin = i < 3 ? contactNormalOnB : dummy;

		m_bodyA->filConstraintJacobianMultiDof(m_linkA, pivotAworld, normalAng, normalLin, jac1, data.scratch_r, data.scratch_v, data.scratch_m);

		fillMultiBodyConstraint(constraintRow, data, jac1, 0,
								dummy, dummy, dummy,  //sucks but let it be this way "for the time being"
								posError,
								infoGlobal,
								-m_maxAppliedImpulse, m_maxAppliedImpulse);
#endif
	}
}

void MultiBodyPoint2Point::debugDraw(class IDebugDraw* drawer)
{
	Transform2 tr;
	tr.setIdentity();

	if (m_rigidBodyA)
	{
		Vec3 pivot = m_rigidBodyA->getCenterOfMassTransform() * m_pivotInA;
		tr.setOrigin(pivot);
		drawer->drawTransform2(tr, 0.1);
	}
	if (m_bodyA)
	{
		Vec3 pivotAworld = m_bodyA->localPosToWorld(m_linkA, m_pivotInA);
		tr.setOrigin(pivotAworld);
		drawer->drawTransform2(tr, 0.1);
	}
	if (m_rigidBodyB)
	{
		// that ideally should draw the same frame
		Vec3 pivot = m_rigidBodyB->getCenterOfMassTransform() * m_pivotInB;
		tr.setOrigin(pivot);
		drawer->drawTransform2(tr, 0.1);
	}
	if (m_bodyB)
	{
		Vec3 pivotBworld = m_bodyB->localPosToWorld(m_linkB, m_pivotInB);
		tr.setOrigin(pivotBworld);
		drawer->drawTransform2(tr, 0.1);
	}
}
