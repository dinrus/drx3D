
#include "../RaytracerSetup.h"

#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/b3Quat.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include "../../TinyRenderer/TinyRenderer.h"
#include <drx3D/Common/Interfaces/Common2dCanvasInterface.h>
#include <drx3D/Physics/Collision/NarrowPhase/SubSimplexConvexCast.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/CollisionCommon.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/ExampleBrowser/CollisionShape2TriangleMesh.h>
#include <drx3D/Importers/MeshUtility/b3ImportMeshUtility.h>
#include <drx3D/OpenGLWindow/GLInstanceGraphicsShape.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <drx3D/Common/DefaultFileIO.h>

struct TinyRendererSetupInternalData
{
	TGAImage m_rgbColorBuffer;
	b3AlignedObjectArray<float> m_depthBuffer;
	b3AlignedObjectArray<float> m_shadowBuffer;
	b3AlignedObjectArray<i32> m_segmentationMaskBuffer;

	i32 m_width;
	i32 m_height;

	AlignedObjectArray<ConvexShape*> m_shapePtr;
	AlignedObjectArray<Transform2> m_transforms;
	AlignedObjectArray<TinyRenderObjectData*> m_renderObjects;

	VoronoiSimplexSolver m_simplexSolver;
	Scalar m_pitch;
	Scalar m_roll;
	Scalar m_yaw;

	i32 m_textureHandle;
	i32 m_animateRenderer;

	Vec3 m_lightPos;

	TinyRendererSetupInternalData(i32 width, i32 height)
		: m_rgbColorBuffer(width, height, TGAImage::RGB),
		  m_width(width),
		  m_height(height),
		  m_pitch(0),
		  m_roll(0),
		  m_yaw(0),
		  m_textureHandle(0),
		  m_animateRenderer(0)
	{
		m_lightPos.setVal(-3, 15, 15);

		m_depthBuffer.resize(m_width * m_height);
		m_shadowBuffer.resize(m_width * m_height);
		//        m_segmentationMaskBuffer.resize(m_width*m_height);
	}
	void updateTransforms()
	{
		i32 numObjects = m_shapePtr.size();
		m_transforms.resize(numObjects);
		for (i32 i = 0; i < numObjects; i++)
		{
			m_transforms[i].setIdentity();
			//Vec3	pos(0.f,-(2.5* numObjects * 0.5)+i*2.5f, 0.f);
			Vec3 pos(0.f, +i * 2.5f, 0.f);
			m_transforms[i].setIdentity();
			m_transforms[i].setOrigin(pos);
			Quat orn;
			if (i < 2)
			{
				orn.setEuler(m_yaw, m_pitch, m_roll);
				m_transforms[i].setRotation(orn);
			}
		}
		if (m_animateRenderer)
		{
			m_pitch += 0.005f;
			m_yaw += 0.01f;
		}
	}
};

struct TinyRendererSetup : public CommonExampleInterface
{
	struct GUIHelperInterface* m_guiHelper;
	struct CommonGraphicsApp* m_app;
	struct TinyRendererSetupInternalData* m_internalData;
	bool m_useSoftware;

	TinyRendererSetup(struct GUIHelperInterface* guiHelper);

	virtual ~TinyRendererSetup();

	virtual void initPhysics();

	virtual void exitPhysics();

	virtual void stepSimulation(float deltaTime);

	virtual void physicsDebugDraw(i32 debugFlags);

	virtual void syncPhysicsToGraphics(struct GraphicsPhysicsBridge& gfxBridge);

	virtual bool mouseMoveCallback(float x, float y);

	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y);

	virtual bool keyboardCallback(i32 key, i32 state);

	virtual void renderScene();

	void animateRenderer(i32 animateRendererIndex)
	{
		m_internalData->m_animateRenderer = animateRendererIndex;
	}

	void selectRenderer(i32 rendererIndex)
	{
		m_useSoftware = (rendererIndex == 0);
	}

	void resetCamera()
	{
		float dist = 11;
		float pitch = -35;
		float yaw = 52;
		float targetPos[3] = {0, 0.46, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

TinyRendererSetup::TinyRendererSetup(struct GUIHelperInterface* gui)
{
	m_useSoftware = false;

	m_guiHelper = gui;
	m_app = gui->getAppInterface();
	m_internalData = new TinyRendererSetupInternalData(gui->getAppInterface()->m_window->getWidth(), gui->getAppInterface()->m_window->getHeight());

	tukk fileName = "textured_sphere_smooth.obj";
	fileName = "cube.obj";
	fileName = "torus/torus_with_plane.obj";

	{
		{
			i32 shapeId = -1;

			b3ImportMeshData meshData;
			DefaultFileIO fileIO;
			if (b3ImportMeshUtility::loadAndRegisterMeshFromFileInternal(fileName, meshData,&fileIO))
			{
				i32 textureIndex = -1;

				if (meshData.m_textureImage1)
				{
					textureIndex = m_guiHelper->getRenderInterface()->registerTexture(meshData.m_textureImage1, meshData.m_textureWidth, meshData.m_textureHeight);
				}

				shapeId = m_guiHelper->getRenderInterface()->registerShape(&meshData.m_gfxShape->m_vertices->at(0).xyzw[0],
																		   meshData.m_gfxShape->m_numvertices,
																		   &meshData.m_gfxShape->m_indices->at(0),
																		   meshData.m_gfxShape->m_numIndices,
																		   D3_GL_TRIANGLES,
																		   textureIndex);

				float position[4] = {0, 0, 0, 1};
				float orn[4] = {0, 0, 0, 1};
				float color[4] = {1, 1, 1, 1};
				float scaling[4] = {1, 1, 1, 1};

				m_guiHelper->getRenderInterface()->registerGraphicsInstance(shapeId, position, orn, color, scaling);
				m_guiHelper->getRenderInterface()->writeTransforms();

				m_internalData->m_shapePtr.push_back(0);
				TinyRenderObjectData* ob = new TinyRenderObjectData(
					m_internalData->m_rgbColorBuffer,
					m_internalData->m_depthBuffer,
					&m_internalData->m_shadowBuffer,
					&m_internalData->m_segmentationMaskBuffer,
					m_internalData->m_renderObjects.size(), -1);

				meshData.m_gfxShape->m_scaling[0] = scaling[0];
				meshData.m_gfxShape->m_scaling[1] = scaling[1];
				meshData.m_gfxShape->m_scaling[2] = scaling[2];

				i32k* indices = &meshData.m_gfxShape->m_indices->at(0);
				ob->registerMeshShape(&meshData.m_gfxShape->m_vertices->at(0).xyzw[0],
									  meshData.m_gfxShape->m_numvertices,
									  indices,
									  meshData.m_gfxShape->m_numIndices, color, meshData.m_textureImage1, meshData.m_textureWidth, meshData.m_textureHeight);

				ob->m_localScaling.setVal(scaling[0], scaling[1], scaling[2]);

				m_internalData->m_renderObjects.push_back(ob);

				delete meshData.m_gfxShape;
				if (!meshData.m_isCached)
				{
					delete meshData.m_textureImage1;
				}
			}
		}
	}
}

TinyRendererSetup::~TinyRendererSetup()
{
	delete m_internalData;
}

tukk itemsanimate[] = {"Fixed", "Rotate"};
void TinyRendererComboCallbackAnimate(i32 combobox, tukk item, uk userPointer)
{
	TinyRendererSetup* cl = (TinyRendererSetup*)userPointer;
	drx3DAssert(cl);
	i32 index = -1;
	i32 numItems = sizeof(itemsanimate) / sizeof(tuk);
	for (i32 i = 0; i < numItems; i++)
	{
		if (!strcmp(item, itemsanimate[i]))
		{
			index = i;
		}
	}
	cl->animateRenderer(index);
}

tukk items[] = {"Software", "OpenGL"};

void TinyRendererComboCallback(i32 combobox, tukk item, uk userPointer)
{
	TinyRendererSetup* cl = (TinyRendererSetup*)userPointer;
	drx3DAssert(cl);
	i32 index = -1;
	i32 numItems = sizeof(items) / sizeof(tuk);
	for (i32 i = 0; i < numItems; i++)
	{
		if (!strcmp(item, items[i]))
		{
			index = i;
		}
	}
	cl->selectRenderer(index);
}

void TinyRendererSetup::initPhysics()
{
	//request a visual bitma/texture we can render to

	m_app->setUpAxis(2);

	CommonRenderInterface* render = m_app->m_renderer;

	m_internalData->m_textureHandle = render->registerTexture(m_internalData->m_rgbColorBuffer.buffer(), m_internalData->m_width, m_internalData->m_height);

	{
		ComboBoxParams comboParams;
		comboParams.m_userPointer = this;
		comboParams.m_numItems = sizeof(items) / sizeof(tuk);
		comboParams.m_startItem = 1;
		comboParams.m_items = items;
		comboParams.m_callback = TinyRendererComboCallback;
		m_guiHelper->getParameterInterface()->registerComboBox(comboParams);
	}

	{
		ComboBoxParams comboParams;
		comboParams.m_userPointer = this;
		comboParams.m_numItems = sizeof(itemsanimate) / sizeof(tuk);
		comboParams.m_startItem = 0;
		comboParams.m_items = itemsanimate;
		comboParams.m_callback = TinyRendererComboCallbackAnimate;
		m_guiHelper->getParameterInterface()->registerComboBox(comboParams);
	}

	{
		SliderParams slider("LightPosX", &m_internalData->m_lightPos[0]);
		slider.m_minVal = -10;
		slider.m_maxVal = 10;
		if (m_guiHelper->getParameterInterface())
			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		SliderParams slider("LightPosY", &m_internalData->m_lightPos[1]);
		slider.m_minVal = -10;
		slider.m_maxVal = 10;
		if (m_guiHelper->getParameterInterface())
			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		SliderParams slider("LightPosZ", &m_internalData->m_lightPos[2]);
		slider.m_minVal = -10;
		slider.m_maxVal = 10;
		if (m_guiHelper->getParameterInterface())
			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
}

void TinyRendererSetup::exitPhysics()
{
}

void TinyRendererSetup::stepSimulation(float deltaTime)
{
	m_internalData->updateTransforms();
}

void TinyRendererSetup::renderScene()
{
	m_internalData->updateTransforms();

	Vec4 from(m_internalData->m_lightPos[0], m_internalData->m_lightPos[1], m_internalData->m_lightPos[2], 1);
	Vec4 toX(m_internalData->m_lightPos[0] + 0.1, m_internalData->m_lightPos[1], m_internalData->m_lightPos[2], 1);
	Vec4 toY(m_internalData->m_lightPos[0], m_internalData->m_lightPos[1] + 0.1, m_internalData->m_lightPos[2], 1);
	Vec4 toZ(m_internalData->m_lightPos[0], m_internalData->m_lightPos[1], m_internalData->m_lightPos[2] + 0.1, 1);
	Vec4 colorX(1, 0, 0, 1);
	Vec4 colorY(0, 1, 0, 1);
	Vec4 colorZ(0, 0, 1, 1);
	i32 width = 2;
	m_guiHelper->getRenderInterface()->drawLine(from, toX, colorX, width);
	m_guiHelper->getRenderInterface()->drawLine(from, toY, colorY, width);
	m_guiHelper->getRenderInterface()->drawLine(from, toZ, colorZ, width);

	if (!m_useSoftware)
	{
		Vec3 lightPos(m_internalData->m_lightPos[0], m_internalData->m_lightPos[1], m_internalData->m_lightPos[2]);
		m_guiHelper->getRenderInterface()->setLightPosition(lightPos);

		for (i32 i = 0; i < m_internalData->m_transforms.size(); i++)
		{
			m_guiHelper->getRenderInterface()->writeSingleInstanceTransformToCPU(m_internalData->m_transforms[i].getOrigin(), m_internalData->m_transforms[i].getRotation(), i);
		}
		m_guiHelper->getRenderInterface()->writeTransforms();
		m_guiHelper->getRenderInterface()->renderScene();
	}
	else
	{
		TGAColor clearColor;
		clearColor.bgra[0] = 200;
		clearColor.bgra[1] = 200;
		clearColor.bgra[2] = 200;
		clearColor.bgra[3] = 255;
		for (i32 y = 0; y < m_internalData->m_height; ++y)
		{
			for (i32 x = 0; x < m_internalData->m_width; ++x)
			{
				m_internalData->m_rgbColorBuffer.set(x, y, clearColor);
				m_internalData->m_depthBuffer[x + y * m_internalData->m_width] = -1e30f;
				m_internalData->m_shadowBuffer[x + y * m_internalData->m_width] = -1e30f;
			}
		}

		ATTRIBUTE_ALIGNED16(Scalar modelMat2[16]);
		ATTRIBUTE_ALIGNED16(float viewMat[16]);
		ATTRIBUTE_ALIGNED16(float projMat[16]);
		CommonRenderInterface* render = this->m_app->m_renderer;
		render->getActiveCamera()->getCameraViewMatrix(viewMat);
		render->getActiveCamera()->getCameraProjectionMatrix(projMat);

		for (i32 o = 0; o < this->m_internalData->m_renderObjects.size(); o++)
		{
			const Transform2& tr = m_internalData->m_transforms[o];
			tr.getOpenGLMatrix(modelMat2);

			for (i32 i = 0; i < 4; i++)
			{
				for (i32 j = 0; j < 4; j++)
				{
					m_internalData->m_renderObjects[o]->m_modelMatrix[i][j] = float(modelMat2[i + 4 * j]);
					m_internalData->m_renderObjects[o]->m_viewMatrix[i][j] = viewMat[i + 4 * j];
					m_internalData->m_renderObjects[o]->m_projectionMatrix[i][j] = projMat[i + 4 * j];

					Vec3 lightDirWorld = Vec3(m_internalData->m_lightPos[0], m_internalData->m_lightPos[1], m_internalData->m_lightPos[2]);

					m_internalData->m_renderObjects[o]->m_lightDirWorld = lightDirWorld.normalized();

					Vec3 lightColor(1.0, 1.0, 1.0);
					m_internalData->m_renderObjects[o]->m_lightColor = lightColor;

					m_internalData->m_renderObjects[o]->m_lightDistance = 10.0;
					m_internalData->m_renderObjects[o]->m_lightAmbientCoeff = 0.6;
					m_internalData->m_renderObjects[o]->m_lightDiffuseCoeff = 0.35;
					m_internalData->m_renderObjects[o]->m_lightSpecularCoeff = 0.05;
				}
			}
			TinyRenderer::renderObjectDepth(*m_internalData->m_renderObjects[o]);
		}

		for (i32 o = 0; o < this->m_internalData->m_renderObjects.size(); o++)
		{
			const Transform2& tr = m_internalData->m_transforms[o];
			tr.getOpenGLMatrix(modelMat2);

			for (i32 i = 0; i < 4; i++)
			{
				for (i32 j = 0; j < 4; j++)
				{
					m_internalData->m_renderObjects[o]->m_modelMatrix[i][j] = float(modelMat2[i + 4 * j]);
					m_internalData->m_renderObjects[o]->m_viewMatrix[i][j] = viewMat[i + 4 * j];
					m_internalData->m_renderObjects[o]->m_projectionMatrix[i][j] = projMat[i + 4 * j];

					Vec3 lightDirWorld = Vec3(m_internalData->m_lightPos[0], m_internalData->m_lightPos[1], m_internalData->m_lightPos[2]);

					m_internalData->m_renderObjects[o]->m_lightDirWorld = lightDirWorld.normalized();

					Vec3 lightColor(1.0, 1.0, 1.0);
					m_internalData->m_renderObjects[o]->m_lightColor = lightColor;

					m_internalData->m_renderObjects[o]->m_lightDistance = 10.0;
					m_internalData->m_renderObjects[o]->m_lightAmbientCoeff = 0.6;
					m_internalData->m_renderObjects[o]->m_lightDiffuseCoeff = 0.35;
					m_internalData->m_renderObjects[o]->m_lightSpecularCoeff = 0.05;
				}
			}
			TinyRenderer::renderObject(*m_internalData->m_renderObjects[o]);
		}
		//m_app->drawText("hello",500,500);
		render->activateTexture(m_internalData->m_textureHandle);
		render->updateTexture(m_internalData->m_textureHandle, m_internalData->m_rgbColorBuffer.buffer());
		float color[4] = {1, 1, 1, 1};
		m_app->drawTexturedRect(0, 0, m_app->m_window->getWidth(), m_app->m_window->getHeight(), color, 0, 0, 1, 1, true);
	}
}

void TinyRendererSetup::physicsDebugDraw(i32 debugDrawFlags)
{
}

bool TinyRendererSetup::mouseMoveCallback(float x, float y)
{
	return false;
}

bool TinyRendererSetup::mouseButtonCallback(i32 button, i32 state, float x, float y)
{
	return false;
}

bool TinyRendererSetup::keyboardCallback(i32 key, i32 state)
{
	return false;
}

void TinyRendererSetup::syncPhysicsToGraphics(GraphicsPhysicsBridge& gfxBridge)
{
}

CommonExampleInterface* TinyRendererCreateFunc(struct CommonExampleOptions& options)
{
	return new TinyRendererSetup(options.m_guiHelper);
}
