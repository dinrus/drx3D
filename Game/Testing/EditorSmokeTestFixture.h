// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef GAME_TESTING_EDITOR_SMOCK_TEST_FIXTURE_H
#define GAME_TESTING_EDITOR_SMOCK_TEST_FIXTURE_H

#include <drx3D/Game/BinariesPathHelper.h>

struct ISystem;
class CEditorGame;

namespace GameTesting
{
	DRX_TEST_FIXTURE(EditorSmokeTestFixture, DrxUnit::ITestFixture, EditorSmokeTestSuite)
    {
    public:
        void SetUp();
        void TearDown();

    private:
        void CreateSystem(SSysInitParams& initParams);
        HMODULE LoadSystemLibrary(const SSysInitParams& systemInit);
        static ISystem* g_system;

    protected:
		BinariesPathHelper m_pathHelper;
        static CEditorGame* g_editorGame;
    };

}

#endif
