// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Game/RecentPlayersMgr.h>


#include <drx3D/Game/Utility/DrxWatch.h>
#include <drx3D/Game/Network/Lobby/SessionNames.h>
#include <drx3D/Game/Network/Squad/SquadUpr.h>
#include <drx3D/Game/UI/UIUpr.h>
#include <drx3D/Game/UI/UICVars.h>
#include <drx3D/Game/Network/FriendUpr/GameFriendsMgr.h>

/*static*/ TGameFriendId CRecentPlayersMgr::SRecentPlayerData::__s_friend_internal_ids = 0x10000;	// so our ids wont clash with gamefriends mgr ids

#if !defined(_RELEASE)
	#define RECENT_PLAYERS_MGR_DEBUG 1
#else
	#define RECENT_PLAYERS_MGR_DEBUG 0
#endif

CRecentPlayersMgr::CRecentPlayersMgr()
{
	CGameLobby *pGameLobby = g_pGame->GetGameLobby();
	DRX_ASSERT(pGameLobby);
	pGameLobby->RegisterGameLobbyEventListener(this);

	CSquadUpr *pSquadUpr = g_pGame->GetSquadUpr();
	DRX_ASSERT(pSquadUpr);
	pSquadUpr->RegisterSquadEventListener(this);

	m_localUserId = DrxUserInvalidID;
}

CRecentPlayersMgr::~CRecentPlayersMgr()
{
	CGameLobby *pGameLobby = g_pGame->GetGameLobby();
	if (pGameLobby)
	{
		pGameLobby->UnregisterGameLobbyEventListener(this);
	}

	CSquadUpr *pSquadUpr = g_pGame->GetSquadUpr();
	if (pSquadUpr)
	{
		pSquadUpr->UnregisterSquadEventListener(this);
	}
}

void CRecentPlayersMgr::Update(const float inFrameTime)
{
	// add cvar driven DrxWatches()
}

void CRecentPlayersMgr::RegisterRecentPlayersMgrEventListener(IRecentPlayersMgrEventListener *pListener)
{
	stl::push_back_unique(m_recentPlayersEventListeners, pListener);
}

void CRecentPlayersMgr::UnregisterRecentPlayersMgrEventListener(IRecentPlayersMgrEventListener *pListener)
{
	stl::find_and_erase(m_recentPlayersEventListeners, pListener);
}

void CRecentPlayersMgr::AddedSquaddie(DrxUserID userId)
{
	AddOrUpdatePlayer(userId, NULL);
}

void CRecentPlayersMgr::UpdatedSquaddie(DrxUserID userId)
{
	AddOrUpdatePlayer(userId, NULL);
}

void CRecentPlayersMgr::InsertedUser(DrxUserID userId, tukk userName)
{
	AddOrUpdatePlayer(userId, userName);
}

const CRecentPlayersMgr::SRecentPlayerData *CRecentPlayersMgr::FindRecentPlayerDataFromInternalId(TGameFriendId inInternalId)
{
	i32k numPlayers = m_players.size();
	const SRecentPlayerData *pFoundData=NULL;

	for (i32 i=0; i<numPlayers; i++)
	{
		const SRecentPlayerData *pPlayerData=&m_players[i];

		if (pPlayerData->m_internalId == inInternalId)
		{
			pFoundData=pPlayerData;
			break;
		}
	}

	return pFoundData;
}

void CRecentPlayersMgr::AddOrUpdatePlayer(DrxUserID &inUserId, tukk inUserName)
{
	bool bUpdated=false;

	if (inUserId == DrxUserInvalidID)
	{
		DrxLog("CRecentPlayersMgr::AddOrUpdatePlayer() ignoring invalid userId for userName=%s", inUserName ? inUserName : "NULL");
		return;
	}

	if (inUserId == m_localUserId)
	{
		DrxLog("CRecentPlayersMgr::AddOrUpdatePlayer() ignoring the local user id=%s; userName=%s", inUserId.get()->GetGUIDAsString().c_str(), inUserName ? inUserName : "NULL");
		return;
	}

	CGameFriendsMgr *pGameFriendsMgr = g_pGame->GetGameFriendsMgr();
	if (pGameFriendsMgr)
	{
		SGameFriendData *pGameFriendData = pGameFriendsMgr->GetFriendByDrxUserId(inUserId);
		if (pGameFriendData)
		{
			if (pGameFriendData->IsFriend())
			{
				DrxLog("CRecentPlayersMgr::AddOrUpdatePlayer() ignoring someone who's already a friend - user id=%s; userName=%s", inUserId.get()->GetGUIDAsString().c_str(), inUserName ? inUserName : "NULL");
				return;
			}
		}
	}

	if (!inUserName)
	{
		DrxLog("CRecentPlayersMgr::AddOrUpdatePlayer() passed a null inUserName. Trying to find userName from userId=%s", inUserId.get()->GetGUIDAsString().c_str());

		if (CSquadUpr *pSquadUpr = g_pGame->GetSquadUpr())
		{
			const SSessionNames *pSessionNames = pSquadUpr->GetSessionNames();
			if(pSessionNames)
			{
				const SSessionNames::SSessionName *pSquaddieName = pSessionNames->GetSessionNameByUserId(inUserId);
				if (pSquaddieName)
				{
					inUserName = pSquaddieName->m_name;
				}
			}
		}
	}

	if (!inUserName)
	{
		DrxLog("CRecentPlayersMgr::AddOrUpdatePlayer() has failed to find a userName for inUserId=%s. Returning.", inUserId.get()->GetGUIDAsString().c_str());
		return;
	}
	
	DrxLog("CRecentPlayersMgr::AddOrUpdatePlayer() inUserId=%s; inUserName=%s", inUserId.get()->GetGUIDAsString().c_str(), inUserName);

	SRecentPlayerData *pPlayerData = FindRecentPlayerDataFromUserId(inUserId);
	if (pPlayerData)
	{
		DrxLog("CRecentPlayersMgr::AddOrUpdatePlayer() found already existing user");

		if (pPlayerData->m_name.compareNoCase(inUserName))
		{
			DrxLog("CRecentPlayersMgr::AddOrUpdatePlayer() found existing user's name is different. Updating from %s to %s", pPlayerData->m_name.c_str(), inUserName);
			pPlayerData->m_name = inUserName;
			bUpdated=true;
		}
	}
	else
	{
		DrxLog("CRecentPlayersMgr::AddOrUpdatePlayer() need to add new user");
		if (m_players.isfull())
		{
			m_players.removeAt(0);
		}

		SRecentPlayerData newPlayerData(inUserId, inUserName);
		m_players.push_back(newPlayerData);
		bUpdated=true;
	}

	if (bUpdated)
	{
		EventRecentPlayersUpdated();
	}
}

CRecentPlayersMgr::SRecentPlayerData *CRecentPlayersMgr::FindRecentPlayerDataFromUserId(DrxUserID &inUserId)
{
	i32k numPlayers = m_players.size();
	SRecentPlayerData *pFoundData=NULL;

	for (i32 i=0; i<numPlayers; i++)
	{
		SRecentPlayerData *pPlayerData=&m_players[i];

		if (pPlayerData->m_userId == inUserId)
		{
			pFoundData=pPlayerData;
			break;
		}
	}

	return pFoundData;
}

void CRecentPlayersMgr::EventRecentPlayersUpdated()
{
	const size_t numListeners = m_recentPlayersEventListeners.size();
	for (size_t i = 0; i < numListeners; ++ i)
	{
		m_recentPlayersEventListeners[i]->UpdatedRecentPlayers();
	}
}

void CRecentPlayersMgr::OnUserChanged(DrxUserID localUserId)
{
	m_players.clear();
	m_localUserId = localUserId;
}
