// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Структура для отслеживания имён в сессиях
-------------------------------------------------------------------------
История:
- 15:03:2010 : Created By Ben Parbury

*************************************************************************/
#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/CoreX/Containers/DrxFixedArray.h>

#include <drx3D/Act/IPlayerProfiles.h>

#include <drx3D/CoreX/Lobby/IDrxLobby.h>
#include <drx3D/CoreX/Lobby/IDrxMatchMaking.h>
#include <drx3D/Game/GameRulesTypes.h>

#pragma once

#define CLAN_TAG_LENGTH		(5)
#define DISPLAY_NAME_LENGTH (DRXLOBBY_USER_NAME_LENGTH + CLAN_TAG_LENGTH + 1)	// +1 for space
#define MAX_ONLINE_STATS_SIZE (1500)
#define MAX_SESSION_NAMES (MAX_PLAYER_LIMIT)

// If you add or remove an enumerate element you should also:
// - Update the numbers so there are no jumps
// - Check that the number of elements doesn't exceed DRXLOBBY_USER_DATA_SIZE_IN_BYTES
enum ELocalUserData
{
	eLUD_SquadId1 = 0,
	eLUD_SquadId2 = 1,
	eLUD_SkillRank1 = 2,
	eLUD_SkillRank2 = 3,
	eLUD_ClanTag1 = 4,
	eLUD_ClanTag2 = 5,
	eLUD_ClanTag3 = 6,
	eLUD_ClanTag4 = 7,
	eLUD_LoadedDLCs = 8,
	eLUD_VoteChoice = 9,
};

struct SSessionNames
{
	struct SSessionName
	{
		static i32k MUTE_REASON_WRONG_TEAM			= (1 << 0);
		static i32k MUTE_REASON_MANUAL					= (1 << 1);
		static i32k MUTE_REASON_NOT_IN_SQUAD		= (1 << 2);
		static i32k MUTE_REASON_MUTE_ALL				= (1 << 3);

		static i32k MUTE_PLAYER_AUTOMUTE_REASONS = (MUTE_REASON_NOT_IN_SQUAD | MUTE_REASON_MUTE_ALL); // Reasons controlled by the automute

		SSessionName(DrxUserID userId, const SDrxMatchMakingConnectionUID &conId, tukk name, u8k* userData, bool isDedicated);
		void Set(DrxUserID userId, const SDrxMatchMakingConnectionUID &conId, tukk name, u8k* userData, bool isDedicated);		
		void SetUserData(u8k *userData);
		void GetDisplayName(DrxFixedStringT<DISPLAY_NAME_LENGTH> &displayName) const;
		void GetClanTagName(DrxFixedStringT<CLAN_TAG_LENGTH> &name) const;
		u16 GetSkillRank() const;

		//---------------------------------------
		DrxUserID m_userId;
		float m_timeWithoutConnection;
		SDrxMatchMakingConnectionUID m_conId;
		char m_name[DRXLOBBY_USER_NAME_LENGTH];
		u8	m_userData[DRXLOBBY_USER_DATA_SIZE_IN_BYTES];
		u8 m_teamId;
		u8 m_rank;
		u8 m_reincarnations;	// Not currently serialised (if needed add to eGUPD_SetTeamAndRank packet)
		u8 m_muted;
		bool m_bMustLeaveBeforeServer;
		bool m_isDedicated;
		bool m_bFullyConnected;		// Server only flag used by the GameLobby to determine if players have finished identifying themselves
	};

	const static i32 k_unableToFind = -1;
	DrxFixedArray<SSessionName, MAX_SESSION_NAMES> m_sessionNames;
	bool m_dirty;		// So the UI knows it needs to be updated

	SSessionNames();
	u32 Size() const;
	void Clear();
	i32 Find(const SDrxMatchMakingConnectionUID &conId) const;
	i32 FindByUserId(const DrxUserID &userId) const;
	i32 FindIgnoringSID(const SDrxMatchMakingConnectionUID &conId) const;
	SSessionNames::SSessionName* GetSessionName(const SDrxMatchMakingConnectionUID &conId, bool ignoreSID);
	SSessionNames::SSessionName* GetSessionNameByUserId(const DrxUserID &userId);
	const SSessionNames::SSessionName* GetSessionNameByUserId(const DrxUserID &userId) const;
	i32 Insert(DrxUserID userId, const SDrxMatchMakingConnectionUID &conId, tukk name, u8k* userData, bool isDedicated);
	void Remove(const SDrxMatchMakingConnectionUID &conId);
	void Update(DrxUserID userId, const SDrxMatchMakingConnectionUID &conId, tukk name, u8k* userData, bool isDedicated, bool findByUserId);
	void RemoveBlankEntries();
	void Tick(float dt);
	bool RemoveEntryWithInvalidConnection();
};
