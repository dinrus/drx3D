
#include "../JointLimit.h"

#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/SharedMemory/PhysicsServerSharedMemory.h>
#include <drx3D/SharedMemory/PhysicsClientC_API.h>
#include <drx3D/SharedMemory/SharedMemoryPublic.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <string>

#include "../../RobotSimulator/RobotSimulatorClientAPI.h"
#include <drx3D/Common/b3Clock.h>

class JointLimit : public CommonExampleInterface
{
	GUIHelperInterface* m_guiHelper;
	RobotSimulatorClientAPI m_robotSim;
	i32 m_options;

public:
	JointLimit(GUIHelperInterface* helper, i32 options)
		: m_guiHelper(helper),
		  m_options(options)
	{
	}

	virtual ~JointLimit()
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

		RobotSimulatorSetPhysicsEngineParameters physicsArgs;
		physicsArgs.m_constraintSolverType = eConstraintSolverLCP_DANTZIG;

		physicsArgs.m_defaultGlobalCFM = 1e-6;

		m_robotSim.setNumSolverIterations(10);

		RobotSimulatorLoadUrdfFileArgs loadArgs;
		i32 humanoid = m_robotSim.loadURDF("test_joints_MB.urdf", loadArgs);

		RobotSimulatorChangeDynamicsArgs dynamicsArgs;
		dynamicsArgs.m_linearDamping = 0;
		dynamicsArgs.m_angularDamping = 0;
		m_robotSim.changeDynamics(humanoid, -1, dynamicsArgs);

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
		float dist = 3;
		float pitch = -10;
		float yaw = 18;
		float targetPos[3] = {0.6, 0.8, 0.3};

		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

class CommonExampleInterface* JointLimitCreateFunc(struct CommonExampleOptions& options)
{
	return new JointLimit(options.m_guiHelper, options.m_option);
}
