// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

struct ILog;
struct IGameplayListener;

namespace DrxUnitTest {
struct IUnitTestUpr;
}

struct STimeDemoFrameInfo
{
	i32   nPolysRendered;
	float fFrameRate;
	i32   nDrawCalls;
};

struct STimeDemoInfo
{
	std::vector<STimeDemoFrameInfo> frames;

	float               lastPlayedTotalTime = 0.0f;
	float               lastAveFrameRate = 0.0f;
	float               minFPS = 0.0f;
	float               maxFPS = 0.0f;
	u32              minFPS_Frame = 0;
	u32              maxFPS_Frame = 0;

	//! How many polygons were recorded.
	u32 nTotalPolysRecorded = 0;

	//! How many polygons were played.
	u32 nTotalPolysPlayed = 0;
};

//! Automatic game testing system.
struct ITestSystem
{
	// <interfuscator:shuffle>
	virtual ~ITestSystem(){}

	//! Can be called through console e.g. #System.ApplicationTest("testcase0").
	//! \param szParam Must not be 0.
	virtual void ApplicationTest(tukk szParam) = 0;

	//! Should be called every system update.
	virtual void  Update() = 0;

	virtual void  BeforeRender() = 0;
	virtual void  AfterRender() = 0;
	virtual void  InitLog() = 0;
	virtual ILog* GetILog() = 0;

	//! To free the system (not reference counted).
	virtual void Release() = 0;

	//! \param fInNSeconds <=0 to deactivate.
	virtual void QuitInNSeconds(const float fInNSeconds) = 0;

	//! Set info about time demo (called by time demo system).
	virtual void SetTimeDemoInfo(STimeDemoInfo* pTimeDemoInfo) = 0;

	//! Retrieve info about last played time demo (return NULL if no time demo info available).
	virtual STimeDemoInfo*                 GetTimeDemoInfo() = 0;

	virtual DrxUnitTest::IUnitTestUpr* GetIUnitTestUpr() = 0;
	// </interfuscator:shuffle>
};

//! \endcond