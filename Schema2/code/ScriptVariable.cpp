// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptVariable.h>

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
	CScriptVariable::CScriptVariable(IScriptFile& file)
		: CScriptElementBase(EScriptElementType::Variable, file)
	{}

	//////////////////////////////////////////////////////////////////////////
	CScriptVariable::CScriptVariable(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName, const CAggregateTypeId& typeId)
		: CScriptElementBase(EScriptElementType::Variable, file)
		, m_guid(guid)
		, m_scopeGUID(scopeGUID)
		, m_declaration(szName, typeId, MakeScriptVariableValueShared(file, typeId))
	{}

	//////////////////////////////////////////////////////////////////////////
	EAccessor CScriptVariable::GetAccessor() const
	{
		return EAccessor::Private;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptVariable::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptVariable::GetScopeGUID() const
	{
		return m_scopeGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptVariable::SetName(tukk szName)
	{
		m_declaration.SetName(szName);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptVariable::GetName() const
	{
		return m_declaration.GetName();
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptVariable::EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const
	{
		SXEMA2_SYSTEM_ASSERT(enumerator);
		if(enumerator)
		{
			m_declaration.EnumerateDependencies(enumerator);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptVariable::Refresh(const SScriptRefreshParams& params) {}

	//////////////////////////////////////////////////////////////////////////
	void CScriptVariable::Serialize(Serialization::IArchive& archive)
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
				m_declaration.SerializeInfo(archive);
				break;
			}
		case ESerializationPass::Load:
			{
				m_declaration.SerializeValue(archive);
				break;
			}
		case ESerializationPass::Save:
			{
				Serialization::SContext fileContext(archive, static_cast<IScriptFile*>(&CScriptElementBase::GetFile())); // #SchematycTODO : Do we still need this?
				archive(m_guid, "guid");
				archive(m_scopeGUID, "scope_guid");
				m_declaration.Serialize(archive);
				break;
			}
		case ESerializationPass::Edit:
			{
				IScriptFile&            file = CScriptElementBase::GetFile();
				Serialization::SContext fileContext(archive, static_cast<IScriptFile*>(&file)); // #SchematycTODO : Do we still need this?
				string                  typeName = GetScriptVariableTypeName(file, m_declaration.GetTypeId());
				archive(typeName, "typeName", "!Type");
				m_declaration.SerializeValue(archive);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptVariable::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		m_guid      = guidRemapper.Remap(m_guid);
		m_scopeGUID = guidRemapper.Remap(m_scopeGUID);
		m_declaration.RemapGUIDs(guidRemapper);
	}

	//////////////////////////////////////////////////////////////////////////
	CAggregateTypeId CScriptVariable::GetTypeId() const
	{
		return m_declaration.GetTypeId();
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyConstPtr CScriptVariable::GetValue() const
	{
		return m_declaration.GetValue();
	}
}
