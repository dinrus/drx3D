// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameRules.h>


using namespace GameTesting;

//--------------------------------------------------------------------------------
DRX_TEST_FIXTURE(GameRulesTestFixture, GameBaseTestFixture, GameTestSuite)
{
	CGameRules &GetGameRules() { return dummyGameRules; }

private:

	CGameRules dummyGameRules;
};

//--------------------------------------------------------------------------------
DRX_TEST_WITH_FIXTURE(TestGameRulesOnCollision, GameRulesTestFixture)
{
	IGameRules::SGameCollision dummyEvent;
	dummyEvent.pCollision = NULL;

	ASSERT_IS_TRUE(GetGameRules().OnCollision(dummyEvent));
}
