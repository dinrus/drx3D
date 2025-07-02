// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptInterfaceFunction.h>

#include <drx3D/CoreX/Serialization/BlackBox.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IScriptView.h>
#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/SerializationUtils.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/IGUIDRemapper.h>

namespace sxema
{
CScriptInterfaceFunction::CScriptInterfaceFunction()
	: CScriptElementBase()
{}

CScriptInterfaceFunction::CScriptInterfaceFunction(const DrxGUID& guid, tukk szName)
	: CScriptElementBase(guid, szName)
{}

void CScriptInterfaceFunction::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const
{
	ScriptParam::EnumerateDependencies(m_inputs, enumerator, type);
	ScriptParam::EnumerateDependencies(m_outputs, enumerator, type);
}

void CScriptInterfaceFunction::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	ScriptParam::RemapDependencies(m_inputs, guidRemapper);
	ScriptParam::RemapDependencies(m_outputs, guidRemapper);
}

void CScriptInterfaceFunction::ProcessEvent(const SScriptEvent& event)
{
	CScriptElementBase::ProcessEvent(event);

	switch (event.id)
	{
	case EScriptEventId::EditorAdd:
	case EScriptEventId::EditorPaste:
		{
			m_userDocumentation.SetCurrentUserAsAuthor();
			break;
		}
	}
}

void CScriptInterfaceFunction::Serialize(Serialization::IArchive& archive)
{
	// #SchematycTODO : Shouldn't this be handled by CScriptElementBase itself?
	CScriptElementBase::Serialize(archive);
	CMultiPassSerializer::Serialize(archive);
	CScriptElementBase::SerializeExtensions(archive);
}

tukk CScriptInterfaceFunction::GetAuthor() const
{
	return m_userDocumentation.author.c_str();
}

tukk CScriptInterfaceFunction::GetDescription() const
{
	return m_userDocumentation.description.c_str();
}

u32 CScriptInterfaceFunction::GetInputCount() const
{
	return m_inputs.size();
}

tukk CScriptInterfaceFunction::GetInputName(u32 inputIdx) const
{
	return inputIdx < m_inputs.size() ? m_inputs[inputIdx].name.c_str() : "";
}

CAnyConstPtr CScriptInterfaceFunction::GetInputValue(u32 inputIdx) const
{
	return inputIdx < m_inputs.size() ? m_inputs[inputIdx].data.GetValue() : CAnyConstPtr();
}

u32 CScriptInterfaceFunction::GetOutputCount() const
{
	return m_outputs.size();
}

tukk CScriptInterfaceFunction::GetOutputName(u32 outputIdx) const
{
	return outputIdx < m_outputs.size() ? m_outputs[outputIdx].name.c_str() : "";
}

CAnyConstPtr CScriptInterfaceFunction::GetOutputValue(u32 outputIdx) const
{
	return outputIdx < m_outputs.size() ? m_outputs[outputIdx].data.GetValue() : CAnyConstPtr();
}

void CScriptInterfaceFunction::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_inputs, "inputs");
	archive(m_outputs, "outputs");
}
void CScriptInterfaceFunction::Load(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_inputs, "inputs");
	archive(m_outputs, "outputs");
	archive(m_userDocumentation, "userDocumentation");
}

void CScriptInterfaceFunction::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_inputs, "inputs");
	archive(m_outputs, "outputs");
	archive(m_userDocumentation, "userDocumentation");
}

void CScriptInterfaceFunction::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	{
		ScriptVariableData::CScopedSerializationConfig serializationConfig(archive);

		const DrxGUID guid = GetGUID();
		serializationConfig.DeclareEnvDataTypes(guid);
		serializationConfig.DeclareScriptEnums(guid);
		serializationConfig.DeclareScriptStructs(guid);

		archive(m_inputs, "inputs", "Inputs");
		archive(m_outputs, "outputs", "Outputs");
	}

	archive(m_userDocumentation, "userDocumentation", "Documentation");
}
} // sxema
