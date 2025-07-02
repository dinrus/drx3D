// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphReceiveSignalNode.h>

#include <drx3D/CoreX/Serialization/Decorators/ActionButton.h>
#include <drx3D/Schema/IObject.h>
#include <drx3D/Schema/CompilerContext.h>
#include <drx3D/Schema/IGraphNodeCompiler.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvSignal.h>
#include <drx3D/Schema/IScriptRegistry.h>
#include <drx3D/Schema/IScriptSignal.h>
#include <drx3D/Schema/IScriptTimer.h>
#include <drx3D/Schema/IScriptComponentInstance.h>
#include <drx3D/Schema/IGUIDRemapper.h>
#include <drx3D/Schema/StackString.h>

#include <drx3D/Schema/RuntimeClass.h>
#include <drx3D/Schema/ScriptView.h>
#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphNodeFactory.h>
#include <drx3D/Schema/SerializationContext.h>

namespace sxema
{

CScriptGraphReceiveSignalNode::CScriptGraphReceiveSignalNode() {}

CScriptGraphReceiveSignalNode::CScriptGraphReceiveSignalNode(const SElementId& signalId, const DrxGUID& objectGUID)
	: m_signalId(signalId)
	, m_objectGUID(objectGUID)
{}

DrxGUID CScriptGraphReceiveSignalNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphReceiveSignalNode::CreateLayout(CScriptGraphNodeLayout& layout)
{
	layout.SetStyleId("Core::ReceiveSignal");

	stack_string subject;

	const IScriptElement* pScriptObject = gEnv->pSchematyc->GetScriptRegistry().GetElement(m_objectGUID);
	if (pScriptObject)
	{
		subject = pScriptObject->GetName();
		subject.append("::");
	}

	if (!GUID::IsEmpty(m_signalId.guid))
	{
		switch (m_signalId.domain)
		{
		case EDomain::Env:
			{
				const IEnvSignal* pEnvSignal = gEnv->pSchematyc->GetEnvRegistry().GetSignal(m_signalId.guid);
				if (pEnvSignal)
				{
					subject.append(pEnvSignal->GetName());

					layout.AddOutput("Out", m_signalId.guid, EScriptGraphPortFlags::Signal);

					const CClassDesc& signalDesc = pEnvSignal->GetDesc();
					for (const CClassMemberDesc& signalMemberDesc : signalDesc.GetMembers())
					{
						ukk pDefaultValue = signalMemberDesc.GetDefaultValue();
						SXEMA_CORE_ASSERT(pDefaultValue);
						if (pDefaultValue)
						{
							const CCommonTypeDesc& signalMemberTypeDesc = signalMemberDesc.GetTypeDesc();
							layout.AddOutputWithData(CUniqueId::FromUInt32(signalMemberDesc.GetId()), signalMemberDesc.GetLabel(), signalMemberTypeDesc.GetGUID(), { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::MultiLink }, CAnyConstRef(signalMemberTypeDesc, pDefaultValue));
						}
					}
				}
				break;
			}
		case EDomain::Script:
			{
				const IScriptElement* pScriptElement = gEnv->pSchematyc->GetScriptRegistry().GetElement(m_signalId.guid);
				if (pScriptElement)
				{
					switch (pScriptElement->GetType())
					{
					case EScriptElementType::Signal:
						{
							const IScriptSignal& scriptSignal = DynamicCast<IScriptSignal>(*pScriptElement);

							subject.append(scriptSignal.GetName());

							layout.AddOutput("Out", m_signalId.guid, EScriptGraphPortFlags::Signal);

							for (u32 signalInputIdx = 0, signalInputCount = scriptSignal.GetInputCount(); signalInputIdx < signalInputCount; ++signalInputIdx)
							{
								CAnyConstPtr pData = scriptSignal.GetInputData(signalInputIdx);
								SXEMA_CORE_ASSERT(pData);
								if (pData)
								{
									layout.AddOutputWithData(CUniqueId::FromGUID(scriptSignal.GetInputGUID(signalInputIdx)), scriptSignal.GetInputName(signalInputIdx), pData->GetTypeDesc().GetGUID(), { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::MultiLink }, *pData);
								}
							}
							break;
						}
					case EScriptElementType::Timer:
						{
							subject.append(pScriptElement->GetName());

							layout.AddOutput("Out", m_signalId.guid, EScriptGraphPortFlags::Signal);
							break;
						}
					}
				}
				break;
			}
		}
	}
	layout.SetName(nullptr, subject.c_str());
}

void CScriptGraphReceiveSignalNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	CRuntimeClass* pClass = context.interfaces.Query<CRuntimeClass>();
	if (pClass)
	{
		const IScriptElement* pOwner = CScriptGraphNodeModel::GetNode().GetGraph().GetElement().GetParent();
		switch (pOwner->GetType())
		{
		case EScriptElementType::Class:
			{
				pClass->AddSignalReceiver(m_signalId.guid, m_objectGUID, compiler.GetGraphIdx(), SRuntimeActivationParams(compiler.GetGraphNodeIdx(), EOutputIdx::Out, EActivationMode::Output));
				compiler.BindCallback(m_signalId.domain == EDomain::Env ? &ExecuteReceiveEnvSignal : &ExecuteReceiveScriptSignal);
				break;
			}
		case EScriptElementType::State:
			{
				u32k stateIdx = pClass->FindState(pOwner->GetGUID());
				if (stateIdx != InvalidIdx)
				{
					pClass->AddStateSignalReceiver(stateIdx, m_signalId.guid, m_objectGUID, compiler.GetGraphIdx(), SRuntimeActivationParams(compiler.GetGraphNodeIdx(), EOutputIdx::Out, EActivationMode::Output));
					compiler.BindCallback(m_signalId.domain == EDomain::Env ? &ExecuteReceiveEnvSignal : &ExecuteReceiveScriptSignal);
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

void CScriptGraphReceiveSignalNode::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_signalId, "signalId");
	archive(m_objectGUID, "objectGUID");
}

void CScriptGraphReceiveSignalNode::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_signalId, "signalId");
	archive(m_objectGUID, "objectGUID");
}

void CScriptGraphReceiveSignalNode::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	Validate(archive, context);
}

void CScriptGraphReceiveSignalNode::Validate(Serialization::IArchive& archive, const ISerializationContext& context)
{
	if (!GUID::IsEmpty(m_signalId.guid))
	{
		switch (m_signalId.domain)
		{
		case EDomain::Env:
			{
				const IEnvSignal* pEnvSignal = gEnv->pSchematyc->GetEnvRegistry().GetSignal(m_signalId.guid);
				if (!pEnvSignal)
				{
					archive.error(*this, "Unable to retrieve environment signal!");
				}
				break;
			}
		case EDomain::Script:
			{
				const IScriptElement* pScriptElement = gEnv->pSchematyc->GetScriptRegistry().GetElement(m_signalId.guid);
				if (!pScriptElement)
				{
					archive.error(*this, "Unable to retrieve script signal!");
				}
				break;
			}
		}
	}
}

void CScriptGraphReceiveSignalNode::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	if (m_signalId.domain == EDomain::Script)
	{
		m_signalId.guid = guidRemapper.Remap(m_signalId.guid);
	}
}

void CScriptGraphReceiveSignalNode::Register(CScriptGraphNodeFactory& factory)
{
	class CCreator : public IScriptGraphNodeCreator
	{
	private:

		class CCreationCommand : public IScriptGraphNodeCreationCommand
		{
		public:

			CCreationCommand(tukk szSubject, const SElementId& signalId, const DrxGUID& objectGUID = DrxGUID())
				: m_subject(szSubject)
				, m_signalId(signalId)
				, m_objectGUID(objectGUID)
			{}

			// IScriptGraphNodeCreationCommand

			virtual tukk GetBehavior() const override
			{
				return "Signal::Receive";
			}

			virtual tukk GetSubject() const override
			{
				return m_subject.c_str();
			}

			virtual tukk GetDescription() const override
			{
				return "Receive signal";
			}

			virtual tukk GetStyleId() const override
			{
				return "Core::ReceiveSignal";
			}

			virtual IScriptGraphNodePtr Execute(const Vec2& pos) override
			{
				return std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphReceiveSignalNode>(m_signalId, m_objectGUID), pos);
			}

			// ~IScriptGraphNodeCreationCommand

		private:

			string     m_subject;
			SElementId m_signalId;
			DrxGUID      m_objectGUID;
		};

		struct SComponent
		{
			inline SComponent(const DrxGUID& _guid, const DrxGUID& _typeGUID, tukk szName)
				: guid(_guid)
				, typeGUID(_typeGUID)
				, name(szName)
			{}

			DrxGUID  guid;
			DrxGUID  typeGUID;
			string name;
		};

		typedef std::vector<SComponent> Components;

	public:

		// IScriptGraphNodeCreator

		virtual DrxGUID GetTypeGUID() const override
		{
			return CScriptGraphReceiveSignalNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphReceiveSignalNode>());
		}

		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) override
		{
			switch (graph.GetType())
			{
			case EScriptGraphType::Signal:
				{
					Components components;
					components.reserve(20);

					auto visitScriptComponentInstance = [&scriptView, &components](const IScriptComponentInstance& scriptComponentInstance) -> EVisitStatus
					{
						CStackString name;
						scriptView.QualifyName(scriptComponentInstance, EDomainQualifier::Global, name);
						components.emplace_back(scriptComponentInstance.GetGUID(), scriptComponentInstance.GetTypeGUID(), name.c_str());
						return EVisitStatus::Continue;
					};
					scriptView.VisitScriptComponentInstances(visitScriptComponentInstance, EDomainScope::Derived);

					auto visitEnvSignal = [&nodeCreationMenu, &scriptView, &components](const IEnvSignal& envSignal) -> EVisitStatus
					{
						const IEnvElement* pEnvScope = envSignal.GetParent();
						switch (pEnvScope->GetType())
						{
						case EEnvElementType::Component:
							{
								const DrxGUID componentTypeGUID = pEnvScope->GetGUID();
								for (const SComponent& component : components)
								{
									if (component.typeGUID == componentTypeGUID)
									{
										CStackString subject = component.name.c_str();
										subject.append("::");
										subject.append(envSignal.GetName());
										nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(subject.c_str(), SElementId(EDomain::Env, envSignal.GetGUID()), component.guid));
									}
								}
								break;
							}
						case EEnvElementType::Action:
							{
								break;
							}
						default:
							{
								CStackString subject;
								scriptView.QualifyName(envSignal, subject);
								nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(subject.c_str(), SElementId(EDomain::Env, envSignal.GetGUID())));
								break;
							}
						}
						return EVisitStatus::Continue;
					};
					scriptView.VisitEnvSignals(visitEnvSignal);

					auto visitScriptSignal = [&nodeCreationMenu, &scriptView](const IScriptSignal& scriptSignal)
					{
						CStackString subject;
						scriptView.QualifyName(scriptSignal, EDomainQualifier::Global, subject);
						nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(subject.c_str(), SElementId(EDomain::Script, scriptSignal.GetGUID())));
					};
					scriptView.VisitAccesibleSignals(visitScriptSignal);

					auto visitScriptTimer = [&nodeCreationMenu, &scriptView](const IScriptTimer& scriptTimer)
					{
						CStackString subject;
						scriptView.QualifyName(scriptTimer, EDomainQualifier::Global, subject);
						nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(subject.c_str(), SElementId(EDomain::Script, scriptTimer.GetGUID())));
					};
					scriptView.VisitAccesibleTimers(visitScriptTimer);

					break;
				}
			}
		}

		// ~IScriptGraphNodeCreator
	};

	factory.RegisterCreator(std::make_shared<CCreator>());
}

void CScriptGraphReceiveSignalNode::GoToSignal()
{
	DrxLinkUtils::ExecuteCommand(DrxLinkUtils::ECommand::Show, m_signalId.guid);
}

SRuntimeResult CScriptGraphReceiveSignalNode::ExecuteReceiveEnvSignal(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	for (u8 outputIdx = EOutputIdx::FirstParam, outputCount = context.node.GetOutputCount(); outputIdx < outputCount; ++outputIdx)
	{
		if (context.node.IsDataOutput(outputIdx))
		{
			CAnyConstPtr pSrcData = context.params.GetInput(context.node.GetOutputId(outputIdx));
			if (pSrcData)
			{
				Any::CopyAssign(*context.node.GetOutputData(outputIdx), *pSrcData);
			}
			else
			{
				return SRuntimeResult(ERuntimeStatus::Error);
			}
		}
	}

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}

SRuntimeResult CScriptGraphReceiveSignalNode::ExecuteReceiveScriptSignal(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	for (u8 outputIdx = EOutputIdx::FirstParam, outputCount = context.node.GetOutputCount(); outputIdx < outputCount; ++outputIdx)
	{
		if (context.node.IsDataOutput(outputIdx))
		{
			CAnyConstPtr pSrcData = context.params.GetInput(context.node.GetOutputId(outputIdx));
			if (pSrcData)
			{
				Any::CopyAssign(*context.node.GetOutputData(outputIdx), *pSrcData);
			}
			else
			{
				return SRuntimeResult(ERuntimeStatus::Error);
			}
		}
	}

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}

const DrxGUID CScriptGraphReceiveSignalNode::ms_typeGUID = "ad2aee64-0a60-4469-8ec7-38b4b720d30c"_drx_guid;

} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphReceiveSignalNode::Register)
