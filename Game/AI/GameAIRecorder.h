// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Описание: Helper for Game-specific items dealing with AI Recorder
  
 -------------------------------------------------------------------------
  История:
  - 17:05:2010: Created by Kevin Kirst, with integrated changes from GameF07

*************************************************************************/

#ifndef __GAMEAIRECORDER_H__
#define __GAMEAIRECORDER_H__

class Agent;


// INCLUDE_GAME_AI_RECORDER - Define to include the game-specific utilities for AI recorder
// INCLUDE_GAME_AI_RECORDER_NET - Define to include the network portion of the Game AI Recorder

#if !defined(_RELEASE) && !defined(DEDICATED_SERVER)
	#define INCLUDE_GAME_AI_RECORDER 1
	// #define INCLUDE_GAME_AI_RECORDER_NET 1
#endif // _RELEASE


// Helper to record a comment inside the AI's recorder
// In:
//	entityId - EntityId of AI whose recorder receives the comment
//	szComment - Comment to add, with additional arguments
void RecordAIComment(EntityId entityId, tukk szComment, ...) PRINTF_PARAMS(2,3);


#ifdef INCLUDE_GAME_AI_RECORDER

#include <drx3D/AI/IAIRecorder.h>

class CGameAIRecorder : public IAIRecorderListener
{
	// For network RMI usage
	friend class CGameRules;

public:
	CGameAIRecorder();

	void Init();
	void Shutdown();
	bool IsRecording() const { return m_bIsRecording; }

	// Use these methods to add bookmarks/comments to AI recorder
	void RequestAIRecordBookmark();
	void RequestAIRecordComment(tukk szComment);

	void RecordLuaComment(const Agent &agent, tukk szComment, ...) const PRINTF_PARAMS(3,4);

	// IAIRecorderListener
	virtual void OnRecordingStart(EAIRecorderMode mode, tukk filename);
	virtual void OnRecordingStop(tukk filename);
	virtual void OnRecordingSaved(tukk filename);
	//~IAIRecorderListener

private:
	void AddRecordBookmark(EntityId requesterId);
	void AddRecordComment(EntityId requesterId, tukk szComment);

	// Remote recording copy
	void OnAddBookmark(const string& sScreenShotPath);
	void CleanupRemoteArchive();
	bool FinalizeRemoteArchive(tukk szRecordingFile);
	bool SendRemoteArchive(tukk szRecordingFile);
	bool AddFileToRemoteArchive(tukk szFile);

private:
	bool m_bIsRecording;
	bool m_bBookmarkAdded;
	IDrxArchive_AutoPtr m_pRemoteArchive;
};

class CGameAIRecorderCVars
{
	friend class CGameAIRecorder;

public:
	static void RegisterCommands();
	static void RegisterVariables();
	static void UnregisterCommands(IConsole* pConsole);
	static void UnregisterVariables(IConsole* pConsole);

	// Console command
	static void CmdAIRecorderAddComment(IConsoleCmdArgs* cmdArgs);

public:
	static tukk ai_remoteRecorder_serverDir;
	static i32 ai_remoteRecorder_enabled;
	static i32 ai_remoteRecorder_onlyBookmarked;
};

#endif //INCLUDE_GAME_AI_RECORDER


#endif //__GAMEAIRECORDER_H__
