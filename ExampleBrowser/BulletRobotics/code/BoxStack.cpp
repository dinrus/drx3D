
#include "../BoxStack.h"

#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/SharedMemory/PhysicsServerSharedMemory.h>
#include <drx3D/SharedMemory/PhysicsClientC_API.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <drx3D/SharedMemory/SharedMemoryPublic.h>
#include <string>

#include "../../RobotSimulator/RobotSimulatorClientAPI.h"
#include <drx3D/Common/b3Clock.h>

class BoxStackExample : public CommonExampleInterface
{
	CommonGraphicsApp* m_app;
	GUIHelperInterface* m_guiHelper;
	RobotSimulatorClientAPI m_robotSim;
	i32 m_options;

public:
	BoxStackExample(GUIHelperInterface* helper, i32 options)
		: m_app(helper->getAppInterface()),
		  m_guiHelper(helper),
		  m_options(options)
	{
		m_app->setUpAxis(2);
	}

	virtual ~BoxStackExample()
	{
	}

	virtual void physicsDebugDraw(i32 debugDrawMode)
	{
		m_robotSim.debugDraw(debugDrawMode);
	}
	virtual void initPhysics()
	{
		i32 mode = eCONNECT_EXISTING_EXAMPLE_BROWSER;
		m_robotSim.setGuiHelper(m_guiHelper);
		bool connected = m_robotSim.connect(mode);

		drx3DPrintf("robotSim connected = %d", connected);

		m_robotSim.configureDebugVisualizer(COV_ENABLE_RGB_BUFFER_PREVIEW, 0);
		m_robotSim.configureDebugVisualizer(COV_ENABLE_DEPTH_BUFFER_PREVIEW, 0);
		m_robotSim.configureDebugVisualizer(COV_ENABLE_SEGMENTATION_MARK_PREVIEW, 0);

		RobotSimulatorLoadUrdfFileArgs args;
		RobotSimulatorChangeDynamicsArgs dynamicsArgs;
		i32 massRatio = 4;
		i32 mass = 1;
		for (i32 i = 0; i < 8; i++)
		{
			args.m_startPosition.setVal(0, 0, i * .06);
			i32 boxIdx = m_robotSim.loadURDF("cube_small.urdf", args);
			dynamicsArgs.m_mass = mass;
			m_robotSim.changeDynamics(boxIdx, -1, dynamicsArgs);
			mass *= massRatio;
		}

		m_robotSim.loadURDF("plane.urdf");
		m_robotSim.setGravity(Vec3(0, 0, -10));
	}

	virtual void exitPhysics()
	{
		m_robotSim.disconnect();
	}
	virtual void stepSimulation(float deltaTime)
	{
		m_robotSim.stepSimulation();
	}
	virtual void renderScene()
	{
		m_robotSim.renderScene();
	}

	virtual bool mouseMoveCallback(float x, float y)
	{
		return m_robotSim.mouseMoveCallback(x, y);
	}
	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y)
	{
		return m_robotSim.mouseButtonCallback(button, state, x, y);
	}
	virtual bool keyboardCallback(i32 key, i32 state)
	{
		return false;
	}

	virtual void resetCamera()
	{
		float dist = 1.5;
		float pitch = -10;
		float yaw = 18;
		float targetPos[3] = {-0.2, 0.8, 0.3};

		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);

		if (m_app->m_renderer && m_app->m_renderer->getActiveCamera())
		{
			m_app->m_renderer->getActiveCamera()->setCameraDistance(dist);
			m_app->m_renderer->getActiveCamera()->setCameraPitch(pitch);
			m_app->m_renderer->getActiveCamera()->setCameraYaw(yaw);
			m_app->m_renderer->getActiveCamera()->setCameraTargetPosition(targetPos[0], targetPos[1], targetPos[2]);
		}
	}
};

class CommonExampleInterface* BoxStackExampleCreateFunc(struct CommonExampleOptions& options)
{
	return new BoxStackExample(options.m_guiHelper, options.m_option);
}
