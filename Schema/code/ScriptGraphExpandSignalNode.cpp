// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphExpandSignalNode.h>

#include <drx3D/CoreX/Serialization/Decorators/ActionButton.h>
#include <drx3D/Schema/IObject.h>
#include <drx3D/Schema/CompilerContext.h>
#include <drx3D/Schema/IGraphNodeCompiler.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvSignal.h>
#include <drx3D/Schema/IScriptRegistry.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/IGUIDRemapper.h>
#include <drx3D/Schema/StackString.h>

#include <drx3D/Schema/RuntimeClass.h>
#include <drx3D/Schema/ScriptView.h>
#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphNodeFactory.h>
#include <drx3D/Schema/SerializationContext.h>

namespace sxema
{

CScriptGraphExpandSignalNode::CScriptGraphExpandSignalNode() {}

CScriptGraphExpandSignalNode::CScriptGraphExpandSignalNode(const SElementId& typeId)
	: m_typeId(typeId)
{}

DrxGUID CScriptGraphExpandSignalNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphExpandSignalNode::CreateLayout(CScriptGraphNodeLayout& layout)
{
	layout.SetStyleId("Core::Data");

	tukk szSubject = g_szNoType;
	if (!GUID::IsEmpty(m_typeId.guid))
	{
		switch (m_typeId.domain)
		{
		case EDomain::Env:
			{
				const IEnvSignal* pEnvSignal = gEnv->pSchematyc->GetEnvRegistry().GetSignal(m_typeId.guid);
				if (pEnvSignal)
				{
					szSubject = pEnvSignal->GetName();

					layout.AddInput("In", m_typeId.guid, EScriptGraphPortFlags::Signal);
					layout.AddOutput("Out", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::Begin });

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
		}
	}
	layout.SetName("Expand", szSubject);
}

void CScriptGraphExpandSignalNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	switch (m_typeId.domain)
	{
	case EDomain::Env:
		{
			const IEnvSignal* pEnvSignal = gEnv->pSchematyc->GetEnvRegistry().GetSignal(m_typeId.guid);
			if (pEnvSignal)
			{
				compiler.BindCallback(&Execute);
			}
			break;
		}
	}
}

void CScriptGraphExpandSignalNode::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_typeId, "typeId");
}

void CScriptGraphExpandSignalNode::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_typeId, "typeId");
}

void CScriptGraphExpandSignalNode::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(Serialization::ActionButton(functor(*this, &CScriptGraphExpandSignalNode::GoToType)), "goToType", "^Go To Type");

	Validate(archive, context);
}

void CScriptGraphExpandSignalNode::Validate(Serialization::IArchive& archive, const ISerializationContext& context)
{
	if (!GUID::IsEmpty(m_typeId.guid))
	{
		switch (m_typeId.domain)
		{
		case EDomain::Env:
			{
				const IEnvElement* pEnvElement = gEnv->pSchematyc->GetEnvRegistry().GetElement(m_typeId.guid);
				if (!pEnvElement)
				{
					archive.error(*this, "Unable to retrieve environment type!");
				}
				break;
			}
		}
	}
}

void CScriptGraphExpandSignalNode::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	if (m_typeId.domain == EDomain::Script)
	{
		m_typeId.guid = guidRemapper.Remap(m_typeId.guid);
	}
}

void CScriptGraphExpandSignalNode::Register(CScriptGraphNodeFactory& factory)
{
	class CCreator : public IScriptGraphNodeCreator
	{
	private:

		class CCreationCommand : public IScriptGraphNodeCreationCommand
		{
		public:

			CCreationCommand(tukk szSubject, const SElementId& typeId)
				: m_subject(szSubject)
				, m_typeId(typeId)
			{}

			// IScriptGraphNodeCreationCommand

			virtual tukk GetBehavior() const override
			{
				return "Expand";
			}

			virtual tukk GetSubject() const override
			{
				return m_subject.c_str();
			}

			virtual tukk GetDescription() const override
			{
				return "Expand structure/signal";
			}

			virtual tukk GetStyleId() const override
			{
				return "Core::Data";
			}

			virtual IScriptGraphNodePtr Execute(const Vec2& pos) override
			{
				return std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphExpandSignalNode>(m_typeId), pos);
			}

			// ~IScriptGraphNodeCreationCommand

		private:

			string     m_subject;
			SElementId m_typeId;
		};

	public:

		// IScriptGraphNodeCreator

		virtual DrxGUID GetTypeGUID() const override
		{
			return CScriptGraphExpandSignalNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphExpandSignalNode>());
		}

		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) override
		{
			switch (graph.GetType())
			{
			case EScriptGraphType::Transition:
				{
					auto visitEnvSignal = [&nodeCreationMenu, &scriptView](const IEnvSignal& envSignal) -> EVisitStatus
					{
						CStackString subject;
						scriptView.QualifyName(envSignal, subject);
						nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(subject.c_str(), SElementId(EDomain::Env, envSignal.GetGUID())));
						return EVisitStatus::Continue;
					};
					scriptView.VisitEnvSignals(visitEnvSignal);
					break;
				}
			}
		}

		// ~IScriptGraphNodeCreator
	};

	factory.RegisterCreator(std::make_shared<CCreator>());
}

void CScriptGraphExpandSignalNode::GoToType()
{
	DrxLinkUtils::ExecuteCommand(DrxLinkUtils::ECommand::Show, m_typeId.guid);
}

SRuntimeResult CScriptGraphExpandSignalNode::Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
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

const DrxGUID CScriptGraphExpandSignalNode::ms_typeGUID = "f4b52ef8-18ec-4f82-bf61-42429b85ebf6"_drx_guid;

} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphExpandSignalNode::Register)
