// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptClass.h>

#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/Schema/IScriptGraph.h>
#include <drx3D/Schema/IScriptRegistry.h>
#include <drx3D/Schema/IScriptSignalReceiver.h>
#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/IGUIDRemapper.h>

#include <drx3D/Schema/CVars.h>
#include <drx3D/Schema/CoreEnvSignals.h>
#include <drx3D/Schema/ScriptGraphNode.h>
#include <drx3D/Schema/ScriptGraphReceiveSignalNode.h>

namespace sxema
{
CScriptClass::CScriptClass()
	: CScriptElementBase(EScriptElementFlags::MustOwnScript)
{}

CScriptClass::CScriptClass(const DrxGUID& guid, tukk szName)
	: CScriptElementBase(guid, szName, EScriptElementFlags::MustOwnScript)
{}

void CScriptClass::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const {}

void CScriptClass::RemapDependencies(IGUIDRemapper& guidRemapper)                                                        {}

void CScriptClass::ProcessEvent(const SScriptEvent& event)
{
	CScriptElementBase::ProcessEvent(event);

	switch (event.id)
	{
	case EScriptEventId::EditorAdd:
		{
			// TODO: This should happen in editor!
			IScriptRegistry& scriptRegistry = gEnv->pSchematyc->GetScriptRegistry();
			scriptRegistry.AddConstructor("ConstructionGraph", this);

			IScriptSignalReceiver* pSignalReceiver = scriptRegistry.AddSignalReceiver("SignalGraph", EScriptSignalReceiverType::Universal, DrxGUID(), this);
			IScriptGraph* pGraph = static_cast<IScriptGraph*>(pSignalReceiver->GetExtensions().QueryExtension(EScriptExtensionType::Graph));
			if (pGraph)
			{
				IScriptGraphNodePtr pStartNode = std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphReceiveSignalNode>(SElementId(EDomain::Env, GetTypeDesc<SStartSignal>().GetGUID()))); // #SchematycTODO : Shouldn't we be using CScriptGraphNodeFactory::CreateNode() instead of instantiating the node directly?!?
				pStartNode->SetPos(Vec2(0.0f, 0.0f));
				pGraph->AddNode(pStartNode);

				IScriptGraphNodePtr pStopNode = std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphReceiveSignalNode>(SElementId(EDomain::Env, GetTypeDesc<SStopSignal>().GetGUID()))); // #SchematycTODO : Shouldn't we be using CScriptGraphNodeFactory::CreateNode() instead of instantiating the node directly?!?
				pStopNode->SetPos(Vec2(0.0f, 100.0f));
				pGraph->AddNode(pStopNode);

				IScriptGraphNodePtr pUpdateNode = std::make_shared<CScriptGraphNode>(gEnv->pSchematyc->CreateGUID(), stl::make_unique<CScriptGraphReceiveSignalNode>(SElementId(EDomain::Env, GetTypeDesc<SUpdateSignal>().GetGUID()))); // #SchematycTODO : Shouldn't we be using CScriptGraphNodeFactory::CreateNode() instead of instantiating the node directly?!?
				pUpdateNode->SetPos(Vec2(0.0f, 200.0f));
				pGraph->AddNode(pUpdateNode);
			}

			m_userDocumentation.SetCurrentUserAsAuthor();
			// ~TODO

			break;
		}
	case EScriptEventId::EditorPaste:
		{
			m_userDocumentation.SetCurrentUserAsAuthor();
			break;
		}
	}
}

void CScriptClass::Serialize(Serialization::IArchive& archive)
{
	// #SchematycTODO : Shouldn't this be handled by CScriptElementBase itself?
	CScriptElementBase::Serialize(archive);
	CMultiPassSerializer::Serialize(archive);
	CScriptElementBase::SerializeExtensions(archive);
}

tukk CScriptClass::GetAuthor() const
{
	return m_userDocumentation.author.c_str();
}

tukk CScriptClass::GetDescription() const
{
	return m_userDocumentation.description.c_str();
}

void CScriptClass::Load(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_userDocumentation, "userDocumentation", "Documentation");
}

void CScriptClass::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_userDocumentation, "userDocumentation", "Documentation");
}

void CScriptClass::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_userDocumentation, "userDocumentation", "Documentation");
}
} // sxema
