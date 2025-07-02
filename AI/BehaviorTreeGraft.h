// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef BehaviorTreeGraft_h
#define BehaviorTreeGraft_h

#pragma once

#include <drx3D/AI/IBehaviorTreeGraft.h>

namespace BehaviorTree
{
class GraftUpr
	: public IGraftUpr
{
public:

	GraftUpr() {}
	~GraftUpr() {}

	void Reset();

	void GraftNodeReady(EntityId entityId, IGraftNode* graftNode);
	void GraftNodeTerminated(EntityId entityId);
	void GraftBehaviorComplete(EntityId entityId);

	// IGraftUpr
	virtual bool RunGraftBehavior(EntityId entityId, tukk behaviorName, XmlNodeRef behaviorXml, IGraftBehaviorListener* listener) override;
	virtual bool RequestGraftMode(EntityId entityId, IGraftModeListener* listener) override;
	virtual void CancelGraftMode(EntityId entityId) override;
	// ~IGraftUpr

private:

	typedef VectorMap<EntityId, IGraftModeListener*> GraftModeRequestsContainer;
	GraftModeRequestsContainer m_graftModeRequests;

	typedef VectorMap<EntityId, IGraftBehaviorListener*> GraftBehaviorRequestsContainer;
	GraftBehaviorRequestsContainer m_graftBehaviorRequests;

	typedef VectorMap<EntityId, IGraftNode*> ActiveGraftNodesContainer;
	ActiveGraftNodesContainer m_activeGraftNodes;
};
}

#endif // BehaviorTreeGraft_h
