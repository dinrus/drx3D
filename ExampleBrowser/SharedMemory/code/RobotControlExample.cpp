#include "../RobotControlExample.h"

#if 0

#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <drx3D/SharedMemory/PhysicsServer.h>
#include <drx3D/SharedMemory/PhysicsClient.h>
#include <drx3D/SharedMemory/SharedMemoryCommon.h>
#include <drx3D/Common/b3Clock.h>
#include <drx3D/SharedMemory/PhysicsClientC_API.h>
#include <drx3D/Common/ResourcePath.h>
#include <string>

//tukk blaatnaam = "basename";

struct MyMotorInfo
{
	STxt m_jointName;
	Scalar m_velTarget;
	Scalar m_posTarget;
    Scalar m_kp;
    Scalar m_kd;

	Scalar m_maxForce;
	i32		m_uIndex;
	i32		m_posIndex;

	i32		m_jointIndex;
	Scalar m_measuredJointPosition;
	Scalar m_measuredJointVelocity;
	Vec3 m_measuredJointForce;
	Vec3 m_measuredJointTorque;

};
#define MAX_NUM_MOTORS 128

class RobotControlExample : public SharedMemoryCommon
{
	PhysicsServerSharedMemory	m_physicsServer;
	PhysicsClientSharedMemory	m_physicsClient;
	b3Clock	m_realtimeClock;

	i32	m_sequenceNumberGenerator;
	bool m_wantsShutdown;

	AlignedObjectArray<SharedMemoryCommand> m_userCommandRequests;


	void	createButton(tukk name, i32 id, bool isTrigger );

public:
    //@todo, add accessor methods
	MyMotorInfo m_motorTargetState[MAX_NUM_MOTORS];

	i32 m_numMotors;
	i32 m_option;
	bool m_verboseOutput;

	RobotControlExample(GUIHelperInterface* helper, i32 option);

	virtual ~RobotControlExample();

	virtual void	initPhysics();

	virtual void	stepSimulation(float deltaTime);
    void prepareControlCommand(SharedMemoryCommand& cmd);

    void enqueueCommand(const SharedMemoryCommand& orgCommand)
	{
		m_userCommandRequests.push_back(orgCommand);
		SharedMemoryCommand& cmd = m_userCommandRequests[m_userCommandRequests.size()-1];
		cmd.m_sequenceNumber = m_sequenceNumberGenerator++;
		cmd.m_timeStamp = m_realtimeClock.getTimeMicroseconds();

		if (m_verboseOutput)
		{
			drx3DPrintf("User put command request %d on queue (queue length = %d)\n",cmd.m_type, m_userCommandRequests.size());
		}
	}

	virtual void resetCamera()
	{
		float dist = 5;
		float pitch = 50;
		float yaw = 35;
		float targetPos[3]={0,0,0};//-3,2.8,-2.5};
		m_guiHelper->resetCamera(dist,pitch,yaw,targetPos[0],targetPos[1],targetPos[2]);
	}

    virtual bool wantsTermination();
    virtual bool isConnected();
	virtual void	renderScene()
	{
		m_physicsServer.renderScene();
	}
	virtual void    exitPhysics(){}

	virtual void	physicsDebugDraw(i32 debugFlags)
	{
		m_physicsServer.physicsDebugDraw(debugFlags);

	}
	virtual bool	mouseMoveCallback(float x,float y){return false;};
	virtual bool	mouseButtonCallback(i32 button, i32 state, float x, float y){return false;}
	virtual bool	keyboardCallback(i32 key, i32 state){return false;}

	virtual void setSharedMemoryKey(i32 key)
	{
		m_physicsServer.setSharedMemoryKey(key);
		m_physicsClient.setSharedMemoryKey(key);
	}

};

bool RobotControlExample::isConnected()
{
    return m_physicsClient.isConnected();
}

void MyCallback2(i32 buttonId, bool buttonState, uk userPtr)
{
	RobotControlExample* cl = (RobotControlExample*) userPtr;

	SharedMemoryCommand command;

	switch (buttonId)
	{
	case  CMD_LOAD_URDF:
		{
			command.m_type =CMD_LOAD_URDF;
			command.m_updateFlags = URDF_ARGS_FILE_NAME|URDF_ARGS_INITIAL_POSITION|URDF_ARGS_INITIAL_ORIENTATION;
            sprintf(command.m_urdfArguments.m_urdfFileName,"r2d2.urdf");//kuka_lwr/kuka.urdf");//r2d2.urdf");
			command.m_urdfArguments.m_initialPosition[0] = 0.0;
			command.m_urdfArguments.m_initialPosition[1] = 0.0;
			command.m_urdfArguments.m_initialPosition[2] = 0.0;
			command.m_urdfArguments.m_initialOrientation[0] = 0.0;
			command.m_urdfArguments.m_initialOrientation[1] = 0.0;
			command.m_urdfArguments.m_initialOrientation[2] = 0.0;
			command.m_urdfArguments.m_initialOrientation[3] = 1.0;
			command.m_urdfArguments.m_useFixedBase = false;
			command.m_urdfArguments.m_useMultiBody = true;
			cl->enqueueCommand(command);
			break;
		}

		case CMD_SEND_PHYSICS_SIMULATION_PARAMETERS:
		{
		    //#ifdef USE_C_API
		    b3InitPhysicsParamCommand(&command);
            b3PhysicsParamSetGravity(&command, 1,1,-10);


//		    #else
//
//			command.m_type = CMD_SEND_PHYSICS_SIMULATION_PARAMETERS;
//			command.m_physSimParamArgs.m_gravityAcceleration[0] = 0;
//			command.m_physSimParamArgs.m_gravityAcceleration[1] = 0;
//			command.m_physSimParamArgs.m_gravityAcceleration[2] = -10;
//			command.m_physSimParamArgs.m_updateFlags = SIM_PARAM_UPDATE_GRAVITY;
//			#endif // USE_C_API

			cl->enqueueCommand(command);
			break;

		};
		case CMD_INIT_POSE:
		{
			///@todo: implement this
			command.m_type = CMD_INIT_POSE;
			cl->enqueueCommand(command);
			break;
		}


	case CMD_CREATE_BOX_COLLISION_SHAPE:
		{
			command.m_type =CMD_CREATE_BOX_COLLISION_SHAPE;
            command.m_updateFlags = BOX_SHAPE_HAS_INITIAL_POSITION;
            command.m_createBoxShapeArguments.m_initialPosition[0] = 0;
            command.m_createBoxShapeArguments.m_initialPosition[1] = 0;
            command.m_createBoxShapeArguments.m_initialPosition[2] = -3;


			cl->enqueueCommand(command);
			break;
		}
	case CMD_REQUEST_ACTUAL_STATE:
		{
			command.m_type =CMD_REQUEST_ACTUAL_STATE;
			cl->enqueueCommand(command);
			break;
		};
	case CMD_STEP_FORWARD_SIMULATION:
		{
			command.m_type =CMD_STEP_FORWARD_SIMULATION;
			cl->enqueueCommand(command);
			break;
		}

    case CMD_SEND_DESIRED_STATE:
		{

			command.m_type =CMD_SEND_DESIRED_STATE;
			cl->prepareControlCommand(command);
			cl->enqueueCommand(command);
			break;
		}
	case CMD_SEND_DRX3D_DATA_STREAM:
		{
			command.m_type = buttonId;
			sprintf(command.m_dataStreamArguments.m_bulletFileName,"slope.bullet");
			command.m_dataStreamArguments.m_streamChunkLength = 0;
			cl->enqueueCommand(command);
			break;
		}

	default:
		{
			drx3DError("Unknown buttonId");
			Assert(0);
		}
	};
}


void RobotControlExample::prepareControlCommand(SharedMemoryCommand& command)
{
	for (i32 i=0;i<MAX_DEGREE_OF_FREEDOM;i++)
	{
		command.m_sendDesiredStateCommandArgument.m_desiredStateQdot[i] = 0;
		command.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[i] = 0;
	}

	switch (m_option)
	{
	case ROBOT_VELOCITY_CONTROL:
		{
			command.m_sendDesiredStateCommandArgument.m_controlMode = CONTROL_MODE_VELOCITY;
			for (i32 i=0;i<MAX_DEGREE_OF_FREEDOM;i++)
			{
				command.m_sendDesiredStateCommandArgument.m_desiredStateQdot[i] = 0;
				command.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[i] = 1000;
			}
			for (i32 i=0;i<m_numMotors;i++)
			{
				Scalar targetVel = m_motorTargetState[i].m_velTarget;

				i32 uIndex = m_motorTargetState[i].m_uIndex;

                command.m_sendDesiredStateCommandArgument.m_desiredStateQdot[uIndex] = targetVel;

			}
			break;
		}
	case ROBOT_PD_CONTROL:
	{
		command.m_sendDesiredStateCommandArgument.m_controlMode = CONTROL_MODE_POSITION_VELOCITY_PD;
		for (i32 i=0;i<m_numMotors;i++)
		{

			i32 uIndex = m_motorTargetState[i].m_uIndex;

			command.m_sendDesiredStateCommandArgument.m_Kp[uIndex] = m_motorTargetState[i].m_kp;
			command.m_sendDesiredStateCommandArgument.m_Kd[uIndex] = m_motorTargetState[i].m_kd;
			command.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[uIndex] = 10000;//max force

			Scalar targetVel = m_motorTargetState[i].m_velTarget;
			command.m_sendDesiredStateCommandArgument.m_desiredStateQdot[uIndex] = targetVel;

			i32 posIndex = m_motorTargetState[i].m_posIndex;
			Scalar targetPos = m_motorTargetState[i].m_posTarget;
			command.m_sendDesiredStateCommandArgument.m_desiredStateQ[posIndex] = targetPos;
		}
		break;
	}
	case ROBOT_PING_PONG_JOINT_FEEDBACK:
	{

		command.m_sendDesiredStateCommandArgument.m_controlMode = CONTROL_MODE_VELOCITY;
		for (i32 i=0;i<MAX_DEGREE_OF_FREEDOM;i++)
		{
			command.m_sendDesiredStateCommandArgument.m_desiredStateQdot[i] = 0;
			command.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[i] = 1000;
		}
		for (i32 i=0;i<m_numMotors;i++)
		{
			Scalar targetVel = m_motorTargetState[i].m_velTarget;

			i32 uIndex = m_motorTargetState[i].m_uIndex;
            command.m_sendDesiredStateCommandArgument.m_desiredStateQdot[uIndex] = m_motorTargetState[i].m_velTarget;

		}
		break;
	}
	default:
		{

			drx3DWarning("Неизвестный режим контроля в RobotControlExample::prepareControlCommand");
		}
	};

}
void	RobotControlExample::createButton(tukk name, i32 buttonId, bool isTrigger )
{
	ButtonParams button(name,buttonId,  isTrigger);
	button.m_callback = MyCallback2;
	button.m_userPointer = this;
	m_guiHelper->getParameterInterface()->registerButtonParameter(button);
}

RobotControlExample::RobotControlExample(GUIHelperInterface* helper, i32 option)
:SharedMemoryCommon(helper),
m_wantsShutdown(false),
m_sequenceNumberGenerator(0),
m_numMotors(0),
m_option(option),
m_verboseOutput(false)
{

	bool useServer = true;
}



RobotControlExample::~RobotControlExample()
{
	bool deInitializeSharedMemory = true;
    m_physicsClient.disconnectSharedMemory();
	m_physicsServer.disconnectSharedMemory(deInitializeSharedMemory);
}

void	RobotControlExample::initPhysics()
{
	///for this testing we use Z-axis up
	i32 upAxis = 2;
	m_guiHelper->setUpAxis(upAxis);

  /*  createEmptyDynamicsWorld();
	//todo: create a special debug drawer that will cache the lines, so we can send the debug info over the wire
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	Vec3 grav(0,0,0);
	grav[upAxis] = 0;//-9.8;
	this->m_dynamicsWorld->setGravity(grav);
    */
	m_physicsServer.connectSharedMemory( m_guiHelper);

	if (m_guiHelper && m_guiHelper->getParameterInterface())
	{
		bool isTrigger = false;

		createButton("Load URDF",CMD_LOAD_URDF,  isTrigger);
		createButton("Step Sim",CMD_STEP_FORWARD_SIMULATION,  isTrigger);
		createButton("Send drx3D Stream",CMD_SEND_DRX3D_DATA_STREAM,  isTrigger);
		createButton("Get State",CMD_REQUEST_ACTUAL_STATE,  isTrigger);
		createButton("Send Desired State",CMD_SEND_DESIRED_STATE,  isTrigger);
		createButton("Create Box Collider",CMD_CREATE_BOX_COLLISION_SHAPE,isTrigger);
		createButton("Set Physics Params",CMD_SEND_PHYSICS_SIMULATION_PARAMETERS,isTrigger);
		createButton("Init Pose",CMD_INIT_POSE,isTrigger);
	} else
	{
		/*
		m_userCommandRequests.push_back(CMD_LOAD_URDF);
		m_userCommandRequests.push_back(CMD_REQUEST_ACTUAL_STATE);
		m_userCommandRequests.push_back(CMD_SEND_DESIRED_STATE);
		m_userCommandRequests.push_back(CMD_REQUEST_ACTUAL_STATE);
		//m_userCommandRequests.push_back(CMD_SET_JOINT_FEEDBACK);
		m_userCommandRequests.push_back(CMD_CREATE_BOX_COLLISION_SHAPE);
		//m_userCommandRequests.push_back(CMD_CREATE_RIGID_BODY);
		m_userCommandRequests.push_back(CMD_STEP_FORWARD_SIMULATION);
		m_userCommandRequests.push_back(CMD_REQUEST_ACTUAL_STATE);
		m_userCommandRequests.push_back(CMD_SHUTDOWN);
		*/
	}

	if (!m_physicsClient.connect())
	{
		drx3DWarning("Н удаётся подключиться к клиенту физики");
	}

}


bool RobotControlExample::wantsTermination()
{
    return m_wantsShutdown;
}



void	RobotControlExample::stepSimulation(float deltaTime)
{

	m_physicsServer.processClientCommands();

	if (m_physicsClient.isConnected())
    {

		SharedMemoryStatus status;
		bool hasStatus = m_physicsClient.processServerStatus(status);
		if ((m_option==ROBOT_PING_PONG_JOINT_FEEDBACK) && hasStatus && status.m_type == CMD_ACTUAL_STATE_UPDATE_COMPLETED)
		{
			//update sensor feedback: joint force/torque data and measured joint positions

			for (i32 i=0;i<m_numMotors;i++)
			{
				i32 jointIndex = m_motorTargetState[i].m_jointIndex;
				i32 positionIndex = m_motorTargetState[i].m_posIndex;
				i32 velocityIndex = m_motorTargetState[i].m_uIndex;

				m_motorTargetState[i].m_measuredJointPosition = status.m_sendActualStateArgs.m_actualStateQ[positionIndex];
				m_motorTargetState[i].m_measuredJointVelocity = status.m_sendActualStateArgs.m_actualStateQdot[velocityIndex];
				m_motorTargetState[i].m_measuredJointForce.setVal(status.m_sendActualStateArgs.m_jointReactionForces[6*jointIndex],
																	status.m_sendActualStateArgs.m_jointReactionForces[6*jointIndex+1],
																	status.m_sendActualStateArgs.m_jointReactionForces[6*jointIndex+2]);
				m_motorTargetState[i].m_measuredJointTorque.setVal(status.m_sendActualStateArgs.m_jointReactionForces[6*jointIndex+3],
																	status.m_sendActualStateArgs.m_jointReactionForces[6*jointIndex+4],
																	status.m_sendActualStateArgs.m_jointReactionForces[6*jointIndex+5]);

				if (m_motorTargetState[i].m_measuredJointPosition>0.1)
				{
					m_motorTargetState[i].m_velTarget = -1.5;
				} else
				{
					m_motorTargetState[i].m_velTarget = 1.5;
				}

				drx3DPrintf("Joint Force (Linear) [%s]=(%f,%f,%f)\n",m_motorTargetState[i].m_jointName.c_str(),m_motorTargetState[i].m_measuredJointForce.x(),m_motorTargetState[i].m_measuredJointForce.y(),m_motorTargetState[i].m_measuredJointForce.z());
				drx3DPrintf("Joint Torque (Angular) [%s]=(%f,%f,%f)\n",m_motorTargetState[i].m_jointName.c_str(),m_motorTargetState[i].m_measuredJointTorque.x(),m_motorTargetState[i].m_measuredJointTorque.y(),m_motorTargetState[i].m_measuredJointTorque.z());

			}


		}

		if (hasStatus && status.m_type == CMD_URDF_LOADING_COMPLETED)
		{
			SharedMemoryCommand sensorCommand;
			sensorCommand.m_type = CMD_CREATE_SENSOR;
			sensorCommand.m_createSensorArguments.m_numJointSensorChanges = 0;

			for (i32 jointIndex=0;jointIndex<m_physicsClient.getNumJoints();jointIndex++)
			{
				b3JointInfo info;
				m_physicsClient.getJointInfo(jointIndex,info);
				if (m_verboseOutput)
				{
					drx3DPrintf("Joint %s at q-index %d and u-index %d\n",info.m_jointName,info.m_qIndex,info.m_uIndex);
				}

                if (info.m_flags & JOINT_HAS_MOTORIZED_POWER)
                {
                    if (m_numMotors<MAX_NUM_MOTORS)
                    {

						switch (m_option)
						{
						case ROBOT_VELOCITY_CONTROL:
							{
		                        char motorName[1024];
								sprintf(motorName,"%s q'", info.m_jointName);
								MyMotorInfo* motorInfo = &m_motorTargetState[m_numMotors];
								motorInfo->m_jointName = info.m_jointName;

								motorInfo->m_velTarget = 0.f;
								motorInfo->m_posTarget = 0.f;

								motorInfo->m_uIndex = info.m_uIndex;

								SliderParams slider(motorName,&motorInfo->m_velTarget);
								slider.m_minVal=-4;
								slider.m_maxVal=4;
								m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
								m_numMotors++;
								break;
							}
						case ROBOT_PD_CONTROL:
						{
							char motorName[1024];

							MyMotorInfo* motorInfo = &m_motorTargetState[m_numMotors];
							motorInfo->m_jointName = info.m_jointName;
							motorInfo->m_velTarget = 0.f;
							motorInfo->m_posTarget = 0.f;
							motorInfo->m_uIndex = info.m_uIndex;
							motorInfo->m_posIndex  = info.m_qIndex;
                            motorInfo->m_kp = 1;
                            motorInfo->m_kd = 0;

                            {
                                sprintf(motorName,"%s kp", info.m_jointName);
                                SliderParams slider(motorName,&motorInfo->m_kp);
                                slider.m_minVal=0;
                                slider.m_maxVal=1;
                                m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
                            }

                            {
                                sprintf(motorName,"%s q", info.m_jointName);
							SliderParams slider(motorName,&motorInfo->m_posTarget);
							slider.m_minVal=-SIMD_PI;
							slider.m_maxVal=SIMD_PI;
							m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
                            }
                            {
                                sprintf(motorName,"%s kd", info.m_jointName);
                                SliderParams slider(motorName,&motorInfo->m_kd);
                                slider.m_minVal=0;
                                slider.m_maxVal=1;
                                m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
                            }

                            {
                                 sprintf(motorName,"%s q'", info.m_jointName);
                            SliderParams slider(motorName,&motorInfo->m_velTarget);
                            slider.m_minVal=-10;
                            slider.m_maxVal=10;
                            m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
                            }
							m_numMotors++;
							break;
						}
						case ROBOT_PING_PONG_JOINT_FEEDBACK:
						{

							if (info.m_flags & JOINT_HAS_MOTORIZED_POWER)
							{
								if (m_numMotors<MAX_NUM_MOTORS)
								{
									MyMotorInfo* motorInfo = &m_motorTargetState[m_numMotors];
									motorInfo->m_jointName = info.m_jointName;
									motorInfo->m_velTarget = 0.f;
									motorInfo->m_posTarget = 0.f;
									motorInfo->m_uIndex = info.m_uIndex;
									motorInfo->m_posIndex  = info.m_qIndex;
									motorInfo->m_jointIndex = jointIndex;
                                    sensorCommand.m_createSensorArguments.m_sensorType[sensorCommand.m_createSensorArguments.m_numJointSensorChanges] = SENSOR_FORCE_TORQUE;
									sensorCommand.m_createSensorArguments.m_jointIndex[sensorCommand.m_createSensorArguments.m_numJointSensorChanges] = jointIndex;
									sensorCommand.m_createSensorArguments.m_enableJointForceSensor[sensorCommand.m_createSensorArguments.m_numJointSensorChanges] = true;
									sensorCommand.m_createSensorArguments.m_numJointSensorChanges++;
									m_numMotors++;
								}
							}



							 break;
						}
						default:
							{
								drx3DWarning("Неизвестный режим контроля в RobotControlExample::stepSimulation");
							}
						};
                    }
                }

			}


			if (sensorCommand.m_createSensorArguments.m_numJointSensorChanges)
			{
				enqueueCommand(sensorCommand);
			}

		}



		if (m_physicsClient.canSubmitCommand())
		{
			if (m_userCommandRequests.size())
			{
				if (m_verboseOutput)
				{
					drx3DPrintf("Outstanding user command requests: %d\n", m_userCommandRequests.size());
				}
				SharedMemoryCommand cmd = m_userCommandRequests[0];

				//a manual 'pop_front', we don't use 'remove' because it will re-order the commands
				for (i32 i=1;i<m_userCommandRequests.size();i++)
				{
					m_userCommandRequests[i-1] = m_userCommandRequests[i];
				}

				m_userCommandRequests.pop_back();
				if (cmd.m_type == CMD_CREATE_SENSOR)
				{
					drx3DPrintf("CMD_CREATE_SENSOR!\n");
				}
				if (cmd.m_type == CMD_SEND_DRX3D_DATA_STREAM)
				{
					char relativeFileName[1024];

					bool fileFound = ResourcePath::findResourcePath(cmd.m_dataStreamArguments.m_bulletFileName,relativeFileName,1024);
					if (fileFound)
					{
						FILE *fp = fopen(relativeFileName, "rb");
						if (fp)
						{
							fseek(fp, 0L, SEEK_END);
							i32 mFileLen = ftell(fp);
							fseek(fp, 0L, SEEK_SET);
							if (mFileLen<SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE)
							{
								tuk data = (tuk)malloc(mFileLen);

								fread(data, mFileLen, 1, fp);
								fclose(fp);
								cmd.m_dataStreamArguments.m_streamChunkLength = mFileLen;
								m_physicsClient.uploadBulletFileToSharedMemory(data,mFileLen);
								if (m_verboseOutput)
								{
									drx3DPrintf("Loaded bullet data chunks into shared memory\n");
								}
								free(data);
							} else
							{
								drx3DWarning("Размер файла drx3D (%d) превышает размер стримингового куска памяти (%d)\n", mFileLen,SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE);
							}
						} else
						{
							drx3DWarning("Не удаётся открыть файл %s\n", relativeFileName);
						}
					} else
					{
						drx3DWarning("Не найден файл %s\n", cmd.m_dataStreamArguments.m_bulletFileName);
					}

				}

				m_physicsClient.submitClientCommand(cmd);
			} else
			{

				if (m_numMotors)
				{
					SharedMemoryCommand command;
					command.m_type =CMD_SEND_DESIRED_STATE;
					prepareControlCommand(command);
					enqueueCommand(command);

					command.m_type =CMD_STEP_FORWARD_SIMULATION;
					enqueueCommand(command);

					command.m_type = CMD_REQUEST_ACTUAL_STATE;
					enqueueCommand(command);
				}

			}
		}
	}
}

extern i32 gSharedMemoryKey;

class CommonExampleInterface*    RobotControlExampleCreateFunc(struct CommonExampleOptions& options)
{
	RobotControlExample* example = new RobotControlExample(options.m_guiHelper, options.m_option);
	if (gSharedMemoryKey>=0)
	{
		example->setSharedMemoryKey(gSharedMemoryKey);
	}
	return example;
}
#endif
