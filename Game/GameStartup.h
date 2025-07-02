// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __GAMESTARTUP_H__
#define __GAMESTARTUP_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Sys/IWindowMessageHandler.h>

#define GAME_FRAMEWORK_FILENAME DrxLibraryDefName("DinrusXAct")

// implemented in GameDll.cpp
extern HMODULE GetFrameworkDLL(tukk dllLocalDir);

class CGameStartupStatic;

#if defined(CVARS_WHITELIST)
class CCVarsWhiteList : public ICVarsWhitelist
{
public:
	// ICVarsWhiteList
	bool IsWhiteListed(const string& command, bool silent);
	// ~ICVarsWhiteList

protected:
private:
};
#endif // defined(CVARS_WHITELIST)

class GameStartupErrorObserver : public IErrorObserver
{
	public:
		void OnAssert(tukk condition, tukk message, tukk fileName, u32 fileLineNumber);
		void OnFatalError(tukk message);
};

class CGameStartup :
	public IGameStartup, public ISystemEventListener, public IWindowMessageHandler
{
	friend class CGameStartupStatic; // to have access to m_pFramework and m_reqModName in RequestLoadMod
public:
	virtual IGameRef Init(SSysInitParams &startupParams);
	virtual IGameRef Reset(tukk modName);
	virtual void Shutdown();
	virtual i32 Update(bool haveFocus, u32 updateFlags);
	virtual bool GetRestartLevel(tuk* levelName);
	virtual tukk GetPatch() const;
	virtual bool GetRestartMod(tuk pModNameBuffer, i32 modNameBufferSizeInBytes);
	virtual i32 Run( tukk  autoStartLevelName );
	virtual u8k* GetRSAKey(u32 *pKeySize) const;

	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);

	static CGameStartup* Create();

protected:
	CGameStartup();
	virtual ~CGameStartup();

private:
	bool IsModAvailable(const string& modName);
	void HandleResizeForVOIP(WPARAM wparam);

public:

	bool InitFramework(SSysInitParams &startupParams);
private:
	void ShutdownFramework();

	static void FullScreenCVarChanged( ICVar *pVar );
	
#if DRX_PLATFORM_WINDOWS
	bool HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
#endif

	IGame*   m_pMod;
	IGameRef m_modRef;
	bool     m_quit;
	bool     m_fullScreenCVarSetup;
	int8     m_nVOIPWasActive;
	HMODULE  m_modDll;
	HMODULE  m_frameworkDll;

	string   m_reqModName;
	bool     m_reqModUnload;

	IGameFramework* m_pFramework;
	GameStartupErrorObserver m_errorObsever;
};

class CGameStartupStatic
{
	friend class CGameStartup; // to set and unset g_pGameStartup on construction and destruction of CGameStartup
public:
	static void RequestLoadMod(IConsoleCmdArgs* pCmdArgs);
	static void RequestUnloadMod(IConsoleCmdArgs* pCmdArgs);
	static void ForceCursorUpdate();
private:
	static CGameStartup* g_pGameStartup;
};

#endif //__GAMESTARTUP_H__
