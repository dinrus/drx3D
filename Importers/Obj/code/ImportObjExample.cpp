#include "../ImportObjExample.h"
#include <vector>
#include  <drx3D/OpenGLWindow/GLInstancingRenderer.h>
#include  <drx3D/Wavefront/tiny_obj_loader.h>
#include  <drx3D/OpenGLWindow/GLInstanceGraphicsShape.h>
#include <drx3D/DynamicsCommon.h>
#include  <drx3D/OpenGLWindow/SimpleOpenGL3App.h>
#include "../Wavefront2GLInstanceGraphicsShape.h"
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Common/DefaultFileIO.h>
#include <drx3D/Common/b3FileUtils.h>

#include <X/stb/stb_image.h>

#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>
#include <drx3D/Importers/MeshUtility/b3ImportMeshUtility.h>

class ImportObjSetup : public CommonRigidBodyBase
{
	STxt m_fileName;

public:
	ImportObjSetup(struct GUIHelperInterface* helper, tukk fileName);
	virtual ~ImportObjSetup();

	virtual void initPhysics();

	virtual void resetCamera()
	{
		float dist = 18;
		float pitch = -46;
		float yaw = 120;
		float targetPos[3] = {-2, -2, -2};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

ImportObjSetup::ImportObjSetup(struct GUIHelperInterface* helper, tukk fileName)
	: CommonRigidBodyBase(helper)
{
	if (fileName)
	{
		m_fileName = fileName;
	}
	else
	{
		m_fileName = "cube.obj";  //"sponza_closed.obj";//sphere8.obj";
	}
}

ImportObjSetup::~ImportObjSetup()
{
}

i32 loadAndRegisterMeshFromFile2(const STxt& fileName, CommonRenderInterface* renderer)
{
	i32 shapeId = -1;

	b3ImportMeshData meshData;
	DefaultFileIO fileIO;
	if (b3ImportMeshUtility::loadAndRegisterMeshFromFileInternal(fileName, meshData,&fileIO))
	{
		i32 textureIndex = -1;

		if (meshData.m_textureImage1)
		{
			textureIndex = renderer->registerTexture(meshData.m_textureImage1, meshData.m_textureWidth, meshData.m_textureHeight);
		}

		shapeId = renderer->registerShape(&meshData.m_gfxShape->m_vertices->at(0).xyzw[0],
										  meshData.m_gfxShape->m_numvertices,
										  &meshData.m_gfxShape->m_indices->at(0),
										  meshData.m_gfxShape->m_numIndices,
										  D3_GL_TRIANGLES,
										  textureIndex);
		delete meshData.m_gfxShape;
		if (!meshData.m_isCached)
		{
			delete meshData.m_textureImage1;
		}
	}
	return shapeId;
}

void ImportObjSetup::initPhysics()
{
	m_guiHelper->setUpAxis(2);
	this->createEmptyDynamicsWorld();
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	m_dynamicsWorld->getDebugDrawer()->setDebugMode(IDebugDraw::DBG_DrawWireframe);

	Transform2 trans;
	trans.setIdentity();
	trans.setRotation(Quat(Vec3(1, 0, 0), SIMD_HALF_PI));
	Vec3 position = trans.getOrigin();
	Quat orn = trans.getRotation();

	Vec3 scaling(1, 1, 1);
	Vec4 color(1, 1, 1,1);

	i32 shapeId = loadAndRegisterMeshFromFile2(m_fileName, m_guiHelper->getRenderInterface());
	if (shapeId >= 0)
	{
		//i32 id =
		m_guiHelper->getRenderInterface()->registerGraphicsInstance(shapeId, position, orn, color, scaling);
	}
}

CommonExampleInterface* ImportObjCreateFunc(struct CommonExampleOptions& options)
{
	return new ImportObjSetup(options.m_guiHelper, options.m_fileName);
}
