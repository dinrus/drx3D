// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 21:05:2009: Created by Federico Rebora

*************************************************************************/

#include <drx3D/Game/Stdafx.h>

#include <drx3D/Game/GameFlowBaseNode.h>


class MockGameFlowNode : public CGameFlowBaseNode
{
public:
	MockGameFlowNode()
	: m_processActivateEventWasCalled(false)
	, m_activationInfoPassedToProcessActivateEvent(0)
	{

	}

	virtual void GetConfiguration(SFlowNodeConfig& configuration)
	{

	}

	virtual void GetMemoryStatistics(IDrxSizer* sizer)
	{

	}

	bool ProcessActivateEventHandlerWasCalled() const
	{
		return m_processActivateEventWasCalled;
	}

	SActivationInfo* GetActivationInfoPassedToActivateEvent() const
	{
		return m_activationInfoPassedToProcessActivateEvent;
	}

	virtual void ProcessActivateEvent(SActivationInfo* activationInfo)
	{
		m_processActivateEventWasCalled = true;
		m_activationInfoPassedToProcessActivateEvent = activationInfo;
	}

private:
	bool m_processActivateEventWasCalled;

	SActivationInfo* m_activationInfoPassedToProcessActivateEvent;
};

DRX_TEST_FIXTURE(GameFlowBaseNodeTestFixture, DrxUnit::ITestFixture, GameTesting::FlowGraphTestSuite)
{

};

DRX_TEST_WITH_FIXTURE(GameFlowBaseNode_TestProcessActivateEventInvokedOnActivateEvent, GameFlowBaseNodeTestFixture)
{
	MockGameFlowNode flowNode;

	flowNode.ProcessEvent(IFlowNode::eFE_Activate, 0);

	ASSERT_IS_TRUE(flowNode.ProcessActivateEventHandlerWasCalled());
}

bool Equals(IFlowNode::SActivationInfo* leftOperand, IFlowNode::SActivationInfo* rightOperand)
{
	return leftOperand == rightOperand;
}

DRX_TEST_WITH_FIXTURE(GameFlowBaseNode_TestProcessEventPropagatesCorrectActivationInfo, GameFlowBaseNodeTestFixture)
{
	MockGameFlowNode flowNode;

	IFlowNode::SActivationInfo activationInfo;

	flowNode.ProcessEvent(IFlowNode::eFE_Activate, &activationInfo);

	ASSERT_ARE_EQUAL(&activationInfo, flowNode.GetActivationInfoPassedToActivateEvent());
}


DRX_TEST_WITH_FIXTURE(GameFlowBaseNode_TestProcessActivateEventNotInvokedOnNonActivateEvents, GameFlowBaseNodeTestFixture)
{
	static u32k notActivateEventsCount = 11;
	const IFlowNode::EFlowEvent nonActivationEvents[notActivateEventsCount] =
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

	for (u32 eventIndex = 0; eventIndex < notActivateEventsCount; ++eventIndex)
	{
		MockGameFlowNode flowNode;

		flowNode.ProcessEvent(nonActivationEvents[eventIndex], 0);

		ASSERT_IS_FALSE(flowNode.ProcessActivateEventHandlerWasCalled());
	}
}

