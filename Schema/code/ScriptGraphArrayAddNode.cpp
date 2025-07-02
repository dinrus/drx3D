// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphArrayAddNode.h>

#include <drx3D/CoreX/Serialization/Enum.h>
#include <drx3D/Schema/IGraphNodeCompiler.h>
#include <drx3D/Schema/IEnvDataType.h>
#include <drx3D/Schema/IScriptVariable.h>
#include <drx3D/Schema/SerializationContext.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/AnyArray.h>
#include <drx3D/Schema/StackString.h>

#include <drx3D/Schema/ScriptView.h>
#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphNodeFactory.h>
#include <drx3D/Schema/SerializationContext.h>

namespace sxema
{

CScriptGraphArrayAddNode::CScriptGraphArrayAddNode(const SElementId& typeId)
	: m_defaultValue(typeId)
{}

DrxGUID CScriptGraphArrayAddNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphArrayAddNode::CreateLayout(CScriptGraphNodeLayout& layout)
{
	layout.SetStyleId("Core::Data");

	layout.AddInput("In", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::MultiLink });
	layout.AddOutput("Default", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::SpacerBelow });

	tukk szSubject = g_szNoType;
	if (!m_defaultValue.IsEmpty())
	{
		szSubject = m_defaultValue.GetTypeName();

		const DrxGUID typeGUID = m_defaultValue.GetTypeId().guid;
		layout.AddInput("Array", typeGUID, { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::Array });
		layout.AddInputWithData(m_defaultValue.GetTypeName(), typeGUID, { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::Persistent, EScriptGraphPortFlags::Editable }, *m_defaultValue.GetValue());
	}
	layout.SetName("Array - Add", szSubject);
}

void CScriptGraphArrayAddNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	if (!m_defaultValue.IsEmpty())
	{
		compiler.BindCallback(&Execute);
	}
}

void CScriptGraphArrayAddNode::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	m_defaultValue.SerializeTypeId(archive);
}

void CScriptGraphArrayAddNode::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	m_defaultValue.SerializeTypeId(archive);
}

void CScriptGraphArrayAddNode::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	ScriptVariableData::CScopedSerializationConfig serializationConfig(archive);

	const DrxGUID guid = CScriptGraphNodeModel::GetNode().GetGraph().GetElement().GetGUID();
	serializationConfig.DeclareEnvDataTypes(guid);
	serializationConfig.DeclareScriptEnums(guid);
	serializationConfig.DeclareScriptStructs(guid);

	m_defaultValue.SerializeTypeId(archive);

}

void CScriptGraphArrayAddNode::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	m_defaultValue.RemapDependencies(guidRemapper);
}

void CScriptGraphArrayAddNode::Register(CScriptGraphNodeFactory& factory)
{
	class CCreator : public IScriptGraphNodeCreator
	{
	private:

		class CCreationCommand : public IScriptGraphNodeCreationCommand
		{
		public:

			inline CCreationCommand(tukk szSubject = g_szNoType, const SElementId& typeId = SElementId())
				: m_subject(szSubject)
				, m_typeId(typeId)
			{}

			// IScriptGraphNodeCreationCommand

			virtual tukk GetBehavior() const override
			{
				return "Array::Add";
			}

			virtual tukk GetSubject() const override
			{
				return m_subject.c_str();
			}

			virtual tukk GetDescription() const override
			{
				return "Add element to end of array";
			}

			virtual tukk GetStyleId() const override
			{
				return "Core::Data";
			}

			virtual IScriptGraphNodePtr Execute(const Vec2& pos) override
			{
				return std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphArrayAddNode>(m_typeId), pos);
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
			return CScriptGraphArrayAddNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphArrayAddNode>());
		}

		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) override
		{
			nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>());

			// #SchematycTODO : This code is duplicated in all array nodes, find a way to reduce the duplication.

			auto visitEnvDataType = [&nodeCreationMenu, &scriptView](const IEnvDataType& envDataType) -> EVisitStatus
			{
				CStackString subject;
				scriptView.QualifyName(envDataType, subject);
				nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(subject.c_str(), SElementId(EDomain::Env, envDataType.GetGUID())));
				return EVisitStatus::Continue;
			};
			scriptView.VisitEnvDataTypes(visitEnvDataType);
		}

		// ~IScriptGraphNodeCreator
	};

	factory.RegisterCreator(std::make_shared<CCreator>());
}

SRuntimeResult CScriptGraphArrayAddNode::Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	CAnyConstPtr pAny = context.node.GetInputData(EInputIdx::Array);
	if (pAny)
	{
		CAnyArrayPtr pArray = DynamicCast<CAnyArrayPtr>(*pAny);
		CAnyConstRef value = *context.node.GetInputData(EInputIdx::Value);

		pArray->PushBack(value);
	}

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}

const DrxGUID CScriptGraphArrayAddNode::ms_typeGUID = "02368e7b-7939-495e-bf65-f044c440f4f3"_drx_guid;

} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphArrayAddNode::Register)
