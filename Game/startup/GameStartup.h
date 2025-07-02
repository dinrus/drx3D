// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CryGame/IGameFramework.h>
#include <drx3D/CoreX/Platform/CryWindows.h>

#define GAME_FRAMEWORK_FILENAME CryLibraryDefName("drx3D_Act")
#define GAME_WINDOW_CLASSNAME   "DRXENGINE"

extern HMODULE GetFrameworkDLL(tukk dllLocalDir);

class GameStartupErrorObserver : public IErrorObserver
{
public:
	// IErrorObserver
	void OnAssert(tukk condition, tukk message, tukk fileName, u32 fileLineNumber) override;
	void OnFatalError(tukk message) override;
	// ~IErrorObserver
};

class CGameStartup : public IGameStartup, public ISystemEventListener
{
public:
	// IGameStartup
	virtual IGameRef    Init(SSysInitParams& startupParams) override;
	virtual void        Shutdown() override;
	virtual i32         Update(bool haveFocus, u32 updateFlags) override;
	virtual bool        GetRestartLevel(tuk* levelName) override;
	virtual tukk GetPatch() const override                              { return nullptr; }
	virtual bool        GetRestartMod(tuk pModName, i32 nameLenMax) override { return false; }
	virtual i32         Run(tukk autoStartLevelName) override;
	// ~IGameStartup

	virtual IGameRef Reset();

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	bool                 InitFramework(SSysInitParams& startupParams);

	static CGameStartup* Create();

protected:
	CGameStartup();
	virtual ~CGameStartup();

private:
	void        ShutdownFramework();

	static void FullScreenCVarChanged(ICVar* pVar);

#if DRX_PLATFORM_WINDOWS
	// IWindowMessageHandler
	bool HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	// ~IWindowMessageHandler
#endif

	IGame*                   m_pGame;
	IGameRef                 m_gameRef;
	bool                     m_quit;
	bool                     m_fullScreenCVarSetup;

	HMODULE                  m_gameDll;
	HMODULE                  m_frameworkDll;

	string                   m_reqModName;

	IGameFramework*          m_pFramework;
	GameStartupErrorObserver m_errorObsever;
};
