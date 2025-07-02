// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef ___TEAM_BALANCING_H___
#define ___TEAM_BALANCING_H___

#include <drx3D/CoreX/Lobby/IDrxLobby.h>
#include <drx3D/Game/SessionNames.h>
#include <drx3D/Game/GameUserPackets.h>

#define TEAM_BALANCING_MAX_PLAYERS			MAX_SESSION_NAMES

struct SPlayerScores;
struct SDrxMatchMakingConnectionUID;

class CTeamBalancing
{
public:
	enum EUpdateTeamType
	{
		eUTT_Unlock,
		eUTT_Switch
	};

	CTeamBalancing()
		: m_pSessionNames(0)
		, m_maxPlayers(0)
		, m_bGameIsBalanced(false)
		, m_bGameHasStarted(false)
		, m_bBalancedTeamsForced(false)
	{

	}

	void Init(SSessionNames *pSessionNames);
	void Reset();

	void AddPlayer(SSessionNames::SSessionName *pPlayer);
	void RemovePlayer(const SDrxMatchMakingConnectionUID &uid);
	void UpdatePlayer(SSessionNames::SSessionName *pPlayer, u32 previousSkill);

	void UpdatePlayerScores(SPlayerScores *pScores, i32 numScores);

	void OnPlayerSpawned(const SDrxMatchMakingConnectionUID &uid);
	void OnPlayerSwitchedTeam(const SDrxMatchMakingConnectionUID &uid, u8 teamId);
	void OnGameFinished(EUpdateTeamType updateType);

	u8 GetTeamId(const SDrxMatchMakingConnectionUID &uid);
	bool IsGameBalanced() const;

	void SetLobbyPlayerCounts(i32 maxPlayers);
	void ForceBalanceTeams();

	i32 GetMaxNewSquadSize();

	void WritePacket(CDrxLobbyPacket *pPacket, GameUserPacketDefinitions packetType, SDrxMatchMakingConnectionUID playerUID); 
	void ReadPacket(CDrxLobbyPacket *pPacket, u32 packetType);

private:
	enum EPlayerGroupType
	{
		ePGT_Unknown,
		ePGT_Squad,
		ePGT_Clan,
		ePGT_Individual,
	};

	typedef u8 TGroupIndex;

	struct STeamBalancing_PlayerSlot
	{
		STeamBalancing_PlayerSlot();
		void Reset();

		SDrxMatchMakingConnectionUID m_uid;

		u32 m_skill;
		u32 m_previousScore;
		u8 m_teamId;
		TGroupIndex m_groupIndex;
		bool m_bLockedOnTeam;
		bool m_bUsed;
	};

	typedef u8 TPlayerIndex;
	typedef DrxFixedStringT<6> TSmallString;
	typedef DrxFixedArray<TPlayerIndex, TEAM_BALANCING_MAX_PLAYERS> TPlayerList;

	struct STeamBalancing_Group
	{
		STeamBalancing_Group();
		void Reset();

		void InitClan(CTeamBalancing *pTeamBalancing, TPlayerIndex playerIndex, tukk pClanName);
		void InitIndividual(CTeamBalancing *pTeamBalancing, TPlayerIndex playerIndex);
		void InitSquad(CTeamBalancing *pTeamBalancing, TPlayerIndex playerIndex, u32 leaderUID);

		void AddMember(CTeamBalancing *pTeamBalancing, TPlayerIndex playerIndex);
		void RemoveMember(CTeamBalancing *pTeamBalancing, TPlayerIndex playerIndex);

		TSmallString m_clanTag;
		u32 m_leaderUID;

		u32 m_totalSkill;
		u32 m_totalPrevScore;

		EPlayerGroupType m_type;

		TPlayerList m_members;

		u8 m_teamId;
		bool m_bLockedOnTeam;
		bool m_bPresentAtGameStart;
		bool m_bUsed;
	};

	struct SBalanceGroup
	{
		SBalanceGroup()
			: m_numPlayers(0)
			, m_totalScore(0)
			, m_desiredTeamId(0)
			, m_bPresentAtGameStart(false)
		{
			memset(m_pPlayers, 0, sizeof(m_pPlayers));
		}

		CTeamBalancing::STeamBalancing_PlayerSlot *m_pPlayers[TEAM_BALANCING_MAX_PLAYERS];
		i32 m_numPlayers;
		u32 m_totalScore;
		u8 m_desiredTeamId;
		bool m_bPresentAtGameStart;

		bool operator<(const SBalanceGroup &rhs) const
		{
			if (m_numPlayers > rhs.m_numPlayers)
			{
				return true;
			}
			else if (m_numPlayers < rhs.m_numPlayers)
			{
				return false;
			}

			if (m_totalScore > rhs.m_totalScore)
			{
				return true;
			}
			else if (m_totalScore < rhs.m_totalScore)
			{
				return false;
			}

			// Equal
			return false;
		}
	};

	STeamBalancing_Group *FindGroupByClan(tukk pClanName);
	STeamBalancing_Group *FindGroupBySquad(u32 leaderUID);
	STeamBalancing_Group *FindEmptyGroup();

	STeamBalancing_PlayerSlot *FindPlayerSlot(const SDrxMatchMakingConnectionUID &conId);
	const STeamBalancing_PlayerSlot *FindPlayerSlot(const SDrxMatchMakingConnectionUID &conId) const;

	STeamBalancing_PlayerSlot* AddPlayer(SDrxMatchMakingConnectionUID uid);

	void UpdatePlayer(STeamBalancing_PlayerSlot *pSlot, SSessionNames::SSessionName *pPlayer, bool bBalanceTeams);
	void UpdatePlayer( STeamBalancing_PlayerSlot *pSlot, u16 skillRank, u32 squadLeaderUID, DrxFixedStringT<CLAN_TAG_LENGTH> &clanTag, bool bBalanceTeams );

		TPlayerIndex GetIndexFromSlot(STeamBalancing_PlayerSlot* pPlayerSlot) const;
	TGroupIndex GetIndexFromGroup(STeamBalancing_Group* pGroup) const;
	void BalanceTeams();

	void CreateBalanceGroups(SBalanceGroup *pGroups, i32 *pNumGroups, i32 *pNumPlayersOnTeams, i32 *pNumTotalPlayers, bool bAllowCommit);
	void AssignDesiredTeamGroups(SBalanceGroup *pGroups, i32 *pNumGroups, i32 *pNumPlayersOnTeams, i32 numTotalPlayers, bool bAllowCommit);
	void AssignMaxPlayersToTeam(SBalanceGroup *pGroups, i32 *pNumGroups, i32 *pNumPlayersOnTeam);

	void WritePlayerToPacket(CDrxLobbyPacket *pPacket, const SSessionNames::SSessionName *pPlayer);
	void ReadPlayerFromPacket(CDrxLobbyPacket *pPacket, bool bBalanceTeams);

	STeamBalancing_Group m_groups[TEAM_BALANCING_MAX_PLAYERS];
	STeamBalancing_PlayerSlot m_players[TEAM_BALANCING_MAX_PLAYERS];

	SSessionNames *m_pSessionNames;
	i32 m_maxPlayers;
	bool m_bGameIsBalanced;
	bool m_bGameHasStarted;
	bool m_bBalancedTeamsForced;
};

#endif // ___TEAM_BALANCING_H___
