// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "Model.h"
#include "GraphViewModel.h"
#include "GraphView.h"
#include "NodeGraph/AbstractPinItem.h"
#include "NodeGraph/AbstractConnectionItem.h"
#include "NodeGraph/NodeWidget.h"
#include "NodeGraph/AbstractNodeContentWidget.h"
#include "NodeGraph/PinGridNodeContentWidget.h"
#include "NodeGraph/ConnectionWidget.h"
#include "NodeGraph/NodeHeaderWidgetStyle.h"
#include "AssetSystem/AssetManager.h"
#include "AssetSystem/DependencyTracker.h"
#include "Notifications/NotificationCenter.h"
#include <IEditor.h>

class CGraphViewModel;

namespace Private_GraphViewModel
{

struct SAssetData
{
	SAssetData(CAsset* pAsset, const string& assetPath, size_t depth) : p(pAsset), path(assetPath), depth(depth)
	{
	}

	CAsset* p = nullptr;
	string  path;
	size_t  depth = 0;
	bool    bSelected = false;
	size_t  node = 0;
};

}

class CNodeEntry : public CAbstractDictionaryEntry
{
public:
	CNodeEntry(const CAssetNodeBase& node)
		: CAbstractDictionaryEntry()
		, m_node(node)
	{}
	virtual ~CNodeEntry() {}

	virtual u32 GetType() const override
	{
		return Type_Entry;
	}
	virtual QVariant GetColumnValue(i32 columnIndex) const override
	{
		return QVariant::fromValue(m_node.GetName());
	}
	virtual QVariant GetIdentifier() const override
	{
		return QVariant::fromValue(QtUtil::ToQString(m_node.GetPath()));
	}
	virtual QString GetToolTip() const
	{
		return m_node.GetToolTipText();
	}
private:
	const CAssetNodeBase& m_node;
};

class CNodesDictionary : public CAbstractDictionary
{
public:
	CNodesDictionary(DrxGraphEditor::CNodeGraphViewModel& model)
		: CAbstractDictionary()
		, m_model(model)
	{
	}
	virtual ~CNodesDictionary() {}

	virtual i32 GetNumColumns() const
	{
		return 1;
	}
	virtual i32 GetNumEntries() const override
	{
		return m_model.GetNodeItemCount();
	}
	virtual const CAbstractDictionaryEntry* GetEntry(i32 index) const override
	{
		return m_model.GetNodeItemByIndex(index)->Cast<CAssetNodeBase>()->GetNodeEntry();
	}

private:
	DrxGraphEditor::CNodeGraphViewModel& m_model;
};

class CGraphRuntimeContext : public DrxGraphEditor::INodeGraphRuntimeContext
{
public:
	CGraphRuntimeContext(CGraphViewModel& model)
		: m_pStyle(new DrxGraphEditor::CNodeGraphViewStyle("Dependencies"))
		, m_pNodesDictionary(new CNodesDictionary(model))
	{
		const DrxGraphEditor::CNodeWidgetStyle* pStyle = m_pStyle.get()->GetNodeWidgetStyle("Node");

		for (const CAssetType* pType : model.GetModel()->GetSupportedTypes())
		{
			AddNodeStyle(GetAssetStyleId(pType, true), pType->GetIcon(), QColor::fromRgb(8, 255, 64), pStyle->GetBackgroundColor());
			AddNodeStyle(GetAssetStyleId(pType, false), pType->GetIcon(), QColor::fromRgb(255, 64, 8), QColor::fromRgb(64, 16, 2));
		}
	}

	virtual tukk GetTypeName() const override
	{
		return "asset_dependency_graph";
	}
	virtual CAbstractDictionary* GetAvailableNodesDictionary() override
	{
		return m_pNodesDictionary.get();
	}
	virtual const DrxGraphEditor::CNodeGraphViewStyle* GetStyle() const override
	{
		return m_pStyle.get();
	}

	static string GetAssetStyleId(const CAssetType* pAssetType, bool bAssetFound)
	{
		if (pAssetType && bAssetFound)
		{
			return pAssetType->GetTypeName();
		}
		else if (pAssetType)
		{
			return string().Format("%s_not_found", pAssetType->GetTypeName());
		}

		return bAssetFound ? "cryasset" : "cryasset_not_found";
	}

protected:

	void AddNodeStyle(tukk szStyleId, DrxIcon& icon, QColor iconColor, QColor backgroundColor)
	{
		DrxGraphEditor::CNodeWidgetStyle* pStyle = new DrxGraphEditor::CNodeWidgetStyle(szStyleId, *(m_pStyle.get()));

		QPixmap iconPixmap = icon.pixmap(pStyle->GetHeaderWidgetStyle().GetNodeIconViewSize(), QIcon::Normal);
		iconPixmap.detach();

		pStyle->GetHeaderWidgetStyle().SetNodeIconMenuColor(iconColor);
		pStyle->GetHeaderWidgetStyle().SetNodeIconViewDefaultColor(iconColor);
		pStyle->GetHeaderWidgetStyle().SetNodeIcon(DrxIcon(iconPixmap));
		pStyle->SetBackgroundColor(backgroundColor);
	}

	std::unique_ptr<DrxGraphEditor::CNodeGraphViewStyle> m_pStyle;
	std::unique_ptr<CNodesDictionary>                    m_pNodesDictionary;
};

class CBasePinItem : public DrxGraphEditor::CAbstractPinItem
{
public:
	CBasePinItem(DrxGraphEditor::CAbstractNodeItem& nodeItem)
		: DrxGraphEditor::CAbstractPinItem(nodeItem.GetViewModel())
		, m_nodeItem(nodeItem)
	{
	}
	virtual DrxGraphEditor::CAbstractNodeItem& GetNodeItem() const override
	{
		return m_nodeItem;
	}
	DrxGraphEditor::CPinWidget* CreateWidget(DrxGraphEditor::CNodeWidget& nodeWidget, DrxGraphEditor::CNodeGraphView& view) override
	{
		return new DrxGraphEditor::CPinWidget(*this, nodeWidget, view, IsOutputPin() && IsConnected());
	}
	virtual bool CanConnect(const CAbstractPinItem* pOtherPin) const override
	{
		// Dependency graph does not allow editing.
		return false;
	}
protected:
	DrxGraphEditor::CAbstractNodeItem& m_nodeItem;
};

class CInPinItem : public CBasePinItem
{
public:
	CInPinItem(DrxGraphEditor::CAbstractNodeItem& nodeItem)
		: CBasePinItem(nodeItem)
	{
	}
	virtual QString GetName() const override
	{
		return QString("Asset");
	}
	virtual QString GetDescription() const override
	{
		return QString("Asset");
	}
	virtual QString GetTypeName() const override
	{
		return QString("Dependency");
	}
	virtual QVariant GetId() const override
	{
		return QVariant::fromValue(QString("In"));
		;
	}
	virtual bool HasId(QVariant id) const override
	{
		QString name = "In";
		return (name == id.value<QString>());
	}
	virtual bool IsInputPin() const override
	{
		return true;
	}
	virtual bool IsOutputPin() const override
	{
		return false;
	}
};

class COutPinItem : public CBasePinItem
{
public:
	COutPinItem(DrxGraphEditor::CAbstractNodeItem& nodeItem)
		: CBasePinItem(nodeItem)
	{
	}
	virtual QString GetName() const override
	{
		return QString("Dependencies");
	}
	virtual QString GetDescription() const override
	{
		return QString("Dependencies");
	}
	virtual QString GetTypeName() const override
	{
		return QString("Dependency");
	}
	virtual QVariant GetId() const override
	{
		return QVariant::fromValue(QString("Out"));
		;
	}
	virtual bool HasId(QVariant id) const override
	{
		QString name = "Out";
		return (name == id.value<QString>());
	}
	virtual bool IsInputPin() const override
	{
		return false;
	}
	virtual bool IsOutputPin() const override
	{
		return true;
	}
};

class CFeatureGridNodeContentWidget : public DrxGraphEditor::CAbstractNodeContentWidget
{
public:
	CFeatureGridNodeContentWidget(DrxGraphEditor::CNodeWidget& node, DrxGraphEditor::CNodeGraphView& view)
		: CAbstractNodeContentWidget(node, view)
	{
	}
};

CAssetNodeBase::CAssetNodeBase(DrxGraphEditor::CNodeGraphViewModel& viewModel, CAsset* pAsset, const CAssetType* pAssetType, const string& path)
	: CAbstractNodeItem(viewModel)
	, m_pAsset(pAsset)
	, m_pAssetType(pAssetType)
	, m_path(path)
{
}

bool CAssetNodeBase::CanBeEdited() const
{
	return m_pAsset && m_pAsset->GetType()->CanBeEdited();
}

void CAssetNodeBase::EditAsset() const
{
	DRX_ASSERT(CanBeEdited());
	m_pAsset->Edit();
}

bool CAssetNodeBase::CanBeCreated() const
{
	return !m_pAsset && m_pAssetType && m_pAssetType->CanBeCreated();
}

void CAssetNodeBase::CreateAsset() const
{
	DRX_ASSERT(CanBeCreated());
	m_pAssetType->Create(m_path + ".cryasset");
}

bool CAssetNodeBase::CanBeImported() const
{
	return !m_pAsset && m_pAssetType && m_pAssetType->IsImported();
}

std::vector<string> CAssetNodeBase::GetSourceFileExtensions() const
{
	DRX_ASSERT(CanBeImported());
	return static_cast<CGraphViewModel*>(&GetViewModel())->GetModel()->GetSourceFileExtensions(m_pAssetType);
}

void CAssetNodeBase::ImportAsset(const string& sourceFilename) const
{
	DRX_ASSERT(CanBeImported());
	m_pAssetType->Import(sourceFilename, PathUtil::GetPathWithoutFilename(m_path), PathUtil::GetFileName(m_path));
}

//! Asset view model.
class CAssetNode : public CAssetNodeBase
{
public:
	enum EPin
	{
		ePin_In = 0,
		ePin_Out
	};
public:
	CAssetNode(DrxGraphEditor::CNodeGraphViewModel& viewModel, CAsset* pAsset, const CAssetType* pAssetType, const string& path)
		: CAssetNodeBase(viewModel, pAsset, pAssetType, path)
		, m_In(*this)
		, m_Out(*this)
		, m_pins({ &m_In, &m_Out })
		, m_nodeEntry(*this)
	{
		DRX_ASSERT(m_pins[ePin_In] == &m_In && m_pins[ePin_Out] == &m_Out);

		SetAcceptsDeletion(false);
		SetAcceptsCopy(false);
		SetAcceptsPaste(false);
		SetAcceptsDeactivation(false);
	}
	virtual ~CAssetNode()
	{
	}

	virtual DrxGraphEditor::CNodeWidget* CreateWidget(DrxGraphEditor::CNodeGraphView& view) override
	{
		CAssetWidget* pNode = new CAssetWidget(*this, view);
		pNode->SetHeaderNameWidth(120);
		auto pContent = new DrxGraphEditor::CPinGridNodeContentWidget(*pNode, view);

		pNode->SetContentWidget(pContent);
		return pNode;
	}
	virtual QVariant GetId() const override
	{
		return QVariant();
	}
	virtual bool HasId(QVariant id) const override
	{
		return false;
	}
	virtual QVariant GetTypeId() const override
	{
		return QVariant();
	}
	virtual const DrxGraphEditor::PinItemArray& GetPinItems() const override
	{
		return m_pins;
	}
	virtual tukk GetStyleId() const override
	{
		return CGraphRuntimeContext::GetAssetStyleId(m_pAssetType, m_pAsset != nullptr);
	}

	virtual QString GetToolTipText() const override
	{
		if (m_pAsset)
		{
			return QtUtil::ToQString(m_path);
		}

		return QString("%1%2").arg(QObject::tr("Asset not found:\n"), QtUtil::ToQString(m_path));
	}

	virtual const CAbstractDictionaryEntry* GetNodeEntry() const override
	{
		return &m_nodeEntry;
	}

private:
	CInPinItem                   m_In;
	COutPinItem                  m_Out;
	DrxGraphEditor::PinItemArray m_pins;
	CNodeEntry                   m_nodeEntry;
};

class CConnectionItem : public DrxGraphEditor::CAbstractConnectionItem
{
public:
	CConnectionItem(DrxGraphEditor::CAbstractPinItem& sourcePin, DrxGraphEditor::CAbstractPinItem& targetPin, DrxGraphEditor::CNodeGraphViewModel& viewModel)
		: DrxGraphEditor::CAbstractConnectionItem(viewModel)
		, m_sourcePin(sourcePin)
		, m_targetPin(targetPin)
	{
		m_sourcePin.AddConnection(*this);
		m_targetPin.AddConnection(*this);

		SetAcceptsDeletion(false);
		SetAcceptsCopy(false);
		SetAcceptsPaste(false);
		SetAcceptsDeactivation(false);
	}
	virtual ~CConnectionItem()
	{
		m_sourcePin.RemoveConnection(*this);
		m_targetPin.RemoveConnection(*this);
	}
	virtual QVariant GetId() const override
	{
		return QVariant();
	}
	virtual bool HasId(QVariant id) const override
	{
		return false;
	}
	virtual DrxGraphEditor::CConnectionWidget* CreateWidget(DrxGraphEditor::CNodeGraphView& view) override
	{
		return new DrxGraphEditor::CConnectionWidget(this, view);
	}
	virtual DrxGraphEditor::CAbstractPinItem& GetSourcePinItem() const override
	{
		return m_sourcePin;
	}
	virtual DrxGraphEditor::CAbstractPinItem& GetTargetPinItem() const override
	{
		return m_targetPin;
	}
private:
	DrxGraphEditor::CAbstractPinItem& m_sourcePin;
	DrxGraphEditor::CAbstractPinItem& m_targetPin;
};

CGraphViewModel::CGraphViewModel(CModel* pModel)
	: m_pRuntimeContext(new CGraphRuntimeContext(*this))
	, m_pModel(pModel)
	, m_bSuppressComplexityWarning(false)
{
	m_pModel->signalBeginChange.Connect(this, &CGraphViewModel::OnBeginModelChange);
	m_pModel->signalEndChange.Connect(this, &CGraphViewModel::OnEndModelChange);
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
	return m_nodes[index].get();
}

DrxGraphEditor::CAbstractNodeItem* CGraphViewModel::GetNodeItemById(QVariant id) const
{
	const auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [path = QtUtil::ToString(id.toString())](const auto& x)
	{
		return x.get()->Cast<CAssetNodeBase>()->GetPath() == path;
	});

	return it != m_nodes.end() ? it->get() : nullptr;
}

u32 CGraphViewModel::GetConnectionItemCount() const
{
	return m_connections.size();
}

DrxGraphEditor::CAbstractConnectionItem* CGraphViewModel::GetConnectionItemByIndex(u32 index) const
{
	DRX_ASSERT(index < m_connections.size());
	return m_connections[index].get();
}

DrxGraphEditor::CAbstractConnectionItem* CGraphViewModel::GetConnectionItemById(QVariant id) const
{
	DRX_ASSERT("NotImplemented");
	return nullptr;
}

void CGraphViewModel::OnBeginModelChange()
{
	m_nodes.clear();
	m_connections.clear();
	SignalInvalidated();
}

void CGraphViewModel::OnEndModelChange()
{
	using namespace Private_GraphViewModel;

	DRX_ASSERT(m_pModel);

	if (!m_pModel->GetAsset())
	{
		return;
	}

	std::vector<SAssetData> assets;
	std::vector<std::pair<size_t, size_t>> connections;

	std::unordered_map<string, size_t, stl::hash_strcmp<string>> map(1000);
	std::vector<size_t> stack(8, 0);
	
	assets.reserve(1000);
	connections.reserve(4000);

	m_pModel->ForAllDependencies(m_pModel->GetAsset(), [this, &stack, &map, &assets, &connections](CAsset* pAsset, const string& assetPath, size_t depth)
	{
		// Ignore dependencies on the engine assets.
		if (assetPath[0] == '%')
		{
			return;
		}

		if (stack.size() <= depth)
		{
			stack.resize(depth + 1, 0);
		}

		CAssetNode* pNode = nullptr;
		const string assetKey = string(assetPath).MakeLower();
		const auto it = map.find(assetKey);
		if (it == map.end())
		{
			map.insert(std::make_pair(assetKey, assets.size()));
			stack[depth] = assets.size();
			assets.emplace_back(pAsset, assetPath, depth);

			if (!pAsset)
			{
				// Mark parent assets.
				for (size_t i = 0; i <= depth; ++i)
				{
					assets[stack[i]].bSelected = true;
				}
			}
		}
		else
		{
			assets[it->second].depth = std::max(assets[it->second].depth, depth);
			stack[depth] = it->second;
		}

		if (depth)
		{
			connections.emplace_back(stack[depth - 1], stack[depth]);
		}
	});

	static const size_t maximumNumberOfNodes = 100;
	const bool bAllNodes = assets.size() <= maximumNumberOfNodes;

	// Create a simple table layout.
	// Having pre-order depth-first traversing of the nodes, we move to the next table row If we do not go to the next depth level.
	size_t row = 0;
	size_t column = 0;
	size_t totalCountOfUnresolvedNodes = 0;
	size_t countOfUnresolvedNodesToBeShown = 0;
	for (SAssetData& asset : assets)
	{
		if (!asset.p)
		{
			++totalCountOfUnresolvedNodes;
		}

		if (!asset.bSelected && !bAllNodes)
		{
			continue;
		}

		if (!bAllNodes && (m_nodes.size() >= maximumNumberOfNodes))
		{
			asset.bSelected = false;
			continue;
		}

		if (asset.depth && column >= asset.depth)
		{
			++row;
		}
		column = asset.depth;

		const CAssetType* pAssetType = asset.p ? asset.p->GetType() : m_pModel->FindAssetTypeByFile(asset.path);
		CAssetNode* pNode = new CAssetNode(*this, asset.p, pAssetType, asset.path);
		pNode->SetAcceptsRenaming(true);
		pNode->SetName(QtUtil::ToQString(PathUtil::GetFileName(asset.path)));
		pNode->SetAcceptsRenaming(false);

		size_t x = column * 500;
		size_t y = row * 100;
		pNode->SetPosition(QPointF(x, y));

		asset.node = m_nodes.size();
		m_nodes.emplace_back(pNode);

		if (!asset.p)
		{
			++countOfUnresolvedNodesToBeShown;
		}
	}

	for (const auto c : connections)
	{
		if (bAllNodes || (assets[c.first].bSelected && assets[c.second].bSelected))
		{
			DrxGraphEditor::CAbstractPinItem* pSrcPinItem = m_nodes[assets[c.first].node]->GetPinItemByIndex(CAssetNode::ePin_Out);
			DrxGraphEditor::CAbstractPinItem* pDstPinItem = m_nodes[assets[c.second].node]->GetPinItemByIndex(CAssetNode::ePin_In);
			m_connections.emplace_back(new CConnectionItem(*pSrcPinItem, *pDstPinItem, *this));
		}
	}

	if (!bAllNodes && !m_bSuppressComplexityWarning)
	{
		GetIEditor()->GetNotificationCenter()->ShowWarning(tr("Dependency Graph"), tr("\"%1\" has too many dependencies to display the whole graph."
			"\n%2 out of %3 missing assets will be displayed.")
			.arg(QtUtil::ToQString(m_pModel->GetAsset()->GetName()))
			.arg(countOfUnresolvedNodesToBeShown)
			.arg(totalCountOfUnresolvedNodes));

		m_bSuppressComplexityWarning = true;
	}

	SignalInvalidated();
}

