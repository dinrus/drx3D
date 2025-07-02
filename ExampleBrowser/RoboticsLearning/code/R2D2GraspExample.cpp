#include "../R2D2GraspExample.h"
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/b3Quat.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/SharedMemory/PhysicsServerSharedMemory.h>
#include <drx3D/SharedMemory/PhysicsClientC_API.h>
#include <string>
#include "../../RobotSimulator/RobotSimulatorClientAPI.h"
#include <drx3D/Common/b3Clock.h>

///quick demo showing the right-handed coordinate system and positive rotations around each axis
class R2D2GraspExample : public CommonExampleInterface
{
	CommonGraphicsApp* m_app;
	GUIHelperInterface* m_guiHelper;
	RobotSimulatorClientAPI m_robotSim;
	i32 m_options;
	i32 m_r2d2Index;

	b3AlignedObjectArray<i32> m_movingInstances;
	enum
	{
		numCubesX = 20,
		numCubesY = 20
	};

public:
	R2D2GraspExample(GUIHelperInterface* helper, i32 options)
		: m_app(helper->getAppInterface()),
		  m_guiHelper(helper),
		  m_options(options),
		  m_r2d2Index(-1)
	{
		m_app->setUpAxis(2);
	}
	virtual ~R2D2GraspExample()
	{
	}

	virtual void physicsDebugDraw(i32 debugDrawMode)
	{
	}
	virtual void initPhysics()
	{
		i32 mode = eCONNECT_EXISTING_EXAMPLE_BROWSER;
		m_robotSim.setGuiHelper(m_guiHelper);
		bool connected = m_robotSim.connect(mode);
		m_robotSim.configureDebugVisualizer(COV_ENABLE_RGB_BUFFER_PREVIEW, 0);
		m_robotSim.configureDebugVisualizer(COV_ENABLE_DEPTH_BUFFER_PREVIEW, 0);
		m_robotSim.configureDebugVisualizer(COV_ENABLE_SEGMENTATION_MARK_PREVIEW, 0);

		drx3DPrintf("robotSim connected = %d", connected);

		if ((m_options & eROBOTIC_LEARN_GRASP) != 0)
		{
			{
				RobotSimulatorLoadUrdfFileArgs args;
				args.m_startPosition.setVal(0, 0, .5);
				m_r2d2Index = m_robotSim.loadURDF("r2d2.urdf", args);

				if (m_r2d2Index >= 0)
				{
					i32 numJoints = m_robotSim.getNumJoints(m_r2d2Index);
					drx3DPrintf("numJoints = %d", numJoints);

					for (i32 i = 0; i < numJoints; i++)
					{
						b3JointInfo jointInfo;
						m_robotSim.getJointInfo(m_r2d2Index, i, &jointInfo);
						drx3DPrintf("joint[%d].m_jointName=%s", i, jointInfo.m_jointName);
					}
					i32 wheelJointIndices[4] = {2, 3, 6, 7};
					i32 wheelTargetVelocities[4] = {-10, -10, -10, -10};
					for (i32 i = 0; i < 4; i++)
					{
						RobotSimulatorJointMotorArgs controlArgs(CONTROL_MODE_VELOCITY);
						controlArgs.m_targetVelocity = wheelTargetVelocities[i];
						controlArgs.m_maxTorqueValue = 1e30;
						m_robotSim.setJointMotorControl(m_r2d2Index, wheelJointIndices[i], controlArgs);
					}
				}
			}
			{
				RobotSimulatorLoadFileResults results;
				m_robotSim.loadSDF("kiva_shelf/model.sdf", results);
			}
			{
				m_robotSim.loadURDF("plane.urdf");
			}

			m_robotSim.setGravity(Vec3(0, 0, -10));
		}

		if ((m_options & eROBOTIC_LEARN_COMPLIANT_CONTACT) != 0)
		{
			RobotSimulatorLoadUrdfFileArgs args;
			RobotSimulatorLoadFileResults results;
			{
				args.m_startPosition.setVal(0, 0, 2.5);
				args.m_startOrientation.setEulerZYX(0, 0.2, 0);
				m_r2d2Index = m_robotSim.loadURDF("cube_soft.urdf", args);
			}
			{
				args.m_startPosition.setVal(0, 2, 2.5);
				args.m_startOrientation.setEulerZYX(0, 0.2, 0);
				m_robotSim.loadURDF("cube_no_friction.urdf", args);
			}
			{
				args.m_startPosition.setVal(0, 0, 0);
				args.m_startOrientation.setEulerZYX(0, 0.2, 0);
				args.m_forceOverrideFixedBase = true;
				m_robotSim.loadURDF("plane.urdf", args);
			}

			m_robotSim.setGravity(Vec3(0, 0, -10));
		}

		if ((m_options & eROBOTIC_LEARN_ROLLING_FRICTION) != 0)
		{
			RobotSimulatorLoadUrdfFileArgs args;
			RobotSimulatorLoadFileResults results;
			{
				args.m_startPosition.setVal(0, 0, 2.5);
				args.m_startOrientation.setEulerZYX(0, 0, 0);
				args.m_useMultiBody = true;
				m_robotSim.loadURDF("sphere2_rolling_friction.urdf", args);
			}
			{
				args.m_startPosition.setVal(0, 2, 2.5);
				args.m_startOrientation.setEulerZYX(0, 0, 0);
				args.m_useMultiBody = true;
				m_robotSim.loadURDF("sphere2.urdf", args);
			}
			{
				args.m_startPosition.setVal(0, 0, 0);
				args.m_startOrientation.setEulerZYX(0, 0.2, 0);
				args.m_useMultiBody = true;
				args.m_forceOverrideFixedBase = true;
				m_robotSim.loadURDF("plane.urdf", args);
			}

			m_robotSim.setGravity(Vec3(0, 0, -10));
		}
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

		//m_app->m_renderer->renderScene();
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
		float dist = 3;
		float pitch = -30;
		float yaw = -75;
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

class CommonExampleInterface* R2D2GraspExampleCreateFunc(struct CommonExampleOptions& options)
{
	return new R2D2GraspExample(options.m_guiHelper, options.m_option);
}
