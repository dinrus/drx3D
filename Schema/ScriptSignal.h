// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptSignal.h>
#include <drx3D/Schema/MultiPassSerializer.h>

#include <drx3D/Schema/ScriptElementBase.h>
#include <drx3D/Schema/ScriptParam.h>
#include <drx3D/Schema/ScriptUserDocumentation.h>

namespace sxema
{
// Forward declare classes.
class CAnyConstPtr;

class CScriptSignal : public CScriptElementBase<IScriptSignal>, public CMultiPassSerializer
{
public:

	CScriptSignal();
	CScriptSignal(const DrxGUID& guid, tukk szName);

	// IScriptElement
	virtual void EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void ProcessEvent(const SScriptEvent& event) override;
	virtual void Serialize(Serialization::IArchive& archive) override;
	// ~IScriptElement

	// IScriptSignal
	virtual tukk  GetAuthor() const override;
	virtual tukk  GetDescription() const override;
	virtual u32       GetInputCount() const override;
	virtual DrxGUID        GetInputGUID(u32 inputIdx) const override;
	virtual tukk  GetInputName(u32 inputIdx) const override;
	virtual SElementId   GetInputTypeId(u32 inputIdx) const override;
	virtual CAnyConstPtr GetInputData(u32 inputIdx) const override;
	// ~IScriptSignal

protected:

	// CMultiPassSerializer
	virtual void LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Load(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
	// ~CMultiPassSerializer

private:

	ScriptParams             m_inputs;
	SScriptUserDocumentation m_userDocumentation;
};
} // sxema
