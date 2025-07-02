// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptStructure.h>

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
	CScriptStructure::CScriptStructure(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName)
		: CScriptElementBase(EScriptElementType::Structure, file)
		, m_guid(guid)
		, m_scopeGUID(scopeGUID)
		, m_name(szName)
	{}

	//////////////////////////////////////////////////////////////////////////
	EAccessor CScriptStructure::GetAccessor() const
	{
		return EAccessor::Private;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptStructure::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptStructure::GetScopeGUID() const
	{
		return m_scopeGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptStructure::SetName(tukk szName)
	{
		m_name = szName;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptStructure::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptStructure::EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const
	{
		SXEMA2_SYSTEM_ASSERT(enumerator);
		if(enumerator)
		{
			for(const CScriptVariableDeclaration& field : m_fields)
			{
				field.EnumerateDependencies(enumerator);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptStructure::Refresh(const SScriptRefreshParams& params)
	{
		if(params.reason == EScriptRefreshReason::EditorAdd)
		{
			m_userDocumentation.SetCurrentUserAsAuthor();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptStructure::Serialize(Serialization::IArchive& archive)
	{
		LOADING_TIME_PROFILE_SECTION;

		CScriptElementBase::Serialize(archive);

		SerializationContext::SetValidatorLink(archive, SValidatorLink(m_guid)); // #SchematycTODO : Can we set this from CScriptElementBase?
		switch(SerializationContext::GetPass(archive))
		{
		case ESerializationPass::PreLoad:
			{
				archive(m_guid, "guid");
				archive(m_scopeGUID, "scope_guid");
				archive(m_name, "name");
				archive(m_fields, "fields");
				archive(m_userDocumentation, "userDocumentation", "Documentation");
				break;
			}
		case ESerializationPass::Load:
			{
				archive(m_fields, "fields");
				break;
			}
		case ESerializationPass::Save:
			{
				Serialization::SContext fileContext(archive, static_cast<IScriptFile*>(&CScriptElementBase::GetFile())); // #SchematycTODO : Do we still need this?
				archive(m_guid, "guid");
				archive(m_scopeGUID, "scope_guid");
				archive(m_name, "name");
				archive(m_fields, "fields");
				archive(m_userDocumentation, "userDocumentation", "Documentation");
				break;
			}
		case ESerializationPass::Edit:
			{
				Serialization::SContext fileContext(archive, static_cast<IScriptFile*>(&CScriptElementBase::GetFile())); // #SchematycTODO : Do we still need this?
				archive(m_fields, "fields", "Fields");
				archive(m_userDocumentation, "userDocumentation", "Documentation");
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptStructure::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		m_guid      = guidRemapper.Remap(m_guid);
		m_scopeGUID = guidRemapper.Remap(m_scopeGUID);
	}

	//////////////////////////////////////////////////////////////////////////
	CAggregateTypeId CScriptStructure::GetTypeId() const
	{
		return CAggregateTypeId::FromScriptTypeGUID(m_guid);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CScriptStructure::GetFieldCount() const
	{
		return m_fields.size();
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptStructure::GetFieldName(size_t fieldIdx) const
	{
		return fieldIdx < m_fields.size() ? m_fields[fieldIdx].GetName() : "";
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyConstPtr CScriptStructure::GetFieldValue(size_t fieldIdx) const
	{
		return fieldIdx < m_fields.size() ? m_fields[fieldIdx].GetValue() : IAnyConstPtr();
	}
}
