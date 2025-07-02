#include "PhysicsServerExampleBullet2.h"

#include <drx3D/Common/b3CommandLineArgs.h"

#include <drx3D/Common/Interfaces/CommonExampleInterface.h"
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h"
#include "SharedMemoryCommon.h"

#include <stdlib.h>

i32 gSharedMemoryKey = -1;

static SharedMemoryCommon* example = NULL;
static bool interrupted = false;

#ifndef _WIN32
#include <signal.h>
#include <err.h>
#include <unistd.h>
static void cleanup(i32 signo)
{
	if (interrupted)
	{  // this is the second time, we're hanging somewhere
		//  if (example) {
		//      example->abort();
		//  }
		drx3DPrintf("Aborting and deleting SharedMemoryCommon object");
		sleep(1);
		delete example;
		errx(EXIT_FAILURE, "aborted example on signal %d", signo);
	}
	interrupted = true;
	warnx("caught signal %d", signo);
}
#endif  //_WIN32

i32 main(i32 argc, tuk argv[])
{
#ifndef _WIN32
	struct sigaction action;
	memset(&action, 0x0, sizeof(action));
	action.sa_handler = cleanup;
	static i32k signos[] = {SIGHUP, SIGINT, SIGQUIT, SIGABRT, SIGSEGV, SIGPIPE, SIGTERM};
	for (i32 ii(0); ii < sizeof(signos) / sizeof(*signos); ++ii)
	{
		if (0 != sigaction(signos[ii], &action, NULL))
		{
			err(EXIT_FAILURE, "signal %d", signos[ii]);
		}
	}
#endif

	b3CommandLineArgs args(argc, argv);

	DummyGUIHelper noGfx;

	CommonExampleOptions options(&noGfx);

	args.GetCmdLineArgument("shared_memory_key", gSharedMemoryKey);
	args.GetCmdLineArgument("sharedMemoryKey", gSharedMemoryKey);

	// options.m_option |= PHYSICS_SERVER_ENABLE_COMMAND_LOGGING;
	// options.m_option |= PHYSICS_SERVER_REPLAY_FROM_COMMAND_LOG;

	example = (SharedMemoryCommon*)PhysicsServerCreateFuncBullet2(options);

	example->initPhysics();

	while (example->isConnected() && !(example->wantsTermination() || interrupted))
	{
		example->stepSimulation(1.f / 60.f);
	}

	example->exitPhysics();

	delete example;

	return 0;
}
