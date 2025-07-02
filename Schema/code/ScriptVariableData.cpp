// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptVariableData.h>

#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvDataType.h>
#include <drx3D/Schema/IScriptRegistry.h>
#include <drx3D/Schema/IScriptEnum.h>
#include <drx3D/Schema/IScriptStruct.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/AnyArray.h>
#include <drx3D/Schema/IGUIDRemapper.h>
#include <drx3D/Schema/StackString.h>

#include <drx3D/Schema/ScriptEnumValue.h>
#include <drx3D/Schema/ScriptStructValue.h>
#include <drx3D/Schema/ScriptView.h>

namespace sxema
{
CScriptVariableData::CScriptVariableData(const SElementId& typeId, bool bSupportsArray)
	: m_typeId(typeId)
	, m_bSupportsArray(bSupportsArray)
	, m_bIsArray(false)
{
	Refresh();
}

bool CScriptVariableData::IsEmpty() const
{
	return !m_pValue;
}

SElementId CScriptVariableData::GetTypeId() const
{
	return m_typeId;
}

tukk CScriptVariableData::GetTypeName() const
{
	switch (m_typeId.domain)
	{
	case EDomain::Env:
		{
			const IEnvDataType* pEnvDataType = gEnv->pSchematyc->GetEnvRegistry().GetDataType(m_typeId.guid);
			if (pEnvDataType)
			{
				return pEnvDataType->GetName();
			}
			break;
		}
	case EDomain::Script:
		{
			const IScriptElement* pScriptElement = gEnv->pSchematyc->GetScriptRegistry().GetElement(m_typeId.guid);
			if (pScriptElement)
			{
				return pScriptElement->GetName();
			}
			break;
		}
	}
	return "";
}

bool CScriptVariableData::SupportsArray() const
{
	return m_bSupportsArray;
}

bool CScriptVariableData::IsArray() const
{
	return m_bIsArray;
}

CAnyConstPtr CScriptVariableData::GetValue() const
{
	return m_pValue.get();
}

void CScriptVariableData::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const
{
	SXEMA_CORE_ASSERT(enumerator);
	if (enumerator)
	{
		if (m_typeId.domain == EDomain::Script)
		{
			enumerator(m_typeId.guid);
		}
	}
}

void CScriptVariableData::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	if (m_typeId.domain == EDomain::Script)
	{
		m_typeId.guid = guidRemapper.Remap(m_typeId.guid);
	}
}

void CScriptVariableData::SerializeTypeId(Serialization::IArchive& archive)
{
	archive(SerializationUtils::QuickSearch(m_typeId), "typeId", "Type");
	if (m_bSupportsArray)
	{
		archive(m_bIsArray, "bIsArray", "Array");
	}

	if (archive.isInput())
	{
		Refresh();
	}
}

void CScriptVariableData::SerializeValue(Serialization::IArchive& archive)
{
	if (archive.isInput() && !m_pValue)
	{
		Refresh();
	}

	if (m_pValue && !m_bIsArray)
	{
		archive(*m_pValue, "value", "Value");
	}
}

void CScriptVariableData::Refresh()
{
	if (!m_pValue || (m_pValue->GetTypeDesc().GetGUID() != m_typeId.guid)) // #SchematycTODO : Should we also check to see if m_bIsArray has changed?
	{
		m_pValue = m_bIsArray ? ScriptVariableData::CreateArrayData(m_typeId) : ScriptVariableData::CreateData(m_typeId);
	}
}

namespace ScriptVariableData
{
CScopedSerializationConfig::CScopedSerializationConfig(Serialization::IArchive& archive, tukk szHeader)
	: m_typeIdQuickSearchConfig(archive, szHeader ? szHeader : "Type", "::")
{}

void CScopedSerializationConfig::DeclareEnvDataTypes(const DrxGUID& scopeGUID, const EnvDataTypeFilter& filter)
{
	CScriptView scriptView(scopeGUID);

	auto visitEnvDataType = [this, &scriptView, &filter](const IEnvDataType& envDataType) -> EVisitStatus
	{
		if (!filter || filter(envDataType))
		{
			CStackString fullName;
			scriptView.QualifyName(envDataType, fullName);

			m_typeIdQuickSearchConfig.AddOption(envDataType.GetName(), SElementId(EDomain::Env, envDataType.GetGUID()), fullName.c_str(), envDataType.GetDescription());
		}
		return EVisitStatus::Continue;
	};
	scriptView.VisitEnvDataTypes(visitEnvDataType);
}

 void CScopedSerializationConfig::DeclareScriptEnums(const DrxGUID& scopeGUID, const ScriptEnumsFilter& filter)
{
	CScriptView scriptView(scopeGUID);

	auto visitScriptEnum = [this, &scriptView, &filter](const IScriptEnum& scriptEnum)
	{
		if (!filter || filter(scriptEnum))
		{
			CStackString fullName;
			scriptView.QualifyName(scriptEnum, EDomainQualifier::Local, fullName);

			m_typeIdQuickSearchConfig.AddOption(scriptEnum.GetName(), SElementId(EDomain::Script, scriptEnum.GetGUID()), fullName.c_str());
		}
	};
	scriptView.VisitAccesibleEnums(visitScriptEnum);
}

void CScopedSerializationConfig::DeclareScriptStructs(const DrxGUID& scopeGUID, const ScriptStructFilter& filter)
{
	CScriptView scriptView(scopeGUID);

	auto visitScriptStruct = [this, &scriptView, &filter](const IScriptStruct& scriptStruct)
	{
		if (!filter || filter(scriptStruct))
		{
			CStackString fullName;
			scriptView.QualifyName(scriptStruct, EDomainQualifier::Local, fullName);

			m_typeIdQuickSearchConfig.AddOption(scriptStruct.GetName(), SElementId(EDomain::Script, scriptStruct.GetGUID()), fullName.c_str());
		}
	};
	scriptView.VisitAccesibleStructs(visitScriptStruct);
}

CAnyValuePtr CreateData(const SElementId& typeId)
{
	switch (typeId.domain)
	{
	case EDomain::Env:
		{
			const IEnvDataType* pEnvDataType = gEnv->pSchematyc->GetEnvRegistry().GetDataType(typeId.guid);
			if (pEnvDataType)
			{
				return CAnyValue::MakeSharedDefault(pEnvDataType->GetDesc());
			}
			break;
		}
	case EDomain::Script:
		{
			const IScriptElement* pScriptElement = gEnv->pSchematyc->GetScriptRegistry().GetElement(typeId.guid);
			if (pScriptElement)
			{
				switch (pScriptElement->GetType())
				{
				case EScriptElementType::Enum:
					{
						return CAnyValue::MakeShared(CScriptEnumValue(DynamicCast<IScriptEnum>(pScriptElement)));
					}
				case EScriptElementType::Struct:
					{
						return CAnyValue::MakeShared(CScriptStructValue(DynamicCast<IScriptStruct>(pScriptElement)));
					}
				}
			}
			break;
		}
	}
	return CAnyValuePtr();
}

CAnyValuePtr CreateArrayData(const SElementId& typeId)
{
	switch (typeId.domain)
	{
	case EDomain::Env:
		{
			const IEnvDataType* pEnvDataType = gEnv->pSchematyc->GetEnvRegistry().GetDataType(typeId.guid);
			if (pEnvDataType)
			{
				return CAnyValue::MakeShared(CAnyArray(pEnvDataType->GetDesc()));
			}
			break;
		}
	case EDomain::Script:
		{
			const IScriptElement* pScriptElement = gEnv->pSchematyc->GetScriptRegistry().GetElement(typeId.guid);
			if (pScriptElement)
			{
				/*switch (pScriptElement->GetType())
				   {
				   case EScriptElementType::Enum:
				   {
				    return std::make_shared<CAnyArray>(GetTypeDesc<CScriptEnumValue>());
				   }
				   case EScriptElementType::Struct:
				   {
				    return std::make_shared<CAnyArray>(GetTypeDesc<IScriptStruct>());
				   }
				   }*/
			}
			break;
		}
	}
	return CAnyValuePtr();
}
} // ScriptVariableData
} // sxema
