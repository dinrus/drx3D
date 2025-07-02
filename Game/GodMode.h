// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef GOD_MODE_H
#define GOD_MODE_H

class CActor;

enum EGodModeState
{
	eGMS_None,
	eGMS_GodMode,
	eGMS_TeamGodMode,
	eGMS_DemiGodMode,
	eGMS_NearDeathExperience,
	eGMS_LAST
};

class CGodMode
{
public:
	static CGodMode& GetInstance();

	void RegisterConsoleVars(IConsole* pConsole);
	void UnregisterConsoleVars(IConsole* pConsole) const;

	void MoveToNextState();
	bool RespawnIfDead(CActor* actor) const;
	bool RespawnPlayerIfDead() const;

	EGodModeState GetCurrentState() const;
	tukk GetCurrentStateString() const;

	void Update(float frameTime);
	void DemiGodDeath();

	bool IsGod() const;
	bool IsDemiGod() const;
	bool IsGodModeActive() const;

	void ClearCheckpointData();
	void SetNewCheckpoint(const Matrix34& rWorldMat);

private:
	CGodMode();
	CGodMode(const CGodMode&);
	CGodMode& operator=(const CGodMode&);

	static const float m_timeToWaitBeforeRespawn;
	static tukk m_godModeCVarName;
	static tukk m_demiGodRevivesAtCheckpointCVarName;

	float m_elapsedTime;
	i32 m_godMode;
	i32 m_demiGodRevivesAtCheckpoint;

	bool m_hasHitCheckpoint;
	bool m_respawningFromDemiGodDeath;
	Matrix34	m_lastCheckpointWorldTM;
};

#endif
