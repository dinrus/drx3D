// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphBeginNode.h>

#include <drx3D/Schema/CompilerContext.h>
#include <drx3D/Schema/IGraphNodeCompiler.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IScriptRegistry.h>
#include <drx3D/Schema/IScriptFunction.h>
#include <drx3D/Schema/Any.h>

#include <drx3D/Schema/RuntimeClass.h>
#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphNodeFactory.h>

namespace sxema
{

CScriptGraphBeginNode::CScriptGraphBeginNode() {}

void CScriptGraphBeginNode::Init()
{
	CScriptGraphNodeModel::GetNode().SetFlags({ EScriptGraphNodeFlags::NotCopyable, EScriptGraphNodeFlags::NotRemovable });
}

DrxGUID CScriptGraphBeginNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphBeginNode::CreateLayout(CScriptGraphNodeLayout& layout)
{
	layout.SetName("Begin");
	layout.SetStyleId("Core::FlowControl::Begin");
	layout.AddOutput("Out", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::Begin });

	const IScriptElement& scriptElement = CScriptGraphNodeModel::GetNode().GetGraph().GetElement();
	switch (scriptElement.GetType())
	{
	case EScriptElementType::Function:
		{
			const IScriptFunction& scriptFunction = DynamicCast<IScriptFunction>(scriptElement);
			for (u32 functionInputIdx = 0, functionInputCount = scriptFunction.GetInputCount(); functionInputIdx < functionInputCount; ++functionInputIdx)
			{
				CAnyConstPtr pData = scriptFunction.GetInputData(functionInputIdx);
				if (pData)
				{
					layout.AddOutputWithData(CUniqueId::FromGUID(scriptFunction.GetInputGUID(functionInputIdx)), scriptFunction.GetInputName(functionInputIdx), scriptFunction.GetInputTypeId(functionInputIdx).guid, { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::MultiLink }, *pData);
				}
			}
			break;
		}
	}
}

void CScriptGraphBeginNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	CRuntimeClass* pClass = context.interfaces.Query<CRuntimeClass>();
	if (pClass)
	{
		const IScriptElement& scriptElement = CScriptGraphNodeModel::GetNode().GetGraph().GetElement();
		switch (scriptElement.GetType())
		{
		case EScriptElementType::Constructor:
			{
				pClass->AddConstructor(compiler.GetGraphIdx(), SRuntimeActivationParams(compiler.GetGraphNodeIdx(), EOutputIdx::Out, EActivationMode::Output));
				compiler.BindCallback(&Execute);
				break;
			}
		case EScriptElementType::Function:
			{
				pClass->AddFunction(scriptElement.GetGUID(), compiler.GetGraphIdx(), SRuntimeActivationParams(compiler.GetGraphNodeIdx(), EOutputIdx::Out, EActivationMode::Output));
				compiler.BindCallback(&ExecuteFunction);
				break;
			}
		case EScriptElementType::StateMachine:
			{
				u32k stateMachineIdx = pClass->FindStateMachine(scriptElement.GetGUID());
				if (stateMachineIdx != InvalidIdx)
				{
					pClass->SetStateMachineBeginFunction(stateMachineIdx, compiler.GetGraphIdx(), SRuntimeActivationParams(compiler.GetGraphNodeIdx(), EOutputIdx::Out, EActivationMode::Output));
					compiler.BindCallback(&Execute);
				}
				else
				{
					SXEMA_COMPILER_ERROR("Failed to retrieve class state machine!");
				}
				break;
			}
		}
	}
}

void CScriptGraphBeginNode::Register(CScriptGraphNodeFactory& factory)
{
	class CCreator : public IScriptGraphNodeCreator
	{
	public:

		// IScriptGraphNodeCreator

		virtual DrxGUID GetTypeGUID() const override
		{
			return CScriptGraphBeginNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphBeginNode>());
		}

		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) override {}

		// ~IScriptGraphNodeCreator
	};

	factory.RegisterCreator(std::make_shared<CCreator>());
}

SRuntimeResult CScriptGraphBeginNode::Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}

SRuntimeResult CScriptGraphBeginNode::ExecuteFunction(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	for (u8 outputIdx = EOutputIdx::FirstParam, outputCount = context.node.GetOutputCount(); outputIdx < outputCount; ++outputIdx)
	{
		if (context.node.IsDataOutput(outputIdx))
		{
			CAnyConstPtr pSrcValue = context.params.GetInput(context.node.GetOutputId(outputIdx));
			if (pSrcValue)
			{
				Any::CopyAssign(*context.node.GetOutputData(outputIdx), *pSrcValue);
			}
		}
	}

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}

const DrxGUID CScriptGraphBeginNode::ms_typeGUID = "12bdfa06-ba95-4e48-bb2d-bb48a7080abc"_drx_guid;

} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphBeginNode::Register)
