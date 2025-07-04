// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphSwitchNode.h>

#include <drx3D/Schema/IGraphNodeCompiler.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvDataType.h>
#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/IScriptEnum.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/StackString.h>
#include <drx3D/Schema/IGUIDRemapper.h>

#include <drx3D/Schema/ScriptVariableData.h>
#include <drx3D/Schema/ScriptView.h>
#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphNodeFactory.h>
#include <drx3D/Schema/SerializationContext.h>

namespace sxema
{
namespace
{

inline bool FilterEnvDataType(const IEnvDataType& envDataType)
{
	return envDataType.GetDesc().GetFlags().Check(ETypeFlags::Switchable);
}

} // Anonymous

CScriptGraphSwitchNode::SCase::SCase() {}

CScriptGraphSwitchNode::SCase::SCase(const CAnyValuePtr& _pValue)
	: pValue(_pValue)
{}

void CScriptGraphSwitchNode::SCase::Serialize(Serialization::IArchive& archive)
{
	if (!pValue)
	{
		CAnyConstPtr* ppDefaultValue = archive.context<CAnyConstPtr>();
		if (ppDefaultValue)
		{
			pValue = CAnyValue::CloneShared(**ppDefaultValue);
		}
	}

	if (pValue)
	{
		archive(*pValue, "value", "^Value");
		if (archive.isValidation())
		{
			CStackString caseString;
			Any::ToString(caseString, *pValue);
			if (caseString.empty())
			{
				archive.error(*this, "Empty case!");
			}
			else if (bIsDuplicate)
			{
				archive.warning(*this, "Duplicate case!");
			}
		}
	}
	else if (archive.isValidation())
	{
		archive.error(*this, "Empty case!");
	}
}

bool CScriptGraphSwitchNode::SCase::operator==(const CScriptGraphSwitchNode::SCase& rhs) const
{
	return (pValue == rhs.pValue) || (pValue && rhs.pValue && Any::Equals(*pValue, *rhs.pValue));
}

CScriptGraphSwitchNode::SRuntimeData::SRuntimeData(const CasesPtr& _pCases)
	: pCases(_pCases)
{}

CScriptGraphSwitchNode::SRuntimeData::SRuntimeData(const SRuntimeData& rhs)
	: pCases(rhs.pCases)
{}

void CScriptGraphSwitchNode::SRuntimeData::ReflectType(CTypeDesc<CScriptGraphSwitchNode::SRuntimeData>& desc)
{
	desc.SetGUID("d4f18128-844e-4269-8108-103c13631c76"_drx_guid);
}

CScriptGraphSwitchNode::CScriptGraphSwitchNode(const SElementId& typeId)
	: m_defaultValue(typeId)
	, m_pValidCases(std::make_shared<Cases>())
{}

DrxGUID CScriptGraphSwitchNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphSwitchNode::CreateLayout(CScriptGraphNodeLayout& layout)
{
	layout.SetStyleId("Core::FlowControl");

	layout.AddInput("In", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::MultiLink });
	layout.AddOutput("Default", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::SpacerBelow });

	tukk szSubject = g_szNoType;
	if (!m_defaultValue.IsEmpty())
	{
		szSubject = m_defaultValue.GetTypeName();

		layout.AddInputWithData(m_defaultValue.GetTypeName(), m_defaultValue.GetTypeId().guid, { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::Persistent, EScriptGraphPortFlags::Editable }, *m_defaultValue.GetValue());

		for (const SCase& _case : * m_pValidCases)
		{
			CStackString caseString;
			Any::ToString(caseString, *_case.pValue);
			layout.AddOutput(caseString.c_str(), DrxGUID(), EScriptGraphPortFlags::Flow);
		}
	}
	layout.SetName("Switch", szSubject);
}

void CScriptGraphSwitchNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	compiler.BindCallback(&Execute);
	compiler.BindData(SRuntimeData(m_pValidCases));
}

void CScriptGraphSwitchNode::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	m_defaultValue.SerializeTypeId(archive);
}

void CScriptGraphSwitchNode::PostLoad(Serialization::IArchive& archive, const ISerializationContext& context)
{
	m_defaultValue.SerializeTypeId(archive);
	SerializeCases(archive);
}

void CScriptGraphSwitchNode::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	m_defaultValue.SerializeTypeId(archive);
	SerializeCases(archive);
}

void CScriptGraphSwitchNode::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	const SElementId prevTypeId = m_defaultValue.GetTypeId();

	{
		ScriptVariableData::CScopedSerializationConfig serializationConfig(archive);

		const DrxGUID guid = CScriptGraphNodeModel::GetNode().GetGraph().GetElement().GetGUID();
		serializationConfig.DeclareEnvDataTypes(guid, SXEMA_DELEGATE(&FilterEnvDataType));
		serializationConfig.DeclareScriptEnums(guid);

		m_defaultValue.SerializeTypeId(archive);
	}

	if (m_defaultValue.GetTypeId() == prevTypeId)
	{
		SerializeCases(archive);
	}
	else
	{
		m_cases.clear();
		m_pValidCases->clear();
	}
}

void CScriptGraphSwitchNode::Validate(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_cases, "cases");
}

void CScriptGraphSwitchNode::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	m_defaultValue.RemapDependencies(guidRemapper);
}

void CScriptGraphSwitchNode::Register(CScriptGraphNodeFactory& factory)
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
				return "Switch";
			}

			virtual tukk GetSubject() const override
			{
				return m_subject.c_str();
			}

			virtual tukk GetDescription() const override
			{
				return "Branch execution based on input value";
			}

			virtual tukk GetStyleId() const override
			{
				return "Core::FlowControl";
			}

			virtual IScriptGraphNodePtr Execute(const Vec2& pos) override
			{
				return std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphSwitchNode>(m_typeId), pos);
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
			return CScriptGraphSwitchNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphSwitchNode>());
		}

		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) override
		{
			nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>());

			auto visitEnvDataType = [&nodeCreationMenu, &scriptView](const IEnvDataType& envDataType) -> EVisitStatus
			{
				if (FilterEnvDataType(envDataType))
				{
					CStackString specialization;
					scriptView.QualifyName(envDataType, specialization);
					nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(specialization.c_str(), SElementId(EDomain::Env, envDataType.GetGUID())));
				}
				return EVisitStatus::Continue;
			};
			scriptView.VisitEnvDataTypes(visitEnvDataType);

			auto visitScriptEnum = [&nodeCreationMenu, &scriptView](const IScriptEnum& scriptEnum)
			{
				CStackString specialization;
				scriptView.QualifyName(scriptEnum, EDomainQualifier::Global, specialization);
				nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(specialization.c_str(), SElementId(EDomain::Script, scriptEnum.GetGUID())));
			};
			scriptView.VisitAccesibleEnums(visitScriptEnum);
		}

		// ~IScriptGraphNodeCreator
	};

	factory.RegisterCreator(std::make_shared<CCreator>());
}

void CScriptGraphSwitchNode::SerializeCases(Serialization::IArchive& archive)
{
	if (!m_defaultValue.IsEmpty())
	{
		CAnyConstPtr pDefaultValue = m_defaultValue.GetValue();
		Serialization::SContext context(archive, &pDefaultValue);
		archive(m_cases, "cases", "Cases");

		if (archive.isInput())
		{
			RefreshCases();
		}
	}
}

void CScriptGraphSwitchNode::RefreshCases()
{
	m_pValidCases->clear();

	std::vector<CStackString> caseStrings;
	caseStrings.reserve(m_cases.size());

	for (SCase& _case : m_cases)
	{
		if (_case.pValue)
		{
			CStackString caseString;
			Any::ToString(caseString, *_case.pValue);
			if (!caseString.empty())
			{
				_case.bIsDuplicate = std::find(caseStrings.begin(), caseStrings.end(), caseString) != caseStrings.end();
				if (!_case.bIsDuplicate)
				{
					m_pValidCases->push_back(_case);
					caseStrings.push_back(caseString);
				}
			}
		}
	}
}

SRuntimeResult CScriptGraphSwitchNode::Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	const SRuntimeData& data = DynamicCast<SRuntimeData>(*context.node.GetData());
	CAnyConstRef value = *context.node.GetInputData(EInputIdx::Value);

	for (u32 caseIdx = 0, caseCount = data.pCases->size(); caseIdx < caseCount; ++caseIdx)
	{
		CAnyConstPtr pValue = (*data.pCases)[caseIdx].pValue;
		if (pValue && Any::Equals(value, *pValue))
		{
			return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::FirstCase + caseIdx);
		}
	}

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Default);
}

const DrxGUID CScriptGraphSwitchNode::ms_typeGUID = "1d081133-e900-4244-add5-f0831d27b16f"_drx_guid;

} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphSwitchNode::Register)
