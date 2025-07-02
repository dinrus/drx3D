// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   Leader.h
   $Id$
   Описание: Header for the CLeader class

   -------------------------------------------------------------------------
   История:
   Created by Kirill Bulatsev
   - 01:06:2005   : serialization support added, related refactoring, moved out to separate files ObstacleAllocator, etc


 *********************************************************************/

#ifndef __Leader_H__
#define __Leader_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/AI/AIActor.h>
#include <drx3D/AI/LeaderAction.h>
#include <drx3D/Entity/IEntity.h>
#include <list>
#include <map>
#include <drx3D/AI/ObjectContainer.h>

class CAIGroup;
class CLeader;
class CPipeUser;
class CLeaderAction;
struct LeaderActionParams;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct TTarget
{
	Vec3       position;
	CAIObject* object;
	TTarget(Vec3& spot, CAIObject* pObject) : position(spot), object(pObject) {};
};

class CLeader :
	public CAIActor//,public ILeader
{
	//	class CUnitAction;

	friend class CLeaderAction;

public:

	typedef std::map<CUnitImg*, Vec3>               CUnitPointMap;
	typedef std::vector<CLeader*>                   TVecLeaders;
	typedef std::vector<CAIObject*>                 TVecTargets;
	typedef std::list<i32>                          TUsableObjectList;
	typedef std::multimap<i32, TUnitList::iterator> TClassUnitMap;
	typedef std::map<INT_PTR, Vec3>                 TSpotList;

	CLeader(i32 iGroupID);
	~CLeader(void);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// SYSTEM/AI SYSTEM RELATED FUNCTIONS
	////////////////////////////////////////////////////////////////////////////////////////////////

	void         Update(EUpdateType type);
	void         Reset(EObjectResetType type);
	void         OnObjectRemoved(CAIObject* pObject);
	void         Serialize(TSerialize ser);

	void         SetAIGroup(CAIGroup* pGroup) { m_pGroup = pGroup; }
	CAIGroup*    GetAIGroup()                 { return m_pGroup; }
	virtual void SetAssociation(CWeakRef<CAIObject> refAssociation);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// CLEADER PROPERTIES FUNCTION
	////////////////////////////////////////////////////////////////////////////////////////////////

	// <Title IsPlayer>
	// Описание: returns true if this is associated to the player AI object
	bool IsPlayer() const;

	// <Title IsIdle>
	// Описание: returns true if there's no LeaderAction active
	bool         IsIdle() const;

	virtual bool IsActive() const { return false; }
	virtual bool IsAgent() const  { return false; }

	////////////////////////////////////////////////////////////////////////////////////////////////
	// TEAM MANAGEMENT FUNCTIONS
	////////////////////////////////////////////////////////////////////////////////////////////////

	// <Title GetActiveUnitPlanCount>
	// Описание: return the number of units with at least one of the specified properties, which have an active plan (i.e. not idle)
	// Arguments:
	//	u32 unitPropMask - binary mask of properties (i.e. UPR_COMBAT_GROUND|UPR_COMBAT_RECON)
	//		only the unit with any of the specified properties will be considered
	//		(default = UPR_ALL : all units are checked)
	i32 GetActiveUnitPlanCount(u32 unitProp = UPR_ALL) const;

	// <Title DeadUnitNotify>
	// Описание: to be called when a unit in the group dies
	// Arguments:
	// CAIObject* pObj = Dying AIObject
	void DeadUnitNotify(CAIActor* pAIObj);

	// <Title AddUnitNotify>
	// Описание: to be called when an AI is added to the leader's group
	// Arguments:
	// CAIObject* pObj = new AIObject joining group
	inline void AddUnitNotify(CAIActor* pAIObj)
	{
		if (m_pCurrentAction)
			m_pCurrentAction->AddUnitNotify(pAIObj);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// POSITIONING/TACTICAL FUNCTIONS
	////////////////////////////////////////////////////////////////////////////////////////////////

	// <Title GetPreferedPos>
	// Описание: Returns the preferred start position for finding obstacles (Dejan?)
	Vec3 GetPreferedPos() const;

	// <Title ForcePreferedPos>
	// Описание:
	inline void ForcePreferedPos(const Vec3& pos) { m_vForcedPreferredPos = pos; };

	// <Title MarkPosition>
	// Описание: marks a given point to be retrieved later
	// Arguments:
	//	Vec3& pos - point to be marked
	inline void MarkPosition(const Vec3& pos) { m_vMarkedPosition = pos; }

	// <Title GetMarkedPosition>
	// Описание: return the previously marked point
	inline const Vec3& GetMarkedPosition() const { return m_vMarkedPosition; }

	// <Title AssignTargetToUnits>
	// Описание: redistribute known attention targets amongst the units with the specified properties
	// Arguments:
	//	u32 unitProp = binary selection mask for unit properties (i.e. UPR_COMBAT_GROUND|UPR_COMBAT_RECON)
	//		only the unit with any of the specified properties will be considered
	//	i32 maxUnitsPerTarget = specifies the maximum number of units that can have the same attention target
	void AssignTargetToUnits(u32 unitProp, i32 maxUnitsPerTarget);

	// <Title GetEnemyLeader>
	// Описание: returns the leader puppet (CAIOject*) of the current Leader's attention target if there is
	//	(see CLeader::GetAttentionTarget()
	CAIObject* GetEnemyLeader() const;

	// <Title ShootSpotsAvailable>
	// Описание: returns true if there are active shoot spots available
	inline bool ShootSpotAvailable() { return false; }

	////////////////////////////////////////////////////////////////////////////////////////////////
	// ACTION RELATED FUNCTIONS
	////////////////////////////////////////////////////////////////////////////////////////////////

	// <Title Attack>
	// Описание: starts an attack action (stopping the current one)
	// Arguments:
	//  LeaderActionParams* pParams: leader action parameters - used depending on the action subtype (default: NULL -> default attack)
	//	pParams->subType - leader Action subtype (LAS_ATTACK_FRONT,LAS_ATTACK_ROW etc) see definition of ELeaderActionSubType
	//  pParams->fDuration - maximum duration of the attack action
	//  pParams->unitProperties -  binary selection mask for unit properties (i.e. UPR_COMBAT_GROUND|UPR_COMBAT_RECON)
	//		only the unit with any of the specified properties will be selected for the action
	void Attack(const LeaderActionParams* pParams = NULL);

	// <Title Search>
	// Описание: starts a search action (stopping the current one)
	// Arguments:
	//	const Vec3& targetPos - last known target position reference
	//	float distance - maximum distance to search (not working yet - it's hardcoded in AI System)
	//	u32 unitProperties = binary selection mask for unit properties (i.e. UPR_COMBAT_GROUND|UPR_COMBAT_RECON)
	//		only the unit with any of the specified properties will be selected for the action
	void Search(const Vec3& targetPos, float distance, u32 unitProperties, i32 searchSpotAIObjectType);

	// <Title AbortExecution>
	// Описание: clear all units' actions and cancel (delete) the current LeaderAction
	// Arguments:
	//	i32 priority (default 0) max priority value for the unit actions to be canceled (0 = all priorities)
	//		if an unit action was created with a priority value higher than this value, it won't be canceled
	void AbortExecution(i32 priority = 0);

	// <Title ClearAllPlannings>
	// Описание: clear all actions of all the unit with specified unit properties
	// Arguments:
	//	i32 priority (default 0) max priority value for the unit actions to be canceled (0 = all priorities)
	//		if an unit action was created with a priority value higher than this value, it won't be canceled
	//	u32 unitProp = binary selection mask for unit properties (i.e. UPR_COMBAT_GROUND|UPR_COMBAT_RECON)
	//		only the unit with any of the specified properties will be considered
	void ClearAllPlannings(u32 unitProp = UPR_ALL, i32 priority = 0);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// FORMATION RELATED FUNCTIONS
	////////////////////////////////////////////////////////////////////////////////////////////////

	bool                       LeaderCreateFormation(tukk szFormationName, const Vec3& vTargetPos, bool bIncludeLeader = true, u32 unitProp = UPR_ALL, CAIObject* pOwner = NULL);
	bool                       ReleaseFormation();

	CWeakRef<CAIObject>        GetNewFormationPoint(CWeakRef<CAIObject> refRequester, i32 iPointType = 0);
	CWeakRef<CAIObject>        GetFormationOwner(CWeakRef<CAIObject> refRequester) const { return m_refFormationOwner;  }
	CWeakRef<CAIObject>        GetFormationPoint(CWeakRef<CAIObject> refRequester) const;

	i32                        GetFormationPointIndex(const CAIObject* pAIObject) const;
	CWeakRef<CAIObject>        GetFormationPointSight(const CPipeUser* pRequester) const;
	void                       FreeFormationPoint(CWeakRef<CAIObject> refRequester) const;

	inline CWeakRef<CAIObject> GetFormationOwner() const      { return m_refFormationOwner; }
	inline CWeakRef<CAIObject> GetPrevFormationOwner() const  { return m_refPrevFormationOwner; }

	inline tukk         GetFormationDescriptor() const { return m_szFormationDescriptor.c_str(); }
	i32                        AssignFormationPoints(bool bIncludeLeader = true, u32 unitProp = UPR_ALL);
	bool                       AssignFormationPoint(CUnitImg& unit, i32 iTeamSize = 0, bool bAnyClass = false, bool bClosestToOwner = false);
	void                       AssignFormationPointIndex(CUnitImg& unit, i32 index);

	// Called by CAIGroup.
	void OnEnemyStatsUpdated(const Vec3& avgEnemyPos, const Vec3& oldAvgEnemyPos);

protected:
	void           UpdateEnemyStats();
	void           ProcessSignal(AISIGNAL& signal);
	CLeaderAction* CreateAction(const LeaderActionParams* params, tukk signalText = NULL);

private:
	void ChangeStance(EStance stance);

protected:
	CLeaderAction*      m_pCurrentAction;

	CWeakRef<CAIObject> m_refFormationOwner;
	CWeakRef<CAIObject> m_refPrevFormationOwner;
	CWeakRef<CAIObject> m_refEnemyTarget;

	string              m_szFormationDescriptor;
	Vec3                m_vForcedPreferredPos;
	Vec3                m_vMarkedPosition; //general purpose memory of a position - used in CLeaderAction_Search, as
	// last average units' position when there was a live target
	bool                m_bLeaderAlive;
	bool                m_bKeepEnabled; // if true, CLeader::Update() will be executed even if the leader puppet is dead

	CAIGroup*           m_pGroup;
};

inline const CLeader* CastToCLeaderSafe(const IAIObject* pAI) { return pAI ? pAI->CastToCLeader() : 0; }
inline CLeader*       CastToCLeaderSafe(IAIObject* pAI)       { return pAI ? pAI->CastToCLeader() : 0; }

#endif // __Leader_H__
