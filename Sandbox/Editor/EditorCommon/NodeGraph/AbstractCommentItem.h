// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AbstractNodeGraphViewModelItem.h"

#include <drx3D/CoreX/Sandbox/DrxSignal.h>

namespace DrxGraphEditor {

class CNodeGraphViewModel;
class CNodeGraphView;
class CCommentWidget;

class EDITOR_COMMON_API CAbstractCommentItem : public CAbstractNodeGraphViewModelItem
{
public:
	enum : i32 { Type = eItemType_Comment };

public:
	CAbstractCommentItem(CNodeGraphViewModel& viewModel);
	virtual ~CAbstractCommentItem();

	// CAbstractNodeGraphViewModelItem
	virtual i32   GetType() const override     { return Type; }

	virtual QPointF GetPosition() const override { return m_position; }
	virtual void    SetPosition(QPointF position) override;
	// ~CAbstractNodeGraphViewModelItem

	virtual CCommentWidget* CreateWidget(CNodeGraphView& view) = 0;
	virtual QVariant        GetIdentifier() const = 0;

public:
	CDrxSignal<void()> SignalPositionChanged;

private:
	QPointF m_position;
};

}

