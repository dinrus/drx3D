// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   UnitTestExcelReporter.cpp
//  Created:     19/03/2008 by Timur.
//  Описание: Implementation of the DinrusX Unit Testing framework
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/UnitTestExcelReporter.h>

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/IConsole.h>

#include <drx3D/CoreX/Platform/DrxLibrary.h>

#if DRX_PLATFORM_WINDOWS
	#include <shellapi.h> // requires <windows.h>
#endif

using namespace DrxUnitTest;

constexpr char kOutputFileName[] = "%USER%/TestResults/UnitTest.xml";
constexpr char kOutputFileNameJUnit[] = "%USER%/TestResults/UnitTestJUnit.xml";

void CUnitTestExcelReporter::OnFinishTesting(const SUnitTestRunContext& context)
{
	// Generate report.
	XmlNodeRef Workbook = GetISystem()->CreateXmlNode("Workbook");
	InitExcelWorkbook(Workbook);
	NewWorksheet("UnitTests");

	XmlNodeRef Column;
	Column = m_CurrTable->newChild("Column");
	Column->setAttr("ss:Width", 200);
	Column = m_CurrTable->newChild("Column");
	Column->setAttr("ss:Width", 50);
	Column = m_CurrTable->newChild("Column");
	Column->setAttr("ss:Width", 80);
	Column = m_CurrTable->newChild("Column");
	Column->setAttr("ss:Width", 200);
	Column = m_CurrTable->newChild("Column");
	Column->setAttr("ss:Width", 200);
	Column = m_CurrTable->newChild("Column");
	Column->setAttr("ss:Width", 60);

	AddRow();
	AddCell("Run Tests", CELL_BOLD);
	AddCell(context.testCount);
	AddRow();
	AddCell("Succeeded Tests", CELL_BOLD);
	AddCell(context.succedTestCount);
	AddRow();
	AddCell("Failed Tests", CELL_BOLD);
	AddCell(context.failedTestCount);
	if (context.failedTestCount != 0)
	{
		SetCellFlags(m_CurrCell, CELL_BOLD | CELL_CENTERED);
	}
	AddRow();
	AddCell("Success Ratio %", CELL_BOLD);
	if (context.testCount > 0)
	{
		AddCell(100 * context.succedTestCount / context.testCount);
	}
	else
	{
		AddCell(0);
	}

	AddRow();
	AddRow();

	AddRow();
	m_CurrRow->setAttr("ss:StyleID", "s25");
	AddCell("Test Name");
	AddCell("Status");
	AddCell("Run Time(ms)");
	AddCell("Failure Description");
	AddCell("File");
	AddCell("Line");

	string name;

	for (const STestResult& res : m_results)
	{
		if (res.bSuccess)
		{
			continue;
		}

		AddRow();
		if (res.autoTestInfo.szTaskName != 0)
		{
			name.Format("[%s] %s:%s.%s", res.testInfo.GetModule(), res.testInfo.GetSuite(), res.testInfo.GetName(), res.autoTestInfo.szTaskName);
		}
		else
		{
			name.Format("[%s] %s:%s", res.testInfo.GetModule(), res.testInfo.GetSuite(), res.testInfo.GetName());
		}
		AddCell(name);
		AddCell("FAIL", CELL_CENTERED);
		AddCell((i32)res.fRunTimeInMs);
		AddCell(res.failureDescription, CELL_CENTERED);
		AddCell(res.testInfo.GetFileName());
		AddCell(res.testInfo.GetLineNumber());
	}

	/////////////////////////////////////////////////////////////////////////////
	NewWorksheet("All Tests");

	Column = m_CurrTable->newChild("Column");
	Column->setAttr("ss:Width", 200);
	Column = m_CurrTable->newChild("Column");
	Column->setAttr("ss:Width", 50);
	Column = m_CurrTable->newChild("Column");
	Column->setAttr("ss:Width", 80);
	Column = m_CurrTable->newChild("Column");
	Column->setAttr("ss:Width", 200);
	Column = m_CurrTable->newChild("Column");
	Column->setAttr("ss:Width", 200);
	Column = m_CurrTable->newChild("Column");
	Column->setAttr("ss:Width", 60);

	AddRow();
	m_CurrRow->setAttr("ss:StyleID", "s25");
	AddCell("Test Name");
	AddCell("Status");
	AddCell("Run Time(ms)");
	AddCell("Failure Description");
	AddCell("File");
	AddCell("Line");

	for (const STestResult& res : m_results)
	{
		AddRow();
		if (res.autoTestInfo.szTaskName != 0)
		{
			name.Format("[%s] %s:%s.%s", res.testInfo.GetModule(), res.testInfo.GetSuite(), res.testInfo.GetName(), res.autoTestInfo.szTaskName);
		}
		else
		{
			name.Format("[%s] %s:%s", res.testInfo.GetModule(), res.testInfo.GetSuite(), res.testInfo.GetName());
		}
		AddCell(name);
		if (res.bSuccess)
		{
			AddCell("OK", CELL_CENTERED);
		}
		else
		{
			AddCell("FAIL", CELL_CENTERED);
		}
		AddCell((i32)res.fRunTimeInMs);
		AddCell(res.failureDescription, CELL_CENTERED);
		AddCell(res.testInfo.GetFileName());
		AddCell(res.testInfo.GetLineNumber());
	}

	bool bSaveSucceed = SaveToFile(kOutputFileName);
	bSaveSucceed &= SaveJUnitCompatableXml();
	PostFinishTesting(context, bSaveSucceed);
}

//////////////////////////////////////////////////////////////////////////
bool CUnitTestExcelReporter::SaveJUnitCompatableXml()
{
	XmlNodeRef root = GetISystem()->CreateXmlNode("testsuite");

	//<testsuite failures="0" time="0.289" errors="0" tests="3" skipped="0"	name="UnitTests.MainClassTest">

	i32 errors = 0;
	i32 skipped = 0;
	i32 failures = 0;
	float totalTime = 0;
	for (const STestResult& res : m_results)
	{
		totalTime += res.fRunTimeInMs;
		failures += (res.bSuccess) ? 0 : 1;
	}
	XmlNodeRef suiteNode = root;
	suiteNode->setAttr("time", totalTime);
	suiteNode->setAttr("errors", errors);
	suiteNode->setAttr("failures", failures);
	suiteNode->setAttr("tests", (i32)m_results.size());
	suiteNode->setAttr("skipped", skipped);
	suiteNode->setAttr("name", "UnitTests");

	for (const STestResult& res : m_results)
	{
		XmlNodeRef testNode = suiteNode->newChild("testcase");
		testNode->setAttr("time", res.fRunTimeInMs);
		testNode->setAttr("name", res.testInfo.GetName());
		//<testcase time="0.146" name="TestPropertyValue"	classname="UnitTests.MainClassTest"/>

		if (!res.bSuccess)
		{
			XmlNodeRef failNode = testNode->newChild("failure");
			failNode->setAttr("type", res.testInfo.GetModule());
			failNode->setAttr("message", res.failureDescription);
			string err;
			err.Format("%s at line %d", res.testInfo.GetFileName(), res.testInfo.GetLineNumber());
			failNode->setContent(err);
		}
	}

	return root->saveToFile(kOutputFileNameJUnit);
}

void CUnitTestExcelReporter::OnSingleTestStart(const IUnitTest& test)
{
	const CUnitTestInfo& testInfo = test.GetInfo();
	string text;
	text.Format("Test Started: [%s] %s:%s", testInfo.GetModule(), testInfo.GetSuite(), testInfo.GetName());
	m_log.Log(text);
}

void CUnitTestExcelReporter::OnSingleTestFinish(const IUnitTest& test, float fRunTimeInMs, bool bSuccess, char const* szFailureDescription)
{
	const CUnitTestInfo& info = test.GetInfo();
	if (bSuccess)
	{
		m_log.Log("UnitTestFinish: [%s]%s:%s | OK (%3.2fms)", info.GetModule(), info.GetSuite(), info.GetName(), fRunTimeInMs);
	}
	else
	{
		m_log.Log("UnitTestFinish: [%s]%s:%s | FAIL (%s)", info.GetModule(), info.GetSuite(), info.GetName(), szFailureDescription);
	}

	STestResult testResult
	{
		info,
		test.GetAutoTestInfo(),
		fRunTimeInMs,
		bSuccess,
		szFailureDescription
	};
	m_results.push_back(testResult);
}

void DrxUnitTest::CUnitTestExcelNotificationReporter::PostFinishTesting(const SUnitTestRunContext& context, bool bSavedReports) const
{
#if DRX_PLATFORM_WINDOWS
	if (!bSavedReports)
	{
		// For local unit testing notify user to close previously opened report.
		// Use primitive windows msgbox because we are supposed to hide all pop-ups during auto testing.
		DrxMessageBox("Unit test failed to save one or more report documents, make sure the file is writable!", "Unit Test", eMB_Error);
	}
	else
	{
		//Open report file if any test failed. Since the notification is used for local testing only, we only need Windows
		if (context.failedTestCount > 0)
		{
			m_log.Log("%d Tests failed, opening report...", context.failedTestCount);
			i32 nAdjustFlags = 0;
			char path[_MAX_PATH];
			tukk szAdjustedPath = gEnv->pDrxPak->AdjustFileName(kOutputFileName, path, nAdjustFlags);
			if (szAdjustedPath != nullptr)
			{
				//should open it with Excel
			if(!::ShellExecute(NULL, "open", szAdjustedPath, NULL, NULL, SW_SHOW))
				{
					m_log.Log("Failed to open report %s, error code: %d", szAdjustedPath, ::GetLastError());
				}
			}
		}
	}
#endif
}
