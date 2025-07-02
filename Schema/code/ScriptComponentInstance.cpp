// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptComponentInstance.h>

#include <drx3D/CoreX/Serialization/BlackBox.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvComponent.h>
#include <drx3D/Schema/IEnvInterface.h>
#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/SerializationUtils.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/IGUIDRemapper.h>

#include <drx3D/Schema/CVars.h>
#include <drx3D/Schema/ScriptGraph.h>
#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphBeginNode.h>
#include <drx3D/Schema/SerializationContext.h>

SERIALIZATION_ENUM_BEGIN_NESTED(sxema, EScriptComponentInstanceFlags, "DrxSchematyc Script Component Instance Flags")
SERIALIZATION_ENUM(sxema::EScriptComponentInstanceFlags::EnvClass, "envClass", "Environment Class")
SERIALIZATION_ENUM_END()

namespace sxema
{

CScriptComponentInstance::CScriptComponentInstance()
	: CScriptElementBase(EScriptElementFlags::None)
{}

CScriptComponentInstance::CScriptComponentInstance(const DrxGUID& guid, tukk szName, const DrxGUID& typeGUID)
	: CScriptElementBase(guid, szName, EScriptElementFlags::None)
	, m_typeGUID(typeGUID)
{
	ApplyComponent(); // #SchematycTODO : Do this on EScriptEventId::Add?
}

EScriptElementAccessor CScriptComponentInstance::GetAccessor() const
{
	return m_accessor;
}

void CScriptComponentInstance::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const {}

void CScriptComponentInstance::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	CScriptElementBase::RemapDependencies(guidRemapper);
}

void CScriptComponentInstance::ProcessEvent(const SScriptEvent& event)
{
	CScriptElementBase::ProcessEvent(event);
}

void CScriptComponentInstance::Serialize(Serialization::IArchive& archive)
{
	// #SchematycTODO : Shouldn't this be handled by CScriptElementBase itself?
	CScriptElementBase::Serialize(archive);
	CMultiPassSerializer::Serialize(archive);
	CScriptElementBase::SerializeExtensions(archive);
}

DrxGUID CScriptComponentInstance::GetTypeGUID() const
{
	return m_typeGUID;
}

ScriptComponentInstanceFlags CScriptComponentInstance::GetComponentInstanceFlags() const
{
	return m_flags;
}

bool CScriptComponentInstance::HasTransform() const
{
	return m_bHasTransform;
}

void CScriptComponentInstance::SetTransform(const CTransformPtr& transform)
{
	m_pTransform = transform;
}

const CTransformPtr& CScriptComponentInstance::GetTransform() const
{
	return m_pTransform;
}

const CClassProperties& CScriptComponentInstance::GetProperties() const
{
	return m_properties;
}

void CScriptComponentInstance::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_typeGUID, "typeGUID");
}

void CScriptComponentInstance::Load(Serialization::IArchive& archive, const ISerializationContext& context)
{
	ApplyComponent();
	archive(m_accessor, "accessor");
	if (m_bHasTransform && m_pTransform)
	{
		archive(*m_pTransform, "transform");
	}
	archive(m_properties, "properties", "Properties");
}

void CScriptComponentInstance::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_accessor, "accessor");
	archive(m_typeGUID, "typeGUID");
	if (m_bHasTransform && m_pTransform)
	{
		archive(*m_pTransform, "transform");
	}
	archive(m_properties, "properties", "Properties");
}

void CScriptComponentInstance::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	if (!m_properties.IsEmpty())
	{
		bool bPublic = m_accessor == EScriptElementAccessor::Public;
		archive(bPublic, "bPublic", "Public");
		if (archive.isInput())
		{
			m_accessor = bPublic ? EScriptElementAccessor::Public : EScriptElementAccessor::Private;
		}
	}
	if (m_bHasTransform && m_pTransform)
	{
		archive(*m_pTransform, "transform", "Transform");
	}
	if(!m_properties.IsEmpty())
	{
		archive(m_properties, "properties", "Properties");
	}
}

void CScriptComponentInstance::Validate(Serialization::IArchive& archive, const ISerializationContext& context)
{
	const IEnvComponent* pEnvComponent = gEnv->pSchematyc->GetEnvRegistry().GetComponent(m_typeGUID);
	if (!pEnvComponent)
	{
		CStackString guid;
		GUID::ToString(guid, m_typeGUID);

		CStackString message;
		message.Format("Unable to retrieve environment component: guid = %s", guid.c_str());

		archive.error(*this, message.c_str());
	}
}

void CScriptComponentInstance::ApplyComponent()
{
	const IEnvComponent* pEnvComponent = gEnv->pSchematyc->GetEnvRegistry().GetComponent(m_typeGUID);
	if (pEnvComponent)
	{
		const CEntityComponentClassDesc& componentDesc = pEnvComponent->GetDesc();
		m_bHasTransform = componentDesc.GetComponentFlags().Check(IEntityComponent::EFlags::Transform);
		if (m_bHasTransform && !m_pTransform)
		{
			m_pTransform = std::make_shared<CTransform>();
		}
		m_properties.Set(componentDesc);
	}
}

} // sxema
