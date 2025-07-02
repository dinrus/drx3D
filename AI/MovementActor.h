// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/IMovementActor.h>
#include <drx3D/AI/MovementRequest.h>
#include <drx3D/AI/IPathfinder.h>
#include <drx3D/AI/MovementUpdateContext.h>

class CMovementActionAbilities : public IMovementActionAbilities
{
	friend struct MovementActor;
public:
	virtual void CreateStartBlocksForPlanner(const MovementRequest& request, OnCreateFunction onCreateFnc) override
	{
		SMovementActionAbilityCallbacks::Blocks createdBlocks;
		m_addStartMovementBlocksCallback.Call<SMovementActionAbilityCallbacks::Blocks&, const MovementRequest&>(createdBlocks, request);
		for (const Movement::BlockPtr& block : createdBlocks)
		{
			onCreateFnc(block);
		}
	}

	virtual void CreateEndBlocksForPlanner(const MovementRequest& request, OnCreateFunction onCreateFnc) override
	{
		SMovementActionAbilityCallbacks::Blocks createdBlocks;
		m_addEndMovementBlocksCallback.Call<SMovementActionAbilityCallbacks::Blocks&, const MovementRequest&>(createdBlocks, request);
		for (const Movement::BlockPtr& block : createdBlocks)
		{
			onCreateFnc(block);
		}
	}

	virtual void PrePathFollowUpdateCall(const MovementUpdateContext& context, bool bIsLastBlock) override
	{
		m_prePathFollowingUpdateCallback.Call(context, bIsLastBlock);
	}

	virtual void PostPathFollowUpdateCall(const MovementUpdateContext& context, bool bIsLastBlock) override
	{
		m_postPathFollowingUpdateCallback.Call(context, bIsLastBlock);
	}
private:

	CFunctorsList<SMovementActionAbilityCallbacks::MovementUpdateCallback> m_prePathFollowingUpdateCallback;
	CFunctorsList<SMovementActionAbilityCallbacks::MovementUpdateCallback> m_postPathFollowingUpdateCallback;
	CFunctorsList<SMovementActionAbilityCallbacks::CreateBlocksCallback> m_addStartMovementBlocksCallback;
	CFunctorsList<SMovementActionAbilityCallbacks::CreateBlocksCallback> m_addEndMovementBlocksCallback;
};

// The movement system processes the movement requests for an actor
// in the order they come. Eaten off front-to-back. This is the queue.
typedef std::deque<MovementRequestID> MovementRequestQueue;

// -------------------------------------------------------------------------------------------

// Contains the information related to one specific actor in the
// context of movement.
// Retrieve information about an actor through this object.
struct MovementActor : public IMovementActor
{
	MovementActor(const EntityId _entityID, IMovementActorAdapter* _pAdapter)
		: entityID(_entityID)
		, requestIdCurrentlyInPlanner(0)
		, pAdapter(_pAdapter)
	{
		assert(pAdapter);
	}

	virtual ~MovementActor() {}

	// IMovementActor
	virtual IMovementActorAdapter&    GetAdapter() const override;
	virtual void                      RequestPathTo(const Vec3& destination, float lengthToTrimFromThePathEnd, const SSnapToNavMeshRulesInfo& snappingRules, const MNMDangersFlags dangersFlags,
	                                                const bool considerActorsAsPathObstacles, const MNMCustomPathCostComputerSharedPtr& pCustomPathCostComputer) override;
	virtual Movement::PathfinderState GetPathfinderState() const override;
	virtual tukk               GetName() const override;
	virtual void                      Log(tukk message) override;

	virtual EntityId                  GetEntityId() const override                   { return entityID; };
	virtual MovementActorCallbacks    GetCallbacks() const override                  { return callbacks; };
	virtual IMovementActionAbilities& GetActionAbilities() override                  { return actionAbilities; }
	// ~IMovementActor

	bool AddActionAbilityCallbacks (const SMovementActionAbilityCallbacks& ability);
	bool RemoveActionAbilityCallbacks(const SMovementActionAbilityCallbacks& ability);

	CAIActor* GetAIActor();

	std::shared_ptr<Movement::IPlanner> planner;
	EntityId                            entityID;
	MovementRequestQueue                requestQueue;
	MovementRequestID                   requestIdCurrentlyInPlanner;

	MovementActorCallbacks              callbacks;
	CMovementActionAbilities            actionAbilities;
private:
	IMovementActorAdapter*              pAdapter;
};
