// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "SubstanceNodeContentWidget.h"

#include <NodeGraph\NodeGraphView.h>
#include <NodeGraph\AbstractNodeGraphViewModelItem.h>
#include <NodeGraph\NodeWidget.h>

#include <QGraphicsGridLayout>
#include <QComboBox>

#include "SubstanceCommon.h"

namespace EditorSubstance
{
	namespace OutputEditor
	{
		class OutputPreviewImage : public QGraphicsLayoutItem, public QGraphicsPixmapItem
		{
		public:
			OutputPreviewImage(QGraphicsItem *parent = 0)
				: QGraphicsLayoutItem(), QGraphicsPixmapItem(parent)
			{

			}

			void setGeometry(const QRectF &geom) override
			{
				prepareGeometryChange();
				QGraphicsLayoutItem::setGeometry(geom);
				setPos(geom.topLeft());
			}

			QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const override
			{
				return constraint;
			}

			void SetPreviewImage(const QPixmap& pixmap)
			{
				setPixmap(pixmap);
			}
		};

		CSubstanceNodeContentWidget::CSubstanceNodeContentWidget(DrxGraphEditor::CNodeWidget& node, DrxGraphEditor::CNodeGraphView& view, const CSubstanceNodeContentWidget::EOutputType outputType, SSubstanceOutput* output, bool showPreview)
			: CAbstractNodeContentWidget(node, view)
			, m_numLeftPins(0)
			, m_numRightPins(0)
			, m_pLastEnteredPin(nullptr)
			, m_pOutput(output)
			, m_outputType(outputType)
			, m_showPreview(showPreview)
		{
			node.SetContentWidget(this);

			m_pLayout = new QGraphicsGridLayout(this);
			m_pLayout->setColumnAlignment(0, Qt::AlignLeft);
			m_pLayout->setColumnAlignment(1, Qt::AlignRight);
			m_pLayout->setVerticalSpacing(2.f);
			m_pLayout->setHorizontalSpacing(15.f);
			m_pLayout->setContentsMargins(5.0f, 5.0f, 5.0f, 5.0f);
			if (m_outputType == Virtual)
			{
				m_pLayout->setColumnMaximumWidth(0, 15);

			}
			setLayout(m_pLayout);

			if (m_outputType == Virtual)
			{
				m_pPreset = new QLabel();
				m_pResolution = new QLabel();
				Update();
				QGraphicsProxyWidget* presetsProxy = new QGraphicsProxyWidget(this);
				QGraphicsProxyWidget* resolutionProxy = new QGraphicsProxyWidget(this);
				presetsProxy->setWidget(m_pPreset);
				resolutionProxy->setWidget(m_pResolution);
				m_pLayout->addItem(presetsProxy, 0, 0, 1, 2);
				m_pLayout->addItem(resolutionProxy, 1, 0, 1, 2);

			}
			UpdateLayout(m_pLayout);

			DrxGraphEditor::CAbstractNodeItem& nodeItem = node.GetItem();
			nodeItem.SignalPinAdded.Connect(this, &CSubstanceNodeContentWidget::OnPinAdded);
			nodeItem.SignalPinRemoved.Connect(this, &CSubstanceNodeContentWidget::OnPinRemoved);
			nodeItem.SignalInvalidated.Connect(this, &CSubstanceNodeContentWidget::OnItemInvalidated);

			for (DrxGraphEditor::CAbstractPinItem* pPinItem : node.GetItem().GetPinItems())
			{
				DrxGraphEditor::CPinWidget* pPinWidget = pPinItem->CreateWidget(node, GetView());
				AddPin(*pPinWidget);
			}

			if (showPreview)
			{
				m_pPreviewImage = new OutputPreviewImage(this);
				m_pLayout->addItem(m_pPreviewImage, m_outputType == Virtual ? 2 : 0, m_outputType == Virtual ? 1 : 0, m_outputType != Virtual ? m_numRightPins : m_numLeftPins, 1, m_outputType == Virtual ? Qt::AlignLeft : Qt::AlignLeft);
			}


		}

		CSubstanceNodeContentWidget::~CSubstanceNodeContentWidget()
		{

		}

		void CSubstanceNodeContentWidget::OnInputEvent(DrxGraphEditor::CNodeWidget* pSender, DrxGraphEditor::SMouseInputEventArgs& args)
		{
			// TODO: In here we should map all positions we forward to the pins.

			const DrxGraphEditor::EMouseEventReason orgReason = args.GetReason();

			DrxGraphEditor::CPinWidget* pHitPinWidget = nullptr;
			if (orgReason != DrxGraphEditor::EMouseEventReason::HoverLeave)
			{
				for (DrxGraphEditor::CPinWidget* pPinWidget : m_pins)
				{
					const QPointF localPoint = pPinWidget->mapFromScene(args.GetScenePos());
					const bool containsPoint = pPinWidget->GetRect().contains(localPoint);
					if (containsPoint)
					{
						pHitPinWidget = pPinWidget;
						break;
					}
				}
			}

			if (m_pLastEnteredPin != nullptr && (m_pLastEnteredPin != pHitPinWidget || pHitPinWidget == nullptr))
			{
				const DrxGraphEditor::EMouseEventReason originalReason = args.GetReason();
				if (originalReason == DrxGraphEditor::EMouseEventReason::HoverLeave || originalReason == DrxGraphEditor::EMouseEventReason::HoverMove)
				{
					// TODO: Should we send the events through the actual pin item instead?
					DrxGraphEditor::SMouseInputEventArgs mouseArgs = DrxGraphEditor::SMouseInputEventArgs(
						DrxGraphEditor::EMouseEventReason::HoverLeave,
						Qt::MouseButton::NoButton, Qt::MouseButton::NoButton, args.GetModifiers(),
						args.GetLocalPos(), args.GetScenePos(), args.GetScreenPos(),
						args.GetLastLocalPos(), args.GetLastScenePos(), args.GetLastScreenPos());

					GetView().OnPinMouseEvent(m_pLastEnteredPin, DrxGraphEditor::SPinMouseEventArgs(mouseArgs));
					// ~TODO

					m_pLastEnteredPin = nullptr;

					args.SetAccepted(true);
					return;
				}
			}

			if (pHitPinWidget)
			{
				DrxGraphEditor::EMouseEventReason reason = orgReason;
				if (m_pLastEnteredPin == nullptr)
				{
					m_pLastEnteredPin = pHitPinWidget;
					if (reason == DrxGraphEditor::EMouseEventReason::HoverMove)
					{
						reason = DrxGraphEditor::EMouseEventReason::HoverEnter;
					}
				}

				// TODO: Should we send the events through the actual pin item instead?
				DrxGraphEditor::SMouseInputEventArgs mouseArgs = DrxGraphEditor::SMouseInputEventArgs(
					reason,
					args.GetButton(), args.GetButtons(), args.GetModifiers(),
					args.GetLocalPos(), args.GetScenePos(), args.GetScreenPos(),
					args.GetLastLocalPos(), args.GetLastScenePos(), args.GetLastScreenPos());

				GetView().OnPinMouseEvent(pHitPinWidget, DrxGraphEditor::SPinMouseEventArgs(mouseArgs));
				// ~TODO

				args.SetAccepted(true);
				return;
			}

			args.SetAccepted(false);
			return;
		}

		void CSubstanceNodeContentWidget::DeleteLater()
		{
			DrxGraphEditor::CAbstractNodeItem& nodeItem = GetNode().GetItem();
			nodeItem.SignalPinAdded.DisconnectObject(this);
			nodeItem.SignalPinRemoved.DisconnectObject(this);

			for (DrxGraphEditor::CPinWidget* pPinWidget : m_pins)
				pPinWidget->DeleteLater();

			CAbstractNodeContentWidget::DeleteLater();
		}

		void CSubstanceNodeContentWidget::OnItemInvalidated()
		{
			DrxGraphEditor::PinItemArray pinItems;
			pinItems.reserve(pinItems.size());

			size_t numKeptPins = 0;

			DrxGraphEditor::PinWidgetArray pinWidgets;
			pinWidgets.swap(m_pins);
			for (DrxGraphEditor::CPinWidget* pPinWidget : pinWidgets)
			{
				m_pLayout->removeItem(pPinWidget);
			}

			m_numLeftPins = m_numRightPins = 0;
			for (DrxGraphEditor::CAbstractPinItem* pPinItem : GetNode().GetItem().GetPinItems())
			{
				QVariant pinId = pPinItem->GetId();
				auto predicate = [pinId](DrxGraphEditor::CPinWidget* pPinWidget) -> bool
				{
					return (pPinWidget && pPinWidget->GetItem().HasId(pinId));
				};

				auto result = std::find_if(pinWidgets.begin(), pinWidgets.end(), predicate);
				if (result == m_pins.end())
				{
					DrxGraphEditor::CPinWidget* pPinWidget = new DrxGraphEditor::CPinWidget(*pPinItem, GetNode(), GetView(), true);
					AddPin(*pPinWidget);
				}
				else
				{
					DrxGraphEditor::CPinWidget* pPinWidget = *result;
					AddPin(*pPinWidget);

					*result = nullptr;
					++numKeptPins;
				}
			}

			for (DrxGraphEditor::CPinWidget* pPinWidget : pinWidgets)
			{
				if (pPinWidget != nullptr)
				{
					RemovePin(*pPinWidget);
					pPinWidget->GetItem().SignalInvalidated.DisconnectObject(this);
					pPinWidget->DeleteLater();
				}
			}

			UpdateLayout(m_pLayout);
		}

		void CSubstanceNodeContentWidget::OnLayoutChanged()
		{
			UpdateLayout(m_pLayout);
		}

		void CSubstanceNodeContentWidget::AddPin(DrxGraphEditor::CPinWidget& pinWidget)
		{
			i32 addOffset = 0;
			if (m_outputType == Virtual)
			{
				addOffset = 2;
			}
			DrxGraphEditor::CAbstractPinItem& pinItem = pinWidget.GetItem();
			if (pinItem.IsInputPin())
			{
				m_pLayout->addItem(&pinWidget, addOffset + m_numLeftPins++, 0);
			}
			else
			{
				m_pLayout->addItem(&pinWidget, addOffset + m_numRightPins++, 1);
			}

			m_pins.push_back(&pinWidget);
		}

		void CSubstanceNodeContentWidget::RemovePin(DrxGraphEditor::CPinWidget& pinWidget)
		{
			DrxGraphEditor::CAbstractPinItem& pinItem = pinWidget.GetItem();
			if (pinItem.IsInputPin())
				--m_numLeftPins;
			else
				--m_numRightPins;

			m_pLayout->removeItem(&pinWidget);
			pinWidget.DeleteLater();

			auto result = std::find(m_pins.begin(), m_pins.end(), &pinWidget);
			DRX_ASSERT(result != m_pins.end());
			if (result != m_pins.end())
				m_pins.erase(result);
		}

		void CSubstanceNodeContentWidget::OnPinAdded(DrxGraphEditor::CAbstractPinItem& item)
		{
			DrxGraphEditor::CPinWidget* pPinWidget = item.CreateWidget(GetNode(), GetView());
			AddPin(*pPinWidget);

			UpdateLayout(m_pLayout);
		}

		void CSubstanceNodeContentWidget::OnPinRemoved(DrxGraphEditor::CAbstractPinItem& item)
		{
			QVariant pinId = item.GetId();

			auto condition = [pinId](DrxGraphEditor::CPinWidget* pPinWidget) -> bool
			{
				return (pPinWidget && pPinWidget->GetItem().HasId(pinId));
			};

			const auto result = std::find_if(m_pins.begin(), m_pins.end(), condition);
			if (result != m_pins.end())
			{
				DrxGraphEditor::CPinWidget* pPinWidget = *result;
				m_pLayout->removeItem(pPinWidget);
				pPinWidget->DeleteLater();

				if (item.IsInputPin())
					--m_numLeftPins;
				else
					--m_numRightPins;

				m_pins.erase(result);
			}

			UpdateLayout(m_pLayout);
		}

		void CSubstanceNodeContentWidget::Update()
		{
			if (m_outputType == Virtual)
			{
				m_pPreset->setText(m_pOutput->preset.c_str());
				for each (auto& var in resolutionNamesMap)
				{
					if (var.second == m_pOutput->resolution)
					{
						m_pResolution->setText(var.first.c_str());
						break;
					}
				}

			}

		}

		void CSubstanceNodeContentWidget::SetPreviewImage(const QPixmap& pixmap)
		{
			if (m_showPreview)
			{
				m_pPreviewImage->SetPreviewImage(pixmap);
			}
		}

	}
}
