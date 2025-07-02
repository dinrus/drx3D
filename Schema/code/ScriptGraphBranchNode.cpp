// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphBranchNode.h>

#include <drx3D/Schema/IGraphNodeCompiler.h>

#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphNodeFactory.h>

namespace sxema
{
CScriptGraphBranchNode::CScriptGraphBranchNode() {}

DrxGUID CScriptGraphBranchNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphBranchNode::CreateLayout(CScriptGraphNodeLayout& layout)
{
	layout.SetName("Branch");
	layout.SetStyleId("Core::FlowControl");

	layout.AddInput("In", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::MultiLink });
	layout.AddInputWithData("Value", GetTypeDesc<bool>().GetGUID(), { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::Persistent, EScriptGraphPortFlags::Editable }, bool(false));

	layout.AddOutput("True", DrxGUID(), EScriptGraphPortFlags::Flow);
	layout.AddOutput("False", DrxGUID(), EScriptGraphPortFlags::Flow);
}

void CScriptGraphBranchNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	compiler.BindCallback(&Execute);
}

void CScriptGraphBranchNode::Register(CScriptGraphNodeFactory& factory)
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
				return "Branch";
			}

			virtual tukk GetSubject() const override
			{
				return nullptr;
			}

			virtual tukk GetDescription() const override
			{
				return "Branch flow";
			}

			virtual tukk GetStyleId() const override
			{
				return "Core::FlowControl";
			}

			virtual IScriptGraphNodePtr Execute(const Vec2& pos) override
			{
				return std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphBranchNode>(), pos);
			}

			// ~IScriptGraphNodeCreationCommand
		};

	public:

		// IScriptGraphNodeCreator

		virtual DrxGUID GetTypeGUID() const override
		{
			return CScriptGraphBranchNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphBranchNode>());
		}

		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) override
		{
			nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>());
		}

		// ~IScriptGraphNodeCreator
	};

	factory.RegisterCreator(std::make_shared<CCreator>());
}

SRuntimeResult CScriptGraphBranchNode::Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	const bool bValue = DynamicCast<bool>(*context.node.GetInputData(EInputIdx::Value));
	if (bValue)
	{
		return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::True);
	}
	else
	{
		return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::False);
	}
}

const DrxGUID CScriptGraphBranchNode::ms_typeGUID = "b30f637b-555a-4e2e-8558-87fa49d470c1"_drx_guid;
} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphBranchNode::Register)
