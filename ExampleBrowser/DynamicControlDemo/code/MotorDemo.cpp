
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>
#include "../MotorDemo.h"
#include <cmath>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

class BroadphaseInterface;
class CollisionShape;
class OverlappingPairCache;
class CollisionDispatcher;
class ConstraintSolver;
struct CollisionAlgorithmCreateFunc;
class DefaultCollisionConfiguration;

#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

class MotorDemo : public CommonRigidBodyBase
{
	float m_Time;
	float m_fCyclePeriod;  // in milliseconds
	float m_fMuscleStrength;

	AlignedObjectArray<class TestRig*> m_rigs;

public:
	MotorDemo(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
	}

	void initPhysics();

	void exitPhysics();

	virtual ~MotorDemo()
	{
	}

	void spawnTestRig(const Vec3& startOffset, bool bFixed);

	//	virtual void keyboardCallback(u8 key, i32 x, i32 y);

	void setMotorTargets(Scalar deltaTime);

	void resetCamera()
	{
		float dist = 11;
		float pitch = -35;
		float yaw = 52;
		float targetPos[3] = {0, 0.46, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

#ifndef M_PI_4
#define M_PI_4 0.785398163397448309616
#endif

#ifndef M_PI_8
#define M_PI_8 0.5 * M_PI_4
#endif

// /LOCAL FUNCTIONS

#define NUM_LEGS 6
#define BODYPART_COUNT 2 * NUM_LEGS + 1
#define JOINT_COUNT BODYPART_COUNT - 1

class TestRig
{
	DynamicsWorld* m_ownerWorld;
	CollisionShape* m_shapes[BODYPART_COUNT];
	RigidBody* m_bodies[BODYPART_COUNT];
	TypedConstraint* m_joints[JOINT_COUNT];

	RigidBody* localCreateRigidBody(Scalar mass, const Transform2& startTransform, CollisionShape* shape)
	{
		bool isDynamic = (mass != 0.f);

		Vec3 localInertia(0, 0, 0);
		if (isDynamic)
			shape->calculateLocalInertia(mass, localInertia);

		DefaultMotionState* myMotionState = new DefaultMotionState(startTransform);
		RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
		RigidBody* body = new RigidBody(rbInfo);

		m_ownerWorld->addRigidBody(body);

		return body;
	}

public:
	TestRig(DynamicsWorld* ownerWorld, const Vec3& positionOffset, bool bFixed)
		: m_ownerWorld(ownerWorld)
	{
		Vec3 vUp(0, 1, 0);

		//
		// Setup geometry
		//
		float fBodySize = 0.25f;
		float fLegLength = 0.45f;
		float fForeLegLength = 0.75f;
		m_shapes[0] = new CapsuleShape(Scalar(fBodySize), Scalar(0.10));
		i32 i;
		for (i = 0; i < NUM_LEGS; i++)
		{
			m_shapes[1 + 2 * i] = new CapsuleShape(Scalar(0.10), Scalar(fLegLength));
			m_shapes[2 + 2 * i] = new CapsuleShape(Scalar(0.08), Scalar(fForeLegLength));
		}

		//
		// Setup rigid bodies
		//
		float fHeight = 0.5;
		Transform2 offset;
		offset.setIdentity();
		offset.setOrigin(positionOffset);

		// root
		Vec3 vRoot = Vec3(Scalar(0.), Scalar(fHeight), Scalar(0.));
		Transform2 transform;
		transform.setIdentity();
		transform.setOrigin(vRoot);
		if (bFixed)
		{
			m_bodies[0] = localCreateRigidBody(Scalar(0.), offset * transform, m_shapes[0]);
		}
		else
		{
			m_bodies[0] = localCreateRigidBody(Scalar(1.), offset * transform, m_shapes[0]);
		}
		// legs
		for (i = 0; i < NUM_LEGS; i++)
		{
			float fAngle = 2 * M_PI * i / NUM_LEGS;
			float fSin = std::sin(fAngle);
			float fCos = std::cos(fAngle);

			transform.setIdentity();
			Vec3 vBoneOrigin = Vec3(Scalar(fCos * (fBodySize + 0.5 * fLegLength)), Scalar(fHeight), Scalar(fSin * (fBodySize + 0.5 * fLegLength)));
			transform.setOrigin(vBoneOrigin);

			// thigh
			Vec3 vToBone = (vBoneOrigin - vRoot).normalize();
			Vec3 vAxis = vToBone.cross(vUp);
			transform.setRotation(Quat(vAxis, M_PI_2));
			m_bodies[1 + 2 * i] = localCreateRigidBody(Scalar(1.), offset * transform, m_shapes[1 + 2 * i]);

			// shin
			transform.setIdentity();
			transform.setOrigin(Vec3(Scalar(fCos * (fBodySize + fLegLength)), Scalar(fHeight - 0.5 * fForeLegLength), Scalar(fSin * (fBodySize + fLegLength))));
			m_bodies[2 + 2 * i] = localCreateRigidBody(Scalar(1.), offset * transform, m_shapes[2 + 2 * i]);
		}

		// Setup some damping on the m_bodies
		for (i = 0; i < BODYPART_COUNT; ++i)
		{
			m_bodies[i]->setDamping(0.05, 0.85);
			m_bodies[i]->setDeactivationTime(0.8);
			//m_bodies[i]->setSleepingThresholds(1.6, 2.5);
			m_bodies[i]->setSleepingThresholds(0.5f, 0.5f);
		}

		//
		// Setup the constraints
		//
		HingeConstraint* hingeC;
		//ConeTwistConstraint* coneC;

		Transform2 localA, localB, localC;

		for (i = 0; i < NUM_LEGS; i++)
		{
			float fAngle = 2 * M_PI * i / NUM_LEGS;
			float fSin = std::sin(fAngle);
			float fCos = std::cos(fAngle);

			// hip joints
			localA.setIdentity();
			localB.setIdentity();
			localA.getBasis().setEulerZYX(0, -fAngle, 0);
			localA.setOrigin(Vec3(Scalar(fCos * fBodySize), Scalar(0.), Scalar(fSin * fBodySize)));
			localB = m_bodies[1 + 2 * i]->getWorldTransform().inverse() * m_bodies[0]->getWorldTransform() * localA;
			hingeC = new HingeConstraint(*m_bodies[0], *m_bodies[1 + 2 * i], localA, localB);
			hingeC->setLimit(Scalar(-0.75 * M_PI_4), Scalar(M_PI_8));
			//hingeC->setLimit(Scalar(-0.1), Scalar(0.1));
			m_joints[2 * i] = hingeC;
			m_ownerWorld->addConstraint(m_joints[2 * i], true);

			// knee joints
			localA.setIdentity();
			localB.setIdentity();
			localC.setIdentity();
			localA.getBasis().setEulerZYX(0, -fAngle, 0);
			localA.setOrigin(Vec3(Scalar(fCos * (fBodySize + fLegLength)), Scalar(0.), Scalar(fSin * (fBodySize + fLegLength))));
			localB = m_bodies[1 + 2 * i]->getWorldTransform().inverse() * m_bodies[0]->getWorldTransform() * localA;
			localC = m_bodies[2 + 2 * i]->getWorldTransform().inverse() * m_bodies[0]->getWorldTransform() * localA;
			hingeC = new HingeConstraint(*m_bodies[1 + 2 * i], *m_bodies[2 + 2 * i], localB, localC);
			//hingeC->setLimit(Scalar(-0.01), Scalar(0.01));
			hingeC->setLimit(Scalar(-M_PI_8), Scalar(0.2));
			m_joints[1 + 2 * i] = hingeC;
			m_ownerWorld->addConstraint(m_joints[1 + 2 * i], true);
		}
	}

	virtual ~TestRig()
	{
		i32 i;

		// Remove all constraints
		for (i = 0; i < JOINT_COUNT; ++i)
		{
			m_ownerWorld->removeConstraint(m_joints[i]);
			delete m_joints[i];
			m_joints[i] = 0;
		}

		// Remove all bodies and shapes
		for (i = 0; i < BODYPART_COUNT; ++i)
		{
			m_ownerWorld->removeRigidBody(m_bodies[i]);

			delete m_bodies[i]->getMotionState();

			delete m_bodies[i];
			m_bodies[i] = 0;
			delete m_shapes[i];
			m_shapes[i] = 0;
		}
	}

	TypedConstraint** GetJoints() { return &m_joints[0]; }
};

void motorPreTickCallback(DynamicsWorld* world, Scalar timeStep)
{
	MotorDemo* motorDemo = (MotorDemo*)world->getWorldUserInfo();

	motorDemo->setMotorTargets(timeStep);
}

void MotorDemo::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	// Setup the basic world

	m_Time = 0;
	m_fCyclePeriod = 2000.f;  // in milliseconds

	//	m_fMuscleStrength = 0.05f;
	// new SIMD solver for joints clips accumulated impulse, so the new limits for the motor
	// should be (numberOfsolverIterations * oldLimits)
	// currently solver uses 10 iterations, so:
	m_fMuscleStrength = 0.5f;

	m_collisionConfiguration = new DefaultCollisionConfiguration();

	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

	Vec3 worldAabbMin(-10000, -10000, -10000);
	Vec3 worldAabbMax(10000, 10000, 10000);
	m_broadphase = new AxisSweep3(worldAabbMin, worldAabbMax);

	m_solver = new SequentialImpulseConstraintSolver;

	m_dynamicsWorld = new DiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);

	m_dynamicsWorld->setInternalTickCallback(motorPreTickCallback, this, true);
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	// Setup a big ground box
	{
		CollisionShape* groundShape = new BoxShape(Vec3(Scalar(200.), Scalar(10.), Scalar(200.)));
		m_collisionShapes.push_back(groundShape);
		Transform2 groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(Vec3(0, -10, 0));
		createRigidBody(Scalar(0.), groundTransform, groundShape);
	}

	// Spawn one ragdoll
	Vec3 startOffset(1, 0.5, 0);
	spawnTestRig(startOffset, false);
	startOffset.setVal(-2, 0.5, 0);
	spawnTestRig(startOffset, true);

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void MotorDemo::spawnTestRig(const Vec3& startOffset, bool bFixed)
{
	TestRig* rig = new TestRig(m_dynamicsWorld, startOffset, bFixed);
	m_rigs.push_back(rig);
}

void PreStep()
{
}

void MotorDemo::setMotorTargets(Scalar deltaTime)
{
	float ms = deltaTime * 1000000.;
	float minFPS = 1000000.f / 60.f;
	if (ms > minFPS)
		ms = minFPS;

	m_Time += ms;

	//
	// set per-frame sinusoidal position targets using angular motor (hacky?)
	//
	for (i32 r = 0; r < m_rigs.size(); r++)
	{
		for (i32 i = 0; i < 2 * NUM_LEGS; i++)
		{
			HingeConstraint* hingeC = static_cast<HingeConstraint*>(m_rigs[r]->GetJoints()[i]);
			Scalar fCurAngle = hingeC->getHingeAngle();

			Scalar fTargetPercent = (i32(m_Time / 1000) % i32(m_fCyclePeriod)) / m_fCyclePeriod;
			Scalar fTargetAngle = 0.5 * (1 + sin(2 * M_PI * fTargetPercent));
			Scalar fTargetLimitAngle = hingeC->getLowerLimit() + fTargetAngle * (hingeC->getUpperLimit() - hingeC->getLowerLimit());
			Scalar fAngleError = fTargetLimitAngle - fCurAngle;
			Scalar fDesiredAngularVel = 1000000.f * fAngleError / ms;
			hingeC->enableAngularMotor(true, fDesiredAngularVel, m_fMuscleStrength);
		}
	}
}

#if 0
void MotorDemo::keyboardCallback(u8 key, i32 x, i32 y)
{
	switch (key)
	{
	case '+': case '=':
		m_fCyclePeriod /= 1.1f;
		if (m_fCyclePeriod < 1.f)
			m_fCyclePeriod = 1.f;
		break;
	case '-': case '_':
		m_fCyclePeriod *= 1.1f;
		break;
	case '[':
		m_fMuscleStrength /= 1.1f;
		break;
	case ']':
		m_fMuscleStrength *= 1.1f;
		break;
	default:
		DemoApplication::keyboardCallback(key, x, y);
	}
}
#endif

void MotorDemo::exitPhysics()
{
	i32 i;

	for (i = 0; i < m_rigs.size(); i++)
	{
		TestRig* rig = m_rigs[i];
		delete rig;
	}

	//cleanup in the reverse order of creation/initialization

	//remove the rigidbodies from the dynamics world and delete them

	for (i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		CollisionObject2* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		RigidBody* body = RigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for (i32 j = 0; j < m_collisionShapes.size(); j++)
	{
		CollisionShape* shape = m_collisionShapes[j];
		delete shape;
	}

	//delete dynamics world
	delete m_dynamicsWorld;

	//delete solver
	delete m_solver;

	//delete broadphase
	delete m_broadphase;

	//delete dispatcher
	delete m_dispatcher;

	delete m_collisionConfiguration;
}

class CommonExampleInterface* MotorControlCreateFunc(struct CommonExampleOptions& options)
{
	return new MotorDemo(options.m_guiHelper);
}
