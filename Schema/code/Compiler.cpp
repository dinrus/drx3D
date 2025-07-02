// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/Compiler.h>

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/IGraphNodeCompiler.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvClass.h>
#include <drx3D/Schema/IEnvDataType.h>
#include <drx3D/Schema/RuntimeGraph.h>
#include <drx3D/Schema/IScript.h>
#include <drx3D/Schema/IScriptExtension.h>
#include <drx3D/Schema/IScriptGraph.h>
#include <drx3D/Schema/IScriptRegistry.h>
#include <drx3D/Schema/ScriptUtils.h>
#include <drx3D/Schema/IScriptBase.h>
#include <drx3D/Schema/IScriptClass.h>
#include <drx3D/Schema/IScriptComponentInstance.h>
#include <drx3D/Schema/IScriptConstructor.h>
#include <drx3D/Schema/IScriptEnum.h>
#include <drx3D/Schema/IScriptSignalReceiver.h>
#include <drx3D/Schema/IScriptState.h>
#include <drx3D/Schema/IScriptStateMachine.h>
#include <drx3D/Schema/IScriptStruct.h>
#include <drx3D/Schema/IScriptTimer.h>
#include <drx3D/Schema/IScriptVariable.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/EnumFlags.h>
#include <drx3D/Schema/StackString.h>

#include <drx3D/Schema/Core.h>
#include <drx3D/Schema/CompilerTaskList.h>
#include <drx3D/Schema/RuntimeClass.h>
#include <drx3D/Schema/RuntimeRegistry.h>
#include <drx3D/Schema/ScriptEnumValue.h>
#include <drx3D/Schema/ScriptStructValue.h>
#include <drx3D/Schema/InterfaceMap.h>

namespace sxema
{
namespace // #SchematycTODO : Move inside CCompiler class?
{
enum class EGraphPortFlags
{
	None      = 0,
	Unused    = BIT(0),
	Signal    = BIT(1),
	Flow      = BIT(2),
	Data      = BIT(3),
	Reference = BIT(4),
	Pull      = BIT(5)
};

typedef CEnumFlags<EGraphPortFlags> GraphPortFlags;

struct SGraphPort
{
	inline SGraphPort(const GraphPortFlags& _flags = EGraphPortFlags::None)
		: flags(_flags)
		, nodeIdx(0)
		, portIdx(0)
	{}

	GraphPortFlags flags;
	u16         nodeIdx;
	u8          portIdx;
};

typedef std::vector<SGraphPort> GraphPorts;

struct SGraphNode
{
	inline SGraphNode(const IScriptGraphNode& _scriptGraphNode, tukk szName)
		: scriptGraphNode(_scriptGraphNode)
		, name(szName)
		, pCallback(nullptr)
	{}

	const IScriptGraphNode&     scriptGraphNode;
	string                      name;
	GraphPorts                  inputs;
	GraphPorts                  outputs;
	RuntimeGraphNodeCallbackPtr pCallback;
	CAnyValuePtr                pData;
};

typedef VectorMap<DrxGUID, u32> GraphNodeLookup;

class CGraphNodeCompiler : public IGraphNodeCompiler
{
public:

	inline CGraphNodeCompiler(u32 graphIdx, u32 graphNodeIdx, SGraphNode& output)
		: m_graphIdx(graphIdx)
		, m_graphNodeIdx(graphNodeIdx)
		, m_output(output)
	{}

	// IGraphNodeCompiler

	virtual u32 GetGraphIdx() const override
	{
		return m_graphIdx;
	}

	virtual u32 GetGraphNodeIdx() const override
	{
		return m_graphNodeIdx;
	}

	virtual void BindCallback(RuntimeGraphNodeCallbackPtr pCallback) override
	{
		m_output.pCallback = pCallback;
	}

	virtual void BindData(const CAnyConstRef& value) override
	{
		m_output.pData = CAnyValue::CloneShared(value);
	}

private:

	u32      m_graphIdx;
	u32      m_graphNodeIdx;
	SGraphNode& m_output;
};
} // Anonymous

void CCompiler::CompileAll()
{
	auto visitScriptElement = [this](IScriptElement& scriptElement) -> EVisitStatus
	{
		IScriptClass* pScriptClass = DynamicCast<IScriptClass>(&scriptElement);
		if (pScriptClass)
		{
			CompileClass(*pScriptClass);
		}
		return EVisitStatus::Recurse;
	};
	gEnv->pSchematyc->GetScriptRegistry().GetRootElement().VisitChildren(visitScriptElement);
}

void CCompiler::CompileDependencies(const DrxGUID& guid)
{
	IScriptRegistry& scriptRegistry = gEnv->pSchematyc->GetScriptRegistry();
	const IScriptElement* pScriptElement = scriptRegistry.GetElement(guid);
	if (pScriptElement)
	{
		std::vector<const IScriptClass*> scriptClasses;
		scriptClasses.reserve(50);

		while (true)
		{
			const EScriptElementType scriptElementType = pScriptElement->GetType();
			if (scriptElementType == EScriptElementType::Root)
			{
				break;
			}
			else if (scriptElementType == EScriptElementType::Class)
			{
				stl::push_back_unique(scriptClasses, DynamicCast<const IScriptClass>(pScriptElement));
				break;
			}
			else
			{
				pScriptElement = pScriptElement->GetParent();
			}
		}

		auto enumerateDependency = [&scriptRegistry, &scriptClasses](const DrxGUID& guid)
		{
			const IScriptElement* pScriptElement = scriptRegistry.GetElement(guid);
			if (pScriptElement)
			{
				EScriptElementType scriptElementType = EScriptElementType::Class;
				while (scriptElementType != EScriptElementType::Root)
				{
					if (scriptElementType == EScriptElementType::Class)
					{
						stl::push_back_unique(scriptClasses, DynamicCast<const IScriptClass>(pScriptElement));
						break;
					}
					else
					{
						pScriptElement = pScriptElement->GetParent();
					}
				}

			}
		};
		pScriptElement->EnumerateDependencies(enumerateDependency, EScriptDependencyType::Compile);

		// #SchematycTODO : What about dependencies of dependencies? Do we also need to consider those?

		for (const IScriptClass* pScriptClass : scriptClasses)
		{
			CompileClass(*pScriptClass);
		}
	}
}

ClassCompilationSignal::Slots& CCompiler::GetClassCompilationSignalSlots()
{
	return m_signals.classCompilation.GetSlots();
}

bool CCompiler::CompileClass(const IScriptClass& scriptClass)
{
	const int64 startTime = DrxGetTicks();

	SXEMA_COMPILER_COMMENT("\nCompiling class: name = %s ...", scriptClass.GetName());

	// Release existing class.

	const DrxGUID classGUID = scriptClass.GetGUID();
	CCore::GetInstance().GetRuntimeRegistryImpl().ReleaseClass(classGUID);

	// Build inheritance chain and find environment class.

	InheritanceChain inheritanceChain;
	inheritanceChain.reserve(10);

	const IEnvClass* pEnvClass = nullptr;
	CAnyConstPtr pEnvClassProperties;

	IScriptRegistry& scriptRegistry = gEnv->pSchematyc->GetScriptRegistry();
	for (const IScriptClass* pScriptClass = &scriptClass; pScriptClass; )
	{
		inheritanceChain.push_back(pScriptClass);

		const IScriptBase* pScriptBase = nullptr;
		for (const IScriptElement* pChildScriptElement = pScriptClass->GetFirstChild(); pChildScriptElement; pChildScriptElement = pChildScriptElement->GetNextSibling())
		{
			if (pChildScriptElement->GetType() == EScriptElementType::Base)
			{
				pScriptBase = DynamicCast<const IScriptBase>(pChildScriptElement);
				break;
			}
		}

		pScriptClass = nullptr;
		if (pScriptBase)
		{
			const SElementId baseClassId = pScriptBase->GetClassId();
			switch (baseClassId.domain)
			{
			case EDomain::Env:
				{
					pEnvClass = gEnv->pSchematyc->GetEnvRegistry().GetClass(baseClassId.guid);
					pEnvClassProperties = pScriptBase->GetEnvClassProperties();
					break;
				}
			case EDomain::Script:
				{
					pScriptClass = DynamicCast<const IScriptClass>(scriptRegistry.GetElement(baseClassId.guid));
					break;
				}
			}
		}
		else
		{
			SXEMA_COMPILER_WARNING("Unable to retrieve base class!");
			break;
		}
	}

	std::reverse(inheritanceChain.begin(), inheritanceChain.end());

	if (pEnvClass)
	{
		CLogMetaData logMetaData;
		SLogScope logScope(logMetaData);

		// Qualify class name based on script location.
		CStackString filePath = scriptClass.GetScript()->GetFilePath();
		u32 begin = filePath.find('/');
		u32 end = filePath.find(".schematyc_");
		CStackString className = filePath.Mid(begin + 1, end - begin - 1);
		className.replace("/", "::");

		// Create new class.

		time_t timeStamp;
		time(&timeStamp);

		CRuntimeClassPtr pClass = std::make_shared<CRuntimeClass>(timeStamp, classGUID, className.c_str(), pEnvClass->GetGUID(), pEnvClassProperties);

		// Configure context.

		CCompilerTaskList tasks;
		CInterfaceMap<void> interfaces;
		SCompilerContext context(tasks, interfaces);
		context.interfaces.Add(pClass.get());

		// Compile component instances.

		for (const IScriptClass* pScriptClass : inheritanceChain)
		{
			CompileComponentInstancesRecursive(context, *pClass, InvalidIdx, *pScriptClass);
		}

		pClass->FinalizeComponentInstances();

		// Compile remaining elements.

		for (const IScriptClass* pScriptClass : inheritanceChain)
		{
			CompileElementsRecursive(context, *pClass, *pScriptClass);
		}

		// Compile graphs.

		while (!tasks.GetPendingGraphs().empty())
		{
			CCompilerTaskList::PendingGraphs pendingGraphs = std::move(tasks.GetPendingGraphs());
			for (const IScriptGraph* pScriptGraph : pendingGraphs)
			{
				CompileGraph(context, *pClass, *pScriptGraph);
			}
		}

		// Finalize and store compiled class.

		pClass->Finalize();

		CCore::GetInstance().GetRuntimeRegistryImpl().RegisterClass(pClass);

		// Send results to log.

		const float time = gEnv->pTimer->TicksToSeconds(DrxGetTicks() - startTime);

		SXEMA_COMPILER_COMMENT("----- %s : time = %f(s), warnings = %d, errors = %d -----\n", logScope.errorCount == 0 ? "Success" : "Failed", time, logScope.warningCount, logScope.errorCount);

		// Notify listeners that class has been compiled.

		m_signals.classCompilation.Send(*pClass);

		return true;
	}
	else
	{
		SXEMA_COMPILER_WARNING("Unable to retrieve environment class!");

		return false;
	}
}

bool CCompiler::CompileComponentInstancesRecursive(SCompilerContext& context, CRuntimeClass& runtimeClass, u32 parentIdx, const IScriptElement& scriptScope) const
{
	for (const IScriptElement* pScriptElement = scriptScope.GetFirstChild(); pScriptElement; pScriptElement = pScriptElement->GetNextSibling())
	{
		switch (pScriptElement->GetType())
		{
		case EScriptElementType::Base:
			{
				CompileComponentInstancesRecursive(context, runtimeClass, parentIdx, *pScriptElement);
				break;
			}
		case EScriptElementType::ComponentInstance:
			{
				CStackString componentName;
				ScriptUtils::QualifyName(componentName, *pScriptElement, EScriptElementType::Class);

				const IScriptComponentInstance& scriptComponentInstance = DynamicCast<const IScriptComponentInstance>(*pScriptElement);
				u32k componentInstanceIdx = runtimeClass.AddComponentInstance(scriptComponentInstance.GetGUID(), componentName.c_str(), scriptComponentInstance.GetAccessor() == EScriptElementAccessor::Public, scriptComponentInstance.GetTypeGUID(), scriptComponentInstance.GetTransform(), scriptComponentInstance.GetProperties(), parentIdx);
				CompileComponentInstancesRecursive(context, runtimeClass, componentInstanceIdx, *pScriptElement);
				break;
			}
		}
	}
	return true;
}

bool CCompiler::CompileElementsRecursive(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptElement& scriptScope) const
{
	for (const IScriptElement* pScriptElement = scriptScope.GetFirstChild(); pScriptElement; pScriptElement = pScriptElement->GetNextSibling())
	{
		switch (pScriptElement->GetType())
		{
		case EScriptElementType::Constructor:
			{
				CompileConstructor(context, runtimeClass, DynamicCast<const IScriptConstructor>(*pScriptElement));
				break;
			}
		case EScriptElementType::StateMachine:
			{
				CompileStateMachine(context, runtimeClass, DynamicCast<const IScriptStateMachine>(*pScriptElement));
				break;
			}
		case EScriptElementType::State:
			{
				CompileState(context, runtimeClass, DynamicCast<const IScriptState>(*pScriptElement));
				break;
			}
		case EScriptElementType::Variable:
			{
				CompileVariable(context, runtimeClass, DynamicCast<const IScriptVariable>(*pScriptElement));
				break;
			}
		case EScriptElementType::Timer:
			{
				CompileTimer(context, runtimeClass, DynamicCast<const IScriptTimer>(*pScriptElement));
				break;
			}
		case EScriptElementType::SignalReceiver:
			{
				CompileSignalReceiver(context, runtimeClass, DynamicCast<const IScriptSignalReceiver>(*pScriptElement));
				break;
			}
		}
		CompileElementsRecursive(context, runtimeClass, *pScriptElement);
	}
	return true;
}

bool CCompiler::CompileConstructor(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptConstructor& scriptConstructor) const
{
	const IScriptGraph* pScriptGraph = scriptConstructor.GetExtensions().QueryExtension<const IScriptGraph>();
	if (pScriptGraph)
	{
		context.tasks.CompileGraph(*pScriptGraph);
	}
	return true;
}

bool CCompiler::CompileStateMachine(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptStateMachine& scriptStateMachine) const
{
	const IScriptGraph* pScriptGraph = scriptStateMachine.GetExtensions().QueryExtension<const IScriptGraph>();
	if (pScriptGraph)
	{
		runtimeClass.AddStateMachine(scriptStateMachine.GetGUID(), scriptStateMachine.GetName());
		context.tasks.CompileGraph(*pScriptGraph);
		return true;
	}
	return false;
}

bool CCompiler::CompileState(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptState& scriptState) const
{
	runtimeClass.AddState(scriptState.GetGUID(), scriptState.GetName());
	return true;
}

bool CCompiler::CompileVariable(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptVariable& scriptVariable) const
{
	CAnyConstPtr pData = scriptVariable.GetData();
	if (pData)
	{
		runtimeClass.AddVariable(scriptVariable.GetGUID(), scriptVariable.GetName(), scriptVariable.GetAccessor() == EScriptElementAccessor::Public, *pData);
		return true;
	}
	return false;
}

bool CCompiler::CompileTimer(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptTimer& scriptTimer) const
{
	const IScriptElement* pScriptScope = scriptTimer.GetParent();
	if (pScriptScope->GetType() != EScriptElementType::State)
	{
		runtimeClass.AddTimer(scriptTimer.GetGUID(), scriptTimer.GetName(), scriptTimer.GetParams());
		return true;
	}
	else
	{
		u32k stateIdx = runtimeClass.FindState(pScriptScope->GetGUID());
		if (stateIdx != InvalidIdx)
		{
			runtimeClass.AddStateTimer(stateIdx, scriptTimer.GetGUID(), scriptTimer.GetName(), scriptTimer.GetParams());
			return true;
		}
		return false;
	}
}

bool CCompiler::CompileSignalReceiver(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptSignalReceiver& scriptSignalReceiver) const
{
	const IScriptGraph* pScriptGraph = scriptSignalReceiver.GetExtensions().QueryExtension<const IScriptGraph>();
	if (pScriptGraph)
	{
		context.tasks.CompileGraph(*pScriptGraph);
	}
	return true;
}

bool CCompiler::CompileGraph(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptGraph& scriptGraph) const
{
	const IScriptElement& scriptElement = scriptGraph.GetElement();
	const DrxGUID graphGUID = scriptElement.GetGUID();
	u32k graphIdx = runtimeClass.AddGraph(graphGUID, scriptElement.GetName());
	if (graphIdx != InvalidIdx)
	{
		CRuntimeGraph& runtimeGraph = *runtimeClass.GetGraph(graphIdx);

		// Collect nodes.

		u32 graphNodeCount = scriptGraph.GetNodeCount();

		std::vector<SGraphNode> graphNodes;
		graphNodes.reserve(graphNodeCount);

		GraphNodeLookup graphNodeLookup;
		graphNodeLookup.reserve(graphNodeCount);

		auto visitScriptGraphNode = [this, &graphNodes, &graphNodeLookup](const IScriptGraphNode& scriptGraphNode) -> EVisitStatus
		{
			SGraphNode graphNode(scriptGraphNode, scriptGraphNode.GetName());

			u32k inputCount = scriptGraphNode.GetInputCount();
			graphNode.inputs.reserve(inputCount);
			for (u32 inputIdx = 0; inputIdx < inputCount; ++inputIdx)
			{
				const ScriptGraphPortFlags scriptGraphNodePortFlags = scriptGraphNode.GetInputFlags(inputIdx);
				GraphPortFlags graphNodePortFlags;
				if (scriptGraphNodePortFlags.CheckAny(EScriptGraphPortFlags::Signal))
				{
					graphNodePortFlags.Add(EGraphPortFlags::Signal);
				}
				if (scriptGraphNodePortFlags.CheckAny(EScriptGraphPortFlags::Flow))
				{
					graphNodePortFlags.Add(EGraphPortFlags::Flow);
				}
				if (scriptGraphNodePortFlags.Check(EScriptGraphPortFlags::Data))
				{
					graphNodePortFlags.Add(EGraphPortFlags::Data);
				}
				graphNode.inputs.push_back(SGraphPort(graphNodePortFlags));
			}

			u32k outputCount = scriptGraphNode.GetOutputCount();
			graphNode.outputs.reserve(outputCount);
			for (u32 outputIdx = 0; outputIdx < outputCount; ++outputIdx)
			{
				const ScriptGraphPortFlags scriptGraphNodePortFlags = scriptGraphNode.GetOutputFlags(outputIdx);
				GraphPortFlags graphNodePortFlags = EGraphPortFlags::Unused;
				if (scriptGraphNodePortFlags.CheckAny(EScriptGraphPortFlags::Signal))
				{
					graphNodePortFlags.Add(EGraphPortFlags::Signal);
				}
				if (scriptGraphNodePortFlags.CheckAny(EScriptGraphPortFlags::Flow))
				{
					graphNodePortFlags.Add(EGraphPortFlags::Flow);
				}
				if (scriptGraphNodePortFlags.Check(EScriptGraphPortFlags::Data))
				{
					graphNodePortFlags.Add(EGraphPortFlags::Data);
					if (scriptGraphNodePortFlags.Check(EScriptGraphPortFlags::Pull))
					{
						graphNodePortFlags.Add(EGraphPortFlags::Pull);
					}
				}
				graphNode.outputs.push_back(SGraphPort(graphNodePortFlags));
			}

			graphNodes.push_back(std::move(graphNode));
			graphNodeLookup.insert(GraphNodeLookup::value_type(scriptGraphNode.GetGUID(), graphNodeLookup.size()));

			return EVisitStatus::Continue;
		};
		scriptGraph.VisitNodes(visitScriptGraphNode);

		graphNodeCount = graphNodes.size();

		// Collect links.

		auto visitScriptGraphLink = [this, &graphNodes, &graphNodeLookup](const IScriptGraphLink& scriptGraphLink) -> EVisitStatus
		{
			GraphNodeLookup::iterator itSrcGraphNode = graphNodeLookup.find(scriptGraphLink.GetSrcNodeGUID());
			GraphNodeLookup::iterator itDstGraphNode = graphNodeLookup.find(scriptGraphLink.GetDstNodeGUID());
			if ((itSrcGraphNode == graphNodeLookup.end()) || (itDstGraphNode == graphNodeLookup.end()))
			{
				// Display error message!
				return EVisitStatus::Error;
			}

			u32k srcGraphNodeIdx = itSrcGraphNode->second;
			SGraphNode& srcGraphNode = graphNodes[srcGraphNodeIdx];
			u32k dstGraphNodeIdx = itDstGraphNode->second;
			SGraphNode& dstGraphNode = graphNodes[dstGraphNodeIdx];

			u32k srcGraphNodeOutputIdx = srcGraphNode.scriptGraphNode.FindOutputById(scriptGraphLink.GetSrcOutputId());
			u32k dstGraphNodeInputIdx = dstGraphNode.scriptGraphNode.FindInputById(scriptGraphLink.GetDstInputId());
			if ((srcGraphNodeOutputIdx == InvalidIdx) || (dstGraphNodeInputIdx == InvalidIdx))
			{
				// Display error message!
				return EVisitStatus::Error;
			}

			SGraphPort& srcGraphNodeOutput = srcGraphNode.outputs[srcGraphNodeOutputIdx];
			SGraphPort& dstGraphNodeInput = dstGraphNode.inputs[dstGraphNodeInputIdx];

			if (srcGraphNodeOutput.flags.CheckAny({ EGraphPortFlags::Signal, EGraphPortFlags::Flow }))
			{
				srcGraphNodeOutput.nodeIdx = dstGraphNodeIdx;
				srcGraphNodeOutput.portIdx = dstGraphNodeInputIdx;
			}

			if (srcGraphNodeOutput.flags.Check(EGraphPortFlags::Data))
			{
				dstGraphNodeInput.flags.Remove(EGraphPortFlags::Data);
				dstGraphNodeInput.flags.Add(EGraphPortFlags::Reference);
				if (srcGraphNodeOutput.flags.Check(EGraphPortFlags::Pull))
				{
					dstGraphNodeInput.flags.Add(EGraphPortFlags::Pull);
				}

				dstGraphNodeInput.nodeIdx = srcGraphNodeIdx;
				dstGraphNodeInput.portIdx = srcGraphNodeOutputIdx;
			}

			srcGraphNodeOutput.flags.Remove(EGraphPortFlags::Unused);

			return EVisitStatus::Continue;
		};

		if (scriptGraph.VisitLinks(visitScriptGraphLink) == EVisitResult::Error)
		{
			// Display error message!
			return false;
		}

		// #SchematycTODO : Sort nodes in order of execution?

		// Compile nodes.

		for (u32 graphNodeIdx = 0; graphNodeIdx < graphNodeCount; ++graphNodeIdx)
		{
			SGraphNode& graphNode = graphNodes[graphNodeIdx];
			CGraphNodeCompiler graphNodeCompiler(graphIdx, graphNodeIdx, graphNode);
			graphNode.scriptGraphNode.Compile(context, graphNodeCompiler);

			if (graphNode.pCallback)
			{
				u32k graphNodeInputCount = graphNode.scriptGraphNode.GetInputCount();
				u32k graphNodeOutputCount = graphNode.scriptGraphNode.GetOutputCount();

				if (!runtimeGraph.AddNode(graphNode.scriptGraphNode.GetGUID(), graphNode.name.c_str(), graphNode.pCallback, graphNodeInputCount, graphNodeOutputCount))
				{
					// Display error message!
					return false;
				}

				for (u32 graphNodeInputIdx = 0; graphNodeInputIdx < graphNodeInputCount; ++graphNodeInputIdx)
				{
					runtimeGraph.SetNodeInputId(graphNodeIdx, graphNodeInputIdx, graphNode.scriptGraphNode.GetInputId(graphNodeInputIdx));
				}

				for (u32 graphNodeOutputIdx = 0; graphNodeOutputIdx < graphNodeOutputCount; ++graphNodeOutputIdx)
				{
					runtimeGraph.SetNodeOutputId(graphNodeIdx, graphNodeOutputIdx, graphNode.scriptGraphNode.GetOutputId(graphNodeOutputIdx));
				}
			}
			else
			{
				SXEMA_COMPILER_ERROR("Failed to compile node : name = %s", graphNode.name.c_str());
				return false;
			}
		}

		// Compile data and populate scratchpad.

		for (u32 graphNodeIdx = 0; graphNodeIdx < graphNodeCount; ++graphNodeIdx)
		{
			SGraphNode& graphNode = graphNodes[graphNodeIdx];

			if (graphNode.pData)
			{
				runtimeGraph.SetNodeData(graphNodeIdx, *graphNode.pData);
			}

			for (u32 graphNodeInputIdx = 0, graphNodeInputCount = graphNode.inputs.size(); graphNodeInputIdx < graphNodeInputCount; ++graphNodeInputIdx)
			{
				if (graphNode.inputs[graphNodeInputIdx].flags.Check(EGraphPortFlags::Data))
				{
					CAnyConstPtr pData = graphNode.scriptGraphNode.GetInputData(graphNodeInputIdx);
					if (pData)
					{
						runtimeGraph.SetNodeInputData(graphNodeIdx, graphNodeInputIdx, *pData);
					}
					else
					{
						// Display error message!
					}
				}
			}

			for (u32 graphNodeOutputIdx = 0, graphNodeOutputCount = graphNode.outputs.size(); graphNodeOutputIdx < graphNodeOutputCount; ++graphNodeOutputIdx)
			{
				if (graphNode.outputs[graphNodeOutputIdx].flags.Check(EGraphPortFlags::Data))
				{
					CAnyConstPtr pData = graphNode.scriptGraphNode.GetOutputData(graphNodeOutputIdx);
					if (pData)
					{
						runtimeGraph.SetNodeOutputData(graphNodeIdx, graphNodeOutputIdx, *pData);
					}
					else
					{
						// Display error message!
					}
				}
			}
		}

		// Create links.

		for (u32 graphNodeIdx = 0; graphNodeIdx < graphNodeCount; ++graphNodeIdx)
		{
			SGraphNode& graphNode = graphNodes[graphNodeIdx];

			for (u32 graphNodeInputIdx = 0, graphNodeInputCount = graphNode.inputs.size(); graphNodeInputIdx < graphNodeInputCount; ++graphNodeInputIdx)
			{
				SGraphPort& graphNodeInput = graphNode.inputs[graphNodeInputIdx];
				if (graphNodeInput.flags.Check(EGraphPortFlags::Reference))
				{
					if (graphNodeInput.flags.Check(EGraphPortFlags::Pull))
					{
						runtimeGraph.AddPullLink(graphNodeInput.nodeIdx, graphNodeInput.portIdx, graphNodeIdx, graphNodeInputIdx);
					}
					else
					{
						runtimeGraph.AddDataLink(graphNodeInput.nodeIdx, graphNodeInput.portIdx, graphNodeIdx, graphNodeInputIdx);
					}
				}
			}

			for (u32 graphNodeOutputIdx = 0, graphNodeOutputCount = graphNode.outputs.size(); graphNodeOutputIdx < graphNodeOutputCount; ++graphNodeOutputIdx)
			{
				SGraphPort& graphNodeOutput = graphNode.outputs[graphNodeOutputIdx];
				if (!graphNodeOutput.flags.Check(EGraphPortFlags::Unused))
				{
					if (graphNodeOutput.flags.CheckAny(EGraphPortFlags::Signal))
					{
						runtimeGraph.AddSignalLink(graphNodeIdx, graphNodeOutputIdx, graphNodeOutput.nodeIdx, graphNodeOutput.portIdx);
					}
					if (graphNodeOutput.flags.CheckAny(EGraphPortFlags::Flow))
					{
						runtimeGraph.AddFlowLink(graphNodeIdx, graphNodeOutputIdx, graphNodeOutput.nodeIdx, graphNodeOutput.portIdx);
					}
				}
			}
		}
	}

	return true;
}
} // sxema
