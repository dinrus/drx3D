// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/**********************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание:

-------------------------------------------------------------------------
История:
*************************************************************************/
#include <drx3D/Game/StdAfx.h>

#include <drx3D/Game/PlayerStateEvents.h>
#include <drx3D/Game/Player.h>


class CPlayerStateEntry : private CStateHierarchy<CPlayer>
{
	DECLARE_STATE_CLASS_BEGIN( CPlayer, CPlayerStateEntry )
	DECLARE_STATE_CLASS_END( CPlayer );
};

DEFINE_STATE_CLASS_BEGIN( CPlayer, CPlayerStateEntry, PLAYER_STATE_ENTRY, Root )
DEFINE_STATE_CLASS_END( CPlayer, CPlayerStateEntry );


const CPlayerStateEntry::TStateIndex CPlayerStateEntry::Root( CPlayer& player, const SStateEvent& event )
{
	const EPlayerStateEvent eventID = static_cast<EPlayerStateEvent> (event.GetEventId());
	switch( eventID )
	{
	case PLAYER_EVENT_ENTRY_PLAYER:
		RequestTransitionState(player, PLAYER_STATE_MOVEMENT);
		break;
	case PLAYER_EVENT_ENTRY_AI:
		RequestTransitionState(player, PLAYER_STATE_AIMOVEMENT);
		break;
	}

	return State_Continue;
}
