// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptAbstractInterface.h>

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
	CScriptAbstractInterface::CScriptAbstractInterface(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName)
		: CScriptElementBase(EScriptElementType::AbstractInterface, file)
		, m_guid(guid)
		, m_scopeGUID(scopeGUID)
		, m_name(szName)
	{}

	//////////////////////////////////////////////////////////////////////////
	EAccessor CScriptAbstractInterface::GetAccessor() const
	{
		return EAccessor::Private;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptAbstractInterface::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptAbstractInterface::GetScopeGUID() const
	{
		return m_scopeGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptAbstractInterface::SetName(tukk szName)
	{
		m_name = szName;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptAbstractInterface::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterface::EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const {}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterface::Refresh(const SScriptRefreshParams& params)
	{
		if(params.reason == EScriptRefreshReason::EditorAdd)
		{
			m_userDocumentation.SetCurrentUserAsAuthor();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterface::Serialize(Serialization::IArchive& archive)
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
				archive(m_userDocumentation, "userDocumentation", "Documentation");
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterface::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		m_guid      = guidRemapper.Remap(m_guid);
		m_scopeGUID = guidRemapper.Remap(m_scopeGUID);
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptAbstractInterface::GetAuthor() const
	{
		return m_userDocumentation.author.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptAbstractInterface::GetDescription() const
	{
		return m_userDocumentation.description.c_str();
	}
}
