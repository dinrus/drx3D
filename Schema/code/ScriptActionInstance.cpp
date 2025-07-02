// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptActionInstance.h>

#include <drx3D/CoreX/Serialization/BlackBox.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvAction.h>
#include <drx3D/Schema/IEnvInterface.h>
#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/SerializationUtils.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/IGUIDRemapper.h>

namespace sxema
{

CScriptActionInstance::CScriptActionInstance() {}

CScriptActionInstance::CScriptActionInstance(const DrxGUID& guid, tukk szName, const DrxGUID& actionTypeGUID, const DrxGUID& componentInstanceGUID)
	: CScriptElementBase(guid, szName)
	, m_actionTypeGUID(actionTypeGUID)
	, m_componentInstanceGUID(componentInstanceGUID)
{
	RefreshProperties();
}

void CScriptActionInstance::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const {}

void CScriptActionInstance::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	m_componentInstanceGUID = guidRemapper.Remap(m_componentInstanceGUID);
}

void CScriptActionInstance::ProcessEvent(const SScriptEvent& event) {}

void CScriptActionInstance::Serialize(Serialization::IArchive& archive)
{
	// #SchematycTODO : Shouldn't this be handled by CScriptElementBase itself?
	CScriptElementBase::Serialize(archive);
	CMultiPassSerializer::Serialize(archive);
	CScriptElementBase::SerializeExtensions(archive);
}

DrxGUID CScriptActionInstance::GetActionTypeGUID() const
{
	return m_actionTypeGUID;
}

DrxGUID CScriptActionInstance::GetComponentInstanceGUID() const
{
	return m_componentInstanceGUID;
}

const CClassProperties& CScriptActionInstance::GetProperties() const
{
	return m_properties;
}

void CScriptActionInstance::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_actionTypeGUID, "actionTypeGUID");
	archive(m_componentInstanceGUID, "componentInstanceGUID");
}

void CScriptActionInstance::Load(Serialization::IArchive& archive, const ISerializationContext& context)
{
	RefreshProperties();
	SerializeProperties(archive);
}

void CScriptActionInstance::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_actionTypeGUID, "actionTypeGUID");
	archive(m_componentInstanceGUID, "componentInstanceGUID");
	SerializeProperties(archive);
}

void CScriptActionInstance::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	if (archive.isOutput())
	{
		const IEnvAction* pEnvAction = gEnv->pSchematyc->GetEnvRegistry().GetAction(m_actionTypeGUID);
		if (pEnvAction)
		{
			string action = pEnvAction->GetName();
			archive(action, "action", "!Action");
		}
	}
	SerializeProperties(archive);
}

void CScriptActionInstance::RefreshProperties()
{
	// #SchematycTODO : Implement!!!
}

void CScriptActionInstance::SerializeProperties(Serialization::IArchive& archive)
{
	// #SchematycTODO : Implement!!!
}

} // sxema
