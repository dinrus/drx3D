#include "../MultiDofDemo.h"
#include <drx3D/OpenGLWindow/SimpleOpenGL3App.h>
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/DantzigSolver.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/LemkeSolver.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/SolveProjectedGaussSeidel.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyMLCPConstraintSolver.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyDynamicsWorld.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLink.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointLimitConstraint.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointMotor.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyPoint2Point.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyFixedConstraint.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodySliderConstraint.h>
#include <drx3D/OpenGLWindow/GLInstancingRenderer.h>
#include <drx3D/Physics/Collision/Shapes/ShapeHull.h>
#include <drx3D/Common/Interfaces/CommonMultiBodyBase.h>

class MultiDofDemo : public CommonMultiBodyBase
{
public:
	MultiDofDemo(GUIHelperInterface* helper);
	virtual ~MultiDofDemo();

	virtual void initPhysics();

	virtual void stepSimulation(float deltaTime);

	virtual void resetCamera()
	{
		float dist = 1;
		float pitch = -35;
		float yaw = 50;
		float targetPos[3] = {-3, 2.8, -2.5};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}

	MultiBody* createFeatherstoneMultiBody_testMultiDof(class MultiBodyDynamicsWorld* world, i32 numLinks, const Vec3& basePosition, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents, bool spherical = false, bool floating = false);
	void addColliders_testMultiDof(MultiBody* pMultiBody, MultiBodyDynamicsWorld* pWorld, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents);
	void addBoxes_testMultiDof();
};

static bool g_floatingBase = false;
static bool g_firstInit = true;
static float scaling = 0.4f;
static float friction = 1.;
static i32 g_constraintSolverType = 0;
#define ARRAY_SIZE_X 5
#define ARRAY_SIZE_Y 5
#define ARRAY_SIZE_Z 5

//maximum number of objects (and allow user to shoot additional boxes)
#define MAX_PROXIES (ARRAY_SIZE_X * ARRAY_SIZE_Y * ARRAY_SIZE_Z + 1024)

#define START_POS_X -5
//#define START_POS_Y 12
#define START_POS_Y 2
#define START_POS_Z -3

MultiDofDemo::MultiDofDemo(GUIHelperInterface* helper)
	: CommonMultiBodyBase(helper)
{
	m_guiHelper->setUpAxis(1);
}
MultiDofDemo::~MultiDofDemo()
{
}

void MultiDofDemo::stepSimulation(float deltaTime)
{
	//use a smaller internal timestep, there are stability issues
	float internalTimeStep = 1. / 240.f;
	m_dynamicsWorld->stepSimulation(deltaTime, 10, internalTimeStep);
}

void MultiDofDemo::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	if (g_firstInit)
	{
		m_guiHelper->getRenderInterface()->getActiveCamera()->setCameraDistance(Scalar(10. * scaling));
		m_guiHelper->getRenderInterface()->getActiveCamera()->setCameraPitch(50);
		g_firstInit = false;
	}
	///collision configuration contains default setup for memory, collision setup
	m_collisionConfiguration = new DefaultCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

	m_broadphase = new DbvtBroadphase();

	if (g_constraintSolverType == 4)
	{
		g_constraintSolverType = 0;
		g_floatingBase = !g_floatingBase;
	}

	MultiBodyConstraintSolver* sol;
	MLCPSolverInterface* mlcp;
	switch (g_constraintSolverType++)
	{
		case 0:
			sol = new MultiBodyConstraintSolver;
			drx3DPrintf("Constraint Solver: Sequential Impulse");
			break;
		case 1:
			mlcp = new SolveProjectedGaussSeidel();
			sol = new MultiBodyMLCPConstraintSolver(mlcp);
			drx3DPrintf("Constraint Solver: MLCP + PGS");
			break;
		case 2:
			mlcp = new DantzigSolver();
			sol = new MultiBodyMLCPConstraintSolver(mlcp);
			drx3DPrintf("Constraint Solver: MLCP + Dantzig");
			break;
		default:
			mlcp = new LemkeSolver();
			sol = new MultiBodyMLCPConstraintSolver(mlcp);
			drx3DPrintf("Constraint Solver: MLCP + Lemke");
			break;
	}

	m_solver = sol;

	//use btMultiBodyDynamicsWorld for Featherstone btMultiBody support
	MultiBodyDynamicsWorld* world = new MultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration);
	m_dynamicsWorld = world;
	//	m_dynamicsWorld->setDebugDrawer(&gDebugDraw);
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	m_dynamicsWorld->setGravity(Vec3(0, -10, 0));
	m_dynamicsWorld->getSolverInfo().m_globalCfm = 1e-3;

	///create a few basic rigid bodies
	Vec3 groundHalfExtents(50, 50, 50);
	CollisionShape* groundShape = new BoxShape(groundHalfExtents);
	//groundShape->initializePolyhedralFeatures();
	//	CollisionShape* groundShape = new StaticPlaneShape(Vec3(0,1,0),50);

	m_collisionShapes.push_back(groundShape);

	Transform2 groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(Vec3(0, -50, 00));

	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////

	bool damping = true;
	bool gyro = true;
	i32 numLinks = 5;
	bool spherical = true;  //set it ot false -to use 1DoF hinges instead of 3DoF sphericals
	bool multibodyOnly = false;
	bool canSleep = false;
	bool selfCollide = true;
	bool multibodyConstraint = false;
	Vec3 linkHalfExtents(0.05, 0.37, 0.1);
	Vec3 baseHalfExtents(0.05, 0.37, 0.1);

	MultiBody* mbC = createFeatherstoneMultiBody_testMultiDof(world, numLinks, Vec3(-0.4f, 3.f, 0.f), baseHalfExtents, linkHalfExtents, spherical, g_floatingBase);
	//mbC->forceMultiDof();							//if !spherical, you can comment this line to check the 1DoF algorithm

	mbC->setCanSleep(canSleep);
	mbC->setHasSelfCollision(selfCollide);
	mbC->setUseGyroTerm(gyro);
	//
	if (!damping)
	{
		mbC->setLinearDamping(0.f);
		mbC->setAngularDamping(0.f);
	}
	else
	{
		mbC->setLinearDamping(0.1f);
		mbC->setAngularDamping(0.9f);
	}
	//
	m_dynamicsWorld->setGravity(Vec3(0, -9.81, 0));
	//m_dynamicsWorld->getSolverInfo().m_numIterations = 100;
	//////////////////////////////////////////////
	if (numLinks > 0)
	{
		Scalar q0 = 45.f * SIMD_PI / 180.f;
		if (!spherical)
		{
			mbC->setJointPosMultiDof(0, &q0);
		}
		else
		{
			Quat quat0(Vec3(1, 1, 0).normalized(), q0);
			quat0.normalize();
			mbC->setJointPosMultiDof(0, quat0);
		}
	}
	///
	addColliders_testMultiDof(mbC, world, baseHalfExtents, linkHalfExtents);

	/////////////////////////////////////////////////////////////////
	Scalar groundHeight = -51.55;
	if (!multibodyOnly)
	{
		Scalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		Vec3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		groundTransform.setIdentity();
		groundTransform.setOrigin(Vec3(0, groundHeight, 0));
		DefaultMotionState* myMotionState = new DefaultMotionState(groundTransform);
		RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		RigidBody* body = new RigidBody(rbInfo);

		//add the body to the dynamics world
		m_dynamicsWorld->addRigidBody(body, 1, 1 + 2);  //,1,1+2);
	}
	/////////////////////////////////////////////////////////////////
	if (!multibodyOnly)
	{
		Vec3 halfExtents(.5, .5, .5);
		BoxShape* colShape = new BoxShape(halfExtents);
		//CollisionShape* colShape = new SphereShape(Scalar(1.));
		m_collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		Transform2 startTransform;
		startTransform.setIdentity();

		Scalar mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		Vec3 localInertia(0, 0, 0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass, localInertia);

		startTransform.setOrigin(Vec3(
			Scalar(0.0),
			0.0,
			Scalar(0.0)));

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		DefaultMotionState* myMotionState = new DefaultMotionState(startTransform);
		RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
		RigidBody* body = new RigidBody(rbInfo);

		m_dynamicsWorld->addRigidBody(body);  //,1,1+2);

		if (multibodyConstraint)
		{
			Vec3 pointInA = -linkHalfExtents;
			//      Vec3 pointInB = halfExtents;
			Matrix3x3 frameInA;
			Matrix3x3 frameInB;
			frameInA.setIdentity();
			frameInB.setIdentity();
			Vec3 jointAxis(1.0, 0.0, 0.0);
			//MultiBodySliderConstraint* p2p = new btMultiBodySliderConstraint(mbC,numLinks-1,body,pointInA,pointInB,frameInA,frameInB,jointAxis);
			MultiBodyFixedConstraint* p2p = new MultiBodyFixedConstraint(mbC, numLinks - 1, mbC, numLinks - 4, pointInA, pointInA, frameInA, frameInB);
			p2p->setMaxAppliedImpulse(2.0);
			m_dynamicsWorld->addMultiBodyConstraint(p2p);
		}
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);

	/////////////////////////////////////////////////////////////////
}

MultiBody* MultiDofDemo::createFeatherstoneMultiBody_testMultiDof(MultiBodyDynamicsWorld* pWorld, i32 numLinks, const Vec3& basePosition, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents, bool spherical, bool floating)
{
	//init the base
	Vec3 baseInertiaDiag(0.f, 0.f, 0.f);
	float baseMass = 1.f;

	if (baseMass)
	{
		CollisionShape* pTempBox = new BoxShape(Vec3(baseHalfExtents[0], baseHalfExtents[1], baseHalfExtents[2]));
		pTempBox->calculateLocalInertia(baseMass, baseInertiaDiag);
		delete pTempBox;
	}

	bool canSleep = false;

	MultiBody* pMultiBody = new MultiBody(numLinks, baseMass, baseInertiaDiag, !floating, canSleep);

	Quat baseOriQuat(0.f, 0.f, 0.f, 1.f);
	pMultiBody->setBasePos(basePosition);
	pMultiBody->setWorldToBaseRot(baseOriQuat);
	Vec3 vel(0, 0, 0);
	//	pMultiBody->setBaseVel(vel);

	//init the links
	Vec3 hingeJointAxis(1, 0, 0);
	float linkMass = 1.f;
	Vec3 linkInertiaDiag(0.f, 0.f, 0.f);

	CollisionShape* pTempBox = new BoxShape(Vec3(linkHalfExtents[0], linkHalfExtents[1], linkHalfExtents[2]));
	pTempBox->calculateLocalInertia(linkMass, linkInertiaDiag);
	delete pTempBox;

	//y-axis assumed up
	Vec3 parentComToCurrentCom(0, -linkHalfExtents[1] * 2.f, 0);                      //par body's COM to cur body's COM offset
	Vec3 currentPivotToCurrentCom(0, -linkHalfExtents[1], 0);                         //cur body's COM to cur body's PIV offset
	Vec3 parentComToCurrentPivot = parentComToCurrentCom - currentPivotToCurrentCom;  //par body's COM to cur body's PIV offset

	//////
	Scalar q0 = 0.f * SIMD_PI / 180.f;
	Quat quat0(Vec3(0, 1, 0).normalized(), q0);
	quat0.normalize();
	/////

	for (i32 i = 0; i < numLinks; ++i)
	{
		if (!spherical)
			pMultiBody->setupRevolute(i, linkMass, linkInertiaDiag, i - 1, Quat(0.f, 0.f, 0.f, 1.f), hingeJointAxis, parentComToCurrentPivot, currentPivotToCurrentCom, true);
		else
			//pMultiBody->setupPlanar(i, linkMass, linkInertiaDiag, i - 1, Quat(0.f, 0.f, 0.f, 1.f)/*quat0*/, Vec3(1, 0, 0), parentComToCurrentPivot*2, false);
			pMultiBody->setupSpherical(i, linkMass, linkInertiaDiag, i - 1, Quat(0.f, 0.f, 0.f, 1.f), parentComToCurrentPivot, currentPivotToCurrentCom, true);
	}

	pMultiBody->finalizeMultiDof();

	///
	pWorld->addMultiBody(pMultiBody);
	///
	return pMultiBody;
}

void MultiDofDemo::addColliders_testMultiDof(MultiBody* pMultiBody, MultiBodyDynamicsWorld* pWorld, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents)
{
	AlignedObjectArray<Quat> world_to_local;
	world_to_local.resize(pMultiBody->getNumLinks() + 1);

	AlignedObjectArray<Vec3> local_origin;
	local_origin.resize(pMultiBody->getNumLinks() + 1);
	world_to_local[0] = pMultiBody->getWorldToBaseRot();
	local_origin[0] = pMultiBody->getBasePos();

	{
		//	float pos[4]={local_origin[0].x(),local_origin[0].y(),local_origin[0].z(),1};
		Scalar quat[4] = {-world_to_local[0].x(), -world_to_local[0].y(), -world_to_local[0].z(), world_to_local[0].w()};

		if (1)
		{
			CollisionShape* box = new BoxShape(baseHalfExtents);
			MultiBodyLinkCollider* col = new MultiBodyLinkCollider(pMultiBody, -1);
			col->setCollisionShape(box);

			Transform2 tr;
			tr.setIdentity();
			tr.setOrigin(local_origin[0]);
			tr.setRotation(Quat(quat[0], quat[1], quat[2], quat[3]));
			col->setWorldTransform(tr);

			pWorld->addCollisionObject(col, 2, 1 + 2);

			col->setFriction(friction);
			pMultiBody->setBaseCollider(col);
		}
	}

	for (i32 i = 0; i < pMultiBody->getNumLinks(); ++i)
	{
		i32k parent = pMultiBody->getParent(i);
		world_to_local[i + 1] = pMultiBody->getParentToLocalRot(i) * world_to_local[parent + 1];
		local_origin[i + 1] = local_origin[parent + 1] + (quatRotate(world_to_local[i + 1].inverse(), pMultiBody->getRVector(i)));
	}

	for (i32 i = 0; i < pMultiBody->getNumLinks(); ++i)
	{
		Vec3 posr = local_origin[i + 1];
		//	float pos[4]={posr.x(),posr.y(),posr.z(),1};

		Scalar quat[4] = {-world_to_local[i + 1].x(), -world_to_local[i + 1].y(), -world_to_local[i + 1].z(), world_to_local[i + 1].w()};

		CollisionShape* box = new BoxShape(linkHalfExtents);
		MultiBodyLinkCollider* col = new MultiBodyLinkCollider(pMultiBody, i);

		col->setCollisionShape(box);
		Transform2 tr;
		tr.setIdentity();
		tr.setOrigin(posr);
		tr.setRotation(Quat(quat[0], quat[1], quat[2], quat[3]));
		col->setWorldTransform(tr);
		col->setFriction(friction);
		pWorld->addCollisionObject(col, 2, 1 + 2);

		pMultiBody->getLink(i).m_collider = col;
	}
}

void MultiDofDemo::addBoxes_testMultiDof()
{
	//create a few dynamic rigidbodies
	// Re-using the same collision is better for memory usage and performance

	BoxShape* colShape = new BoxShape(Vec3(1, 1, 1));
	//CollisionShape* colShape = new SphereShape(Scalar(1.));
	m_collisionShapes.push_back(colShape);

	/// Create Dynamic Objects
	Transform2 startTransform;
	startTransform.setIdentity();

	Scalar mass(1.f);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	Vec3 localInertia(0, 0, 0);
	if (isDynamic)
		colShape->calculateLocalInertia(mass, localInertia);

	float start_x = START_POS_X - ARRAY_SIZE_X / 2;
	float start_y = START_POS_Y;
	float start_z = START_POS_Z - ARRAY_SIZE_Z / 2;

	for (i32 k = 0; k < ARRAY_SIZE_Y; k++)
	{
		for (i32 i = 0; i < ARRAY_SIZE_X; i++)
		{
			for (i32 j = 0; j < ARRAY_SIZE_Z; j++)
			{
				startTransform.setOrigin(Vec3(
					Scalar(3.0 * i + start_x),
					Scalar(3.0 * k + start_y),
					Scalar(3.0 * j + start_z)));

				//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
				DefaultMotionState* myMotionState = new DefaultMotionState(startTransform);
				RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
				RigidBody* body = new RigidBody(rbInfo);

				m_dynamicsWorld->addRigidBody(body);  //,1,1+2);
			}
		}
	}
}

class CommonExampleInterface* MultiDofCreateFunc(struct CommonExampleOptions& options)
{
	return new MultiDofDemo(options.m_guiHelper);
}
