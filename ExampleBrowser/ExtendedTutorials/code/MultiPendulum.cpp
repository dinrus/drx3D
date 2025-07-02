#include "../MultiPendulum.h"
#include <cmath>
#include <iterator>
#include <vector>  // TODO: Should I use another data structure?

#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>

static Scalar gPendulaQty = 2;  //TODO: This would actually be an Integer, but the Slider does not like integers, so I floor it when changed

static Scalar gDisplacedPendula = 1;  //TODO: This is an i32 as well

static Scalar gPendulaRestitution = 1;  // Default pendulum restitution is 1 to restore all force

static Scalar gSphereRadius = 1;  // The sphere radius

static Scalar gCurrentPendulumLength = 8;

static Scalar gInitialPendulumLength = 8;  // Default pendulum length (distance between two spheres)

static Scalar gDisplacementForce = 30;  // The default force with which we move the pendulum

static Scalar gForceScalar = 0;  // default force scalar to apply a displacement

struct MultiPendulumExample : public CommonRigidBodyBase
{
	MultiPendulumExample(struct GUIHelperInterface* helper) : CommonRigidBodyBase(helper)
	{
	}

	virtual ~MultiPendulumExample()
	{
	}

	virtual void initPhysics();                                                                                                                 // build a multi pendulum
	virtual void renderScene();                                                                                                                 // render the scene to screen
	virtual void createMultiPendulum(SphereShape* colShape, Scalar pendulaQty, const Vec3& position, Scalar length, Scalar mass);  // create a multi pendulum at the indicated x and y position, the specified number of pendula formed into a chain, each with indicated length and mass
	virtual void changePendulaLength(Scalar length);                                                                                          // change the pendulum length
	virtual void changePendulaRestitution(Scalar restitution);                                                                                // change the pendula restitution
	virtual void stepSimulation(float deltaTime);                                                                                               // step the simulation
	virtual bool keyboardCallback(i32 key, i32 state);                                                                                          // handle keyboard callbacks
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

static MultiPendulumExample* mex = NULL;  // Handle to the example to access it via functions. Do not use this in your simulation!

void onMultiPendulaLengthChanged(float pendulaLength, uk );  // Change the pendula length

void onMultiPendulaRestitutionChanged(float pendulaRestitution, uk );  // change the pendula restitution

void applyMForceWithForceScalar(float forceScalar);

void MultiPendulumExample::initPhysics()
{  // Setup your physics scene

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
		slider.m_callback = onMultiPendulaRestitutionChanged;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(
			slider);
	}

	{  // create a slider to change the pendulum length
		SliderParams slider("Pendula Length", &gCurrentPendulumLength);
		slider.m_minVal = 0;
		slider.m_maxVal = 49;
		slider.m_clampToNotches = false;
		slider.m_callback = onMultiPendulaLengthChanged;
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

	{  // create the multipendulum starting at the indicated position below and where each pendulum has the following mass
		Scalar pendulumMass(1.f);

		Vec3 position(0.0f, 15.0f, 0.0f);  // initial top-most pendulum position

		// Re-using the same collision is better for memory usage and performance
		SphereShape* pendulumShape = new SphereShape(gSphereRadius);
		m_collisionShapes.push_back(pendulumShape);

		// create multi-pendulum
		createMultiPendulum(pendulumShape, std::floor(gPendulaQty), position,
							gInitialPendulumLength, pendulumMass);
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void MultiPendulumExample::stepSimulation(float deltaTime)
{
	applyMForceWithForceScalar(gForceScalar);  // apply force defined by apply force slider

	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->stepSimulation(deltaTime);
	}
}

void MultiPendulumExample::createMultiPendulum(SphereShape* colShape,
											   Scalar pendulaQty, const Vec3& position,
											   Scalar length, Scalar mass)
{
	// The multi-pendulum looks like this (names when built):
	//..........0......./.......1...../.......2......./..etc...:pendulum build iterations
	// O   parentSphere
	// |
	// O   childSphere  / parentSphere
	// |
	// O   ............./ childSphere / parentSphere
	// |
	// O   .........................../ childSphere
	// etc.

	//create the top element of the pendulum
	Transform2 startTransform;
	startTransform.setIdentity();

	// position the top sphere
	startTransform.setOrigin(position);

	startTransform.setRotation(Quat(0, 0, 0, 1));  // zero rotation

	RigidBody* topSphere = createRigidBody(mass, startTransform, colShape);

	// disable the deactivation when object does not move anymore
	topSphere->setActivationState(DISABLE_DEACTIVATION);

	//make top sphere position "fixed" in the world by attaching it to a the world with a point to point constraint
	// The pivot is defined in the reference frame of topSphere, so the attachment should be exactly at the center of topSphere
	Vec3 constraintPivot(0.0f, 0.0f, 0.0f);
	Point2PointConstraint* p2pconst = new Point2PointConstraint(
		*topSphere, constraintPivot);

	p2pconst->setDbgDrawSize(Scalar(5.f));  // set the size of the debug drawing

	// add the constraint to the world
	m_dynamicsWorld->addConstraint(p2pconst, true);

	RigidBody* parentSphere = topSphere;  // set the top sphere as the parent sphere for the next sphere to be created

	for (i32 i = 0; i < pendulaQty; i++)
	{  // produce the number of pendula

		// create joint element to make the pendulum rotate it

		// position the joint sphere at the same position as the top sphere
		startTransform.setOrigin(position - Vec3(0, length * (i), 0));

		startTransform.setRotation(Quat(0, 0, 0, 1));  // zero rotation

		RigidBody* jointSphere = createRigidBody(mass, startTransform,
												   colShape);
		jointSphere->setFriction(0);  // we do not need friction here

		// disable the deactivation when object does not move anymore
		jointSphere->setActivationState(DISABLE_DEACTIVATION);

		//create constraint between parentSphere and jointSphere
		// this is represented by the constraint pivot in the local frames of reference of both constrained spheres
		Transform2 constraintPivotInParentSphereRF, constraintPivotInJointSphereRF;

		constraintPivotInParentSphereRF.setIdentity();
		constraintPivotInJointSphereRF.setIdentity();

		// the orientation of a point-to-point constraint does not matter, as is has no rotational limits

		//Obtain the position of parentSphere in local reference frame of the jointSphere (the pivot is therefore in the center of parentSphere)
		Vec3 parentSphereInJointSphereRF =
			(jointSphere->getWorldTransform().inverse()(
				parentSphere->getWorldTransform().getOrigin()));
		constraintPivotInJointSphereRF.setOrigin(parentSphereInJointSphereRF);

		Point2PointConstraint* p2pconst = new Point2PointConstraint(
			*parentSphere, *jointSphere, constraintPivotInParentSphereRF.getOrigin(), constraintPivotInJointSphereRF.getOrigin());

		p2pconst->setDbgDrawSize(Scalar(5.f));  // set the size of the debug drawing

		// add the constraint to the world
		m_dynamicsWorld->addConstraint(p2pconst, true);

		// create a slider constraint to change the length of the pendula while it swings

		startTransform.setIdentity();  // reset start transform

		// position the child sphere below the joint sphere
		startTransform.setOrigin(position - Vec3(0, length * (i + 1), 0));

		startTransform.setRotation(Quat(0, 0, 0, 1));  // zero rotation

		RigidBody* childSphere = createRigidBody(mass, startTransform,
												   colShape);
		childSphere->setFriction(0);  // we do not need friction here
		pendula.push_back(childSphere);

		// disable the deactivation when object does not move anymore
		childSphere->setActivationState(DISABLE_DEACTIVATION);

		//create slider constraint between jointSphere and childSphere
		// this is represented by the constraint pivot in the local frames of reference of both constrained spheres
		// furthermore we need to rotate the constraint appropriately to orient it correctly in space
		Transform2 constraintPivotInChildSphereRF;

		constraintPivotInJointSphereRF.setIdentity();
		constraintPivotInChildSphereRF.setIdentity();

		// the orientation of a point-to-point constraint does not matter, as is has no rotational limits

		//Obtain the position of jointSphere in local reference frame of the childSphere (the pivot is therefore in the center of jointSphere)
		Vec3 jointSphereInChildSphereRF =
			(childSphere->getWorldTransform().inverse()(
				jointSphere->getWorldTransform().getOrigin()));
		constraintPivotInChildSphereRF.setOrigin(jointSphereInChildSphereRF);

		// the slider constraint is x aligned per default, but we want it to be y aligned, therefore we rotate it
		Quat qt;
		qt.setEuler(0, 0, -SIMD_HALF_PI);
		constraintPivotInJointSphereRF.setRotation(qt);  //we use Y like up Axis
		constraintPivotInChildSphereRF.setRotation(qt);  //we use Y like up Axis

		SliderConstraint* sliderConst = new SliderConstraint(*jointSphere,
																 *childSphere, constraintPivotInJointSphereRF, constraintPivotInChildSphereRF, true);

		sliderConst->setDbgDrawSize(Scalar(5.f));  // set the size of the debug drawing

		// set limits
		// the initial setup of the constraint defines the origins of the limit dimensions,
		// therefore we set both limits directly to the current position of the parentSphere
		sliderConst->setLowerLinLimit(Scalar(0));
		sliderConst->setUpperLinLimit(Scalar(0));
		sliderConst->setLowerAngLimit(Scalar(0));
		sliderConst->setUpperAngLimit(Scalar(0));
		constraints.push_back(sliderConst);

		// add the constraint to the world
		m_dynamicsWorld->addConstraint(sliderConst, true);
		parentSphere = childSphere;
	}
}

void MultiPendulumExample::changePendulaLength(Scalar length)
{
	Scalar lowerLimit = -gInitialPendulumLength;
	for (std::vector<SliderConstraint*>::iterator sit = constraints.begin();
		 sit != constraints.end(); sit++)
	{
		Assert((*sit) && "Null constraint");

		// if the pendulum is being shortened beyond it's own length, we don't let the lower sphere to go past the upper one
		if (lowerLimit <= length)
		{
			(*sit)->setLowerLinLimit(length + lowerLimit);
			(*sit)->setUpperLinLimit(length + lowerLimit);
		}
	}
}

void MultiPendulumExample::changePendulaRestitution(Scalar restitution)
{
	for (std::vector<RigidBody*>::iterator rit = pendula.begin();
		 rit != pendula.end(); rit++)
	{
		Assert((*rit) && "Null constraint");

		(*rit)->setRestitution(restitution);
	}
}

void MultiPendulumExample::renderScene()
{
	CommonRigidBodyBase::renderScene();
}

bool MultiPendulumExample::keyboardCallback(i32 key, i32 state)
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

void MultiPendulumExample::applyPendulumForce(Scalar pendulumForce)
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

void onMultiPendulaLengthChanged(float pendulaLength, uk )
{  // Change the pendula length
	if (mex)
	{
		mex->changePendulaLength(pendulaLength);
	}
	//drx3DPrintf("Pendula length changed to %f \n",sliderValue );
}

void onMultiPendulaRestitutionChanged(float pendulaRestitution, uk )
{  // change the pendula restitution
	if (mex)
	{
		mex->changePendulaRestitution(pendulaRestitution);
	}
}

void applyMForceWithForceScalar(float forceScalar)
{
	if (mex)
	{
		Scalar appliedForce = forceScalar * gDisplacementForce;

		if (fabs(gForceScalar) < 0.2f)
			gForceScalar = 0;

		mex->applyPendulumForce(appliedForce);
	}
}

CommonExampleInterface* ET_MultiPendulumCreateFunc(
	CommonExampleOptions& options)
{
	mex = new MultiPendulumExample(options.m_guiHelper);
	return mex;
}
