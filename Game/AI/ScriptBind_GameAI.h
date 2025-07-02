// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef ScriptBind_GameAI_h
#define ScriptBind_GameAI_h

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>
#include <drx3D/Game/RangeModule.h> // For RangeID

struct IGameFramework;
struct ISystem;

class CScriptBind_GameAI : public CScriptableBase
{
public:
	CScriptBind_GameAI(ISystem* system, IGameFramework* gameFramework);

	virtual void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

private:
	void RegisterMethods();
	void RegisterGlobals();

	i32 RegisterWithModule(IFunctionHandler* pH, tukk moduleName, ScriptHandle entityID);
	i32 UnregisterWithModule(IFunctionHandler* pH, tukk moduleName, ScriptHandle entityID);
	i32 UnregisterWithAllModules(IFunctionHandler* pH, ScriptHandle entityID);
	i32 PauseModule(IFunctionHandler* pH, tukk moduleName, ScriptHandle entityID);
	i32 PauseAllModules(IFunctionHandler* pH, ScriptHandle entityID);
	i32 ResumeModule(IFunctionHandler* pH, tukk moduleName, ScriptHandle entityID);
	i32 ResumeAllModules(IFunctionHandler* pH, ScriptHandle entityID);
	bool GetEntitiesInRange(const Vec3& center, float radius, tukk pClassName, SEntityProximityQuery* pQuery) const;
	i32 GetClosestEntityToTarget(IFunctionHandler* funcHandler, Vec3 attackerPos, Vec3 targetPos, tukk pClassName, float radius, float maxAngle);
	i32 GetBattleFrontPosition(IFunctionHandler* pH, i32 groupID);

	i32 ResetAdvantagePointOccupancyControl(IFunctionHandler* pH);
	i32 OccupyAdvantagePoint(IFunctionHandler* pH, ScriptHandle entityID, Vec3 point);
	i32 ReleaseAdvantagePointFor(IFunctionHandler* pH, ScriptHandle entityID);
	i32 IsAdvantagePointOccupied(IFunctionHandler* pH, Vec3 point);

	i32 StartSearchModuleFor(IFunctionHandler* pH, i32 groupID, Vec3 targetPos);
	i32 StopSearchModuleFor(IFunctionHandler* pH, i32 groupID);
	i32 GetNextSearchSpot(IFunctionHandler* pH, ScriptHandle entityID, float closenessToAgentWeight, float closenessToTargetWeight, float minDistanceFromAgent);
	i32 MarkAssignedSearchSpotAsUnreachable(IFunctionHandler* pH, ScriptHandle entityID);

	i32 ResetRanges(IFunctionHandler* pH, ScriptHandle entityID);
	i32 AddRange(IFunctionHandler* pH, ScriptHandle entityID, float range, tukk enterSignal, tukk leaveSignal);
	i32 GetRangeState(IFunctionHandler* pH, ScriptHandle entityID, i32 rangeID);
	i32 ChangeRange(IFunctionHandler* pH, ScriptHandle entityID, i32 rangeID, float distance);

	i32 ResetAloneDetector(IFunctionHandler* pH, ScriptHandle entityID);
	i32 SetupAloneDetector(IFunctionHandler* pH, ScriptHandle entityID, float range, tukk aloneSignal, tukk notAloneSignal);
	i32 AddActorClassToAloneDetector(IFunctionHandler* pH, ScriptHandle entityID, tukk entityClassName);
	i32 RemoveActorClassFromAloneDetector(IFunctionHandler* pH, ScriptHandle entityID, tukk entityClassName);
	i32 IsAloneForAloneDetector(IFunctionHandler* pH, ScriptHandle entityID);

	i32 RegisterObjectVisible(IFunctionHandler *pH, ScriptHandle entityID);
	i32 UnregisterObjectVisible(IFunctionHandler *pH, ScriptHandle entityID);

	i32 IsAISystemEnabled(IFunctionHandler *pH);

	i32 RegisterEntityForAISquadUpr(IFunctionHandler* pH, ScriptHandle entityId);
	i32 RemoveEntityForAISquadUpr(IFunctionHandler* pH, ScriptHandle entityId);
	i32 GetSquadId(IFunctionHandler* pH, ScriptHandle entityId);
	i32 GetSquadMembers(IFunctionHandler* pH, i32 squadId);
	i32 GetAveragePositionOfSquadScopeUsers(IFunctionHandler* pH, ScriptHandle entityId, tukk squadScopeName);
	i32 GetSquadScopeUserCount(IFunctionHandler* pH, ScriptHandle entityId, tukk squadScopeName);

	i32 IsSwimmingUnderwater(IFunctionHandler* pH, ScriptHandle entityID);

	ISystem* m_system;
	IGameFramework* m_gameFramework;
	IScriptSystem* m_scriptSystem;
};

#endif // ScriptBind_GameAI_h
