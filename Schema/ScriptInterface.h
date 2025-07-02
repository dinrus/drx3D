// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptInterface.h>
#include <drx3D/Schema/MultiPassSerializer.h>

#include <drx3D/Schema/ScriptElementBase.h>
#include <drx3D/Schema/ScriptUserDocumentation.h>

namespace sxema
{
class CScriptInterface : public CScriptElementBase<IScriptInterface>, public CMultiPassSerializer
{
public:

	CScriptInterface();
	CScriptInterface(const DrxGUID& guid, tukk szName);

	// IScriptElement
	virtual void EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void ProcessEvent(const SScriptEvent& event) override;
	virtual void Serialize(Serialization::IArchive& archive) override;
	// ~IScriptElement

	// IScriptInterface
	virtual tukk GetAuthor() const override;
	virtual tukk GetDescription() const override;
	// ~IScriptInterface

protected:

	// CMultiPassSerializer
	virtual void Load(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
	// ~CMultiPassSerializer

private:

	SScriptUserDocumentation m_userDocumentation;
};
} // sxema
