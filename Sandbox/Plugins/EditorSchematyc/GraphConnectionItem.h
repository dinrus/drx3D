// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/AbstractConnectionItem.h>

#include "GraphPinItem.h"

class QColor;

namespace Schematyc {

struct IScriptGraphLink;

}

namespace DrxSchematycEditor {

class CConnectionItem : public DrxGraphEditor::CAbstractConnectionItem
{
public:
	CConnectionItem(Schematyc::IScriptGraphLink& scriptGraphLink, CPinItem& sourcePin, CPinItem& targetPin, DrxGraphEditor::CNodeGraphViewModel& model);
	virtual ~CConnectionItem();

	// DrxGraphEditor::CAbstractConnectionItem
	virtual DrxGraphEditor::CConnectionWidget* CreateWidget(DrxGraphEditor::CNodeGraphView& view) override;
	virtual tukk                        GetStyleId() const                { return m_styleId; }

	virtual DrxGraphEditor::CAbstractPinItem&  GetSourcePinItem() const override { return m_sourcePin; }
	virtual DrxGraphEditor::CAbstractPinItem&  GetTargetPinItem() const override { return m_targetPin; }

	virtual QVariant                           GetId() const override;
	virtual bool                               HasId(QVariant id) const override;
	// ~DrxGraphEditor::CAbstractConnectionItem

	Schematyc::IScriptGraphLink& GetScriptLink() const { return m_scriptGraphLink; }

private:
	Schematyc::IScriptGraphLink& m_scriptGraphLink;
	CPinItem&                    m_sourcePin;
	CPinItem&                    m_targetPin;

	tukk                  m_styleId;
};

}

