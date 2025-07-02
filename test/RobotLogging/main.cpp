#include "Utils/RobotLoggingUtil.h"

#ifndef ENABLE_GTEST

#include <assert.h>
#define ASSERT_EQ(a, b) assert((a) == (b));
#else
#include <gtest/gtest.h>
#define printf
#endif

void testMinitaurLogging()
{
	tukk fileName = "d:/logTest.txt";
	btAlignedObjectArray<STxt> structNames;
	STxt structTypes;
	btAlignedObjectArray<MinitaurLogRecord> logRecords;
	bool verbose = false;

	i32 status = readMinitaurLogFile(fileName, structNames, structTypes, logRecords, verbose);

	for (i32 i = 0; i < logRecords.size(); i++)
	{
		for (i32 j = 0; j < structTypes.size(); j++)
		{
			switch (structTypes[j])
			{
				case 'I':
				{
					i32 v = logRecords[i].m_values[j].m_intVal;
					printf("record %d, %s = %d\n", i, structNames[j].c_str(), v);
					break;
				}
				case 'f':
				{
					float v = logRecords[i].m_values[j].m_floatVal;
					printf("record %d, %s = %f\n", i, structNames[j].c_str(), v);
					break;
				}
				case 'B':
				{
					i32 v = logRecords[i].m_values[j].m_charVal;
					printf("record %d, %s = %d\n", i, structNames[j].c_str(), v);
					break;
				}
				default:
				{
				}
			}
		}
	}
}

#ifdef ENABLE_GTEST
TEST(RobotLoggingTest, LogMinitaur)
{
	testMinitaurLogging();
}
#endif

i32 main(i32 argc, tuk argv[])
{
	//b3SetCustomPrintfFunc(myprintf);
	//b3SetCustomWarningMessageFunc(myprintf);

#ifdef ENABLE_GTEST

#if _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//uk testWhetherMemoryLeakDetectionWorks = malloc(1);
#endif
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
#else
	testMinitaurLogging();
#endif
}