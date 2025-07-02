// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptTimer.h>
#include <drx3D/Schema/MultiPassSerializer.h>
#include <drx3D/Schema/ITimerSystem.h>

#include <drx3D/Schema/ScriptElementBase.h>
#include <drx3D/Schema/ScriptUserDocumentation.h>

namespace sxema
{
class CScriptTimer : public CScriptElementBase<IScriptTimer>, public CMultiPassSerializer
{
public:

	CScriptTimer();
	CScriptTimer(const DrxGUID& guid, tukk szName);

	// IScriptElement
	virtual void EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void ProcessEvent(const SScriptEvent& event) override;
	virtual void Serialize(Serialization::IArchive& archive) override;
	// ~IScriptElement

	// IScriptTimer
	virtual tukk  GetAuthor() const override;
	virtual tukk  GetDescription() const override;
	virtual STimerParams GetParams() const override;
	// ~IScriptTimer

protected:

	// CMultiPassSerializer
	virtual void Load(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
	// ~CMultiPassSerializer

private:

	void SerializeParams(Serialization::IArchive& archive);
	void ValidateDuration(STimerDuration& duration, Serialization::IArchive* pArchive, bool bApplyCorrections) const;

private:

	STimerParams             m_params;
	SScriptUserDocumentation m_userDocumentation;
};
} // sxema
