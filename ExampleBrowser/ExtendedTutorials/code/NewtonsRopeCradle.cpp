#include "../NewtonsRopeCradle.h"
#include <cmath>
#include <iterator>
#include <vector>  // TODO: Should I use another data structure?
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>
#include <drx3D/Physics/SoftBody/SoftRigidDynamicsWorld.h>
#include <drx3D/Physics/SoftBody/SoftBodyHelpers.h>
#include <drx3D/Physics/SoftBody/SoftBodyRigidBodyCollisionConfiguration.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>

static Scalar gPendulaQty = 5;  // Number of pendula in newton's cradle
//TODO: This would actually be an Integer, but the Slider does not like integers, so I floor it when changed

static Scalar gDisplacedPendula = 1;  // number of displaced pendula
//TODO: This is an i32 as well

static Scalar gPendulaRestitution = 1;  // pendula restition when hitting against each other

static Scalar gSphereRadius = 1;  // pendula radius

static Scalar gInitialPendulumWidth = 4;  // default pendula width

static Scalar gInitialPendulumHeight = 8;  // default pendula height

static Scalar gRopeResolution = 1;  // default rope resolution (number of links as in a chain)

static Scalar gDisplacementForce = 30;  // default force to displace the pendula

static Scalar gForceScalar = 0;  // default force scalar to apply a displacement

struct NewtonsRopeCradleExample : public CommonRigidBodyBase
{
	NewtonsRopeCradleExample(struct GUIHelperInterface* helper) : CommonRigidBodyBase(helper)
	{
	}
	virtual ~NewtonsRopeCradleExample() {}
	virtual void initPhysics();
	virtual void stepSimulation(float deltaTime);
	virtual void renderScene();
	virtual void applyPendulumForce(Scalar pendulumForce);
	void createEmptyDynamicsWorld()
	{
		m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();
		m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

		m_broadphase = new DbvtBroadphase();

		m_solver = new SequentialImpulseConstraintSolver;

		m_dynamicsWorld = new SoftRigidDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
		m_dynamicsWorld->setGravity(Vec3(0, -10, 0));

		softBodyWorldInfo.m_broadphase = m_broadphase;
		softBodyWorldInfo.m_dispatcher = m_dispatcher;
		softBodyWorldInfo.m_gravity = m_dynamicsWorld->getGravity();
		softBodyWorldInfo.m_sparsesdf.Initialize();
	}

	virtual void createRopePendulum(SphereShape* colShape,
									const Vec3& position, const Quat& pendulumOrientation, Scalar width, Scalar height, Scalar mass);
	virtual void changePendulaRestitution(Scalar restitution);
	virtual void connectWithRope(RigidBody* body1, RigidBody* body2);
	virtual bool keyboardCallback(i32 key, i32 state);

	virtual SoftRigidDynamicsWorld* getSoftDynamicsWorld()
	{
		///just make it a SoftRigidDynamicsWorld please
		///or we will add type checking
		return (SoftRigidDynamicsWorld*)m_dynamicsWorld;
	}
	void resetCamera()
	{
		float dist = 41;
		float pitch = -35;
		float yaw = 52;
		float targetPos[3] = {0, 0.46, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}

	std::vector<SliderConstraint*> constraints;
	std::vector<RigidBody*> pendula;

	SoftBodyWorldInfo softBodyWorldInfo;
};

static NewtonsRopeCradleExample* nex = NULL;

void onRopePendulaRestitutionChanged(float pendulaRestitution, uk );

void applyRForceWithForceScalar(float forceScalar);

void NewtonsRopeCradleExample::initPhysics()
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
		slider.m_callback = onRopePendulaRestitutionChanged;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(
			slider);
	}

	{  // create a slider to change the rope resolution
		SliderParams slider("Rope Resolution", &gRopeResolution);
		slider.m_minVal = 1;
		slider.m_maxVal = 20;
		slider.m_clampToIntegers = true;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(
			slider);
	}

	{  // create a slider to change the pendulum width
		SliderParams slider("Pendulum Width", &gInitialPendulumWidth);
		slider.m_minVal = 0;
		slider.m_maxVal = 40;
		slider.m_clampToNotches = false;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(
			slider);
	}

	{  // create a slider to change the pendulum height
		SliderParams slider("Pendulum Height", &gInitialPendulumHeight);
		slider.m_minVal = 0;
		slider.m_maxVal = 40;
		slider.m_clampToNotches = false;
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
		Scalar pendulumMass(1.0f);

		Vec3 position(0.0f, 15.0f, 0.0f);  // initial left-most pendulum position
		Quat orientation(0, 0, 0, 1);   // orientation of the pendula

		// Re-using the same collision is better for memory usage and performance
		SphereShape* pendulumShape = new SphereShape(gSphereRadius);
		m_collisionShapes.push_back(pendulumShape);

		for (i32 i = 0; i < std::floor(gPendulaQty); i++)
		{
			// create pendulum
			createRopePendulum(pendulumShape, position, orientation, gInitialPendulumWidth,
							   gInitialPendulumHeight, pendulumMass);

			// displace the pendula 1.05 sphere size, so that they all nearly touch (small spacings in between)
			position.setX(position.x() - 2.1f * gSphereRadius);
		}
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void NewtonsRopeCradleExample::connectWithRope(RigidBody* body1, RigidBody* body2)
{
	SoftBody* softBodyRope0 = SoftBodyHelpers::CreateRope(softBodyWorldInfo, body1->getWorldTransform().getOrigin(), body2->getWorldTransform().getOrigin(), gRopeResolution, 0);
	softBodyRope0->setTotalMass(0.1f);

	softBodyRope0->appendAnchor(0, body1);
	softBodyRope0->appendAnchor(softBodyRope0->m_nodes.size() - 1, body2);

	softBodyRope0->m_cfg.piterations = 5;
	softBodyRope0->m_cfg.kDP = 0.005f;
	softBodyRope0->m_cfg.kSHR = 1;
	softBodyRope0->m_cfg.kCHR = 1;
	softBodyRope0->m_cfg.kKHR = 1;

	getSoftDynamicsWorld()->addSoftBody(softBodyRope0);
}

void NewtonsRopeCradleExample::stepSimulation(float deltaTime)
{
	applyRForceWithForceScalar(gForceScalar);  // apply force defined by apply force slider

	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->stepSimulation(deltaTime);
	}
}

void NewtonsRopeCradleExample::createRopePendulum(SphereShape* colShape,
												  const Vec3& position, const Quat& pendulumOrientation, Scalar width, Scalar height, Scalar mass)
{
	// The pendulum looks like this (names when built):
	// O   O  topSphere1 topSphere2
	//  \ /
	//   O   bottomSphere

	//create a dynamic pendulum
	Transform2 startTransform;
	startTransform.setIdentity();

	// calculate sphere positions
	Vec3 topSphere1RelPosition(0, 0, width);
	Vec3 topSphere2RelPosition(0, 0, -width);
	Vec3 bottomSphereRelPosition(0, -height, 0);

	// position the top sphere above ground with appropriate orientation
	startTransform.setOrigin(Vec3(0, 0, 0));                            // no translation intitially
	startTransform.setRotation(pendulumOrientation);                         // pendulum rotation
	startTransform.setOrigin(startTransform * topSphere1RelPosition);        // rotate this position
	startTransform.setOrigin(position + startTransform.getOrigin());         // add non-rotated position to the relative position
	RigidBody* topSphere1 = createRigidBody(0, startTransform, colShape);  // make top sphere static

	// position the top sphere above ground with appropriate orientation
	startTransform.setOrigin(Vec3(0, 0, 0));                            // no translation intitially
	startTransform.setRotation(pendulumOrientation);                         // pendulum rotation
	startTransform.setOrigin(startTransform * topSphere2RelPosition);        // rotate this position
	startTransform.setOrigin(position + startTransform.getOrigin());         // add non-rotated position to the relative position
	RigidBody* topSphere2 = createRigidBody(0, startTransform, colShape);  // make top sphere static

	// position the bottom sphere below the top sphere
	startTransform.setOrigin(Vec3(0, 0, 0));                        // no translation intitially
	startTransform.setRotation(pendulumOrientation);                     // pendulum rotation
	startTransform.setOrigin(startTransform * bottomSphereRelPosition);  // rotate this position
	startTransform.setOrigin(position + startTransform.getOrigin());     // add non-rotated position to the relative position
	RigidBody* bottomSphere = createRigidBody(mass, startTransform, colShape);
	bottomSphere->setFriction(0);  // we do not need friction here
	pendula.push_back(bottomSphere);

	// disable the deactivation when objects do not move anymore
	topSphere1->setActivationState(DISABLE_DEACTIVATION);
	topSphere2->setActivationState(DISABLE_DEACTIVATION);
	bottomSphere->setActivationState(DISABLE_DEACTIVATION);

	bottomSphere->setRestitution(gPendulaRestitution);  // set pendula restitution

	// add ropes between spheres
	connectWithRope(topSphere1, bottomSphere);
	connectWithRope(topSphere2, bottomSphere);
}

void NewtonsRopeCradleExample::renderScene()
{
	CommonRigidBodyBase::renderScene();
	SoftRigidDynamicsWorld* softWorld = getSoftDynamicsWorld();

	for (i32 i = 0; i < softWorld->getSoftBodyArray().size(); i++)
	{
		SoftBody* psb = (SoftBody*)softWorld->getSoftBodyArray()[i];
		//if (softWorld->getDebugDrawer() && !(softWorld->getDebugDrawer()->getDebugMode() & (IDebugDraw::DBG_DrawWireframe)))
		{
			SoftBodyHelpers::DrawFrame(psb, softWorld->getDebugDrawer());
			SoftBodyHelpers::Draw(psb, softWorld->getDebugDrawer(), softWorld->getDrawFlags());
		}
	}
}

void NewtonsRopeCradleExample::changePendulaRestitution(Scalar restitution)
{
	for (std::vector<RigidBody*>::iterator rit = pendula.begin();
		 rit != pendula.end(); rit++)
	{
		Assert((*rit) && "Null constraint");

		(*rit)->setRestitution(restitution);
	}
}

bool NewtonsRopeCradleExample::keyboardCallback(i32 key, i32 state)
{
	//drx3DPrintf("Key pressed: %d in state %d \n",key,state);

	// key 3
	switch (key)
	{
		case '3' /*ASCII for 3*/:
		{
			applyPendulumForce(gDisplacementForce);
			return true;
		}
	}

	return false;
}

void NewtonsRopeCradleExample::applyPendulumForce(Scalar pendulumForce)
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

void onRopePendulaRestitutionChanged(float pendulaRestitution, uk )
{
	if (nex)
	{
		nex->changePendulaRestitution(pendulaRestitution);
	}
}

void applyRForceWithForceScalar(float forceScalar)
{
	if (nex)
	{
		Scalar appliedForce = forceScalar * gDisplacementForce;

		if (fabs(gForceScalar) < 0.2f)
			gForceScalar = 0;

		nex->applyPendulumForce(appliedForce);
	}
}

CommonExampleInterface* ET_NewtonsRopeCradleCreateFunc(
	CommonExampleOptions& options)
{
	nex = new NewtonsRopeCradleExample(options.m_guiHelper);
	return nex;
}
