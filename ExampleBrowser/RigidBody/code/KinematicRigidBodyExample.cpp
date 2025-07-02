#include "../KinematicRigidBodyExample.h"
//#define USE_MOTIONSTATE 1
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/Transform2Util.h>

#define ARRAY_SIZE_Y 5
#define ARRAY_SIZE_X 5
#define ARRAY_SIZE_Z 5

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include  <drx3D/OpenGLWindow/ShapeData.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

void kinematicPreTickCallback(DynamicsWorld* world, Scalar deltaTime)
{
	RigidBody* groundBody = (RigidBody*)world->getWorldUserInfo();
	Transform2 predictedTrans;
	Vec3 linearVelocity(0, 0, 0);
	Vec3 angularVelocity(0, 0.1, 0);
	Transform2Util::integrateTransform(groundBody->getWorldTransform(), linearVelocity, angularVelocity, deltaTime, predictedTrans);
#ifdef USE_MOTIONSTATE
	groundBody->getMotionState()->setWorldTransform(predictedTrans);
#else
	m_groundBody->setWorldTransform(predictedTrans);
#endif
}

struct KinematicRigidBodyExample : public CommonRigidBodyBase
{
	RigidBody* m_groundBody;

	KinematicRigidBodyExample(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper),
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

	virtual ~KinematicRigidBodyExample() {}
	virtual void initPhysics();
	virtual void renderScene();
	void resetCamera()
	{
		float dist = 4;
		float pitch = -30;
		float yaw = 50;
		float targetPos[3] = {0, 0, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

void KinematicRigidBodyExample::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	createEmptyDynamicsWorld();
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	if (m_dynamicsWorld->getDebugDrawer())
		m_dynamicsWorld->getDebugDrawer()->setDebugMode(IDebugDraw::DBG_DrawWireframe + IDebugDraw::DBG_DrawContactPoints);

	///create a few basic rigid bodies
	Scalar halfExtentsX = 10.0;
	Scalar halfExtentsY = 0.1;
	Scalar halfExtentsZ = 10.0;

	BoxShape* groundShape = createBoxShape(Vec3(Scalar(10.), Scalar(0.1), Scalar(10.)));
	Transform2 groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(Vec3(0, -halfExtentsY, 0));
	m_collisionShapes.push_back(groundShape);



	{
		Scalar mass(0.);
		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		Vec3 localInertia(0, 0, 0);

#ifdef USE_MOTIONSTATE
		DefaultMotionState* myMotionState = new DefaultMotionState(groundTransform);
		RigidBody::RigidBodyConstructionInfo cInfo(mass, myMotionState, groundShape, localInertia);
		m_groundBody = new RigidBody(cInfo);
#else
		m_groundBody = new RigidBody(mass, 0, shape, localInertia);
		m_groundBody->setWorldTransform(startTransform);
#endif  //

		m_groundBody->setUserIndex(-1);
		
		m_groundBody->forceActivationState(DISABLE_DEACTIVATION);
		m_groundBody->setCollisionFlags(CollisionObject2::CF_KINEMATIC_OBJECT | CollisionObject2::CF_STATIC_OBJECT);
		m_dynamicsWorld->addRigidBody(m_groundBody);
		
	}
	m_dynamicsWorld->setInternalTickCallback(kinematicPreTickCallback, m_groundBody, true);
	{
		i32 strideInBytes = 9 * sizeof(float);
		i32 numVertices = sizeof(cube_vertices_textured) / strideInBytes;
		i32 numIndices = sizeof(cube_indices) / sizeof(i32);
		Scalar textureScaling = 40.0;
		AlignedObjectArray<GfxVertexFormat1> verts;
		verts.resize(numVertices);
		for (i32 i = 0; i < numVertices; i++)
		{
			verts[i].x = halfExtentsX * cube_vertices_textured[i * 9];
			verts[i].y = halfExtentsY * cube_vertices_textured[i * 9 + 1];
			verts[i].z = halfExtentsZ * cube_vertices_textured[i * 9 + 2];
			verts[i].w = cube_vertices_textured[i * 9 + 3];
			verts[i].nx = cube_vertices_textured[i * 9 + 4];
			verts[i].ny = cube_vertices_textured[i * 9 + 5];
			verts[i].nz = cube_vertices_textured[i * 9 + 6];
			verts[i].u = cube_vertices_textured[i * 9 + 7] * textureScaling;
			verts[i].v = cube_vertices_textured[i * 9 + 8] * textureScaling;
		}

		i32 red = 173;
		i32 green = 199;
		i32 blue = 255;
		AlignedObjectArray<u8> rgbaTexture;
		i32 textureWidth = 256;
		i32 textureHeight = 256;
		rgbaTexture.resize(textureWidth * textureHeight * 3);

		for (i32 i = 0; i < textureWidth * textureHeight * 3; i++)
			rgbaTexture[i] = 255;
		for (i32 i = 0; i < textureWidth; i++)
		{
			for (i32 j = 0; j < textureHeight; j++)
			{
				i32 a = i < textureWidth / 2 ? 1 : 0;
				i32 b = j < textureWidth / 2 ? 1 : 0;

				if (a == b)
				{
					rgbaTexture[(i + j * textureWidth) * 3 + 0] = red;
					rgbaTexture[(i + j * textureWidth) * 3 + 1] = green;
					rgbaTexture[(i + j * textureWidth) * 3 + 2] = blue;
				}
			}
		}
		bool flipPixelsY = false;
		i32 textureIndex = m_guiHelper->getRenderInterface()->registerTexture(&rgbaTexture[0], textureWidth, textureHeight, flipPixelsY);
		i32 shapeId = m_guiHelper->getRenderInterface()->registerShape(&verts[0].x, numVertices, cube_indices, numIndices, D3_GL_TRIANGLES, textureIndex);
		Vec3 scaling(1, 1, 1);
		Vec4 color(1, 1, 1, 1);
		Quat orn;
		groundTransform.getBasis().getRotation(orn);
		i32 graphicsInstanceId = m_guiHelper->getRenderInterface()->registerGraphicsInstance(shapeId, groundTransform.getOrigin(), orn, color, scaling);
		groundShape->setUserIndex(shapeId);
		m_groundBody->setUserIndex(graphicsInstanceId);
	
	}

	if (1)
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

void KinematicRigidBodyExample::renderScene()
{
	CommonRigidBodyBase::renderScene();
}

CommonExampleInterface* KinematicRigidBodyExampleCreateFunc(CommonExampleOptions& options)
{
	return new KinematicRigidBodyExample(options.m_guiHelper);
}

D3_STANDALONE_EXAMPLE(KinematicRigidBodyExampleCreateFunc)
