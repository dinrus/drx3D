#include "../ImportColladaSetup.h"
#include <vector>
#include <drx3D/OpenGLWindow/GLInstancingRenderer.h>
#include <drx3D/OpenGLWindow/GLInstanceGraphicsShape.h>
#include <drx3D/DynamicsCommon.h>
#include <drx3D/OpenGLWindow/SimpleOpenGL3App.h>
#include "../LoadMeshFromCollada.h"
#include <drx3D/Common/b3FileUtils.h>
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Common/DefaultFileIO.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

class ImportColladaSetup : public CommonRigidBodyBase
{
public:
	ImportColladaSetup(struct GUIHelperInterface* helper);
	virtual ~ImportColladaSetup();

	virtual void initPhysics();
	virtual void resetCamera()
	{
		float dist = 16;
		float pitch = -28;
		float yaw = -140;
		float targetPos[3] = {-4, -3, -3};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

ImportColladaSetup::ImportColladaSetup(struct GUIHelperInterface* helper)
	: CommonRigidBodyBase(helper)
{
}

ImportColladaSetup::~ImportColladaSetup()
{
}

static i32 ColladaGraphicsInstanceSortfnc(const ColladaGraphicsInstance& a, const ColladaGraphicsInstance& b)
{
	if (a.m_shapeIndex < b.m_shapeIndex) return +1;
	if (a.m_shapeIndex > b.m_shapeIndex) return -1;
	return 0;
}

void ImportColladaSetup::initPhysics()
{
	i32 upAxis = 1;
	m_guiHelper->setUpAxis(upAxis);
	this->createEmptyDynamicsWorld();
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	m_dynamicsWorld->getDebugDrawer()->setDebugMode(IDebugDraw::DBG_DrawWireframe);

	static i32 fileIndex = 0;

	tukk fileNames[] = {
		"duck.dae",
		"seymourplane_triangulate.dae",
	};
	tukk fileName = fileNames[fileIndex];
	i32 numFiles = sizeof(fileNames) / sizeof(tukk);

	char relativeFileName[1024];

	if (!ResourcePath::findResourcePath(fileName, relativeFileName, 1024,0))
		return;

	Vec3 shift(0, 0, 0);
	Vec3 scaling(1, 1, 1);
	//	i32 index=10;

	{
		AlignedObjectArray<GLInstanceGraphicsShape> visualShapes;
		AlignedObjectArray<ColladaGraphicsInstance> visualShapeInstances;

		float unitMeterScaling(1);
		Transform2 upAxisTrans;
		upAxisTrans.setIdentity();

		Vec4 color(0, 0, 1,1);

#ifdef COMPARE_WITH_ASSIMP
		static i32 useAssimp = 0;
		if (useAssimp)
		{
			LoadMeshFromColladaAssimp(relativeFileName, visualShapes, visualShapeInstances, upAxisTrans, unitMeterScaling);
			fileIndex++;
			if (fileIndex >= numFiles)
			{
				fileIndex = 0;
			}
			color.setVal(1, 0, 0);
		}
		else
		{
			LoadMeshFromCollada(relativeFileName, visualShapes, visualShapeInstances, upAxisTrans, unitMeterScaling);
		}
		useAssimp = 1 - useAssimp;
#else
		fileIndex++;
		if (fileIndex >= numFiles)
		{
			fileIndex = 0;
		}
		DefaultFileIO fileIO;
		LoadMeshFromCollada(relativeFileName, visualShapes, visualShapeInstances, upAxisTrans, unitMeterScaling, upAxis,&fileIO);
#endif  // COMPARE_WITH_ASSIMP

		//at the moment our graphics engine requires instances that share the same visual shape to be added right after registering the shape
		//so perform a sort, just to be sure
		visualShapeInstances.quickSort(ColladaGraphicsInstanceSortfnc);

		for (i32 i = 0; i < visualShapeInstances.size(); i++)
		{
			ColladaGraphicsInstance* instance = &visualShapeInstances[i];
			GLInstanceGraphicsShape* gfxShape = &visualShapes[instance->m_shapeIndex];
			Vec3 position(0, 0, 0);   // = scaling*Vec3(instance->m_pos[0],instance->m_pos[1],instance->m_pos[2]);
			Quat orn(0, 0, 0, 1);  //instance->m_orn[0],instance->m_orn[1],instance->m_orn[2],instance->m_orn[3]);

			//sort the visualShapeInstances, then iterate etc
			//void LoadMeshFromCollada(tukk relativeFileName,
			//AlignedObjectArray<GLInstanceGraphicsShape>& visualShapes,
			//AlignedObjectArray<GLInstanceGraphicsInstance> visualShapeInstances);

			if (gfxShape)
			{
				//Transform2 trans;
				//trans.setIdentity();
				//trans.setRotation(Quat(Vec3(1,0,0),SIMD_HALF_PI));

				b3AlignedObjectArray<GLInstanceVertex> verts;
				verts.resize(gfxShape->m_vertices->size());

				for (i32 i = 0; i < gfxShape->m_vertices->size(); i++)
				{
					verts[i].normal[0] = gfxShape->m_vertices->at(i).normal[0];
					verts[i].normal[1] = gfxShape->m_vertices->at(i).normal[1];
					verts[i].normal[2] = gfxShape->m_vertices->at(i).normal[2];
					verts[i].uv[0] = gfxShape->m_vertices->at(i).uv[0];
					verts[i].uv[1] = gfxShape->m_vertices->at(i).uv[1];
					verts[i].xyzw[0] = gfxShape->m_vertices->at(i).xyzw[0];
					verts[i].xyzw[1] = gfxShape->m_vertices->at(i).xyzw[1];
					verts[i].xyzw[2] = gfxShape->m_vertices->at(i).xyzw[2];
					verts[i].xyzw[3] = gfxShape->m_vertices->at(i).xyzw[3];
				}

				//compensate upAxisTrans and unitMeterScaling here
				Matrix4x4 upAxisMat;
				upAxisMat.setPureRotation(upAxisTrans.getRotation());
				Matrix4x4 unitMeterScalingMat;
				unitMeterScalingMat.setPureScaling(Vec3(unitMeterScaling, unitMeterScaling, unitMeterScaling));
				Matrix4x4 worldMat = unitMeterScalingMat * upAxisMat * instance->m_worldTransform;
				//Matrix4x4 worldMat = instance->m_worldTransform;
				for (i32 v = 0; v < verts.size(); v++)
				{
					Vec3 pos(verts[v].xyzw[0], verts[v].xyzw[1], verts[v].xyzw[2]);
					pos = worldMat * pos;
					verts[v].xyzw[0] = float(pos[0]);
					verts[v].xyzw[1] = float(pos[1]);
					verts[v].xyzw[2] = float(pos[2]);
				}

				i32 shapeId = m_guiHelper->getRenderInterface()->registerShape(&verts[0].xyzw[0], gfxShape->m_numvertices, &gfxShape->m_indices->at(0), gfxShape->m_numIndices);

				//Vec3 instanceScaling(instance->m_scaling[0],instance->m_scaling[1],instance->m_scaling[2]);
				m_guiHelper->getRenderInterface()->registerGraphicsInstance(shapeId, position, orn, color, scaling);
			}
		}
	}
}

class CommonExampleInterface* ImportColladaCreateFunc(struct CommonExampleOptions& options)
{
	return new ImportColladaSetup(options.m_guiHelper);
}
