#include "../RigidBodyFromObj.h"
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Common/b3FileUtils.h>
#include <drx3D/Importers/Obj/LoadMeshFromObj.h>
#include <drx3D/OpenGLWindow/GLInstanceGraphicsShape.h>
#include <drx3D/Common/DefaultFileIO.h>

struct RigidBodyFromObjExample : public CommonRigidBodyBase
{
	i32 m_options;

	RigidBodyFromObjExample(struct GUIHelperInterface* helper, i32 options)
		: CommonRigidBodyBase(helper),
		  m_options(options)
	{
	}
	virtual ~RigidBodyFromObjExample() {}
	virtual void initPhysics();
	virtual void renderScene();
	void resetCamera()
	{
		float dist = 11;
		float pitch = -35;
		float yaw = 52;
		float targetPos[3] = {0, 0.46, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

void RigidBodyFromObjExample::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	createEmptyDynamicsWorld();

	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	//if (m_dynamicsWorld->getDebugDrawer())
	//	m_dynamicsWorld->getDebugDrawer()->setDebugMode(IDebugDraw::DBG_DrawWireframe+btIDebugDraw::DBG_DrawContactPoints);

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

	//load our obj mesh
	tukk fileName = "teddy.obj";  //sphere8.obj";//sponza_closed.obj";//sphere8.obj";
	char relativeFileName[1024];
	if (ResourcePath::findResourcePath(fileName, relativeFileName, 1024,0))
	{
		char pathPrefix[1024];
		b3FileUtils::extractPath(relativeFileName, pathPrefix, 1024);
	}

	DefaultFileIO fileIO;
	GLInstanceGraphicsShape* glmesh = LoadMeshFromObj(relativeFileName, "",&fileIO);
	printf("[INFO] Obj loaded: Extracted %d verticed from obj file [%s]\n", glmesh->m_numvertices, fileName);

	const GLInstanceVertex& v = glmesh->m_vertices->at(0);
	ConvexHullShape* shape = new ConvexHullShape((const Scalar*)(&(v.xyzw[0])), glmesh->m_numvertices, sizeof(GLInstanceVertex));

	float scaling[4] = {0.1, 0.1, 0.1, 1};

	Vec3 localScaling(scaling[0], scaling[1], scaling[2]);
	shape->setLocalScaling(localScaling);

	if (m_options & OptimizeConvexObj)
	{
		shape->optimizeConvexHull();
	}

	if (m_options & ComputePolyhedralFeatures)
	{
		shape->initializePolyhedralFeatures();
	}

	//shape->setMargin(0.001);
	m_collisionShapes.push_back(shape);

	Transform2 startTransform;
	startTransform.setIdentity();

	Scalar mass(1.f);
	bool isDynamic = (mass != 0.f);
	Vec3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	float color[4] = {1, 1, 1, 1};
	float orn[4] = {0, 0, 0, 1};
	float pos[4] = {0, 3, 0, 0};
	Vec3 position(pos[0], pos[1], pos[2]);
	startTransform.setOrigin(position);
	RigidBody* body = createRigidBody(mass, startTransform, shape);

	bool useConvexHullForRendering = ((m_options & ObjUseConvexHullForRendering) != 0);

	if (!useConvexHullForRendering)
	{
		i32 shapeId = m_guiHelper->registerGraphicsShape(&glmesh->m_vertices->at(0).xyzw[0],
														 glmesh->m_numvertices,
														 &glmesh->m_indices->at(0),
														 glmesh->m_numIndices,
														 D3_GL_TRIANGLES, -1);
		shape->setUserIndex(shapeId);
		i32 renderInstance = m_guiHelper->registerGraphicsInstance(shapeId, pos, orn, color, scaling);
		body->setUserIndex(renderInstance);
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void RigidBodyFromObjExample::renderScene()
{
	CommonRigidBodyBase::renderScene();
}

CommonExampleInterface* ET_RigidBodyFromObjCreateFunc(CommonExampleOptions& options)
{
	return new RigidBodyFromObjExample(options.m_guiHelper, options.m_option);
}

D3_STANDALONE_EXAMPLE(ET_RigidBodyFromObjCreateFunc)
