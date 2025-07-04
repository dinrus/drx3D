// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "TypesDictionary.h"

#include "ScriptBrowserUtils.h"

#include <DrxSchematyc/Reflection/TypeDesc.h>

#include <DrxSchematyc/Script/IScriptView.h>
#include <DrxSchematyc/Script/IScriptRegistry.h>

#include <DrxSchematyc/Env/IEnvRegistry.h>
#include <DrxSchematyc/Env/IEnvElement.h>
#include <DrxSchematyc/Env/Elements/IEnvDataType.h>

namespace DrxSchematycEditor {

CTypeDictionaryEntry::CTypeDictionaryEntry()
{

}

CTypeDictionaryEntry::~CTypeDictionaryEntry()
{

}

QVariant CTypeDictionaryEntry::GetColumnValue(i32 columnIndex) const
{
	switch (columnIndex)
	{
	case CTypesDictionary::Column_Name:
		return QVariant::fromValue(m_name);
	default:
		break;
	}

	return QVariant();
}

CTypesDictionary::CTypesDictionary(const Schematyc::IScriptElement* pScriptScope)
{
	Load(pScriptScope);
}

CTypesDictionary::~CTypesDictionary()
{

}

const CAbstractDictionaryEntry* CTypesDictionary::GetEntry(i32 index) const
{
	if (index < m_types.size())
	{
		return static_cast<const CAbstractDictionaryEntry*>(&m_types[index]);
	}

	return nullptr;
}

QString CTypesDictionary::GetColumnName(i32 index) const
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

void CTypesDictionary::Load(const Schematyc::IScriptElement* pScriptScope)
{
	if (pScriptScope)
	{
		m_types.reserve(50);

		Schematyc::IScriptViewPtr pScriptView = gEnv->pSchematyc->CreateScriptView(pScriptScope->GetGUID());

		auto visitEnvType = [this, &pScriptView](const Schematyc::IEnvDataType& envType) -> Schematyc::EVisitStatus
		{
			Schematyc::CStackString name;
			pScriptView->QualifyName(envType, name);

			CTypeDictionaryEntry entry;
			entry.m_name = name.c_str();
			entry.m_elementId = Schematyc::SElementId(Schematyc::EDomain::Env, envType.GetGUID());
			m_types.push_back(entry);

			return Schematyc::EVisitStatus::Continue;
		};
		pScriptView->VisitEnvDataTypes(visitEnvType);

		auto visitScriptEnum = [this, &pScriptView](const Schematyc::IScriptEnum& scriptEnum)
		{
			Schematyc::CStackString name;
			pScriptView->QualifyName(scriptEnum, Schematyc::EDomainQualifier::Global, name);

			CTypeDictionaryEntry entry;
			entry.m_name = name.c_str();
			entry.m_elementId = Schematyc::SElementId(Schematyc::EDomain::Script, scriptEnum.GetGUID());
			m_types.push_back(entry);
		};
		pScriptView->VisitAccesibleEnums(visitScriptEnum);

		auto visitScriptStruct = [this, &pScriptView](const Schematyc::IScriptStruct& scriptStruct)
		{
			Schematyc::CStackString name;
			pScriptView->QualifyName(scriptStruct, Schematyc::EDomainQualifier::Global, name);

			CTypeDictionaryEntry entry;
			entry.m_name = name.c_str();
			entry.m_elementId = Schematyc::SElementId(Schematyc::EDomain::Script, scriptStruct.GetGUID());
			m_types.push_back(entry);

			return Schematyc::EVisitStatus::Continue;
		};
		//pScriptView->VisitScriptStructs(visitScriptStruct, EDomainScope::Local);
	}
}

}

