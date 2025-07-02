// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptProperty.h>

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
	CScriptProperty::CScriptProperty(IScriptFile& file)
		: CScriptElementBase(EScriptElementType::Property, file)
		, m_overridePolicy(EOverridePolicy::Default)
	{}

	//////////////////////////////////////////////////////////////////////////
	CScriptProperty::CScriptProperty(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName, const SGUID& refGUID, const CAggregateTypeId& typeId)
		: CScriptElementBase(EScriptElementType::Property, file)
		, m_guid(guid)
		, m_scopeGUID(scopeGUID)
		, m_refGUID(refGUID)
		, m_overridePolicy(EOverridePolicy::Default)
		, m_declaration(szName, typeId, MakeScriptVariableValueShared(file, typeId))
	{}

	//////////////////////////////////////////////////////////////////////////
	EAccessor CScriptProperty::GetAccessor() const
	{
		return EAccessor::Public;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptProperty::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptProperty::GetScopeGUID() const
	{
		return m_scopeGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptProperty::SetName(tukk szName)
	{
		m_declaration.SetName(szName);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptProperty::GetName() const
	{
		return m_declaration.GetName();
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptProperty::EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const
	{
		SXEMA2_SYSTEM_ASSERT(enumerator);
		if(enumerator)
		{
			if(m_refGUID)
			{
				enumerator(m_refGUID);
			}
			m_declaration.EnumerateDependencies(enumerator);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptProperty::Refresh(const SScriptRefreshParams& params) {}

	//////////////////////////////////////////////////////////////////////////
	void CScriptProperty::Serialize(Serialization::IArchive& archive)
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
				archive(m_refGUID, "refGUID");
				archive(m_overridePolicy, "overridePolicy");
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
				archive(m_refGUID, "refGUID");
				archive(m_overridePolicy, "overridePolicy");
				m_declaration.Serialize(archive);
				break;
			}
		case ESerializationPass::Edit:
			{
				IScriptFile&            file = CScriptElementBase::GetFile();
				Serialization::SContext fileContext(archive, static_cast<IScriptFile*>(&file)); // #SchematycTODO : Do we still need this?

				string typeName = GetScriptVariableTypeName(file, m_declaration.GetTypeId());
				archive(typeName, "typeName", "!Type");

				const bool bDerived = m_refGUID;
				if(bDerived)
				{
					archive(m_overridePolicy, "overridePolicy", "Override");
				}
				if(!bDerived || (m_overridePolicy != EOverridePolicy::Default))
				{
					m_declaration.SerializeValue(archive);
				}
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptProperty::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		m_guid      = guidRemapper.Remap(m_guid);
		m_scopeGUID = guidRemapper.Remap(m_scopeGUID);
		m_refGUID   = guidRemapper.Remap(m_refGUID);
		m_declaration.RemapGUIDs(guidRemapper);
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptProperty::GetRefGUID() const
	{
		return m_refGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	EOverridePolicy CScriptProperty::GetOverridePolicy() const
	{
		return m_overridePolicy;
	}

	//////////////////////////////////////////////////////////////////////////
	CAggregateTypeId CScriptProperty::GetTypeId() const
	{
		return m_declaration.GetTypeId();
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyConstPtr CScriptProperty::GetValue() const
	{
		return m_declaration.GetValue();
	}
}
