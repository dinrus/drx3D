// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/AbstractPinItem.h>
namespace EditorSubstance
{
	namespace OutputEditor
	{

		enum EOutputPinType {
			eOutputRGBA = 1,
			eOutputRGB = 2,
			eOutputR = 4,
			eOutputG = 8,
			eOutputB = 16,
			eOutputA = 32,
			eIndividualOutputs = eOutputA | eOutputB | eOutputG | eOutputR,

		};

		static std::map<EOutputPinType, string> pinNameMap{
			{ eOutputRGBA, "RGBA" },
			{ eOutputRGB, "RGB" },
			{ eOutputR, "R" },
			{ eOutputG, "G" },
			{ eOutputB, "B" },
			{ eOutputA, "A" },
		};

		class CSubstanceBasePinItem : public DrxGraphEditor::CAbstractPinItem
		{
		public:
			CSubstanceBasePinItem(DrxGraphEditor::CAbstractNodeItem& nodeItem, EOutputPinType pinType);
			virtual DrxGraphEditor::CAbstractNodeItem& GetNodeItem() const override { return m_nodeItem; }
			DrxGraphEditor::CPinWidget* CreateWidget(DrxGraphEditor::CNodeWidget& nodeWidget, DrxGraphEditor::CNodeGraphView& view) override;

			virtual bool CanConnect(const CAbstractPinItem* pOtherPin) const override;

			virtual QString GetName() const override;
			virtual QString GetDescription() const override;

			virtual QString GetTypeName() const override;

			virtual QVariant GetId() const override;
			virtual bool HasId(QVariant id) const override;

		protected:
			DrxGraphEditor::CAbstractNodeItem& m_nodeItem;
			EOutputPinType m_pinType;
		};

	}
}
