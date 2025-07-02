// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IEnvClass.h>
#include <drx3D/Schema/MultiPassSerializer.h>
#include <drx3D/Schema/IScriptClass.h>

#include <drx3D/Schema/ScriptElementBase.h>
#include <drx3D/Schema/ScriptUserDocumentation.h>

namespace sxema
{
class CScriptClass : public CScriptElementBase<IScriptClass>, public CMultiPassSerializer
{
public:

	CScriptClass();
	CScriptClass(const DrxGUID& guid, tukk szName);

	// IScriptElement
	virtual void EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void ProcessEvent(const SScriptEvent& event) override;
	virtual void Serialize(Serialization::IArchive& archive) override;
	// ~IScriptElement

	// IScriptClass
	virtual tukk GetAuthor() const override;
	virtual tukk GetDescription() const override;
	// ~IScriptClass

protected:

	// CMultiPassSerializer
	virtual void Load(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
	// ~CMultiPassSerializer

private:

	SScriptUserDocumentation m_userDocumentation;
};
}
