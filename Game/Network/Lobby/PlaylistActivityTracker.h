// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Описание: PlaylistActivityTracker - reports joining a playlist game to webserver, and retrieves counts of current activity from the same
  
 -------------------------------------------------------------------------
  История:
  - 05:01:2012: Created by Andrew Blackwell

*************************************************************************/

#ifndef __PLAYLISTACTIVITYUTILITY_H__
#define __PLAYLISTACTIVITYUTILITY_H__

//////////////////////////////////////////////////////////////////////////
// Important Includes
#include <drx3D/CoreX/Lobby/IDrxTCPService.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/Act/ILevelSystem.h>
#include <drx3D/Game/DownloadMgr.h>


#define MAX_PLAYLISTS 10
#define MAX_VARIANTS 6
#define MAX_LEN_PLAYLIST_NAME 20

//////////////////////////////////////////////////////////////////////////
// CPlaylistActivityTracker definition
class CPlaylistActivityTracker : public IDataListener
{
public:
	//Constructor/Destructor
	CPlaylistActivityTracker();
	~CPlaylistActivityTracker();

//------------------------------------------------------------------------
// public types
	enum EActivityTrackerState
	{
		eATS_Idle,
		eATS_GetActivity,
		eATS_OnPlaylist,
	};

	struct VariantActivity
	{
		char		name[ MAX_LEN_PLAYLIST_NAME ];
		u32	nPlayers;
		u32	nGames;
		u8		id;
	};

	struct PlaylistActivity
	{
		VariantActivity							variants[ MAX_VARIANTS ];
		char												name[ MAX_LEN_PLAYLIST_NAME ];
		u32											nPlayers;
		u32											nGames;
		ILevelRotation::TExtInfoId	id;
		u32											nVariants;
		
	};

	typedef void			( *PlayListActivityCallback )( bool success, const PlaylistActivity* results, i32 numResults );
	
//------------------------------------------------------------------------
// public functions
	void Update( float dt );
	void SetState( EActivityTrackerState state );

	//'Advertise' joining a playlist
	//returns false if request fails to start
	bool JoinedPlaylist( ILevelRotation::TExtInfoId playlistId, u32 variantId );

	//'Advertise' creating a game
	//returns false if request fails to start
	bool CreatedGame( ILevelRotation::TExtInfoId playlistId, u32 variantId );

	//Request info on a playlist/playlists, with callback for results
	//returns false if request fails to start
	bool RequestCurrentActivity( PlayListActivityCallback callback = NULL );
	
	bool GetCachedActivity( ILevelRotation::TExtInfoId playlistId, u32& players, u32& games ) const;
	bool GetCachedActivity( ILevelRotation::TExtInfoId playlistId, u32 variantId, u32& players, u32& games ) const;
	
	// IDataListener
	virtual void			DataDownloaded( CDownloadableResourcePtr inResource );
	virtual void			DataFailedToDownload( CDownloadableResourcePtr inResource );
	// ~IDataListener

//------------------------------------------------------------------------
// private members
private:

	enum ERequestTaskType
	{
		eRTT_AnnounceJoinPlaylist,
		eRTT_AnnounceCreateGame,
		eRTT_RequestActivity,
		eRTT_MaxCount,
	};

	enum EPlatformType
	{
		ePT_Unknown=0,
		ePT_PC,
		ePT_PS4,
		ePT_XBOXONE,
	};

	bool	UploadData( tukk pUrlPath, tukk pUrlParams, i32 receiveSize, ERequestTaskType taskType );
	bool	AnyActiveRequestsOfType( ERequestTaskType type );
	
	void	ReleaseResourceReference( CDownloadableResourcePtr resource );
	i32		GetPlatformType();

	//server we communicate with
	ICVar					*m_serverNameCVar;
	ICVar					*m_serverPortCVar;

	//Downloadable resources for receiving data, we may have multiple in flight at the same time
	CDownloadableResourcePtr m_downloadableResources [eRTT_MaxCount]; 


	//callback to pass processed data to
	PlayListActivityCallback m_currentActivityCallback;

	//state data for update
	EActivityTrackerState	m_state;
	float	m_timeUntilNextAction;

	//cache of playlist activity
	u32						m_nKnownPlaylists;
	PlaylistActivity	m_activityData[ MAX_PLAYLISTS ];
};

#endif //__PLAYLISTACTIVITYUTILITY_H__
