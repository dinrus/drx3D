// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptEnumValue.h>

#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/IScriptEnum.h>

namespace sxema
{

CScriptEnumValue::CScriptEnumValue(const IScriptEnum* pEnum)
	: m_pEnum(pEnum)
	, m_constantIdx(0)
{}

CScriptEnumValue::CScriptEnumValue(const CScriptEnumValue& rhs)
	: m_pEnum(rhs.m_pEnum)
	, m_constantIdx(rhs.m_constantIdx)
{}

bool CScriptEnumValue::Serialize(Serialization::IArchive& archive, tukk szName, tukk szLabel)
{
	if (m_pEnum)
	{
		if (archive.caps(archive.INPLACE))
		{
			archive(m_constantIdx, szName);
		}
		else if (archive.isEdit())
		{
			Serialization::StringList constants;
			for (u32 constantIdx = 0, constantCount = m_pEnum->GetConstantCount(); constantIdx < constantCount; ++constantIdx)
			{
				constants.push_back(m_pEnum->GetConstant(constantIdx));
			}
			if (archive.isInput())
			{
				Serialization::StringListValue constant(constants, 0);
				archive(constant, szName, szLabel);
				m_constantIdx = constant.index();
			}
			else if (archive.isOutput())
			{
				Serialization::StringListValue constant(constants, m_constantIdx);
				archive(constant, szName, szLabel);
			}
		}
		else
		{
			if (archive.isInput())
			{
				string constant;
				archive(constant, szName);
				m_constantIdx = m_pEnum->FindConstant(constant.c_str());
			}
			else if (archive.isOutput())
			{
				string constant = m_pEnum->GetConstant(m_constantIdx);
				archive(constant, szName);
			}
		}
	}
	return true;
}

void CScriptEnumValue::ToString(IString& output) const
{
	if (m_pEnum)
	{
		output.assign(m_pEnum->GetConstant(m_constantIdx));
	}
}

void CScriptEnumValue::ReflectType(CTypeDesc<CScriptEnumValue>& desc)
{
	desc.SetGUID("6353d2e0-683d-424d-a4d9-16c4d6e350f9"_drx_guid);
	desc.SetToStringOperator<&CScriptEnumValue::ToString>();
}

bool Serialize(Serialization::IArchive& archive, CScriptEnumValue& value, tukk szName, tukk szLabel)
{
	return value.Serialize(archive, szName, szLabel);
}

} // sxema
