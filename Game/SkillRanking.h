// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание:	Class for calculating player skill rankings

-------------------------------------------------------------------------
История:
- 22:09:2010 : Created by Colin Gulliver

*************************************************************************/

#ifndef __SKILL_RANKING_H__
#define __SKILL_RANKING_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/CoreX/Containers/DrxFixedArray.h>

class CSkillRanking
{
public:
	CSkillRanking();

	void AddPlayer(EntityId id, u16 skillPoints, i32 playerPoints, i32 teamId, float fractionTimeInGame);

	void TeamGameFinished(i32 team1Score, i32 team2Score);
	void NonTeamGameFinished();

	bool GetSkillPoints(EntityId id, u16 &result);

	void NextGame();
private:

	struct SSkillPlayerData
	{
		SSkillPlayerData() : m_skillPoints(0), m_newSkillScore(0), m_playerPoints(0), m_teamId(0) {}

		void Set(EntityId id, u16 skillPoints, i32 playerPoints, i32 teamId, float fractionTimeInGame)
		{
			m_id = id;
			m_skillPoints = skillPoints;
			m_newSkillScore = m_skillPoints;
			m_playerPoints = (playerPoints > 0 ? playerPoints : 0);		// Make sure points is >= 0
			m_teamId = teamId;

			// Scale points depending on time in game in an attempt  to simulate what the score 
			// would have been had the person been in for the whole game
			if (fractionTimeInGame > 0.f)
			{
				m_playerPoints = (i32) ( ((float)m_playerPoints) / fractionTimeInGame );
			}
		}

		EntityId m_id;
		u16 m_skillPoints;
		u16 m_newSkillScore;
		i32 m_playerPoints;
		i32 m_teamId;
	};

	float GetPlayerFactor(i32 playerIndex, float averagePlayerPoints, float averageSkillRank);
	u16 GetNewSkillRank(u16 currentSkillRank, float totalFactor);

	static i32k MAX_PLAYERS = MAX_PLAYER_LIMIT;

	SSkillPlayerData m_players[MAX_PLAYERS];
	u32 m_numPlayers;
};

#endif // __SKILL_RANKING_H__
