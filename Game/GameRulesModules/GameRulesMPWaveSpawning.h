// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 07:03:2012 : Created by Jonathan Bunner
*************************************************************************/

#ifndef __GAMERULES_MP_WAVE_SPAWNING_H__
#define __GAMERULES_MP_WAVE_SPAWNING_H__

#include <drx3D/Game/GameRulesMPSimpleSpawning.h>

class CGameRulesMPWaveSpawning : public CGameRulesRSSpawning
{

public:
	typedef CGameRulesRSSpawning TInherited;

	CGameRulesMPWaveSpawning();
	virtual ~CGameRulesMPWaveSpawning();

	// IGameRulesMPSpawningModule
	virtual void Init(XmlNodeRef xml);
	virtual void Update(float frameTime);
	virtual float GetTimeFromDeathTillAutoReviveForTeam(i32 teamId) const;
	// ~IGameRulesMPSpawningModule

	// IGameRulesRevivedListener
	virtual void EntityRevived(EntityId entityId);
	// ~IGameRulesRevivedListener

	float GetTeamReviveTimeOverride(i32 teamId) const; 

private:
	typedef u8 TRuleFlags;

	// helpers
	bool WaveSpawningEnabled() const; 
	bool WaveSpawningEnabled(i32 teamId) const; 
	void ResetWaveRespawnTimer(); 

	// data
	float m_fWaveSpawnInterval;
	float m_fWaveSpawnTimer;
	
	// Team 1 / 2 or both teams may opt in for wave spawning
	// LogicState //
	enum ERuleFlag
	{
		ERuleFlag_None									= 0,
		ERuleFlag_team1WaveRespawningEnabled			= 1 << 0,
		ERuleFlag_team2WaveRespawningEnabled			= 1 << 1,
	};
	TRuleFlags m_ruleFlags;
	// ~State // 
};

#endif // __GAMERULES_MP_WAVE_SPAWNING_H__