// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptSignalReceiver.h>
#include <drx3D/Schema/MultiPassSerializer.h>
#include <drx3D/Schema/GUID.h>

#include <drx3D/Schema/ScriptElementBase.h>
#include <drx3D/Schema/ScriptUserDocumentation.h>

namespace sxema
{
class CScriptSignalReceiver : public CScriptElementBase<IScriptSignalReceiver>, public CMultiPassSerializer
{
public:

	CScriptSignalReceiver();
	CScriptSignalReceiver(const DrxGUID& guid, tukk szName, EScriptSignalReceiverType type, const DrxGUID& signalGUID);

	// IScriptElement
	virtual void EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void ProcessEvent(const SScriptEvent& event) override;
	virtual void Serialize(Serialization::IArchive& archive) override;
	// ~IScriptElement

	// IScriptSignalReceiver
	virtual EScriptSignalReceiverType GetSignalReceiverType() const override;
	virtual DrxGUID                     GetSignalGUID() const override;
	// ~IScriptSignalReceiver

protected:

	// CMultiPassSerializer
	virtual void LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Load(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Validate(Serialization::IArchive& archive, const ISerializationContext& context) override;
	// ~CMultiPassSerializer

private:

	void CreateGraph();
	void GoToSignal();

private:

	EScriptSignalReceiverType m_type;
	DrxGUID                     m_signalGUID;
	SScriptUserDocumentation  m_userDocumentation;
};
} // sxema
