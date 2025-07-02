// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Class for returning a fake session list to session searches.
Aim is to use this to test match making performance.
The fake session lists are loaded from XML files.

-------------------------------------------------------------------------
История:
- 20:07:2011 : Created By Andrew Blackwell

*************************************************************************/

#include <drx3D/Game/StdAfx.h>

//////////////////////////////////////////////////////////////////////////
// This Include
#include <drx3D/Game/SessionSearchSimulator.h>

#include <drx3D/Game/GameLobbyData.h>
#include <drx3D/Game/GameLobby.h>
#include <drx3D/Game/PlayerProgression.h>

//-------------------------------------------------------------------------
//Constructor
CSessionSearchSimulator::CSessionSearchSimulator()
:	m_currentNode( 0 )
{
}

//--------------------------------------------------------------------------
//Destructor
CSessionSearchSimulator::~CSessionSearchSimulator()
{
}

//--------------------------------------------------------------------------
bool CSessionSearchSimulator::OpenSessionListXML( tukk filepath )
{
	m_sessionListXML = GetISystem()->LoadXmlFromFile( filepath );

	if( m_sessionListXML )
	{
		m_currentFilepath.Format( filepath );
		if( strcmpi( m_sessionListXML->getTag(), "MatchMakingTelemetryXML" ) == 0 )
		{
			//XML appears to be in the format we expect
			if( m_sessionListXML->getChildCount() > 0 )
			{
				m_currentNode = 0;
				return true;
			}
		}
	}

	return false;
}

//--------------------------------------------------------------------------
bool CSessionSearchSimulator::OutputSessionListBlock( DrxLobbyTaskID& taskID, DrxMatchmakingSessionSearchCallback cb, uk cbArg )
{
	bool noMoreSessions = false;
	bool retval = false;
	XmlNodeRef currentNode;

	if( m_sessionListXML && m_currentNode < m_sessionListXML->getChildCount() )
	{
		//skip any nodes that aren't found session nodes
		currentNode = m_sessionListXML->getChild( m_currentNode );

		while( strcmpi( currentNode->getTag(), "foundSession" ) != 0)
		{
			++m_currentNode;
			if( m_currentNode >= m_sessionListXML->getChildCount() )
			{
				noMoreSessions = true;
				break;
			}

			currentNode = m_sessionListXML->getChild( m_currentNode );
		}
	}
	else
	{
		noMoreSessions = true;
	}

	//if we didn't find any session nodes
	if( noMoreSessions )
	{
		//end call, return false
		(cb)( taskID, eCLE_Success, NULL, cbArg );
		retval = false;
	}	
	else
	{
		//for each node until current node isn't a session node
		while( strcmpi( currentNode->getTag(), "foundSession" ) == 0)
		{
			//get the node's data
			SDrxSessionSearchResult sessionData;
			SDrxLobbyUserData userData[16];

			sessionData.m_data.m_data = userData;

			u32 ping;
			if( currentNode->getAttr( "ping", ping ) )
			{	
				sessionData.m_ping = ping;
			}

			u32 filledslots;
			if( currentNode->getAttr( "filledSlots", filledslots ) )
			{
				sessionData.m_numFilledSlots = filledslots;
			}

			tukk pSessionStr;
			if( currentNode->getAttr( "sessionId", &pSessionStr ) )
			{
				SDrxFakeSessionID* pSessionID = new SDrxFakeSessionID();
				drx_strcpy( pSessionID->m_idStr, pSessionStr );
			
				sessionData.m_id = pSessionID;
			}

			tukk pSessionName;
			if( currentNode->getAttr( "servername", &pSessionName ) )
			{
				drx_strcpy( sessionData.m_data.m_name, pSessionName );
			}

			//generic data elements
			u32 iData = 0;

			//all sessions returned should be the same version, playlist and variant as we are currently running/searching for
			sessionData.m_data.m_data[ iData ].m_id = LID_MATCHDATA_VERSION;
			sessionData.m_data.m_data[ iData ].m_type = eCLUDT_Int32;
			sessionData.m_data.m_data[ iData ].m_int32 = GameLobbyData::GetVersion();
			iData++;

			sessionData.m_data.m_data[ iData ].m_id = LID_MATCHDATA_PLAYLIST;
			sessionData.m_data.m_data[ iData ].m_type = eCLUDT_Int32;
			sessionData.m_data.m_data[ iData ].m_int32 = GameLobbyData::GetPlaylistId();
			iData++;

			sessionData.m_data.m_data[ iData ].m_id = LID_MATCHDATA_VARIANT;
			sessionData.m_data.m_data[ iData ].m_type = eCLUDT_Int32;
			sessionData.m_data.m_data[ iData ].m_int32 = GameLobbyData::GetVariantId();
			iData++;

			sessionData.m_data.m_data[ iData ].m_id = LID_MATCHDATA_REQUIRED_DLCS;
			sessionData.m_data.m_data[ iData ].m_type = eCLUDT_Int32;
			sessionData.m_data.m_data[ iData ].m_int32 = 0;
			iData++;

			u32 skillDiff;
			if( currentNode->getAttr( "rankDifference", skillDiff ) )
			{
				sessionData.m_data.m_data[ iData ].m_id = LID_MATCHDATA_SKILL;
				sessionData.m_data.m_data[ iData ].m_type = eCLUDT_Int32;
				sessionData.m_data.m_data[ iData ].m_int32 = CPlayerProgression::GetInstance()->GetData(EPP_SkillRank) - skillDiff;
				iData++;
			}

			u32 languageID;
			if( currentNode->getAttr( "language", languageID ) )
			{
				sessionData.m_data.m_data[ iData ].m_id = LID_MATCHDATA_LANGUAGE;
				sessionData.m_data.m_data[ iData ].m_type = eCLUDT_Int32;
				sessionData.m_data.m_data[ iData ].m_int32 = languageID;
				iData++;
			}

			u32 regionID;
			if( currentNode->getAttr( "region", regionID ) )
			{
				//match the parameter used by the different platforms for region data
#if GAMELOBBY_USE_COUNTRY_FILTERING
				sessionData.m_data.m_data[ iData ].m_id = LID_MATCHDATA_COUNTRY;
				sessionData.m_data.m_data[ iData ].m_type = eCLUDT_Int32;
				sessionData.m_data.m_data[ iData ].m_int32 = regionID;
				iData++;
#endif
			}

			u32 activeStatus = CGameLobby::eAS_Lobby;
			tukk statusStr;
			if( currentNode->getAttr( "status", &statusStr ) )
			{
				if( strcmpi( statusStr, "Lobby" ) == 0)
				{
					activeStatus = CGameLobby::eAS_Lobby;
				}
				else if( strcmpi( statusStr, "InGame" ) == 0)
				{
					activeStatus = CGameLobby::eAS_Game;
				}
				else if( strcmpi( statusStr, "EndGame" ) == 0)
				{
					activeStatus = CGameLobby::eAS_EndGame;
				}
				else if( strcmpi( statusStr, "StartGame" ) == 0)
				{
					activeStatus = CGameLobby::eAS_StartingGame;
				}
			}

			sessionData.m_data.m_data[ iData ].m_id = LID_MATCHDATA_ACTIVE;
			sessionData.m_data.m_data[ iData ].m_type = eCLUDT_Int32;
			sessionData.m_data.m_data[ iData ].m_int32 = activeStatus;
			iData++;

			sessionData.m_data.m_numData = iData;
								
			//pass the data to the callback
			(cb)( taskID, eCLE_SuccessContinue, &sessionData, cbArg );

			//check we're not out of nodes
			++m_currentNode;
			if( m_currentNode >= m_sessionListXML->getChildCount() )
			{
				break;
			}
			currentNode = m_sessionListXML->getChild( m_currentNode );
		}

		//end call normally, return true
		(cb)( taskID, eCLE_Success, NULL, cbArg );
		retval = true;
	}

	return retval;
}


