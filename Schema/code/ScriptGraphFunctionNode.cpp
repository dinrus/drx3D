// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphFunctionNode.h>

#include <drx3D/CoreX/Serialization/Decorators/ActionButton.h>
#include <drx3D/Schema/CompilerContext.h>
#include <drx3D/Schema/IGraphNodeCompiler.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvComponent.h>
#include <drx3D/Schema/IEnvFunction.h>
#include <drx3D/Schema/IScriptRegistry.h>
#include <drx3D/Schema/IScriptComponentInstance.h>
#include <drx3D/Schema/IScriptFunction.h>
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

CScriptGraphFunctionNode::SEnvGlobalFunctionRuntimeData::SEnvGlobalFunctionRuntimeData(const IEnvFunction* _pEnvFunction)
	: pEnvFunction(_pEnvFunction)
{}

CScriptGraphFunctionNode::SEnvGlobalFunctionRuntimeData::SEnvGlobalFunctionRuntimeData(const SEnvGlobalFunctionRuntimeData& rhs)
	: pEnvFunction(rhs.pEnvFunction)
{}

void CScriptGraphFunctionNode::SEnvGlobalFunctionRuntimeData::ReflectType(CTypeDesc<CScriptGraphFunctionNode::SEnvGlobalFunctionRuntimeData>& desc)
{
	desc.SetGUID("90c48655-4a34-49cc-a618-44ae349c9c7b"_drx_guid);
}

CScriptGraphFunctionNode::SEnvComponentFunctionRuntimeData::SEnvComponentFunctionRuntimeData(const IEnvFunction* _pEnvFunction, u32 _componentIdx)
	: pEnvFunction(_pEnvFunction)
	, componentIdx(_componentIdx)
{}

CScriptGraphFunctionNode::SEnvComponentFunctionRuntimeData::SEnvComponentFunctionRuntimeData(const SEnvComponentFunctionRuntimeData& rhs)
	: pEnvFunction(rhs.pEnvFunction)
	, componentIdx(rhs.componentIdx)
{}

void CScriptGraphFunctionNode::SEnvComponentFunctionRuntimeData::ReflectType(CTypeDesc<CScriptGraphFunctionNode::SEnvComponentFunctionRuntimeData>& desc)
{
	desc.SetGUID("205a9972-3dc7-4d20-97f6-a322ae2d9e37"_drx_guid);
}

CScriptGraphFunctionNode::SScriptFunctionRuntimeData::SScriptFunctionRuntimeData(u32 _functionIdx)
	: functionIdx(_functionIdx)
{}

CScriptGraphFunctionNode::SScriptFunctionRuntimeData::SScriptFunctionRuntimeData(const SScriptFunctionRuntimeData& rhs)
	: functionIdx(rhs.functionIdx)
{}

void CScriptGraphFunctionNode::SScriptFunctionRuntimeData::ReflectType(CTypeDesc<CScriptGraphFunctionNode::SScriptFunctionRuntimeData>& desc)
{
	desc.SetGUID("e049b617-7e1e-4f61-aefc-b827e5d353f5"_drx_guid);
}

CScriptGraphFunctionNode::CScriptGraphFunctionNode() {}

CScriptGraphFunctionNode::CScriptGraphFunctionNode(const SElementId& functionId, const DrxGUID& objectGUID)
	: m_functionId(functionId)
	, m_objectGUID(objectGUID)
{}

DrxGUID CScriptGraphFunctionNode::GetTypeGUID() const
{
	return ms_typeGUID;
}

void CScriptGraphFunctionNode::CreateLayout(CScriptGraphNodeLayout& layout)
{
	layout.SetStyleId("Core::Function");

	stack_string subject;

	const IScriptElement* pScriptObject = gEnv->pSchematyc->GetScriptRegistry().GetElement(m_objectGUID);
	if (pScriptObject)
	{
		subject = pScriptObject->GetName();
		subject.append("::");
	}

	if (!GUID::IsEmpty(m_functionId.guid))
	{
		layout.AddInput("In", DrxGUID(), { EScriptGraphPortFlags::Flow, EScriptGraphPortFlags::MultiLink });
		layout.AddOutput("Out", DrxGUID(), EScriptGraphPortFlags::Flow);

		IEnvRegistry& envRegistry = gEnv->pSchematyc->GetEnvRegistry();
		switch (m_functionId.domain)
		{
		case EDomain::Env:
			{
				const IEnvFunction* pEnvFunction = envRegistry.GetFunction(m_functionId.guid); // #SchematycTODO : Should we be using a script view to retrieve this?
				if (pEnvFunction)
				{
					subject.append(pEnvFunction->GetName());

					CreateInputsAndOutputs(layout, *pEnvFunction);
				}
				break;
			}
		case EDomain::Script:
			{
				const IScriptElement* pScriptElement = gEnv->pSchematyc->GetScriptRegistry().GetElement(m_functionId.guid); // #SchematycTODO : Should we be using a script view to retrieve this?
				if (pScriptElement && (pScriptElement->GetType() == EScriptElementType::Function))
				{
					subject.append(pScriptElement->GetName());

					CreateInputsAndOutputs(layout, DynamicCast<IScriptFunction>(*pScriptElement));
				}
				break;
			}
		}
	}

	layout.SetName(nullptr, subject.c_str());
}

void CScriptGraphFunctionNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	CRuntimeClass* pClass = context.interfaces.Query<CRuntimeClass>();
	if (pClass)
	{
		if (!GUID::IsEmpty(m_functionId.guid))
		{
			switch (m_functionId.domain)
			{
			case EDomain::Env:
				{
					const IEnvFunction* pEnvFunction = gEnv->pSchematyc->GetEnvRegistry().GetFunction(m_functionId.guid);
					if (pEnvFunction)
					{
						if (GUID::IsEmpty(m_objectGUID))
						{
							if (!pEnvFunction->GetFunctionFlags().Check(EEnvFunctionFlags::Member))
							{
								compiler.BindCallback(&ExecuteEnvGlobalFunction);
								compiler.BindData(SEnvGlobalFunctionRuntimeData(pEnvFunction));
							}
							else
							{
								SXEMA_COMPILER_ERROR("Unable to find object on which to call function!");
							}
						}
						else
						{
							const IScriptElement* pScriptObject = gEnv->pSchematyc->GetScriptRegistry().GetElement(m_objectGUID);
							if (pScriptObject)
							{
								switch (pScriptObject->GetType())
								{
								case EScriptElementType::ComponentInstance:
									{
										const IScriptComponentInstance& scriptComponentInstance = DynamicCast<IScriptComponentInstance>(*pScriptObject);
										if (scriptComponentInstance.GetTypeGUID() == pEnvFunction->GetObjectTypeDesc()->GetGUID()) // #SchematycTODO : Check object type desc is not null before dereferencing?
										{
											compiler.BindCallback(&ExecuteEnvComponentFunction);

											u32k componentIdx = pClass->FindComponentInstance(m_objectGUID);
											compiler.BindData(SEnvComponentFunctionRuntimeData(pEnvFunction, componentIdx));
										}
										else
										{
											SXEMA_COMPILER_ERROR("Unable to find object on which to call function!");
										}
										break;
									}
								}
							}
							else
							{
								SXEMA_COMPILER_ERROR("Unable to find object on which to call function!");
							}
						}
					}
					else
					{
						SXEMA_COMPILER_ERROR("Unable to find environment function!");
					}
					break;
				}
			case EDomain::Script:
				{
					const IScriptFunction* pScriptFunction = DynamicCast<IScriptFunction>(gEnv->pSchematyc->GetScriptRegistry().GetElement(m_functionId.guid));
					if (pScriptFunction)
					{
						const IScriptGraph* pScriptGraph = pScriptFunction->GetExtensions().QueryExtension<const IScriptGraph>();
						if (pScriptGraph)
						{
							context.tasks.CompileGraph(*pScriptGraph);
							compiler.BindCallback(&ExecuteScriptFunction);

							u32k functionIdx = pClass->FindOrReserveFunction(pScriptFunction->GetGUID());
							compiler.BindData(SScriptFunctionRuntimeData(functionIdx));
						}
						else
						{
							SXEMA_COMPILER_ERROR("Unable to find script graph!");
						}
					}
					else
					{
						SXEMA_COMPILER_ERROR("Unable to find script function!");
					}
					break;
				}
			}
		}
	}
}

void CScriptGraphFunctionNode::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_functionId, "functionId");
	archive(m_objectGUID, "objectGUID");
}

void CScriptGraphFunctionNode::Load(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_functionId, "functionId");
	archive(m_objectGUID, "objectGUID");
}

void CScriptGraphFunctionNode::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_functionId, "functionId");
	archive(m_objectGUID, "objectGUID");
}

void CScriptGraphFunctionNode::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	if (m_functionId.domain == EDomain::Script)
	{
		archive(Serialization::ActionButton(functor(*this, &CScriptGraphFunctionNode::GoToFunction)), "goToFunction", "^Go To Function");
	}

	Validate(archive, context);
}

void CScriptGraphFunctionNode::Validate(Serialization::IArchive& archive, const ISerializationContext& context)
{
	if (!GUID::IsEmpty(m_functionId.guid))
	{
		switch (m_functionId.domain)
		{
		case EDomain::Env:
			{
				const IEnvFunction* pEnvFunction = gEnv->pSchematyc->GetEnvRegistry().GetFunction(m_functionId.guid);
				if (pEnvFunction)
				{
					if (pEnvFunction->GetFlags().Check(EEnvElementFlags::Deprecated))
					{
						archive.warning(*this, "Function is deprecated!");
					}
				}
				else
				{
					archive.error(*this, "Unable to retrieve environment function!");
				}
				break;
			}
		}
	}
}

void CScriptGraphFunctionNode::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	if (m_functionId.domain == EDomain::Script)
	{
		m_functionId.guid = guidRemapper.Remap(m_functionId.guid);
	}
	m_objectGUID = guidRemapper.Remap(m_objectGUID);
}

void CScriptGraphFunctionNode::Register(CScriptGraphNodeFactory& factory)
{
	class CCreator : public IScriptGraphNodeCreator
	{
	private:

		class CCreationCommand : public IScriptGraphNodeCreationCommand
		{
		public:

			CCreationCommand(tukk szSubject, tukk szDescription, const SElementId& functionId, const DrxGUID& objectGUID = DrxGUID())
				: m_subject(szSubject)
				, m_description(szDescription)
				, m_functionId(functionId)
				, m_objectGUID(objectGUID)
			{}

			// IScriptGraphNodeCreationCommand

			virtual tukk GetBehavior() const override
			{
				return "Function";
			}

			virtual tukk GetSubject() const override
			{
				return m_subject.c_str();
			}

			virtual tukk GetDescription() const override
			{
				return m_description.c_str();
			}

			virtual tukk GetStyleId() const override
			{
				return "Core::Function";
			}

			virtual IScriptGraphNodePtr Execute(const Vec2& pos) override
			{
				return std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphFunctionNode>(m_functionId, m_objectGUID), pos);
			}

			// ~IScriptGraphNodeCreationCommand

		private:

			string     m_subject;
			string     m_description;
			SElementId m_functionId;
			DrxGUID    m_objectGUID;
		};

	public:

		// IScriptGraphNodeCreator

		virtual DrxGUID GetTypeGUID() const override
		{
			return CScriptGraphFunctionNode::ms_typeGUID;
		}

		virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) override
		{
			return std::make_shared<CScriptGraphNode>(guid, stl::make_unique<CScriptGraphFunctionNode>());
		}

		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) override
		{
			struct SObject
			{
				inline SObject(const DrxGUID& _guid, const DrxGUID& _typeGUID, tukk szName)
					: guid(_guid)
					, typeGUID(_typeGUID)
					, name(szName)
				{}

				DrxGUID guid;
				DrxGUID typeGUID;
				string  name;
			};

			const EScriptGraphType graphType = graph.GetType();
			if (graphType == EScriptGraphType::Transition)
			{
				return;
			}

			std::vector<SObject> objects;
			objects.reserve(20);

			auto visitScriptComponentInstance = [&scriptView, &objects](const IScriptComponentInstance& scriptComponentInstance) -> EVisitStatus
			{
				CStackString name;
				//scriptView.QualifyName(scriptComponentInstance, EDomainQualifier::Global, name);
				name = "Components::";
				name += scriptComponentInstance.GetName();
				objects.emplace_back(scriptComponentInstance.GetGUID(), scriptComponentInstance.GetTypeGUID(), name.c_str());
				return EVisitStatus::Continue;
			};
			scriptView.VisitScriptComponentInstances(visitScriptComponentInstance, EDomainScope::Derived);

			auto visitEnvFunction = [&nodeCreationMenu, &scriptView, graphType, &objects](const IEnvFunction& envFunction) -> EVisitStatus
			{
				if (envFunction.GetFlags().Check(EEnvElementFlags::Deprecated))
				{
					return EVisitStatus::Continue;
				}

				// #SchematycTODO : Create utility functions to determine which nodes are callable from which graphs?

				if ((graphType == EScriptGraphType::Construction) && !envFunction.GetFunctionFlags().Check(EEnvFunctionFlags::Construction))
				{
					return EVisitStatus::Continue;
				}

				if (envFunction.GetFunctionFlags().Check(EEnvFunctionFlags::Member))
				{
					const DrxGUID objectTypeGUID = envFunction.GetObjectTypeDesc()->GetGUID();
					for (SObject& object : objects)
					{
						if (object.typeGUID == objectTypeGUID)
						{
							CStackString subject = object.name.c_str();
							subject.append("::");
							subject.append(envFunction.GetName());
							nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(subject.c_str(), envFunction.GetDescription(), SElementId(EDomain::Env, envFunction.GetGUID()), object.guid));
						}
					}
				}
				else
				{
					CStackString subject;
					scriptView.QualifyName(envFunction, subject);
					nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(subject.c_str(), envFunction.GetDescription(), SElementId(EDomain::Env, envFunction.GetGUID())));
				}
				return EVisitStatus::Continue;
			};
			gEnv->pSchematyc->GetEnvRegistry().VisitFunctions(visitEnvFunction);

			if (graphType == EScriptGraphType::Construction)
			{
				return;
			}

			auto visitScriptFunction = [&nodeCreationMenu, &scriptView](const IScriptFunction& scriptFunction) -> EVisitStatus
			{
				CStackString subject;
				scriptView.QualifyName(scriptFunction, EDomainQualifier::Global, subject);
				nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(subject.c_str(), scriptFunction.GetDescription(), SElementId(EDomain::Script, scriptFunction.GetGUID())));
				return EVisitStatus::Continue;
			};
			scriptView.VisitScriptFunctions(visitScriptFunction);

			// Library functions
			CScriptView gloablView(gEnv->pSchematyc->GetScriptRegistry().GetRootElement().GetGUID());
			auto visitLibraries = [&nodeCreationMenu](const IScriptFunction& scriptFunction) -> EVisitStatus
			{
				CStackString subject;
				QualifyScriptElementName(gEnv->pSchematyc->GetScriptRegistry().GetRootElement(), scriptFunction, EDomainQualifier::Global, subject);
				nodeCreationMenu.AddCommand(std::make_shared<CCreationCommand>(subject.c_str(), scriptFunction.GetDescription(), SElementId(EDomain::Script, scriptFunction.GetGUID())));

				return EVisitStatus::Continue;
			};
			gloablView.VisitScriptModuleFunctions(visitLibraries);
		}

		// ~IScriptGraphNodeCreator
	};

	factory.RegisterCreator(std::make_shared<CCreator>());
}

void CScriptGraphFunctionNode::CreateInputsAndOutputs(CScriptGraphNodeLayout& layout, const IEnvFunction& envFunction)
{
	for (u32 inputIdx = 0, inputCount = envFunction.GetInputCount(); inputIdx < inputCount; ++inputIdx)
	{
		CAnyConstPtr pData = envFunction.GetInputData(inputIdx);
		if (pData)
		{
			layout.AddInputWithData(CUniqueId::FromUInt32(envFunction.GetInputId(inputIdx)), envFunction.GetInputName(inputIdx), pData->GetTypeDesc().GetGUID(), { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::Persistent, EScriptGraphPortFlags::Editable }, *pData);
		}
	}

	for (u32 outputIdx = 0, outputCount = envFunction.GetOutputCount(); outputIdx < outputCount; ++outputIdx)
	{
		CAnyConstPtr pData = envFunction.GetOutputData(outputIdx);
		if (pData)
		{
			layout.AddOutputWithData(CUniqueId::FromUInt32(envFunction.GetOutputId(outputIdx)), envFunction.GetOutputName(outputIdx), pData->GetTypeDesc().GetGUID(), { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::MultiLink }, *pData);
		}
	}
}

void CScriptGraphFunctionNode::CreateInputsAndOutputs(CScriptGraphNodeLayout& layout, const IScriptFunction& scriptFunction)
{
	for (u32 inputIdx = 0, inputCount = scriptFunction.GetInputCount(); inputIdx < inputCount; ++inputIdx)
	{
		CAnyConstPtr pData = scriptFunction.GetInputData(inputIdx);
		if (pData)
		{
			layout.AddInputWithData(CUniqueId::FromGUID(scriptFunction.GetInputGUID(inputIdx)), scriptFunction.GetInputName(inputIdx), pData->GetTypeDesc().GetGUID(), { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::Persistent, EScriptGraphPortFlags::Editable }, *pData);
		}
	}

	for (u32 outputIdx = 0, outputCount = scriptFunction.GetOutputCount(); outputIdx < outputCount; ++outputIdx)
	{
		CAnyConstPtr pData = scriptFunction.GetOutputData(outputIdx);
		if (pData)
		{
			layout.AddOutputWithData(CUniqueId::FromGUID(scriptFunction.GetOutputGUID(outputIdx)), scriptFunction.GetOutputName(outputIdx), pData->GetTypeDesc().GetGUID(), { EScriptGraphPortFlags::Data, EScriptGraphPortFlags::MultiLink }, *pData);
		}
	}
}

void CScriptGraphFunctionNode::GoToFunction()
{
	DrxLinkUtils::ExecuteCommand(DrxLinkUtils::ECommand::Show, m_functionId.guid);
}

SRuntimeResult CScriptGraphFunctionNode::ExecuteEnvGlobalFunction(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	SEnvGlobalFunctionRuntimeData& data = DynamicCast<SEnvGlobalFunctionRuntimeData>(*context.node.GetData());

	StackRuntimeParamMap params;
	context.node.BindParams(params); // #SchematycTODO : Rather than populating the runtime parameter map every time we reference a node it might make more sense to pre-allocate node instances every time we instantiate a graph.

	data.pEnvFunction->Execute(params, nullptr);

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}

SRuntimeResult CScriptGraphFunctionNode::ExecuteEnvComponentFunction(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	SEnvComponentFunctionRuntimeData& data = DynamicCast<SEnvComponentFunctionRuntimeData>(*context.node.GetData());
	uk pComponent = static_cast<CObject*>(context.pObject)->GetComponent(data.componentIdx); // #SchematycTODO : How can we ensure this pointer is correct for the implementation, not just the interface?
	assert(pComponent);

	StackRuntimeParamMap params;
	context.node.BindParams(params); // #SchematycTODO : Rather than populating the runtime parameter map every time we reference a node it might make more sense to pre-allocate node instances every time we instantiate a graph.

	data.pEnvFunction->Execute(params, pComponent);

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}

SRuntimeResult CScriptGraphFunctionNode::ExecuteScriptFunction(SRuntimeContext& context, const SRuntimeActivationParams& activationParams)
{
	SScriptFunctionRuntimeData& data = DynamicCast<SScriptFunctionRuntimeData>(*context.node.GetData());

	StackRuntimeParamMap params;
	context.node.BindParams(params); // #SchematycTODO : Rather than populating the runtime parameter map every time we reference a node it might make more sense to pre-allocate node instances every time we instantiate a graph.

	static_cast<CObject*>(context.pObject)->ExecuteFunction(data.functionIdx, params);

	return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
}

const DrxGUID CScriptGraphFunctionNode::ms_typeGUID = "1bcfd811-b8b7-4032-a90c-311dfa4454c6"_drx_guid;

} // sxema

SXEMA_REGISTER_SCRIPT_GRAPH_NODE(sxema::CScriptGraphFunctionNode::Register)
