// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptActionInstance.h>

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

namespace sxema2
{
	//////////////////////////////////////////////////////////////////////////
	CScriptActionInstance::CScriptActionInstance(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName, const SGUID& actionGUID, const SGUID& componentInstanceGUID)
		: CScriptElementBase(EScriptElementType::ActionInstance, file)
		, m_guid(guid)
		, m_scopeGUID(scopeGUID)
		, m_name(szName)
		, m_actionGUID(actionGUID)
		, m_componentInstanceGUID(componentInstanceGUID)
	{
		RefreshAction();
	}

	//////////////////////////////////////////////////////////////////////////
	EAccessor CScriptActionInstance::GetAccessor() const
	{
		return EAccessor::Private;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptActionInstance::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptActionInstance::GetScopeGUID() const
	{
		return m_scopeGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptActionInstance::SetName(tukk szName)
	{
		m_name = szName;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptActionInstance::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptActionInstance::EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const {}

	//////////////////////////////////////////////////////////////////////////
	void CScriptActionInstance::Refresh(const SScriptRefreshParams& params) {}

	//////////////////////////////////////////////////////////////////////////
	void CScriptActionInstance::Serialize(Serialization::IArchive& archive)
	{
		LOADING_TIME_PROFILE_SECTION;
		
		CScriptElementBase::Serialize(archive);

		SerializationContext::SetValidatorLink(archive, SValidatorLink(m_guid)); // #SchematycTODO : Can we set this from CScriptElementBase?
		switch(SerializationContext::GetPass(archive))
		{
		case ESerializationPass::PreLoad:
		case ESerializationPass::Save:
			{
				archive(m_guid, "guid");
				archive(m_scopeGUID, "scope_guid");
				archive(m_name, "name");
				archive(m_actionGUID, "action_guid");
				archive(m_componentInstanceGUID, "component_instance_guid");
				if(archive.isInput())
				{
					RefreshAction();
				}
				SerializeProperties(archive);
				break;
			}
		case ESerializationPass::Edit:
			{
				if(archive.isOutput())
				{
					IActionFactoryConstPtr pFactory = gEnv->pSchematyc2->GetEnvRegistry().GetActionFactory(m_actionGUID);
					if(pFactory)
					{
						string action = pFactory->GetName();
						archive(action, "action", "!Action");
					}
				}
				SerializeProperties(archive);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptActionInstance::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		m_guid                  = guidRemapper.Remap(m_guid);
		m_scopeGUID             = guidRemapper.Remap(m_scopeGUID);
		m_componentInstanceGUID = guidRemapper.Remap(m_componentInstanceGUID);
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptActionInstance::GetActionGUID() const
	{
		return m_actionGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptActionInstance::GetComponentInstanceGUID() const
	{
		return m_componentInstanceGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	IPropertiesConstPtr CScriptActionInstance::GetProperties() const
	{
		return m_pProperties;
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptActionInstance::RefreshAction()
	{
		IActionFactoryConstPtr pFactory = gEnv->pSchematyc2->GetEnvRegistry().GetActionFactory(m_actionGUID);
		m_pProperties = pFactory ? pFactory->CreateProperties() : IPropertiesPtr();
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptActionInstance::SerializeProperties(Serialization::IArchive& archive)
	{
		if(m_pProperties)
		{
			GameSerialization::IContextPtr      pSerializationContext;
			const IScriptComponentInstance* pComponentInstance = CScriptElementBase::GetFile().GetComponentInstance(m_componentInstanceGUID);
			if(pComponentInstance)
			{
				IPropertiesConstPtr pComponentProperties = pComponentInstance->GetProperties();
				if(pComponentProperties)
				{
					pSerializationContext = pComponentProperties->BindSerializationContext(archive);
				}
			}
			archive(*m_pProperties, "properties", "Properties");
		}
	}
}
