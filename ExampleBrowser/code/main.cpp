#include "../OpenGLExampleBrowser.h"

#include <drx3D/Common/b3CommandLineArgs.h>
#include <drx3D/Common/b3Clock.h>

#include "../ExampleEntries.h"
#include <drx3D/Common/b3Logging.h>

#include <drx3D/Importers/Obj/ImportObjExample.h>
#include "../Importers/ImportBspExample.h"
#include <drx3D/Importers/Collada/ImportColladaSetup.h>
#include <drx3D/Importers/STL/ImportSTLSetup.h>
#include <drx3D/Importers/URDF/ImportURDFSetup.h>
#include <drx3D/Importers/SDF/ImportSDFSetup.h>
#include <drx3D/Importers/STL/ImportSTLSetup.h>
#include <drx3D/Importers/Bullet/SerializeSetup.h>

#include <drx3D/Maths/Linear/AlignedAllocator.h>

static double gMinUpdateTimeMicroSecs = 1000.;

static bool interrupted = false;
static OpenGLExampleBrowser* sExampleBrowser = 0;

#ifndef _WIN32
#include <signal.h>
#include <err.h>
#include <unistd.h>
static void cleanup(i32 signo)
{
	if (!interrupted)
	{  // this is the second time, we're hanging somewhere
		drx3DPrintf("Прерывание и удаление объекта SharedMemoryCommon");
		delete sExampleBrowser;
		sleep(1);
		sExampleBrowser = 0;
		errx(EXIT_FAILURE, "пример прерван по сигналу %d", signo);
	}
	else
	{
		drx3DPrintf("действий нет");
		exit(EXIT_FAILURE);
	}
	interrupted = true;
	warnx("уловлен сигнал %d", signo);
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
			err(EXIT_FAILURE, "сигнал %d", signos[ii]);
		}
	}
#endif

	{
		b3CommandLineArgs args(argc, argv);
		b3Clock clock;
		args.GetCmdLineArgument("minUpdateTimeMicroSecs", gMinUpdateTimeMicroSecs);

		ExampleEntriesAll examples;
		examples.initExampleEntries();

		OpenGLExampleBrowser* exampleBrowser = new OpenGLExampleBrowser(&examples);
		sExampleBrowser = exampleBrowser;  //for <CTRL-C> etc, cleanup shared memory
		bool init = exampleBrowser->init(argc, argv);
		exampleBrowser->registerFileImporter(".urdf", ImportURDFCreateFunc);
		exampleBrowser->registerFileImporter(".sdf", ImportSDFCreateFunc);
		exampleBrowser->registerFileImporter(".obj", ImportObjCreateFunc);
		exampleBrowser->registerFileImporter(".stl", ImportSTLCreateFunc);
		exampleBrowser->registerFileImporter(".bullet", SerializeBulletCreateFunc);

		clock.reset();
		if (init)
		{
			do
			{
				float deltaTimeInSeconds = clock.getTimeMicroseconds() / 1000000.f;
				if (deltaTimeInSeconds > 0.1)
				{
					deltaTimeInSeconds = 0.1;
				}
				if (deltaTimeInSeconds < (gMinUpdateTimeMicroSecs / 1e6))
				{
					b3Clock::usleep(gMinUpdateTimeMicroSecs / 10.);
				}
				else
				{
					clock.reset();
					exampleBrowser->update(deltaTimeInSeconds);
				}
			} while (!exampleBrowser->requestedExit() && !interrupted);
		}
		delete exampleBrowser;
	}

#ifdef DRX3D_DEBUG_MEMORY_ALLOCATIONS
	i32 numBytesLeaked = DumpMemoryLeaks();
	Assert(numBytesLeaked == 0);
#endif  //DRX3D_DEBUG_MEMORY_ALLOCATIONS

	return 0;
}
