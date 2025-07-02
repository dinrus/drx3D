#include "../PhysicsClientExample.h"

#include <drx3D/Common/Interfaces/CommonMultiBodyBase.h>
#include <drx3D/Common/Interfaces/Common2dCanvasInterface.h>
#include <drx3D/SharedMemory/SharedMemoryCommon.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <drx3D/SharedMemory/PhysicsServerCommandProcessor.h>
#include <drx3D/SharedMemory/PhysicsClientC_API.h>
#include <drx3D/SharedMemory/PhysicsClient.h>
//#include "SharedMemoryCommands.h"
#include <drx3D/SharedMemory/PhysicsLoopBackC_API.h>
#include <drx3D/SharedMemory/PhysicsDirectC_API.h>
#include <drx3D/SharedMemory/PhysicsClientC_API.h>
#include <drx3D/SharedMemory/PhysicsServerSharedMemory.h>
#include <stdio.h>
#ifdef _WIN32
#define safe_printf _snprintf
#else
#define safe_printf snprintf
#endif
struct MyMotorInfo2
{
	Scalar m_velTarget;
	Scalar m_maxForce;
	Scalar m_posTarget;
	i32 m_uIndex;
	i32 m_qIndex;
};

static i32 camVisualizerWidth = 228;   //1024/3;
static i32 camVisualizerHeight = 192;  //768/3;

enum CustomCommands
{
	CMD_CUSTOM_SET_REALTIME_SIMULATION = CMD_MAX_CLIENT_COMMANDS + 1,
	CMD_CUSTOM_SET_GRAVITY
};

#define MAX_NUM_MOTORS 128

class PhysicsClientExample : public SharedMemoryCommon
{
protected:
	b3PhysicsClientHandle m_physicsClientHandle;

	//this m_physicsServer is only used when option eCLIENTEXAMPLE_SERVER is enabled
	PhysicsServerSharedMemory m_physicsServer;

	bool m_wantsTermination;
	AlignedObjectArray<i32> m_userCommandRequests;
	AlignedObjectArray<i32> m_bodyUniqueIds;

	i32 m_sharedMemoryKey;
	i32 m_selectedBody;
	i32 m_prevSelectedBody;
	struct Common2dCanvasInterface* m_canvas;
	i32 m_canvasRGBIndex;
	i32 m_canvasDepthIndex;
	i32 m_canvasSegMaskIndex;

	Scalar m_lightPos[3];
	Scalar m_specularCoeff;

	void createButton(tukk name, i32 id, bool isTrigger);

	void createButtons();

	//@todo, add accessor methods
	// MyMotorInfo2 m_motorTargetVelocities[MAX_NUM_MOTORS];
	MyMotorInfo2 m_motorTargetPositions[MAX_NUM_MOTORS];
	i32 m_numMotors;
	i32 m_options;
	bool m_isOptionalServerConnected;

public:
	PhysicsClientExample(GUIHelperInterface* helper, i32 options);
	virtual ~PhysicsClientExample();

	virtual void initPhysics();
	void selectComboBox(i32 comboIndex, tukk name)
	{
		if (m_guiHelper && m_guiHelper->getParameterInterface())
		{
			i32 itemIndex = i32(atoi(name));
			i32 bodyIndex = m_bodyUniqueIds[itemIndex];
			if (m_selectedBody != bodyIndex)
			{
				m_selectedBody = bodyIndex;
			}
		}
	}
	virtual void stepSimulation(float deltaTime);

	virtual void resetCamera()
	{
		float dist = 3.45;
		float pitch = -16.2;
		float yaw = 287;
		float targetPos[3] = {2.05, 0.02, 0.53};  //-3,2.8,-2.5};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}

	virtual bool wantsTermination()
	{
		return m_wantsTermination;
	}

	virtual bool isConnected()
	{
		return (m_physicsClientHandle != 0);
	}

	void enqueueCommand(i32 commandId);

	void prepareAndSubmitCommand(i32 commandId);

	virtual void exitPhysics(){};
	virtual void renderScene()
	{
		if (m_options == eCLIENTEXAMPLE_SERVER)
		{
			i32 renderFlags = 0;
			m_physicsServer.renderScene(renderFlags);
		}

		b3DebugLines debugLines;
		b3GetDebugLines(m_physicsClientHandle, &debugLines);
		i32 numLines = debugLines.m_numDebugLines;

		i32 lineWidth = 1;

		if (1)
		{
			AlignedObjectArray<Vec3FloatData> points;
			points.resize(numLines * 2);
			AlignedObjectArray<u32> indices;
			indices.resize(numLines * 2);

			for (i32 i = 0; i < numLines; i++)
			{
				points[i * 2].m_floats[0] = debugLines.m_linesFrom[i * 3 + 0];
				points[i * 2].m_floats[1] = debugLines.m_linesFrom[i * 3 + 1];
				points[i * 2].m_floats[2] = debugLines.m_linesFrom[i * 3 + 2];
				points[i * 2 + 1].m_floats[0] = debugLines.m_linesTo[i * 3 + 0];
				points[i * 2 + 1].m_floats[1] = debugLines.m_linesTo[i * 3 + 1];
				points[i * 2 + 1].m_floats[2] = debugLines.m_linesTo[i * 3 + 2];
				indices[i * 2] = i * 2;
				indices[i * 2 + 1] = i * 2 + 1;
			}

			float color[4] = {0.2, 0.2, 1, 1};

			if (points.size() && indices.size())
			{
				m_guiHelper->getRenderInterface()->drawLines(&points[0].m_floats[0], color, points.size(), sizeof(Vec3FloatData), &indices[0], indices.size(), lineWidth);
			}
		}
		else
		{
			for (i32 i = 0; i < numLines; i++)
			{
				m_guiHelper->getRenderInterface()->drawLine(debugLines.m_linesFrom, debugLines.m_linesTo, debugLines.m_linesColor, lineWidth);
			}
		}
	}

	void prepareControlCommand(b3SharedMemoryCommandHandle commandHandle)
	{
		for (i32 i = 0; i < m_numMotors; i++)
		{
			Scalar targetPos = m_motorTargetPositions[i].m_posTarget;
			i32 qIndex = m_motorTargetPositions[i].m_qIndex;
			i32 uIndex = m_motorTargetPositions[i].m_uIndex;
			static i32 serial = 0;
			serial++;
			//  drx3DPrintf("# motors = %d, cmd[%d] qIndex = %d, uIndex = %d, targetPos = %f", m_numMotors, serial, qIndex,uIndex,targetPos);

			b3JointControlSetDesiredPosition(commandHandle, qIndex, targetPos);
			b3JointControlSetDesiredVelocity(commandHandle, uIndex, 0);
			b3JointControlSetKp(commandHandle, qIndex, 0.2);
			b3JointControlSetKd(commandHandle, uIndex, 1.);

			b3JointControlSetMaximumForce(commandHandle, uIndex, 5000);
		}
	}
	virtual void physicsDebugDraw(i32 debugFlags)
	{
		if (m_options == eCLIENTEXAMPLE_SERVER)
		{
			m_physicsServer.physicsDebugDraw(debugFlags);
		}
	}
	virtual bool mouseMoveCallback(float x, float y) { return false; };
	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y) { return false; }
	virtual bool keyboardCallback(i32 key, i32 state) { return false; }

	virtual void setSharedMemoryKey(i32 key)
	{
		m_sharedMemoryKey = key;
	}
};

void MyComboBoxCallback(i32 combobox, tukk item, uk userPointer)
{
	//drx3DPrintf("Item selected %s", item);

	PhysicsClientExample* cl = (PhysicsClientExample*)userPointer;
	drx3DAssert(cl);
	if (cl)
	{
		cl->selectComboBox(combobox, item);
	}
}

void MyCallback(i32 buttonId, bool buttonState, uk userPtr)
{
	PhysicsClientExample* cl = (PhysicsClientExample*)userPtr;
	drx3DAssert(cl);

	if (cl && buttonState)
	{
		cl->enqueueCommand(buttonId);
	}
}

void PhysicsClientExample::enqueueCommand(i32 commandId)
{
	m_userCommandRequests.push_back(commandId);
}

void PhysicsClientExample::prepareAndSubmitCommand(i32 commandId)
{
	switch (commandId)
	{
		case CMD_LOAD_URDF:
		{
			b3SharedMemoryCommandHandle commandHandle = b3LoadUrdfCommandInit(m_physicsClientHandle, "kuka_iiwa/model.urdf");
			//setting the initial position, orientation and other arguments are optional
			double startPosX = 0;
			static double startPosY = 0;
			double startPosZ = 0;
			b3LoadUrdfCommandSetStartPosition(commandHandle, startPosX, startPosY, startPosZ);
			startPosY += 2.f;
			//            ret = b3LoadUrdfCommandSetUseFixedBase(commandHandle, 1);
			b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			break;
		}

		case CMD_LOAD_SDF:
		{
#ifdef DRX3D_DEBUG
			b3SharedMemoryCommandHandle commandHandle = b3LoadSdfCommandInit(m_physicsClientHandle, "two_cubes.sdf");
#else
			b3SharedMemoryCommandHandle commandHandle = b3LoadSdfCommandInit(m_physicsClientHandle, "kitchens/1.sdf");  //two_cubes.sdf");//kitchens/1.sdf");//kuka_iiwa/model.sdf");
#endif
			b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			break;
		}
		case CMD_REQUEST_CAMERA_IMAGE_DATA:
		{
			///request an image from a simulated camera, using a software renderer.
			b3SharedMemoryCommandHandle commandHandle = b3InitRequestCameraImage(m_physicsClientHandle);
			//b3RequestCameraImageSelectRenderer(commandHandle,ER_DRX3D_HARDWARE_OPENGL);

			float viewMatrix[16];
			float projectionMatrix[16];
			m_guiHelper->getRenderInterface()->getActiveCamera()->getCameraProjectionMatrix(projectionMatrix);
			m_guiHelper->getRenderInterface()->getActiveCamera()->getCameraViewMatrix(viewMatrix);

			b3RequestCameraImageSetCameraMatrices(commandHandle, viewMatrix, projectionMatrix);
			b3RequestCameraImageSetPixelResolution(commandHandle, camVisualizerWidth, camVisualizerHeight);
			float lightPos[3];
			lightPos[0] = m_lightPos[0];
			lightPos[1] = m_lightPos[1];
			lightPos[2] = m_lightPos[2];
			b3RequestCameraImageSetLightDirection(commandHandle, lightPos);
			b3RequestCameraImageSetLightSpecularCoeff(commandHandle, m_specularCoeff);
			b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			break;
		}
		case CMD_CREATE_BOX_COLLISION_SHAPE:
		{
			b3SharedMemoryCommandHandle commandHandle = b3CreateBoxShapeCommandInit(m_physicsClientHandle);
			b3CreateBoxCommandSetStartPosition(commandHandle, 0, 0, -1.5);
			b3CreateBoxCommandSetColorRGBA(commandHandle, 0, 0, 1, 1);
			b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			break;
		}
		case CMD_CREATE_RIGID_BODY:
		{
			b3SharedMemoryCommandHandle commandHandle = b3CreateBoxShapeCommandInit(m_physicsClientHandle);
			b3CreateBoxCommandSetStartPosition(commandHandle, 0, 0, 0);
			b3CreateBoxCommandSetMass(commandHandle, 1);
			b3CreateBoxCommandSetCollisionShapeType(commandHandle, COLLISION_SHAPE_TYPE_CYLINDER_Y);
			b3CreateBoxCommandSetColorRGBA(commandHandle, 1, 1, 0, 1);
			double radius = 0.2;
			double halfHeight = 0.5;
			b3CreateBoxCommandSetHalfExtents(commandHandle, radius, halfHeight, radius);
			b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			break;
		}

		case CMD_REQUEST_ACTUAL_STATE:
		{
			if (m_selectedBody >= 0)
			{
				b3SharedMemoryCommandHandle commandHandle = b3RequestActualStateCommandInit(m_physicsClientHandle, m_selectedBody);
				b3SharedMemoryStatusHandle statusHandle = b3SubmitClientCommandAndWaitStatus(m_physicsClientHandle, commandHandle);
				b3SubmitClientCommand(m_physicsClientHandle, commandHandle);

				i32 numJoints = b3GetNumJoints(m_physicsClientHandle, m_selectedBody);
				for (i32 i = 0; i < numJoints; ++i)
				{
					struct b3JointSensorState sensorState;
					b3GetJointState(m_physicsClientHandle, statusHandle, i, &sensorState);
					//drx3DPrintf("Joint %d: %f", i, sensorState.m_jointMotorTorque);
				}
			}

			break;
		};

		case CMD_INIT_POSE:
		{
			if (m_selectedBody >= 0)
			{
				b3SharedMemoryCommandHandle commandHandle = b3CreatePoseCommandInit(m_physicsClientHandle, m_selectedBody);
				static i32 toggle = 0;
				double pos[3] = {0, 0, 0};
				pos[toggle] = 2;
				toggle++;
				if (toggle > 2)
					toggle = 0;

				Quat orn;
				orn.setVal(0, 0, 0, 1);

				switch (toggle)
				{
					case 0:
						orn = Quat(Vec3(1, 0, 0), SIMD_HALF_PI);
						break;
					case 1:
						orn = Quat(Vec3(0, 1, 0), SIMD_HALF_PI);
						break;
					case 2:
						orn = Quat(Vec3(0, 0, 1), SIMD_HALF_PI);
						break;

					default:
						orn.setVal(0, 0, 0, 1);
				};

				b3CreatePoseCommandSetBaseOrientation(commandHandle, orn[0], orn[1], orn[2], orn[3]);
				b3CreatePoseCommandSetBasePosition(commandHandle, pos[0], pos[1], pos[2]);
				i32 numJoints = b3GetNumJoints(m_physicsClientHandle, m_selectedBody);
				static double jointPos = SIMD_PI / 2.f;

				for (i32 i = 0; i < numJoints; i++)
				{
					b3JointInfo info;
					b3GetJointInfo(m_physicsClientHandle, m_selectedBody, i, &info);
					if ((info.m_jointType == 0) || (info.m_jointType == 1))  //revolute or prismatic
					{
						b3CreatePoseCommandSetJointPosition(m_physicsClientHandle, commandHandle, i, jointPos);
					}
				}
				jointPos += SIMD_PI / 8.0;
				b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			}
			break;
		}
		case CMD_STEP_FORWARD_SIMULATION:
		{
			b3SharedMemoryCommandHandle commandHandle = b3InitStepSimulationCommand(m_physicsClientHandle);
			b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			break;
		}

		case CMD_REQUEST_DEBUG_LINES:
		{
			b3SharedMemoryCommandHandle commandHandle = b3InitRequestDebugLinesCommand(m_physicsClientHandle, IDebugDraw::DBG_DrawWireframe);
			b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			break;
		}
		case CMD_SEND_DESIRED_STATE:
		{
			if (m_selectedBody >= 0)
			{
				// b3SharedMemoryCommandHandle command = b3JointControlCommandInit( m_physicsClientHandle, m_selectedBody, CONTROL_MODE_VELOCITY);
				b3SharedMemoryCommandHandle command = b3JointControlCommandInit2(m_physicsClientHandle, m_selectedBody, CONTROL_MODE_POSITION_VELOCITY_PD);
				//  drx3DPrintf("prepare control command for body %d", m_selectedBody);

				prepareControlCommand(command);

				b3SubmitClientCommand(m_physicsClientHandle, command);
			}
			break;
		}
		case CMD_RESET_SIMULATION:
		{
			b3SharedMemoryCommandHandle commandHandle = b3InitResetSimulationCommand(m_physicsClientHandle);
			b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			break;
		}
		case CMD_SEND_DRX3D_DATA_STREAM:
		{
#if 0

            //this worked, but needs C-API and a streaming options, similar to debug lines
            command.m_type = buttonId;
            cl->enqueueCommand(command);
#endif

			break;
		}
		case CMD_CUSTOM_SET_GRAVITY:
		{
			b3SharedMemoryCommandHandle commandHandle = b3InitPhysicsParamCommand(m_physicsClientHandle);
			b3PhysicsParamSetGravity(commandHandle, 0.0, 0.0, -9.8);
			b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			break;
		}

		case CMD_CUSTOM_SET_REALTIME_SIMULATION:
		{
			b3SharedMemoryCommandHandle commandHandle = b3InitPhysicsParamCommand(m_physicsClientHandle);
			b3PhysicsParamSetRealTimeSimulation(commandHandle, 1);
			b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			break;
		}

		case CMD_CALCULATE_INVERSE_DYNAMICS:
		{
			if (m_selectedBody >= 0)
			{
				AlignedObjectArray<double> jointPositionsQ;
				AlignedObjectArray<double> jointVelocitiesQdot;
				AlignedObjectArray<double> jointAccelerations;
				i32 numJoints = b3GetNumJoints(m_physicsClientHandle, m_selectedBody);
				if (numJoints)
				{
					drx3DPrintf("Compute inverse dynamics for joint accelerations:");
					jointPositionsQ.resize(numJoints);
					jointVelocitiesQdot.resize(numJoints);
					jointAccelerations.resize(numJoints);
					for (i32 i = 0; i < numJoints; i++)
					{
						jointAccelerations[i] = 100;
						drx3DPrintf("Desired joint acceleration[%d]=%f", i, jointAccelerations[i]);
					}
					b3SharedMemoryCommandHandle commandHandle = b3Calculatedrx3d_inverseCommandInit(m_physicsClientHandle,
																									  m_selectedBody, &jointPositionsQ[0], &jointVelocitiesQdot[0], &jointAccelerations[0]);
					b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
				}
			}
			break;
		}
		case CMD_REQUEST_CONTACT_POINT_INFORMATION:
		{
			b3SharedMemoryCommandHandle commandHandle = b3InitRequestContactPointInformation(m_physicsClientHandle);
			b3SetContactFilterBodyA(commandHandle, 0);
			b3SetContactFilterBodyB(commandHandle, 1);
			b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			break;
		}
		case CMD_SAVE_WORLD:
		{
			b3SharedMemoryCommandHandle commandHandle = b3SaveWorldCommandInit(m_physicsClientHandle, "saveWorld.py");
			b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			break;
		}
		case CMD_REQUEST_VISUAL_SHAPE_INFO:
		{
			if (m_selectedBody >= 0)
			{
				//request visual shape information
				b3SharedMemoryCommandHandle commandHandle = b3InitRequestVisualShapeInformation(m_physicsClientHandle, m_selectedBody);
				b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			}
			break;
		}
		case CMD_SET_SHADOW:
		{
			b3SharedMemoryCommandHandle commandHandle = b3InitRequestCameraImage(m_physicsClientHandle);
			float viewMatrix[16];
			float projectionMatrix[16];
			m_guiHelper->getRenderInterface()->getActiveCamera()->getCameraProjectionMatrix(projectionMatrix);
			m_guiHelper->getRenderInterface()->getActiveCamera()->getCameraViewMatrix(viewMatrix);

			b3RequestCameraImageSetCameraMatrices(commandHandle, viewMatrix, projectionMatrix);
			b3RequestCameraImageSetPixelResolution(commandHandle, camVisualizerWidth, camVisualizerHeight);
			bool hasShadow = true;
			b3RequestCameraImageSetShadow(commandHandle, hasShadow);
			b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			break;
		}
		case CMD_UPDATE_VISUAL_SHAPE:
		{
			i32 objectUniqueId = 0;
			i32 linkIndex = -1;
			i32 shapeIndex = -1;
			i32 textureIndex = -2;
			double rgbaColor[4] = {0.0, 1.0, 0.0, 1.0};
			b3SharedMemoryCommandHandle commandHandle = b3InitUpdateVisualShape2(m_physicsClientHandle, objectUniqueId, linkIndex, shapeIndex);
			b3UpdateVisualShapeRGBAColor(commandHandle, rgbaColor);
			b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
			break;
		}

		default:
		{
			drx3DError("Unknown buttonId");
			Assert(0);
		}
	};
}

struct Bullet2CommandProcessorCreation3 : public CommandProcessorCreationInterface
{
	virtual class CommandProcessorInterface* createCommandProcessor()
	{
		PhysicsServerCommandProcessor* proc = new PhysicsServerCommandProcessor;
		return proc;
	}

	virtual void deleteCommandProcessor(CommandProcessorInterface* proc)
	{
		delete proc;
	}
};

static Bullet2CommandProcessorCreation3 sB2PC2;

PhysicsClientExample::PhysicsClientExample(GUIHelperInterface* helper, i32 options)
	: SharedMemoryCommon(helper),
	  m_physicsClientHandle(0),
	  m_physicsServer(&sB2PC2, 0, 0),
	  m_wantsTermination(false),
	  m_sharedMemoryKey(SHARED_MEMORY_KEY),
	  m_selectedBody(-1),
	  m_prevSelectedBody(-1),
	  m_canvas(0),
	  m_canvasRGBIndex(-1),
	  m_canvasDepthIndex(-1),
	  m_canvasSegMaskIndex(-1),
	  m_specularCoeff(1.0),
	  m_numMotors(0),
	  m_options(options),
	  m_isOptionalServerConnected(false)

{
	drx3DPrintf("Started PhysicsClientExample\n");
}

PhysicsClientExample::~PhysicsClientExample()
{
	if (m_physicsClientHandle)
	{
		b3ProcessServerStatus(m_physicsClientHandle);
		b3DisconnectSharedMemory(m_physicsClientHandle);
	}

	if (m_options == eCLIENTEXAMPLE_SERVER)
	{
		bool deInitializeSharedMemory = true;
		m_physicsServer.disconnectSharedMemory(deInitializeSharedMemory);
	}

	if (m_canvas)
	{
		if (m_canvasRGBIndex >= 0)
			m_canvas->destroyCanvas(m_canvasRGBIndex);
		if (m_canvasDepthIndex >= 0)
			m_canvas->destroyCanvas(m_canvasDepthIndex);
		if (m_canvasSegMaskIndex >= 0)
			m_canvas->destroyCanvas(m_canvasSegMaskIndex);
	}

	drx3DPrintf("~PhysicsClientExample\n");
}

void PhysicsClientExample::createButton(tukk name, i32 buttonId, bool isTrigger)
{
	ButtonParams button(name, buttonId, isTrigger);
	button.m_callback = MyCallback;
	button.m_userPointer = this;
	m_guiHelper->getParameterInterface()->registerButtonParameter(button);
}

void PhysicsClientExample::createButtons()
{
	bool isTrigger = false;

	if (m_guiHelper && m_guiHelper->getParameterInterface())
	{
		m_guiHelper->getParameterInterface()->removeAllParameters();

		createButton("Load URDF", CMD_LOAD_URDF, isTrigger);
		createButton("Load SDF", CMD_LOAD_SDF, isTrigger);
		createButton("Save World", CMD_SAVE_WORLD, isTrigger);
		createButton("Set Shadow", CMD_SET_SHADOW, isTrigger);
		createButton("Update Visual Shape", CMD_UPDATE_VISUAL_SHAPE, isTrigger);
		createButton("Get Camera Image", CMD_REQUEST_CAMERA_IMAGE_DATA, isTrigger);
		createButton("Step Sim", CMD_STEP_FORWARD_SIMULATION, isTrigger);
		createButton("Realtime Sim", CMD_CUSTOM_SET_REALTIME_SIMULATION, isTrigger);
		createButton("Get Visual Shape Info", CMD_REQUEST_VISUAL_SHAPE_INFO, isTrigger);
		createButton("Send drx3D Stream", CMD_SEND_DRX3D_DATA_STREAM, isTrigger);
		if (m_options != eCLIENTEXAMPLE_SERVER)
		{
			createButton("Get State", CMD_REQUEST_ACTUAL_STATE, isTrigger);
		}
		createButton("Send Desired State", CMD_SEND_DESIRED_STATE, isTrigger);
		createButton("Create Box Collider", CMD_CREATE_BOX_COLLISION_SHAPE, isTrigger);
		createButton("Create Cylinder Body", CMD_CREATE_RIGID_BODY, isTrigger);
		createButton("Reset Simulation", CMD_RESET_SIMULATION, isTrigger);
		createButton("Initialize Pose", CMD_INIT_POSE, isTrigger);
		createButton("Set gravity", CMD_CUSTOM_SET_GRAVITY, isTrigger);
		createButton("Compute Inverse Dynamics", CMD_CALCULATE_INVERSE_DYNAMICS, isTrigger);
		createButton("Get Contact Point Info", CMD_REQUEST_CONTACT_POINT_INFORMATION, isTrigger);

		if (m_bodyUniqueIds.size())
		{
			if (m_selectedBody < 0)
				m_selectedBody = 0;

			ComboBoxParams comboParams;
			comboParams.m_comboboxId = 0;
			comboParams.m_numItems = m_bodyUniqueIds.size();
			comboParams.m_startItem = m_selectedBody;
			comboParams.m_callback = MyComboBoxCallback;
			comboParams.m_userPointer = this;
			//todo: get the real object name

			tukk* blarray = new tukk [m_bodyUniqueIds.size()];

			for (i32 i = 0; i < m_bodyUniqueIds.size(); i++)
			{
				tuk bla = new char[16];
				sprintf(bla, "%d", i);
				blarray[i] = bla;
				comboParams.m_items = blarray;  //{&bla};
			}
			m_guiHelper->getParameterInterface()->registerComboBox(comboParams);
		}

		if (m_physicsClientHandle && m_selectedBody >= 0)
		{
			m_numMotors = 0;

			i32 numJoints = b3GetNumJoints(m_physicsClientHandle, m_selectedBody);
			for (i32 i = 0; i < numJoints; i++)
			{
				b3JointInfo info;
				b3GetJointInfo(m_physicsClientHandle, m_selectedBody, i, &info);
				//drx3DPrintf("Joint %s at q-index %d and u-index %d\n",info.m_jointName,info.m_qIndex,info.m_uIndex);

				if (info.m_flags & JOINT_HAS_MOTORIZED_POWER)
				{
					if (m_numMotors < MAX_NUM_MOTORS)
					{
						char motorName[1026];
						safe_printf(motorName, sizeof(motorName), "%s q", info.m_jointName);
						// MyMotorInfo2* motorInfo = &m_motorTargetVelocities[m_numMotors];
						MyMotorInfo2* motorInfo = &m_motorTargetPositions[m_numMotors];
						motorInfo->m_velTarget = 0.f;
						motorInfo->m_posTarget = 0.f;
						motorInfo->m_uIndex = info.m_uIndex;
						motorInfo->m_qIndex = info.m_qIndex;

						// SliderParams slider(motorName,&motorInfo->m_velTarget);
						// slider.m_minVal=-4;
						// slider.m_maxVal=4;
						SliderParams slider(motorName, &motorInfo->m_posTarget);
						slider.m_minVal = -4;
						slider.m_maxVal = 4;
						if (m_guiHelper && m_guiHelper->getParameterInterface())
						{
							m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
						}
						m_numMotors++;
					}
				}
			}
		}

		{
			SliderParams sliderLightPosX("light source position x", &m_lightPos[0]);
			SliderParams sliderLightPosY("light source position y", &m_lightPos[1]);
			SliderParams sliderLightPosZ("light source position z", &m_lightPos[2]);
			SliderParams sliderSpecularCoeff("specular coefficient", &m_specularCoeff);
			sliderLightPosX.m_minVal = -1.5;
			sliderLightPosX.m_maxVal = 1.5;
			sliderLightPosY.m_minVal = -1.5;
			sliderLightPosY.m_maxVal = 1.5;
			sliderLightPosZ.m_minVal = -1.5;
			sliderLightPosZ.m_maxVal = 1.5;
			sliderSpecularCoeff.m_minVal = 0;
			sliderSpecularCoeff.m_maxVal = 5.0;
			if (m_guiHelper && m_guiHelper->getParameterInterface())
			{
				m_guiHelper->getParameterInterface()->registerSliderFloatParameter(sliderLightPosX);
				m_guiHelper->getParameterInterface()->registerSliderFloatParameter(sliderLightPosY);
				m_guiHelper->getParameterInterface()->registerSliderFloatParameter(sliderLightPosZ);
				m_guiHelper->getParameterInterface()->registerSliderFloatParameter(sliderSpecularCoeff);
			}
		}
	}
}

void PhysicsClientExample::initPhysics()
{
	if (m_guiHelper && m_guiHelper->getParameterInterface())
	{
		i32 upAxis = 2;
		m_guiHelper->setUpAxis(upAxis);

		createButtons();
	}
	else
	{
		MyCallback(CMD_LOAD_URDF, true, this);
		MyCallback(CMD_STEP_FORWARD_SIMULATION, true, this);

		MyCallback(CMD_RESET_SIMULATION, true, this);
	}

	m_selectedBody = -1;
	m_prevSelectedBody = -1;

	m_lightPos[0] = 1.0;
	m_lightPos[1] = 1.0;
	m_lightPos[2] = 1.0;

	{
		m_canvas = m_guiHelper->get2dCanvasInterface();
		if (m_canvas)
		{
			m_canvasRGBIndex = m_canvas->createCanvas("Synthetic Camera RGB data", camVisualizerWidth, camVisualizerHeight, 8, 55);
			m_canvasDepthIndex = m_canvas->createCanvas("Synthetic Camera Depth data", camVisualizerWidth, camVisualizerHeight, 8, 75 + camVisualizerHeight);
			m_canvasSegMaskIndex = m_canvas->createCanvas("Synthetic Camera Segmentation Mask", camVisualizerWidth, camVisualizerHeight, 8, 95 + camVisualizerHeight * 2);

			for (i32 i = 0; i < camVisualizerWidth; i++)
			{
				for (i32 j = 0; j < camVisualizerHeight; j++)
				{
					u8 red = 255;
					u8 green = 255;
					u8 blue = 255;
					u8 alpha = 255;
					if (i == j)
					{
						red = 0;
						green = 0;
						blue = 0;
					}
					m_canvas->setPixel(m_canvasRGBIndex, i, j, red, green, blue, alpha);
					m_canvas->setPixel(m_canvasDepthIndex, i, j, red, green, blue, alpha);
					m_canvas->setPixel(m_canvasSegMaskIndex, i, j, red, green, blue, alpha);
				}
			}
			m_canvas->refreshImageData(m_canvasRGBIndex);
			m_canvas->refreshImageData(m_canvasDepthIndex);
			m_canvas->refreshImageData(m_canvasSegMaskIndex);
		}
	}

	if (m_options == eCLIENTEXAMPLE_SERVER)
	{
		m_isOptionalServerConnected = m_physicsServer.connectSharedMemory(m_guiHelper);
	}

	if (m_options == eCLIENTEXAMPLE_DIRECT)
	{
		m_physicsClientHandle = b3ConnectPhysicsDirect();
	}
	else
	{
		m_physicsClientHandle = b3ConnectSharedMemory(m_sharedMemoryKey);
		//m_physicsClientHandle  = b3ConnectPhysicsLoopback(SHARED_MEMORY_KEY);
	}

	if (!b3CanSubmitCommand(m_physicsClientHandle))
	{
		drx3DWarning("Cannot connect to physics client");
	}
}

void PhysicsClientExample::stepSimulation(float deltaTime)
{
	if (m_options == eCLIENTEXAMPLE_SERVER)
	{
		for (i32 i = 0; i < 100; i++)
		{
			m_physicsServer.processClientCommands();
		}
	}

	if (m_prevSelectedBody != m_selectedBody)
	{
		createButtons();
		m_prevSelectedBody = m_selectedBody;
	}

	//while (!b3CanSubmitCommand(m_physicsClientHandle))
	{
		b3SharedMemoryStatusHandle status = b3ProcessServerStatus(m_physicsClientHandle);
		bool hasStatus = (status != 0);
		if (hasStatus)
		{
			i32 statusType = b3GetStatusType(status);
			if (statusType == CMD_ACTUAL_STATE_UPDATE_COMPLETED)
			{
				//drx3DPrintf("bla\n");
			}
			if (statusType == CMD_CAMERA_IMAGE_COMPLETED)
			{
				//	static i32 counter=0;
				//	char msg[1024];
				//	sprintf(msg,"Camera image %d OK\n",counter++);
				b3CameraImageData imageData;
				b3GetCameraImageData(m_physicsClientHandle, &imageData);
				if (m_canvas)
				{
					//compute depth image range
					float minDepthValue = 1e20f;
					float maxDepthValue = -1e20f;

					for (i32 i = 0; i < camVisualizerWidth; i++)
					{
						for (i32 j = 0; j < camVisualizerHeight; j++)
						{
							i32 xIndex = i32(float(i) * (float(imageData.m_pixelWidth) / float(camVisualizerWidth)));
							i32 yIndex = i32(float(j) * (float(imageData.m_pixelHeight) / float(camVisualizerHeight)));
							Clamp(xIndex, 0, imageData.m_pixelWidth);
							Clamp(yIndex, 0, imageData.m_pixelHeight);

							if (m_canvasDepthIndex >= 0)
							{
								i32 depthPixelIndex = (xIndex + yIndex * imageData.m_pixelWidth);
								float depthValue = imageData.m_depthValues[depthPixelIndex];
								//todo: rescale the depthValue to [0..255]
								if (depthValue > -1e20)
								{
									maxDepthValue = d3Max(maxDepthValue, depthValue);
									minDepthValue = d3Min(minDepthValue, depthValue);
								}
							}
						}
					}

					for (i32 i = 0; i < camVisualizerWidth; i++)
					{
						for (i32 j = 0; j < camVisualizerHeight; j++)
						{
							i32 xIndex = i32(float(i) * (float(imageData.m_pixelWidth) / float(camVisualizerWidth)));
							i32 yIndex = i32(float(j) * (float(imageData.m_pixelHeight) / float(camVisualizerHeight)));
							Clamp(yIndex, 0, imageData.m_pixelHeight);
							Clamp(xIndex, 0, imageData.m_pixelWidth);
							i32 bytesPerPixel = 4;  //RGBA

							if (m_canvasRGBIndex >= 0)
							{
								i32 rgbPixelIndex = (xIndex + yIndex * imageData.m_pixelWidth) * bytesPerPixel;
								m_canvas->setPixel(m_canvasRGBIndex, i, j,
												   imageData.m_rgbColorData[rgbPixelIndex],
												   imageData.m_rgbColorData[rgbPixelIndex + 1],
												   imageData.m_rgbColorData[rgbPixelIndex + 2],
												   255);  //alpha set to 255
							}

							if (m_canvasDepthIndex >= 0)
							{
								i32 depthPixelIndex = (xIndex + yIndex * imageData.m_pixelWidth);
								float depthValue = imageData.m_depthValues[depthPixelIndex];
								//todo: rescale the depthValue to [0..255]
								if (depthValue > -1e20)
								{
									i32 rgb = 0;

									if (maxDepthValue != minDepthValue)
									{
										rgb = (depthValue - minDepthValue) * (255. / (Fabs(maxDepthValue - minDepthValue)));
										if (rgb < 0 || rgb > 255)
										{
											//printf("rgb=%d\n",rgb);
										}
									}
									m_canvas->setPixel(m_canvasDepthIndex, i, j,
													   rgb,
													   rgb,
													   255, 255);  //alpha set to 255
								}
								else
								{
									m_canvas->setPixel(m_canvasDepthIndex, i, j,
													   0,
													   0,
													   0, 255);  //alpha set to 255
								}
							}
							if (m_canvasSegMaskIndex >= 0 && (0 != imageData.m_segmentationMaskValues))
							{
								i32 segmentationMaskPixelIndex = (xIndex + yIndex * imageData.m_pixelWidth);
								i32 segmentationMask = imageData.m_segmentationMaskValues[segmentationMaskPixelIndex];
								Vec4 palette[4] = {Vec4(32, 255, 32, 255),
														Vec4(32, 32, 255, 255),
														Vec4(255, 255, 32, 255),
														Vec4(32, 255, 255, 255)};
								if (segmentationMask >= 0)
								{
									i32 obIndex = segmentationMask & ((1 << 24) - 1);
									i32 linkIndex = (segmentationMask >> 24) - 1;

									Vec4 rgb = palette[(obIndex + linkIndex) & 3];
									m_canvas->setPixel(m_canvasSegMaskIndex, i, j,
													   rgb.x(),
													   rgb.y(),
													   rgb.z(), 255);  //alpha set to 255
								}
								else
								{
									m_canvas->setPixel(m_canvasSegMaskIndex, i, j,
													   0,
													   0,
													   0, 255);  //alpha set to 255
								}
							}
						}
					}
					if (m_canvasRGBIndex >= 0)
						m_canvas->refreshImageData(m_canvasRGBIndex);
					if (m_canvasDepthIndex >= 0)
						m_canvas->refreshImageData(m_canvasDepthIndex);
					if (m_canvasSegMaskIndex >= 0)
						m_canvas->refreshImageData(m_canvasSegMaskIndex);
				}

				// drx3DPrintf(msg);
			}
			if (statusType == CMD_CALCULATED_INVERSE_DYNAMICS_COMPLETED)
			{
				i32 bodyUniqueId;
				i32 dofCount;

				b3GetStatusdrx3d_inverseJointForces(status,
													  &bodyUniqueId,
													  &dofCount,
													  0);

				AlignedObjectArray<double> jointForces;
				if (dofCount)
				{
					jointForces.resize(dofCount);
					b3GetStatusdrx3d_inverseJointForces(status,
														  0,
														  0,
														  &jointForces[0]);
					for (i32 i = 0; i < dofCount; i++)
					{
						drx3DPrintf("jointForces[%d]=%f", i, jointForces[i]);
					}
				}
			}
			if (statusType == CMD_CALCULATED_INVERSE_DYNAMICS_FAILED)
			{
				drx3DWarning("Inverse Dynamics computations failed");
			}

			if (statusType == CMD_CAMERA_IMAGE_FAILED)
			{
				drx3DWarning("Camera image FAILED\n");
			}

			if (statusType == CMD_SDF_LOADING_COMPLETED)
			{
				i32 bodyIndicesOut[1024];
				i32 bodyCapacity = 1024;
				i32 numBodies = b3GetStatusBodyIndices(status, bodyIndicesOut, bodyCapacity);
				if (numBodies > bodyCapacity)
				{
					drx3DWarning("loadSDF number of bodies (%d) exceeds the internal body capacity (%d)", numBodies, bodyCapacity);
				}
				else
				{
					for (i32 i = 0; i < numBodies; i++)
					{
						i32 bodyUniqueId = bodyIndicesOut[i];
						m_bodyUniqueIds.push_back(bodyUniqueId);
						i32 numJoints = b3GetNumJoints(m_physicsClientHandle, bodyUniqueId);
						if (numJoints > 0)
						{
							m_selectedBody = bodyUniqueId;
						}
						/*                        i32 numJoints =  b3GetNumJoints(m_physicsClientHandle,bodyUniqueId);
                        drx3DPrintf("numJoints = %d", numJoints);
                        for (i32 i=0;i<numJoints;i++)
                        {
                            b3JointInfo info;
                            b3GetJointInfo(m_physicsClientHandle,bodyUniqueId,i,&info);
                            drx3DPrintf("Joint %s at q-index %d and u-index %d\n",info.m_jointName,info.m_qIndex,info.m_uIndex);
                        }
 
 */
					}
				}

				//i32 numJoints = b3GetNumJoints(m_physicsClientHandle,bodyIndex);

				//i32 bodyIndex = b3GetStatusBodyIndex(status);
				/*if (bodyIndex>=0)
				{
					i32 numJoints = b3GetNumJoints(m_physicsClientHandle,bodyIndex);
            
					for (i32 i=0;i<numJoints;i++)
					{
						b3JointInfo info;
						b3GetJointInfo(m_physicsClientHandle,bodyIndex,i,&info);
						drx3DPrintf("Joint %s at q-index %d and u-index %d\n",info.m_jointName,info.m_qIndex,info.m_uIndex);
					}
					ComboBoxParams comboParams;
					comboParams.m_comboboxId = bodyIndex;
					comboParams.m_numItems = 1;
					comboParams.m_startItem = 0;
					comboParams.m_callback = MyComboBoxCallback;
					comboParams.m_userPointer = this;
					tukk bla = "bla";
					tukk blarray[1];
					blarray[0] = bla;
				
					comboParams.m_items=blarray;//{&bla};
					m_guiHelper->getParameterInterface()->registerComboBox(comboParams);
				}
				*/
			}

			if (statusType == CMD_URDF_LOADING_COMPLETED)
			{
				i32 bodyIndex = b3GetStatusBodyIndex(status);
				if (bodyIndex >= 0)
				{
					m_bodyUniqueIds.push_back(bodyIndex);
					m_selectedBody = bodyIndex;
					i32 numJoints = b3GetNumJoints(m_physicsClientHandle, bodyIndex);

					for (i32 i = 0; i < numJoints; i++)
					{
						b3JointInfo info;
						b3GetJointInfo(m_physicsClientHandle, bodyIndex, i, &info);
						//drx3DPrintf("Joint %s at q-index %d and u-index %d\n",info.m_jointName,info.m_qIndex,info.m_uIndex);
					}
				}
			}
			if (statusType == CMD_CONTACT_POINT_INFORMATION_FAILED)
			{
				drx3DWarning("Cannot get contact information");
			}
			if (statusType == CMD_VISUAL_SHAPE_INFO_FAILED)
			{
				drx3DWarning("Cannot get visual shape information");
			}
			if (statusType == CMD_VISUAL_SHAPE_UPDATE_FAILED)
			{
				drx3DWarning("Cannot update visual shape");
			}
			if (statusType == CMD_VISUAL_SHAPE_INFO_COMPLETED)
			{
				b3VisualShapeInformation shapeInfo;
				b3GetVisualShapeInformation(m_physicsClientHandle, &shapeInfo);
				drx3DPrintf("Num visual shapes: %d", shapeInfo.m_numVisualShapes);
			}
			if (statusType == CMD_VISUAL_SHAPE_UPDATE_COMPLETED)
			{
				drx3DPrintf("Visual shape update completed.");
			}
			if (statusType == CMD_CONTACT_POINT_INFORMATION_COMPLETED)
			{
				b3ContactInformation contactPointData;
				b3GetContactPointInformation(m_physicsClientHandle, &contactPointData);
				drx3DPrintf("Num Contacts: %d\n", contactPointData.m_numContactPoints);
			}
		}
	}
	if (b3CanSubmitCommand(m_physicsClientHandle))
	{
		if (m_userCommandRequests.size())
		{
			//drx3DPrintf("Outstanding user command requests: %d\n", m_userCommandRequests.size());
			i32 commandId = m_userCommandRequests[0];

			//a manual 'pop_front', we don't use 'remove' because it will re-order the commands
			for (i32 i = 1; i < m_userCommandRequests.size(); i++)
			{
				m_userCommandRequests[i - 1] = m_userCommandRequests[i];
			}

			m_userCommandRequests.pop_back();

			//for the CMD_RESET_SIMULATION we need to do something special: clear the GUI sliders
			if (commandId == CMD_RESET_SIMULATION)
			{
				m_selectedBody = -1;
				m_numMotors = 0;
				m_bodyUniqueIds.clear();
				createButtons();
				b3SharedMemoryCommandHandle commandHandle = b3InitResetSimulationCommand(m_physicsClientHandle);
				if (m_options == eCLIENTEXAMPLE_SERVER)
				{
					b3SubmitClientCommand(m_physicsClientHandle, commandHandle);
					while (!b3CanSubmitCommand(m_physicsClientHandle))
					{
						m_physicsServer.processClientCommands();
						b3SharedMemoryStatusHandle status = b3ProcessServerStatus(m_physicsClientHandle);
						bool hasStatus = (status != 0);
						if (hasStatus)
						{
							//i32 statusType = b3GetStatusType(status);
							//drx3DPrintf("Status after reset: %d",statusType);
						}
					}
				}
				else
				{
					prepareAndSubmitCommand(commandId);
				}
			}
			else
			{
				prepareAndSubmitCommand(commandId);
			}
		}
		else
		{
			if (m_numMotors)
			{
				enqueueCommand(CMD_SEND_DESIRED_STATE);
			}
			enqueueCommand(CMD_STEP_FORWARD_SIMULATION);
			if (m_options != eCLIENTEXAMPLE_SERVER)
			{
				//enqueueCommand(CMD_REQUEST_DEBUG_LINES);
			}
		}
	}
}

extern i32 gSharedMemoryKey;

class CommonExampleInterface* PhysicsClientCreateFunc(struct CommonExampleOptions& options)
{
	PhysicsClientExample* example = new PhysicsClientExample(options.m_guiHelper, options.m_option);
	if (gSharedMemoryKey >= 0)
	{
		example->setSharedMemoryKey(gSharedMemoryKey);
	}
	return example;
}
