
#include <gtest/gtest.h>

#include "Bullet3Common/b3Logging.h"

void myerrorprintf(tukk msg)
{
	printf("%s", msg);
}

static bool sVerboseWarning = true;

void mywarningprintf(tukk msg)
{
	if (sVerboseWarning)
	{
		//OutputDebugStringA(msg);
		printf("%s", msg);
	}
}

static bool sVerbosePrintf = true;  //false;

void myprintf(tukk msg)
{
	if (sVerbosePrintf)
	{
		//OutputDebugStringA(msg);
		printf("%s", msg);
	}
}

i32 gArgc = 0;
tuk* gArgv = 0;

i32 main(i32 argc, tuk* argv)
{
#if _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//uk testWhetherMemoryLeakDetectionWorks = malloc(1);
#endif
	::testing::InitGoogleTest(&argc, argv);

	gArgc = argc;
	gArgv = argv;

	b3SetCustomPrintfFunc(myprintf);
	b3SetCustomWarningMessageFunc(mywarningprintf);
	b3SetCustomErrorMessageFunc(myerrorprintf);

	return RUN_ALL_TESTS();
}
