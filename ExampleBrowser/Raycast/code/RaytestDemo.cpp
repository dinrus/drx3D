#include "../RaytestDemo.h"
///BulletDynamicsCommon.h is the main drx3D include file, contains most common include files.
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/Collision/NarrowPhase/RaycastCallback.h>
#include <drx3D/Physics/Collision/Gimpact/GImpactShape.h>

#include <stdio.h>  //printf debugging

#include <drx3D/Maths/Linear/AlignedObjectArray.h>

///RaytestDemo shows how to use the CollisionWorld::rayTest feature

#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

class RaytestDemo : public CommonRigidBodyBase
{
public:
	RaytestDemo(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
	}
	virtual ~RaytestDemo()
	{
	}
	virtual void initPhysics();

	virtual void exitPhysics();

	void castRays();

	virtual void stepSimulation(float deltaTime);

	virtual void resetCamera()
	{
		float dist = 18;
		float pitch = -30;
		float yaw = 129;
		float targetPos[3] = {-4.6, -4.7, -5.75};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

void RaytestDemo::castRays()
{
	static float up = 0.f;
	static float dir = 1.f;
	//add some simple animation
	//if (!m_idle)
	{
		up += 0.01 * dir;

		if (Fabs(up) > 2)
		{
			dir *= -1.f;
		}

		Transform2 tr = m_dynamicsWorld->getCollisionObjectArray()[1]->getWorldTransform();
		static float angle = 0.f;
		angle += 0.01f;
		tr.setRotation(Quat(Vec3(0, 1, 0), angle));
		m_dynamicsWorld->getCollisionObjectArray()[1]->setWorldTransform(tr);
	}

	///step the simulation
	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->updateAabbs();
		m_dynamicsWorld->computeOverlappingPairs();

		Vec3 red(1, 0, 0);
		Vec3 blue(0, 0, 1);

		///all hits
		{
			Vec3 from(-30, 1 + up, 0);
			Vec3 to(30, 1, 0);
			m_dynamicsWorld->getDebugDrawer()->drawLine(from, to, Vec4(0, 0, 0, 1));
			CollisionWorld::AllHitsRayResultCallback allResults(from, to);
			allResults.m_flags |= TriangleRaycastCallback::kF_KeepUnflippedNormal;
			//kF_UseGjkConvexRaytest flag is now enabled by default, use the faster but more approximate algorithm
			//allResults.m_flags |= TriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;
			allResults.m_flags |= TriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;

			m_dynamicsWorld->rayTest(from, to, allResults);

			for (i32 i = 0; i < allResults.m_hitFractions.size(); i++)
			{
				Vec3 p = from.lerp(to, allResults.m_hitFractions[i]);
				m_dynamicsWorld->getDebugDrawer()->drawSphere(p, 0.1, red);
				m_dynamicsWorld->getDebugDrawer()->drawLine(p, p + allResults.m_hitNormalWorld[i], red);
			}
		}

		///first hit
		{
			Vec3 from(-30, 1.2, 0);
			Vec3 to(30, 1.2, 0);
			m_dynamicsWorld->getDebugDrawer()->drawLine(from, to, Vec4(0, 0, 1, 1));

			CollisionWorld::ClosestRayResultCallback closestResults(from, to);
			closestResults.m_flags |= TriangleRaycastCallback::kF_FilterBackfaces;

			m_dynamicsWorld->rayTest(from, to, closestResults);

			if (closestResults.hasHit())
			{
				Vec3 p = from.lerp(to, closestResults.m_closestHitFraction);
				m_dynamicsWorld->getDebugDrawer()->drawSphere(p, 0.1, blue);
				m_dynamicsWorld->getDebugDrawer()->drawLine(p, p + closestResults.m_hitNormalWorld, blue);
			}
		}
	}
}

void RaytestDemo::stepSimulation(float deltaTime)
{
	castRays();
	CommonRigidBodyBase::stepSimulation(deltaTime);
}

void RaytestDemo::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	///collision configuration contains default setup for memory, collision setup
	m_collisionConfiguration = new DefaultCollisionConfiguration();
	//m_collisionConfiguration->setConvexConvexMultipointIterations();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

	m_broadphase = new DbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	SequentialImpulseConstraintSolver* sol = new SequentialImpulseConstraintSolver;
	m_solver = sol;

	m_dynamicsWorld = new DiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	m_dynamicsWorld->setGravity(Vec3(0, -10, 0));

	///create a few basic rigid bodies
	CollisionShape* groundShape = new BoxShape(Vec3(Scalar(50.), Scalar(50.), Scalar(50.)));
	//	CollisionShape* groundShape = new StaticPlaneShape(Vec3(0,1,0),50);

	m_collisionShapes.push_back(groundShape);

	Transform2 groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(Vec3(0, -50, 0));

	//We can also use DemoApplication::localCreateRigidBody, but for clarity it is provided here:
	{
		Scalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		Vec3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		DefaultMotionState* myMotionState = new DefaultMotionState(groundTransform);
		RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		RigidBody* body = new RigidBody(rbInfo);
		body->setFriction(1);
		//add the body to the dynamics world
		m_dynamicsWorld->addRigidBody(body);
	}

	{
		Vec3 convexPoints[] = {Vec3(-1, -1, -1), Vec3(-1, -1, 1), Vec3(-1, 1, 1), Vec3(-1, 1, -1),
									Vec3(2, 0, 0)};

		Vec3 quad[] = {
			Vec3(0, 1, -1),
			Vec3(0, 1, 1),
			Vec3(0, -1, 1),
			Vec3(0, -1, -1)};

		TriangleMesh* mesh = new TriangleMesh();
		mesh->addTriangle(quad[0], quad[1], quad[2], true);
		mesh->addTriangle(quad[0], quad[2], quad[3], true);

		BvhTriangleMeshShape* trimesh = new BvhTriangleMeshShape(mesh, true, true);
		//GImpactMeshShape * trimesh = new btGImpactMeshShape(mesh);
		//trimesh->updateBound();

#define NUM_SHAPES 6
		CollisionShape* colShapes[NUM_SHAPES] = {
			trimesh,
			new ConvexHullShape(&convexPoints[0].getX(), sizeof(convexPoints) / sizeof(Vec3), sizeof(Vec3)),
			new SphereShape(1),
			new CapsuleShape(0.2, 1),
			new CylinderShape(Vec3(0.2, 1, 0.2)),
			new BoxShape(Vec3(1, 1, 1))};

		for (i32 i = 0; i < NUM_SHAPES; i++)
			m_collisionShapes.push_back(colShapes[i]);

		for (i32 i = 0; i < 6; i++)
		{
			//create a few dynamic rigidbodies
			// Re-using the same collision is better for memory usage and performance

			/// Create Dynamic Objects
			Transform2 startTransform;
			startTransform.setIdentity();
			startTransform.setOrigin(Vec3((i - 3) * 5, 1, 0));

			Scalar mass(1.f);

			if (!i)
				mass = 0.f;

			//rigidbody is dynamic if and only if mass is non zero, otherwise static
			bool isDynamic = (mass != 0.f);

			Vec3 localInertia(0, 0, 0);
			CollisionShape* colShape = colShapes[i % NUM_SHAPES];
			if (isDynamic)
				colShape->calculateLocalInertia(mass, localInertia);

			RigidBody::RigidBodyConstructionInfo rbInfo(mass, 0, colShape, localInertia);
			rbInfo.m_startWorldTransform = startTransform;
			RigidBody* body = new RigidBody(rbInfo);
			body->setRollingFriction(0.03);
			body->setSpinningFriction(0.03);
			body->setFriction(1);
			body->setAnisotropicFriction(colShape->getAnisotropicRollingFrictionDirection(), CollisionObject2::CF_ANISOTROPIC_ROLLING_FRICTION);

			m_dynamicsWorld->addRigidBody(body);
		}
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void RaytestDemo::exitPhysics()
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
			delete body->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for (i32 j = 0; j < m_collisionShapes.size(); j++)
	{
		CollisionShape* shape = m_collisionShapes[j];
		delete shape;
	}
	m_collisionShapes.clear();

	delete m_dynamicsWorld;
	m_dynamicsWorld = 0;

	delete m_solver;
	m_solver = 0;

	delete m_broadphase;
	m_broadphase = 0;

	delete m_dispatcher;
	m_dispatcher = 0;

	delete m_collisionConfiguration;
	m_collisionConfiguration = 0;
}

class CommonExampleInterface* RaytestCreateFunc(struct CommonExampleOptions& options)
{
	return new RaytestDemo(options.m_guiHelper);
}
