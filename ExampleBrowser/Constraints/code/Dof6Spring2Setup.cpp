#include "../Dof6Spring2Setup.h"
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/NNCGConstraintSolver.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/MLCPSolver.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/SolveProjectedGaussSeidel.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/LemkeSolver.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/DantzigSolver.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofSpring2Constraint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

#ifndef M_PI_4
#define M_PI_4 0.785398163397448309616
#endif

extern float g_additionalBodyMass;

//comment this out to compare with original spring constraint
#define USE_6DOF2
#ifdef USE_6DOF2
#define CONSTRAINT_TYPE Generic6DofSpring2Constraint
#define EXTRAPARAMS
#else
#define CONSTRAINT_TYPE Generic6DofSpringConstraint
#define EXTRAPARAMS , true
#endif

#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

struct Dof6Spring2Setup : public CommonRigidBodyBase
{
	struct Dof6Spring2SetupInternalData* m_data;

	Dof6Spring2Setup(struct GUIHelperInterface* helper);
	virtual ~Dof6Spring2Setup();
	virtual void initPhysics();

	virtual void stepSimulation(float deltaTime);

	void animate();

	virtual void resetCamera()
	{
		float dist = 5;
		float pitch = -35;
		float yaw = 722;
		float targetPos[3] = {4, 2, -11};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

struct Dof6Spring2SetupInternalData
{
	RigidBody* m_TranslateSpringBody;
	RigidBody* m_TranslateSpringBody2;
	RigidBody* m_RotateSpringBody;
	RigidBody* m_RotateSpringBody2;
	RigidBody* m_BouncingTranslateBody;
	RigidBody* m_MotorBody;
	RigidBody* m_ServoMotorBody;
	RigidBody* m_ChainLeftBody;
	RigidBody* m_ChainRightBody;
	CONSTRAINT_TYPE* m_ServoMotorConstraint;
	CONSTRAINT_TYPE* m_ChainLeftConstraint;
	CONSTRAINT_TYPE* m_ChainRightConstraint;

	float mDt;

	u32 frameID;
	Dof6Spring2SetupInternalData()
		: mDt(1. / 60.), frameID(0)
	{
	}
};

Dof6Spring2Setup::Dof6Spring2Setup(struct GUIHelperInterface* helper)
	: CommonRigidBodyBase(helper)
{
	m_data = new Dof6Spring2SetupInternalData;
}
Dof6Spring2Setup::~Dof6Spring2Setup()
{
	exitPhysics();
	delete m_data;
}
void Dof6Spring2Setup::initPhysics()
{
	// Setup the basic world

	m_guiHelper->setUpAxis(1);

	m_collisionConfiguration = new DefaultCollisionConfiguration();
	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);
	Vec3 worldAabbMin(-10000, -10000, -10000);
	Vec3 worldAabbMax(10000, 10000, 10000);
	m_broadphase = new AxisSweep3(worldAabbMin, worldAabbMax);

	/////// uncomment the corresponding line to test a solver.
	//m_solver = new SequentialImpulseConstraintSolver;
	m_solver = new NNCGConstraintSolver;
	//m_solver = new btMLCPSolver(new SolveProjectedGaussSeidel());
	//m_solver = new btMLCPSolver(new btDantzigSolver());
	//m_solver = new btMLCPSolver(new btLemkeSolver());

	m_dynamicsWorld = new DiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
	m_dynamicsWorld->getDispatchInfo().m_useContinuous = true;
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	m_dynamicsWorld->setGravity(Vec3(0, 0, 0));

	// Setup a big ground box
	{
		CollisionShape* groundShape = new BoxShape(Vec3(Scalar(200.), Scalar(5.), Scalar(200.)));
		Transform2 groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(Vec3(0, -10, 0));
#define CREATE_GROUND_COLLISION_OBJECT 1
#ifdef CREATE_GROUND_COLLISION_OBJECT
		CollisionObject2* fixedGround = new CollisionObject2();
		fixedGround->setCollisionShape(groundShape);
		fixedGround->setWorldTransform(groundTransform);
		m_dynamicsWorld->addCollisionObject(fixedGround);
#else
		localCreateRigidBody(Scalar(0.), groundTransform, groundShape);
#endif  //CREATE_GROUND_COLLISION_OBJECT
	}

	m_dynamicsWorld->getSolverInfo().m_numIterations = 100;

	CollisionShape* shape;
	Vec3 localInertia(0, 0, 0);
	DefaultMotionState* motionState;
	Transform2 bodyTransform;
	Scalar mass;
	Transform2 localA;
	Transform2 localB;
	CONSTRAINT_TYPE* constraint;

	//static body centered in the origo
	mass = 0.0;
	shape = new BoxShape(Vec3(0.5, 0.5, 0.5));
	localInertia = Vec3(0, 0, 0);
	bodyTransform.setIdentity();
	motionState = new DefaultMotionState(bodyTransform);
	RigidBody* staticBody = new RigidBody(mass, motionState, shape, localInertia);

	/////////// box with undamped translate spring attached to static body
	/////////// the box should oscillate left-to-right forever
	{
		mass = 1.0;
		shape = new BoxShape(Vec3(0.5, 0.5, 0.5));
		shape->calculateLocalInertia(mass, localInertia);
		bodyTransform.setIdentity();
		bodyTransform.setOrigin(Vec3(-2, 0, -5));
		motionState = new DefaultMotionState(bodyTransform);
		m_data->m_TranslateSpringBody = new RigidBody(mass, motionState, shape, localInertia);
		m_data->m_TranslateSpringBody->setActivationState(DISABLE_DEACTIVATION);
		m_dynamicsWorld->addRigidBody(m_data->m_TranslateSpringBody);
		localA.setIdentity();
		localA.getOrigin() = Vec3(0, 0, -5);
		localB.setIdentity();
		constraint = new CONSTRAINT_TYPE(*staticBody, *m_data->m_TranslateSpringBody, localA, localB EXTRAPARAMS);
		constraint->setLimit(0, 1, -1);
		constraint->setLimit(1, 0, 0);
		constraint->setLimit(2, 0, 0);
		constraint->setLimit(3, 0, 0);
		constraint->setLimit(4, 0, 0);
		constraint->setLimit(5, 0, 0);
		constraint->enableSpring(0, true);
		constraint->setStiffness(0, 100);
#ifdef USE_6DOF2
		constraint->setDamping(0, 0);
#else
		constraint->setDamping(0, 1);
#endif
		constraint->setEquilibriumPoint(0, 0);
		constraint->setDbgDrawSize(Scalar(2.f));
		m_dynamicsWorld->addConstraint(constraint, true);
	}

	/////////// box with rotate spring, attached to static body
	/////////// box should swing (rotate) left-to-right forever
	{
		mass = 1.0;
		shape = new BoxShape(Vec3(0.5, 0.5, 0.5));
		shape->calculateLocalInertia(mass, localInertia);
		bodyTransform.setIdentity();
		bodyTransform.getBasis().setEulerZYX(0, 0, M_PI_2);
		motionState = new DefaultMotionState(bodyTransform);
		m_data->m_RotateSpringBody = new RigidBody(mass, motionState, shape, localInertia);
		m_data->m_RotateSpringBody->setActivationState(DISABLE_DEACTIVATION);
		m_dynamicsWorld->addRigidBody(m_data->m_RotateSpringBody);
		localA.setIdentity();
		localA.getOrigin() = Vec3(0, 0, 0);
		localB.setIdentity();
		localB.setOrigin(Vec3(0, 0.5, 0));
		constraint = new CONSTRAINT_TYPE(*staticBody, *m_data->m_RotateSpringBody, localA, localB EXTRAPARAMS);
		constraint->setLimit(0, 0, 0);
		constraint->setLimit(1, 0, 0);
		constraint->setLimit(2, 0, 0);
		constraint->setLimit(3, 0, 0);
		constraint->setLimit(4, 0, 0);
		constraint->setLimit(5, 1, -1);
		constraint->enableSpring(5, true);
		constraint->setStiffness(5, 100);
#ifdef USE_6DOF2
		constraint->setDamping(5, 0);
#else
		constraint->setDamping(5, 1);
#endif
		constraint->setEquilibriumPoint(0, 0);
		constraint->setDbgDrawSize(Scalar(2.f));
		m_dynamicsWorld->addConstraint(constraint, true);
	}

	/////////// box with bouncing constraint, translation is bounced at the positive x limit, but not at the negative limit
	/////////// bouncing can not be set independently at low and high limits, so two constraints will be created: one that defines the low (non bouncing) limit, and one that defines the high (bouncing) limit
	/////////// the box should move to the left (as an impulse will be applied to it periodically) until it reaches its limit, then bounce back
	{
		mass = 1.0;
		shape = new BoxShape(Vec3(0.5, 0.5, 0.5));
		shape->calculateLocalInertia(mass, localInertia);
		bodyTransform.setIdentity();
		bodyTransform.setOrigin(Vec3(0, 0, -3));
		motionState = new DefaultMotionState(bodyTransform);
		m_data->m_BouncingTranslateBody = new RigidBody(mass, motionState, shape, localInertia);
		m_data->m_BouncingTranslateBody->setActivationState(DISABLE_DEACTIVATION);
		m_data->m_BouncingTranslateBody->setDeactivationTime(Scalar(20000000));
		m_dynamicsWorld->addRigidBody(m_data->m_BouncingTranslateBody);
		localA.setIdentity();
		localA.getOrigin() = Vec3(0, 0, 0);
		localB.setIdentity();
		constraint = new CONSTRAINT_TYPE(*staticBody, *m_data->m_BouncingTranslateBody, localA, localB EXTRAPARAMS);
		constraint->setLimit(0, -2, SIMD_INFINITY);
		constraint->setLimit(1, 0, 0);
		constraint->setLimit(2, -3, -3);
		constraint->setLimit(3, 0, 0);
		constraint->setLimit(4, 0, 0);
		constraint->setLimit(5, 0, 0);
#ifdef USE_6DOF2
		constraint->setBounce(0, 0);
#else  //bounce is named restitution in 6dofspring, but not implemented for translational limit motor, so the following line has no effect
		constraint->getTranslationalLimitMotor()->m_restitution = 0.0;
#endif
		constraint->setParam(DRX3D_CONSTRAINT_STOP_ERP, 0.995, 0);
		constraint->setParam(DRX3D_CONSTRAINT_STOP_CFM, 0.0, 0);
		constraint->setDbgDrawSize(Scalar(2.f));
		m_dynamicsWorld->addConstraint(constraint, true);
		constraint = new CONSTRAINT_TYPE(*staticBody, *m_data->m_BouncingTranslateBody, localA, localB EXTRAPARAMS);
		constraint->setLimit(0, -SIMD_INFINITY, 2);
		constraint->setLimit(1, 0, 0);
		constraint->setLimit(2, -3, -3);
		constraint->setLimit(3, 0, 0);
		constraint->setLimit(4, 0, 0);
		constraint->setLimit(5, 0, 0);
#ifdef USE_6DOF2
		constraint->setBounce(0, 1);
#else  //bounce is named restitution in 6dofspring, but not implemented for translational limit motor, so the following line has no effect
		constraint->getTranslationalLimitMotor()->m_restitution = 1.0;
#endif
		constraint->setParam(DRX3D_CONSTRAINT_STOP_ERP, 0.995, 0);
		constraint->setParam(DRX3D_CONSTRAINT_STOP_CFM, 0.0, 0);
		constraint->setDbgDrawSize(Scalar(2.f));
		m_dynamicsWorld->addConstraint(constraint, true);
	}

	/////////// box with rotational motor, attached to static body
	/////////// the box should rotate around the y axis
	{
		mass = 1.0;
		shape = new BoxShape(Vec3(0.5, 0.5, 0.5));
		shape->calculateLocalInertia(mass, localInertia);
		bodyTransform.setIdentity();
		bodyTransform.setOrigin(Vec3(4, 0, 0));
		motionState = new DefaultMotionState(bodyTransform);
		m_data->m_MotorBody = new RigidBody(mass, motionState, shape, localInertia);
		m_data->m_MotorBody->setActivationState(DISABLE_DEACTIVATION);
		m_dynamicsWorld->addRigidBody(m_data->m_MotorBody);
		localA.setIdentity();
		localA.getOrigin() = Vec3(4, 0, 0);
		localB.setIdentity();
		constraint = new CONSTRAINT_TYPE(*staticBody, *m_data->m_MotorBody, localA, localB EXTRAPARAMS);
		constraint->setLimit(0, 0, 0);
		constraint->setLimit(1, 0, 0);
		constraint->setLimit(2, 0, 0);
		constraint->setLimit(3, 0, 0);
		constraint->setLimit(4, 0, 0);
		constraint->setLimit(5, 1, -1);
#ifdef USE_6DOF2
		constraint->enableMotor(5, true);
		constraint->setTargetVelocity(5, 3.f);
		constraint->setMaxMotorForce(5, 600.f);
#else
		constraint->getRotationalLimitMotor(2)->m_enableMotor = true;
		constraint->getRotationalLimitMotor(2)->m_targetVelocity = 3.f;
		constraint->getRotationalLimitMotor(2)->m_maxMotorForce = 600.f;
#endif
		constraint->setDbgDrawSize(Scalar(2.f));
		m_dynamicsWorld->addConstraint(constraint, true);
	}

	/////////// box with rotational servo motor, attached to static body
	/////////// the box should rotate around the y axis until it reaches its target
	/////////// the target will be negated periodically
	{
		mass = 1.0;
		shape = new BoxShape(Vec3(0.5, 0.5, 0.5));
		shape->calculateLocalInertia(mass, localInertia);
		bodyTransform.setIdentity();
		bodyTransform.setOrigin(Vec3(7, 0, 0));
		motionState = new DefaultMotionState(bodyTransform);
		m_data->m_ServoMotorBody = new RigidBody(mass, motionState, shape, localInertia);
		m_data->m_ServoMotorBody->setActivationState(DISABLE_DEACTIVATION);
		m_dynamicsWorld->addRigidBody(m_data->m_ServoMotorBody);
		localA.setIdentity();
		localA.getOrigin() = Vec3(7, 0, 0);
		localB.setIdentity();
		constraint = new CONSTRAINT_TYPE(*staticBody, *m_data->m_ServoMotorBody, localA, localB EXTRAPARAMS);
		constraint->setLimit(0, 0, 0);
		constraint->setLimit(1, 0, 0);
		constraint->setLimit(2, 0, 0);
		constraint->setLimit(3, 0, 0);
		constraint->setLimit(4, 0, 0);
		constraint->setLimit(5, 1, -1);
#ifdef USE_6DOF2
		constraint->enableMotor(5, true);
		constraint->setTargetVelocity(5, 3.f);
		constraint->setMaxMotorForce(5, 600.f);
		constraint->setServo(5, true);
		constraint->setServoTarget(5, M_PI_2);
#else
		constraint->getRotationalLimitMotor(2)->m_enableMotor = true;
		constraint->getRotationalLimitMotor(2)->m_targetVelocity = 3.f;
		constraint->getRotationalLimitMotor(2)->m_maxMotorForce = 600.f;
		//servo motor is not implemented in 6dofspring constraint
#endif
		constraint->setDbgDrawSize(Scalar(2.f));
		m_dynamicsWorld->addConstraint(constraint, true);
		m_data->m_ServoMotorConstraint = constraint;
	}

	////////// chain of boxes linked together with fully limited rotational and translational constraints
	////////// the chain will be pulled to the left and to the right periodically. They should strictly stick together.
	{
		Scalar limitConstraintStrength = 0.6;
		i32 bodycount = 10;
		RigidBody* prevBody = 0;
		for (i32 i = 0; i < bodycount; ++i)
		{
			mass = 1.0;
			shape = new BoxShape(Vec3(0.5, 0.5, 0.5));
			shape->calculateLocalInertia(mass, localInertia);
			bodyTransform.setIdentity();
			bodyTransform.setOrigin(Vec3(-i, 0, 3));
			motionState = new DefaultMotionState(bodyTransform);
			RigidBody* body = new RigidBody(mass, motionState, shape, localInertia);
			body->setActivationState(DISABLE_DEACTIVATION);
			m_dynamicsWorld->addRigidBody(body);
			if (prevBody != 0)
			{
				localB.setIdentity();
				localB.setOrigin(Vec3(0.5, 0, 0));
				Transform2 localA;
				localA.setIdentity();
				localA.setOrigin(Vec3(-0.5, 0, 0));
				CONSTRAINT_TYPE* constraint = new CONSTRAINT_TYPE(*prevBody, *body, localA, localB EXTRAPARAMS);
				constraint->setLimit(0, -0.01, 0.01);
				constraint->setLimit(1, 0, 0);
				constraint->setLimit(2, 0, 0);
				constraint->setLimit(3, 0, 0);
				constraint->setLimit(4, 0, 0);
				constraint->setLimit(5, 0, 0);
				for (i32 a = 0; a < 6; ++a)
				{
					constraint->setParam(DRX3D_CONSTRAINT_STOP_ERP, 0.9, a);
					constraint->setParam(DRX3D_CONSTRAINT_STOP_CFM, 0.0, a);
				}
				constraint->setDbgDrawSize(Scalar(1.f));
				m_dynamicsWorld->addConstraint(constraint, true);

				if (i < bodycount - 1)
				{
					localA.setIdentity();
					localA.getOrigin() = Vec3(0, 0, 3);
					localB.setIdentity();
					CONSTRAINT_TYPE* constraintZY = new CONSTRAINT_TYPE(*staticBody, *body, localA, localB EXTRAPARAMS);
					constraintZY->setLimit(0, 1, -1);
					constraintZY->setDbgDrawSize(Scalar(1.f));
					m_dynamicsWorld->addConstraint(constraintZY, true);
				}
			}
			else
			{
				localA.setIdentity();
				localA.getOrigin() = Vec3(bodycount, 0, 3);
				localB.setIdentity();
				localB.setOrigin(Vec3(0, 0, 0));
				m_data->m_ChainLeftBody = body;
				m_data->m_ChainLeftConstraint = new CONSTRAINT_TYPE(*staticBody, *body, localA, localB EXTRAPARAMS);
				m_data->m_ChainLeftConstraint->setLimit(3, 0, 0);
				m_data->m_ChainLeftConstraint->setLimit(4, 0, 0);
				m_data->m_ChainLeftConstraint->setLimit(5, 0, 0);
				for (i32 a = 0; a < 6; ++a)
				{
					m_data->m_ChainLeftConstraint->setParam(DRX3D_CONSTRAINT_STOP_ERP, limitConstraintStrength, a);
					m_data->m_ChainLeftConstraint->setParam(DRX3D_CONSTRAINT_STOP_CFM, 0.0, a);
				}
				m_data->m_ChainLeftConstraint->setDbgDrawSize(Scalar(1.f));
				m_dynamicsWorld->addConstraint(m_data->m_ChainLeftConstraint, true);
			}
			prevBody = body;
		}
		m_data->m_ChainRightBody = prevBody;
		localA.setIdentity();
		localA.getOrigin() = Vec3(-bodycount, 0, 3);
		localB.setIdentity();
		localB.setOrigin(Vec3(0, 0, 0));
		m_data->m_ChainRightConstraint = new CONSTRAINT_TYPE(*staticBody, *m_data->m_ChainRightBody, localA, localB EXTRAPARAMS);
		m_data->m_ChainRightConstraint->setLimit(3, 0, 0);
		m_data->m_ChainRightConstraint->setLimit(4, 0, 0);
		m_data->m_ChainRightConstraint->setLimit(5, 0, 0);
		for (i32 a = 0; a < 6; ++a)
		{
			m_data->m_ChainRightConstraint->setParam(DRX3D_CONSTRAINT_STOP_ERP, limitConstraintStrength, a);
			m_data->m_ChainRightConstraint->setParam(DRX3D_CONSTRAINT_STOP_CFM, 0.0, a);
		}
	}
	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void Dof6Spring2Setup::animate()
{
/////// servo motor: flip its target periodically
#ifdef USE_6DOF2
	static float servoNextFrame = -1;
	if (servoNextFrame < 0)
	{
		m_data->m_ServoMotorConstraint->getRotationalLimitMotor(2)->m_servoTarget *= -1;
		servoNextFrame = 3.0;
	}
	servoNextFrame -= m_data->mDt;
#endif

	/////// constraint chain: pull the chain left and right periodically
	static float chainNextFrame = -1;
	static bool left = true;
	if (chainNextFrame < 0)
	{
		if (!left)
		{
			m_data->m_ChainRightBody->setActivationState(ACTIVE_TAG);
			m_dynamicsWorld->removeConstraint(m_data->m_ChainRightConstraint);
			m_data->m_ChainLeftConstraint->setDbgDrawSize(Scalar(2.f));
			m_dynamicsWorld->addConstraint(m_data->m_ChainLeftConstraint, true);
		}
		else
		{
			m_data->m_ChainLeftBody->setActivationState(ACTIVE_TAG);
			m_dynamicsWorld->removeConstraint(m_data->m_ChainLeftConstraint);
			m_data->m_ChainRightConstraint->setDbgDrawSize(Scalar(2.f));
			m_dynamicsWorld->addConstraint(m_data->m_ChainRightConstraint, true);
		}
		chainNextFrame = 3.0;
		left = !left;
	}
	chainNextFrame -= m_data->mDt;

	/////// bouncing constraint: push the box periodically
	m_data->m_BouncingTranslateBody->setActivationState(ACTIVE_TAG);
	static float bounceNextFrame = -1;
	if (bounceNextFrame < 0)
	{
		m_data->m_BouncingTranslateBody->applyCentralImpulse(Vec3(10, 0, 0));
		bounceNextFrame = 3.0;
	}
	bounceNextFrame -= m_data->mDt;

	m_data->frameID++;
}

void Dof6Spring2Setup::stepSimulation(float deltaTime)
{
	animate();
	m_dynamicsWorld->stepSimulation(deltaTime);
}

class CommonExampleInterface* Dof6Spring2CreateFunc(CommonExampleOptions& options)
{
	return new Dof6Spring2Setup(options.m_guiHelper);
}
