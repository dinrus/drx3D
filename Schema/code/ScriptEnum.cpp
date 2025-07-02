// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptEnum.h>

#include <drx3D/CoreX/Serialization/BlackBox.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/SerializationUtils.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/IGUIDRemapper.h>

namespace sxema
{
CScriptEnum::CScriptEnum()
	: CScriptElementBase(EScriptElementFlags::CanOwnScript)
{}

CScriptEnum::CScriptEnum(const DrxGUID& guid, tukk szName)
	: CScriptElementBase(guid, szName, EScriptElementFlags::CanOwnScript)
{}

void CScriptEnum::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const {}

void CScriptEnum::RemapDependencies(IGUIDRemapper& guidRemapper) {}

void CScriptEnum::ProcessEvent(const SScriptEvent& event)
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

void CScriptEnum::Serialize(Serialization::IArchive& archive)
{
	LOADING_TIME_PROFILE_SECTION;

	CScriptElementBase::Serialize(archive);

	switch (SerializationContext::GetPass(archive))
	{
	case ESerializationPass::LoadDependencies:
	case ESerializationPass::Save:
		{
			archive(m_constants, "constants", "Constants");
			archive(m_userDocumentation, "userDocumentation", "Documentation");
			break;
		}
	case ESerializationPass::Edit:
		{
			archive(m_constants, "constants", "Constants");
			archive(m_userDocumentation, "userDocumentation", "Documentation");
			break;
		}
	}

	CScriptElementBase::SerializeExtensions(archive);
}

u32 CScriptEnum::GetConstantCount() const
{
	return m_constants.size();
}

u32 CScriptEnum::FindConstant(tukk szConstant) const
{
	SXEMA_CORE_ASSERT(szConstant);
	if (szConstant)
	{
		for (Constants::const_iterator itConstant = m_constants.begin(), itEndConstant = m_constants.end(); itConstant != itEndConstant; ++itConstant)
		{
			if (strcmp(itConstant->c_str(), szConstant) == 0)
			{
				return static_cast<u32>(itConstant - m_constants.begin());
			}
		}
	}
	return InvalidIdx;
}

tukk CScriptEnum::GetConstant(u32 constantIdx) const
{
	return constantIdx < m_constants.size() ? m_constants[constantIdx].c_str() : "";
}
} // sxema
