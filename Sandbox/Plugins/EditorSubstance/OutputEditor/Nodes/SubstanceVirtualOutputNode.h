// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SubstanceOutputNodeBase.h"
#include "SubstanceCommon.h"

namespace EditorSubstance
{
	namespace OutputEditor
	{

		class CSubstanceNodeContentWidget;
		class CSubstanceOutPinItem;
		class COriginalOutputNode;

		struct ConnectionInfo
		{
			CSubstanceOutPinItem* pin;
			COriginalOutputNode* node;
		};

		class CVirtualOutputNode : public CSubstanceOutputNodeBase
		{
		public:
			CVirtualOutputNode(const SSubstanceOutput& output, DrxGraphEditor::CNodeGraphViewModel& viewModel);
			virtual bool CanDelete() const override { return true; }

			virtual tukk GetStyleId() const override { return "Node::Input"; }

			SSubstanceOutput* GetOutput() { return &m_pOutput; }

			virtual void Serialize(Serialization::IArchive& ar);

			virtual void UpdatePinState();
			virtual void PropagateNetworkToOutput();

			virtual ESubstanceGraphNodeType GetNodeType() const override { return eInput; }
		protected:
			virtual ConnectionInfo GetPinConnectionInfo(i32 index);
			virtual void HandleIndividualConnection(i32 pinIndex);

		};


	}
}
