#include "../GyroscopicSetup.h"
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

struct GyroscopicSetup : public CommonRigidBodyBase
{
	GyroscopicSetup(struct GUIHelperInterface* helper);

	virtual ~GyroscopicSetup()
	{
	}
	virtual void initPhysics();

	virtual void physicsDebugDraw(i32 debugFlags);

	void resetCamera()
	{
		float dist = 20;
		float pitch = -16;
		float yaw = 180;
		float targetPos[3] = {-2.4, 0.4, -0.24};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

static i32 gyroflags[4] = {
	0,  //none, no gyroscopic term
	DRX3D_ENABLE_GYROSCOPIC_FORCE_EXPLICIT,
	DRX3D_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_WORLD,
	DRX3D_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY};

static tukk gyroNames[4] = {
	"No Gyroscopic",
	"Explicit",
	"Implicit (World)",
	"Implicit (Body)"};

GyroscopicSetup::GyroscopicSetup(struct GUIHelperInterface* helper)
	: CommonRigidBodyBase(helper)
{
}

void GyroscopicSetup::initPhysics()
{
	m_guiHelper->setUpAxis(1);
	createEmptyDynamicsWorld();
	m_dynamicsWorld->setGravity(Vec3(0, 0, 0));
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	Vec3 positions[4] = {
		Vec3(-10, 8, 4),
		Vec3(-5, 8, 4),
		Vec3(0, 8, 4),
		Vec3(5, 8, 4),
	};

	for (i32 i = 0; i < 4; i++)
	{
		CylinderShapeZ* pin = new CylinderShapeZ(Vec3(0.1, 0.1, 0.2));
		BoxShape* box = new BoxShape(Vec3(1, 0.1, 0.1));
		box->setMargin(0.01);
		pin->setMargin(0.01);
		CompoundShape* compound = new CompoundShape();
		compound->addChildShape(Transform2::getIdentity(), pin);
		Transform2 offsetBox(Matrix3x3::getIdentity(), Vec3(0, 0, 0.2));
		compound->addChildShape(offsetBox, box);
		Scalar masses[2] = {0.3, 0.1};
		Vec3 localInertia;
		Transform2 principal;
		compound->calculatePrincipalAxisTransform(masses, principal, localInertia);

		RigidBody* body = new RigidBody(1, 0, compound, localInertia);
		Transform2 tr;
		tr.setIdentity();
		tr.setOrigin(positions[i]);
		body->setCenterOfMassTransform(tr);
		body->setAngularVelocity(Vec3(0, 0.1, 10));  //51));
		//body->setLinearVelocity(Vec3(3, 0, 0));
		body->setFriction(Sqrt(1));
		m_dynamicsWorld->addRigidBody(body);
		body->setFlags(gyroflags[i]);
		m_dynamicsWorld->getSolverInfo().m_maxGyroscopicForce = 10.f;
		body->setDamping(0.0000f, 0.000f);
	}

	{
		//CollisionShape* groundShape = new BoxShape(Vec3(Scalar(50.),Scalar(50.),Scalar(0.5)));
		CollisionShape* groundShape = new StaticPlaneShape(Vec3(0, 1, 0), 0);

		m_collisionShapes.push_back(groundShape);
		Transform2 groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(Vec3(0, 0, 0));
		RigidBody* groundBody;
		groundBody = createRigidBody(0, groundTransform, groundShape);
		groundBody->setFriction(Sqrt(2));
	}
	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void GyroscopicSetup::physicsDebugDraw(i32 debugFlags)
{
	CommonRigidBodyBase::physicsDebugDraw(debugFlags);

	//render method names above objects
	for (i32 i = 0; i < m_dynamicsWorld->getNumCollisionObjects(); i++)
	{
		RigidBody* body = RigidBody::upcast(m_dynamicsWorld->getCollisionObjectArray()[i]);
		if (body && body->getInvMass() > 0)
		{
			Transform2 tr = body->getWorldTransform();
			Vec3 pos = tr.getOrigin() + Vec3(0, 0, 2);
			Scalar size = 1;
			m_guiHelper->drawText3D(gyroNames[i], pos.x(), pos.y(), pos.z(), size);
		}
	}
}

class CommonExampleInterface* GyroscopicCreateFunc(CommonExampleOptions& options)
{
	return new GyroscopicSetup(options.m_guiHelper);
}
