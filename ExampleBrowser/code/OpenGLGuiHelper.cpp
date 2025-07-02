#include "../OpenGLGuiHelper.h"

//#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/Collision/Shapes/HeightfieldTerrainShape.h>
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
//#include <drx3D/Common/b3Scalar.h>
#include "../CollisionShape2TriangleMesh.h"
#include <drx3D/Physics/SoftBody/SoftBodyHelpers.h>

#include <drx3D/OpenGLWindow/ShapeData.h>
#include <drx3D/OpenGLWindow/SimpleCamera.h>

#define DRX3D_LINE_BATCH_SIZE 512

struct MyDebugVec3
{
	MyDebugVec3(const Vec3& org)
		: x(org.x()),
		  y(org.y()),
		  z(org.z())
	{
	}

	float x;
	float y;
	float z;
};

ATTRIBUTE_ALIGNED16(class)
MyDebugDrawer : public IDebugDraw
{
	CommonGraphicsApp* m_glApp;
	i32 m_debugMode;

	AlignedObjectArray<MyDebugVec3> m_linePoints;
	AlignedObjectArray<u32> m_lineIndices;

	Vec3 m_currentLineColor;
	DefaultColors m_ourColors;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	MyDebugDrawer(CommonGraphicsApp * app)
		: m_glApp(app), m_debugMode(IDebugDraw::DBG_DrawWireframe | IDebugDraw::DBG_DrawAabb), m_currentLineColor(-1, -1, -1)
	{
	}

	virtual ~MyDebugDrawer()
	{
	}
	virtual DefaultColors getDefaultColors() const
	{
		return m_ourColors;
	}
	///the default implementation for setDefaultColors has no effect. A derived class can implement it and store the colors.
	virtual void setDefaultColors(const DefaultColors& colors)
	{
		m_ourColors = colors;
	}

	virtual void drawLine(const Vec3& from1, const Vec3& to1, const Vec3& color1)
	{
		//float from[4] = {from1[0],from1[1],from1[2],from1[3]};
		//float to[4] = {to1[0],to1[1],to1[2],to1[3]};
		//float color[4] = {color1[0],color1[1],color1[2],color1[3]};
		//m_glApp->m_instancingRenderer->drawLine(from,to,color);
		if (m_currentLineColor != color1 || m_linePoints.size() >= DRX3D_LINE_BATCH_SIZE)
		{
			flushLines();
			m_currentLineColor = color1;
		}
		MyDebugVec3 from(from1);
		MyDebugVec3 to(to1);

		m_linePoints.push_back(from);
		m_linePoints.push_back(to);

		m_lineIndices.push_back(m_lineIndices.size());
		m_lineIndices.push_back(m_lineIndices.size());
	}

	virtual void drawContactPoint(const Vec3& PointOnB, const Vec3& normalOnB, Scalar distance, i32 lifeTime, const Vec3& color)
	{
		drawLine(PointOnB, PointOnB + normalOnB * distance, color);
		Vec3 ncolor(0, 0, 0);
		drawLine(PointOnB, PointOnB + normalOnB * 0.01, ncolor);
	}

	virtual void reportErrorWarning(tukk warningString)
	{
	}

	virtual void draw3dText(const Vec3& location, tukk textString)
	{
	}

	virtual void setDebugMode(i32 debugMode)
	{
		m_debugMode = debugMode;
	}

	virtual i32 getDebugMode() const
	{
		return m_debugMode;
	}

	virtual void flushLines()
	{
		i32 sz = m_linePoints.size();
		if (sz)
		{
			float debugColor[4];
			debugColor[0] = m_currentLineColor.x();
			debugColor[1] = m_currentLineColor.y();
			debugColor[2] = m_currentLineColor.z();
			debugColor[3] = 1.f;
			m_glApp->m_renderer->drawLines(&m_linePoints[0].x, debugColor,
										   m_linePoints.size(), sizeof(MyDebugVec3),
										   &m_lineIndices[0],
										   m_lineIndices.size(),
										   1);
			m_linePoints.clear();
			m_lineIndices.clear();
		}
	}
};

static Vec4 sColors[4] =
	{
		Vec4(60. / 256., 186. / 256., 84. / 256., 1),
		Vec4(244. / 256., 194. / 256., 13. / 256., 1),
		Vec4(219. / 256., 50. / 256., 54. / 256., 1),
		Vec4(72. / 256., 133. / 256., 237. / 256., 1),

		//Vec4(1,1,0,1),
};

struct MyHashShape
{
	i32 m_shapeKey;
	i32 m_shapeType;
	Vec3 m_sphere0Pos;
	Vec3 m_sphere1Pos;
	Vec3 m_halfExtents;
	Scalar m_radius0;
	Scalar m_radius1;
	Transform2 m_childTransform;
	i32 m_deformFunc;
	i32 m_upAxis;
	Scalar m_halfHeight;

	MyHashShape()
		: m_shapeKey(0),
		  m_shapeType(0),
		  m_sphere0Pos(Vec3(0, 0, 0)),
		  m_sphere1Pos(Vec3(0, 0, 0)),
		  m_halfExtents(Vec3(0, 0, 0)),
		  m_radius0(0),
		  m_radius1(0),
		  m_deformFunc(0),
		  m_upAxis(-1),
		  m_halfHeight(0)
	{
		m_childTransform.setIdentity();
	}

	bool equals(const MyHashShape& other) const
	{
		bool sameShapeType = m_shapeType == other.m_shapeType;
		bool sameSphere0 = m_sphere0Pos == other.m_sphere0Pos;
		bool sameSphere1 = m_sphere1Pos == other.m_sphere1Pos;
		bool sameHalfExtents = m_halfExtents == other.m_halfExtents;
		bool sameRadius0 = m_radius0 == other.m_radius0;
		bool sameRadius1 = m_radius1 == other.m_radius1;
		bool sameTransform = m_childTransform == other.m_childTransform;
		bool sameUpAxis = m_upAxis == other.m_upAxis;
		bool sameHalfHeight = m_halfHeight == other.m_halfHeight;
		return sameShapeType && sameSphere0 && sameSphere1 && sameHalfExtents && sameRadius0 && sameRadius1 && sameTransform && sameUpAxis && sameHalfHeight;
	}
	//to our success
	SIMD_FORCE_INLINE u32 getHash() const
	{
		u32 key = m_shapeKey;
		// Thomas Wang's hash
		key += ~(key << 15);
		key ^= (key >> 10);
		key += (key << 3);
		key ^= (key >> 6);
		key += ~(key << 11);
		key ^= (key >> 16);

		return key;
	}
};

struct OpenGLGuiHelperInternalData
{
	struct CommonGraphicsApp* m_glApp;
	class MyDebugDrawer* m_debugDraw;
	bool m_vrMode;
	i32 m_vrSkipShadowPass;

	AlignedObjectArray<u8> m_rgbaPixelBuffer1;
	AlignedObjectArray<float> m_depthBuffer1;
	AlignedObjectArray<i32> m_segmentationMaskBuffer;
	HashMap<MyHashShape, i32> m_hashShapes;

	VisualizerFlagCallback m_visualizerFlagCallback;

	i32 m_checkedTexture;
	i32 m_checkedTextureGrey;

	OpenGLGuiHelperInternalData()
		: m_vrMode(false),
		  m_vrSkipShadowPass(0),
		  m_visualizerFlagCallback(0),
		  m_checkedTexture(-1),
		  m_checkedTextureGrey(-1)
	{
	}
};

void OpenGLGuiHelper::setVRMode(bool vrMode)
{
	m_data->m_vrMode = vrMode;
	m_data->m_vrSkipShadowPass = 0;
}

OpenGLGuiHelper::OpenGLGuiHelper(CommonGraphicsApp* glApp, bool useOpenGL2)
{
	m_data = new OpenGLGuiHelperInternalData;
	m_data->m_glApp = glApp;
	m_data->m_debugDraw = 0;
}


	OpenGLGuiHelper::~OpenGLGuiHelper()
{
	delete m_data->m_debugDraw;

	delete m_data;
}

void OpenGLGuiHelper::setBackgroundColor(const double rgbBackground[3])
{
	this->getRenderInterface()->setBackgroundColor(rgbBackground);
}

struct CommonRenderInterface* OpenGLGuiHelper::getRenderInterface()
{
	return m_data->m_glApp->m_renderer;
}

const struct CommonRenderInterface* OpenGLGuiHelper::getRenderInterface() const
{
	return m_data->m_glApp->m_renderer;
}

void OpenGLGuiHelper::createRigidBodyGraphicsObject(RigidBody* body, const Vec3& color)
{
	createCollisionObjectGraphicsObject(body, color);
}


class MyTriangleCollector2 : public TriangleCallback
{
public:
	AlignedObjectArray<GLInstanceVertex>* m_pVerticesOut;
	AlignedObjectArray<i32>* m_pIndicesOut;
	Vec3 m_aabbMin, m_aabbMax;
	Scalar m_textureScaling;

	MyTriangleCollector2(const Vec3& aabbMin, const Vec3& aabbMax)
		:m_aabbMin(aabbMin), m_aabbMax(aabbMax), m_textureScaling(1)
	{
		m_pVerticesOut = 0;
		m_pIndicesOut = 0;
	}

	virtual void processTriangle(Vec3* tris, i32 partId, i32 triangleIndex)
	{
		for (i32 k = 0; k < 3; k++)
		{
			GLInstanceVertex v;
			v.xyzw[3] = 0;
			
			Vec3 normal = (tris[0] - tris[1]).cross(tris[0] - tris[2]);
			normal.safeNormalize();
			for (i32 l = 0; l < 3; l++)
			{
				v.xyzw[l] = tris[k][l];
				v.normal[l] = normal[l];
			}
			
			Vec3 extents = m_aabbMax - m_aabbMin;
			
			v.uv[0] = (1.-((v.xyzw[0] - m_aabbMin[0]) / (m_aabbMax[0] - m_aabbMin[0])))*m_textureScaling;
			v.uv[1] = (1.-(v.xyzw[1] - m_aabbMin[1]) / (m_aabbMax[1] - m_aabbMin[1]))*m_textureScaling;

			m_pIndicesOut->push_back(m_pVerticesOut->size());
			m_pVerticesOut->push_back(v);
		}
	}
};
void OpenGLGuiHelper::createCollisionObjectGraphicsObject(CollisionObject2* body, const Vec3& color)
{
	if (body->getUserIndex() < 0)
	{
		CollisionShape* shape = body->getCollisionShape();
		Transform2 startTransform = body->getWorldTransform();
		i32 graphicsShapeId = shape->getUserIndex();
		if (graphicsShapeId >= 0)
		{
			//	Assert(graphicsShapeId >= 0);
			//the graphics shape is already scaled
			Vec3 localScaling(1, 1, 1);
			i32 graphicsInstanceId = m_data->m_glApp->m_renderer->registerGraphicsInstance(graphicsShapeId, startTransform.getOrigin(), startTransform.getRotation(), color, localScaling);
			body->setUserIndex(graphicsInstanceId);

			SoftBody* sb = SoftBody::upcast(body);
			if (sb)
			{
				i32 graphicsInstanceId = body->getUserIndex();
				changeInstanceFlags(graphicsInstanceId, D3_INSTANCE_DOUBLE_SIDED);
			}
		}
	}
}

i32 OpenGLGuiHelper::registerTexture(u8k* texels, i32 width, i32 height)
{
	i32 textureId = m_data->m_glApp->m_renderer->registerTexture(texels, width, height);
	return textureId;
}

void OpenGLGuiHelper::removeTexture(i32 textureUid)
{
	m_data->m_glApp->m_renderer->removeTexture(textureUid);
}

void OpenGLGuiHelper::changeTexture(i32 textureUniqueId, u8k* rgbTexels, i32 width, i32 height)
{
	bool flipPixelsY = true;
	m_data->m_glApp->m_renderer->updateTexture(textureUniqueId, rgbTexels, flipPixelsY);
}

i32 OpenGLGuiHelper::registerGraphicsShape(const float* vertices, i32 numvertices, i32k* indices, i32 numIndices, i32 primitiveType, i32 textureId)
{
	if (textureId == -2)
	{
		if (m_data->m_checkedTextureGrey < 0)
		{
			m_data->m_checkedTextureGrey = createCheckeredTexture(192, 192, 192);
		}
		textureId = m_data->m_checkedTextureGrey;
	}

	i32 shapeId = m_data->m_glApp->m_renderer->registerShape(vertices, numvertices, indices, numIndices, primitiveType, textureId);
	return shapeId;
}

i32 OpenGLGuiHelper::registerGraphicsInstance(i32 shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling)
{
	return m_data->m_glApp->m_renderer->registerGraphicsInstance(shapeIndex, position, quaternion, color, scaling);
}

void OpenGLGuiHelper::removeAllGraphicsInstances()
{
	m_data->m_hashShapes.clear();
	m_data->m_glApp->m_renderer->removeAllInstances();
}

void OpenGLGuiHelper::removeGraphicsInstance(i32 graphicsUid)
{
	if (graphicsUid >= 0)
	{
		m_data->m_glApp->m_renderer->removeGraphicsInstance(graphicsUid);
	};
}

i32 OpenGLGuiHelper::getShapeIndexFromInstance(i32 instanceUid)
{
	return m_data->m_glApp->m_renderer->getShapeIndexFromInstance(instanceUid);
}

void OpenGLGuiHelper::replaceTexture(i32 shapeIndex, i32 textureUid)
{
	if (shapeIndex >= 0)
	{
		m_data->m_glApp->m_renderer->replaceTexture(shapeIndex, textureUid);
	};
}
void OpenGLGuiHelper::changeInstanceFlags(i32 instanceUid, i32 flags)
{
	if (instanceUid >= 0)
	{
		//careful, flags/instanceUid is swapped
		m_data->m_glApp->m_renderer->writeSingleInstanceFlagsToCPU(  flags, instanceUid);
	}
}
void OpenGLGuiHelper::changeScaling(i32 instanceUid, const double scaling[3])
{
	if (instanceUid >= 0)
	{
		m_data->m_glApp->m_renderer->writeSingleInstanceScaleToCPU(scaling, instanceUid);
	};
}

void OpenGLGuiHelper::changeRGBAColor(i32 instanceUid, const double rgbaColor[4])
{
	if (instanceUid >= 0)
	{
		m_data->m_glApp->m_renderer->writeSingleInstanceColorToCPU(rgbaColor, instanceUid);
	};
}
void OpenGLGuiHelper::changeSpecularColor(i32 instanceUid, const double specularColor[3])
{
	if (instanceUid >= 0)
	{
		m_data->m_glApp->m_renderer->writeSingleInstanceSpecularColorToCPU(specularColor, instanceUid);
	};
}
i32 OpenGLGuiHelper::createCheckeredTexture(i32 red, i32 green, i32 blue)
{
	i32 texWidth = 1024;
	i32 texHeight = 1024;
	AlignedObjectArray<u8> texels;
	texels.resize(texWidth * texHeight * 3);
	for (i32 i = 0; i < texWidth * texHeight * 3; i++)
		texels[i] = 255;

	for (i32 i = 0; i < texWidth; i++)
	{
		for (i32 j = 0; j < texHeight; j++)
		{
			i32 a = i < texWidth / 2 ? 1 : 0;
			i32 b = j < texWidth / 2 ? 1 : 0;

			if (a == b)
			{
				texels[(i + j * texWidth) * 3 + 0] = red;
				texels[(i + j * texWidth) * 3 + 1] = green;
				texels[(i + j * texWidth) * 3 + 2] = blue;
				//					texels[(i+j*texWidth)*4+3] = 255;
			}
			/*else
				{
					texels[i*3+0+j*texWidth] = 255;
					texels[i*3+1+j*texWidth] = 255;
					texels[i*3+2+j*texWidth] = 255;
				}
				*/
		}
	}

	i32 texId = registerTexture(&texels[0], texWidth, texHeight);
	return texId;
}

void OpenGLGuiHelper::createCollisionShapeGraphicsObject(CollisionShape* collisionShape)
{
	//already has a graphics object?
	if (collisionShape->getUserIndex() >= 0)
		return;

	if (m_data->m_checkedTexture < 0)
	{
		m_data->m_checkedTexture = createCheckeredTexture(173, 199, 255);
	}

	if (m_data->m_checkedTextureGrey < 0)
	{
		m_data->m_checkedTextureGrey = createCheckeredTexture(192, 192, 192);
	}

	AlignedObjectArray<GLInstanceVertex> gfxVertices;
	AlignedObjectArray<i32> indices;
	i32 strideInBytes = 9 * sizeof(float);
	if (collisionShape->getShapeType() == BOX_SHAPE_PROXYTYPE)
	{
		BoxShape* boxShape = (BoxShape*)collisionShape;
		
		
		AlignedObjectArray<float> transformedVertices;

		Vec3 halfExtents = boxShape->getHalfExtentsWithMargin();

		MyHashShape shape;
		shape.m_shapeType = boxShape->getShapeType();
		shape.m_halfExtents = halfExtents;
		shape.m_deformFunc = 0;  ////no deform
		i32 graphicsShapeIndex = -1;
		i32* graphicsShapeIndexPtr = m_data->m_hashShapes[shape];

		if (graphicsShapeIndexPtr)
		{
			graphicsShapeIndex = *graphicsShapeIndexPtr;
		}
		else
		{
			i32 numVertices = sizeof(cube_vertices_textured) / strideInBytes;
			transformedVertices.resize(numVertices * 9);
			for (i32 i = 0; i < numVertices; i++)
			{
				Vec3 vert;
				vert.setVal(cube_vertices_textured[i * 9 + 0],
					cube_vertices_textured[i * 9 + 1],
					cube_vertices_textured[i * 9 + 2]);

				Vec3 trVer = halfExtents * vert;
				transformedVertices[i * 9 + 0] = trVer[0];
				transformedVertices[i * 9 + 1] = trVer[1];
				transformedVertices[i * 9 + 2] = trVer[2];
				transformedVertices[i * 9 + 3] = cube_vertices_textured[i * 9 + 3];
				transformedVertices[i * 9 + 4] = cube_vertices_textured[i * 9 + 4];
				transformedVertices[i * 9 + 5] = cube_vertices_textured[i * 9 + 5];
				transformedVertices[i * 9 + 6] = cube_vertices_textured[i * 9 + 6];
				transformedVertices[i * 9 + 7] = cube_vertices_textured[i * 9 + 7];
				transformedVertices[i * 9 + 8] = cube_vertices_textured[i * 9 + 8];
			}

			i32 numIndices = sizeof(cube_indices) / sizeof(i32);
			graphicsShapeIndex = registerGraphicsShape(&transformedVertices[0], numVertices, cube_indices, numIndices, D3_GL_TRIANGLES, m_data->m_checkedTextureGrey);
			m_data->m_hashShapes.insert(shape, graphicsShapeIndex);
		}

		collisionShape->setUserIndex(graphicsShapeIndex);
		return;
	}


	if (collisionShape->getShapeType() == TERRAIN_SHAPE_PROXYTYPE)
	{
		const HeightfieldTerrainShape* heightField = static_cast<const HeightfieldTerrainShape*>(collisionShape);
		
		
		Vec3 aabbMin, aabbMax;
		Transform2 tr;
		tr.setIdentity();
		heightField->getAabb(tr, aabbMin, aabbMax);
		MyTriangleCollector2  col(aabbMin, aabbMax);
		if (heightField->getUserValue3())
		{
			col.m_textureScaling = heightField->getUserValue3();
		}
		col.m_pVerticesOut = &gfxVertices;
		col.m_pIndicesOut = &indices;
		for (i32 k = 0; k < 3; k++)
		{
			aabbMin[k] = -DRX3D_LARGE_FLOAT;
			aabbMax[k] = DRX3D_LARGE_FLOAT;
		}
		heightField->processAllTriangles(&col, aabbMin, aabbMax);
		if (gfxVertices.size() && indices.size())
		{
			i32 userImage = heightField->getUserIndex2();
			if (userImage == -1)
			{
				userImage = m_data->m_checkedTexture;
			}
			i32 shapeId = m_data->m_glApp->m_renderer->registerShape(&gfxVertices[0].xyzw[0], gfxVertices.size(), &indices[0], indices.size(),1, userImage);
			collisionShape->setUserIndex(shapeId);
		}
		return;
	}


	if (collisionShape->getShapeType() == SOFTBODY_SHAPE_PROXYTYPE)
	{
		computeSoftBodyVertices(collisionShape, gfxVertices, indices);
		if (gfxVertices.size() && indices.size())
		{
			i32 shapeId = registerGraphicsShape(&gfxVertices[0].xyzw[0], gfxVertices.size(), &indices[0], indices.size(), D3_GL_TRIANGLES,m_data->m_checkedTexture);

			drx3DAssert(shapeId >= 0);
			collisionShape->setUserIndex(shapeId);
		}
	}
	if (collisionShape->getShapeType() == MULTI_SPHERE_SHAPE_PROXYTYPE)
	{
		MultiSphereShape* ms = (MultiSphereShape*)collisionShape;
		if (ms->getSphereCount() == 2)
		{
			AlignedObjectArray<float> transformedVertices;
			i32 numVertices = sizeof(textured_detailed_sphere_vertices) / strideInBytes;
			transformedVertices.resize(numVertices * 9);
			Vec3 sphere0Pos = ms->getSpherePosition(0);
			Vec3 sphere1Pos = ms->getSpherePosition(1);
			Vec3 fromTo = sphere1Pos - sphere0Pos;
			MyHashShape shape;
			shape.m_sphere0Pos = sphere0Pos;
			shape.m_sphere1Pos = sphere1Pos;
			shape.m_radius0 = 2. * ms->getSphereRadius(0);
			shape.m_radius1 = 2. * ms->getSphereRadius(1);
			shape.m_deformFunc = 1;  //vert.dot(fromTo)
			i32 graphicsShapeIndex = -1;
			i32* graphicsShapeIndexPtr = m_data->m_hashShapes[shape];

			if (graphicsShapeIndexPtr)
			{
				//cache hit
				graphicsShapeIndex = *graphicsShapeIndexPtr;
			}
			else
			{
				//cache miss
				for (i32 i = 0; i < numVertices; i++)
				{
					Vec3 vert;
					vert.setVal(textured_detailed_sphere_vertices[i * 9 + 0],
								  textured_detailed_sphere_vertices[i * 9 + 1],
								  textured_detailed_sphere_vertices[i * 9 + 2]);

					Vec3 trVer(0, 0, 0);

					if (vert.dot(fromTo) > 0)
					{
						Scalar radiusScale = 2. * ms->getSphereRadius(1);
						trVer = radiusScale * vert;
						trVer += sphere1Pos;
					}
					else
					{
						Scalar radiusScale = 2. * ms->getSphereRadius(0);
						trVer = radiusScale * vert;
						trVer += sphere0Pos;
					}

					transformedVertices[i * 9 + 0] = trVer[0];
					transformedVertices[i * 9 + 1] = trVer[1];
					transformedVertices[i * 9 + 2] = trVer[2];
					transformedVertices[i * 9 + 3] = textured_detailed_sphere_vertices[i * 9 + 3];
					transformedVertices[i * 9 + 4] = textured_detailed_sphere_vertices[i * 9 + 4];
					transformedVertices[i * 9 + 5] = textured_detailed_sphere_vertices[i * 9 + 5];
					transformedVertices[i * 9 + 6] = textured_detailed_sphere_vertices[i * 9 + 6];
					transformedVertices[i * 9 + 7] = textured_detailed_sphere_vertices[i * 9 + 7];
					transformedVertices[i * 9 + 8] = textured_detailed_sphere_vertices[i * 9 + 8];
				}

				i32 numIndices = sizeof(textured_detailed_sphere_indices) / sizeof(i32);
				graphicsShapeIndex = registerGraphicsShape(&transformedVertices[0], numVertices, textured_detailed_sphere_indices, numIndices, D3_GL_TRIANGLES, m_data->m_checkedTextureGrey);

				m_data->m_hashShapes.insert(shape, graphicsShapeIndex);
			}
			collisionShape->setUserIndex(graphicsShapeIndex);
			return;
		}
	}

	

	if (collisionShape->getShapeType() == SPHERE_SHAPE_PROXYTYPE)
	{
		SphereShape* sphereShape = (SphereShape*)collisionShape;
		Scalar radius = sphereShape->getRadius();
		Scalar sphereSize = 2. * radius;
		Vec3 radiusScale(sphereSize, sphereSize, sphereSize);
		AlignedObjectArray<float> transformedVertices;

		MyHashShape shape;
		shape.m_radius0 = sphereSize;
		shape.m_deformFunc = 0;  ////no deform
		i32 graphicsShapeIndex = -1;
		i32* graphicsShapeIndexPtr = m_data->m_hashShapes[shape];

		if (graphicsShapeIndexPtr)
		{
			graphicsShapeIndex = *graphicsShapeIndexPtr;
		}
		else
		{
			i32 numVertices = sizeof(textured_detailed_sphere_vertices) / strideInBytes;
			transformedVertices.resize(numVertices * 9);
			for (i32 i = 0; i < numVertices; i++)
			{
				Vec3 vert;
				vert.setVal(textured_detailed_sphere_vertices[i * 9 + 0],
							  textured_detailed_sphere_vertices[i * 9 + 1],
							  textured_detailed_sphere_vertices[i * 9 + 2]);

				Vec3 trVer = radiusScale * vert;
				transformedVertices[i * 9 + 0] = trVer[0];
				transformedVertices[i * 9 + 1] = trVer[1];
				transformedVertices[i * 9 + 2] = trVer[2];
				transformedVertices[i * 9 + 3] = textured_detailed_sphere_vertices[i * 9 + 3];
				transformedVertices[i * 9 + 4] = textured_detailed_sphere_vertices[i * 9 + 4];
				transformedVertices[i * 9 + 5] = textured_detailed_sphere_vertices[i * 9 + 5];
				transformedVertices[i * 9 + 6] = textured_detailed_sphere_vertices[i * 9 + 6];
				transformedVertices[i * 9 + 7] = textured_detailed_sphere_vertices[i * 9 + 7];
				transformedVertices[i * 9 + 8] = textured_detailed_sphere_vertices[i * 9 + 8];
			}

			i32 numIndices = sizeof(textured_detailed_sphere_indices) / sizeof(i32);
			graphicsShapeIndex = registerGraphicsShape(&transformedVertices[0], numVertices, textured_detailed_sphere_indices, numIndices, D3_GL_TRIANGLES, m_data->m_checkedTextureGrey);
			m_data->m_hashShapes.insert(shape, graphicsShapeIndex);
		}

		collisionShape->setUserIndex(graphicsShapeIndex);
		return;
	}
	if (collisionShape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
	{
		CompoundShape* compound = (CompoundShape*)collisionShape;
		if (compound->getNumChildShapes() == 1)
		{
			if (compound->getChildShape(0)->getShapeType() == SPHERE_SHAPE_PROXYTYPE)
			{
				SphereShape* sphereShape = (SphereShape*)compound->getChildShape(0);
				Scalar radius = sphereShape->getRadius();
				Scalar sphereSize = 2. * radius;
				Vec3 radiusScale(sphereSize, sphereSize, sphereSize);

				MyHashShape shape;
				shape.m_radius0 = sphereSize;
				shape.m_deformFunc = 0;  //no deform
				shape.m_childTransform = compound->getChildTransform(0);

				i32 graphicsShapeIndex = -1;
				i32* graphicsShapeIndexPtr = m_data->m_hashShapes[shape];

				if (graphicsShapeIndexPtr)
				{
					graphicsShapeIndex = *graphicsShapeIndexPtr;
				}
				else
				{
					AlignedObjectArray<float> transformedVertices;
					i32 numVertices = sizeof(textured_detailed_sphere_vertices) / strideInBytes;
					transformedVertices.resize(numVertices * 9);
					for (i32 i = 0; i < numVertices; i++)
					{
						Vec3 vert;
						vert.setVal(textured_detailed_sphere_vertices[i * 9 + 0],
									  textured_detailed_sphere_vertices[i * 9 + 1],
									  textured_detailed_sphere_vertices[i * 9 + 2]);

						Vec3 trVer = compound->getChildTransform(0) * (radiusScale * vert);
						transformedVertices[i * 9 + 0] = trVer[0];
						transformedVertices[i * 9 + 1] = trVer[1];
						transformedVertices[i * 9 + 2] = trVer[2];
						transformedVertices[i * 9 + 3] = textured_detailed_sphere_vertices[i * 9 + 3];
						transformedVertices[i * 9 + 4] = textured_detailed_sphere_vertices[i * 9 + 4];
						transformedVertices[i * 9 + 5] = textured_detailed_sphere_vertices[i * 9 + 5];
						transformedVertices[i * 9 + 6] = textured_detailed_sphere_vertices[i * 9 + 6];
						transformedVertices[i * 9 + 7] = textured_detailed_sphere_vertices[i * 9 + 7];
						transformedVertices[i * 9 + 8] = textured_detailed_sphere_vertices[i * 9 + 8];
					}

					i32 numIndices = sizeof(textured_detailed_sphere_indices) / sizeof(i32);
					graphicsShapeIndex = registerGraphicsShape(&transformedVertices[0], numVertices, textured_detailed_sphere_indices, numIndices, D3_GL_TRIANGLES, m_data->m_checkedTextureGrey);
					m_data->m_hashShapes.insert(shape, graphicsShapeIndex);
				}

				collisionShape->setUserIndex(graphicsShapeIndex);
				return;
			}
			if (compound->getChildShape(0)->getShapeType() == CAPSULE_SHAPE_PROXYTYPE)
			{
				CapsuleShape* sphereShape = (CapsuleShape*)compound->getChildShape(0);
				i32 up = sphereShape->getUpAxis();
				Scalar halfHeight = sphereShape->getHalfHeight();

				Scalar radius = sphereShape->getRadius();
				Scalar sphereSize = 2. * radius;

				Vec3 radiusScale = Vec3(sphereSize, sphereSize, sphereSize);

				MyHashShape shape;
				shape.m_radius0 = sphereSize;
				shape.m_deformFunc = 2;  //no deform
				shape.m_childTransform = compound->getChildTransform(0);
				shape.m_upAxis = up;

				i32 graphicsShapeIndex = -1;
				i32* graphicsShapeIndexPtr = m_data->m_hashShapes[shape];

				if (graphicsShapeIndexPtr)
				{
					graphicsShapeIndex = *graphicsShapeIndexPtr;
				}
				else
				{
					AlignedObjectArray<float> transformedVertices;
					i32 numVertices = sizeof(textured_detailed_sphere_vertices) / strideInBytes;
					transformedVertices.resize(numVertices * 9);
					for (i32 i = 0; i < numVertices; i++)
					{
						Vec3 vert;
						vert.setVal(textured_detailed_sphere_vertices[i * 9 + 0],
									  textured_detailed_sphere_vertices[i * 9 + 1],
									  textured_detailed_sphere_vertices[i * 9 + 2]);

						Vec3 trVer = (radiusScale * vert);
						if (trVer[up] > 0)
							trVer[up] += halfHeight;
						else
							trVer[up] -= halfHeight;

						trVer = compound->getChildTransform(0) * trVer;

						transformedVertices[i * 9 + 0] = trVer[0];
						transformedVertices[i * 9 + 1] = trVer[1];
						transformedVertices[i * 9 + 2] = trVer[2];
						transformedVertices[i * 9 + 3] = textured_detailed_sphere_vertices[i * 9 + 3];
						transformedVertices[i * 9 + 4] = textured_detailed_sphere_vertices[i * 9 + 4];
						transformedVertices[i * 9 + 5] = textured_detailed_sphere_vertices[i * 9 + 5];
						transformedVertices[i * 9 + 6] = textured_detailed_sphere_vertices[i * 9 + 6];
						transformedVertices[i * 9 + 7] = textured_detailed_sphere_vertices[i * 9 + 7];
						transformedVertices[i * 9 + 8] = textured_detailed_sphere_vertices[i * 9 + 8];
					}

					i32 numIndices = sizeof(textured_detailed_sphere_indices) / sizeof(i32);
					graphicsShapeIndex = registerGraphicsShape(&transformedVertices[0], numVertices, textured_detailed_sphere_indices, numIndices, D3_GL_TRIANGLES, m_data->m_checkedTextureGrey);
					m_data->m_hashShapes.insert(shape, graphicsShapeIndex);
				}

				collisionShape->setUserIndex(graphicsShapeIndex);
				return;
			}

			if (compound->getChildShape(0)->getShapeType() == MULTI_SPHERE_SHAPE_PROXYTYPE)
			{
				MultiSphereShape* ms = (MultiSphereShape*)compound->getChildShape(0);
				if (ms->getSphereCount() == 2)
				{
					AlignedObjectArray<float> transformedVertices;
					i32 numVertices = sizeof(textured_detailed_sphere_vertices) / strideInBytes;
					transformedVertices.resize(numVertices * 9);
					Vec3 sphere0Pos = ms->getSpherePosition(0);
					Vec3 sphere1Pos = ms->getSpherePosition(1);
					Vec3 fromTo = sphere1Pos - sphere0Pos;
					Scalar radiusScale1 = 2.0 * ms->getSphereRadius(1);
					Scalar radiusScale0 = 2.0 * ms->getSphereRadius(0);

					MyHashShape shape;
					shape.m_radius0 = radiusScale0;
					shape.m_radius1 = radiusScale1;
					shape.m_deformFunc = 4;
					shape.m_sphere0Pos = sphere0Pos;
					shape.m_sphere1Pos = sphere1Pos;
					shape.m_childTransform = compound->getChildTransform(0);

					i32 graphicsShapeIndex = -1;
					i32* graphicsShapeIndexPtr = m_data->m_hashShapes[shape];

					if (graphicsShapeIndexPtr)
					{
						graphicsShapeIndex = *graphicsShapeIndexPtr;
					}
					else
					{
						for (i32 i = 0; i < numVertices; i++)
						{
							Vec3 vert;
							vert.setVal(textured_detailed_sphere_vertices[i * 9 + 0],
										  textured_detailed_sphere_vertices[i * 9 + 1],
										  textured_detailed_sphere_vertices[i * 9 + 2]);

							Vec3 trVer(0, 0, 0);
							if (vert.dot(fromTo) > 0)
							{
								trVer = vert * radiusScale1;
								trVer += sphere1Pos;
								trVer = compound->getChildTransform(0) * trVer;
							}
							else
							{
								trVer = vert * radiusScale0;
								trVer += sphere0Pos;
								trVer = compound->getChildTransform(0) * trVer;
							}

							transformedVertices[i * 9 + 0] = trVer[0];
							transformedVertices[i * 9 + 1] = trVer[1];
							transformedVertices[i * 9 + 2] = trVer[2];
							transformedVertices[i * 9 + 3] = textured_detailed_sphere_vertices[i * 9 + 3];
							transformedVertices[i * 9 + 4] = textured_detailed_sphere_vertices[i * 9 + 4];
							transformedVertices[i * 9 + 5] = textured_detailed_sphere_vertices[i * 9 + 5];
							transformedVertices[i * 9 + 6] = textured_detailed_sphere_vertices[i * 9 + 6];
							transformedVertices[i * 9 + 7] = textured_detailed_sphere_vertices[i * 9 + 7];
							transformedVertices[i * 9 + 8] = textured_detailed_sphere_vertices[i * 9 + 8];
						}

						i32 numIndices = sizeof(textured_detailed_sphere_indices) / sizeof(i32);
						graphicsShapeIndex = registerGraphicsShape(&transformedVertices[0], numVertices, textured_detailed_sphere_indices, numIndices, D3_GL_TRIANGLES, m_data->m_checkedTextureGrey);
						m_data->m_hashShapes.insert(shape, graphicsShapeIndex);
					}
					collisionShape->setUserIndex(graphicsShapeIndex);
					return;
				}
			}
		}
	}
	if (collisionShape->getShapeType() == CAPSULE_SHAPE_PROXYTYPE)
	{
		CapsuleShape* sphereShape = (CapsuleShape*)collisionShape;  //Y up
		i32 up = sphereShape->getUpAxis();
		Scalar halfHeight = sphereShape->getHalfHeight();

		Scalar radius = sphereShape->getRadius();
		Scalar sphereSize = 2. * radius;
		Vec3 radiusScale(sphereSize, sphereSize, sphereSize);

		MyHashShape shape;
		shape.m_radius0 = sphereSize;
		shape.m_deformFunc = 3;
		shape.m_upAxis = up;
		shape.m_halfHeight = halfHeight;
		i32 graphicsShapeIndex = -1;
		i32* graphicsShapeIndexPtr = m_data->m_hashShapes[shape];

		if (graphicsShapeIndexPtr)
		{
			graphicsShapeIndex = *graphicsShapeIndexPtr;
		}
		else
		{
			AlignedObjectArray<float> transformedVertices;
			i32 numVertices = sizeof(textured_detailed_sphere_vertices) / strideInBytes;
			transformedVertices.resize(numVertices * 9);
			for (i32 i = 0; i < numVertices; i++)
			{
				Vec3 vert;
				vert.setVal(textured_detailed_sphere_vertices[i * 9 + 0],
							  textured_detailed_sphere_vertices[i * 9 + 1],
							  textured_detailed_sphere_vertices[i * 9 + 2]);

				Vec3 trVer = radiusScale * vert;
				if (trVer[up] > 0)
					trVer[up] += halfHeight;
				else
					trVer[up] -= halfHeight;

				transformedVertices[i * 9 + 0] = trVer[0];
				transformedVertices[i * 9 + 1] = trVer[1];
				transformedVertices[i * 9 + 2] = trVer[2];
				transformedVertices[i * 9 + 3] = textured_detailed_sphere_vertices[i * 9 + 3];
				transformedVertices[i * 9 + 4] = textured_detailed_sphere_vertices[i * 9 + 4];
				transformedVertices[i * 9 + 5] = textured_detailed_sphere_vertices[i * 9 + 5];
				transformedVertices[i * 9 + 6] = textured_detailed_sphere_vertices[i * 9 + 6];
				transformedVertices[i * 9 + 7] = textured_detailed_sphere_vertices[i * 9 + 7];
				transformedVertices[i * 9 + 8] = textured_detailed_sphere_vertices[i * 9 + 8];
			}

			i32 numIndices = sizeof(textured_detailed_sphere_indices) / sizeof(i32);
			graphicsShapeIndex = registerGraphicsShape(&transformedVertices[0], numVertices, textured_detailed_sphere_indices, numIndices, D3_GL_TRIANGLES, m_data->m_checkedTextureGrey);
			m_data->m_hashShapes.insert(shape, graphicsShapeIndex);
		}
		collisionShape->setUserIndex(graphicsShapeIndex);
		return;
	}
	if (collisionShape->getShapeType() == STATIC_PLANE_PROXYTYPE)
	{
		const StaticPlaneShape* staticPlaneShape = static_cast<const StaticPlaneShape*>(collisionShape);
		Scalar planeConst = staticPlaneShape->getPlaneConstant();
		const Vec3& planeNormal = staticPlaneShape->getPlaneNormal();
		Vec3 planeOrigin = planeNormal * planeConst;
		Vec3 vec0, vec1;
		PlaneSpace1(planeNormal, vec0, vec1);

		Scalar vecLen = 128;
		Vec3 verts[4];

		verts[0] = planeOrigin + vec0 * vecLen + vec1 * vecLen;
		verts[1] = planeOrigin - vec0 * vecLen + vec1 * vecLen;
		verts[2] = planeOrigin - vec0 * vecLen - vec1 * vecLen;
		verts[3] = planeOrigin + vec0 * vecLen - vec1 * vecLen;

		i32 startIndex = 0;
		indices.push_back(startIndex + 0);
		indices.push_back(startIndex + 1);
		indices.push_back(startIndex + 2);
		indices.push_back(startIndex + 0);
		indices.push_back(startIndex + 2);
		indices.push_back(startIndex + 3);
		Transform2 parentTransform;
		parentTransform.setIdentity();
		Vec3 triNormal = parentTransform.getBasis() * planeNormal;

		gfxVertices.resize(4);

		for (i32 i = 0; i < 4; i++)
		{
			Vec3 vtxPos;
			Vec3 pos = parentTransform * verts[i];

			gfxVertices[i].xyzw[0] = pos[0];
			gfxVertices[i].xyzw[1] = pos[1];
			gfxVertices[i].xyzw[2] = pos[2];
			gfxVertices[i].xyzw[3] = 1;
			gfxVertices[i].normal[0] = triNormal[0];
			gfxVertices[i].normal[1] = triNormal[1];
			gfxVertices[i].normal[2] = triNormal[2];
		}

		//verts[0] = planeOrigin + vec0*vecLen + vec1*vecLen;
		//verts[1] = planeOrigin - vec0*vecLen + vec1*vecLen;
		//verts[2] = planeOrigin - vec0*vecLen - vec1*vecLen;
		//verts[3] = planeOrigin + vec0*vecLen - vec1*vecLen;

		gfxVertices[0].uv[0] = vecLen / 2;
		gfxVertices[0].uv[1] = vecLen / 2;
		gfxVertices[1].uv[0] = -vecLen / 2;
		gfxVertices[1].uv[1] = vecLen / 2;
		gfxVertices[2].uv[0] = -vecLen / 2;
		gfxVertices[2].uv[1] = -vecLen / 2;
		gfxVertices[3].uv[0] = vecLen / 2;
		gfxVertices[3].uv[1] = -vecLen / 2;

		i32 shapeId = registerGraphicsShape(&gfxVertices[0].xyzw[0], gfxVertices.size(), &indices[0], indices.size(), D3_GL_TRIANGLES, m_data->m_checkedTexture);
		collisionShape->setUserIndex(shapeId);
		return;
	}

	Transform2 startTrans;
	startTrans.setIdentity();
	//todo: create some textured objects for popular objects, like plane, cube, sphere, capsule

	{
		AlignedObjectArray<Vec3> vertexPositions;
		AlignedObjectArray<Vec3> vertexNormals;
		CollisionShape2TriangleMesh(collisionShape, startTrans, vertexPositions, vertexNormals, indices);
		gfxVertices.resize(vertexPositions.size());
		for (i32 i = 0; i < vertexPositions.size(); i++)
		{
			for (i32 j = 0; j < 4; j++)
			{
				gfxVertices[i].xyzw[j] = vertexPositions[i][j];
			}
			for (i32 j = 0; j < 3; j++)
			{
				gfxVertices[i].normal[j] = vertexNormals[i][j];
			}
			for (i32 j = 0; j < 2; j++)
			{
				gfxVertices[i].uv[j] = 0.5;  //we don't have UV info...
			}
		}
	}

	if (gfxVertices.size() && indices.size())
	{
		i32 shapeId = registerGraphicsShape(&gfxVertices[0].xyzw[0], gfxVertices.size(), &indices[0], indices.size(), D3_GL_TRIANGLES, -1);
		collisionShape->setUserIndex(shapeId);
	}
}
void OpenGLGuiHelper::syncPhysicsToGraphics(const DiscreteDynamicsWorld* rbWorld)
{
	//in VR mode, we skip the synchronization for the second eye
	if (m_data->m_vrMode && m_data->m_vrSkipShadowPass == 1)
		return;

	i32 numCollisionObjects = rbWorld->getNumCollisionObjects();
	{
		D3_PROFILE("write all InstanceTransformToCPU");
		for (i32 i = 0; i < numCollisionObjects; i++)
		{
			//D3_PROFILE("writeSingleInstanceTransformToCPU");
			CollisionObject2* colObj = rbWorld->getCollisionObjectArray()[i];
			CollisionShape* collisionShape = colObj->getCollisionShape();
			if (collisionShape->getShapeType() == SOFTBODY_SHAPE_PROXYTYPE && collisionShape->getUserIndex() >= 0)
			{
				const SoftBody* psb = (const SoftBody*)colObj;
				AlignedObjectArray<GLInstanceVertex> gfxVertices;
				
				if (psb->m_renderNodes.size() > 0)
				{
					
					gfxVertices.resize(psb->m_renderNodes.size());
					for (i32 i = 0; i < psb->m_renderNodes.size(); i++)  // Foreach face
					{
						gfxVertices[i].xyzw[0] = psb->m_renderNodes[i].m_x[0];
						gfxVertices[i].xyzw[1] = psb->m_renderNodes[i].m_x[1];
						gfxVertices[i].xyzw[2] = psb->m_renderNodes[i].m_x[2];
						gfxVertices[i].xyzw[3] = psb->m_renderNodes[i].m_x[3];
						gfxVertices[i].uv[0] = psb->m_renderNodes[i].m_uv1[0];
						gfxVertices[i].uv[1] = psb->m_renderNodes[i].m_uv1[1];
						//gfxVertices[i].normal[0] = psb->m_renderNodes[i].
						gfxVertices[i].normal[0] = psb->m_renderNodes[i].m_normal[0];
						gfxVertices[i].normal[1] = psb->m_renderNodes[i].m_normal[1];
						gfxVertices[i].normal[2] = psb->m_renderNodes[i].m_normal[2];
					}
				}
				else
				{
					AlignedObjectArray<i32> indices;
					computeSoftBodyVertices(collisionShape, gfxVertices, indices);
				}
				m_data->m_glApp->m_renderer->updateShape(collisionShape->getUserIndex(), &gfxVertices[0].xyzw[0], gfxVertices.size());
				continue;
			}
			Vec3 pos = colObj->getWorldTransform().getOrigin();
			Quat orn = colObj->getWorldTransform().getRotation();
			i32 index = colObj->getUserIndex();
			if (index >= 0)
			{
				m_data->m_glApp->m_renderer->writeSingleInstanceTransformToCPU(pos, orn, index);
			}
		}
	}
	{
		D3_PROFILE("writeTransforms");
		m_data->m_glApp->m_renderer->writeTransforms();
	}
}

void OpenGLGuiHelper::render(const DiscreteDynamicsWorld* rbWorld)
{
	if (m_data->m_vrMode)
	{
		//in VR, we skip the shadow generation for the second eye

		if (m_data->m_vrSkipShadowPass >= 1)
		{
			m_data->m_glApp->m_renderer->renderSceneInternal(D3_USE_SHADOWMAP_RENDERMODE);
			m_data->m_vrSkipShadowPass = 0;
		}
		else
		{
			m_data->m_glApp->m_renderer->renderScene();
			m_data->m_vrSkipShadowPass++;
		}
	}
	else
	{
		m_data->m_glApp->m_renderer->renderScene();
	}
}
void OpenGLGuiHelper::createPhysicsDebugDrawer(DiscreteDynamicsWorld* rbWorld)
{
	Assert(rbWorld);
	if (m_data->m_debugDraw)
	{
		delete m_data->m_debugDraw;
		m_data->m_debugDraw = 0;
	}

	m_data->m_debugDraw = new MyDebugDrawer(m_data->m_glApp);
	rbWorld->setDebugDrawer(m_data->m_debugDraw);

	m_data->m_debugDraw->setDebugMode(
		IDebugDraw::DBG_DrawWireframe + IDebugDraw::DBG_DrawAabb
		//IDebugDraw::DBG_DrawContactPoints
	);
}

struct Common2dCanvasInterface* OpenGLGuiHelper::get2dCanvasInterface()
{
	return m_data->m_glApp->m_2dCanvasInterface;
}

CommonParameterInterface* OpenGLGuiHelper::getParameterInterface()
{
	return m_data->m_glApp->m_parameterInterface;
}

void OpenGLGuiHelper::setUpAxis(i32 axis)
{
	m_data->m_glApp->setUpAxis(axis);
}

void OpenGLGuiHelper::setVisualizerFlagCallback(VisualizerFlagCallback callback)
{
	m_data->m_visualizerFlagCallback = callback;
}

void OpenGLGuiHelper::setVisualizerFlag(i32 flag, i32 enable)
{
	if (getRenderInterface() && flag == 16)  //COV_ENABLE_PLANAR_REFLECTION
	{
		getRenderInterface()->setPlaneReflectionShapeIndex(enable);
	}
	if (m_data->m_visualizerFlagCallback)
		(m_data->m_visualizerFlagCallback)(flag, enable != 0);
}

void OpenGLGuiHelper::resetCamera(float camDist, float yaw, float pitch, float camPosX, float camPosY, float camPosZ)
{
	if (getRenderInterface() && getRenderInterface()->getActiveCamera())
	{
		getRenderInterface()->getActiveCamera()->setCameraDistance(camDist);
		getRenderInterface()->getActiveCamera()->setCameraPitch(pitch);
		getRenderInterface()->getActiveCamera()->setCameraYaw(yaw);
		getRenderInterface()->getActiveCamera()->setCameraTargetPosition(camPosX, camPosY, camPosZ);
	}
}

bool OpenGLGuiHelper::getCameraInfo(i32* width, i32* height, float viewMatrix[16], float projectionMatrix[16], float camUp[3], float camForward[3], float hor[3], float vert[3], float* yaw, float* pitch, float* camDist, float cameraTarget[3]) const
{
	if (getRenderInterface() && getRenderInterface()->getActiveCamera())
	{
		*width = m_data->m_glApp->m_window->getWidth();
		*height = m_data->m_glApp->m_window->getHeight();
		getRenderInterface()->getActiveCamera()->getCameraViewMatrix(viewMatrix);
		getRenderInterface()->getActiveCamera()->getCameraProjectionMatrix(projectionMatrix);
		getRenderInterface()->getActiveCamera()->getCameraUpVector(camUp);
		getRenderInterface()->getActiveCamera()->getCameraForwardVector(camForward);

		float top = 1.f;
		float bottom = -1.f;
		float tanFov = (top - bottom) * 0.5f / 1;
		float fov = Scalar(2.0) * Atan(tanFov);
		Vec3 camPos, camTarget;
		getRenderInterface()->getActiveCamera()->getCameraPosition(camPos);
		getRenderInterface()->getActiveCamera()->getCameraTargetPosition(camTarget);
		Vec3 rayFrom = camPos;
		Vec3 rayForward = (camTarget - camPos);
		rayForward.normalize();
		float farPlane = 10000.f;
		rayForward *= farPlane;

		Vec3 rightOffset;
		Vec3 cameraUp = Vec3(camUp[0], camUp[1], camUp[2]);
		Vec3 vertical = cameraUp;
		Vec3 hori;
		hori = rayForward.cross(vertical);
		hori.normalize();
		vertical = hori.cross(rayForward);
		vertical.normalize();
		float tanfov = tanf(0.5f * fov);
		hori *= 2.f * farPlane * tanfov;
		vertical *= 2.f * farPlane * tanfov;
		Scalar aspect = float(*width) / float(*height);
		hori *= aspect;
		//compute 'hor' and 'vert' vectors, useful to generate raytracer rays
		hor[0] = hori[0];
		hor[1] = hori[1];
		hor[2] = hori[2];
		vert[0] = vertical[0];
		vert[1] = vertical[1];
		vert[2] = vertical[2];

		*yaw = getRenderInterface()->getActiveCamera()->getCameraYaw();
		*pitch = getRenderInterface()->getActiveCamera()->getCameraPitch();
		*camDist = getRenderInterface()->getActiveCamera()->getCameraDistance();
		cameraTarget[0] = camTarget[0];
		cameraTarget[1] = camTarget[1];
		cameraTarget[2] = camTarget[2];
		return true;
	}
	return false;
}

void OpenGLGuiHelper::setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16])
{
	m_data->m_glApp->m_renderer->setProjectiveTextureMatrices(viewMatrix, projectionMatrix);
}

void OpenGLGuiHelper::setProjectiveTexture(bool useProjectiveTexture)
{
	m_data->m_glApp->m_renderer->setProjectiveTexture(useProjectiveTexture);
}

void OpenGLGuiHelper::copyCameraImageData(const float viewMatrix[16], const float projectionMatrix[16],
										  u8* pixelsRGBA, i32 rgbaBufferSizeInPixels,
										  float* depthBuffer, i32 depthBufferSizeInPixels,
										  i32* segmentationMaskBuffer, i32 segmentationMaskBufferSizeInPixels,
										  i32 startPixelIndex, i32 destinationWidth,
										  i32 destinationHeight, i32* numPixelsCopied)
{
	i32 sourceWidth = d3Min(destinationWidth, (i32)(m_data->m_glApp->m_window->getWidth() * m_data->m_glApp->m_window->getRetinaScale()));
	i32 sourceHeight = d3Min(destinationHeight, (i32)(m_data->m_glApp->m_window->getHeight() * m_data->m_glApp->m_window->getRetinaScale()));
	m_data->m_glApp->setViewport(sourceWidth, sourceHeight);

	if (numPixelsCopied)
		*numPixelsCopied = 0;

	i32 numTotalPixels = destinationWidth * destinationHeight;
	i32 numRemainingPixels = numTotalPixels - startPixelIndex;
	i32 numBytesPerPixel = 4;  //RGBA
	i32 numRequestedPixels = d3Min(rgbaBufferSizeInPixels, numRemainingPixels);
	if (numRequestedPixels)
	{
		if (startPixelIndex == 0)
		{
			CommonCameraInterface* oldCam = getRenderInterface()->getActiveCamera();
			SimpleCamera tempCam;
			getRenderInterface()->setActiveCamera(&tempCam);
			getRenderInterface()->getActiveCamera()->setVRCamera(viewMatrix, projectionMatrix);
			{
				DRX3D_PROFILE("renderScene");
				getRenderInterface()->renderScene();
			}

			{
				DRX3D_PROFILE("copy pixels");
				AlignedObjectArray<u8> sourceRgbaPixelBuffer;
				AlignedObjectArray<float> sourceDepthBuffer;
				//copy the image into our local cache
				sourceRgbaPixelBuffer.resize(sourceWidth * sourceHeight * numBytesPerPixel);
				sourceDepthBuffer.resize(sourceWidth * sourceHeight);
				{
					DRX3D_PROFILE("getScreenPixels");
					m_data->m_glApp->getScreenPixels(&(sourceRgbaPixelBuffer[0]), sourceRgbaPixelBuffer.size(), &sourceDepthBuffer[0], sizeof(float) * sourceDepthBuffer.size());
				}

				m_data->m_rgbaPixelBuffer1.resize(destinationWidth * destinationHeight * numBytesPerPixel);
				m_data->m_depthBuffer1.resize(destinationWidth * destinationHeight);
				//rescale and flip
				{
					DRX3D_PROFILE("resize and flip");
					for (i32 j = 0; j < destinationHeight; j++)
					{
						for (i32 i = 0; i < destinationWidth; i++)
						{
							i32 xIndex = i32(float(i) * (float(sourceWidth) / float(destinationWidth)));
							i32 yIndex = i32(float(destinationHeight - 1 - j) * (float(sourceHeight) / float(destinationHeight)));
							Clamp(xIndex, 0, sourceWidth);
							Clamp(yIndex, 0, sourceHeight);
							i32 bytesPerPixel = 4;  //RGBA

							i32 sourcePixelIndex = (xIndex + yIndex * sourceWidth) * bytesPerPixel;
							i32 sourceDepthIndex = xIndex + yIndex * sourceWidth;
#define COPY4PIXELS 1
#ifdef COPY4PIXELS
							i32* dst = (i32*)&m_data->m_rgbaPixelBuffer1[(i + j * destinationWidth) * 4 + 0];
							i32* src = (i32*)&sourceRgbaPixelBuffer[sourcePixelIndex + 0];
							*dst = *src;

#else
							m_data->m_rgbaPixelBuffer1[(i + j * destinationWidth) * 4 + 0] = sourceRgbaPixelBuffer[sourcePixelIndex + 0];
							m_data->m_rgbaPixelBuffer1[(i + j * destinationWidth) * 4 + 1] = sourceRgbaPixelBuffer[sourcePixelIndex + 1];
							m_data->m_rgbaPixelBuffer1[(i + j * destinationWidth) * 4 + 2] = sourceRgbaPixelBuffer[sourcePixelIndex + 2];
							m_data->m_rgbaPixelBuffer1[(i + j * destinationWidth) * 4 + 3] = 255;
#endif
							if (depthBuffer)
							{
								m_data->m_depthBuffer1[i + j * destinationWidth] = sourceDepthBuffer[sourceDepthIndex];
							}
						}
					}
				}
			}

			//segmentation mask

			if (segmentationMaskBuffer)
			{
				{
					m_data->m_glApp->m_window->startRendering();
					m_data->m_glApp->setViewport(sourceWidth, sourceHeight);
					DRX3D_PROFILE("renderScene");
					getRenderInterface()->renderSceneInternal(D3_SEGMENTATION_MASK_RENDERMODE);
				}

				{
					DRX3D_PROFILE("copy pixels");
					AlignedObjectArray<u8> sourceRgbaPixelBuffer;
					AlignedObjectArray<float> sourceDepthBuffer;
					//copy the image into our local cache
					sourceRgbaPixelBuffer.resize(sourceWidth * sourceHeight * numBytesPerPixel);
					sourceDepthBuffer.resize(sourceWidth * sourceHeight);
					{
						DRX3D_PROFILE("getScreenPixelsSegmentationMask");
						m_data->m_glApp->getScreenPixels(&(sourceRgbaPixelBuffer[0]), sourceRgbaPixelBuffer.size(), &sourceDepthBuffer[0], sizeof(float) * sourceDepthBuffer.size());
					}
					m_data->m_segmentationMaskBuffer.resize(destinationWidth * destinationHeight, -1);

					//rescale and flip
					{
						DRX3D_PROFILE("resize and flip segmentation mask");
						for (i32 j = 0; j < destinationHeight; j++)
						{
							for (i32 i = 0; i < destinationWidth; i++)
							{
								i32 xIndex = i32(float(i) * (float(sourceWidth) / float(destinationWidth)));
								i32 yIndex = i32(float(destinationHeight - 1 - j) * (float(sourceHeight) / float(destinationHeight)));
								Clamp(xIndex, 0, sourceWidth);
								Clamp(yIndex, 0, sourceHeight);
								i32 bytesPerPixel = 4;  //RGBA
								i32 sourcePixelIndex = (xIndex + yIndex * sourceWidth) * bytesPerPixel;
								i32 sourceDepthIndex = xIndex + yIndex * sourceWidth;

								if (segmentationMaskBuffer)
								{
									float depth = sourceDepthBuffer[sourceDepthIndex];
									if (depth < 1)
									{
										i32 segMask = sourceRgbaPixelBuffer[sourcePixelIndex + 0] + 256 * (sourceRgbaPixelBuffer[sourcePixelIndex + 1]) + 256 * 256 * (sourceRgbaPixelBuffer[sourcePixelIndex + 2]);
										m_data->m_segmentationMaskBuffer[i + j * destinationWidth] = segMask;
									}
									else
									{
										m_data->m_segmentationMaskBuffer[i + j * destinationWidth] = -1;
									}
								}
							}
						}
					}
				}
			}

			getRenderInterface()->setActiveCamera(oldCam);

			if (1)
			{
				getRenderInterface()->getActiveCamera()->disableVRCamera();
				DrawGridData dg;
				dg.upAxis = m_data->m_glApp->getUpAxis();
				getRenderInterface()->updateCamera(dg.upAxis);
				m_data->m_glApp->m_window->startRendering();
			}
		}
		if (pixelsRGBA)
		{
			DRX3D_PROFILE("copy rgba pixels");

			for (i32 i = 0; i < numRequestedPixels * numBytesPerPixel; i++)
			{
				pixelsRGBA[i] = m_data->m_rgbaPixelBuffer1[i + startPixelIndex * numBytesPerPixel];
			}
		}
		if (depthBuffer)
		{
			DRX3D_PROFILE("copy depth buffer pixels");

			for (i32 i = 0; i < numRequestedPixels; i++)
			{
				depthBuffer[i] = m_data->m_depthBuffer1[i + startPixelIndex];
			}
		}
		if (segmentationMaskBuffer)
		{
			DRX3D_PROFILE("copy segmentation mask pixels");
			for (i32 i = 0; i < numRequestedPixels; i++)
			{
				segmentationMaskBuffer[i] = m_data->m_segmentationMaskBuffer[i + startPixelIndex];
			}
		}
		if (numPixelsCopied)
			*numPixelsCopied = numRequestedPixels;
	}

	m_data->m_glApp->setViewport(-1, -1);
}

struct MyConvertPointerSizeT
{
	union {
		ukk m_ptr;
		size_t m_int;
	};
};
bool shapePointerCompareFunc(const CollisionObject2* colA, const CollisionObject2* colB)
{
	MyConvertPointerSizeT a, b;
	a.m_ptr = colA->getCollisionShape();
	b.m_ptr = colB->getCollisionShape();
	return (a.m_int < b.m_int);
}

void OpenGLGuiHelper::autogenerateGraphicsObjects(DiscreteDynamicsWorld* rbWorld)
{
	//sort the collision objects based on collision shape, the gfx library requires instances that re-use a shape to be added after eachother

	AlignedObjectArray<CollisionObject2*> sortedObjects;
	sortedObjects.reserve(rbWorld->getNumCollisionObjects());
	for (i32 i = 0; i < rbWorld->getNumCollisionObjects(); i++)
	{
		CollisionObject2* colObj = rbWorld->getCollisionObjectArray()[i];
		sortedObjects.push_back(colObj);
	}
	sortedObjects.quickSort(shapePointerCompareFunc);
	for (i32 i = 0; i < sortedObjects.size(); i++)
	{
		CollisionObject2* colObj = sortedObjects[i];
		//RigidBody* body = RigidBody::upcast(colObj);
		//does this also work for btMultiBody/MultiBodyLinkCollider?
		SoftBody* sb = SoftBody::upcast(colObj);
		if (sb)
		{
			colObj->getCollisionShape()->setUserPointer(sb);
		}
		createCollisionShapeGraphicsObject(colObj->getCollisionShape());
		i32 colorIndex = colObj->getBroadphaseHandle()->getUid() & 3;

		Vec4 color;
		color = sColors[colorIndex];
		if (colObj->getCollisionShape()->getShapeType() == STATIC_PLANE_PROXYTYPE)
		{
			color.setVal(1, 1, 1, 1);
		}
		createCollisionObjectGraphicsObject(colObj, color);
		
	}
}

void OpenGLGuiHelper::drawText3D(tukk txt, float position[3], float orientation[4], float color[4], float size, i32 optionFlags)
{
	D3_PROFILE("OpenGLGuiHelper::drawText3D");

	Assert(m_data->m_glApp);
	m_data->m_glApp->drawText3D(txt, position, orientation, color, size, optionFlags);
}

void OpenGLGuiHelper::drawText3D(tukk txt, float posX, float posY, float posZ, float size)
{
	D3_PROFILE("OpenGLGuiHelper::drawText3D");

	Assert(m_data->m_glApp);
	m_data->m_glApp->drawText3D(txt, posX, posY, posZ, size);
}

struct CommonGraphicsApp* OpenGLGuiHelper::getAppInterface()
{
	return m_data->m_glApp;
}

void OpenGLGuiHelper::dumpFramesToVideo(tukk mp4FileName)
{
	if (m_data->m_glApp)
	{
		m_data->m_glApp->dumpFramesToVideo(mp4FileName);
	}
}

void OpenGLGuiHelper::computeSoftBodyVertices(CollisionShape* collisionShape,
											  AlignedObjectArray<GLInstanceVertex>& gfxVertices,
											  AlignedObjectArray<i32>& indices)
{
	if (collisionShape->getUserPointer() == 0)
		return;
	drx3DAssert(collisionShape->getUserPointer());
	SoftBody* psb = (SoftBody*)collisionShape->getUserPointer();
	gfxVertices.resize(psb->m_faces.size() * 3);

	for (i32 i = 0; i < psb->m_faces.size(); i++)  // Foreach face
	{
		for (i32 k = 0; k < 3; k++)  // Foreach vertex on a face
		{
			i32 currentIndex = i * 3 + k;
			for (i32 j = 0; j < 3; j++)
			{
				gfxVertices[currentIndex].xyzw[j] = psb->m_faces[i].m_n[k]->m_x[j];
			}
			for (i32 j = 0; j < 3; j++)
			{
				gfxVertices[currentIndex].normal[j] = psb->m_faces[i].m_n[k]->m_n[j];
			}
			for (i32 j = 0; j < 2; j++)
			{
				gfxVertices[currentIndex].uv[j] = psb->m_faces[i].m_n[k]->m_x[j];
			}
			indices.push_back(currentIndex);
		}
	}
}

void OpenGLGuiHelper::updateShape(i32 shapeIndex, float* vertices, i32 numVertices)
{
	m_data->m_glApp->m_renderer->updateShape(shapeIndex, vertices, numVertices);
}