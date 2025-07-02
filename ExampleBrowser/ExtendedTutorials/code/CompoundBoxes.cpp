#include "../CompoundBoxes.h"
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

struct CompoundBoxesExample : public CommonRigidBodyBase
{
	CompoundBoxesExample(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
	}
	virtual ~CompoundBoxesExample() {}
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

void CompoundBoxesExample::initPhysics()
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
		BoxShape* cube = createBoxShape(Vec3(0.5, 0.5, 0.5));
		m_collisionShapes.push_back(cube);

		// create a new compound shape for making an L-beam from `cube`s
		CompoundShape* compoundShape = new CompoundShape();

		Transform2 transform;

		// add cubes in an L-beam fashion to the compound shape
		transform.setIdentity();
		transform.setOrigin(Vec3(0, 0, 0));
		compoundShape->addChildShape(transform, cube);

		transform.setIdentity();
		transform.setOrigin(Vec3(0, -1, 0));
		compoundShape->addChildShape(transform, cube);

		transform.setIdentity();
		transform.setOrigin(Vec3(0, 0, 1));
		compoundShape->addChildShape(transform, cube);

		Scalar masses[3] = {1, 1, 1};
		Transform2 principal;
		Vec3 inertia;
		compoundShape->calculatePrincipalAxisTransform(masses, principal, inertia);

		// new compund shape to store
		CompoundShape* compound2 = new CompoundShape();
		m_collisionShapes.push_back(compound2);
#if 0
		// less efficient way to add the entire compund shape 
		// to a new compund shape as a child
		compound2->addChildShape(principal.inverse(), compoundShape);
#else
		// recompute the shift to make sure the compound shape is re-aligned
		for (i32 i = 0; i < compoundShape->getNumChildShapes(); i++)
			compound2->addChildShape(compoundShape->getChildTransform(i) * principal.inverse(),
									 compoundShape->getChildShape(i));
#endif
		delete compoundShape;

		transform.setIdentity();
		transform.setOrigin(Vec3(0, 10, 0));
		createRigidBody(1.0, transform, compound2);
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void CompoundBoxesExample::renderScene()
{
	CommonRigidBodyBase::renderScene();
}

CommonExampleInterface* ET_CompoundBoxesCreateFunc(CommonExampleOptions& options)
{
	return new CompoundBoxesExample(options.m_guiHelper);
}
