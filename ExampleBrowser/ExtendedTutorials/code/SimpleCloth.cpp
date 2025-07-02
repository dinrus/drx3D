#include "../SimpleCloth.h"
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>
#include <drx3D/Physics/SoftBody/SoftRigidDynamicsWorld.h>
#include <drx3D/Physics/SoftBody/SoftBodyHelpers.h>
#include <drx3D/Physics/SoftBody/SoftBodyRigidBodyCollisionConfiguration.h>

struct SimpleClothExample : public CommonRigidBodyBase
{
	SimpleClothExample(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
	}
	virtual ~SimpleClothExample() {}
	virtual void initPhysics();
	virtual void renderScene();
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

	void createSoftBody(const Scalar size, i32k num_x, i32k num_z, i32k fixed = 1 + 2);
	SoftBodyWorldInfo softBodyWorldInfo;
};

void SimpleClothExample::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	createEmptyDynamicsWorld();

	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	if (m_dynamicsWorld->getDebugDrawer())
		m_dynamicsWorld->getDebugDrawer()->setDebugMode(IDebugDraw::DBG_DrawWireframe + IDebugDraw::DBG_DrawContactPoints);

	///create a few basic rigid bodies
	BoxShape* groundShape = createBoxShape(Vec3(Scalar(50.), Scalar(50.), Scalar(50.)));
	m_collisionShapes.push_back(groundShape);

	Transform2 groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(Vec3(0, -50, 0));
	{
		Scalar mass(0.);
		createRigidBody(mass, groundTransform, groundShape, Vec4(0, 0, 1, 1));
	}

	{
		const Scalar s = 4;  //size of cloth patch
		i32k NUM_X = 31;  //vertices on X axis
		i32k NUM_Z = 31;  //vertices on Z axis
		createSoftBody(s, NUM_X, NUM_Z);
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void SimpleClothExample::createSoftBody(const Scalar s,
										i32k numX,
										i32k numY,
										i32k fixed)
{
	SoftBody* cloth = SoftBodyHelpers::CreatePatch(softBodyWorldInfo,
													   Vec3(-s / 2, s + 1, 0),
													   Vec3(+s / 2, s + 1, 0),
													   Vec3(-s / 2, s + 1, +s),
													   Vec3(+s / 2, s + 1, +s),
													   numX, numY,
													   fixed, true);

	cloth->getCollisionShape()->setMargin(0.001f);
	cloth->getCollisionShape()->setUserPointer((uk )cloth);
	cloth->generateBendingConstraints(2, cloth->appendMaterial());
	cloth->setTotalMass(10);
	//cloth->m_cfg.citerations = 10;
	//	cloth->m_cfg.diterations = 10;
	cloth->m_cfg.piterations = 5;
	cloth->m_cfg.kDP = 0.005f;
	getSoftDynamicsWorld()->addSoftBody(cloth);
}

void SimpleClothExample::renderScene()
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

CommonExampleInterface* ET_SimpleClothCreateFunc(CommonExampleOptions& options)
{
	return new SimpleClothExample(options.m_guiHelper);
}
