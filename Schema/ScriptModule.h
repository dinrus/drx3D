// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptModule.h>
#include <drx3D/Schema/MultiPassSerializer.h>

#include <drx3D/Schema/ScriptElementBase.h>

namespace sxema
{
class CScriptModule : public CScriptElementBase<IScriptModule>, public CMultiPassSerializer
{
public:

	CScriptModule();
	CScriptModule(const DrxGUID& guid, tukk szName);

	// IScriptElement
	virtual void EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void ProcessEvent(const SScriptEvent& event) override;
	virtual void Serialize(Serialization::IArchive& archive) override;
	// ~IScriptElement

protected:
	// CMultiPassSerializer
	virtual void Save(Serialization::IArchive& archive, const ISerializationContext& context) override { CMultiPassSerializer::Save(archive, context); }
	// ~CMultiPassSerializer
};
} // sxema
