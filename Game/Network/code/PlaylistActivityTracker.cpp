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

//////////////////////////////////////////////////////////////////////////
// Pre-compiled Header
#include <drx3D/Game/StdAfx.h>
// This Include
#include <drx3D/Game/PlaylistActivityTracker.h>

// Game Includes
#include <drx3D/Game/Game.h>
#include <drx3D/Game/Network/Lobby/GameLobby.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/Game/PlaylistUpr.h>

// Engine Includes
#include <drx3D/CoreX/Lobby/IDrxTCPService.h>

#define k_defaultPlaylistActivityServerName		""	
#define k_defaultPlaylistActivityServerPort		8042

const float k_activityUpdateFrequency = 60.0f;		//1 min, just depends on how up to date we want this to be
const float k_onPlaylistUpdateFrequency = 600.0f;	//10 min, should match how long server side marks us as active for

//------------------------------------------------------------------------
// Constructor
// cppcheck-suppress uninitMemberVar
CPlaylistActivityTracker::CPlaylistActivityTracker()
:	m_nKnownPlaylists(0)
, m_state(eATS_Idle)
, m_timeUntilNextAction(-1.0f)
, m_currentActivityCallback(nullptr)
{
	m_serverNameCVar = REGISTER_STRING("g_playlistActivity_server", k_defaultPlaylistActivityServerName, 0, "Usage: g_playlistActivity_server <ip or hostname>\n");
	m_serverPortCVar = REGISTER_INT("g_playlistActivity_port", k_defaultPlaylistActivityServerPort, 0, "Usage: g_playlistActivity_port <port>\n");

	for(i32 i = 0; i < eRTT_MaxCount; ++i)
	{
		m_downloadableResources[i] = NULL;
	}
}

//------------------------------------------------------------------------
// Destructor
CPlaylistActivityTracker::~CPlaylistActivityTracker()
{
	//clean up downloadable resources
	for( i32 i = 0; i < eRTT_MaxCount; i++ )
	{
		if( m_downloadableResources[ i ] != NULL )
		{
			m_downloadableResources[ i ]->CancelDownload();
			m_downloadableResources[ i ] = NULL;
		}
	}

	gEnv->pConsole->UnregisterVariable( "g_playlistActivity_server" );
	gEnv->pConsole->UnregisterVariable( "g_playlistActivity_port" );
}

//------------------------------------------------------------------------
void CPlaylistActivityTracker::Update( float dt )
{
	//update downloadable resources
	for( i32 i = 0; i < eRTT_MaxCount; i++ )
	{
		if( m_downloadableResources[ i ] != NULL )
		{
			m_downloadableResources[ i ]->DispatchCallbacks();
		}
	}

	if( m_state != eATS_Idle )
	{
		//check conditions
		if( m_state == eATS_OnPlaylist )
		{	
			CGameLobby* pLobby = g_pGame->GetGameLobby();

			if( !pLobby || pLobby->GetState() == eLS_None )
			{
				//game lobby isn't active so this must be the wrong state we're in
				//set us back to get activity because we're still in multiplayer
				m_state = eATS_GetActivity;
				//small delay on next poll in case other things are going on
				m_timeUntilNextAction = 10.0f;
			}
		}

		m_timeUntilNextAction -= dt;
		
		if( m_timeUntilNextAction <= 0.0f )
		{
			switch( m_state )
			{
			case eATS_GetActivity:
				{
					//we're in some state where we want a regular update of the playlist activity
					//e.g. on the multiplayer menus, ask for the data again
					RequestCurrentActivity();
					m_timeUntilNextAction = k_activityUpdateFrequency;
					
					break;
				}
			case eATS_OnPlaylist:
				{
					//re-send the on a playlist info
					CPlaylistUpr* pPlaylists = g_pGame->GetPlaylistUpr();

					if( const SPlaylist* pCurrentPlaylist = pPlaylists->GetCurrentPlaylist() )					
					{
						i32 variantIndex = pPlaylists->GetActiveVariantIndex();
						JoinedPlaylist( pCurrentPlaylist->id, variantIndex );
					}

					m_timeUntilNextAction = k_onPlaylistUpdateFrequency;
				
					break;
				}
			}
		}

	}

}

#define MAX_CONTENT			(128)

//------------------------------------------------------------------------
bool CPlaylistActivityTracker::JoinedPlaylist( ILevelRotation::TExtInfoId playlistId, u32 variantId )
{
	bool success = false;

	//ensure service is initialised
	if( ! AnyActiveRequestsOfType( eRTT_AnnounceJoinPlaylist ) )
	{
		//construct the http request
		DrxFixedStringT<MAX_CONTENT> httpParams;

		httpParams.Format( "?platform=1&playlist=%d&variant=%d", playlistId, variantId );

		success = UploadData( "playlist/join/", httpParams.c_str(), 1024, eRTT_AnnounceJoinPlaylist );
	}

	return success;
}

//------------------------------------------------------------------------
bool CPlaylistActivityTracker::CreatedGame( ILevelRotation::TExtInfoId playlistId, u32 variantId )
{
	bool success = false;

	//ensure service is initialised
	if(! AnyActiveRequestsOfType( eRTT_AnnounceCreateGame ) )
	{
		//construct the http request
		DrxFixedStringT<MAX_CONTENT> httpParams;

		httpParams.Format( "?platform=1&playlist=%d&variant=%d", playlistId, variantId );

		success = UploadData( "playlist/game_created/", httpParams.c_str(), 1024, eRTT_AnnounceCreateGame );
	}

	return success;
}

//------------------------------------------------------------------------
bool CPlaylistActivityTracker::RequestCurrentActivity( PlayListActivityCallback callback )
{
	bool success = false;

	// Don't start another request if we're already waiting on one
	if( ! AnyActiveRequestsOfType( eRTT_RequestActivity ) )
	{
		DrxFixedStringT<MAX_CONTENT> httpParams;
		httpParams.Format( "?platform=1");

		success = UploadData( "playlist/all_playlist_activity/", httpParams.c_str(), 4096, eRTT_RequestActivity );

		m_currentActivityCallback = callback;
	}

	return success;
}

//------------------------------------------------------------------------
bool CPlaylistActivityTracker::UploadData( tukk pUrlPath, tukk pUrlParams, i32 receiveSize, ERequestTaskType taskType )
{
	CDownloadableResourcePtr newResource = new CDownloadableResource;

	newResource->SetDownloadInfo( pUrlParams, pUrlPath, m_serverNameCVar->GetString(), m_serverPortCVar->GetIVal(), receiveSize, "PlaylistRequest" );
	newResource->AddDataListener( this );
	m_downloadableResources[ taskType ] = newResource;

	return true;
}


//------------------------------------------------------------------------
bool CPlaylistActivityTracker::GetCachedActivity( ILevelRotation::TExtInfoId playlistId, u32& players, u32& games ) const
{
	bool exists = false;

	for( u32	i = 0; i < m_nKnownPlaylists; i++ )
	{
		if( m_activityData[ i ].id == playlistId )
		{
			players = m_activityData[ i ].nPlayers;
			games = m_activityData[ i ].nGames;
			exists = true;
			break;
		}
	}

	return exists;
}

//------------------------------------------------------------------------
bool CPlaylistActivityTracker::GetCachedActivity( ILevelRotation::TExtInfoId playlistId, u32 variantId, u32& players, u32& games ) const
{
	bool exists = false;
	
	for( u32	i = 0; i < m_nKnownPlaylists; i++ )
	{
		if( m_activityData[ i ].id == playlistId )
		{
			const PlaylistActivity& playlist = m_activityData[ i ];
			for( u32	 j = 0; j < playlist.nVariants; j++ )
			{
				if( playlist.variants[ j ].id == variantId )
				{
					players = playlist.variants[ j ].nPlayers;
					games = playlist.variants[ j ].nGames;
					exists = true;
					break;
				}
			}
			break;
		}
	}

	return exists;
}

//------------------------------------------------------------------------
void CPlaylistActivityTracker::SetState( EActivityTrackerState state )
{
	m_state = state;
	m_timeUntilNextAction = 0.0f;
}

//------------------------------------------------------------------------
void CPlaylistActivityTracker::DataDownloaded( CDownloadableResourcePtr inResource )
{
	//get the task type of the resource by finding it in our vector
	ERequestTaskType taskType = eRTT_MaxCount;
	for( i32 i = 0; i < eRTT_MaxCount; i++ )
	{
		if( m_downloadableResources[ i ] == inResource )
		{
			taskType = (ERequestTaskType)i;
			break;
		}
	}

	if( taskType == eRTT_RequestActivity )
	{
		bool dataRead = false;
		//parse xml
		i32 bufferSize = -1;
		tuk buffer;
		inResource->GetRawData( &buffer, &bufferSize );

		//debug output data stream to log
		DrxLog( "PlaylistActivity: Full processed Data %s", buffer );

		IXmlParser* parser = GetISystem()->GetXmlUtils()->CreateXmlParser();
		XmlNodeRef topNode = parser->ParseBuffer(buffer, bufferSize, false);
		if( topNode && stricmp( topNode->getTag(), "playlists" ) == 0 )
		{
			//Iterate over playlists
			i32k numberOfPlaylistElements = topNode->getChildCount();
			i32 destPlaylistIdx = 0;	//separate index to write to in case there are non-playlist node
			for(i32 playlistIdx = 0; playlistIdx < numberOfPlaylistElements && destPlaylistIdx < MAX_PLAYLISTS; ++playlistIdx )
			{
				//get and check playlist node
				const XmlNodeRef playlistNode = topNode->getChild( playlistIdx );
				if( playlistNode && stricmp( playlistNode->getTag(), "playlist" ) == 0 )
				{
					PlaylistActivity& currentPlaylist = m_activityData[ destPlaylistIdx ];

					//copy name and Id into structure
					tukk playlistName = NULL;
					if( playlistNode->getAttr( "name", &playlistName ) )
					{
						drx_strcpy( currentPlaylist.name, playlistName );
					}

					i32 id = 0;
					if( playlistNode->getAttr( "index", id ) )
					{
						currentPlaylist.id = id;
					}

					//clear totals
					currentPlaylist.nPlayers = 0;
					currentPlaylist.nGames = 0;

					//Iterate over variants
					i32k numberOfVariantElements = playlistNode->getChildCount();
					currentPlaylist.nVariants = 0; //use count as index to write to in case there are non-variant nodes
					for(i32 variantIdx = 0; variantIdx < numberOfVariantElements && currentPlaylist.nVariants < MAX_VARIANTS; ++variantIdx )
					{
						//get and check variant node
						const XmlNodeRef variantNode = playlistNode->getChild( variantIdx );
						if( variantNode && stricmp( variantNode->getTag(), "variant" ) == 0 )
						{
							VariantActivity& currentVariant = currentPlaylist.variants[ currentPlaylist.nVariants ];

							//copy data into structure
							tukk variantName = NULL;
							if( variantNode->getAttr( "name", &variantName ) )
							{
								drx_strcpy( currentVariant.name, variantName );
							}

							i32 variantId = 0;
							if( variantNode->getAttr( "index", variantId ) )
							{
								currentVariant.id = variantId;
							}

							i32 count = 0;
							if( variantNode->getAttr( "players", count ) )
							{
								currentVariant.nPlayers = count;
								//increase total
								currentPlaylist.nPlayers += count;
							}

							if( variantNode->getAttr( "games", count ) )
							{
								currentVariant.nGames = count;
								//increase total
								currentPlaylist.nGames += count;
							}

							currentPlaylist.nVariants++;

							dataRead = true;
						}
					}

					destPlaylistIdx++;
				}
			}//end loop
			m_nKnownPlaylists = destPlaylistIdx;
		}

		if( m_currentActivityCallback )
		{
			m_currentActivityCallback( dataRead, m_activityData, m_nKnownPlaylists );
		}
		
	}
	else
	{
		//other requests don't have a payload to deliver to us
	}

	ReleaseResourceReference( inResource );
}

//------------------------------------------------------------------------
void CPlaylistActivityTracker::DataFailedToDownload( CDownloadableResourcePtr inResource )
{
	ReleaseResourceReference( inResource );
}

//------------------------------------------------------------------------
void CPlaylistActivityTracker::ReleaseResourceReference( CDownloadableResourcePtr resource )
{
	for( i32 i = 0; i < eRTT_MaxCount; i++ )
	{
		if( m_downloadableResources[ i ] == resource )
		{
			m_downloadableResources[ i ] = NULL;
			break;
		}
	}
}

//------------------------------------------------------------------------
bool CPlaylistActivityTracker::AnyActiveRequestsOfType( ERequestTaskType type )
{
	return ( m_downloadableResources[ type ] != NULL );
}

//------------------------------------------------------------------------
i32 CPlaylistActivityTracker::GetPlatformType()
{
	i32 platformType;

	platformType=ePT_PC;

	return platformType;
}






