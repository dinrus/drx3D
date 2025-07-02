#include "../ConstraintPhysicsSetup.h"
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>

struct ConstraintPhysicsSetup : public CommonRigidBodyBase
{
	ConstraintPhysicsSetup(struct GUIHelperInterface* helper);
	virtual ~ConstraintPhysicsSetup();
	virtual void initPhysics();

	virtual void stepSimulation(float deltaTime);

	virtual void resetCamera()
	{
		float dist = 7;
		float pitch = -44;
		float yaw = 721;
		float targetPos[3] = {8, 1, -11};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

ConstraintPhysicsSetup::ConstraintPhysicsSetup(struct GUIHelperInterface* helper)
	: CommonRigidBodyBase(helper)
{
}
ConstraintPhysicsSetup::~ConstraintPhysicsSetup()
{
}

static Scalar val;
static Scalar targetVel = 0;
static Scalar maxImpulse = 10000;
static HingeAccumulatedAngleConstraint* spDoorHinge = 0;
static Scalar actualHingeVelocity = 0.f;

static Vec3 AxisA(0, 1, 0);

void ConstraintPhysicsSetup::stepSimulation(float deltaTime)
{
	val = spDoorHinge->getAccumulatedHingeAngle() * SIMD_DEGS_PER_RAD;
	if (m_dynamicsWorld)
	{
		spDoorHinge->enableAngularMotor(true, targetVel, maxImpulse);

		m_dynamicsWorld->stepSimulation(deltaTime, 10, 1. / 240.);

		HingeConstraint* hinge = spDoorHinge;

		if (hinge)
		{
			const RigidBody& bodyA = hinge->getRigidBodyA();
			const RigidBody& bodyB = hinge->getRigidBodyB();

			Transform2 trA = bodyA.getWorldTransform();
			Vec3 angVelA = bodyA.getAngularVelocity();
			Vec3 angVelB = bodyB.getAngularVelocity();

			{
				Vec3 ax1 = trA.getBasis() * hinge->getFrameOffsetA().getBasis().getColumn(2);
				Scalar vel = angVelA.dot(ax1);
				vel -= angVelB.dot(ax1);
				printf("hinge velocity (q) = %f\n", vel);
				actualHingeVelocity = vel;
			}
			Vec3 ortho0, ortho1;
			PlaneSpace1(AxisA, ortho0, ortho1);
			{
				Scalar vel2 = angVelA.dot(ortho0);
				vel2 -= angVelB.dot(ortho0);
				printf("hinge orthogonal1 velocity (q) = %f\n", vel2);
			}
			{
				Scalar vel0 = angVelA.dot(ortho1);
				vel0 -= angVelB.dot(ortho1);
				printf("hinge orthogonal0 velocity (q) = %f\n", vel0);
			}
		}
	}
}

void ConstraintPhysicsSetup::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	createEmptyDynamicsWorld();

	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	i32 mode = IDebugDraw::DBG_DrawWireframe + IDebugDraw::DBG_DrawConstraints + IDebugDraw::DBG_DrawConstraintLimits;
	m_dynamicsWorld->getDebugDrawer()->setDebugMode(mode);

	{
		SliderParams slider("target vel", &targetVel);
		slider.m_minVal = -4;
		slider.m_maxVal = 4;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	{
		SliderParams slider("max impulse", &maxImpulse);
		slider.m_minVal = 0;
		slider.m_maxVal = 1000;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	{
		SliderParams slider("actual vel", &actualHingeVelocity);
		slider.m_minVal = -4;
		slider.m_maxVal = 4;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	val = 1.f;
	{
		SliderParams slider("angle", &val);
		slider.m_minVal = -720;
		slider.m_maxVal = 720;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	{  // create a door using hinge constraint attached to the world
		CollisionShape* pDoorShape = new BoxShape(Vec3(2.0f, 5.0f, 0.2f));
		m_collisionShapes.push_back(pDoorShape);
		Transform2 doorTrans;
		doorTrans.setIdentity();
		doorTrans.setOrigin(Vec3(-5.0f, -2.0f, 0.0f));
		RigidBody* pDoorBody = createRigidBody(1.0, doorTrans, pDoorShape);
		pDoorBody->setActivationState(DISABLE_DEACTIVATION);
		const Vec3 PivotA(10.f + 2.1f, -2.0f, 0.0f);  // right next to the door slightly outside

		spDoorHinge = new HingeAccumulatedAngleConstraint(*pDoorBody, PivotA, AxisA);

		m_dynamicsWorld->addConstraint(spDoorHinge);

		spDoorHinge->setDbgDrawSize(Scalar(5.f));
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

class CommonExampleInterface* ConstraintCreateFunc(CommonExampleOptions& options)
{
	return new ConstraintPhysicsSetup(options.m_guiHelper);
}
