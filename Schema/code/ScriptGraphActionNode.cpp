// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphActionNode.h>

#include <drx3D/Schema/CompilerContext.h>
#include <drx3D/Schema/IGraphNodeCompiler.h>
#include <drx3D/Schema/EnvUtils.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvAction.h>
#include <drx3D/Schema/IEnvSignal.h>
#include <drx3D/Schema/ActionDesc.h>
#include <drx3D/Schema/IGUIDRemapper.h>
#include <drx3D/Schema/StackString.h>

#include <drx3D/Schema/Object.h>
#include <drx3D/Schema/RuntimeClass.h>
#include <drx3D/Schema/ScriptView.h>
#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphNodeFactory.h>
#include <drx3D/Schema/SerializationContext.h>

namespace sxema
{

CScriptGraphActionNode::SRuntimeData::SRuntimeData(u32 _actionIdx)
	: actionIdx(_actionIdx)
{}

CScriptGraphActionNode::SRuntimeData::SRuntimeData(const SRuntimeData& rhs)
	: actionIdx(rhs.actionIdx)
{}

void CScriptGraphActionNode::SRuntimeData::ReflectType(CTypeDesc<CScriptGraphActionNode::SRuntimeData>& desc)
{
	desc.SetGUID("691015af-4d45-4644-8214-b984ab860dd2"_drx_guid);
}

CScriptGraphActionNode::CScriptGraphActionNode() {}

CScriptGraphActionNode::CScriptGraphActionNode(const DrxGUID& actionTypeGUID)
	: m_actionTypeGUID(actionTypeGUID)
{}

DrxGUID CScriptGraphActionNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphActionNode::CreateLayout(CScriptGraphNodeLayout& layout)
{
	layout.SetStyleId("Core::Function"); // #SchematycTODO : Actions should have their own unique style!!!

	layout.AddInput("Start", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::MultiLink, EScriptGraphPortFlags::End });

	tukk szSubject = nullptr;
	const IEnvAction* pEnvAction = gEnv->pSchematyc->GetEnvRegistry().GetAction(m_actionTypeGUID);
	if (pEnvAction)
	{
		szSubject = pEnvAction->GetName();

		const CActionDesc& actionDesc = pEnvAction->GetDesc();
		for (const CClassMemberDesc& actionMemberDesc : actionDesc.GetMembers())
		{
			ukk pDefaultValue = actionMemberDesc.GetDefaultValue();
			if (pDefaultValue)
			{
				const CCommonTypeDesc& actionMemberTypeDesc = actionMemberDesc.GetTypeDesc();
				layout.AddInputWithData(CUniqueId::FromUInt32(actionMemberDesc.GetId()), actionMemberDesc.GetLabel(), actionMemberTypeDesc.GetGUID(), { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::Persistent, EScriptGraphPortFlags::Editable }, CAnyConstRef(actionMemberTypeDesc, pDefaultValue));
			}
		}

		auto visitEnvSignal = [&layout](const IEnvSignal& envSignal)
		{
			layout.AddOutput(CUniqueId::FromGUID(envSignal.GetGUID()), envSignal.GetName(), envSignal.GetGUID(), { EScriptGraphPortFlags::Signal, EScriptGraphPortFlags::Begin });
		};
		EnvUtils::VisitChildren<IEnvSignal>(*pEnvAction, visitEnvSignal);
	}
	layout.SetName(nullptr, szSubject);
}

void CScriptGraphActionNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	CRuntimeClass* pClass = context.interfaces.Query<CRuntimeClass>();
	if (pClass)
	{
		const IEnvAction* pEnvAction = gEnv->pSchematyc->GetEnvRegistry().GetAction(m_actionTypeGUID);
		if (pEnvAction)
		{
			const DrxGUID guid = CScriptGraphNodeModel::GetNode().GetGUID();

			const IScriptElement* pOwner = CScriptGraphNodeModel::GetNode().GetGraph().GetElement().GetParent();
			switch (pOwner->GetType())
			{
			case EScriptElementType::Class:
				{
					u32k actionIdx = pClass->AddAction(guid, m_actionTypeGUID);
					compiler.BindCallback(&Execute);
					compiler.BindData(SRuntimeData(actionIdx));

					u32 outputIdx = 0;

					auto visitEnvSignal = [&compiler, pClass, &guid, &outputIdx](const IEnvSignal& envSignal)
					{
						pClass->AddSignalReceiver(envSignal.GetGUID(), guid, compiler.GetGraphIdx(), SRuntimeActivationParams(compiler.GetGraphNodeIdx(), outputIdx++, EActivationMode::Output));
					};
					EnvUtils::VisitChildren<IEnvSignal>(*pEnvAction, visitEnvSignal);
					break;
				}
			case EScriptElementType::State:
				{
					u32k stateIdx = pClass->FindState(pOwner->GetGUID());
					if (stateIdx != InvalidIdx)
					{
						u32k actionIdx = pClass->AddStateAction(stateIdx, guid, m_actionTypeGUID);
						compiler.BindCallback(&Execute);
						compiler.BindData(SRuntimeData(actionIdx));

						u32 outputIdx = 0;

						auto visitEnvSignal = [&compiler, pClass, stateIdx, &guid, &outputIdx](const IEnvSignal& envSignal)
						{
							pClass->AddStateSignalReceiver(stateIdx, envSignal.GetGUID(), guid, compiler.GetGraphIdx(), SRuntimeActivationParams(compiler.GetGraphNodeIdx(), outputIdx++, EActivationMode::Output));
						};
						EnvUtils::VisitChildren<IEnvSignal>(*pEnvAction, visitEnvSignal);
					}
					else
					{
						SXEMA_COMPILER_ERROR("Failed to retrieve class state!");
					}
					break;
				}
			}

			
		}
	}
}

void CScriptGraphActionNode::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_actionTypeGUID, "actionTypeGUID");
}

void CScriptGraphActionNode::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_actionTypeGUID, "actionTypeGUID");
}

void CScriptGraphActionNode::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	Validate(archive, context);
}

void CScriptGraphActionNode::Validate(Serialization::IArchive& archive, const ISerializationContext& context) {}

void CScriptGraphActionNode::Register(CScriptGraphNodeFactory& factory)
{
	class CCreator : public IScriptGraphNodeCreator
	{
	private:

		class CCreationCommand : public IScriptGraphNodeCreationCommand
		{
		public:

			CCreationCommand(tukk szSubject, const DrxGUID& actionTypeGUID)
				: m_subject(szSubject)
				, m_actionTypeGUID(actionTypeGUID)
			{}

			// IScriptGraphNodeCreationCommand

			virtual tukk GetBehavior() const override
			{
				return "Action";
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
				return "Core::Action";
			}

			virtual IScriptGraphNodePtr Execute(const Vec2& pos) override
			{
				return std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphActionNode>(m_actionTypeGUID), pos);
			}

			// ~IScriptGraphNodeCreationCommand

		private:

			string m_subject;
			DrxGUID  m_actionTypeGUID;
		};

	public:

		// IScriptGraphNodeCreator

		virtual DrxGUID GetTypeGUID() const override
		{
			return CScriptGraphActionNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphActionNode>());
		}

		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) override
		{
			if (!gEnv->pSchematyc->IsExperimentalFeatureEnabled("ActionNode"))
			{
				return;
			}

			switch (graph.GetType())
			{
			case EScriptGraphType::Signal:
				{
					nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>("Timer", "6937eddc-f25c-44dc-a759-501d2e5da0df"_drx_guid));
					nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>("DebugText", "8ea4441b-e080-4cca-8b3e-973e017404d3"_drx_guid));
					break;
				}
			}
		}

		// ~IScriptGraphNodeCreator
	};

	factory.RegisterCreator(std::make_shared<CCreator>());
}

SRuntimeResult CScriptGraphActionNode::Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	if (activationParams.IsInput(EInputIdx::Start))
	{
		const SRuntimeData& data = DynamicCast<SRuntimeData>(*context.node.GetData());
		CObject* pObject = static_cast<CObject*>(context.pObject);

		StackRuntimeParamMap params;
		context.node.BindParams(params);

		pObject->StartAction(data.actionIdx, params);

		return SRuntimeResult(ERuntimeStatus::Break);
	}
	else
	{
		return SRuntimeResult(ERuntimeStatus::Continue, activationParams.portIdx);
	}
}

const DrxGUID CScriptGraphActionNode::ms_typeGUID = "4bb42a1b-6268-4c7a-9c13-37192f874cb1"_drx_guid;

} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphActionNode::Register)
