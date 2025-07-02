// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph\AbstractNodeContentWidget.h>
#include <NodeGraph\PinWidget.h>

#include <QGraphicsWidget>
#include <QVector>
#include <QLabel>

class QGraphicsGridLayout;
class QGraphicsWidget;
struct SSubstanceOutput;

namespace DrxGraphEditor
{
	class CNodeWidget;
	class CPinWidget;

}
namespace EditorSubstance
{
	namespace OutputEditor
	{
		class OutputPreviewImage;

		class CSubstanceNodeContentWidget : public DrxGraphEditor::CAbstractNodeContentWidget
		{
			Q_OBJECT

		public:
			enum EOutputType
			{
				Standard,
				Virtual
			};

			CSubstanceNodeContentWidget(DrxGraphEditor::CNodeWidget& node, DrxGraphEditor::CNodeGraphView& view, const EOutputType outputType, SSubstanceOutput* output, bool showPreview);

			virtual void OnInputEvent(DrxGraphEditor::CNodeWidget* pSender, DrxGraphEditor::SMouseInputEventArgs& args) override;
			void Update();
			void SetPreviewImage(const QPixmap& pixmap);
		protected:
			virtual ~CSubstanceNodeContentWidget();

			// CAbstractNodeContentWidget
			virtual void DeleteLater() override;
			virtual void OnItemInvalidated() override;
			virtual void OnLayoutChanged() override;
			// ~CAbstractNodeContentWidget

			void AddPin(DrxGraphEditor::CPinWidget& pinWidget);
			void RemovePin(DrxGraphEditor::CPinWidget& pinWidget);

			void OnPinAdded(DrxGraphEditor::CAbstractPinItem& item);
			void OnPinRemoved(DrxGraphEditor::CAbstractPinItem& item);

		private:
			QGraphicsGridLayout* m_pLayout;
			u8                m_numLeftPins;
			u8                m_numRightPins;
			QLabel*				 m_pPreset;
			QLabel*				 m_pResolution;
			SSubstanceOutput*	 m_pOutput;
			EOutputType			 m_outputType;
			DrxGraphEditor::CPinWidget*          m_pLastEnteredPin;
			OutputPreviewImage*  m_pPreviewImage;
			bool				 m_showPreview;
		};

	}
}
