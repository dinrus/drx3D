// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   AIGroup.h
   $Id$
   Описание:

   -------------------------------------------------------------------------
   История:
   - ?
   - 2 Mar 2009	: Evgeny Adamenkov: Removed IRenderer

 *********************************************************************/
#ifndef _AIGROUP_H_
#define _AIGROUP_H_

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/AI/IAISystem.h>
#include <drx3D/AI/UnitImg.h>
#include <drx3D/AI/Leader.h>
#include <drx3D/AI/IAgent.h>
#include <drx3D/AI/IAIGroup.h>
#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/AI/Adapters.h>

struct IAIGroupTactic;
class CLeader;
typedef u32 GroupScopeID;

//====================================================================
// CAIGroup
//====================================================================
class CAIGroup
	: public CAIGroupAdapter
{
public:

	typedef std::vector<CWeakRef<const CAIObject>> TSetAIObjects;

	CAIGroup(i32 groupID);
	~CAIGroup();

	// Inherited IAIGroup
	virtual i32  GetGroupId() { return m_groupID; }
	virtual i32  GetGroupCount(i32 flags);
	virtual void NotifyReinfDone(const IAIObject* obj, bool isDone);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// TEAM MANAGEMENT FUNCTIONS
	////////////////////////////////////////////////////////////////////////////////////////////////

	void     SetLeader(CLeader* pLeader);
	CLeader* GetLeader();
	//CLeader*		MakeMeLeader(IAIObject* pAIObject);

	// <Title AddMember>
	// Описание: Add a new AIObject to the group
	// Arguments:
	//	CAIObject* pMember - new AIObject to add
	void AddMember(CAIActor* pMember);

	// <Title RemoveMember>
	// Описание: Removes a member from the group
	// Arguments:
	//	CAIObject* pMember - AIObject to remove ; no members will be removed if pMember is not in the team
	bool RemoveMember(CAIActor* pMember);

	// <Title IsMember>
	// Описание: returns true if the given AIObject is in the group
	// Arguments:
	// CAIObject* pMember - AIObject to retrieve
	bool IsMember(const IAIObject* pMember) const;

	// <Title GetUnits>
	// Описание: returns the unit (team members) list container
	TUnitList& GetUnits() { return m_Units; }

	// <Title GetUnitImg>
	// Описание: returns the unit image of the given AIObject, if it's found in the team
	// Arguments:
	//	CAIObject* pAIObject - AIObject to retrieve
	TUnitList::iterator GetUnit(const CAIActor* pAIObject);

	// <Title SetUnitProperties>
	// Описание: set the given unit's properties (UPR_*)
	// See definition of EUnitProperties in IAgent.h for a complete list of unit properties
	// Arguments:
	//	IAIObject* pMember - AIObject to set properties (if it's found in the team)
	//	u32 unitPropMask - binary mask of properties (i.e. UPR_COMBAT_GROUND|UPR_COMBAT_RECON)
	void SetUnitProperties(const IAIObject* pMember, u32 unitPropMask);

	// <Title GetUnitCount>
	// Описание: returns the number of units which have any of the specified properties
	// Arguments:
	//	u32 unitPropMask - binary mask of properties (i.e. UPR_COMBAT_GROUND|UPR_COMBAT_RECON)
	//			only the unit with any of the specified properties will be considered
	//		(default = UPR_ALL : returns the total number of units)
	i32 GetUnitCount(u32 unitPropMask = UPR_ALL) const;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// POSITIONING/TACTICAL FUNCTIONS
	////////////////////////////////////////////////////////////////////////////////////////////////

	// <Title GetAttentionTarget>
	// Описание: return the IAIObject* most threatening attention target among all the units
	// Arguments:
	//	(optional) bool bHostileOnly - if true, only hostile targets will be considered (default = false)
	//	(optional) bool bLiveOnly - if true, only live targets (=no dummy) will be considered (default = false)

	// Suppressed passedByValue warnings for smart pointers like CWeakRef<>
	// cppcheck-suppress passedByValue
	CWeakRef<CAIObject> GetAttentionTarget(bool bHostileOnly = false, bool bLiveOnly = false, const CWeakRef<CAIObject> refSkipTarget = NILREF) const;

	// <Title GetTargetCount>
	// Описание: return the number of targets amongst all units
	// Arguments:
	//	(optional) bool bHostileOnly - if true, only hostile targets will be considered (default = false)
	//	(optional) bool bLiveOnly - if true, only live targets (=no dummy) will be considered (default = false)
	i32 GetTargetCount(bool bHostileOnly = false, bool bLiveOnly = false) const;

	// <Title GetUnits>
	// Описание: returns the unit (team members) list container
	TSetAIObjects& GetTargets() { return m_Targets; }

	// <Title GetAveragePosition>
	// Описание: return average position of the units in the team
	// Arguments:
	//	(optional) eAvPositionMode mode - selects the unit variable to be checked for selection:
	//				AVMODE_ANY - all unit will be processed
	//				AVMODE_PROPERTIES - unit properties will be used for filter
	//				AVMODE_CLASS - unit class will be used for filter
	//	(optional) u32 unitFilter - filter value (UPR_* if properties, UNIT_CLASS_* if class)
	Vec3 GetAveragePosition(eAvPositionMode mode = AVMODE_ANY, u32 unitClassOrProperties = -1) const;

	// <Title GetForemostUnit>
	// gets the foremost unit in the given direction. starting from the average position
	// Arguments:
	//	Vec3 direction - direction to use
	//	u32 unitProp = binary selection mask for unit properties (i.e. UPR_COMBAT_GROUND|UPR_COMBAT_RECON)
	//		only the unit with any of the specified properties will be considered
	CAIObject* GetForemostUnit(const Vec3& direction, u32 unitProp) const;

	// <Title GetEnemyAveragePosition>
	// Описание: return the average position of all the known enemy targets
	inline const Vec3& GetEnemyAveragePosition() const { return m_vAverageEnemyPosition; }

	// <Title GetLiveEnemyAveragePosition>
	// Описание: return the average position of all the known enemy live targets
	inline const Vec3& GetLiveEnemyAveragePosition() const { return m_vAverageLiveEnemyPosition; }

	// <Title GetEnemyAverageDirection>
	// Описание: return the average direction of all the known enemy targets
	// Arguments:
	//	 bool bLiveOnly - if true, only live targets (=no dummy) will be considered
	//	 bool bSmart - if true, average direction will be set to ZERO if the enemy targets' directions are too different from
	//						each other (high variance)
	Vec3 GetEnemyAverageDirection(bool bLiveOnly, bool bSmart) const;

	// <Title GetForemostEnemy>
	// Описание: return the enemy which is foremost in the given direction, starting from enemy average position
	// Arguments:
	//	 Vec3 direction - direction to use
	//	 bool bLiveOnly - if true, only live (non dummy) targets are considered
	const CAIObject* GetForemostEnemy(const Vec3& direction, bool bLiveOnly = false) const;

	// <Title GetForemostEnemyPosition>
	// Описание: return the position of the enemy which is foremost in the given direction, starting from enemy average position
	// Arguments:
	//	 Vec3 direction - direction to use
	//	 bool bLiveOnly - if true, only live (non dummy) targets are considered
	Vec3 GetForemostEnemyPosition(const Vec3& direction, bool bLiveOnly = false) const;

	// <Title GetEnemyPositionKnown>
	// Описание: returns the attention target position of a unit (if specified in the parameter),
	//	or - if unit is not specified - .. Dejan? (not clear what's happening with the distance)
	Vec3 GetEnemyPosition(const CAIObject* pUnit = NULL) const;

	// <Title GetEnemyPositionKnown>
	// Описание: returns an alive, hostile attention target amongst the units' - if there is - or the beacon if there's not
	//	(note: seems replaceable by either GetEnemyLiveAveragePosition() or GetForemostEnemyPosition())
	Vec3 GetEnemyPositionKnown() const;

	// Returns true if the scope could have been entered (If no group member already entered it),
	// false otherwise.
	// If the memberID doesn't belong to this group then false is returned
	bool EnterGroupScope(const CAIActor* pMember, const GroupScopeID scopeID, u32k allowedConcurrentUsers);
	// The group scope is released by the memberID. If the memberID is not in the scope
	// the function doesn't do anything.
	void                LeaveGroupScope(const CAIActor* pMember, const GroupScopeID scopeID);
	void                LeaveAllGroupScopesForActor(const CAIActor* pMember);
	u32              GetAmountOfActorsInScope(const GroupScopeID scopeID) const;
	static GroupScopeID GetGroupScopeId(tukk scopeName);

	void                NotifyBodyChecked(const CAIObject* pObject);
	void                OnObjectRemoved(CAIObject* pObject);
	void                UpdateGroupCountStatus();
	void                Update();
	void                Reset();
	void                Serialize(TSerialize ser);
	void                DebugDraw();

private:

	void UpdateReinforcementLogic();

	void SetUnitClass(CUnitImg& unit) const;

	i32           m_groupID;          // the group id.
	i32           m_count;            // Current group count.
	i32           m_countMax;         // Max number of entities in the group.
	i32           m_countEnabled;     // Current enabled group count.
	i32           m_countEnabledMax;  // Max number of enabled entities in the group.
	i32           m_countDead;        // Number of dead entities.
	i32           m_countCheckedDead; // Number of dead bodies seen.

	TSetAIObjects m_Targets;
	Vec3          m_vEnemyPositionKnown;
	Vec3          m_vAverageEnemyPosition;
	Vec3          m_vAverageLiveEnemyPosition;
	bool          m_bUpdateEnemyStats;

	TUnitList     m_Units;
	CLeader*      m_pLeader;

	typedef std::set<tAIObjectID>                  MembersInScope;
	typedef std::map<GroupScopeID, MembersInScope> GroupScopes;
	GroupScopes m_groupScopes;

	enum EReinforcementState
	{
		REINF_NONE,
		//		REINF_ALERTED_COMBAT_PENDING,
		//		REINF_ALERTED_COMBAT,
		REINF_DONE_PENDING,
		REINF_DONE,
		LAST_REINF,
	};

	struct SAIReinforcementSpot
	{
		CAIObject* pAI;
		i32        type;
		bool       whenAllAlerted;
		bool       whenInCombat;
		i32        groupBodyCount;
		float      avoidWhenTargetInRadius;
	};

	struct SAIRefinforcementCallDebug
	{
		SAIRefinforcementCallDebug(const Vec3& from, const Vec3& to, float t, tukk perf) :
			from(from), to(to), t(t), performer(perf) {}
		Vec3   from, to;
		string performer;
		float  t;
	};

	i32                               m_reinforcementSpotsAllAlerted;
	i32                               m_reinforcementSpotsCombat;
	i32                               m_reinforcementSpotsBodyCount;

	bool                              m_pendingDecSpotsAllAlerted;
	bool                              m_pendingDecSpotsCombat;
	bool                              m_pendingDecSpotsBodyCount;

	CWeakRef<CPuppet>                 m_refReinfPendingUnit;

	std::vector<SAIReinforcementSpot> m_reinforcementSpots;

#ifdef DRXAISYS_DEBUG
	std::vector<SAIRefinforcementCallDebug> m_DEBUG_reinforcementCalls;
#endif

	bool                m_reinforcementSpotsAcquired;
	EReinforcementState m_reinforcementState;
};

#endif
