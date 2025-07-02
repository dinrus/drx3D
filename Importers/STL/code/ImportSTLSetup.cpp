#include "../ImportSTLSetup.h"
#include <vector>
#include <drx3D/OpenGLWindow/GLInstancingRenderer.h>
#include <drx3D/OpenGLWindow/GLInstanceGraphicsShape.h>
#include <drx3D/DynamicsCommon.h>
#include <drx3D/OpenGLWindow/SimpleOpenGL3App.h>
#include "../LoadMeshFromSTL.h"
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Common/DefaultFileIO.h>

class ImportSTLSetup : public CommonRigidBodyBase
{
	tukk m_fileName;
	Vec3 m_scaling;

public:
	ImportSTLSetup(struct GUIHelperInterface* helper, tukk fileName);
	virtual ~ImportSTLSetup();

	virtual void initPhysics();
	virtual void resetCamera()
	{
		float dist = 3.5;
		float pitch = -28;
		float yaw = -136;
		float targetPos[3] = {0.47, 0, -0.64};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

ImportSTLSetup::ImportSTLSetup(struct GUIHelperInterface* helper, tukk fileName)
	: CommonRigidBodyBase(helper),
	  m_scaling(Vec3(10, 10, 10))
{
	if (fileName)
	{
		m_fileName = fileName;
		m_scaling = Vec3(0.01, 0.01, 0.01);
	}
	else
	{
		m_fileName = "l_finger_tip.stl";
	}
}

ImportSTLSetup::~ImportSTLSetup()
{
}

void ImportSTLSetup::initPhysics()
{
	m_guiHelper->setUpAxis(2);
	this->createEmptyDynamicsWorld();
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	m_dynamicsWorld->getDebugDrawer()->setDebugMode(IDebugDraw::DBG_DrawWireframe);

	char relativeFileName[1024];
	if (!ResourcePath::findResourcePath(m_fileName, relativeFileName, 1024,0))
	{
		drx3DWarning("Не найден файл %s\n", m_fileName);
		return;
	}

	Vec3 shift(0, 0, 0);
	//	i32 index=10;

	{
		DefaultFileIO fileIO;
		GLInstanceGraphicsShape* gfxShape = LoadMeshFromSTL(relativeFileName,&fileIO);

		Transform2 trans;
		trans.setIdentity();
		trans.setRotation(Quat(Vec3(1, 0, 0), SIMD_HALF_PI));

		Vec3 position = trans.getOrigin();
		Quat orn = trans.getRotation();

		Vec4 color(0, 0, 1,1);

		i32 shapeId = m_guiHelper->getRenderInterface()->registerShape(&gfxShape->m_vertices->at(0).xyzw[0], gfxShape->m_numvertices, &gfxShape->m_indices->at(0), gfxShape->m_numIndices);

		m_guiHelper->getRenderInterface()->registerGraphicsInstance(shapeId, position, orn, color, m_scaling);
	}
}

class CommonExampleInterface* ImportSTLCreateFunc(struct CommonExampleOptions& options)
{
	return new ImportSTLSetup(options.m_guiHelper, options.m_fileName);
}
