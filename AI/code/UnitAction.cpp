// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   UnitAction.cpp
   $Id$
   Описание: Refactoring CLeadeActions; adding Serialization support

   -------------------------------------------------------------------------
   История:
   - 2:6:2005   15:23 : Created by Kirill Bulatsev

 *********************************************************************/

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/UnitAction.h>
#include <drx3D/AI/AISignal.h>
#include <drx3D/Network/ISerialize.h>

//
//----------------------------------------------------------------------------------------------------
// cppcheck-suppress uninitMemberVar
CUnitAction::CUnitAction() :
	m_Action(UA_NONE)
{
}

//
//----------------------------------------------------------------------------------------------------
CUnitAction::CUnitAction(EUnitAction eAction, bool bBlocking) :
	m_Action(eAction),
	m_BlockingPlan(bBlocking),
	m_Priority(PRIORITY_NORMAL),
	m_fDistance(1),
	m_Tag(0)
{
}

//
//----------------------------------------------------------------------------------------------------
CUnitAction::CUnitAction(EUnitAction eAction, bool bBlocking, const Vec3& point) :
	m_Action(eAction),
	m_BlockingPlan(bBlocking),
	m_Point(point),
	m_Priority(PRIORITY_NORMAL),
	m_fDistance(1),
	m_Tag(0)
{
}
//
//----------------------------------------------------------------------------------------------------
CUnitAction::CUnitAction(EUnitAction eAction, bool bBlocking, const Vec3& point, const Vec3& dir) :
	m_Action(eAction),
	m_BlockingPlan(bBlocking),
	m_Point(point),
	m_Direction(dir),
	m_Priority(PRIORITY_NORMAL),
	m_fDistance(1),
	m_Tag(0)
{
}
//
//----------------------------------------------------------------------------------------------------
CUnitAction::CUnitAction(EUnitAction eAction, bool bBlocking, float fDistance) :
	m_Action(eAction),
	m_BlockingPlan(bBlocking),
	m_fDistance(fDistance),
	m_Priority(PRIORITY_NORMAL),
	m_Tag(0)
{
}

//
//----------------------------------------------------------------------------------------------------
CUnitAction::CUnitAction(EUnitAction eAction, bool bBlocking, tukk szText) :
	m_Action(eAction),
	m_BlockingPlan(bBlocking),
	m_SignalText(szText),
	m_Priority(PRIORITY_NORMAL),
	m_fDistance(1),
	m_Tag(0)
{
}

//
//----------------------------------------------------------------------------------------------------
CUnitAction::CUnitAction(EUnitAction eAction, bool bBlocking, tukk szText, const AISignalExtraData& data) :
	m_Action(eAction),
	m_BlockingPlan(bBlocking),
	m_SignalText(szText),
	m_Priority(PRIORITY_NORMAL),
	m_fDistance(1),
	m_SignalData(data),
	m_Tag(0)
{
}

//
//----------------------------------------------------------------------------------------------------
CUnitAction::CUnitAction(EUnitAction eAction, bool bBlocking, i32 priority) :
	m_Action(eAction),
	m_BlockingPlan(bBlocking),
	m_Priority(priority),
	m_fDistance(1),
	m_Tag(0)
{
}

//
//----------------------------------------------------------------------------------------------------
CUnitAction::CUnitAction(EUnitAction eAction, bool bBlocking, i32 priority, const Vec3& point) :
	m_Action(eAction),
	m_BlockingPlan(bBlocking),
	m_Point(point),
	m_Priority(priority),
	m_fDistance(1),
	m_Tag(0)
{
}

//
//----------------------------------------------------------------------------------------------------
CUnitAction::CUnitAction(EUnitAction eAction, bool bBlocking, i32 priority, tukk szText) :
	m_Action(eAction),
	m_BlockingPlan(bBlocking),
	m_SignalText(szText),
	m_Priority(priority),
	m_fDistance(1),
	m_Tag(0)
{
}

//
//----------------------------------------------------------------------------------------------------
CUnitAction::CUnitAction(EUnitAction eAction, bool bBlocking, i32 priority, tukk szText, const AISignalExtraData& data) :
	m_Action(eAction),
	m_BlockingPlan(bBlocking),
	m_SignalText(szText),
	m_Priority(priority),
	m_fDistance(1),
	m_SignalData(data),
	m_Tag(0)
{
}

//
//----------------------------------------------------------------------------------------------------
CUnitAction::~CUnitAction()
{
	//unlock all my blocked actions
	UnlockMyBlockedActions();
	NotifyMyBlockingActions();
}

//
//----------------------------------------------------------------------------------------------------
void CUnitAction::BlockedBy(CUnitAction* blockingAction)
{
	m_BlockingActions.push_back(blockingAction);
	blockingAction->m_BlockedActions.push_back(this);
}

//
//----------------------------------------------------------------------------------------------------
void CUnitAction::UnlockMyBlockedActions()
{
	// unlock all the blocked actions
	//
	for (TActionList::iterator itAction = m_BlockedActions.begin(); itAction != m_BlockedActions.end(); ++itAction)
	{
		CUnitAction* blockedAction = (*itAction);
		TActionList::iterator itOtherUnitAction = find(blockedAction->m_BlockingActions.begin(), blockedAction->m_BlockingActions.end(), this);

		if (itOtherUnitAction != blockedAction->m_BlockingActions.end())
			blockedAction->m_BlockingActions.erase(itOtherUnitAction);// delete myself's pointer from the other action's blocking actions list

	}
}

//
//----------------------------------------------------------------------------------------------------
void CUnitAction::NotifyMyBlockingActions()
{
	// remove myself from the Blocking actions' list (so they won't block a finished action)
	for (TActionList::iterator itAction = m_BlockingActions.begin(); itAction != m_BlockingActions.end(); ++itAction)
	{
		CUnitAction* blockingAction = (*itAction);
		TActionList::iterator itOtherUnitAction = std::find(blockingAction->m_BlockedActions.begin(), blockingAction->m_BlockedActions.end(), this);

		if (itOtherUnitAction != blockingAction->m_BlockedActions.end())
			blockingAction->m_BlockedActions.erase(itOtherUnitAction);// delete myself's pointer from the other action's blocking actions list

	}
}
