// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ClipboardItems.h"

#include "ParticleGraphItems.h"
#include "ParticleGraphModel.h"

#include "Widgets/FeatureGridNodeContentWidget.h"

#include <NodeGraph/NodeWidget.h>

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/STL.h>

namespace DrxParticleEditor {

void CClipboardItemCollection::SConnection::Serialize(Serialization::IArchive& archive)
{
	archive(sourceNodeIndex, "sorceNode");
	archive(sourceFeatureIndex, "srcFeature");
	archive(targetNodeIndex, "targetNode");
}

void CClipboardItemCollection::SNode::Serialize(Serialization::IArchive& archive)
{
	archive(positionX, "posX");
	archive(positionY, "posY");
	archive(dataBuffer, "nodeData");
}

void CClipboardItemCollection::SFeature::Serialize(Serialization::IArchive& archive)
{
	archive(groupName, "groupName");
	archive(featureName, "featureName");
	archive(dataBuffer, "nodeData");
}

CClipboardItemCollection::CClipboardItemCollection(CParticleGraphModel& model)
	: m_model(model)
{}

void CClipboardItemCollection::Serialize(Serialization::IArchive& archive)
{
	DrxGraphEditor::CItemCollection::Serialize(archive);

	std::vector<SNode> nodeSer;
	std::vector<SConnection> connectionSer;
	std::vector<SFeature> featureSer;

	if (archive.isOutput())
	{
		NodeIndexByInstance nodeIndexByInstance;

		std::vector<CFeatureItem*> features;
		for (DrxGraphEditor::CAbstractNodeGraphViewModelItem* pAbstractItem : GetCustomItems())
		{
			switch (pAbstractItem->GetType())
			{
			case eCustomItemType_Feature:
				features.push_back(static_cast<CFeatureItem*>(pAbstractItem));
				break;
			}
		}

		if (GetNodes().size())
		{
			for (DrxGraphEditor::CAbstractNodeItem* pAbstractNodeItem : GetNodes())
			{
				CNodeItem* pNodeItem = static_cast<CNodeItem*>(pAbstractNodeItem);

				const size_t index = nodeSer.size();
				nodeIndexByInstance.emplace(pNodeItem, index);

				DynArray<char> nodeData;
				Serialization::SaveJsonBuffer(nodeData, *pAbstractNodeItem);
				nodeData.push_back('\0');

				SNode n;
				n.positionX = pNodeItem->GetPosition().x() - GetItemsRect().center().x();
				n.positionY = pNodeItem->GetPosition().y() - GetItemsRect().center().y();
				n.dataBuffer = nodeData.data();

				nodeSer.emplace_back(n);
			}

			for (DrxGraphEditor::CAbstractConnectionItem* pConnectionItem : GetConnections())
			{
				auto sourceNode = nodeIndexByInstance.find(static_cast<CNodeItem*>(&pConnectionItem->GetSourcePinItem().GetNodeItem()));
				auto targetNode = nodeIndexByInstance.find(static_cast<CNodeItem*>(&pConnectionItem->GetTargetPinItem().GetNodeItem()));

				if (sourceNode != nodeIndexByInstance.end() && targetNode != nodeIndexByInstance.end())
				{
					CBasePinItem* pSourcePin = pConnectionItem->GetSourcePinItem().Cast<CBasePinItem>();
					CBasePinItem* pTargetPin = pConnectionItem->GetTargetPinItem().Cast<CBasePinItem>();

					if (pSourcePin && pTargetPin && (pSourcePin->GetPinType() == EPinType::Feature) && (pTargetPin->GetPinType() == EPinType::Parent))
					{
						CFeaturePinItem* pFeaturePin = pSourcePin->Cast<CFeaturePinItem>();
						CParentPinItem* pParentPin = pTargetPin->Cast<CParentPinItem>();

						if (pFeaturePin && pParentPin)
						{
							SConnection c;
							c.sourceNodeIndex = sourceNode->second;
							c.sourceFeatureIndex = pFeaturePin->GetFeatureItem().GetIndex();
							c.targetNodeIndex = targetNode->second;
							connectionSer.emplace_back(c);
						}
					}
				}
			}

			archive(nodeSer, "nodes");
			archive(connectionSer, "connections");
		}
		else if (features.size())
		{
			for (CFeatureItem* pFeatureItem : features)
			{
				const pfx2::SParticleFeatureParams& params = pFeatureItem->GetFeatureInterface().GetFeatureParams();

				DynArray<char> buffer;
				Serialization::SaveJsonBuffer(buffer, pFeatureItem->GetFeatureInterface());
				buffer.push_back(0);

				SFeature f;
				f.groupName = params.m_groupName;
				f.featureName = params.m_featureName;
				f.dataBuffer = buffer.data();

				featureSer.emplace_back(f);
			}

			archive(featureSer, "features");
		}
	}
	else
	{
		NodesByIndex nodes;

		archive(nodeSer, "nodes");
		archive(connectionSer, "connections");
		archive(featureSer, "features");

		if (nodeSer.size() > 0)
		{
			const QPointF sceenPos = GetScenePosition();
			for (SNode& n : nodeSer)
			{
				if (CNodeItem* pNodeItem = m_model.CreateNode(n.dataBuffer, QPointF(n.positionX + sceenPos.x(), n.positionY + sceenPos.y())))
				{
					AddNode(*pNodeItem);
					nodes.push_back(pNodeItem);
				}
			}

			for (SConnection& c : connectionSer)
			{
				if (c.sourceNodeIndex < nodes.size() && c.sourceNodeIndex >= 0 && c.targetNodeIndex < nodes.size() && c.targetNodeIndex >= 0)
				{
					CFeaturePinItem* pSourcePin = nodes[c.sourceNodeIndex]->GetFeatureItems().at(c.sourceFeatureIndex)->GetPinItem();
					CParentPinItem* pTargetPin = nodes[c.targetNodeIndex]->GetParentPinItem();

					if (pSourcePin && pTargetPin)
					{
						if (CConnectionItem* pConnectionItem = m_model.CreateConnection(*pSourcePin, *pTargetPin))
						{
							AddConnection(*pConnectionItem);
						}
					}
				}
			}
		}
		else if (featureSer.size() > 0)
		{
			if (DrxGraphEditor::CNodeGraphViewGraphicsWidget* pViewWidget = GetViewWidget())
			{
				DrxGraphEditor::CNodeWidget* pNodeWidget = pViewWidget->Cast<DrxGraphEditor::CNodeWidget>();
				if (pNodeWidget == nullptr)
				{
					if (CFeatureWidget* pFeatureWidget = pViewWidget->Cast<CFeatureWidget>())
					{
						if (featureSer.size() == 1)
						{
							SFeature& f = *featureSer.begin();

							i32k index = pFeatureWidget->GetItem().GetIndex();
							CNodeItem& nodeItem = pFeatureWidget->GetItem().GetNodeItem();
							CFeatureItem* pFeatureItem = nodeItem.GetFeatureItems().at(index);

							pfx2::IParticleFeature& feature = pFeatureItem->GetFeatureInterface();
							const pfx2::SParticleFeatureParams& params = feature.GetFeatureParams();
							if (f.groupName == params.m_groupName && f.featureName == params.m_featureName)
							{
								Serialization::LoadJsonBuffer(feature, f.dataBuffer.data(), f.dataBuffer.size());
							}
						}
						else
						{
							pNodeWidget = &pFeatureWidget->GetNodeWidget();
						}
					}
					else
					{
						if (DrxGraphEditor::CPinWidget* pPinWidget = pViewWidget->Cast<DrxGraphEditor::CPinWidget>())
						{
							pNodeWidget = &pPinWidget->GetNodeWidget();
						}
					}
				}

				if (pNodeWidget)
				{
					if (CNodeItem* pNodeItem = pNodeWidget->GetItem().Cast<CNodeItem>())
					{
						i32k index = pNodeItem->GetNumFeatures();
						for (SFeature& f : featureSer)
						{
							for (uint i = 0; i < GetParticleSystem()->GetNumFeatureParams(); ++i)
							{
								const pfx2::SParticleFeatureParams& params = GetParticleSystem()->GetFeatureParam(i);
								if (f.groupName == params.m_groupName && f.featureName == params.m_featureName)
								{
									pNodeItem->AddFeature(index, params);
									CFeatureItem* pFeatureItem = pNodeItem->GetFeatureItems().at(index);
									pfx2::IParticleFeature& feature = pFeatureItem->GetFeatureInterface();

									Serialization::LoadJsonBuffer(feature, f.dataBuffer.data(), f.dataBuffer.size());
								}
							}
						}
					}
				}
			}
		}
	}
}

}

