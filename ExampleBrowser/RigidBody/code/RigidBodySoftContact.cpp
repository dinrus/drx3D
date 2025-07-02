#include "../RigidBodySoftContact.h"

#include <drx3D/DynamicsCommon.h>
#define ARRAY_SIZE_Y 1
#define ARRAY_SIZE_X 1
#define ARRAY_SIZE_Z 1

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/MLCPSolver.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/SolveProjectedGaussSeidel.h>

struct RigidBodySoftContact : public CommonRigidBodyBase
{
	RigidBodySoftContact(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
	}
	virtual ~RigidBodySoftContact()
	{
	}
	virtual void initPhysics();
	virtual void renderScene();
	void resetCamera()
	{
		float dist = 3;
		float pitch = -35;
		float yaw = 52;
		float targetPos[3] = {0, 0.46, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

void RigidBodySoftContact::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	//createEmptyDynamicsWorld();
	{
		///collision configuration contains default setup for memory, collision setup
		m_collisionConfiguration = new DefaultCollisionConfiguration();
		//m_collisionConfiguration->setConvexConvexMultipointIterations();

		///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
		m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

		m_broadphase = new DbvtBroadphase();

		///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
		SequentialImpulseConstraintSolver* sol = new SequentialImpulseConstraintSolver;
		//MLCPSolver* sol = new btMLCPSolver(new SolveProjectedGaussSeidel());
		m_solver = sol;

		m_dynamicsWorld = new DiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);

		m_dynamicsWorld->setGravity(Vec3(0, -10, 0));
	}

	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	if (m_dynamicsWorld->getDebugDrawer())
		m_dynamicsWorld->getDebugDrawer()->setDebugMode(IDebugDraw::DBG_DrawWireframe + IDebugDraw::DBG_DrawContactPoints);
	m_dynamicsWorld->getSolverInfo().m_erp2 = 0.f;
	m_dynamicsWorld->getSolverInfo().m_globalCfm = 0.f;
	m_dynamicsWorld->getSolverInfo().m_numIterations = 3;
	m_dynamicsWorld->getSolverInfo().m_solverMode = SOLVER_SIMD;  // | SOLVER_RANDMIZE_ORDER;
	m_dynamicsWorld->getSolverInfo().m_splitImpulse = false;

	///create a few basic rigid bodies
	BoxShape* groundShape = createBoxShape(Vec3(Scalar(50.), Scalar(50.), Scalar(50.)));

	//groundShape->initializePolyhedralFeatures();
	//	CollisionShape* groundShape = new StaticPlaneShape(Vec3(0,1,0),50);

	m_collisionShapes.push_back(groundShape);

	Transform2 groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(Vec3(0, -50, 0));

	{
		Scalar mass(0.);
		RigidBody* body = createRigidBody(mass, groundTransform, groundShape, Vec4(0, 0, 1, 1));

		body->setContactStiffnessAndDamping(300, 10);
	}

	{
		//create a few dynamic rigidbodies
		// Re-using the same collision is better for memory usage and performance

		//BoxShape* colShape = createBoxShape(Vec3(1,1,1));

		CollisionShape* childShape = new SphereShape(Scalar(0.5));
		CompoundShape* colShape = new CompoundShape();
		colShape->addChildShape(Transform2::getIdentity(), childShape);

		m_collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		Transform2 startTransform;
		startTransform.setIdentity();

		startTransform.setRotation(Quat(Vec3(1, 1, 1), SIMD_PI / 10.));
		Scalar mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		Vec3 localInertia(0, 0, 0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass, localInertia);

		for (i32 k = 0; k < ARRAY_SIZE_Y; k++)
		{
			for (i32 i = 0; i < ARRAY_SIZE_X; i++)
			{
				for (i32 j = 0; j < ARRAY_SIZE_Z; j++)
				{
					startTransform.setOrigin(Vec3(
						Scalar(2.0 * i + 0.1),
						Scalar(3 + 2.0 * k),
						Scalar(2.0 * j)));

					RigidBody* body;
					body = createRigidBody(mass, startTransform, colShape);
					//body->setAngularVelocity(Vec3(1,1,1));
				}
			}
		}
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void RigidBodySoftContact::renderScene()
{
	CommonRigidBodyBase::renderScene();
}

CommonExampleInterface* RigidBodySoftContactCreateFunc(CommonExampleOptions& options)
{
	return new RigidBodySoftContact(options.m_guiHelper);
}
