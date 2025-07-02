#include <drx3D/Physics/Dynamics/MultiBody/MultiBodySliderConstraint.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofSpring2Constraint.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>

#define BTMBSLIDERCONSTRAINT_DIM 5
#define EPSILON 0.000001

MultiBodySliderConstraint::MultiBodySliderConstraint(MultiBody* body, i32 link, RigidBody* bodyB, const Vec3& pivotInA, const Vec3& pivotInB, const Matrix3x3& frameInA, const Matrix3x3& frameInB, const Vec3& jointAxis)
	: MultiBodyConstraint(body, 0, link, -1, BTMBSLIDERCONSTRAINT_DIM, false, MULTIBODY_CONSTRAINT_SLIDER),
	  m_rigidBodyA(0),
	  m_rigidBodyB(bodyB),
	  m_pivotInA(pivotInA),
	  m_pivotInB(pivotInB),
	  m_frameInA(frameInA),
	  m_frameInB(frameInB),
	  m_jointAxis(jointAxis)
{
	m_data.resize(BTMBSLIDERCONSTRAINT_DIM);  //at least store the applied impulses
}

MultiBodySliderConstraint::MultiBodySliderConstraint(MultiBody* bodyA, i32 linkA, MultiBody* bodyB, i32 linkB, const Vec3& pivotInA, const Vec3& pivotInB, const Matrix3x3& frameInA, const Matrix3x3& frameInB, const Vec3& jointAxis)
	: MultiBodyConstraint(bodyA, bodyB, linkA, linkB, BTMBSLIDERCONSTRAINT_DIM, false, MULTIBODY_CONSTRAINT_SLIDER),
	  m_rigidBodyA(0),
	  m_rigidBodyB(0),
	  m_pivotInA(pivotInA),
	  m_pivotInB(pivotInB),
	  m_frameInA(frameInA),
	  m_frameInB(frameInB),
	  m_jointAxis(jointAxis)
{
	m_data.resize(BTMBSLIDERCONSTRAINT_DIM);  //at least store the applied impulses
}

void MultiBodySliderConstraint::finalizeMultiDof()
{
	//not implemented yet
	Assert(0);
}

MultiBodySliderConstraint::~MultiBodySliderConstraint()
{
}

i32 MultiBodySliderConstraint::getIslandIdA() const
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

i32 MultiBodySliderConstraint::getIslandIdB() const
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
void MultiBodySliderConstraint::createConstraintRows(MultiBodyConstraintArray& constraintRows, MultiBodyJacobianData& data, const ContactSolverInfo& infoGlobal)
{
	// Convert local points back to world
	Vec3 pivotAworld = m_pivotInA;
	Matrix3x3 frameAworld = m_frameInA;
	Vec3 jointAxis = m_jointAxis;
	if (m_rigidBodyA)
	{
		pivotAworld = m_rigidBodyA->getCenterOfMassTransform() * m_pivotInA;
		frameAworld = m_frameInA.transpose() * Matrix3x3(m_rigidBodyA->getOrientation());
		jointAxis = quatRotate(m_rigidBodyA->getOrientation(), m_jointAxis);
	}
	else if (m_bodyA)
	{
		pivotAworld = m_bodyA->localPosToWorld(m_linkA, m_pivotInA);
		frameAworld = m_bodyA->localFrameToWorld(m_linkA, m_frameInA);
		jointAxis = m_bodyA->localDirToWorld(m_linkA, m_jointAxis);
	}
	Vec3 pivotBworld = m_pivotInB;
	Matrix3x3 frameBworld = m_frameInB;
	if (m_rigidBodyB)
	{
		pivotBworld = m_rigidBodyB->getCenterOfMassTransform() * m_pivotInB;
		frameBworld = m_frameInB.transpose() * Matrix3x3(m_rigidBodyB->getOrientation());
	}
	else if (m_bodyB)
	{
		pivotBworld = m_bodyB->localPosToWorld(m_linkB, m_pivotInB);
		frameBworld = m_bodyB->localFrameToWorld(m_linkB, m_frameInB);
	}

	Vec3 constraintAxis[2];
	for (i32 i = 0; i < 3; ++i)
	{
		constraintAxis[0] = frameAworld.getColumn(i).cross(jointAxis);
		if (constraintAxis[0].safeNorm() > EPSILON)
		{
			constraintAxis[0] = constraintAxis[0].normalized();
			constraintAxis[1] = jointAxis.cross(constraintAxis[0]);
			constraintAxis[1] = constraintAxis[1].normalized();
			break;
		}
	}

	Matrix3x3 relRot = frameAworld.inverse() * frameBworld;
	Vec3 angleDiff;
	Generic6DofSpring2Constraint::matrixToEulerXYZ(relRot, angleDiff);

	i32 numDim = BTMBSLIDERCONSTRAINT_DIM;
	for (i32 i = 0; i < numDim; i++)
	{
		MultiBodySolverConstraint& constraintRow = constraintRows.expandNonInitializing();
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

		if (m_rigidBodyA)
		{
			constraintRow.m_solverBodyIdA = m_rigidBodyA->getCompanionId();
		}
		if (m_rigidBodyB)
		{
			constraintRow.m_solverBodyIdB = m_rigidBodyB->getCompanionId();
		}

		Vec3 constraintNormalLin(0, 0, 0);
		Vec3 constraintNormalAng(0, 0, 0);
		Scalar posError = 0.0;
		if (i < 2)
		{
			constraintNormalLin = constraintAxis[i];
			posError = (pivotAworld - pivotBworld).dot(constraintNormalLin);
			fillMultiBodyConstraint(constraintRow, data, 0, 0, constraintNormalAng,
									constraintNormalLin, pivotAworld, pivotBworld,
									posError,
									infoGlobal,
									-m_maxAppliedImpulse, m_maxAppliedImpulse);
		}
		else
		{  //i>=2
			constraintNormalAng = frameAworld.getColumn(i % 3);
			posError = angleDiff[i % 3];
			fillMultiBodyConstraint(constraintRow, data, 0, 0, constraintNormalAng,
									constraintNormalLin, pivotAworld, pivotBworld,
									posError,
									infoGlobal,
									-m_maxAppliedImpulse, m_maxAppliedImpulse, true);
		}
	}
}

void MultiBodySliderConstraint::debugDraw(class IDebugDraw* drawer)
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
