// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   IAIGroup.h
   $Id$
   Описание:

   -------------------------------------------------------------------------
   История:
   - 2 Mar 2009			 : Evgeny Adamenkov: Removed IRenderer

 *********************************************************************/

#pragma once

#include <drx3D/AI/IAISystem.h> // <> required for Interfuscator
#include <drx3D/Network/SerializeFwd.h>

#ifdef max
	#undef max
#endif

struct IAIGroup
{
	enum eAvPositionMode
	{
		AVMODE_ANY,
		AVMODE_PROPERTIES,
		AVMODE_CLASS
	};

	// <interfuscator:shuffle>
	virtual ~IAIGroup() {}

	//! \return the ID of the group.
	virtual i32 GetGroupId() = 0;

	//! \return the number of units in the group, see IAISystem::EGroupFlags.
	virtual i32 GetGroupCount(i32 flags) = 0;

	//! \return true if the given AIObject is in the group.
	virtual bool IsMember(const IAIObject* pAI) const = 0;

	//! Sets specified unit's properties, see EUnitProperties.
	virtual void       SetUnitProperties(const IAIObject* obj, u32 properties) = 0;
	virtual IAIObject* GetAttentionTarget(bool bHostileOnly, bool bLiveOnly) const = 0;
	virtual Vec3       GetAveragePosition(eAvPositionMode mode = AVMODE_ANY, u32 unitClassOrProperties = -1) const = 0;
	virtual i32        GetUnitCount(u32 unitPropMask = UPR_ALL) const = 0;

	//! Gets the number of attention targets in the group.
	virtual i32 GetTargetCount(bool bHostileOnly, bool bLiveOnly) const = 0;

	//! triggers reinforcements state.
	virtual void NotifyReinfDone(const IAIObject* obj, bool isDone) = 0;
	// </interfuscator:shuffle>
};

//! Query types for AIGroupTactic.GetState.
//! The exact meaning is described along side with each group tactic implementation.
enum EGroupStateType
{
	GE_GROUP_STATE,
	GE_UNIT_STATE,
	GE_ADVANCE_POS,
	GE_SEEK_POS,
	GE_DEFEND_POS,
	GE_LEADER_COUNT,
	GE_MOST_LOST_UNIT,
	GE_MOVEMENT_SIGNAL,
	GE_NEAREST_SEEK,
};

//! Notify types for AIGroupTactic.Notify.
//! The exact meaning is described along side with each group tactic implementation.
enum EGroupNotifyType
{
	GN_INIT,
	GN_MARK_DEFEND_POS,
	GN_CLEAR_DEFEND_POS,
	GN_AVOID_CURRENT_POS,
	GN_PREFER_ATTACK,
	GN_PREFER_FLEE,
	GN_NOTIFY_ADVANCING,
	GN_NOTIFY_COVERING,
	GN_NOTIFY_WEAK_COVERING,
	GN_NOTIFY_HIDING,
	GN_NOTIFY_SEEKING,
	GN_NOTIFY_ALERTED,
	GN_NOTIFY_UNAVAIL,
	GN_NOTIFY_IDLE,
	GN_NOTIFY_SEARCHING,
	GN_NOTIFY_REINFORCE,
};

//! State describing the over group state.
enum EGroupState
{
	GS_IDLE,
	GS_ALERTED,
	GS_COVER,
	GS_ADVANCE,
	GS_SEEK,
	GS_SEARCH,
	GS_REINFORCE,
	LAST_GS // Must be last.
};

//! List of possible unit types to register with GN_INIT.
enum EGroupUnitType
{
	GU_HUMAN_CAMPER,
	GU_HUMAN_COVER,
	GU_HUMAN_SNEAKER,
	GU_HUMAN_LEADER,
	GU_HUMAN_SNEAKER_SPECOP,
	GU_ALIEN_MELEE,
	GU_ALIEN_ASSAULT,
	GU_ALIEN_MELEE_DEFEND,
	GU_ALIEN_ASSAULT_DEFEND,
	GU_ALIEN_EVADE,
};

struct IAIGroupTactic
{
	// <interfuscator:shuffle>
	virtual ~IAIGroupTactic() {};

	//! Updates the group tactic.
	virtual void Update(float dt) = 0;

	//! Called when new member is added to group.
	virtual void OnMemberAdded(IAIObject* pMember) = 0;

	//! Called when new member is removed from the group.
	virtual void OnMemberRemoved(IAIObject* pMember) = 0;

	//! Callback called when an attention target of a unit belonging to the group is changed.
	virtual void OnUnitAttentionTargetChanged() = 0;

	//! Called on AIsystem reset, should reset the state of the tactic.
	virtual void Reset() = 0;

	//! Notify or report tactic, see EGroupNotifyType.
	virtual void NotifyState(IAIObject* pRequester, i32 type, float param) = 0;

	//! Returns queried state related to the tactic, see EGroupStateType.
	virtual i32 GetState(IAIObject* pRequester, i32 type, float param) = 0;

	//! Returns queried point related to the tactic, see EGroupStateType.
	virtual Vec3 GetPoint(IAIObject* pRequester, i32 type, float param) = 0;

	//! Serialize the state of the tactic.
	virtual void Serialize(TSerialize ser) = 0;

	//! Draw debug information about the group tactic. See console var ai_DrawGroupTactic.
	virtual void DebugDraw() = 0;
	// </interfuscator:shuffle>

	void GetMemoryUsage(IDrxSizer* pSizer) const { /*LATER*/ }
};
