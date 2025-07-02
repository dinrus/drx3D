// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/Stdafx.h>
#include <drx3D/Game/GameStartup.h>
#include <drx3D/Game/EditorSmokeTestFixture.h>
#include <drx3D/Game/Editor/EditorGame.h>
#include <drx3D/CoreX/Platform/DrxLibrary.h>
#include <drx3D/Entity/IEntitySystem.h>
using namespace GameTesting;

ISystem* EditorSmokeTestFixture::g_system = NULL;
CEditorGame* EditorSmokeTestFixture::g_editorGame = NULL;

#ifdef WIN32

void EditorSmokeTestFixture::CreateSystem(SSysInitParams& initParams)
{
    m_pathHelper.FillBinariesDir(initParams);
    initParams.bEditor = true;

    HMODULE module = LoadSystemLibrary(initParams);
    DRX_ASSERT_MESSAGE(module, "Не удаётся загрузить drx3DSys.dll");

    SetCurrentDirectory(m_pathHelper.GetCurrentDir());
    PFNCREATESYSTEMINTERFACE createSystemInterface = (PFNCREATESYSTEMINTERFACE) GetProcAddress(module, "CreateSystemInterface");
    DRX_ASSERT_MESSAGE(createSystemInterface, "не удаётся наййти CreateSystemInterface");

    g_system = createSystemInterface(initParams);
    DRX_ASSERT_MESSAGE(g_system, "Не удаётся создать ISystem");
    ModuleInitISystem(g_system, "EditorSmokeTestFixture");
}

HMODULE EditorSmokeTestFixture::LoadSystemLibrary(const SSysInitParams& systemInit)
{
    char binFullPath[256];
    strcpy_s(binFullPath, m_pathHelper.GetCurrentDir());
    strcat_s(binFullPath, systemInit.szBinariesDir);
    SetCurrentDirectory(binFullPath);
    return DrxLoadLibrary("drx3DSys.dll");
}

#else

void EditorSmokeTestFixture::CreateSystem(SSysInitParams& initParams)
{

}

HMODULE EditorSmokeTestFixture::LoadSystemLibrary(const SSysInitParams& systemInit)
{
    return NULL;
}

#endif

void EditorSmokeTestFixture::SetUp()
{
    m_pathHelper.CacheCurrDir();

    if (!g_system)
    {
        SSysInitParams initParams;
        CreateSystem(initParams);

        g_editorGame = new CEditorGame(initParams.szBinariesDir);

        struct DummyGameToEditorInterface : public IGameToEditorInterface
        {
            virtual void SetUIEnums(tukk sEnumName, tukk* sStringsArray, i32 nStringCount) {}
        };

        ASSERT_IS_TRUE(g_editorGame->Init(g_system, new DummyGameToEditorInterface()));
    }
}

void EditorSmokeTestFixture::TearDown()
{
    m_pathHelper.RestoreCachedCurrDir();
}

DRX_TEST_WITH_FIXTURE(LoadCXPLevelInEditorMode, EditorSmokeTestFixture)
{
    tukk level = "Game/Levels/Crysis2_CXP";

    g_editorGame->OnBeforeLevelLoad();
    ASSERT_IS_TRUE(gEnv->p3DEngine->InitLevelForEditor( level, "Mission0"));
    g_editorGame->OnAfterLevelLoad(level, level);

    g_editorGame->SetGameMode(true);

    SEntityEvent event;
    event.event = ENTITY_EVENT_RESET;
    event.nParam[0] = 1;
    gEnv->pEntitySystem->SendEventToAll( event );

    event.event = ENTITY_EVENT_LEVEL_LOADED;
    gEnv->pEntitySystem->SendEventToAll( event );

    event.event = ENTITY_EVENT_START_GAME;
    gEnv->pEntitySystem->SendEventToAll( event );
}

