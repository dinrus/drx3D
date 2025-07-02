#include <gtest/gtest.h>
#include "test.c"
#include "Bullet3Common/b3Logging.h"

void myprintf(tukk bla)
{
}

i32 main(i32 argc, tuk* argv)
{
	b3SetCustomPrintfFunc(myprintf);
	b3SetCustomWarningMessageFunc(myprintf);

#if _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//uk testWhetherMemoryLeakDetectionWorks = malloc(1);
#endif
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
