
#include <drx3D/Physics/Collision/Shapes/Box2dShape.h>
#include <drx3D/Physics/Collision/Dispatch/EmptyCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/Box2dBox2dCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/Convex2dConvex2dAlgorithm.h>

#include <drx3D/Physics/Collision/Shapes/Box2dShape.h>
#include <drx3D/Physics/Collision/Shapes/Convex2dShape.h>
#include <drx3D/Physics/Collision/NarrowPhase/MinkowskiPenetrationDepthSolver.h>

///create 125 (5x5x5) dynamic object
#define ARRAY_SIZE_X 5
#define ARRAY_SIZE_Y 5
#define ARRAY_SIZE_Z 1

//maximum number of objects (and allow user to shoot additional boxes)
#define MAX_PROXIES (ARRAY_SIZE_X * ARRAY_SIZE_Y * ARRAY_SIZE_Z + 1024)

///scaling of the objects (0.1 = 20 centimeter boxes )
#define SCALING 1
#define START_POS_X -5
#define START_POS_Y -5
#define START_POS_Z -3

#include "../Planar2D.h"

///BulletDynamicsCommon.h is the main drx3D include file, contains most common include files.
#include <drx3D/DynamicsCommon.h>
#include <stdio.h>  //printf debugging

#include <drx3D/Maths/Linear/AlignedObjectArray.h>

class BroadphaseInterface;
class CollisionShape;
class OverlappingPairCache;
class CollisionDispatcher;
class ConstraintSolver;
struct CollisionAlgorithmCreateFunc;
class DefaultCollisionConfiguration;
class GL_DialogDynamicsWorld;

#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

class Planar2D : public CommonRigidBodyBase
{
	//keep the collision shapes, for deletion/cleanup
	AlignedObjectArray<CollisionShape*> m_collisionShapes;

	BroadphaseInterface* m_broadphase;

	CollisionDispatcher* m_dispatcher;

	ConstraintSolver* m_solver;

	DefaultCollisionConfiguration* m_collisionConfiguration;

	Convex2dConvex2dAlgorithm::CreateFunc* m_convexAlgo2d;
	VoronoiSimplexSolver* m_simplexSolver;
	MinkowskiPenetrationDepthSolver* m_pdSolver;
	Box2dBox2dCollisionAlgorithm::CreateFunc* m_box2dbox2dAlgo;

public:
	Planar2D(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
	}
	virtual ~Planar2D()
	{
		exitPhysics();
	}

	void initPhysics();

	void exitPhysics();

	void resetCamera()
	{
		float dist = 9;
		float pitch = -11;
		float yaw = 539;
		float targetPos[3] = {8.6, 10.5, -20.6};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

void Planar2D::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	///collision configuration contains default setup for memory, collision setup
	m_collisionConfiguration = new DefaultCollisionConfiguration();
	//m_collisionConfiguration->setConvexConvexMultipointIterations();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

	m_simplexSolver = new VoronoiSimplexSolver();
	m_pdSolver = new MinkowskiPenetrationDepthSolver();

	m_convexAlgo2d = new Convex2dConvex2dAlgorithm::CreateFunc(m_simplexSolver, m_pdSolver);
	m_box2dbox2dAlgo = new Box2dBox2dCollisionAlgorithm::CreateFunc();

	m_dispatcher->registerCollisionCreateFunc(CONVEX_2D_SHAPE_PROXYTYPE, CONVEX_2D_SHAPE_PROXYTYPE, m_convexAlgo2d);
	m_dispatcher->registerCollisionCreateFunc(BOX_2D_SHAPE_PROXYTYPE, CONVEX_2D_SHAPE_PROXYTYPE, m_convexAlgo2d);
	m_dispatcher->registerCollisionCreateFunc(CONVEX_2D_SHAPE_PROXYTYPE, BOX_2D_SHAPE_PROXYTYPE, m_convexAlgo2d);
	m_dispatcher->registerCollisionCreateFunc(BOX_2D_SHAPE_PROXYTYPE, BOX_2D_SHAPE_PROXYTYPE, m_box2dbox2dAlgo);

	m_broadphase = new DbvtBroadphase();
	//m_broadphase = new SimpleBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	SequentialImpulseConstraintSolver* sol = new SequentialImpulseConstraintSolver;
	m_solver = sol;

	m_dynamicsWorld = new DiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
	//m_dynamicsWorld->getSolverInfo().m_erp = 1.f;
	//m_dynamicsWorld->getSolverInfo().m_numIterations = 4;
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	m_dynamicsWorld->setGravity(Vec3(0, -10, 0));

	///create a few basic rigid bodies
	CollisionShape* groundShape = new BoxShape(Vec3(Scalar(150.), Scalar(50.), Scalar(150.)));
	//	CollisionShape* groundShape = new StaticPlaneShape(Vec3(0,1,0),50);

	m_collisionShapes.push_back(groundShape);

	Transform2 groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(Vec3(0, -43, 0));

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

		//add the body to the dynamics world
		m_dynamicsWorld->addRigidBody(body);
	}

	{
		//create a few dynamic rigidbodies
		// Re-using the same collision is better for memory usage and performance

		Scalar u = Scalar(1 * SCALING - 0.04);
		Vec3 points[3] = {Vec3(0, u, 0), Vec3(-u, -u, 0), Vec3(u, -u, 0)};
		ConvexShape* childShape0 = new BoxShape(Vec3(Scalar(SCALING * 1), Scalar(SCALING * 1), Scalar(0.04)));
		ConvexShape* colShape = new Convex2dShape(childShape0);
		//CollisionShape* colShape = new Box2dShape(Vec3(SCALING*1,SCALING*1,0.04));
		ConvexShape* childShape1 = new ConvexHullShape(&points[0].getX(), 3);
		ConvexShape* colShape2 = new Convex2dShape(childShape1);
		ConvexShape* childShape2 = new CylinderShapeZ(Vec3(Scalar(SCALING * 1), Scalar(SCALING * 1), Scalar(0.04)));
		ConvexShape* colShape3 = new Convex2dShape(childShape2);

		m_collisionShapes.push_back(colShape);
		m_collisionShapes.push_back(colShape2);
		m_collisionShapes.push_back(colShape3);

		m_collisionShapes.push_back(childShape0);
		m_collisionShapes.push_back(childShape1);
		m_collisionShapes.push_back(childShape2);

		//UniformScalingShape* colShape = new UniformScalingShape(convexColShape,1.f);
		colShape->setMargin(Scalar(0.03));
		//CollisionShape* colShape = new SphereShape(Scalar(1.));

		/// Create Dynamic Objects
		Transform2 startTransform;
		startTransform.setIdentity();

		Scalar mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		Vec3 localInertia(0, 0, 0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass, localInertia);

		//		float start_x = START_POS_X - ARRAY_SIZE_X/2;
		//		float start_y = START_POS_Y;
		//		float start_z = START_POS_Z - ARRAY_SIZE_Z/2;

		Vec3 x(-ARRAY_SIZE_X, 8.0f, -20.f);
		Vec3 y;
		Vec3 deltaX(SCALING * 1, SCALING * 2, 0.f);
		Vec3 deltaY(SCALING * 2, 0.0f, 0.f);

		for (i32 i = 0; i < ARRAY_SIZE_X; ++i)
		{
			y = x;

			for (i32 j = i; j < ARRAY_SIZE_Y; ++j)
			{
				startTransform.setOrigin(y - Vec3(-10, 0, 0));

				//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
				DefaultMotionState* myMotionState = new DefaultMotionState(startTransform);
				RigidBody::RigidBodyConstructionInfo rbInfo(0, 0, 0);
				switch (j % 3)
				{
#if 1
					case 0:
						rbInfo = RigidBody::RigidBodyConstructionInfo(mass, myMotionState, colShape, localInertia);
						break;
					case 1:
						rbInfo = RigidBody::RigidBodyConstructionInfo(mass, myMotionState, colShape3, localInertia);
						break;
#endif
					default:
						rbInfo = RigidBody::RigidBodyConstructionInfo(mass, myMotionState, colShape2, localInertia);
				}
				RigidBody* body = new RigidBody(rbInfo);
				//body->setContactProcessingThreshold(colShape->getContactBreakingThreshold());
				body->setActivationState(ISLAND_SLEEPING);
				body->setLinearFactor(Vec3(1, 1, 0));
				body->setAngularFactor(Vec3(0, 0, 1));

				m_dynamicsWorld->addRigidBody(body);
				body->setActivationState(ISLAND_SLEEPING);

				//	y += -0.8*deltaY;
				y += deltaY;
			}

			x += deltaX;
		}
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void Planar2D::exitPhysics()
{
	//cleanup in the reverse order of creation/initialization

	//remove the rigidbodies from the dynamics world and delete them
	i32 i;
	if (m_dynamicsWorld)
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

	delete m_convexAlgo2d;
	delete m_pdSolver;
	delete m_simplexSolver;
	delete m_box2dbox2dAlgo;

	m_dynamicsWorld = 0;
	m_solver = 0;
	m_broadphase = 0;
	m_dispatcher = 0;
	m_collisionConfiguration = 0;
	m_convexAlgo2d = 0;
	m_pdSolver = 0;
	m_simplexSolver = 0;
	m_box2dbox2dAlgo = 0;
}

CommonExampleInterface* Planar2DCreateFunc(struct CommonExampleOptions& options)
{
	return new Planar2D(options.m_guiHelper);
}
