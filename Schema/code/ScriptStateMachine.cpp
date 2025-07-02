// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptStateMachine.h>

#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/Schema/IScriptGraph.h>
#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/IGUIDRemapper.h>

#include <drx3D/Schema/ScriptView.h>
#include <drx3D/Schema/ScriptGraph.h>
#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphBeginNode.h>

SERIALIZATION_ENUM_BEGIN_NESTED(sxema, EScriptStateMachineLifetime, "DrxSchematyc Script State Machine Lifetime")
SERIALIZATION_ENUM(sxema::EScriptStateMachineLifetime::Persistent, "persistent", "Persistent")
SERIALIZATION_ENUM(sxema::EScriptStateMachineLifetime::Task, "task", "Task")
SERIALIZATION_ENUM_END()

namespace sxema
{
CScriptStateMachine::CScriptStateMachine()
	: CScriptElementBase(EScriptElementFlags::CanOwnScript)
	, m_lifetime(EScriptStateMachineLifetime::Persistent)
{
	CreateTransitionGraph();
}

CScriptStateMachine::CScriptStateMachine(const DrxGUID& guid, tukk szName, EScriptStateMachineLifetime lifetime, const DrxGUID& contextGUID, const DrxGUID& partnerGUID)
	: CScriptElementBase(guid, szName, EScriptElementFlags::CanOwnScript)
	, m_lifetime(lifetime)
	, m_contextGUID(contextGUID)
	, m_partnerGUID(partnerGUID)
{
	CreateTransitionGraph();
}

void CScriptStateMachine::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const {}

void CScriptStateMachine::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	m_partnerGUID = guidRemapper.Remap(m_partnerGUID);
	m_contextGUID = guidRemapper.Remap(m_contextGUID);
	m_transitionGraphGUID = guidRemapper.Remap(m_transitionGraphGUID);
}

void CScriptStateMachine::ProcessEvent(const SScriptEvent& event)
{
	switch (event.id)
	{
	case EScriptEventId::EditorAdd:
		{
			// TODO: This should happen in editor!
			IScriptGraph* pGraph = static_cast<IScriptGraph*>(CScriptElementBase::GetExtensions().QueryExtension(EScriptExtensionType::Graph));
			SXEMA_CORE_ASSERT(pGraph);
			if (pGraph)
			{
				// TODO : Shouldn't we be using CScriptGraphNodeFactory::CreateNode() instead of instantiating the node directly?!?
				pGraph->AddNode(std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphBeginNode>()));
				// ~TODO
			}
			// ~TODO

			break;
		}
	case EScriptEventId::EditorFixUp:
	case EScriptEventId::EditorPaste:
		{
			RefreshTransitionGraph();
			break;
		}
	}

	CScriptElementBase::ProcessEvent(event);
}

void CScriptStateMachine::Serialize(Serialization::IArchive& archive)
{
	// #SchematycTODO : Shouldn't this be handled by CScriptElementBase itself?
	CScriptElementBase::Serialize(archive);
	CMultiPassSerializer::Serialize(archive);
	CScriptElementBase::SerializeExtensions(archive);
}

EScriptStateMachineLifetime CScriptStateMachine::GetLifetime() const
{
	return m_lifetime;
}

DrxGUID CScriptStateMachine::GetContextGUID() const
{
	return m_contextGUID;
}

DrxGUID CScriptStateMachine::GetPartnerGUID() const
{
	return m_partnerGUID;
}

void CScriptStateMachine::Load(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_lifetime, "lifetime", "!Lifetime");
	archive(m_contextGUID, "contextGUID");
	archive(m_transitionGraphGUID, "transition_graph_guid");
	archive(m_partnerGUID, "partnerGUID");
}

void CScriptStateMachine::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_lifetime, "lifetime", "!Lifetime");
	archive(m_contextGUID, "contextGUID");
	archive(m_transitionGraphGUID, "transition_graph_guid");

	archive(m_partnerGUID, "partnerGUID");
}

void CScriptStateMachine::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	/*
	   typedef std::vector<DrxGUID> GUIDs;

	   GUIDs partnerGUIDs;
	   Serialization::StringList partnerNames;
	   partnerGUIDs.reserve(16);
	   partnerNames.reserve(16);
	   partnerGUIDs.push_back(DrxGUID());
	   partnerNames.push_back("None");

	   CScriptView scriptView(GetParent()->GetGUID());

	   auto visitStateMachine = [this, &partnerGUIDs, &partnerNames](const IScriptStateMachine& stateMachine) -> EVisitStatus
	   {
	   if (stateMachine.GetLifetime() == EScriptStateMachineLifetime::Persistent)
	   {
	    const DrxGUID stateMachineGUID = stateMachine.GetGUID();
	    if (stateMachineGUID != CScriptElementBase::GetGUID())
	    {
	      partnerGUIDs.push_back(stateMachineGUID);
	      partnerNames.push_back(stateMachine.GetName());
	    }
	   }
	   return EVisitStatus::Continue;
	   };
	   scriptView.VisitScriptStateMachines(visitStateMachine, EDomainScope::Local);

	   GUIDs::const_iterator itPartnerGUID = std::find(partnerGUIDs.begin(), partnerGUIDs.end(), m_partnerGUID);
	   i32k partnerIdx = itPartnerGUID != partnerGUIDs.end() ? static_cast<i32>(itPartnerGUID - partnerGUIDs.begin()) : 0;
	   Serialization::StringListValue partnerName(partnerNames, partnerIdx);
	   archive(partnerName, "partnerName", "Partner");
	   if (archive.isInput())
	   {
	   m_partnerGUID = partnerGUIDs[partnerName.index()];
	   }
	 */
}

void CScriptStateMachine::CreateTransitionGraph()
{
	CScriptElementBase::AddExtension(std::make_shared<CScriptGraph>(*this, EScriptGraphType::Transition));
}

void CScriptStateMachine::RefreshTransitionGraph()
{
	if (gEnv->IsEditor())
	{
		/*IScriptFile& file = CScriptElementBase::GetFile();
		   if(!file.IsDummyFile() && (!m_transitionGraphGUID || !file.GetGraph(m_transitionGraphGUID)))
		   {
		   IDocGraph* pTransitionGraph = file.AddGraph(SScriptGraphParams(CScriptElementBase::GetGUID(), "Transitions", EScriptGraphType::Transition_DEPRECATED, DrxGUID()));
		   SXEMA_CORE_ASSERT(pTransitionGraph);
		   if(pTransitionGraph)
		   {
		    IScriptGraphNode* pBeginNode = pTransitionGraph->AddNode(EScriptGraphNodeType::BeginState, DrxGUID(), DrxGUID(), Vec2(0.0f, 0.0f));
		    if(m_lifetime == EScriptStateMachineLifetime::Task)
		    {
		      IScriptGraphNode* pEndNode = pTransitionGraph->AddNode(EScriptGraphNodeType::EndState, DrxGUID(), DrxGUID(), Vec2(200.0f, 0.0f));
		      pTransitionGraph->AddLink(pBeginNode->GetGUID(), pBeginNode->GetOutputName(0), pEndNode->GetGUID(), pEndNode->GetInputName(0));
		    }
		    m_transitionGraphGUID = pTransitionGraph->GetGUID();
		   }
		   }*/
	}
}
} // sxema
