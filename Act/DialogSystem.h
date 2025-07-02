// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DialogSystem.h
//  Version:     v1.00
//  Created:     07/07/2006 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Dialog System
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DIALOGSYSTEM_H__
#define __DIALOGSYSTEM_H__

#pragma once

#include <drx3D/Network/SerializeFwd.h>

#include <drx3D/Act/DialogScript.h>
#include <drx3D/CoreX/Audio/Dialog/IDialogSystem.h>
#include <drx3D/Act/ILevelSystem.h>
#include <drx3D/Act/DialogQueuesUpr.h>

class CDialogSession;
class CDialogActorContext;

#ifndef _RELEASE
	#define DEBUGINFO_DIALOGBUFFER
#endif

class CDialogSystem : public IDialogSystem, ILevelSystemListener
{
public:
	typedef i32 SessionID;
	typedef i32 ActorContextID;

	CDialogSystem();
	virtual ~CDialogSystem();

	void GetMemoryStatistics(IDrxSizer* s);

	// Later go into IDialogSystem i/f
	virtual void Update(const float dt);
	virtual void Shutdown();
	virtual void Serialize(TSerialize ser);   // serializes load/save. After load serialization PostLoad needs to be called

	// IDialogSystem
	virtual bool                     Init();
	virtual void                     Reset(bool bUnload);
	virtual IDialogScriptIteratorPtr CreateScriptIterator();
	virtual bool                     ReloadScripts(tukk levelName = NULL);
	// ~IDialogSystem

	// ILevelSystemListener
	virtual void OnLevelNotFound(tukk levelName)                    {};
	virtual void OnLoadingStart(ILevelInfo* pLevel)                        {};
	virtual void OnLoadingLevelEntitiesStart(ILevelInfo* pLevelInfo)       {}
	virtual void OnLoadingComplete(ILevelInfo* pLevel);
	virtual void OnLoadingError(ILevelInfo* pLevel, tukk error)     {};
	virtual void OnLoadingProgress(ILevelInfo* pLevel, i32 progressAmount) {};
	virtual void OnUnloadComplete(ILevelInfo* pLevel)                      {};
	// ~ILevelSystemListener

	SessionID            CreateSession(const string& scriptID);
	bool                 DeleteSession(SessionID id);
	CDialogSession*      GetSession(SessionID id) const;
	CDialogSession*      GetActiveSession(SessionID id) const;
	CDialogActorContext* GetActiveSessionActorContext(ActorContextID id) const;
	const CDialogScript* GetScriptByID(const string& scriptID) const;

	virtual bool         IsEntityInDialog(EntityId entityId) const;
	bool                 FindSessionAndActorForEntity(EntityId entityId, SessionID& outSessionID, CDialogScript::TActorID& outActorId) const;

	// called from CDialogSession
	bool AddSession(CDialogSession* pSession);
	bool RemoveSession(CDialogSession* pSession);

	// Debug dumping
	void                  Dump(i32 verbosity = 0);
	void                  DumpSessions();

	CDialogQueuesUpr* GetDialogQueueUpr() { return &m_dialogQueueUpr; }

	static i32    sDiaLOGLevel;       // CVar ds_LogLevel
	static i32    sPrecacheSounds;    // CVar ds_PrecacheSounds
	static i32    sAutoReloadScripts; // CVar to reload scripts when jumping into GameMode
	static i32    sLoadSoundSynchronously;
	static i32    sLoadExcelScripts; // CVar to load legacy Excel based Dialogs
	static i32    sWarnOnMissingLoc; // CVar ds_WarnOnMissingLoc
	static ICVar* ds_LevelNameOverride;

protected:
	void            ReleaseScripts();
	void            ReleaseSessions();
	void            ReleasePendingDeletes();
	void            RestoreSessions();
	CDialogSession* InternalCreateSession(const string& scriptID, SessionID sessionID);

protected:
	class CDialogScriptIterator;
	typedef std::map<SessionID, CDialogSession*> TDialogSessionMap;
	typedef std::vector<CDialogSession*>         TDialogSessionVec;

	i32                    m_nextSessionID;
	TDialogScriptMap       m_dialogScriptMap;
	TDialogSessionMap      m_allSessions;
	TDialogSessionVec      m_activeSessions;
	TDialogSessionVec      m_pendingDeleteSessions;
	std::vector<SessionID> m_restoreSessions;
	CDialogQueuesUpr   m_dialogQueueUpr;
};

#endif
