// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AbstractNodeGraphViewModelItem.h"

#include <drx3D/CoreX/Sandbox/DrxSignal.h>

class QString;
class QPixmap;

namespace DrxGraphEditor {

class CNodeGraphViewModel;

class CAbstractNodeItem;
class CAbstractConnectionItem;

class CNodeWidget;
class CPinWidget;
class CNodeGraphView;

typedef std::set<CAbstractConnectionItem*> ConnectionItemSet;

class EDITOR_COMMON_API CAbstractPinItem : public CAbstractNodeGraphViewModelItem
{
	friend class CAbstractNodeItem;

public:
	enum { Type = eItemType_Pin };

public:
	CAbstractPinItem(CNodeGraphViewModel& viewModel);
	virtual ~CAbstractPinItem();

	// CAbstractNodeGraphViewModelItem
	virtual i32 GetType() const override       { return Type; }

	virtual bool  IsDeactivated() const override { return m_isDeactivated; }
	virtual void  SetDeactivated(bool isDeactivated) override;
	// ~CAbstractNodeGraphViewModelItem

	virtual CPinWidget*              CreateWidget(CNodeWidget& nodeWidget, CNodeGraphView& view) { return nullptr; }
	virtual tukk              GetStyleId() const                                          { return "Pin"; }

	virtual CAbstractNodeItem&       GetNodeItem() const = 0;

	virtual QString                  GetName() const = 0;
	virtual QString                  GetDescription() const = 0;
	virtual QString                  GetTypeName() const = 0;

	virtual QVariant                 GetId() const = 0;
	virtual bool                     HasId(QVariant id) const = 0;

	virtual bool                     IsInputPin() const = 0;
	virtual bool                     IsOutputPin() const = 0;

	virtual bool                     CanConnect(const CAbstractPinItem* pOtherPin) const = 0;
	virtual bool                     IsConnected() const        { return (m_connections.size() > 0); }

	virtual const ConnectionItemSet& GetConnectionItems() const { return m_connections; };

	virtual void                     AddConnection(CAbstractConnectionItem& connection);
	virtual void                     RemoveConnection(CAbstractConnectionItem& connection);
	virtual void                     Disconnect();

	u32                           GetIndex() const;

public:
	CDrxSignal<void()> SignalDeactivatedChanged;

protected:
	DrxGraphEditor::ConnectionItemSet m_connections;

private:
	u32 m_index;
	bool   m_isDeactivated : 1;
};

}

