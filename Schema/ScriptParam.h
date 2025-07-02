// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/MultiPassSerializer.h>
#include <drx3D/Schema/ScriptDependencyEnumerator.h>
#include <drx3D/Schema/GUID.h>

#include <drx3D/Schema/ScriptVariableData.h>

namespace sxema
{
// Forward declare interfaces.
struct IGUIDRemapper;

struct SScriptParam : public CMultiPassSerializer
{
	// CMultiPassSerializer
	virtual void LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Load(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
	// ~CMultiPassSerializer

	DrxGUID               guid;
	string              name;
	CScriptVariableData data;
};

typedef std::vector<SScriptParam> ScriptParams;

namespace ScriptParam
{
void EnumerateDependencies(const ScriptParams& params, const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type);
void RemapDependencies(ScriptParams& params, IGUIDRemapper& guidRemapper);
} // ScriptParam
} // sxema
