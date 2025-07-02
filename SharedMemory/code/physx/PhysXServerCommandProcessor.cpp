#ifdef DRX3D_ENABLE_PHYSX
#include "PhysXServerCommandProcessor.h"

#include "../../Utils/ChromeTraceUtil.h"

#include <stdio.h>
#include "../SharedMemoryCommands.h"
#include <drx3D/Maths/Linear/Quickprof.h"
#include <drx3D/Common/b3AlignedObjectArray.h"
#include <drx3D/Maths/Linear/MinMax.h"
#include <drx3D/Common/b3FileUtils.h"
#include <drx3D/Common/b3CommandLineArgs.h"
#include "../../Utils/ResourcePath.h"
#include <drx3D/Common/b3ResizablePool.h"
#include "PxPhysicsAPI.h"
#include <drx3D/Common/DefaultFileIO.h>
#include "PhysXUrdfImporter.h"
#include "PxTolerancesScale.h"
#include "PxDefaultCpuDispatcher.h"
#include "PxDefaultSimulationFilterShader.h"
#include "URDF2PhysX.h"
#include "../b3PluginManager.h"
#include "PxRigidActorExt.h"
#include <drx3D/Maths/Linear/Threads.h"

#define STATIC_EGLRENDERER_PLUGIN
#ifdef STATIC_EGLRENDERER_PLUGIN
#include "../plugins/eglPlugin/eglRendererPlugin.h"
#endif  //STATIC_EGLRENDERER_PLUGIN

//for serialization of data to client
#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h"
#include <drx3D/Extras/Serialize/BulletFileLoader/BulletFile.h"
#include <drx3D/Maths/Linear/Serializer.h"
#include "PhysXUserData.h"

class MyPhysXErrorCallback : public physx::PxErrorCallback
{
public:
	MyPhysXErrorCallback()
	{
	}
	~MyPhysXErrorCallback()
	{
	}

	virtual void reportError(physx::PxErrorCode::Enum code, tukk message, tukk file, i32 line)
	{
		drx3DPrintf("%s in file:%s line:%d\n", message, file, line);
	}
};

class CustomProfilerCallback : public physx::PxProfilerCallback
{
public:
	virtual ~CustomProfilerCallback() {}

	virtual uk zoneStart(tukk eventName, bool detached, uint64_t contextId)
	{
		b3EnterProfileZone(eventName);
		return 0;
	}

	virtual void zoneEnd(uk profilerData, tukk eventName, bool detached, uint64_t contextId)
	{
		b3LeaveProfileZone();
	}

};

static CustomProfilerCallback gCustomProfilerCallback;


struct InternalPhysXBodyData
{
	physx::PxArticulationReducedCoordinate*		mArticulation;
	physx::PxRigidDynamic* m_rigidDynamic;
	physx::PxRigidStatic* m_rigidStatic;

	STxt m_bodyName;

	InternalPhysXBodyData()
	{
		clear();
	}
	//physx::PxArticulationJointReducedCoordinate*	gDriveJoint;
	void clear()
	{
		mArticulation = 0;
		m_rigidDynamic = 0;
		m_rigidStatic = 0;

		m_bodyName = "";
	}
};



typedef b3PoolBodyHandle<InternalPhysXBodyData> InternalPhysXBodyHandle;



struct PhysXServerCommandProcessorInternalData : public physx::PxSimulationEventCallback, public physx::PxContactModifyCallback
{
	bool m_isConnected;
	bool m_verboseOutput;
	double m_physicsDeltaTime;
	i32 m_numSimulationSubSteps;
	SpinMutex m_taskLock;

	AlignedObjectArray<b3ContactPointData> m_contactPoints;
	
	

	void onContactModify(physx::PxContactModifyPair* const pairs, physx::PxU32 count)
	{
		for (physx::PxU32 i = 0; i<count; i++)
		{
			//...
		}
	}

	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
	{
		D3_PROFILE("onContact");
		//todo: are there really multiple threads calling 'onContact'?
		m_taskLock.lock();

		AlignedObjectArray<physx::PxContactPairPoint> contacts;
		for (physx::PxU32 i = 0; i < nbPairs; i++)
		{
			physx::PxU32 contactCount = pairs[i].contactCount;
			if (contactCount)
			{
				contacts.resize(contactCount);
				pairs[i].extractContacts(&contacts[0], contactCount);
				for (physx::PxU32 j = 0; j < contactCount; j++)
				{
					const physx::PxContactPairPoint& contact = contacts[i];
					b3ContactPointData srcPt;
					MyPhysXUserData* udA = (MyPhysXUserData*)pairHeader.actors[0]->userData;
					MyPhysXUserData* udB = (MyPhysXUserData*)pairHeader.actors[1]->userData;
					srcPt.m_bodyUniqueIdA = udA->m_bodyUniqueId;
					srcPt.m_linkIndexA = udA->m_linkIndex;
					srcPt.m_bodyUniqueIdB = udB->m_bodyUniqueId;
					srcPt.m_linkIndexB = udB->m_linkIndex;
					srcPt.m_positionOnAInWS[0] = contact.position.x + contact.separation*contact.normal.x;
					srcPt.m_positionOnAInWS[1] = contact.position.y + contact.separation*contact.normal.y;
					srcPt.m_positionOnAInWS[2] = contact.position.z + contact.separation*contact.normal.z;
					srcPt.m_positionOnBInWS[0] = contact.position.x - contact.separation*contact.normal.x;
					srcPt.m_positionOnBInWS[1] = contact.position.y - contact.separation*contact.normal.y;
					srcPt.m_positionOnBInWS[2] = contact.position.z - contact.separation*contact.normal.z;
					srcPt.m_contactNormalOnBInWS[0] = contact.normal.x;
					srcPt.m_contactNormalOnBInWS[1] = contact.normal.y;
					srcPt.m_contactNormalOnBInWS[2] = contact.normal.z;
					srcPt.m_contactDistance = contact.separation;
					srcPt.m_contactFlags = 0;
					srcPt.m_linearFrictionDirection1[0] = 0;
					srcPt.m_linearFrictionDirection1[1] = 0;
					srcPt.m_linearFrictionDirection1[2] = 0;
					srcPt.m_linearFrictionDirection2[0] = 0;
					srcPt.m_linearFrictionDirection2[1] = 0;
					srcPt.m_linearFrictionDirection2[2] = 0;
					
					srcPt.m_linearFrictionForce2 = 0;
					
					srcPt.m_normalForce = contact.impulse.dot(contact.normal);
					//compute friction direction from impulse projected in contact plane using contact normal.
					physx::PxVec3 fric = contact.impulse - contact.normal*srcPt.m_normalForce;
					double fricForce = fric.normalizeSafe();
					if (fricForce)
					{
						srcPt.m_linearFrictionDirection1[0] = fric.x;
						srcPt.m_linearFrictionDirection1[1] = fric.y;
						srcPt.m_linearFrictionDirection1[2] = fric.z;
						srcPt.m_linearFrictionForce1 = fricForce;
					}
					m_contactPoints.push_back(srcPt);
					// std::cout << "Contact: bw " << pairHeader.actors[0]->getName() << " and " << pairHeader.actors[1]->getName() << " | " << contacts[j].position.x << "," << contacts[j].position.y << ","
					// 		  << contacts[j].position.z << std::endl;
				}
			}
		}
		m_taskLock.unlock();
	}

	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
	{
		PX_UNUSED(constraints);
		PX_UNUSED(count);
	}

	void onWake(physx::PxActor** actors, physx::PxU32 count)
	{
		PX_UNUSED(actors);
		PX_UNUSED(count);
	}

	void onSleep(physx::PxActor** actors, physx::PxU32 count)
	{
		PX_UNUSED(actors);
		PX_UNUSED(count);
	}

	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		PX_UNUSED(pairs);
		PX_UNUSED(count);
	}

	void onAdvance(const physx::PxRigidBody* const*, const physx::PxTransform*, const physx::PxU32) 
	{
	}
	

	b3PluginManager m_pluginManager;


	physx::PxDefaultAllocator		m_allocator;
	MyPhysXErrorCallback	m_errorCallback;
	physx::PxFoundation*			m_foundation;
	physx::PxPhysics*				m_physics;
	physx::PxCooking*		m_cooking;
	physx::PxDefaultCpuDispatcher*	m_dispatcher;
	physx::PxScene*				m_scene;
	physx::PxMaterial*				m_material;
	//physx::PxPvd*                  m_pvd;

	b3ResizablePool<InternalPhysXBodyHandle> m_bodyHandles;
	



	b3AlignedObjectArray<i32> m_mjcfRecentLoadedBodies;

	i32 m_profileTimingLoggingUid;
	i32 m_stateLoggersUniqueId;
	STxt m_profileTimingFileName;
	b3CommandLineArgs	m_commandLineArgs;
	i32 m_userDebugParametersUid;
	HashMap<HashInt, double> m_userDebugParameters;
	AlignedObjectArray<i32> m_graphicsIndexToSegmentationMask;

	PhysXServerCommandProcessorInternalData(PhysXServerCommandProcessor* sdk, i32 argc, tuk argv[])
		: m_isConnected(false),
		  m_verboseOutput(false),
		  m_physicsDeltaTime(1. / 240.),
		  m_numSimulationSubSteps(0),
		m_pluginManager(sdk),
		m_profileTimingLoggingUid(-1),
		m_stateLoggersUniqueId(1),
		m_commandLineArgs(argc,argv),
		m_userDebugParametersUid(0)
	{

		m_foundation = NULL;
		m_physics = NULL;
		m_cooking = NULL;
		m_dispatcher = NULL;
		m_scene = NULL;
		m_material = NULL;
		//m_pvd = NULL;

#ifdef STATIC_EGLRENDERER_PLUGIN
		{
			bool initPlugin = false;
			b3PluginFunctions funcs(initPlugin_eglRendererPlugin, exitPlugin_eglRendererPlugin, executePluginCommand_eglRendererPlugin);
			funcs.m_getRendererFunc = getRenderInterface_eglRendererPlugin;
			i32 renderPluginId = m_pluginManager.registerStaticLinkedPlugin("eglRendererPlugin", funcs, initPlugin);
			m_pluginManager.selectPluginRenderer(renderPluginId);
		}
#endif  //STATIC_EGLRENDERER_PLUGIN
	}
};

PhysXServerCommandProcessor::PhysXServerCommandProcessor(i32 argc, tuk argv[])
{

	m_data = new PhysXServerCommandProcessorInternalData(this, argc, argv);
}

PhysXServerCommandProcessor::~PhysXServerCommandProcessor()
{
	delete m_data;
}



physx::PxFilterFlags MyPhysXFilter(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, ukk constantBlock, physx::PxU32 constantBlockSize)
{
	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);
	PX_UNUSED(constantBlock);
	PX_UNUSED(constantBlockSize);
	// if (filterData0.word2 != 0 && filterData0.word2 == filterData1.word2)
	// 	return physx::PxFilterFlag::eKILL;
	pairFlags |= physx::PxPairFlag::eCONTACT_DEFAULT | physx::PxPairFlag::eNOTIFY_TOUCH_FOUND
		| physx::PxPairFlag::eDETECT_DISCRETE_CONTACT | physx::PxPairFlag::eNOTIFY_CONTACT_POINTS | physx::PxPairFlag::eMODIFY_CONTACTS;
	return physx::PxFilterFlag::eDEFAULT;
}



bool PhysXServerCommandProcessor::connect()
{
	if (m_data->m_isConnected)
	{
		printf("already connected\n");
		return true;
	}

	i32 result = 0;
	{
		
		m_data->m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_data->m_allocator, m_data->m_errorCallback);
		// This call should be performed after PVD is initialized, otherwise it will have no effect.
		PxSetProfilerCallback(&gCustomProfilerCallback);

		m_data->m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_data->m_foundation, physx::PxTolerancesScale(), true, 0);
		m_data->m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_data->m_foundation, physx::PxCookingParams(physx::PxTolerancesScale()));

		
		physx::PxU32 numCores = 1;
		m_data->m_commandLineArgs.GetCmdLineArgument("numCores", numCores);
		printf("PhysX numCores=%d\n", numCores);
		m_data->m_dispatcher = physx::PxDefaultCpuDispatcherCreate(numCores == 0 ? 0 : numCores - 1);
		
		physx::PxSceneDesc sceneDesc(m_data->m_physics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);

		
		sceneDesc.solverType = physx::PxSolverType::ePGS;
		STxt solver;
		m_data->m_commandLineArgs.GetCmdLineArgument("solver", solver);
		
		if (solver=="tgs")
		{
			sceneDesc.solverType = physx::PxSolverType::eTGS;
			printf("PhysX using TGS\n");
		}
		else
		{
			printf("PhysX using PGS\n");
		}
		
		sceneDesc.cpuDispatcher = m_data->m_dispatcher;
		
		//todo: add some boolean, to allow enable/disable of this contact filtering
		bool enableContactCallback = false;
		m_data->m_commandLineArgs.GetCmdLineArgument("enableContactCallback", enableContactCallback);

		if (enableContactCallback)
		{
			sceneDesc.filterShader = MyPhysXFilter;
			sceneDesc.simulationEventCallback = this->m_data;
			sceneDesc.contactModifyCallback = this->m_data;
			printf("PhysX enableContactCallback\n");
		}
		else
		{
			sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		}
		
		
		
		
		m_data->m_scene = m_data->m_physics->createScene(sceneDesc);

		m_data->m_material = m_data->m_physics->createMaterial(0.5f, 0.5f, 0.f);

		PxInitExtensions(*m_data->m_physics, 0);


		//PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
		//gScene->addActor(*groundPlane);

		result = 1;
	}
	if (result == 1)
	{
		m_data->m_isConnected = true;
		return true;
	}

	return false;
}

void PhysXServerCommandProcessor::resetSimulation()
{
	//gArticulation->release();
	m_data->m_scene->release();
	m_data->m_dispatcher->release();
	m_data->m_cooking->release();
	m_data->m_physics->release();
	//PxPvdTransport* transport = gPvd->getTransport();
	//gPvd->release();
	//transport->release();
	PxCloseExtensions();
	
	m_data->m_foundation->release();
}

void PhysXServerCommandProcessor::disconnect()
{
	resetSimulation();

	m_data->m_isConnected = false;
}

bool PhysXServerCommandProcessor::isConnected() const
{
	return m_data->m_isConnected;
}


bool PhysXServerCommandProcessor::processCustomCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;

	SharedMemoryStatus& serverCmd = serverStatusOut;
	serverCmd.m_type = CMD_CUSTOM_COMMAND_FAILED;
	serverCmd.m_customCommandResultArgs.m_pluginUniqueId = -1;

	if (clientCmd.m_updateFlags & CMD_CUSTOM_COMMAND_LOAD_PLUGIN)
	{
		//pluginPath could be registered or load from disk
		tukk postFix = "";
		if (clientCmd.m_updateFlags & CMD_CUSTOM_COMMAND_LOAD_PLUGIN_POSTFIX)
		{
			postFix = clientCmd.m_customCommandArgs.m_postFix;
		}

		i32 pluginUniqueId = m_data->m_pluginManager.loadPlugin(clientCmd.m_customCommandArgs.m_pluginPath, postFix);
		if (pluginUniqueId >= 0)
		{
			serverCmd.m_customCommandResultArgs.m_pluginUniqueId = pluginUniqueId;
			serverCmd.m_type = CMD_CUSTOM_COMMAND_COMPLETED;
		}
	}
	if (clientCmd.m_updateFlags & CMD_CUSTOM_COMMAND_UNLOAD_PLUGIN)
	{
		m_data->m_pluginManager.unloadPlugin(clientCmd.m_customCommandArgs.m_pluginUniqueId);
		serverCmd.m_type = CMD_CUSTOM_COMMAND_COMPLETED;
	}
	if (clientCmd.m_updateFlags & CMD_CUSTOM_COMMAND_EXECUTE_PLUGIN_COMMAND)
	{
		i32 result = m_data->m_pluginManager.executePluginCommand(clientCmd.m_customCommandArgs.m_pluginUniqueId, &clientCmd.m_customCommandArgs.m_arguments);
		serverCmd.m_customCommandResultArgs.m_executeCommandResult = result;
		serverCmd.m_type = CMD_CUSTOM_COMMAND_COMPLETED;
	}
	return hasStatus;
}


bool PhysXServerCommandProcessor::processStateLoggingCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	DRX3D_PROFILE("CMD_STATE_LOGGING");

	serverStatusOut.m_type = CMD_STATE_LOGGING_FAILED;
	bool hasStatus = true;

	if (clientCmd.m_stateLoggingArguments.m_logType == STATE_LOGGING_PROFILE_TIMINGS)
	{
		if (m_data->m_profileTimingLoggingUid < 0)
		{
			b3ChromeUtilsStartTimings();
			m_data->m_profileTimingFileName = clientCmd.m_stateLoggingArguments.m_fileName;
			i32 loggerUid = m_data->m_stateLoggersUniqueId++;
			serverStatusOut.m_type = CMD_STATE_LOGGING_START_COMPLETED;
			serverStatusOut.m_stateLoggingResultArgs.m_loggingUniqueId = loggerUid;
			m_data->m_profileTimingLoggingUid = loggerUid;
		}
	}

	if ((clientCmd.m_updateFlags & STATE_LOGGING_STOP_LOG) && clientCmd.m_stateLoggingArguments.m_loggingUniqueId >= 0)
	{
		if (clientCmd.m_stateLoggingArguments.m_loggingUniqueId == m_data->m_profileTimingLoggingUid)
		{
			serverStatusOut.m_type = CMD_STATE_LOGGING_COMPLETED;
			b3ChromeUtilsStopTimingsAndWriteJsonFile(m_data->m_profileTimingFileName.c_str());
			m_data->m_profileTimingLoggingUid = -1;
		}
	}


	return hasStatus;
}


bool PhysXServerCommandProcessor::processInitPoseCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;

	DRX3D_PROFILE("CMD_INIT_POSE");

	if (m_data->m_verboseOutput)
	{
		drx3DPrintf("Server Init Pose not implemented yet");
	}
	i32 bodyUniqueId = clientCmd.m_initPoseArgs.m_bodyUniqueId;
	InternalPhysXBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);

	Vec3 baseLinVel(0, 0, 0);
	Vec3 baseAngVel(0, 0, 0);

	if (clientCmd.m_updateFlags & INIT_POSE_HAS_BASE_LINEAR_VELOCITY)
	{
		baseLinVel.setVal(clientCmd.m_initPoseArgs.m_initialStateQdot[0],
			clientCmd.m_initPoseArgs.m_initialStateQdot[1],
			clientCmd.m_initPoseArgs.m_initialStateQdot[2]);
	}
	if (clientCmd.m_updateFlags & INIT_POSE_HAS_BASE_ANGULAR_VELOCITY)
	{
		baseAngVel.setVal(clientCmd.m_initPoseArgs.m_initialStateQdot[3],
			clientCmd.m_initPoseArgs.m_initialStateQdot[4],
			clientCmd.m_initPoseArgs.m_initialStateQdot[5]);
	}
	Vec3 basePos(0, 0, 0);
	if (clientCmd.m_updateFlags & INIT_POSE_HAS_INITIAL_POSITION)
	{
		basePos = Vec3(
			clientCmd.m_initPoseArgs.m_initialStateQ[0],
			clientCmd.m_initPoseArgs.m_initialStateQ[1],
			clientCmd.m_initPoseArgs.m_initialStateQ[2]);
	}
	Quat baseOrn(0, 0, 0, 1);
	if (clientCmd.m_updateFlags & INIT_POSE_HAS_INITIAL_ORIENTATION)
	{
		baseOrn.setVal(clientCmd.m_initPoseArgs.m_initialStateQ[3],
			clientCmd.m_initPoseArgs.m_initialStateQ[4],
			clientCmd.m_initPoseArgs.m_initialStateQ[5],
			clientCmd.m_initPoseArgs.m_initialStateQ[6]);
	}
	if (body && body->mArticulation)
	{
		physx::PxArticulationCache* c = body->mArticulation->createCache();
		body->mArticulation->copyInternalStateToCache(*c, physx::PxArticulationCache::ePOSITION | physx::PxArticulationCache::eVELOCITY);// physx::PxArticulationCache::eALL);
		physx::PxArticulationLink* physxLinks[64];
		physx::PxU32 bufferSize = 64;
		physx::PxU32 startIndex = 0;
		i32 numLinks2 = body->mArticulation->getLinks(physxLinks, bufferSize, startIndex);

		AlignedObjectArray<i32> dofStarts;
		dofStarts.resize(numLinks2);
		dofStarts[0] = 0; //We know that the root link does not have a joint
		//cache positions in PhysX may be reshuffled, see
		//http://gameworksdocs.nvidia.com/PhysX/4.0/documentation/PhysXGuide/Manual/Articulations.html

		for (i32  i = 1; i < numLinks2; ++i)
		{
			i32  llIndex = physxLinks[i]->getLinkIndex();
			i32  dofs = physxLinks[i]->getInboundJointDof();

			dofStarts[llIndex] = dofs;
		}

		i32 count = 0;
		for (i32 i = 1; i < numLinks2; ++i)
		{
			i32  dofs = dofStarts[i];
			dofStarts[i] = count;
			count += dofs;
		}

		if (numLinks2 > 0)
		{
			i32 dofs = physxLinks[0]->getInboundJointDof();
			physx::PxTransform pt = physxLinks[0]->getGlobalPose();
			physx::PxVec3 linVel = physxLinks[0]->getLinearVelocity();
			physx::PxVec3 angVel = physxLinks[0]->getAngularVelocity();
			

			if (clientCmd.m_updateFlags & INIT_POSE_HAS_BASE_LINEAR_VELOCITY)
			{
				physxLinks[0]->setLinearVelocity(physx::PxVec3(baseLinVel[0], baseLinVel[1], baseLinVel[2]));
			}

			if (clientCmd.m_updateFlags & INIT_POSE_HAS_BASE_ANGULAR_VELOCITY)
			{
				physxLinks[0]->setAngularVelocity(physx::PxVec3(baseAngVel[0], baseAngVel[1], baseAngVel[2]));
			}

			if (clientCmd.m_updateFlags & INIT_POSE_HAS_INITIAL_POSITION)
			{
				pt.p.x = basePos[0];
				pt.p.y = basePos[0];
				pt.p.z = basePos[0];
				physxLinks[0]->setGlobalPose(pt);
			}
			if (clientCmd.m_updateFlags & INIT_POSE_HAS_INITIAL_ORIENTATION)
			{
				Assert(clientCmd.m_initPoseArgs.m_hasInitialStateQ[3] &&
					clientCmd.m_initPoseArgs.m_hasInitialStateQ[4] &&
					clientCmd.m_initPoseArgs.m_hasInitialStateQ[5] &&
					clientCmd.m_initPoseArgs.m_hasInitialStateQ[6]);
				pt.q.x = baseOrn[0];
				pt.q.y = baseOrn[1];
				pt.q.z = baseOrn[2];
				pt.q.w = baseOrn[3];
				physxLinks[0]->setGlobalPose(pt);
			}
		}
		if (clientCmd.m_updateFlags & INIT_POSE_HAS_JOINT_STATE)
		{
			i32 uDofIndex = 6;
			i32 posVarCountIndex = 7;
			
			//skip 'root' link
			for (i32 i = 1; i < numLinks2; i++)
			{
				i32 physxCacheLinkIndex = physxLinks[i]->getLinkIndex();
				i32 dofs = physxLinks[i]->getInboundJointDof();
				i32 posVarCount = dofs;//??
				bool hasPosVar = posVarCount > 0;

				for (i32 j = 0; j < posVarCount; j++)
				{
					if (clientCmd.m_initPoseArgs.m_hasInitialStateQ[posVarCountIndex + j] == 0)
					{
						hasPosVar = false;
						break;
					}
				}
				if (hasPosVar)
				{
					if (posVarCount == 1)
					{
						c->jointPosition[dofStarts[physxCacheLinkIndex]] = clientCmd.m_initPoseArgs.m_initialStateQ[posVarCountIndex];
					}
					if (posVarCount == 3)
					{
						Quat q(
							clientCmd.m_initPoseArgs.m_initialStateQ[posVarCountIndex],
							clientCmd.m_initPoseArgs.m_initialStateQ[posVarCountIndex + 1],
							clientCmd.m_initPoseArgs.m_initialStateQ[posVarCountIndex + 2],
							clientCmd.m_initPoseArgs.m_initialStateQ[posVarCountIndex + 3]);
						q.normalize();
						//mb->setJointPosMultiDof(i, &q[0]);
						//double vel[6] = { 0, 0, 0, 0, 0, 0 };
						//mb->setJointVelMultiDof(i, vel);
					}
				}

				bool hasVel = true;
				for (i32 j = 0; j < posVarCount; j++)
				{
					if (clientCmd.m_initPoseArgs.m_hasInitialStateQdot[uDofIndex + j] == 0)
					{
						hasVel = false;
						break;
					}
				}

				if (hasVel)
				{
					if (posVarCount == 1)
					{
						Scalar vel = clientCmd.m_initPoseArgs.m_initialStateQdot[uDofIndex];
						c->jointVelocity[dofStarts[physxCacheLinkIndex]] = vel;
					}
					if (posVarCount == 3)
					{
						//mb->setJointVelMultiDof(i, &clientCmd.m_initPoseArgs.m_initialStateQdot[uDofIndex]);
					}
				}
				
				posVarCountIndex += dofs;
				uDofIndex += dofs;// mb->getLink(i).m_dofCount;
			}
		}

		body->mArticulation->applyCache(*c, physx::PxArticulationCache::ePOSITION| physx::PxArticulationCache::eVELOCITY);
		body->mArticulation->releaseCache(*c);
		
	}

	SharedMemoryStatus& serverCmd = serverStatusOut;
	serverCmd.m_type = CMD_CLIENT_COMMAND_COMPLETED;
	return hasStatus;
}

bool PhysXServerCommandProcessor::processSendDesiredStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;
	DRX3D_PROFILE("CMD_SEND_DESIRED_STATE");
	
	i32 bodyUniqueId = clientCmd.m_sendDesiredStateCommandArgument.m_bodyUniqueId;
	InternalPhysXBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);
	if (body && body->mArticulation)
	{
		physx::PxArticulationLink* physxLinks[64];
		physx::PxU32 bufferSize = 64;
		physx::PxU32 startIndex = 0;
		i32 numLinks2 = body->mArticulation->getLinks(physxLinks, bufferSize, startIndex);
		//http://gameworksdocs.nvidia.com/PhysX/4.0/documentation/PhysXGuide/Manual/Articulations.html


		switch (clientCmd.m_sendDesiredStateCommandArgument.m_controlMode)
		{
		case CONTROL_MODE_VELOCITY:
		{
			if (m_data->m_verboseOutput)
			{
				drx3DPrintf("Using CONTROL_MODE_VELOCITY");
			}

			i32 numMotors = 0;
			//find the joint motors and apply the desired velocity and maximum force/torque
			{
				i32 dofIndex = 6;  //skip the 3 linear + 3 angular degree of freedom entries of the base
				for (i32 link = 1; link < numLinks2; link++)
				{
					i32 dofs = physxLinks[link]->getInboundJointDof();
					physx::PxReal stiffness = 10.f;
					physx::PxReal damping = 0.1f;
					physx::PxReal forceLimit = PX_MAX_F32;

					if (dofs == 1)
					{
						physx::PxArticulationJointReducedCoordinate* joint = static_cast<physx::PxArticulationJointReducedCoordinate*>(physxLinks[link]->getInboundJoint());
						Scalar desiredVelocity = 0.f;
						bool hasDesiredVelocity = false;
						physx::PxReal stiffness = 10.f;

						if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] & SIM_DESIRED_STATE_HAS_QDOT) != 0)
						{
							desiredVelocity = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQdot[dofIndex];
							Scalar kd = 0.1f;
							if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] & SIM_DESIRED_STATE_HAS_KD) != 0)
							{
								kd = clientCmd.m_sendDesiredStateCommandArgument.m_Kd[dofIndex];
							}
							joint->setDriveVelocity(physx::PxArticulationAxis::eTWIST, desiredVelocity);
							physx::PxReal damping = kd;
							stiffness = 0;
							joint->setDriveTarget(physx::PxArticulationAxis::eTWIST, 0.f);
							physx::PxReal forceLimit = 1000000.f;
							if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] & SIM_DESIRED_STATE_HAS_MAX_FORCE) != 0)
							{
								forceLimit = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[dofIndex];
							}
							joint->setDrive(physx::PxArticulationAxis::eTWIST, stiffness, damping, forceLimit);
						}
					}
					dofIndex += dofs;
				}
			}
			break;
		}

		case CONTROL_MODE_POSITION_VELOCITY_PD:
		{
			if (m_data->m_verboseOutput)
			{
				drx3DPrintf("Using CONTROL_MODE_VELOCITY");
			}

			i32 numMotors = 0;
			//find the joint motors and apply the desired velocity and maximum force/torque
			{
				i32 dofIndex = 6;  //skip the 3 linear + 3 angular degree of freedom entries of the base
				i32 posIndex = 7;  //skip 3 positional and 4 orientation (quaternion) positional degrees of freedom of the base
				for (i32 link = 1; link < numLinks2; link++)
				{
					i32 dofs = physxLinks[link]->getInboundJointDof();
					physx::PxReal stiffness = 10.f;
					physx::PxReal damping = 0.1f;
					physx::PxReal forceLimit = PX_MAX_F32;
					

					if (dofs == 1)
					{
						physx::PxArticulationJointReducedCoordinate* joint = static_cast<physx::PxArticulationJointReducedCoordinate*>(physxLinks[link]->getInboundJoint());
						Scalar desiredVelocity = 0.f;
						bool hasDesiredVelocity = false;
						physx::PxReal stiffness = 10.f;
						Scalar kd = 0.1f;
						Scalar kp = 0.f;
						
						if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] & SIM_DESIRED_STATE_HAS_QDOT) != 0)
						{
							desiredVelocity = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQdot[dofIndex];
						}

						Scalar desiredPosition = 0.f;
						if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[posIndex] & SIM_DESIRED_STATE_HAS_Q) != 0)
						{
							desiredPosition = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQ[posIndex];


							if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] & SIM_DESIRED_STATE_HAS_KD) != 0)
							{
								kd = clientCmd.m_sendDesiredStateCommandArgument.m_Kd[dofIndex];
							}
							physx::PxReal damping = kd;

							if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] & SIM_DESIRED_STATE_HAS_KP) != 0)
							{
								kp = clientCmd.m_sendDesiredStateCommandArgument.m_Kp[dofIndex];
								stiffness = kp;
							}

							joint->setDriveVelocity(physx::PxArticulationAxis::eTWIST, desiredVelocity);



							physx::PxReal forceLimit = 1000000.f;
							if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] & SIM_DESIRED_STATE_HAS_MAX_FORCE) != 0)
							{
								forceLimit = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[dofIndex];
							}
							bool isAcceleration = false;

							joint->setDriveTarget(physx::PxArticulationAxis::eTWIST, desiredPosition);
							joint->setDrive(physx::PxArticulationAxis::eTWIST, stiffness, damping, forceLimit, isAcceleration);
						}
					}

					dofIndex += dofs;
					posIndex += dofs;
				}
			}
			break;
		}
		default:
		{
		}
		}

	}
	serverStatusOut.m_type = CMD_DESIRED_STATE_RECEIVED_COMPLETED;
	return hasStatus;
}



bool PhysXServerCommandProcessor::processChangeDynamicsInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;
	DRX3D_PROFILE("CMD_CHANGE_DYNAMICS_INFO");

	i32 bodyUniqueId = clientCmd.m_changeDynamicsInfoArgs.m_bodyUniqueId;
	i32 linkIndex = clientCmd.m_changeDynamicsInfoArgs.m_linkIndex;
	double mass = clientCmd.m_changeDynamicsInfoArgs.m_mass;
	double lateralFriction = clientCmd.m_changeDynamicsInfoArgs.m_lateralFriction;
	double spinningFriction = clientCmd.m_changeDynamicsInfoArgs.m_spinningFriction;
	double rollingFriction = clientCmd.m_changeDynamicsInfoArgs.m_rollingFriction;
	double restitution = clientCmd.m_changeDynamicsInfoArgs.m_restitution;
	Vec3 newLocalInertiaDiagonal(clientCmd.m_changeDynamicsInfoArgs.m_localInertiaDiagonal[0],
		clientCmd.m_changeDynamicsInfoArgs.m_localInertiaDiagonal[1],
		clientCmd.m_changeDynamicsInfoArgs.m_localInertiaDiagonal[2]);

	Assert(bodyUniqueId >= 0);

	InternalPhysXBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);

	if (body->mArticulation)
	{

		physx::PxArticulationLink* physxLinks[64];
		physx::PxU32 bufferSize = 64;
		physx::PxU32 startIndex = 0;
		i32 physxLinkIndex = linkIndex + 1;
		
		i32 numLinks2 = body->mArticulation->getLinks(physxLinks, bufferSize, startIndex);

		if (physxLinkIndex >= 0 && physxLinkIndex < numLinks2)
		{
			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_MASS)
			{
				physx::PxArticulationLink* childLink = physxLinks[physxLinkIndex];
				physx::PxRigidBodyExt::updateMassAndInertia(*childLink, mass);
			}
		}
	}
	if (body->m_rigidDynamic)
	{
		//body->m_rigidDynamic->setMass(mass);
		//also update inertia
		if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_MASS)
		{
			physx::PxRigidBodyExt::updateMassAndInertia(*body->m_rigidDynamic, mass);
		}
	}
	if (body->m_rigidStatic)
	{
		
	}
#if 0
	if (body && body->m_multiBody)
	{
		btMultiBody* mb = body->m_multiBody;

		if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ACTIVATION_STATE)
		{
			if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateWakeUp)
			{
				mb->wakeUp();
			}
			if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateSleep)
			{
				mb->goToSleep();
			}
			if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateEnableSleeping)
			{
				mb->setCanSleep(true);
			}
			if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateDisableSleeping)
			{
				mb->setCanSleep(false);
			}
		}

		if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LINEAR_DAMPING)
		{
			mb->setLinearDamping(clientCmd.m_changeDynamicsInfoArgs.m_linearDamping);
		}
		if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ANGULAR_DAMPING)
		{
			mb->setAngularDamping(clientCmd.m_changeDynamicsInfoArgs.m_angularDamping);
		}

		if (linkIndex == -1)
		{
			if (mb->getBaseCollider())
			{
				if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_RESTITUTION)
				{
					mb->getBaseCollider()->setRestitution(restitution);
				}

				if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_CONTACT_STIFFNESS_AND_DAMPING)
				{
					mb->getBaseCollider()->setContactStiffnessAndDamping(clientCmd.m_changeDynamicsInfoArgs.m_contactStiffness, clientCmd.m_changeDynamicsInfoArgs.m_contactDamping);
				}
				if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LATERAL_FRICTION)
				{
					mb->getBaseCollider()->setFriction(lateralFriction);
				}
				if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_SPINNING_FRICTION)
				{
					mb->getBaseCollider()->setSpinningFriction(spinningFriction);
				}
				if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ROLLING_FRICTION)
				{
					mb->getBaseCollider()->setRollingFriction(rollingFriction);
				}

				if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_FRICTION_ANCHOR)
				{
					if (clientCmd.m_changeDynamicsInfoArgs.m_frictionAnchor)
					{
						mb->getBaseCollider()->setCollisionFlags(mb->getBaseCollider()->getCollisionFlags() | CollisionObject2::CF_HAS_FRICTION_ANCHOR);
					}
					else
					{
						mb->getBaseCollider()->setCollisionFlags(mb->getBaseCollider()->getCollisionFlags() & ~CollisionObject2::CF_HAS_FRICTION_ANCHOR);
					}
				}
			}
			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_MASS)
			{
				mb->setBaseMass(mass);
				if (mb->getBaseCollider() && mb->getBaseCollider()->getCollisionShape())
				{
					Vec3 localInertia;
					mb->getBaseCollider()->getCollisionShape()->calculateLocalInertia(mass, localInertia);
					mb->setBaseInertia(localInertia);
				}

				//handle switch from static/fixedBase to dynamic and vise-versa
				if (mass > 0)
				{
					bool isDynamic = true;
					if (mb->hasFixedBase())
					{
						i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
						i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);

						m_data->m_dynamicsWorld->removeCollisionObject(mb->getBaseCollider());
						i32 oldFlags = mb->getBaseCollider()->getCollisionFlags();
						mb->getBaseCollider()->setCollisionFlags(oldFlags & ~CollisionObject2::CF_STATIC_OBJECT);
						mb->setFixedBase(false);
						m_data->m_dynamicsWorld->addCollisionObject(mb->getBaseCollider(), collisionFilterGroup, collisionFilterMask);

					}
				}
				else
				{
					if (!mb->hasFixedBase())
					{
						bool isDynamic = false;
						i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
						i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);
						i32 oldFlags = mb->getBaseCollider()->getCollisionFlags();
						mb->getBaseCollider()->setCollisionFlags(oldFlags | CollisionObject2::CF_STATIC_OBJECT);
						m_data->m_dynamicsWorld->removeCollisionObject(mb->getBaseCollider());
						mb->setFixedBase(true);
						m_data->m_dynamicsWorld->addCollisionObject(mb->getBaseCollider(), collisionFilterGroup, collisionFilterMask);
					}
				}

			}
			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LOCAL_INERTIA_DIAGONAL)
			{
				mb->setBaseInertia(newLocalInertiaDiagonal);
			}
		}
		else
		{
			if (linkIndex >= 0 && linkIndex < mb->getNumLinks())
			{
				if (mb->getLinkCollider(linkIndex))
				{
					if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_RESTITUTION)
					{
						mb->getLinkCollider(linkIndex)->setRestitution(restitution);
					}
					if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_SPINNING_FRICTION)
					{
						mb->getLinkCollider(linkIndex)->setSpinningFriction(spinningFriction);
					}
					if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ROLLING_FRICTION)
					{
						mb->getLinkCollider(linkIndex)->setRollingFriction(rollingFriction);
					}

					if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_FRICTION_ANCHOR)
					{
						if (clientCmd.m_changeDynamicsInfoArgs.m_frictionAnchor)
						{
							mb->getLinkCollider(linkIndex)->setCollisionFlags(mb->getLinkCollider(linkIndex)->getCollisionFlags() | CollisionObject2::CF_HAS_FRICTION_ANCHOR);
						}
						else
						{
							mb->getLinkCollider(linkIndex)->setCollisionFlags(mb->getLinkCollider(linkIndex)->getCollisionFlags() & ~CollisionObject2::CF_HAS_FRICTION_ANCHOR);
						}
					}

					if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LATERAL_FRICTION)
					{
						mb->getLinkCollider(linkIndex)->setFriction(lateralFriction);
					}

					if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_CONTACT_STIFFNESS_AND_DAMPING)
					{
						mb->getLinkCollider(linkIndex)->setContactStiffnessAndDamping(clientCmd.m_changeDynamicsInfoArgs.m_contactStiffness, clientCmd.m_changeDynamicsInfoArgs.m_contactDamping);
					}
				}
				if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_MASS)
				{
					mb->getLink(linkIndex).m_mass = mass;
					if (mb->getLinkCollider(linkIndex) && mb->getLinkCollider(linkIndex)->getCollisionShape())
					{
						Vec3 localInertia;
						mb->getLinkCollider(linkIndex)->getCollisionShape()->calculateLocalInertia(mass, localInertia);
						mb->getLink(linkIndex).m_inertiaLocal = localInertia;
					}
				}
				if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LOCAL_INERTIA_DIAGONAL)
				{
					mb->getLink(linkIndex).m_inertiaLocal = newLocalInertiaDiagonal;
				}
			}
		}
	}
	else
	{
		if (body && body->m_rigidBody)
		{
			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ACTIVATION_STATE)
			{
				if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateEnableSleeping)
				{
					body->m_rigidBody->forceActivationState(ACTIVE_TAG);
				}
				if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateDisableSleeping)
				{
					body->m_rigidBody->forceActivationState(DISABLE_DEACTIVATION);
				}
				if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateWakeUp)
				{
					body->m_rigidBody->forceActivationState(ACTIVE_TAG);
				}
				if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateSleep)
				{
					body->m_rigidBody->forceActivationState(ISLAND_SLEEPING);
				}
			}

			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LINEAR_DAMPING)
			{
				Scalar angDamping = body->m_rigidBody->getAngularDamping();
				body->m_rigidBody->setDamping(clientCmd.m_changeDynamicsInfoArgs.m_linearDamping, angDamping);
			}
			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ANGULAR_DAMPING)
			{
				Scalar linDamping = body->m_rigidBody->getLinearDamping();
				body->m_rigidBody->setDamping(linDamping, clientCmd.m_changeDynamicsInfoArgs.m_angularDamping);
			}

			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_CONTACT_STIFFNESS_AND_DAMPING)
			{
				body->m_rigidBody->setContactStiffnessAndDamping(clientCmd.m_changeDynamicsInfoArgs.m_contactStiffness, clientCmd.m_changeDynamicsInfoArgs.m_contactDamping);
			}
			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_RESTITUTION)
			{
				body->m_rigidBody->setRestitution(restitution);
			}
			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LATERAL_FRICTION)
			{
				body->m_rigidBody->setFriction(lateralFriction);
			}
			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_SPINNING_FRICTION)
			{
				body->m_rigidBody->setSpinningFriction(spinningFriction);
			}
			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ROLLING_FRICTION)
			{
				body->m_rigidBody->setRollingFriction(rollingFriction);
			}

			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_FRICTION_ANCHOR)
			{
				if (clientCmd.m_changeDynamicsInfoArgs.m_frictionAnchor)
				{
					body->m_rigidBody->setCollisionFlags(body->m_rigidBody->getCollisionFlags() | CollisionObject2::CF_HAS_FRICTION_ANCHOR);
				}
				else
				{
					body->m_rigidBody->setCollisionFlags(body->m_rigidBody->getCollisionFlags() & ~CollisionObject2::CF_HAS_FRICTION_ANCHOR);
				}
			}

			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_MASS)
			{
				Vec3 localInertia;
				if (body->m_rigidBody->getCollisionShape())
				{
					body->m_rigidBody->getCollisionShape()->calculateLocalInertia(mass, localInertia);
				}
				body->m_rigidBody->setMassProps(mass, localInertia);
			}
			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LOCAL_INERTIA_DIAGONAL)
			{
				Scalar orgMass = body->m_rigidBody->getInvMass();
				if (orgMass > 0)
				{
					body->m_rigidBody->setMassProps(mass, newLocalInertiaDiagonal);
				}
			}

			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_CONTACT_PROCESSING_THRESHOLD)
			{
				body->m_rigidBody->setContactProcessingThreshold(clientCmd.m_changeDynamicsInfoArgs.m_contactProcessingThreshold);
			}

			if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_CCD_SWEPT_SPHERE_RADIUS)
			{
				body->m_rigidBody->setCcdSweptSphereRadius(clientCmd.m_changeDynamicsInfoArgs.m_ccdSweptSphereRadius);
				//for a given sphere radius, use a motion threshold of half the radius, before the ccd algorithm is enabled
				body->m_rigidBody->setCcdMotionThreshold(clientCmd.m_changeDynamicsInfoArgs.m_ccdSweptSphereRadius / 2.);
			}
		}
	}

	b3Notification notification;
	notification.m_notificationType = LINK_DYNAMICS_CHANGED;
	notification.m_linkArgs.m_bodyUniqueId = bodyUniqueId;
	notification.m_linkArgs.m_linkIndex = linkIndex;
	m_data->m_pluginManager.addNotification(notification);


#endif
	SharedMemoryStatus& serverCmd = serverStatusOut;
	serverCmd.m_type = CMD_CLIENT_COMMAND_COMPLETED;

	
	return hasStatus;
}

bool PhysXServerCommandProcessor::processRequestPhysicsSimulationParametersCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;
	SharedMemoryStatus& serverCmd = serverStatusOut;
	serverCmd.m_type = CMD_REQUEST_PHYSICS_SIMULATION_PARAMETERS_COMPLETED;

	serverCmd.m_simulationParameterResultArgs.m_allowedCcdPenetration = 0;// m_data->m_dynamicsWorld->getDispatchInfo().m_allowedCcdPenetration;
	serverCmd.m_simulationParameterResultArgs.m_collisionFilterMode = 0;// m_data->m_broadphaseCollisionFilterCallback->m_filterMode;
	serverCmd.m_simulationParameterResultArgs.m_deltaTime = m_data->m_physicsDeltaTime;
	serverCmd.m_simulationParameterResultArgs.m_contactBreakingThreshold = 0;// gContactBreakingThreshold;
	serverCmd.m_simulationParameterResultArgs.m_contactSlop = 0;// m_data->m_dynamicsWorld->getSolverInfo().m_linearSlop;
	serverCmd.m_simulationParameterResultArgs.m_enableSAT = 0;// m_data->m_dynamicsWorld->getDispatchInfo().m_enableSatConvex;

	serverCmd.m_simulationParameterResultArgs.m_defaultGlobalCFM = 0;// m_data->m_dynamicsWorld->getSolverInfo().m_globalCfm;
	serverCmd.m_simulationParameterResultArgs.m_defaultContactERP = 0;// m_data->m_dynamicsWorld->getSolverInfo().m_erp2;
	serverCmd.m_simulationParameterResultArgs.m_defaultNonContactERP = 0;// m_data->m_dynamicsWorld->getSolverInfo().m_erp;
	
	serverCmd.m_simulationParameterResultArgs.m_deterministicOverlappingPairs = 0;// m_data->m_dynamicsWorld->getDispatchInfo().m_deterministicOverlappingPairs;
	serverCmd.m_simulationParameterResultArgs.m_enableConeFriction = 0;// (m_data->m_dynamicsWorld->getSolverInfo().m_solverMode & SOLVER_DISABLE_IMPLICIT_CONE_FRICTION) ? 0 : 1;
	serverCmd.m_simulationParameterResultArgs.m_enableFileCaching = 0;// b3IsFileCachingEnabled();
	serverCmd.m_simulationParameterResultArgs.m_frictionCFM = 0;// m_data->m_dynamicsWorld->getSolverInfo().m_frictionCFM;
	serverCmd.m_simulationParameterResultArgs.m_frictionERP = 0;// m_data->m_dynamicsWorld->getSolverInfo().m_frictionERP;

	physx::PxVec3 grav = m_data->m_scene->getGravity();

	serverCmd.m_simulationParameterResultArgs.m_gravityAcceleration[0] = grav.x;
	serverCmd.m_simulationParameterResultArgs.m_gravityAcceleration[1] = grav.y;
	serverCmd.m_simulationParameterResultArgs.m_gravityAcceleration[2] = grav.z;
	serverCmd.m_simulationParameterResultArgs.m_internalSimFlags = 0;
	serverCmd.m_simulationParameterResultArgs.m_jointFeedbackMode = 0;
	
	serverCmd.m_simulationParameterResultArgs.m_numSimulationSubSteps = 0;// m_data->m_numSimulationSubSteps;
	serverCmd.m_simulationParameterResultArgs.m_numSolverIterations = 0;// m_data->m_dynamicsWorld->getSolverInfo().m_numIterations;
	serverCmd.m_simulationParameterResultArgs.m_restitutionVelocityThreshold = 0;// m_data->m_dynamicsWorld->getSolverInfo().m_restitutionVelocityThreshold;

	serverCmd.m_simulationParameterResultArgs.m_solverResidualThreshold = 0;// m_data->m_dynamicsWorld->getSolverInfo().m_leastSquaresResidualThreshold;
	serverCmd.m_simulationParameterResultArgs.m_splitImpulsePenetrationThreshold = 0;// m_data->m_dynamicsWorld->getSolverInfo().m_splitImpulsePenetrationThreshold;
	serverCmd.m_simulationParameterResultArgs.m_useRealTimeSimulation = 0;// m_data->m_useRealTimeSimulation;
	serverCmd.m_simulationParameterResultArgs.m_useSplitImpulse = 0;// m_data->m_dynamicsWorld->getSolverInfo().m_splitImpulse;

	return hasStatus;
}

bool PhysXServerCommandProcessor::processRequestContactpointInformationCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	SharedMemoryStatus& serverCmd = serverStatusOut;
	i32 totalBytesPerContact = sizeof(b3ContactPointData);
	i32 contactPointStorage = bufferSizeInBytes / totalBytesPerContact - 1;

	b3ContactPointData* contactData = (b3ContactPointData*)bufferServerToClient;

	i32 startContactPointIndex = clientCmd.m_requestContactPointArguments.m_startingContactPointIndex;
	i32 numContactPointBatch = d3Min(i32(m_data->m_contactPoints.size()), contactPointStorage);

	i32 endContactPointIndex = startContactPointIndex + numContactPointBatch;
	serverCmd.m_sendContactPointArgs.m_numContactPointsCopied = 0;
	for (i32 i = startContactPointIndex; i < endContactPointIndex; i++)
	{
		const b3ContactPointData& srcPt = m_data->m_contactPoints[i];
		b3ContactPointData& destPt = contactData[serverCmd.m_sendContactPointArgs.m_numContactPointsCopied];
		destPt = srcPt;
		serverCmd.m_sendContactPointArgs.m_numContactPointsCopied++;
	}
	serverCmd.m_sendContactPointArgs.m_startingContactPointIndex = startContactPointIndex;
	serverCmd.m_sendContactPointArgs.m_numRemainingContactPoints = m_data->m_contactPoints.size() - startContactPointIndex - serverCmd.m_sendContactPointArgs.m_numContactPointsCopied;
	serverCmd.m_numDataStreamBytes = totalBytesPerContact * serverCmd.m_sendContactPointArgs.m_numContactPointsCopied;
	serverCmd.m_type = CMD_CONTACT_POINT_INFORMATION_COMPLETED;
	

	return true;
}

bool PhysXServerCommandProcessor::processCreateCollisionShapeCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	return false;
}


bool PhysXServerCommandProcessor::processSetAdditionalSearchPathCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;

	DRX3D_PROFILE("CMD_SET_ADDITIONAL_SEARCH_PATH");
	ResourcePath::setAdditionalSearchPath(clientCmd.m_searchPathArgs.m_path);
	serverStatusOut.m_type = CMD_CLIENT_COMMAND_COMPLETED;
	return hasStatus;
}

bool PhysXServerCommandProcessor::processUserDebugDrawCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	///dummy, so commands don't fail
	bool hasStatus = true;
	DRX3D_PROFILE("CMD_USER_DEBUG_DRAW");
	SharedMemoryStatus& serverCmd = serverStatusOut;
	i32 uid = 0;
	serverCmd.m_userDebugDrawArgs.m_debugItemUniqueId = uid;
	serverCmd.m_type = CMD_USER_DEBUG_DRAW_FAILED;

	if (clientCmd.m_updateFlags & USER_DEBUG_ADD_PARAMETER)
	{
		i32 uid = m_data->m_userDebugParametersUid++;
		double value = clientCmd.m_userDebugDrawArgs.m_startValue;
		m_data->m_userDebugParameters.insert(uid, value);
		serverCmd.m_userDebugDrawArgs.m_debugItemUniqueId = uid;
		serverCmd.m_type = CMD_USER_DEBUG_DRAW_COMPLETED;
	}
	if (clientCmd.m_updateFlags & USER_DEBUG_READ_PARAMETER)
	{
		double* valPtr = m_data->m_userDebugParameters[clientCmd.m_userDebugDrawArgs.m_itemUniqueId];
		if (valPtr)
		{
			serverCmd.m_userDebugDrawArgs.m_parameterValue = *valPtr;
			serverCmd.m_type = CMD_USER_DEBUG_DRAW_PARAMETER_COMPLETED;
		}
	}
	if (clientCmd.m_updateFlags & USER_DEBUG_REMOVE_ALL)
	{
		m_data->m_userDebugParameters.clear();
		serverCmd.m_type = CMD_USER_DEBUG_DRAW_COMPLETED;
	}
	if (clientCmd.m_updateFlags & USER_DEBUG_REMOVE_ONE_ITEM)
	{
		m_data->m_userDebugParameters.remove(clientCmd.m_userDebugDrawArgs.m_itemUniqueId);
		serverCmd.m_type = CMD_USER_DEBUG_DRAW_COMPLETED;
	}
	return hasStatus;

}

bool PhysXServerCommandProcessor::processCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	//	DRX3D_PROFILE("processCommand");

	i32 sz = sizeof(SharedMemoryStatus);
	i32 sz2 = sizeof(SharedMemoryCommand);

	bool hasStatus = true;

	serverStatusOut.m_type = CMD_INVALID_STATUS;
	serverStatusOut.m_numDataStreamBytes = 0;
	serverStatusOut.m_dataStream = 0;

	//consume the command
	switch (clientCmd.m_type)
	{
		case CMD_REQUEST_INTERNAL_DATA:
		{
			hasStatus = processRequestInternalDataCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		};

		case CMD_SYNC_BODY_INFO:
		{
			hasStatus = processSyncBodyInfoCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}
		case CMD_SYNC_USER_DATA:
		{
			hasStatus = processSyncUserDataCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}
		case CMD_REQUEST_BODY_INFO:
		{
			hasStatus = processRequestBodyInfoCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}
		case CMD_STEP_FORWARD_SIMULATION:
		{
			hasStatus = processForwardDynamicsCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}

		case CMD_SEND_PHYSICS_SIMULATION_PARAMETERS:
		{
			hasStatus = processSendPhysicsParametersCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		};

		case CMD_REQUEST_ACTUAL_STATE:
		{
			hasStatus = processRequestActualStateCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}

		case CMD_RESET_SIMULATION:
		{
			hasStatus = processResetSimulationCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}

		case CMD_LOAD_URDF:
		{
			hasStatus = processLoadURDFCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}
		case CMD_CUSTOM_COMMAND:
		{
			hasStatus = processCustomCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}
		case CMD_STATE_LOGGING:
		{
			hasStatus = processStateLoggingCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}

		case CMD_INIT_POSE:
		{
			hasStatus = processInitPoseCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}

		case CMD_SEND_DESIRED_STATE:
		{
			hasStatus = processSendDesiredStateCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}

		case CMD_CHANGE_DYNAMICS_INFO:
		{
			hasStatus = processChangeDynamicsInfoCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		};

		case CMD_REQUEST_PHYSICS_SIMULATION_PARAMETERS:
		{
			hasStatus = processRequestPhysicsSimulationParametersCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}

		case CMD_REQUEST_CONTACT_POINT_INFORMATION:
		{
			hasStatus = processRequestContactpointInformationCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}

		case CMD_CREATE_COLLISION_SHAPE:
		{
			hasStatus = processCreateCollisionShapeCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}

		case CMD_SET_ADDITIONAL_SEARCH_PATH:
		{
			hasStatus = processSetAdditionalSearchPathCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}

		case CMD_USER_DEBUG_DRAW:
		{
			hasStatus = processUserDebugDrawCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}

		case CMD_REQUEST_CAMERA_IMAGE_DATA:
		{
			hasStatus = processRequestCameraImageCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
			break;
		}

#if 0
	case CMD_SET_VR_CAMERA_STATE:
		{
			hasStatus = processSetVRCameraStateCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_REQUEST_VR_EVENTS_DATA:
		{
			hasStatus = processRequestVREventsCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		};
	case CMD_REQUEST_MOUSE_EVENTS_DATA:
		{
			hasStatus = processRequestMouseEventsCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		};
	case CMD_REQUEST_KEYBOARD_EVENTS_DATA:
		{
			hasStatus = processRequestKeyboardEventsCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		};

	case CMD_REQUEST_RAY_CAST_INTERSECTIONS:
		{

			hasStatus = processRequestRaycastIntersectionsCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		};
	case CMD_REQUEST_DEBUG_LINES:
		{
			hasStatus = processRequestDebugLinesCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}

	
	case CMD_REQUEST_BODY_INFO:
		{
			hasStatus = processRequestBodyInfoCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_SAVE_WORLD:
		{
			hasStatus = processSaveWorldCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_LOAD_SDF:
		{
			hasStatus = processLoadSDFCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	
	case CMD_CREATE_VISUAL_SHAPE:
		{
			hasStatus = processCreateVisualShapeCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_CREATE_MULTI_BODY:
		{
			hasStatus = processCreateMultiBodyCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	
	
	case CMD_LOAD_MJCF:
	{
		hasStatus = processLoadMJCFCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}

	case CMD_LOAD_SOFT_BODY:
		{
			hasStatus = processLoadSoftBodyCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_CREATE_SENSOR:
		{
			hasStatus = processCreateSensorCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_PROFILE_TIMING:
		{
			hasStatus = processProfileTimingCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}

	
	case CMD_REQUEST_COLLISION_INFO:
		{
			hasStatus = processRequestCollisionInfoCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}

	
	
	
	case CMD_GET_DYNAMICS_INFO:
		{
			hasStatus = processGetDynamicsInfoCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}


	
	
	case CMD_CREATE_RIGID_BODY:
		{
			hasStatus = processCreateRigidBodyCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_CREATE_BOX_COLLISION_SHAPE:
		{
			//for backward compatibility, CMD_CREATE_BOX_COLLISION_SHAPE is the same as CMD_CREATE_RIGID_BODY
			hasStatus = processCreateRigidBodyCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_PICK_BODY:
		{
			hasStatus = processPickBodyCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_MOVE_PICKED_BODY:
		{
			hasStatus = processMovePickedBodyCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_REMOVE_PICKING_CONSTRAINT_BODY:
		{
			hasStatus = processRemovePickingConstraintCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_REQUEST_AABB_OVERLAP:
		{
			hasStatus = processRequestAabbOverlapCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_REQUEST_OPENGL_VISUALIZER_CAMERA:
		{
			hasStatus = processRequestOpenGLVisualizeCameraCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_CONFIGURE_OPENGL_VISUALIZER:
		{
			hasStatus = processConfigureOpenGLVisualizerCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	
	case CMD_CALCULATE_INVERSE_DYNAMICS:
		{
			hasStatus = processdrx3d_inverseCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_CALCULATE_JACOBIAN:
		{
			hasStatus = processCalculateJacobianCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_CALCULATE_MASS_MATRIX:
		{
			hasStatus = processCalculateMassMatrixCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_APPLY_EXTERNAL_FORCE:
		{
			hasStatus = processApplyExternalForceCommand(clientCmd,serverStatusOut,bufferServerToClient, bufferSizeInBytes);
			break;
		}
	case CMD_REMOVE_BODY:
	{
		hasStatus = processRemoveBodyCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}
	case CMD_USER_CONSTRAINT:
	{
		hasStatus = processCreateUserConstraintCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}
	case CMD_CALCULATE_INVERSE_KINEMATICS:
	{
		hasStatus = processCalculateInverseKinematicsCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}
	case CMD_REQUEST_VISUAL_SHAPE_INFO:
	{
		hasStatus = processRequestVisualShapeInfoCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}
	case CMD_REQUEST_COLLISION_SHAPE_INFO:
	{
		hasStatus = processRequestCollisionShapeInfoCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}
	case CMD_UPDATE_VISUAL_SHAPE:
	{
		hasStatus = processUpdateVisualShapeCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}
	case CMD_CHANGE_TEXTURE:
	{
		hasStatus = processChangeTextureCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}
	case CMD_LOAD_TEXTURE:
	{
		hasStatus = processLoadTextureCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}
	case CMD_RESTORE_STATE:
	{
		hasStatus = processRestoreStateCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}

	case CMD_SAVE_STATE:
	{
		hasStatus = processSaveStateCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}

	case CMD_LOAD_BULLET:
	{
		hasStatus = processLoadBulletCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}
	case CMD_SAVE_BULLET:
	{
		hasStatus = processSaveBulletCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}
	case CMD_LOAD_MJCF:
	{
		hasStatus = processLoadMJCFCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}

	case CMD_REQUEST_USER_DATA:
	{
		hasStatus = processRequestUserDataCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}
	case CMD_ADD_USER_DATA:
	{
		hasStatus = processAddUserDataCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}
	case CMD_REMOVE_USER_DATA:
	{
		hasStatus = processRemoveUserDataCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		break;
	}
#endif

	default:
	{
		DRX3D_PROFILE("CMD_UNKNOWN");
		printf("Unknown command encountered: %d", clientCmd.m_type);
		SharedMemoryStatus& serverCmd = serverStatusOut;
		serverCmd.m_type = CMD_UNKNOWN_COMMAND_FLUSHED;
		hasStatus = true;
	}
	};

	return hasStatus;
}



bool PhysXServerCommandProcessor::processRequestCameraImageCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;
	DRX3D_PROFILE("CMD_REQUEST_CAMERA_IMAGE_DATA");
	i32 startPixelIndex = clientCmd.m_requestPixelDataArguments.m_startPixelIndex;
	i32 width = clientCmd.m_requestPixelDataArguments.m_pixelWidth;
	i32 height = clientCmd.m_requestPixelDataArguments.m_pixelHeight;
	i32 numPixelsCopied = 0;

	i32 oldWidth;
	i32 oldHeight;
	m_data->m_pluginManager.getRenderInterface()->getWidthAndHeight(oldWidth, oldHeight);

	serverStatusOut.m_type = CMD_CAMERA_IMAGE_FAILED;
	
	if ((clientCmd.m_requestPixelDataArguments.m_startPixelIndex == 0) &&
		(clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_PIXEL_WIDTH_HEIGHT) != 0)
	{
		if (m_data->m_pluginManager.getRenderInterface())
		{

			m_data->m_pluginManager.getRenderInterface()->setWidthAndHeight(clientCmd.m_requestPixelDataArguments.m_pixelWidth,
				clientCmd.m_requestPixelDataArguments.m_pixelHeight);
		}
	}
	i32 flags = 0;
	if (clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_HAS_FLAGS)
	{
		flags = clientCmd.m_requestPixelDataArguments.m_flags;
	}
	if (m_data->m_pluginManager.getRenderInterface())
	{
		m_data->m_pluginManager.getRenderInterface()->setFlags(flags);
	}

	i32 numTotalPixels = width * height;
	i32 numRemainingPixels = numTotalPixels - startPixelIndex;

	if (numRemainingPixels > 0)
	{
		i32 totalBytesPerPixel = 4 + 4 + 4;  //4 for rgb, 4 for depth, 4 for segmentation mask
		i32 maxNumPixels = bufferSizeInBytes / totalBytesPerPixel - 1;
		u8* pixelRGBA = (u8*)bufferServerToClient;
		i32 numRequestedPixels = d3Min(maxNumPixels, numRemainingPixels);

		float* depthBuffer = (float*)(bufferServerToClient + numRequestedPixels * 4);
		i32* segmentationMaskBuffer = (i32*)(bufferServerToClient + numRequestedPixels * 8);

		serverStatusOut.m_numDataStreamBytes = numRequestedPixels * totalBytesPerPixel;
		float viewMat[16];
		float projMat[16];
		float projTextureViewMat[16];
		float projTextureProjMat[16];
		
		if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_HAS_CAMERA_MATRICES) == 0)
		{
			b3OpenGLVisualizerCameraInfo tmpCamResult;
			bool result = m_data->m_pluginManager.getRenderInterface()->getCameraInfo(
				&tmpCamResult.m_width,
				&tmpCamResult.m_height,
				tmpCamResult.m_viewMatrix,
				tmpCamResult.m_projectionMatrix,
				tmpCamResult.m_camUp,
				tmpCamResult.m_camForward,
				tmpCamResult.m_horizontal,
				tmpCamResult.m_vertical,
				&tmpCamResult.m_yaw,
				&tmpCamResult.m_pitch,
				&tmpCamResult.m_dist,
				tmpCamResult.m_target);
			if (result)
			{
				for (i32 i = 0; i < 16; i++)
				{
					viewMat[i] = tmpCamResult.m_viewMatrix[i];
					projMat[i] = tmpCamResult.m_projectionMatrix[i];
				}
			}
			else
			{
				//failed
				m_data->m_pluginManager.getRenderInterface()->setWidthAndHeight(oldWidth, oldHeight);
				return hasStatus;
			}
		}
		else
		{
			for (i32 i = 0; i < 16; i++)
			{
				viewMat[i] = clientCmd.m_requestPixelDataArguments.m_viewMatrix[i];
				projMat[i] = clientCmd.m_requestPixelDataArguments.m_projectionMatrix[i];
			}
		}
		
		{
			if (m_data->m_pluginManager.getRenderInterface())
			{
				if (clientCmd.m_requestPixelDataArguments.m_startPixelIndex == 0)
				{
					//   printf("-------------------------------\nRendering\n");

					if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_LIGHT_DIRECTION) != 0)
					{
						m_data->m_pluginManager.getRenderInterface()->setLightDirection(clientCmd.m_requestPixelDataArguments.m_lightDirection[0], clientCmd.m_requestPixelDataArguments.m_lightDirection[1], clientCmd.m_requestPixelDataArguments.m_lightDirection[2]);
					}

					if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_LIGHT_COLOR) != 0)
					{
						m_data->m_pluginManager.getRenderInterface()->setLightColor(clientCmd.m_requestPixelDataArguments.m_lightColor[0], clientCmd.m_requestPixelDataArguments.m_lightColor[1], clientCmd.m_requestPixelDataArguments.m_lightColor[2]);
					}

					if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_LIGHT_DISTANCE) != 0)
					{
						m_data->m_pluginManager.getRenderInterface()->setLightDistance(clientCmd.m_requestPixelDataArguments.m_lightDistance);
					}

					if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_SHADOW) != 0)
					{
						m_data->m_pluginManager.getRenderInterface()->setShadow((clientCmd.m_requestPixelDataArguments.m_hasShadow != 0));
					}

					if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_AMBIENT_COEFF) != 0)
					{
						m_data->m_pluginManager.getRenderInterface()->setLightAmbientCoeff(clientCmd.m_requestPixelDataArguments.m_lightAmbientCoeff);
					}

					if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_DIFFUSE_COEFF) != 0)
					{
						m_data->m_pluginManager.getRenderInterface()->setLightDiffuseCoeff(clientCmd.m_requestPixelDataArguments.m_lightDiffuseCoeff);
					}

					if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_SPECULAR_COEFF) != 0)
					{
						m_data->m_pluginManager.getRenderInterface()->setLightSpecularCoeff(clientCmd.m_requestPixelDataArguments.m_lightSpecularCoeff);
					}


					if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_HAS_CAMERA_MATRICES) != 0)
					{
						m_data->m_pluginManager.getRenderInterface()->render(
							clientCmd.m_requestPixelDataArguments.m_viewMatrix,
							clientCmd.m_requestPixelDataArguments.m_projectionMatrix);
					}
					else
					{
						b3OpenGLVisualizerCameraInfo tmpCamResult;
						bool result = m_data->m_pluginManager.getRenderInterface()->getCameraInfo(
							&tmpCamResult.m_width,
							&tmpCamResult.m_height,
							tmpCamResult.m_viewMatrix,
							tmpCamResult.m_projectionMatrix,
							tmpCamResult.m_camUp,
							tmpCamResult.m_camForward,
							tmpCamResult.m_horizontal,
							tmpCamResult.m_vertical,
							&tmpCamResult.m_yaw,
							&tmpCamResult.m_pitch,
							&tmpCamResult.m_dist,
							tmpCamResult.m_target);
						if (result)
						{
							m_data->m_pluginManager.getRenderInterface()->render(tmpCamResult.m_viewMatrix, tmpCamResult.m_projectionMatrix);
						}
						else
						{
							m_data->m_pluginManager.getRenderInterface()->render();
						}
					}
				}
			}

			if (m_data->m_pluginManager.getRenderInterface())
			{
				if ((flags & ER_USE_PROJECTIVE_TEXTURE) != 0)
				{
					m_data->m_pluginManager.getRenderInterface()->setProjectiveTexture(true);
					if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_HAS_PROJECTIVE_TEXTURE_MATRICES) != 0)
					{
						for (i32 i = 0; i < 16; i++)
						{
							projTextureViewMat[i] = clientCmd.m_requestPixelDataArguments.m_projectiveTextureViewMatrix[i];
							projTextureProjMat[i] = clientCmd.m_requestPixelDataArguments.m_projectiveTextureProjectionMatrix[i];
						}
					}
					else  // If no specified matrices for projective texture, then use the camera matrices.
					{
						for (i32 i = 0; i < 16; i++)
						{
							projTextureViewMat[i] = viewMat[i];
							projTextureProjMat[i] = projMat[i];
						}
					}
					m_data->m_pluginManager.getRenderInterface()->setProjectiveTextureMatrices(projTextureViewMat, projTextureProjMat);
				}
				else
				{
					m_data->m_pluginManager.getRenderInterface()->setProjectiveTexture(false);
				}

				if ((flags & ER_NO_SEGMENTATION_MASK) != 0)
				{
					segmentationMaskBuffer = 0;
				}

				m_data->m_pluginManager.getRenderInterface()->copyCameraImageData(pixelRGBA, numRequestedPixels,
					depthBuffer, numRequestedPixels,
					segmentationMaskBuffer, numRequestedPixels,
					startPixelIndex, &width, &height, &numPixelsCopied);
				m_data->m_pluginManager.getRenderInterface()->setProjectiveTexture(false);
			}

			#if 0
			m_data->m_guiHelper->debugDisplayCameraImageData(clientCmd.m_requestPixelDataArguments.m_viewMatrix,
				clientCmd.m_requestPixelDataArguments.m_projectionMatrix, pixelRGBA, numRequestedPixels,
				depthBuffer, numRequestedPixels,
				segmentationMaskBuffer, numRequestedPixels,
				startPixelIndex, width, height, &numPixelsCopied);
			#endif		
		}

		//each pixel takes 4 RGBA values and 1 float = 8 bytes
	}
	else
	{
	}

	m_data->m_pluginManager.getRenderInterface()->setWidthAndHeight(oldWidth, oldHeight);
	serverStatusOut.m_type = CMD_CAMERA_IMAGE_COMPLETED;

	serverStatusOut.m_sendPixelDataArguments.m_numPixelsCopied = numPixelsCopied;
	serverStatusOut.m_sendPixelDataArguments.m_numRemainingPixels = numRemainingPixels - numPixelsCopied;
	serverStatusOut.m_sendPixelDataArguments.m_startingPixelIndex = startPixelIndex;
	serverStatusOut.m_sendPixelDataArguments.m_imageWidth = width;
	serverStatusOut.m_sendPixelDataArguments.m_imageHeight = height;
	return hasStatus;

}

bool PhysXServerCommandProcessor::processRequestInternalDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;
	DRX3D_PROFILE("CMD_REQUEST_INTERNAL_DATA");
	SharedMemoryStatus& serverCmd = serverStatusOut;
	serverCmd.m_type = CMD_REQUEST_INTERNAL_DATA_COMPLETED;
	serverCmd.m_numDataStreamBytes = 0;
	return hasStatus;
}

bool PhysXServerCommandProcessor::processSyncBodyInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;
	DRX3D_PROFILE("CMD_SYNC_BODY_INFO");
	i32 actualNumBodies = 0;
	serverStatusOut.m_sdfLoadedArgs.m_numBodies = 0;
	serverStatusOut.m_sdfLoadedArgs.m_numUserConstraints = 0;
	serverStatusOut.m_type = CMD_SYNC_BODY_INFO_COMPLETED;
	return hasStatus;
}

bool PhysXServerCommandProcessor::processSyncUserDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;
	DRX3D_PROFILE("CMD_SYNC_USER_DATA");
	i32 numIdentifiers = 0;
	serverStatusOut.m_syncUserDataArgs.m_numUserDataIdentifiers = numIdentifiers;
	serverStatusOut.m_type = CMD_SYNC_USER_DATA_COMPLETED;
	return hasStatus;
}

struct MyPhysXURDFImporter : public PhysXURDFImporter
{
	b3PluginManager& m_pluginManager;
	
	MyPhysXURDFImporter(struct CommonFileIOInterface* fileIO, double globalScaling, i32 flags, b3PluginManager& pluginManager)
		:PhysXURDFImporter(fileIO, globalScaling, flags),
		m_pluginManager(pluginManager)
	{

	}

	i32 convertLinkVisualShapes3(
			i32 linkIndex, tukk pathPrefix, const Transform2& localInertiaFrame,
			const UrdfLink* linkPtr, const UrdfModel* model,
			i32 collisionObjectUniqueId, i32 bodyUniqueId, struct  CommonFileIOInterface* fileIO) const
	{

		if (m_pluginManager.getRenderInterface())
		{
			i32 graphicsUniqueId = m_pluginManager.getRenderInterface()->convertVisualShapes(linkIndex, pathPrefix, localInertiaFrame, linkPtr, model, collisionObjectUniqueId, bodyUniqueId, fileIO);
			return graphicsUniqueId;
		}
		return 0;
	}

};


bool PhysXServerCommandProcessor::processLoadURDFCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	DRX3D_PROFILE("CMD_LOAD_URDF");
	serverStatusOut.m_type = CMD_URDF_LOADING_FAILED;
	serverStatusOut.m_numDataStreamBytes = 0;

	const UrdfArgs& urdfArgs = clientCmd.m_urdfArguments;

	Assert(m_data->m_foundation);
	if (!m_data->m_foundation)
	{
		drx3DError("loadUrdf: No valid m_dynamicsWorld");
		return false;
	}

	bool useMultiBody = (clientCmd.m_updateFlags & URDF_ARGS_USE_MULTIBODY) ? (urdfArgs.m_useMultiBody != 0) : true;
	bool useFixedBase = (clientCmd.m_updateFlags & URDF_ARGS_USE_FIXED_BASE) ? (urdfArgs.m_useFixedBase != 0) : false;

	Scalar globalScaling = 1.f;
	if (clientCmd.m_updateFlags & URDF_ARGS_USE_GLOBAL_SCALING)
	{
		globalScaling = urdfArgs.m_globalScaling;
	}

	DefaultFileIO fileIO;
	
	Vec3 initialPos(0, 0, 0);
	Quat initialOrn(0, 0, 0, 1);
	if (clientCmd.m_updateFlags & URDF_ARGS_INITIAL_POSITION)
	{
		initialPos[0] = urdfArgs.m_initialPosition[0];
		initialPos[1] = urdfArgs.m_initialPosition[1];
		initialPos[2] = urdfArgs.m_initialPosition[2];
	}
	i32 urdfFlags = 0;
	if (clientCmd.m_updateFlags & URDF_ARGS_HAS_CUSTOM_URDF_FLAGS)
	{
		urdfFlags = urdfArgs.m_urdfFlags;
	}
	if (clientCmd.m_updateFlags & URDF_ARGS_INITIAL_ORIENTATION)
	{
		initialOrn[0] = urdfArgs.m_initialOrientation[0];
		initialOrn[1] = urdfArgs.m_initialOrientation[1];
		initialOrn[2] = urdfArgs.m_initialOrientation[2];
		initialOrn[3] = urdfArgs.m_initialOrientation[3];
	}


	MyPhysXURDFImporter u2p(&fileIO, globalScaling, urdfArgs.m_urdfFlags, m_data->m_pluginManager);
	
	

	bool loadOk = u2p.loadURDF(urdfArgs.m_urdfFileName, useFixedBase);


	if (loadOk)
	{
		

		for (i32 m = 0; m < u2p.getNumModels(); m++)
		{
			u2p.activateModel(m);

			Transform2 rootTrans;
			rootTrans.setOrigin(initialPos);
			rootTrans.setRotation(initialOrn);
			u2p.setRootTransformInWorld(rootTrans);

			//get a body index
			i32 bodyUniqueId = m_data->m_bodyHandles.allocHandle();

			InternalPhysXBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(bodyUniqueId);

			//sd.m_bodyUniqueIds.push_back(bodyUniqueId);

			
			
			u2p.setBodyUniqueId(bodyUniqueId);
			{
				Scalar mass = 0;
				//bodyHandle->m_rootLocalInertialFrame.setIdentity();
				bodyHandle->m_bodyName = u2p.getBodyName();
				Vec3 localInertiaDiagonal(0, 0, 0);
				i32 urdfLinkIndex = u2p.getRootLinkIndex();
				//u2p.getMassAndInertia2((urdfLinkIndex, mass, localInertiaDiagonal, bodyHandle->m_rootLocalInertialFrame, flags);
			}
			
			physx::PxBase* baseObj = URDF2PhysX(m_data->m_foundation,m_data->m_physics, m_data->m_cooking, m_data->m_scene, u2p, urdfArgs.m_urdfFlags, u2p.getPathPrefix(), rootTrans, &fileIO, useMultiBody);

			physx::PxRigidDynamic* c = baseObj->is<physx::PxRigidDynamic>();
			physx::PxRigidStatic* rigidStatic = baseObj->is<physx::PxRigidStatic>();
			physx::PxRigidDynamic* rigidDynamic = baseObj->is<physx::PxRigidDynamic>();
			physx::PxArticulationReducedCoordinate* articulation = 0;
			if (rigidDynamic)
			{
				bodyHandle->m_rigidDynamic = rigidDynamic;
			}
			if (rigidStatic)
			{
				bodyHandle->m_rigidStatic = rigidStatic;
				serverStatusOut.m_numDataStreamBytes = 0;
				serverStatusOut.m_type = CMD_URDF_LOADING_COMPLETED;
				serverStatusOut.m_dataStreamArguments.m_bodyUniqueId = bodyUniqueId;
			}
			if ((rigidDynamic == 0) && (rigidStatic == 0))
			{
				articulation = (physx::PxArticulationReducedCoordinate*)baseObj;
				serverStatusOut.m_type = CMD_URDF_LOADING_COMPLETED;
				serverStatusOut.m_dataStreamArguments.m_bodyUniqueId = bodyUniqueId;
			}
			if (rigidStatic || rigidDynamic || articulation)
			{
				serverStatusOut.m_type = CMD_URDF_LOADING_COMPLETED;
				serverStatusOut.m_dataStreamArguments.m_bodyUniqueId = bodyUniqueId;
				sprintf(serverStatusOut.m_dataStreamArguments.m_bodyName, "%s", bodyHandle->m_bodyName.c_str());
			}
			
			if (articulation)
			{
				bodyHandle->mArticulation = articulation;
				
				

				btDefaultSerializer ser(bufferSizeInBytes, (u8*)bufferServerToClient);

				ser.startSerialization();

				i32 len = sizeof(MultiBodyData); 
				Chunk* chunk = ser.allocate(len, 1);


				btMultiBodyData *mbd = (MultiBodyData *)chunk->m_oldPtr;
				Transform2 rootTrans;
				u2p.getRootTransformInWorld(rootTrans);
				rootTrans.getOrigin().serialize(mbd->m_baseWorldPosition);
				rootTrans.getRotation().serialize(mbd->m_baseWorldOrientation);
				Vec3 zero(0, 0, 0);

				zero.serialize(mbd->m_baseLinearVelocity);
				zero.serialize(mbd->m_baseAngularVelocity);
				
				ser.registerNameForPointer(bodyHandle->m_bodyName.c_str(), bodyHandle->m_bodyName.c_str());
				{
					char *name = (char *)ser.findNameForPointer(bodyHandle->m_bodyName.c_str());
					mbd->m_baseName = (char *)ser.getUniquePointer(name);
					if (mbd->m_baseName)
					{
						ser.serializeName(name);
					}
				}
				mbd->m_numLinks = articulation->getNbLinks()-1;

				if (mbd->m_numLinks)
				{
					i32 sz = sizeof(MultiBodyLinkData);
					i32 numElem = mbd->m_numLinks;
					Chunk *chunk = ser.allocate(sz, numElem);

					physx::PxArticulationLink* physxLinks[64];
					physx::PxU32 bufferSize = 64;
					physx::PxU32 startIndex = 0;
					i32 numLinks2 = articulation->getLinks(physxLinks, bufferSize, startIndex);
					
					btMultiBodyLinkData *memPtr = (MultiBodyLinkData *)chunk->m_oldPtr;
					for (i32 j = 0; j < numElem; j++, memPtr++)
					{
						i32 i = j + 1;
						
						
						memPtr->m_jointType = 0;//todo 
						memPtr->m_dofCount = physxLinks[i]->getInboundJointDof(); 
						memPtr->m_posVarCount = physxLinks[i]->getInboundJointDof(); //??

						physx::PxVec3 li = physxLinks[i]->getMassSpaceInertiaTensor();
						Vec3 localInertia(li[0], li[1], li[2]);
						localInertia.serialize(memPtr->m_linkInertia);

						memPtr->m_linkMass = physxLinks[i]->getMass();
						memPtr->m_parentIndex = i>0? physxLinks[i]->getInboundJoint()->getParentArticulationLink().getLinkIndex(): -1;
						memPtr->m_jointDamping = 0;//todophysxLinks[i]->getLinearDamping();//??
						memPtr->m_jointFriction = 0;//todo
						memPtr->m_jointLowerLimit = 0;//todogetLink(i).m_jointLowerLimit;
						memPtr->m_jointUpperLimit = 0;//todogetLink(i).m_jointUpperLimit;
						memPtr->m_jointMaxForce = 0;//todogetLink(i).m_jointMaxForce;
						memPtr->m_jointMaxVelocity = 0;//todogetLink(i).m_jointMaxVelocity;

						//getLink(i).m_eVector.serialize(memPtr->m_parentComToThisPivotOffset);
						//getLink(i).m_dVector.serialize(memPtr->m_thisPivotToThisComOffset);
						//getLink(i).m_zeroRotParentToThis.serialize(memPtr->m_zeroRotParentToThis);
						
						
						{
							char *name = (char *)ser.findNameForPointer(physxLinks[i]->getName());
							memPtr->m_linkName = (char *)ser.getUniquePointer(name);
							if (memPtr->m_linkName)
							{
								ser.serializeName(name);
							}
						}
						{
							char *name = (char *)ser.findNameForPointer(physxLinks[i]->getName());
							memPtr->m_jointName = (char *)ser.getUniquePointer(name);
							if (memPtr->m_jointName)
							{
								ser.serializeName(name);
							}
						}
						memPtr->m_linkCollider = (CollisionObject2Data *)ser.getUniquePointer(0);
					}
					ser.finalizeChunk(chunk, btMultiBodyLinkDataName, DRX3D_ARRAY_CODE, (uk )articulation);
				}
				mbd->m_links = mbd->m_numLinks ? (MultiBodyLinkData *)ser.getUniquePointer((uk )articulation) : 0;

				// Fill padding with zeros to appease msan.
#ifdef DRX3D_USE_DOUBLE_PRECISION
				memset(mbd->m_padding, 0, sizeof(mbd->m_padding));
#endif

				tukk structType = btMultiBodyDataName;
				ser.finalizeChunk(chunk, structType, DRX3D_MULTIBODY_CODE,0);
				i32 streamSizeInBytes = ser.getCurrentBufferSize();
				serverStatusOut.m_numDataStreamBytes = streamSizeInBytes;
				
			}
		}
#if 0
		Transform2 rootTrans;
		rootTrans.setOrigin(pos);
		rootTrans.setRotation(orn);
		u2b.setRootTransformInWorld(rootTrans);
		bool ok = processImportedObjects(fileName, bufferServerToClient, bufferSizeInBytes, useMultiBody, flags, u2b);
		if (ok)
		{
			if (m_data->m_sdfRecentLoadedBodies.size() == 1)
			{
				*bodyUniqueIdPtr = m_data->m_sdfRecentLoadedBodies[0];
			}
			m_data->m_sdfRecentLoadedBodies.clear();
		}
#endif
		return true;
	}
	return false;
}

bool PhysXServerCommandProcessor::processRequestBodyInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;
	DRX3D_PROFILE("CMD_REQUEST_BODY_INFO");

	const SdfRequestInfoArgs& sdfInfoArgs = clientCmd.m_sdfRequestInfoArgs;
	//stream info into memory
	i32 streamSizeInBytes = 0;  //createBodyInfoStream(sdfInfoArgs.m_bodyUniqueId, bufferServerToClient, bufferSizeInBytes);

	serverStatusOut.m_type = CMD_BODY_INFO_COMPLETED;

	
	serverStatusOut.m_numDataStreamBytes = streamSizeInBytes;

	return hasStatus;
}

bool PhysXServerCommandProcessor::processForwardDynamicsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;

	DRX3D_PROFILE("CMD_STEP_FORWARD_SIMULATION");
	
	i32 numArt = m_data->m_scene->getNbArticulations();
	
	{
		D3_PROFILE("clear Contacts");
		m_data->m_contactPoints.clear();
	}
	{
		D3_PROFILE("PhysX_simulate_fetchResults");
		
		m_data->m_scene->simulate(m_data->m_physicsDeltaTime);
		m_data->m_scene->fetchResults(true);
	}
	{
		D3_PROFILE("syncTransform");
		if (m_data->m_pluginManager.getRenderInterface())
		{

			//sync transforms...

			b3AlignedObjectArray<i32> usedHandles;
			m_data->m_bodyHandles.getUsedHandles(usedHandles);

			for (i32 i = 0; i < usedHandles.size(); i++)
			{
				InternalPhysXBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(usedHandles[i]);

				
				physx::PxRigidActor* actor = 0;
				if (bodyHandle->m_rigidDynamic)
					actor = bodyHandle->m_rigidDynamic;
				if (bodyHandle->m_rigidStatic)
					actor = bodyHandle->m_rigidStatic;

				if (actor)
				{
					Transform2 tr;
					tr.setIdentity();
					physx::PxTransform pt = actor->getGlobalPose();
					tr.setOrigin(Vec3(pt.p[0], pt.p[1], pt.p[2]));
					tr.setRotation(Quat(pt.q.x, pt.q.y, pt.q.z, pt.q.w));
					Vec3 localScaling(1, 1, 1);//??
					MyPhysXUserData* ud = (MyPhysXUserData*)actor->userData;
					m_data->m_pluginManager.getRenderInterface()->syncTransform(ud->m_graphicsUniqueId, tr, localScaling);
				}

				if (bodyHandle->mArticulation)
				{
					
					physx::PxArticulationLink* physxLinks[64];
					physx::PxU32 bufferSize = 64;
					physx::PxU32 startIndex = 0;
					i32 numLinks2 = bodyHandle->mArticulation->getLinks(physxLinks, bufferSize, startIndex);

					for (i32 l = 0; l < numLinks2; l++)
					{
						MyPhysXUserData* ud = (MyPhysXUserData*)physxLinks[l]->userData;
						if (ud)
						{
							Transform2 tr;
							tr.setIdentity();
							physx::PxTransform pt = physxLinks[l]->getGlobalPose();
							tr.setOrigin(Vec3(pt.p[0], pt.p[1], pt.p[2]));
							tr.setRotation(Quat(pt.q.x, pt.q.y, pt.q.z, pt.q.w));
							Vec3 localScaling(1, 1, 1);//??
							m_data->m_pluginManager.getRenderInterface()->syncTransform(ud->m_graphicsUniqueId, tr, localScaling);
						}
					}
				}
			}
		}
		
		{
			D3_PROFILE("render");
			//m_data->m_pluginManager.getRenderInterface()->render();
			u8* pixelRGBA = 0;
			i32 numRequestedPixels = 0;
			float* depthBuffer = 0;
			i32* segmentationMaskBuffer = 0;
			i32 startPixelIndex = 0;
			
			i32 width = 1024;
			i32 height = 768;
			m_data->m_pluginManager.getRenderInterface()->getWidthAndHeight(width, height);
			i32 numPixelsCopied = 0;
			

			m_data->m_pluginManager.getRenderInterface()->copyCameraImageData(pixelRGBA, numRequestedPixels,
				depthBuffer, numRequestedPixels,
				segmentationMaskBuffer, numRequestedPixels,
				startPixelIndex, &width, &height, &numPixelsCopied);
		}
	}

	SharedMemoryStatus& serverCmd = serverStatusOut;
	serverCmd.m_type = CMD_STEP_FORWARD_SIMULATION_COMPLETED;
	return hasStatus;
}

bool PhysXServerCommandProcessor::processSendPhysicsParametersCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;

	DRX3D_PROFILE("CMD_SEND_PHYSICS_SIMULATION_PARAMETERS");

	if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_DELTA_TIME)
	{
		m_data->m_physicsDeltaTime = clientCmd.m_physSimParamArgs.m_deltaTime;
	}

	if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_GRAVITY)
	{
		Vec3 grav(clientCmd.m_physSimParamArgs.m_gravityAcceleration[0],
			clientCmd.m_physSimParamArgs.m_gravityAcceleration[1],
			clientCmd.m_physSimParamArgs.m_gravityAcceleration[2]);
		
		m_data->m_scene->setGravity(physx::PxVec3(grav[0], grav[1], grav[2]));

		if (m_data->m_verboseOutput)
		{
			drx3DPrintf("Updated Gravity: %f,%f,%f", grav[0], grav[1], grav[2]);
		}
	}
#if 0
	if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_NUM_SOLVER_ITERATIONS)
	{
		
		m_data->m_dynamicsWorld->getSolverInfo().m_numIterations = clientCmd.m_physSimParamArgs.m_numSolverIterations;
	}


	if (clientCmd.m_updateFlags & SIM_PARAM_ENABLE_CONE_FRICTION)
	{
		if (clientCmd.m_physSimParamArgs.m_enableConeFriction)
		{
			m_data->m_dynamicsWorld->getSolverInfo().m_solverMode &=~SOLVER_DISABLE_IMPLICIT_CONE_FRICTION;
		} else
		{
			m_data->m_dynamicsWorld->getSolverInfo().m_solverMode |=SOLVER_DISABLE_IMPLICIT_CONE_FRICTION;
		}
	}
	if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_DETERMINISTIC_OVERLAPPING_PAIRS)
	{
		m_data->m_dynamicsWorld->getDispatchInfo().m_deterministicOverlappingPairs = (clientCmd.m_physSimParamArgs.m_deterministicOverlappingPairs!=0);
	}

	if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_CCD_ALLOWED_PENETRATION)
	{
		m_data->m_dynamicsWorld->getDispatchInfo().m_allowedCcdPenetration = clientCmd.m_physSimParamArgs.m_allowedCcdPenetration;
	}
	
	if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_JOINT_FEEDBACK_MODE)
	{
		gJointFeedbackInWorldSpace = (clientCmd.m_physSimParamArgs.m_jointFeedbackMode&JOINT_FEEDBACK_IN_WORLD_SPACE)!=0;
		gJointFeedbackInJointFrame = (clientCmd.m_physSimParamArgs.m_jointFeedbackMode&JOINT_FEEDBACK_IN_JOINT_FRAME)!=0;
	}

	
	if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_REAL_TIME_SIMULATION)
	{
		m_data->m_useRealTimeSimulation = (clientCmd.m_physSimParamArgs.m_useRealTimeSimulation!=0);
	}
					
	//see 
	if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_INTERNAL_SIMULATION_FLAGS)
	{
		//these flags are for internal/temporary/easter-egg/experimental demo purposes, use at own risk
		gInternalSimFlags = clientCmd.m_physSimParamArgs.m_internalSimFlags;
	}

	

	if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_SOLVER_RESIDULAL_THRESHOLD)
	{
		m_data->m_dynamicsWorld->getSolverInfo().m_leastSquaresResidualThreshold = clientCmd.m_physSimParamArgs.m_solverResidualThreshold;
	}
	

	if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_CONTACT_BREAKING_THRESHOLD)
	{
		gContactBreakingThreshold = clientCmd.m_physSimParamArgs.m_contactBreakingThreshold;
	}
	
	if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_CONTACT_SLOP)
	{
		m_data->m_dynamicsWorld->getSolverInfo().m_linearSlop = clientCmd.m_physSimParamArgs.m_contactSlop;
	}

	if (clientCmd.m_updateFlags&SIM_PARAM_ENABLE_SAT)
	{
		m_data->m_dynamicsWorld->getDispatchInfo().m_enableSatConvex = clientCmd.m_physSimParamArgs.m_enableSAT!=0;
	}


	if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_COLLISION_FILTER_MODE)
	{
		m_data->m_broadphaseCollisionFilterCallback->m_filterMode = clientCmd.m_physSimParamArgs.m_collisionFilterMode;
	}

	if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_USE_SPLIT_IMPULSE)
	{
		m_data->m_dynamicsWorld->getSolverInfo().m_splitImpulse = clientCmd.m_physSimParamArgs.m_useSplitImpulse;
	}
	if (clientCmd.m_updateFlags &SIM_PARAM_UPDATE_SPLIT_IMPULSE_PENETRATION_THRESHOLD)
	{
		m_data->m_dynamicsWorld->getSolverInfo().m_splitImpulsePenetrationThreshold = clientCmd.m_physSimParamArgs.m_splitImpulsePenetrationThreshold;
	}
					

    if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_NUM_SIMULATION_SUB_STEPS)
    {
        m_data->m_numSimulationSubSteps = clientCmd.m_physSimParamArgs.m_numSimulationSubSteps;
    }

	if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_DEFAULT_CONTACT_ERP)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_erp2 = clientCmd.m_physSimParamArgs.m_defaultContactERP;
    }

	if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_DEFAULT_NON_CONTACT_ERP)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_erp = clientCmd.m_physSimParamArgs.m_defaultNonContactERP;
    }

	if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_DEFAULT_FRICTION_ERP)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_frictionERP = clientCmd.m_physSimParamArgs.m_frictionERP;
    }

    if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_DEFAULT_GLOBAL_CFM)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_globalCfm = clientCmd.m_physSimParamArgs.m_defaultGlobalCFM;
    }

    if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_DEFAULT_FRICTION_CFM)
    {
		m_data->m_dynamicsWorld->getSolverInfo().m_frictionCFM = clientCmd.m_physSimParamArgs.m_frictionCFM;
    }

	if (clientCmd.m_updateFlags&SIM_PARAM_UPDATE_RESTITUTION_VELOCITY_THRESHOLD)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_restitutionVelocityThreshold = clientCmd.m_physSimParamArgs.m_restitutionVelocityThreshold;
    }

					

	if (clientCmd.m_updateFlags&SIM_PARAM_ENABLE_FILE_CACHING)
    {
		b3EnableFileCaching(clientCmd.m_physSimParamArgs.m_enableFileCaching);
    }

#endif

	SharedMemoryStatus& serverCmd = serverStatusOut;
	serverCmd.m_type = CMD_CLIENT_COMMAND_COMPLETED;
	return hasStatus;
}

bool PhysXServerCommandProcessor::processRequestActualStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;
	serverStatusOut.m_type = CMD_ACTUAL_STATE_UPDATE_FAILED;

	i32 bodyUniqueId = clientCmd.m_requestActualStateInformationCommandArgument.m_bodyUniqueId;
	InternalPhysXBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(bodyUniqueId);

	if (bodyHandle->mArticulation)
	{
		DRX3D_PROFILE("CMD_REQUEST_ACTUAL_STATE");
		if (m_data->m_verboseOutput)
		{
			drx3DPrintf("Sending the actual state (Q,U)");
		}

		{
			SharedMemoryStatus& serverCmd = serverStatusOut;
			serverStatusOut.m_type = CMD_ACTUAL_STATE_UPDATE_COMPLETED;

			serverCmd.m_sendActualStateArgs.m_bodyUniqueId = bodyUniqueId;
			serverCmd.m_sendActualStateArgs.m_numLinks = bodyHandle->mArticulation->getNbLinks()-1; //skip base!

			i32 totalDegreeOfFreedomQ = 0;
			i32 totalDegreeOfFreedomU = 0;

			if (serverCmd.m_sendActualStateArgs.m_numLinks >= MAX_DEGREE_OF_FREEDOM)
			{
				serverStatusOut.m_type = CMD_ACTUAL_STATE_UPDATE_FAILED;
				hasStatus = true;
				return hasStatus;
			}

			bool computeForwardKinematics = ((clientCmd.m_updateFlags & ACTUAL_STATE_COMPUTE_FORWARD_KINEMATICS) != 0);
			bool computeLinkVelocities = ((clientCmd.m_updateFlags & ACTUAL_STATE_COMPUTE_LINKVELOCITY) != 0);

			if (computeForwardKinematics || computeLinkVelocities)
			{
				//todo:check this
			}

			physx::PxArticulationLink* physxLinks[64];
			physx::PxU32 bufferSize = 64;
			physx::PxU32 startIndex = 0;
			i32 numLinks2 = bodyHandle->mArticulation->getLinks(physxLinks, bufferSize, startIndex);

			{
				serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[0] = 0;
				serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[1] = 0;
				serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[2] = 0;

				serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[3] = 0;
				serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[4] = 0;
				serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[5] = 0;
				serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[6] = 1;

				physx::PxArticulationLink* l = physxLinks[0];
				physx::PxVec3 pos = l->getGlobalPose().p;
				physx::PxQuat orn = l->getGlobalPose().q;
				//base position in world space, carthesian
				serverCmd.m_sendActualStateArgs.m_actualStateQ[0] = pos[0];
				serverCmd.m_sendActualStateArgs.m_actualStateQ[1] = pos[1];
				serverCmd.m_sendActualStateArgs.m_actualStateQ[2] = pos[2];

				//base orientation, quaternion x,y,z,w, in world space, carthesian
				serverCmd.m_sendActualStateArgs.m_actualStateQ[3] = orn.x;
				serverCmd.m_sendActualStateArgs.m_actualStateQ[4] = orn.y;
				serverCmd.m_sendActualStateArgs.m_actualStateQ[5] = orn.z;
				serverCmd.m_sendActualStateArgs.m_actualStateQ[6] = orn.w;
				totalDegreeOfFreedomQ += 7;  //pos + quaternion

				physx::PxVec3 linVel = l->getLinearVelocity();
				physx::PxVec3 angVel = l->getAngularVelocity();

				//base linear velocity (in world space, carthesian)
				serverCmd.m_sendActualStateArgs.m_actualStateQdot[0] = linVel.x;
				serverCmd.m_sendActualStateArgs.m_actualStateQdot[1] = linVel.y;
				serverCmd.m_sendActualStateArgs.m_actualStateQdot[2] = linVel.z;

				//base angular velocity (in world space, carthesian)
				serverCmd.m_sendActualStateArgs.m_actualStateQdot[3] = angVel.x;
				serverCmd.m_sendActualStateArgs.m_actualStateQdot[4] = angVel.y;
				serverCmd.m_sendActualStateArgs.m_actualStateQdot[5] = angVel.z;
				totalDegreeOfFreedomU += 6;                                      //3 linear and 3 angular DOF
			}

			physx::PxArticulationCache* c = bodyHandle->mArticulation->createCache();
			bodyHandle->mArticulation->copyInternalStateToCache(*c, physx::PxArticulationCache::ePOSITION | physx::PxArticulationCache::eVELOCITY);// physx::PxArticulationCache::eALL);

			AlignedObjectArray<i32> dofStarts;
			dofStarts.resize(numLinks2);
			dofStarts[0] = 0; //We know that the root link does not have a joint
							  //cache positions in PhysX may be reshuffled, see
							  //http://gameworksdocs.nvidia.com/PhysX/4.0/documentation/PhysXGuide/Manual/Articulations.html

			for (i32 i = 1; i < numLinks2; ++i)
			{
				i32  llIndex = physxLinks[i]->getLinkIndex();
				i32  dofs = physxLinks[i]->getInboundJointDof();

				dofStarts[llIndex] = dofs;
			}

			i32 count = 0;
			for (i32 i = 1; i < numLinks2; ++i)
			{
				i32  dofs = dofStarts[i];
				dofStarts[i] = count;
				count += dofs;
			}

			//start at 1, 0 is the base/root, handled above
			for (i32 l = 1; l < numLinks2; l++)
			{
				i32  dofs = physxLinks[l]->getInboundJointDof();
				i32  llIndex = physxLinks[l]->getLinkIndex();
				
				for (i32 d = 0; d < dofs; d++)
				{
					serverCmd.m_sendActualStateArgs.m_actualStateQ[totalDegreeOfFreedomQ++] = c->jointPosition[dofStarts[llIndex + d]];
					serverCmd.m_sendActualStateArgs.m_actualStateQ[totalDegreeOfFreedomU++] = c->jointVelocity[dofStarts[llIndex + d]];
				}
			}

			serverCmd.m_sendActualStateArgs.m_numDegreeOfFreedomQ = totalDegreeOfFreedomQ;
			serverCmd.m_sendActualStateArgs.m_numDegreeOfFreedomU = totalDegreeOfFreedomU;

			hasStatus = true;
		}
	}


	return hasStatus;
}

bool PhysXServerCommandProcessor::processResetSimulationCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = true;
	DRX3D_PROFILE("CMD_RESET_SIMULATION");

	resetSimulation();

	SharedMemoryStatus& serverCmd = serverStatusOut;
	serverCmd.m_type = CMD_RESET_SIMULATION_COMPLETED;
	return hasStatus;
}

bool PhysXServerCommandProcessor::receiveStatus(struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	return false;
}

#endif  //DRX3D_ENABLE_PHYSX