// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptAbstractInterfaceImplementation.h>

#include <drx3D/CoreX/Serialization/BlackBox.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema2/GUIDRemapper.h>
#include <drx3D/Schema2/IAbstractInterface.h>
#include <drx3D/Schema2/ICompiler.h>
#include <drx3D/Schema2/Deprecated/DocUtils.h>
#include <drx3D/Schema2/IEnvRegistry.h>
#include <drx3D/Schema2/IScriptGraph.h>
#include <drx3D/Schema2/ISerializationContext.h>
#include <drx3D/Schema2/SerializationUtils.h>

namespace sxema2
{
	//////////////////////////////////////////////////////////////////////////
	CScriptAbstractInterfaceImplementation::CScriptAbstractInterfaceImplementation(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, EDomain domain, const SGUID& refGUID)
		: CScriptElementBase(EScriptElementType::AbstractInterfaceImplementation, file)
		, m_guid(guid)
		, m_scopeGUID(scopeGUID)
		, m_domain(domain)
		, m_refGUID(refGUID)
	{}

	//////////////////////////////////////////////////////////////////////////
	EAccessor CScriptAbstractInterfaceImplementation::GetAccessor() const
	{
		return EAccessor::Private;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptAbstractInterfaceImplementation::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptAbstractInterfaceImplementation::GetScopeGUID() const
	{
		return m_scopeGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptAbstractInterfaceImplementation::SetName(tukk szName)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptAbstractInterfaceImplementation::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceImplementation::EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const
	{
		SXEMA2_SYSTEM_ASSERT(enumerator);
		if(enumerator)
		{
			if(m_domain == EDomain::Script)
			{
				enumerator(m_refGUID);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceImplementation::Refresh(const SScriptRefreshParams& params)
	{
		if((params.reason == EScriptRefreshReason::EditorAdd) || (params.reason == EScriptRefreshReason::EditorFixUp) || (params.reason == EScriptRefreshReason::EditorPaste) || (params.reason == EScriptRefreshReason::EditorDependencyModified))
		{
			switch(m_domain)
			{
			case EDomain::Env:
				{
					IAbstractInterfaceConstPtr pAbstractInterface = gEnv->pSchematyc2->GetEnvRegistry().GetAbstractInterface(m_refGUID);
					if(pAbstractInterface)
					{
						m_name = pAbstractInterface->GetName();
						RefreshEnvAbstractrInterfaceFunctions(*pAbstractInterface);
					}
					break;
				}
			case EDomain::Script:
				{
					ScriptIncludeRecursionUtils::TGetAbstractInterfaceResult abstractInterface = ScriptIncludeRecursionUtils::GetAbstractInterface(CScriptElementBase::GetFile(), m_refGUID);
					if(abstractInterface.second)
					{
						m_name = abstractInterface.second->GetName();
						RefreshScriptAbstractrInterfaceFunctions(*abstractInterface.first);
						RefreshScriptAbstractrInterfaceTasks(*abstractInterface.first);
					}
					break;
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceImplementation::Serialize(Serialization::IArchive& archive)
	{
		LOADING_TIME_PROFILE_SECTION;

		CScriptElementBase::Serialize(archive);

		SerializationContext::SetValidatorLink(archive, SValidatorLink(m_guid)); // #SchematycTODO : Can we set this from CScriptElementBase?
		switch(SerializationContext::GetPass(archive))
		{
		case ESerializationPass::PreLoad:
		case ESerializationPass::Save:
			{
				archive(m_guid, "guid");
				archive(m_scopeGUID, "scope_guid");
				archive(m_name, "name");
				archive(m_domain, "origin"); // #SchematycTODO : Update name and patch files!
				archive(m_refGUID, "abstract_interface_guid");
				break;
			}
		case ESerializationPass::Validate:
			{
				Validate(archive);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceImplementation::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		m_guid      = guidRemapper.Remap(m_guid);
		m_scopeGUID = guidRemapper.Remap(m_scopeGUID);
		m_refGUID   = guidRemapper.Remap(m_refGUID);
	}

	//////////////////////////////////////////////////////////////////////////
	EDomain CScriptAbstractInterfaceImplementation::GetDomain() const
	{
		return m_domain;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptAbstractInterfaceImplementation::GetRefGUID() const
	{
		return m_refGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceImplementation::RefreshEnvAbstractrInterfaceFunctions(const IAbstractInterface& abstractInterface)
	{
		// Collect graphs.
		IScriptFile&    file = CScriptElementBase::GetFile();
		TDocGraphVector graphs;
		DocUtils::CollectGraphs(file, m_guid, false, graphs);
		// Iterate through all functions in abstract interface and create corresponding graphs (if they don't already exist).
		for(size_t functionIdx = 0, functionCount = abstractInterface.GetFunctionCount(); functionIdx < functionCount; ++ functionIdx)
		{
			IAbstractInterfaceFunctionConstPtr pFunction = abstractInterface.GetFunction(functionIdx);
			const SGUID                        functionGUID = pFunction->GetGUID();
			bool                               bGraphExists = false;
			for(const IDocGraph* pGraph : graphs)
			{
				if(pGraph->GetContextGUID() == functionGUID)
				{
					bGraphExists = true;
					break;
				}
			}
			if(!bGraphExists)
			{
				IDocGraph* pGraph = file.AddGraph(SScriptGraphParams(m_guid, pFunction->GetName(), EScriptGraphType::AbstractInterfaceFunction, functionGUID));
				DRX_ASSERT(pGraph);
				if(pGraph)
				{
					pGraph->AddNode(EScriptGraphNodeType::Begin, SGUID(), SGUID(), Vec2(0.0f, 0.0f));	// #SchematycTODO : This should be handled by the graph itself!!!
					graphs.push_back(pGraph);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceImplementation::RefreshScriptAbstractrInterfaceFunctions(const IScriptFile& abstractrInterfaceFile)
	{
		typedef std::map<SGUID, IDocGraph*> FunctionGraphs;

		IScriptFile&   file = CScriptElementBase::GetFile();
		FunctionGraphs functionGraphs;

		auto visitGraph = [&functionGraphs] (IDocGraph& graph) -> EVisitStatus
		{
			if(graph.GetType() == EScriptGraphType::AbstractInterfaceFunction)
			{
				functionGraphs.insert(FunctionGraphs::value_type(graph.GetContextGUID(), &graph));
			}
			return EVisitStatus::Continue;
		};
		file.VisitGraphs(DocGraphVisitor::FromLambdaFunction(visitGraph), m_guid, false);

		auto visitAbstractInterfaceFunction = [this, &file, &functionGraphs] (const IScriptAbstractInterfaceFunction& function) -> EVisitStatus
		{
			const SGUID              functionGUID = function.GetGUID();
			FunctionGraphs::iterator itFunctionGraph = functionGraphs.find(functionGUID);
			if(itFunctionGraph == functionGraphs.end())
			{
				IDocGraph* pGraph = file.AddGraph(SScriptGraphParams(m_guid, function.GetName(), EScriptGraphType::AbstractInterfaceFunction, functionGUID));
				DRX_ASSERT(pGraph);
				if(pGraph)
				{
					pGraph->AddNode(EScriptGraphNodeType::Begin, SGUID(), SGUID(), Vec2(0.0f, 0.0f)); // #SchematycTODO : This should be handled by the graph itself!!!
				}
			}
			else
			{
				functionGraphs.erase(itFunctionGraph);
			}
			return EVisitStatus::Continue;
		};
		abstractrInterfaceFile.VisitAbstractInterfaceFunctions(ScriptAbstractInterfaceFunctionConstVisitor::FromLambdaFunction(visitAbstractInterfaceFunction), m_refGUID, false);

		for(FunctionGraphs::value_type& functionGraph : functionGraphs)
		{
			functionGraph.second->SetElementFlags(EScriptElementFlags::Discard);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceImplementation::RefreshScriptAbstractrInterfaceTasks(const IScriptFile& abstractrInterfaceFile)
	{
		typedef std::unordered_map<SGUID, IScriptStateMachine*> TaskStateMachines;

		IScriptFile&      file = CScriptElementBase::GetFile();
		TaskStateMachines taskStateMachines;

		auto visitStateMachine = [&taskStateMachines] (IScriptStateMachine& stateMachine) -> EVisitStatus
		{
			if(stateMachine.GetLifetime() == EScriptStateMachineLifetime::Task)
			{
				taskStateMachines.insert(TaskStateMachines::value_type(stateMachine.GetContextGUID(), &stateMachine));
			}
			return EVisitStatus::Continue;
		};
		file.VisitStateMachines(ScriptStateMachineVisitor::FromLambdaFunction(visitStateMachine), m_guid, false);

		auto visitAbstractInterfaceTask = [this, &abstractrInterfaceFile, &file, &taskStateMachines] (const IScriptAbstractInterfaceTask& task) -> EVisitStatus
		{
			const SGUID                 taskGUID = task.GetGUID();
			TaskStateMachines::iterator itTaskStateMachine = taskStateMachines.find(taskGUID);
			if(itTaskStateMachine == taskStateMachines.end())
			{
				file.AddStateMachine(m_guid, task.GetName(), EScriptStateMachineLifetime::Task, taskGUID, SGUID());
			}
			else
			{
				taskStateMachines.erase(itTaskStateMachine);
			}
			RefreshScriptAbstractrInterfaceTaskPropertiess(abstractrInterfaceFile, taskGUID);
			return EVisitStatus::Continue;
		};
		abstractrInterfaceFile.VisitAbstractInterfaceTasks(ScriptAbstractInterfaceTaskConstVisitor::FromLambdaFunction(visitAbstractInterfaceTask), m_refGUID, false);

		for(TaskStateMachines::value_type& taskStateMachine : taskStateMachines)
		{
			taskStateMachine.second->SetElementFlags(EScriptElementFlags::Discard);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceImplementation::RefreshScriptAbstractrInterfaceTaskPropertiess(const IScriptFile& abstractrInterfaceFile, const SGUID& taskGUID) {}

	//////////////////////////////////////////////////////////////////////////
	void CScriptAbstractInterfaceImplementation::Validate(Serialization::IArchive& archive)
	{
		if(m_refGUID)
		{
			switch(m_domain)
			{
			case EDomain::Env:
				{
					IAbstractInterfaceConstPtr pAbstractInterface = gEnv->pSchematyc2->GetEnvRegistry().GetAbstractInterface(m_refGUID);
					if(!pAbstractInterface)
					{
						archive.error(*this, "Unable to retrieve abstract interface!");
					}
					break;
				}
			case EDomain::Script:
				{
					ScriptIncludeRecursionUtils::TGetAbstractInterfaceResult abstractInterface = ScriptIncludeRecursionUtils::GetAbstractInterface(CScriptElementBase::GetFile(), m_refGUID);
					if(!abstractInterface.second)
					{
						archive.error(*this, "Unable to retrieve abstract interface!");
					}
					break;
				}
			}
		}
	}
}
