// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptRoot.h>
#include <drx3D/Schema/MultiPassSerializer.h>

#include <drx3D/Schema/ScriptElementBase.h>

namespace sxema
{
class CScriptRoot : public CScriptElementBase<IScriptRoot>, public CMultiPassSerializer
{
public:

	CScriptRoot();

	// IScriptElement
	virtual void EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void ProcessEvent(const SScriptEvent& event) override;
	virtual void Serialize(Serialization::IArchive& archive) override;
	// ~IScriptElement
};
} // sxema
