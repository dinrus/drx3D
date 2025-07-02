#include <drx3D/Importers/URDF/MyMultiBodyCreator.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyDynamicsWorld.h>
#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofSpring2Constraint.h>
#include <drx3D/Importers/URDF/URDFJointTypes.h>

MyMultiBodyCreator::MyMultiBodyCreator(GUIHelperInterface* guiHelper)
	: m_bulletMultiBody(0),
	  m_rigidBody(0),
	  m_guiHelper(guiHelper)
{
}

class MultiBody* MyMultiBodyCreator::allocateMultiBody(i32 /* urdfLinkIndex */, i32 totalNumJoints, Scalar mass, const Vec3& localInertiaDiagonal, bool isFixedBase, bool canSleep)
{
	//	m_urdf2mbLink.resize(totalNumJoints+1,-2);
	m_mb2urdfLink.resize(totalNumJoints + 1, -2);

	m_bulletMultiBody = new MultiBody(totalNumJoints, mass, localInertiaDiagonal, isFixedBase, canSleep);
	//if (canSleep)
	//	m_bulletMultiBody->goToSleep();
	return m_bulletMultiBody;
}

class RigidBody* MyMultiBodyCreator::allocateRigidBody(i32 urdfLinkIndex, Scalar mass, const Vec3& localInertiaDiagonal, const Transform2& initialWorldTrans, class CollisionShape* colShape)
{
	RigidBody::RigidBodyConstructionInfo rbci(mass, 0, colShape, localInertiaDiagonal);
	rbci.m_startWorldTransform = initialWorldTrans;
	Scalar sleep_threshold = Scalar(0.22360679775);//sqrt(0.05) to be similar to btMultiBody (SLEEP_THRESHOLD)
	rbci.m_angularSleepingThreshold = sleep_threshold;
	rbci.m_linearSleepingThreshold = sleep_threshold;
	
	RigidBody* body = new RigidBody(rbci);
	if (m_rigidBody == 0)
	{
		//only store the root of the multi body
		m_rigidBody = body;
	}
	return body;
}

class MultiBodyLinkCollider* MyMultiBodyCreator::allocateMultiBodyLinkCollider(i32 /*urdfLinkIndex*/, i32 mbLinkIndex, MultiBody* multiBody)
{
	MultiBodyLinkCollider* mbCol = new MultiBodyLinkCollider(multiBody, mbLinkIndex);
	return mbCol;
}

class Generic6DofSpring2Constraint* MyMultiBodyCreator::allocateGeneric6DofSpring2Constraint(i32 urdfLinkIndex, RigidBody& rbA /*parent*/, RigidBody& rbB, const Transform2& offsetInA, const Transform2& offsetInB, i32 rotateOrder)
{
	Generic6DofSpring2Constraint* c = new Generic6DofSpring2Constraint(rbA, rbB, offsetInA, offsetInB, (RotateOrder)rotateOrder);

	return c;
}

class Generic6DofSpring2Constraint* MyMultiBodyCreator::createPrismaticJoint(i32 urdfLinkIndex, RigidBody& rbA /*parent*/, RigidBody& rbB, const Transform2& offsetInA, const Transform2& offsetInB,
																			   const Vec3& jointAxisInJointSpace, Scalar jointLowerLimit, Scalar jointUpperLimit)
{
	i32 rotateOrder = 0;
	Generic6DofSpring2Constraint* dof6 = allocateGeneric6DofSpring2Constraint(urdfLinkIndex, rbA, rbB, offsetInA, offsetInB, rotateOrder);
	//todo(erwincoumans) for now, we only support principle axis along X, Y or Z
	i32 principleAxis = jointAxisInJointSpace.closestAxis();

	GenericConstraintUserInfo* userInfo = new GenericConstraintUserInfo;
	userInfo->m_jointAxisInJointSpace = jointAxisInJointSpace;
	userInfo->m_jointAxisIndex = principleAxis;

	userInfo->m_urdfJointType = URDFPrismaticJoint;
	userInfo->m_lowerJointLimit = jointLowerLimit;
	userInfo->m_upperJointLimit = jointUpperLimit;
	userInfo->m_urdfIndex = urdfLinkIndex;
	dof6->setUserConstraintPtr(userInfo);

	switch (principleAxis)
	{
		case 0:
		{
			dof6->setLinearLowerLimit(Vec3(jointLowerLimit, 0, 0));
			dof6->setLinearUpperLimit(Vec3(jointUpperLimit, 0, 0));
			break;
		}
		case 1:
		{
			dof6->setLinearLowerLimit(Vec3(0, jointLowerLimit, 0));
			dof6->setLinearUpperLimit(Vec3(0, jointUpperLimit, 0));
			break;
		}
		case 2:
		default:
		{
			dof6->setLinearLowerLimit(Vec3(0, 0, jointLowerLimit));
			dof6->setLinearUpperLimit(Vec3(0, 0, jointUpperLimit));
		}
	};

	dof6->setAngularLowerLimit(Vec3(0, 0, 0));
	dof6->setAngularUpperLimit(Vec3(0, 0, 0));
	m_6DofConstraints.push_back(dof6);
	return dof6;
}

class Generic6DofSpring2Constraint* MyMultiBodyCreator::createRevoluteJoint(i32 urdfLinkIndex, RigidBody& rbA /*parent*/, RigidBody& rbB, const Transform2& offsetInA, const Transform2& offsetInB,
																			  const Vec3& jointAxisInJointSpace, Scalar jointLowerLimit, Scalar jointUpperLimit)
{
	Generic6DofSpring2Constraint* dof6 = 0;

	//only handle principle axis at the moment,
	//@todo(erwincoumans) orient the constraint for non-principal axis
	i32 principleAxis = jointAxisInJointSpace.closestAxis();
	switch (principleAxis)
	{
		case 0:
		{
			dof6 = allocateGeneric6DofSpring2Constraint(urdfLinkIndex, rbA, rbB, offsetInA, offsetInB, RO_ZYX);
			dof6->setLinearLowerLimit(Vec3(0, 0, 0));
			dof6->setLinearUpperLimit(Vec3(0, 0, 0));

			dof6->setAngularLowerLimit(Vec3(jointLowerLimit, 0, 0));
			dof6->setAngularUpperLimit(Vec3(jointUpperLimit, 0, 0));

			break;
		}
		case 1:
		{
			dof6 = allocateGeneric6DofSpring2Constraint(urdfLinkIndex, rbA, rbB, offsetInA, offsetInB, RO_XZY);
			dof6->setLinearLowerLimit(Vec3(0, 0, 0));
			dof6->setLinearUpperLimit(Vec3(0, 0, 0));

			dof6->setAngularLowerLimit(Vec3(0, jointLowerLimit, 0));
			dof6->setAngularUpperLimit(Vec3(0, jointUpperLimit, 0));
			break;
		}
		case 2:
		default:
		{
			dof6 = allocateGeneric6DofSpring2Constraint(urdfLinkIndex, rbA, rbB, offsetInA, offsetInB, RO_XYZ);
			dof6->setLinearLowerLimit(Vec3(0, 0, 0));
			dof6->setLinearUpperLimit(Vec3(0, 0, 0));

			dof6->setAngularLowerLimit(Vec3(0, 0, jointLowerLimit));
			dof6->setAngularUpperLimit(Vec3(0, 0, jointUpperLimit));
		}
	};

	GenericConstraintUserInfo* userInfo = new GenericConstraintUserInfo;
	userInfo->m_jointAxisInJointSpace = jointAxisInJointSpace;
	userInfo->m_jointAxisIndex = 3 + principleAxis;

	if (jointLowerLimit > jointUpperLimit)
	{
		userInfo->m_urdfJointType = URDFContinuousJoint;
	}
	else
	{
		userInfo->m_urdfJointType = URDFRevoluteJoint;
		userInfo->m_lowerJointLimit = jointLowerLimit;
		userInfo->m_upperJointLimit = jointUpperLimit;
	}
	userInfo->m_urdfIndex = urdfLinkIndex;
	dof6->setUserConstraintPtr(userInfo);
	m_6DofConstraints.push_back(dof6);
	return dof6;
}

class Generic6DofSpring2Constraint* MyMultiBodyCreator::createFixedJoint(i32 urdfLinkIndex, RigidBody& rbA /*parent*/, RigidBody& rbB, const Transform2& offsetInA, const Transform2& offsetInB)
{
	Generic6DofSpring2Constraint* dof6 = allocateGeneric6DofSpring2Constraint(urdfLinkIndex, rbA, rbB, offsetInA, offsetInB);

	GenericConstraintUserInfo* userInfo = new GenericConstraintUserInfo;
	userInfo->m_urdfIndex = urdfLinkIndex;
	userInfo->m_urdfJointType = URDFFixedJoint;

	dof6->setUserConstraintPtr(userInfo);

	dof6->setLinearLowerLimit(Vec3(0, 0, 0));
	dof6->setLinearUpperLimit(Vec3(0, 0, 0));

	dof6->setAngularLowerLimit(Vec3(0, 0, 0));
	dof6->setAngularUpperLimit(Vec3(0, 0, 0));
	m_6DofConstraints.push_back(dof6);
	return dof6;
}

void MyMultiBodyCreator::addLinkMapping(i32 urdfLinkIndex, i32 mbLinkIndex)
{
	if (m_mb2urdfLink.size() < (mbLinkIndex + 1))
	{
		m_mb2urdfLink.resize((mbLinkIndex + 1), -2);
	}
	//    m_urdf2mbLink[urdfLinkIndex] = mbLinkIndex;
	m_mb2urdfLink[mbLinkIndex] = urdfLinkIndex;
}

void MyMultiBodyCreator::createRigidBodyGraphicsInstance(i32 linkIndex, RigidBody* body, const Vec3& colorRgba, i32 graphicsIndex)
{
	m_guiHelper->createRigidBodyGraphicsObject(body, colorRgba);
}

void MyMultiBodyCreator::createRigidBodyGraphicsInstance2(i32 linkIndex, class RigidBody* body, const Vec3& colorRgba, const Vec3& specularColor, i32 graphicsIndex)
{
	m_guiHelper->createRigidBodyGraphicsObject(body, colorRgba);
	i32 graphicsInstanceId = body->getUserIndex();
	Vec3DoubleData speculard;
	specularColor.serializeDouble(speculard);
	m_guiHelper->changeSpecularColor(graphicsInstanceId, speculard.m_floats);
}

void MyMultiBodyCreator::createCollisionObjectGraphicsInstance(i32 linkIndex, class CollisionObject2* colObj, const Vec3& colorRgba)
{
	m_guiHelper->createCollisionObjectGraphicsObject(colObj, colorRgba);
}

void MyMultiBodyCreator::createCollisionObjectGraphicsInstance2(i32 linkIndex, class CollisionObject2* col, const Vec4& colorRgba, const Vec3& specularColor)
{
	createCollisionObjectGraphicsInstance(linkIndex, col, colorRgba);
	i32 graphicsInstanceId = col->getUserIndex();
	Vec3DoubleData speculard;
	specularColor.serializeDouble(speculard);
	m_guiHelper->changeSpecularColor(graphicsInstanceId, speculard.m_floats);
}

MultiBody* MyMultiBodyCreator::getBulletMultiBody()
{
	return m_bulletMultiBody;
}
