
#include "../Hinge2Vehicle.h"

#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/Collision/Shapes/HeightfieldTerrainShape.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/DantzigSolver.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/SolveProjectedGaussSeidel.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/MLCPSolver.h>

class VehicleTuning;

class CollisionShape;

#include <drx3D/Physics/Dynamics/ConstraintSolver/HingeConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SliderConstraint.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/CollisionCommon.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/CommonWindowInterface.h>
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

class Hinge2Vehicle : public CommonRigidBodyBase
{
public:
	/* extra stuff*/
	Vec3 m_cameraPosition;

	RigidBody* m_carChassis;
	RigidBody* localCreateRigidBody(Scalar mass, const Transform2& worldTransform, CollisionShape* colSape);

	GUIHelperInterface* m_guiHelper;
	i32 m_wheelInstances[4];

	bool m_useDefaultCamera;
	//----------------------------

	class TriangleIndexVertexArray* m_indexVertexArrays;

	Vec3* m_vertices;

	CollisionShape* m_wheelShape;

	float m_cameraHeight;

	float m_minCameraDistance;
	float m_maxCameraDistance;

	Hinge2Vehicle(struct GUIHelperInterface* helper);

	virtual ~Hinge2Vehicle();

	virtual void stepSimulation(float deltaTime);

	virtual void resetForklift();

	virtual void clientResetScene();

	virtual void displayCallback();

	virtual void specialKeyboard(i32 key, i32 x, i32 y);

	virtual void specialKeyboardUp(i32 key, i32 x, i32 y);

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
		float targetPos[3] = {0,0,2};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}

	/*static DemoApplication* Create()
	{
		Hinge2Vehicle* demo = new Hinge2Vehicle();
		demo->myinit();
		demo->initPhysics();
		return demo;
	}
	*/
};

static Scalar maxMotorImpulse = 4000.f;


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

#ifndef M_PI_4
#define M_PI_4 0.785398163397448309616
#endif

//static i32 rightIndex = 0;
//static i32 upIndex = 1;
//static i32 forwardIndex = 2;
static Vec3 wheelDirectionCS0(0, -1, 0);
static Vec3 wheelAxleCS(-1, 0, 0);

static bool useMCLPSolver = false;  //true;

#include <stdio.h>  //printf debugging

#include "../Hinge2Vehicle.h"

//static i32k maxProxies = 32766;
//static i32k maxOverlap = 65535;

static float gEngineForce = 0.f;

static float defaultBreakingForce = 10.f;
static float gBreakingForce = 100.f;

static float maxEngineForce = 1000.f;  //this should be engine/velocity dependent
//static float	maxBreakingForce = 100.f;

static float gVehicleSteering = 0.f;
static float steeringIncrement = 0.04f;
static float steeringClamp = 0.3f;
static float wheelRadius = 0.5f;
static float wheelWidth = 0.4f;
//static float	wheelFriction = 1000;//DRX3D_LARGE_FLOAT;
//static float	suspensionStiffness = 20.f;
//static float	suspensionDamping = 2.3f;
//static float	suspensionCompression = 4.4f;
//static float	rollInfluence = 0.1f;//1.0f;

//static Scalar suspensionRestLength(0.6);

#define CUBE_HALF_EXTENTS 1

////////////////////////////////////

Hinge2Vehicle::Hinge2Vehicle(struct GUIHelperInterface* helper)
	: CommonRigidBodyBase(helper),
	  m_carChassis(0),
	  m_guiHelper(helper),
	  m_indexVertexArrays(0),
	  m_vertices(0),
	  m_cameraHeight(4.f),
	  m_minCameraDistance(3.f),
	  m_maxCameraDistance(10.f)
{
	helper->setUpAxis(1);

	m_wheelShape = 0;
	m_cameraPosition = Vec3(30, 30, 30);
	m_useDefaultCamera = false;
}

void Hinge2Vehicle::exitPhysics()
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

	delete m_wheelShape;
	m_wheelShape = 0;

	//delete solver
	delete m_solver;
	m_solver = 0;

	//delete broadphase
	delete m_broadphase;
	m_broadphase = 0;

	//delete dispatcher
	delete m_dispatcher;
	m_dispatcher = 0;

	delete m_collisionConfiguration;
	m_collisionConfiguration = 0;
}

Hinge2Vehicle::~Hinge2Vehicle()
{
	//exitPhysics();
}

void Hinge2Vehicle::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	CollisionShape* groundShape = new BoxShape(Vec3(50, 3, 50));
	m_collisionShapes.push_back(groundShape);
	m_collisionConfiguration = new DefaultCollisionConfiguration();
	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);
	Vec3 worldMin(-1000, -1000, -1000);
	Vec3 worldMax(1000, 1000, 1000);
	m_broadphase = new AxisSweep3(worldMin, worldMax);
	if (useMCLPSolver)
	{
		DantzigSolver* mlcp = new DantzigSolver();
		//SolveProjectedGaussSeidel* mlcp = new SolveProjectedGaussSeidel;
		MLCPSolver* sol = new MLCPSolver(mlcp);
		m_solver = sol;
	}
	else
	{
		m_solver = new SequentialImpulseConstraintSolver();
	}
	m_dynamicsWorld = new DiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
	if (useMCLPSolver)
	{
		m_dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 1;  //for direct solver it is better to have a small A matrix
	}
	else
	{
		m_dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 128;  //for direct solver, it is better to solve multiple objects together, small batches have high overhead
	}
	m_dynamicsWorld->getSolverInfo().m_numIterations = 100;
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

	const Scalar FALLHEIGHT = 5;
	tr.setOrigin(Vec3(0, FALLHEIGHT, 0));

	const Scalar chassisMass = 2.0f;
	const Scalar wheelMass = 1.0f;
	m_carChassis = localCreateRigidBody(chassisMass, tr, compound);  //chassisShape);
	//m_carChassis->setDamping(0.2,0.2);

	//m_wheelShape = new CylinderShapeX(Vec3(wheelWidth,wheelRadius,wheelRadius));
	m_wheelShape = new CylinderShapeX(Vec3(wheelWidth, wheelRadius, wheelRadius));

	Vec3 wheelPos[4] = {
		Vec3(Scalar(-1.), Scalar(FALLHEIGHT-0.25), Scalar(1.25)),
		Vec3(Scalar(1.), Scalar(FALLHEIGHT-0.25), Scalar(1.25)),
		Vec3(Scalar(1.), Scalar(FALLHEIGHT-0.25), Scalar(-1.25)),
		Vec3(Scalar(-1.), Scalar(FALLHEIGHT-0.25), Scalar(-1.25))};

	for (i32 i = 0; i < 4; i++)
	{
		// create a Hinge2 joint
		// create two rigid bodies
		// static bodyA (parent) on top:

		RigidBody* pBodyA = this->m_carChassis;
		pBodyA->setActivationState(DISABLE_DEACTIVATION);
		// dynamic bodyB (child) below it :
		Transform2 tr;
		tr.setIdentity();
		tr.setOrigin(wheelPos[i]);

		RigidBody* pBodyB = createRigidBody(wheelMass, tr, m_wheelShape);
		pBodyB->setFriction(1110);
		pBodyB->setActivationState(DISABLE_DEACTIVATION);
		// add some data to build constraint frames
		Vec3 parentAxis(0.f, 1.f, 0.f);
		Vec3 childAxis(1.f, 0.f, 0.f);
		Vec3 anchor = tr.getOrigin();
		Hinge2Constraint* pHinge2 = new Hinge2Constraint(*pBodyA, *pBodyB, anchor, parentAxis, childAxis);

		//m_guiHelper->get2dCanvasInterface();

		//pHinge2->setLowerLimit(-SIMD_HALF_PI * 0.5f);
		//pHinge2->setUpperLimit(SIMD_HALF_PI * 0.5f);
		
		// add constraint to world
		m_dynamicsWorld->addConstraint(pHinge2, true);

		// Drive engine.
		pHinge2->enableMotor(3, true);
		pHinge2->setMaxMotorForce(3, 1000);
		pHinge2->setTargetVelocity(3, 0);

		// Steering engine.
		pHinge2->enableMotor(5, true);
		pHinge2->setMaxMotorForce(5, 1000);
		pHinge2->setTargetVelocity(5, 0);

		pHinge2->setParam( DRX3D_CONSTRAINT_CFM, 0.15f, 2 );
		pHinge2->setParam( DRX3D_CONSTRAINT_ERP, 0.35f, 2 );

		pHinge2->setDamping( 2, 2.0 );
		pHinge2->setStiffness( 2, 40.0 );

		pHinge2->setDbgDrawSize(Scalar(5.f));
	}

	resetForklift();

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void Hinge2Vehicle::physicsDebugDraw(i32 debugFlags)
{
	if (m_dynamicsWorld && m_dynamicsWorld->getDebugDrawer())
	{
		m_dynamicsWorld->getDebugDrawer()->setDebugMode(debugFlags);
		m_dynamicsWorld->debugDrawWorld();
	}
}

//to be implemented by the demo
void Hinge2Vehicle::renderScene()
{
	m_guiHelper->syncPhysicsToGraphics(m_dynamicsWorld);

	m_guiHelper->render(m_dynamicsWorld);

	Vec3 wheelColor(1, 0, 0);

	Vec3 worldBoundsMin, worldBoundsMax;
	getDynamicsWorld()->getBroadphase()->getBroadphaseAabb(worldBoundsMin, worldBoundsMax);
}

void Hinge2Vehicle::stepSimulation(float deltaTime)
{
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

void Hinge2Vehicle::displayCallback(void)
{
	//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//renderme();

	//optional but useful: debug drawing
	if (m_dynamicsWorld)
		m_dynamicsWorld->debugDrawWorld();

	//	glFlush();
	//	glutSwapBuffers();
}

void Hinge2Vehicle::clientResetScene()
{
	exitPhysics();
	initPhysics();
}

void Hinge2Vehicle::resetForklift()
{
	gVehicleSteering = 0.f;
	gBreakingForce = defaultBreakingForce;
	gEngineForce = 0.f;

	m_carChassis->setCenterOfMassTransform(Transform2::getIdentity());
	m_carChassis->setLinearVelocity(Vec3(0, 0, 0));
	m_carChassis->setAngularVelocity(Vec3(0, 0, 0));
	m_dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(m_carChassis->getBroadphaseHandle(), getDynamicsWorld()->getDispatcher());
}

bool Hinge2Vehicle::keyboardCallback(i32 key, i32 state)
{
	bool handled = false;
	bool isShiftPressed = m_guiHelper->getAppInterface()->m_window->isModifierKeyPressed(B3G_SHIFT);

	if (state)
	{
		if (isShiftPressed)
		{
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

					delete m_solver;
					if (useMCLPSolver)
					{
						DantzigSolver* mlcp = new DantzigSolver();
						//SolveProjectedGaussSeidel* mlcp = new SolveProjectedGaussSeidel;
						MLCPSolver* sol = new MLCPSolver(mlcp);
						m_solver = sol;
					}
					else
					{
						m_solver = new SequentialImpulseConstraintSolver();
					}

					m_dynamicsWorld->setConstraintSolver(m_solver);

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
	}
	return handled;
}

void Hinge2Vehicle::specialKeyboardUp(i32 key, i32 x, i32 y)
{
}

void Hinge2Vehicle::specialKeyboard(i32 key, i32 x, i32 y)
{
}

RigidBody* Hinge2Vehicle::localCreateRigidBody(Scalar mass, const Transform2& startTransform, CollisionShape* shape)
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

CommonExampleInterface* Hinge2VehicleCreateFunc(struct CommonExampleOptions& options)
{
	return new Hinge2Vehicle(options.m_guiHelper);
}
