#include "../SerialChains.h"
#include <drx3D/OpenGLWindow/SimpleOpenGL3App.h>
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/DantzigSolver.h>
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

class SerialChains : public CommonMultiBodyBase
{
public:
	SerialChains(GUIHelperInterface* helper);
	virtual ~SerialChains();

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

	MultiBody* createFeatherstoneMultiBody(class MultiBodyDynamicsWorld* world, i32 numLinks, const Vec3& basePosition, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents, bool spherical = false, bool fixedBase = false);
	void createGround(const Vec3& halfExtents = Vec3(50, 50, 50), Scalar zOffSet = Scalar(-1.55));
	void addColliders(MultiBody* pMultiBody, MultiBodyDynamicsWorld* pWorld, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents);
};

static bool g_fixedBase = true;
static bool g_firstInit = true;
static float scaling = 0.4f;
static float friction = 1.;
static i32 g_constraintSolverType = 0;

SerialChains::SerialChains(GUIHelperInterface* helper)
	: CommonMultiBodyBase(helper)
{
	m_guiHelper->setUpAxis(1);
}

SerialChains::~SerialChains()
{
	// Do nothing
}

void SerialChains::stepSimulation(float deltaTime)
{
	//use a smaller internal timestep, there are stability issues
	float internalTimeStep = 1. / 240.f;
	m_dynamicsWorld->stepSimulation(deltaTime, 10, internalTimeStep);
}

void SerialChains::initPhysics()
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

	if (g_constraintSolverType == 3)
	{
		g_constraintSolverType = 0;
		g_fixedBase = !g_fixedBase;
	}

	MLCPSolverInterface* mlcp;
	switch (g_constraintSolverType++)
	{
		case 0:
			m_solver = new MultiBodyConstraintSolver;
			drx3DPrintf("Constraint Solver: Sequential Impulse");
			break;
		case 1:
			mlcp = new SolveProjectedGaussSeidel();
			m_solver = new MultiBodyMLCPConstraintSolver(mlcp);
			drx3DPrintf("Constraint Solver: MLCP + PGS");
			break;
		default:
			mlcp = new DantzigSolver();
			m_solver = new MultiBodyMLCPConstraintSolver(mlcp);
			drx3DPrintf("Constraint Solver: MLCP + Dantzig");
			break;
	}

	MultiBodyDynamicsWorld* world = new MultiBodyDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
	m_dynamicsWorld = world;
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	m_dynamicsWorld->setGravity(Vec3(0, -10, 0));
	m_dynamicsWorld->getSolverInfo().m_globalCfm = Scalar(1e-4);  //todo: what value is good?

	///create a few basic rigid bodies
	Vec3 groundHalfExtents(50, 50, 50);
	CollisionShape* groundShape = new BoxShape(groundHalfExtents);

	m_collisionShapes.push_back(groundShape);

	Transform2 groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(Vec3(0, -50, 00));

	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////

	bool damping = true;
	bool gyro = true;
	i32 numLinks = 5;
	bool spherical = true;      //set it ot false -to use 1DoF hinges instead of 3DoF sphericals
	bool multibodyOnly = true;  //false
	bool canSleep = true;
	bool selfCollide = true;
	Vec3 linkHalfExtents(0.05, 0.37, 0.1);
	Vec3 baseHalfExtents(0.05, 0.37, 0.1);

	MultiBody* mbC1 = createFeatherstoneMultiBody(world, numLinks, Vec3(-0.4f, 3.f, 0.f), linkHalfExtents, baseHalfExtents, spherical, g_fixedBase);
	MultiBody* mbC2 = createFeatherstoneMultiBody(world, numLinks, Vec3(-0.4f, 3.0f, 0.5f), linkHalfExtents, baseHalfExtents, spherical, g_fixedBase);

	mbC1->setCanSleep(canSleep);
	mbC1->setHasSelfCollision(selfCollide);
	mbC1->setUseGyroTerm(gyro);

	if (!damping)
	{
		mbC1->setLinearDamping(0.f);
		mbC1->setAngularDamping(0.f);
	}
	else
	{
		mbC1->setLinearDamping(0.1f);
		mbC1->setAngularDamping(0.9f);
	}
	//
	m_dynamicsWorld->setGravity(Vec3(0, -9.81, 0));
	//////////////////////////////////////////////
	if (numLinks > 0)
	{
		Scalar q0 = 45.f * SIMD_PI / 180.f;
		if (!spherical)
		{
			mbC1->setJointPosMultiDof(0, &q0);
		}
		else
		{
			Quat quat0(Vec3(1, 1, 0).normalized(), q0);
			quat0.normalize();
			mbC1->setJointPosMultiDof(0, quat0);
		}
	}
	///
	addColliders(mbC1, world, baseHalfExtents, linkHalfExtents);

	mbC2->setCanSleep(canSleep);
	mbC2->setHasSelfCollision(selfCollide);
	mbC2->setUseGyroTerm(gyro);
	//
	if (!damping)
	{
		mbC2->setLinearDamping(0.f);
		mbC2->setAngularDamping(0.f);
	}
	else
	{
		mbC2->setLinearDamping(0.1f);
		mbC2->setAngularDamping(0.9f);
	}
	//
	m_dynamicsWorld->setGravity(Vec3(0, -9.81, 0));
	//////////////////////////////////////////////
	if (numLinks > 0)
	{
		Scalar q0 = -45.f * SIMD_PI / 180.f;
		if (!spherical)
		{
			mbC2->setJointPosMultiDof(0, &q0);
		}
		else
		{
			Quat quat0(Vec3(1, 1, 0).normalized(), q0);
			quat0.normalize();
			mbC2->setJointPosMultiDof(0, quat0);
		}
	}
	///
	addColliders(mbC2, world, baseHalfExtents, linkHalfExtents);

	/////////////////////////////////////////////////////////////////
	Scalar groundHeight = -51.55;
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
	/////////////////////////////////////////////////////////////////

	createGround();

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);

	/////////////////////////////////////////////////////////////////
}

MultiBody* SerialChains::createFeatherstoneMultiBody(MultiBodyDynamicsWorld* pWorld, i32 numLinks, const Vec3& basePosition, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents, bool spherical, bool fixedBase)
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

	MultiBody* pMultiBody = new MultiBody(numLinks, baseMass, baseInertiaDiag, fixedBase, canSleep);

	Quat baseOriQuat(0.f, 0.f, 0.f, 1.f);
	pMultiBody->setBasePos(basePosition);
	pMultiBody->setWorldToBaseRot(baseOriQuat);
	Vec3 vel(0, 0, 0);

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

void SerialChains::createGround(const Vec3& halfExtents, Scalar zOffSet)
{
	CollisionShape* groundShape = new BoxShape(halfExtents);
	m_collisionShapes.push_back(groundShape);

	// rigidbody is dynamic if and only if mass is non zero, otherwise static
	Scalar mass(0.);
	const bool isDynamic = (mass != 0.f);

	Vec3 localInertia(0, 0, 0);
	if (isDynamic)
		groundShape->calculateLocalInertia(mass, localInertia);

	// using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	Transform2 groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(Vec3(0, -halfExtents.z() + zOffSet, 0));
	DefaultMotionState* myMotionState = new DefaultMotionState(groundTransform);
	RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
	RigidBody* body = new RigidBody(rbInfo);

	// add the body to the dynamics world
	m_dynamicsWorld->addRigidBody(body, 1, 1 + 2);
}

void SerialChains::addColliders(MultiBody* pMultiBody, MultiBodyDynamicsWorld* pWorld, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents)
{
	AlignedObjectArray<Quat> world_to_local;
	world_to_local.resize(pMultiBody->getNumLinks() + 1);

	AlignedObjectArray<Vec3> local_origin;
	local_origin.resize(pMultiBody->getNumLinks() + 1);
	world_to_local[0] = pMultiBody->getWorldToBaseRot();
	local_origin[0] = pMultiBody->getBasePos();

	{
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

CommonExampleInterface* SerialChainsCreateFunc(CommonExampleOptions& options)
{
	return new SerialChains(options.m_guiHelper);
}
