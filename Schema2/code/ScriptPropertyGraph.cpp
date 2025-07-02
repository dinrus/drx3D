// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptPropertyGraph.h>

#include <drx3D/Schema2/IFramework.h>

#include <drx3D/Schema2/DomainContext.h>
#include <drx3D/Schema2/Deprecated/DocGraphNodes/DocGraphNodes.h>
#include <drx3D/Schema2/ScriptGraphBeginNode.h>

namespace sxema2
{
	CScriptPropertyGraph::CScriptPropertyGraph(IScriptElement& element)
		: CScriptGraphBase(element)
	{}

	void CScriptPropertyGraph::Refresh_New(const SScriptRefreshParams& params)
	{
		if((params.reason == EScriptRefreshReason::EditorAdd) || ((params.reason == EScriptRefreshReason::EditorFixUp) && !BeginNodeExists()))
		{
			AddNode_New(std::make_shared<CScriptGraphBeginNode>(gEnv->pSchematyc2->CreateGUID()));
		}

		CScriptGraphBase::Refresh_New(params);
	}

	void CScriptPropertyGraph::Serialize_New(Serialization::IArchive& archive)
	{
		CScriptGraphBase::Serialize_New(archive);
	}

	EScriptGraphType CScriptPropertyGraph::GetType() const
	{
		return EScriptGraphType::Property;
	}

	bool CScriptPropertyGraph::BeginNodeExists() const
	{
		bool beginNodeExists = false;
		auto visitNode = [&beginNodeExists] (const IScriptGraphNode& node) -> EVisitStatus
		{
			if(node.GetTypeGUID() == CScriptGraphBeginNode::s_typeGUID)
			{
				beginNodeExists = true;
				return EVisitStatus::Stop;
			}
			return EVisitStatus::Continue;
		};
		CScriptGraphBase::VisitNodes(ScriptGraphNodeConstVisitor::FromLambdaFunction(visitNode));
		return beginNodeExists;
	}
}
