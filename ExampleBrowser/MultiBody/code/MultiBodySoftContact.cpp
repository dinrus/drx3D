#include "../MultiBodySoftContact.h"
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointFeedback.h>
#include <drx3D/Common/Interfaces/CommonMultiBodyBase.h>
#include <drx3D/Common/ResourcePath.h>

//static Scalar radius(0.2);

struct MultiBodySoftContact : public CommonMultiBodyBase
{
	MultiBody* m_multiBody;
	AlignedObjectArray<MultiBodyJointFeedback*> m_jointFeedbacks;

	bool m_once;

public:
	MultiBodySoftContact(struct GUIHelperInterface* helper);
	virtual ~MultiBodySoftContact();

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

MultiBodySoftContact::MultiBodySoftContact(struct GUIHelperInterface* helper)
	: CommonMultiBodyBase(helper),
	  m_once(true)
{
}

MultiBodySoftContact::~MultiBodySoftContact()
{
}

void MultiBodySoftContact::initPhysics()
{
	i32 upAxis = 2;

	m_guiHelper->setUpAxis(upAxis);

	Vec4 colors[4] =
		{
			Vec4(1, 0, 0, 1),
			Vec4(0, 1, 0, 1),
			Vec4(0, 1, 1, 1),
			Vec4(1, 1, 0, 1),
		};
	i32 curColor = 0;

	this->createEmptyDynamicsWorld();
	m_dynamicsWorld->setGravity(Vec3(0, 0, -10));

	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	m_dynamicsWorld->getDebugDrawer()->setDebugMode(
		//IDebugDraw::DBG_DrawConstraints
		+IDebugDraw::DBG_DrawWireframe + IDebugDraw::DBG_DrawContactPoints + IDebugDraw::DBG_DrawAabb);  //+btIDebugDraw::DBG_DrawConstraintLimits);

	//create a static ground object
	if (1)
	{
		Vec3 groundHalfExtents(50, 50, 50);
		BoxShape* box = new BoxShape(groundHalfExtents);
		box->initializePolyhedralFeatures();

		m_guiHelper->createCollisionShapeGraphicsObject(box);
		Transform2 start;
		start.setIdentity();
		Vec3 groundOrigin(0, 0, -50.5);
		start.setOrigin(groundOrigin);
		//	start.setRotation(groundOrn);
		RigidBody* body = createRigidBody(0, start, box);

		//setContactStiffnessAndDamping will enable compliant rigid body contact
		body->setContactStiffnessAndDamping(300, 10);
		Vec4 color = colors[curColor];
		curColor++;
		curColor &= 3;
		m_guiHelper->createRigidBodyGraphicsObject(body, color);
	}

	{
		CollisionShape* childShape = new SphereShape(Scalar(0.5));
		m_guiHelper->createCollisionShapeGraphicsObject(childShape);

		Scalar mass = 1;
		Vec3 baseInertiaDiag;
		bool isFixed = (mass == 0);
		childShape->calculateLocalInertia(mass, baseInertiaDiag);
		MultiBody* pMultiBody = new MultiBody(0, 1, baseInertiaDiag, false, false);
		Transform2 startTrans;
		startTrans.setIdentity();
		startTrans.setOrigin(Vec3(0, 0, 3));

		pMultiBody->setBaseWorldTransform(startTrans);

		MultiBodyLinkCollider* col = new MultiBodyLinkCollider(pMultiBody, -1);
		col->setCollisionShape(childShape);
		pMultiBody->setBaseCollider(col);
		bool isDynamic = (mass > 0 && !isFixed);
		i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
		i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);

		m_dynamicsWorld->addCollisionObject(col, collisionFilterGroup, collisionFilterMask);  //, 2,1+2);

		pMultiBody->finalizeMultiDof();

		m_dynamicsWorld->addMultiBody(pMultiBody);

		AlignedObjectArray<Quat> scratch_q;
		AlignedObjectArray<Vec3> scratch_m;
		pMultiBody->forwardKinematics(scratch_q, scratch_m);
		AlignedObjectArray<Quat> world_to_local;
		AlignedObjectArray<Vec3> local_origin;
		pMultiBody->updateCollisionObjectWorldTransforms(world_to_local, local_origin);
	}
	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void MultiBodySoftContact::stepSimulation(float deltaTime)
{
	if (/* DISABLES CODE */ (0))  //m_once)
	{
		m_once = false;
		m_multiBody->addJointTorque(0, 10.0);

		Scalar torque = m_multiBody->getJointTorque(0);
		drx3DPrintf("t = %f,%f,%f\n", torque, torque, torque);  //[0],torque[1],torque[2]);
	}

	m_dynamicsWorld->stepSimulation(deltaTime);
}

class CommonExampleInterface* MultiBodySoftContactCreateFunc(struct CommonExampleOptions& options)
{
	return new MultiBodySoftContact(options.m_guiHelper);
}
