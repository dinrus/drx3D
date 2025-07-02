///FractureDemo shows how to break objects.
///It assumes a CompoundShaps (where the childshapes are the pre-fractured pieces)
///The btFractureBody is a class derived from RigidBody, dealing with the collision impacts.
///Press the F key to toggle between fracture and glue mode
///This is preliminary work

#define CUBE_HALF_EXTENTS 1.f
#define EXTRA_HEIGHT 1.f
///scaling of the objects (0.1 = 20 centimeter boxes )
#define SCALING 1.
#define START_POS_X -5
#define START_POS_Y -5
#define START_POS_Z -3

#include "../FractureDemo.h"
///DynamicsCommon.h is the main drx3D include file, contains most common include files.
#include <drx3D/DynamicsCommon.h>

#include <stdio.h>  //printf debugging

i32 sFrameNumber = 0;

#include "../FractureBody.h"
#include "../FractureDynamicsWorld.h"
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

///FractureDemo shows basic breaking and glueing of objects
class FractureDemo : public CommonRigidBodyBase
{
public:
	FractureDemo(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
	}
	virtual ~FractureDemo()
	{
	}
	void initPhysics();

	void exitPhysics();

	virtual void stepSimulation(float deltaTime)
	{
		CommonRigidBodyBase::stepSimulation(deltaTime);

		{
			DRX3D_PROFILE("recreate graphics");
			//@todo: make this graphics re-creation better
			//right now: brute force remove all graphics objects, and re-create them every frame
			m_guiHelper->getRenderInterface()->removeAllInstances();
			for (i32 i = 0; i < m_dynamicsWorld->getNumCollisionObjects(); i++)
			{
				CollisionObject2* colObj = m_dynamicsWorld->getCollisionObjectArray()[i];
				colObj->getCollisionShape()->setUserIndex(-1);
				colObj->setUserIndex(-1);
			}
			m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
		}
	}

	virtual bool keyboardCallback(i32 key, i32 state);

	void resetCamera()
	{
		float dist = 41;
		float pitch = -35;
		float yaw = 52;
		float targetPos[3] = {0, 0.46, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

void FractureDemo::initPhysics()
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

	//m_dynamicsWorld = new DiscreteDynamicsWorld(m_dispatcher,m_broadphase,m_solver,m_collisionConfiguration);

	FractureDynamicsWorld* fractureWorld = new FractureDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
	m_dynamicsWorld = fractureWorld;
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	//m_splitImpulse removes the penetration resolution from the applied impulse, otherwise objects might fracture due to deep penetrations.
	m_dynamicsWorld->getSolverInfo().m_splitImpulse = true;

	{
		///create a few basic rigid bodies
		CollisionShape* groundShape = new BoxShape(Vec3(50, 1, 50));
		///	CollisionShape* groundShape = new StaticPlaneShape(Vec3(0,1,0),0);
		m_collisionShapes.push_back(groundShape);
		Transform2 groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(Vec3(0, 0, 0));
		createRigidBody(0.f, groundTransform, groundShape);
	}

	{
		///create a few basic rigid bodies
		CollisionShape* shape = new BoxShape(Vec3(1, 1, 1));
		m_collisionShapes.push_back(shape);
		Transform2 tr;
		tr.setIdentity();
		tr.setOrigin(Vec3(5, 2, 0));
		createRigidBody(0.f, tr, shape);
	}

	{
		//create a few dynamic rigidbodies
		// Re-using the same collision is better for memory usage and performance

		CollisionShape* colShape = new BoxShape(Vec3(SCALING * 1, SCALING * 1, SCALING * 1));
		//CollisionShape* colShape = new CapsuleShape(SCALING*0.4,SCALING*1);
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

		i32 gNumObjects = 10;

		for (i32 i = 0; i < gNumObjects; i++)
		{
			Transform2 trans;
			trans.setIdentity();

			Vec3 pos(i * 2 * CUBE_HALF_EXTENTS, 20, 0);
			trans.setOrigin(pos);

			//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
			DefaultMotionState* myMotionState = new DefaultMotionState(trans);
			RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
			FractureBody* body = new FractureBody(rbInfo, m_dynamicsWorld);
			body->setLinearVelocity(Vec3(0, -10, 0));

			m_dynamicsWorld->addRigidBody(body);
		}
	}

	fractureWorld->stepSimulation(1. / 60., 0);
	fractureWorld->glueCallback();

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

#if 0
void FractureDemo::showMessage()
{
	if((getDebugMode() & btIDebugDraw::DBG_DrawText))
	{
		setOrthographicProjection();
		glDisable(GL_LIGHTING);
		glColor3f(0, 0, 0);
		char buf[124];

		i32 lineWidth=380;
		i32 xStart = m_glutScreenWidth - lineWidth;
		i32 yStart = 20;

		btFractureDynamicsWorld* world = (FractureDynamicsWorld*)m_dynamicsWorld;
		if (world->getFractureMode())
		{
			sprintf(buf,"Fracture mode");
		} else
		{
			sprintf(buf,"Glue mode");
		}
		GLDebugDrawString(xStart,yStart,buf);
		sprintf(buf,"f to toggle fracture/glue mode");
		yStart+=20;
		GLDebugDrawString(xStart,yStart,buf);
		sprintf(buf,"space to restart, mouse to pick/shoot");
		yStart+=20;
		GLDebugDrawString(xStart,yStart,buf);

		resetPerspectiveProjection();
		glEnable(GL_LIGHTING);
	}

}
#endif

#if 0
void FractureDemo::displayCallback(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderme();

	showMessage();

	//optional but useful: debug drawing to detect problems
	if (m_dynamicsWorld)
		m_dynamicsWorld->debugDrawWorld();

	glFlush();
	swapBuffers();
}
#endif

bool FractureDemo::keyboardCallback(i32 key, i32 state)
{
	if (key == 'f' && (state == 0))
	{
		FractureDynamicsWorld* world = (FractureDynamicsWorld*)m_dynamicsWorld;
		world->setFractureMode(!world->getFractureMode());
		if (world->getFractureMode())
		{
			drx3DPrintf("Fracturing mode");
		}
		else
		{
			drx3DPrintf("Gluing mode");
		}
		return true;
	}

	return false;
}

#if 0
void FractureDemo::keyboardUpCallback(u8 key, i32 x, i32 y)
{
	if (key=='f')
	{
		btFractureDynamicsWorld* world = (FractureDynamicsWorld*)m_dynamicsWorld;
		world->setFractureMode(!world->getFractureMode());
	}

	PlatformDemoApplication::keyboardUpCallback(key,x,y);

}
#endif

#if 0
void	FractureDemo::shootBox(const Vec3& destination)
{

	if (m_dynamicsWorld)
	{
		Scalar mass = 1.f;
		Transform2 startTransform;
		startTransform.setIdentity();
		Vec3 camPos = getCameraPosition();
		startTransform.setOrigin(camPos);

		setShootBoxShape ();

		Assert((!m_shootBoxShape || m_shootBoxShape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		Vec3 localInertia(0,0,0);
		if (isDynamic)
			m_shootBoxShape->calculateLocalInertia(mass,localInertia);

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

		btFractureBody* body = new btFractureBody(mass,0,m_shootBoxShape,localInertia,&mass,1,m_dynamicsWorld);

		body->setWorldTransform(startTransform);

		m_dynamicsWorld->addRigidBody(body);


		body->setLinearFactor(Vec3(1,1,1));
		//body->setRestitution(1);

		Vec3 linVel(destination[0]-camPos[0],destination[1]-camPos[1],destination[2]-camPos[2]);
		linVel.normalize();
		linVel*=m_ShootBoxInitialSpeed;

		body->getWorldTransform().setOrigin(camPos);
		body->getWorldTransform().setRotation(Quat(0,0,0,1));
		body->setLinearVelocity(linVel);
		body->setAngularVelocity(Vec3(0,0,0));
		body->setCcdMotionThreshold(1.);
		body->setCcdSweptSphereRadius(0.2f);

	}
}
#endif

void FractureDemo::exitPhysics()
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

class CommonExampleInterface* FractureDemoCreateFunc(struct CommonExampleOptions& options)
{
	return new FractureDemo(options.m_guiHelper);
}
