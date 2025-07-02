
#include <drx3D/SharedMemory/SharedMemoryInProcessPhysicsC_API.h>
#include <drx3D/Common/b3Clock.h>

#include <drx3D/SharedMemory/PhysicsClientSharedMemory.h>
#include <drx3D/ExampleBrowser/InProcessExampleBrowser.h>
#include <stdio.h>
#include <string.h>
#include <drx3D/ExampleBrowser/SharedMemory/PhysicsServerExampleBullet2.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/SharedMemory/InProcessMemory.h>
#include <drx3D/SharedMemory/RemoteGUIHelper.h>

#include <drx3D/Common/b3Logging.h>
class InProcessPhysicsClientSharedMemoryMainThread : public PhysicsClientSharedMemory
{
	InProcessExampleBrowserMainThreadInternalData* m_data;
	b3Clock m_clock;

public:
	InProcessPhysicsClientSharedMemoryMainThread(i32 argc, tuk argv[], bool useInProcessMemory)
	{
		i32 newargc = argc + 3;
		tuk* newargv = (tuk*)malloc(sizeof(uk ) * newargc);
		tuk t0 = (tuk)"--unused";
		newargv[0] = t0;
		for (i32 i = 0; i < argc; i++)
			newargv[i + 1] = argv[i];
		newargv[argc + 1] = (tuk)"--logtostderr";
		newargv[argc + 2] = (tuk)"--start_demo_name=Physics Server";

		m_data = CreateInProcessExampleBrowserMainThread(newargc, newargv, useInProcessMemory);
		SharedMemoryInterface* shMem = GetSharedMemoryInterfaceMainThread(m_data);

		setSharedMemoryInterface(shMem);
	}

	virtual ~InProcessPhysicsClientSharedMemoryMainThread()
	{
		setSharedMemoryInterface(0);
		ShutDownExampleBrowserMainThread(m_data);
	}

	// return non-null if there is a status, nullptr otherwise
	virtual const struct SharedMemoryStatus* processServerStatus()
	{
		{
			if (IsExampleBrowserMainThreadTerminated(m_data))
			{
				PhysicsClientSharedMemory::disconnectSharedMemory();
			}
		}
		{
			u64 ms = m_clock.getTimeMilliseconds();
			if (ms > 2)
			{
				D3_PROFILE("m_clock.reset()");

				UpdateInProcessExampleBrowserMainThread(m_data);
				m_clock.reset();
			}
		}
		{
			b3Clock::usleep(0);
		}
		const SharedMemoryStatus* stat = 0;

		{
			stat = PhysicsClientSharedMemory::processServerStatus();
		}

		return stat;
	}

	virtual bool submitClientCommand(const struct SharedMemoryCommand& command)
	{
		//        btUpdateInProcessExampleBrowserMainThread(m_data);
		return PhysicsClientSharedMemory::submitClientCommand(command);
	}
};

DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerAndConnectMainThread(i32 argc, tuk argv[])
{
	InProcessPhysicsClientSharedMemoryMainThread* cl = new InProcessPhysicsClientSharedMemoryMainThread(argc, argv, 1);
	cl->setSharedMemoryKey(SHARED_MEMORY_KEY + 1);
	cl->connect();
	return (b3PhysicsClientHandle)cl;
}

DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerAndConnectMainThreadSharedMemory(i32 argc, tuk argv[])
{
	InProcessPhysicsClientSharedMemoryMainThread* cl = new InProcessPhysicsClientSharedMemoryMainThread(argc, argv, 0);
	cl->setSharedMemoryKey(SHARED_MEMORY_KEY + 1);
	cl->connect();
	return (b3PhysicsClientHandle)cl;
}



class InProcessPhysicsClientSharedMemory : public PhysicsClientSharedMemory
{
	InProcessExampleBrowserInternalData* m_data;
	tuk* m_newargv;

public:
	InProcessPhysicsClientSharedMemory(i32 argc, tuk argv[], bool useInProcessMemory)
	{
		i32 newargc = argc + 2;
		m_newargv = (tuk*)malloc(sizeof(uk ) * newargc);
		tuk t0 = (tuk)"--unused";
		m_newargv[0] = t0;

		for (i32 i = 0; i < argc; i++)
			m_newargv[i + 1] = argv[i];

		tuk t1 = (tuk)"--start_demo_name=Physics Server";
		m_newargv[argc + 1] = t1;
		m_data = CreateInProcessExampleBrowser(newargc, m_newargv, useInProcessMemory);
		SharedMemoryInterface* shMem = GetSharedMemoryInterface(m_data);
		setSharedMemoryInterface(shMem);
	}

	virtual ~InProcessPhysicsClientSharedMemory()
	{
		setSharedMemoryInterface(0);
		ShutDownExampleBrowser(m_data);
		free(m_newargv);
	}
};



DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerAndConnect(i32 argc, tuk argv[])
{
	InProcessPhysicsClientSharedMemory* cl = new InProcessPhysicsClientSharedMemory(argc, argv, 1);
	cl->setSharedMemoryKey(SHARED_MEMORY_KEY + 1);
	cl->connect();
	return (b3PhysicsClientHandle)cl;
}
DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerAndConnectSharedMemory(i32 argc, tuk argv[])
{
	InProcessPhysicsClientSharedMemory* cl = new InProcessPhysicsClientSharedMemory(argc, argv, 0);
	cl->setSharedMemoryKey(SHARED_MEMORY_KEY + 1);
	cl->connect();
	return (b3PhysicsClientHandle)cl;
}

class InProcessPhysicsClientExistingExampleBrowser : public PhysicsClientSharedMemory
{
	CommonExampleInterface* m_physicsServerExample;
	SharedMemoryInterface* m_sharedMem;
	b3Clock m_clock;
	zu64 m_prevTime;
	struct GUIHelperInterface* m_guiHelper;

public:
	InProcessPhysicsClientExistingExampleBrowser(struct GUIHelperInterface* guiHelper, bool useInProcessMemory, bool skipGraphicsUpdate, bool ownsGuiHelper)
	{
		m_guiHelper = 0;
		if (ownsGuiHelper)
		{
			m_guiHelper = guiHelper;
		}
		
		m_sharedMem = 0;
		CommonExampleOptions options(guiHelper);

		if (useInProcessMemory)
		{
			m_sharedMem = new InProcessMemory;
			options.m_sharedMem = m_sharedMem;
		}

		options.m_skipGraphicsUpdate = skipGraphicsUpdate;
		m_physicsServerExample = PhysicsServerCreateFuncBullet2(options);
		m_physicsServerExample->initPhysics();
		//m_physicsServerExample->resetCamera();
		setSharedMemoryInterface(m_sharedMem);
		m_clock.reset();
		m_prevTime = m_clock.getTimeMicroseconds();
	}
	virtual ~InProcessPhysicsClientExistingExampleBrowser()
	{
		m_physicsServerExample->exitPhysics();
		//s_instancingRenderer->removeAllInstances();
		delete m_physicsServerExample;
		delete m_sharedMem;
		delete m_guiHelper;
	}

	// return non-null if there is a status, nullptr otherwise
	virtual const struct SharedMemoryStatus* processServerStatus()
	{
		m_physicsServerExample->updateGraphics();

		zu64 curTime = m_clock.getTimeMicroseconds();
		zu64 dtMicro = curTime - m_prevTime;
		m_prevTime = curTime;

		double dt = double(dtMicro) / 1000000.;

		m_physicsServerExample->stepSimulation(dt);
		{
			b3Clock::usleep(0);
		}
		const SharedMemoryStatus* stat = 0;

		{
			stat = PhysicsClientSharedMemory::processServerStatus();
		}

		return stat;
	}

	virtual void renderScene()
	{
		m_physicsServerExample->renderScene();
	}
	virtual void debugDraw(i32 debugDrawMode)
	{
		m_physicsServerExample->physicsDebugDraw(debugDrawMode);
	}
	virtual bool mouseMoveCallback(float x, float y)
	{
		return m_physicsServerExample->mouseMoveCallback(x, y);
	}
	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y)
	{
		return m_physicsServerExample->mouseButtonCallback(button, state, x, y);
	}
};

void b3InProcessDebugDrawInternal(b3PhysicsClientHandle clientHandle, i32 debugDrawMode)
{
	InProcessPhysicsClientExistingExampleBrowser* cl = (InProcessPhysicsClientExistingExampleBrowser*)clientHandle;
	cl->debugDraw(debugDrawMode);
}
void b3InProcessRenderSceneInternal(b3PhysicsClientHandle clientHandle)
{
	InProcessPhysicsClientExistingExampleBrowser* cl = (InProcessPhysicsClientExistingExampleBrowser*)clientHandle;
	cl->renderScene();
}

i32 b3InProcessMouseMoveCallback(b3PhysicsClientHandle clientHandle, float x, float y)
{
	InProcessPhysicsClientExistingExampleBrowser* cl = (InProcessPhysicsClientExistingExampleBrowser*)clientHandle;
	return cl->mouseMoveCallback(x, y);
}
i32 b3InProcessMouseButtonCallback(b3PhysicsClientHandle clientHandle, i32 button, i32 state, float x, float y)
{
	InProcessPhysicsClientExistingExampleBrowser* cl = (InProcessPhysicsClientExistingExampleBrowser*)clientHandle;
	return cl->mouseButtonCallback(button, state, x, y);
}

DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerFromExistingExampleBrowserAndConnect(uk guiHelperPtr)
{
	static DummyGUIHelper noGfx;

	GUIHelperInterface* guiHelper = (GUIHelperInterface*)guiHelperPtr;
	if (!guiHelper)
	{
		guiHelper = &noGfx;
	}
	bool useInprocessMemory = true;
	bool skipGraphicsUpdate = false;

	InProcessPhysicsClientExistingExampleBrowser* cl = new InProcessPhysicsClientExistingExampleBrowser(guiHelper, useInprocessMemory, skipGraphicsUpdate, false);

	cl->connect();
	return (b3PhysicsClientHandle)cl;
}

extern i32 gSharedMemoryKey;

DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerFromExistingExampleBrowserAndConnect3(uk guiHelperPtr, i32 sharedMemoryKey)
{
	static DummyGUIHelper noGfx;

	gSharedMemoryKey = sharedMemoryKey;
	GUIHelperInterface* guiHelper = (GUIHelperInterface*)guiHelperPtr;
	if (!guiHelper)
	{
		guiHelper = &noGfx;
	}
	bool useInprocessMemory = false;
	bool skipGraphicsUpdate = true;
	InProcessPhysicsClientExistingExampleBrowser* cl = new InProcessPhysicsClientExistingExampleBrowser(guiHelper, useInprocessMemory, skipGraphicsUpdate, false);

	cl->setSharedMemoryKey(sharedMemoryKey + 1);
	cl->connect();
	//backward compatiblity
	gSharedMemoryKey = SHARED_MEMORY_KEY;
	return (b3PhysicsClientHandle)cl;
}

DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerFromExistingExampleBrowserAndConnect4(uk guiHelperPtr, i32 sharedMemoryKey)
{
	gSharedMemoryKey = sharedMemoryKey;
	GUIHelperInterface* guiHelper = (GUIHelperInterface*)guiHelperPtr;
	bool ownsGuiHelper = false;
	if (!guiHelper)
	{
		guiHelper = new RemoteGUIHelper();
		ownsGuiHelper = true;
	}
	bool useInprocessMemory = false;
	bool skipGraphicsUpdate = false;
	InProcessPhysicsClientExistingExampleBrowser* cl = new InProcessPhysicsClientExistingExampleBrowser(guiHelper, useInprocessMemory, skipGraphicsUpdate, ownsGuiHelper);

	cl->setSharedMemoryKey(sharedMemoryKey + 1);
	cl->connect();
	//backward compatiblity
	gSharedMemoryKey = SHARED_MEMORY_KEY;
	return (b3PhysicsClientHandle)cl;
}

//#ifdef DRX3D_ENABLE_CLSOCKET
#include <drx3D/SharedMemory/RemoteGUIHelperTCP.h>

DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerFromExistingExampleBrowserAndConnectTCP(tukk hostName, i32 port)
{
	bool ownsGuiHelper = true;
	GUIHelperInterface* guiHelper = new RemoteGUIHelperTCP(hostName, port);
	
	bool useInprocessMemory = true;
	bool skipGraphicsUpdate = false;
	InProcessPhysicsClientExistingExampleBrowser* cl = new InProcessPhysicsClientExistingExampleBrowser(guiHelper, useInprocessMemory, skipGraphicsUpdate, ownsGuiHelper);

	//cl->setSharedMemoryKey(sharedMemoryKey + 1);
	cl->connect();
	//backward compatiblity
	gSharedMemoryKey = SHARED_MEMORY_KEY;
	return (b3PhysicsClientHandle)cl;
}



//backward compatiblity
DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerFromExistingExampleBrowserAndConnect2(uk guiHelperPtr)
{
	return b3CreateInProcessPhysicsServerFromExistingExampleBrowserAndConnect3(guiHelperPtr, SHARED_MEMORY_KEY);
}

#include <drx3D/SharedMemory/SharedMemoryCommands.h>
#include <drx3D/SharedMemory/PhysicsClientSharedMemory.h>
#include <drx3D/SharedMemory/GraphicsSharedMemoryBlock.h>
#include <drx3D/SharedMemory/PosixSharedMemory.h>
#include <drx3D/SharedMemory/Win32SharedMemory.h>

class InProcessGraphicsServerSharedMemory : public PhysicsClientSharedMemory
{
	InProcessExampleBrowserInternalData* m_data2;
	tuk* m_newargv;
	SharedMemoryCommand m_command;
	
	GraphicsSharedMemoryBlock* m_testBlock1;
	SharedMemoryInterface* m_sharedMemory;

public:
	InProcessGraphicsServerSharedMemory(i32 port)
	{
		i32 newargc = 3;
		m_newargv = (tuk*)malloc(sizeof(uk ) * newargc);
		tuk t0 = (tuk)"--unused";
		m_newargv[0] = t0;

		tuk t1 = (tuk)"--start_demo_name=Graphics Server";
		char portArg[1024];
		sprintf(portArg, "--port=%d", port);
		
		m_newargv[1] = t1;
		m_newargv[2] = portArg;
		bool useInProcessMemory = false;
		m_data2 = CreateInProcessExampleBrowser(newargc, m_newargv, useInProcessMemory);
		SharedMemoryInterface* shMem = GetSharedMemoryInterface(m_data2);
		
		setSharedMemoryInterface(shMem);
		///////////////////

#ifdef _WIN32
		m_sharedMemory = new Win32SharedMemoryServer();
#else
		m_sharedMemory = new PosixSharedMemory();
#endif

			/// server always has to create and initialize shared memory
		bool allowCreation = false;
		m_testBlock1 = (GraphicsSharedMemoryBlock*)m_sharedMemory->allocateSharedMemory(
			GRAPHICS_SHARED_MEMORY_KEY, GRAPHICS_SHARED_MEMORY_SIZE, allowCreation);

	}

	virtual ~InProcessGraphicsServerSharedMemory()
	{
		m_sharedMemory->releaseSharedMemory(GRAPHICS_SHARED_MEMORY_KEY, GRAPHICS_SHARED_MEMORY_SIZE);
		delete m_sharedMemory;
		
		setSharedMemoryInterface(0);
		ShutDownExampleBrowser(m_data2);
		free(m_newargv);
	}
	virtual bool canSubmitCommand() const
	{
		if (m_testBlock1)
		{
			if (m_testBlock1->m_magicId != GRAPHICS_SHARED_MEMORY_MAGIC_NUMBER)
			{
				return false;
			}
		}
		return true;
	}

	virtual struct SharedMemoryCommand* getAvailableSharedMemoryCommand()
	{
		return &m_command;
	}

	virtual bool submitClientCommand(const struct SharedMemoryCommand& command)
	{
		switch (command.m_type)
		{
		default:
		{
		}
		}
		return true;
	}


};


class InProcessGraphicsServerSharedMemoryMainThread : public PhysicsClientSharedMemory
{
	
	InProcessExampleBrowserMainThreadInternalData* m_data2;
	tuk* m_newargv;
	SharedMemoryCommand m_command;

	GraphicsSharedMemoryBlock* m_testBlock1;
	SharedMemoryInterface* m_sharedMemory;
	b3Clock m_clock;

public:
	InProcessGraphicsServerSharedMemoryMainThread(i32 port)
	{
		i32 newargc = 3;
		m_newargv = (tuk*)malloc(sizeof(uk ) * newargc);
		tuk t0 = (tuk)"--unused";
		m_newargv[0] = t0;

		
		tuk t1 = (tuk)"--start_demo_name=Graphics Server";
		m_newargv[1] = t1;
		char portArg[1024];
		sprintf(portArg, "--port=%d", port);
		m_newargv[2] = portArg;

		bool useInProcessMemory = false;
		m_data2 = CreateInProcessExampleBrowserMainThread(newargc, m_newargv, useInProcessMemory);
		SharedMemoryInterface* shMem = GetSharedMemoryInterfaceMainThread(m_data2);

		setSharedMemoryInterface(shMem);
		///////////////////

#ifdef _WIN32
		m_sharedMemory = new Win32SharedMemoryServer();
#else
		m_sharedMemory = new PosixSharedMemory();
#endif

		/// server always has to create and initialize shared memory
		bool allowCreation = false;
		m_testBlock1 = (GraphicsSharedMemoryBlock*)m_sharedMemory->allocateSharedMemory(
			GRAPHICS_SHARED_MEMORY_KEY, GRAPHICS_SHARED_MEMORY_SIZE, allowCreation);
		m_clock.reset();
	}

	virtual ~InProcessGraphicsServerSharedMemoryMainThread()
	{
		m_sharedMemory->releaseSharedMemory(GRAPHICS_SHARED_MEMORY_KEY, GRAPHICS_SHARED_MEMORY_SIZE);
		delete m_sharedMemory;

		setSharedMemoryInterface(0);
		ShutDownExampleBrowserMainThread(m_data2);
		free(m_newargv);
	}
	virtual bool canSubmitCommand() const
	{
		UpdateInProcessExampleBrowserMainThread(m_data2);
		if (m_testBlock1)
		{
			if (m_testBlock1->m_magicId != GRAPHICS_SHARED_MEMORY_MAGIC_NUMBER)
			{
				return false;
			}
		}
		return true;
	}

	virtual struct SharedMemoryCommand* getAvailableSharedMemoryCommand()
	{
		return &m_command;
	}

	virtual bool submitClientCommand(const struct SharedMemoryCommand& command)
	{
		switch (command.m_type)
		{
		default:
		{
		}
		}
		return true;
	}

	// return non-null if there is a status, nullptr otherwise
	virtual const struct SharedMemoryStatus* processServerStatus()
	{
		{
			if (IsExampleBrowserMainThreadTerminated(m_data2))
			{
				PhysicsClientSharedMemory::disconnectSharedMemory();
			}
		}
		{
			u64 ms = m_clock.getTimeMilliseconds();
			if (ms > 2)
			{
				D3_PROFILE("m_clock.reset()");

				UpdateInProcessExampleBrowserMainThread(m_data2);
				m_clock.reset();
			}
		}
		{
			b3Clock::usleep(0);
		}
		const SharedMemoryStatus* stat = 0;

		{
			stat = PhysicsClientSharedMemory::processServerStatus();
		}

		return stat;
	}

};



DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessGraphicsServerAndConnectSharedMemory(i32 port)
{
	InProcessGraphicsServerSharedMemory* cl = new InProcessGraphicsServerSharedMemory(port);
	cl->setSharedMemoryKey(SHARED_MEMORY_KEY + 1);
	cl->connect();
	return (b3PhysicsClientHandle)cl;
}

DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessGraphicsServerAndConnectMainThreadSharedMemory(i32 port)
{
	InProcessGraphicsServerSharedMemoryMainThread* cl = new InProcessGraphicsServerSharedMemoryMainThread(port);
	cl->setSharedMemoryKey(SHARED_MEMORY_KEY + 1);
	cl->connect();
	return (b3PhysicsClientHandle)cl;
}

//#endif
