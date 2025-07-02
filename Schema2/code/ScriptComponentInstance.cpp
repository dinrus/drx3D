// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptComponentInstance.h>

#include <drx3D/CoreX/Serialization/BlackBox.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema2/GUIDRemapper.h>
#include <drx3D/Schema2/IAbstractInterface.h>
#include <drx3D/Schema2/ICompiler.h>
#include <drx3D/Schema2/Deprecated/DocUtils.h>
#include <drx3D/Schema2/IEnvRegistry.h>
#include <drx3D/Schema2/ISerializationContext.h>
#include <drx3D/Schema2/SerializationUtils.h>

#include <drx3D/Schema2/ScriptPropertyGraph.h>
#include <drx3D/Schema2/SerializationContext.h>

SERIALIZATION_ENUM_BEGIN_NESTED(sxema2, EScriptComponentInstanceFlags, "sxema Script Component Instance Flags")
	SERIALIZATION_ENUM(sxema2::EScriptComponentInstanceFlags::Foundation, "foundation", "Foundation")
SERIALIZATION_ENUM_END()

namespace sxema2
{
	//////////////////////////////////////////////////////////////////////////
	CScriptComponentInstance::CScriptComponentInstance(IScriptFile& file)
		: CScriptElementBase(EScriptElementType::ComponentInstance, file)
		, m_flags(EScriptComponentInstanceFlags::None)
	{}

	//////////////////////////////////////////////////////////////////////////
	CScriptComponentInstance::CScriptComponentInstance(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName, const SGUID& componentGUID, EScriptComponentInstanceFlags flags)
		: CScriptElementBase(EScriptElementType::ComponentInstance, file)
		, m_guid(guid)
		, m_scopeGUID(scopeGUID)
		, m_name(szName)
		, m_componentGUID(componentGUID)
		, m_flags(flags)
	{
		ApplyComponent();
	}

	//////////////////////////////////////////////////////////////////////////
	EAccessor CScriptComponentInstance::GetAccessor() const
	{
		return EAccessor::Private;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptComponentInstance::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptComponentInstance::GetScopeGUID() const
	{
		return m_scopeGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptComponentInstance::SetName(tukk szName)
	{
		m_name = szName;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptComponentInstance::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptComponentInstance::EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const {}

	//////////////////////////////////////////////////////////////////////////
	void CScriptComponentInstance::Refresh(const SScriptRefreshParams& params)
	{
		CScriptElementBase::Refresh(params);
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptComponentInstance::Serialize(Serialization::IArchive& archive)
	{
		LOADING_TIME_PROFILE_SECTION;

		SerializationContext::SetValidatorLink(archive, SValidatorLink(m_guid)); // #SchematycTODO : Can we set this from CScriptElementBase?
		switch(SerializationContext::GetPass(archive))
		{
		case ESerializationPass::PreLoad:
		case ESerializationPass::Save:
			{
				archive(m_guid, "guid");
				archive(m_scopeGUID, "scope_guid");
				archive(m_name, "name");
				archive(m_componentGUID, "component_guid");

				if(archive.isInput())
				{
					ApplyComponent();
				}

				if(m_pProperties)
				{
					PopulateSerializationContext(archive);
					archive(*m_pProperties, "properties", "Properties");
				}
				break;
			}
		case ESerializationPass::Edit:
			{
				if(m_pProperties)
				{
					PopulateSerializationContext(archive);
					archive(*m_pProperties, "properties", "Properties");
				}
				break;
			}
		}

		CScriptElementBase::Serialize(archive);
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptComponentInstance::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		CScriptElementBase::RemapGUIDs(guidRemapper);
		m_guid      = guidRemapper.Remap(m_guid);
		m_scopeGUID = guidRemapper.Remap(m_scopeGUID);
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptComponentInstance::GetComponentGUID() const
	{
		return m_componentGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	EScriptComponentInstanceFlags CScriptComponentInstance::GetFlags() const
	{
		return m_flags;
	}

	//////////////////////////////////////////////////////////////////////////
	IPropertiesConstPtr CScriptComponentInstance::GetProperties() const
	{
		return m_pProperties;
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptComponentInstance::ApplyComponent()
	{
		IComponentFactoryConstPtr pFactory = gEnv->pSchematyc2->GetEnvRegistry().GetComponentFactory(m_componentGUID);
		if(pFactory)
		{
			m_pProperties = pFactory->CreateProperties();
			if((pFactory->GetFlags() & EComponentFlags::CreatePropertyGraph) != 0)
			{
				CScriptElementBase::AddExtension(std::make_shared<CScriptPropertyGraph>(*this));
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptComponentInstance::PopulateSerializationContext(Serialization::IArchive& archive)
	{
		sxema2::ISerializationContext* pSerializationContext = SerializationContext::Get(archive);
		if(pSerializationContext)
		{
			IComponentFactoryConstPtr pComponentFactory = gEnv->pSchematyc2->GetEnvRegistry().GetComponentFactory(m_componentGUID);
			if(pComponentFactory)
			{
				TScriptComponentInstanceConstVector componentInstances;
				DocUtils::CollectComponentInstances(CScriptElementBase::GetFile(), m_scopeGUID, true, componentInstances);
				for(size_t dependencyIdx = 0, dependencyCount = pComponentFactory->GetDependencyCount(); dependencyIdx < dependencyCount; ++ dependencyIdx)
				{
					const SGUID dependencyGUID = pComponentFactory->GetDependencyGUID(dependencyIdx);
					for(const IScriptComponentInstance* pComponentInstance : componentInstances)
					{
						if(pComponentInstance->GetComponentGUID() == dependencyGUID)
						{
							const IPropertiesConstPtr& pComponentProperties = pComponentInstance->GetProperties();
							SXEMA2_SYSTEM_ASSERT(pComponentProperties);
							if(pComponentProperties)
							{
								pSerializationContext->AddDependency(pComponentProperties->ToVoidPtr(), pComponentProperties->GetEnvTypeId());
							}
							break;
						}
					}
				}
			}
		}
	}
}
