// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ParticleGraphModel.h"
#include "ParticleGraphItems.h"

#include "Models/ClipboardItems.h"

#include "Widgets/FeatureGridNodeContentWidget.h"

#include <DrxIcon.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/STL.h>

#include <NodeGraph/NodeWidget.h>
#include <NodeGraph/NodeGraphUndo.h>

#include <QVariant>

// TODO: Replace when CNodeStyle was moved into its own header.
#include "NodeGraph/NodeWidgetStyle.h"
#include "NodeGraph/NodeHeaderWidgetStyle.h"
#include "NodeGraph/NodeGraphViewStyle.h"
#include "NodeGraph/ConnectionWidgetStyle.h"
#include "NodeGraph/NodePinWidgetStyle.h"

#include <QtUtil.h>
// ~TODO

namespace DrxParticleEditor {

CParticleGraphModel::CParticleGraphModel(pfx2::IParticleEffectPfx2& effect)
	: m_effect(effect)
	, m_isValid(true)
	, m_pSolorNode(nullptr)
{
	u32 numVisibleNodes = 0;
	CNodeItem* pSoloNode = nullptr;

	u32k nodeCount = m_effect.GetNumComponents();
	if (nodeCount)
	{
		for (u32 i = 0; i < nodeCount; ++i)
		{
			pfx2::IParticleComponent* pComponent = m_effect.GetComponent(i);
			CNodeItem* pNodeItem = new CNodeItem(*pComponent, *this);
			m_nodes.push_back(pNodeItem);

			if (pNodeItem->IsVisible())
			{
				++numVisibleNodes;
				if (pSoloNode == nullptr)
					pSoloNode = pNodeItem;
			}
		}
	}

	if (numVisibleNodes == 1 && nodeCount > 1)
	{
		ToggleSoloNode(*pSoloNode);
	}

	ExtractConnectionsFromNodes();
}

CParticleGraphModel::~CParticleGraphModel()
{
	for (CConnectionItem* pConnectionitem : m_connections)
	{
		delete pConnectionitem;
	}

	for (CNodeItem* pNodeItem : m_nodes)
	{
		delete pNodeItem;
	}
}

QString CParticleGraphModel::GetGraphName()
{
	return m_effect.GetName();
}

u32 CParticleGraphModel::GetNodeItemCount() const
{
	return m_nodes.size();
}

DrxGraphEditor::CAbstractNodeItem* CParticleGraphModel::GetNodeItemByIndex(u32 index) const
{
	if (index < m_nodes.size())
	{
		return static_cast<DrxGraphEditor::CAbstractNodeItem*>(m_nodes[index]);
	}

	return nullptr;
}

DrxGraphEditor::CAbstractNodeItem* CParticleGraphModel::GetNodeItemById(QVariant id) const
{
	return GetNodeItemById(QtUtil::ToString(id.value<QString>()));
}

void CParticleGraphModel::ToggleSoloNode(CNodeItem& node)
{
	if (m_pSolorNode != &node)
	{
		node.SetVisible(true);
		m_pSolorNode = &node;
		for (CNodeItem* pNodeItem : m_nodes)
		{
			if (pNodeItem != &node)
				pNodeItem->SetVisible(false);
		}
	}
	else
	{
		m_pSolorNode = nullptr;
		for (CNodeItem* pNodeItem : m_nodes)
		{
			pNodeItem->SetVisible(true);
		}
	}
}

DrxGraphEditor::CAbstractNodeItem* CParticleGraphModel::CreateNode(QVariant identifier, const QPointF& position)
{
	const string templateName = QtUtil::ToString(identifier.value<QString>());
	return CreateNode(templateName.c_str(), position);
}

bool CParticleGraphModel::RemoveNode(DrxGraphEditor::CAbstractNodeItem& node)
{
	CNodeItem* pNodeItem = &static_cast<CNodeItem&>(node);
	for (DrxGraphEditor::CAbstractPinItem* pPin : pNodeItem->GetPinItems())
	{
		const DrxGraphEditor::ConnectionItemSet& connections = pPin->GetConnectionItems();
		while (connections.size())
		{
			RemoveConnection(**connections.begin());
		}
	}

	// TODO: Move this into a CNodeGraphViewModel method that gets called from here.
	if (GetIEditor()->GetIUndoManager()->IsUndoRecording())
	{
		CUndo::Record(new DrxGraphEditor::CUndoNodeRemove(node));
	}

	SignalRemoveNode(node);
	// ~TODO

	u32k index = pNodeItem->GetIndex();
	DRX_ASSERT(index < m_nodes.size());
	if (index < m_nodes.size())
	{
		m_nodes.erase(m_nodes.begin() + index);

		delete pNodeItem;

		m_effect.RemoveComponent(index);
		return true;
	}

	delete pNodeItem;
	return false;
}

u32 CParticleGraphModel::GetConnectionItemCount() const
{
	return m_connections.size();
}

DrxGraphEditor::CAbstractConnectionItem* CParticleGraphModel::GetConnectionItemByIndex(u32 index) const
{
	if (index < m_connections.size())
	{
		return m_connections[index];
	}

	return nullptr;
}

DrxGraphEditor::CAbstractConnectionItem* CParticleGraphModel::GetConnectionItemById(QVariant id) const
{
	// TODO: Use a map for this.
	for (CConnectionItem* pConnectionItem : m_connections)
	{
		if (pConnectionItem->HasId(id))
			return pConnectionItem;
	}
	return nullptr;
}

DrxGraphEditor::CAbstractConnectionItem* CParticleGraphModel::CreateConnection(DrxGraphEditor::CAbstractPinItem& sourcePin, DrxGraphEditor::CAbstractPinItem& targetPin)
{
	return CreateConnection(static_cast<CBasePinItem&>(sourcePin), static_cast<CBasePinItem&>(targetPin));
}

bool CParticleGraphModel::RemoveConnection(DrxGraphEditor::CAbstractConnectionItem& connection)
{
	auto result = std::find(m_connections.begin(), m_connections.end(), &connection);
	if (result != m_connections.end())
	{
		// TODO: Move this into a CNodeGraphViewModel method that gets called from here.
		if (GetIEditor()->GetIUndoManager()->IsUndoRecording())
		{
			CUndo::Record(new DrxGraphEditor::CUndoConnectionRemove(connection));
		}

		SignalRemoveConnection(connection);
		// ~TODO

		m_connections.erase(result);

		CNodeItem& sourceNode = static_cast<CNodeItem&>(connection.GetSourcePinItem().GetNodeItem());
		CNodeItem& targetNode = static_cast<CNodeItem&>(connection.GetTargetPinItem().GetNodeItem());

		if (static_cast<CBasePinItem&>(connection.GetSourcePinItem()).GetPinType() == EPinType::Feature)
		{
			CFeaturePinItem& featurePin = static_cast<CFeaturePinItem&>(connection.GetSourcePinItem());
			pfx2::IParticleFeature& feature = featurePin.GetFeatureItem().GetFeatureInterface();
			feature.DisconnectFrom(targetNode.GetIdentifier());
		}
		else if (static_cast<CBasePinItem&>(connection.GetSourcePinItem()).GetPinType() == EPinType::Child)
		{
			targetNode.GetComponentInterface().SetParent(nullptr);
		}

		// TODO: ConnectTo(...) should do the job for us!
		sourceNode.GetComponentInterface().SetChanged();
		targetNode.GetComponentInterface().SetChanged();
		// ~TODO

		CConnectionItem* pConnection = static_cast<CConnectionItem*>(&connection);
		pConnection->OnConnectionRemoved();
		delete pConnection;

		return true;
	}

	return false;
}

DrxGraphEditor::CItemCollection* CParticleGraphModel::CreateClipboardItemsCollection()
{
	return new CClipboardItemCollection(*this);
}

CNodeItem* CParticleGraphModel::GetNodeItemById(string id) const
{
	for (CNodeItem* pNodeItem : m_nodes)
		if (id == pNodeItem->GetName())
			return pNodeItem;

	return nullptr;
}

void CParticleGraphModel::ExtractConnectionsFromNodes()
{
	for (CNodeItem* pSourceNodeItem : m_nodes)
	{
		for (CFeatureItem* pSourceFeatureItem : pSourceNodeItem->GetFeatureItems())
		{
			if (CBasePinItem* pSourcePin = pSourceFeatureItem->GetPinItem())
			{
				const pfx2::IParticleFeature& sourceFeature = pSourceFeatureItem->GetFeatureInterface();
				for (u32 c = 0; c < sourceFeature.GetNumConnectors(); ++c)
				{
					if (CNodeItem* pTargetNodeItem = GetNodeItemById(string(sourceFeature.GetConnectorName(c))))
					{
						CBasePinItem* pTargetPin = pTargetNodeItem->GetParentPinItem();

						DRX_ASSERT(pSourcePin && pTargetPin);
						CConnectionItem* pConnectionItem = new CConnectionItem(*pSourcePin, *pTargetPin, *this);
						m_connections.push_back(pConnectionItem);
					}
				}
			}
		}
	}

	for (CNodeItem* pChildNodeItem : m_nodes)
	{
		auto& component = pChildNodeItem->GetComponentInterface();
		if (auto* pParent = component.GetParent())
		{
			if (CNodeItem* pParentNodeItem = GetNodeItemById(string(pParent->GetName())))
			{
				CBasePinItem* pSourcePin = pParentNodeItem->GetChildPinItem();
				CBasePinItem* pTargetPin = pChildNodeItem->GetParentPinItem();
				DRX_ASSERT(pSourcePin && pTargetPin);

				if (pTargetPin->CanConnect(pSourcePin))
				{
					CConnectionItem* pConnectionItem = new CConnectionItem(*pSourcePin, *pTargetPin, *this);
					m_connections.push_back(pConnectionItem);
				}
			}
		}
	}
}

CNodeItem* CParticleGraphModel::CreateNode(const string& buffer, const QPointF& position)
{
	if (pfx2::IParticleComponent* pComponent = m_effect.AddComponent())
	{
		if (Serialization::LoadJsonBuffer(*pComponent, buffer.data(), buffer.size()))
		{
			// TODO: We need to remove all connections from the node because the backend serializes
			//			 all connections with the features. Remove this when fixed on backend side.
			u32k fe = pComponent->GetNumFeatures();
			for (u32 fi = 0; fi < fe; ++fi)
			{
				pfx2::IParticleFeature* pFeature = pComponent->GetFeature(fi);
				while (tukk szConnectorName = pFeature->GetConnectorName(0))
				{
					pFeature->DisconnectFrom(szConnectorName);
				}
			}
			// ~TODO

			pComponent->SetNodePosition(Vec2(position.x(), position.y()));
			return CreateNodeItem(*pComponent);
		}
		else
		{
			m_effect.RemoveComponent(m_effect.GetNumComponents() - 1);
		}
	}

	return nullptr;
}

CNodeItem* CParticleGraphModel::CreateNode(tukk szTemplateName, const QPointF& position)
{
	if (pfx2::IParticleComponent* pComponent = m_effect.AddComponent())
	{
		if (szTemplateName == nullptr || *szTemplateName == '\0' || Serialization::LoadJsonFile(*pComponent, szTemplateName))
		{
			pComponent->SetNodePosition(Vec2(position.x(), position.y()));
			return CreateNodeItem(*pComponent);
		}
		else
		{
			m_effect.RemoveComponent(m_effect.GetNumComponents() - 1);
		}
	}

	return nullptr;
}

CConnectionItem* CParticleGraphModel::CreateConnection(CBasePinItem& sourcePin, CBasePinItem& targetPin)
{
	if (sourcePin.CanConnect(&targetPin))
	{
		CConnectionItem* pConnectionItem = new CConnectionItem(sourcePin, targetPin, *this);
		m_connections.push_back(pConnectionItem);

		CNodeItem& sourceNode = static_cast<CNodeItem&>(sourcePin.GetNodeItem());
		CNodeItem& targetNode = static_cast<CNodeItem&>(targetPin.GetNodeItem());

		if (sourcePin.GetPinType() == EPinType::Feature)
		{
			CFeaturePinItem& featurePin = static_cast<CFeaturePinItem&>(sourcePin);
			pfx2::IParticleFeature& feature = featurePin.GetFeatureItem().GetFeatureInterface();
			feature.ConnectTo(targetNode.GetIdentifier());
		}
		else if (sourcePin.GetPinType() == EPinType::Child)
		{
			targetNode.GetComponentInterface().SetParent(&sourceNode.GetComponentInterface());
		}

		// TODO: ConnectTo(...) should do the job for us!
		sourceNode.GetComponentInterface().SetChanged();
		targetNode.GetComponentInterface().SetChanged();
		// ~TODO

		// TODO: Move this into a CNodeGraphViewModel method that gets called from here.
		SignalCreateConnection(*pConnectionItem);

		if (GetIEditor()->GetIUndoManager()->IsUndoRecording())
		{
			CUndo::Record(new DrxGraphEditor::CUndoConnectionCreate(*pConnectionItem));
		}
		// ~TODO

		return pConnectionItem;
	}

	return nullptr;
}

CNodeItem* CParticleGraphModel::CreateNodeItem(pfx2::IParticleComponent& component)
{
	CNodeItem* pNodeItem = new CNodeItem(component, *this);
	u32k crc = CCrc32::Compute(component.GetName());
	m_nodes.push_back(pNodeItem);

	// TODO: AddComponent(...) should do the job for us!
	m_effect.SetChanged();
	// ~TODO

	// TODO: Move this into a CNodeGraphViewModel method that gets called from here.
	SignalCreateNode(*pNodeItem);

	if (GetIEditor()->GetIUndoManager()->IsUndoRecording())
	{
		CUndo::Record(new DrxGraphEditor::CUndoNodeCreate(*pNodeItem));
	}
	// ~TODO

	return pNodeItem;
}

void AddNodeStyle(DrxGraphEditor::CNodeGraphViewStyle& viewStyle, tukk szStyleId, tukk szIcon, QColor color, bool coloredHeaderIconText = true)
{
	DrxGraphEditor::CNodeWidgetStyle* pStyle = new DrxGraphEditor::CNodeWidgetStyle(szStyleId, viewStyle);
	DrxGraphEditor::CNodeHeaderWidgetStyle& headerStyle = pStyle->GetHeaderWidgetStyle();
	headerStyle.SetNodeIcon(QIcon());
}

void AddConnectionStyle(DrxGraphEditor::CNodeGraphViewStyle& viewStyle, tukk szStyleId, float width)
{
	DrxGraphEditor::CConnectionWidgetStyle* pStyle = new DrxGraphEditor::CConnectionWidgetStyle(szStyleId, viewStyle);
	pStyle->SetWidth(width);
}

void AddPinStyle(DrxGraphEditor::CNodeGraphViewStyle& viewStyle, tukk szStyleId, tukk szIcon, QColor color)
{
	DrxIcon icon(szIcon, {
			{ QIcon::Mode::Normal, color }
	  });

	DrxGraphEditor::CNodePinWidgetStyle* pStyle = new DrxGraphEditor::CNodePinWidgetStyle(szStyleId, viewStyle);
	pStyle->SetIcon(icon);
	pStyle->SetColor(color);
}

DrxGraphEditor::CNodeGraphViewStyle* CreateStyle()
{
	DrxGraphEditor::CNodeGraphViewStyle* pViewStyle = new DrxGraphEditor::CNodeGraphViewStyle("Particles");

	//AddNodeStyle(*pViewStyle, "Node", "", QColor(255, 100, 150));
	//AddConnectionStyle(*pViewStyle, "Connection", 2.0);
	//AddPinStyle(*pViewStyle, "Pin", "icons:Graph/Node_connection_arrow_R.ico", QColor(200, 200, 200));

	return pViewStyle;
}

CParticleGraphRuntimeContext::CParticleGraphRuntimeContext()
{
	m_pStyle = CreateStyle();
}

CParticleGraphRuntimeContext::~CParticleGraphRuntimeContext()
{
	m_pStyle->deleteLater();
}

}

