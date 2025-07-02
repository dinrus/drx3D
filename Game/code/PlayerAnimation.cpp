// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание: Helper interface for player animation control

-------------------------------------------------------------------------
История:
- 08.9.11: Created by Tom Berry

*************************************************************************/

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Game/PlayerAnimation.h>

SMannequinPlayerParams PlayerMannequin;

void InitPlayerMannequin( IActionController* pActionController )
{
	DRX_ASSERT( g_pGame );
	IGameFramework* pGameFramework = g_pGame->GetIGameFramework();
	DRX_ASSERT( pGameFramework );
	CMannequinUserParamsUpr& mannequinUserParams = pGameFramework->GetMannequinInterface().GetMannequinUserParamsUpr();

	mannequinUserParams.RegisterParams< SMannequinPlayerParams >( pActionController, &PlayerMannequin );
}

