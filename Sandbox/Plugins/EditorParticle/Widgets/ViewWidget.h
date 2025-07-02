// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/IDrxGraphEditor.h>
#include <NodeGraph/NodeGraphView.h>

#include <QWidget>

class QAdvancedPropertyTree;

namespace DrxParticleEditor {

class CFeatureWidget;
class CNodeItem;
class CFeatureItem;

struct SFeatureMouseEventArgs : public DrxGraphEditor::SMouseInputEventArgs
{
	SFeatureMouseEventArgs(DrxGraphEditor::SMouseInputEventArgs& eventArgs)
		: DrxGraphEditor::SMouseInputEventArgs(static_cast<DrxGraphEditor::SMouseInputEventArgs &&>(eventArgs))
	{}
};

// TODO: Move this into a separate source file.
class CItemProperties : public QWidget
{
	Q_OBJECT

	struct SFeatureSerializer
	{
		CFeatureItem* pItem;
		string        name;

		SFeatureSerializer()
			: pItem(nullptr)
		{}

		void Serialize(Serialization::IArchive& archive);
	};

public:
	CItemProperties(DrxGraphEditor::GraphItemSet& selectedItems);
	~CItemProperties();

	void Serialize(Serialization::IArchive& archive);

protected:
	virtual void showEvent(QShowEvent* pEvent) override;

	void         OnPushUndo();
	void         OnItemsChanged();
	void         OnItemsDeletion();

private:
	QAdvancedPropertyTree*          m_pPropertyTree;
	Serialization::SStructs         m_structs;

	CNodeItem*                      m_pNodeItem;
	std::vector<SFeatureSerializer> m_features;
	bool                            m_isPushingUndo;
};
// ~TODO

class CGraphView : public DrxGraphEditor::CNodeGraphView
{
	enum ECustomAction : u32
	{
		eCustomAction_MoveFeature = CNodeGraphView::eAction_UserAction,
	};

public:
	CGraphView();
	~CGraphView();

	void OnFeatureMouseEvent(QGraphicsItem* pSender, SFeatureMouseEventArgs& args);

private:
	// DrxGraphEditor::CNodeGraphView
	virtual QWidget* CreatePropertiesWidget(DrxGraphEditor::GraphItemSet& selectedItems) override;

	virtual bool     PopulateNodeContextMenu(DrxGraphEditor::CAbstractNodeItem& node, QMenu& menu) override;

	virtual bool     DeleteCustomItem(DrxGraphEditor::CAbstractNodeGraphViewModelItem& item) override;
	virtual void     OnRemoveCustomItem(DrxGraphEditor::CAbstractNodeGraphViewModelItem& item) override;
	// ~DrxGraphEditor::CNodeGraphView

	void ShowFeatureContextMenu(CFeatureWidget* pFeatureWidget, QPointF screenPos);
	bool PopulateMenuWithFeatures(tukk szTitle, DrxGraphEditor::CAbstractNodeItem& node, QMenu& menu, u32 index = ~0);

	bool MoveFeatureToIndex(CFeatureWidget& featureWidget, u32 destIndex);

private:
	CFeatureWidget* m_pMovingFeatureWidget;
};

}

