// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptStructValue.h>

#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/IScriptStruct.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/Assert.h>

namespace sxema
{

CScriptStructValue::CScriptStructValue(const IScriptStruct* pStruct)
	: m_pStruct(pStruct)
{
	Refresh();
}

CScriptStructValue::CScriptStructValue(const CScriptStructValue& rhs)
	: m_pStruct(rhs.m_pStruct)
	, m_fields(rhs.m_fields)
{
	Refresh();
}

void CScriptStructValue::Serialize(Serialization::IArchive& archive)
{
	if (archive.isOutput()) // #SchematycTODO : Ensure we don't refresh immediately after output and invalidate strings.
	{
		Refresh();
	}
	for (FieldMap::iterator iField = m_fields.begin(), iEndField = m_fields.end(); iField != iEndField; ++iField)
	{
		if (iField->second)
		{
			tukk fieldName = iField->first.c_str();
			archive(*iField->second, fieldName, fieldName);
		}
	}
}

void CScriptStructValue::Refresh()
{
	if (m_pStruct)
	{
		// #SchematycTODO : How do we know when we really need to refresh?
		FieldMap newFields;
		for (u32 iField = 0, fieldCount = m_pStruct->GetFieldCount(); iField < fieldCount; ++iField)
		{
			tukk szFieldName = m_pStruct->GetFieldName(iField);

			CAnyConstPtr pDefaultValue = m_pStruct->GetFieldValue(iField);
			CAnyValuePtr pNewValue;
			if (pDefaultValue)
			{
				FieldMap::iterator iPrevField = m_fields.find(szFieldName);
				if ((iPrevField != m_fields.end()) && iPrevField->second && (iPrevField->second->GetTypeDesc() == pDefaultValue->GetTypeDesc()))
				{
					pNewValue = iPrevField->second;
				}
				else
				{
					pNewValue = CAnyValue::CloneShared(*pDefaultValue);
				}
			}

			newFields.insert(FieldMap::value_type(szFieldName, pNewValue));
		}
		std::swap(m_fields, newFields);
	}
	else
	{
		m_fields.clear();
	}
}

void CScriptStructValue::ReflectType(CTypeDesc<CScriptStructValue>& desc)
{
	desc.SetGUID("a9774423-c635-4b2f-96ff-d7013230aded"_drx_guid);
}

} // sxema
