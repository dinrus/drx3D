// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/AbstractNodeGraphViewModel.h>

#include "NodeGraphRuntimeContext.h"

namespace Schematyc {

struct IScriptGraph;

}

namespace DrxSchematycEditor {

class CNodeGraphRuntimeContext;
class CNodeItem;
class CConnectionItem;

class CNodeGraphViewModel : public DrxGraphEditor::CNodeGraphViewModel
{
	typedef std::vector<CNodeItem*>                          NodesByIndex;
	typedef std::unordered_map<DrxGUID, CNodeItem*> NodesByGuid;
	typedef std::vector<CConnectionItem*>                    ConnectionsByIndex;

public:
	CNodeGraphViewModel(Schematyc::IScriptGraph& scriptGraph /*, CNodeGraphRuntimeContext& context*/);
	virtual ~CNodeGraphViewModel();

	virtual DrxGraphEditor::INodeGraphRuntimeContext& GetRuntimeContext() override { return m_runtimeContext; }
	virtual QString                                   GetGraphName() override      { return QString(); }

	virtual u32                                    GetNodeItemCount() const override;
	virtual DrxGraphEditor::CAbstractNodeItem*        GetNodeItemByIndex(u32 index) const override;
	virtual DrxGraphEditor::CAbstractNodeItem*        GetNodeItemById(QVariant id) const override;
	virtual DrxGraphEditor::CAbstractNodeItem*        CreateNode(QVariant typeId, const QPointF& position = QPointF()) override;
	virtual bool                                      RemoveNode(DrxGraphEditor::CAbstractNodeItem& node) override;

	virtual u32                                    GetConnectionItemCount() const override;
	virtual DrxGraphEditor::CAbstractConnectionItem*  GetConnectionItemByIndex(u32 index) const override;
	virtual DrxGraphEditor::CAbstractConnectionItem*  GetConnectionItemById(QVariant id) const override;
	virtual DrxGraphEditor::CAbstractConnectionItem*  CreateConnection(DrxGraphEditor::CAbstractPinItem& sourcePin, DrxGraphEditor::CAbstractPinItem& targetPin) override;
	virtual bool                                      RemoveConnection(DrxGraphEditor::CAbstractConnectionItem& connection) override;

	virtual DrxGraphEditor::CItemCollection*          CreateClipboardItemsCollection() override;

	Schematyc::IScriptGraph&                          GetScriptGraph() const { return m_scriptGraph; }

	virtual CNodeItem*                                CreateNode(DrxGUID typeGuid);
	void                                              Refresh();

private:
	Schematyc::IScriptGraph& m_scriptGraph;
	CNodeGraphRuntimeContext m_runtimeContext;

	NodesByIndex             m_nodesByIndex;
	NodesByGuid              m_nodesByGuid;
	ConnectionsByIndex       m_connectionsByIndex;
};

}

