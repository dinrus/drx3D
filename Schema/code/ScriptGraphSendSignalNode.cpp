// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphSendSignalNode.h>

#include <drx3D/CoreX/Serialization/Decorators/ActionButton.h>
#include <drx3D/Schema/IObject.h>
#include <drx3D/Schema/CompilerContext.h>
#include <drx3D/Schema/IGraphNodeCompiler.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IScriptRegistry.h>
#include <drx3D/Schema/IScriptSignal.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/IGUIDRemapper.h>
#include <drx3D/Schema/StackString.h>

#include <drx3D/Schema/RuntimeClass.h>
#include <drx3D/Schema/ScriptView.h>
#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphNodeFactory.h>
#include <drx3D/Schema/SerializationContext.h>

#include <drx3D/Entity/IEntitySystem.h>

SERIALIZATION_ENUM_BEGIN_NESTED2(sxema, CScriptGraphSendSignalNode, ETarget, "DrxSchematyc Script Graph Send Signal Node Target")
SERIALIZATION_ENUM(sxema::CScriptGraphSendSignalNode::ETarget::Self, "Self", "Send To Self")
SERIALIZATION_ENUM(sxema::CScriptGraphSendSignalNode::ETarget::Object, "Object", "Send To Object")
SERIALIZATION_ENUM(sxema::CScriptGraphSendSignalNode::ETarget::Broadcast, "Broadcast", "Broadcast To All Objects")
SERIALIZATION_ENUM(sxema::CScriptGraphSendSignalNode::ETarget::Entity, "Entity", "Send to Entity")
SERIALIZATION_ENUM_END()

namespace sxema
{

CScriptGraphSendSignalNode::SRuntimeData::SRuntimeData(const DrxGUID& _signalGUID)
	: signalGUID(_signalGUID)
{}

CScriptGraphSendSignalNode::SRuntimeData::SRuntimeData(const SRuntimeData& rhs)
	: signalGUID(rhs.signalGUID)
{}

void CScriptGraphSendSignalNode::SRuntimeData::ReflectType(CTypeDesc<CScriptGraphSendSignalNode::SRuntimeData>& desc)
{
	desc.SetGUID("a88a4c08-22df-493b-ab27-973a893acefb"_drx_guid);
}

CScriptGraphSendSignalNode::CScriptGraphSendSignalNode()
	: m_target(ETarget::Entity)
{}

CScriptGraphSendSignalNode::CScriptGraphSendSignalNode(const DrxGUID& signalGUID)
	: m_signalGUID(signalGUID)
	, m_target(ETarget::Entity)
{}

DrxGUID CScriptGraphSendSignalNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphSendSignalNode::CreateLayout(CScriptGraphNodeLayout& layout)
{
	layout.SetStyleId("Core::SendSignal");

	tukk szSubject = nullptr;
	if (!GUID::IsEmpty(m_signalGUID))
	{
		layout.AddInput("In", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::MultiLink });
		layout.AddOutput("Out", DrxGUID(), EScriptGraphPortFlags::Flow);

		if (m_target == ETarget::Object)
		{
			layout.AddInputWithData("ObjectId", GetTypeDesc<ObjectId>().GetGUID(), { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::Persistent, EScriptGraphPortFlags::Editable }, ObjectId());
		}
		else if (m_target == ETarget::Entity)
		{
			layout.AddInputWithData("EntityId", GetTypeDesc<ExplicitEntityId>().GetGUID(), { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::Persistent, EScriptGraphPortFlags::Editable }, INVALID_ENTITYID);
		}

		const IScriptSignal* pScriptSignal = DynamicCast<IScriptSignal>(gEnv->pSchematyc->GetScriptRegistry().GetElement(m_signalGUID));
		if (pScriptSignal)
		{
			szSubject = pScriptSignal->GetName();

			for (u32 inputIdx = 0, inputCount = pScriptSignal->GetInputCount(); inputIdx < inputCount; ++inputIdx)
			{
				CAnyConstPtr pData = pScriptSignal->GetInputData(inputIdx);
				if (pData)
				{
					layout.AddInputWithData(CUniqueId::FromGUID(pScriptSignal->GetInputGUID(inputIdx)), pScriptSignal->GetInputName(inputIdx), pScriptSignal->GetInputTypeId(inputIdx).guid, { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::Persistent, EScriptGraphPortFlags::Editable }, *pData);
				}
			}
		}
	}
	layout.SetName(nullptr, szSubject);
}

void CScriptGraphSendSignalNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	CRuntimeClass* pClass = context.interfaces.Query<CRuntimeClass>();
	if (pClass)
	{
		if (!GUID::IsEmpty(m_signalGUID))
		{
			const IScriptSignal* pScriptSignal = DynamicCast<IScriptSignal>(gEnv->pSchematyc->GetScriptRegistry().GetElement(m_signalGUID));
			if (pScriptSignal)
			{
				switch (m_target)
				{
				case ETarget::Self:
					{
						compiler.BindCallback(&ExecuteSendToSelf);
						break;
					}
				case ETarget::Object:
					{
						compiler.BindCallback(&ExecuteSendToObject);
						break;
					}
				case ETarget::Entity:
				{
					compiler.BindCallback(&ExecuteSendToEntity);
					break;
				}
				case ETarget::Broadcast:
					{
						compiler.BindCallback(&ExecuteBroadcast);
						break;
					}
				}

				compiler.BindData(SRuntimeData(m_signalGUID));
			}
		}
	}
}

void CScriptGraphSendSignalNode::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_signalGUID, "signalGUID");
	archive(m_target, "target");
}

void CScriptGraphSendSignalNode::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_signalGUID, "signalGUID");
	archive(m_target, "target");
}

void CScriptGraphSendSignalNode::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_target, "target", "Target");

	Validate(archive, context);
}

void CScriptGraphSendSignalNode::Validate(Serialization::IArchive& archive, const ISerializationContext& context)
{
	if (!GUID::IsEmpty(m_signalGUID))
	{
		const IScriptSignal* pScriptSignal = DynamicCast<IScriptSignal>(gEnv->pSchematyc->GetScriptRegistry().GetElement(m_signalGUID));
		if (!pScriptSignal)
		{
			archive.error(*this, "Unable to retrieve script signal!");
		}
	}
}

void CScriptGraphSendSignalNode::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	m_signalGUID = guidRemapper.Remap(m_signalGUID);
}

void CScriptGraphSendSignalNode::Register(CScriptGraphNodeFactory& factory)
{
	class CCreator : public IScriptGraphNodeCreator
	{
	private:

		class CCreationCommand : public IScriptGraphNodeCreationCommand
		{
		public:

			CCreationCommand(tukk szSubject, const DrxGUID& signalGUID)
				: m_subject(szSubject)
				, m_signalGUID(signalGUID)
			{}

			// IScriptGraphNodeCreationCommand

			virtual tukk GetBehavior() const override
			{
				return "Signal::Send";
			}

			virtual tukk GetSubject() const override
			{
				return m_subject.c_str();
			}

			virtual tukk GetDescription() const override
			{
				return "Send signal";
			}

			virtual tukk GetStyleId() const override
			{
				return "Core::SendSignal";
			}

			virtual IScriptGraphNodePtr Execute(const Vec2& pos) override
			{
				return std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphSendSignalNode>(m_signalGUID), pos);
			}

			// ~IScriptGraphNodeCreationCommand

		private:

			string m_subject;
			DrxGUID  m_signalGUID;
		};

	public:

		// IScriptGraphNodeCreator

		virtual DrxGUID GetTypeGUID() const override
		{
			return CScriptGraphSendSignalNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphSendSignalNode>());
		}

		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) override
		{
			switch (graph.GetType())
			{
			case EScriptGraphType::Signal:
			case EScriptGraphType::Function:
				{
					auto visitScriptSignal = [&nodeCreationMenu, &scriptView](const IScriptSignal& scriptSignal)
					{
						CStackString subject;
						scriptView.QualifyName(scriptSignal, EDomainQualifier::Global, subject);
						nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(subject.c_str(), scriptSignal.GetGUID()));
					};
					scriptView.VisitAccesibleSignals(visitScriptSignal);
					break;
				}
			}
		}

		// ~IScriptGraphNodeCreator
	};

	factory.RegisterCreator(std::make_shared<CCreator>());
}

void CScriptGraphSendSignalNode::GoToSignal()
{
	DrxLinkUtils::ExecuteCommand(DrxLinkUtils::ECommand::Show, m_signalGUID);
}

SRuntimeResult CScriptGraphSendSignalNode::ExecuteSendToSelf(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	struct EInputIdx
	{
		enum : u32
		{
			In = 0,
			FirstParam
		};
	};

	const SRuntimeData& data = DynamicCast<SRuntimeData>(*context.node.GetData());
	SObjectSignal signal(data.signalGUID);

	for (u8 inputIdx = EInputIdx::FirstParam, inputCount = context.node.GetInputCount(); inputIdx < inputCount; ++inputIdx)
	{
		if (context.node.IsDataInput(inputIdx))
		{
			signal.params.BindInput(context.node.GetInputId(inputIdx), context.node.GetInputData(inputIdx));
		}
	}

	static_cast<IObject*>(context.pObject)->ProcessSignal(signal);

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}

SRuntimeResult CScriptGraphSendSignalNode::ExecuteSendToObject(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	struct EInputIdx
	{
		enum : u32
		{
			In = 0,
			ObjectId,
			FirstParam
		};
	};

	const SRuntimeData& data = DynamicCast<SRuntimeData>(*context.node.GetData());
	const ObjectId objectId = DynamicCast<ObjectId>(*context.node.GetInputData(EInputIdx::ObjectId));
	SObjectSignal signal(data.signalGUID);

	for (u8 inputIdx = EInputIdx::FirstParam, inputCount = context.node.GetInputCount(); inputIdx < inputCount; ++inputIdx)
	{
		if (context.node.IsDataInput(inputIdx))
		{
			signal.params.BindInput(context.node.GetInputId(inputIdx), context.node.GetInputData(inputIdx));
		}
	}

	gEnv->pSchematyc->SendSignal(objectId, signal);

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}

SRuntimeResult CScriptGraphSendSignalNode::ExecuteSendToEntity(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	struct EInputIdx
	{
		enum : u32
		{
			In = 0,
			EntityId,
			FirstParam
		};
	};

	const SRuntimeData& data = DynamicCast<SRuntimeData>(*context.node.GetData());
	const ExplicitEntityId entityId = DynamicCast<ExplicitEntityId>(*context.node.GetInputData(EInputIdx::EntityId));

	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(static_cast<EntityId>(entityId));
	if (!pEntity || !pEntity->GetSchematycObject())
	{
		return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
	}
	ObjectId objectId = pEntity->GetSchematycObject()->GetId();
	SObjectSignal signal(data.signalGUID);

	for (u8 inputIdx = EInputIdx::FirstParam, inputCount = context.node.GetInputCount(); inputIdx < inputCount; ++inputIdx)
	{
		if (context.node.IsDataInput(inputIdx))
		{
			signal.params.BindInput(context.node.GetInputId(inputIdx), context.node.GetInputData(inputIdx));
		}
	}

	gEnv->pSchematyc->SendSignal(objectId, signal);

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}


SRuntimeResult CScriptGraphSendSignalNode::ExecuteBroadcast(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	struct EInputIdx
	{
		enum : u32
		{
			In = 0,
			FirstParam
		};
	};

	const SRuntimeData& data = DynamicCast<SRuntimeData>(*context.node.GetData());
	SObjectSignal signal(data.signalGUID);

	for (u8 inputIdx = EInputIdx::FirstParam, inputCount = context.node.GetInputCount(); inputIdx < inputCount; ++inputIdx)
	{
		if (context.node.IsDataInput(inputIdx))
		{
			signal.params.BindInput(context.node.GetInputId(inputIdx), context.node.GetInputData(inputIdx));
		}
	}

	gEnv->pSchematyc->BroadcastSignal(signal);

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}

const DrxGUID CScriptGraphSendSignalNode::ms_typeGUID = "bfcebe12-b479-4cd4-90e2-5ceab24ea12e"_drx_guid;

} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphSendSignalNode::Register)
