// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 13:05:2009   Created by Federico Rebora
*************************************************************************/

#pragma once

#ifndef FLOW_GRAPH_TESTING_H_INCLUDED
#define FLOW_GRAPH_TESTING_H_INCLUDED

#include <DrxFlowGraph/IFlowSystem.h>

DrxUnit::StringStream& operator << (DrxUnit::StringStream& stringStream, const SInputPortConfig& portConfig);

namespace GameTesting
{
	class CFlowNodeTestingFacility
	{
	public:
		CFlowNodeTestingFacility(IFlowNode& nodeToTest, u32k inputPortsCount);
		~CFlowNodeTestingFacility();

		void ProcessEvent(IFlowNode::EFlowEvent event);

		template <class InputType>
		void SetInputByIndex(u32k inputIndex, const InputType& value)
		{
			m_inputData[inputIndex].Set(value);
		}

	private:
		IFlowNode& m_nodeToTest;
		TFlowInputData* m_inputData;
	};
}

#endif //FLOW_GRAPH_TESTING_H_INCLUDED
