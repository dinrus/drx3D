// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptInterfaceFunction.h>
#include <drx3D/Schema/MultiPassSerializer.h>

#include <drx3D/Schema/ScriptElementBase.h>
#include <drx3D/Schema/ScriptParam.h>
#include <drx3D/Schema/ScriptUserDocumentation.h>

namespace sxema
{
// Forward declare classes.
class CAnyConstPtr;

class CScriptInterfaceFunction : public CScriptElementBase<IScriptInterfaceFunction>, public CMultiPassSerializer
{
public:

	CScriptInterfaceFunction();
	CScriptInterfaceFunction(const DrxGUID& guid, tukk szName);

	// IScriptElement
	virtual void EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void ProcessEvent(const SScriptEvent& event) override;
	virtual void Serialize(Serialization::IArchive& archive) override;
	// ~IScriptElement

	// IScriptInterfaceFunction
	virtual tukk  GetAuthor() const override;
	virtual tukk  GetDescription() const override;
	virtual u32       GetInputCount() const override;
	virtual tukk  GetInputName(u32 inputIdx) const override;
	virtual CAnyConstPtr GetInputValue(u32 inputIdx) const override;
	virtual u32       GetOutputCount() const override;
	virtual tukk  GetOutputName(u32 outputIdx) const override;
	virtual CAnyConstPtr GetOutputValue(u32 outputIdx) const override;
	// ~IScriptInterfaceFunction

protected:

	// CMultiPassSerializer
	virtual void LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Load(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
	// ~CMultiPassSerializer

private:

	ScriptParams             m_inputs;
	ScriptParams             m_outputs;
	SScriptUserDocumentation m_userDocumentation;
};
} // sxema
