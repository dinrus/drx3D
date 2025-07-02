// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "GraphView.h"
#include "NodeGraph/AbstractNodeGraphViewModel.h"
#include "Nodes/SubstanceOutputNodeBase.h"
#include "GraphViewModel.h"
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>

namespace EditorSubstance
{
	namespace OutputEditor
	{


		CGraphView::CGraphView(DrxGraphEditor::CNodeGraphViewModel* p)
		{
			SetModel(p);
		}

		QWidget* CGraphView::CreatePropertiesWidget(DrxGraphEditor::GraphItemSet& selectedItems)
		{
			CGraphViewModel* model = static_cast<CGraphViewModel*>(GetModel());
			return new CSubstanceGraphViewPropertiesWidget(selectedItems, model->GetShowPreviews());
		}

		CSubstanceGraphViewPropertiesWidget::CSubstanceGraphViewPropertiesWidget(DrxGraphEditor::GraphItemSet& items, bool showPreview)
			: DrxGraphEditor::CNodeGraphItemPropertiesWidget(items)
		{
			if (showPreview && items.size() == 1)
			{
				LoadPreviewImage(**items.begin());
			}
		}

		CSubstanceGraphViewPropertiesWidget::CSubstanceGraphViewPropertiesWidget(DrxGraphEditor::CAbstractNodeGraphViewModelItem& item, bool showPreview)
			: DrxGraphEditor::CNodeGraphItemPropertiesWidget(item)
		{
			if (showPreview)
			{
				LoadPreviewImage(item);
			}
		}

		void CSubstanceGraphViewPropertiesWidget::LoadPreviewImage(const DrxGraphEditor::CAbstractNodeGraphViewModelItem& item)
		{
			i32k type = item.GetType();
			if (type == DrxGraphEditor::eItemType_Node)
			{
				QGroupBox* rgb = new QGroupBox("RGB");
				rgb->setLayout(new QVBoxLayout);
				QGroupBox* alpha = new QGroupBox("Alpha");
				alpha->setLayout(new QVBoxLayout);

				const CSubstanceOutputNodeBase* node = static_cast<const CSubstanceOutputNodeBase*>(&item);
				m_pictureRGB = new QLabel();
				m_pictureRGB->setPixmap(QPixmap::fromImage(node->GetPreviewImage().convertToFormat(QImage::Format_RGBX8888)));
				m_pictureAlpha = new QLabel();				
				m_pictureAlpha->setPixmap(QPixmap::fromImage(node->GetPreviewImage().alphaChannel()));
				rgb->layout()->addWidget(m_pictureRGB);
				alpha->layout()->addWidget(m_pictureAlpha);
				addWidget(rgb);
				addWidget(alpha);
			}
			
		}

	}
}

