#include "../PhysicsServerSharedMemory.h"
#include "../PosixSharedMemory.h"
#include "../Win32SharedMemory.h"

#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/DynamicsCommon.h>

#include <drx3D/Maths/Linear/Transform2.h>

#include <drx3D/Common/b3Logging.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include "../SharedMemoryBlock.h"

#include "../PhysicsCommandProcessorInterface.h"

//number of shared memory blocks == number of simultaneous connections
#define MAX_SHARED_MEMORY_BLOCKS 2

struct PhysicsServerSharedMemoryInternalData
{
	///end handle management

	SharedMemoryInterface* m_sharedMemory;
	bool m_ownsSharedMemory;

	SharedMemoryBlock* m_testBlocks[MAX_SHARED_MEMORY_BLOCKS];
	i32 m_sharedMemoryKey;
	bool m_areConnected[MAX_SHARED_MEMORY_BLOCKS];
	bool m_verboseOutput;
	CommandProcessorInterface* m_commandProcessor;
	CommandProcessorCreationInterface* m_commandProcessorCreator;

	PhysicsServerSharedMemoryInternalData()
		: m_sharedMemory(0),
		  m_ownsSharedMemory(false),
		  m_sharedMemoryKey(SHARED_MEMORY_KEY),
		  m_verboseOutput(false),
		  m_commandProcessor(0)

	{
		for (i32 i = 0; i < MAX_SHARED_MEMORY_BLOCKS; i++)
		{
			m_testBlocks[i] = 0;
			m_areConnected[i] = false;
		}
	}

	SharedMemoryStatus& createServerStatus(i32 statusType, i32 sequenceNumber, i32 timeStamp, i32 blockIndex)
	{
		SharedMemoryStatus& serverCmd = m_testBlocks[blockIndex]->m_serverCommands[0];
		serverCmd.m_type = statusType;
		serverCmd.m_sequenceNumber = sequenceNumber;
		serverCmd.m_timeStamp = timeStamp;
		return serverCmd;
	}
	void submitServerStatus(SharedMemoryStatus& status, i32 blockIndex)
	{
		m_testBlocks[blockIndex]->m_numServerCommands++;
	}
};

PhysicsServerSharedMemory::PhysicsServerSharedMemory(CommandProcessorCreationInterface* commandProcessorCreator, SharedMemoryInterface* sharedMem, i32 bla)
{
	m_data = new PhysicsServerSharedMemoryInternalData();
	m_data->m_commandProcessorCreator = commandProcessorCreator;
	if (sharedMem)
	{
		m_data->m_sharedMemory = sharedMem;
		m_data->m_ownsSharedMemory = false;
	}
	else
	{
#ifdef _WIN32
		m_data->m_sharedMemory = new Win32SharedMemoryServer();
#else
		m_data->m_sharedMemory = new PosixSharedMemory();
#endif
		m_data->m_ownsSharedMemory = true;
	}

	m_data->m_commandProcessor = commandProcessorCreator->createCommandProcessor();
}

PhysicsServerSharedMemory::~PhysicsServerSharedMemory()
{
	if (m_data->m_sharedMemory)
	{
		if (m_data->m_verboseOutput)
		{
			drx3DPrintf("m_sharedMemory\n");
		}
		if (m_data->m_ownsSharedMemory)
		{
			delete m_data->m_sharedMemory;
		}
		m_data->m_sharedMemory = 0;
	}

	m_data->m_commandProcessorCreator->deleteCommandProcessor(m_data->m_commandProcessor);
	delete m_data;
}

/*void PhysicsServerSharedMemory::resetDynamicsWorld()
{
	m_data->m_commandProcessor->deleteDynamicsWorld();
	m_data->m_commandProcessor ->createEmptyDynamicsWorld();
}
*/
void PhysicsServerSharedMemory::setSharedMemoryKey(i32 key)
{
	m_data->m_sharedMemoryKey = key;
}

bool PhysicsServerSharedMemory::connectSharedMemory(struct GUIHelperInterface* guiHelper)
{
	m_data->m_commandProcessor->setGuiHelper(guiHelper);

	bool allowCreation = true;
	bool allConnected = false;
	i32 numConnected = 0;

	i32 counter = 0;
	for (i32 block = 0; block < MAX_SHARED_MEMORY_BLOCKS; block++)
	{
		if (m_data->m_areConnected[block])
		{
			allConnected = true;
			numConnected++;
			drx3DWarning("connectSharedMemory, while already connected");
			continue;
		}
		do
		{
			m_data->m_testBlocks[block] = (SharedMemoryBlock*)m_data->m_sharedMemory->allocateSharedMemory(m_data->m_sharedMemoryKey + block, SHARED_MEMORY_SIZE, allowCreation);
			if (m_data->m_testBlocks[block])
			{
				i32 magicId = m_data->m_testBlocks[block]->m_magicId;
				if (m_data->m_verboseOutput)
				{
					drx3DPrintf("magicId = %d\n", magicId);
				}

				if (m_data->m_testBlocks[block]->m_magicId != SHARED_MEMORY_MAGIC_NUMBER)
				{
					InitSharedMemoryBlock(m_data->m_testBlocks[block]);
					if (m_data->m_verboseOutput)
					{
						drx3DPrintf("Created and initialized shared memory block\n");
					}
					m_data->m_areConnected[block] = true;
					numConnected++;
				}
				else
				{
					m_data->m_sharedMemory->releaseSharedMemory(m_data->m_sharedMemoryKey + block, SHARED_MEMORY_SIZE);
					m_data->m_testBlocks[block] = 0;
					m_data->m_areConnected[block] = false;
				}
			}
			else
			{
				//drx3DError("Cannot connect to shared memory");
				m_data->m_areConnected[block] = false;
			}
		} while (counter++ < 10 && !m_data->m_areConnected[block]);
		if (!m_data->m_areConnected[block])
		{
			drx3DError("Server cannot connect to shared memory.\n");
		}
	}

	allConnected = (numConnected == MAX_SHARED_MEMORY_BLOCKS);

	return allConnected;
}

void PhysicsServerSharedMemory::disconnectSharedMemory(bool deInitializeSharedMemory)
{
	//m_data->m_commandProcessor->deleteDynamicsWorld();

	m_data->m_commandProcessor->setGuiHelper(0);

	if (m_data->m_verboseOutput)
	{
		drx3DPrintf("releaseSharedMemory1\n");
	}
	for (i32 block = 0; block < MAX_SHARED_MEMORY_BLOCKS; block++)
	{
		if (m_data->m_testBlocks[block])
		{
			if (m_data->m_verboseOutput)
			{
				drx3DPrintf("m_testBlock1\n");
			}
			if (deInitializeSharedMemory)
			{
				m_data->m_testBlocks[block]->m_magicId = 0;
				if (m_data->m_verboseOutput)
				{
					drx3DPrintf("De-initialized shared memory, magic id = %d\n", m_data->m_testBlocks[block]->m_magicId);
				}
			}
			Assert(m_data->m_sharedMemory);
			m_data->m_sharedMemory->releaseSharedMemory(m_data->m_sharedMemoryKey + block, SHARED_MEMORY_SIZE);
		}
		m_data->m_testBlocks[block] = 0;
		m_data->m_areConnected[block] = false;
	}
}

void PhysicsServerSharedMemory::releaseSharedMemory()
{
	disconnectSharedMemory(true);
}

void PhysicsServerSharedMemory::stepSimulationRealTime(double dtInSec, const struct b3VRControllerEvent* vrEvents, i32 numVREvents, const struct b3KeyboardEvent* keyEvents, i32 numKeyEvents, const struct b3MouseEvent* mouseEvents, i32 numMouseEvents)
{
	m_data->m_commandProcessor->stepSimulationRealTime(dtInSec, vrEvents, numVREvents, keyEvents, numKeyEvents, mouseEvents, numMouseEvents);
}

void PhysicsServerSharedMemory::enableRealTimeSimulation(bool enableRealTimeSim)
{
	m_data->m_commandProcessor->enableRealTimeSimulation(enableRealTimeSim);
}

bool PhysicsServerSharedMemory::isRealTimeSimulationEnabled() const
{
	return m_data->m_commandProcessor->isRealTimeSimulationEnabled();
}

void PhysicsServerSharedMemory::reportNotifications()
{
	m_data->m_commandProcessor->reportNotifications();
}

void PhysicsServerSharedMemory::processClientCommands()
{
	//handle client commands in any of the plugins
	m_data->m_commandProcessor->processClientCommands();

	//now handle the client commands from the shared memory
	for (i32 block = 0; block < MAX_SHARED_MEMORY_BLOCKS; block++)
	{
		if (m_data->m_areConnected[block] && m_data->m_testBlocks[block])
		{
			m_data->m_commandProcessor->replayLogCommand(&m_data->m_testBlocks[block]->m_bulletStreamDataServerToClientRefactor[0], SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE);

			///we ignore overflow of integer for now
			if (m_data->m_testBlocks[block]->m_numClientCommands > m_data->m_testBlocks[block]->m_numProcessedClientCommands)
			{
				//DRX3D_PROFILE("processClientCommand");

				//until we implement a proper ring buffer, we assume always maximum of 1 outstanding commands
				Assert(m_data->m_testBlocks[block]->m_numClientCommands == m_data->m_testBlocks[block]->m_numProcessedClientCommands + 1);

				const SharedMemoryCommand& clientCmd = m_data->m_testBlocks[block]->m_clientCommands[0];

				m_data->m_testBlocks[block]->m_numProcessedClientCommands++;
				//todo, timeStamp
				i32 timeStamp = 0;
				SharedMemoryStatus& serverStatusOut = m_data->createServerStatus(CMD_DRX3D_DATA_STREAM_RECEIVED_COMPLETED, clientCmd.m_sequenceNumber, timeStamp, block);
				bool hasStatus = m_data->m_commandProcessor->processCommand(clientCmd, serverStatusOut, &m_data->m_testBlocks[block]->m_bulletStreamDataServerToClientRefactor[0], SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE);
				if (hasStatus)
				{
					m_data->submitServerStatus(serverStatusOut, block);
				}
			}
		}
	}
}

void PhysicsServerSharedMemory::renderScene(i32 renderFlags)
{
	m_data->m_commandProcessor->renderScene(renderFlags);
}

void PhysicsServerSharedMemory::syncPhysicsToGraphics()
{
	m_data->m_commandProcessor->syncPhysicsToGraphics();
}

void PhysicsServerSharedMemory::physicsDebugDraw(i32 debugDrawFlags)
{
	m_data->m_commandProcessor->physicsDebugDraw(debugDrawFlags);
}

bool PhysicsServerSharedMemory::pickBody(const Vec3& rayFromWorld, const Vec3& rayToWorld)
{
	return m_data->m_commandProcessor->pickBody(rayFromWorld, rayToWorld);
}

bool PhysicsServerSharedMemory::movePickedBody(const Vec3& rayFromWorld, const Vec3& rayToWorld)
{
	return m_data->m_commandProcessor->movePickedBody(rayFromWorld, rayToWorld);
}
void PhysicsServerSharedMemory::removePickingConstraint()
{
	m_data->m_commandProcessor->removePickingConstraint();
}

void PhysicsServerSharedMemory::enableCommandLogging(bool enable, tukk fileName)
{
	m_data->m_commandProcessor->enableCommandLogging(enable, fileName);
}

void PhysicsServerSharedMemory::replayFromLogFile(tukk fileName)
{
	m_data->m_commandProcessor->replayFromLogFile(fileName);
}

const Vec3& PhysicsServerSharedMemory::getVRTeleportPosition() const
{
	return m_data->m_commandProcessor->getVRTeleportPosition();
}
void PhysicsServerSharedMemory::setVRTeleportPosition(const Vec3& vrTeleportPos)
{
	m_data->m_commandProcessor->setVRTeleportPosition(vrTeleportPos);
}

const Quat& PhysicsServerSharedMemory::getVRTeleportOrientation() const
{
	return m_data->m_commandProcessor->getVRTeleportOrientation();
}
void PhysicsServerSharedMemory::setVRTeleportOrientation(const Quat& vrTeleportOrn)
{
	m_data->m_commandProcessor->setVRTeleportOrientation(vrTeleportOrn);
}
