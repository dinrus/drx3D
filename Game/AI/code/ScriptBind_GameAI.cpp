// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/ScriptBind_GameAI.h>
#include <drx3D/Game/GameAIEnv.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/SearchModule.h>
#include <drx3D/Game/AIBattleFront.h>
#include <drx3D/Game/Assignment.h>
#include <drx3D/Game/AloneDetectorModule.h>
#include <IDrxMannequin.h>
#include <drx3D/Game/IAnimatedCharacter.h>
#include <drx3D/Game/UI/HUD/HUDEventDispatcher.h>
#include <drx3D/Game/Actor.h>


CScriptBind_GameAI::CScriptBind_GameAI(ISystem* system, IGameFramework* gameFramework)
: m_system(system)
, m_gameFramework(gameFramework)
, m_scriptSystem(system->GetIScriptSystem())
{
	Init(m_scriptSystem, m_system);
	SetGlobalName("GameAI");

	RegisterMethods();
	RegisterGlobals();
}

void CScriptBind_GameAI::RegisterMethods()
{
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_GameAI::

	SCRIPT_REG_TEMPLFUNC(RegisterWithModule, "moduleName, entityId");
	SCRIPT_REG_TEMPLFUNC(UnregisterWithModule, "moduleName, entityId");
	SCRIPT_REG_TEMPLFUNC(UnregisterWithAllModules, "entityId");
	SCRIPT_REG_TEMPLFUNC(PauseModule, "moduleName, entityId");
	SCRIPT_REG_TEMPLFUNC(PauseAllModules, "entityId");
	SCRIPT_REG_TEMPLFUNC(ResumeModule, "moduleName, entityId");
	SCRIPT_REG_TEMPLFUNC(ResumeAllModules, "entityId");
	SCRIPT_REG_TEMPLFUNC(GetClosestEntityToTarget, "attackerPos, targetPos, radius, maxAngle");
	SCRIPT_REG_TEMPLFUNC(GetBattleFrontPosition, "groupID");

 	SCRIPT_REG_FUNC(ResetAdvantagePointOccupancyControl);
 	SCRIPT_REG_TEMPLFUNC(OccupyAdvantagePoint, "entityId, point");
 	SCRIPT_REG_TEMPLFUNC(ReleaseAdvantagePointFor, "entityId");
	SCRIPT_REG_TEMPLFUNC(IsAdvantagePointOccupied, "point");

	SCRIPT_REG_TEMPLFUNC(StartSearchModuleFor, "groupID, point, [targetId], [searchSpotTimeout]");
	SCRIPT_REG_TEMPLFUNC(StopSearchModuleFor, "groupID");
	SCRIPT_REG_TEMPLFUNC(GetNextSearchSpot, "entityId, closenessToAgentWeight, closenessToTargetWeight, minDistanceFromAgent, [closenessToTargetCurrentPosWeight]");
	SCRIPT_REG_TEMPLFUNC(MarkAssignedSearchSpotAsUnreachable, "entityId");

	SCRIPT_REG_TEMPLFUNC(ResetRanges, "entityID");
	SCRIPT_REG_TEMPLFUNC(AddRange, "entityID, range, enterSignal, leaveSignal");
	SCRIPT_REG_TEMPLFUNC(GetRangeState, "entityID, rangeID");
	SCRIPT_REG_TEMPLFUNC(ChangeRange, "entityID, rangeID, distance");

	SCRIPT_REG_TEMPLFUNC(ResetAloneDetector, "entityID");
	SCRIPT_REG_TEMPLFUNC(SetupAloneDetector, "entityID, range, aloneSignal, notAloneSignal");
	SCRIPT_REG_TEMPLFUNC(AddActorClassToAloneDetector, "entityID, entityClassName");
	SCRIPT_REG_TEMPLFUNC(RemoveActorClassFromAloneDetector, "entityID, entityClassName");
	SCRIPT_REG_TEMPLFUNC(IsAloneForAloneDetector, "entityID");

	SCRIPT_REG_TEMPLFUNC(RegisterObjectVisible, "entityID");
	SCRIPT_REG_TEMPLFUNC(UnregisterObjectVisible, "entityID");

	SCRIPT_REG_FUNC(IsAISystemEnabled);

	SCRIPT_REG_TEMPLFUNC(RegisterEntityForAISquadUpr, "entityID");
	SCRIPT_REG_TEMPLFUNC(RemoveEntityForAISquadUpr, "entityID");

	SCRIPT_REG_TEMPLFUNC(GetSquadId, "entityId");
	SCRIPT_REG_TEMPLFUNC(GetSquadMembers, "squadId");
	SCRIPT_REG_TEMPLFUNC(GetAveragePositionOfSquadScopeUsers, "entityId, squadScopeName");
	SCRIPT_REG_TEMPLFUNC(GetSquadScopeUserCount, "entityId, squadScopeName");

	SCRIPT_REG_TEMPLFUNC(IsSwimmingUnderwater, "entityID");

#undef SCRIPT_REG_CLASSNAME
}

void CScriptBind_GameAI::RegisterGlobals()
{
	m_scriptSystem->SetGlobalValue("InsideRange", RangeContainer::Range::Inside);
	m_scriptSystem->SetGlobalValue("OutsideRange", RangeContainer::Range::Outside);
	m_scriptSystem->SetGlobalValue("UseAttentionTargetDistance", RangeContainer::Range::UseAttentionTargetDistance);
	m_scriptSystem->SetGlobalValue("UseLiveTargetDistance", RangeContainer::Range::UseLiveTargetDistance);

	m_scriptSystem->SetGlobalValue("VOR_Always", eVOR_Always);
	m_scriptSystem->SetGlobalValue("VOR_UseMaxViewDist", eVOR_UseMaxViewDist);
	m_scriptSystem->SetGlobalValue("VOR_OnlyWhenMoving", eVOR_OnlyWhenMoving);
	m_scriptSystem->SetGlobalValue("VOR_FlagNotifyOnSeen", eVOR_FlagNotifyOnSeen);
	m_scriptSystem->SetGlobalValue("VOR_FlagDropOnceInvisible", eVOR_FlagDropOnceInvisible);

	m_scriptSystem->SetGlobalValue("Assignment_NoAssignment", Assignment::NoAssignment);
	m_scriptSystem->SetGlobalValue("Assignment_DefendArea", Assignment::DefendArea);
	m_scriptSystem->SetGlobalValue("Assignment_HoldPosition", Assignment::HoldPosition);
	m_scriptSystem->SetGlobalValue("Assignment_CombatMove", Assignment::CombatMove);
	m_scriptSystem->SetGlobalValue("Assignment_ScanSpot", Assignment::ScanSpot);
	m_scriptSystem->SetGlobalValue("Assignment_ScorchSpot", Assignment::ScorchSpot);
	m_scriptSystem->SetGlobalValue("Assignment_PsychoCombatAllowed", Assignment::PsychoCombatAllowed);
}

i32 CScriptBind_GameAI::RegisterWithModule(IFunctionHandler* pH, tukk moduleName, ScriptHandle entityID)
{
	g_pGame->GetGameAISystem()->EnterModule((EntityId)entityID.n, moduleName);
	return pH->EndFunction();
}

i32 CScriptBind_GameAI::UnregisterWithModule(IFunctionHandler* pH, tukk moduleName, ScriptHandle entityID)
{
	g_pGame->GetGameAISystem()->LeaveModule((EntityId)entityID.n, moduleName);
	return pH->EndFunction();
}

i32 CScriptBind_GameAI::UnregisterWithAllModules(IFunctionHandler* pH, ScriptHandle entityID)
{
	g_pGame->GetGameAISystem()->LeaveAllModules((EntityId)entityID.n);
	return pH->EndFunction();
}

i32 CScriptBind_GameAI::PauseModule(IFunctionHandler* pH, tukk moduleName, ScriptHandle entityID)
{
	g_pGame->GetGameAISystem()->PauseModule((EntityId)entityID.n, moduleName);
	return pH->EndFunction();
}

i32 CScriptBind_GameAI::PauseAllModules(IFunctionHandler* pH, ScriptHandle entityID)
{
	g_pGame->GetGameAISystem()->PauseAllModules((EntityId)entityID.n);
	return pH->EndFunction();
}

i32 CScriptBind_GameAI::ResumeModule(IFunctionHandler* pH, tukk moduleName, ScriptHandle entityID)
{
	g_pGame->GetGameAISystem()->ResumeModule((EntityId)entityID.n, moduleName);
	return pH->EndFunction();
}


i32 CScriptBind_GameAI::ResumeAllModules(IFunctionHandler* pH, ScriptHandle entityID)
{
	g_pGame->GetGameAISystem()->ResumeAllModules(static_cast<EntityId>(entityID.n));
	return pH->EndFunction();
}

bool CScriptBind_GameAI::GetEntitiesInRange(const Vec3& center, float radius, tukk pClassName, SEntityProximityQuery* pQuery) const
{
	DRX_ASSERT(pQuery);

	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;
	IEntityClass* pClass = pEntitySystem->GetClassRegistry()->FindClass(pClassName);
	if (!pClass)
		return false;

	pQuery->box.min = center - Vec3(radius, radius, radius);
	pQuery->box.max = center + Vec3(radius, radius, radius);
	pQuery->pEntityClass = pClass;
	pEntitySystem->QueryProximity(*pQuery);

	return true;
}

i32 CScriptBind_GameAI::GetClosestEntityToTarget(IFunctionHandler* funcHandler, Vec3 attackerPos, Vec3 targetPos, tukk pClassName, float radius, float maxAngle)
{
	SEntityProximityQuery query;

	if (GetEntitiesInRange(targetPos, radius, pClassName, &query))
	{
		i32 closestIndex = -1;
		float closestDistSq = FLT_MAX;
		float cosMaxAngle = cosf(maxAngle);
		Vec3 attackerToTargetDir = (targetPos - attackerPos).GetNormalized();

		for (i32 i = 0; i < query.nCount; ++i)
		{
			IEntity* pEntity = query.pEntities[i];
			if (pEntity)
			{
				float targetToEntityDistSq = (pEntity->GetWorldPos() - targetPos).GetLengthSquared();
				if (targetToEntityDistSq < closestDistSq)
				{
					// Closer than the current, but is the angle valid?
					Vec3 attackerToEntityDir = (pEntity->GetWorldPos() - attackerPos).GetNormalized();
					if (attackerToTargetDir.Dot(attackerToEntityDir) > cosMaxAngle)
					{
						closestIndex = i;
						closestDistSq = targetToEntityDistSq;
					}
				}
			}
		}

		if (closestIndex != -1)
		{
			return funcHandler->EndFunction(query.pEntities[closestIndex]->GetScriptTable());
		}
	}

	return funcHandler->EndFunction();
}

i32 CScriptBind_GameAI::GetBattleFrontPosition(IFunctionHandler* pH, i32 groupID)
{
	if(CAIBattleFrontModule* battleFrontModule = static_cast<CAIBattleFrontModule*>(g_pGame->GetGameAISystem()->FindModule("BattleFront")))
		if(CAIBattleFrontGroup* battleFrontGroup = battleFrontModule->GetGroupByID(groupID))
			return pH->EndFunction(battleFrontGroup->GetBattleFrontPosition());

	return pH->EndFunction();
}

i32 CScriptBind_GameAI::ResetAdvantagePointOccupancyControl(IFunctionHandler* pH)
{
	g_pGame->GetGameAISystem()->GetAdvantagePointOccupancyControl().Reset();
	return pH->EndFunction();
}

i32 CScriptBind_GameAI::OccupyAdvantagePoint(IFunctionHandler* pH, ScriptHandle entityID, Vec3 point)
{
	g_pGame->GetGameAISystem()->GetAdvantagePointOccupancyControl().OccupyAdvantagePoint(static_cast<EntityId>(entityID.n), point);
	return pH->EndFunction();
}

i32 CScriptBind_GameAI::ReleaseAdvantagePointFor(IFunctionHandler* pH, ScriptHandle entityID)
{
	g_pGame->GetGameAISystem()->GetAdvantagePointOccupancyControl().ReleaseAdvantagePoint(static_cast<EntityId>(entityID.n));
	return pH->EndFunction();
}

i32 CScriptBind_GameAI::IsAdvantagePointOccupied(IFunctionHandler* pH, Vec3 point)
{
	g_pGame->GetGameAISystem()->GetAdvantagePointOccupancyControl().IsAdvantagePointOccupied(point);
	return pH->EndFunction();
}

i32 CScriptBind_GameAI::StartSearchModuleFor(IFunctionHandler* pH, i32 groupID, Vec3 targetPos)
{
	ScriptHandle target(0);
	if (pH->GetParamType(3) == svtPointer)
		pH->GetParam(3, target);
	EntityId targetID = static_cast<EntityId>(target.n);

	float searchSpotTimeout(0);
	if (pH->GetParamType(4) == svtNumber)
		pH->GetParam(4, searchSpotTimeout);

	if(!gGameAIEnv.searchModule->GroupExist(groupID))
		gGameAIEnv.searchModule->GroupEnter(groupID, targetPos, targetID, searchSpotTimeout);

	return pH->EndFunction();
}

i32 CScriptBind_GameAI::StopSearchModuleFor(IFunctionHandler* pH, i32 groupID)
{
	if(gGameAIEnv.searchModule->GroupExist(groupID))
		gGameAIEnv.searchModule->GroupLeave(groupID);

	return pH->EndFunction();
}

i32 CScriptBind_GameAI::GetNextSearchSpot(IFunctionHandler* pH,
	ScriptHandle entityID,
	float closenessToAgentWeight,
	float closenessToTargetWeight,
	float minDistanceFromAgent
	)
{
	SearchSpotQuery query;
	query.closenessToAgentWeight = closenessToAgentWeight;
	query.closenessToTargetWeight = closenessToTargetWeight;
	query.minDistanceFromAgent = minDistanceFromAgent;

	if (pH->GetParamType(5) == svtNumber)
		pH->GetParam(5, query.closenessToTargetCurrentPosWeight);
	else
		query.closenessToTargetCurrentPosWeight = 0.0f;

	if (gGameAIEnv.searchModule->GetNextSearchPoint((EntityId)entityID.n, &query))
	{
		return pH->EndFunction(Script::SetCachedVector(query.result, pH, 1));
	}

	return pH->EndFunction();
}

i32 CScriptBind_GameAI::MarkAssignedSearchSpotAsUnreachable(IFunctionHandler* pH, ScriptHandle entityID)
{
	gGameAIEnv.searchModule->MarkAssignedSearchSpotAsUnreachable((EntityId)entityID.n);
	return pH->EndFunction();
}

i32 CScriptBind_GameAI::ResetRanges(IFunctionHandler* pH, ScriptHandle entityID)
{
	if (RangeContainer* rangeContainer = gGameAIEnv.rangeModule->GetRunningInstance((EntityId)entityID.n))
		rangeContainer->ResetRanges();

	return pH->EndFunction();
}

i32 CScriptBind_GameAI::AddRange(IFunctionHandler* pH, ScriptHandle entityID, float range, tukk enterSignal, tukk leaveSignal)
{
	if (RangeContainer* rangeContainer = gGameAIEnv.rangeModule->GetRunningInstance((EntityId)entityID.n))
	{
		RangeContainer::Range r;
		r.enterSignal = enterSignal;
		r.leaveSignal = leaveSignal;
		r.rangeSq = square(range);

		if (pH->GetParamCount() >= 5)
		{
			i32 targetMode = 0;
			if (pH->GetParam(5, targetMode))
			{
				r.targetMode = static_cast<RangeContainer::Range::TargetMode>(targetMode);
			}
		}

		RangeContainer::RangeID rangeID = rangeContainer->AddRange(r);
		return pH->EndFunction(rangeID);
	}

	return pH->EndFunction();
}

i32 CScriptBind_GameAI::ResetAloneDetector(IFunctionHandler* pH, ScriptHandle entityID)
{
	if(AloneDetectorContainer* aloneDetectorContainer = gGameAIEnv.aloneDetectorModule->GetRunningInstance(static_cast<EntityId>(entityID.n)))
		aloneDetectorContainer->ResetDetector();

	return pH->EndFunction();
}

i32 CScriptBind_GameAI::SetupAloneDetector(IFunctionHandler* pH, ScriptHandle entityID, float range, tukk aloneSignal, tukk notAloneSignal)
{
	if(AloneDetectorContainer* aloneDetectorContainer = gGameAIEnv.aloneDetectorModule->GetRunningInstance(static_cast<EntityId>(entityID.n)))
		aloneDetectorContainer->SetupDetector(AloneDetectorContainer::AloneDetectorSetup(range,aloneSignal,notAloneSignal));

	return pH->EndFunction();
}

i32 CScriptBind_GameAI::AddActorClassToAloneDetector(IFunctionHandler* pH, ScriptHandle entityID, tukk entityClassName)
{
	if(AloneDetectorContainer* aloneDetectorContainer = gGameAIEnv.aloneDetectorModule->GetRunningInstance(static_cast<EntityId>(entityID.n)))
		aloneDetectorContainer->AddEntityClass(entityClassName);

	return pH->EndFunction();
}

i32 CScriptBind_GameAI::RemoveActorClassFromAloneDetector(IFunctionHandler* pH, ScriptHandle entityID, tukk entityClassName)
{
	if(AloneDetectorContainer* aloneDetectorContainer = gGameAIEnv.aloneDetectorModule->GetRunningInstance(static_cast<EntityId>(entityID.n)))
		aloneDetectorContainer->RemoveEntityClass(entityClassName);

	return pH->EndFunction();
}

i32 CScriptBind_GameAI::IsAloneForAloneDetector(IFunctionHandler* pH, ScriptHandle entityID)
{
	if(AloneDetectorContainer* aloneDetectorContainer = gGameAIEnv.aloneDetectorModule->GetRunningInstance(static_cast<EntityId>(entityID.n)))
		return pH->EndFunction(aloneDetectorContainer->IsAlone());

	return pH->EndFunction();
}

i32 CScriptBind_GameAI::GetRangeState(IFunctionHandler* pH, ScriptHandle entityID, i32 rangeID)
{
	if (RangeContainer* rangeContainer = gGameAIEnv.rangeModule->GetRunningInstance((EntityId)entityID.n))
	{
		if (const RangeContainer::Range* range = rangeContainer->GetRange((RangeContainer::RangeID)rangeID))
			return pH->EndFunction(range->state);
	}

	return pH->EndFunction();	
}

i32 CScriptBind_GameAI::ChangeRange(IFunctionHandler* pH, ScriptHandle entityID, i32 rangeID, float distance)
{
	if (RangeContainer* rangeContainer = gGameAIEnv.rangeModule->GetRunningInstance((EntityId)entityID.n))
	{
		rangeContainer->ChangeRange((RangeContainer::RangeID)rangeID, distance);
	}

	return pH->EndFunction();
}

i32 CScriptBind_GameAI::RegisterObjectVisible(IFunctionHandler *pH, ScriptHandle entityID)
{
	i32k paramCount = pH->GetParamCount();

	u32 visibleObjectRule = eVOR_Default;
	i32 factionId = 0;
	if (paramCount > 1)
	{
		pH->GetParam(2, visibleObjectRule);

		if (paramCount > 2)
		{
			pH->GetParam(3, factionId);
		}
	}

	const bool bRegistered = g_pGame->GetGameAISystem()->GetVisibleObjectsHelper().RegisterObject((EntityId)entityID.n, factionId, visibleObjectRule);
	return pH->EndFunction(bRegistered);
}

i32 CScriptBind_GameAI::UnregisterObjectVisible(IFunctionHandler *pH, ScriptHandle entityID)
{
	const bool bUnregistered = g_pGame->GetGameAISystem()->GetVisibleObjectsHelper().UnregisterObject((EntityId)entityID.n);
	return pH->EndFunction(bUnregistered);
}

i32 CScriptBind_GameAI::IsAISystemEnabled(IFunctionHandler *pH)
{
	bool enabled = gEnv->pAISystem && gEnv->pAISystem->IsEnabled();
	return pH->EndFunction(enabled);
}

/* 
	Squad Upr	
*/
i32 CScriptBind_GameAI::RegisterEntityForAISquadUpr(IFunctionHandler* pH, ScriptHandle entityID)
{
	EntityId entID = static_cast<EntityId>(entityID.n);
	if(entID)
	{
		g_pGame->GetGameAISystem()->GetAISquadUpr().RegisterAgent(entID);
	}

	return pH->EndFunction();
}

i32 CScriptBind_GameAI::RemoveEntityForAISquadUpr(IFunctionHandler* pH, ScriptHandle entityID)
{
	EntityId entID = static_cast<EntityId>(entityID.n);
	if(entID)
	{
		g_pGame->GetGameAISystem()->GetAISquadUpr().UnregisterAgent(entID);
	}

	return pH->EndFunction();
}

i32 CScriptBind_GameAI::GetSquadId(IFunctionHandler* pH, ScriptHandle entityId)
{
	AISquadUpr& squadUpr = g_pGame->GetGameAISystem()->GetAISquadUpr();

	const SquadId squadId = squadUpr.GetSquadIdForAgent(static_cast<SquadId>(entityId.n));

	if (squadId!=UnknownSquadId)
		return pH->EndFunction(squadId);
	else
		return pH->EndFunction(); // lua expects a nil for invalid squad value
}

i32 CScriptBind_GameAI::GetSquadMembers(IFunctionHandler* pH, i32 squadId)
{
	// Query the squad manager for the entity id's of all members of the squad

	AISquadUpr& squadUpr = g_pGame->GetGameAISystem()->GetAISquadUpr();
	AISquadUpr::MemberIDArray memberIDs;
	squadUpr.GetSquadMembers(squadId, memberIDs);

	// Go through all members and construct a table where
	//   key = entity ID
	//   value = entity script table

	SmartScriptTable memberTable(m_pSS);

	AISquadUpr::MemberIDArray::const_iterator it = memberIDs.begin();
	AISquadUpr::MemberIDArray::const_iterator end = memberIDs.end();

	for (; it != end; ++it)
	{
		const EntityId id = *it;

		if (IEntity* entity = gEnv->pEntitySystem->GetEntity(id))
		{
			memberTable->SetAt(id, entity->GetScriptTable());
		}
	}

	return pH->EndFunction(memberTable);
}

i32 CScriptBind_GameAI::GetAveragePositionOfSquadScopeUsers(IFunctionHandler* pH, ScriptHandle entityIdHandle, tukk squadScopeName)
{
	AISquadUpr& squadUpr = g_pGame->GetGameAISystem()->GetAISquadUpr();

	const EntityId entityId = static_cast<EntityId>(entityIdHandle.n);
	const SquadId squadId = squadUpr.GetSquadIdForAgent(entityId);
	const SquadScopeID squadScopeID(squadScopeName);

	AISquadUpr::MemberIDArray memberIDs;
	squadUpr.GetSquadMembersInScope(squadId, squadScopeID, memberIDs);

	// Calculate the average position of all the members
	AISquadUpr::MemberIDArray::const_iterator it = memberIDs.begin();
	AISquadUpr::MemberIDArray::const_iterator end = memberIDs.end();

	Vec3 averagePosition(ZERO);
	float count = 0.0f;

	for (; it != end; ++it)
	{
		const EntityId id = *it;

		if (IEntity* entity = gEnv->pEntitySystem->GetEntity(id))
		{
			averagePosition += entity->GetWorldPos();
			count += 1.0f;
		}
	}

	if (count > 0.0f)
		averagePosition /= count;
	else
		averagePosition.zero();

	return pH->EndFunction(averagePosition);
}

i32 CScriptBind_GameAI::GetSquadScopeUserCount(IFunctionHandler* pH, ScriptHandle entityIdHandle, tukk squadScopeName)
{
	AISquadUpr& squadUpr = g_pGame->GetGameAISystem()->GetAISquadUpr();

	const EntityId entityId = static_cast<EntityId>(entityIdHandle.n);
	const SquadId squadId = squadUpr.GetSquadIdForAgent(entityId);
	const SquadScopeID squadScopeID(squadScopeName);

	AISquadUpr::MemberIDArray memberIDs;
	squadUpr.GetSquadMembersInScope(squadId, squadScopeID, memberIDs);

	return pH->EndFunction(static_cast<u32>(memberIDs.size()));
}
 // ============================================================================
 //	Query if an actor entity is swimming underwater.
 //
 //	In:		The function handler (NULL is invalid!)
 //	In:		The script handle of the target entity.
 //
 //	Returns:	A default result code (in Lua: true if swimming underwater; 
 //	            otherwise false).
 //
 i32 CScriptBind_GameAI::IsSwimmingUnderwater(IFunctionHandler* pH, ScriptHandle entityID)
 {	
	 bool swimmingUnderwaterFlag = false;

	 CActor *pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor((EntityId)entityID.n));
	 if (pActor != NULL)
	 {
		 swimmingUnderwaterFlag = (pActor->IsSwimming() && pActor->IsHeadUnderWater());
	 }

	 return pH->EndFunction(swimmingUnderwaterFlag);
 }
