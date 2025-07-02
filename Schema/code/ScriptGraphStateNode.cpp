// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphStateNode.h>

#include <drx3D/Schema/CompilerContext.h>
#include <drx3D/Schema/IGraphNodeCompiler.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvSignal.h>
#include <drx3D/Schema/IScriptRegistry.h>
#include <drx3D/Schema/IScriptSignal.h>
#include <drx3D/Schema/IScriptState.h>
#include <drx3D/Schema/IScriptTimer.h>
#include <drx3D/Schema/IScriptVariable.h>
#include <drx3D/Schema/IGUIDRemapper.h>
#include <drx3D/Schema/StackString.h>

#include <drx3D/Schema/RuntimeClass.h>
#include <drx3D/Schema/ScriptView.h>
#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphNodeFactory.h>
#include <drx3D/Schema/SerializationContext.h>

SERIALIZATION_ENUM_BEGIN_NESTED2(sxema, CScriptGraphStateNode, EOutputType, "DrxSchematyc Script Graph State Node Output Type")
SERIALIZATION_ENUM(sxema::CScriptGraphStateNode::EOutputType::EnvSignal, "EnvSignal", "Environment Signal")
SERIALIZATION_ENUM(sxema::CScriptGraphStateNode::EOutputType::ScriptSignal, "ScriptSignal", "Script Signal")
SERIALIZATION_ENUM(sxema::CScriptGraphStateNode::EOutputType::ScriptTimer, "ScriptTimer", "Script Timer")
SERIALIZATION_ENUM_END()

namespace sxema
{

CScriptGraphStateNode::SRuntimeData::SRuntimeData(u32 _stateIdx)
	: stateIdx(_stateIdx)
{}

CScriptGraphStateNode::SRuntimeData::SRuntimeData(const SRuntimeData& rhs)
	: stateIdx(rhs.stateIdx)
{}

void CScriptGraphStateNode::SRuntimeData::ReflectType(CTypeDesc<CScriptGraphStateNode::SRuntimeData>& desc)
{
	desc.SetGUID("3b0cfc25-30ed-44fb-afae-6b92e007bb0e"_drx_guid);
}

CScriptGraphStateNode::SOutputParams::SOutputParams()
	: type(EOutputType::Unknown)
{}

CScriptGraphStateNode::SOutputParams::SOutputParams(EOutputType _type, const DrxGUID& _guid)
	: type(_type)
	, guid(_guid)
{}

void CScriptGraphStateNode::SOutputParams::Serialize(Serialization::IArchive& archive)
{
	archive(type, "type");
	archive(guid, "guid");
}

bool CScriptGraphStateNode::SOutputParams::operator==(const SOutputParams& rhs) const
{
	return (type == rhs.type) && (guid == rhs.guid);
}

CScriptGraphStateNode::CScriptGraphStateNode() {}

CScriptGraphStateNode::CScriptGraphStateNode(const DrxGUID& stateGUID)
	: m_stateGUID(stateGUID)
{}

DrxGUID CScriptGraphStateNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphStateNode::CreateLayout(CScriptGraphNodeLayout& layout)
{
	tukk szSubject = nullptr;
	if (!GUID::IsEmpty(m_stateGUID))
	{
		const IScriptState* pScriptState = DynamicCast<IScriptState>(gEnv->pSchematyc->GetScriptRegistry().GetElement(m_stateGUID));
		if (pScriptState)
		{
			szSubject = pScriptState->GetName();
		}
	}
	layout.SetName("State", szSubject);

	layout.SetStyleId("Core::State");

	layout.AddInput("Select", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::MultiLink, EScriptGraphPortFlags::End });

	CScriptView scriptView(m_stateGUID);

	auto visitScriptSignal = [&layout, &scriptView](const IScriptSignal& scriptSignal)
	{
		layout.AddOutput(CUniqueId::FromGUID(scriptSignal.GetGUID()), scriptSignal.GetName(), scriptSignal.GetGUID(), { EScriptGraphPortFlags::Signal, EScriptGraphPortFlags::Begin });
	};
	scriptView.VisitEnclosedSignals(visitScriptSignal);

	auto visitScriptTimer = [&layout, &scriptView](const IScriptTimer& scriptTimer)
	{
		layout.AddOutput(CUniqueId::FromGUID(scriptTimer.GetGUID()), scriptTimer.GetName(), scriptTimer.GetGUID(), { EScriptGraphPortFlags::Signal, EScriptGraphPortFlags::Begin });
	};
	scriptView.VisitEnclosedTimers(visitScriptTimer);

	for (const Output& output : m_outputs)
	{
		switch (output.value.type)
		{
		case EOutputType::EnvSignal:
			{
				const IEnvSignal* pEnvSignal = gEnv->pSchematyc->GetEnvRegistry().GetSignal(output.value.guid);
				if (pEnvSignal)
				{
					layout.AddOutput(CUniqueId::FromGUID(output.value.guid), pEnvSignal->GetName(), output.value.guid, { EScriptGraphPortFlags::Signal, EScriptGraphPortFlags::Begin });
				}
				break;
			}
		case EOutputType::ScriptSignal:
			{
				const IScriptSignal* pScriptSignal = DynamicCast<IScriptSignal>(gEnv->pSchematyc->GetScriptRegistry().GetElement(output.value.guid));
				if (pScriptSignal)
				{
					layout.AddOutput(CUniqueId::FromGUID(output.value.guid), pScriptSignal->GetName(), output.value.guid, { EScriptGraphPortFlags::Signal, EScriptGraphPortFlags::Begin });
				}
			}
		case EOutputType::ScriptTimer:
			{
				const IScriptTimer* pScriptTimer = DynamicCast<IScriptTimer>(gEnv->pSchematyc->GetScriptRegistry().GetElement(output.value.guid));
				if (pScriptTimer)
				{
					layout.AddOutput(CUniqueId::FromGUID(output.value.guid), pScriptTimer->GetName(), output.value.guid, { EScriptGraphPortFlags::Signal, EScriptGraphPortFlags::Begin });
				}
				break;
			}
		}
	}
}

void CScriptGraphStateNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	CRuntimeClass* pClass = context.interfaces.Query<CRuntimeClass>();
	if (pClass)
	{
		u32k stateIdx = pClass->FindState(m_stateGUID);
		if (stateIdx != InvalidIdx)
		{
			compiler.BindCallback(&Execute);

			u32k stateIdx = pClass->FindState(m_stateGUID);
			compiler.BindData(SRuntimeData(stateIdx));

			const CScriptGraphNode& node = CScriptGraphNodeModel::GetNode();
			for (u32 outputIdx = 0, outputCount = node.GetOutputCount(); outputIdx < outputCount; ++outputIdx)
			{
				pClass->AddStateTransition(stateIdx, node.GetOutputTypeGUID(outputIdx), compiler.GetGraphIdx(), SRuntimeActivationParams(compiler.GetGraphNodeIdx(), outputIdx, EActivationMode::Output)); // #SchematycTODO : Pass output/transition type as well as guid?
			}
		}
		else
		{
			SXEMA_COMPILER_ERROR("Failed to retrieve class state!");
		}
	}
}

void CScriptGraphStateNode::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_stateGUID, "stateGUID");
	archive(m_outputs, "outputs");
}

void CScriptGraphStateNode::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_stateGUID, "stateGUID");
	archive(m_outputs, "outputs");
}

void CScriptGraphStateNode::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	SerializationUtils::CScopedQuickSearchConfig<SOutputParams> quickSearchConfig(archive, "Output", "::");
	{
		CScriptView scriptView(CScriptGraphNodeModel::GetNode().GetGraph().GetElement().GetGUID()); // #SchematycTODO : Add GetScriptView function to CScriptGraphNodeModel?

		auto visitEnvSignal = [&quickSearchConfig, &scriptView](const IEnvSignal& envSignal) -> EVisitStatus
		{
			const SOutputParams outputParams(EOutputType::EnvSignal, envSignal.GetGUID());

			CStackString fullName;
			scriptView.QualifyName(envSignal, fullName);

			quickSearchConfig.AddOption(envSignal.GetName(), outputParams, fullName.c_str(), envSignal.GetDescription());
			return EVisitStatus::Continue;
		};
		scriptView.VisitEnvSignals(visitEnvSignal);

		auto visitScriptSignal = [&quickSearchConfig, &scriptView](const IScriptSignal& scriptSignal)
		{
			const SOutputParams outputParams(EOutputType::ScriptSignal, scriptSignal.GetGUID());

			CStackString fullName;
			scriptView.QualifyName(scriptSignal, EDomainQualifier::Global, fullName);

			quickSearchConfig.AddOption(scriptSignal.GetName(), outputParams, fullName.c_str(), scriptSignal.GetDescription());
		};
		scriptView.VisitAccesibleSignals(visitScriptSignal);

		auto visitScriptTimer = [&quickSearchConfig, &scriptView](const IScriptTimer& scriptTimer)
		{
			const SOutputParams outputParams(EOutputType::ScriptTimer, scriptTimer.GetGUID());

			CStackString fullName;
			scriptView.QualifyName(scriptTimer, EDomainQualifier::Global, fullName);

			quickSearchConfig.AddOption(scriptTimer.GetName(), outputParams, fullName.c_str(), scriptTimer.GetDescription());
		};
		scriptView.VisitAccesibleTimers(visitScriptTimer);
	}
	archive(m_outputs, "outputs", "Outputs");

	Validate(archive, context);
}

void CScriptGraphStateNode::Validate(Serialization::IArchive& archive, const ISerializationContext& context)
{
	if (!GUID::IsEmpty(m_stateGUID))
	{
		const IScriptState* pScriptState = DynamicCast<IScriptState>(gEnv->pSchematyc->GetScriptRegistry().GetElement(m_stateGUID));
		if (!pScriptState)
		{
			archive.error(*this, "Failed to retrieve state!");
		}
	}

	// #SchematycTODO : We should also validate outputs.
}

void CScriptGraphStateNode::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	for (Output& output : m_outputs)
	{
		switch (output.value.type)
		{
		case EOutputType::ScriptSignal:
		case EOutputType::ScriptTimer:
			{
				output.value.guid = guidRemapper.Remap(output.value.guid);
				break;
			}
		}
	}
}

void CScriptGraphStateNode::Register(CScriptGraphNodeFactory& factory)
{
	class CCreator : public IScriptGraphNodeCreator
	{
	private:

		class CCreationCommand : public IScriptGraphNodeCreationCommand
		{
		public:

			CCreationCommand(tukk szSubject, const DrxGUID& stateGUID)
				: m_subject(szSubject)
				, m_stateGUID(stateGUID)
			{}

			// IScriptGraphNodeCreationCommand

			virtual tukk GetBehavior() const override
			{
				return "State";
			}

			virtual tukk GetSubject() const override
			{
				return m_subject.c_str();
			}

			virtual tukk GetDescription() const override
			{
				return nullptr;
			}

			virtual tukk GetStyleId() const override
			{
				return "Core::State";
			}

			virtual IScriptGraphNodePtr Execute(const Vec2& pos) override
			{
				return std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphStateNode>(m_stateGUID), pos);
			}

			// ~IScriptGraphNodeCreationCommand

		private:

			string m_subject;
			DrxGUID  m_stateGUID;
		};

	public:

		// IScriptGraphNodeCreator

		virtual DrxGUID GetTypeGUID() const override
		{
			return CScriptGraphStateNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphStateNode>());
		}

		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) override
		{
			switch (graph.GetType())
			{
			case EScriptGraphType::Transition:
				{
					auto visitScriptState = [&nodeCreationMenu, &scriptView](const IScriptState& scriptState)
					{
						CStackString subject;
						scriptView.QualifyName(scriptState, EDomainQualifier::Local, subject);
						nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(subject.c_str(), scriptState.GetGUID()));
					};
					scriptView.VisitAccesibleStates(visitScriptState);
					break;
				}
			}
		}

		// ~IScriptGraphNodeCreator
	};

	factory.RegisterCreator(std::make_shared<CCreator>());
}

SRuntimeResult CScriptGraphStateNode::Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	if (activationParams.IsInput(EInputIdx::In))
	{
		const SRuntimeData& data = DynamicCast<SRuntimeData>(*context.node.GetData());
		context.params.SetOutput(CUniqueId::FromUInt32(0), data.stateIdx); // #SchematycTODO : Create enum of transition function input and output ids!
		return SRuntimeResult(ERuntimeStatus::Return);
	}
	else
	{
		return SRuntimeResult(ERuntimeStatus::Continue, activationParams.portIdx);
	}
}

const DrxGUID CScriptGraphStateNode::ms_typeGUID = "4e1f8b82-1a47-4679-9f29-7f28df27cf35"_drx_guid;

} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphStateNode::Register)
