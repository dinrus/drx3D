// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptEnumeration.h>

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
	CScriptEnumeration::CScriptEnumeration(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName)
		: CScriptElementBase(EScriptElementType::Enumeration, file)
		, m_guid(guid)
		, m_scopeGUID(scopeGUID)
		, m_name(szName)
	{}

	//////////////////////////////////////////////////////////////////////////
	EAccessor CScriptEnumeration::GetAccessor() const
	{
		return EAccessor::Private;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptEnumeration::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptEnumeration::GetScopeGUID() const
	{
		return m_scopeGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptEnumeration::SetName(tukk szName)
	{
		m_name = szName;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptEnumeration::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptEnumeration::EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const {}

	//////////////////////////////////////////////////////////////////////////
	void CScriptEnumeration::Refresh(const SScriptRefreshParams& params)
	{
		if(params.reason == EScriptRefreshReason::EditorAdd)
		{
			m_userDocumentation.SetCurrentUserAsAuthor();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptEnumeration::Serialize(Serialization::IArchive& archive)
	{
		LOADING_TIME_PROFILE_SECTION;
		
		CScriptElementBase::Serialize(archive);

		SerializationContext::SetValidatorLink(archive, SValidatorLink(m_guid)); // #SchematycTODO : Can we set this from CScriptElementBase?
		switch(SerializationContext::GetPass(archive))
		{
		case ESerializationPass::PreLoad:
		case ESerializationPass::Save:
			{
				archive(m_constants, "constants", "Constants");
				archive(m_userDocumentation, "userDocumentation", "Documentation");
				archive(m_guid, "guid");
				archive(m_scopeGUID, "scope_guid");
				archive(m_name, "name");
				break;
			}
		case ESerializationPass::Edit:
			{
				archive(m_constants, "constants", "Constants");
				archive(m_userDocumentation, "userDocumentation", "Documentation");
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptEnumeration::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		m_guid      = guidRemapper.Remap(m_guid);
		m_scopeGUID = guidRemapper.Remap(m_scopeGUID);
	}

	//////////////////////////////////////////////////////////////////////////
	CAggregateTypeId CScriptEnumeration::GetTypeId() const
	{
		return CAggregateTypeId::FromScriptTypeGUID(m_guid);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CScriptEnumeration::GetConstantCount() const
	{
		return m_constants.size();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CScriptEnumeration::FindConstant(tukk szConstant) const
	{
		SXEMA2_SYSTEM_ASSERT(szConstant);
		if(szConstant)
		{
			for(Constants::const_iterator itConstant = m_constants.begin(), itEndConstant = m_constants.end(); itConstant != itEndConstant; ++ itConstant)
			{
				if(strcmp(itConstant->c_str(), szConstant) == 0)
				{
					return itConstant - m_constants.begin();
				}
			}
		}
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptEnumeration::GetConstant(size_t constantIdx) const
	{
		return constantIdx < m_constants.size() ? m_constants[constantIdx].c_str() : "";
	}
}
