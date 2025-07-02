// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptAbstractInterfaceFunction.h>

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
	CScriptAbstractInterfaceFunction::CScriptAbstractInterfaceFunction(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName)
		: CScriptElementBase(EScriptElementType::AbstractInterfaceFunction, file)
		, m_guid(guid)
		, m_scopeGUID(scopeGUID)
		, m_name(szName)
	{}

	//////////////////////////////////////////////////////////////////////////
	EAccessor CScriptAbstractInterfaceFunction::GetAccessor() const
	{
		return EAccessor::Private;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptAbstractInterfaceFunction::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptAbstractInterfaceFunction::GetScopeGUID() const
	{
		return m_scopeGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptAbstractInterfaceFunction::SetName(tukk szName)
	{
		m_name = szName;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptAbstractInterfaceFunction::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceFunction::EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const
	{
		SXEMA2_SYSTEM_ASSERT(enumerator);
		if(enumerator)
		{
			for(const CScriptVariableDeclaration& input : m_inputs)
			{
				input.EnumerateDependencies(enumerator);
			}
			for(const CScriptVariableDeclaration& output : m_outputs)
			{
				output.EnumerateDependencies(enumerator);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceFunction::Refresh(const SScriptRefreshParams& params)
	{
		if(params.reason == EScriptRefreshReason::EditorAdd)
		{
			m_userDocumentation.SetCurrentUserAsAuthor();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceFunction::Serialize(Serialization::IArchive& archive)
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
				archive(m_inputs, "inputs");
				archive(m_outputs, "outputs");
				archive(m_userDocumentation, "userDocumentation", "Documentation");
				break;
			}
		case ESerializationPass::Load:
			{
				archive(m_inputs, "inputs");
				archive(m_outputs, "outputs");
				break;
			}
		case ESerializationPass::Save:
			{
				Serialization::SContext fileContext(archive, static_cast<IScriptFile*>(&CScriptElementBase::GetFile())); // #SchematycTODO : Do we still need this?
				archive(m_guid, "guid");
				archive(m_scopeGUID, "scope_guid");
				archive(m_name, "name");
				archive(m_inputs, "inputs");
				archive(m_outputs, "outputs");
				archive(m_userDocumentation, "userDocumentation", "Documentation");
				break;
			}
		case ESerializationPass::Edit:
			{
				Serialization::SContext fileContext(archive, static_cast<IScriptFile*>(&CScriptElementBase::GetFile())); // #SchematycTODO : Do we still need this?
				archive(m_inputs, "inputs", "Inputs");
				archive(m_outputs, "outputs", "Outputs");
				archive(m_userDocumentation, "userDocumentation", "Documentation");
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceFunction::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		m_guid      = guidRemapper.Remap(m_guid);
		m_scopeGUID = guidRemapper.Remap(m_scopeGUID);
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptAbstractInterfaceFunction::GetAuthor() const
	{
		return m_userDocumentation.author.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptAbstractInterfaceFunction::GetDescription() const
	{
		return m_userDocumentation.description.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CScriptAbstractInterfaceFunction::GetInputCount() const
	{
		return m_inputs.size();
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptAbstractInterfaceFunction::GetInputName(size_t inputIdx) const
	{
		return inputIdx < m_inputs.size() ? m_inputs[inputIdx].GetName() : "";
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyConstPtr CScriptAbstractInterfaceFunction::GetInputValue(size_t inputIdx) const
	{
		return inputIdx < m_inputs.size() ? m_inputs[inputIdx].GetValue() : IAnyConstPtr();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CScriptAbstractInterfaceFunction::GetOutputCount() const
	{
		return m_outputs.size();
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptAbstractInterfaceFunction::GetOutputName(size_t outputIdx) const
	{
		return outputIdx < m_outputs.size() ? m_outputs[outputIdx].GetName() : "";
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyConstPtr CScriptAbstractInterfaceFunction::GetOutputValue(size_t outputIdx) const
	{
		return outputIdx < m_outputs.size() ? m_outputs[outputIdx].GetValue() : IAnyConstPtr();
	}
}
