// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "GraphViewModel.h"

#include "GraphNodeItem.h"
#include "GraphConnectionItem.h"

#include "NodeGraphClipboard.h"

#include <NodeGraph/NodeGraphUndo.h>

#include <DrxSchematyc/Script/IScriptGraph.h>
#include <DrxSchematyc/Script/IScriptRegistry.h>

namespace DrxSchematycEditor {

CNodeGraphViewModel::CNodeGraphViewModel(Schematyc::IScriptGraph& scriptGraph /*, Schematyc::CNodeGraphRuntimeContext& context*/)
	: m_scriptGraph(scriptGraph)
	, m_runtimeContext(scriptGraph)
{
	{
		auto visitor = [this](Schematyc::IScriptGraphNode& scriptGraphNode) -> Schematyc::EVisitStatus
		{
			CNodeItem* pNodeItem = new CNodeItem(scriptGraphNode, *this);
			m_nodesByIndex.push_back(pNodeItem);
			m_nodesByGuid.emplace(pNodeItem->GetGUID(), pNodeItem);

			return Schematyc::EVisitStatus::Continue;
		};
		scriptGraph.VisitNodes(visitor);
	}

	{
		auto visitor = [this](Schematyc::IScriptGraphLink& scriptGraphLink) -> Schematyc::EVisitStatus
		{
			CNodeItem* pSrcNodeItem = m_nodesByGuid[scriptGraphLink.GetSrcNodeGUID()];
			CNodeItem* pDstNodeItem = m_nodesByGuid[scriptGraphLink.GetDstNodeGUID()];

			if (pSrcNodeItem && pDstNodeItem)
			{
				CPinItem* pSrcPinItem = pSrcNodeItem->GetPinItemById(CPinId(scriptGraphLink.GetSrcOutputId(), CPinId::EType::Output));
				CPinItem* pDstPinItem = pDstNodeItem->GetPinItemById(CPinId(scriptGraphLink.GetDstInputId(), CPinId::EType::Input));

				if (pSrcPinItem == nullptr || pSrcPinItem->IsInputPin() || pDstPinItem == nullptr || pDstPinItem->IsOutputPin())
					return Schematyc::EVisitStatus::Continue;

				if (pSrcNodeItem && pDstPinItem)
				{
					CConnectionItem* pConnectionItem = new CConnectionItem(scriptGraphLink, *pSrcPinItem, *pDstPinItem, *this);
					m_connectionsByIndex.push_back(pConnectionItem);
				}
			}

			return Schematyc::EVisitStatus::Continue;
		};
		scriptGraph.VisitLinks(visitor);
	}
}

CNodeGraphViewModel::~CNodeGraphViewModel()
{
	for (CConnectionItem* pConnectionitem : m_connectionsByIndex)
	{
		delete pConnectionitem;
	}

	for (CNodeItem* pNodeItem : m_nodesByIndex)
	{
		delete pNodeItem;
	}
}

u32 CNodeGraphViewModel::GetNodeItemCount() const
{
	return m_scriptGraph.GetNodeCount();
}

DrxGraphEditor::CAbstractNodeItem* CNodeGraphViewModel::GetNodeItemByIndex(u32 index) const
{
	if (index < m_nodesByIndex.size())
	{
		return m_nodesByIndex[index];
	}
	return nullptr;
}

DrxGraphEditor::CAbstractNodeItem* CNodeGraphViewModel::GetNodeItemById(QVariant id) const
{
	const DrxGUID guid = id.value<DrxGUID>();
	for (CNodeItem* pNodeItem : m_nodesByIndex)
	{
		if (pNodeItem->GetGUID() == guid)
			return pNodeItem;
	}

	return nullptr;
}

DrxGraphEditor::CAbstractNodeItem* CNodeGraphViewModel::CreateNode(QVariant typeId, const QPointF& position)
{
	Schematyc::IScriptGraphNodeCreationCommand* pCommand = reinterpret_cast<Schematyc::IScriptGraphNodeCreationCommand*>(typeId.value<quintptr>());
	if (pCommand)
	{
		Schematyc::IScriptGraphNodePtr pScriptNode = pCommand->Execute(Vec2(position.x(), position.y()));
		if (pScriptNode && m_scriptGraph.AddNode(pScriptNode))
		{
			// TODO: We shouldn't need to do this here.
			pScriptNode->ProcessEvent(Schematyc::SScriptEvent(Schematyc::EScriptEventId::EditorAdd));
			// ~TODO

			CNodeItem* pNodeItem = new CNodeItem(*pScriptNode, *this);
			m_nodesByIndex.push_back(pNodeItem);

			// TODO: Move this into a CNodeGraphViewModel method that gets called from here.
			SignalCreateNode(*pNodeItem);

			if (GetIEditor()->GetIUndoManager()->IsUndoRecording())
			{
				CUndo::Record(new DrxGraphEditor::CUndoNodeCreate(*pNodeItem));
			}
			// ~TODO

			return pNodeItem;
		}
	}
	else
	{
		DrxGraphEditor::CAbstractNodeItem* pNodeItem = CreateNode(typeId.value<Schematyc::DrxGUID>());
		if (pNodeItem)
		{
			pNodeItem->SetPosition(position);
			return pNodeItem;
		}
	}

	return nullptr;
}

bool CNodeGraphViewModel::RemoveNode(DrxGraphEditor::CAbstractNodeItem& node)
{
	CNodeItem* pNodeItem = static_cast<CNodeItem*>(&node);
	if (!pNodeItem->IsRemovable())
		return false;

	// TODO: Move this to NodeGraphView.
	const DrxGraphEditor::PinItemArray pins(pNodeItem->GetPinItems());
	for (DrxGraphEditor::CAbstractPinItem* pPin : pins)
	{
		DRX_ASSERT_MESSAGE(pPin, "Value of pPin must be not null.");
		if (pPin)
		{
			const DrxGraphEditor::ConnectionItemSet& connections = pPin->GetConnectionItems();
			for (auto itr = connections.begin(); itr != connections.end(); ++itr)
			{
				DrxGraphEditor::CAbstractConnectionItem* pConnection = *itr;
				RemoveConnection(*pConnection);
			}
		}
	}
	// ~TODO

	NodesByIndex::iterator result = std::find(m_nodesByIndex.begin(), m_nodesByIndex.end(), pNodeItem);
	if (result != m_nodesByIndex.end())
	{
		// TODO: Move this into a CNodeGraphViewModel method that gets called from here.
		if (GetIEditor()->GetIUndoManager()->IsUndoRecording())
		{
			CUndo::Record(new DrxGraphEditor::CUndoNodeRemove(node));
		}

		SignalRemoveNode(*pNodeItem);
		// ~TODO
		m_nodesByIndex.erase(result);
		m_scriptGraph.RemoveNode(pNodeItem->GetGUID());

		delete pNodeItem;

		gEnv->pSchematyc->GetScriptRegistry().ElementModified(m_scriptGraph.GetElement());
		return true;
	}

	m_scriptGraph.RemoveNode(pNodeItem->GetGUID());
	gEnv->pSchematyc->GetScriptRegistry().ElementModified(m_scriptGraph.GetElement());
	return false;
}

u32 CNodeGraphViewModel::GetConnectionItemCount() const
{
	return m_connectionsByIndex.size();
}

DrxGraphEditor::CAbstractConnectionItem* CNodeGraphViewModel::GetConnectionItemByIndex(u32 index) const
{
	if (index < m_connectionsByIndex.size())
	{
		return m_connectionsByIndex[index];
	}
	return nullptr;
}

DrxGraphEditor::CAbstractConnectionItem* CNodeGraphViewModel::GetConnectionItemById(QVariant id) const
{
	const Schematyc::IScriptGraphLink* pGraphLink = reinterpret_cast<Schematyc::IScriptGraphLink*>(id.value<quintptr>());
	for (CConnectionItem* pConnection : m_connectionsByIndex)
	{
		if (&pConnection->GetScriptLink() == pGraphLink)
		{
			return pConnection;
		}
	}
	return nullptr;
}

DrxGraphEditor::CAbstractConnectionItem* CNodeGraphViewModel::CreateConnection(DrxGraphEditor::CAbstractPinItem& sourcePin, DrxGraphEditor::CAbstractPinItem& targetPin)
{
	CPinItem& sourcePinItem = static_cast<CPinItem&>(sourcePin);
	CPinItem& targetPinItem = static_cast<CPinItem&>(targetPin);

	CNodeItem& sourceNode = static_cast<CNodeItem&>(sourcePinItem.GetNodeItem());
	CNodeItem& targetNode = static_cast<CNodeItem&>(targetPinItem.GetNodeItem());

	if (sourcePinItem.CanConnect(&targetPinItem))
	{
		Schematyc::IScriptGraphLink* pScriptLink = m_scriptGraph.AddLink(sourceNode.GetGUID(), sourcePinItem.GetPortId(), targetNode.GetGUID(), targetPinItem.GetPortId());
		if (pScriptLink)
		{
			CConnectionItem* pConnectionItem = new CConnectionItem(*pScriptLink, sourcePinItem, targetPinItem, *this);
			m_connectionsByIndex.push_back(pConnectionItem);

			gEnv->pSchematyc->GetScriptRegistry().ElementModified(m_scriptGraph.GetElement());

			// TODO: Move this into a CNodeGraphViewModel method that gets called from here.
			SignalCreateConnection(*pConnectionItem);

			if (GetIEditor()->GetIUndoManager()->IsUndoRecording())
			{
				CUndo::Record(new DrxGraphEditor::CUndoConnectionCreate(*pConnectionItem));
			}
			// ~TODO

			return pConnectionItem;
		}
	}

	return nullptr;
}

bool CNodeGraphViewModel::RemoveConnection(DrxGraphEditor::CAbstractConnectionItem& connection)
{
	CConnectionItem* pConnectionItem = static_cast<CConnectionItem*>(&connection);

	CPinItem& sourcePinItem = static_cast<CPinItem&>(connection.GetSourcePinItem());
	CPinItem& targetPinItem = static_cast<CPinItem&>(connection.GetTargetPinItem());

	CNodeItem& sourceNode = static_cast<CNodeItem&>(sourcePinItem.GetNodeItem());
	CNodeItem& targetNode = static_cast<CNodeItem&>(targetPinItem.GetNodeItem());

	u32k linkIndex = m_scriptGraph.FindLink(sourceNode.GetGUID(), sourcePinItem.GetPortId(), targetNode.GetGUID(), targetPinItem.GetPortId());
	DRX_ASSERT_MESSAGE(linkIndex != Schematyc::InvalidIdx, "View model and backend are out of sync!");
	if (linkIndex != Schematyc::InvalidIdx)
	{
		m_scriptGraph.RemoveLink(linkIndex);
		gEnv->pSchematyc->GetScriptRegistry().ElementModified(m_scriptGraph.GetElement());
	}

	const ConnectionsByIndex::iterator result = std::find(m_connectionsByIndex.begin(), m_connectionsByIndex.end(), &connection);
	DRX_ASSERT_MESSAGE(result != m_connectionsByIndex.end(), "Connection not found in model");
	if (result != m_connectionsByIndex.end())
	{
		// TODO: Move this into a CNodeGraphViewModel method that gets called from here.
		if (GetIEditor()->GetIUndoManager()->IsUndoRecording())
		{
			CUndo::Record(new DrxGraphEditor::CUndoConnectionRemove(connection));
		}

		SignalRemoveConnection(connection);
		// ~TODO

		m_connectionsByIndex.erase(result);
		delete pConnectionItem;
	}

	return true;
}

DrxGraphEditor::CItemCollection* CNodeGraphViewModel::CreateClipboardItemsCollection()
{
	return new CNodeGraphClipboard(*this);
}

CNodeItem* CNodeGraphViewModel::CreateNode(DrxGUID typeGuid)
{
	Schematyc::IScriptGraphNodePtr pScriptNode = m_scriptGraph.AddNode(typeGuid);
	if (pScriptNode)
	{
		// TODO: This should happen in backend!
		pScriptNode->ProcessEvent(Schematyc::SScriptEvent(Schematyc::EScriptEventId::EditorAdd));
		// ~TODO

		CNodeItem* pNodeItem = new CNodeItem(*pScriptNode, *this);
		m_nodesByIndex.push_back(pNodeItem);

		// TODO: Move this into a CNodeGraphViewModel method that gets called from here.
		SignalCreateNode(*pNodeItem);

		if (GetIEditor()->GetIUndoManager()->IsUndoRecording())
		{
			CUndo::Record(new DrxGraphEditor::CUndoNodeCreate(*pNodeItem));
		}
		// ~TODO
		return pNodeItem;
	}
	return nullptr;
}

void CNodeGraphViewModel::Refresh()
{
	for (CNodeItem* pNode : m_nodesByIndex)
	{
		pNode->Refresh();
	}
}

}

