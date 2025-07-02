// Разработка 2018-2023 DinrusPro / Dinrus Group. All rights reserved.

#include "StdAfx.h"
#include "GameStartup.h"
#include <drx3D/CoreX/Platform/platform_impl.inl>
#include <drx3D/CoreX/Platform/CryLibrary.h>
#include <CryInput/IHardwareMouse.h>
#include "game/Game.h"

#if ENABLE_AUTO_TESTER
static CAutoTester s_autoTesterSingleton;
#endif

#if defined(_LIB)
extern "C" IGameFramework * CreateGameFramework();
#endif

#define DLL_INITFUNC_CREATEGAME "CreateGameFramework"

#if DRX_PLATFORM_WINDOWS
void debugLogCallStack()
{
	// Print call stack for each find.
	tukk funcs[32];
	i32 nCount = 32;

	CryLogAlways("    ----- CallStack () -----");
	gEnv->pSystem->debug_GetCallStack(funcs, nCount);
	for (i32 i = 1; i < nCount; i++) // start from 1 to skip this function.
	{
		CryLogAlways("    %02d) %s", i, funcs[i]);
	}
}
#endif

void GameStartupErrorObserver::OnAssert(tukk condition, tukk message, tukk fileName, u32 fileLineNumber)
{
}

void GameStartupErrorObserver::OnFatalError(tukk message)
{
	CryLogAlways("---FATAL ERROR: message:%s", message);

#if DRX_PLATFORM_WINDOWS
	gEnv->pSystem->debug_LogCallStack();
	CryLogAlways("----------------------------------------");
#endif
}

//////////////////////////////////////////////////////////////////////////

CGameStartup* CGameStartup::Create()
{
	static char buff[sizeof(CGameStartup)];
	return new(buff) CGameStartup();
}

CGameStartup::CGameStartup()
	: m_pGame(nullptr),
	m_gameRef(&m_pGame),
	m_quit(false),
	m_gameDll(0),
	m_frameworkDll(nullptr),
	m_pFramework(nullptr),
	m_fullScreenCVarSetup(false)
{
}

CGameStartup::~CGameStartup()
{
	if (m_pGame)
	{
		m_pGame->Shutdown();
		m_pGame = nullptr;
	}

	if (m_gameDll)
	{
		CryFreeLibrary(m_gameDll);
		m_gameDll = 0;
	}

	ShutdownFramework();
}

IGameRef CGameStartup::Init(SSysInitParams& startupParams)
{
	if (!InitFramework(startupParams))
	{
		return nullptr;
	}

	startupParams.pSystem = GetISystem();
	IGameRef pOut = Reset();

	if (!m_pFramework->CompleteInit())
	{
		pOut->Shutdown();
		return nullptr;
	}

	if (startupParams.bExecuteCommandLine)
		GetISystem()->ExecuteCommandLine();

	GetISystem()->GetISystemEventDispatcher()->RegisterListener(this);
	GetISystem()->RegisterErrorObserver(&m_errorObsever);

#if defined(DRX_UNIT_TESTING)
	#if defined(_LIB)
	CryUnitTest::Test* pTest = CryUnitTest::Test::m_pFirst;
	for (; pTest != 0; pTest = pTest->m_pNext)
	{
		CryUnitTest::IUnitTestUpr* pTestUpr = GetISystem()->GetITestSystem()->GetIUnitTestUpr();
		if (pTestUpr)
		{
			pTest->m_unitTestInfo.module = "StaticBinary";
			pTestUpr->CreateTest(pTest->m_unitTestInfo);
		}
	}
	#endif

	CryUnitTest::IUnitTestUpr* pTestUpr = GetISystem()->GetITestSystem()->GetIUnitTestUpr();
	if (pTestUpr)
	{
		const ICmdLineArg* pSkipUnitTest = GetISystem()->GetICmdLine()->FindArg(eCLAT_Pre, "skip_unit_tests");
		if (!pSkipUnitTest)
		{
			const ICmdLineArg* pUseUnitTestExcelReporter = GetISystem()->GetICmdLine()->FindArg(eCLAT_Pre, "use_unit_test_excel_reporter");
			if (pUseUnitTestExcelReporter)
			{
				GetISystem()->GetITestSystem()->GetIUnitTestUpr()->RunAllTests(CryUnitTest::ExcelReporter);
			}
			else
			{
				GetISystem()->GetITestSystem()->GetIUnitTestUpr()->RunAllTests(CryUnitTest::MinimalReporter);
			}
		}
	}
#endif

	return pOut;
}

IGameRef CGameStartup::Reset()
{
	static char pGameBuffer[sizeof(CGame)];
	m_pGame = new((uk )pGameBuffer)CGame();

	if (m_pGame->Init(m_pFramework))
	{
		return m_gameRef;
	}

	return nullptr;
}

void CGameStartup::Shutdown()
{
	this->~CGameStartup();
}

i32 CGameStartup::Update(bool haveFocus, u32 updateFlags)
{
	// The frame profile system already creates an "overhead" profile label
	// in StartFrame(). Hence we have to set the FRAMESTART before.
	DRX_PROFILE_FRAMESTART("Main");

#if defined(JOBMANAGER_SUPPORT_PROFILING)
	gEnv->GetJobUpr()->SetFrameStartTime(gEnv->pTimer->GetAsyncTime());
#endif

	i32 returnCode = 0;

	if (gEnv->pConsole)
	{
#if DRX_PLATFORM_WINDOWS
		if (gEnv && gEnv->pRenderer && gEnv->pRenderer->GetHWND())
		{
			bool focus = (::GetFocus() == gEnv->pRenderer->GetHWND());
			static bool focused = focus;
			if (focus != focused)
			{
				if (GetISystem()->GetISystemEventDispatcher())
				{
					GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_CHANGE_FOCUS, focus, 0);
				}
				focused = focus;
			}
		}
#endif
	}

	if (m_pGame)
	{
		returnCode = m_pGame->Update(haveFocus, updateFlags);
	}

	if (!m_fullScreenCVarSetup && gEnv->pConsole)
	{
		ICVar* pVar = gEnv->pConsole->GetCVar("r_Fullscreen");
		if (pVar)
		{
			pVar->SetOnChangeCallback(FullScreenCVarChanged);
			m_fullScreenCVarSetup = true;
		}
	}

#if ENABLE_AUTO_TESTER
	s_autoTesterSingleton.Update();
#endif

	return returnCode;
}

void CGameStartup::FullScreenCVarChanged(ICVar* pVar)
{
	if (GetISystem()->GetISystemEventDispatcher())
	{
		GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_TOGGLE_FULLSCREEN, pVar->GetIVal(), 0);
	}
}

bool CGameStartup::GetRestartLevel(tuk* levelName)
{
	if (GetISystem()->IsRelaunch())
		*levelName = (tuk)(gEnv->pGame->GetIGameFramework()->GetLevelName());
	return GetISystem()->IsRelaunch();
}

i32 CGameStartup::Run(tukk autoStartLevelName)
{
	gEnv->pConsole->ExecuteString("exec autoexec.cfg");

	if (autoStartLevelName)
	{
		//load savegame
		if (CryStringUtils::stristr(autoStartLevelName, DRX_SAVEGAME_FILE_EXT) != 0)
		{
			CryFixedStringT<256> fileName(autoStartLevelName);
			// NOTE! two step trimming is intended!
			fileName.Trim(" ");  // first:  remove enclosing spaces (outside ")
			fileName.Trim("\""); // second: remove potential enclosing "
			gEnv->pGame->GetIGameFramework()->LoadGame(fileName.c_str());
		}
		else  //start specified level
		{
			CryFixedStringT<256> mapCmd("map ");
			mapCmd += autoStartLevelName;
			gEnv->pConsole->ExecuteString(mapCmd.c_str());
		}
	}
	else
	{
		gEnv->pConsole->ExecuteString("map example");
	}

#if DRX_PLATFORM_WINDOWS
	if (!(gEnv && gEnv->pSystem) || (!gEnv->IsEditor() && !gEnv->IsDedicated()))
	{
		::ShowCursor(FALSE);
		if (GetISystem()->GetIHardwareMouse())
			GetISystem()->GetIHardwareMouse()->DecrementCounter();
	}
#else
	if (gEnv && gEnv->pHardwareMouse)
		gEnv->pHardwareMouse->DecrementCounter();
#endif

#if !DRX_PLATFORM_DURANGO
	for (;; )
	{
		if (!Update(true, 0))
		{
			break;
		}
	}
#endif

	return 0;
}

void CGameStartup::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_RANDOM_SEED:
		drx_random_seed(gEnv->bNoRandomSeed ? 0 : (u32)wparam);
		break;
	case ESYSTEM_EVENT_FAST_SHUTDOWN:
		m_quit = true;
		break;
	}
}

bool CGameStartup::InitFramework(SSysInitParams& startupParams)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Init Game Framework");

#if !defined(_LIB)
	m_frameworkDll = GetFrameworkDLL(startupParams.szBinariesDir);

	if (!m_frameworkDll)
	{
		CryFatalError("Failed to open the GameFramework DLL!");
		return false;
	}

	IGameFramework::TEntryFunction CreateGameFramework = (IGameFramework::TEntryFunction)CryGetProcAddress(m_frameworkDll, DLL_INITFUNC_CREATEGAME);

	if (!CreateGameFramework)
	{
		CryFatalError("Specified GameFramework DLL is not valid!");
		return false;
	}
#endif

	m_pFramework = CreateGameFramework();

	if (!m_pFramework)
	{
		CryFatalError("Failed to create the GameFramework Interface!");
		return false;
	}

	if (!m_pFramework->Init(startupParams))
	{
		CryFatalError("Failed to initialize CryENGINE!");
		return false;
	}

	ModuleInitISystem(m_pFramework->GetISystem(), GAME_NAME);

#if DRX_PLATFORM_WINDOWS
	if (gEnv->pRenderer)
	{
		SetWindowLongPtr(reinterpret_cast<HWND>(gEnv->pRenderer->GetHWND()), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}
#endif

	return true;
}

void CGameStartup::ShutdownFramework()
{
	if (m_pFramework)
	{
		m_pFramework->Shutdown();
		m_pFramework = nullptr;
	}
}
