#include "../Bridge.h"
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

i32k TOTAL_PLANKS = 10;
struct BridgeExample : public CommonRigidBodyBase
{
	BridgeExample(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
	}
	virtual ~BridgeExample() {}
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

void BridgeExample::initPhysics()
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

	//create two fixed boxes to hold the planks

	{
		//create a few dynamic rigidbodies
		// Re-using the same collision is better for memory usage and performance
		Scalar plankWidth = 0.4;
		Scalar plankHeight = 0.2;
		Scalar plankBreadth = 1;
		Scalar plankOffset = plankWidth;  //distance between two planks
		Scalar bridgeWidth = plankWidth * TOTAL_PLANKS + plankOffset * (TOTAL_PLANKS - 1);
		Scalar bridgeHeight = 5;
		Scalar halfBridgeWidth = bridgeWidth * 0.5f;

		BoxShape* colShape = createBoxShape(Vec3(plankWidth, plankHeight, plankBreadth));

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

		//create a set of boxes to represent bridge
		AlignedObjectArray<RigidBody*> boxes;
		i32 lastBoxIndex = TOTAL_PLANKS - 1;
		for (i32 i = 0; i < TOTAL_PLANKS; ++i)
		{
			float t = float(i) / lastBoxIndex;
			t = -(t * 2 - 1.0f) * halfBridgeWidth;
			startTransform.setOrigin(Vec3(
				Scalar(t),
				bridgeHeight,
				Scalar(0)));
			boxes.push_back(createRigidBody((i == 0 || i == lastBoxIndex) ? 0 : mass, startTransform, colShape));
		}

		//add N-1 spring constraints
		for (i32 i = 0; i < TOTAL_PLANKS - 1; ++i)
		{
			RigidBody* b1 = boxes[i];
			RigidBody* b2 = boxes[i + 1];

			Point2PointConstraint* leftSpring = new Point2PointConstraint(*b1, *b2, Vec3(-0.5, 0, -0.5), Vec3(0.5, 0, -0.5));
			m_dynamicsWorld->addConstraint(leftSpring);

			Point2PointConstraint* rightSpring = new Point2PointConstraint(*b1, *b2, Vec3(-0.5, 0, 0.5), Vec3(0.5, 0, 0.5));
			m_dynamicsWorld->addConstraint(rightSpring);
		}
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void BridgeExample::renderScene()
{
	CommonRigidBodyBase::renderScene();
}

CommonExampleInterface* ET_BridgeCreateFunc(CommonExampleOptions& options)
{
	return new BridgeExample(options.m_guiHelper);
}
