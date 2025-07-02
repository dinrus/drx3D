// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphForNode.h>

#include <drx3D/Schema/IGraphNodeCompiler.h>

#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphNodeFactory.h>

namespace sxema
{

CScriptGraphForNode::SRuntimeData::SRuntimeData()
	: pos(InvalidIdx)
	, bBreak(false)
{}

CScriptGraphForNode::SRuntimeData::SRuntimeData(const SRuntimeData& rhs)
	: pos(rhs.pos)
	, bBreak(rhs.bBreak)
{}

void CScriptGraphForNode::SRuntimeData::ReflectType(CTypeDesc<SRuntimeData>& desc)
{
	desc.SetGUID("2cf6b75a-caed-4d69-914a-08ca7b0e5a67"_drx_guid);
}

CScriptGraphForNode::CScriptGraphForNode() {}

DrxGUID CScriptGraphForNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphForNode::CreateLayout(CScriptGraphNodeLayout& layout)
{
	layout.SetName("For");
	layout.SetStyleId("Core::FlowControl");

	layout.AddInput("In", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::MultiLink });
	layout.AddInputWithData("Begin", GetTypeDesc<i32>().GetGUID(), { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::Persistent, EScriptGraphPortFlags::Editable }, i32(0));
	layout.AddInputWithData("End", GetTypeDesc<i32>().GetGUID(), { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::Persistent, EScriptGraphPortFlags::Editable }, i32(0));
	layout.AddInput("Break", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::MultiLink, EScriptGraphPortFlags::End });

	layout.AddOutput("Out", DrxGUID(), EScriptGraphPortFlags::Flow);
	layout.AddOutput("Loop", DrxGUID(), EScriptGraphPortFlags::Flow);
	layout.AddOutputWithData("Pos", GetTypeDesc<i32>().GetGUID(), { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::MultiLink }, i32(0));
}

void CScriptGraphForNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	compiler.BindCallback(&Execute);
	compiler.BindData(SRuntimeData());
}

void CScriptGraphForNode::Register(CScriptGraphNodeFactory& factory)
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
				return "For";
			}

			virtual tukk GetSubject() const override
			{
				return nullptr;
			}

			virtual tukk GetDescription() const override
			{
				return "For loop";
			}

			virtual tukk GetStyleId() const override
			{
				return "Core::FlowControl";
			}

			virtual IScriptGraphNodePtr Execute(const Vec2& pos) override
			{
				return std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphForNode>(), pos);
			}

			// ~IScriptGraphNodeCreationCommand
		};

	public:

		// IScriptGraphNodeCreator

		virtual DrxGUID GetTypeGUID() const override
		{
			return CScriptGraphForNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphForNode>());
		}

		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) override
		{
			nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>());
		}

		// ~IScriptGraphNodeCreator
	};

	factory.RegisterCreator(std::make_shared<CCreator>());
}

SRuntimeResult CScriptGraphForNode::Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	SRuntimeData& data = DynamicCast<SRuntimeData>(*context.node.GetData());

	if (activationParams.mode == EActivationMode::Input)
	{
		switch (activationParams.portIdx)
		{
		case EInputIdx::In:
			{
				data.pos = DynamicCast<i32>(*context.node.GetInputData(EInputIdx::Begin));
				break;
			}
		case EInputIdx::Break:
			{
				data.bBreak;
				return SRuntimeResult(ERuntimeStatus::Break);
			}
		}
	}

	if (!data.bBreak)
	{
		DynamicCast<i32>(*context.node.GetOutputData(EOutputIdx::Pos)) = data.pos;

		i32k end = DynamicCast<i32>(*context.node.GetInputData(EInputIdx::End));
		if (data.pos < end)
		{
			++data.pos;
			return SRuntimeResult(ERuntimeStatus::ContinueAndRepeat, EOutputIdx::Loop);
		}
	}

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}

const DrxGUID CScriptGraphForNode::ms_typeGUID = "a902d2a5-cc66-49e0-8c2e-e52b48cc7159"_drx_guid;

} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphForNode::Register)
