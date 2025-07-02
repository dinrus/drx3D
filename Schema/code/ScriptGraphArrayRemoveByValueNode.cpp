// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphArrayRemoveByValueNode.h>

#include <drx3D/Schema/IGraphNodeCompiler.h>
#include <drx3D/Schema/IEnvDataType.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/AnyArray.h>
#include <drx3D/Schema/StackString.h>

#include <drx3D/Schema/ScriptView.h>
#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphNodeFactory.h>
#include <drx3D/Schema/SerializationContext.h>

namespace sxema
{
CScriptGraphArrayRemoveByValueNode::CScriptGraphArrayRemoveByValueNode(const SElementId& reference)
	: m_defaultValue(reference)
{}

DrxGUID CScriptGraphArrayRemoveByValueNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphArrayRemoveByValueNode::CreateLayout(CScriptGraphNodeLayout& layout)
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

	layout.SetName("Array - Remove By Value", szSubject);
}

void CScriptGraphArrayRemoveByValueNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	if (!m_defaultValue.IsEmpty())
	{
		compiler.BindCallback(&Execute);
	}
}

void CScriptGraphArrayRemoveByValueNode::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	m_defaultValue.SerializeTypeId(archive);
}

void CScriptGraphArrayRemoveByValueNode::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	m_defaultValue.SerializeTypeId(archive);
}

void CScriptGraphArrayRemoveByValueNode::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	{
		ScriptVariableData::CScopedSerializationConfig serializationConfig(archive);

		const DrxGUID guid = CScriptGraphNodeModel::GetNode().GetGraph().GetElement().GetGUID();
		serializationConfig.DeclareEnvDataTypes(guid);
		serializationConfig.DeclareScriptEnums(guid);
		serializationConfig.DeclareScriptStructs(guid);

		m_defaultValue.SerializeTypeId(archive);
	}
}

void CScriptGraphArrayRemoveByValueNode::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	m_defaultValue.RemapDependencies(guidRemapper);
}

void CScriptGraphArrayRemoveByValueNode::Register(CScriptGraphNodeFactory& factory)
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
				return "Array::Remove By Value";
			}

			virtual tukk GetSubject() const override
			{
				return m_subject.c_str();
			}

			virtual tukk GetDescription() const override
			{
				return "Remove all elements of a specific value from array";
			}

			virtual tukk GetStyleId() const override
			{
				return "Core::Data";
			}

			virtual IScriptGraphNodePtr Execute(const Vec2& pos) override
			{
				return std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphArrayRemoveByValueNode>(m_typeId), pos);
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
			return CScriptGraphArrayRemoveByValueNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphArrayRemoveByValueNode>());
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

SRuntimeResult CScriptGraphArrayRemoveByValueNode::Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	CAnyConstPtr pAny = context.node.GetInputData(EInputIdx::Array);
	if (pAny)
	{
		CAnyArrayPtr pArray = DynamicCast<CAnyArrayPtr>(*pAny);
		CAnyConstRef value = *context.node.GetInputData(EInputIdx::Value);

		pArray->RemoveByValue(value);
	}

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}

const DrxGUID CScriptGraphArrayRemoveByValueNode::ms_typeGUID = "aa5e9cf1-aba7-438a-904e-86174f5ba85c"_drx_guid;
} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphArrayRemoveByValueNode::Register)
