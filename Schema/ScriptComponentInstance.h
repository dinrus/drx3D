// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptComponentInstance.h>
#include <drx3D/Schema/MultiPassSerializer.h>
#include <drx3D/Schema/ClassProperties.h>
#include <drx3D/Schema/Transform.h>

#include <drx3D/Schema/ScriptElementBase.h>

namespace sxema
{

class CScriptComponentInstance : public CScriptElementBase<IScriptComponentInstance>, public CMultiPassSerializer
{
public:

	CScriptComponentInstance();
	CScriptComponentInstance(const DrxGUID& guid, tukk szName, const DrxGUID& typeGUID);

	// IScriptElement
	virtual EScriptElementAccessor GetAccessor() const override;
	virtual void                   EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void                   RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void                   ProcessEvent(const SScriptEvent& event) override;
	virtual void                   Serialize(Serialization::IArchive& archive) override;
	// ~IScriptElement

	// IScriptComponentInstance
	virtual DrxGUID                        GetTypeGUID() const override;
	virtual ScriptComponentInstanceFlags GetComponentInstanceFlags() const override;
	virtual bool                         HasTransform() const override;
	virtual void                         SetTransform(const CTransformPtr& transform) override;
	virtual const CTransformPtr&         GetTransform() const override;
	virtual const CClassProperties&      GetProperties() const override;
	// ~IScriptComponentInstance

protected:

	// CMultiPassSerializer
	virtual void LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Load(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Validate(Serialization::IArchive& archive, const ISerializationContext& context) override;
	// ~CMultiPassSerializer

private:

	void ApplyComponent();

private:

	EScriptElementAccessor       m_accessor = EScriptElementAccessor::Private;
	DrxGUID                      m_typeGUID;
	ScriptComponentInstanceFlags m_flags;
	bool                         m_bHasTransform = false;
	CTransformPtr                m_pTransform;
	CClassProperties             m_properties;
};

} // sxema
