#include "eglRendererVisualShapeConverter.h"
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
#include <drx3D/Maths/Linear/MinMax.h>

#ifdef __APPLE__
#include <drx3D/OpenGLWindow/MacOpenGLWindow.h>
typedef MacOpenGLWindow DefaultOpenGLWindow;
#else
#ifdef _WIN32
#include <drx3D/OpenGLWindow/Win32OpenGLWindow.h>
typedef Win32OpenGLWindow DefaultOpenGLWindow;
#else
#ifdef DRX3D_USE_EGL
#include <drx3D/OpenGLWindow/EGLOpenGLWindow.h>
typedef EGLOpenGLWindow DefaultOpenGLWindow;
#else
#include <drx3D/OpenGLWindow/X11OpenGLWindow.h>
typedef X11OpenGLWindow DefaultOpenGLWindow;
#endif  //DRX3D_USE_EGL
#endif  // _WIN32
#endif  //__APPLE__

#include <drx3D/OpenGLWindow/GLInstancingRenderer.h>
#include <drx3D/OpenGLWindow/GLRenderToTexture.h>

static void printGLString(tukk name, GLenum s)
{
	tukk v = (tukk)glGetString(s);
	printf("%s = %s\n", name, v);
}

using namespace std;

struct MyTexture3
{
	u8* textureData1;
	i32 m_width;
	i32 m_height;
	bool m_isCached;
	i32 m_innerTexUid;
};

struct EGLRendererObjectArray
{
	
	AlignedObjectArray<i32> m_graphicsInstanceIds;
	i32 m_objectUniqueId;
	i32 m_linkIndex;
	Transform2 m_worldTransform;
	Vec3 m_localScaling;
	b3AlignedObjectArray<GLInstanceVertex> m_vertices;
	
	EGLRendererObjectArray()
	{
		m_worldTransform.setIdentity();
		m_localScaling.setVal(1, 1, 1);
	}

};

//#define START_WIDTH 2560
//#define START_HEIGHT 2048

#define START_WIDTH 1024
#define START_HEIGHT 768

struct HashVisual
{
	UrdfShape m_vis;
	Transform2 m_tr;

	i32 getHash() const
	{
		if (m_vis.m_geometry.m_meshFileName.length())
		{
			HashString s = m_vis.m_geometry.m_meshFileName.c_str();
			return s.getHash();
		}
		return 0;
	}
	bool equals(const HashVisual& other) const
	{
		if ((m_vis.m_geometry.m_type == URDF_GEOM_MESH) &&
			(other.m_vis.m_geometry.m_type == URDF_GEOM_MESH))
		{
			bool sameTr = m_tr == other.m_tr;
			bool sameVis = m_vis.m_geometry.m_meshFileName == other.m_vis.m_geometry.m_meshFileName;
			bool sameLocalFrame = m_vis.m_linkLocalFrame == other.m_vis.m_linkLocalFrame;
			return sameTr&&sameVis&&sameLocalFrame;
		}
		return false;
	}
};

struct EGLRendererVisualShapeConverterInternalData
{
	class CommonWindowInterface* m_window;
	class GLInstancingRenderer* m_instancingRenderer;
	AlignedObjectArray<u8> m_rgbaPixelBuffer1;
	AlignedObjectArray<float> m_depthBuffer1;

	AlignedObjectArray<u8> m_segmentationMaskSourceRgbaPixelBuffer;
	AlignedObjectArray<float> m_segmentationMaskSourceDepthBuffer;

	AlignedObjectArray<i32> m_graphicsIndexToSegmentationMask;
	HashMap<HashInt, EGLRendererObjectArray*> m_swRenderInstances;
	HashMap<HashPtr, i32> m_cachedTextureIds;
	HashMap<HashVisual, i32> m_cachedVisualShapes;

	AlignedObjectArray<b3VisualShapeData> m_visualShapes;

	
	i32 m_upAxis;
	i32 m_swWidth;
	i32 m_swHeight;
	AlignedObjectArray<u8> m_sourceRgbaPixelBuffer;
	AlignedObjectArray<float> m_sourceDepthBuffer;

	TGAImage m_rgbColorBuffer;
	b3AlignedObjectArray<MyTexture3> m_textures;
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

	bool m_leftMouseButton;
	bool m_middleMouseButton;
	bool m_rightMouseButton;
	float m_wheelMultiplier;
	float m_mouseMoveMultiplier;
	float m_mouseXpos;
	float m_mouseYpos;
	bool m_mouseInitialized;
	
	AlignedObjectArray<u8> m_checkeredTexels;


	EGLRendererVisualShapeConverterInternalData()
		: m_upAxis(2),
		  m_swWidth(START_WIDTH),
		  m_swHeight(START_HEIGHT),
		  m_rgbColorBuffer(START_WIDTH, START_HEIGHT, TGAImage::RGB),
		  m_lightDirection(Vec3(-5, -40, 200 )),
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
		  m_flags(0),
		m_leftMouseButton(false),
		m_middleMouseButton(false),
		m_rightMouseButton(false),
		m_wheelMultiplier(0.01f),
		m_mouseMoveMultiplier(0.4f),
		m_mouseXpos(0.f),
		m_mouseYpos(0.f),
		m_mouseInitialized(false)
		
	{
		m_depthBuffer.resize(m_swWidth * m_swHeight);
		m_shadowBuffer.resize(m_swWidth * m_swHeight);
		m_segmentationMaskBuffer.resize(m_swWidth * m_swHeight, -1);

		// OpenGL window
		bool allowRetina = true;
		m_window = new DefaultOpenGLWindow();
		m_window->setAllowRetina(allowRetina);
		b3gWindowConstructionInfo ci;
		ci.m_title = "PyBullet";
		ci.m_width = m_swWidth;
		ci.m_height = m_swHeight;
		ci.m_renderDevice = -1;

		m_window->createWindow(ci);
		m_window->setWindowTitle(ci.m_title);
		drx3DAssert(glGetError() == GL_NO_ERROR);
		{
			printGLString("Version", GL_VERSION);
			printGLString("Vendor", GL_VENDOR);
			printGLString("Renderer", GL_RENDERER);
		}
		glClearColor(.7f, .7f, .8f, 1.f);

		m_window->startRendering();

		drx3DAssert(glGetError() == GL_NO_ERROR);

		glGetError();  //don't remove this call, it is needed for Ubuntu
		drx3DAssert(glGetError() == GL_NO_ERROR);
		i32 maxNumObjectCapacity = 128 * 1024;
		i32 maxShapeCapacityInBytes = 128 * 1024 * 1024;
		m_instancingRenderer = new GLInstancingRenderer(maxNumObjectCapacity, maxShapeCapacityInBytes);
		drx3DAssert(glGetError() == GL_NO_ERROR);
		m_instancingRenderer->init();
		drx3DAssert(glGetError() == GL_NO_ERROR);
		m_instancingRenderer->resize(m_swWidth, m_swHeight);
		m_instancingRenderer->InitShaders();
		drx3DAssert(glGetError() == GL_NO_ERROR);
		m_instancingRenderer->setActiveCamera(&m_camera);
		drx3DAssert(glGetError() == GL_NO_ERROR);
		m_instancingRenderer->updateCamera();
		drx3DAssert(glGetError() == GL_NO_ERROR);
		m_instancingRenderer->setLightPosition(m_lightDirection);
		m_window->endRendering();
	}

	virtual ~EGLRendererVisualShapeConverterInternalData()
	{
		delete m_instancingRenderer;
		m_window->closeWindow();
		delete m_window;
	}
};



static EGLRendererVisualShapeConverter* gWindow = 0;

static void SimpleResizeCallback(float widthf, float heightf)
{
	i32 width = (i32)widthf;
	i32 height = (i32)heightf;
	if (gWindow && gWindow->m_data->m_instancingRenderer)
	{
		gWindow->m_data->m_instancingRenderer->resize(width, height);
		gWindow->setWidthAndHeight(width, height);
	}
	

	//if (gApp && gApp->m_instancingRenderer)
	//	gApp->m_instancingRenderer->resize(width, height);
	//
	//if (gApp && gApp->m_primRenderer)
	//	gApp->m_primRenderer->setScreenSize(width, height);
}

#if 0
static void SimpleKeyboardCallback(i32 key, i32 state)
{
	if (key == B3G_ESCAPE) //&& gApp && gApp->m_window)
	{
		//gApp->m_window->setRequestExit();
	}
	else
	{
		//gApp->defaultKeyboardCallback(key,state);
	}
}
#endif
static void SimpleMouseButtonCallback(i32 button, i32 state, float x, float y)
{
	if (gWindow)
	{
		gWindow->mouseButtonCallback(button, state, x, y);
	}
}

static void SimpleMouseMoveCallback(float x, float y)
{
	if (gWindow)
	{
		gWindow->mouseMoveCallback(x, y);
	}
}

static void SimpleWheelCallback(float deltax, float deltay)
{
	float wheelMultiplier = 0.01f;
	if (gWindow && gWindow->m_data->m_instancingRenderer)
	{
		class GLInstancingRenderer* renderer = gWindow->m_data->m_instancingRenderer;
		b3Vec3 cameraTargetPosition, cameraPosition, cameraUp = b3MakeVector3(0, 0, 0);
		i32 upAxis = renderer->getActiveCamera()->getCameraUpAxis();
		cameraUp[upAxis] = 1;
		CommonCameraInterface* camera = renderer->getActiveCamera();

		camera->getCameraPosition(cameraPosition);
		camera->getCameraTargetPosition(cameraTargetPosition);

		bool m_leftMouseButton = false;

		if (!m_leftMouseButton)
		{
			float cameraDistance = camera->getCameraDistance();
			if (deltay < 0 || cameraDistance > 1)
			{
				cameraDistance -= deltay * 0.01f;
				if (cameraDistance < 1)
					cameraDistance = 1;
				camera->setCameraDistance(cameraDistance);
			}
			else
			{
				b3Vec3 fwd = cameraTargetPosition - cameraPosition;
				fwd.normalize();
				cameraTargetPosition += fwd * deltay * wheelMultiplier;  //todo: expose it in the GUI?
			}
		}
		else
		{
			if (b3Fabs(deltax) > b3Fabs(deltay))
			{
				b3Vec3 fwd = cameraTargetPosition - cameraPosition;
				b3Vec3 side = cameraUp.cross(fwd);
				side.normalize();
				cameraTargetPosition += side * deltax * wheelMultiplier;
			}
			else
			{
				cameraTargetPosition -= cameraUp * deltay * wheelMultiplier;
			}
		}

		camera->setCameraTargetPosition(cameraTargetPosition[0], cameraTargetPosition[1], cameraTargetPosition[2]);
	}
}

void defaultMouseButtonCallback(i32 button, i32 state, float x, float y)
{
	if (gWindow)
	{
		gWindow->mouseButtonCallback(button, state, x, y);
	}
}
void defaultMouseMoveCallback(float x, float y)
{
	if (gWindow)
	{
		gWindow->mouseMoveCallback(x, y);
	}  //m_window &&  m_renderer

	
}

void EGLRendererVisualShapeConverter::mouseButtonCallback(i32 button, i32 state, float x, float y)
{
	if (button == 0)
		m_data->m_leftMouseButton = (state == 1);
	if (button == 1)
		m_data->m_middleMouseButton = (state == 1);

	if (button == 2)
		m_data->m_rightMouseButton = (state == 1);

	m_data->m_mouseXpos = x;
	m_data->m_mouseYpos = y;
	m_data->m_mouseInitialized = true;
}
void EGLRendererVisualShapeConverter::mouseMoveCallback(float x, float y)
{
	class GLInstancingRenderer* renderer = m_data->m_instancingRenderer;
	if (renderer == 0)
		return;
	
	CommonCameraInterface* camera = renderer->getActiveCamera();

	bool isAltPressed = m_data->m_window->isModifierKeyPressed(B3G_ALT);
	bool isControlPressed = m_data->m_window->isModifierKeyPressed(B3G_CONTROL);

	if (isAltPressed || isControlPressed)
	{
		float xDelta = x - m_data->m_mouseXpos;
		float yDelta = y - m_data->m_mouseYpos;
		float cameraDistance = camera->getCameraDistance();
		float pitch = camera->getCameraPitch();
		float yaw = camera->getCameraYaw();

		float targPos[3];
		float camPos[3];

		camera->getCameraTargetPosition(targPos);
		camera->getCameraPosition(camPos);

		b3Vec3 cameraPosition = b3MakeVector3(b3Scalar(camPos[0]),
			b3Scalar(camPos[1]),
			b3Scalar(camPos[2]));

		b3Vec3 cameraTargetPosition = b3MakeVector3(b3Scalar(targPos[0]),
			b3Scalar(targPos[1]),
			b3Scalar(targPos[2]));
		b3Vec3 cameraUp = b3MakeVector3(0, 0, 0);
		cameraUp[camera->getCameraUpAxis()] = 1.f;

		if (m_data->m_leftMouseButton)
		{
			//			if (b3Fabs(xDelta)>b3Fabs(yDelta))
			//			{
			pitch -= yDelta * m_data->m_mouseMoveMultiplier;
			//			} else
			//			{
			yaw -= xDelta * m_data->m_mouseMoveMultiplier;
			//			}
		}

		if (m_data->m_middleMouseButton)
		{
			cameraTargetPosition += cameraUp * yDelta * 0.01;

			b3Vec3 fwd = cameraTargetPosition - cameraPosition;
			b3Vec3 side = cameraUp.cross(fwd);
			side.normalize();
			cameraTargetPosition += side * xDelta * 0.01;
		}
		if (m_data->m_rightMouseButton)
		{
			cameraDistance -= xDelta * 0.01f;
			cameraDistance -= yDelta * 0.01f;
			if (cameraDistance < 1)
				cameraDistance = 1;
			if (cameraDistance > 1000)
				cameraDistance = 1000;
		}
		camera->setCameraDistance(cameraDistance);
		camera->setCameraPitch(pitch);
		camera->setCameraYaw(yaw);
		camera->setCameraTargetPosition(cameraTargetPosition[0], cameraTargetPosition[1], cameraTargetPosition[2]);
	}
	m_data->m_mouseXpos = x;
	m_data->m_mouseYpos = y;
	m_data->m_mouseInitialized = true;
}
EGLRendererVisualShapeConverter::EGLRendererVisualShapeConverter()
{
	
	m_data = new EGLRendererVisualShapeConverterInternalData();
	
	
	float dist = 1.5;
	float pitch = -10;
	float yaw = -80;
	float targetPos[3] = {0, 0, 0};
	m_data->m_camera.setCameraUpAxis(m_data->m_upAxis);
	resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	gWindow = this;
	m_data->m_window->setResizeCallback(SimpleResizeCallback);
	m_data->m_window->setWheelCallback(SimpleWheelCallback);
	m_data->m_window->setMouseButtonCallback(SimpleMouseButtonCallback);
	m_data->m_window->setMouseMoveCallback(SimpleMouseMoveCallback);
}
EGLRendererVisualShapeConverter::~EGLRendererVisualShapeConverter()
{
	gWindow = 0;
	resetAll();
	delete m_data;
	
}

void EGLRendererVisualShapeConverter::setLightDirection(float x, float y, float z)
{
	m_data->m_lightDirection.setVal(x, y, z);
	m_data->m_hasLightDirection = true;
}

void EGLRendererVisualShapeConverter::setLightColor(float x, float y, float z)
{
	m_data->m_lightColor.setVal(x, y, z);
	m_data->m_hasLightColor = true;
}

void EGLRendererVisualShapeConverter::setLightDistance(float dist)
{
	m_data->m_lightDistance = dist;
	m_data->m_hasLightDistance = true;
}

void EGLRendererVisualShapeConverter::setShadow(bool hasShadow)
{
	m_data->m_hasShadow = hasShadow;
}
void EGLRendererVisualShapeConverter::setFlags(i32 flags)
{
	m_data->m_flags = flags;
}

void EGLRendererVisualShapeConverter::setLightAmbientCoeff(float ambientCoeff)
{
	m_data->m_lightAmbientCoeff = ambientCoeff;
	m_data->m_hasLightAmbientCoeff = true;
}

void EGLRendererVisualShapeConverter::setLightDiffuseCoeff(float diffuseCoeff)
{
	m_data->m_lightDiffuseCoeff = diffuseCoeff;
	m_data->m_hasLightDiffuseCoeff = true;
}

void EGLRendererVisualShapeConverter::setLightSpecularCoeff(float specularCoeff)
{
	m_data->m_lightSpecularCoeff = specularCoeff;
	m_data->m_hasLightSpecularCoeff = true;
}

///todo: merge into single file with TinyRendererVisualShapeConverter
static void convertURDFToVisualShape2(const UrdfShape* visual, tukk urdfPathPrefix, const Transform2& visualTransform, AlignedObjectArray<GLInstanceVertex>& verticesOut, AlignedObjectArray<i32>& indicesOut, AlignedObjectArray<MyTexture3>& texturesOut, b3VisualShapeData& visualShapeOut, struct CommonFileIOInterface* fileIO, i32 flags)
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

			BoxShape* boxShape = new BoxShape(extents * 0.5f);
			//ConvexShape* boxShape = new ConeShapeX(extents[2]*0.5,extents[0]*0.5);
			convexColShape = boxShape;
			convexColShape->setMargin(0.001);
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
		case URDF_GEOM_MESH:
		{
			strncpy(visualShapeOut.m_meshAssetFileName, visual->m_geometry.m_meshFileName.c_str(), VISUAL_SHAPE_MAX_PATH_LEN);
			visualShapeOut.m_meshAssetFileName[VISUAL_SHAPE_MAX_PATH_LEN - 1] = 0;

			visualShapeOut.m_dimensions[0] = visual->m_geometry.m_meshScale[0];
			visualShapeOut.m_dimensions[1] = visual->m_geometry.m_meshScale[1];
			visualShapeOut.m_dimensions[2] = visual->m_geometry.m_meshScale[2];

			switch (visual->m_geometry.m_meshFileType)
			{
				case UrdfGeometry::FILE_OBJ:
				{
					//glmesh = LoadMeshFromObj(fullPath,visualPathPrefix);
					b3ImportMeshData meshData;
					if (b3ImportMeshUtility::loadAndRegisterMeshFromFileInternal(visual->m_geometry.m_meshFileName, meshData, fileIO))
					{
						if (flags&URDF_USE_MATERIAL_COLORS_FROM_MTL)
						{
							if (meshData.m_flags & D3_IMPORT_MESH_HAS_RGBA_COLOR)
							{
								visualShapeOut.m_rgbaColor[0] = meshData.m_rgbaColor[0];
								visualShapeOut.m_rgbaColor[1] = meshData.m_rgbaColor[1];
								visualShapeOut.m_rgbaColor[2] = meshData.m_rgbaColor[2];
								
								if (flags&URDF_USE_MATERIAL_TRANSPARANCY_FROM_MTL)
								{
									visualShapeOut.m_rgbaColor[3] = meshData.m_rgbaColor[3];
								} else
								{
									visualShapeOut.m_rgbaColor[3] = 1;
								}
							}
						}
						if (meshData.m_textureImage1)
						{
							MyTexture3 texData;
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
					glmesh = LoadMeshFromSTL(visual->m_geometry.m_meshFileName.c_str(), fileIO);
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
										upAxis, fileIO);

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
					// should never get here (findExistingMeshFile returns false if it doesn't recognize extension)
					Assert(0);
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
				drx3DWarning("issue extracting mesh from COLLADA/STL file %s\n", visual->m_geometry.m_meshFileName.c_str());
			}
			break;
		}  // case mesh

		case URDF_GEOM_PLANE:
			// TODO: plane in egl renderer
			// TODO: export visualShapeOut for external render
			break;

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

static Vec4 sColors[4] =
{
		Vec4(60. / 256., 186. / 256., 84. / 256., 1),
		Vec4(244. / 256., 194. / 256., 13. / 256., 1),
		Vec4(219. / 256., 50. / 256., 54. / 256., 1),
		Vec4(72. / 256., 133. / 256., 237. / 256., 1),
};

i32 EGLRendererVisualShapeConverter::registerShapeAndInstance(const b3VisualShapeData& visualShape, const float* vertices, i32 numvertices, i32k* indices, i32 numIndices, i32 primitiveType, i32 textureId, i32 orgGraphicsUniqueId, i32 bodyUniqueId, i32 linkIndex)
{
	i32 uniqueId = orgGraphicsUniqueId;
	float rgbaColor[4] = { (float)visualShape.m_rgbaColor[0],(float)visualShape.m_rgbaColor[1], (float)visualShape.m_rgbaColor[2],(float)visualShape.m_rgbaColor[3] };

	EGLRendererObjectArray** visualsPtr = m_data->m_swRenderInstances[uniqueId];
	if (visualsPtr == 0)
	{
		m_data->m_swRenderInstances.insert(uniqueId, new EGLRendererObjectArray);
	}
	visualsPtr = m_data->m_swRenderInstances[uniqueId];
	if (visualsPtr)
	{
		EGLRendererObjectArray* visuals = *visualsPtr;

		//////////////

		D3_PROFILE("m_instancingRenderer register");

		// register mesh to m_instancingRenderer too.

		i32 innerTexId = -1;
		if (textureId >= 0 && textureId < m_data->m_textures.size())
		{
			innerTexId = m_data->m_textures[textureId].m_innerTexUid;
		}
		
		i32 shapeIndex = m_data->m_instancingRenderer->registerShape(&vertices[0],
			numvertices, &indices[0], numIndices, D3_GL_TRIANGLES, innerTexId);

		double scaling[3] = { visualShape.m_dimensions[0], visualShape.m_dimensions[1],visualShape.m_dimensions[2] };
		double position[4] = { visualShape.m_localVisualFrame[0],visualShape.m_localVisualFrame[1],visualShape.m_localVisualFrame[2],1 };
		double orn[4] = { visualShape.m_localVisualFrame[3],visualShape.m_localVisualFrame[4],visualShape.m_localVisualFrame[5],visualShape.m_localVisualFrame[6]};
		
		i32 graphicsIndex = m_data->m_instancingRenderer->registerGraphicsInstance(
			shapeIndex,
			position,
			orn,
			visualShape.m_rgbaColor,
			scaling);

		i32 segmentationMask = bodyUniqueId + ((linkIndex + 1) << 24);
		{
			if (graphicsIndex >= 0)
			{
				//visuals->m_graphicsInstanceIds.push_back(graphicsIndex);

				if (m_data->m_graphicsIndexToSegmentationMask.size() < (graphicsIndex + 1))
				{
					m_data->m_graphicsIndexToSegmentationMask.resize(graphicsIndex + 1);
				}
				m_data->m_graphicsIndexToSegmentationMask[graphicsIndex] = segmentationMask;
			}
		}

		m_data->m_instancingRenderer->writeTransforms();
		GLInstanceVertex* orgVertices = (GLInstanceVertex*)vertices;
		visuals->m_graphicsInstanceIds.push_back(graphicsIndex);
		visuals->m_vertices.resize(numvertices);
		for (i32 v = 0; v < numvertices; v++)
		{
			visuals->m_vertices[v] = orgVertices[v];
		}
	}

	m_data->m_visualShapes.push_back(visualShape);
	return uniqueId;
}



void EGLRendererVisualShapeConverter::updateShape(i32 shapeUniqueId, const Vec3* vertices, i32 numVertices, const Vec3* normals, i32 numNormals)
{
	EGLRendererObjectArray** visualsPtr = m_data->m_swRenderInstances[shapeUniqueId];
	if (visualsPtr)
	{
		EGLRendererObjectArray* visuals = *visualsPtr;
		for (i32 v = 0; v < visuals->m_graphicsInstanceIds.size(); v++)
		{
			i32 graphicsIndex = visuals->m_graphicsInstanceIds[v];
			if (graphicsIndex >= 0)
			{
				Assert(visuals->m_vertices.size() == numVertices);
				if (visuals->m_vertices.size() == numVertices)
				{
					for (i32 i = 0; i < numVertices; i++)
					{
						visuals->m_vertices[i].xyzw[0] = vertices[i][0];
						visuals->m_vertices[i].xyzw[1] = vertices[i][1];
						visuals->m_vertices[i].xyzw[2] = vertices[i][2];
					}
				}
				if (visuals->m_vertices.size() == numNormals)
				{
					for (i32 i = 0; i < numVertices; i++)
					{
						visuals->m_vertices[i].normal[0] = normals[i][0];
						visuals->m_vertices[i].normal[1] = normals[i][1];
						visuals->m_vertices[i].normal[2] = normals[i][2];
					}
				}
				m_data->m_instancingRenderer->updateShape(graphicsIndex, &visuals->m_vertices[0].xyzw[0],visuals->m_vertices.size());
			}
		}
	}
}


// If you are getting segfaults in this function it may be ecause you are
// compliling the plugin with differently from pybullet, try compiling the
// plugin with distutils too.
i32 EGLRendererVisualShapeConverter::convertVisualShapes(
	i32 linkIndex, tukk pathPrefix, const Transform2& localInertiaFrame,
	const UrdfLink* linkPtr, const UrdfModel* model,
	i32 orgGraphicsUniqueId, i32 bodyUniqueId, struct  CommonFileIOInterface* fileIO)
{
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
			AlignedObjectArray<MyTexture3> textures;
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
			// see note at function header
			Transform2 childTrans = vis->m_linkLocalFrame;

			i32 colorIndex = linkIndex;  //colObj? colObj->getBroadphaseHandle()->getUid() & 3 : 0;
			if (colorIndex < 0)
				colorIndex = 0;
			colorIndex &= 3;
			Vec4 color;
			color = sColors[colorIndex];
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

			EGLRendererObjectArray** visualsPtr = m_data->m_swRenderInstances[orgGraphicsUniqueId];
			if (visualsPtr == 0)
			{
				m_data->m_swRenderInstances.insert(orgGraphicsUniqueId, new EGLRendererObjectArray);
			}
			visualsPtr = m_data->m_swRenderInstances[orgGraphicsUniqueId];

			Assert(visualsPtr);
			EGLRendererObjectArray* visuals = *visualsPtr;
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
			i32 shapeIndex = -1;
			HashVisual tmp;
			{
				D3_PROFILE("convertURDFToVisualShape2");



				Transform2 tr = localInertiaFrame.inverse() * childTrans;
				tmp.m_vis = *vis;
				tmp.m_tr = tr;

				i32* bla = m_data->m_cachedVisualShapes[tmp];
				if (bla)
				{
					shapeIndex = *bla;
				}
				else
				{
					convertURDFToVisualShape2(vis, pathPrefix, tr, vertices, indices, textures, visualShape, fileIO, m_data->m_flags);

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
						MyTexture3 texData;
						texData.m_width = texWidth;
						texData.m_height = texHeight;
						texData.textureData1 = &m_data->m_checkeredTexels[0];
						texData.m_isCached = true;
						textures.push_back(texData);
					}
				}
			}
			m_data->m_visualShapes.push_back(visualShape);

			i32 textureIndex = -1;
			if (shapeIndex < 0)
			{
				if (vertices.size() && indices.size())
				{
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
						i32* bla = m_data->m_cachedTextureIds[textureImage1];
						if (bla)
						{
							textureIndex = *bla;
						}
						else
						{
							textureIndex = m_data->m_instancingRenderer->registerTexture(textureImage1, textureWidth, textureHeight);
							m_data->m_cachedTextureIds.insert(textureImage1, textureIndex);
						}
					}
				}
			}


			{
				D3_PROFILE("m_instancingRenderer register");

				// register mesh to m_instancingRenderer too.

				if (shapeIndex < 0 && vertices.size() > 0)
				{
					shapeIndex = m_data->m_instancingRenderer->registerShape(&vertices[0].xyzw[0], vertices.size(), &indices[0], indices.size(), D3_GL_TRIANGLES, textureIndex);
					m_data->m_cachedVisualShapes.insert(tmp, shapeIndex);
				}
				double scaling[3] = { 1, 1, 1 };
				i32 graphicsIndex = m_data->m_instancingRenderer->registerGraphicsInstance(shapeIndex, &visualShape.m_localVisualFrame[0], &visualShape.m_localVisualFrame[3], &visualShape.m_rgbaColor[0], scaling);
				
				i32 segmentationMask = bodyUniqueId + ((linkIndex + 1) << 24);
				{
					if (graphicsIndex >= 0)
					{
						visuals->m_graphicsInstanceIds.push_back(graphicsIndex);
						visuals->m_vertices.resize(vertices.size());
						for (i32 v = 0; v < vertices.size(); v++)
						{
							visuals->m_vertices[v] = vertices[v];
						}

						if (m_data->m_graphicsIndexToSegmentationMask.size() < (graphicsIndex + 1))
						{
							m_data->m_graphicsIndexToSegmentationMask.resize(graphicsIndex + 1);
						}
						m_data->m_graphicsIndexToSegmentationMask[graphicsIndex] = segmentationMask;
					}
				}

				m_data->m_instancingRenderer->writeTransforms();
			}

			for (i32 i = 0; i < textures.size(); i++)
			{
				if (!textures[i].m_isCached)
				{
					free(textures[i].textureData1);
				}
			}
		}
		
	}
	return orgGraphicsUniqueId;
}

i32 EGLRendererVisualShapeConverter::getNumVisualShapes(i32 bodyUniqueId)
{
	i32 start = -1;
	//find first one, then count how many
	for (i32 i = 0; i < m_data->m_visualShapes.size(); i++)
	{
		if (m_data->m_visualShapes[i].m_objectUniqueId == bodyUniqueId)
		{
			start = i;
			break;
		}
	}
	i32 count = 0;

	if (start >= 0)
	{
		for (i32 i = start; i < m_data->m_visualShapes.size(); i++)
		{
			if (m_data->m_visualShapes[i].m_objectUniqueId == bodyUniqueId)
			{
				count++;
			}
			else
			{
				//storage of each visual shape for a given body unique id assumed to be contiguous
				break;
			}
		}
	}
	return count;
}

i32 EGLRendererVisualShapeConverter::getVisualShapesData(i32 bodyUniqueId, i32 shapeIndex, struct b3VisualShapeData* shapeData)
{
	i32 start = -1;
	//find first one, then count how many
	for (i32 i = 0; i < m_data->m_visualShapes.size(); i++)
	{
		if (m_data->m_visualShapes[i].m_objectUniqueId == bodyUniqueId)
		{
			start = i;
			break;
		}
	}
	//i32 count = 0;

	if (start >= 0)
	{
		if (start + shapeIndex < m_data->m_visualShapes.size())
		{
			*shapeData = m_data->m_visualShapes[start + shapeIndex];
			return 1;
		}
	}
	return 0;
}

void EGLRendererVisualShapeConverter::changeInstanceFlags(i32 bodyUniqueId, i32 linkIndex, i32 shapeIndex, i32 flags)
{
	for (i32 i = 0; i < m_data->m_swRenderInstances.size(); i++)
	{
		EGLRendererObjectArray** ptrptr = m_data->m_swRenderInstances.getAtIndex(i);
		if (ptrptr && *ptrptr)
		{
			EGLRendererObjectArray* visuals = *ptrptr;
			for (i32 v = 0; v < visuals->m_graphicsInstanceIds.size(); v++)
			{
				i32 graphicsInstance = visuals->m_graphicsInstanceIds[v];
				if (graphicsInstance >= 0)
				{
					m_data->m_instancingRenderer->writeSingleInstanceFlagsToCPU(flags, graphicsInstance);
				}
			}
		}
	}
}

void EGLRendererVisualShapeConverter::changeShapeTexture(i32 bodyUniqueId, i32 linkIndex, i32 shapeIndex, i32 textureUniqueId)
{
	Assert(textureUniqueId < m_data->m_textures.size());
	if (textureUniqueId >= 0 && textureUniqueId < m_data->m_textures.size())
	{
		for (i32 i = 0; i < m_data->m_swRenderInstances.size(); i++)
		{
			EGLRendererObjectArray** ptrptr = m_data->m_swRenderInstances.getAtIndex(i);
			if (ptrptr && *ptrptr)
			{
				EGLRendererObjectArray* visuals = *ptrptr;
				if ((bodyUniqueId == visuals->m_objectUniqueId) && (linkIndex == visuals->m_linkIndex))
				{
					for (i32 i = 0; i < visuals->m_graphicsInstanceIds.size(); i++)
					{
						m_data->m_instancingRenderer->replaceTexture(visuals->m_graphicsInstanceIds[i], textureUniqueId);
					}
					
				}
			}
		}
	}
}

void EGLRendererVisualShapeConverter::changeRGBAColor(i32 bodyUniqueId, i32 linkIndex, i32 shapeIndex, const double rgbaColor[4])
{
	for (i32 i = 0; i < m_data->m_swRenderInstances.size(); i++)
	{
		EGLRendererObjectArray** ptrptr = m_data->m_swRenderInstances.getAtIndex(i);
		if (ptrptr && *ptrptr)
		{
			float rgba[4] = {(float)rgbaColor[0], (float)rgbaColor[1], (float)rgbaColor[2], (float)rgbaColor[3]};
			EGLRendererObjectArray* visuals = *ptrptr;
			if ((bodyUniqueId == visuals->m_objectUniqueId) && (linkIndex == visuals->m_linkIndex))
			{
				for (i32 i = 0; i < visuals->m_graphicsInstanceIds.size(); i++)
				{
					m_data->m_instancingRenderer->writeSingleInstanceColorToCPU(rgbaColor, visuals->m_graphicsInstanceIds[i]);
				}
				
			}
		}
	}
}

void EGLRendererVisualShapeConverter::setUpAxis(i32 axis)
{
	m_data->m_upAxis = axis;
	m_data->m_camera.setCameraUpAxis(axis);
	m_data->m_camera.update();
	m_data->m_instancingRenderer->updateCamera();
}
void EGLRendererVisualShapeConverter::resetCamera(float camDist, float yaw, float pitch, float camPosX, float camPosY, float camPosZ)
{
	m_data->m_camera.setCameraDistance(camDist);
	m_data->m_camera.setCameraPitch(pitch);
	m_data->m_camera.setCameraYaw(yaw);
	m_data->m_camera.setCameraTargetPosition(camPosX, camPosY, camPosZ);
	m_data->m_camera.setAspectRatio((float)m_data->m_swWidth / (float)m_data->m_swHeight);
	m_data->m_camera.update();
	
}

void EGLRendererVisualShapeConverter::clearBuffers(TGAColor& clearColor)
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

void EGLRendererVisualShapeConverter::render()
{
	//mode the the actual render code inside 'copyImageData' since we need to know the width/height
}

void EGLRendererVisualShapeConverter::render(const float viewMat[16], const float projMat[16])
{
	// This code is very similar to that of
	// PhysicsServerCommandProcessor::processRequestCameraImageCommand
	// maybe code from there should be moved.

	// Tiny allows rendering with viewMat, projMat explicitly, but
	// GLInstancingRender calls m_activeCamera, so set this.

	m_data->m_camera.setVRCamera(viewMat, projMat);

	render();

// don't disableVRCamera, see issue https://github.com/bulletphysics/bullet3/issues/2390
// todo: check out why
//
//	m_data->m_camera.disableVRCamera();

	//cout<<viewMat[4*0 + 0]<<" "<<viewMat[4*0+1]<<" "<<viewMat[4*0+2]<<" "<<viewMat[4*0+3] << endl;
	//cout<<viewMat[4*1 + 0]<<" "<<viewMat[4*1+1]<<" "<<viewMat[4*1+2]<<" "<<viewMat[4*1+3] << endl;
	//cout<<viewMat[4*2 + 0]<<" "<<viewMat[4*2+1]<<" "<<viewMat[4*2+2]<<" "<<viewMat[4*2+3] << endl;
	//cout<<viewMat[4*3 + 0]<<" "<<viewMat[4*3+1]<<" "<<viewMat[4*3+2]<<" "<<viewMat[4*3+3] << endl;
}

void EGLRendererVisualShapeConverter::getWidthAndHeight(i32& width, i32& height)
{
	width = m_data->m_swWidth;
	height = m_data->m_swHeight;
}

void EGLRendererVisualShapeConverter::setWidthAndHeight(i32 width, i32 height)
{
	m_data->m_swWidth = width;
	m_data->m_swHeight = height;

	m_data->m_depthBuffer.resize(m_data->m_swWidth * m_data->m_swHeight);
	m_data->m_shadowBuffer.resize(m_data->m_swWidth * m_data->m_swHeight);
	m_data->m_segmentationMaskBuffer.resize(m_data->m_swWidth * m_data->m_swHeight);
	m_data->m_rgbColorBuffer = TGAImage(width, height, TGAImage::RGB);
}

void EGLRendererVisualShapeConverter::setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16])
{
	m_data->m_instancingRenderer->setProjectiveTextureMatrices(viewMatrix,projectionMatrix);
}

void EGLRendererVisualShapeConverter::setProjectiveTexture(bool useProjectiveTexture)
{
	m_data->m_instancingRenderer->setProjectiveTexture(useProjectiveTexture);
}

//copied from OpenGLGuiHelper.cpp
void EGLRendererVisualShapeConverter::copyCameraImageDataGL(
	u8* pixelsRGBA, i32 rgbaBufferSizeInPixels,
	float* depthBuffer, i32 depthBufferSizeInPixels,
	i32* segmentationMaskBuffer, i32 segmentationMaskSizeInPixels,
	i32 startPixelIndex, i32* widthPtr, i32* heightPtr, i32* numPixelsCopied)
{
	if (numPixelsCopied)
		*numPixelsCopied = 0;

	i32 destinationWidth = *widthPtr;
	i32 destinationHeight = *heightPtr;
	i32 sourceWidth =  d3Min(destinationWidth, (i32)(m_data->m_window->getWidth() * m_data->m_window->getRetinaScale()));
	i32 sourceHeight = d3Min(destinationHeight, (i32)(m_data->m_window->getHeight() * m_data->m_window->getRetinaScale()));

	i32 numTotalPixels = (*widthPtr) * (*heightPtr);
	i32 numRemainingPixels = numTotalPixels - startPixelIndex;
	i32 numBytesPerPixel = 4;  //RGBA
	i32 numRequestedPixels = d3Min(rgbaBufferSizeInPixels, numRemainingPixels);
	if (1)
	{
		if (startPixelIndex == 0)
		{
			m_data->m_window->endRendering();
			m_data->m_window->startRendering();
			glViewport(0,0, sourceWidth*m_data->m_window->getRetinaScale(), sourceHeight*m_data->m_window->getRetinaScale());
			D3_PROFILE("m_instancingRenderer render");
			m_data->m_instancingRenderer->writeTransforms();
			if (m_data->m_hasLightDirection)
			{
				m_data->m_instancingRenderer->setLightPosition(m_data->m_lightDirection);
			}
			m_data->m_instancingRenderer->setActiveCamera(&m_data->m_camera);
			m_data->m_instancingRenderer->updateCamera(m_data->m_upAxis);

			m_data->m_instancingRenderer->renderScene();

			i32 numBytesPerPixel = 4;  //RGBA

			if (pixelsRGBA || depthBuffer)
			{
				{
					DRX3D_PROFILE("copy pixels");

					//copy the image into our local cache
					m_data->m_sourceRgbaPixelBuffer.resize(sourceWidth * sourceHeight * numBytesPerPixel);
					m_data->m_sourceDepthBuffer.resize(sourceWidth * sourceHeight);
					{
						DRX3D_PROFILE("getScreenPixels");
						i32 rgbaBufferSizeInPixels = m_data->m_sourceRgbaPixelBuffer.size();
						i32 depthBufferSizeInPixels = m_data->m_sourceDepthBuffer.size();
						// Copied from SimpleOpenGL3App::getScreenPixels
						drx3DAssert((sourceWidth * sourceHeight * 4) == rgbaBufferSizeInPixels);
						//glClear(GL_COLOR_BUFFER_BIT);
						//drx3DWarning("EGL\n");
						if ((sourceWidth * sourceHeight * 4) == rgbaBufferSizeInPixels)  // remove this if
						{
							glReadPixels(0, 0, sourceWidth, sourceHeight, GL_RGBA, GL_UNSIGNED_BYTE, &(m_data->m_sourceRgbaPixelBuffer[0]));
							i32 glstat;
							glstat = glGetError();
							drx3DAssert(glstat == GL_NO_ERROR);
						}
						if ((sourceWidth * sourceHeight) == depthBufferSizeInPixels)
						{
							glReadPixels(0, 0, sourceWidth, sourceHeight, GL_DEPTH_COMPONENT, GL_FLOAT, &(m_data->m_sourceDepthBuffer[0]));
							i32 glstat;
							glstat = glGetError();
							drx3DAssert(glstat == GL_NO_ERROR);
						}
					}
				}
			
				m_data->m_rgbaPixelBuffer1.resize((*widthPtr) * (*heightPtr) * numBytesPerPixel);
				m_data->m_depthBuffer1.resize((*widthPtr) * (*heightPtr));
				//rescale and flip
				{
					DRX3D_PROFILE("resize and flip");
					for (i32 j = 0; j < *heightPtr; j++)
					{
						for (i32 i = 0; i < *widthPtr; i++)
						{
							i32 xIndex = i32(float(i) * (float(sourceWidth) / float(*widthPtr)));
							i32 yIndex = i32(float(*heightPtr - 1 - j) * (float(sourceHeight) / float(*heightPtr)));
							Clamp(xIndex, 0, sourceWidth);
							Clamp(yIndex, 0, sourceHeight);
							i32 bytesPerPixel = 4;  //RGBA

							i32 sourcePixelIndex = (xIndex + yIndex * sourceWidth) * bytesPerPixel;
							i32 sourceDepthIndex = xIndex + yIndex * sourceWidth;
	#define COPY4PIXELS 1
	#ifdef COPY4PIXELS
							i32* dst = (i32*)&m_data->m_rgbaPixelBuffer1[(i + j * (*widthPtr)) * 4 + 0];
							i32* src = (i32*)&m_data->m_sourceRgbaPixelBuffer[sourcePixelIndex + 0];
							*dst = *src;

	#else
							m_data->m_rgbaPixelBuffer1[(i + j * widthPtr) * 4 + 0] = sourceRgbaPixelBuffer[sourcePixelIndex + 0];
							m_data->m_rgbaPixelBuffer1[(i + j * widthPtr) * 4 + 1] = sourceRgbaPixelBuffer[sourcePixelIndex + 1];
							m_data->m_rgbaPixelBuffer1[(i + j * widthPtr) * 4 + 2] = sourceRgbaPixelBuffer[sourcePixelIndex + 2];
							m_data->m_rgbaPixelBuffer1[(i + j * widthPtr) * 4 + 3] = 255;
	#endif
							if (depthBuffer)
							{
								m_data->m_depthBuffer1[i + j * (*widthPtr)] = m_data->m_sourceDepthBuffer[sourceDepthIndex];
							}
						}
					}
				}
			}

			if (segmentationMaskBuffer)
			{
				{
					m_data->m_window->startRendering();
					glViewport(0,0, sourceWidth*m_data->m_window->getRetinaScale(), sourceHeight*m_data->m_window->getRetinaScale());
					DRX3D_PROFILE("renderScene");
					m_data->m_instancingRenderer->renderSceneInternal(D3_SEGMENTATION_MASK_RENDERMODE);
				}

				{
					DRX3D_PROFILE("copy pixels");

					//copy the image into our local cache
					m_data->m_segmentationMaskSourceRgbaPixelBuffer.resize(sourceWidth * sourceHeight * numBytesPerPixel);
					m_data->m_segmentationMaskSourceDepthBuffer.resize(sourceWidth * sourceHeight);
					{
						DRX3D_PROFILE("getScreenPixels");
						{
							glReadPixels(0, 0, sourceWidth, sourceHeight, GL_DEPTH_COMPONENT, GL_FLOAT, &(m_data->m_segmentationMaskSourceDepthBuffer[0]));
							i32 glstat;
							glstat = glGetError();
							drx3DAssert(glstat == GL_NO_ERROR);
						}
						{
							glReadPixels(0, 0, sourceWidth, sourceHeight, GL_RGBA, GL_UNSIGNED_BYTE, &(m_data->m_segmentationMaskSourceRgbaPixelBuffer[0]));
							i32 glstat;
							glstat = glGetError();
							drx3DAssert(glstat == GL_NO_ERROR);
						}
					}
				}
				m_data->m_segmentationMaskBuffer.resize(destinationWidth * destinationHeight, -1);

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

							if (segmentationMaskBuffer)
							{
								float depth = m_data->m_segmentationMaskSourceDepthBuffer[sourceDepthIndex];
								if (depth < 1)
								{
									i32 segMask = m_data->m_segmentationMaskSourceRgbaPixelBuffer[sourcePixelIndex + 0] + 256 * (m_data->m_segmentationMaskSourceRgbaPixelBuffer[sourcePixelIndex + 1]) + 256 * 256 * (m_data->m_segmentationMaskSourceRgbaPixelBuffer[sourcePixelIndex + 2]);
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
			glViewport(0, 0, m_data->m_window->getWidth() * m_data->m_window->getRetinaScale(), m_data->m_window->getHeight() * m_data->m_window->getRetinaScale());
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
			DRX3D_PROFILE("copy segmentation mask buffer pixels");
			for (i32 i = 0; i < numRequestedPixels; i++)
			{
				i32 graphicsIndexSegMask = m_data->m_segmentationMaskBuffer[i + startPixelIndex];
				i32 segMask = -1;
				if (graphicsIndexSegMask >= 0 && graphicsIndexSegMask < m_data->m_graphicsIndexToSegmentationMask.size())
				{
					segMask = m_data->m_graphicsIndexToSegmentationMask[graphicsIndexSegMask];
				}
				if ((m_data->m_flags & ER_SEGMENTATION_MASK_OBJECT_AND_LINKINDEX) == 0)
				{
					if (segMask >= 0)
					{
						segMask &= ((1 << 24) - 1);
					}
				}
				segmentationMaskBuffer[i] = segMask;
			}
		}

		if (numPixelsCopied)
		{
			*numPixelsCopied = numRequestedPixels;
		}
	}
}

void EGLRendererVisualShapeConverter::copyCameraImageData(u8* pixelsRGBA, i32 rgbaBufferSizeInPixels,
														  float* depthBuffer, i32 depthBufferSizeInPixels,
														  i32* segmentationMaskBuffer, i32 segmentationMaskSizeInPixels,
														  i32 startPixelIndex, i32* widthPtr, i32* heightPtr, i32* numPixelsCopied)
{
	D3_PROFILE("copyCameraImageDataGL");
	copyCameraImageDataGL(pixelsRGBA, rgbaBufferSizeInPixels,
						  depthBuffer, depthBufferSizeInPixels,
						  segmentationMaskBuffer, segmentationMaskSizeInPixels,
						  startPixelIndex, widthPtr, heightPtr, numPixelsCopied);
}

void EGLRendererVisualShapeConverter::removeVisualShape(i32 collisionObjectUniqueId)
{
	EGLRendererObjectArray** ptrptr = m_data->m_swRenderInstances[collisionObjectUniqueId];
	if (ptrptr && *ptrptr)
	{
		EGLRendererObjectArray* ptr = *ptrptr;
		if (ptr)
		{
			for (i32 i = 0; i < ptr->m_graphicsInstanceIds.size(); i++)
			{
				m_data->m_instancingRenderer->removeGraphicsInstance(ptr->m_graphicsInstanceIds[i]);
			}
			
		}
		delete ptr;
		m_data->m_swRenderInstances.remove(collisionObjectUniqueId);
	}
}

void EGLRendererVisualShapeConverter::resetAll()
{
	m_data->m_cachedTextureIds.clear();
	
	for (i32 i = 0; i < m_data->m_swRenderInstances.size(); i++)
	{
		EGLRendererObjectArray** ptrptr = m_data->m_swRenderInstances.getAtIndex(i);
		if (ptrptr && *ptrptr)
		{
			EGLRendererObjectArray* ptr = *ptrptr;
			
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
	m_data->m_visualShapes.clear();
	m_data->m_graphicsIndexToSegmentationMask.clear();
	m_data->m_instancingRenderer->removeAllInstances();
	m_data->m_cachedVisualShapes.clear();
}



i32 EGLRendererVisualShapeConverter::registerTexture(u8* texels, i32 width, i32 height)
{
	MyTexture3 texData;
	texData.m_width = width;
	texData.m_height = height;
	texData.textureData1 = texels;
	texData.m_isCached = true;
	texData.m_innerTexUid = m_data->m_instancingRenderer->registerTexture(texels, width, height);
	m_data->m_textures.push_back(texData);
	return m_data->m_textures.size() - 1;
}

i32 EGLRendererVisualShapeConverter::registerTextureInternal(u8* texels, i32 width, i32 height)
{
	MyTexture3 texData;
	texData.m_width = width;
	texData.m_height = height;
	texData.textureData1 = texels;
	texData.m_isCached = false;
	texData.m_innerTexUid = m_data->m_instancingRenderer->registerTexture(texels, width, height);
	m_data->m_textures.push_back(texData);
	return m_data->m_textures.size() - 1;
}

i32 EGLRendererVisualShapeConverter::loadTextureFile(tukk filename, struct CommonFileIOInterface* fileIO)
{
	D3_PROFILE("loadTextureFile");
	i32 width, height, n;
	u8* image = 0;

	if (fileIO)
	{
		b3AlignedObjectArray<char> buffer;
		buffer.reserve(1024);
		i32 fileId = fileIO->fileOpen(filename,"rb");
		if (fileId>=0)
		{
			i32 size = fileIO->getFileSize(fileId);
			if (size>0)
			{
				buffer.resize(size);
				i32 actual = fileIO->fileRead(fileId,&buffer[0],size);
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
	} else
	{
		image = stbi_load(filename, &width, &height, &n, 3);
	}

	if (image && (width >= 0) && (height >= 0))
	{
		return registerTextureInternal(image, width, height);
	}
	return -1;
}

void EGLRendererVisualShapeConverter::syncTransform(i32 collisionObjectUniqueId, const Transform2& worldTransform, const Vec3& localScaling)
{
	EGLRendererObjectArray** renderObjPtr = m_data->m_swRenderInstances[collisionObjectUniqueId];
	if (renderObjPtr)
	{
		EGLRendererObjectArray* renderObj = *renderObjPtr;
		renderObj->m_worldTransform = worldTransform;
		renderObj->m_localScaling = localScaling;
		for (i32 i = 0; i < renderObj->m_graphicsInstanceIds.size(); i++)
		{
			i32 graphicsInstanceId = renderObj->m_graphicsInstanceIds[i];
			if (graphicsInstanceId >= 0)
			{
				Vec3 pos = worldTransform.getOrigin();
				Quat orn = worldTransform.getRotation();
				m_data->m_instancingRenderer->writeSingleInstanceTransformToCPU(pos, orn, graphicsInstanceId);
			}
		}
	}
}

bool EGLRendererVisualShapeConverter::getCameraInfo(i32* width, i32* height, float viewMatrix[16], float projectionMatrix[16], float camUp[3], float camForward[3], float hor[3], float vert[3], float* yaw, float* pitch, float* camDist, float cameraTarget[3]) const
{
	if (m_data->m_instancingRenderer && m_data->m_instancingRenderer->getActiveCamera())
	{
		*width = m_data->m_window->getWidth() * m_data->m_window->getRetinaScale();
		*height = m_data->m_window->getHeight() * m_data->m_window->getRetinaScale();
		m_data->m_instancingRenderer->getActiveCamera()->getCameraViewMatrix(viewMatrix);
		m_data->m_instancingRenderer->getActiveCamera()->getCameraProjectionMatrix(projectionMatrix);
		m_data->m_instancingRenderer->getActiveCamera()->getCameraUpVector(camUp);
		m_data->m_instancingRenderer->getActiveCamera()->getCameraForwardVector(camForward);

		float top = 1.f;
		float bottom = -1.f;
		float tanFov = (top - bottom) * 0.5f / 1;
		float fov = Scalar(2.0) * Atan(tanFov);
		Vec3 camPos, camTarget;
		m_data->m_instancingRenderer->getActiveCamera()->getCameraPosition(camPos);
		m_data->m_instancingRenderer->getActiveCamera()->getCameraTargetPosition(camTarget);
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
		hor[0] = hori[0] * m_data->m_window->getRetinaScale();
		hor[1] = hori[1] * m_data->m_window->getRetinaScale();
		hor[2] = hori[2] * m_data->m_window->getRetinaScale();
		vert[0] = vertical[0] * m_data->m_window->getRetinaScale();
		vert[1] = vertical[1] * m_data->m_window->getRetinaScale();
		vert[2] = vertical[2] * m_data->m_window->getRetinaScale();

		*yaw = m_data->m_instancingRenderer->getActiveCamera()->getCameraYaw();
		*pitch = m_data->m_instancingRenderer->getActiveCamera()->getCameraPitch();
		*camDist = m_data->m_instancingRenderer->getActiveCamera()->getCameraDistance();
		cameraTarget[0] = camTarget[0];
		cameraTarget[1] = camTarget[1];
		cameraTarget[2] = camTarget[2];
		return true;
	}
	return false;
}
