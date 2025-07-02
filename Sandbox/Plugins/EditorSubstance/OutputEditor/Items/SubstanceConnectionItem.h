// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "NodeGraph/AbstractConnectionItem.h"
#include "OutputEditor/Pins/SubstanceOutPinItem.h"
#include "OutputEditor/Pins/SubstanceInPinItem.h"

namespace EditorSubstance
{
	namespace OutputEditor
	{

		class CSubstanceConnectionItem : public DrxGraphEditor::CAbstractConnectionItem
		{
		public:
			CSubstanceConnectionItem(CSubstanceOutPinItem& sourcePin, CSubstanceInPinItem& targetPin, DrxGraphEditor::CNodeGraphViewModel& viewModel);

			virtual ~CSubstanceConnectionItem();

			virtual QVariant GetId() const override
			{
				return QVariant::fromValue(m_id);
			}
			virtual bool HasId(QVariant id) const override
			{
				return (GetId().value<u32>() == id.value<u32>());
			}
			virtual DrxGraphEditor::CConnectionWidget* CreateWidget(DrxGraphEditor::CNodeGraphView& view) override;

			virtual CSubstanceOutPinItem& GetSourcePinItem() const override
			{
				return m_sourcePin;
			}
			virtual CSubstanceInPinItem& GetTargetPinItem() const override
			{
				return m_targetPin;
			}

			virtual tukk GetStyleId() const override
			{
				return "Connection::Substance";
			}
		private:
			CSubstanceOutPinItem& m_sourcePin;
			CSubstanceInPinItem& m_targetPin;

			u32        m_id;
		};

	}
}
