// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptVariable.h>
#include <drx3D/Schema/MultiPassSerializer.h>

#include <drx3D/Schema/ScriptElementBase.h>
#include <drx3D/Schema/ScriptVariableData.h>

namespace sxema
{
// Forward declare classes.
class CAnyConstPtr;

class CScriptVariable : public CScriptElementBase<IScriptVariable>, public CMultiPassSerializer
{
public:

	CScriptVariable();
	CScriptVariable(const DrxGUID& guid, tukk szName, const SElementId& typeId, const DrxGUID& baseGUID);

	// IScriptElement
	virtual EScriptElementAccessor GetAccessor() const override;
	virtual void                   EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void                   RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void                   ProcessEvent(const SScriptEvent& event) override;
	virtual void                   Serialize(Serialization::IArchive& archive) override;
	// ~IScriptElement

	// IScriptVariable
	virtual SElementId      GetTypeId() const override;
	virtual bool            IsArray() const override;
	virtual CAnyConstPtr    GetData() const override;
	virtual DrxGUID           GetBaseGUID() const override;
	virtual EOverridePolicy GetOverridePolicy() const override;
	// ~IScriptVariable

protected:

	// CMultiPassSerializer
	virtual void LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Load(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
	// ~CMultiPassSerializer

private:

	EScriptElementAccessor m_accessor = EScriptElementAccessor::Private;
	CScriptVariableData    m_data;
	DrxGUID                  m_baseGUID;
	EOverridePolicy        m_overridePolicy = EOverridePolicy::Default;
};
} // sxema
