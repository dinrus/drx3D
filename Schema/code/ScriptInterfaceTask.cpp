// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptInterfaceTask.h>

#include <drx3D/CoreX/Serialization/BlackBox.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvInterface.h>
#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/SerializationUtils.h>
#include <drx3D/Schema/IGUIDRemapper.h>

namespace sxema
{
CScriptInterfaceTask::CScriptInterfaceTask() {}

CScriptInterfaceTask::CScriptInterfaceTask(const DrxGUID& guid, tukk szName)
	: CScriptElementBase(guid, szName)
{}

void CScriptInterfaceTask::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const {}

void CScriptInterfaceTask::RemapDependencies(IGUIDRemapper& guidRemapper)                                                        {}

void CScriptInterfaceTask::ProcessEvent(const SScriptEvent& event)
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

void CScriptInterfaceTask::Serialize(Serialization::IArchive& archive)
{
	// #SchematycTODO : Shouldn't this be handled by CScriptElementBase itself?
	CScriptElementBase::Serialize(archive);
	CMultiPassSerializer::Serialize(archive);
	CScriptElementBase::SerializeExtensions(archive);
}

tukk CScriptInterfaceTask::GetAuthor() const
{
	return m_userDocumentation.author.c_str();
}

tukk CScriptInterfaceTask::GetDescription() const
{
	return m_userDocumentation.description.c_str();
}

void CScriptInterfaceTask::Load(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_userDocumentation, "userDocumentation");
}

void CScriptInterfaceTask::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_userDocumentation, "userDocumentation");
}

void CScriptInterfaceTask::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_userDocumentation, "userDocumentation", "Documentation");
}
} // sxema
