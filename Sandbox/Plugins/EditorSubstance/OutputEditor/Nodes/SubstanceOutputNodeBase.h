// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <NodeGraph/AbstractNodeItem.h>
#include "SubstanceCommon.h"
#include "OutputEditor/SubstanceNodeContentWidget.h"

#define PIN_INDEX_RGBA 0
#define PIN_INDEX_RGB 1
#define PIN_INDEX_R 2
#define PIN_INDEX_G 3
#define PIN_INDEX_B 4
#define PIN_INDEX_A 5


namespace EditorSubstance
{
	namespace OutputEditor
	{

		enum ESubstanceGraphNodeType
		{
			eOutput,
			eInput,
		};

		class CSubstanceOutputNodeBase : public DrxGraphEditor::CAbstractNodeItem
		{
		public:
			CSubstanceOutputNodeBase(const SSubstanceOutput& output, DrxGraphEditor::CNodeGraphViewModel& viewModel);
			virtual ~CSubstanceOutputNodeBase();

			virtual QVariant GetId() const override;
			virtual bool HasId(QVariant id) const override;
			virtual QVariant GetTypeId() const override;
			virtual const DrxGraphEditor::PinItemArray& GetPinItems() const override;

			string NameAsString() const;
			virtual bool CanDelete() const = 0;
			virtual ESubstanceGraphNodeType GetNodeType() const = 0;
			virtual DrxGraphEditor::CNodeWidget* CreateWidget(DrxGraphEditor::CNodeGraphView& view) override;
			void SetPreviewImage(std::shared_ptr<QImage> image);
			void ResetPreviewImage();
			const QImage& GetPreviewImage() const;
			static const string GetIdSuffix(const ESubstanceGraphNodeType& nodeType);

			virtual void SetName(const QString& name) override;

		protected:

		protected:
			CSubstanceNodeContentWidget::EOutputType m_outputType;
			SSubstanceOutput m_pOutput;
			std::shared_ptr<QImage> m_originalImage;
			CSubstanceNodeContentWidget* m_pNodeContentWidget;
			DrxGraphEditor::PinItemArray m_pins;
		};

	}
}
