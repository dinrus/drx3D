// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/AbstractNodeItem.h>
#include <NodeGraph/NodeWidget.h>
#include <NodeGraph/NodeHeaderIconWidget.h>
#include <NodeGraph/NodeGraphViewStyle.h>

#include "GraphPinItem.h"
#include "NodeGraphRuntimeContext.h"

#include <unordered_map>

class QPixmap;
class QIcon;

namespace Schematyc {

struct IScriptGraphNode;

}

namespace DrxSchematycEditor {

class CNodeItem : public DrxGraphEditor::CAbstractNodeItem
{
public:
	CNodeItem(Schematyc::IScriptGraphNode& scriptNode, DrxGraphEditor::CNodeGraphViewModel& model);
	virtual ~CNodeItem();

	// DrxGraphEditor::CAbstractNodeItem
	virtual DrxGraphEditor::CNodeWidget*        CreateWidget(DrxGraphEditor::CNodeGraphView& view) override;
	virtual tukk                         GetStyleId() const override;
	virtual void                                Serialize(Serialization::IArchive& archive) override;

	virtual QPointF                             GetPosition() const override;
	virtual void                                SetPosition(QPointF position) override;

	virtual QVariant                            GetId() const override;
	virtual bool                                HasId(QVariant id) const override;

	virtual QVariant                            GetTypeId() const override;

	virtual const DrxGraphEditor::PinItemArray& GetPinItems() const override    { return m_pins; };
	virtual QString                             GetName() const override        { return m_shortName; }

	virtual QString                             GetToolTipText() const override { return m_fullQualifiedName; }

	virtual DrxGraphEditor::CAbstractPinItem*   GetPinItemById(QVariant id) const;

	// DrxGraphEditor::CAbstractNodeItem

	CPinItem*                    GetPinItemById(CPinId id) const;

	Schematyc::IScriptGraphNode& GetScriptElement() const { return m_scriptNode; }
	DrxGUID                      GetGUID() const;

	bool                         IsRemovable() const;
	bool                         IsCopyAllowed() const { return true; }
	bool                         IsPasteAllowed() const;

	void                         Refresh(bool forceRefresh = false);

protected:
	void LoadFromScriptElement();
	void RefreshName();
	void Validate();

private:
	QString                      m_shortName;
	QString                      m_fullQualifiedName;
	DrxGraphEditor::PinItemArray m_pins;

	Schematyc::IScriptGraphNode& m_scriptNode;
	QColor                       m_headerTextColor;
	bool                         m_isDirty;
};

}

