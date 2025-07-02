// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id: DummyPlayer.cpp$
$DateTime$
Описание: A dummy player used to simulate a client player for profiling purposes

-------------------------------------------------------------------------
История:
- 01/07/2010 11:15:00: Created by Martin Sherburn

*************************************************************************/

#include <drx3D/Game/StdAfx.h>

#if (USE_DEDICATED_INPUT)

#include <drx3D/Game/DummyPlayer.h>
#include <drx3D/Game/NetPlayerInput.h>
#include <drx3D/Game/Weapon.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/Game/GameRules.h>

CDummyPlayer::CDummyPlayer()
{
	m_netPlayerProgression.SetRandomValues();
}

CDummyPlayer::~CDummyPlayer()
{
}

bool CDummyPlayer::Init( IGameObject * pGameObject )
{
	bool bResult = CPlayer::Init(pGameObject);
	m_isPlayer = true;
	return bResult;
}

void CDummyPlayer::Update(SEntityUpdateContext& ctx, i32 updateSlot)
{
	CPlayer::Update(ctx, updateSlot);

	if(!gEnv->bServer)
		return;

	if (!GetPlayerInput() || (GetPlayerInput()->GetType() != IPlayerInput::DEDICATED_INPUT))
	{
		CDedicatedInput* const pDedicatedInput = new CDedicatedInput(this);

		pDedicatedInput->GiveItems();
		
		m_pPlayerInput.reset(pDedicatedInput);
	}
}

EDefaultableBool CDummyPlayer::GetFire()
{
	CDedicatedInput* pPlayerInput = (CDedicatedInput*)GetPlayerInput();
	if (pPlayerInput)
	{
		return pPlayerInput->GetFire();
	}
	return eDB_Default;
}

void CDummyPlayer::SetFire(EDefaultableBool value)
{
	CDedicatedInput* pPlayerInput = (CDedicatedInput*)GetPlayerInput();
	if (pPlayerInput)
	{
		pPlayerInput->SetFire(value);
	}
}

EDefaultableBool CDummyPlayer::GetMove()
{
	CDedicatedInput* pPlayerInput = (CDedicatedInput*)GetPlayerInput();
	if (pPlayerInput)
	{
		return pPlayerInput->GetMove();
	}
	return eDB_Default;
}

void CDummyPlayer::SetMove(EDefaultableBool value)
{
	CDedicatedInput* pPlayerInput = (CDedicatedInput*)GetPlayerInput();
	if (pPlayerInput)
	{
		pPlayerInput->SetMove(value);
	}
}

void CDummyPlayer::OnChangeTeam()
{
	SetMultiplayerModelName();
	FullyUpdateActorModel();
}

#endif //USE_DEDICATED_INPUT
