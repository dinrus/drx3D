// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AbstractNodeContentWidget.h"
#include "PinWidget.h"

#include <QGraphicsWidget>
#include <QVector>

class QGraphicsGridLayout;
class QGraphicsWidget;

namespace DrxGraphEditor {

class CNodeWidget;
class CPinWidget;

class EDITOR_COMMON_API CPinGridNodeContentWidget : public CAbstractNodeContentWidget
{
	Q_OBJECT

public:
	CPinGridNodeContentWidget(CNodeWidget& node, CNodeGraphView& view);

	virtual void OnInputEvent(CNodeWidget* pSender, SMouseInputEventArgs& args) override;

protected:
	virtual ~CPinGridNodeContentWidget();

	// CAbstractNodeContentWidget
	virtual void DeleteLater() override;
	virtual void OnItemInvalidated() override;
	virtual void OnLayoutChanged() override;
	// ~CAbstractNodeContentWidget

	void AddPin(CPinWidget& pinWidget);
	void RemovePin(CPinWidget& pinWidget);

	void OnPinAdded(CAbstractPinItem& item);
	void OnPinRemoved(CAbstractPinItem& item);

private:
	QGraphicsGridLayout* m_pLayout;
	u8                m_numLeftPins;
	u8                m_numRightPins;
	CPinWidget*          m_pLastEnteredPin;
};

}

