// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptActionInstance.h>
#include <drx3D/Schema/MultiPassSerializer.h>
#include <drx3D/Schema/ClassProperties.h>

#include <drx3D/Schema/ScriptElementBase.h>

namespace sxema
{

class CScriptActionInstance : public CScriptElementBase<IScriptActionInstance>, public CMultiPassSerializer
{
public:

	CScriptActionInstance();
	CScriptActionInstance(const DrxGUID& guid, tukk szName, const DrxGUID& actionTypeGUID, const DrxGUID& componentInstanceGUID);

	// IScriptElement
	virtual void EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void ProcessEvent(const SScriptEvent& event) override;
	virtual void Serialize(Serialization::IArchive& archive) override;
	// ~IScriptElement

	// IScriptActionInstance
	virtual DrxGUID                   GetActionTypeGUID() const override;
	virtual DrxGUID                   GetComponentInstanceGUID() const override;
	virtual const CClassProperties& GetProperties() const override;
	// ~IScriptActionInstance

protected:

	// CMultiPassSerializer
	virtual void LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Load(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
	// ~CMultiPassSerializer

private:

	void RefreshProperties();
	void SerializeProperties(Serialization::IArchive& archive);

private:

	DrxGUID            m_actionTypeGUID;
	DrxGUID            m_componentInstanceGUID;
	CClassProperties m_properties;
};

} // sxema
