// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptInterface.h>

#include <drx3D/CoreX/Serialization/BlackBox.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/SerializationUtils.h>
#include <drx3D/Schema/IGUIDRemapper.h>

namespace sxema
{
CScriptInterface::CScriptInterface()
	: CScriptElementBase(EScriptElementFlags::CanOwnScript)
{}

CScriptInterface::CScriptInterface(const DrxGUID& guid, tukk szName)
	: CScriptElementBase(guid, szName, EScriptElementFlags::CanOwnScript)
{}

void CScriptInterface::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const {}

void CScriptInterface::RemapDependencies(IGUIDRemapper& guidRemapper) {}

void CScriptInterface::ProcessEvent(const SScriptEvent& event)
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

void CScriptInterface::Serialize(Serialization::IArchive& archive)
{
	// #SchematycTODO : Shouldn't this be handled by CScriptElementBase itself?
	CScriptElementBase::Serialize(archive);
	CMultiPassSerializer::Serialize(archive);
	CScriptElementBase::SerializeExtensions(archive);
}

tukk CScriptInterface::GetAuthor() const
{
	return m_userDocumentation.author.c_str();
}

tukk CScriptInterface::GetDescription() const
{
	return m_userDocumentation.description.c_str();
}

void CScriptInterface::Load(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_userDocumentation, "userDocumentation");
}

void CScriptInterface::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_userDocumentation, "userDocumentation");
}

void CScriptInterface::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_userDocumentation, "userDocumentation", "Documentation");
}
} // sxema
