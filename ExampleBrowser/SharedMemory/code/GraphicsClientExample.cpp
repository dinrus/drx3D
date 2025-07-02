#include "../GraphicsClientExample.h"
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Common/b3Logging.h>
#include <drx3D/SharedMemory/GraphicsSharedMemoryCommands.h>
#include <drx3D/SharedMemory/PosixSharedMemory.h>
#include <drx3D/SharedMemory/Win32SharedMemory.h>
#include <drx3D/SharedMemory/GraphicsSharedMemoryBlock.h>
#include <drx3D/Common/b3Scalar.h>

class GraphicsClientExample : public CommonExampleInterface
{
protected:
	
	GUIHelperInterface* m_guiHelper;
	bool m_waitingForServer;
	GraphicsSharedMemoryBlock* m_testBlock1;
	SharedMemoryInterface* m_sharedMemory;
	GraphicsSharedMemoryStatus m_lastServerStatus;
	i32 m_sharedMemoryKey;
	bool m_isConnected;

public:
	GraphicsClientExample(GUIHelperInterface* helper, i32 options);
	virtual ~GraphicsClientExample();

	virtual void initPhysics();
	virtual void stepSimulation(float deltaTime);

	virtual void resetCamera()
	{
		float dist = 3.45;
		float pitch = -16.2;
		float yaw = 287;
		float targetPos[3] = {2.05, 0.02, 0.53};  //-3,2.8,-2.5};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}


	virtual bool isConnected()
	{
		return m_isConnected;
	}

	bool canSubmitCommand() const
	{
		if (m_isConnected && !m_waitingForServer)
		{
			if (m_testBlock1->m_magicId == GRAPHICS_SHARED_MEMORY_MAGIC_NUMBER)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		return false;
	}

	struct GraphicsSharedMemoryCommand* getAvailableSharedMemoryCommand()
	{
		static i32 sequence = 0;
		if (m_testBlock1)
		{
			m_testBlock1->m_clientCommands[0].m_sequenceNumber = sequence++;
			return &m_testBlock1->m_clientCommands[0];
		}
		return 0;
	}

	bool submitClientCommand(const GraphicsSharedMemoryCommand& command)
	{
		/// at the moment we allow a maximum of 1 outstanding command, so we check for this
		// once the server processed the command and returns a status, we clear the flag
		// "m_data->m_waitingForServer" and allow submitting the next command

		if (!m_waitingForServer)
		{
			//printf("submit command of type %d\n", command.m_type);

			if (&m_testBlock1->m_clientCommands[0] != &command)
			{
				m_testBlock1->m_clientCommands[0] = command;
			}
			m_testBlock1->m_numClientCommands++;
			m_waitingForServer = true;
			return true;
		}
		return false;
	}


	const GraphicsSharedMemoryStatus* processServerStatus()
	{
		// SharedMemoryStatus* stat = 0;

		if (!m_testBlock1)
		{
			m_lastServerStatus.m_type = GFX_CMD_SHARED_MEMORY_NOT_INITIALIZED;
			return &m_lastServerStatus;
		}

		if (!m_waitingForServer)
		{
			return 0;
		}

		if (m_testBlock1->m_magicId != GRAPHICS_SHARED_MEMORY_MAGIC_NUMBER)
		{
			m_lastServerStatus.m_type = GFX_CMD_SHARED_MEMORY_NOT_INITIALIZED;
			return &m_lastServerStatus;
		}


		if (m_testBlock1->m_numServerCommands >
			m_testBlock1->m_numProcessedServerCommands)
		{
			D3_PROFILE("processServerCMD");
			drx3DAssert(m_testBlock1->m_numServerCommands ==
				m_testBlock1->m_numProcessedServerCommands + 1);

			const GraphicsSharedMemoryStatus& serverCmd = m_testBlock1->m_serverCommands[0];

			m_lastServerStatus = serverCmd;
			
			//       EnumSharedMemoryServerStatus s = (EnumSharedMemoryServerStatus)serverCmd.m_type;
			// consume the command
			switch (serverCmd.m_type)
			{
				case GFX_CMD_CLIENT_COMMAND_COMPLETED:
				{
					D3_PROFILE("CMD_CLIENT_COMMAND_COMPLETED");

					
					break;
				}
				default:
				{
				}
			}

			m_testBlock1->m_numProcessedServerCommands++;
			// we don't have more than 1 command outstanding (in total, either server or client)
			drx3DAssert(m_testBlock1->m_numProcessedServerCommands ==
				m_testBlock1->m_numServerCommands);

			if (m_testBlock1->m_numServerCommands ==
				m_testBlock1->m_numProcessedServerCommands)
			{
				m_waitingForServer = false;
			}
			else
			{
				m_waitingForServer = true;
			}


			return &m_lastServerStatus;
		}
		return 0;
	}

	bool connect()
	{
		/// server always has to create and initialize shared memory
		bool allowCreation = false;
		m_testBlock1 = (GraphicsSharedMemoryBlock*)m_sharedMemory->allocateSharedMemory(
			m_sharedMemoryKey, GRAPHICS_SHARED_MEMORY_SIZE, allowCreation);

		if (m_testBlock1)
		{
			if (m_testBlock1->m_magicId != GRAPHICS_SHARED_MEMORY_MAGIC_NUMBER)
			{
				drx3DError("Error connecting to shared memory: please start server before client\n");
				m_sharedMemory->releaseSharedMemory(m_sharedMemoryKey,
					GRAPHICS_SHARED_MEMORY_SIZE);
				m_testBlock1 = 0;
				return false;
			}
			else
			{
				m_isConnected = true;
			}
		}
		else
		{
			drx3DWarning("Cannot connect to shared memory");
			return false;
		}
		return true;
	}


	void disconnect()
	{
		if (m_isConnected && m_sharedMemory)
		{
			m_sharedMemory->releaseSharedMemory(m_sharedMemoryKey, GRAPHICS_SHARED_MEMORY_SIZE);
		}
		m_isConnected = false;
	}

	virtual void exitPhysics(){};
	
	virtual void physicsDebugDraw(i32 debugFlags)
	{
	}

	virtual void renderScene()
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


};



GraphicsClientExample::GraphicsClientExample(GUIHelperInterface* helper, i32 options)
	: m_guiHelper(helper),
	m_waitingForServer(false),
	m_testBlock1(0)
{
#ifdef _WIN32
	m_sharedMemory = new Win32SharedMemoryClient();
#else
	m_sharedMemory = new PosixSharedMemory();
#endif
	m_sharedMemoryKey = GRAPHICS_SHARED_MEMORY_KEY;
	m_isConnected = false;
	drx3DPrintf("Started GraphicsClientExample\n");
	connect();
}

GraphicsClientExample::~GraphicsClientExample()
{
	disconnect();
	delete m_sharedMemory;
}


void GraphicsClientExample::initPhysics()
{
	if (m_guiHelper && m_guiHelper->getParameterInterface())
	{
		i32 upAxis = 2;
		m_guiHelper->setUpAxis(upAxis);
	}
	
}

void GraphicsClientExample::stepSimulation(float deltaTime)
{
	GraphicsSharedMemoryCommand* cmd = getAvailableSharedMemoryCommand();
	if (cmd)
	{
		cmd->m_updateFlags = 0;
		cmd->m_type = GFX_CMD_0;
		submitClientCommand(*cmd);
	}
	const GraphicsSharedMemoryStatus* status = processServerStatus();
	if (status)
	{
		//handle it
	}
}

class CommonExampleInterface* GraphicsClientCreateFunc(struct CommonExampleOptions& options)
{
	GraphicsClientExample* example = new GraphicsClientExample(options.m_guiHelper, options.m_option);
	
	
	return example;
}
