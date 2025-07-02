// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#define DRX_SAVEGAME_FILENAME "DRXENGINE"
#define DRX_SAVEGAME_FILE_EXT ".CSF"

#include <drx3D/Sys/ICmdLine.h>
#include <drx3D/Network/INetwork.h>

struct IAIActorProxy;
struct IGameFramework;
struct IGameStateRecorder;
struct IGameAudio;
struct IGameWarningsListener;
struct SGameStartParams;
struct SRenderingPassInfo;
struct IGameToEditorInterface;
struct IGameWebDebugService;
struct IGameplayListener;

// Declare legacy GameDLL as deprecated except for the legacy modules that expose functionality for it
#if !defined(eDrxModule) || (eDrxModule != eDrxM_GameFramework && eDrxModule != eDrxM_LegacyGameDLL     \
	&& eDrxModule != eDrxM_Editor && eDrxModule != eDrxM_FlowGraph && eDrxModule != eDrxM_AudioSystem   \
	&& eDrxModule != eDrxM_3DEngine && eDrxModule != eDrxM_AISystem && eDrxModule != eDrxM_EntitySystem \
	&& eDrxModule != eDrxM_Movie && eDrxModule != eDrxM_System && eDrxModule != eDrxM_Legacy)

	#define DRX_DEPRECATED_GAME_DLL DRX_DEPRECATED("(v5.3) IGame, IEditorGame and IGameStartup have been replaced by IDrxPlugin and will be removed in a future update.")

#else
	#define DRX_DEPRECATED_GAME_DLL
#endif

//! Legacy functionality for the Main interface of a game, replaced with IDrxPlugin (see templates for example implementation)
//! The IGame interface should be implemented in the GameDLL.
//! Game systems residing in the GameDLL can be initialized and updated inside the Game object.
//! \see IEditorGame.
struct IGame
{
	DRX_DEPRECATED_GAME_DLL IGame() = default;

	//! Interface used to communicate what entities/entity archetypes need to be precached.
	//! Game code can further do some data mining to figure out the resources needed for the entities
	struct IResourcesPreCache
	{
		virtual void QueueEntityClass(tukk szEntityClass) = 0;
		virtual void QueueEntityArchetype(tukk szEntityArchetype) = 0;
	};

	struct ExportFilesInfo
	{
		ExportFilesInfo(tukk _baseFileName, u32k _fileCount)
			: m_pBaseFileName(_baseFileName)
			, m_fileCount(_fileCount)
		{
		}

		ILINE u32      GetFileCount() const    { return m_fileCount; }
		ILINE tukk GetBaseFileName() const { return m_pBaseFileName; }

		static void       GetNameForFile(tukk baseFileName, u32k fileIdx, tuk outputName, size_t outputNameSize)
		{
			assert(baseFileName != NULL);
			drx_sprintf(outputName, outputNameSize, "%s_%u", baseFileName, fileIdx);
		}

	private:
		tukk  m_pBaseFileName;
		u32k m_fileCount;
	};

	// <interfuscator:shuffle>
	virtual ~IGame(){}

	//! Initialize the MOD.
	//! The shutdown method, must be called independent of this method's return value.
	//! \param pCmdLine Pointer to the command line interface.
	//! \param pFramework Pointer to the IGameFramework interface.
	//! \return 0 if something went wrong with initialization, non-zero otherwise.
	virtual bool Init(/*IGameFramework* pFramework*/) = 0;

	//! Init editor related things.
	virtual void InitEditor(IGameToEditorInterface* pGameToEditor) = 0;

	virtual void GetMemoryStatistics(IDrxSizer* s) = 0;

	//! Finish initializing the MOD.
	//! Called after the game framework has finished its CompleteInit.
	//! This is the point at which to register game flow nodes etc.
	virtual bool CompleteInit() { return true; };

	//! Shuts down the MOD and delete itself.
	virtual void Shutdown() = 0;

	//! Notify game of pre-physics update.
	virtual void PrePhysicsUpdate() {}

	//! Updates the MOD.
	//! \param haveFocus true if the game has the input focus.
	//! \return 0 to terminate the game (i.e. when quitting), non-zero to continue.
	virtual i32 Update(bool haveFocus, u32 updateFlags) = 0;

	//! Called on the game when entering/exiting game mode in editor
	//! \param bStart true if we enter game mode, false if we exit it.
	virtual void EditorResetGame(bool bStart) = 0;

	//! \return Name of the mode. (e.g. "Capture The Flag").
	virtual tukk GetLongName() = 0;

	//! \return A short description of the mode. (e.g. "dc")
	virtual tukk GetName() = 0;

	//! Loads a specified action map, used mainly for loading the default action map
	virtual void LoadActionMaps(tukk filename) = 0;

	//! \return Pointer to the game framework being used.
	virtual IGameFramework* GetIGameFramework() = 0;

	//! Mapping level filename to "official" name.
	//! \return c_str or NULL.
	virtual tukk GetMappedLevelName(tukk levelName) const = 0;

	//! Add a game warning that is shown to the player
	//! \return A unique handle to the warning or 0 for any error.
	virtual u32 AddGameWarning(tukk stringId, tukk paramMessage, IGameWarningsListener* pListener = NULL) = 0;

	//! Called from 3DEngine in RenderScene, so polygons and meshes can be added to the scene from game
	virtual void OnRenderScene(const SRenderingPassInfo& passInfo) = 0;

	//! Render Game Warnings.
	virtual void RenderGameWarnings() = 0;

	//! Remove a game warning.
	virtual void RemoveGameWarning(tukk stringId) = 0;

	//! Callback to game for game specific actions on level end.
	//! \retval false, if the level end should continue.
	//! \retval true, if the game handles the end level action and calls ScheduleEndLevel directly.
	virtual bool GameEndLevel(tukk stringId) = 0;

	//! Creates a GameStateRecorder instance in GameDll and returns the non-owning pointer to the caller (DinrusXAct/GamePlayRecorder).
	virtual IGameStateRecorder* CreateGameStateRecorder(IGameplayListener* pL) = 0;

	virtual void                FullSerialize(TSerialize ser) = 0;
	virtual void                PostSerialize() = 0;

	//! Editor export interface hook, to allow the game to export its own data into the level paks.
	//! \return Exported file information.
	virtual IGame::ExportFilesInfo ExportLevelData(tukk levelName, tukk missionName) const = 0;

	//! Interface hook to load all game exported data when the level is loaded.
	virtual void LoadExportedLevelData(tukk levelName, tukk missionName) = 0;

	//! Interface hook to sync game exported data from level paks when the level is loaded in editor
	virtual void LoadExportedLevelDataInEditor(tukk szLevelName, tukk szMissionName) {}

	//! Access to game interface.
	virtual uk GetGameInterface() = 0;

	//! Access game specific resource precache interface
	virtual IResourcesPreCache* GetResourceCache() { return nullptr; }

	//! Retrieves IGameWebDebugService for web-socket based remote debugging.
	virtual IGameWebDebugService* GetIWebDebugService() { return nullptr; };
	// </interfuscator:shuffle>
};

//! \endcond