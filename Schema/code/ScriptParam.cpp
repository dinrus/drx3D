// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptParam.h>

#include <drx3D/Schema/ICore.h>
#include <drx3D/Schema/IGUIDRemapper.h>

namespace sxema
{
void SScriptParam::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	data.SerializeTypeId(archive);
}

void SScriptParam::Load(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(guid, "guid");
	archive(name, "name");
	data.SerializeValue(archive);
}

void SScriptParam::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(guid, "guid");
	archive(name, "name");
	data.SerializeTypeId(archive);
	data.SerializeValue(archive);
}

void SScriptParam::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(name, "name", "^");
	data.SerializeTypeId(archive);
	data.SerializeValue(archive);

	if (archive.isInput() && GUID::IsEmpty(guid))
	{
		guid = gEnv->pSchematyc->CreateGUID();
	}
}

namespace ScriptParam
{
void EnumerateDependencies(const ScriptParams& params, const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type)
{
	SXEMA_CORE_ASSERT(enumerator);
	if (enumerator)
	{
		for (const SScriptParam& param : params)
		{
			param.data.EnumerateDependencies(enumerator, type);
		}
	}
}

void RemapDependencies(ScriptParams& params, IGUIDRemapper& guidRemapper)
{
	for (SScriptParam& param : params)
	{
		param.data.RemapDependencies(guidRemapper);
	}
}
} // ScriptParam
} // sxema
