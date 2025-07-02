// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Class for interface between C++ and Lua for MatchMaking
-------------------------------------------------------------------------
История:
- 01:08:2011 : Created By Andrew Blackwell

*************************************************************************/

//////////////////////////////////////////////////////////////////////////
//Header Guard
#ifndef __MATCHMAKINGHANDLER_H__
#define __MATCHMAKINGHANDLER_H__

//////////////////////////////////////////////////////////////////////////
// Important Includes
//For eLDI_Max
#include <drx3D/Game/GameLobbyData.h>


#if GAMELOBBY_USE_COUNTRY_FILTERING
#define FIND_GAMES_SEARCH_NUM_DATA	13
#else
#define FIND_GAMES_SEARCH_NUM_DATA	12
#endif

class CMatchMakingHandler
{
public:
	CMatchMakingHandler();
	~CMatchMakingHandler();

	IScriptTable*	GetScriptTable() { return m_pScript; }


	//////////////////////////////////////////////////////////////////////////
	// These functions call directly to Lua
	void	OnEnterMatchMaking();
	void	OnLeaveMatchMaking();

	void	Update( float dt );

	void	OnHostMigrationFinished( bool sucess, bool isNewHost );
	void	OnSearchResult( SDrxSessionSearchResult* pSession );

	//////////////////////////////////////////////////////////////////////////
	// These functions are useful things for matchmaking
	static float	LegacyC2MatchMakingScore( SDrxSessionSearchResult* session, CGameLobby *lobby, bool includeRand );
	static bool		AllowedToCreateGame();
	bool					IsJoiningSession();
	DrxSessionID	GetSessionId( i32 sessionIndex );
	void					MMLog( tukk message, bool isError );

	//////////////////////////////////////////////////////////////////////////
	// These functions handle requests which have come from matchmaking (and usually require a response at a later time)
	void Search( i32 freeSlotsRequired, i32 maxResults, SDrxSessionSearchData* searchParameters, i32 numSearchParameters );
	void Join( i32 sessionIndex );
	void Merge( i32 sessionIndex );
	void CancelSearch();

	void ClearSessionParameters();
	void NewSessionParameter( ELOBBYIDS paramID, ScriptAnyValue valueVal );

	void RequestSubscribedUpdate( float timeToCall );

	//////////////////////////////////////////////////////////////////////////
	// These functions are callbacks and callback-likes
	static void SearchCallback( DrxLobbyTaskID taskID, EDrxLobbyError error, SDrxSessionSearchResult* session, uk arg );
	
	void GameLobbyJoinFinished( EDrxLobbyError error );
	void GameLobbyCreateFinished( EDrxLobbyError error );
	void AdjustCreateSessionData( SDrxSessionData* pData, u32 maxDataItems );
			
private:

	//private data types etc.
	enum EMatchMakingHandlerTask
	{
		eMMHT_None,
		eMMHT_EndSearch,
		eMMHT_EndJoin,
		eMMHT_EndMerge,
		eMMHT_EndCreate,
		eMMHT_EndHostMigrate,
	};

	struct SessionDetails
	{
		char									m_name[MAX_SESSION_NAME_LENGTH];
		DrxSessionID					m_id;
	};

	typedef std::map< i32, SessionDetails >	TSessionIdMap;
	
	union TaskData{ i32 iDat; bool bDat; };

	struct WaitingTaskEntry
	{
		EMatchMakingHandlerTask		taskType;
		bool											taskSuccess;
		TaskData									taskData;
	};

	//private functions
	bool	LoadScript();
	void	OnInit( CGameLobbyUpr* pLobbyUpr );

	bool AddWaitingTask( EMatchMakingHandlerTask taskID, bool taskSuccess, TaskData* pData = NULL );

	//Static members
	static i32				s_currentMMSearchID;
	static i32k	k_maxTasks = 4;

	//Data members
	SDrxLobbyUserData	m_sessionParams[ eLDI_Num ];
	TSessionIdMap			m_sessionIdMap;
	WaitingTaskEntry		m_waitingTasksQueue[ k_maxTasks ];

	SmartScriptTable	m_pScript;
	
	float							m_startTime;
	float							m_subscribedUpdateTimer;

	u32						m_nSessionParams;
	i32								m_sessionIdIndex;

	//for the waiting task queue
	u8							m_currentTask;
	u8							m_newTask;

	bool							m_bActive;
	bool							m_bIsMerging;

};

#endif	//__MATCHMAKINGHANDLER_H__

