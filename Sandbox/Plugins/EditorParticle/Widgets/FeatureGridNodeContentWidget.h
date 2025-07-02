// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/AbstractNodeContentWidget.h>
#include <NodeGraph/NodeGraphViewGraphicsWidget.h>
#include <NodeGraph/NodeGraphViewStyle.h>

#include <QWidget>
#include <QVector>
#include <QGraphicsItemAnimation>
#include <QTimeLine>

class QGraphicsLinearLayout;
class QGraphicsItemAnimation;

namespace {

class CGraphViewWidget;
class CNodeWidget;
class CPinWidget;

}

namespace DrxParticleEditor {

class CFeaturePinWidget;
class CParentPinItem;
class CChildPinItem;
class CFeatureItem;

class CFeatureGridNodeContentWidget;

class CFeatureWidget : public DrxGraphEditor::CNodeGraphViewGraphicsWidget
{
public:
	enum : i32 { Type = DrxGraphEditor::eGraphViewWidgetType_UserType + 1 };

public:
	CFeatureWidget(CFeatureItem& feature, CFeatureGridNodeContentWidget& contentWidget, DrxGraphEditor::CNodeGraphView& view);

	// DrxGraphEditor::CNodeGraphViewGraphicsWidget
	virtual void                                             DeleteLater() override;
	virtual i32                                            GetType() const override { return Type; }
	virtual DrxGraphEditor::CAbstractNodeGraphViewModelItem* GetAbstractItem() const override;

	virtual void                                             OnItemInvalidated() override;

	virtual void                                             OnSelectionChanged(bool isSelected);
	virtual void                                             OnHighlightedChanged(bool isHighlighted);
	virtual void                                             OnDeactivatedChanged(bool isDeactivated);
	// ~DrxGraphEditor::CNodeGraphViewGraphicsWidget

	// QGraphicsWidget
	virtual void paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget) override;

	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* pEvent) override;
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* pEvent) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* pEvent) override;

	virtual void mousePressEvent(QGraphicsSceneMouseEvent* pEvent) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* pEvent) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* pEvent) override;

	virtual void moveEvent(QGraphicsSceneMoveEvent* pEvent) override;
	// ~QGraphicsWidget

	CFeatureItem&                  GetItem() const          { return m_feature; }
	DrxGraphEditor::CPinWidget*    GetPinWidget() const     { return m_pPin; }
	DrxGraphEditor::CNodeWidget&   GetNodeWidget() const;
	CFeatureGridNodeContentWidget& GetContentWidget() const { return m_contentWidget; }
	void                           UpdateStyle();

protected:
	virtual ~CFeatureWidget();

	void OnNodeDeactivatedChanged(bool isDeactivated);

private:
	CFeatureItem&                  m_feature;
	CFeatureGridNodeContentWidget& m_contentWidget;
	DrxGraphEditor::CPinWidget*    m_pPin;
	QGraphicsLinearLayout*         m_pLayout;
	QCheckBox*                     m_pEnabledCheckbox;
	QLabel*                        m_pTitle;
	QColor                         m_color;
};

class CFeatureSlotWidget : public QGraphicsWidget
{
public:
	CFeatureSlotWidget();
	CFeatureSlotWidget(const CFeatureSlotWidget& other);
	~CFeatureSlotWidget();

	void            InitFromFeature(CFeatureWidget& featureWidget);
	CFeatureWidget* GetFeature() const                         { return m_pFeatureWidget; }
	void            SetFeature(CFeatureWidget* pFeatureWidget) { MoveIntoSlot(nullptr);  m_pFeatureWidget = pFeatureWidget; }
	void            MoveIntoSlot(CFeatureWidget* pFeatureWidget);

	u32          GetIndex() const { return m_index; }

	// QGraphicsWidget
	virtual QRectF boundingRect() const override;
	virtual void   updateGeometry() override;
	virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint) const override;
	// ~QGraphicsWidget

private:
	CFeatureWidget*         m_pFeatureWidget;

	QGraphicsItemAnimation* m_pAnimation;
	QTimeLine               m_timeLine;

	QRectF                  m_geometry;
	QRectF                  m_boundingRect;
	u32                  m_index;
};

class CFeatureGridNodeContentWidget : public DrxGraphEditor::CAbstractNodeContentWidget
{
public:
	typedef std::map<CFeatureItem*, CFeatureWidget*> FeaturesWidgetsByItems;

public:
	CFeatureGridNodeContentWidget(DrxGraphEditor::CNodeWidget& node, CParentPinItem& parentPin, CChildPinItem& childPin, DrxGraphEditor::CNodeGraphView& view);
	virtual ~CFeatureGridNodeContentWidget();

	void                          BeginFeatureMove(CFeatureWidget& featureWidget);
	void                          MoveFeature(const QPointF& delta);
	u32                        EndFeatureMove();

	const FeaturesWidgetsByItems& GetFeatureWidgetsMap() const { return m_featureWidgetsByItems; }

protected:
	virtual void OnItemInvalidated() override;

	void         OnFeatureAdded(CFeatureItem& feature);
	void         OnFeatureRemoved(CFeatureItem& feature);
	void         OnFeatureMoved(CFeatureItem& feature);

private:
	QGraphicsLinearLayout*      m_pFeaturesLayout;
	DrxGraphEditor::CPinWidget* m_pParentPin;
	DrxGraphEditor::CPinWidget* m_pChildPin;
	FeaturesWidgetsByItems      m_featureWidgetsByItems;

	QVector<CFeatureSlotWidget> m_featureSlots;
	QVector<CFeatureWidget*>    m_floatingFeatures;
	CFeatureWidget*             m_pGrabbedFeature;
	u32                      m_grabbedFeatureIndex;
};

}

