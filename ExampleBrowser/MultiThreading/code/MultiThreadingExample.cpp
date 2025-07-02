
#include "../MultiThreadingExample.h"
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include "../../RenderingExamples/TimeSeriesCanvas.h"
#include <X/stb/stb_image.h>
#include <drx3D/Common/b3Quat.h>
#include <drx3D/Common/b3Matrix3x3.h>
#include <drx3D/Common/b3Clock.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#define stdvector AlignedObjectArray

#define MAGIC_RESET_NUMBER 64738

void SampleThreadFunc(uk userPtr, uk lsMemory);
uk SamplelsMemoryFunc();
void SamplelsMemoryReleaseFunc(uk ptr);

#include <stdio.h>
//#include "BulletMultiThreaded/PlatformDefinitions.h"

#ifndef _WIN32
#include <drx3D/MT/PosixThreadSupport.h>

ThreadSupportInterface* createThreadSupport(i32 numThreads)
{
	PosixThreadSupport::ThreadConstructionInfo constructionInfo("testThreads",
	SampleThreadFunc,
	SamplelsMemoryFunc,
	SamplelsMemoryReleaseFunc,
	  numThreads);
	ThreadSupportInterface* threadSupport = new PosixThreadSupport(constructionInfo);

	return threadSupport;
}

#elif defined(_WIN32)
#include <drx3D/MT/b3Win32ThreadSupport.h>

ThreadSupportInterface* createThreadSupport(i32 numThreads)
{
	b3Win32ThreadSupport::Win32ThreadConstructionInfo threadConstructionInfo("testThreads", SampleThreadFunc, SamplelsMemoryFunc, SamplelsMemoryReleaseFunc, numThreads);
	b3Win32ThreadSupport* threadSupport = new b3Win32ThreadSupport(threadConstructionInfo);
	return threadSupport;
}
#endif

struct SampleJobInterface
{
	virtual void executeJob(i32 threadIndex) = 0;
};

struct SampleJob1 : public SampleJobInterface
{
	float m_fakeWork;
	i32 m_jobId;

	SampleJob1(i32 jobId)
		: m_fakeWork(0),
		  m_jobId(jobId)
	{
	}
	virtual ~SampleJob1() {}

	virtual void executeJob(i32 threadIndex)
	{
		printf("start SampleJob1 %d using threadIndex %d\n", m_jobId, threadIndex);
		//do some fake work
		for (i32 i = 0; i < 1000000; i++)
			m_fakeWork = b3Scalar(1.21) * m_fakeWork;
		printf("finished SampleJob1 %d using threadIndex %d\n", m_jobId, threadIndex);
	}
};

struct SampleArgs
{
	SampleArgs()
	{
	}
	b3CriticalSection* m_cs;

	AlignedObjectArray<SampleJobInterface*> m_jobQueue;

	void submitJob(SampleJobInterface* job)
	{
		m_cs->lock();
		m_jobQueue.push_back(job);
		m_cs->unlock();
	}
	SampleJobInterface* consumeJob()
	{
		SampleJobInterface* job = 0;
		m_cs->lock();
		i32 sz = m_jobQueue.size();
		if (sz)
		{
			job = m_jobQueue[sz - 1];
			m_jobQueue.pop_back();
		}
		m_cs->unlock();
		return job;
	}
};
SampleArgs args;
struct SampleThreadLocalStorage
{
	i32 threadId;
};

void SampleThreadFunc(uk userPtr, uk lsMemory)
{
	printf("SampleThreadFunc thread started\n");

	SampleThreadLocalStorage* localStorage = (SampleThreadLocalStorage*)lsMemory;

	SampleArgs* args = (SampleArgs*)userPtr;

	bool requestExit = false;
	while (!requestExit)
	{
		SampleJobInterface* job = args->consumeJob();
		if (job)
		{
			job->executeJob(localStorage->threadId);
		}

		b3Clock::usleep(250);

		args->m_cs->lock();
		i32 exitMagicNumber = args->m_cs->getSharedParam(1);
		requestExit = (exitMagicNumber == MAGIC_RESET_NUMBER);
		args->m_cs->unlock();
	}

	printf("finished\n");
	//do nothing
}

uk SamplelsMemoryFunc()
{
	//don't create local store memory, just return 0
	return new SampleThreadLocalStorage;
}

void SamplelsMemoryReleaseFunc(uk ptr)
{
	SampleThreadLocalStorage* p = (SampleThreadLocalStorage*)ptr;
	delete p;
}

class MultiThreadingExample : public CommonExampleInterface
{
	CommonGraphicsApp* m_app;
	ThreadSupportInterface* m_threadSupport;
	AlignedObjectArray<SampleJob1*> m_jobs;
	i32 m_numThreads;

public:
	MultiThreadingExample(GUIHelperInterface* guiHelper, i32 tutorialIndex)
		: m_app(guiHelper->getAppInterface()),
		  m_threadSupport(0),
		  m_numThreads(8)
	{
		//i32 numBodies = 1;

		m_app->setUpAxis(1);
	}
	virtual ~MultiThreadingExample()
	{
	}

	virtual void renderScene()
	{
	}
	virtual void initPhysics()
	{
		drx3DPrintf("initPhysics");

		m_threadSupport = createThreadSupport(m_numThreads);

		for (i32 i = 0; i < m_threadSupport->getNumTasks(); i++)
		{
			SampleThreadLocalStorage* storage = (SampleThreadLocalStorage*)m_threadSupport->getThreadLocalMemory(i);
			drx3DAssert(storage);
			storage->threadId = i;
		}

		args.m_cs = m_threadSupport->createCriticalSection();
		args.m_cs->setSharedParam(0, 100);

		for (i32 i = 0; i < 100; i++)
		{
			SampleJob1* job = new SampleJob1(i);
			args.submitJob(job);
		}

		i32 i;
		for (i = 0; i < m_numThreads; i++)
		{
			m_threadSupport->runTask(D3_THREAD_SCHEDULE_TASK, (uk )&args, i);
		}
		drx3DPrintf("Threads started");
	}
	virtual void exitPhysics()
	{
		drx3DPrintf("exitPhysics, stopping threads");
		bool blockingWait = false;
		i32 arg0, arg1;

		args.m_cs->lock();
		//terminate all threads
		args.m_cs->setSharedParam(1, MAGIC_RESET_NUMBER);
		args.m_cs->unlock();

		if (blockingWait)
		{
			for (i32 i = 0; i < m_numThreads; i++)
			{
				m_threadSupport->waitForResponse(&arg0, &arg1);
				printf("finished waiting for response: %d %d\n", arg0, arg1);
			}
		}
		else
		{
			i32 numActiveThreads = m_numThreads;
			while (numActiveThreads)
			{
				if (m_threadSupport->isTaskCompleted(&arg0, &arg1, 0))
				{
					numActiveThreads--;
					printf("numActiveThreads = %d\n", numActiveThreads);
				}
				else
				{
					//				printf("polling..");
				}
			};
		}

		delete m_threadSupport;

		drx3DPrintf("Threads stopped");
		for (i32 i = 0; i < m_jobs.size(); i++)
		{
			delete m_jobs[i];
		}
		m_jobs.clear();
	}

	virtual void stepSimulation(float deltaTime)
	{
	}

	virtual void physicsDebugDraw(i32 debugDrawFlags)
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
		float dist = 10.5;
		float pitch = -32;
		float yaw = 136;
		float targetPos[3] = {0, 0, 0};
		if (m_app->m_renderer && m_app->m_renderer->getActiveCamera())
		{
			m_app->m_renderer->getActiveCamera()->setCameraDistance(dist);
			m_app->m_renderer->getActiveCamera()->setCameraPitch(pitch);
			m_app->m_renderer->getActiveCamera()->setCameraYaw(yaw);
			m_app->m_renderer->getActiveCamera()->setCameraTargetPosition(targetPos[0], targetPos[1], targetPos[2]);
		}
	}
};

class CommonExampleInterface* MultiThreadingExampleCreateFunc(struct CommonExampleOptions& options)
{
	return new MultiThreadingExample(options.m_guiHelper, options.m_option);
}
