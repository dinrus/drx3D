// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FEATURETESTER_H__
#define __FEATURETESTER_H__

#include <drx3D/Game/AutoEnum.h>
#include <drx3D/Game/GameCodeCoverage/IGameCodeCoverageListener.h>
#include <drx3D/Game/Utility/SingleAllocTextBlock.h>
#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageEnabled.h>
#include <drx3D/Game/GameMechanismBase.h>
#include <drx3D/Game/Actor.h>

#if defined(_RELEASE) && !defined(PERFORMANCE_BUILD)
	// Final release - don't enable feature tester!
	#define ENABLE_FEATURE_TESTER            0
#else
	// Feel free to turn this on/off or add more conditions (platform, _DEBUG etc.) here...
	#define ENABLE_FEATURE_TESTER            1
#endif

#if ENABLE_FEATURE_TESTER

// Only include if FT enabled
#include <drx3D/Game/FeatureTestMgr.h>

struct SFeatureTestInstructionOrParam;
struct SFeatureTestDataLoadWorkspace;
struct IFeatureTestTextHandler;
class CAutoTester;
class CActor;

#define FeatureTestPauseReasonList(f)         \
	f(kFTPauseReason_none)                      \
	f(kFTPauseReason_untilTimeHasPassed)        \
	f(kFTPauseReason_untilCCCPointsHit)         \
	f(kFTPauseReason_untilWeaponIsReadyToUse)   \
	f(kFTPauseReason_untilPlayerIsAlive)        \
	f(kFTPauseReason_untilPlayerAimingAtEnemy)  \
	f(kFTPauseReason_untilFGTestsComplete)			\

#define FeatureTestCommandList(f)             \
	f(kFTC_End)                                 \
	f(kFTC_Fail)                                \
	f(kFTC_SetItem)                             \
	f(kFTC_SetAmmo)                             \
	f(kFTC_Wait)                                \
	f(kFTC_WaitSingleFrame)                     \
	f(kFTC_WaitUntilHitAllExpectedCCCPoints)    \
	f(kFTC_OverrideButtonInput_Press)           \
	f(kFTC_OverrideButtonInput_Release)         \
	f(kFTC_OverrideAnalogInput)                 \
	f(kFTC_WatchCCCPoint)                       \
	f(kFTC_ResetCCCPointHitCounters)            \
	f(kFTC_CheckNumCCCPointHits)                \
	f(kFTC_SetResponseToHittingCCCPoint)        \
	f(kFTC_DoConsoleCommand)                    \
	f(kFTC_RunFeatureTest)                      \
	f(kFTC_TrySpawnPlayer)                      \
	f(kFTC_WaitUntilPlayerIsAlive)              \
	f(kFTC_WaitUntilPlayerAimingAt)             \
	f(kFTC_MovePlayerToOtherEntity)             \
	f(kFTC_JumpIfConditionsDoNotMatch)          \
	f(kFTC_Jump)                                \
	f(kFTC_SetLocalPlayerLookAt)                \
	f(kFTC_RunFlowGraphFeatureTests)						\

#define FeatureTestRequirementList(f)         \
	f(kFTReq_noLevelLoaded)                     \
	f(kFTReq_inLevel)                           \
	f(kFTReq_localPlayerExists)                 \
	f(kFTReq_remotePlayerExists)                \

#define FeatureTestCheckpointHitResponseList(f)   \
	f(kFTCHR_nothing)                               \
	f(kFTCHR_failTest)                              \
	f(kFTCHR_completeTest)                          \
	f(kFTCHR_completeSubroutine)                    \
	f(kFTCHR_restartTest)                           \
	f(kFTCHR_restartSubroutine)                     \
	f(kFTCHR_expectedNext)                          \

#define FeatureTestPlayerSelectionList(f)     \
	f(kFTPS_localPlayer)                        \
	f(kFTPS_firstRemotePlayer)                  \
	f(kFTPS_secondRemotePlayer)                 \

#define FeatureTestAimConditionList(f)        \
	f(kFTAC_nobody)                             \
	f(kFTAC_anybody)                            \
	f(kFTAC_friend)                             \
	f(kFTAC_enemy)                              \

AUTOENUM_BUILDENUMWITHTYPE(EFTPauseReason, FeatureTestPauseReasonList);
AUTOENUM_BUILDENUMWITHTYPE_WITHINVALID_WITHNUM(EFeatureTestCommand, FeatureTestCommandList, kFTC_Invalid, kFTC_Num);
AUTOENUM_BUILDFLAGS_WITHZERO(FeatureTestRequirementList, kFTReq_none);
AUTOENUM_BUILDENUMWITHTYPE_WITHNUM(EFTCheckpointHitResponse, FeatureTestCheckpointHitResponseList, kFTCHR_num);
AUTOENUM_BUILDFLAGS_WITHZERO(FeatureTestPlayerSelectionList, kFTPS_nobody);
AUTOENUM_BUILDENUMWITHTYPE_WITHNUM(EFTAimCondition, FeatureTestAimConditionList, kFTAC_num);

#define DO_COMMAND_PROTOTYPE(name)        void Instruction_ ## name ();

class CFeatureTester : public CGameMechanismBase
#if ENABLE_GAME_CODE_COVERAGE
	, public IGameCodeCoverageListener
#endif
{
	friend class CFeatureTestArgumentAutoComplete;

	private:
	static i32k kMaxWatchedCheckpoints = 16;
	static i32k kMaxSimultaneouslyOverriddenInputs = 8;

	typedef void (CFeatureTester::*InstructionFunc)();

	struct SCheckpointCount
	{
		tukk               m_checkpointName;
		tukk               m_customMessage;
		i32                       m_timesHit;
		i32                       m_stackLevelAtWhichAdded;
		float                     m_restartDelay;
		EFTCheckpointHitResponse  m_hitResponse;

		// For spotting cross-project code checkpoints getting hit...
		size_t                    m_checkpointMgrHandle;
		u32                    m_checkpointMgrNumHitsSoFar;
	};

	union UPausedInfo
	{
		struct
		{
			TBitfield m_waitUntilPlayerIsAlive_whichPlayers;
		};
		struct
		{
			TBitfield m_waitUntilTargettingAnEnemy_whichPlayers;
			EFTAimCondition m_waitUntilTargettingAnEnemy_requireTargetOfType;
		};
	};

	struct SFeatureTest
	{
		tukk  m_setName;
		tukk  m_testName;
		tukk  m_owners;
		tukk  m_prerequisite;
		tukk  m_testDescription;
		tukk  m_iterateOverParams;
		float m_maxTime;
		float m_scaleSpeed;
		TBitfield m_requirementBitfield;
		u16 m_offsetIntoInstructionBuffer;
		bool m_enabled;
		bool m_autoRunThis;
		bool m_everPassed;
	};

	struct SStackedTestCallInfo
	{
		const SFeatureTest *                    m_calledTest;
		const SFeatureTestInstructionOrParam *  m_returnToHereWhenDone;
	};

	struct SStack
	{
		static i32k          k_stackSize = 3;
		i32                       m_count;
		SStackedTestCallInfo      m_info[k_stackSize];
	};

	struct SCurrentlyOverriddenInput
	{
		tukk  m_inputName;
		EActionActivationMode m_mode;
		bool m_sendHoldEventEveryFrame;
	};

	struct SListOfEntityClasses
	{
		static i32k k_max = 4;
		i32 m_num;
		IEntityClass * m_classPtr[k_max];
	};

	struct SListOfCheckpoints
	{
		static i32k k_max = 4;
		i32 m_num;
		tukk  m_checkpointName[k_max];
	};

	struct SAutoAimPosition
	{
		EntityId m_entityId;
		EBonesID m_boneId;
	};

	public:
	CFeatureTester();
	~CFeatureTester();
	static string GetContextString();

	ILINE static CFeatureTester * GetInstance()
	{
		return s_instance;
	}

	CFeatureTestMgr& GetMapFeatureTestMgr() { return *m_pFeatureTestMgr; }

	ILINE bool GetIsActive()
	{
		return m_currentTest || m_numFeatureTestsLeftToAutoRun || m_pFeatureTestMgr->IsRunning();
	}

	ILINE void InformAutoTesterOfResults(CAutoTester * autoTester)
	{
		m_informAutoTesterOfResults = autoTester;

		if(m_pFeatureTestMgr)
			m_pFeatureTestMgr->SetAutoTester(m_informAutoTesterOfResults);
	}

	ILINE CAutoTester * GetAutoTesterIfActive()
	{
		return m_informAutoTesterOfResults;
	}

	private:
	void Update(float dt);
	void PreProcessCommandNode(const IItemParamsNode * node);
	i32 PreprocessTestSet(const IItemParamsNode *testsListNode);
	bool ConvertSwitchNodeToInstructions(const IItemParamsNode * cmdParams, SFeatureTest & createTest, SFeatureTestDataLoadWorkspace * loadWorkspace);
	bool LoadCommands(const IItemParamsNode * node, SFeatureTest & createTest, SFeatureTestDataLoadWorkspace * loadWorkspace);
	bool ReadTestSet(const IItemParamsNode *testsListNode, SFeatureTestDataLoadWorkspace * loadWorkspace);
	TBitfield ReadPlayerSelection(const IItemParamsNode * paramsNode);
	void ReadSettings(const IItemParamsNode * topSettingsNode);
	void LoadTestData(tukk  filename);
	void UnloadTestData();
	bool AddInstructionAndParams(EFeatureTestCommand cmd, const IItemParamsNode * paramsNode, SFeatureTestDataLoadWorkspace * loadWorkspace);
	void AttemptStartTestWithTimeout(const SFeatureTest * test, tukk  actionName, i32 offsetIntoIterationList, float dt);
	tukk  StartTest(const SFeatureTest * test, tukk  actionName, float delay, i32 offsetIntoIterationList = 0);
	void InterruptCurrentTestIfOneIsRunning();
	void ListTestInstructions(const SFeatureTestInstructionOrParam * instruction, IFeatureTestTextHandler & textHandler) const;
	bool CompleteSubroutine();
	void DoneWithTest(const SFeatureTest * doneWithThisTest, tukk  actionName);
	void SendInputToLocalPlayer(tukk  inputName, EActionActivationMode mode, float value);
	const SFeatureTestInstructionOrParam * GetNextInstructionOrParam();
	SCheckpointCount * WatchCheckpoint(tukk  cpName);
	SCheckpointCount * FindWatchedCheckpointDataByName(tukk  name, i32 stackLevel);
	SFeatureTest * FindTestByName(tukk  name);
	bool FeatureTestFailureFunc(tukk  conditionTxt, tukk  messageTxt);
	CActor * GetPlayerForSelectionFlag(TBitfield flag);
	IActor * GetNthNonLocalActor(i32 skipThisManyBeforeReturning);
	void SetCheckpointHitResponse(SCheckpointCount * checkpoint, EFTCheckpointHitResponse response);
	void SetPauseStateAndTimeout(EFTPauseReason pauseReason, float timeOut);
	void RemoveWatchedCheckpointsAddedAtCurrentStackLevel(tukk  reason);
	void SubmitResultToAutoTester(const SFeatureTest * test, float timeTaken, tukk  failureMessage);
	void SubmitTestStartedMessageToAutoTester(const SFeatureTest * test);
	string GetListOfCheckpointsExpected();
	tukk  GetTextParam();
	bool IsActorAliveAndPlaying(IActor * actor);
	tukk  GetActorInfoString(IActor * iActor, DrxFixedStringT<256> & reply);
	void SendTextMessageToRemoteClients(tukk  msg);
	void DisplayCaption(const SFeatureTest * test);
	void InformWatchedCheckpointHit(SCheckpointCount * watchedCheckpoint);
	void InformCodeCoverageLabelHit(tukk  cpLabel);
	bool CheckPrerequisite(const SFeatureTest * test);
	void PollForNewCheckpointHits();
	void ClearLocalPlayerAutoAim();

	ILINE void PauseExecution(bool pause = true)
	{
		m_abortUntilNextFrame |= (pause == true);
	}

	// StopTest:
	// Pass in NULL to stop quietly, "" to count as a successful run or any other tuk ptr for a test failure
	void StopTest(tukk  failureMsg);

	// Static functions triggered by console commands...
	static void CmdStartTest(IConsoleCmdArgs *pArgs);
	static void CmdLoad(IConsoleCmdArgs *pArgs);
	static void CmdReload(IConsoleCmdArgs *pArgs);
	static void CmdRunAll(IConsoleCmdArgs *pArgs);

#if ENABLE_GAME_CODE_COVERAGE
	// IGameCodeCoverageListener
	void InformCodeCoverageCheckpointHit(CGameCodeCoverageCheckPoint * cp);
#endif

	// Instructions
	FeatureTestCommandList(DO_COMMAND_PROTOTYPE)

	// Static private data
	static CFeatureTester *                 s_instance;
	static InstructionFunc                  s_instructionFunctions[];

	// Non-static member vars
	CSingleAllocTextBlock                   m_singleAllocTextBlock;
	SCheckpointCount                        m_checkpointCountArray[kMaxWatchedCheckpoints];
	SCurrentlyOverriddenInput               m_currentlyOverriddenInputs[kMaxSimultaneouslyOverriddenInputs];
	SListOfEntityClasses                    m_listEntityClassesWhenFail;
	SListOfCheckpoints                      m_checkpointsWhichAlwaysFailATest;
	SAutoAimPosition                        m_localPlayerAutoAimSettings;
	UPausedInfo                             m_pausedInfo;
	SStack                                  m_runFeatureTestStack;
	string                                  m_currentlyLoadedFileName;

	const SFeatureTestInstructionOrParam *  m_currentTestNextInstruction;
	SFeatureTest *                          m_featureTestArray;
	const SFeatureTest *                    m_currentTest;
	const SFeatureTest *					m_haveJustWrittenMessageAboutUnfinishedTest;
	CAutoTester *                           m_informAutoTesterOfResults;
	SFeatureTestInstructionOrParam *        m_singleBufferContainingAllInstructions;
	EFTPauseReason                          m_pause_state;
	float                                   m_pause_timeLeft;
	float                                   m_pause_originalTimeOut;
	float                                   m_timeSinceCurrentTestBegan;
	i32                                     m_numWatchedCheckpoints;
	i32                                     m_numOverriddenInputs;
	i32                                     m_numTests;
	i32                                     m_numFeatureTestsLeftToAutoRun;
	i32                                     m_saveScreenshotWhenFail;
	bool                                    m_pause_enableCountdown;
	bool                                    m_abortUntilNextFrame;
	u8                                   m_waitUntilCCCPointHit_numStillToHit;

	struct
	{
		i32                                   m_numParams;
		i32                                   m_nextIterationCharOffset;
		char *                                m_currentParams[9];
	} m_iterateOverParams;

	struct
	{
		const SFeatureTest *                  m_test;
		i32                                   m_charOffset;
	} m_nextIteration;

	CFeatureTestMgr*												m_pFeatureTestMgr;
};

#endif

#endif //__FEATURETESTER_H__
