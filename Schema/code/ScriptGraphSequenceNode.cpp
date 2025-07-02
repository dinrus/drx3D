// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphSequenceNode.h>

#include <drx3D/Schema/IGraphNodeCompiler.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvDataType.h>
#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/IScriptEnum.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/StackString.h>
#include <drx3D/Schema/IGUIDRemapper.h>

#include <drx3D/Schema/ScriptVariableData.h>
#include <drx3D/Schema/ScriptView.h>
#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphNodeFactory.h>
#include <drx3D/Schema/SerializationContext.h>

namespace sxema
{
void CScriptGraphSequenceNode::SOutput::Serialize(Serialization::IArchive& archive)
{
	archive(name, "name", "^");

	if (archive.isValidation())
	{
		if (name.empty())
		{
			archive.error(*this, "Empty output!");
		}
		else if (bIsDuplicate)
		{
			archive.warning(*this, "Duplicate output!");
		}
	}
}

DrxGUID CScriptGraphSequenceNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphSequenceNode::CreateLayout(CScriptGraphNodeLayout& layout)
{
	layout.SetName("Sequence");
	layout.SetStyleId("Core::FlowControl");

	layout.AddInput("In", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::MultiLink });

	for (const SOutput& output : m_outputs)
	{
		if (!output.bIsDuplicate)
		{
			layout.AddOutput(output.name.c_str(), DrxGUID(), EScriptGraphPortFlags::Flow);
		}
	}
}

void CScriptGraphSequenceNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	compiler.BindCallback(&Execute);
}

void CScriptGraphSequenceNode::PostLoad(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_outputs, "outputs");
	RefreshOutputs();
}

void CScriptGraphSequenceNode::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_outputs, "outputs");
}

void CScriptGraphSequenceNode::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_outputs, "outputs", "Outputs");
	RefreshOutputs();
}

void CScriptGraphSequenceNode::Validate(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_outputs, "outputs");
}

void CScriptGraphSequenceNode::Register(CScriptGraphNodeFactory& factory)
{
	class CCreator : public IScriptGraphNodeCreator
	{
	private:

		class CCreationCommand : public IScriptGraphNodeCreationCommand
		{
		public:

			// IScriptGraphNodeCreationCommand

			virtual tukk GetBehavior() const override
			{
				return "Sequence";
			}

			virtual tukk GetSubject() const override
			{
				return nullptr;
			}

			virtual tukk GetDescription() const override
			{
				return "Split flow into multiple sequences";
			}

			virtual tukk GetStyleId() const override
			{
				return "Core::FlowControl";
			}

			virtual IScriptGraphNodePtr Execute(const Vec2& pos) override
			{
				return std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphSequenceNode>(), pos);
			}

			// ~IScriptGraphNodeCreationCommand
		};

	public:

		// IScriptGraphNodeCreator

		virtual DrxGUID GetTypeGUID() const override
		{
			return CScriptGraphSequenceNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphSequenceNode>());
		}

		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) override
		{
			nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>());
		}

		// ~IScriptGraphNodeCreator
	};

	factory.RegisterCreator(std::make_shared<CCreator>());
}

void CScriptGraphSequenceNode::RefreshOutputs()
{
	std::vector<string> outputNames;
	outputNames.reserve(m_outputs.size());

	for (SOutput& output : m_outputs)
	{
		output.bIsDuplicate = std::find(outputNames.begin(), outputNames.end(), output.name) != outputNames.end();
		if (!output.bIsDuplicate)
		{
			outputNames.push_back(output.name);
		}
	}
}

SRuntimeResult CScriptGraphSequenceNode::Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	u32k outputIdx = activationParams.mode == EActivationMode::Output ? activationParams.portIdx + 1 : 0;
	if (outputIdx < (context.node.GetOutputCount() - 1))
	{
		return SRuntimeResult(ERuntimeStatus::ContinueAndRepeat, outputIdx);
	}
	else
	{
		return SRuntimeResult(ERuntimeStatus::Continue, outputIdx);
	}
}

const DrxGUID CScriptGraphSequenceNode::ms_typeGUID = "87aa640f-b8c6-449f-bc37-2d5400009290"_drx_guid;
} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphSequenceNode::Register)
