#ifdef USE_GTEST
#include <gtest/gtest.h>
#include "../pendulum_gold.h"
#endif

#include <drx3D/Common/Interfaces/CommonMultiBodyBase.h>

static Scalar radius(0.05);

struct Pendulum : public CommonMultiBodyBase
{
	MultiBody* m_multiBody;
	AlignedObjectArray<MultiBodyJointFeedback*> m_jointFeedbacks;

public:
	Pendulum(struct GUIHelperInterface* helper);
	virtual ~Pendulum();
	virtual void initPhysics();
	virtual void stepSimulation(float deltaTime);
	virtual void resetCamera()
	{
		float dist = 5;
		float pitch = -21;
		float yaw = 270;
		float targetPos[3] = {0, 0, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

Pendulum::Pendulum(struct GUIHelperInterface* helper)
	: CommonMultiBodyBase(helper)
{
}

Pendulum::~Pendulum()
{
}

void Pendulum::initPhysics()
{
	i32 upAxis = 1;

	m_guiHelper->setUpAxis(upAxis);

	this->createEmptyDynamicsWorld();
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	if (m_dynamicsWorld->getDebugDrawer())
	{
		m_dynamicsWorld->getDebugDrawer()->setDebugMode(
			//IDebugDraw::DBG_DrawConstraints
			+IDebugDraw::DBG_DrawWireframe + IDebugDraw::DBG_DrawContactPoints + IDebugDraw::DBG_DrawAabb);  //+btIDebugDraw::DBG_DrawConstraintLimits);
	}
	{
		bool floating = false;
		bool damping = false;
		bool gyro = false;
		i32 numLinks = 1;
		bool canSleep = false;
		bool selfCollide = false;
		Vec3 linkHalfExtents(0.05, 0.5, 0.1);
		Vec3 baseHalfExtents(0.05, 0.5, 0.1);

		Vec3 baseInertiaDiag(0.f, 0.f, 0.f);
		float baseMass = 0.f;

		MultiBody* pMultiBody = new MultiBody(numLinks, baseMass, baseInertiaDiag, !floating, canSleep);
		//pMultiBody->useRK4Integration(true);
		m_multiBody = pMultiBody;
		pMultiBody->setBaseWorldTransform(Transform2::getIdentity());

		//init the links
		Vec3 hingeJointAxis(1, 0, 0);

		//y-axis assumed up
		Vec3 parentComToCurrentCom(0, -linkHalfExtents[1], 0);
		Vec3 currentPivotToCurrentCom(0, -linkHalfExtents[1], 0);
		Vec3 parentComToCurrentPivot = parentComToCurrentCom - currentPivotToCurrentCom;

		for (i32 i = 0; i < numLinks; ++i)
		{
			float linkMass = 10.f;
			Vec3 linkInertiaDiag(0.f, 0.f, 0.f);
			CollisionShape* shape = 0;
			{
				shape = new SphereShape(radius);
			}
			shape->calculateLocalInertia(linkMass, linkInertiaDiag);
			delete shape;

			pMultiBody->setupRevolute(i, linkMass, linkInertiaDiag, i - 1,
									  Quat(0.f, 0.f, 0.f, 1.f),
									  hingeJointAxis,
									  parentComToCurrentPivot,
									  currentPivotToCurrentCom, false);
		}

		pMultiBody->finalizeMultiDof();

		MultiBodyDynamicsWorld* world = m_dynamicsWorld;

		world->addMultiBody(pMultiBody);
		pMultiBody->setCanSleep(canSleep);
		pMultiBody->setHasSelfCollision(selfCollide);
		pMultiBody->setUseGyroTerm(gyro);
		//

		if (!damping)
		{
			pMultiBody->setLinearDamping(0.f);
			pMultiBody->setAngularDamping(0.f);
		}
		else
		{
			pMultiBody->setLinearDamping(0.1f);
			pMultiBody->setAngularDamping(0.9f);
		}
		m_dynamicsWorld->setGravity(Vec3(0, -9.81, 0));

		for (i32 i = 0; i < pMultiBody->getNumLinks(); ++i)
		{
			CollisionShape* shape = new SphereShape(radius);
			m_guiHelper->createCollisionShapeGraphicsObject(shape);
			MultiBodyLinkCollider* col = new MultiBodyLinkCollider(pMultiBody, i);
			col->setCollisionShape(shape);
			bool isDynamic = 1;
			i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
			i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);
			world->addCollisionObject(col, collisionFilterGroup, collisionFilterMask);  //,2,1+2);
			Vec4 color(1, 0, 0, 1);
			m_guiHelper->createCollisionObjectGraphicsObject(col, color);
			pMultiBody->getLink(i).m_collider = col;
		}

		AlignedObjectArray<Quat> scratch_q;
		AlignedObjectArray<Vec3> scratch_m;
		pMultiBody->forwardKinematics(scratch_q, scratch_m);
		AlignedObjectArray<Quat> world_to_local;
		AlignedObjectArray<Vec3> local_origin;
		pMultiBody->updateCollisionObjectWorldTransforms(world_to_local, local_origin);
	}
}

void Pendulum::stepSimulation(float deltaTime)
{
	m_multiBody->addJointTorque(0, 20.0);
#ifdef USE_GTEST
	m_dynamicsWorld->stepSimulation(1. / 1000.0, 0);
#else
	m_dynamicsWorld->stepSimulation(deltaTime);
#endif
	Vec3 from = m_multiBody->getBaseWorldTransform().getOrigin();
	Vec3 to = m_multiBody->getLink(0).m_collider->getWorldTransform().getOrigin();
	Vec4 color(1, 0, 0, 1);
	if (m_guiHelper->getRenderInterface())
	{
		m_guiHelper->getRenderInterface()->drawLine(from, to, color, Scalar(1));
	}
}

#ifdef USE_GTEST

TEST(BulletDynamicsTest, pendulum)
{
	DummyGUIHelper noGfx;
	Pendulum* setup = new Pendulum(&noGfx);
	setup->initPhysics();
	i32 numGoldValues = sizeof(sPendulumGold) / sizeof(float);
	for (i32 i = 0; i < 2000; i++)
	{
		setup->stepSimulation(0.001);
		i32 index = i * 2 + 1;
		ASSERT_LE(index, numGoldValues);
		ASSERT_NEAR(setup->m_multiBody->getJointPos(0), sPendulumGold[index], 0.005);
	}
	setup->exitPhysics();
	delete setup;
}

i32 main(i32 argc, tuk* argv)
{
#if _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//uk testWhetherMemoryLeakDetectionWorks = malloc(1);
#endif
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

#endif  //USE_GTEST

class CommonExampleInterface* TestPendulumCreateFunc(struct CommonExampleOptions& options)
{
	return new Pendulum(options.m_guiHelper);
}
