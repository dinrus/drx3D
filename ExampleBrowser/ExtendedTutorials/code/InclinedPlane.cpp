#include "../InclinedPlane.h"

#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>

static Scalar gTilt = 20.0f / 180.0f * SIMD_PI;  // tilt the ramp 20 degrees

static Scalar gRampFriction = 1;  // set ramp friction to 1

static Scalar gRampRestitution = 0;  // set ramp restitution to 0 (no restitution)

static Scalar gBoxFriction = 1;  // set box friction to 1

static Scalar gBoxRestitution = 0;  // set box restitution to 0

static Scalar gSphereFriction = 1;  // set sphere friction to 1

static Scalar gSphereRollingFriction = 1;     // set sphere rolling friction to 1
static Scalar gSphereSpinningFriction = 0.3;  // set sphere spinning friction to 0.3

static Scalar gSphereRestitution = 0;  // set sphere restitution to 0

// handles for changes
static RigidBody* ramp = NULL;
static RigidBody* gBox = NULL;
static RigidBody* gSphere = NULL;

struct InclinedPlaneExample : public CommonRigidBodyBase
{
	InclinedPlaneExample(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
	}
	virtual ~InclinedPlaneExample() {}
	virtual void initPhysics();
	virtual void resetScene();
	virtual void renderScene();
	virtual void stepSimulation(float deltaTime);
	virtual bool keyboardCallback(i32 key, i32 state);
	void resetCamera()
	{
		float dist = 41;
		float pitch = -35;
		float yaw = 52;
		float targetPos[3] = {0, 0.46, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

void onBoxFrictionChanged(float friction, uk userPtr);

void onBoxRestitutionChanged(float restitution, uk userPtr);

void onSphereFrictionChanged(float friction, uk userPtr);

void onSphereRestitutionChanged(float restitution, uk userPtr);

void onRampInclinationChanged(float inclination, uk userPtr);

void onRampFrictionChanged(float friction, uk userPtr);

void onRampRestitutionChanged(float restitution, uk userPtr);

void InclinedPlaneExample::initPhysics()
{
	{  // create slider to change the ramp tilt
		SliderParams slider("Ramp Tilt", &gTilt);
		slider.m_minVal = 0;
		slider.m_maxVal = SIMD_PI / 2.0f;
		slider.m_clampToNotches = false;
		slider.m_callback = onRampInclinationChanged;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	{  // create slider to change the ramp friction
		SliderParams slider("Ramp Friction", &gRampFriction);
		slider.m_minVal = 0;
		slider.m_maxVal = 10;
		slider.m_clampToNotches = false;
		slider.m_callback = onRampFrictionChanged;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	{  // create slider to change the ramp restitution
		SliderParams slider("Ramp Restitution", &gRampRestitution);
		slider.m_minVal = 0;
		slider.m_maxVal = 1;
		slider.m_clampToNotches = false;
		slider.m_callback = onRampRestitutionChanged;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	{  // create slider to change the box friction
		SliderParams slider("Box Friction", &gBoxFriction);
		slider.m_minVal = 0;
		slider.m_maxVal = 10;
		slider.m_clampToNotches = false;
		slider.m_callback = onBoxFrictionChanged;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	{  // create slider to change the box restitution
		SliderParams slider("Box Restitution", &gBoxRestitution);
		slider.m_minVal = 0;
		slider.m_maxVal = 1;
		slider.m_clampToNotches = false;
		slider.m_callback = onBoxRestitutionChanged;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	{  // create slider to change the sphere friction
		SliderParams slider("Sphere Friction", &gSphereFriction);
		slider.m_minVal = 0;
		slider.m_maxVal = 10;
		slider.m_clampToNotches = false;
		slider.m_callback = onSphereFrictionChanged;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	{  // create slider to change the sphere rolling friction
		SliderParams slider("Sphere Rolling Friction", &gSphereRollingFriction);
		slider.m_minVal = 0;
		slider.m_maxVal = 10;
		slider.m_clampToNotches = false;
		slider.m_callback = onSphereRestitutionChanged;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	{  // create slider to change the sphere rolling friction
		SliderParams slider("Sphere Spinning", &gSphereSpinningFriction);
		slider.m_minVal = 0;
		slider.m_maxVal = 2;
		slider.m_clampToNotches = false;
		slider.m_callback = onSphereRestitutionChanged;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	{  // create slider to change the sphere restitution
		SliderParams slider("Sphere Restitution", &gSphereRestitution);
		slider.m_minVal = 0;
		slider.m_maxVal = 1;
		slider.m_clampToNotches = false;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	m_guiHelper->setUpAxis(1);  // set Y axis as up axis

	createEmptyDynamicsWorld();

	// create debug drawer
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	if (m_dynamicsWorld->getDebugDrawer())
		m_dynamicsWorld->getDebugDrawer()->setDebugMode(IDebugDraw::DBG_DrawWireframe + IDebugDraw::DBG_DrawContactPoints);

	{  // create a static ground
		BoxShape* groundShape = createBoxShape(Vec3(Scalar(50.), Scalar(50.), Scalar(50.)));
		m_collisionShapes.push_back(groundShape);

		Transform2 groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(Vec3(0, -50, 0));

		Scalar mass(0.);
		createRigidBody(mass, groundTransform, groundShape, Vec4(0, 0, 1, 1));
	}

	{  //create a static inclined plane
		BoxShape* inclinedPlaneShape = createBoxShape(Vec3(Scalar(20.), Scalar(1.), Scalar(10.)));
		m_collisionShapes.push_back(inclinedPlaneShape);

		Transform2 startTransform;
		startTransform.setIdentity();

		// position the inclined plane above ground
		startTransform.setOrigin(Vec3(
			Scalar(0),
			Scalar(15),
			Scalar(0)));

		Quat incline;
		incline.setRotation(Vec3(0, 0, 1), gTilt);
		startTransform.setRotation(incline);

		Scalar mass(0.);
		ramp = createRigidBody(mass, startTransform, inclinedPlaneShape);
		ramp->setFriction(gRampFriction);
		ramp->setRestitution(gRampRestitution);
	}

	{  //create a cube above the inclined plane
		BoxShape* boxShape = createBoxShape(Vec3(1, 1, 1));

		m_collisionShapes.push_back(boxShape);

		Transform2 startTransform;
		startTransform.setIdentity();

		Scalar boxMass(1.f);

		startTransform.setOrigin(
			Vec3(Scalar(0), Scalar(20), Scalar(2)));

		gBox = createRigidBody(boxMass, startTransform, boxShape);
		gBox->forceActivationState(DISABLE_DEACTIVATION);  // to prevent the box on the ramp from disabling
		gBox->setFriction(gBoxFriction);
		gBox->setRestitution(gBoxRestitution);
	}

	{  //create a sphere above the inclined plane
		SphereShape* sphereShape = new SphereShape(Scalar(1));

		m_collisionShapes.push_back(sphereShape);

		Transform2 startTransform;
		startTransform.setIdentity();

		Scalar sphereMass(1.f);

		startTransform.setOrigin(
			Vec3(Scalar(0), Scalar(20), Scalar(4)));

		gSphere = createRigidBody(sphereMass, startTransform, sphereShape);
		gSphere->forceActivationState(DISABLE_DEACTIVATION);  // to prevent the sphere on the ramp from disabling
		gSphere->setFriction(gSphereFriction);
		gSphere->setRestitution(gSphereRestitution);
		gSphere->setRollingFriction(gSphereRollingFriction);
		gSphere->setSpinningFriction(gSphereSpinningFriction);
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void InclinedPlaneExample::resetScene()
{
	{  //reset a cube above the inclined plane

		Transform2 startTransform;
		startTransform.setIdentity();

		startTransform.setOrigin(
			Vec3(Scalar(0), Scalar(20), Scalar(2)));

		gBox->setWorldTransform(startTransform);
		Vec3 zero(0, 0, 0);
		gBox->setAngularVelocity(zero);
		gBox->setLinearVelocity(zero);
		gBox->clearForces();
	}

	{  //reset a sphere above the inclined plane
		Transform2 startTransform;
		startTransform.setIdentity();

		startTransform.setOrigin(
			Vec3(Scalar(0), Scalar(20), Scalar(4)));

		gSphere->setWorldTransform(startTransform);
		Vec3 zero(0, 0, 0);
		gSphere->setAngularVelocity(zero);
		gSphere->setLinearVelocity(zero);
		gSphere->clearForces();
	}
}

void InclinedPlaneExample::stepSimulation(float deltaTime)
{
	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->stepSimulation(deltaTime);
	}
}

void InclinedPlaneExample::renderScene()
{
	CommonRigidBodyBase::renderScene();
}

bool InclinedPlaneExample::keyboardCallback(i32 key, i32 state)
{
	//	drx3DPrintf("Key pressed: %d in state %d \n",key,state);

	switch (key)
	{
		case 32 /*ASCII for space*/:
		{
			resetScene();
			break;
		}
	}

	return false;
}

// GUI parameter modifiers
void onBoxFrictionChanged(float friction, uk )
{
	if (gBox)
	{
		gBox->setFriction(friction);
		//		drx3DPrintf("Friction of box changed to %f",friction );
	}
}

void onBoxRestitutionChanged(float restitution, uk )
{
	if (gBox)
	{
		gBox->setRestitution(restitution);
		//drx3DPrintf("Restitution of box changed to %f",restitution);
	}
}

void onSphereFrictionChanged(float friction, uk )
{
	if (gSphere)
	{
		gSphere->setFriction(friction);
		//drx3DPrintf("Friction of sphere changed to %f",friction );
	}
}

void onSphereRestitutionChanged(float restitution, uk )
{
	if (gSphere)
	{
		gSphere->setRestitution(restitution);
		//drx3DPrintf("Restitution of sphere changed to %f",restitution);
	}
}

void onRampInclinationChanged(float inclination, uk )
{
	if (ramp)
	{
		Transform2 startTransform;
		startTransform.setIdentity();

		// position the inclined plane above ground
		startTransform.setOrigin(
			Vec3(Scalar(0), Scalar(15), Scalar(0)));

		Quat incline;
		incline.setRotation(Vec3(0, 0, 1), gTilt);
		startTransform.setRotation(incline);
		ramp->setWorldTransform(startTransform);
		//drx3DPrintf("Inclination of ramp changed to %f",inclination );
	}
}

void onRampFrictionChanged(float friction, uk )
{
	if (ramp)
	{
		ramp->setFriction(friction);
		//drx3DPrintf("Friction of ramp changed to %f \n",friction );
	}
}

void onRampRestitutionChanged(float restitution, uk )
{
	if (ramp)
	{
		ramp->setRestitution(restitution);
		//drx3DPrintf("Restitution of ramp changed to %f \n",restitution);
	}
}

CommonExampleInterface* ET_InclinedPlaneCreateFunc(CommonExampleOptions& options)
{
	return new InclinedPlaneExample(options.m_guiHelper);
}
