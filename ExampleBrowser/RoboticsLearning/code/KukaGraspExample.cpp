#include "../KukaGraspExample.h"
#include <drx3D/SharedMemory/IKTrajectoryHelper.h>

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
class KukaGraspExample : public CommonExampleInterface
{
	CommonGraphicsApp* m_app;
	GUIHelperInterface* m_guiHelper;
	RobotSimulatorClientAPI m_robotSim;

	i32 m_kukaIndex;

	IKTrajectoryHelper m_ikHelper;
	i32 m_targetSphereInstance;
	b3Vec3 m_targetPos;
	b3Vec3 m_worldPos;
	b3Vec4 m_targetOri;
	b3Vec4 m_worldOri;
	double m_time;
	//	i32 m_options;

	b3AlignedObjectArray<i32> m_movingInstances;
	enum
	{
		numCubesX = 20,
		numCubesY = 20
	};

public:
	KukaGraspExample(GUIHelperInterface* helper, i32 /* options */)
		: m_app(helper->getAppInterface()),
		  m_guiHelper(helper),
		  //	m_options(options),
		  m_kukaIndex(-1),
		  m_time(0)
	{
		m_targetPos.setVal(0.5, 0, 1);
		m_worldPos.setVal(0, 0, 0);
		m_app->setUpAxis(2);
	}
	virtual ~KukaGraspExample()
	{
	}

	virtual void initPhysics()
	{
		///create some graphics proxy for the tracking target
		///the endeffector tries to track it using Inverse Kinematics
		{
			i32 sphereId = m_app->registerGraphicsUnitSphereShape(SPHERE_LOD_MEDIUM);
			b3Quat orn(0, 0, 0, 1);
			b3Vec4 color = b3MakeVector4(1., 0.3, 0.3, 1);
			b3Vec3 scaling = b3MakeVector3(.02, .02, .02);
			m_targetSphereInstance = m_app->m_renderer->registerGraphicsInstance(sphereId, m_targetPos, orn, color, scaling);
		}
		m_app->m_renderer->writeTransforms();

		i32 mode = eCONNECT_EXISTING_EXAMPLE_BROWSER;
		m_robotSim.setGuiHelper(m_guiHelper);
		bool connected = m_robotSim.connect(mode);
		m_robotSim.configureDebugVisualizer(COV_ENABLE_RGB_BUFFER_PREVIEW, 0);
		m_robotSim.configureDebugVisualizer(COV_ENABLE_DEPTH_BUFFER_PREVIEW, 0);
		m_robotSim.configureDebugVisualizer(COV_ENABLE_SEGMENTATION_MARK_PREVIEW, 0);

		//			0;//m_robotSim.connect(m_guiHelper);
		drx3DPrintf("robotSim connected = %d", connected);

		{
			m_kukaIndex = m_robotSim.loadURDF("kuka_iiwa/model.urdf");
			if (m_kukaIndex >= 0)
			{
				i32 numJoints = m_robotSim.getNumJoints(m_kukaIndex);
				drx3DPrintf("numJoints = %d", numJoints);

				for (i32 i = 0; i < numJoints; i++)
				{
					b3JointInfo jointInfo;
					m_robotSim.getJointInfo(m_kukaIndex, i, &jointInfo);
					drx3DPrintf("joint[%d].m_jointName=%s", i, jointInfo.m_jointName);
				}
				/*
                i32 wheelJointIndices[4]={2,3,6,7};
                i32 wheelTargetVelocities[4]={-10,-10,-10,-10};
                for (i32 i=0;i<4;i++)
                {
                    b3JointMotorArgs controlArgs(CONTROL_MODE_VELOCITY);
                    controlArgs.m_targetVelocity = wheelTargetVelocities[i];
                    controlArgs.m_maxTorqueValue = 1e30;
                    m_robotSim.setJointMotorControl(m_kukaIndex,wheelJointIndices[i],controlArgs);
                }
                 */
			}

			{
				m_robotSim.loadURDF("plane.urdf");
				m_robotSim.setGravity(Vec3(0, 0, 0));
			}
		}
	}
	virtual void exitPhysics()
	{
		m_robotSim.disconnect();
	}
	virtual void stepSimulation(float deltaTime)
	{
		float dt = deltaTime;
		Clamp(dt, 0.0001f, 0.01f);

		m_time += dt;
		m_targetPos.setVal(0.4 - 0.4 * b3Cos(m_time), 0, 0.8 + 0.4 * b3Cos(m_time));
		m_targetOri.setVal(0, 1.0, 0, 0);
		m_targetPos.setVal(0.2 * b3Cos(m_time), 0.2 * b3Sin(m_time), 1.1);

		i32 numJoints = m_robotSim.getNumJoints(m_kukaIndex);

		if (numJoints == 7)
		{
			double q_current[7] = {0, 0, 0, 0, 0, 0, 0};

			b3JointStates2 jointStates;

			if (m_robotSim.getJointStates(m_kukaIndex, jointStates))
			{
				//skip the base positions (7 values)
				drx3DAssert(7 + numJoints == jointStates.m_numDegreeOfFreedomQ);
				for (i32 i = 0; i < numJoints; i++)
				{
					q_current[i] = jointStates.m_actualStateQ[i + 7];
				}
			}
			// compute body position and orientation
			b3LinkState linkState;
			bool computeVelocity = true;
			bool computeForwardKinematics = true;
			m_robotSim.getLinkState(0, 6, computeVelocity, computeForwardKinematics, &linkState);

			m_worldPos.setVal(linkState.m_worldLinkFramePosition[0], linkState.m_worldLinkFramePosition[1], linkState.m_worldLinkFramePosition[2]);
			m_worldOri.setVal(linkState.m_worldLinkFrameOrientation[0], linkState.m_worldLinkFrameOrientation[1], linkState.m_worldLinkFrameOrientation[2], linkState.m_worldLinkFrameOrientation[3]);

			b3Vec3DoubleData targetPosDataOut;
			m_targetPos.serializeDouble(targetPosDataOut);
			b3Vec3DoubleData worldPosDataOut;
			m_worldPos.serializeDouble(worldPosDataOut);
			b3Vec3DoubleData targetOriDataOut;
			m_targetOri.serializeDouble(targetOriDataOut);
			b3Vec3DoubleData worldOriDataOut;
			m_worldOri.serializeDouble(worldOriDataOut);

			RobotSimulatorInverseKinematicArgs ikargs;
			RobotSimulatorInverseKinematicsResults ikresults;

			ikargs.m_bodyUniqueId = m_kukaIndex;
			//			ikargs.m_currentJointPositions = q_current;
			//			ikargs.m_numPositions = 7;
			ikargs.m_endEffectorTargetPosition[0] = targetPosDataOut.m_floats[0];
			ikargs.m_endEffectorTargetPosition[1] = targetPosDataOut.m_floats[1];
			ikargs.m_endEffectorTargetPosition[2] = targetPosDataOut.m_floats[2];

			//ikargs.m_flags |= D3_HAS_IK_TARGET_ORIENTATION;
			ikargs.m_flags |= D3_HAS_JOINT_DAMPING;

			ikargs.m_endEffectorTargetOrientation[0] = targetOriDataOut.m_floats[0];
			ikargs.m_endEffectorTargetOrientation[1] = targetOriDataOut.m_floats[1];
			ikargs.m_endEffectorTargetOrientation[2] = targetOriDataOut.m_floats[2];
			ikargs.m_endEffectorTargetOrientation[3] = targetOriDataOut.m_floats[3];
			ikargs.m_endEffectorLinkIndex = 6;

			// Settings based on default KUKA arm setting
			ikargs.m_lowerLimits.resize(numJoints);
			ikargs.m_upperLimits.resize(numJoints);
			ikargs.m_jointRanges.resize(numJoints);
			ikargs.m_restPoses.resize(numJoints);
			ikargs.m_jointDamping.resize(numJoints, 0.5);
			ikargs.m_lowerLimits[0] = -2.32;
			ikargs.m_lowerLimits[1] = -1.6;
			ikargs.m_lowerLimits[2] = -2.32;
			ikargs.m_lowerLimits[3] = -1.6;
			ikargs.m_lowerLimits[4] = -2.32;
			ikargs.m_lowerLimits[5] = -1.6;
			ikargs.m_lowerLimits[6] = -2.4;
			ikargs.m_upperLimits[0] = 2.32;
			ikargs.m_upperLimits[1] = 1.6;
			ikargs.m_upperLimits[2] = 2.32;
			ikargs.m_upperLimits[3] = 1.6;
			ikargs.m_upperLimits[4] = 2.32;
			ikargs.m_upperLimits[5] = 1.6;
			ikargs.m_upperLimits[6] = 2.4;
			ikargs.m_jointRanges[0] = 5.8;
			ikargs.m_jointRanges[1] = 4;
			ikargs.m_jointRanges[2] = 5.8;
			ikargs.m_jointRanges[3] = 4;
			ikargs.m_jointRanges[4] = 5.8;
			ikargs.m_jointRanges[5] = 4;
			ikargs.m_jointRanges[6] = 6;
			ikargs.m_restPoses[0] = 0;
			ikargs.m_restPoses[1] = 0;
			ikargs.m_restPoses[2] = 0;
			ikargs.m_restPoses[3] = SIMD_HALF_PI;
			ikargs.m_restPoses[4] = 0;
			ikargs.m_restPoses[5] = -SIMD_HALF_PI * 0.66;
			ikargs.m_restPoses[6] = 0;
			ikargs.m_jointDamping[0] = 10.0;
			ikargs.m_numDegreeOfFreedom = numJoints;

			if (m_robotSim.calculateInverseKinematics(ikargs, ikresults))
			{
				//copy the IK result to the desired state of the motor/actuator
				for (i32 i = 0; i < numJoints; i++)
				{
					RobotSimulatorJointMotorArgs t(CONTROL_MODE_POSITION_VELOCITY_PD);
					t.m_targetPosition = ikresults.m_calculatedJointPositions[i];
					t.m_maxTorqueValue = 100.0;
					t.m_kp = 1.0;
					t.m_targetVelocity = 0;
					t.m_kd = 1.0;
					m_robotSim.setJointMotorControl(m_kukaIndex, i, t);
				}
			}
		}

		m_robotSim.stepSimulation();
	}
	virtual void renderScene()
	{
		m_robotSim.renderScene();

		b3Quat orn(0, 0, 0, 1);

		m_app->m_renderer->writeSingleInstanceTransformToCPU(m_targetPos, orn, m_targetSphereInstance);
		m_app->m_renderer->writeTransforms();

		//draw the end-effector target sphere

		//m_app->m_renderer->renderScene();
	}

	virtual void physicsDebugDraw(i32 debugDrawMode)
	{
		m_robotSim.debugDraw(debugDrawMode);
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
		float yaw = 0;
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

class CommonExampleInterface* KukaGraspExampleCreateFunc(struct CommonExampleOptions& options)
{
	return new KukaGraspExample(options.m_guiHelper, options.m_option);
}
