// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef GameAISystem_h
#define GameAISystem_h

#include <drx3D/Game/AI/IGameAIModule.h>
#include <drx3D/Game/AI/AdvantagePointOccupancyControl.h>
#include <drx3D/Game/DeathUpr.h>
#include <drx3D/Game/VisibleObjectsHelper.h>
#include <drx3D/Game/TargetTrackThreatModifier.h>
#include <drx3D/Game/GameAIRecorder.h>
#include <drx3D/Game/AIAwarenessToPlayerHelper.h>
#include <drx3D/Game/AICounters.h>
#include <drx3D/Game/AISquadUpr.h>
#include <drx3D/Game/EnvironmentDisturbanceUpr.h>
#include <drx3D/Game/AICorpse.h>

class CGameAISystem
{
public:
	CGameAISystem();
	~CGameAISystem();
	IGameAIModule* FindModule(tukk moduleName) const;
	void EnterModule(EntityId entityID, tukk moduleName);
	void LeaveModule(EntityId entityID, tukk moduleName);
	void LeaveAllModules(EntityId entityID);
	void PauseModule(EntityId entityID, tukk moduleName);
	void PauseAllModules(EntityId entityID);
	void ResumeModule(EntityId entityID, tukk moduleName);
	void ResumeAllModules(EntityId entityID);
	void Update(float frameTime);
	void Reset(bool bUnload);
	void Serialize(TSerialize ser);
	void PostSerialize();

	CAdvantagePointOccupancyControl& GetAdvantagePointOccupancyControl() { return m_advantagePointOccupancyControl; }
	GameAI::DeathUpr* GetDeathUpr() { return m_pDeathUpr; }
	CVisibleObjectsHelper& GetVisibleObjectsHelper() { return m_visibleObjectsHelper; }
	CAIAwarenessToPlayerHelper& GetAIAwarenessToPlayerHelper() { return m_AIAwarenessToPlayerHelper; }
	CAICounters& GetAICounters() { return m_AICounters; }
	AISquadUpr& GetAISquadUpr() { return m_AISquadUpr; }
	GameAI::EnvironmentDisturbanceUpr& GetEnvironmentDisturbanceUpr() { return m_environmentDisturbanceUpr; }

#ifdef INCLUDE_GAME_AI_RECORDER
	CGameAIRecorder &GetGameAIRecorder() { return m_gameAIRecorder; }
	const CGameAIRecorder &GetGameAIRecorder() const { return m_gameAIRecorder; }
#endif //INCLUDE_GAME_AI_RECORDER

	enum State
	{
		Idle,
		UpdatingModules
	};

private:
	void UpdateModules(float frameTime);
	void UpdateSubSystems(float frameTime);
	void ResetModules(bool bUnload);
	void ResetSubSystems(bool bUnload);
	void Error(tukk functionName) const;
	void InformContentCreatorOfError(string logMessage) const;
	void GetCallStack(string& callstack) const;
	typedef std::vector<IGameAIModule*> Modules;
	Modules m_modules;

	CAdvantagePointOccupancyControl m_advantagePointOccupancyControl;
	GameAI::DeathUpr* m_pDeathUpr;
	CVisibleObjectsHelper m_visibleObjectsHelper;
	CAIAwarenessToPlayerHelper m_AIAwarenessToPlayerHelper;
	CAICounters m_AICounters;
	CTargetTrackThreatModifier m_targetTrackThreatModifier;
	AISquadUpr m_AISquadUpr;
	GameAI::EnvironmentDisturbanceUpr m_environmentDisturbanceUpr;
	State m_state;

	CAICorpseUpr* m_pCorpsesUpr;

#ifdef INCLUDE_GAME_AI_RECORDER
	CGameAIRecorder m_gameAIRecorder;
#endif //INCLUDE_GAME_AI_RECORDER
};

#endif // GameAISystem_h
