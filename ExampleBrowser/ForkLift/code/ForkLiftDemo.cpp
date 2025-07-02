///September 2006: VehicleDemo is work in progress, this file is mostly just a placeholder
///This VehicleDemo file is very early in development, please check it later
///@todo is a basic engine model:
///A function that maps user input (throttle) into torque/force applied on the wheels
///with gears etc.
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/Collision/Shapes/HeightfieldTerrainShape.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/DantzigSolver.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/SolveProjectedGaussSeidel.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/MLCPSolver.h>

class VehicleTuning;
struct VehicleRaycaster;
class CollisionShape;

#include <drx3D/Physics/Dynamics/Vehicle/RaycastVehicle.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/HingeConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SliderConstraint.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/CollisionCommon.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/CommonWindowInterface.h>
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>

///VehicleDemo shows how to setup and use the built-in raycast vehicle
class ForkLiftDemo : public CommonExampleInterface
{
public:
	GUIHelperInterface* m_guiHelper;

	/* extra stuff*/
	Vec3 m_cameraPosition;
	class DiscreteDynamicsWorld* m_dynamicsWorld;
	DiscreteDynamicsWorld* getDynamicsWorld()
	{
		return m_dynamicsWorld;
	}
	RigidBody* m_carChassis;
	RigidBody* localCreateRigidBody(Scalar mass, const Transform2& worldTransform, CollisionShape* colSape);

	i32 m_wheelInstances[4];

	//----------------------------
	RigidBody* m_liftBody;
	Vec3 m_liftStartPos;
	HingeConstraint* m_liftHinge;

	RigidBody* m_forkBody;
	Vec3 m_forkStartPos;
	SliderConstraint* m_forkSlider;

	RigidBody* m_loadBody;
	Vec3 m_loadStartPos;

	void lockLiftHinge(void);
	void lockForkSlider(void);

	bool m_useDefaultCamera;
	//----------------------------

	AlignedObjectArray<CollisionShape*> m_collisionShapes;

	class BroadphaseInterface* m_overlappingPairCache;

	class CollisionDispatcher* m_dispatcher;

	class ConstraintSolver* m_constraintSolver;

	class DefaultCollisionConfiguration* m_collisionConfiguration;

	class TriangleIndexVertexArray* m_indexVertexArrays;

	Vec3* m_vertices;

	RaycastVehicle::VehicleTuning m_tuning;
	VehicleRaycaster* m_vehicleRayCaster;
	RaycastVehicle* m_vehicle;
	CollisionShape* m_wheelShape;

	float m_cameraHeight;

	float m_minCameraDistance;
	float m_maxCameraDistance;

	ForkLiftDemo(struct GUIHelperInterface* helper);

	virtual ~ForkLiftDemo();

	virtual void stepSimulation(float deltaTime);

	virtual void resetForklift();

	virtual void clientResetScene();

	virtual void displayCallback();

	virtual void specialKeyboard(i32 key, i32 x, i32 y);

	virtual void specialKeyboardUp(i32 key, i32 x, i32 y);

	virtual bool mouseMoveCallback(float x, float y)
	{
		return false;
	}

	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y)
	{
		return false;
	}

	virtual bool keyboardCallback(i32 key, i32 state);

	virtual void renderScene();

	virtual void physicsDebugDraw(i32 debugFlags);

	void initPhysics();
	void exitPhysics();

	virtual void resetCamera()
	{
		float dist = 8;
		float pitch = -32;
		float yaw = -45;
		float targetPos[3] = {-0.33, -0.72, 4.5};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}

	/*static DemoApplication* Create()
	{
		ForkLiftDemo* demo = new ForkLiftDemo();
		demo->myinit();
		demo->initPhysics();
		return demo;
	}
	*/
};

Scalar maxMotorImpulse = 4000.f;

//the sequential impulse solver has difficulties dealing with large mass ratios (differences), between loadMass and the fork parts
Scalar loadMass = 350.f;  //
//Scalar loadMass = 10.f;//this should work fine for the SI solver

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

#ifndef M_PI_4
#define M_PI_4 0.785398163397448309616
#endif

i32 rightIndex = 0;
i32 upIndex = 1;
i32 forwardIndex = 2;
Vec3 wheelDirectionCS0(0, -1, 0);
Vec3 wheelAxleCS(-1, 0, 0);

bool useMCLPSolver = true;

#include <stdio.h>  //printf debugging

#include "../ForkLiftDemo.h"

///RaycastVehicle is the interface for the constraint that implements the raycast vehicle
///notice that for higher-quality slow-moving vehicles, another approach might be better
///implementing explicit hinged-wheel constraints with cylinder collision, rather then raycasts
float gEngineForce = 0.f;

float defaultBreakingForce = 10.f;
float gBreakingForce = 100.f;

float maxEngineForce = 1000.f;  //this should be engine/velocity dependent
float maxBreakingForce = 100.f;

float gVehicleSteering = 0.f;
float steeringIncrement = 0.04f;
float steeringClamp = 0.3f;
float wheelRadius = 0.5f;
float wheelWidth = 0.4f;
float wheelFriction = 1000;  //DRX3D_LARGE_FLOAT;
float suspensionStiffness = 20.f;
float suspensionDamping = 2.3f;
float suspensionCompression = 4.4f;
float rollInfluence = 0.1f;  //1.0f;

Scalar suspensionRestLength(0.6);

#define CUBE_HALF_EXTENTS 1

////////////////////////////////////

ForkLiftDemo::ForkLiftDemo(struct GUIHelperInterface* helper)
	: m_guiHelper(helper),
	  m_carChassis(0),
	  m_liftBody(0),
	  m_forkBody(0),
	  m_loadBody(0),
	  m_indexVertexArrays(0),
	  m_vertices(0),
	  m_cameraHeight(4.f),
	  m_minCameraDistance(3.f),
	  m_maxCameraDistance(10.f)
{
	helper->setUpAxis(1);
	m_vehicle = 0;
	m_wheelShape = 0;
	m_cameraPosition = Vec3(30, 30, 30);
	m_useDefaultCamera = false;
	//	setTexturing(true);
	//	setShadows(true);
}

void ForkLiftDemo::exitPhysics()
{
	//cleanup in the reverse order of creation/initialization

	//remove the rigidbodies from the dynamics world and delete them
	i32 i;
	for (i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		CollisionObject2* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		RigidBody* body = RigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			while (body->getNumConstraintRefs())
			{
				TypedConstraint* constraint = body->getConstraintRef(0);
				m_dynamicsWorld->removeConstraint(constraint);
				delete constraint;
			}
			delete body->getMotionState();
			m_dynamicsWorld->removeRigidBody(body);
		}
		else
		{
			m_dynamicsWorld->removeCollisionObject(obj);
		}
		delete obj;
	}

	//delete collision shapes
	for (i32 j = 0; j < m_collisionShapes.size(); j++)
	{
		CollisionShape* shape = m_collisionShapes[j];
		delete shape;
	}
	m_collisionShapes.clear();

	delete m_indexVertexArrays;
	delete m_vertices;

	//delete dynamics world
	delete m_dynamicsWorld;
	m_dynamicsWorld = 0;

	delete m_vehicleRayCaster;
	m_vehicleRayCaster = 0;

	delete m_vehicle;
	m_vehicle = 0;

	delete m_wheelShape;
	m_wheelShape = 0;

	//delete solver
	delete m_constraintSolver;
	m_constraintSolver = 0;

	//delete broadphase
	delete m_overlappingPairCache;
	m_overlappingPairCache = 0;

	//delete dispatcher
	delete m_dispatcher;
	m_dispatcher = 0;

	delete m_collisionConfiguration;
	m_collisionConfiguration = 0;
}

ForkLiftDemo::~ForkLiftDemo()
{
	//exitPhysics();
}

void ForkLiftDemo::initPhysics()
{
	i32 upAxis = 1;

	m_guiHelper->setUpAxis(upAxis);

	Vec3 groundExtents(50, 50, 50);
	groundExtents[upAxis] = 3;
	CollisionShape* groundShape = new BoxShape(groundExtents);
	m_collisionShapes.push_back(groundShape);
	m_collisionConfiguration = new DefaultCollisionConfiguration();
	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);
	Vec3 worldMin(-1000, -1000, -1000);
	Vec3 worldMax(1000, 1000, 1000);
	m_overlappingPairCache = new AxisSweep3(worldMin, worldMax);
	if (useMCLPSolver)
	{
		DantzigSolver* mlcp = new DantzigSolver();
		//SolveProjectedGaussSeidel* mlcp = new SolveProjectedGaussSeidel;
		MLCPSolver* sol = new MLCPSolver(mlcp);
		m_constraintSolver = sol;
	}
	else
	{
		m_constraintSolver = new SequentialImpulseConstraintSolver();
	}
	m_dynamicsWorld = new DiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_constraintSolver, m_collisionConfiguration);
	if (useMCLPSolver)
	{
		m_dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 1;  //for direct solver it is better to have a small A matrix
	}
	else
	{
		m_dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 128;  //for direct solver, it is better to solve multiple objects together, small batches have high overhead
	}
	m_dynamicsWorld->getSolverInfo().m_globalCfm = 0.00001;

	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	//m_dynamicsWorld->setGravity(Vec3(0,0,0));
	Transform2 tr;
	tr.setIdentity();
	tr.setOrigin(Vec3(0, -3, 0));

	//either use heightfield or triangle mesh

	//create ground object
	localCreateRigidBody(0, tr, groundShape);

	CollisionShape* chassisShape = new BoxShape(Vec3(1.f, 0.5f, 2.f));
	m_collisionShapes.push_back(chassisShape);

	CompoundShape* compound = new CompoundShape();
	m_collisionShapes.push_back(compound);
	Transform2 localTrans;
	localTrans.setIdentity();
	//localTrans effectively shifts the center of mass with respect to the chassis
	localTrans.setOrigin(Vec3(0, 1, 0));

	compound->addChildShape(localTrans, chassisShape);

	{
		CollisionShape* suppShape = new BoxShape(Vec3(0.5f, 0.1f, 0.5f));
		Transform2 suppLocalTrans;
		suppLocalTrans.setIdentity();
		//localTrans effectively shifts the center of mass with respect to the chassis
		suppLocalTrans.setOrigin(Vec3(0, 1.0, 2.5));
		compound->addChildShape(suppLocalTrans, suppShape);
	}

	tr.setOrigin(Vec3(0, 0.f, 0));

	m_carChassis = localCreateRigidBody(800, tr, compound);  //chassisShape);
	//m_carChassis->setDamping(0.2,0.2);

	m_wheelShape = new CylinderShapeX(Vec3(wheelWidth, wheelRadius, wheelRadius));

	m_guiHelper->createCollisionShapeGraphicsObject(m_wheelShape);
	i32 wheelGraphicsIndex = m_wheelShape->getUserIndex();

	const float position[4] = {0, 10, 10, 0};
	const float quaternion[4] = {0, 0, 0, 1};
	const float color[4] = {0, 1, 0, 1};
	const float scaling[4] = {1, 1, 1, 1};

	for (i32 i = 0; i < 4; i++)
	{
		m_wheelInstances[i] = m_guiHelper->registerGraphicsInstance(wheelGraphicsIndex, position, quaternion, color, scaling);
	}

	{
		CollisionShape* liftShape = new BoxShape(Vec3(0.5f, 2.0f, 0.05f));
		m_collisionShapes.push_back(liftShape);
		Transform2 liftTrans;
		m_liftStartPos = Vec3(0.0f, 2.5f, 3.05f);
		liftTrans.setIdentity();
		liftTrans.setOrigin(m_liftStartPos);
		m_liftBody = localCreateRigidBody(10, liftTrans, liftShape);

		Transform2 localA, localB;
		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, M_PI_2, 0);
		localA.setOrigin(Vec3(0.0, 1.0, 3.05));
		localB.getBasis().setEulerZYX(0, M_PI_2, 0);
		localB.setOrigin(Vec3(0.0, -1.5, -0.05));
		m_liftHinge = new HingeConstraint(*m_carChassis, *m_liftBody, localA, localB);
		//		m_liftHinge->setLimit(-LIFT_EPS, LIFT_EPS);
		m_liftHinge->setLimit(0.0f, 0.0f);
		m_dynamicsWorld->addConstraint(m_liftHinge, true);

		CollisionShape* forkShapeA = new BoxShape(Vec3(1.0f, 0.1f, 0.1f));
		m_collisionShapes.push_back(forkShapeA);
		CompoundShape* forkCompound = new CompoundShape();
		m_collisionShapes.push_back(forkCompound);
		Transform2 forkLocalTrans;
		forkLocalTrans.setIdentity();
		forkCompound->addChildShape(forkLocalTrans, forkShapeA);

		CollisionShape* forkShapeB = new BoxShape(Vec3(0.1f, 0.02f, 0.6f));
		m_collisionShapes.push_back(forkShapeB);
		forkLocalTrans.setIdentity();
		forkLocalTrans.setOrigin(Vec3(-0.9f, -0.08f, 0.7f));
		forkCompound->addChildShape(forkLocalTrans, forkShapeB);

		CollisionShape* forkShapeC = new BoxShape(Vec3(0.1f, 0.02f, 0.6f));
		m_collisionShapes.push_back(forkShapeC);
		forkLocalTrans.setIdentity();
		forkLocalTrans.setOrigin(Vec3(0.9f, -0.08f, 0.7f));
		forkCompound->addChildShape(forkLocalTrans, forkShapeC);

		Transform2 forkTrans;
		m_forkStartPos = Vec3(0.0f, 0.6f, 3.2f);
		forkTrans.setIdentity();
		forkTrans.setOrigin(m_forkStartPos);
		m_forkBody = localCreateRigidBody(5, forkTrans, forkCompound);

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, 0, M_PI_2);
		localA.setOrigin(Vec3(0.0f, -1.9f, 0.05f));
		localB.getBasis().setEulerZYX(0, 0, M_PI_2);
		localB.setOrigin(Vec3(0.0, 0.0, -0.1));
		m_forkSlider = new SliderConstraint(*m_liftBody, *m_forkBody, localA, localB, true);
		m_forkSlider->setLowerLinLimit(0.1f);
		m_forkSlider->setUpperLinLimit(0.1f);
		//		m_forkSlider->setLowerAngLimit(-LIFT_EPS);
		//		m_forkSlider->setUpperAngLimit(LIFT_EPS);
		m_forkSlider->setLowerAngLimit(0.0f);
		m_forkSlider->setUpperAngLimit(0.0f);
		m_dynamicsWorld->addConstraint(m_forkSlider, true);

		CompoundShape* loadCompound = new CompoundShape();
		m_collisionShapes.push_back(loadCompound);
		CollisionShape* loadShapeA = new BoxShape(Vec3(2.0f, 0.5f, 0.5f));
		m_collisionShapes.push_back(loadShapeA);
		Transform2 loadTrans;
		loadTrans.setIdentity();
		loadCompound->addChildShape(loadTrans, loadShapeA);
		CollisionShape* loadShapeB = new BoxShape(Vec3(0.1f, 1.0f, 1.0f));
		m_collisionShapes.push_back(loadShapeB);
		loadTrans.setIdentity();
		loadTrans.setOrigin(Vec3(2.1f, 0.0f, 0.0f));
		loadCompound->addChildShape(loadTrans, loadShapeB);
		CollisionShape* loadShapeC = new BoxShape(Vec3(0.1f, 1.0f, 1.0f));
		m_collisionShapes.push_back(loadShapeC);
		loadTrans.setIdentity();
		loadTrans.setOrigin(Vec3(-2.1f, 0.0f, 0.0f));
		loadCompound->addChildShape(loadTrans, loadShapeC);
		loadTrans.setIdentity();
		m_loadStartPos = Vec3(0.0f, 3.5f, 7.0f);
		loadTrans.setOrigin(m_loadStartPos);
		m_loadBody = localCreateRigidBody(loadMass, loadTrans, loadCompound);
	}

	/// create vehicle
	{
		m_vehicleRayCaster = new DefaultVehicleRaycaster(m_dynamicsWorld);
		m_vehicle = new RaycastVehicle(m_tuning, m_carChassis, m_vehicleRayCaster);

		///never deactivate the vehicle
		m_carChassis->setActivationState(DISABLE_DEACTIVATION);

		m_dynamicsWorld->addVehicle(m_vehicle);

		float connectionHeight = 1.2f;

		bool isFrontWheel = true;

		//choose coordinate system
		m_vehicle->setCoordinateSystem(rightIndex, upIndex, forwardIndex);

		Vec3 connectionPointCS0(CUBE_HALF_EXTENTS - (0.3 * wheelWidth), connectionHeight, 2 * CUBE_HALF_EXTENTS - wheelRadius);

		m_vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, m_tuning, isFrontWheel);
		connectionPointCS0 = Vec3(-CUBE_HALF_EXTENTS + (0.3 * wheelWidth), connectionHeight, 2 * CUBE_HALF_EXTENTS - wheelRadius);

		m_vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, m_tuning, isFrontWheel);
		connectionPointCS0 = Vec3(-CUBE_HALF_EXTENTS + (0.3 * wheelWidth), connectionHeight, -2 * CUBE_HALF_EXTENTS + wheelRadius);
		isFrontWheel = false;
		m_vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, m_tuning, isFrontWheel);
		connectionPointCS0 = Vec3(CUBE_HALF_EXTENTS - (0.3 * wheelWidth), connectionHeight, -2 * CUBE_HALF_EXTENTS + wheelRadius);
		m_vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, m_tuning, isFrontWheel);

		for (i32 i = 0; i < m_vehicle->getNumWheels(); i++)
		{
			WheelInfo& wheel = m_vehicle->getWheelInfo(i);
			wheel.m_suspensionStiffness = suspensionStiffness;
			wheel.m_wheelsDampingRelaxation = suspensionDamping;
			wheel.m_wheelsDampingCompression = suspensionCompression;
			wheel.m_frictionSlip = wheelFriction;
			wheel.m_rollInfluence = rollInfluence;
		}
	}

	resetForklift();

	//	setCameraDistance(26.f);

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void ForkLiftDemo::physicsDebugDraw(i32 debugFlags)
{
	if (m_dynamicsWorld && m_dynamicsWorld->getDebugDrawer())
	{
		m_dynamicsWorld->getDebugDrawer()->setDebugMode(debugFlags);
		m_dynamicsWorld->debugDrawWorld();
	}
}

//to be implemented by the demo
void ForkLiftDemo::renderScene()
{
	m_guiHelper->syncPhysicsToGraphics(m_dynamicsWorld);

	for (i32 i = 0; i < m_vehicle->getNumWheels(); i++)
	{
		//synchronize the wheels with the (interpolated) chassis worldtransform
		m_vehicle->updateWheelTransform(i, true);

		CommonRenderInterface* renderer = m_guiHelper->getRenderInterface();
		if (renderer)
		{
			Transform2 tr = m_vehicle->getWheelInfo(i).m_worldTransform;
			Vec3 pos = tr.getOrigin();
			Quat orn = tr.getRotation();
			renderer->writeSingleInstanceTransformToCPU(pos, orn, m_wheelInstances[i]);
		}
	}

	m_guiHelper->render(m_dynamicsWorld);

	ATTRIBUTE_ALIGNED16(Scalar)
	m[16];
	i32 i;

	Vec3 wheelColor(1, 0, 0);

	Vec3 worldBoundsMin, worldBoundsMax;
	getDynamicsWorld()->getBroadphase()->getBroadphaseAabb(worldBoundsMin, worldBoundsMax);

	for (i = 0; i < m_vehicle->getNumWheels(); i++)
	{
		//synchronize the wheels with the (interpolated) chassis worldtransform
		m_vehicle->updateWheelTransform(i, true);
		//draw wheels (cylinders)
		m_vehicle->getWheelInfo(i).m_worldTransform.getOpenGLMatrix(m);
		//		m_shapeDrawer->drawOpenGL(m,m_wheelShape,wheelColor,getDebugMode(),worldBoundsMin,worldBoundsMax);
	}

#if 0
	i32 lineWidth=400;
	i32 xStart = m_glutScreenWidth - lineWidth;
	i32 yStart = 20;

	if((getDebugMode() & btIDebugDraw::DBG_NoHelpText)==0)
	{
		setOrthographicProjection();
		glDisable(GL_LIGHTING);
		glColor3f(0, 0, 0);
		char buf[124];
		
		sprintf(buf,"SHIFT+Cursor Left/Right - rotate lift");
		GLDebugDrawString(xStart,20,buf);
		yStart+=20;
		sprintf(buf,"SHIFT+Cursor UP/Down - fork up/down");
		yStart+=20;
		GLDebugDrawString(xStart,yStart,buf);

		if (m_useDefaultCamera)
		{
			sprintf(buf,"F5 - camera mode (free)");
		} else
		{
			sprintf(buf,"F5 - camera mode (follow)");
		}
		yStart+=20;
		GLDebugDrawString(xStart,yStart,buf);

		yStart+=20;
		if (m_dynamicsWorld->getConstraintSolver()->getSolverType()==DRX3D_MLCP_SOLVER)
		{
			sprintf(buf,"F6 - solver (direct MLCP)");
		} else
		{
			sprintf(buf,"F6 - solver (sequential impulse)");
		}
		GLDebugDrawString(xStart,yStart,buf);
		DiscreteDynamicsWorld* world = (DiscreteDynamicsWorld*) m_dynamicsWorld;
		if (world->getLatencyMotionStateInterpolation())
		{
			sprintf(buf,"F7 - motionstate interpolation (on)");
		} else
		{
			sprintf(buf,"F7 - motionstate interpolation (off)");
		}
		yStart+=20;
		GLDebugDrawString(xStart,yStart,buf);

		sprintf(buf,"Click window for keyboard focus");
		yStart+=20;
		GLDebugDrawString(xStart,yStart,buf);


		resetPerspectiveProjection();
		glEnable(GL_LIGHTING);
	}
#endif
}

void ForkLiftDemo::stepSimulation(float deltaTime)
{
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	{
		i32 wheelIndex = 2;
		m_vehicle->applyEngineForce(gEngineForce, wheelIndex);
		m_vehicle->setBrake(gBreakingForce, wheelIndex);
		wheelIndex = 3;
		m_vehicle->applyEngineForce(gEngineForce, wheelIndex);
		m_vehicle->setBrake(gBreakingForce, wheelIndex);

		wheelIndex = 0;
		m_vehicle->setSteeringValue(gVehicleSteering, wheelIndex);
		wheelIndex = 1;
		m_vehicle->setSteeringValue(gVehicleSteering, wheelIndex);
	}

	float dt = deltaTime;

	if (m_dynamicsWorld)
	{
		//during idle mode, just run 1 simulation step maximum
		i32 maxSimSubSteps = 2;

		i32 numSimSteps;
		numSimSteps = m_dynamicsWorld->stepSimulation(dt, maxSimSubSteps);

		if (m_dynamicsWorld->getConstraintSolver()->getSolverType() == DRX3D_MLCP_SOLVER)
		{
			MLCPSolver* sol = (MLCPSolver*)m_dynamicsWorld->getConstraintSolver();
			i32 numFallbacks = sol->getNumFallbacks();
			if (numFallbacks)
			{
				static i32 totalFailures = 0;
				totalFailures += numFallbacks;
				printf("MLCP solver failed %d times, falling back to SequentialImpulseSolver (SI)\n", totalFailures);
			}
			sol->setNumFallbacks(0);
		}

//#define VERBOSE_FEEDBACK
#ifdef VERBOSE_FEEDBACK
		if (!numSimSteps)
			printf("Interpolated transforms\n");
		else
		{
			if (numSimSteps > maxSimSubSteps)
			{
				//detect dropping frames
				printf("Dropped (%i) simulation steps out of %i\n", numSimSteps - maxSimSubSteps, numSimSteps);
			}
			else
			{
				printf("Simulated (%i) steps\n", numSimSteps);
			}
		}
#endif  //VERBOSE_FEEDBACK
	}
}

void ForkLiftDemo::displayCallback(void)
{
	//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//renderme();

	//optional but useful: debug drawing
	if (m_dynamicsWorld)
		m_dynamicsWorld->debugDrawWorld();

	//	glFlush();
	//	glutSwapBuffers();
}

void ForkLiftDemo::clientResetScene()
{
	exitPhysics();
	initPhysics();
}

void ForkLiftDemo::resetForklift()
{
	gVehicleSteering = 0.f;
	gBreakingForce = defaultBreakingForce;
	gEngineForce = 0.f;

	m_carChassis->setCenterOfMassTransform(Transform2::getIdentity());
	m_carChassis->setLinearVelocity(Vec3(0, 0, 0));
	m_carChassis->setAngularVelocity(Vec3(0, 0, 0));
	m_dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(m_carChassis->getBroadphaseHandle(), getDynamicsWorld()->getDispatcher());
	if (m_vehicle)
	{
		m_vehicle->resetSuspension();
		for (i32 i = 0; i < m_vehicle->getNumWheels(); i++)
		{
			//synchronize the wheels with the (interpolated) chassis worldtransform
			m_vehicle->updateWheelTransform(i, true);
		}
	}
	Transform2 liftTrans;
	liftTrans.setIdentity();
	liftTrans.setOrigin(m_liftStartPos);
	m_liftBody->activate();
	m_liftBody->setCenterOfMassTransform(liftTrans);
	m_liftBody->setLinearVelocity(Vec3(0, 0, 0));
	m_liftBody->setAngularVelocity(Vec3(0, 0, 0));

	Transform2 forkTrans;
	forkTrans.setIdentity();
	forkTrans.setOrigin(m_forkStartPos);
	m_forkBody->activate();
	m_forkBody->setCenterOfMassTransform(forkTrans);
	m_forkBody->setLinearVelocity(Vec3(0, 0, 0));
	m_forkBody->setAngularVelocity(Vec3(0, 0, 0));

	//	m_liftHinge->setLimit(-LIFT_EPS, LIFT_EPS);
	m_liftHinge->setLimit(0.0f, 0.0f);
	m_liftHinge->enableAngularMotor(false, 0, 0);

	m_forkSlider->setLowerLinLimit(0.1f);
	m_forkSlider->setUpperLinLimit(0.1f);
	m_forkSlider->setPoweredLinMotor(false);

	Transform2 loadTrans;
	loadTrans.setIdentity();
	loadTrans.setOrigin(m_loadStartPos);
	m_loadBody->activate();
	m_loadBody->setCenterOfMassTransform(loadTrans);
	m_loadBody->setLinearVelocity(Vec3(0, 0, 0));
	m_loadBody->setAngularVelocity(Vec3(0, 0, 0));
}

bool ForkLiftDemo::keyboardCallback(i32 key, i32 state)
{
	bool handled = false;
	bool isShiftPressed = m_guiHelper->getAppInterface()->m_window->isModifierKeyPressed(B3G_SHIFT);

	if (state)
	{
		if (isShiftPressed)
		{
			switch (key)
			{
				case B3G_LEFT_ARROW:
				{
					m_liftHinge->setLimit(-M_PI / 16.0f, M_PI / 8.0f);
					m_liftHinge->enableAngularMotor(true, -0.1, maxMotorImpulse);
					handled = true;
					break;
				}
				case B3G_RIGHT_ARROW:
				{
					m_liftHinge->setLimit(-M_PI / 16.0f, M_PI / 8.0f);
					m_liftHinge->enableAngularMotor(true, 0.1, maxMotorImpulse);
					handled = true;
					break;
				}
				case B3G_UP_ARROW:
				{
					m_forkSlider->setLowerLinLimit(0.1f);
					m_forkSlider->setUpperLinLimit(3.9f);
					m_forkSlider->setPoweredLinMotor(true);
					m_forkSlider->setMaxLinMotorForce(maxMotorImpulse);
					m_forkSlider->setTargetLinMotorVelocity(1.0);
					handled = true;
					break;
				}
				case B3G_DOWN_ARROW:
				{
					m_forkSlider->setLowerLinLimit(0.1f);
					m_forkSlider->setUpperLinLimit(3.9f);
					m_forkSlider->setPoweredLinMotor(true);
					m_forkSlider->setMaxLinMotorForce(maxMotorImpulse);
					m_forkSlider->setTargetLinMotorVelocity(-1.0);
					handled = true;
					break;
				}
			}
		}
		else
		{
			switch (key)
			{
				case B3G_LEFT_ARROW:
				{
					handled = true;
					gVehicleSteering += steeringIncrement;
					if (gVehicleSteering > steeringClamp)
						gVehicleSteering = steeringClamp;

					break;
				}
				case B3G_RIGHT_ARROW:
				{
					handled = true;
					gVehicleSteering -= steeringIncrement;
					if (gVehicleSteering < -steeringClamp)
						gVehicleSteering = -steeringClamp;

					break;
				}
				case B3G_UP_ARROW:
				{
					handled = true;
					gEngineForce = maxEngineForce;
					gBreakingForce = 0.f;
					break;
				}
				case B3G_DOWN_ARROW:
				{
					handled = true;
					gEngineForce = -maxEngineForce;
					gBreakingForce = 0.f;
					break;
				}

				case B3G_F7:
				{
					handled = true;
					DiscreteDynamicsWorld* world = (DiscreteDynamicsWorld*)m_dynamicsWorld;
					world->setLatencyMotionStateInterpolation(!world->getLatencyMotionStateInterpolation());
					printf("world latencyMotionStateInterpolation = %d\n", world->getLatencyMotionStateInterpolation());
					break;
				}
				case B3G_F6:
				{
					handled = true;
					//switch solver (needs demo restart)
					useMCLPSolver = !useMCLPSolver;
					printf("switching to useMLCPSolver = %d\n", useMCLPSolver);

					delete m_constraintSolver;
					if (useMCLPSolver)
					{
						DantzigSolver* mlcp = new DantzigSolver();
						//SolveProjectedGaussSeidel* mlcp = new SolveProjectedGaussSeidel;
						MLCPSolver* sol = new MLCPSolver(mlcp);
						m_constraintSolver = sol;
					}
					else
					{
						m_constraintSolver = new SequentialImpulseConstraintSolver();
					}

					m_dynamicsWorld->setConstraintSolver(m_constraintSolver);

					//exitPhysics();
					//initPhysics();
					break;
				}

				case B3G_F5:
					handled = true;
					m_useDefaultCamera = !m_useDefaultCamera;
					break;
				default:
					break;
			}
		}
	}
	else
	{
		switch (key)
		{
			case B3G_UP_ARROW:
			{
				lockForkSlider();
				gEngineForce = 0.f;
				gBreakingForce = defaultBreakingForce;
				handled = true;
				break;
			}
			case B3G_DOWN_ARROW:
			{
				lockForkSlider();
				gEngineForce = 0.f;
				gBreakingForce = defaultBreakingForce;
				handled = true;
				break;
			}
			case B3G_LEFT_ARROW:
			case B3G_RIGHT_ARROW:
			{
				lockLiftHinge();
				handled = true;
				break;
			}
			default:

				break;
		}
	}
	return handled;
}

void ForkLiftDemo::specialKeyboardUp(i32 key, i32 x, i32 y)
{
#if 0

#endif
}

void ForkLiftDemo::specialKeyboard(i32 key, i32 x, i32 y)
{
#if 0
	if (key==GLUT_KEY_END)
		return;

	//	printf("key = %i x=%i y=%i\n",key,x,y);

	i32 state;
	state=glutGetModifiers();
	if (state & GLUT_ACTIVE_SHIFT) 
	{
		switch (key) 
			{
			case GLUT_KEY_LEFT : 
				{
				
					m_liftHinge->setLimit(-M_PI/16.0f, M_PI/8.0f);
					m_liftHinge->enableAngularMotor(true, -0.1, maxMotorImpulse);
					break;
				}
			case GLUT_KEY_RIGHT : 
				{
					
					m_liftHinge->setLimit(-M_PI/16.0f, M_PI/8.0f);
					m_liftHinge->enableAngularMotor(true, 0.1, maxMotorImpulse);
					break;
				}
			case GLUT_KEY_UP :
				{
					m_forkSlider->setLowerLinLimit(0.1f);
					m_forkSlider->setUpperLinLimit(3.9f);
					m_forkSlider->setPoweredLinMotor(true);
					m_forkSlider->setMaxLinMotorForce(maxMotorImpulse);
					m_forkSlider->setTargetLinMotorVelocity(1.0);
					break;
				}
			case GLUT_KEY_DOWN :
				{
					m_forkSlider->setLowerLinLimit(0.1f);
					m_forkSlider->setUpperLinLimit(3.9f);
					m_forkSlider->setPoweredLinMotor(true);
					m_forkSlider->setMaxLinMotorForce(maxMotorImpulse);
					m_forkSlider->setTargetLinMotorVelocity(-1.0);
					break;
				}

			default:
				DemoApplication::specialKeyboard(key,x,y);
				break;
			}

	} else
	{
			switch (key) 
			{
			case GLUT_KEY_LEFT : 
				{
					gVehicleSteering += steeringIncrement;
					if (	gVehicleSteering > steeringClamp)
						gVehicleSteering = steeringClamp;

					break;
				}
			case GLUT_KEY_RIGHT : 
				{
					gVehicleSteering -= steeringIncrement;
					if (	gVehicleSteering < -steeringClamp)
						gVehicleSteering = -steeringClamp;

					break;
				}
			case GLUT_KEY_UP :
				{
					gEngineForce = maxEngineForce;
					gBreakingForce = 0.f;
					break;
				}
			case GLUT_KEY_DOWN :
				{
					gEngineForce = -maxEngineForce;
					gBreakingForce = 0.f;
					break;
				}

			case GLUT_KEY_F7:
				{
					DiscreteDynamicsWorld* world = (DiscreteDynamicsWorld*)m_dynamicsWorld;
					world->setLatencyMotionStateInterpolation(!world->getLatencyMotionStateInterpolation());
					printf("world latencyMotionStateInterpolation = %d\n", world->getLatencyMotionStateInterpolation());
					break;
				}
			case GLUT_KEY_F6:
				{
					//switch solver (needs demo restart)
					useMCLPSolver = !useMCLPSolver;
					printf("switching to useMLCPSolver = %d\n", useMCLPSolver);

					delete m_constraintSolver;
					if (useMCLPSolver)
					{
						btDantzigSolver* mlcp = new btDantzigSolver();
						//SolveProjectedGaussSeidel* mlcp = new SolveProjectedGaussSeidel;
						btMLCPSolver* sol = new btMLCPSolver(mlcp);
						m_constraintSolver = sol;
					} else
					{
						m_constraintSolver = new SequentialImpulseConstraintSolver();
					}

					m_dynamicsWorld->setConstraintSolver(m_constraintSolver);


					//exitPhysics();
					//initPhysics();
					break;
				}

			case GLUT_KEY_F5:
				m_useDefaultCamera = !m_useDefaultCamera;
				break;
			default:
				DemoApplication::specialKeyboard(key,x,y);
				break;
			}

	}
	//	glutPostRedisplay();

#endif
}

void ForkLiftDemo::lockLiftHinge(void)
{
	Scalar hingeAngle = m_liftHinge->getHingeAngle();
	Scalar lowLim = m_liftHinge->getLowerLimit();
	Scalar hiLim = m_liftHinge->getUpperLimit();
	m_liftHinge->enableAngularMotor(false, 0, 0);
	if (hingeAngle < lowLim)
	{
		//		m_liftHinge->setLimit(lowLim, lowLim + LIFT_EPS);
		m_liftHinge->setLimit(lowLim, lowLim);
	}
	else if (hingeAngle > hiLim)
	{
		//		m_liftHinge->setLimit(hiLim - LIFT_EPS, hiLim);
		m_liftHinge->setLimit(hiLim, hiLim);
	}
	else
	{
		//		m_liftHinge->setLimit(hingeAngle - LIFT_EPS, hingeAngle + LIFT_EPS);
		m_liftHinge->setLimit(hingeAngle, hingeAngle);
	}
	return;
}  // ForkLiftDemo::lockLiftHinge()

void ForkLiftDemo::lockForkSlider(void)
{
	Scalar linDepth = m_forkSlider->getLinearPos();
	Scalar lowLim = m_forkSlider->getLowerLinLimit();
	Scalar hiLim = m_forkSlider->getUpperLinLimit();
	m_forkSlider->setPoweredLinMotor(false);
	if (linDepth <= lowLim)
	{
		m_forkSlider->setLowerLinLimit(lowLim);
		m_forkSlider->setUpperLinLimit(lowLim);
	}
	else if (linDepth > hiLim)
	{
		m_forkSlider->setLowerLinLimit(hiLim);
		m_forkSlider->setUpperLinLimit(hiLim);
	}
	else
	{
		m_forkSlider->setLowerLinLimit(linDepth);
		m_forkSlider->setUpperLinLimit(linDepth);
	}
	return;
}  // ForkLiftDemo::lockForkSlider()

RigidBody* ForkLiftDemo::localCreateRigidBody(Scalar mass, const Transform2& startTransform, CollisionShape* shape)
{
	Assert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	Vec3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

#define USE_MOTIONSTATE 1
#ifdef USE_MOTIONSTATE
	DefaultMotionState* myMotionState = new DefaultMotionState(startTransform);

	RigidBody::RigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

	RigidBody* body = new RigidBody(cInfo);
	//body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

#else
	RigidBody* body = new RigidBody(mass, 0, shape, localInertia);
	body->setWorldTransform(startTransform);
#endif  //

	m_dynamicsWorld->addRigidBody(body);
	return body;
}

CommonExampleInterface* ForkLiftCreateFunc(struct CommonExampleOptions& options)
{
	return new ForkLiftDemo(options.m_guiHelper);
}
