#include "../Chain.h"
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

i32k TOTAL_BOXES = 10;
struct ChainExample : public CommonRigidBodyBase
{
	ChainExample(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
	}
	virtual ~ChainExample() {}
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

void ChainExample::initPhysics()
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
		BoxShape* colShape = createBoxShape(Vec3(1, 1, 0.25));

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

		AlignedObjectArray<RigidBody*> boxes;
		i32 lastBoxIndex = TOTAL_BOXES - 1;
		for (i32 i = 0; i < TOTAL_BOXES; ++i)
		{
			startTransform.setOrigin(Vec3(
				Scalar(0),
				Scalar(5 + i * 2),
				Scalar(0)));
			boxes.push_back(createRigidBody((i == lastBoxIndex) ? 0 : mass, startTransform, colShape));
		}

		//add N-1 spring constraints
		for (i32 i = 0; i < TOTAL_BOXES - 1; ++i)
		{
			RigidBody* b1 = boxes[i];
			RigidBody* b2 = boxes[i + 1];

			Point2PointConstraint* leftSpring = new Point2PointConstraint(*b1, *b2, Vec3(-0.5, 1, 0), Vec3(-0.5, -1, 0));

			m_dynamicsWorld->addConstraint(leftSpring);

			Point2PointConstraint* rightSpring = new Point2PointConstraint(*b1, *b2, Vec3(0.5, 1, 0), Vec3(0.5, -1, 0));

			m_dynamicsWorld->addConstraint(rightSpring);
		}
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void ChainExample::renderScene()
{
	CommonRigidBodyBase::renderScene();
}

CommonExampleInterface* ET_ChainCreateFunc(CommonExampleOptions& options)
{
	return new ChainExample(options.m_guiHelper);
}
