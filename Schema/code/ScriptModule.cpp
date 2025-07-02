// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptModule.h>

#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/IGUIDRemapper.h>

namespace sxema {

CScriptModule::CScriptModule()
	: CScriptElementBase(EScriptElementFlags::MustOwnScript)
{}

CScriptModule::CScriptModule(const DrxGUID& guid, tukk szName)
	: CScriptElementBase(guid, szName, EScriptElementFlags::MustOwnScript)
{}

void CScriptModule::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const {}

void CScriptModule::RemapDependencies(IGUIDRemapper& guidRemapper)                                                        {}

void CScriptModule::ProcessEvent(const SScriptEvent& event)                                                               {}

void CScriptModule::Serialize(Serialization::IArchive& archive)
{
	// #SchematycTODO : Shouldn't this be handled by CScriptElementBase itself?
	CScriptElementBase::Serialize(archive);
	CMultiPassSerializer::Serialize(archive);
	CScriptElementBase::SerializeExtensions(archive);
}

} // sxema
