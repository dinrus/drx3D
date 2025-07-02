#include "../SimpleJoint.h"
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

struct SimpleJointExample : public CommonRigidBodyBase
{
	SimpleJointExample(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
	}
	virtual ~SimpleJointExample() {}
	virtual void initPhysics();
	virtual void renderScene();
	void resetCamera()
	{
		float dist = 41;
		float pitch = -35;
		float yaw = 52;
		float targetPos[3] = {0, 0.46, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

void SimpleJointExample::initPhysics()
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
		//create a few dynamic rigidbodies
		// Re-using the same collision is better for memory usage and performance
		BoxShape* colShape = createBoxShape(Vec3(1, 1, 1));

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
			Scalar(0),
			Scalar(10),
			Scalar(0)));
		RigidBody* dynamicBox = createRigidBody(mass, startTransform, colShape);

		//create a static rigid body
		mass = 0;
		startTransform.setOrigin(Vec3(
			Scalar(0),
			Scalar(20),
			Scalar(0)));

		RigidBody* staticBox = createRigidBody(mass, startTransform, colShape);

		//create a simple p2pjoint constraint
		Point2PointConstraint* p2p = new Point2PointConstraint(*dynamicBox, *staticBox, Vec3(0, 3, 0), Vec3(0, 0, 0));
		p2p->m_setting.m_damping = 0.0625;
		p2p->m_setting.m_impulseClamp = 0.95;
		m_dynamicsWorld->addConstraint(p2p);
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void SimpleJointExample::renderScene()
{
	CommonRigidBodyBase::renderScene();
}

CommonExampleInterface* ET_SimpleJointCreateFunc(CommonExampleOptions& options)
{
	return new SimpleJointExample(options.m_guiHelper);
}
