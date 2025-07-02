// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/NodeGraphView.h>
#include <NodeGraph/NodeGraphItemPropertiesWidget.h>

namespace DrxGraphEditor
{

class CNodeGraphViewStyle;
class CNodeGraphViewBackground;

}
class QLabel;
namespace EditorSubstance
{
	namespace OutputEditor
	{

		class CSubstanceGraphViewPropertiesWidget : public DrxGraphEditor::CNodeGraphItemPropertiesWidget
		{
			Q_OBJECT
		public:
			CSubstanceGraphViewPropertiesWidget(DrxGraphEditor::GraphItemSet& items, bool showPreview);
			CSubstanceGraphViewPropertiesWidget(DrxGraphEditor::CAbstractNodeGraphViewModelItem& item, bool showPreview);
		protected:
			void LoadPreviewImage(const DrxGraphEditor::CAbstractNodeGraphViewModelItem& item);
		private:
			QLabel* m_pictureRGB;
			QLabel* m_pictureAlpha;
		};

		class CGraphView : public DrxGraphEditor::CNodeGraphView
		{
		public:
			CGraphView(DrxGraphEditor::CNodeGraphViewModel* pViewModel);

			virtual QWidget* CreatePropertiesWidget(DrxGraphEditor::GraphItemSet& selectedItems) override;


		private:
		};
	}
}
