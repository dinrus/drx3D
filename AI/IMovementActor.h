// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __IMOVEMENTACTOR_H__
#define __IMOVEMENTACTOR_H__

#include <drx3D/AI/IMovementSystem.h>

struct IMovementActorAdapter;

struct IMovementActionAbilities
{
	typedef std::function<void(const Movement::BlockPtr&)> OnCreateFunction;
	
	virtual void CreateStartBlocksForPlanner(const MovementRequest& request, OnCreateFunction onCreateFnc) = 0;
	virtual void CreateEndBlocksForPlanner(const MovementRequest& request, OnCreateFunction onCreateFnc) = 0;
	virtual void PrePathFollowUpdateCall(const MovementUpdateContext& context, bool bIsLastBlock) = 0;
	virtual void PostPathFollowUpdateCall(const MovementUpdateContext& context, bool bIsLastBlock) = 0;
};

struct IMovementActor
{
	virtual ~IMovementActor() {}

	virtual IMovementActorAdapter&    GetAdapter() const = 0;
	virtual void                      RequestPathTo(const Vec3& destination, float lengthToTrimFromThePathEnd, const SSnapToNavMeshRulesInfo& snappingRules, const MNMDangersFlags dangersFlags = eMNMDangers_None,
	                                                const bool considerActorsAsPathObstacles = false, const MNMCustomPathCostComputerSharedPtr& pCustomPathCostComputer = nullptr) = 0;
	virtual Movement::PathfinderState GetPathfinderState() const = 0;
	virtual tukk               GetName() const = 0;
	virtual void                      Log(tukk message) = 0;

	virtual EntityId                  GetEntityId() const = 0;
	virtual MovementActorCallbacks    GetCallbacks() const = 0;
	virtual IMovementActionAbilities& GetActionAbilities() = 0;
};

#endif // __IMOVEMENTACTOR_H__
