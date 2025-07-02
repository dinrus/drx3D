#include "../InProcessExampleBrowser.h"

//#define EXAMPLE_CONSOLE_ONLY
#ifdef EXAMPLE_CONSOLE_ONLY
#include "../EmptyBrowser.h"
typedef EmptyBrowser DefaultBrowser;
#else
#include "../OpenGLExampleBrowser.h"
typedef OpenGLExampleBrowser DefaultBrowser;
#endif  //EXAMPLE_CONSOLE_ONLY

#include <drx3D/Common/b3CommandLineArgs.h>
#include <drx3D/Common/b3Clock.h>

#include "../ExampleEntries.h"
#include <drx3D/Common/b3Scalar.h>
#include <drx3D/SharedMemory/InProcessMemory.h>

void ExampleBrowserThreadFunc(uk userPtr, uk lsMemory);
uk ExampleBrowserMemoryFunc();
void ExampleBrowserMemoryReleaseFunc(uk ptr);

#include <stdio.h>

#include <drx3D/Common/b3Logging.h>
#include "../ExampleEntries.h"
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include "../EmptyExample.h"

#include "../SharedMemory/PhysicsServerExample.h"
#include "../SharedMemory/PhysicsServerExampleBullet2.h"
#include "../SharedMemory/GraphicsServerExample.h"


#include "../SharedMemory/PhysicsClientExample.h"

#ifndef _WIN32
#include <drx3D/MT/PosixThreadSupport.h>

static ThreadSupportInterface* createExampleBrowserThreadSupport(i32 numThreads)
{
	PosixThreadSupport::ThreadConstructionInfo constructionInfo("testThreads",
																  ExampleBrowserThreadFunc,
																  ExampleBrowserMemoryFunc,
																  ExampleBrowserMemoryReleaseFunc,
																  numThreads);
	ThreadSupportInterface* threadSupport = new PosixThreadSupport(constructionInfo);

	return threadSupport;
}

#elif defined(_WIN32)
#include "../MultiThreading/b3Win32ThreadSupport.h"

ThreadSupportInterface* createExampleBrowserThreadSupport(i32 numThreads)
{
	b3Win32ThreadSupport::Win32ThreadConstructionInfo threadConstructionInfo("testThreads", ExampleBrowserThreadFunc, ExampleBrowserMemoryFunc, ExampleBrowserMemoryReleaseFunc, numThreads);
	b3Win32ThreadSupport* threadSupport = new b3Win32ThreadSupport(threadConstructionInfo);
	return threadSupport;
}
#endif

class ExampleEntriesPhysicsServer : public ExampleEntries
{
	struct ExampleEntriesInternalData2* m_data;

public:
	ExampleEntriesPhysicsServer();
	virtual ~ExampleEntriesPhysicsServer();

	static void registerExampleEntry(i32 menuLevel, tukk name, tukk description, CommonExampleInterface::CreateFunc* createFunc, i32 option = 0);

	virtual void initExampleEntries();

	virtual void initOpenCLExampleEntries();

	virtual i32 getNumRegisteredExamples();

	virtual CommonExampleInterface::CreateFunc* getExampleCreateFunc(i32 index);

	virtual tukk getExampleName(i32 index);

	virtual tukk getExampleDescription(i32 index);

	virtual i32 getExampleOption(i32 index);
};

struct ExampleEntryPhysicsServer
{
	i32 m_menuLevel;
	tukk m_name;
	tukk m_description;
	CommonExampleInterface::CreateFunc* m_createFunc;
	i32 m_option;

	ExampleEntryPhysicsServer(i32 menuLevel, tukk name)
		: m_menuLevel(menuLevel), m_name(name), m_description(0), m_createFunc(0), m_option(0)
	{
	}

	ExampleEntryPhysicsServer(i32 menuLevel, tukk name, tukk description, CommonExampleInterface::CreateFunc* createFunc, i32 option = 0)
		: m_menuLevel(menuLevel), m_name(name), m_description(description), m_createFunc(createFunc), m_option(option)
	{
	}
};

struct ExampleEntriesInternalData2
{
	AlignedObjectArray<ExampleEntryPhysicsServer> m_allExamples;
};

static ExampleEntryPhysicsServer gDefaultExamplesPhysicsServer[] =
	{

		ExampleEntryPhysicsServer(0, "Контроль Роботизации"),

		ExampleEntryPhysicsServer(1, "Сервер Физики ", "Создаёт сервер физики, коммунцирующий с клиентом физкии через разделяемую память.",
								  PhysicsServerCreateFuncBullet2),
		ExampleEntryPhysicsServer(1, "Сервер Физики  (RTC)", "Создаёт сервер физики, коммунцирующий с клиентом физкии через разделяемую память. При каждом обновлении этот Сервер Физики ьудет продолжать вызов 'stepSimulation' на основе часов реального времени (RTC).",
								  PhysicsServerCreateFuncBullet2, PHYSICS_SERVER_USE_RTC_CLOCK),

		ExampleEntryPhysicsServer(1, "Сервер Физики (Logging)", "Создаёт сервер физики, коммунцирующий с клиентом физкии через разделяемую память. Логирует все команды в файл.",
								  PhysicsServerCreateFuncBullet2, PHYSICS_SERVER_ENABLE_COMMAND_LOGGING),
		ExampleEntryPhysicsServer(1, "Сервер Физики (Replay Log)", "Создаёт сервер физики, который воспроизводит повторно лог команд с диска.",
								  PhysicsServerCreateFuncBullet2, PHYSICS_SERVER_REPLAY_FROM_COMMAND_LOG),
		ExampleEntryPhysicsServer(1, "Сервер Графики", "Создаёт сервер графики.", GraphicsServerCreateFuncBullet),

};

ExampleEntriesPhysicsServer::ExampleEntriesPhysicsServer()
{
	m_data = new ExampleEntriesInternalData2;
}

ExampleEntriesPhysicsServer::~ExampleEntriesPhysicsServer()
{
	delete m_data;
}

void ExampleEntriesPhysicsServer::initOpenCLExampleEntries()
{
}

void ExampleEntriesPhysicsServer::initExampleEntries()
{
	m_data->m_allExamples.clear();

	i32 numDefaultEntries = sizeof(gDefaultExamplesPhysicsServer) / sizeof(ExampleEntryPhysicsServer);
	for (i32 i = 0; i < numDefaultEntries; i++)
	{
		m_data->m_allExamples.push_back(gDefaultExamplesPhysicsServer[i]);
	}
}

void ExampleEntriesPhysicsServer::registerExampleEntry(i32 menuLevel, tukk name, tukk description, CommonExampleInterface::CreateFunc* createFunc, i32 option)
{
}

i32 ExampleEntriesPhysicsServer::getNumRegisteredExamples()
{
	return m_data->m_allExamples.size();
}

CommonExampleInterface::CreateFunc* ExampleEntriesPhysicsServer::getExampleCreateFunc(i32 index)
{
	return m_data->m_allExamples[index].m_createFunc;
}

i32 ExampleEntriesPhysicsServer::getExampleOption(i32 index)
{
	return m_data->m_allExamples[index].m_option;
}

tukk ExampleEntriesPhysicsServer::getExampleName(i32 index)
{
	return m_data->m_allExamples[index].m_name;
}

tukk ExampleEntriesPhysicsServer::getExampleDescription(i32 index)
{
	return m_data->m_allExamples[index].m_description;
}

struct ExampleBrowserArgs
{
	ExampleBrowserArgs()
		: m_fakeWork(1), m_argc(0)
	{
	}
	b3CriticalSection* m_cs;
	float m_fakeWork;
	i32 m_argc;
	tuk* m_argv;
};

struct ExampleBrowserThreadLocalStorage
{
	SharedMemoryInterface* m_sharedMem;
	i32 threadId;
};

enum TestExampleBrowserCommunicationEnums
{
	eRequestTerminateExampleBrowser = 13,
	eExampleBrowserIsUnInitialized,
	eExampleBrowserIsInitialized,
	eExampleBrowserInitializationFailed,
	eExampleBrowserHasTerminated
};

static double gMinUpdateTimeMicroSecs = 4000.;

void ExampleBrowserThreadFunc(uk userPtr, uk lsMemory)
{
	printf("ExampleBrowserThreadFunc пущена\n");

	ExampleBrowserThreadLocalStorage* localStorage = (ExampleBrowserThreadLocalStorage*)lsMemory;

	ExampleBrowserArgs* args = (ExampleBrowserArgs*)userPtr;
	//i32 workLeft = true;
	b3CommandLineArgs args2(args->m_argc, args->m_argv);
	i32 minUpdateMs = 4000;
	if (args2.GetCmdLineArgument("minGraphicsUpdateTimeMs", minUpdateMs))
	{
		gMinUpdateTimeMicroSecs = minUpdateMs;
	}
	b3Clock clock;

	ExampleEntriesPhysicsServer examples;
	examples.initExampleEntries();

	DefaultBrowser* exampleBrowser = new DefaultBrowser(&examples);
	exampleBrowser->setSharedMemoryInterface(localStorage->m_sharedMem);

	bool init = exampleBrowser->init(args->m_argc, args->m_argv);
	clock.reset();
	if (init)
	{
		args->m_cs->lock();
		args->m_cs->setSharedParam(0, eExampleBrowserIsInitialized);
		args->m_cs->unlock();

		do
		{
			clock.usleep(0);

			//D3_PROFILE("ExampleBrowserThreadFunc");
			float deltaTimeInSeconds = clock.getTimeMicroseconds() / 1000000.f;
			{
				if (deltaTimeInSeconds > 0.1)
				{
					deltaTimeInSeconds = 0.1;
				}
				if (deltaTimeInSeconds < (gMinUpdateTimeMicroSecs / 1e6))
				{
					//D3_PROFILE("clock.usleep");
					exampleBrowser->updateGraphics();
				}
				else
				{
					//D3_PROFILE("exampleBrowser->update");
					clock.reset();
					exampleBrowser->updateGraphics();
					exampleBrowser->update(deltaTimeInSeconds);
				}
			}

		} while (!exampleBrowser->requestedExit() && (args->m_cs->getSharedParam(0) != eRequestTerminateExampleBrowser));
	}
	else
	{
		args->m_cs->lock();
		args->m_cs->setSharedParam(0, eExampleBrowserInitializationFailed);
		args->m_cs->unlock();
	}

	delete exampleBrowser;
	args->m_cs->lock();
	args->m_cs->setSharedParam(0, eExampleBrowserHasTerminated);
	args->m_cs->unlock();
	printf("finished\n");
	//do nothing
}

uk ExampleBrowserMemoryFunc()
{
	//don't create local store memory, just return 0
	return new ExampleBrowserThreadLocalStorage;
}

void ExampleBrowserMemoryReleaseFunc(uk ptr)
{
	ExampleBrowserThreadLocalStorage* p = (ExampleBrowserThreadLocalStorage*)ptr;
	delete p;
}

struct InProcessExampleBrowserInternalData
{
	ExampleBrowserArgs m_args;
	ThreadSupportInterface* m_threadSupport;
	SharedMemoryInterface* m_sharedMem;
};

InProcessExampleBrowserInternalData* CreateInProcessExampleBrowser(i32 argc, tuk* argv2, bool useInProcessMemory)
{
	InProcessExampleBrowserInternalData* data = new InProcessExampleBrowserInternalData;

	data->m_sharedMem = useInProcessMemory ? new InProcessMemory : 0;

	i32 numThreads = 1;
	i32 i;

	data->m_threadSupport = createExampleBrowserThreadSupport(numThreads);

	printf("argc=%d\n", argc);
	for (i = 0; i < argc; i++)
	{
		printf("argv[%d] = %s\n", i, argv2[i]);
	}

	for (i = 0; i < data->m_threadSupport->getNumTasks(); i++)
	{
		ExampleBrowserThreadLocalStorage* storage = (ExampleBrowserThreadLocalStorage*)data->m_threadSupport->getThreadLocalMemory(i);
		drx3DAssert(storage);
		storage->threadId = i;
		storage->m_sharedMem = data->m_sharedMem;
	}

	data->m_args.m_cs = data->m_threadSupport->createCriticalSection();
	data->m_args.m_cs->setSharedParam(0, eExampleBrowserIsUnInitialized);
	data->m_args.m_argc = argc;
	data->m_args.m_argv = argv2;

	for (i = 0; i < numThreads; i++)
	{
		data->m_threadSupport->runTask(D3_THREAD_SCHEDULE_TASK, (uk )&data->m_args, i);
	}

	while (data->m_args.m_cs->getSharedParam(0) == eExampleBrowserIsUnInitialized)
	{
		b3Clock::usleep(1000);
	}

	return data;
}

bool IsExampleBrowserTerminated(InProcessExampleBrowserInternalData* data)
{
	return (data->m_args.m_cs->getSharedParam(0) == eExampleBrowserHasTerminated);
}

SharedMemoryInterface* GetSharedMemoryInterface(InProcessExampleBrowserInternalData* data)
{
	return data->m_sharedMem;
}

void ShutDownExampleBrowser(InProcessExampleBrowserInternalData* data)
{
	i32 numActiveThreads = 1;

	data->m_args.m_cs->lock();
	data->m_args.m_cs->setSharedParam(0, eRequestTerminateExampleBrowser);
	data->m_args.m_cs->unlock();

	while (numActiveThreads)
	{
		i32 arg0, arg1;
		if (data->m_threadSupport->isTaskCompleted(&arg0, &arg1, 0))
		{
			numActiveThreads--;
			printf("numActiveThreads = %d\n", numActiveThreads);
		}
		else
		{
			//                              printf("polling..");
			b3Clock::usleep(0);
		}
	};

	printf("ShutDownExampleBrowser останавливает потоки\n");
	data->m_threadSupport->deleteCriticalSection(data->m_args.m_cs);

	delete data->m_threadSupport;
	delete data->m_sharedMem;
	delete data;
}

struct InProcessExampleBrowserMainThreadInternalData
{
	ExampleEntriesPhysicsServer m_examples;
	DefaultBrowser* m_exampleBrowser;
	SharedMemoryInterface* m_sharedMem;
	b3Clock m_clock;
};

InProcessExampleBrowserMainThreadInternalData* CreateInProcessExampleBrowserMainThread(i32 argc, tuk* argv, bool useInProcessMemory)
{
	InProcessExampleBrowserMainThreadInternalData* data = new InProcessExampleBrowserMainThreadInternalData;
	data->m_examples.initExampleEntries();
	data->m_exampleBrowser = new DefaultBrowser(&data->m_examples);
	data->m_sharedMem = useInProcessMemory ? new InProcessMemory : 0;
	data->m_exampleBrowser->setSharedMemoryInterface(data->m_sharedMem);
	bool init;
	init = data->m_exampleBrowser->init(argc, argv);
	data->m_clock.reset();
	return data;
}

bool IsExampleBrowserMainThreadTerminated(InProcessExampleBrowserMainThreadInternalData* data)
{
	return data->m_exampleBrowser->requestedExit();
}

void UpdateInProcessExampleBrowserMainThread(InProcessExampleBrowserMainThreadInternalData* data)
{
	float deltaTimeInSeconds = data->m_clock.getTimeMicroseconds() / 1000000.f;
	data->m_clock.reset();
	data->m_exampleBrowser->updateGraphics();
	data->m_exampleBrowser->update(deltaTimeInSeconds);
}
void ShutDownExampleBrowserMainThread(InProcessExampleBrowserMainThreadInternalData* data)
{
	data->m_exampleBrowser->setSharedMemoryInterface(0);
	delete data->m_exampleBrowser;
	delete data;
}

class SharedMemoryInterface* GetSharedMemoryInterfaceMainThread(InProcessExampleBrowserMainThreadInternalData* data)
{
	return data->m_sharedMem;
}
