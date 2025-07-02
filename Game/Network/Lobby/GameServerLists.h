// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef ___GAME_SERVER_LISTS_H___
#define ___GAME_SERVER_LISTS_H___

#include <drx3D/Game/Network/GameNetworkDefines.h>

//#include <drx3D/Game/FrontEnd/Multiplayer/UIServerList.h>

#if IMPLEMENT_PC_BLADES

#include <drx3D/CoreX/Lobby/IDrxLobby.h>
#include <drx3D/Act/IPlayerProfiles.h>
struct SServerInfo
{
	SServerInfo():
		m_numPlayers(0),
		m_maxPlayers(0),
		m_region(0),
		m_ping(-1),
		m_maxSpectators(0),
		m_numSpectators(0),
		m_serverId(-1),
		m_sessionId(DrxSessionInvalidID),
		m_official(false),
		m_ranked(false),
		m_reqPassword(false),
		m_friends(false)
	{	}

	bool IsFull() const { return (m_numPlayers == m_maxPlayers); }
	bool IsEmpty() const { return (m_numPlayers == 0); }

	typedef DrxFixedStringT<32> TFixedString32;

	TFixedString32    m_hostName;
	TFixedString32    m_mapName;
	TFixedString32    m_mapDisplayName;
	TFixedString32		m_gameTypeName;
	TFixedString32		m_gameTypeDisplayName;
	TFixedString32    m_gameVariantName;
	TFixedString32    m_gameVariantDisplayName;
	TFixedString32    m_gameVersion;

	i32				m_numPlayers;
	i32				m_maxPlayers;
	i32				m_region;
	i32       m_ping;
	i32				m_maxSpectators;
	i32				m_numSpectators;

	i32       m_serverId;
	// SPersistentGameId		m_sessionFavouriteKeyId; // TODO: michiel?
	DrxSessionID  m_sessionId;

	bool			m_official;
	bool			m_ranked;
	bool			m_reqPassword;
	bool			m_friends;
};

class CGameServerLists : public IPlayerProfileListener
{
public:
	CGameServerLists();
	virtual ~CGameServerLists();

	enum EGameServerLists
	{
		eGSL_Recent,
		eGSL_Favourite,
		eGSL_Size
	};

	const bool Add(const EGameServerLists list, tukk name, u32k favouriteId, bool bFromProfile);
	const bool Remove(const EGameServerLists list, u32k favouriteId);

	void ServerFound( const SServerInfo &serverInfo, const EGameServerLists list, u32k favouriteId );
	void ServerNotFound( const EGameServerLists list, u32k favouriteId );

	const bool InList(const EGameServerLists list, u32k favouriteId) const;
	i32k GetTotal(const EGameServerLists list) const;

	void PopulateMenu(const EGameServerLists list) const;

	void SaveChanges();

	//IPlayerProfileListener
	void SaveToProfile(IPlayerProfile* pProfile, bool online, u32 reason);
	void LoadFromProfile(IPlayerProfile* pProfile, bool online, u32 reason);
	//~IPlayerProfileListener

#if 0 && !defined(_RELEASE)
	static void CmdAddFavourite(IConsoleCmdArgs* pCmdArgs);
	static void CmdRemoveFavourite(IConsoleCmdArgs* pCmdArgs);
	static void CmdListFavourites(IConsoleCmdArgs* pCmdArgs);
	static void CmdShowFavourites(IConsoleCmdArgs* pCmdArgs);

	static void CmdAddRecent(IConsoleCmdArgs* pCmdArgs);
	static void CmdRemoveRecent(IConsoleCmdArgs* pCmdArgs);
	static void CmdListRecent(IConsoleCmdArgs* pCmdArgs);
	static void CmdShowRecent(IConsoleCmdArgs* pCmdArgs);
#endif

	static i32k k_maxServersStoredInList = 50;

protected:
	struct SServerInfoInt
	{
		SServerInfoInt(tukk name, u32 favouriteId);
		bool operator == (const SServerInfoInt & other) const;

		string m_name;
		u32 m_favouriteId;
	};

	struct SListRules
	{
		SListRules();

		void Reset();
		void PreApply(std::list<SServerInfoInt>* pList, const SServerInfoInt &pNewInfo);

		uint m_limit;
		bool m_unique;
	};

	void Reset();

	std::list<SServerInfoInt> m_list[eGSL_Size];
	SListRules m_rule[eGSL_Size];

	bool m_bHasChanges;
};

#endif //IMPLEMENT_PC_BLADES

#endif //___GAME_SERVER_LISTS_H___