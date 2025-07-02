///create 125 (5x5x5) dynamic object
#define ARRAY_SIZE_X 5
#define ARRAY_SIZE_Y 5
#define ARRAY_SIZE_Z 5

//maximum number of objects (and allow user to shoot additional boxes)
#define MAX_PROXIES (ARRAY_SIZE_X * ARRAY_SIZE_Y * ARRAY_SIZE_Z + 1024)

///scaling of the objects (0.1 = 20 centimeter boxes )
#define SCALING 1.
#define START_POS_X -5
#define START_POS_Y -5
#define START_POS_Z -3

#include "../RollingFrictionDemo.h"
///BulletDynamicsCommon.h is the main drx3D include file, contains most common include files.
#include <drx3D/DynamicsCommon.h>
#include <stdio.h>  //printf debugging

#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>
#include <drx3D/Common/ResourcePath.h>

///The RollingFrictionDemo shows the use of rolling friction.
///Spheres will come to a rest on a sloped plane using a constraint. Damping cannot achieve the same.
///Generally it is best to leave the rolling friction coefficient zero (or close to zero).
class RollingFrictionDemo : public CommonRigidBodyBase
{
public:
	RollingFrictionDemo(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
	}
	virtual ~RollingFrictionDemo()
	{
	}
	void initPhysics();

	void exitPhysics();

	void resetCamera()
	{
		float dist = 35;
		float pitch = -14;
		float yaw = 0;
		float targetPos[3] = {0, 0, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

void RollingFrictionDemo::initPhysics()
{
	m_guiHelper->setUpAxis(2);

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
	//	m_dynamicsWorld->getSolverInfo().m_singleAxisRollingFrictionThreshold = 0.f;//faster but lower quality
	m_dynamicsWorld->setGravity(Vec3(0, 0, -10));

	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	{
		///create a few basic rigid bodies
		CollisionShape* groundShape = new BoxShape(Vec3(Scalar(10.), Scalar(5.), Scalar(25.)));

		m_collisionShapes.push_back(groundShape);

		Transform2 groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(Vec3(0, 0, -28));
		groundTransform.setRotation(Quat(Vec3(0, 1, 0), SIMD_PI * 0.03));
		//We can also use DemoApplication::localCreateRigidBody, but for clarity it is provided here:
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
		body->setFriction(.5);

		//add the body to the dynamics world
		m_dynamicsWorld->addRigidBody(body);
	}

	{
		///create a few basic rigid bodies
		CollisionShape* groundShape = new BoxShape(Vec3(Scalar(100.), Scalar(100.), Scalar(50.)));

		m_collisionShapes.push_back(groundShape);

		Transform2 groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(Vec3(0, 0, -54));
		//We can also use DemoApplication::localCreateRigidBody, but for clarity it is provided here:
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
		body->setFriction(.1);
		//add the body to the dynamics world
		m_dynamicsWorld->addRigidBody(body);
	}

	{
		//create a few dynamic rigidbodies
		// Re-using the same collision is better for memory usage and performance
#define NUM_SHAPES 10
		CollisionShape* colShapes[NUM_SHAPES] = {
			new SphereShape(Scalar(0.5)),
			new CapsuleShape(0.25, 0.5),
			new CapsuleShapeX(0.25, 0.5),
			new CapsuleShapeZ(0.25, 0.5),
			new ConeShape(0.25, 0.5),
			new ConeShapeX(0.25, 0.5),
			new ConeShapeZ(0.25, 0.5),
			new CylinderShape(Vec3(0.25, 0.5, 0.25)),
			new CylinderShapeX(Vec3(0.5, 0.25, 0.25)),
			new CylinderShapeZ(Vec3(0.25, 0.25, 0.5)),
		};
		for (i32 i = 0; i < NUM_SHAPES; i++)
			m_collisionShapes.push_back(colShapes[i]);

		/// Create Dynamic Objects
		Transform2 startTransform;
		startTransform.setIdentity();

		Scalar mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static

		float start_x = START_POS_X - ARRAY_SIZE_X / 2;
		float start_y = START_POS_Y;
		float start_z = START_POS_Z - ARRAY_SIZE_Z / 2;

		{
			i32 shapeIndex = 0;
			for (i32 k = 0; k < ARRAY_SIZE_Y; k++)
			{
				for (i32 i = 0; i < ARRAY_SIZE_X; i++)
				{
					for (i32 j = 0; j < ARRAY_SIZE_Z; j++)
					{
						startTransform.setOrigin(SCALING * Vec3(
															   Scalar(2.0 * i + start_x),
															   Scalar(2.0 * j + start_z),
															   Scalar(20 + 2.0 * k + start_y)));

						shapeIndex++;
						CollisionShape* colShape = colShapes[shapeIndex % NUM_SHAPES];
						bool isDynamic = (mass != 0.f);
						Vec3 localInertia(0, 0, 0);

						if (isDynamic)
							colShape->calculateLocalInertia(mass, localInertia);

						//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
						DefaultMotionState* myMotionState = new DefaultMotionState(startTransform);
						RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
						RigidBody* body = new RigidBody(rbInfo);
						body->setFriction(1.f);
						body->setRollingFriction(.1);
						body->setSpinningFriction(0.1);
						body->setAnisotropicFriction(colShape->getAnisotropicRollingFrictionDirection(), CollisionObject2::CF_ANISOTROPIC_ROLLING_FRICTION);

						m_dynamicsWorld->addRigidBody(body);
					}
				}
			}
		}
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);

	if (0)
	{
		Serializer* s = new DefaultSerializer;
		m_dynamicsWorld->serialize(s);
		char resourcePath[1024];
		if (ResourcePath::findResourcePath("slope.bullet", resourcePath, 1024,0))
		{
			FILE* f = fopen(resourcePath, "wb");
			fwrite(s->getBufferPointer(), s->getCurrentBufferSize(), 1, f);
			fclose(f);
		}
	}
}

void RollingFrictionDemo::exitPhysics()
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

	delete m_solver;

	delete m_broadphase;

	delete m_dispatcher;

	delete m_collisionConfiguration;
}

class CommonExampleInterface* RollingFrictionCreateFunc(struct CommonExampleOptions& options)
{
	return new RollingFrictionDemo(options.m_guiHelper);
}
