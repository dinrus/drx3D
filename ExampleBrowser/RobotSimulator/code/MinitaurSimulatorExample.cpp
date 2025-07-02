#include "../MinitaurSimulatorExample.h"
#include "../MinitaurSetup.h"
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/b3Quat.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/SharedMemory/PhysicsServerSharedMemory.h>
#include <drx3D/SharedMemory/SharedMemoryPublic.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <drx3D/SharedMemory/PhysicsClientC_API.h>
#include <string>
#include "../RobotSimulatorClientAPI.h"
#include <drx3D/Common/b3Clock.h>

///quick demo showing the right-handed coordinate system and positive rotations around each axis
class MinitaurSimulatorExample : public CommonExampleInterface
{
	CommonGraphicsApp* m_app;
	GUIHelperInterface* m_guiHelper;
	RobotSimulatorClientAPI m_robotSim;
	
	
	i32 m_options;
	
	double m_time;
	
	Scalar m_gravityAccelerationZ;
	
	MinitaurSetup m_minitaur;
	i32 m_minitaurUid;
	
	

public:
	MinitaurSimulatorExample(GUIHelperInterface* helper, i32 options)
		: m_app(helper->getAppInterface()),
		  m_guiHelper(helper),
		  m_options(options),
		m_gravityAccelerationZ(-10),
		m_minitaurUid(-1)
	{
		m_app->setUpAxis(2);
	}
	virtual ~MinitaurSimulatorExample()
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
		//hide the perception camera views for rbd, depth and segmentation mask
		m_robotSim.configureDebugVisualizer(COV_ENABLE_RGB_BUFFER_PREVIEW, 0);
		m_robotSim.configureDebugVisualizer(COV_ENABLE_DEPTH_BUFFER_PREVIEW, 0);
		m_robotSim.configureDebugVisualizer(COV_ENABLE_SEGMENTATION_MARK_PREVIEW, 0);
		drx3DPrintf("robotSim connected = %d", connected);

		SliderParams slider("Gravity", &m_gravityAccelerationZ);
		slider.m_minVal = -10;
		slider.m_maxVal = 10;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
		
		//when in the debugger, don't crash when a command isn't processed immediately, give it 10 seconds
		m_robotSim.setTimeOut(10);
		
		m_robotSim.loadURDF("plane.urdf");
		
		m_minitaurUid = m_minitaur.setupMinitaur(&m_robotSim, Vec3(0, 0, .3));
	
		
		{
			RobotSimulatorLoadUrdfFileArgs args;
			args.m_startPosition.setVal(0, 0, 1);
			args.m_startOrientation.setEulerZYX(0, 0, 0);
			args.m_useMultiBody = true;
			m_robotSim.loadURDF("cube_small.urdf", args);
		}
	}
	
	virtual void exitPhysics()
	{
		m_robotSim.disconnect();
	}
	
	virtual void stepSimulation(float deltaTime)
	{
		m_robotSim.setGravity(Vec3(0, 0, m_gravityAccelerationZ));
		m_robotSim.stepSimulation();
		for (i32 i = 0; i < m_robotSim.getNumJoints(m_minitaurUid);i++)
		{
			b3JointSensorState state;
			m_robotSim.getJointState(this->m_minitaurUid, i, &state);
		}

		b3JointStates2 states;
		m_robotSim.getJointStates(m_minitaurUid, states);

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
		if (m_app->m_renderer && m_app->m_renderer->getActiveCamera())
		{
			m_app->m_renderer->getActiveCamera()->setCameraDistance(dist);
			m_app->m_renderer->getActiveCamera()->setCameraPitch(pitch);
			m_app->m_renderer->getActiveCamera()->setCameraYaw(yaw);
			m_app->m_renderer->getActiveCamera()->setCameraTargetPosition(targetPos[0], targetPos[1], targetPos[2]);
		}
	}

};
class CommonExampleInterface* MinitaurSimulatorExampleCreateFunc(struct CommonExampleOptions& options)
{
	return new MinitaurSimulatorExample(options.m_guiHelper, options.m_option);
}
