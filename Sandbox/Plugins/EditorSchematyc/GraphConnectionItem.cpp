// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "GraphConnectionItem.h"

#include "GraphPinItem.h"

#include "VariableStorage/AbstractVariableTypesModel.h"

#include <DrxSchematyc/Script/IScriptGraph.h>
#include <NodeGraph/ConnectionWidget.h>

#include <QColor>

namespace DrxSchematycEditor {

CConnectionItem::CConnectionItem(Schematyc::IScriptGraphLink& scriptGraphLink, CPinItem& sourcePin, CPinItem& targetPin, DrxGraphEditor::CNodeGraphViewModel& model)
	: CAbstractConnectionItem(model)
	, m_scriptGraphLink(scriptGraphLink)
	, m_sourcePin(sourcePin)
	, m_targetPin(targetPin)
{
	// TODO: This should happen in CAbstractConnectionItem constructor.
	m_sourcePin.AddConnection(*this);
	m_targetPin.AddConnection(*this);
	// ~TODO

	const EPinType pinType = sourcePin.GetPinType();
	if (pinType == EPinType::Data)
		m_styleId = "Connection::Data";
	else if (pinType == EPinType::Execution || pinType == EPinType::Signal)
		m_styleId = "Connection::Execution";
	else
		m_styleId = "Connection";
}

CConnectionItem::~CConnectionItem()
{
	// TODO: This should happen in CAbstractConnectionItem destructor.
	m_sourcePin.RemoveConnection(*this);
	m_targetPin.RemoveConnection(*this);
	// ~TODO
}

DrxGraphEditor::CConnectionWidget* CConnectionItem::CreateWidget(DrxGraphEditor::CNodeGraphView& view)
{
	return new DrxGraphEditor::CConnectionWidget(this, view);
}

QVariant CConnectionItem::GetId() const
{
	return QVariant::fromValue(reinterpret_cast<quintptr>(&m_scriptGraphLink));
}

bool CConnectionItem::HasId(QVariant id) const
{
	const Schematyc::IScriptGraphLink* pGraphLink = reinterpret_cast<Schematyc::IScriptGraphLink*>(id.value<quintptr>());
	return (&m_scriptGraphLink == pGraphLink);
}

}

