// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptClass.h>

#include <drx3D/CoreX/Serialization/BlackBox.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema2/GUIDRemapper.h>
#include <drx3D/Schema2/Deprecated/DocUtils.h>
#include <drx3D/Schema2/IEnvRegistry.h>
#include <drx3D/Schema2/ISerializationContext.h>
#include <drx3D/Schema2/SerializationUtils.h>

namespace sxema2
{
	//////////////////////////////////////////////////////////////////////////
	CScriptClass::CScriptClass(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName, const SGUID& foundationGUID)
		: CScriptElementBase(EScriptElementType::Class, file)
		, m_guid(guid)
		, m_scopeGUID(scopeGUID)
		, m_name(szName)
		, m_foundationGUID(foundationGUID)
	{}

	//////////////////////////////////////////////////////////////////////////
	EAccessor CScriptClass::GetAccessor() const
	{
		return EAccessor::Private;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptClass::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptClass::GetScopeGUID() const
	{
		return m_scopeGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptClass::SetName(tukk szName)
	{
		m_name = szName;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptClass::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptClass::EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const {}

	//////////////////////////////////////////////////////////////////////////
	void CScriptClass::Refresh(const SScriptRefreshParams& params)
	{
		switch(params.reason)
		{
		case EScriptRefreshReason::EditorAdd:
			{
				RefreshFoundationProperties();
				m_userDocumentation.SetCurrentUserAsAuthor();
			}
		case EScriptRefreshReason::EditorFixUp:
		case EScriptRefreshReason::EditorPaste:
		case EScriptRefreshReason::EditorEnvModified:
		case EScriptRefreshReason::EditorDependencyModified:
			{
				RefreshFoundationComponents();
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptClass::Serialize(Serialization::IArchive& archive)
	{
		LOADING_TIME_PROFILE_SECTION;
		
		CScriptElementBase::Serialize(archive);

		SerializationContext::SetValidatorLink(archive, SValidatorLink(m_guid)); // #SchematycTODO : Can we set this from CScriptElementBase?
		switch(SerializationContext::GetPass(archive))
		{
		case ESerializationPass::PreLoad:
		case ESerializationPass::Save:
			{
				Serialization::SContext fileContext(archive, static_cast<IScriptFile*>(&CScriptElementBase::GetFile())); // #SchematycTODO : Do we still need this?
				Serialization::SContext context(archive, static_cast<IScriptClass*>(this)); // #SchematycTODO : Do we still need this?

				archive(m_guid, "guid");
				archive(m_scopeGUID, "scope_guid");
				archive(m_name, "name");
				archive(m_foundationGUID, "foundation_guid");
				if(archive.isInput())
				{
					RefreshFoundationProperties();
				}
				if(m_pFoundationProperties)
				{
					archive(*m_pFoundationProperties, "foundation_properties", "Foundation Properties");
				}
				archive(m_userDocumentation, "userDocumentation", "Documentation");
				break;
			}
		case ESerializationPass::Edit:
			{
				Serialization::SContext fileContext(archive, static_cast<IScriptFile*>(&CScriptElementBase::GetFile())); // #SchematycTODO : Do we still need this?
				Serialization::SContext context(archive, static_cast<IScriptClass*>(this)); // #SchematycTODO : Do we still need this?

				if(m_pFoundationProperties)
				{
					archive(*m_pFoundationProperties, "foundation_properties", "Foundation Properties");
				}
				archive(m_userDocumentation, "userDocumentation", "Documentation");
				break;
			}
		case ESerializationPass::Validate:
			{
				Validate(archive);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptClass::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		m_guid      = guidRemapper.Remap(m_guid);
		m_scopeGUID = guidRemapper.Remap(m_scopeGUID);
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptClass::GetAuthor() const
	{
		return m_userDocumentation.author.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptClass::GetDescription() const
	{
		return m_userDocumentation.description.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptClass::GetFoundationGUID() const
	{
		return m_foundationGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	IPropertiesConstPtr CScriptClass::GetFoundationProperties() const
	{
		return m_pFoundationProperties;
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptClass::RefreshFoundationProperties()
	{
		m_pFoundationProperties.reset();
		if(m_foundationGUID)
		{
			IFoundationConstPtr pFoundation = gEnv->pSchematyc2->GetEnvRegistry().GetFoundation(m_foundationGUID);
			SXEMA2_SYSTEM_ASSERT(pFoundation);
			if(pFoundation)
			{
				IPropertiesConstPtr pFoundationProperties = pFoundation->GetProperties();
				if(pFoundationProperties)
				{
					m_pFoundationProperties = pFoundationProperties->Clone();
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptClass::RefreshFoundationComponents()
	{
		// #SchematycTODO : If component has 'HIDE_IN_EDITOR' flag set we don't need to serialize it!!!
		GUIDVector          requiredComponentGUIDs;
		IFoundationConstPtr pFoundation = gEnv->pSchematyc2->GetEnvRegistry().GetFoundation(m_foundationGUID);
		if(pFoundation)
		{
			for(size_t foundationComponentIdx = 0, foundationComponentCount = pFoundation->GetComponentCount(); foundationComponentIdx < foundationComponentCount; ++ foundationComponentIdx)
			{
				requiredComponentGUIDs.push_back(pFoundation->GetComponentGUID(foundationComponentIdx));
			}
		}
		IEnvRegistry&                       envRegistry = gEnv->pSchematyc2->GetEnvRegistry();
		IScriptFile&                        file = CScriptElementBase::GetFile();
		TScriptComponentInstanceConstVector componentInstances;
		DocUtils::CollectComponentInstances(file, m_guid, true, componentInstances);
		for(const IScriptComponentInstance* pComponentInstance : componentInstances)
		{
			IComponentFactoryConstPtr pComponentFactory = envRegistry.GetComponentFactory(pComponentInstance->GetComponentGUID());
			SXEMA2_SYSTEM_ASSERT(pComponentFactory);
			if(pComponentFactory)
			{
				for(size_t componentDependencyIdx = 0, componentDependencyCount = pComponentFactory->GetDependencyCount(); componentDependencyIdx < componentDependencyCount; ++ componentDependencyIdx)
				{
					if(pComponentFactory->GetDependencyType(componentDependencyIdx) == EComponentDependencyType::Hard)
					{
						requiredComponentGUIDs.push_back(pComponentFactory->GetDependencyGUID(componentDependencyIdx));
					}
				}
			}
		}
		for(size_t requiredComponentIdx = 0, requiredComponentCount = requiredComponentGUIDs.size(); requiredComponentIdx < requiredComponentCount; ++ requiredComponentIdx)
		{
			const SGUID               componentGUID = requiredComponentGUIDs[requiredComponentIdx];
			IComponentFactoryConstPtr pComponentFactory = envRegistry.GetComponentFactory(componentGUID);
			SXEMA2_SYSTEM_ASSERT(pComponentFactory);
			if(pComponentFactory)
			{
				TScriptComponentInstanceConstVector componentInstances;
				DocUtils::CollectComponentInstances(file, m_guid, true, componentInstances);
				bool bComponentExists = false;
				for(const IScriptComponentInstance* pComponentInstance : componentInstances)
				{
					if(pComponentInstance->GetComponentGUID() == componentGUID)
					{
						bComponentExists = true;
						break;
					}
				}
				if(!bComponentExists)
				{
					file.AddComponentInstance(m_guid, pComponentFactory->GetName(), componentGUID, EScriptComponentInstanceFlags::Foundation);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptClass::Validate(Serialization::IArchive& archive)
	{
		if(m_foundationGUID)
		{
			IFoundationConstPtr pFoundation = gEnv->pSchematyc2->GetEnvRegistry().GetFoundation(m_foundationGUID);
			if(!pFoundation)
			{
				archive.error(*this, "Unable to retrieve foundation!");
			}
		}
	}
}
