// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/AbstractNodeGraphViewModel.h>

#include "ParticleNodeTreeModel.h"

#include <drx3D/CoreX/Extension/DrxGUID.h>
#include <DrxParticleSystem/IParticlesPfx2.h>

namespace DrxParticleEditor {

class CNodeItem;
class CBasePinItem;
class CConnectionItem;
class CFeatureItem;

class CParticleGraphModel;

class CParticleGraphRuntimeContext : public DrxGraphEditor::INodeGraphRuntimeContext
{
public:
	CParticleGraphRuntimeContext();
	~CParticleGraphRuntimeContext();

	// DrxGraphEditor::INodeGraphRuntimeContext
	virtual tukk                                GetTypeName() const override           { return "ParticleFX2"; }
	virtual CAbstractDictionary*                       GetAvailableNodesDictionary() override { return &m_nodeTreeModel; }
	virtual const DrxGraphEditor::CNodeGraphViewStyle* GetStyle() const                       { return m_pStyle; }
	// ~DrxGraphEditor::INodeGraphRuntimeContext

private:
	CNodesDictionary                     m_nodeTreeModel;
	DrxGraphEditor::CNodeGraphViewStyle* m_pStyle;
};

class CParticleGraphModel : public DrxGraphEditor::CNodeGraphViewModel
{
	Q_OBJECT

public:
	CParticleGraphModel(pfx2::IParticleEffectPfx2& effect);
	virtual ~CParticleGraphModel();

	// NodeGraph::CNodeGraphViewModel
	virtual DrxGraphEditor::INodeGraphRuntimeContext& GetRuntimeContext() { return m_runtimeContext; }
	virtual QString                                   GetGraphName() override;

	virtual u32                                    GetNodeItemCount() const override;
	virtual DrxGraphEditor::CAbstractNodeItem*        GetNodeItemByIndex(u32 index) const override;
	virtual DrxGraphEditor::CAbstractNodeItem*        GetNodeItemById(QVariant id) const override;
	virtual DrxGraphEditor::CAbstractNodeItem*        CreateNode(QVariant identifier, const QPointF& position = QPointF()) override;
	virtual bool                                      RemoveNode(DrxGraphEditor::CAbstractNodeItem& node) override;

	virtual u32                                    GetConnectionItemCount() const override;
	virtual DrxGraphEditor::CAbstractConnectionItem*  GetConnectionItemByIndex(u32 index) const override;
	virtual DrxGraphEditor::CAbstractConnectionItem*  GetConnectionItemById(QVariant id) const override;
	virtual DrxGraphEditor::CAbstractConnectionItem*  CreateConnection(DrxGraphEditor::CAbstractPinItem& sourcePin, DrxGraphEditor::CAbstractPinItem& targetPin) override;
	virtual bool                                      RemoveConnection(DrxGraphEditor::CAbstractConnectionItem& connection) override;

	virtual DrxGraphEditor::CItemCollection*          CreateClipboardItemsCollection() override;
	// ~NodeGraph::CNodeGraphViewModel

	CNodeItem*                       CreateNode(const string& data, const QPointF& position);
	CNodeItem*                       CreateNode(tukk szTemplateName, const QPointF& position);
	CConnectionItem*                 CreateConnection(CBasePinItem& sourcePin, CBasePinItem& targetPin);

	const pfx2::IParticleEffectPfx2& GetEffectInterface() const { return m_effect; }

	CNodeItem*                       GetNodeItemById(string identifier) const;

	void                             ToggleSoloNode(CNodeItem& node);
	CNodeItem*                       GetSoloNode() const { return m_pSolorNode; }

protected:
	void       ExtractConnectionsFromNodes();

	CNodeItem* CreateNodeItem(pfx2::IParticleComponent& component);

private:
	std::vector<CNodeItem*>       m_nodes;
	std::vector<CConnectionItem*> m_connections;

	pfx2::IParticleEffectPfx2&    m_effect;

	bool                          m_isValid;
	CNodeItem*                    m_pSolorNode;
	CParticleGraphRuntimeContext  m_runtimeContext; // XXX Share between all models.
};

}

