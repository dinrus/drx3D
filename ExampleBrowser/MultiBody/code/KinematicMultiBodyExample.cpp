#include "../KinematicMultiBodyExample.h"
//#define USE_MOTIONSTATE 1
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/Transform2Util.h>
#define ARRAY_SIZE_Y 5
#define ARRAY_SIZE_X 5
#define ARRAY_SIZE_Z 5

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/OpenGLWindow/ShapeData.h>
#include <drx3D/Common/Interfaces/CommonMultiBodyBase.h>

namespace {

void kinematicPreTickCallback(DynamicsWorld* world, Scalar deltaTime)
{
	MultiBody* groundBody = (MultiBody*)world->getWorldUserInfo();
	Transform2 predictedTrans;
	Vec3 linearVelocity(0, 0, 0);
	Vec3 angularVelocity(0, 0.1, 0);
	Transform2Util::integrateTransform(groundBody->getBaseWorldTransform(), linearVelocity, angularVelocity, deltaTime, predictedTrans);
	groundBody->setBaseWorldTransform(predictedTrans);
	groundBody->setBaseVel(linearVelocity);
	groundBody->setBaseOmega(angularVelocity);

	static float time = 0.0;
	time += deltaTime;
	double old_joint_pos = groundBody->getJointPos(0);
	double joint_pos = 0.5 * sin(time * 3.0 - 0.3);
	double joint_vel = (joint_pos - old_joint_pos) / deltaTime;
	groundBody->setJointPosMultiDof(0, &joint_pos);
	groundBody->setJointVelMultiDof(0, &joint_vel);
}

struct KinematicMultiBodyExample : public CommonMultiBodyBase
{
	MultiBody* m_groundBody;

	KinematicMultiBodyExample(struct GUIHelperInterface* helper)
		: CommonMultiBodyBase(helper),
		  m_groundBody(0)
	{
	}

	virtual void stepSimulation(float deltaTime)
	{
		if (m_dynamicsWorld)
		{
			m_dynamicsWorld->stepSimulation(deltaTime);
		}
	}

	virtual ~KinematicMultiBodyExample() {}
	virtual void initPhysics();
	void resetCamera()
	{
		float dist = 4;
		float pitch = -30;
		float yaw = 50;
		float targetPos[3] = {0, 0, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

void KinematicMultiBodyExample::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	createEmptyDynamicsWorld();
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	if (m_dynamicsWorld->getDebugDrawer())
		m_dynamicsWorld->getDebugDrawer()->setDebugMode(IDebugDraw::DBG_DrawWireframe + IDebugDraw::DBG_DrawContactPoints);

	///create a kinematic multibody
	BoxShape* groundShape = createBoxShape(Vec3(Scalar(10.), Scalar(0.1), Scalar(10.)));
	m_collisionShapes.push_back(groundShape);

	BoxShape* secondLevelShape = createBoxShape(Vec3(Scalar(0.5), Scalar(0.1), Scalar(0.5)));
	m_collisionShapes.push_back(secondLevelShape);

	{
		bool floating = false;
		i32 numLinks = 1;
		bool canSleep = false;
		Vec3 baseInertiaDiag(0.f, 0.f, 0.f);
		float baseMass = 1.f;
		Vec3 secondLevelInertiaDiag(0.f, 0.f, 0.f);
		float secondLevelMass = 0.1f;

		if (baseMass)
		{
			CollisionShape* pTempBox = new BoxShape(Vec3(10, 10, 10));
			pTempBox->calculateLocalInertia(baseMass, baseInertiaDiag);
			delete pTempBox;
		}
		if (secondLevelMass)
		{
			CollisionShape* pTempBox = new BoxShape(Vec3(0.5, 0.5, 0.5));
			pTempBox->calculateLocalInertia(secondLevelMass, secondLevelInertiaDiag);
			delete pTempBox;
		}
		Transform2 startTransform;
		startTransform.setIdentity();

		m_groundBody = new MultiBody(numLinks, baseMass, baseInertiaDiag, !floating, canSleep);
		m_groundBody->setBasePos(startTransform.getOrigin());
		m_groundBody->setWorldToBaseRot(startTransform.getRotation());

		//init the child link - second level.
		Vec3 hingeJointAxis(0, 1, 0);
		m_groundBody->setupRevolute(0, secondLevelMass, secondLevelInertiaDiag, -1, Quat(0.f, 0.f, 0.f, 1.f), hingeJointAxis, Vec3(0, 0.5, 0), Vec3(0, 0, 0), true);

		m_groundBody->finalizeMultiDof();
		m_dynamicsWorld->addMultiBody(m_groundBody);

		// add collision geometries
		bool isDynamic = false; // Kinematic is not treated as dynamic here.
		i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
		i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);

		MultiBodyLinkCollider* col = new MultiBodyLinkCollider(m_groundBody, -1);
		col->setCollisionShape(groundShape);
		m_dynamicsWorld->addCollisionObject(col, collisionFilterGroup, collisionFilterMask);  //, 2,1+2);
		m_groundBody->setBaseCollider(col);
		m_groundBody->setBaseDynamicType(CollisionObject2::CF_KINEMATIC_OBJECT);

		MultiBodyLinkCollider* secondLevelCol = new MultiBodyLinkCollider(m_groundBody, 0);
		secondLevelCol->setCollisionShape(secondLevelShape);
		m_dynamicsWorld->addCollisionObject(secondLevelCol, collisionFilterGroup, collisionFilterMask);
		m_groundBody->getLink(0).m_collider = secondLevelCol;
		m_groundBody->setLinkDynamicType(0, CollisionObject2::CF_KINEMATIC_OBJECT);
	}
	m_dynamicsWorld->setInternalTickCallback(kinematicPreTickCallback, m_groundBody, true);

	{
		//create a few dynamic rigidbodies
		// Re-using the same collision is better for memory usage and performance
		BoxShape* colShape = createBoxShape(Vec3(.1, .1, .1));

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

		for (i32 k = 0; k < ARRAY_SIZE_Y; k++)
		{
			for (i32 i = 0; i < ARRAY_SIZE_X; i++)
			{
				for (i32 j = 0; j < ARRAY_SIZE_Z; j++)
				{
					startTransform.setOrigin(Vec3(
						Scalar(0.2 * i),
						Scalar(2 + .2 * k),
						Scalar(0.2 * j)));

					createRigidBody(mass, startTransform, colShape);
				}
			}
		}
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

}

CommonExampleInterface* KinematicMultiBodyExampleCreateFunc(CommonExampleOptions& options)
{
	return new KinematicMultiBodyExample(options.m_guiHelper);
}

D3_STANDALONE_EXAMPLE(KinematicMultiBodyExampleCreateFunc)
