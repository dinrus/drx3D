#include "../NewtonsCradle.h"
#include <cmath>
#include <iterator>
#include <vector>  // TODO: Should I use another data structure?
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>

static Scalar gPendulaQty = 5;  // Number of pendula in newton's cradle
//TODO: This would actually be an Integer, but the Slider does not like integers, so I floor it when changed

static Scalar gDisplacedPendula = 1;  // number of displaced pendula
//TODO: This is an i32 as well

static Scalar gPendulaRestitution = 1;  // pendula restitution when hitting against each other

static Scalar gSphereRadius = 1;  // pendula radius

static Scalar gCurrentPendulumLength = 8;  // current pendula length

static Scalar gInitialPendulumLength = 8;  // default pendula length

static Scalar gDisplacementForce = 30;  // default force to displace the pendula

static Scalar gForceScalar = 0;  // default force scalar to apply a displacement

struct NewtonsCradleExample : public CommonRigidBodyBase
{
	NewtonsCradleExample(struct GUIHelperInterface* helper) : CommonRigidBodyBase(helper)
	{
	}
	virtual ~NewtonsCradleExample()
	{
	}
	virtual void initPhysics();
	virtual void renderScene();
	virtual void createPendulum(SphereShape* colShape, const Vec3& position, Scalar length, Scalar mass);
	virtual void changePendulaLength(Scalar length);
	virtual void changePendulaRestitution(Scalar restitution);
	virtual void stepSimulation(float deltaTime);
	virtual bool keyboardCallback(i32 key, i32 state);
	virtual void applyPendulumForce(Scalar pendulumForce);
	void resetCamera()
	{
		float dist = 41;
		float pitch = -35;
		float yaw = 52;
		float targetPos[3] = {0, 0.46, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1],
								 targetPos[2]);
	}

	std::vector<SliderConstraint*> constraints;  // keep a handle to the slider constraints
	std::vector<RigidBody*> pendula;             // keep a handle to the pendula
};

static NewtonsCradleExample* nex = NULL;

void onPendulaLengthChanged(float pendulaLength, uk userPtr);  // Change the pendula length

void onPendulaRestitutionChanged(float pendulaRestitution, uk userPtr);  // change the pendula restitution

void applyForceWithForceScalar(float forceScalar);

void NewtonsCradleExample::initPhysics()
{
	{  // create a slider to change the number of pendula
		SliderParams slider("Number of Pendula", &gPendulaQty);
		slider.m_minVal = 1;
		slider.m_maxVal = 50;
		slider.m_clampToIntegers = true;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(
			slider);
	}

	{  // create a slider to change the number of displaced pendula
		SliderParams slider("Number of Displaced Pendula", &gDisplacedPendula);
		slider.m_minVal = 0;
		slider.m_maxVal = 49;
		slider.m_clampToIntegers = true;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(
			slider);
	}

	{  // create a slider to change the pendula restitution
		SliderParams slider("Pendula Restitution", &gPendulaRestitution);
		slider.m_minVal = 0;
		slider.m_maxVal = 1;
		slider.m_clampToNotches = false;
		slider.m_callback = onPendulaRestitutionChanged;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(
			slider);
	}

	{  // create a slider to change the pendulum length
		SliderParams slider("Pendula Length", &gCurrentPendulumLength);
		slider.m_minVal = 0;
		slider.m_maxVal = 49;
		slider.m_clampToNotches = false;
		slider.m_callback = onPendulaLengthChanged;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(
			slider);
	}

	{  // create a slider to change the force to displace the lowest pendulum
		SliderParams slider("Displacement force", &gDisplacementForce);
		slider.m_minVal = 0.1;
		slider.m_maxVal = 200;
		slider.m_clampToNotches = false;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(
			slider);
	}

	{  // create a slider to apply the force by slider
		SliderParams slider("Apply displacement force", &gForceScalar);
		slider.m_minVal = -1;
		slider.m_maxVal = 1;
		slider.m_clampToNotches = false;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(
			slider);
	}

	m_guiHelper->setUpAxis(1);

	createEmptyDynamicsWorld();

	// create a debug drawer
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	if (m_dynamicsWorld->getDebugDrawer())
		m_dynamicsWorld->getDebugDrawer()->setDebugMode(
			IDebugDraw::DBG_DrawWireframe + IDebugDraw::DBG_DrawContactPoints + IDebugDraw::DBG_DrawConstraints + IDebugDraw::DBG_DrawConstraintLimits);

	{  // create the pendula starting at the indicated position below and where each pendulum has the following mass
		Scalar pendulumMass(1.f);

		Vec3 position(0.0f, 15.0f, 0.0f);  // initial left-most pendulum position
		Quat orientation(0, 0, 0, 1);   // orientation of the pendula

		// Re-using the same collision is better for memory usage and performance
		SphereShape* pendulumShape = new SphereShape(gSphereRadius);
		m_collisionShapes.push_back(pendulumShape);

		for (i32 i = 0; i < std::floor(gPendulaQty); i++)
		{
			// create pendulum
			createPendulum(pendulumShape, position, gInitialPendulumLength, pendulumMass);

			// displace the pendula 1.05 sphere size, so that they all nearly touch (small spacings in between
			position.setX(position.x() - 2.1f * gSphereRadius);
		}
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void NewtonsCradleExample::stepSimulation(float deltaTime)
{
	applyForceWithForceScalar(gForceScalar);  // apply force defined by apply force slider

	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->stepSimulation(deltaTime);
	}
}

void NewtonsCradleExample::createPendulum(SphereShape* colShape, const Vec3& position, Scalar length, Scalar mass)
{
	// The pendulum looks like this (names when built):
	// O   topSphere
	// |
	// O   bottomSphere

	//create a dynamic pendulum
	Transform2 startTransform;
	startTransform.setIdentity();

	// position the top sphere above ground with a moving x position
	startTransform.setOrigin(position);
	startTransform.setRotation(Quat(0, 0, 0, 1));  // zero rotation
	RigidBody* topSphere = createRigidBody(mass, startTransform, colShape);

	// position the bottom sphere below the top sphere
	startTransform.setOrigin(
		Vec3(position.x(), Scalar(position.y() - length),
				  position.z()));

	startTransform.setRotation(Quat(0, 0, 0, 1));  // zero rotation
	RigidBody* bottomSphere = createRigidBody(mass, startTransform, colShape);
	bottomSphere->setFriction(0);  // we do not need friction here
	pendula.push_back(bottomSphere);

	// disable the deactivation when objects do not move anymore
	topSphere->setActivationState(DISABLE_DEACTIVATION);
	bottomSphere->setActivationState(DISABLE_DEACTIVATION);

	bottomSphere->setRestitution(gPendulaRestitution);  // set pendula restitution

	//make the top sphere position "fixed" to the world by attaching with a point to point constraint
	// The pivot is defined in the reference frame of topSphere, so the attachment is exactly at the center of the topSphere
	Vec3 constraintPivot(Vec3(0.0f, 0.0f, 0.0f));
	Point2PointConstraint* p2pconst = new Point2PointConstraint(*topSphere,
																	constraintPivot);

	p2pconst->setDbgDrawSize(Scalar(5.f));  // set the size of the debug drawing

	// add the constraint to the world
	m_dynamicsWorld->addConstraint(p2pconst, true);

	//create constraint between spheres
	// this is represented by the constraint pivot in the local frames of reference of both constrained spheres
	// furthermore we need to rotate the constraint appropriately to orient it correctly in space
	Transform2 constraintPivotInTopSphereRF, constraintPivotInBottomSphereRF;

	constraintPivotInTopSphereRF.setIdentity();
	constraintPivotInBottomSphereRF.setIdentity();

	// the slider constraint is x aligned per default, but we want it to be y aligned, therefore we rotate it
	Quat qt;
	qt.setEuler(0, 0, -SIMD_HALF_PI);
	constraintPivotInTopSphereRF.setRotation(qt);     //we use Y like up Axis
	constraintPivotInBottomSphereRF.setRotation(qt);  //we use Y like up Axis

	//Obtain the position of topSphere in local reference frame of bottomSphere (the pivot is therefore in the center of topSphere)
	Vec3 topSphereInBottomSphereRF =
		(bottomSphere->getWorldTransform().inverse()(
			topSphere->getWorldTransform().getOrigin()));
	constraintPivotInBottomSphereRF.setOrigin(topSphereInBottomSphereRF);

	SliderConstraint* sliderConst = new SliderConstraint(*topSphere,
															 *bottomSphere, constraintPivotInTopSphereRF, constraintPivotInBottomSphereRF, true);

	sliderConst->setDbgDrawSize(Scalar(5.f));  // set the size of the debug drawing

	// set limits
	// the initial setup of the constraint defines the origins of the limit dimensions,
	// therefore we set both limits directly to the current position of the topSphere
	sliderConst->setLowerLinLimit(Scalar(0));
	sliderConst->setUpperLinLimit(Scalar(0));
	sliderConst->setLowerAngLimit(Scalar(0));
	sliderConst->setUpperAngLimit(Scalar(0));
	constraints.push_back(sliderConst);

	// add the constraint to the world
	m_dynamicsWorld->addConstraint(sliderConst, true);
}

void NewtonsCradleExample::changePendulaLength(Scalar length)
{
	Scalar lowerLimit = -gInitialPendulumLength;
	for (std::vector<SliderConstraint*>::iterator sit = constraints.begin();
		 sit != constraints.end(); sit++)
	{
		Assert((*sit) && "Null constraint");

		//if the pendulum is being shortened beyond it's own length, we don't let the lower sphere to go past the upper one
		if (lowerLimit <= length)
		{
			(*sit)->setLowerLinLimit(length + lowerLimit);
			(*sit)->setUpperLinLimit(length + lowerLimit);
		}
	}
}

void NewtonsCradleExample::changePendulaRestitution(Scalar restitution)
{
	for (std::vector<RigidBody*>::iterator rit = pendula.begin();
		 rit != pendula.end(); rit++)
	{
		Assert((*rit) && "Null constraint");

		(*rit)->setRestitution(restitution);
	}
}

void NewtonsCradleExample::renderScene()
{
	CommonRigidBodyBase::renderScene();
}

bool NewtonsCradleExample::keyboardCallback(i32 key, i32 state)
{
	//drx3DPrintf("Key pressed: %d in state %d \n",key,state);

	//key 1, key 2, key 3
	switch (key)
	{
		case '1' /*ASCII for 1*/:
		{
			//assumption: Sphere are aligned in Z axis
			Scalar newLimit = Scalar(gCurrentPendulumLength + 0.1);

			changePendulaLength(newLimit);
			gCurrentPendulumLength = newLimit;

			drx3DPrintf("Increase pendulum length to %f", gCurrentPendulumLength);
			return true;
		}
		case '2' /*ASCII for 2*/:
		{
			//assumption: Sphere are aligned in Z axis
			Scalar newLimit = Scalar(gCurrentPendulumLength - 0.1);

			//is being shortened beyond it's own length, we don't let the lower sphere to go over the upper one
			if (0 <= newLimit)
			{
				changePendulaLength(newLimit);
				gCurrentPendulumLength = newLimit;
			}

			drx3DPrintf("Decrease pendulum length to %f", gCurrentPendulumLength);
			return true;
		}
		case '3' /*ASCII for 3*/:
		{
			applyPendulumForce(gDisplacementForce);
			return true;
		}
	}

	return false;
}

void NewtonsCradleExample::applyPendulumForce(Scalar pendulumForce)
{
	if (pendulumForce != 0)
	{
		drx3DPrintf("Apply %f to pendulum", pendulumForce);
		for (i32 i = 0; i < gDisplacedPendula; i++)
		{
			if (gDisplacedPendula >= 0 && gDisplacedPendula <= gPendulaQty)
				pendula[i]->applyCentralForce(Vec3(pendulumForce, 0, 0));
		}
	}
}

// GUI parameter modifiers

void onPendulaLengthChanged(float pendulaLength, uk )
{
	if (nex)
	{
		nex->changePendulaLength(pendulaLength);
		//drx3DPrintf("Pendula length changed to %f \n",sliderValue );
	}
}

void onPendulaRestitutionChanged(float pendulaRestitution, uk )
{
	if (nex)
	{
		nex->changePendulaRestitution(pendulaRestitution);
	}
}

void applyForceWithForceScalar(float forceScalar)
{
	if (nex)
	{
		Scalar appliedForce = forceScalar * gDisplacementForce;

		if (fabs(gForceScalar) < 0.2f)
			gForceScalar = 0;

		nex->applyPendulumForce(appliedForce);
	}
}

CommonExampleInterface* ET_NewtonsCradleCreateFunc(
	CommonExampleOptions& options)
{
	nex = new NewtonsCradleExample(options.m_guiHelper);
	return nex;
}
