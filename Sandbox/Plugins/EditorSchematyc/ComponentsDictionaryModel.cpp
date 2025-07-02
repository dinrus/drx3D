// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ComponentsDictionaryModel.h"

#include "ScriptBrowserUtils.h"

#include <DrxSchematyc/Reflection/TypeDesc.h>

#include <DrxSchematyc/Script/IScriptView.h>
#include <DrxSchematyc/Script/IScriptRegistry.h>
#include <DrxSchematyc/Script/Elements/IScriptComponentInstance.h>

#include <DrxSchematyc/Env/IEnvRegistry.h>
#include <DrxSchematyc/Env/IEnvElement.h>
#include <DrxSchematyc/Env/Elements/IEnvComponent.h>

namespace DrxSchematycEditor {

CComponentDictionaryEntry::CComponentDictionaryEntry()
{
}

CComponentDictionaryEntry::~CComponentDictionaryEntry()
{

}

QVariant CComponentDictionaryEntry::GetColumnValue(i32 columnIndex) const
{
	switch (columnIndex)
	{
	case CComponentsDictionary::Column_Name:
		return QVariant::fromValue(m_name);
	default:
		break;
	}

	return QVariant();
}

QString CComponentDictionaryEntry::GetToolTip() const
{
	return m_description;
}

CComponentsDictionary::CComponentsDictionary(const Schematyc::IScriptElement* pScriptScope)
{
	if (pScriptScope)
		Load(pScriptScope);
}

CComponentsDictionary::~CComponentsDictionary()
{

}

const CAbstractDictionaryEntry* CComponentsDictionary::GetEntry(i32 index) const
{
	if (index < m_components.size())
	{
		return &m_components[index];
	}

	return nullptr;
}

QString CComponentsDictionary::GetColumnName(i32 index) const
{
	switch (index)
	{
	case Column_Name:
		return QString("Name");
	default:
		break;
	}

	return QString();
}

void CComponentsDictionary::Load(const Schematyc::IScriptElement* pScriptScope)
{
	if (pScriptScope)
	{
		m_components.reserve(100);

		bool bAttach = false;

		const Schematyc::IScriptComponentInstance* pScriptComponentInstance = Schematyc::DynamicCast<Schematyc::IScriptComponentInstance>(pScriptScope);
		if (pScriptComponentInstance)
		{
			Schematyc::IEnvRegistry& registry = gEnv->pSchematyc->GetEnvRegistry();
			const Schematyc::IEnvComponent* pEnvComponent = registry.GetComponent(pScriptComponentInstance->GetTypeGUID());
			if (pEnvComponent)
			{
				if (pEnvComponent->GetDesc().GetComponentFlags().Check(IEntityComponent::EFlags::Socket))
				{
					bAttach = true;
				}
				else
				{
					return;
				}
			}
		}

		Schematyc::IScriptViewPtr pScriptView = gEnv->pSchematyc->CreateScriptView(pScriptScope->GetGUID());

		VectorSet<DrxGUID> singletonExclusions;
		auto visitScriptComponentInstance = [this, &singletonExclusions](const Schematyc::IScriptComponentInstance& scriptComponentInstance) -> Schematyc::EVisitStatus
		{
			singletonExclusions.insert(scriptComponentInstance.GetTypeGUID());
			return Schematyc::EVisitStatus::Continue;
		};
		pScriptView->VisitScriptComponentInstances(visitScriptComponentInstance, Schematyc::EDomainScope::Derived);

		auto visitEnvComponentFactory = [this, bAttach, &singletonExclusions, &pScriptView](const Schematyc::IEnvComponent& envComponent) -> Schematyc::EVisitStatus
		{
			auto componentFlags = envComponent.GetDesc().GetComponentFlags();
			if (!bAttach || componentFlags.Check(IEntityComponent::EFlags::Attach))
			{
				const DrxGUID envComponentGUID = envComponent.GetGUID();
				if (!componentFlags.Check(IEntityComponent::EFlags::Singleton) || (singletonExclusions.find(envComponentGUID) == singletonExclusions.end()))
				{
					Schematyc::CStackString fullName;
					pScriptView->QualifyName(envComponent, fullName);

					CComponentDictionaryEntry entry;
					entry.m_identifier = envComponentGUID;
					entry.m_name = envComponent.GetName();
					entry.m_fullName = fullName.c_str();
					entry.m_description = envComponent.GetDescription();

					m_components.emplace_back(entry);
				}
			}
			return Schematyc::EVisitStatus::Continue;
		};
		pScriptView->VisitEnvComponents(visitEnvComponentFactory);
	}
}

}

