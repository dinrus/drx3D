// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "StateMachineItem.h"

#include "ScriptBrowserUtils.h"
#include "ObjectModel.h"

#include <DrxSchematyc/Script/Elements/IScriptStateMachine.h>
#include <DrxSchematyc/Script/Elements/IScriptState.h>
#include <DrxSchematyc/SerializationUtils/ISerializationContext.h>

#include <StateItem.h>

#include <DrxIcon.h>
#include <QtUtil.h>

namespace DrxSchematycEditor {

CStateMachineItem::CStateMachineItem(Schematyc::IScriptStateMachine& scriptStateMachine, CAbstractObjectStructureModel& model)
	: CAbstractObjectStructureModelItem(model)
	, m_scriptStateMachine(scriptStateMachine)
	, m_pParentItem(nullptr)
{
	m_name = m_scriptStateMachine.GetName();
	LoadFromScriptElement();
}

CStateMachineItem::~CStateMachineItem()
{
	for (CStateItem* pItem : m_states)
		delete pItem;
}

void CStateMachineItem::SetName(QString name)
{
	Schematyc::CStackString uniqueName = QtUtil::ToString(name).c_str();
	Schematyc::ScriptBrowserUtils::MakeScriptElementNameUnique(uniqueName, m_model.GetScriptElement());

	m_scriptStateMachine.SetName(uniqueName);
	m_name = m_scriptStateMachine.GetName();
}

const DrxIcon* CStateMachineItem::GetIcon() const
{
	std::unique_ptr<DrxIcon> pIcon;
	if (pIcon.get() == nullptr)
	{
		pIcon = stl::make_unique<DrxIcon>("icons:schematyc/script_state_machine.png");
	}

	return pIcon.get();
}

CAbstractObjectStructureModelItem* CStateMachineItem::GetChildItemByIndex(u32 index) const
{
	if (index < m_states.size())
	{
		return m_states[index];
	}
	return nullptr;
}

u32 CStateMachineItem::GetChildItemIndex(const CAbstractObjectStructureModelItem& item) const
{
	if (item.GetType() == eObjectItemType_State)
	{
		u32 index = 0;
		for (const CStateItem* pItem : m_states)
		{
			if (pItem == &item)
				return index;
			++index;
		}
	}

	return 0xffffffff;
}

u32 CStateMachineItem::GetIndex() const
{
	if (m_pParentItem)
	{
		return m_pParentItem->GetChildItemIndex(*this);
	}
	else
	{
		return GetModel().GetChildItemIndex(*this);
	}

	return 0xffffffff;
}

void CStateMachineItem::Serialize(Serialization::IArchive& archive)
{
	// TODO: This will only work for serialization to properties in inspector!
	Schematyc::SSerializationContextParams serParams(archive, Schematyc::ESerializationPass::Edit);
	Schematyc::ISerializationContextPtr pSerializationContext = gEnv->pSchematyc->CreateSerializationContext(serParams);
	// ~TODO

	m_scriptStateMachine.Serialize(archive);
}

bool CStateMachineItem::AllowsRenaming() const
{
	const bool allowsRenaming = !m_scriptStateMachine.GetFlags().Check(Schematyc::EScriptElementFlags::FixedName);
	return allowsRenaming;
}

CStateItem* CStateMachineItem::GetStateItemByIndex(u32 index) const
{
	if (index < m_states.size())
	{
		return m_states[index];
	}
	return nullptr;
}

CStateItem* CStateMachineItem::CreateState()
{
	Schematyc::CStackString name = "State";
	Schematyc::ScriptBrowserUtils::MakeScriptElementNameUnique(name, m_model.GetScriptElement());

	Schematyc::IScriptState* pStateElement = gEnv->pSchematyc->GetScriptRegistry().AddState(name, m_model.GetScriptElement());
	if (pStateElement)
	{
		CStateItem* pStateItem = new CStateItem(*pStateElement, m_model);
		pStateItem->SetParentItem(this);

		m_states.push_back(pStateItem);
		GetModel().SignalObjectStructureItemAdded(*pStateItem);

		return pStateItem;
	}

	return nullptr;
}

bool CStateMachineItem::RemoveState()
{
	// TODO: Missing implementation.
	DRX_ASSERT_MESSAGE(false, "Missing impl.");
	return false;
}

DrxGUID CStateMachineItem::GetGUID() const
{
	return m_scriptStateMachine.GetGUID();
}

void CStateMachineItem::LoadFromScriptElement()
{
	m_name = m_scriptStateMachine.GetName();

	Schematyc::IScriptElement* pElement = m_scriptStateMachine.GetFirstChild();
	while (pElement)
	{
		const Schematyc::EScriptElementType elementType = pElement->GetType();
		switch (elementType)
		{
		case Schematyc::EScriptElementType::State:
			{
				Schematyc::IScriptState& scriptState = static_cast<Schematyc::IScriptState&>(*pElement);
				CStateItem* pStateItem = new CStateItem(scriptState, m_model);
				pStateItem->SetParentItem(this);
				m_states.push_back(pStateItem);
			}
			break;
		default:
			break;   // Something unexpected!
		}

		pElement = pElement->GetNextSibling();
	}
}

}

