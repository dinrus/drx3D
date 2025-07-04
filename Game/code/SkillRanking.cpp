// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание:	Class for calculating player skill rankings

-------------------------------------------------------------------------
История:
- 22:09:2010 : Created by Colin Gulliver

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/SkillRanking.h>

#if !defined(_RELEASE) && !DRX_PLATFORM_ORBIS 
	#define ENABLE_SKILL_DEBUG 1
#else
	#define ENABLE_SKILL_DEBUG 0
#endif

#if ENABLE_SKILL_DEBUG
    #define	SKILL_LOG(...) DrxLog("[SKILL] " __VA_ARGS__);
	#define SKILL_ASSERT(condition, message, ...) if (!condition) { SKILL_LOG(message, __VA_ARGS__); DRX_ASSERT(condition); }
#else
	#define SKILL_LOG(...) {}
	#define SKILL_ASSERT(condition, message, ...) {}
#endif

//---------------------------------------------------------------------------
CSkillRanking::CSkillRanking() : m_numPlayers(0)
{
}

//---------------------------------------------------------------------------
void CSkillRanking::AddPlayer( EntityId id, u16 skillPoints, i32 playerPoints, i32 teamId, float fractionTimeInGame )
{
	SKILL_ASSERT((m_numPlayers < MAX_PLAYERS), "player id %u added but there's no more space", id);
	if (m_numPlayers < MAX_PLAYERS)
	{
		m_players[m_numPlayers].Set(id, skillPoints, playerPoints, teamId, fractionTimeInGame);
		++ m_numPlayers;

		SKILL_LOG("player added, id=%u, skillPoints=%u, playerPoints=%d, teamId=%d, fractionTimeInGame=%f", id, skillPoints, playerPoints, teamId, fractionTimeInGame);
	}
}

//---------------------------------------------------------------------------
float CSkillRanking::GetPlayerFactor(i32 playerIndex, float averagePlayerPoints, float averageSkillRank)
{
	const float playerPoints = (float) m_players[playerIndex].m_playerPoints;
	const float actualPlayerScore = playerPoints / averagePlayerPoints;
	const float expectedPlayerScore = ((float) m_players[playerIndex].m_skillPoints) / averageSkillRank;

	const float playerFactor = actualPlayerScore - expectedPlayerScore;

	return playerFactor;
}

//---------------------------------------------------------------------------
u16 CSkillRanking::GetNewSkillRank(u16 currentSkillRank, float totalFactor)
{
	const float fNumPointsToAdd = totalFactor * 32.f;
	i32 numPointsToAdd = 0;
	if (fNumPointsToAdd > 0.f)
	{
		numPointsToAdd = (i32) floor(fNumPointsToAdd);
	}
	else
	{
		numPointsToAdd = (i32) ceil(fNumPointsToAdd);
	}

	i32 newRank = numPointsToAdd + (i32) currentSkillRank;
	newRank = CLAMP(newRank, 0, 0xFFFF);
	return (u16) newRank;
}

//---------------------------------------------------------------------------
void CSkillRanking::TeamGameFinished( i32 team1Score, i32 team2Score )
{
	u32k numPlayers = m_numPlayers;
	SKILL_LOG("Team game finished, team1Score=%d, team2Score=%d, numPlayers=%u", team1Score, team2Score, numPlayers);

	float teamSkillScore[3] = { 0.f };	// Use array size 3 so that we can use the teamId as an index
	i32 teamMemberCount[3] = { 0 };

	float totalPlayerPoints = 0.f;

	// Determine if we've got enough players to change scores (at least 1 per team)
	for (u32 i = 0; i < numPlayers; ++ i)
	{
		i32k teamId = m_players[i].m_teamId;
		teamSkillScore[teamId] += (float) m_players[i].m_skillPoints;
		totalPlayerPoints += (float) m_players[i].m_playerPoints;
		++ teamMemberCount[teamId];
	}
	SKILL_LOG("  team1 score=%f, members=%d, team2 score=%f, members=%d", teamSkillScore[1], teamMemberCount[1], teamSkillScore[2], teamMemberCount[2]);
	if ((teamMemberCount[1] == 0) || (teamMemberCount[2] == 0))
	{
		SKILL_LOG("    At least 1 team has no members, all player's scores stay the same");
		return;
	}

	// Now we know we've got enough players we can start calculating scores, can't adjust scores until after this
	// point since scores need to stay the same if we don't have enough players
	const float totalSkillScore = teamSkillScore[1] + teamSkillScore[2];
	const float finalTeamScores[3] = { 0.f, (float) team1Score, (float) team2Score };
	const float actualTotalScores = finalTeamScores[1] + finalTeamScores[2];

	if ((totalSkillScore == 0.f) || (actualTotalScores == 0.f) || (totalPlayerPoints == 0.f))
	{
		SKILL_LOG("  neither team scored or no players have a skill ranking, bailing (totalSkillScore=%f, actualTotalScores=%f, totalPlayerPoints=%f)", totalSkillScore, actualTotalScores, totalPlayerPoints);
		return;
	}

	// Calculate team factors - based on expected team score vs actual team score
	float teamFactor[3] = { 0.f };
	for (i32 i = 1; i < 3; ++ i)
	{
		const float expectedTeamScore = teamSkillScore[i] / totalSkillScore;
		const float actualTeamScores = finalTeamScores[i] / actualTotalScores;

		teamFactor[i] = actualTeamScores - expectedTeamScore;
		SKILL_LOG("    team %i: expectedTeamScore=%f, actualTeamScore=%f", i, expectedTeamScore, actualTeamScores);
	}
	SKILL_LOG("  totalSkillScore=%f, totalActualScore=%f, team1Factor=%f, team2Factor=%f", totalSkillScore, actualTotalScores, teamFactor[1], teamFactor[2]);

	const float fNumPlayers = (float) numPlayers;
	const float averagePlayerPoints = totalPlayerPoints / fNumPlayers;
	const float averageSkillScore = totalSkillScore / fNumPlayers;

	SKILL_LOG("  averagePlayerPoints=%f, averageSkillScore=%f", averagePlayerPoints, averageSkillScore);

	for (u32 i = 0; i < numPlayers; ++ i)
	{
		i32k teamId = m_players[i].m_teamId;

		const float playerFactor = GetPlayerFactor(i, averagePlayerPoints, averageSkillScore);

		const float totalFactor = playerFactor + teamFactor[teamId];

		m_players[i].m_newSkillScore = GetNewSkillRank(m_players[i].m_skillPoints, totalFactor);

		i32k diff = m_players[i].m_newSkillScore - m_players[i].m_skillPoints;
		SKILL_LOG("      id %u, team=%i, pp=%d, totalFactor=%f, skill=%d->%d (%s%d)", m_players[i].m_id, m_players[i].m_teamId, m_players[i].m_playerPoints, totalFactor, m_players[i].m_skillPoints, m_players[i].m_newSkillScore, diff >= 0 ? "+" : "", diff);
	}
}

//---------------------------------------------------------------------------
void CSkillRanking::NonTeamGameFinished()
{
	SKILL_LOG("Non-team game finished");

	u32k numPlayers = m_numPlayers;

	if (numPlayers < 2)
	{
		SKILL_LOG("  Not enough players in game, no change to scores (numPlayers=%u)", numPlayers);
		return;
	}

	float totalPlayerPoints = 0.f;
	float totalSkillScore = 0.f;

	for (u32 i = 0; i < numPlayers; ++ i)
	{
		i32k teamId = m_players[i].m_teamId;
		totalPlayerPoints += (float) m_players[i].m_playerPoints;
		totalSkillScore += (float) m_players[i].m_skillPoints;
	}

	if ((totalSkillScore == 0.f) || (totalPlayerPoints == 0.f))
	{
		SKILL_LOG("  no players scored or no players have a skill ranking, bailing (totalSkillScore=%f, totalPlayerPoints=%f)", totalSkillScore, totalPlayerPoints);
		return;
	}

	const float fNumPlayers = (float) numPlayers;
	const float averagePlayerPoints = totalPlayerPoints / fNumPlayers;
	const float averageSkillScore = totalSkillScore / fNumPlayers;

	for (u32 i = 0; i < numPlayers; ++ i)
	{
		const float playerFactor = GetPlayerFactor(i, averagePlayerPoints, averageSkillScore);

		const float totalFactor = (playerFactor * 2.f);		// Double player factor since we don't have a team one

		m_players[i].m_newSkillScore = GetNewSkillRank(m_players[i].m_skillPoints, totalFactor);

		i32k diff = m_players[i].m_newSkillScore - m_players[i].m_skillPoints;
		SKILL_LOG("    id %u, pp=%d, totalFactor=%f, skill=%d->%d (%s%d)", m_players[i].m_id, m_players[i].m_playerPoints, totalFactor, m_players[i].m_skillPoints, m_players[i].m_newSkillScore, diff >= 0 ? "+" : "", diff);
	}
}

//---------------------------------------------------------------------------
bool CSkillRanking::GetSkillPoints( EntityId id, u16 &result )
{
	u32k numPlayers = m_numPlayers;

	for (u32 i = 0; i < numPlayers; ++ i)
	{
		if (m_players[i].m_id == id)
		{
			result = m_players[i].m_newSkillScore;
			return true;
		}
	}

	return false;
}

//---------------------------------------------------------------------------
void CSkillRanking::NextGame()
{
	u32k numPlayers = m_numPlayers;

	for (u32 i = 0; i < numPlayers; ++ i)
	{
		m_players[i].m_skillPoints = m_players[i].m_newSkillScore;
	}
}

#undef SKILL_LOG
#undef SKILL_ASSERT
