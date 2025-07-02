#include "TinyRendererVisualShapeConverter.h"

#include <drx3D/Importers/URDF/URDFImporterInterface.h>
#include <drx3D/CollisionCommon.h>
#include <drx3D/Importers/Obj/LoadMeshFromObj.h>
#include <drx3D/Importers/STL/LoadMeshFromSTL.h>
#include <drx3D/Importers/Collada/LoadMeshFromCollada.h>
#include <drx3D/Physics/Collision/Shapes/ShapeHull.h>  //to create a tesselation of a generic ConvexShape
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Common/b3FileUtils.h>
#include <string>
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/ExampleBrowser/TinyRenderer/TinyRenderer.h>
#include <drx3D/OpenGLWindow/SimpleCamera.h>
#include <drx3D/Importers/MeshUtility/b3ImportMeshUtility.h>
#include <iostream>
#include <fstream>
#include <drx3D/Importers/URDF/UrdfParser.h>
#include <drx3D/SharedMemory/SharedMemoryPublic.h>  //for b3VisualShapeData
#include <drx3D/ExampleBrowser/TinyRenderer/model.h>
#include <X/stb/stb_image.h>
#include <drx3D/OpenGLWindow/ShapeData.h>
struct MyTexture2
{
	u8* textureData1;
	i32 m_width;
	i32 m_height;
	bool m_isCached;
};

struct TinyRendererObjectArray
{
	AlignedObjectArray<TinyRenderObjectData*> m_renderObjects;
	i32 m_objectUniqueId;
	i32 m_linkIndex;
	Transform2 m_worldTransform;
	Vec3 m_localScaling;

	TinyRendererObjectArray()
	{
		m_worldTransform.setIdentity();
		m_localScaling.setVal(1, 1, 1);
	}
};

#define START_WIDTH 640
#define START_HEIGHT 480

struct TinyRendererVisualShapeConverterInternalData
{
	HashMap<HashInt, TinyRendererObjectArray*> m_swRenderInstances;

	// Maps bodyUniqueId to a list of visual shapes belonging to the body.
	HashMap<HashInt, AlignedObjectArray<b3VisualShapeData> > m_visualShapesMap;

	AlignedObjectArray<u8> m_checkeredTexels;


	i32 m_upAxis;
	i32 m_swWidth;
	i32 m_swHeight;
	TGAImage m_rgbColorBuffer;
	b3AlignedObjectArray<MyTexture2> m_textures;
	b3AlignedObjectArray<float> m_depthBuffer;
	b3AlignedObjectArray<float> m_shadowBuffer;
	b3AlignedObjectArray<i32> m_segmentationMaskBuffer;
	Vec3 m_lightDirection;
	bool m_hasLightDirection;
	Vec3 m_lightColor;
	bool m_hasLightColor;
	float m_lightDistance;
	bool m_hasLightDistance;
	float m_lightAmbientCoeff;
	bool m_hasLightAmbientCoeff;
	float m_lightDiffuseCoeff;
	bool m_hasLightDiffuseCoeff;
	float m_lightSpecularCoeff;
	bool m_hasLightSpecularCoeff;
	bool m_hasShadow;
	i32 m_flags;
	SimpleCamera m_camera;

	TinyRendererVisualShapeConverterInternalData()
		: m_upAxis(2),
		m_swWidth(START_WIDTH),
		m_swHeight(START_HEIGHT),
		m_rgbColorBuffer(START_WIDTH, START_HEIGHT, TGAImage::RGB),
		m_lightDirection(Vec3(-5, 200, -40)),
		m_hasLightDirection(false),
		m_lightColor(Vec3(1.0, 1.0, 1.0)),
		m_hasLightColor(false),
		m_lightDistance(2.0),
		m_hasLightDistance(false),
		m_lightAmbientCoeff(0.6),
		m_hasLightAmbientCoeff(false),
		m_lightDiffuseCoeff(0.35),
		m_hasLightDiffuseCoeff(false),
		m_lightSpecularCoeff(0.05),
		m_hasLightSpecularCoeff(false),
		m_hasShadow(false),
		m_flags(0)
	{
		m_depthBuffer.resize(m_swWidth * m_swHeight);
		m_shadowBuffer.resize(m_swWidth * m_swHeight);
		m_segmentationMaskBuffer.resize(m_swWidth * m_swHeight, -1);
	}

	virtual ~TinyRendererVisualShapeConverterInternalData()
	{
	}
};

TinyRendererVisualShapeConverter::TinyRendererVisualShapeConverter()
{
	m_data = new TinyRendererVisualShapeConverterInternalData();

	float dist = 1.5;
	float pitch = -10;
	float yaw = -80;
	float targetPos[3] = { 0, 0, 0 };
	m_data->m_camera.setCameraUpAxis(m_data->m_upAxis);
	resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
}
TinyRendererVisualShapeConverter::~TinyRendererVisualShapeConverter()
{
	resetAll();
	delete m_data;
}

void TinyRendererVisualShapeConverter::setLightDirection(float x, float y, float z)
{
	m_data->m_lightDirection.setVal(x, y, z);
	m_data->m_hasLightDirection = true;
}

void TinyRendererVisualShapeConverter::setLightColor(float x, float y, float z)
{
	m_data->m_lightColor.setVal(x, y, z);
	m_data->m_hasLightColor = true;
}

void TinyRendererVisualShapeConverter::setLightDistance(float dist)
{
	m_data->m_lightDistance = dist;
	m_data->m_hasLightDistance = true;
}

void TinyRendererVisualShapeConverter::setShadow(bool hasShadow)
{
	m_data->m_hasShadow = hasShadow;
}
void TinyRendererVisualShapeConverter::setFlags(i32 flags)
{
	m_data->m_flags = flags;
}

void TinyRendererVisualShapeConverter::setLightAmbientCoeff(float ambientCoeff)
{
	m_data->m_lightAmbientCoeff = ambientCoeff;
	m_data->m_hasLightAmbientCoeff = true;
}

void TinyRendererVisualShapeConverter::setLightDiffuseCoeff(float diffuseCoeff)
{
	m_data->m_lightDiffuseCoeff = diffuseCoeff;
	m_data->m_hasLightDiffuseCoeff = true;
}

void TinyRendererVisualShapeConverter::setLightSpecularCoeff(float specularCoeff)
{
	m_data->m_lightSpecularCoeff = specularCoeff;
	m_data->m_hasLightSpecularCoeff = true;
}

static void convertURDFToVisualShape(const UrdfShape* visual, tukk urdfPathPrefix, const Transform2& visualTransform, AlignedObjectArray<GLInstanceVertex>& verticesOut, AlignedObjectArray<i32>& indicesOut, AlignedObjectArray<MyTexture2>& texturesOut, b3VisualShapeData& visualShapeOut, struct CommonFileIOInterface* fileIO, i32 flags)
{
	visualShapeOut.m_visualGeometryType = visual->m_geometry.m_type;
	visualShapeOut.m_dimensions[0] = 0;
	visualShapeOut.m_dimensions[1] = 0;
	visualShapeOut.m_dimensions[2] = 0;
	memset(visualShapeOut.m_meshAssetFileName, 0, sizeof(visualShapeOut.m_meshAssetFileName));
#if 0
	if (visual->m_geometry.m_hasLocalMaterial)
	{
		visualShapeOut.m_rgbaColor[0] = visual->m_geometry.m_localMaterial.m_matColor.m_rgbaColor[0];
		visualShapeOut.m_rgbaColor[1] = visual->m_geometry.m_localMaterial.m_matColor.m_rgbaColor[1];
		visualShapeOut.m_rgbaColor[2] = visual->m_geometry.m_localMaterial.m_matColor.m_rgbaColor[2];
		visualShapeOut.m_rgbaColor[3] = visual->m_geometry.m_localMaterial.m_matColor.m_rgbaColor[3];
	}
#endif

	GLInstanceGraphicsShape* glmesh = 0;

	ConvexShape* convexColShape = 0;

	switch (visual->m_geometry.m_type)
	{
	case URDF_GEOM_CYLINDER:
	case URDF_GEOM_CAPSULE:
	{
		Vec3 p1 = visual->m_geometry.m_capsuleFrom;
		Vec3 p2 = visual->m_geometry.m_capsuleTo;
		Transform2 tr;
		tr.setIdentity();
		Scalar rad, len;
		Vec3 center(0, 0, 0);
		Vec3 axis(0, 0, 1);
		AlignedObjectArray<Vec3> vertices;
		i32 numSteps = 32;

		if (visual->m_geometry.m_hasFromTo)
		{
			Vec3 v = p2 - p1;
			Vec3 dir = v.normalized();
			tr = visual->m_linkLocalFrame;
			len = v.length();
			rad = visual->m_geometry.m_capsuleRadius;
			Vec3 ax1, ax2;
			PlaneSpace1(dir, ax1, ax2);

			for (i32 i = 0; i < numSteps; i++)
			{
				{
					Vec3 vert = p1 + ax1 * rad * Sin(SIMD_2_PI * (float(i) / numSteps)) + ax2 * rad * Cos(SIMD_2_PI * (float(i) / numSteps));
					vertices.push_back(vert);
				}
				{
					Vec3 vert = p2 + ax1 * rad * Sin(SIMD_2_PI * (float(i) / numSteps)) + ax2 * rad * Cos(SIMD_2_PI * (float(i) / numSteps));
					vertices.push_back(vert);
				}
			}
			if (visual->m_geometry.m_type == URDF_GEOM_CAPSULE)
			{
				Vec3 pole1 = p1 - dir * rad;
				Vec3 pole2 = p2 + dir * rad;
				vertices.push_back(pole1);
				vertices.push_back(pole2);
			}
		}
		else
		{
			//assume a capsule along the Z-axis, centered at the origin
			tr = visual->m_linkLocalFrame;
			len = visual->m_geometry.m_capsuleHeight;
			rad = visual->m_geometry.m_capsuleRadius;
			for (i32 i = 0; i < numSteps; i++)
			{
				Vec3 vert(rad * Sin(SIMD_2_PI * (float(i) / numSteps)), rad * Cos(SIMD_2_PI * (float(i) / numSteps)), len / 2.);
				vertices.push_back(vert);
				vert[2] = -len / 2.;
				vertices.push_back(vert);
			}
			if (visual->m_geometry.m_type == URDF_GEOM_CAPSULE)
			{
				Vec3 pole1(0, 0, +len / 2. + rad);
				Vec3 pole2(0, 0, -len / 2. - rad);
				vertices.push_back(pole1);
				vertices.push_back(pole2);
			}
		}
		visualShapeOut.m_localVisualFrame[0] = tr.getOrigin()[0];
		visualShapeOut.m_localVisualFrame[1] = tr.getOrigin()[1];
		visualShapeOut.m_localVisualFrame[2] = tr.getOrigin()[2];
		visualShapeOut.m_localVisualFrame[3] = tr.getRotation()[0];
		visualShapeOut.m_localVisualFrame[4] = tr.getRotation()[1];
		visualShapeOut.m_localVisualFrame[5] = tr.getRotation()[2];
		visualShapeOut.m_localVisualFrame[6] = tr.getRotation()[3];
		visualShapeOut.m_dimensions[0] = len;
		visualShapeOut.m_dimensions[1] = rad;

		ConvexHullShape* cylZShape = new ConvexHullShape(&vertices[0].x(), vertices.size(), sizeof(Vec3));
		//CapsuleShape* cylZShape = new CapsuleShape(rad,len);//ConvexHullShape(&vertices[0].x(), vertices.size(), sizeof(Vec3));

		cylZShape->setMargin(0.001);
		convexColShape = cylZShape;
		break;
	}
	case URDF_GEOM_BOX:
	{
		visualShapeOut.m_dimensions[0] = visual->m_geometry.m_boxSize[0];
		visualShapeOut.m_dimensions[1] = visual->m_geometry.m_boxSize[1];
		visualShapeOut.m_dimensions[2] = visual->m_geometry.m_boxSize[2];

		Vec3 extents = visual->m_geometry.m_boxSize;


		i32 strideInBytes = 9 * sizeof(float);
		i32 numVertices = sizeof(cube_vertices_textured) / strideInBytes;
		i32 numIndices = sizeof(cube_indices) / sizeof(i32);

		glmesh = new GLInstanceGraphicsShape;
		//		i32 index = 0;
		glmesh->m_indices = new b3AlignedObjectArray<i32>();
		glmesh->m_vertices = new b3AlignedObjectArray<GLInstanceVertex>();

		glmesh->m_indices->resize(numIndices);
		for (i32 k = 0; k < numIndices; k++)
		{
			glmesh->m_indices->at(k) = cube_indices[k];
		}
		glmesh->m_vertices->resize(numVertices);

		Scalar halfExtentsX = extents[0] * 0.5;
		Scalar halfExtentsY = extents[1] * 0.5;
		Scalar halfExtentsZ = extents[2] * 0.5;
		GLInstanceVertex* verts = &glmesh->m_vertices->at(0);
		Scalar textureScaling = 1;

		for (i32 i = 0; i < numVertices; i++)
		{

			verts[i].xyzw[0] = halfExtentsX * cube_vertices_textured[i * 9];
			verts[i].xyzw[1] = halfExtentsY * cube_vertices_textured[i * 9 + 1];
			verts[i].xyzw[2] = halfExtentsZ * cube_vertices_textured[i * 9 + 2];
			verts[i].xyzw[3] = cube_vertices_textured[i * 9 + 3];
			verts[i].normal[0] = cube_vertices_textured[i * 9 + 4];
			verts[i].normal[1] = cube_vertices_textured[i * 9 + 5];
			verts[i].normal[2] = cube_vertices_textured[i * 9 + 6];
			verts[i].uv[0] = cube_vertices_textured[i * 9 + 7] * textureScaling;
			verts[i].uv[1] = cube_vertices_textured[i * 9 + 8] * textureScaling;
		}

		glmesh->m_numIndices = numIndices;
		glmesh->m_numvertices = numVertices;


		break;
	}
	case URDF_GEOM_SPHERE:
	{
		visualShapeOut.m_dimensions[0] = visual->m_geometry.m_sphereRadius;

		Scalar radius = visual->m_geometry.m_sphereRadius;
		SphereShape* sphereShape = new SphereShape(radius);
		convexColShape = sphereShape;
		convexColShape->setMargin(0.001);
		break;
	}
	case URDF_GEOM_MESH:
	{
		strncpy(visualShapeOut.m_meshAssetFileName, visual->m_geometry.m_meshFileName.c_str(), VISUAL_SHAPE_MAX_PATH_LEN);
		visualShapeOut.m_meshAssetFileName[VISUAL_SHAPE_MAX_PATH_LEN - 1] = 0;

		visualShapeOut.m_dimensions[0] = visual->m_geometry.m_meshScale[0];
		visualShapeOut.m_dimensions[1] = visual->m_geometry.m_meshScale[1];
		visualShapeOut.m_dimensions[2] = visual->m_geometry.m_meshScale[2];

		switch (visual->m_geometry.m_meshFileType)
		{
		case UrdfGeometry::MEMORY_VERTICES:
		{
			glmesh = new GLInstanceGraphicsShape;
			//		i32 index = 0;
			glmesh->m_indices = new b3AlignedObjectArray<i32>();
			glmesh->m_vertices = new b3AlignedObjectArray<GLInstanceVertex>();
			glmesh->m_vertices->resize(visual->m_geometry.m_vertices.size());
			glmesh->m_indices->resize(visual->m_geometry.m_indices.size());

			for (i32 i = 0; i < visual->m_geometry.m_vertices.size(); i++)
			{
				glmesh->m_vertices->at(i).xyzw[0] = visual->m_geometry.m_vertices[i].x();
				glmesh->m_vertices->at(i).xyzw[1] = visual->m_geometry.m_vertices[i].y();
				glmesh->m_vertices->at(i).xyzw[2] = visual->m_geometry.m_vertices[i].z();
				glmesh->m_vertices->at(i).xyzw[3] = 1;
				Vec3 normal(visual->m_geometry.m_vertices[i]);
				if (visual->m_geometry.m_normals.size() == visual->m_geometry.m_vertices.size())
				{
					normal = visual->m_geometry.m_normals[i];
				}
				else
				{
					normal.safeNormalize();
				}

				Vec3 uv(0.5, 0.5, 0);
				if (visual->m_geometry.m_uvs.size() == visual->m_geometry.m_vertices.size())
				{
					uv = visual->m_geometry.m_uvs[i];
				}
				glmesh->m_vertices->at(i).normal[0] = normal[0];
				glmesh->m_vertices->at(i).normal[1] = normal[1];
				glmesh->m_vertices->at(i).normal[2] = normal[2];
				glmesh->m_vertices->at(i).uv[0] = uv[0];
				glmesh->m_vertices->at(i).uv[1] = uv[1];

			}
			for (i32 i = 0; i < visual->m_geometry.m_indices.size(); i++)
			{
				glmesh->m_indices->at(i) = visual->m_geometry.m_indices[i];

			}
			glmesh->m_numIndices = visual->m_geometry.m_indices.size();
			glmesh->m_numvertices = visual->m_geometry.m_vertices.size();

			break;
		}
		case UrdfGeometry::FILE_OBJ:
		{
			//glmesh = LoadMeshFromObj(fullPath,visualPathPrefix);
			b3ImportMeshData meshData;

			if (b3ImportMeshUtility::loadAndRegisterMeshFromFileInternal(visual->m_geometry.m_meshFileName, meshData, fileIO))
			{
				if (flags & URDF_USE_MATERIAL_COLORS_FROM_MTL)
				{
					if (meshData.m_flags & D3_IMPORT_MESH_HAS_RGBA_COLOR)
					{
						visualShapeOut.m_rgbaColor[0] = meshData.m_rgbaColor[0];
						visualShapeOut.m_rgbaColor[1] = meshData.m_rgbaColor[1];
						visualShapeOut.m_rgbaColor[2] = meshData.m_rgbaColor[2];

						if (flags & URDF_USE_MATERIAL_TRANSPARANCY_FROM_MTL)
						{
							visualShapeOut.m_rgbaColor[3] = meshData.m_rgbaColor[3];
						}
						else
						{
							visualShapeOut.m_rgbaColor[3] = 1;
						}
					}
				}
				if (meshData.m_textureImage1)
				{
					MyTexture2 texData;
					texData.m_width = meshData.m_textureWidth;
					texData.m_height = meshData.m_textureHeight;
					texData.textureData1 = meshData.m_textureImage1;
					texData.m_isCached = meshData.m_isCached;
					texturesOut.push_back(texData);
				}
				glmesh = meshData.m_gfxShape;
			}
			break;
		}
		case UrdfGeometry::FILE_STL:

			char relativeFileName[1024];
			if (fileIO->findResourcePath(visual->m_geometry.m_meshFileName.c_str(), relativeFileName, 1024))
			{
				glmesh = LoadMeshFromSTL(relativeFileName, fileIO);
			}
			break;
		case UrdfGeometry::FILE_COLLADA:
		{
			AlignedObjectArray<GLInstanceGraphicsShape> visualShapes;
			AlignedObjectArray<ColladaGraphicsInstance> visualShapeInstances;
			Transform2 upAxisTrans;
			upAxisTrans.setIdentity();
			float unitMeterScaling = 1;
			i32 upAxis = 2;

			LoadMeshFromCollada(visual->m_geometry.m_meshFileName.c_str(),
				visualShapes,
				visualShapeInstances,
				upAxisTrans,
				unitMeterScaling,
				upAxis,
				fileIO);

			glmesh = new GLInstanceGraphicsShape;
			//		i32 index = 0;
			glmesh->m_indices = new b3AlignedObjectArray<i32>();
			glmesh->m_vertices = new b3AlignedObjectArray<GLInstanceVertex>();

			for (i32 i = 0; i < visualShapeInstances.size(); i++)
			{
				ColladaGraphicsInstance* instance = &visualShapeInstances[i];
				GLInstanceGraphicsShape* gfxShape = &visualShapes[instance->m_shapeIndex];

				b3AlignedObjectArray<GLInstanceVertex> verts;
				verts.resize(gfxShape->m_vertices->size());

				i32 baseIndex = glmesh->m_vertices->size();

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

				i32 curNumIndices = glmesh->m_indices->size();
				i32 additionalIndices = gfxShape->m_indices->size();
				glmesh->m_indices->resize(curNumIndices + additionalIndices);
				for (i32 k = 0; k < additionalIndices; k++)
				{
					glmesh->m_indices->at(curNumIndices + k) = gfxShape->m_indices->at(k) + baseIndex;
				}

				//compensate upAxisTrans and unitMeterScaling here
				Matrix4x4 upAxisMat;
				upAxisMat.setIdentity();
				//								upAxisMat.setPureRotation(upAxisTrans.getRotation());
				Matrix4x4 unitMeterScalingMat;
				unitMeterScalingMat.setPureScaling(Vec3(unitMeterScaling, unitMeterScaling, unitMeterScaling));
				Matrix4x4 worldMat = unitMeterScalingMat * upAxisMat * instance->m_worldTransform;
				//Matrix4x4 worldMat = instance->m_worldTransform;
				i32 curNumVertices = glmesh->m_vertices->size();
				i32 additionalVertices = verts.size();
				glmesh->m_vertices->reserve(curNumVertices + additionalVertices);

				for (i32 v = 0; v < verts.size(); v++)
				{
					Vec3 pos(verts[v].xyzw[0], verts[v].xyzw[1], verts[v].xyzw[2]);
					pos = worldMat * pos;
					verts[v].xyzw[0] = float(pos[0]);
					verts[v].xyzw[1] = float(pos[1]);
					verts[v].xyzw[2] = float(pos[2]);
					glmesh->m_vertices->push_back(verts[v]);
				}
			}
			glmesh->m_numIndices = glmesh->m_indices->size();
			glmesh->m_numvertices = glmesh->m_vertices->size();
			//glmesh = LoadMeshFromCollada(visual->m_geometry.m_meshFileName.c_str());
			break;
		}

		default:
		{
			// should never get here (findExistingMeshFile returns false if it doesn't recognize extension)
			Assert(0);
		}
		}

		if (glmesh && glmesh->m_vertices && (glmesh->m_numvertices > 0))
		{
			//apply the geometry scaling
			for (i32 i = 0; i < glmesh->m_vertices->size(); i++)
			{
				glmesh->m_vertices->at(i).xyzw[0] *= visual->m_geometry.m_meshScale[0];
				glmesh->m_vertices->at(i).xyzw[1] *= visual->m_geometry.m_meshScale[1];
				glmesh->m_vertices->at(i).xyzw[2] *= visual->m_geometry.m_meshScale[2];
			}
		}
		else
		{
			if (visual->m_geometry.m_meshFileType != UrdfGeometry::MEMORY_VERTICES)
			{
				drx3DWarning("issue extracting mesh from COLLADA/STL file %s\n", visual->m_geometry.m_meshFileName.c_str());
			}
		}
		break;
	}  // case mesh
	case URDF_GEOM_HEIGHTFIELD:
	{
		glmesh = new GLInstanceGraphicsShape;
		glmesh->m_scaling[0] = 1;
		glmesh->m_scaling[1] = 1;
		glmesh->m_scaling[2] = 1;
		glmesh->m_scaling[3] = 1;
		//		i32 index = 0;
		glmesh->m_indices = new b3AlignedObjectArray<i32>();
		glmesh->m_vertices = new b3AlignedObjectArray<GLInstanceVertex>();
		glmesh->m_indices->reserve(visual->m_geometry.m_indices.size());
		for (i32 i = 0; i < visual->m_geometry.m_indices.size(); i++)
		{
			glmesh->m_indices->push_back(visual->m_geometry.m_indices[i]);
		}
		glmesh->m_vertices->reserve(visual->m_geometry.m_vertices.size());
		for (i32 v = 0; v < visual->m_geometry.m_vertices.size(); v++)
		{
			GLInstanceVertex vtx;
			vtx.xyzw[0] = visual->m_geometry.m_vertices[v].x();
			vtx.xyzw[1] = visual->m_geometry.m_vertices[v].y();
			vtx.xyzw[2] = visual->m_geometry.m_vertices[v].z();
			vtx.xyzw[3] = 1;
			vtx.uv[0] = visual->m_geometry.m_uvs[v].x();
			vtx.uv[1] = visual->m_geometry.m_uvs[v].y();
			vtx.normal[0] = visual->m_geometry.m_normals[v].x();
			vtx.normal[1] = visual->m_geometry.m_normals[v].y();
			vtx.normal[2] = visual->m_geometry.m_normals[v].z();
			glmesh->m_vertices->push_back(vtx);
		}

		glmesh->m_numIndices = glmesh->m_indices->size();
		glmesh->m_numvertices = glmesh->m_vertices->size();
		break;
	}
	case URDF_GEOM_PLANE:
	{
		glmesh = new GLInstanceGraphicsShape;
		//		i32 index = 0;
		glmesh->m_indices = new b3AlignedObjectArray<i32>();
		glmesh->m_vertices = new b3AlignedObjectArray<GLInstanceVertex>();
		glmesh->m_indices->push_back(0);
		glmesh->m_indices->push_back(1);
		glmesh->m_indices->push_back(2);
		glmesh->m_indices->push_back(0);
		glmesh->m_indices->push_back(2);
		glmesh->m_indices->push_back(3);
		glmesh->m_scaling[0] = 1;
		glmesh->m_scaling[1] = 1;
		glmesh->m_scaling[2] = 1;
		glmesh->m_scaling[3] = 1;

		Scalar planeConst = 0;
		Vec3 planeNormal = visual->m_geometry.m_planeNormal;
		Vec3 planeOrigin = planeNormal * planeConst;
		Vec3 vec0, vec1;
		PlaneSpace1(planeNormal, vec0, vec1);

		Scalar vecLen = 128;
		Vec3 verts[4];

		verts[0] = planeOrigin + vec0 * vecLen + vec1 * vecLen;
		verts[1] = planeOrigin - vec0 * vecLen + vec1 * vecLen;
		verts[2] = planeOrigin - vec0 * vecLen - vec1 * vecLen;
		verts[3] = planeOrigin + vec0 * vecLen - vec1 * vecLen;

		GLInstanceVertex vtx;
		vtx.xyzw[0] = verts[0][0]; vtx.xyzw[1] = verts[0][1]; vtx.xyzw[2] = 0; vtx.xyzw[3] = 0;
		vtx.normal[0] = 0; vtx.normal[1] = 0; vtx.normal[2] = 1;
		vtx.uv[0] = vecLen; vtx.uv[1] = vecLen;
		glmesh->m_vertices->push_back(vtx);

		vtx.xyzw[0] = verts[1][0]; vtx.xyzw[1] = verts[1][1]; vtx.xyzw[2] = 0; vtx.xyzw[3] = 0;
		vtx.normal[0] = 0; vtx.normal[1] = 0; vtx.normal[2] = 1;
		vtx.uv[0] = 0; vtx.uv[1] = vecLen;
		glmesh->m_vertices->push_back(vtx);

		vtx.xyzw[0] = verts[2][0]; vtx.xyzw[1] = verts[2][1]; vtx.xyzw[2] = 0; vtx.xyzw[3] = 0;
		vtx.normal[0] = 0; vtx.normal[1] = 0; vtx.normal[2] = 1;
		vtx.uv[0] = 0; vtx.uv[1] = 0;
		glmesh->m_vertices->push_back(vtx);

		vtx.xyzw[0] = verts[3][0]; vtx.xyzw[1] = verts[3][1]; vtx.xyzw[2] = 0; vtx.xyzw[3] = 0;
		vtx.normal[0] = 0; vtx.normal[1] = 0; vtx.normal[2] = 1;
		vtx.uv[0] = vecLen; vtx.uv[1] = 0;
		glmesh->m_vertices->push_back(vtx);

		glmesh->m_numIndices = glmesh->m_indices->size();
		glmesh->m_numvertices = glmesh->m_vertices->size();
		break;
	}
	default:
	{
		drx3DWarning("TinyRenderer: unknown visual geometry type %i\n", visual->m_geometry.m_type);
	}
	}

	//if we have a convex, tesselate into localVertices/localIndices
	if ((glmesh == 0) && convexColShape)
	{
		ShapeHull* hull = new ShapeHull(convexColShape);
		hull->buildHull(0.0);
		{
			//	i32 strideInBytes = 9*sizeof(float);
			i32 numVertices = hull->numVertices();
			i32 numIndices = hull->numIndices();

			glmesh = new GLInstanceGraphicsShape;
			//	i32 index = 0;
			glmesh->m_indices = new b3AlignedObjectArray<i32>();
			glmesh->m_vertices = new b3AlignedObjectArray<GLInstanceVertex>();

			for (i32 i = 0; i < numVertices; i++)
			{
				GLInstanceVertex vtx;
				Vec3 pos = hull->getVertexPointer()[i];
				vtx.xyzw[0] = pos.x();
				vtx.xyzw[1] = pos.y();
				vtx.xyzw[2] = pos.z();
				vtx.xyzw[3] = 1.f;
				Vec3 normal = pos.safeNormalize();
				vtx.normal[0] = normal.x();
				vtx.normal[1] = normal.y();
				vtx.normal[2] = normal.z();
				Scalar u = Atan2(normal[0], normal[2]) / (2 * SIMD_PI) + 0.5;
				Scalar v = normal[1] * 0.5 + 0.5;
				vtx.uv[0] = u;
				vtx.uv[1] = v;
				glmesh->m_vertices->push_back(vtx);
			}

			AlignedObjectArray<i32> indices;
			for (i32 i = 0; i < numIndices; i++)
			{
				glmesh->m_indices->push_back(hull->getIndexPointer()[i]);
			}

			glmesh->m_numvertices = glmesh->m_vertices->size();
			glmesh->m_numIndices = glmesh->m_indices->size();
		}
		delete hull;
		delete convexColShape;
		convexColShape = 0;
	}

	if (glmesh && glmesh->m_numIndices > 0 && glmesh->m_numvertices > 0)
	{
		i32 baseIndex = verticesOut.size();

		for (i32 i = 0; i < glmesh->m_indices->size(); i++)
		{
			indicesOut.push_back(glmesh->m_indices->at(i) + baseIndex);
		}

		for (i32 i = 0; i < glmesh->m_vertices->size(); i++)
		{
			GLInstanceVertex& v = glmesh->m_vertices->at(i);
			Vec3 vert(v.xyzw[0], v.xyzw[1], v.xyzw[2]);
			Vec3 vt = visualTransform * vert;
			v.xyzw[0] = vt[0];
			v.xyzw[1] = vt[1];
			v.xyzw[2] = vt[2];
			Vec3 triNormal(v.normal[0], v.normal[1], v.normal[2]);
			triNormal = visualTransform.getBasis() * triNormal;
			v.normal[0] = triNormal[0];
			v.normal[1] = triNormal[1];
			v.normal[2] = triNormal[2];
			verticesOut.push_back(v);
		}
	}
	delete glmesh;
}

static Vec4 sGoogleyColors[4] =
{
	Vec4(60. / 256., 186. / 256., 84. / 256., 1),
	Vec4(244. / 256., 194. / 256., 13. / 256., 1),
	Vec4(219. / 256., 50. / 256., 54. / 256., 1),
	Vec4(72. / 256., 133. / 256., 237. / 256., 1),

	//Vec4(1,1,0,1),
};

i32  TinyRendererVisualShapeConverter::convertVisualShapes(
	i32 linkIndex, tukk pathPrefix, const Transform2& localInertiaFrame,
	const UrdfLink* linkPtr, const UrdfModel* model, i32 orgGraphicsUniqueId,
	i32 bodyUniqueId, struct CommonFileIOInterface* fileIO)

{
	i32 uniqueId = orgGraphicsUniqueId;
	Assert(orgGraphicsUniqueId >= 0);
	Assert(linkPtr);  // TODO: remove if (not doing it now, because diff will be 50+ lines)
	if (linkPtr)
	{
		bool useVisual;
		i32 cnt = 0;
		if (linkPtr->m_visualArray.size() > 0)
		{
			useVisual = true;
			cnt = linkPtr->m_visualArray.size();
		}
		else
		{
			// We have to see something, take collision shape. Useful for MuJoCo xml, where there are no explicit visual shapes.
			useVisual = false;
			cnt = linkPtr->m_collisionArray.size();
		}

		for (i32 v1 = 0; v1 < cnt; v1++)
		{
			AlignedObjectArray<MyTexture2> textures;
			AlignedObjectArray<GLInstanceVertex> vertices;
			AlignedObjectArray<i32> indices;
			Transform2 startTrans;
			startTrans.setIdentity();
			//i32 graphicsIndex = -1;

			const UrdfShape* vis;
			if (useVisual)
			{
				vis = &linkPtr->m_visualArray[v1];
			}
			else
			{
				vis = &linkPtr->m_collisionArray[v1];
			}
			Transform2 childTrans = vis->m_linkLocalFrame;

			i32 colorIndex = linkIndex;  //colObj? colObj->getBroadphaseHandle()->getUid() & 3 : 0;
			if (colorIndex < 0)
				colorIndex = 0;
			colorIndex &= 3;
			Vec4 color;
			color = (m_data->m_flags & URDF_GOOGLEY_UNDEFINED_COLORS) ? sGoogleyColors[colorIndex] : Vec4(1, 1, 1, 1);
			float rgbaColor[4] = { (float)color[0], (float)color[1], (float)color[2], (float)color[3] };
			//if (colObj->getCollisionShape()->getShapeType()==STATIC_PLANE_PROXYTYPE)
			//{
			//	color.setVal(1,1,1,1);
			//}
			if (model)
			{
				if (useVisual)
				{
					HashString matName(linkPtr->m_visualArray[v1].m_materialName.c_str());
					UrdfMaterial* const* matPtr = model->m_materials[matName];
					if (matPtr)
					{
						for (i32 i = 0; i < 4; i++)
						{
							rgbaColor[i] = (*matPtr)->m_matColor.m_rgbaColor[i];
						}
						//printf("UrdfMaterial %s, rgba = %f,%f,%f,%f\n",mat->m_name.c_str(),mat->m_rgbaColor[0],mat->m_rgbaColor[1],mat->m_rgbaColor[2],mat->m_rgbaColor[3]);
						//m_data->m_linkColors.insert(linkIndex,mat->m_rgbaColor);
					}
					else
					{
						///programmatic created models may have the color in the visual
						if (vis && vis->m_geometry.m_hasLocalMaterial)
						{
							for (i32 i = 0; i < 4; i++)
							{
								rgbaColor[i] = vis->m_geometry.m_localMaterial.m_matColor.m_rgbaColor[i];
							}
						}
					}
				}
			}
			else
			{
				if (vis && vis->m_geometry.m_hasLocalMaterial)
				{
					for (i32 i = 0; i < 4; i++)
					{
						rgbaColor[i] = vis->m_geometry.m_localMaterial.m_matColor.m_rgbaColor[i];
					}
				}
			}

			TinyRendererObjectArray** visualsPtr = m_data->m_swRenderInstances[uniqueId];
			if (visualsPtr == 0)
			{
				m_data->m_swRenderInstances.insert(uniqueId, new TinyRendererObjectArray);
			}
			visualsPtr = m_data->m_swRenderInstances[uniqueId];

			Assert(visualsPtr);
			TinyRendererObjectArray* visuals = *visualsPtr;
			visuals->m_objectUniqueId = bodyUniqueId;
			visuals->m_linkIndex = linkIndex;

			b3VisualShapeData visualShape;
			visualShape.m_objectUniqueId = bodyUniqueId;
			visualShape.m_linkIndex = linkIndex;
			visualShape.m_localVisualFrame[0] = vis->m_linkLocalFrame.getOrigin()[0];
			visualShape.m_localVisualFrame[1] = vis->m_linkLocalFrame.getOrigin()[1];
			visualShape.m_localVisualFrame[2] = vis->m_linkLocalFrame.getOrigin()[2];
			visualShape.m_localVisualFrame[3] = vis->m_linkLocalFrame.getRotation()[0];
			visualShape.m_localVisualFrame[4] = vis->m_linkLocalFrame.getRotation()[1];
			visualShape.m_localVisualFrame[5] = vis->m_linkLocalFrame.getRotation()[2];
			visualShape.m_localVisualFrame[6] = vis->m_linkLocalFrame.getRotation()[3];
			visualShape.m_rgbaColor[0] = rgbaColor[0];
			visualShape.m_rgbaColor[1] = rgbaColor[1];
			visualShape.m_rgbaColor[2] = rgbaColor[2];
			visualShape.m_rgbaColor[3] = rgbaColor[3];
			visualShape.m_openglTextureId = -1;
			visualShape.m_tinyRendererTextureId = -1;
			visualShape.m_textureUniqueId = -1;

			{
				D3_PROFILE("convertURDFToVisualShape");
				convertURDFToVisualShape(vis, pathPrefix, localInertiaFrame.inverse() * childTrans, vertices, indices, textures, visualShape, fileIO, m_data->m_flags);
				if ((vis->m_geometry.m_type == URDF_GEOM_PLANE) || (vis->m_geometry.m_type == URDF_GEOM_HEIGHTFIELD))
				{
					i32 texWidth = 1024;
					i32 texHeight = 1024;
					if (m_data->m_checkeredTexels.size() == 0)
					{

						i32 red = 173;
						i32 green = 199;
						i32 blue = 255;
						//create a textured surface

						m_data->m_checkeredTexels.resize(texWidth * texHeight * 3);
						for (i32 i = 0; i < texWidth * texHeight * 3; i++)
							m_data->m_checkeredTexels[i] = 255;

						for (i32 i = 0; i < texWidth; i++)
						{
							for (i32 j = 0; j < texHeight; j++)
							{
								i32 a = i < texWidth / 2 ? 1 : 0;
								i32 b = j < texWidth / 2 ? 1 : 0;

								if (a == b)
								{
									m_data->m_checkeredTexels[(i + j * texWidth) * 3 + 0] = red;
									m_data->m_checkeredTexels[(i + j * texWidth) * 3 + 1] = green;
									m_data->m_checkeredTexels[(i + j * texWidth) * 3 + 2] = blue;
								}
							}
						}
					}
					MyTexture2 texData;
					texData.m_width = texWidth;
					texData.m_height = texHeight;
					texData.textureData1 = &m_data->m_checkeredTexels[0];
					texData.m_isCached = true;
					textures.push_back(texData);
				}
			}

			rgbaColor[0] = visualShape.m_rgbaColor[0];
			rgbaColor[1] = visualShape.m_rgbaColor[1];
			rgbaColor[2] = visualShape.m_rgbaColor[2];
			rgbaColor[3] = visualShape.m_rgbaColor[3];

			if (vertices.size() && indices.size())
			{
				TinyRenderObjectData* tinyObj = new TinyRenderObjectData(m_data->m_rgbColorBuffer, m_data->m_depthBuffer, &m_data->m_shadowBuffer, &m_data->m_segmentationMaskBuffer, bodyUniqueId, linkIndex);
				u8* textureImage1 = 0;
				i32 textureWidth = 0;
				i32 textureHeight = 0;
				bool isCached = false;
				if (textures.size())
				{
					textureImage1 = textures[0].textureData1;
					textureWidth = textures[0].m_width;
					textureHeight = textures[0].m_height;
					isCached = textures[0].m_isCached;
				}

				{
					D3_PROFILE("registerMeshShape");

					tinyObj->registerMeshShape(&vertices[0].xyzw[0], vertices.size(), &indices[0], indices.size(), rgbaColor,
						textureImage1, textureWidth, textureHeight);
				}
				visuals->m_renderObjects.push_back(tinyObj);
			}

			Assert(textures.size() <= 1);
			for (i32 i = 0; i < textures.size(); i++)
			{
				visualShape.m_tinyRendererTextureId = m_data->m_textures.size();
				m_data->m_textures.push_back(textures[i]);
			}
			AlignedObjectArray<b3VisualShapeData>* shapes = m_data->m_visualShapesMap[visualShape.m_objectUniqueId];
			if (!shapes)
			{
				m_data->m_visualShapesMap.insert(visualShape.m_objectUniqueId, AlignedObjectArray<b3VisualShapeData>());
				shapes = m_data->m_visualShapesMap[visualShape.m_objectUniqueId];
			}
			shapes->push_back(visualShape);
		}
	}

	return uniqueId;
}

i32 TinyRendererVisualShapeConverter::registerShapeAndInstance( const b3VisualShapeData& visualShape, const float* vertices, i32 numvertices, i32k* indices, i32 numIndices, i32 primitiveType, i32 textureId, i32 orgGraphicsUniqueId, i32 bodyUniqueId, i32 linkIndex)
{
	AlignedObjectArray<b3VisualShapeData>* shapes1 =
		m_data->m_visualShapesMap[bodyUniqueId];
	if (!shapes1)
	{
		m_data->m_visualShapesMap.insert(bodyUniqueId,
			AlignedObjectArray<b3VisualShapeData>());
		shapes1 = m_data->m_visualShapesMap[bodyUniqueId];
	}
	if (numvertices && numIndices)
	{
		TinyRenderObjectData* tinyObj = new TinyRenderObjectData(m_data->m_rgbColorBuffer, m_data->m_depthBuffer, &m_data->m_shadowBuffer, &m_data->m_segmentationMaskBuffer, bodyUniqueId, linkIndex);
		//those are primary soft bodies, possibly cloth, make them double-sided by default
		tinyObj->m_doubleSided = true;

		float rgbaColor[4] = { (float)visualShape.m_rgbaColor[0],(float)visualShape.m_rgbaColor[1],(float)visualShape.m_rgbaColor[2],(float)visualShape.m_rgbaColor[3] };
		{
			D3_PROFILE("registerMeshShape");


			tinyObj->registerMeshShape(vertices,
				numvertices,
				indices,
				numIndices,
				rgbaColor,
				m_data->m_textures[textureId].textureData1, m_data->m_textures[textureId].m_width, m_data->m_textures[textureId].m_height);
		}

		TinyRendererObjectArray** visualsPtr = m_data->m_swRenderInstances[orgGraphicsUniqueId];
		if (visualsPtr == 0)
		{
			m_data->m_swRenderInstances.insert(orgGraphicsUniqueId, new TinyRendererObjectArray);
		}
		visualsPtr = m_data->m_swRenderInstances[orgGraphicsUniqueId];

		if (visualsPtr && *visualsPtr)
		{
			TinyRendererObjectArray* visuals = *visualsPtr;
			if (visuals)
			{
				visuals->m_linkIndex = linkIndex;
				visuals->m_objectUniqueId = bodyUniqueId;
				visuals->m_renderObjects.push_back(tinyObj);
			}
			
			shapes1->push_back(visualShape);
		}
	}

	return orgGraphicsUniqueId;
}

void TinyRendererVisualShapeConverter::updateShape(i32 shapeUniqueId, const Vec3* vertices, i32 numVertices, const Vec3* normals, i32 numNormals)
{
	TinyRendererObjectArray** visualsPtr = m_data->m_swRenderInstances[shapeUniqueId];
	if (visualsPtr != 0)
	{

		TinyRendererObjectArray* visuals = *visualsPtr;
		if (visuals->m_renderObjects.size() == 1)
		{
			TinyRenderObjectData* renderObj = visuals->m_renderObjects[0];

			if (renderObj->m_model->nverts() == numVertices)
			{
				TinyRender::Vec3f* verts = renderObj->m_model->readWriteVertices();
				//just do a sync
				for (i32 i = 0; i < numVertices; i++)
				{
					const Vec3& vtx = vertices[i];
					verts[i].x = vtx.x();
					verts[i].y = vtx.y();
					verts[i].z = vtx.z();
				}
				if (renderObj->m_model->nnormals() == numNormals)
				{
					TinyRender::Vec3f* norms = renderObj->m_model->readWriteNormals();
					for (i32 i = 0; i < numNormals; i++)
					{
						const Vec3& normal = normals[i];
						norms[i].x = normal.x();
						norms[i].y = normal.y();
						norms[i].z = normal.z();
					}
				}
			}
		}
	}
}

i32 TinyRendererVisualShapeConverter::getNumVisualShapes(i32 bodyUniqueId)
{
	AlignedObjectArray<b3VisualShapeData>* shapes = m_data->m_visualShapesMap[bodyUniqueId];
	if (shapes)
	{
		return shapes->size();
	}
	return 0;
}

i32 TinyRendererVisualShapeConverter::getVisualShapesData(i32 bodyUniqueId, i32 shapeIndex, struct b3VisualShapeData* shapeData)
{
	AlignedObjectArray<b3VisualShapeData>* shapes = m_data->m_visualShapesMap[bodyUniqueId];
	if (!shapes)
	{
		return 0;
	}
	if (shapes->size() <= shapeIndex)
	{
		return 0;
	}
	*shapeData = shapes->at(shapeIndex);
	return 1;
}

void TinyRendererVisualShapeConverter::changeInstanceFlags(i32 bodyUniqueId, i32 linkIndex, i32 shapeIndex, i32 flags)
{
	bool doubleSided = (flags & 4) != 0;
	AlignedObjectArray<b3VisualShapeData>* shapes = m_data->m_visualShapesMap[bodyUniqueId];
	if (!shapes)
	{
		return;
	}
	i32 start = -1;

	for (i32 i = 0; i < m_data->m_swRenderInstances.size(); i++)
	{
		TinyRendererObjectArray** ptrptr = m_data->m_swRenderInstances.getAtIndex(i);
		if (ptrptr && *ptrptr)
		{
			TinyRendererObjectArray* visuals = *ptrptr;
			if ((bodyUniqueId == visuals->m_objectUniqueId) && (linkIndex == visuals->m_linkIndex))
			{
				for (i32 q = 0; q < visuals->m_renderObjects.size(); q++)
				{
					if (shapeIndex < 0 || q == shapeIndex)
					{
						visuals->m_renderObjects[q]->m_doubleSided = doubleSided;
					}
				}
			}
		}
	}

}

void TinyRendererVisualShapeConverter::changeRGBAColor(i32 bodyUniqueId, i32 linkIndex, i32 shapeIndex, const double rgbaColor[4])
{
	AlignedObjectArray<b3VisualShapeData>* shapes = m_data->m_visualShapesMap[bodyUniqueId];
	if (!shapes)
	{
		return;
	}
	i32 start = -1;
	for (i32 i = 0; i < shapes->size(); i++)
	{
		if (shapes->at(i).m_linkIndex == linkIndex)
		{
			shapes->at(i).m_rgbaColor[0] = rgbaColor[0];
			shapes->at(i).m_rgbaColor[1] = rgbaColor[1];
			shapes->at(i).m_rgbaColor[2] = rgbaColor[2];
			shapes->at(i).m_rgbaColor[3] = rgbaColor[3];
		}
	}

	for (i32 i = 0; i < m_data->m_swRenderInstances.size(); i++)
	{
		TinyRendererObjectArray** ptrptr = m_data->m_swRenderInstances.getAtIndex(i);
		if (ptrptr && *ptrptr)
		{
			float rgba[4] = { (float)rgbaColor[0], (float)rgbaColor[1], (float)rgbaColor[2], (float)rgbaColor[3] };
			TinyRendererObjectArray* visuals = *ptrptr;
			if ((bodyUniqueId == visuals->m_objectUniqueId) && (linkIndex == visuals->m_linkIndex))
			{
				for (i32 q = 0; q < visuals->m_renderObjects.size(); q++)
				{
					if (shapeIndex < 0 || q == shapeIndex)
					{
						visuals->m_renderObjects[q]->m_model->setColorRGBA(rgba);
					}
				}
			}
		}
	}
}

void TinyRendererVisualShapeConverter::setUpAxis(i32 axis)
{
	m_data->m_upAxis = axis;
	m_data->m_camera.setCameraUpAxis(axis);
	m_data->m_camera.update();
}
void TinyRendererVisualShapeConverter::resetCamera(float camDist, float yaw, float pitch, float camPosX, float camPosY, float camPosZ)
{
	m_data->m_camera.setCameraDistance(camDist);
	m_data->m_camera.setCameraPitch(pitch);
	m_data->m_camera.setCameraYaw(yaw);
	m_data->m_camera.setCameraTargetPosition(camPosX, camPosY, camPosZ);
	m_data->m_camera.setAspectRatio((float)m_data->m_swWidth / (float)m_data->m_swHeight);
	m_data->m_camera.update();
}

void TinyRendererVisualShapeConverter::clearBuffers(TGAColor& clearColor)
{
	float farPlane = m_data->m_camera.getCameraFrustumFar();
	for (i32 y = 0; y < m_data->m_swHeight; ++y)
	{
		for (i32 x = 0; x < m_data->m_swWidth; ++x)
		{
			m_data->m_rgbColorBuffer.set(x, y, clearColor);
			m_data->m_depthBuffer[x + y * m_data->m_swWidth] = -farPlane;
			m_data->m_shadowBuffer[x + y * m_data->m_swWidth] = -1e30f;
			m_data->m_segmentationMaskBuffer[x + y * m_data->m_swWidth] = -1;
		}
	}
}

void TinyRendererVisualShapeConverter::render()
{
	ATTRIBUTE_ALIGNED16(float viewMat[16]);
	ATTRIBUTE_ALIGNED16(float projMat[16]);

	m_data->m_camera.getCameraProjectionMatrix(projMat);
	m_data->m_camera.getCameraViewMatrix(viewMat);

	render(viewMat, projMat);
}

void TinyRendererVisualShapeConverter::syncTransform(i32 shapeUniqueId, const Transform2& worldTransform, const Vec3& localScaling)
{
	TinyRendererObjectArray** renderObjPtr = m_data->m_swRenderInstances[shapeUniqueId];
	if (renderObjPtr)
	{
		TinyRendererObjectArray* renderObj = *renderObjPtr;
		renderObj->m_worldTransform = worldTransform;
		renderObj->m_localScaling = localScaling;
	}
}

void TinyRendererVisualShapeConverter::render(const float viewMat[16], const float projMat[16])
{
	//clear the color buffer
	TGAColor clearColor;
	clearColor.bgra[0] = 255;
	clearColor.bgra[1] = 255;
	clearColor.bgra[2] = 255;
	clearColor.bgra[3] = 255;

	float near = projMat[14] / (projMat[10] - 1);
	float far = projMat[14] / (projMat[10] + 1);

	m_data->m_camera.setCameraFrustumNear(near);
	m_data->m_camera.setCameraFrustumFar(far);

	clearBuffers(clearColor);

	ATTRIBUTE_ALIGNED16(Scalar modelMat[16]);

	Vec3 lightDirWorld(-5, 200, -40);
	if (m_data->m_hasLightDirection)
	{
		lightDirWorld = m_data->m_lightDirection;
	}
	else
	{
		switch (m_data->m_upAxis)
		{
		case 1:
			lightDirWorld = Vec3(-50.f, 100, 30);
			break;
		case 2:
			lightDirWorld = Vec3(-50.f, 30, 100);
			break;
		default:
		{
		}
		};
	}

	lightDirWorld.normalize();

	Vec3 lightColor(1.0, 1.0, 1.0);
	if (m_data->m_hasLightColor)
	{
		lightColor = m_data->m_lightColor;
	}

	float lightDistance = 2.0;
	if (m_data->m_hasLightDistance)
	{
		lightDistance = m_data->m_lightDistance;
	}

	float lightAmbientCoeff = 0.6;
	if (m_data->m_hasLightAmbientCoeff)
	{
		lightAmbientCoeff = m_data->m_lightAmbientCoeff;
	}

	float lightDiffuseCoeff = 0.35;
	if (m_data->m_hasLightDiffuseCoeff)
	{
		lightDiffuseCoeff = m_data->m_lightDiffuseCoeff;
	}

	float lightSpecularCoeff = 0.05;
	if (m_data->m_hasLightSpecularCoeff)
	{
		lightSpecularCoeff = m_data->m_lightSpecularCoeff;
	}

	if (m_data->m_hasShadow)
	{
		for (i32 n = 0; n < m_data->m_swRenderInstances.size(); n++)
		{
			TinyRendererObjectArray** visualArrayPtr = m_data->m_swRenderInstances.getAtIndex(n);
			if (0 == visualArrayPtr)
				continue;  //can this ever happen?
			TinyRendererObjectArray* visualArray = *visualArrayPtr;

			for (i32 v = 0; v < visualArray->m_renderObjects.size(); v++)
			{
				TinyRenderObjectData* renderObj = visualArray->m_renderObjects[v];

				//sync the object transform
				const Transform2& tr = visualArray->m_worldTransform;
				tr.getOpenGLMatrix(modelMat);

				for (i32 i = 0; i < 4; i++)
				{
					for (i32 j = 0; j < 4; j++)
					{
						renderObj->m_projectionMatrix[i][j] = projMat[i + 4 * j];
						renderObj->m_modelMatrix[i][j] = modelMat[i + 4 * j];
						renderObj->m_viewMatrix[i][j] = viewMat[i + 4 * j];
					}
				}
				renderObj->m_localScaling = visualArray->m_localScaling;
				renderObj->m_lightDirWorld = lightDirWorld;
				renderObj->m_lightColor = lightColor;
				renderObj->m_lightDistance = lightDistance;
				renderObj->m_lightAmbientCoeff = lightAmbientCoeff;
				renderObj->m_lightDiffuseCoeff = lightDiffuseCoeff;
				renderObj->m_lightSpecularCoeff = lightSpecularCoeff;
				TinyRenderer::renderObjectDepth(*renderObj);
			}
		}
	}

	for (i32 n = 0; n < m_data->m_swRenderInstances.size(); n++)
	{
		TinyRendererObjectArray** visualArrayPtr = m_data->m_swRenderInstances.getAtIndex(n);
		if (0 == visualArrayPtr)
			continue;  //can this ever happen?
		TinyRendererObjectArray* visualArray = *visualArrayPtr;

		for (i32 v = 0; v < visualArray->m_renderObjects.size(); v++)
		{
			TinyRenderObjectData* renderObj = visualArray->m_renderObjects[v];

			//sync the object transform
			const Transform2& tr = visualArray->m_worldTransform;
			tr.getOpenGLMatrix(modelMat);

			for (i32 i = 0; i < 4; i++)
			{
				for (i32 j = 0; j < 4; j++)
				{
					renderObj->m_projectionMatrix[i][j] = projMat[i + 4 * j];
					renderObj->m_modelMatrix[i][j] = modelMat[i + 4 * j];
					renderObj->m_viewMatrix[i][j] = viewMat[i + 4 * j];
				}
			}
			renderObj->m_localScaling = visualArray->m_localScaling;
			renderObj->m_lightDirWorld = lightDirWorld;
			renderObj->m_lightColor = lightColor;
			renderObj->m_lightDistance = lightDistance;
			renderObj->m_lightAmbientCoeff = lightAmbientCoeff;
			renderObj->m_lightDiffuseCoeff = lightDiffuseCoeff;
			renderObj->m_lightSpecularCoeff = lightSpecularCoeff;
			TinyRenderer::renderObject(*renderObj);
		}
	}
	//printf("write tga \n");
	//m_data->m_rgbColorBuffer.write_tga_file("camera.tga");
	//	printf("flipped!\n");
	m_data->m_rgbColorBuffer.flip_vertically();

	//flip z-buffer and segmentation Buffer
	{
		i32 half = m_data->m_swHeight >> 1;
		for (i32 j = 0; j < half; j++)
		{
			u64 l1 = j * m_data->m_swWidth;
			u64 l2 = (m_data->m_swHeight - 1 - j) * m_data->m_swWidth;
			for (i32 i = 0; i < m_data->m_swWidth; i++)
			{
				Swap(m_data->m_depthBuffer[l1 + i], m_data->m_depthBuffer[l2 + i]);
				Swap(m_data->m_shadowBuffer[l1 + i], m_data->m_shadowBuffer[l2 + i]);
				Swap(m_data->m_segmentationMaskBuffer[l1 + i], m_data->m_segmentationMaskBuffer[l2 + i]);
			}
		}
	}
}

void TinyRendererVisualShapeConverter::getWidthAndHeight(i32& width, i32& height)
{
	width = m_data->m_swWidth;
	height = m_data->m_swHeight;
}

void TinyRendererVisualShapeConverter::setWidthAndHeight(i32 width, i32 height)
{
	m_data->m_swWidth = width;
	m_data->m_swHeight = height;

	m_data->m_depthBuffer.resize(m_data->m_swWidth * m_data->m_swHeight);
	m_data->m_shadowBuffer.resize(m_data->m_swWidth * m_data->m_swHeight);
	m_data->m_segmentationMaskBuffer.resize(m_data->m_swWidth * m_data->m_swHeight);
	m_data->m_rgbColorBuffer = TGAImage(width, height, TGAImage::RGB);
}

void TinyRendererVisualShapeConverter::copyCameraImageData(u8* pixelsRGBA, i32 rgbaBufferSizeInPixels,
	float* depthBuffer, i32 depthBufferSizeInPixels,
	i32* segmentationMaskBuffer, i32 segmentationMaskSizeInPixels,
	i32 startPixelIndex, i32* widthPtr, i32* heightPtr, i32* numPixelsCopied)
{
	i32 w = m_data->m_rgbColorBuffer.get_width();
	i32 h = m_data->m_rgbColorBuffer.get_height();

	if (numPixelsCopied)
		*numPixelsCopied = 0;

	if (widthPtr)
		*widthPtr = w;

	if (heightPtr)
		*heightPtr = h;

	i32 numTotalPixels = w * h;
	i32 numRemainingPixels = numTotalPixels - startPixelIndex;
	i32 numBytesPerPixel = 4;  //RGBA
	i32 numRequestedPixels = d3Min(rgbaBufferSizeInPixels, numRemainingPixels);
	if (numRequestedPixels)
	{
		for (i32 i = 0; i < numRequestedPixels; i++)
		{
			if (depthBuffer)
			{
				float farPlane = m_data->m_camera.getCameraFrustumFar();
				float nearPlane = m_data->m_camera.getCameraFrustumNear();

				// TinyRenderer returns clip coordinates, transform to eye coordinates first
				float z_c = -m_data->m_depthBuffer[i + startPixelIndex];
				// float distance = (farPlane - nearPlane) / (farPlane + nearPlane) * (z_c + 2. * farPlane * nearPlane / (farPlane - nearPlane));

				// The depth buffer value is between 0 and 1
				// float a = farPlane / (farPlane - nearPlane);
				// float b = farPlane * nearPlane / (nearPlane - farPlane);
				// depthBuffer[i] = a + b / distance;

				// Simply the above expressions
				depthBuffer[i] = farPlane * (nearPlane + z_c) / (2. * farPlane * nearPlane + farPlane * z_c - nearPlane * z_c);
			}
			if (segmentationMaskBuffer)
			{
				i32 segMask = m_data->m_segmentationMaskBuffer[i + startPixelIndex];
				if ((m_data->m_flags & ER_SEGMENTATION_MASK_OBJECT_AND_LINKINDEX) == 0)
				{
					//if we don't explicitly request link index, clear it out
					//object index are the lower 24bits
					if (segMask >= 0)
					{
						segMask &= ((1 << 24) - 1);
					}
				}
				segmentationMaskBuffer[i] = segMask;
			}

			if (pixelsRGBA)
			{
				pixelsRGBA[i * numBytesPerPixel] = m_data->m_rgbColorBuffer.buffer()[(i + startPixelIndex) * 3 + 0];
				pixelsRGBA[i * numBytesPerPixel + 1] = m_data->m_rgbColorBuffer.buffer()[(i + startPixelIndex) * 3 + 1];
				pixelsRGBA[i * numBytesPerPixel + 2] = m_data->m_rgbColorBuffer.buffer()[(i + startPixelIndex) * 3 + 2];
				pixelsRGBA[i * numBytesPerPixel + 3] = 255;
			}
		}

		if (numPixelsCopied)
			*numPixelsCopied = numRequestedPixels;
	}
}

void TinyRendererVisualShapeConverter::removeVisualShape(i32 shapeUniqueId)
{
	TinyRendererObjectArray** ptrptr = m_data->m_swRenderInstances[shapeUniqueId];
	if (ptrptr && *ptrptr)
	{
		TinyRendererObjectArray* ptr = *ptrptr;
		if (ptr)
		{
			m_data->m_visualShapesMap.remove(ptr->m_objectUniqueId);
			for (i32 o = 0; o < ptr->m_renderObjects.size(); o++)
			{
				delete ptr->m_renderObjects[o];
			}
		}
		delete ptr;
		m_data->m_swRenderInstances.remove(shapeUniqueId);
	}
}

void TinyRendererVisualShapeConverter::resetAll()
{


	for (i32 i = 0; i < m_data->m_swRenderInstances.size(); i++)
	{
		TinyRendererObjectArray** ptrptr = m_data->m_swRenderInstances.getAtIndex(i);
		if (ptrptr && *ptrptr)
		{
			TinyRendererObjectArray* ptr = *ptrptr;
			if (ptr)
			{
				for (i32 o = 0; o < ptr->m_renderObjects.size(); o++)
				{
					delete ptr->m_renderObjects[o];
				}
			}
			delete ptr;
		}
	}

	for (i32 i = 0; i < m_data->m_textures.size(); i++)
	{
		if (!m_data->m_textures[i].m_isCached)
		{
			free(m_data->m_textures[i].textureData1);
		}
	}
	m_data->m_textures.clear();
	m_data->m_swRenderInstances.clear();
	m_data->m_visualShapesMap.clear();
}

void TinyRendererVisualShapeConverter::changeShapeTexture(i32 bodyUniqueId, i32 jointIndex, i32 shapeIndex, i32 textureUniqueId)
{
	Assert(textureUniqueId < m_data->m_textures.size());
	if (textureUniqueId >= -1 && textureUniqueId < m_data->m_textures.size())
	{
		for (i32 n = 0; n < m_data->m_swRenderInstances.size(); n++)
		{
			TinyRendererObjectArray** visualArrayPtr = m_data->m_swRenderInstances.getAtIndex(n);
			if (0 == visualArrayPtr)
				continue;  //can this ever happen?
			TinyRendererObjectArray* visualArray = *visualArrayPtr;

			if (visualArray->m_objectUniqueId == bodyUniqueId && visualArray->m_linkIndex == jointIndex)
			{
				for (i32 v = 0; v < visualArray->m_renderObjects.size(); v++)
				{
					TinyRenderObjectData* renderObj = visualArray->m_renderObjects[v];

					if ((shapeIndex < 0) || (shapeIndex == v))
					{
						if (textureUniqueId >= 0)
						{
							renderObj->m_model->setDiffuseTextureFromData(m_data->m_textures[textureUniqueId].textureData1, m_data->m_textures[textureUniqueId].m_width, m_data->m_textures[textureUniqueId].m_height);
						}
						else
						{
							renderObj->m_model->setDiffuseTextureFromData(0, 0, 0);
						}
					}
				}
			}
		}
	}
}

i32 TinyRendererVisualShapeConverter::registerTexture(u8* texels, i32 width, i32 height)
{
	MyTexture2 texData;
	texData.m_width = width;
	texData.m_height = height;
	texData.textureData1 = texels;
	texData.m_isCached = true;
	m_data->m_textures.push_back(texData);
	return m_data->m_textures.size() - 1;
}

i32 TinyRendererVisualShapeConverter::loadTextureFile(tukk filename, struct CommonFileIOInterface* fileIO)
{
	D3_PROFILE("loadTextureFile");
	i32 width, height, n;
	u8* image = 0;
	if (fileIO)
	{
		b3AlignedObjectArray<char> buffer;
		buffer.reserve(1024);
		i32 fileId = fileIO->fileOpen(filename, "rb");
		if (fileId >= 0)
		{
			i32 size = fileIO->getFileSize(fileId);
			if (size > 0)
			{
				buffer.resize(size);
				i32 actual = fileIO->fileRead(fileId, &buffer[0], size);
				if (actual != size)
				{
					drx3DWarning("image filesize mismatch!\n");
					buffer.resize(0);
				}
			}
			fileIO->fileClose(fileId);
		}
		if (buffer.size())
		{
			image = stbi_load_from_memory((u8k*)&buffer[0], buffer.size(), &width, &height, &n, 3);
		}
	}
	else
	{
		image = stbi_load(filename, &width, &height, &n, 3);
	}

	if (image && (width >= 0) && (height >= 0))
	{
		MyTexture2 texData;
		texData.m_width = width;
		texData.m_height = height;
		texData.textureData1 = image;
		texData.m_isCached = false;
		m_data->m_textures.push_back(texData);
		return m_data->m_textures.size() - 1;
	}
	return -1;
}
