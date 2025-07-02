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

//////////////////////////////////////////////////////////////////////////
//Header Guard
#ifndef __SESSIONSEARCHSIMULATOR_H__
#define __SESSIONSEARCHSIMULATOR_H__

//For SDrxSessionID and DRXSESSIONID_STRINGLEN
#include <drx3D/CoreX/Lobby/IDrxLobby.h>
//For DrxMatchmakingSessionSearchCallback
#include <drx3D/CoreX/Lobby/IDrxMatchMaking.h>


//////////////////////////////////////////////////////////////////////////
//The session search simulator class
class CSessionSearchSimulator
{
public:
	CSessionSearchSimulator();
	~CSessionSearchSimulator();

	bool OpenSessionListXML( tukk filepath );
	bool OutputSessionListBlock( DrxLobbyTaskID& taskID, DrxMatchmakingSessionSearchCallback cb, uk cbArg );

	//Inlines
	tukk GetCurrentFilepath() { return m_currentFilepath.c_str(); }

private:
	DrxFixedStringT< IDrxPak::g_nMaxPath >	m_currentFilepath;

	XmlNodeRef	m_sessionListXML;
	i32	m_currentNode;
};

//////////////////////////////////////////////////////////////////////////
// Fake Session IDs used by Session Search Simulator.
// They are only required to display session IDs read in as strings
struct SDrxFakeSessionID : public SDrxSessionID
{
	char m_idStr[DRXSESSIONID_STRINGLEN];
	
	bool operator == (const SDrxSessionID &other)
	{
		char otherIdStr[DRXSESSIONID_STRINGLEN];
		other.AsCStr( otherIdStr, DRXSESSIONID_STRINGLEN );
		return ( strcmpi( m_idStr, otherIdStr ) == 0 );
	}
	
	bool operator < (const SDrxSessionID &other)
	{
		char otherIdStr[DRXSESSIONID_STRINGLEN];
		other.AsCStr( otherIdStr, DRXSESSIONID_STRINGLEN );
		return ( strcmpi( m_idStr, otherIdStr ) < 0 );
	}
	
	bool IsFromInvite() const
	{
		return false;
	}

	void AsCStr( tuk pOutString, i32 inBufferSize ) const
	{
		if (pOutString && inBufferSize > 0)
		{
			size_t len = inBufferSize > sizeof(m_idStr) ? sizeof(m_idStr) : inBufferSize;
			memcpy( pOutString, m_idStr, len);
			pOutString[len-1]=0;
		}
	}
};


#endif	//__SESSIONSEARCHSIMULATOR_H__

