// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id: ReplayObject.cpp$
$DateTime$
Описание: A replay entity spawned during KillCam replay

-------------------------------------------------------------------------
История:
- 03/19/2010 09:15:00: Created by Martin Sherburn

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/ReplayObject.h>
#include <drx3D/Act/IItemSystem.h>

CReplayObject::CReplayObject()
: m_timeSinceSpawn(0)
{
}

CReplayObject::~CReplayObject()
{
}

//------------------------------------------------------------------------
bool CReplayObject::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);

	return true;
}

//------------------------------------------------------------------------
bool CReplayObject::ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params )
{
	ResetGameObject();

	DRX_ASSERT_MESSAGE(false, "CReplayObject::ReloadExtension not implemented");
	
	return false;
}

//------------------------------------------------------------------------
bool CReplayObject::GetEntityPoolSignature( TSerialize signature )
{
	DRX_ASSERT_MESSAGE(false, "CReplayObject::GetEntityPoolSignature not implemented");
	
	return true;
}

//------------------------------------------------------------------------
//- REPLAY ACTION
//------------------------------------------------------------------------

CReplayObjectAction::CReplayObjectAction( FragmentID fragID, const TagState &fragTags, u32 optionIdx, bool trumpsPrevious, i32k priority )
	: BaseClass(priority, fragID)
	, m_trumpsPrevious(trumpsPrevious)
{
	m_fragTags = fragTags;
	m_optionIdx = optionIdx;
}

EPriorityComparison CReplayObjectAction::ComparePriority( const IAction &actionCurrent ) const
{
	if (m_trumpsPrevious)
	{
		return ((IAction::Installed == actionCurrent.GetStatus() && IAction::Installing & ~actionCurrent.GetFlags()) ? Higher : BaseClass::ComparePriority(actionCurrent));
	}
	else
	{
		return Equal;
	}
}

//////////////////////////////////////////////////////////////////////////

void CReplayItemList::AddItem( const EntityId itemId )
{
	m_items.push_back(itemId);
}

void CReplayItemList::OnActionControllerDeleted()
{
	IItemSystem* pItemSys = g_pGame->GetIGameFramework()->GetIItemSystem();
	TItemVec::const_iterator end = m_items.end();
	for(TItemVec::const_iterator it = m_items.begin(); it!=end; ++it)
	{
		if(IItem* pItem = pItemSys->GetItem(*it))
		{
			pItem->UpdateCurrentActionController();
		}
	}

	m_items.clear();

}
