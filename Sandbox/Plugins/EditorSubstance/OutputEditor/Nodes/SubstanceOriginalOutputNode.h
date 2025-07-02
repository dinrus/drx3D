// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SubstanceOutputNodeBase.h"

namespace EditorSubstance
{
	namespace OutputEditor
	{


		class COriginalOutputNode : public CSubstanceOutputNodeBase
		{
		public:
			COriginalOutputNode(const SSubstanceOutput& output, DrxGraphEditor::CNodeGraphViewModel& viewModel);
			virtual bool CanDelete() const override { return true; }

			virtual tukk GetStyleId() const override
			{
				return "Node::Output";
			}

			virtual ESubstanceGraphNodeType GetNodeType() const override { return eOutput; }
		};

	}
}
