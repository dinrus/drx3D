
#include "../DynamicTexturedCubeDemo.h"
#include <drx3D/Common/b3Logging.h>
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/b3Quat.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include "../../GwenGUISupport/GraphingTexture.h"

#include <drx3D/Common/Interfaces/Common2dCanvasInterface.h>
#include "../TimeSeriesCanvas.h"
#include "../TimeSeriesFontData.h"
#include <drx3D/Importers/MeshUtility/b3ImportMeshUtility.h>
#include <drx3D/OpenGLWindow/GLInstanceGraphicsShape.h>
#include "../TinyVRGui.h"
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>

class DynamicTexturedCubeDemo : public CommonExampleInterface
{
	CommonGraphicsApp* m_app;
	b3AlignedObjectArray<i32> m_movingInstances;

	TinyVRGui* m_tinyVrGUI;

	enum
	{
		numCubesX = 1,
		numCubesY = 1
	};

public:
	DynamicTexturedCubeDemo(CommonGraphicsApp* app)
		: m_app(app),
		  m_tinyVrGUI(0)
	{
		m_app->setUpAxis(2);

		{
			b3Vec3 extents = b3MakeVector3(100, 100, 100);
			extents[m_app->getUpAxis()] = 1;

			i32 xres = 20;
			i32 yres = 20;

			b3Vec4 color0 = b3MakeVector4(0.1, 0.1, 0.5, 1);
			b3Vec4 color1 = b3MakeVector4(0.6, 0.6, 0.6, 1);
			m_app->registerGrid(xres, yres, color0, color1);
		}

		ComboBoxParams comboParams;
		comboParams.m_comboboxId = 0;
		comboParams.m_numItems = 0;
		comboParams.m_startItem = 0;
		comboParams.m_callback = 0;     //MyComboBoxCallback;
		comboParams.m_userPointer = 0;  //this;

		m_tinyVrGUI = new TinyVRGui(comboParams, m_app->m_renderer);
		m_tinyVrGUI->init();

		m_app->m_renderer->writeTransforms();
	}
	virtual ~DynamicTexturedCubeDemo()
	{
		delete m_tinyVrGUI;
	}

	virtual void physicsDebugDraw(i32 debugDrawMode)
	{
	}
	virtual void initPhysics()
	{
	}
	virtual void exitPhysics()
	{
	}

	virtual void stepSimulation(float deltaTime)
	{
		static b3Transform tr = b3Transform::getIdentity();
		static b3Scalar t = 0.f;
		t += deltaTime;
		tr.setOrigin(b3MakeVector3(0., 0., 2.) + b3MakeVector3(0., 0., 0.02 * b3Sin(t)));

		m_tinyVrGUI->tick(deltaTime, tr);

		m_app->m_renderer->writeTransforms();
	}
	virtual void renderScene()
	{
		m_app->m_renderer->renderScene();
	}

	virtual void physicsDebugDraw()
	{
	}
	virtual bool mouseMoveCallback(float x, float y)
	{
		return false;
	}
	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y)
	{
		return false;
	}
	virtual bool keyboardCallback(i32 key, i32 state)
	{
		return false;
	}

	virtual void resetCamera()
	{
		float dist = 1.15;
		float pitch = -33.7;
		float yaw = 396;
		float targetPos[3] = {-0.5, 0.7, 1.45};
		if (m_app->m_renderer && m_app->m_renderer->getActiveCamera())
		{
			m_app->m_renderer->getActiveCamera()->setCameraDistance(dist);
			m_app->m_renderer->getActiveCamera()->setCameraPitch(pitch);
			m_app->m_renderer->getActiveCamera()->setCameraYaw(yaw);
			m_app->m_renderer->getActiveCamera()->setCameraTargetPosition(targetPos[0], targetPos[1], targetPos[2]);
		}
	}
};

class CommonExampleInterface* DynamicTexturedCubeDemoCreateFunc(struct CommonExampleOptions& options)
{
	return new DynamicTexturedCubeDemo(options.m_guiHelper->getAppInterface());
}
