// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CRECENTPLAYERSMGR_H__
#define __CRECENTPLAYERSMGR_H__

//---------------------------------------------------------------------------

#include <drx3D/Game/Network/Squad/ISquadEventListener.h>
#include <drx3D/Game/Network/Lobby/IGameLobbyEventListener.h>
#include <drx3D/CoreX/Containers/DrxFixedArray.h>
#include <drx3D/CoreX/Lobby/IDrxLobby.h>
#include <drx3D/Game/Network/FriendUpr/GameFriendData.h>
#include <drx3D/Game/IRecentPlayersMgrEventListener.h>

//---------------------------------------------------------------------------
// CRecentPlayersMgr
//     A class to manage and store potential new friends a player has encountered since running the game
//---------------------------------------------------------------------------
class CRecentPlayersMgr : public ISquadEventListener, public IGameLobbyEventListener
{
public:
	typedef struct SRecentPlayerData
	{
		DrxUserID m_userId;
		TLobbyUserNameString m_name;
		TGameFriendId m_internalId;

		static TGameFriendId __s_friend_internal_ids;

		SRecentPlayerData(DrxUserID &inUserId, tukk inNameString)
		{
			m_userId = inUserId;
			m_name = inNameString;
			m_internalId = __s_friend_internal_ids++;
		}
	};
	
	static i32k k_maxNumRecentPlayers=50;
	typedef DrxFixedArray<SRecentPlayerData, k_maxNumRecentPlayers> TRecentPlayersArray;


	CRecentPlayersMgr();
	virtual ~CRecentPlayersMgr();

	void Update(const float inFrameTime);

	void RegisterRecentPlayersMgrEventListener(IRecentPlayersMgrEventListener *pListener);
	void UnregisterRecentPlayersMgrEventListener(IRecentPlayersMgrEventListener *pListener);

	// ISquadEventListener
	virtual void AddedSquaddie(DrxUserID userId);
	virtual void RemovedSquaddie(DrxUserID userId) {}
	virtual void UpdatedSquaddie(DrxUserID userId);
	virtual void SquadLeaderChanged(DrxUserID oldLeaderId, DrxUserID newLeaderId) {}
	virtual void SquadNameChanged(tukk pInNewName) { }
	virtual void SquadEvent(ISquadEventListener::ESquadEventType eventType) { }
	// ~ISquadEventListener

	// IGameLobbyEventListener
	virtual void InsertedUser(DrxUserID userId, tukk userName);
	virtual void SessionChanged(const DrxSessionHandle inOldSession, const DrxSessionHandle inNewSession) { }
	// ~IGameLobbyEventListener

	const TRecentPlayersArray *GetRecentPlayers() const { return &m_players; }
	const SRecentPlayerData *FindRecentPlayerDataFromInternalId(TGameFriendId inInternalId);

	void OnUserChanged(DrxUserID localUserId);

protected:
	typedef std::vector<IRecentPlayersMgrEventListener*> TRecentPlayersFriendsEventListenersVec;
	TRecentPlayersFriendsEventListenersVec m_recentPlayersEventListeners;

	TRecentPlayersArray m_players;
	DrxUserID m_localUserId;

	void AddOrUpdatePlayer(DrxUserID &inUserId, tukk inUserName);
	SRecentPlayerData *FindRecentPlayerDataFromUserId(DrxUserID &inUserId);
	void EventRecentPlayersUpdated();
};
//---------------------------------------------------------------------------

#endif //__CRECENTPLAYERSMGR_H__
