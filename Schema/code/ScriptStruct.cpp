// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptStruct.h>

#include <drx3D/CoreX/Serialization/BlackBox.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/SerializationUtils.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/Assert.h>

namespace sxema
{
CScriptStruct::CScriptStruct()
	: CScriptElementBase(EScriptElementFlags::CanOwnScript)
{}

CScriptStruct::CScriptStruct(const DrxGUID& guid, tukk szName)
	: CScriptElementBase(guid, szName, EScriptElementFlags::CanOwnScript)
{}

void CScriptStruct::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const
{
	ScriptParam::EnumerateDependencies(m_fields, enumerator, type);
}

void CScriptStruct::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	ScriptParam::RemapDependencies(m_fields, guidRemapper);
}

void CScriptStruct::ProcessEvent(const SScriptEvent& event)
{
	CScriptElementBase::ProcessEvent(event);

	switch (event.id)
	{
	case EScriptEventId::EditorAdd:
	case EScriptEventId::EditorPaste:
		{
			m_userDocumentation.SetCurrentUserAsAuthor();
			break;
		}
	}
}

void CScriptStruct::Serialize(Serialization::IArchive& archive)
{
	// #SchematycTODO : Shouldn't this be handled by CScriptElementBase itself?
	CScriptElementBase::Serialize(archive);
	CMultiPassSerializer::Serialize(archive);
	CScriptElementBase::SerializeExtensions(archive);
}

u32 CScriptStruct::GetFieldCount() const
{
	return m_fields.size();
}

tukk CScriptStruct::GetFieldName(u32 fieldIdx) const
{
	return fieldIdx < m_fields.size() ? m_fields[fieldIdx].name : "";
}

CAnyConstPtr CScriptStruct::GetFieldValue(u32 fieldIdx) const
{
	return fieldIdx < m_fields.size() ? m_fields[fieldIdx].data.GetValue() : CAnyConstPtr();
}

void CScriptStruct::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_fields, "fields");
}

void CScriptStruct::Load(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_fields, "fields");
	archive(m_userDocumentation, "userDocumentation", "Documentation");
}

void CScriptStruct::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_fields, "fields");
	archive(m_userDocumentation, "userDocumentation", "Documentation");
}

void CScriptStruct::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	{
		ScriptVariableData::CScopedSerializationConfig serializationConfig(archive);

		const DrxGUID guid = GetGUID();
		serializationConfig.DeclareEnvDataTypes(guid);
		serializationConfig.DeclareScriptEnums(guid);

		archive(m_fields, "fields", "Fields");
	}

	archive(m_userDocumentation, "userDocumentation", "Documentation");
}
} // sxema
