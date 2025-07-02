// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptVariable.h>

#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvInterface.h>
#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/SerializationUtils.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/IGUIDRemapper.h>

namespace sxema
{
CScriptVariable::CScriptVariable()
	: m_data(SElementId(), true)
{}

CScriptVariable::CScriptVariable(const DrxGUID& guid, tukk szName, const SElementId& typeId, const DrxGUID& baseGUID)
	: CScriptElementBase(guid, szName)
	, m_data(typeId, true)
	, m_baseGUID(baseGUID)
{}

EScriptElementAccessor CScriptVariable::GetAccessor() const
{
	return m_accessor;
}

void CScriptVariable::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const
{
	SXEMA_CORE_ASSERT(enumerator);
	if (enumerator)
	{
		m_data.EnumerateDependencies(enumerator, type);

		if (!GUID::IsEmpty(m_baseGUID))
		{
			enumerator(m_baseGUID);
		}

		CScriptElementBase::EnumerateDependencies(enumerator, type);
	}
}

void CScriptVariable::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	CScriptElementBase::RemapDependencies(guidRemapper);

	m_data.RemapDependencies(guidRemapper);

	m_baseGUID = guidRemapper.Remap(m_baseGUID);
}

void CScriptVariable::ProcessEvent(const SScriptEvent& event) {}

void CScriptVariable::Serialize(Serialization::IArchive& archive)
{
	// #SchematycTODO : Shouldn't this be handled by CScriptElementBase itself?
	CScriptElementBase::Serialize(archive);
	CMultiPassSerializer::Serialize(archive);
	CScriptElementBase::SerializeExtensions(archive);
}

SElementId CScriptVariable::GetTypeId() const
{
	return m_data.GetTypeId();
}

bool CScriptVariable::IsArray() const
{
	return m_data.IsArray();
}

CAnyConstPtr CScriptVariable::GetData() const
{
	return m_data.GetValue();
}

DrxGUID CScriptVariable::GetBaseGUID() const
{
	return m_baseGUID;
}

EOverridePolicy CScriptVariable::GetOverridePolicy() const
{
	return m_overridePolicy;
}

void CScriptVariable::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	m_data.SerializeTypeId(archive);
	archive(m_baseGUID, "baseGUID");
}

void CScriptVariable::Load(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_accessor, "accessor");
	m_data.SerializeValue(archive);
	archive(m_overridePolicy, "overridePolicy");
}

void CScriptVariable::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_accessor, "accessor");
	m_data.SerializeTypeId(archive);
	m_data.SerializeValue(archive);
	archive(m_baseGUID, "baseGUID");
	archive(m_overridePolicy, "overridePolicy");
}

void CScriptVariable::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	{
		ScriptVariableData::CScopedSerializationConfig serializationConfig(archive);

		const DrxGUID guid = CScriptElementBase::GetGUID();
		serializationConfig.DeclareEnvDataTypes(guid);
		serializationConfig.DeclareScriptEnums(guid);
		serializationConfig.DeclareScriptStructs(guid);

		m_data.SerializeTypeId(archive);
	}

	bool bPublic = m_accessor == EScriptElementAccessor::Public;
	archive(bPublic, "bPublic", "Public");
	if (archive.isInput())
	{
		m_accessor = bPublic ? EScriptElementAccessor::Public : EScriptElementAccessor::Private;
	}
	
	const bool bDerived = !GUID::IsEmpty(m_baseGUID);
	if (bDerived)
	{
		archive(m_overridePolicy, "overridePolicy", "Override");
	}
	if (!bDerived || (m_overridePolicy != EOverridePolicy::Default))
	{
		m_data.SerializeValue(archive);
	}
}
} // sxema
