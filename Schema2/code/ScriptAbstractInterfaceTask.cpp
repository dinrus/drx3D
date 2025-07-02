// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptAbstractInterfaceTask.h>

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
	CScriptAbstractInterfaceTask::CScriptAbstractInterfaceTask(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk name)
		: CScriptElementBase(EScriptElementType::AbstractInterfaceTask, file)
		, m_guid(guid)
		, m_scopeGUID(scopeGUID)
		, m_name(name)
	{}

	//////////////////////////////////////////////////////////////////////////
	EAccessor CScriptAbstractInterfaceTask::GetAccessor() const
	{
		return EAccessor::Private;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptAbstractInterfaceTask::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptAbstractInterfaceTask::GetScopeGUID() const
	{
		return m_scopeGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptAbstractInterfaceTask::SetName(tukk szName)
	{
		m_name = szName;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptAbstractInterfaceTask::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceTask::EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const {}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceTask::Refresh(const SScriptRefreshParams& params)
	{
		if(params.reason == EScriptRefreshReason::EditorAdd)
		{
			m_userDocumentation.SetCurrentUserAsAuthor();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceTask::Serialize(Serialization::IArchive& archive)
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
				archive(m_userDocumentation, "userDocumentation", "Documentation");
				break;
			}
		case ESerializationPass::Edit:
			{
				Serialization::SContext fileContext(archive, static_cast<IScriptFile*>(&CScriptElementBase::GetFile())); // #SchematycTODO : Do we still need this?
				archive(m_userDocumentation, "userDocumentation", "Documentation");
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceTask::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		m_guid      = guidRemapper.Remap(m_guid);
		m_scopeGUID = guidRemapper.Remap(m_scopeGUID);
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptAbstractInterfaceTask::GetAuthor() const
	{
		return m_userDocumentation.author.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptAbstractInterfaceTask::GetDescription() const
	{
		return m_userDocumentation.description.c_str();
	}
}
