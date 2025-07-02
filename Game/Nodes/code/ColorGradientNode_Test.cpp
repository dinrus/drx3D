// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 12:05:2009   Created by Federico Rebora
*************************************************************************/

#include <drx3D/Game/Stdafx.h>

#include <drx3D/Game/ColorGradientNode.h>

#include <drx3D/Game/Testing/MockDrxSizer.h>
#include <drx3D/Game/Testing/FlowGraphTesting.h>

#include <drx3D/Game/EngineFacade/EngineFacade.h>
#include <drx3D/Game/EngineFacade/GameFacade.h>

DRX_TEST_FIXTURE(ColorGradientNodeTestFixture, DrxUnit::ITestFixture, GameTesting::FlowGraphTestSuite)
{
	class CMockGameFacade : public EngineFacade::CDummyGameFacade
	{
	public:
		void TriggerFadingColorChart(const string& filePath, const float fadeInTimeInSeconds)
		{
			m_lastTriggeredColorChartPath = filePath;
			m_lastTriggeredFadeTime = fadeInTimeInSeconds;
		}

		const string& GetLastTriggeredColorChartPath() const
		{
			return m_lastTriggeredColorChartPath;
		}

		float GetLastTriggeredFadeInTime() const
		{
			return m_lastTriggeredFadeTime;
		}

	private:
		string m_lastTriggeredColorChartPath;
		float m_lastTriggeredFadeTime;
	};

	ColorGradientNodeTestFixture()
	{
		m_environment = IGameEnvironment::Create(
			IEngineFacadePtr(new EngineFacade::CDummyEngineFacade),
			IGameFacadePtr(new CMockGameFacade));

		m_colorGradientNodeToTest.reset(new CFlowNode_ColorGradient(GetEnvironment(), 0));
	}

	void ConfigureAndProcessEvent(IFlowNode::EFlowEvent event, const string filePath, const float fadeInTime)
	{
		GameTesting::CFlowNodeTestingFacility flowNodeTester(*m_colorGradientNodeToTest, CFlowNode_ColorGradient::eInputPorts_Count);

		flowNodeTester.SetInputByIndex(CFlowNode_ColorGradient::eInputPorts_TexturePath, filePath);
		flowNodeTester.SetInputByIndex(CFlowNode_ColorGradient::eInputPorts_TransitionTime, fadeInTime);

		flowNodeTester.ProcessEvent(event);
	}

	IGameEnvironment& GetEnvironment() const
	{
		return* m_environment;
	}

	EngineFacade::CDummyEngineFacade& GetEngine() const
	{
		return static_cast<EngineFacade::CDummyEngineFacade&>(GetEnvironment().GetEngine());
	}

	CMockGameFacade& GetGame() const
	{
		return static_cast<CMockGameFacade&>(GetEnvironment().GetGame());
	}

	shared_ptr<CFlowNode_ColorGradient> m_colorGradientNodeToTest;

private:
	
	IGameEnvironmentPtr m_environment;

};

DRX_TEST_WITH_FIXTURE(TestColorGradientNode_MemoryStatisticsAreSetCorrectly, ColorGradientNodeTestFixture)
{
	CMockDrxSizer sizer;
	m_colorGradientNodeToTest->GetMemoryStatistics(&sizer);

	ASSERT_ARE_EQUAL(sizeof(CFlowNode_ColorGradient), sizer.GetTotalSize());
}

DRX_TEST_WITH_FIXTURE(TestColorGradientNode_IsSetInAdvancedCategory, ColorGradientNodeTestFixture)
{
	SFlowNodeConfig flowNodeConfiguration;
	m_colorGradientNodeToTest->GetConfiguration(flowNodeConfiguration);

	ASSERT_ARE_EQUAL(static_cast<u32>(EFLN_ADVANCED), flowNodeConfiguration.GetCategory());
}

DRX_TEST_WITH_FIXTURE(TestColorGradientNode_SetsNoOutputs, ColorGradientNodeTestFixture)
{
	SFlowNodeConfig flowNodeConfiguration;
	m_colorGradientNodeToTest->GetConfiguration(flowNodeConfiguration);

	const SOutputPortConfig* nullOutputPorts = 0;

	ASSERT_ARE_EQUAL(nullOutputPorts, flowNodeConfiguration.pOutputPorts);
}

DRX_TEST_WITH_FIXTURE(TestColorGradientNode_SetsNodeInputs, ColorGradientNodeTestFixture)
{
	SFlowNodeConfig flowNodeConfiguration;
	m_colorGradientNodeToTest->GetConfiguration(flowNodeConfiguration);

	const SInputPortConfig* inputPorts = CFlowNode_ColorGradient::inputPorts;
	ASSERT_ARE_EQUAL(inputPorts, flowNodeConfiguration.pInputPorts);
}


DRX_TEST_WITH_FIXTURE(TestColorGradientNode_SetsTriggerInput, ColorGradientNodeTestFixture)
{
	SFlowNodeConfig flowNodeConfiguration;
	m_colorGradientNodeToTest->GetConfiguration(flowNodeConfiguration);

	const SInputPortConfig inputToTest = InputPortConfig_Void("Trigger", _HELP(""));
	ASSERT_ARE_EQUAL(inputToTest, flowNodeConfiguration.pInputPorts[CFlowNode_ColorGradient::eInputPorts_Trigger]);
}


DRX_TEST_WITH_FIXTURE(TestColorGradientNode_SetsTexturePathInput, ColorGradientNodeTestFixture)
{
	SFlowNodeConfig flowNodeConfiguration;
	m_colorGradientNodeToTest->GetConfiguration(flowNodeConfiguration);

	const SInputPortConfig inputToTest = InputPortConfig<string>("TexturePath", _HELP(""));
	ASSERT_ARE_EQUAL(inputToTest, flowNodeConfiguration.pInputPorts[CFlowNode_ColorGradient::eInputPorts_TexturePath]);
}

DRX_TEST_WITH_FIXTURE(TestColorGradientNode_SetsTransitionTimeInput, ColorGradientNodeTestFixture)
{
	SFlowNodeConfig flowNodeConfiguration;
	m_colorGradientNodeToTest->GetConfiguration(flowNodeConfiguration);

	const SInputPortConfig inputToTest = InputPortConfig<float>("TransitionTime", _HELP(""));
	ASSERT_ARE_EQUAL(inputToTest, flowNodeConfiguration.pInputPorts[CFlowNode_ColorGradient::eInputPorts_TransitionTime]);
}

DRX_TEST_WITH_FIXTURE(TestColorGradientNode_InputsAreNullTerminated, ColorGradientNodeTestFixture)
{
	SFlowNodeConfig flowNodeConfiguration;
	m_colorGradientNodeToTest->GetConfiguration(flowNodeConfiguration);

	const SInputPortConfig inputToTest = {0};
	ASSERT_ARE_EQUAL(inputToTest, CFlowNode_ColorGradient::inputPorts[CFlowNode_ColorGradient::eInputPorts_Count]);
}

DRX_TEST_WITH_FIXTURE(TestColorGradientNode_ActivationLoadsCorrectColorChartFile1, ColorGradientNodeTestFixture)
{
	const string filePath("filePath1");
	ConfigureAndProcessEvent(IFlowNode::eFE_Activate, filePath, 0.0f);

	const string pathOfLastLoadedColorChart = GetGame().GetLastTriggeredColorChartPath();

	ASSERT_ARE_EQUAL(filePath, pathOfLastLoadedColorChart);
}

DRX_TEST_WITH_FIXTURE(TestColorGradientNode_ActivationLoadsCorrectColorChartFile2, ColorGradientNodeTestFixture)
{
	const string filePath("filePath2");
	ConfigureAndProcessEvent(IFlowNode::eFE_Activate, filePath, 0.0f);

	const string pathOfLastLoadedColorChart = GetGame().GetLastTriggeredColorChartPath();

	ASSERT_ARE_EQUAL(filePath, pathOfLastLoadedColorChart);
}

DRX_TEST_WITH_FIXTURE(TestColorGradientNode_ActivationSetsCorrectFadeInTime1, ColorGradientNodeTestFixture)
{
	const float expectedFadeInTime = 10.0f;

	const string filePath("");
	ConfigureAndProcessEvent(IFlowNode::eFE_Activate, filePath, expectedFadeInTime);

	const float resultedFadeInTime = GetGame().GetLastTriggeredFadeInTime();

	ASSERT_ARE_EQUAL(expectedFadeInTime, resultedFadeInTime);
}

DRX_TEST_WITH_FIXTURE(TestColorGradientNode_ActivationSetsCorrectFadeInTime2, ColorGradientNodeTestFixture)
{
	const float expectedFadeInTime = 5.0f;

	const string filePath("");
	ConfigureAndProcessEvent(IFlowNode::eFE_Activate, filePath, expectedFadeInTime);

	const float resultedFadeInTime = GetGame().GetLastTriggeredFadeInTime();

	ASSERT_ARE_EQUAL(expectedFadeInTime, resultedFadeInTime);
}


DRX_TEST_WITH_FIXTURE(TestColorGradientNode_OnlyActivationEventsSetUpColorCharts, ColorGradientNodeTestFixture)
{
	const string filePath("filePath");

	static u32k numberOfNonActivationEvents = 11;
	const IFlowNode::EFlowEvent nonActivationEvents[numberOfNonActivationEvents] =
	{
		IFlowNode::eFE_Update,
		IFlowNode::eFE_FinalActivate,
		IFlowNode::eFE_Initialize,
		IFlowNode::eFE_FinalInitialize,
		IFlowNode::eFE_SetEntityId,
		IFlowNode::eFE_Suspend,
		IFlowNode::eFE_Resume,
		IFlowNode::eFE_ConnectInputPort,
		IFlowNode::eFE_DisconnectInputPort,
		IFlowNode::eFE_ConnectOutputPort,
		IFlowNode::eFE_DisconnectOutputPort,
	};

	for (u32 eventIndex = 0; eventIndex < numberOfNonActivationEvents; ++eventIndex)
	{
		ConfigureAndProcessEvent(nonActivationEvents[eventIndex], filePath, 0.0f);

		ASSERT_ARE_EQUAL(string(""), GetGame().GetLastTriggeredColorChartPath());
	}
}

