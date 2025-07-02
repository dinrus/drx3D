// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "SubstanceOriginalOutputNode.h"
#include "OutputEditor/GraphViewModel.h"
#include "OutputEditor/Pins/SubstanceOutPinItem.h"
#include "NodeGraph/NodeWidget.h"
#include "NodeGraph/PinGridNodeContentWidget.h"
#include "OutputEditor/SubstanceNodeContentWidget.h"

namespace EditorSubstance
{
	namespace OutputEditor
	{

		COriginalOutputNode::COriginalOutputNode(const SSubstanceOutput& output, DrxGraphEditor::CNodeGraphViewModel& viewModel)
			: CSubstanceOutputNodeBase(output, viewModel)
		{
			SetAcceptsRenaming(true);
			m_outputType = CSubstanceNodeContentWidget::Standard;
			for each (auto var in pinNameMap)
			{
				m_pins.push_back(new CSubstanceOutPinItem(*this, var.first));
			}
		}

	}
}

