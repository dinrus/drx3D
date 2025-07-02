// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
AutoTester.cpp 

-	[09/11/2009] : Created by James Bamford

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/AutoTester.h>

#if ENABLE_AUTO_TESTER

#include <drx3D/Game/FeatureTester.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/Game/Utility/DrxWatch.h>
#include <drx3D/Act/ILevelSystem.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/UI/HUD/HUDUtils.h>
#include <drx3D/Game/Utility/DesignerWarning.h>
#include <drx3D/Act/IPlayerProfiles.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/GameRulesModules/IGameRulesSpawningModule.h>
#include <drx3D/Game/ITelemetryCollector.h>

AUTOENUM_BUILDNAMEARRAY(CAutoTester::s_autoTesterStateNames, AutoTesterStateList);

#ifndef _RELEASE
static AUTOENUM_BUILDNAMEARRAY(s_writeResultsFlagNames, WriteResultsFlagList);
#endif

CAutoTester * CAutoTester::s_instance = NULL;

CAutoTester::CAutoTester()
{
	DRX_ASSERT(s_instance == NULL);
	s_instance = this;
	m_started=false;
	m_finished=false;
	m_createVerboseFilename=true;
	m_writeResultsCompleteTestCasePass=false;
	m_state = ATEST_STATE_NONE;
	m_quitWhenDone = false;
	m_includeThisInFileName[0] = '\0';
}

CAutoTester::~CAutoTester()
{
	DRX_ASSERT(s_instance == this);
	s_instance = NULL;
}

// equivalent to a start - but without setting up any new parameters
void CAutoTester::Restart()
{
	m_finished=false;
	m_started=true;
	assert(m_state != ATEST_STATE_NONE);
}

void CAutoTester::Start(tukk stateSetup, tukk outputPath, bool quitWhenDone)
{
	// If the string has any '#' characters in it (the "new" seperator) then we'll split the string up at those characters...
	// If not, we'll use '!' (the "old" seperator)
	char breakAtChar = strchr(stateSetup, '#') ? '#' : '!';

	DrxLogAlways("CAutoTester::Start() stateSetup=\"%s\" outputPath=\"%s\" seperator=\"%c\"", stateSetup, outputPath, breakAtChar);
	assert(!m_started);
	m_started=true;
	
	m_outputPath=outputPath;
	m_quitWhenDone = quitWhenDone;

	tukk str=stateSetup;

	// .dll hell stops simple access to memReplay functionality so can't look for running -memReplay
	// memReplay commands all early exit if they fail anyways
	gEnv->pConsole->ExecuteString("memReplayDumpSymbols");

#define MAX_TOKEN_LEN 128

	char tokenName[MAX_TOKEN_LEN];
	char valueName[MAX_TOKEN_LEN];
	bool success;

	memset(&m_stateData, 0, sizeof(m_stateData));

	success = GetParam(&str, tokenName, MAX_TOKEN_LEN, valueName, MAX_TOKEN_LEN, breakAtChar);

	assert (m_state == ATEST_STATE_NONE);

	if (success && stricmp(tokenName, "state") == 0)
	{
		m_state = FindStateFromStr(valueName);
		drx_strcpy(m_includeThisInFileName, valueName);

		DrxLogAlways("CAutoTester::Start initializing state (name='%s', id=%d, %s)", valueName, m_state, (m_state == ATEST_STATE_NONE) ? "invalid" : "valid");
		DesignerWarning (m_state != ATEST_STATE_NONE, "'%s' is not the name of an auto-test state", valueName);

		// If anything needs to default to something non-zero, set it here... 
		switch (m_state)
		{
			case ATEST_STATE_TEST_PERFORMANCE:
				m_stateData.testPerformance.m_subState = CAutoTester::k_testPerformance_substate_ingame;
				break;
		}

		// Read each parameter...
		while (GetParam(&str, tokenName, MAX_TOKEN_LEN, valueName, MAX_TOKEN_LEN, breakAtChar))
		{
			bool paramUsed = false;

			if (stricmp(tokenName, "outputName") == 0)
			{
				drx_strcpy(m_includeThisInFileName, valueName);
				DrxLogAlways("CAutoTester::Start has set output name to '%s'", m_includeThisInFileName);
				m_createVerboseFilename=false;
				paramUsed = true;
			}
			else
			{
				switch (m_state)
				{
				case ATEST_STATE_TEST_NUM_CLIENTS:
					if (stricmp(tokenName, "timeToRun") == 0)
					{
						m_stateData.testNumClients.m_timeOut = (float)atof(valueName);
						paramUsed = true;
					}
					else if (stricmp(tokenName, "numClients") == 0)
					{
						m_stateData.testNumClients.m_numClientsExpected = atoi(valueName);
						paramUsed = true;
					}
					break;

				case ATEST_STATE_TEST_NUM_CLIENTS_LEVEL_ROTATE:
					if (stricmp(tokenName, "timeToRunEachLevel") == 0)
					{
						m_stateData.testNumClientsRotate.m_levelTimeOut = (float)atof(valueName);
						paramUsed = true;
					}
					else if (stricmp(tokenName, "numClients") == 0)
					{
						m_stateData.testNumClientsRotate.m_numClientsExpected = atoi(valueName);
						paramUsed = true;
					}
					else if (stricmp(tokenName, "timeToRunFirstLevel") == 0)
					{														 
						m_stateData.testNumClientsRotate.m_firstLevelTimeOut = (float)atof(valueName);
						paramUsed = true;
					}
					break;

#if ENABLE_FEATURE_TESTER
				case ATEST_STATE_TEST_FEATURES:
					if (stricmp(tokenName, "rules") == 0)
					{
						DrxLogAlways ("CAutoTester::Start executing 'sv_gamerules %s'", valueName);
						gEnv->pConsole->ExecuteString(string().Format("sv_gamerules %s", valueName));
						paramUsed = true;
					}
					else if (stricmp(tokenName, "level") == 0)
					{
						IGameRulesSystem * gameRulesSystem = gEnv->pGame->GetIGameFramework()->GetIGameRulesSystem();
						tukk  sv_gamerules = gEnv->pConsole->GetCVar("sv_gamerules")->GetString();
						tukk  fullGameRulesName = gameRulesSystem->GetGameRulesName(sv_gamerules);
						bool isMultiPlayerMode = (fullGameRulesName && strcmp(fullGameRulesName, "SinglePlayer") != 0);
						tukk  additionalParams = isMultiPlayerMode ? " s nb" : "";
						if (gEnv->bMultiplayer != isMultiPlayerMode)
						{
							GameWarning("Auto-tester is loading '%s' in mode '%s' while environment is set up for %s", valueName, sv_gamerules, gEnv->bMultiplayer ? "multi-player" : "single-player");
						}
						DrxLogAlways ("CAutoTester::Start executing 'map %s%s' (mode is '%s' i.e. '%s' so isMultiPlayer=%d)", valueName, additionalParams, sv_gamerules, fullGameRulesName, isMultiPlayerMode);
						if (isMultiPlayerMode)
						{
							gEnv->pConsole->ExecuteString("net_setonlinemode lan");
							// Simulate some delay here as the game doesn't like trying to load a map before other
							// systems have caught up.
							gEnv->pConsole->ExecuteString("wait_frames 120");
							gEnv->pConsole->ExecuteString(string().Format("map %s%s", valueName, additionalParams),false,true);
						}
						else
						{
							gEnv->pConsole->ExecuteString(string().Format("map %s%s", valueName, additionalParams));
						}
						paramUsed = true;
					}
					else if (stricmp(tokenName, "set") == 0)
					{
						drx_strcpy(m_stateData.testRunFeatureTests.m_setNames, valueName);
						paramUsed = true;
					}
					else if (stricmp(tokenName, "file") == 0)
					{
						drx_strcpy(m_stateData.testRunFeatureTests.m_loadFileName, valueName);
						paramUsed = true;
					}
					break;
#endif

				case ATEST_STATE_TEST_PERFORMANCE:
					if (stricmp(tokenName, "configFile") == 0)
					{
						drx_strcpy(m_stateData.testPerformance.m_configFile, valueName);
						//m_stateData.testPerformance.m_configFile = valueName;
						DrxLogAlways ("CAutoTester::Start configFile set '%s'", valueName);
						paramUsed = true;
					}
					else if (stricmp(tokenName, "timeToRun") == 0)
					{
						m_stateData.testPerformance.m_timeOut = (float)atof(valueName);
						paramUsed = true;
					}
					else if (stricmp(tokenName, "delayToStart") == 0)
					{
						m_stateData.testPerformance.m_delayToStart = (float)atof(valueName);
						paramUsed = true;
					}
					break;
				}

				DesignerWarning(paramUsed, "Unexpected key/value pair '%s'='%s' in %s setup string!", tokenName, valueName, s_autoTesterStateNames[m_state]);
			}
		}

		// All parameters have been read... now finish initialization
		switch (m_state)
		{
		case ATEST_STATE_TEST_NUM_CLIENTS_LEVEL_ROTATE:
			{
				if (m_stateData.testNumClientsRotate.m_firstLevelTimeOut == 0)
				{
					DrxLogAlways("no timeToRunFirstLevel set, setting to timeToRunEachLevel=%d", (i32)m_stateData.testNumClientsRotate.m_levelTimeOut);
					m_stateData.testNumClientsRotate.m_firstLevelTimeOut = m_stateData.testNumClientsRotate.m_levelTimeOut;
				}

				float timeSeconds=gEnv->pTimer->GetFrameStartTime().GetSeconds();
				m_stateData.testNumClientsRotate.m_nextTimeOut = timeSeconds + m_stateData.testNumClientsRotate.m_firstLevelTimeOut;

				// level rotation is setup with +sv_levelrotation levelrotation.xml and also -root to override looking in the profile dir
#if 0
				ILevelRotationFile *levelRotFile = new CDebugLevelRotationFile("../levelrotation.xml");
				ILevelRotation *pLevelRotation = g_pGame->GetIGameFramework()->GetILevelSystem()->GetLevelRotation();
				pLevelRotation->Load(levelRotFile);
				levelRotFile->Complete();
#endif
				gEnv->pConsole->ExecuteString("g_nextlevel");	// has to be a better way of doing this

				break;
			}

		case ATEST_STATE_TEST_FEATURES:
#if ENABLE_FEATURE_TESTER
			m_writeResultsCompleteTestCasePass = true;

			if (m_stateData.testRunFeatureTests.m_loadFileName[0])
			{
				gEnv->pConsole->ExecuteString(string().Format("ft_load %s", m_stateData.testRunFeatureTests.m_loadFileName));
			}

			if (m_stateData.testRunFeatureTests.m_setNames[0])
			{
				CFeatureTester::GetInstance()->InformAutoTesterOfResults(this);
				gEnv->pConsole->ExecuteString(string().Format("ft_runAll %s", m_stateData.testRunFeatureTests.m_setNames));
			}
			else
			{
				CFeatureTester::GetInstance()->InformAutoTesterOfResults(this);
				gEnv->pConsole->ExecuteString("ft_map_runAll");
			}
#else
			DesignerWarning(false, "Feature tester is not included in this build!");
#endif
			break;
		}
	}
	else
	{
		DRX_ASSERT_MESSAGE(0, string().Format("CAutoTester::Start() failed to find state at start in %s", stateSetup));
	}

	// TODO will maybe need to load in the existing file if we want all tests in the same file... junit/bamboo should cope with each test in a different file?
}

void CAutoTester::AddTestCaseResult(tukk testSuiteName, XmlNodeRef &testCase, bool passed)
{
	if (g_pGameCVars->autotest_enabled)
	{
		STestSuite &testSuite = m_testSuites[testSuiteName];

		testSuite.m_testCases.push_back(testCase);
		if (passed)
		{
			testSuite.m_numTestCasesPassed++;
		}
		else
		{
			testSuite.m_numTestCasesFailed++;
		}

		DrxLog ("CAutoTester::AddTestCaseResult() Suite '%s' now contains %" PRISIZE_T " tests (%d passes and %d failures)", testSuiteName, testSuite.m_testCases.size(), testSuite.m_numTestCasesPassed, testSuite.m_numTestCasesFailed);
	}
}

/*static*/ bool CAutoTester::SaveToValidXmlFile( const XmlNodeRef &xmlToSave, tukk fileName)
{
#if DRX_PLATFORM_WINDOWS
	DrxSetFileAttributes( fileName,0x00000080 ); // FILE_ATTRIBUTE_NORMAL
#endif
	XmlString xmlStr = xmlToSave->getXML();

	CDebugAllowFileAccess allowFileAccess;
	FILE *file = gEnv->pDrxPak->FOpen( fileName,"wt" );
	allowFileAccess.End();
	if (file)
	{
		tukk sxml = (tukk)xmlStr;
		char xmlHeader[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
		gEnv->pDrxPak->FWrite(xmlHeader, strlen(xmlHeader), file);
		gEnv->pDrxPak->FWrite( sxml,xmlStr.length(),file );
		gEnv->pDrxPak->FFlush(file);
		gEnv->pDrxPak->FClose(file);
		return true;
	}
	return false;
}

// create the xmlnode before passing it in
void CAutoTester::CreateTestCase(XmlNodeRef &testCase, tukk testName, bool passed, tukk failedType, tukk failedMessage)
{
	assert(testName);

	testCase->setTag("testcase");
	testCase->setAttr("name", testName ? testName : "NULL");
	testCase->setAttr("time", 0);

	if (!passed)
	{
		assert(failedType);
		assert(failedMessage);

		XmlNodeRef failedCase = GetISystem()->CreateXmlNode();
		failedCase->setTag("failure");
		failedCase->setAttr("type", failedType ? failedType : "NULL");
		failedCase->setAttr("message", failedMessage ? failedMessage : "NULL");
		testCase->addChild(failedCase);
	}
}

void CAutoTester::AddSimpleTestCase(tukk  groupName, tukk  testName, float duration, tukk  failureReason, tukk  owners)
{
	bool passed = true;
	XmlNodeRef testCase = GetISystem()->CreateXmlNode();
	testCase->setTag("testcase");
	testCase->setAttr("name", testName);
	
	if (owners)
	{
		testCase->setAttr("owners", owners);
	}

	if (duration >= 0.f)
	{
		testCase->setAttr("time", duration);
	}

	// Set whatever other attributes are useful here!

	if (failureReason == NULL || failureReason[0] == '\0')
	{
		DrxLogAlways ("CAutoTester::AddSimpleTestCase() Group '%s' test '%s' passed!", groupName, testName);
		testCase->setAttr("status", "run");
	}
	else
	{
		DrxLogAlways ("CAutoTester::AddSimpleTestCase() Group '%s' test '%s' failed: %s", groupName, testName, failureReason);
		XmlNodeRef failedCase = GetISystem()->CreateXmlNode();
		failedCase->setTag("failure");
		failedCase->setAttr("type", "TestCaseFailed");
		failedCase->setAttr("message", failureReason);
		testCase->addChild(failedCase);
		passed = false;
	}

	AddTestCaseResult(string().Format("%s: %s", m_includeThisInFileName, groupName ? groupName : "No group name specified"), testCase, passed);
}

void CAutoTester::Stop()
{
	tukk  mapName;
	
	DrxLogAlways("CAutoTester::Stop called");
	if (g_pGame && g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel())
	{
		mapName = g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel()->GetName();
	}
	else
	{
		mapName = "NoMapLoaded";
	}

	if (g_pGame)
	{
		ITelemetryCollector *itc = g_pGame->GetITelemetryCollector();
		if (itc)
			itc->OutputMemoryUsage("AutoTester::Stop", mapName);
	}

	m_finished=true;
	
	gEnv->pConsole->ExecuteString("memReplayStop");

	WriteResults(kWriteResultsFlag_writeDoneMarkerFile);
	m_testSuites.clear();
}


void CAutoTester::GetOutputPathFileName(const string& baseName, string& outputFileName)
{

	tukk  mapName;
	string gameRulesName;
	tukk  serverText = "";
	tukk  dedicatedText = gEnv->IsDedicated() ? "_Dedicated" : "";

	CGameRules * rules = g_pGame ? g_pGame->GetGameRules() : NULL;
	if (rules)
	{
		gameRulesName.Format("@ui_rules_%s", rules->GetEntity()->GetClass()->GetName());
		gameRulesName = CHUDUtils::LocalizeString(gameRulesName.c_str(), NULL, NULL);
		serverText = gEnv->bServer ? "_Server" : "_Client";
	}
	else
	{
		gameRulesName = "FrontEnd";
	}

	if (g_pGame && g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel())
	{
		mapName = g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel()->GetName();
	}
	else
	{
		mapName = "NoMapLoaded";
	}


	char mapNameChrs[256];
	char gameRulesNameChrs[256];

	drx_strcpy(mapNameChrs, mapName);
	drx_strcpy(gameRulesNameChrs, gameRulesName);

	for (i32 i=0; mapNameChrs[i]; i++)
	{
		if (mapNameChrs[i] == '/' || mapNameChrs[i] == ' ')
		{
			mapNameChrs[i] = '_'; 
		}
	}

	for (i32 i=0; gameRulesNameChrs[i]; i++)
	{
		if (gameRulesNameChrs[i] == '/' || gameRulesNameChrs[i] == ' ')
		{
			gameRulesNameChrs[i] = '_';
		}
	}



	string justTheFilename;

	if (m_createVerboseFilename)
	{
		justTheFilename.Format("%s_%s_%s_%s%s%s.xml", baseName.c_str(), m_includeThisInFileName, mapNameChrs, gameRulesNameChrs, dedicatedText, serverText); // TODO add datestamp if needed
	}
	else
	{
		// If m_createVerboseFilename is false, then only include things in the filename which WON'T CHANGE between the test starting and ending [TF]
		justTheFilename.Format("%s_%s%s.xml", baseName.c_str(), m_includeThisInFileName, dedicatedText);
	}

	DrxLogAlways("justTheFilename is %s", justTheFilename.c_str());
	outputFileName.Format("./User/Xml-Reports/%s", justTheFilename.c_str());
}

void CAutoTester::WriteTestManifest(const XmlNodeRef& testManifest)
{
	
	string outputFileName;
	this->GetOutputPathFileName("testmanifest", outputFileName);
	DrxLogAlways ("Outputting testmanifest to %s", outputFileName.c_str());
	SaveToValidXmlFile(testManifest, outputFileName.c_str());
	DrxLogAlways ("testmanifest: %s has just been written", outputFileName.c_str());

}

void CAutoTester::WriteResults(TBitfield flags, const string * additionalTestSuiteName, const XmlNodeRef * additionalTestCase)
{
	i32k& autotest_enabled = g_pGameCVars->autotest_enabled;
	if (!autotest_enabled)
	{
		return;
	}

	//If result generation is skipped, exit early
	if (autotest_enabled == 2)
	{
		if (m_quitWhenDone)
		{
			gEnv->pConsole->ExecuteString("quit");
		}

		return;
	}

	tukk  mapName;
	string gameRulesName;
	tukk  serverText = "";
	tukk  dedicatedText = gEnv->IsDedicated() ? "_Dedicated" : "";

	CGameRules * rules = g_pGame ? g_pGame->GetGameRules() : NULL;
	if (rules)
	{
		gameRulesName.Format("@ui_rules_%s", rules->GetEntity()->GetClass()->GetName());
		gameRulesName = CHUDUtils::LocalizeString(gameRulesName.c_str(), NULL, NULL);
		serverText = gEnv->bServer ? "_Server" : "_Client";
	}
	else
	{
		gameRulesName = "FrontEnd";
	}

	 if (g_pGame && g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel())
	 {
		 mapName = g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel()->GetName();
	 }
	 else
	 {
		 mapName = "NoMapLoaded";
	 }

#ifndef _RELEASE
	 DrxLogAlways ("CAutoTester::WriteResults(%s): map is '%s', game mode is '%s'", AutoEnum_GetStringFromBitfield(flags, s_writeResultsFlagNames, WriteResultsFlagList_numBits).c_str(), mapName, gameRulesName.c_str());
	 INDENT_LOG_DURING_SCOPE();
#endif

#if 0
	// Designer warning verification is happening via the output for clients AND servers
	// no need to calculate it here just for server at the moment
	i32 numDesignerWarnings = GetNumDesignerWarningsHit();
	if (numDesignerWarnings > 0)
	{
		XmlNodeRef testCase = GetISystem()->CreateXmlNode();
		string name;
		name.Format("Level: %s; gameRules: %s; Number of DesignerWarnings: %d", mapName, gameRulesName.c_str(), numDesignerWarnings);
		CreateTestCase(testCase, name.c_str(), false, "DesignerWarnings", name.c_str());
		testCase->setAttr("numDesignerWarnings", numDesignerWarnings);
		AddTestCaseResult("Designer warnings", testCase, false);
	}
#endif

	XmlNodeRef testSuites = GetISystem()->CreateXmlNode("testSuites");
	testSuites->setTag("testsuites");

	for (TTestSuites::const_iterator it=m_testSuites.begin(); it!=m_testSuites.end(); ++it)
	{
		const STestSuite &testSuiteStruct = it->second;
		i32 numTests = testSuiteStruct.m_testCases.size();
		i32 numFails = testSuiteStruct.m_numTestCasesFailed;

		XmlNodeRef testSuite = GetISystem()->CreateXmlNode("AutoTest");
		testSuite->setTag("testsuite");
		testSuite->setAttr("name", it->first);
		testSuite->setAttr("LevelName", mapName);
		testSuite->setAttr("GameRulesName", gameRulesName);

		if (additionalTestSuiteName && it->first == *additionalTestSuiteName)
		{
			DrxLog ("Writing additional test case to existing '%s' suite", additionalTestSuiteName->c_str());
			++ numFails; // Assumption here that any additional test data provided is a failure... [TF]
			++ numTests;
			testSuite->addChild(*additionalTestCase);
			additionalTestSuiteName = NULL;
		}

		testSuite->setAttr("tests", numTests);
		testSuite->setAttr("failures", numFails);
		testSuite->setAttr("errors", 0);
		testSuite->setAttr("time", 0);

		for (size_t i = 0; i < testSuiteStruct.m_testCases.size(); i++)
		{
			testSuite->addChild(testSuiteStruct.m_testCases[i]);
		}

		testSuites->addChild(testSuite);
	}

	if (additionalTestSuiteName) // Still haven't written our additional test case because the suite name didn't match any existing suite...
	{
		DrxLog ("CAutoTester writing additional test case to unique '%s' suite", additionalTestSuiteName->c_str());
		XmlNodeRef testSuite = GetISystem()->CreateXmlNode("AutoTest");
		testSuite->setTag("testsuite");
		testSuite->setAttr("name", *additionalTestSuiteName);
		testSuite->setAttr("LevelName", mapName);
		testSuite->setAttr("GameRulesName", gameRulesName);
		testSuite->setAttr("tests", 1);
		testSuite->setAttr("failures", 1);
		testSuite->setAttr("errors", 0);
		testSuite->setAttr("time", 0);
		testSuite->addChild(*additionalTestCase);
		testSuites->addChild(testSuite);
	}

	char mapNameChrs[256];
	char gameRulesNameChrs[256];

	drx_strcpy(mapNameChrs, mapName);
	drx_strcpy(gameRulesNameChrs, gameRulesName);

	for (i32 i=0; mapNameChrs[i]; i++)
	{
		if (mapNameChrs[i] == '/' || mapNameChrs[i] == ' ')
		{
			mapNameChrs[i] = '_'; 
		}
	}

	for (i32 i=0; gameRulesNameChrs[i]; i++)
	{
		if (gameRulesNameChrs[i] == '/' || gameRulesNameChrs[i] == ' ')
		{
			gameRulesNameChrs[i] = '_';
		}
	}

	tukk  resultsCompleteFailMessage = NULL;

	if (flags & kWriteResultsFlag_unfinished)
	{
		resultsCompleteFailMessage = "Didn't finish all tests!";
	}
	else if (m_testSuites.size() == 0)
	{
		resultsCompleteFailMessage = "Ran zero tests!";
	}

	if (resultsCompleteFailMessage || m_writeResultsCompleteTestCasePass)
	{
		i32 numFailures = 0;
		XmlNodeRef testSuite = GetISystem()->CreateXmlNode("testsuite");
		testSuite->setAttr("name", "Are results complete?");

		XmlNodeRef unfinishedFailure = GetISystem()->CreateXmlNode("testcase");

		if (m_createVerboseFilename)
		{
			unfinishedFailure->setAttr("name", string().Format("%s_%s_%s%s%s", m_includeThisInFileName, mapNameChrs, gameRulesNameChrs, dedicatedText, serverText));
		}
		else
		{
			unfinishedFailure->setAttr("name", string().Format("%s%s", m_includeThisInFileName, dedicatedText));
		}

		if (resultsCompleteFailMessage)
		{
			XmlNodeRef failedCase = GetISystem()->CreateXmlNode("failure");
			failedCase->setAttr("type", "Unfinished");
			failedCase->setAttr("message", resultsCompleteFailMessage);
			unfinishedFailure->addChild(failedCase);
			++ numFailures;
		}
		else
		{
			unfinishedFailure->setAttr("status", "run");
		}

		testSuite->setAttr("tests", 1);
		testSuite->setAttr("errors", 0);
		testSuite->setAttr("time", 0);
		testSuite->setAttr("failures", numFailures);
		testSuite->addChild(unfinishedFailure);

		testSuites->addChild(testSuite);
	}

	string justTheFilename;

	if (m_createVerboseFilename)
	{
		justTheFilename.Format("autotest_%s_%s_%s%s%s.xml", m_includeThisInFileName, mapNameChrs, gameRulesNameChrs, dedicatedText, serverText); // TODO add datestamp if needed
	}
	else
	{
		// If m_createVerboseFilename is false, then only include things in the filename which WON'T CHANGE between the test starting and ending [TF]
		justTheFilename.Format("autotest_%s%s.xml", m_includeThisInFileName, dedicatedText);
	}

	m_outputPath.Format("./User/Xml-Reports/%s", justTheFilename.c_str());

#ifndef _RELEASE
	DrxLogAlways ("Outputting test to %s", m_outputPath.c_str());
#endif

	SaveToValidXmlFile(testSuites, m_outputPath.c_str());

	if (flags & kWriteResultsFlag_writeDoneMarkerFile)
	{
		XmlNodeRef finished = GetISystem()->CreateXmlNode("Finished");
		SaveToValidXmlFile(finished, m_outputPath + ".done");

		if (m_quitWhenDone)
		{
			gEnv->pConsole->ExecuteString("quit");
		}
	}
}

// TODO parameterise and refactor this now its mainly duplicated between the two runs
void CAutoTester::UpdateTestNumClients()
{
	if(gEnv->bServer)
	{
		IGameFramework *pFramework = gEnv->pGame->GetIGameFramework();
		i32 numChannels = 1; //local channel

		if(pFramework)
		{
			INetNub *pNub = pFramework->GetServerNetNub();

			if(pNub)
			{
				numChannels = pNub->GetNumChannels();
			}
		}

		if (numChannels > m_stateData.testNumClients.m_maxNumClientsConnected)
		{
			m_stateData.testNumClients.m_maxNumClientsConnected=numChannels;
		}

		float timeSeconds=gEnv->pTimer->GetFrameStartTime().GetSeconds();
		DrxWatch("time=%f; numClients=%d; maxNumClients=%d; numClientsExpected=%d", timeSeconds, numChannels, m_stateData.testNumClients.m_maxNumClientsConnected, m_stateData.testNumClients.m_numClientsExpected);

		if (timeSeconds > m_stateData.testNumClients.m_debugTimer)
		{
			m_stateData.testNumClients.m_debugTimer = timeSeconds+2.0f;
			DrxLogAlways("CAutoTester::UpdateTestNumClients() updating time=%f; numClients=%d; maxNumClients=%d; numClientsExpected=%d", timeSeconds, numChannels, m_stateData.testNumClients.m_maxNumClientsConnected, m_stateData.testNumClients.m_numClientsExpected);
		}

		if (timeSeconds > m_stateData.testNumClients.m_timeOut)
		{
			DrxLogAlways("CAutoTester::UpdateTestNumClients() testing num clients and time has expired. numClients=%d; maxNumClients=%d; numClientsExpected=%d", numChannels, m_stateData.testNumClients.m_maxNumClientsConnected, m_stateData.testNumClients.m_numClientsExpected);
			INDENT_LOG_DURING_SCOPE();

			bool passed=false;

			string mapName = g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel()->GetName();
			string gameRulesName;
			gameRulesName.Format("@ui_rules_%s", g_pGame->GetGameRules()->GetEntity()->GetClass()->GetName());
			gameRulesName = CHUDUtils::LocalizeString(gameRulesName.c_str(), NULL, NULL);

			XmlNodeRef testCase = GetISystem()->CreateXmlNode();
			string nameStr;
			nameStr.Format("Level: %s; gameRules: %s; numClients=%d; timeTested=%.1f seconds", mapName.c_str(), gameRulesName.c_str(), numChannels, m_stateData.testNumClients.m_timeOut);
			testCase->setTag("testcase");
			testCase->setAttr("name", nameStr.c_str());
			testCase->setAttr("time", 0);

			testCase->setAttr("numClients", numChannels);
			testCase->setAttr("maxNumClientsConnected",m_stateData.testNumClients.m_maxNumClientsConnected);
			testCase->setAttr("numClientsExpected", m_stateData.testNumClients.m_numClientsExpected);

			if (numChannels == m_stateData.testNumClients.m_maxNumClientsConnected)
			{
				DrxLogAlways("CAutoTester::UpdateTestNumClients() testing num clients and time has expired. We have the same number of clients are our maxNumClients %d", numChannels);

				if (numChannels == m_stateData.testNumClients.m_numClientsExpected)	// may want to remove this check as keeping the number that joined should be sufficient
				{
					DrxLogAlways("CAutoTester::UpdateTestNumClients() testing num clients and time has expired. We have the same number of clients as we expected to have %d", numChannels);
					testCase->setAttr("status", "run");
					passed=true;
				}
				else
				{
					DrxLogAlways("CAutoTester::UpdateTestNumClients() testing num clients and time has expired. We DON'T have the same number of clients %d as we expected to have %d", numChannels, m_stateData.testNumClients.m_numClientsExpected);
					//testCase->setAttr("status", "failed");
					XmlNodeRef failedCase = GetISystem()->CreateXmlNode();
					failedCase->setTag("failure");
					failedCase->setAttr("type", "NotEnoughClients");
					failedCase->setAttr("message", string().Format("testing num clients and time has expired. We DON'T have the same number of clients %d as we expected to have %d", numChannels, m_stateData.testNumClients.m_numClientsExpected));
					testCase->addChild(failedCase);
				}
			}
			else
			{
				//testCase->setAttr("status", "failed");
				XmlNodeRef failedCase = GetISystem()->CreateXmlNode();
				failedCase->setTag("failure");
				failedCase->setAttr("type", "NotEnoughClients");
				failedCase->setAttr("message", string().Format("testing num clients and time has expired. We DON'T have the same number of clients %d as we peaked at %d", numChannels, m_stateData.testNumClients.m_maxNumClientsConnected));
				testCase->addChild(failedCase);
			}

			AddTestCaseResult("Test Clients In Levels", testCase, passed);
			Stop();
		}
	}
}

void CAutoTester::UpdateTestNumClientsLevelRotate()
{
	if(gEnv->bServer)
	{
		IGameFramework *pFramework = gEnv->pGame->GetIGameFramework();
		i32 numChannels = 1; //local channel

		if(pFramework)
		{
			INetNub *pNub = pFramework->GetServerNetNub();

			if(pNub)
			{
				numChannels = pNub->GetNumChannels();
			}
		}

		if (numChannels > m_stateData.testNumClientsRotate.m_maxNumClientsConnected)
		{
			m_stateData.testNumClientsRotate.m_maxNumClientsConnected=numChannels;
		}

		float timeSeconds=gEnv->pTimer->GetFrameStartTime().GetSeconds();
		DrxWatch("time=%f; nextTimeOut=%f; numClients=%d; maxNumClients=%d; numClientsExpected=%d", timeSeconds, m_stateData.testNumClientsRotate.m_nextTimeOut, numChannels, m_stateData.testNumClientsRotate.m_maxNumClientsConnected, m_stateData.testNumClientsRotate.m_numClientsExpected);

		if (timeSeconds > m_stateData.testNumClientsRotate.m_debugTimer)
		{
			m_stateData.testNumClientsRotate.m_debugTimer = timeSeconds+2.0f;
			DrxLogAlways("CAutoTester::UpdateTestNumClientsLevelRotate() updating time=%f; nextTimeOut=%f; numClients=%d; maxNumClients=%d; numClientsExpected=%d", timeSeconds, m_stateData.testNumClientsRotate.m_nextTimeOut, numChannels, m_stateData.testNumClientsRotate.m_maxNumClientsConnected, m_stateData.testNumClientsRotate.m_numClientsExpected);
		}

		if (timeSeconds > m_stateData.testNumClientsRotate.m_nextTimeOut)
		{
			DrxLogAlways("CAutoTester::UpdateTestNumClientsLevelRotate() testing num clients and time has expired. numClients=%d; maxNumClients=%d; numClientsExpected=%d", numChannels, m_stateData.testNumClientsRotate.m_maxNumClientsConnected, m_stateData.testNumClientsRotate.m_numClientsExpected);

			bool passed=false;

			ILevelRotation *pLevelRotation = g_pGame->GetIGameFramework()->GetILevelSystem()->GetLevelRotation();

			string mapName = g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel()->GetName();
			string gameRulesName;
			gameRulesName.Format("@ui_rules_%s", g_pGame->GetGameRules()->GetEntity()->GetClass()->GetName());
			gameRulesName = CHUDUtils::LocalizeString(gameRulesName.c_str(), NULL, NULL);

			XmlNodeRef testCase = GetISystem()->CreateXmlNode();
			string nameStr;
			if (m_stateData.testNumClientsRotate.m_levelIndex == 0)
			{
				nameStr.Format("%02d/%d) Level: %s; gameRules: %s; numClients=%d; timeTested=%.1f seconds", m_stateData.testNumClientsRotate.m_levelIndex+1, pLevelRotation->GetLength(), mapName.c_str(), gameRulesName.c_str(), numChannels, m_stateData.testNumClientsRotate.m_firstLevelTimeOut);
			}
			else
			{
				nameStr.Format("%02d/%d) Level: %s; gameRules: %s; numClients=%d; timeTested=%.1f seconds", m_stateData.testNumClientsRotate.m_levelIndex+1, pLevelRotation->GetLength(), mapName.c_str(), gameRulesName.c_str(), numChannels, m_stateData.testNumClientsRotate.m_levelTimeOut);
			}

			DrxLogAlways("CAutoTester::UpdateTestNumClientsLevelRotate() outputting a test result with these details [%s]", nameStr.c_str());

			testCase->setTag("testcase");
			testCase->setAttr("name", nameStr.c_str());
			testCase->setAttr("time", 0);

			testCase->setAttr("numClients", numChannels);
			testCase->setAttr("maxNumClientsConnected",m_stateData.testNumClientsRotate.m_maxNumClientsConnected);
			testCase->setAttr("numClientsExpected", m_stateData.testNumClientsRotate.m_numClientsExpected);

			if (numChannels == m_stateData.testNumClientsRotate.m_maxNumClientsConnected)
			{
				DrxLogAlways("CAutoTester::UpdateTestNumClientsLevelRotate() testing num clients and time has expired. We have the same number of clients are our maxNumClients %d", numChannels);

				if (numChannels == m_stateData.testNumClientsRotate.m_numClientsExpected)	// may want to remove this check as keeping the number that joined should be sufficient
				{
					DrxLogAlways("CAutoTester::UpdateTestNumClientsLevelRotate() testing num clients and time has expired. We have the same number of clients as we expected to have %d", numChannels);
					testCase->setAttr("status", "run");
					passed=true;
				}
				else
				{
					DrxLogAlways("CAutoTester::UpdateTestNumClientsLevelRotate() testing num clients and time has expired. We DON'T have the same number of clients %d as we expected to have %d", numChannels, m_stateData.testNumClientsRotate.m_numClientsExpected);
					//testCase->setAttr("status", "failed");
					XmlNodeRef failedCase = GetISystem()->CreateXmlNode();
					failedCase->setTag("failure");
					failedCase->setAttr("type", "NotEnoughClients");
					failedCase->setAttr("message", string().Format("testing num clients and time has expired. We DON'T have the same number of clients %d as we expected to have %d", numChannels, m_stateData.testNumClientsRotate.m_numClientsExpected));
					testCase->addChild(failedCase);
				}
			}
			else
			{
				//testCase->setAttr("status", "failed");
				XmlNodeRef failedCase = GetISystem()->CreateXmlNode();
				failedCase->setTag("failure");
				failedCase->setAttr("type", "NotEnoughClients");
				failedCase->setAttr("message", string().Format("testing num clients and time has expired. We DON'T have the same number of clients %d as we peaked at %d", numChannels, m_stateData.testNumClientsRotate.m_maxNumClientsConnected));
				testCase->addChild(failedCase);
			}

			AddTestCaseResult("Test Clients In Level Rotation", testCase, passed);
			Stop();


		
			if (pLevelRotation->GetNext() != 0)
			{
				DrxLogAlways("CAutoTester::UpdateTestNumClientsLevelRotate() has found we're not at the end of the level rotation moving on to the next level - levelIndex=%d; rotation->GetNext()=%d\n", m_stateData.testNumClientsRotate.m_levelIndex+1, pLevelRotation->GetNext());
				Restart();
				gEnv->pConsole->ExecuteString("g_nextlevel");	// has to be a better way of doing this
				m_stateData.testNumClientsRotate.m_nextTimeOut = timeSeconds + m_stateData.testNumClientsRotate.m_levelTimeOut;
				m_stateData.testNumClientsRotate.m_levelIndex++;
			}
			else
			{
				DrxLogAlways("CAutoTester::UpdateTestNumClientsLevelRotate() has found we ARE at the end of the level rotation. Not doing anymore tests\n");
			}
		}
	}
}

void CAutoTester::UpdateTestFeatureTests()
{
#if ENABLE_FEATURE_TESTER
	if (! CFeatureTester::GetInstance()->GetIsActive())
	{
		CFeatureTester::GetInstance()->InformAutoTesterOfResults(NULL);
		DrxLogAlways ("CAutoTester::UpdateTestFeatureTests() discovered that the feature tester is no longer active! We're done!");
		INDENT_LOG_DURING_SCOPE();
		Stop();
	}
#endif
}

void CAutoTester::UpdatePerformanceTestInGame()
{
	IActor* pPlayer = gEnv->pGame->GetIGameFramework()->GetClientActor();
	if (pPlayer)
	{
		float timeSeconds=gEnv->pTimer->GetFrameStartTime().GetSeconds();

		if (pPlayer->IsDead())
		{
			DrxLogAlways("CAutoTester::UpdatePerformanceTestInGame() detected that our player is dead - requesting revive");

			IGameRulesSpawningModule *pSpawningModule = g_pGame->GetGameRules()->GetSpawningModule();
			if (pSpawningModule)
			{
				DrxLog("CAutoTester::UpdatePerformanceTestInGame() Requesting revive");
				pSpawningModule->ClRequestRevive(pPlayer->GetEntityId());
			}
		}
		else
		{
			if (!m_stateData.testPerformance.m_bConfigExecuted)
			{
				if (timeSeconds > m_stateData.testPerformance.m_delayToStart)
				{
					DrxLogAlways("CAutoTester::UpdatePerformanceTestInGame() executing configfile=%s", m_stateData.testPerformance.m_configFile);
					gEnv->pConsole->ExecuteString(string().Format("exec %s", m_stateData.testPerformance.m_configFile));
					m_stateData.testPerformance.m_bConfigExecuted = true;
				}
			}
		}
	
		//DrxWatch("time=%f; timeOut=%f", timeSeconds, m_stateData.testPerformance.m_timeOut);

		if (timeSeconds > m_stateData.testPerformance.m_timeOut)
		{
			DrxLogAlways("CAutoTester::UpdatePerformanceTestInGame() finished running test - disconnecting to generate telemetry");
			gEnv->pConsole->ExecuteString("disconnect");
			m_stateData.testPerformance.m_subState = CAutoTester::k_testPerformance_substate_waiting_to_submit_telemetry;
			m_stateData.testPerformance.m_timeOut = timeSeconds + 10.0f;
		}
	}
}

void CAutoTester::UpdatePerformanceTestWaitingForTelemetry()
{
	float timeSeconds=gEnv->pTimer->GetFrameStartTime().GetSeconds();
	if (timeSeconds > m_stateData.testPerformance.m_timeOut)
	{
		DrxLogAlways("CAutoTester::UpdatePerformanceTestWaitingForTelemetry() finished waiting to let game generate telemetry. Passing test!");
		AddSimpleTestCase("Performance test", string().Format("%s - Succeeded outputting telemetry", m_stateData.testPerformance.m_configFile), timeSeconds, NULL);
		Stop();
	}
}

void CAutoTester::UpdatePerformanceTest()
{
	switch(m_stateData.testPerformance.m_subState)
	{
		case CAutoTester::k_testPerformance_substate_ingame:
		{
			UpdatePerformanceTestInGame();
			break;
		}
		case CAutoTester::k_testPerformance_substate_waiting_to_submit_telemetry:
		{
			UpdatePerformanceTestWaitingForTelemetry();
			break;
		}
	}
}


void CAutoTester::Update()
{
	if (m_started)
	{
		DrxWatch("[AUTOTESTER] %s \"%s\" tests", m_finished ? "Finished" : "Running", m_includeThisInFileName);
		for (TTestSuites::const_iterator it=m_testSuites.begin(); it!=m_testSuites.end(); ++it)
		{
			const STestSuite &testSuiteStruct = it->second;
			DrxWatch("[AUTOTESTER] Suite '%s' %spass=%d$o %sfail=%d$o", it->first.c_str(), testSuiteStruct.m_numTestCasesPassed ? "$3" : "$9", testSuiteStruct.m_numTestCasesPassed, testSuiteStruct.m_numTestCasesFailed ? "$4" : "$9", testSuiteStruct.m_numTestCasesFailed);
		}

		if (! m_finished)
		{
			switch (m_state)
			{
				case ATEST_STATE_TEST_NUM_CLIENTS:
					UpdateTestNumClients();
					break;
				case ATEST_STATE_TEST_NUM_CLIENTS_LEVEL_ROTATE:
					UpdateTestNumClientsLevelRotate();
					break;
				case ATEST_STATE_TEST_FEATURES:
					UpdateTestFeatureTests();
					break;
				case ATEST_STATE_TEST_PERFORMANCE:
					UpdatePerformanceTest();
					break;
				default:
					DRX_ASSERT_MESSAGE(0, string().Format("CAutoTester::Update() unrecognised state %d (%s)", m_state, s_autoTesterStateNames[m_state]));
					break;
			}
		}
	}
	else if (g_pGameCVars->autotest_enabled)
	{
		DrxLogAlways("CAutoTester::Update() autotest_enabled");
		INDENT_LOG_DURING_SCOPE();

		gEnv->pConsole->ExecuteString("i_forcefeedback 0");

		Start(g_pGameCVars->autotest_state_setup->GetString(), "./User/Xml-Reports/autotest_.xml", g_pGameCVars->autotest_quit_when_done != 0);		// TODO add datestamp if needed
	}
}

CAutoTester::AutoTestState CAutoTester::FindStateFromStr(tukk stateName)
{
	AutoTestState state=ATEST_STATE_NONE;

	for (i32 i=0; i<ATEST_STATE_NUM; i++)
	{
		bool match = stricmp(stateName, s_autoTesterStateNames[i]) == 0;
		DrxLog ("Does '%s' match '%s'... %s", stateName, s_autoTesterStateNames[i], match ? "yes!" : "no");
		if (match)
		{
			state=(AutoTestState)i;
			break;
		}
	}
	return state;
}

bool CAutoTester::GetParam(tukk* str, char outName[], i32 inNameMaxSize, char outValue[], i32 inValueMaxSize, char breakAtChar)
{
	bool success=false;
	i32 nameLength;
	i32 valueLength;

	assert(str);
	assert(outName);
	assert(outValue);

	outName[0]=0;
	outValue[0]=0;

	// Get Name
	if (*str)
	{
		nameLength=drx_copyStringUntilFindChar(outName, *str, inNameMaxSize, ':');

		if (nameLength)
		{
			*str += nameLength;

			valueLength=drx_copyStringUntilFindChar(outValue, *str, inValueMaxSize, breakAtChar);
			success=true;

			// Get Value
			if (valueLength)
			{
				*str+= valueLength;
			}
			else
			{
				*str = NULL;
			}
		}
		else
		{
			DesignerWarning (outName[0] == '\0', "Syntax error in auto tester initialization string: found '%s' at end of string, expected 'key:value' format", outName);
		}
	}

	if (success)
	{
		DrxLogAlways ("CAutoTester::GetParam found parameter called '%s' with value '%s'", outName, outValue);
	}
	else
	{
		DrxLogAlways ("CAutoTester::GetParam found no more parameters");
	}

	return success;
}

#endif // ENABLE_AUTO_TESTER

