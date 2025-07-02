// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphSetNode.h>

#include <drx3D/Schema/CompilerContext.h>
#include <drx3D/Schema/IGraphNodeCompiler.h>
#include <drx3D/Schema/IScriptVariable.h>
#include <drx3D/Schema/IScriptComponentInstance.h>
#include <drx3D/Schema/Any.h>
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
CScriptGraphSetNode::SRuntimeData::SRuntimeData(u32 _pos)
	: pos(_pos)
{}

CScriptGraphSetNode::SRuntimeData::SRuntimeData(const SRuntimeData& rhs)
	: pos(rhs.pos)
{}

void CScriptGraphSetNode::SRuntimeData::ReflectType(CTypeDesc<CScriptGraphSetNode::SRuntimeData>& desc)
{
	desc.SetGUID("1a4b1431-c8fe-46f5-aecd-fc8c11500e99"_drx_guid);
}

CScriptGraphSetNode::CScriptGraphSetNode() {}

CScriptGraphSetNode::CScriptGraphSetNode(const DrxGUID& referenceGUID)
	: m_referenceGUID(referenceGUID)
{}

DrxGUID CScriptGraphSetNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphSetNode::CreateLayout(CScriptGraphNodeLayout& layout)
{
	layout.SetStyleId("Core::Data");

	CStackString subject;
	if (!GUID::IsEmpty(m_referenceGUID))
	{
		layout.AddInput("In", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::MultiLink });
		layout.AddOutput("Out", DrxGUID(), EScriptGraphPortFlags::Flow);

		CScriptView scriptView(CScriptGraphNodeModel::GetNode().GetGraph().GetElement().GetGUID());
		const IScriptElement* pReferenceElement = scriptView.GetScriptElement(m_referenceGUID);
		if (pReferenceElement)
		{
			switch (pReferenceElement->GetType())
			{
			case EScriptElementType::Variable:
				{
					const IScriptVariable& variable = DynamicCast<IScriptVariable>(*pReferenceElement);
					CAnyConstPtr pData = variable.GetData();
					if (pData)
					{
						subject = variable.GetName();

						layout.AddInputWithData("Value", variable.GetTypeId().guid, { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::MultiLink, EScriptGraphPortFlags::Persistent, EScriptGraphPortFlags::Editable }, *pData);
					}
					break;
				}
			}
		}
	}
	layout.SetName("Set", subject.c_str());
}

void CScriptGraphSetNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	if (!GUID::IsEmpty(m_referenceGUID))
	{
		CScriptView scriptView(CScriptGraphNodeModel::GetNode().GetGraph().GetElement().GetGUID());
		const IScriptElement* pReferenceElement = scriptView.GetScriptElement(m_referenceGUID);
		if (pReferenceElement)
		{
			switch (pReferenceElement->GetType())
			{
			case EScriptElementType::Variable:
				{
					const IScriptVariable& variable = DynamicCast<IScriptVariable>(*pReferenceElement);
					const CRuntimeClass* pClass = context.interfaces.Query<const CRuntimeClass>();
					if (pClass)
					{
						u32k variablePos = pClass->GetVariablePos(variable.GetGUID());
						if (variablePos != InvalidIdx)
						{
							compiler.BindCallback(&Execute);
							compiler.BindData(SRuntimeData(variablePos));
						}
					}
					break;
				}
			}
		}
	}
}

void CScriptGraphSetNode::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_referenceGUID, "referenceGUID");
}

void CScriptGraphSetNode::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_referenceGUID, "referenceGUID");
}

void CScriptGraphSetNode::Validate(Serialization::IArchive& archive, const ISerializationContext& context)
{
	if (!GUID::IsEmpty(m_referenceGUID))
	{
		CScriptView scriptView(CScriptGraphNodeModel::GetNode().GetGraph().GetElement().GetGUID());
		const IScriptElement* pReferenceElement = scriptView.GetScriptElement(m_referenceGUID);
		if (!pReferenceElement)
		{
			archive.error(*this, "Failed to retrieve reference!");
		}
	}
}

void CScriptGraphSetNode::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	m_referenceGUID = guidRemapper.Remap(m_referenceGUID);
}

void CScriptGraphSetNode::Register(CScriptGraphNodeFactory& factory)
{
	class CCreator : public IScriptGraphNodeCreator
	{
	private:

		class CCreationCommand : public IScriptGraphNodeCreationCommand
		{
		public:

			CCreationCommand(tukk szSubject, const DrxGUID& referenceGUID)
				: m_subject(szSubject)
				, m_referenceGUID(referenceGUID)
			{}

			// IScriptGraphNodeCreationCommand

			virtual tukk GetBehavior() const override
			{
				return "Set";
			}

			virtual tukk GetSubject() const override
			{
				return m_subject.c_str();
			}

			virtual tukk GetDescription() const override
			{
				return "Set value of variable";
			}

			virtual tukk GetStyleId() const override
			{
				return "Core::Data";
			}

			virtual IScriptGraphNodePtr Execute(const Vec2& pos) override
			{
				return std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphSetNode>(m_referenceGUID), pos);
			}

			// ~IScriptGraphNodeCreationCommand

		private:

			string m_subject;
			DrxGUID  m_referenceGUID;
		};

	public:

		// IScriptGraphNodeCreator

		virtual DrxGUID GetTypeGUID() const override
		{
			return CScriptGraphSetNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphSetNode>());
		}

		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) override
		{
			switch (graph.GetType())
			{
			case EScriptGraphType::Construction:
			case EScriptGraphType::Signal:
			case EScriptGraphType::Function:
				{
					auto visitScriptVariable = [&nodeCreationMenu, &scriptView](const IScriptVariable& variable) -> EVisitStatus
					{
						if (!variable.IsArray())
						{
							CStackString subject;
							scriptView.QualifyName(variable, EDomainQualifier::Global, subject);
							nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(subject.c_str(), variable.GetGUID()));
						}
						return EVisitStatus::Continue;
					};
					scriptView.VisitScriptVariables(visitScriptVariable, EDomainScope::Derived);

					// Library variables
					// TODO: Not yet supported.
					/*CScriptView gloablView(gEnv->pSchematyc->GetScriptRegistry().GetRootElement().GetGUID());
					   auto visitLibraries = [&nodeCreationMenu](const IScriptVariable& scriptVariable) -> EVisitStatus
					   {
					   if (!scriptVariable.IsArray())
					   {
					    CStackString subject;
					    QualifyScriptElementName(gEnv->pSchematyc->GetScriptRegistry().GetRootElement(), scriptVariable, EDomainQualifier::Global, subject);
					    nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(subject.c_str(), scriptVariable.GetGUID()));
					   }
					   return EVisitStatus::Continue;
					   };
					   gloablView.VisitScriptModuleVariables(visitLibraries);*/
					// ~TODO

					break;
				}
			}
		}

		// ~IScriptGraphNodeCreator
	};

	factory.RegisterCreator(std::make_shared<CCreator>());
}

SRuntimeResult CScriptGraphSetNode::Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	const SRuntimeData& data = DynamicCast<SRuntimeData>(*context.node.GetData());
	Any::CopyAssign(*static_cast<CObject*>(context.pObject)->GetScratchpad().Get(data.pos), *context.node.GetInputData(EInputIdx::Value));

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}

const DrxGUID CScriptGraphSetNode::ms_typeGUID = "23145b7a-4ce3-45b8-a34b-1c997ea6448f"_drx_guid;
} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphSetNode::Register)
