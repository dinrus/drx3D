// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! Implementation of the DinrusX Unit Testing framework

#pragma once

#include <drx3D/Sys/DrxUnitTest.h>
#include <drx3D/Sys/ILog.h>
#include <drx3D/CoreX/String/DrxString.h>

struct SAutoTestsContext;

namespace DrxUnitTest
{

class CUnitTest final : public IUnitTest
{
public:
	CUnitTest(CUnitTestInfo info) : m_info(std::move(info)) {}
	virtual const CUnitTestInfo& GetInfo() const override { return m_info; };
	virtual const SAutoTestInfo& GetAutoTestInfo() const override
	{
		return m_info.GetTest().m_autoTestInfo;
	}
	virtual void Init() override
	{
		m_info.GetTest().Init();
	};
	virtual void Run() override
	{
		m_info.GetTest().Run();
	};
	virtual void Done() override
	{
		m_info.GetTest().Done();
	};

	CUnitTestInfo m_info;
};

class CLogUnitTestReporter : public IUnitTestReporter
{
public:
	CLogUnitTestReporter(ILog& log) : m_log(log) {}
private:
	ILog& m_log;
private:
	virtual void OnStartTesting(const SUnitTestRunContext& context) override;
	virtual void OnFinishTesting(const SUnitTestRunContext& context) override;
	virtual void OnSingleTestStart(const IUnitTest& test) override;
	virtual void OnSingleTestFinish(const IUnitTest& test, float fRunTimeInMs, bool bSuccess, char const* szFailureDescription) override;
};

class CMinimalLogUnitTestReporter : public IUnitTestReporter
{
public:
	CMinimalLogUnitTestReporter(ILog& log) : m_log(log) {}
private:
	ILog& m_log;
	i32   m_nRunTests = 0;
	i32   m_nSucceededTests = 0;
	i32   m_nFailedTests = 0;
	float m_fTimeTaken = 0.f;

private:
	virtual void OnStartTesting(const SUnitTestRunContext& context) override;
	virtual void OnFinishTesting(const SUnitTestRunContext& context) override;
	virtual void OnSingleTestStart(const IUnitTest& test) override {}
	virtual void OnSingleTestFinish(const IUnitTest& test, float fRunTimeInMs, bool bSuccess, char const* szFailureDescription) override;
};

class CUnitTestUpr : public IUnitTestUpr, IErrorObserver
{
public:
	CUnitTestUpr(ILog& logToUse);
	virtual ~CUnitTestUpr();

public:
	virtual IUnitTest* GetTestInstance(const CUnitTestInfo& info) override;
	virtual i32        RunAllTests(EReporterType reporterType) override;
	virtual void       RunAutoTests(tukk szSuiteName, tukk szTestName) override; //!< Currently not in use
	virtual void       Update() override;                                                      //!< Currently not in use

	virtual void       SetExceptionCause(tukk szExpression, tukk szFile, i32 line) override;
private:
	void               StartTesting(SUnitTestRunContext& context, EReporterType reporterToUse);
	void               EndTesting(SUnitTestRunContext& context);
	void               RunTest(CUnitTest& test, SUnitTestRunContext& context);
	bool               IsTestMatch(const CUnitTest& Test, const string& sSuiteName, const string& sTestName) const;

	//! Implement IErrorObserver
	virtual void       OnAssert(tukk szCondition, tukk szMessage, tukk szFileName, u32 fileLineNumber) override;
	virtual void       OnFatalError(tukk szMessage) override;

private:
	ILog&                                   m_log;
	std::vector<std::unique_ptr<CUnitTest>> m_tests;
	string                                  m_failureMsg;
	std::unique_ptr<SAutoTestsContext>      m_pAutoTestsContext;
	bool m_bRunningTest = false;
};
} // namespace DrxUnitTest
