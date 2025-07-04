// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "GraphViewModel.h"

#include <IEditor.h>
#include "EditorSubstanceManager.h"

#include "Nodes/SubstanceVirtualOutputNode.h"
#include "Nodes/SubstanceOriginalOutputNode.h"
#include "Items/SubstanceConnectionItem.h"
#include "SubstanceGraphRuntimeContext.h"

#include "Dialogs/QStringDialog.h"
#include "Controls/QuestionDialog.h"

#define NODE_VERTICAL_OFFSET 180
#define NODE_HORIZONTAL_OFFSET 400

namespace EditorSubstance
{
	namespace OutputEditor
	{


		CGraphViewModel::CGraphViewModel(const std::vector<SSubstanceOutput>& originalOutputs, const std::vector<SSubstanceOutput>& customOutputs, bool showPreviews)
			: m_pRuntimeContext(new CSubstanceOutputsGraphRuntimeContext())
			, m_showPreviews(showPreviews)
		{
			i32 pos = 0;
			for (const SSubstanceOutput& defaultOutput : originalOutputs)
			{
				CSubstanceOutputNodeBase* pNode = new COriginalOutputNode(defaultOutput, *this);
				pNode->MoveBy(0, pos++ * NODE_VERTICAL_OFFSET);
				u32k crc = GetNodeCRC(defaultOutput.name, pNode->GetNodeType());
				m_nodeItemByNameCrc.emplace(crc, pNode);
				m_nodes.push_back(pNode);
			}
			pos = 0;
			for (const SSubstanceOutput& virtualOutput : customOutputs)
			{
				CSubstanceOutputNodeBase* pNode = new CVirtualOutputNode(virtualOutput, *this);
				pNode->MoveBy(NODE_HORIZONTAL_OFFSET, pos++ * NODE_VERTICAL_OFFSET);
				u32k crc = GetNodeCRC(virtualOutput.name, pNode->GetNodeType());
				m_nodeItemByNameCrc.emplace(crc, pNode);
				m_nodes.push_back(pNode);

				i32 startIndex = 0;
				if (virtualOutput.channels[0].sourceId != -1
					&& (virtualOutput.channels[0].sourceId == virtualOutput.channels[1].sourceId)
					&& (virtualOutput.channels[0].sourceId == virtualOutput.channels[2].sourceId))
				{
					if ((virtualOutput.channels[0].channel == -1 || virtualOutput.channels[0].channel == 0)
						&& (virtualOutput.channels[1].channel == -1 || virtualOutput.channels[1].channel == 1)
						&& (virtualOutput.channels[2].channel == -1 || virtualOutput.channels[2].channel == 2))
					{
						// RGB mapping is the same, now we need to find out, if also A is applicable
						CSubstanceConnectionItem* pConnectionItem = nullptr;

						if ((virtualOutput.channels[0].sourceId == virtualOutput.channels[3].sourceId)
							&& (virtualOutput.channels[3].channel == -1 || virtualOutput.channels[3].channel == 3))
						{
							CSubstanceOutputNodeBase* targetNode = GetNodeItemById(virtualOutput.sourceOutputs[0] + CSubstanceOutputNodeBase::GetIdSuffix(eOutput));
							if (targetNode)
							{
								pConnectionItem = new CSubstanceConnectionItem(*static_cast<CSubstanceOutPinItem*>(targetNode->GetPinItemByIndex(PIN_INDEX_RGBA)), *static_cast<CSubstanceInPinItem*>(pNode->GetPinItemByIndex(PIN_INDEX_RGBA)), *this);
								startIndex = 4;
							}
						}
						else {
							CSubstanceOutputNodeBase* targetNode = GetNodeItemById(virtualOutput.sourceOutputs[0] + CSubstanceOutputNodeBase::GetIdSuffix(eOutput));
							if (targetNode)
							{
								startIndex = 3;
								pConnectionItem = new CSubstanceConnectionItem(*static_cast<CSubstanceOutPinItem*>(targetNode->GetPinItemByIndex(PIN_INDEX_RGB)), *static_cast<CSubstanceInPinItem*>(pNode->GetPinItemByIndex(PIN_INDEX_RGB)), *this);
							}
						}
						if (pConnectionItem)
						{
							m_connections.push_back(pConnectionItem);
						}
					}
				}
				for (size_t i = startIndex; i < 4; i++)
				{
					if (virtualOutput.channels[i].sourceId != -1)
					{
						CSubstanceOutputNodeBase* targetNode = GetNodeItemById(virtualOutput.sourceOutputs[virtualOutput.channels[i].sourceId] + CSubstanceOutputNodeBase::GetIdSuffix(eOutput));
						if (targetNode)
						{
							CSubstanceConnectionItem* pConnectionItem = new CSubstanceConnectionItem(*static_cast<CSubstanceOutPinItem*>(targetNode->GetPinItemByIndex((virtualOutput.channels[i].channel == -1 ? i : virtualOutput.channels[i].channel) + 2)),
								*static_cast<CSubstanceInPinItem*>(pNode->GetPinItemByIndex(i + 2)), *this);
							m_connections.push_back(pConnectionItem);
						}
					}
				}
				static_cast<CVirtualOutputNode*>(pNode)->UpdatePinState();
			}

		}

		DrxGraphEditor::INodeGraphRuntimeContext& CGraphViewModel::GetRuntimeContext()
		{
			return *(m_pRuntimeContext.get());
		}

		QString CGraphViewModel::GetGraphName()
		{
			return QtUtil::ToQString("CGraphViewModel::GetGraphName()");
		}

		u32 CGraphViewModel::GetNodeItemCount() const
		{
			return m_nodes.size();
		}

		DrxGraphEditor::CAbstractNodeItem* CGraphViewModel::GetNodeItemByIndex(u32 index) const
		{
			DRX_ASSERT(index < m_nodes.size());
			return m_nodes[index];
		}

		DrxGraphEditor::CAbstractNodeItem* CGraphViewModel::GetNodeItemById(QVariant id) const
		{
			return GetNodeItemById(QtUtil::ToString(id.value<QString>()));
		}

		CSubstanceOutputNodeBase* CGraphViewModel::GetNodeItemById(string id) const
		{
			u32k crc = GetNodeCRC(id);
			const auto result = m_nodeItemByNameCrc.find(crc);
			if (result != m_nodeItemByNameCrc.end())
			{
				return result->second;
			}
			return nullptr;
		}

		u32 CGraphViewModel::GetConnectionItemCount() const
		{
			return m_connections.size();
		}

		DrxGraphEditor::CAbstractConnectionItem* CGraphViewModel::GetConnectionItemByIndex(u32 index) const
		{
			DRX_ASSERT(index < m_connections.size());
			return m_connections[index];
		}

		DrxGraphEditor::CAbstractConnectionItem* CGraphViewModel::GetConnectionItemById(QVariant id) const
		{
			DRX_ASSERT("NotImplemented");
			return nullptr;
		}

		DrxGraphEditor::CAbstractConnectionItem* CGraphViewModel::CreateConnection(DrxGraphEditor::CAbstractPinItem& sourcePin, DrxGraphEditor::CAbstractPinItem& targetPin)
		{
			return CreateConnection(static_cast<CSubstanceOutPinItem&>(sourcePin), static_cast<CSubstanceInPinItem&>(targetPin));
		}

		CSubstanceConnectionItem* CGraphViewModel::CreateConnection(CSubstanceOutPinItem& sourcePin, CSubstanceInPinItem& targetPin)
		{
			if (sourcePin.CanConnect(&targetPin))
			{
				CSubstanceConnectionItem* pConnectionItem = new CSubstanceConnectionItem(sourcePin, targetPin, *this);
				m_connections.push_back(pConnectionItem);

				COriginalOutputNode& sourceNode = static_cast<COriginalOutputNode&>(sourcePin.GetNodeItem());
				CVirtualOutputNode& targetNode = static_cast<CVirtualOutputNode&>(targetPin.GetNodeItem());
				targetNode.UpdatePinState();
				// TODO: Move this into a CNodeGraphViewModel method that gets called from here.
				SignalCreateConnection(*pConnectionItem);

				//if (GetIEditor()->GetIUndoManager()->IsUndoRecording())
				//{
				//	CUndo::Record(new DrxGraphEditor::CUndoConnectionCreate(*pConnectionItem));
				//}
				//// ~TODO
				SignalOutputsChanged();
				return pConnectionItem;
			}

			return nullptr;
		}

		bool CGraphViewModel::RemoveConnection(DrxGraphEditor::CAbstractConnectionItem& connection)
		{
			auto result = std::find(m_connections.begin(), m_connections.end(), &connection);
			if (result != m_connections.end())
			{
				//if (GetIEditor()->GetIUndoManager()->IsUndoRecording())
				//{
				//	CUndo::Record(new DrxGraphEditor::CUndoConnectionRemove(connection));
				//}

				SignalRemoveConnection(connection);
				m_connections.erase(result);

				COriginalOutputNode& sourceNode = static_cast<COriginalOutputNode&>(connection.GetSourcePinItem().GetNodeItem());
				CVirtualOutputNode& targetNode = static_cast<CVirtualOutputNode&>(connection.GetTargetPinItem().GetNodeItem());



				CSubstanceConnectionItem* pConnection = static_cast<CSubstanceConnectionItem*>(&connection);
				delete pConnection;
				targetNode.UpdatePinState();
				SignalOutputsChanged();
				return true;
			}

			return false;
		}

		DrxGraphEditor::CAbstractNodeItem* CGraphViewModel::CreateNode(QVariant identifier, const QPointF& position /*= QPointF()*/)
		{
			const string nodeType = QtUtil::ToString(identifier.value<QString>());
			if (nodeType == "VirtualOutput")
			{
				return CreateNode(eInput, position);
			}
			else if (nodeType == "OriginalOutput")
			{
				return CreateNode(eOutput, position);
			}
			return nullptr;
		}

		CSubstanceOutputNodeBase* CGraphViewModel::CreateNode(ESubstanceGraphNodeType nodeType, const QPointF& position)
		{
			CSubstanceOutputNodeBase* pNodeItem = nullptr;

			SSubstanceOutput newOutput;
			string baseName(nodeType == eInput ? "graph_output" : "texture");
			
			i32 startIndex(1);
			newOutput.name = baseName;
			while (m_nodeItemByNameCrc.count(GetNodeCRC(newOutput.name, nodeType)) != 0)
			{
				newOutput.name = string().Format("%s_%02d", baseName, startIndex++);
				
				if (startIndex == 100)
				{
					startIndex = 1;
					baseName += "_";
				}
			}
			switch (nodeType)
			{

			case eInput:
			{				
				newOutput.preset = *CManager::Instance()->GetTexturePresetsForFile(newOutput.name).begin();
				pNodeItem = new CVirtualOutputNode(newOutput, *this);
			}
			break;
			default:
				pNodeItem = new COriginalOutputNode(newOutput, *this);
				break;
			}

			
			u32k crc = GetNodeCRC(pNodeItem->NameAsString(), pNodeItem->GetNodeType());
			m_nodes.push_back(pNodeItem);
			m_nodeItemByNameCrc.emplace(crc, pNodeItem);
			pNodeItem->SetPosition(position);
			SignalCreateNode(*pNodeItem);
			SignalOutputsChanged();
	



			// TODO: Move this into a CNodeGraphViewModel method that gets called from here.
			//if (GetIEditor()->GetIUndoManager()->IsUndoRecording())
			//{
			//	CUndo::Record(new DrxGraphEditor::CUndoNodeCreate(*pNodeItem));
			//}
			// ~TODO

			return pNodeItem;
		}

		bool CGraphViewModel::RemoveNode(DrxGraphEditor::CAbstractNodeItem& node)
		{
			CSubstanceOutputNodeBase* pNodeItem = &static_cast<CSubstanceOutputNodeBase&>(node);
			if (!pNodeItem->CanDelete())
				return false;

			for (DrxGraphEditor::CAbstractPinItem* pPin : pNodeItem->GetPinItems())
			{
				const DrxGraphEditor::ConnectionItemSet& connections = pPin->GetConnectionItems();
				while (connections.size())
				{
					RemoveConnection(**connections.begin());
				}
			}

			//if (GetIEditor()->GetIUndoManager()->IsUndoRecording())
			//{
			//	CUndo::Record(new DrxGraphEditor::CUndoNodeRemove(node));
			//}
			CVirtualOutputNode* pVirtualNodeItem = static_cast<CVirtualOutputNode*>(pNodeItem);
			SignalRemoveNode(node);

			const auto result = m_nodeItemByNameCrc.find(GetNodeCRC(pNodeItem->NameAsString(), pVirtualNodeItem->GetNodeType()));
			if (result != m_nodeItemByNameCrc.end())
			{
				m_nodes.erase(std::remove(m_nodes.begin(), m_nodes.end(), result->second), m_nodes.end());
				m_nodeItemByNameCrc.erase(result);
				delete pNodeItem;
				return true;
			}

			delete pNodeItem;
			SignalOutputsChanged();
			return false;
		}

		void CGraphViewModel::UpdateNodeCrc(tukk szOldName, tukk szNewName, const ESubstanceGraphNodeType& nodeType)
		{
			u32k oldCrc = GetNodeCRC(szOldName, nodeType);

			auto result = m_nodeItemByNameCrc.find(oldCrc);
			if (result != m_nodeItemByNameCrc.end())
			{
				CSubstanceOutputNodeBase* pNodeItem = result->second;
				m_nodeItemByNameCrc.erase(result);

				u32k newCrc = GetNodeCRC(szNewName, nodeType);
				m_nodeItemByNameCrc.emplace(newCrc, pNodeItem);
				return;
			}

			string message;
			message.Format("Couldn't find node '%s'", szOldName);
			DRX_ASSERT_MESSAGE(0, message);
		}

		std::vector<SSubstanceOutput*> CGraphViewModel::GetOutputs() const
		{
			std::vector<SSubstanceOutput*> result;
			for (auto node : m_nodes)
			{

				if (node->GetNodeType() == eInput)
				{
					CVirtualOutputNode* virtualNode = static_cast<CVirtualOutputNode*>(node);
					virtualNode->PropagateNetworkToOutput();
					result.push_back(virtualNode->GetOutput());
				}

			}
			return result;
		}

		u32k CGraphViewModel::GetNodeCRC(const string& name, const ESubstanceGraphNodeType& nodeType)
		{
		
			return GetNodeCRC(name + CSubstanceOutputNodeBase::GetIdSuffix(nodeType));
		}

		u32k CGraphViewModel::GetNodeCRC(const string& name)
		{
			string nameLow(name);
			nameLow.MakeLower();
			u32k crc = CCrc32::Compute(nameLow);
			return crc;
		}

		bool CGraphViewModel::IsNameUnique(const string& name, const ESubstanceGraphNodeType& nodeType)
		{
			return m_nodeItemByNameCrc.count(GetNodeCRC(name, nodeType)) == 0;
		}

	}
}
