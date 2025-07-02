// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptGroup.h>

#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/Schema2/GUIDRemapper.h>
#include <drx3D/Schema2/ISerializationContext.h>

namespace sxema2
{
	//////////////////////////////////////////////////////////////////////////
	CScriptGroup::CScriptGroup(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName)
		: CScriptElementBase(EScriptElementType::Group, file)
		, m_guid(guid)
		, m_scopeGUID(scopeGUID)
		, m_name(szName)
	{}

	//////////////////////////////////////////////////////////////////////////
	EAccessor CScriptGroup::GetAccessor() const
	{
		return EAccessor::Private;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptGroup::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptGroup::GetScopeGUID() const
	{
		return m_scopeGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptGroup::SetName(tukk szName)
	{
		m_name = szName;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptGroup::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptGroup::EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const {}

	//////////////////////////////////////////////////////////////////////////
	void CScriptGroup::Refresh(const SScriptRefreshParams& params) {}

	//////////////////////////////////////////////////////////////////////////
	void CScriptGroup::Serialize(Serialization::IArchive& archive)
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
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptGroup::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		m_guid      = guidRemapper.Remap(m_guid);
		m_scopeGUID = guidRemapper.Remap(m_scopeGUID);
	}
}
