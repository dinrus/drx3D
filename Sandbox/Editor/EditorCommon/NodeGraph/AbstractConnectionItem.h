// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AbstractNodeGraphViewModelItem.h"

#include <drx3D/CoreX/Sandbox/DrxSignal.h>

class QColor;

namespace DrxGraphEditor {

class CNodeGraphViewModel;
class CNodeGraphView;

class CAbstractPinItem;
class CConnectionWidget;

class EDITOR_COMMON_API CAbstractConnectionItem : public CAbstractNodeGraphViewModelItem
{
public:
	enum : i32 { Type = eItemType_Connection };

public:
	CAbstractConnectionItem(CNodeGraphViewModel& viewModel);
	virtual ~CAbstractConnectionItem();

	// CAbstractNodeGraphViewModelItem
	virtual i32 GetType() const override       { return Type; }

	virtual bool  IsDeactivated() const override { return m_isDeactivated; }
	virtual void  SetDeactivated(bool isDeactivated) override;
	// ~CAbstractNodeGraphViewModelItem

	virtual QVariant           GetId() const = 0;
	virtual bool               HasId(QVariant id) const = 0;

	virtual CConnectionWidget* CreateWidget(CNodeGraphView& view) = 0;
	virtual tukk        GetStyleId() const { return "Connection"; }

	virtual CAbstractPinItem& GetSourcePinItem() const = 0;
	virtual CAbstractPinItem& GetTargetPinItem() const = 0;

public:
	CDrxSignal<void()> SignalDeactivatedChanged;

private:
	bool m_isDeactivated : 1;
};

}

