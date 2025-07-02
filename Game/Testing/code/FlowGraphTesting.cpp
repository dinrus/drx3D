// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 06:06:2009   Created by Federico Rebora
*************************************************************************/

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Game/FlowGraphTesting.h>

DrxUnit::StringStream& operator << (DrxUnit::StringStream& stringStream, const SInputPortConfig& portConfig)
{
    stringStream << portConfig.name << ":" << portConfig.humanName;
    return stringStream;
}

namespace GameTesting
{
	CFlowNodeTestingFacility::CFlowNodeTestingFacility( IFlowNode& nodeToTest, u32k inputPortsCount ) : m_nodeToTest(nodeToTest)
		, m_inputData(0)
	{
		DRX_ASSERT(inputPortsCount > 0);

		SFlowNodeConfig flowNodeConfiguration;
		nodeToTest.GetConfiguration(flowNodeConfiguration);

		m_inputData = new TFlowInputData[inputPortsCount];
		for (u32 inputIndex = 0; inputIndex < inputPortsCount; ++inputIndex)
		{
			const SInputPortConfig& inputPort = flowNodeConfiguration.pInputPorts[inputIndex];
			const TFlowInputData& defaultData = inputPort.defaultData;

			m_inputData[inputIndex] = defaultData;
		}
	}

	CFlowNodeTestingFacility::~CFlowNodeTestingFacility()
	{
		delete[] m_inputData;
		m_inputData = 0;
	}

	void CFlowNodeTestingFacility::ProcessEvent( IFlowNode::EFlowEvent event )
	{
		IFlowNode::SActivationInfo activationInformation(0, 0, 0, m_inputData);
		m_nodeToTest.ProcessEvent(event, &activationInformation);
	}
}