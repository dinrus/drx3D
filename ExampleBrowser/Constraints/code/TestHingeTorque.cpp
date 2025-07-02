#include "../TestHingeTorque.h"
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>

i32 collisionFilterGroup = i32(BroadphaseProxy::CharacterFilter);
i32 collisionFilterMask = i32(BroadphaseProxy::AllFilter ^ (BroadphaseProxy::CharacterFilter));
static Scalar radius(0.2);

struct TestHingeTorque : public CommonRigidBodyBase
{
	bool m_once;
	AlignedObjectArray<JointFeedback*> m_jointFeedback;

	TestHingeTorque(struct GUIHelperInterface* helper);
	virtual ~TestHingeTorque();
	virtual void initPhysics();

	virtual void stepSimulation(float deltaTime);

	virtual void resetCamera()
	{
		float dist = 5;
		float pitch = -21;
		float yaw = 270;
		float targetPos[3] = {-1.34, 3.4, -0.44};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

TestHingeTorque::TestHingeTorque(struct GUIHelperInterface* helper)
	: CommonRigidBodyBase(helper),
	  m_once(true)
{
}
TestHingeTorque::~TestHingeTorque()
{
	for (i32 i = 0; i < m_jointFeedback.size(); i++)
	{
		delete m_jointFeedback[i];
	}
}

void TestHingeTorque::stepSimulation(float deltaTime)
{
	if (0)  //m_once)
	{
		m_once = false;
		HingeConstraint* hinge = (HingeConstraint*)m_dynamicsWorld->getConstraint(0);

		RigidBody& bodyA = hinge->getRigidBodyA();
		Transform2 trA = bodyA.getWorldTransform();
		Vec3 hingeAxisInWorld = trA.getBasis() * hinge->getFrameOffsetA().getBasis().getColumn(2);
		hinge->getRigidBodyA().applyTorque(-hingeAxisInWorld * 10);
		hinge->getRigidBodyB().applyTorque(hingeAxisInWorld * 10);
	}

	m_dynamicsWorld->stepSimulation(1. / 240, 0);

	static i32 count = 0;
	if ((count & 0x0f) == 0)
	{
		RigidBody* base = RigidBody::upcast(m_dynamicsWorld->getCollisionObjectArray()[0]);

		drx3DPrintf("base angvel = %f,%f,%f", base->getAngularVelocity()[0],
				 base->getAngularVelocity()[1],

				 base->getAngularVelocity()[2]);

		RigidBody* child = RigidBody::upcast(m_dynamicsWorld->getCollisionObjectArray()[1]);

		drx3DPrintf("child angvel = %f,%f,%f", child->getAngularVelocity()[0],
				 child->getAngularVelocity()[1],

				 child->getAngularVelocity()[2]);

		for (i32 i = 0; i < m_jointFeedback.size(); i++)
		{
			drx3DPrintf("Applied force at the COM/Inertial frame B[%d]:(%f,%f,%f), torque B:(%f,%f,%f)\n", i,

					 m_jointFeedback[i]->m_appliedForceBodyB.x(),
					 m_jointFeedback[i]->m_appliedForceBodyB.y(),
					 m_jointFeedback[i]->m_appliedForceBodyB.z(),
					 m_jointFeedback[i]->m_appliedTorqueBodyB.x(),
					 m_jointFeedback[i]->m_appliedTorqueBodyB.y(),
					 m_jointFeedback[i]->m_appliedTorqueBodyB.z());
		}
	}
	count++;

	//CommonRigidBodyBase::stepSimulation(deltaTime);
}

void TestHingeTorque::initPhysics()
{
	i32 upAxis = 1;
	m_guiHelper->setUpAxis(upAxis);

	createEmptyDynamicsWorld();
	m_dynamicsWorld->getSolverInfo().m_splitImpulse = false;

	m_dynamicsWorld->setGravity(Vec3(0, 0, -10));

	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	i32 mode = IDebugDraw::DBG_DrawWireframe + IDebugDraw::DBG_DrawConstraints + IDebugDraw::DBG_DrawConstraintLimits;
	m_dynamicsWorld->getDebugDrawer()->setDebugMode(mode);

	{  // create a door using hinge constraint attached to the world

		i32 numLinks = 2;
		//    bool selfCollide = false;
		Vec3 linkHalfExtents(0.05, 0.37, 0.1);
		Vec3 baseHalfExtents(0.05, 0.37, 0.1);

		BoxShape* baseBox = new BoxShape(baseHalfExtents);
		Vec3 basePosition = Vec3(-0.4f, 3.f, 0.f);
		Transform2 baseWorldTrans;
		baseWorldTrans.setIdentity();
		baseWorldTrans.setOrigin(basePosition);

		//mbC->forceMultiDof();							//if !spherical, you can comment this line to check the 1DoF algorithm
		//init the base
		Vec3 baseInertiaDiag(0.f, 0.f, 0.f);
		float baseMass = 0.f;
		float linkMass = 1.f;

		RigidBody* base = createRigidBody(baseMass, baseWorldTrans, baseBox);
		m_dynamicsWorld->removeRigidBody(base);
		base->setDamping(0, 0);
		m_dynamicsWorld->addRigidBody(base, collisionFilterGroup, collisionFilterMask);
		BoxShape* linkBox1 = new BoxShape(linkHalfExtents);
		SphereShape* linkSphere = new SphereShape(radius);

		RigidBody* prevBody = base;

		for (i32 i = 0; i < numLinks; i++)
		{
			Transform2 linkTrans;
			linkTrans = baseWorldTrans;

			linkTrans.setOrigin(basePosition - Vec3(0, linkHalfExtents[1] * 2.f * (i + 1), 0));

			CollisionShape* colOb = 0;

			if (i == 0)
			{
				colOb = linkBox1;
			}
			else
			{
				colOb = linkSphere;
			}
			RigidBody* linkBody = createRigidBody(linkMass, linkTrans, colOb);
			m_dynamicsWorld->removeRigidBody(linkBody);
			m_dynamicsWorld->addRigidBody(linkBody, collisionFilterGroup, collisionFilterMask);
			linkBody->setDamping(0, 0);
			TypedConstraint* con = 0;

			if (i == 0)
			{
				//create a hinge constraint
				Vec3 pivotInA(0, -linkHalfExtents[1], 0);
				Vec3 pivotInB(0, linkHalfExtents[1], 0);
				Vec3 axisInA(1, 0, 0);
				Vec3 axisInB(1, 0, 0);
				bool useReferenceA = true;
				HingeConstraint* hinge = new HingeConstraint(*prevBody, *linkBody,
																 pivotInA, pivotInB,
																 axisInA, axisInB, useReferenceA);
				con = hinge;
			}
			else
			{
				Transform2 pivotInA(Quat::getIdentity(), Vec3(0, -radius, 0));  //par body's COM to cur body's COM offset
				Transform2 pivotInB(Quat::getIdentity(), Vec3(0, radius, 0));   //cur body's COM to cur body's PIV offset
				Generic6DofSpring2Constraint* fixed = new Generic6DofSpring2Constraint(*prevBody, *linkBody,
																						   pivotInA, pivotInB);
				fixed->setLinearLowerLimit(Vec3(0, 0, 0));
				fixed->setLinearUpperLimit(Vec3(0, 0, 0));
				fixed->setAngularLowerLimit(Vec3(0, 0, 0));
				fixed->setAngularUpperLimit(Vec3(0, 0, 0));

				con = fixed;
			}
			Assert(con);
			if (con)
			{
				JointFeedback* fb = new JointFeedback();
				m_jointFeedback.push_back(fb);
				con->setJointFeedback(fb);

				m_dynamicsWorld->addConstraint(con, true);
			}
			prevBody = linkBody;
		}
	}

	if (1)
	{
		Vec3 groundHalfExtents(1, 1, 0.2);
		groundHalfExtents[upAxis] = 1.f;
		BoxShape* box = new BoxShape(groundHalfExtents);
		box->initializePolyhedralFeatures();

		Transform2 start;
		start.setIdentity();
		Vec3 groundOrigin(-0.4f, 3.f, 0.f);
		//	Vec3 basePosition = Vec3(-0.4f, 3.f, 0.f);
		Quat groundOrn(Vec3(0, 1, 0), 0.25 * SIMD_PI);

		groundOrigin[upAxis] -= .5;
		groundOrigin[2] -= 0.6;
		start.setOrigin(groundOrigin);
		//	start.setRotation(groundOrn);
		RigidBody* body = createRigidBody(0, start, box);
		body->setFriction(0);
	}
	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

class CommonExampleInterface* TestHingeTorqueCreateFunc(CommonExampleOptions& options)
{
	return new TestHingeTorque(options.m_guiHelper);
}
