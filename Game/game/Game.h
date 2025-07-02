// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CryGame/IGame.h>
#include <CryGame/IGameFramework.h>
#include <IActionMapUpr.h>

static tukk GAME_NAME = "GameZero";
static tukk GAME_LONGNAME = "DRXENGINE SDK Game Example";
static tukk GAME_GUID = "{00000000-1111-2222-3333-444444444444}";

class CGame : public IGame
{
public:
	CGame();
	virtual ~CGame();

	// IGame
	virtual bool                   Init(IGameFramework* pFramework) override;
	virtual bool                   CompleteInit() override { return true; };
	virtual void                   Shutdown() override;
	virtual i32                    Update(bool haveFocus, u32 updateFlags) override;
	virtual void                   EditorResetGame(bool bStart) override   {}
	virtual void                   PlayerIdSet(EntityId playerId) override {}
	virtual IGameFramework*        GetIGameFramework() override            { return m_pGameFramework; }
	virtual tukk            GetLongName() override;
	virtual tukk            GetName() override;
	virtual void                   GetMemoryStatistics(ICrySizer* s) override;
	virtual void                   OnClearPlayerIds() override                                                                                         {}
	virtual IGame::TSaveGameName   CreateSaveGameName() override                                                                                       { return TSaveGameName(); }
	virtual tukk            GetMappedLevelName(tukk levelName) const override                                                            { return ""; }
	virtual IGameStateRecorder*    CreateGameStateRecorder(IGameplayListener* pL) override                                                             { return nullptr; }
	virtual const bool             DoInitialSavegame() const override                                                                                  { return true; }
	virtual void                   LoadActionMaps(tukk filename) override                                                                       {}
	virtual u32                 AddGameWarning(tukk stringId, tukk paramMessage, IGameWarningsListener* pListener = nullptr) override { return 0; }
	virtual void                   RenderGameWarnings() override                                                                                       {}
	virtual void                   RemoveGameWarning(tukk stringId) override                                                                    {}
	virtual bool                   GameEndLevel(tukk stringId) override                                                                         { return false; }
	virtual void                   OnRenderScene(const SRenderingPassInfo& passInfo) override                                                          {}
	virtual void                   RegisterGameFlowNodes() override;
	virtual void                   FullSerialize(TSerialize ser) override                                                                              {}
	virtual void                   PostSerialize() override                                                                                            {}
	virtual IGame::ExportFilesInfo ExportLevelData(tukk levelName, tukk missionName) const override                                      { return IGame::ExportFilesInfo(levelName, 0); }
	virtual void                   LoadExportedLevelData(tukk levelName, tukk missionName) override                                      {}
	virtual IGamePhysicsSettings*  GetIGamePhysicsSettings() override                                                                                  { return nullptr; }
	virtual void                   InitEditor(IGameToEditorInterface* pGameToEditor) override                                                          {}
	virtual uk                  GetGameInterface() override                                                                                         { return nullptr; }
	// ~IGame

private:
	IGameFramework* m_pGameFramework;
};
