// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphReturnNode.h>

#include <drx3D/Schema/IGraphNodeCompiler.h>
#include <drx3D/Schema/IScriptFunction.h>
#include <drx3D/Schema/Any.h>

#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphNodeFactory.h>
#include <drx3D/Schema/SerializationContext.h>

namespace sxema
{

CScriptGraphReturnNode::CScriptGraphReturnNode() {}

DrxGUID CScriptGraphReturnNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphReturnNode::CreateLayout(CScriptGraphNodeLayout& layout)
{
	layout.SetName("Return");
	layout.SetStyleId("Core::FlowControl::End");

	layout.AddInput("In", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::MultiLink, EScriptGraphPortFlags::End });

	const IScriptElement& scriptElement = CScriptGraphNodeModel::GetNode().GetGraph().GetElement();
	switch (scriptElement.GetType())
	{
	case EScriptElementType::Function:
		{
			const IScriptFunction& scriptFunction = DynamicCast<IScriptFunction>(scriptElement);
			for (u32 functionOutputIdx = 0, functionOutputCount = scriptFunction.GetOutputCount(); functionOutputIdx < functionOutputCount; ++functionOutputIdx)
			{
				CAnyConstPtr pData = scriptFunction.GetOutputData(functionOutputIdx);
				if (pData)
				{
					layout.AddInputWithData(CUniqueId::FromGUID(scriptFunction.GetOutputGUID(functionOutputIdx)), scriptFunction.GetOutputName(functionOutputIdx), scriptFunction.GetOutputTypeId(functionOutputIdx).guid, { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::MultiLink, EScriptGraphPortFlags::Persistent, EScriptGraphPortFlags::Editable }, *pData);
				}
			}
			break;
		}
	}
}

void CScriptGraphReturnNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	compiler.BindCallback(&Execute);
}

void CScriptGraphReturnNode::Register(CScriptGraphNodeFactory& factory)
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
				return "Return";
			}

			virtual tukk GetSubject() const override
			{
				return nullptr;
			}

			virtual tukk GetDescription() const override
			{
				return "Return from function";
			}

			virtual tukk GetStyleId() const override
			{
				return "Core::FlowControl::End";
			}

			virtual IScriptGraphNodePtr Execute(const Vec2& pos) override
			{
				return std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphReturnNode>(), pos);
			}

			// ~IScriptGraphNodeCreationCommand
		};

	public:

		// IScriptGraphNodeCreator

		virtual DrxGUID GetTypeGUID() const override
		{
			return CScriptGraphReturnNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphReturnNode>());
		}

		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) override
		{
			switch (graph.GetType())
			{
			case EScriptGraphType::Function:
				{
					nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>());
					break;
				}
			}
		}

		// ~IScriptGraphNodeCreator
	};

	factory.RegisterCreator(std::make_shared<CCreator>());
}

SRuntimeResult CScriptGraphReturnNode::Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	for (u8 inputIdx = EInputIdx::FirstParam, inputCount = context.node.GetInputCount(); inputIdx < inputCount; ++inputIdx)
	{
		if (context.node.IsDataInput(inputIdx))
		{
			context.params.SetOutput(context.node.GetInputId(inputIdx), *context.node.GetInputData(inputIdx));
		}
	}

	return SRuntimeResult(ERuntimeStatus::Return);
}

const DrxGUID CScriptGraphReturnNode::ms_typeGUID = "f898c51c-717b-408d-aca0-75129b2ec4cf"_drx_guid;

} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphReturnNode::Register)
