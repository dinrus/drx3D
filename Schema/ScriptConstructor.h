// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptConstructor.h>

#include <drx3D/Schema/ScriptElementBase.h>
#include <drx3D/Schema/ScriptUserDocumentation.h>

namespace sxema
{
class CScriptConstructor : public CScriptElementBase<IScriptConstructor>
{
public:

	CScriptConstructor();
	CScriptConstructor(const DrxGUID& guid, tukk szName);

	// IScriptElement
	virtual void EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void ProcessEvent(const SScriptEvent& event) override;
	virtual void Serialize(Serialization::IArchive& archive) override;
	// ~IScriptElement

private:

	void CreateGraph();

private:

	SScriptUserDocumentation m_userDocumentation;
};
} // sxema
