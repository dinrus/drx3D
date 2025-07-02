// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
AutoTester.cpp 

-	[09/11/2009] : Created by James Bamford

*************************************************************************/

#ifndef __AUTOTESTER_H__
#define __AUTOTESTER_H__

#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
	#define ENABLE_AUTO_TESTER            1
#else
	#define ENABLE_AUTO_TESTER            0
#endif

#if ENABLE_AUTO_TESTER

#include <drx3D/Game/AutoEnum.h>



#define AutoTesterStateList(f)									\
	f(ATEST_STATE_NONE)														\
	f(ATEST_STATE_TEST_NUM_CLIENTS)								\
	f(ATEST_STATE_TEST_NUM_CLIENTS_LEVEL_ROTATE)	\
	f(ATEST_STATE_TEST_FEATURES)                  \
	f(ATEST_STATE_TEST_PERFORMANCE)               \

#define WriteResultsFlagList(f)             \
	f(kWriteResultsFlag_writeDoneMarkerFile)  \
	f(kWriteResultsFlag_unfinished)           \

class CAutoTester
{
protected:
	AUTOENUM_BUILDENUMWITHTYPE_WITHNUM(AutoTestState, AutoTesterStateList, ATEST_STATE_NUM);
	enum SubState
	{
		k_testPerformance_substate_ingame=0,
		k_testPerformance_substate_waiting_to_submit_telemetry,
	};
	union
	{
		struct
		{
			float m_timeOut;							// time to actually make the test on num clients
			i32 m_numClientsExpected;			// num of clients to actually expect after timeout
			i32 m_maxNumClientsConnected;	// max num of clients ever connected
			float m_debugTimer;						// timer for debug output to try and track down autotest builds not finishing when they should be
		} testNumClients;
		struct  
		{
			i32 m_levelIndex;							// counter to just improve the test output to be ordered by the level loading order
			float m_firstLevelTimeOut;		// how long to run the 1st level - allow more time for clients to slowly join
			float m_levelTimeOut;					// how long to run each level
			float m_nextTimeOut;					// time to actually start next level
			i32 m_numClientsExpected;			// num of clients to actually expect after timeout
			i32 m_maxNumClientsConnected;	// max num of clients ever connected
			float m_debugTimer;						// timer for debug output to try and track down autotest builds not finishing when they should be
		} testNumClientsRotate;
		struct  
		{
			char m_configFile[256];
			bool m_bConfigExecuted;
			float m_delayToStart;
			float m_timeOut;							// time to actually run the test
			SubState m_subState;
		} testPerformance;
		struct 
		{
			char m_setNames[128];
			char m_loadFileName[64];
		} testRunFeatureTests;
	} m_stateData;

	typedef struct STestSuite_s 
	{
		std::vector<XmlNodeRef> m_testCases;
		i32 m_numTestCasesPassed;
		i32 m_numTestCasesFailed;

		STestSuite_s()
		{
			m_numTestCasesPassed=0;
			m_numTestCasesFailed=0;
		}

	} STestSuite;

	typedef std::map<string, STestSuite> TTestSuites;
	TTestSuites m_testSuites;

	DrxFixedStringT<255> m_outputPath;
	char m_includeThisInFileName[48];

	AutoTestState m_state;
	bool m_started;
	bool m_finished;
	bool m_createVerboseFilename;
	bool m_writeResultsCompleteTestCasePass;
	static CAutoTester *s_instance;
	static tukk  s_autoTesterStateNames[];

	bool m_quitWhenDone;
public:
	AUTOENUM_BUILDFLAGS_WITHZERO(WriteResultsFlagList, kWriteResultsFlag_none);

	CAutoTester();
	virtual ~CAutoTester();

	void Restart();
	void Start(tukk stateSetup, tukk outputPath, bool quitWhenDone);
	void AddTestCaseResult(tukk testSuiteName, XmlNodeRef &testCase, bool passed);
	static bool SaveToValidXmlFile(const XmlNodeRef &xmlToSave, tukk fileName);
	void CreateTestCase(XmlNodeRef &testCase, tukk testName, bool passed, tukk failedType=NULL, tukk failedMessage=NULL);
	void AddSimpleTestCase(tukk  groupName, tukk  testName, float duration = -1.f, tukk  failureReason = NULL, tukk  owners = NULL);
	void GetOutputPathFileName(const string& baseName, string& outputFileName);
	void WriteTestManifest(const XmlNodeRef& testManifest);
	void WriteResults(TBitfield flags, const string * additionalTestSuiteName = NULL, const XmlNodeRef * additionalTestCase = NULL);
	void Stop();

	ILINE tukk  GetTestName()
	{
		return m_includeThisInFileName;
	}

	void UpdateTestNumClients();
	void UpdateTestNumClientsLevelRotate();
	void UpdateTestFeatureTests();

	void UpdatePerformanceTestInGame();
	void UpdatePerformanceTestWaitingForTelemetry();
	void UpdatePerformanceTest();
	void Update();

protected:
	AutoTestState FindStateFromStr(tukk stateName);
	bool GetParam(tukk* str, char outName[], i32 inNameMaxSize, char outValue[], i32 inValueMaxSize, char breakAtChar);
};

#endif // ENABLE_AUTO_TESTER

#endif // __AUTOTESTER_H__
