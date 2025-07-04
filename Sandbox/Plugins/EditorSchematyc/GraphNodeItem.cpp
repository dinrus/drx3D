// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"

#include "GraphNodeItem.h"
#include "GraphPinItem.h"
#include "GraphViewModel.h"

#include <DrxSchematyc/Script/IScriptGraph.h>

#include <NodeGraph/NodeWidget.h>
#include <NodeGraph/PinGridNodeContentWidget.h>
#include <NodeGraph/NodeGraphUndo.h>

#include <QString>

namespace DrxSchematycEditor {

CNodeItem::CNodeItem(Schematyc::IScriptGraphNode& scriptNode, DrxGraphEditor::CNodeGraphViewModel& model)
	: CAbstractNodeItem(model)
	, m_scriptNode(scriptNode)
	, m_isDirty(false)
{
	SetAcceptsRenaming(false);
	SetAcceptsDeactivation(false);

	LoadFromScriptElement();
}

CNodeItem::~CNodeItem()
{
	for (DrxGraphEditor::CAbstractPinItem* pItem : m_pins)
		delete pItem;
}

DrxGraphEditor::CNodeWidget* CNodeItem::CreateWidget(DrxGraphEditor::CNodeGraphView& view)
{
	DrxGraphEditor::CNodeWidget* pNodeWidget = new DrxGraphEditor::CNodeWidget(*this, view);
	DrxGraphEditor::CPinGridNodeContentWidget* pContent = new DrxGraphEditor::CPinGridNodeContentWidget(*pNodeWidget, view);

	return pNodeWidget;
}

void CNodeItem::Serialize(Serialization::IArchive& archive)
{
	Schematyc::ESerializationPass serPass;
	if (archive.isEdit())
	{
		serPass = Schematyc::ESerializationPass::Edit;
	}
	else
	{
		serPass = archive.isInput() ? Schematyc::ESerializationPass::Load : Schematyc::ESerializationPass::Save;
	}

	if (serPass == Schematyc::ESerializationPass::Load)
	{
		Schematyc::SSerializationContextParams serializationContextParams(archive, Schematyc::ESerializationPass::LoadDependencies);
		Schematyc::ISerializationContextPtr pSerializationContext = gEnv->pSchematyc->CreateSerializationContext(serializationContextParams);

		m_scriptNode.Serialize(archive);
	}

	Schematyc::SSerializationContextParams serializationContextParams(archive, serPass);
	Schematyc::ISerializationContextPtr pSerializationContext = gEnv->pSchematyc->CreateSerializationContext(serializationContextParams);

	if (archive.isOutput() && archive.isEdit())
	{
		Refresh(true);

		// TODO: This should happen in Serialize(...) function not here.
		gEnv->pSchematyc->GetScriptRegistry().ElementModified(m_scriptNode.GetGraph().GetElement());
		// ~TODO
	}

	m_scriptNode.Serialize(archive);

	if (archive.isInput())
	{
		// We only want to do an immediate refresh if the change doesn't come from
		// the editor. Otherwise we wait for the output serialization pass.
		if (!archive.isEdit())
		{
			Refresh(true);

			// TODO: This should happen in Serialize(...) function not here.
			gEnv->pSchematyc->GetScriptRegistry().ElementModified(m_scriptNode.GetGraph().GetElement());
			// ~TODO
		}

		Validate();
	}
}

QPointF CNodeItem::GetPosition() const
{
	Vec2 pos = m_scriptNode.GetPos();
	return QPointF(pos.x, pos.y);
}

void CNodeItem::SetPosition(QPointF position)
{
	Vec2 pos(position.x(), position.y());
	m_scriptNode.SetPos(pos);

	CAbstractNodeItem::SetPosition(position);
	if (GetIEditor()->GetIUndoManager()->IsUndoRecording())
	{
		DrxGraphEditor::CUndoNodeMove* pUndoObject = new DrxGraphEditor::CUndoNodeMove(*this);
		CUndo::Record(pUndoObject);
	}
}

QVariant CNodeItem::GetId() const
{
	return QVariant::fromValue(m_scriptNode.GetGUID());
}

bool CNodeItem::HasId(QVariant id) const
{
	return (id.value<DrxGUID>() == m_scriptNode.GetGUID());
}

QVariant CNodeItem::GetTypeId() const
{
	return QVariant::fromValue(m_scriptNode.GetTypeGUID());
}

DrxGraphEditor::CAbstractPinItem* CNodeItem::GetPinItemById(QVariant id) const
{
	const CPinId pinId = id.value<CPinId>();
	return GetPinItemById(pinId);
}

CPinItem* CNodeItem::GetPinItemById(CPinId id) const
{
	if (id.IsInput())
	{
		for (DrxGraphEditor::CAbstractPinItem* pAbstractPinItem : m_pins)
		{
			CPinItem* pPinItem = static_cast<CPinItem*>(pAbstractPinItem);
			if (pPinItem->IsInputPin() && pPinItem->GetPortId() == id.GetPortId())
			{
				return pPinItem;
			}
		}
	}
	else
	{
		for (DrxGraphEditor::CAbstractPinItem* pAbstractPinItem : m_pins)
		{
			CPinItem* pPinItem = static_cast<CPinItem*>(pAbstractPinItem);
			if (pPinItem->IsOutputPin() && pPinItem->GetPortId() == id.GetPortId())
			{
				return pPinItem;
			}
		}
	}

	return nullptr;
}

DrxGUID CNodeItem::GetGUID() const
{
	return m_scriptNode.GetGUID();
}

tukk CNodeItem::GetStyleId() const
{
	return m_scriptNode.GetStyleId();
}

bool CNodeItem::IsRemovable() const
{
	return !m_scriptNode.GetFlags().Check(Schematyc::EScriptGraphNodeFlags::NotRemovable);
}

bool CNodeItem::IsPasteAllowed() const
{
	return !m_scriptNode.GetFlags().Check(Schematyc::EScriptGraphNodeFlags::NotCopyable);
}

void CNodeItem::Refresh(bool forceRefresh)
{
	// TODO: This is a workaround for broken mappings after undo.
	m_scriptNode.GetGraph().FixMapping(m_scriptNode);
	// ~TODO

	if (m_isDirty || forceRefresh)
	{
		m_isDirty = false;

		// TODO: We shouldn't have to do this here but it's the only way to refresh the whole node for now!
		m_scriptNode.ProcessEvent(Schematyc::SScriptEvent(Schematyc::EScriptEventId::EditorPaste));
		// ~TODO

		RefreshName();

		// We iterate through all pins and check what was removed/added.
		u32k numPins = m_scriptNode.GetInputCount() + m_scriptNode.GetOutputCount();

		DrxGraphEditor::PinItemArray pins;
		pins.reserve(numPins);

		u32k numInputPins = m_scriptNode.GetInputCount();
		for (u32 i = 0; i < numInputPins; ++i)
		{
			CPinId pinId(m_scriptNode.GetInputId(i), CPinId::EType::Input);
			auto predicate = [pinId](DrxGraphEditor::CAbstractPinItem* pAbstractPinItem) -> bool
			{
				CPinItem* pPinItem = static_cast<CPinItem*>(pAbstractPinItem);
				return (pPinItem && pinId == pPinItem->GetPinId());
			};

			auto result = std::find_if(m_pins.begin(), m_pins.end(), predicate);
			if (result == m_pins.end())
			{
				CPinItem* pPinItem = new CPinItem(i, EPinFlag::Input, *this, GetViewModel());
				pins.push_back(pPinItem);

				SignalPinAdded(*pPinItem);
			}
			else
			{
				CPinItem* pPinItem = static_cast<CPinItem*>(*result);
				pins.push_back(pPinItem);

				*result = nullptr;
			}
		}

		u32k numOutputPins = m_scriptNode.GetOutputCount();
		for (u32 i = 0; i < numOutputPins; ++i)
		{
			CPinId pinId(m_scriptNode.GetOutputId(i), CPinId::EType::Output);
			auto predicate = [pinId](DrxGraphEditor::CAbstractPinItem* pAbstractPinItem) -> bool
			{
				CPinItem* pPinItem = static_cast<CPinItem*>(pAbstractPinItem);
				return (pPinItem && pinId == pPinItem->GetPinId());
			};

			auto result = std::find_if(m_pins.begin(), m_pins.end(), predicate);
			if (result == m_pins.end())
			{
				CPinItem* pPinItem = new CPinItem(i, EPinFlag::Output, *this, GetViewModel());
				pins.push_back(pPinItem);

				SignalPinAdded(*pPinItem);
			}
			else
			{
				CPinItem* pPinItem = static_cast<CPinItem*>(*result);
				pins.push_back(pPinItem);

				*result = nullptr;
			}
		}

		m_pins.swap(pins);
		DRX_ASSERT(m_pins.size() == numPins);

		for (DrxGraphEditor::CAbstractPinItem* pPinItem : pins)
		{
			if (pPinItem != nullptr)
			{
				// TODO: We need to destroy the connections since Schematyc backend did so already without
				//			 notifying the editor. Remove that as soon as we have proper communication between
				//			 editor and backend.
				pPinItem->Disconnect();
				// ~TODO

				SignalPinRemoved(*pPinItem);
				delete pPinItem;
			}
		}

		size_t inputIdx = 0;
		size_t outputIdx = 0;
		for (u32 i = 0; i < m_pins.size(); ++i)
		{
			CPinItem* pPinItem = static_cast<CPinItem*>(m_pins.at(i));
			if (pPinItem->IsInputPin())
				pPinItem->UpdateWithNewIndex(inputIdx++);
			else
				pPinItem->UpdateWithNewIndex(outputIdx++);

			pPinItem->SignalInvalidated();
		}
	}
}

void CNodeItem::LoadFromScriptElement()
{
	// TODO: Just call RefreshLayout(...) here?!
	m_scriptNode.ProcessEvent(Schematyc::SScriptEvent(Schematyc::EScriptEventId::EditorRefresh));
	// N.B. After the release of 5.3 we should figure out whether it's really necessary to refresh elements upon initial selection (as opposed to only refreshing when changes are made).
	// ~TODO

	RefreshName();

	const Vec2 pos = m_scriptNode.GetPos();
	SetPosition(QPoint(pos.x, pos.y));

	const QString styleId(GetStyleId());

	u32k numInputs = m_scriptNode.GetInputCount();
	for (u32 i = 0; i < numInputs; ++i)
	{
		DrxGraphEditor::CAbstractPinItem* pPinItem = new CPinItem(i, EPinFlag::Input, *this, GetViewModel());
		m_pins.push_back(pPinItem);
	}

	u32k numOutputs = m_scriptNode.GetOutputCount();
	for (u32 i = 0; i < numOutputs; ++i)
	{
		DrxGraphEditor::CAbstractPinItem* pPinItem = new CPinItem(i, EPinFlag::Output, *this, GetViewModel());
		m_pins.push_back(pPinItem);
	}

	SetAcceptsDeletion(IsRemovable());
	SetAcceptsCopy(IsCopyAllowed());
	SetAcceptsPaste(IsPasteAllowed());

	Validate();
}

void CNodeItem::RefreshName()
{
	m_shortName = m_scriptNode.GetName();

	CNodeItem::SignalNameChanged();
}

void CNodeItem::Validate()
{
	u32 warningCount = 0;
	u32 errorCount = 0;

	auto validateScriptNode = [&warningCount, &errorCount](Schematyc::EValidatorMessageType messageType, tukk szMessage)
	{
		switch (messageType)
		{
		case Schematyc::EValidatorMessageType::Warning:
			{
				++warningCount;
				break;
			}
		case Schematyc::EValidatorMessageType::Error:
			{
				++errorCount;
				break;
			}
		}
	};
	m_scriptNode.Validate(validateScriptNode);

	CAbstractNodeItem::SetWarnings(warningCount > 0);
	CAbstractNodeItem::SetErrors(errorCount > 0);

	SignalValidated(*this);
}

}

