// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/BehaviorTreeGraft.h>
#include <drx3D/AI/BehaviorTreeUpr.h>
#include <drx3D/AI/IBehaviorTree.h>

namespace BehaviorTree
{
void GraftUpr::Reset()
{
	m_activeGraftNodes.clear();
	m_graftModeRequests.clear();
	m_graftBehaviorRequests.clear();
}

void GraftUpr::GraftNodeReady(EntityId entityId, IGraftNode* graftNode)
{
	std::pair<ActiveGraftNodesContainer::iterator, bool> insertResult = m_activeGraftNodes.insert(std::make_pair(entityId, graftNode));
	if (!insertResult.second)
	{
		IEntity* entity = gEnv->pEntitySystem->GetEntity(entityId);
		gEnv->pLog->LogError("Graft Upr: More than one graft node is running for the entity '%s'", entity->GetName());
		return;
	}

	GraftModeRequestsContainer::iterator requestIt = m_graftModeRequests.find(entityId);
	if (requestIt != m_graftModeRequests.end() && requestIt->second != NULL)
	{
		requestIt->second->GraftModeReady(entityId);
	}
	else
	{
		IEntity* entity = gEnv->pEntitySystem->GetEntity(entityId);
		gEnv->pLog->LogError("Graft Upr: A graft node is running for the entity '%s' without a corresponding request.", entity->GetName());
	}
}

void GraftUpr::GraftNodeTerminated(EntityId entityId)
{
	GraftModeRequestsContainer::iterator graftModeRequestIt = m_graftModeRequests.find(entityId);
	if (graftModeRequestIt != m_graftModeRequests.end() && graftModeRequestIt->second != NULL)
	{
		graftModeRequestIt->second->GraftModeInterrupted(entityId);
		m_graftModeRequests.erase(graftModeRequestIt);
	}

	ActiveGraftNodesContainer::iterator activeGraftNodeIt = m_activeGraftNodes.find(entityId);
	if (activeGraftNodeIt != m_activeGraftNodes.end())
	{
		m_activeGraftNodes.erase(activeGraftNodeIt);
	}

	m_graftBehaviorRequests.erase(entityId);
}

void GraftUpr::GraftBehaviorComplete(EntityId entityId)
{
	GraftBehaviorRequestsContainer::iterator graftBehaviorRequestIt = m_graftBehaviorRequests.find(entityId);
	if (graftBehaviorRequestIt != m_graftBehaviorRequests.end() && graftBehaviorRequestIt->second != NULL)
	{
		graftBehaviorRequestIt->second->GraftBehaviorComplete(entityId);
	}
	else
	{
		IEntity* entity = gEnv->pEntitySystem->GetEntity(entityId);
		gEnv->pLog->LogError("Graft Upr: A graft behavior is complete for the entity '%s' without a corresponding request.", entity->GetName());
	}
}

bool GraftUpr::RunGraftBehavior(EntityId entityId, tukk behaviorName, XmlNodeRef behaviorXml, IGraftBehaviorListener* listener)
{
	m_graftBehaviorRequests[entityId] = listener;

	ActiveGraftNodesContainer::iterator activeGraftNodeIt = m_activeGraftNodes.find(entityId);
	if (activeGraftNodeIt != m_activeGraftNodes.end())
	{
		return activeGraftNodeIt->second->RunBehavior(entityId, behaviorName, behaviorXml);
	}

	IEntity* entity = gEnv->pEntitySystem->GetEntity(entityId);
	gEnv->pLog->LogError("Graft Upr: There is no active graft node to run the behavior '%s' by the entity '%s'.", behaviorName, entity->GetName());
	return false;
}

bool GraftUpr::RequestGraftMode(EntityId entityId, IGraftModeListener* listener)
{
	assert(entityId);

	GraftModeRequestsContainer::iterator graftModeRequestIt = m_graftModeRequests.find(entityId);
	const bool graftModeAlreadyRequested = graftModeRequestIt != m_graftModeRequests.end();
	if (graftModeAlreadyRequested)
	{
		IEntity* entity = gEnv->pEntitySystem->GetEntity(entityId);
		gEnv->pLog->LogError("Graft Upr: More the one graft mode request was performed for the entity '%s'", entity->GetName());
		return false;
	}
	else
	{
		m_graftModeRequests.insert(std::make_pair(entityId, listener));
		Event graftRequestEvent("OnGraftRequested");
		gAIEnv.pBehaviorTreeUpr->HandleEvent(entityId, graftRequestEvent);
		return true;
	}
}

void GraftUpr::CancelGraftMode(EntityId entityId)
{
	m_graftModeRequests.erase(entityId);
	Event graftRequestEvent("OnGraftCanceled");
	gAIEnv.pBehaviorTreeUpr->HandleEvent(entityId, graftRequestEvent);
}
}
