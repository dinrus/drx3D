// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Script bindings needed to implement matchmaking in Lua
Hopefully covering things we may need in the future
-------------------------------------------------------------------------
История:
- 01:08:2011 : Created By Andrew Blackwell

*************************************************************************/


#ifndef __SCRIPTBIND_MATCHMAKING_H__
#define __SCRIPTBIND_MATCHMAKING_H__

#if _MSC_VER > 1000
# pragma once
#endif

//////////////////////////////////////////////////////////////////////////
// Base Class include
#include <drx3D/Script/ScriptHelpers.h>


//////////////////////////////////////////////////////////////////////////
// Pre-Declarations
struct ISystem;
class CMatchMakingHandler;

class CScriptBind_MatchMaking :
	public CScriptableBase
{
public:
	CScriptBind_MatchMaking( ISystem *pSystem );
	virtual ~CScriptBind_MatchMaking();

	virtual void GetMemoryUsage( IDrxSizer *pSizer ) const
	{
		pSizer->AddObject( this, sizeof(*this) );
	}

	void AttachTo( CMatchMakingHandler *pMatchmaking, CGameLobbyUpr *pLobbyUpr );

	//////////////////////////////////////////////////////////////////////////
	// Functions to Query status
	i32	IsSquadLeaderOrSolo( IFunctionHandler *pH );
	i32	IsJoiningSession( IFunctionHandler *pH );
	i32	IsSessionHost( IFunctionHandler *pH );
	i32	IsInSession( IFunctionHandler *pH );
	i32	HasGameStarted( IFunctionHandler *pH );
	i32	HaveEnoughPlayersToStart( IFunctionHandler *pH );
	i32	GetNumPlayersInCurrentSession( IFunctionHandler *pH );
	i32	GetNumPlayersInSquad( IFunctionHandler *pH );
	i32	GetMaxNumPlayers( IFunctionHandler *pH );

//	i32 GetMySkillScore( IFunctionHandler* pH );
	i32 GetAverageSkillScore( IFunctionHandler* pH /*,bool squadOnly*/ );

	i32 GetCurrentRegion( IFunctionHandler* pH );
	i32 GetCurrentLanguage( IFunctionHandler* pH );
	
	i32 GetCurrentMatchMakingVersionNum( IFunctionHandler* pH );

	i32 GetCurrentPlaylist( IFunctionHandler* pH );
	i32 GetCurrentVariant( IFunctionHandler* pH );
	i32 GetAvailableDLCs( IFunctionHandler* pH );
	i32 GetCurrentPing( IFunctionHandler* pH );

//	TODO - support these if required
//	i32 GetCurrentGameMode( IFunctionHandler* pH );
//	i32 GetCurrentMap( IFunctionHandler* pH );

	//////////////////////////////////////////////////////////////////////////
	// Functions to Request action
	i32 StartSearch( IFunctionHandler *pH, i32 freeSlots, i32 maxResults, SmartScriptTable searchParams );
	i32	MergeWithServer( IFunctionHandler *pH, i32 sessionId );
	i32	JoinServer( IFunctionHandler *pH, i32 sessionId );
	i32	CreateServer( IFunctionHandler *pH, SmartScriptTable sessionParams );
	i32	CancelSearch( IFunctionHandler *pH );
	i32 RequestUpdateCall( IFunctionHandler* pH, float timeToCall );

	i32 MMLog( IFunctionHandler *pH, tukk message, bool isError );

private:
	void RegisterMethods();
	void RegisterGlobals();

	ISystem*						m_pSystem;
	CGameLobbyUpr*	m_pLobbyUpr;
};


#endif //__SCRIPTBIND_MATCHMAKING_H__
