// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptFunction.h>

#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/Any.h>

#include <drx3D/Schema/ScriptGraph.h>
#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphBeginNode.h>

namespace sxema
{
CScriptFunction::CScriptFunction()
	: CScriptElementBase(EScriptElementFlags::CanOwnScript)
{
	CreateGraph();
}

CScriptFunction::CScriptFunction(const DrxGUID& guid, tukk szName)
	: CScriptElementBase(guid, szName, EScriptElementFlags::CanOwnScript)
{
	CreateGraph();
}

void CScriptFunction::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const
{
	ScriptParam::EnumerateDependencies(m_inputs, enumerator, type);
	ScriptParam::EnumerateDependencies(m_outputs, enumerator, type);
}

void CScriptFunction::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	ScriptParam::RemapDependencies(m_inputs, guidRemapper);
	ScriptParam::RemapDependencies(m_outputs, guidRemapper);
}

void CScriptFunction::ProcessEvent(const SScriptEvent& event)
{
	CScriptElementBase::ProcessEvent(event);

	switch (event.id)
	{
	case EScriptEventId::EditorAdd:
		{
			// TODO: This should happen in editor!
			IScriptGraph* pGraph = static_cast<IScriptGraph*>(CScriptElementBase::GetExtensions().QueryExtension(EScriptExtensionType::Graph));
			SXEMA_CORE_ASSERT(pGraph);
			if (pGraph)
			{
				// TODO : Shouldn't we be using CScriptGraphNodeFactory::CreateNode() instead of instantiating the node directly?!?
				pGraph->AddNode(std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphBeginNode>()));
				// ~TODO
			}

			m_userDocumentation.SetCurrentUserAsAuthor();
			// ~TODO

			break;
		}
	case EScriptEventId::EditorPaste:
		{
			m_userDocumentation.SetCurrentUserAsAuthor();
			break;
		}
	}
}

void CScriptFunction::Serialize(Serialization::IArchive& archive)
{
	// #SchematycTODO : Shouldn't this be handled by CScriptElementBase itself?
	CScriptElementBase::Serialize(archive);
	CMultiPassSerializer::Serialize(archive);
	CScriptElementBase::SerializeExtensions(archive);
}

tukk CScriptFunction::GetAuthor() const
{
	return m_userDocumentation.author.c_str();
}

tukk CScriptFunction::GetDescription() const
{
	return m_userDocumentation.description.c_str();
}

u32 CScriptFunction::GetInputCount() const
{
	return m_inputs.size();
}

DrxGUID CScriptFunction::GetInputGUID(u32 inputIdx) const
{
	return inputIdx < m_inputs.size() ? m_inputs[inputIdx].guid : DrxGUID();
}

tukk CScriptFunction::GetInputName(u32 inputIdx) const
{
	return inputIdx < m_inputs.size() ? m_inputs[inputIdx].name.c_str() : "";
}

SElementId CScriptFunction::GetInputTypeId(u32 inputIdx) const
{
	return inputIdx < m_inputs.size() ? m_inputs[inputIdx].data.GetTypeId() : SElementId();
}

CAnyConstPtr CScriptFunction::GetInputData(u32 inputIdx) const
{
	return inputIdx < m_inputs.size() ? m_inputs[inputIdx].data.GetValue() : CAnyConstPtr();
}

u32 CScriptFunction::GetOutputCount() const
{
	return m_outputs.size();
}

DrxGUID CScriptFunction::GetOutputGUID(u32 outputIdx) const
{
	return outputIdx < m_outputs.size() ? m_outputs[outputIdx].guid : DrxGUID();
}

tukk CScriptFunction::GetOutputName(u32 outputIdx) const
{
	return outputIdx < m_outputs.size() ? m_outputs[outputIdx].name.c_str() : "";
}

SElementId CScriptFunction::GetOutputTypeId(u32 outputIdx) const
{
	return outputIdx < m_outputs.size() ? m_outputs[outputIdx].data.GetTypeId() : SElementId();
}

CAnyConstPtr CScriptFunction::GetOutputData(u32 outputIdx) const
{
	return outputIdx < m_outputs.size() ? m_outputs[outputIdx].data.GetValue() : CAnyConstPtr();
}

void CScriptFunction::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_inputs, "inputs");
	archive(m_outputs, "outputs");
}

void CScriptFunction::Load(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_inputs, "inputs");
	archive(m_outputs, "outputs");
	archive(m_userDocumentation, "userDocumentation");
}

void CScriptFunction::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_inputs, "inputs");
	archive(m_outputs, "outputs");
	archive(m_userDocumentation, "userDocumentation");
}

void CScriptFunction::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
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

void CScriptFunction::CreateGraph()
{
	CScriptElementBase::AddExtension(std::make_shared<CScriptGraph>(*this, EScriptGraphType::Function));
}
} // sxema
