// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptEnum.h>

#include <drx3D/Schema/ScriptElementBase.h>
#include <drx3D/Schema/ScriptUserDocumentation.h>

namespace sxema
{
// Forward declare structures.
struct SScriptEnumTypeInfo;
// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(SScriptEnumTypeInfo)

class CScriptEnum : public CScriptElementBase<IScriptEnum>
{
private:

	typedef std::vector<string> Constants;

public:

	CScriptEnum();
	CScriptEnum(const DrxGUID& guid, tukk szName);

	// IScriptElement
	virtual void EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void ProcessEvent(const SScriptEvent& event) override;
	virtual void Serialize(Serialization::IArchive& archive) override;
	// ~IScriptElement

	// IScriptEnum
	virtual u32      GetConstantCount() const override;
	virtual u32      FindConstant(tukk szConstant) const override;
	virtual tukk GetConstant(u32 constantIdx) const override;
	// ~IScriptEnum

private:

	Constants                m_constants;
	SScriptUserDocumentation m_userDocumentation;
};
} // sxema
