// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements vehicle seat specific Mannequin actions

   -------------------------------------------------------------------------
   История:
   - 06:02:2012: Created by Tom Berry

*************************************************************************/
#include <drx3D/Act/StdAfx.h>

#include <drx3D/Act/Vehicle.h>
#include <drx3D/Act/VehicleSeat.h>

#include <drx3D/Act/VehicleSeatAnimActions.h>

void CVehicleSeatAnimActionEnter::Enter()
{
	BaseAction::Enter();

	bool isThirdPerson = !m_pSeat->ShouldEnterInFirstPerson();

	for (size_t i = 0; i < m_pSeat->GetViewCount(); ++i)
	{
		IVehicleView* pView = m_pSeat->GetView(i + 1);
		if (pView && pView->IsThirdPerson() == isThirdPerson)
		{
			m_pSeat->SetView(i + 1);
			break;
		}
	}

	IActor* pActor = m_pSeat->GetPassengerActor();
	DRX_ASSERT(gEnv->bMultiplayer || pActor);

	IAnimatedCharacter* pAnimChar = pActor ? pActor->GetAnimatedCharacter() : nullptr;
	if (pAnimChar)
	{
		pAnimChar->SetMovementControlMethods(eMCM_Animation, eMCM_Animation);
		pAnimChar->RequestPhysicalColliderMode(eColliderMode_Disabled, eColliderModeLayer_Game, "CVehicleSeatAnimActionEnter::Enter");
	}
}

void CVehicleSeatAnimActionExit::Enter()
{
	BaseAction::Enter();

	bool isThirdPerson = !m_pSeat->ShouldExitInFirstPerson();

	for (size_t i = 0; i < m_pSeat->GetViewCount(); ++i)
	{
		IVehicleView* pView = m_pSeat->GetView(i + 1);
		if (pView && pView->IsThirdPerson() == isThirdPerson)
		{
			m_pSeat->SetView(i + 1);
			break;
		}
	}

	IActor* pActor = m_pSeat->GetPassengerActor();
	DRX_ASSERT(gEnv->bMultiplayer || pActor);

	IAnimatedCharacter* pAnimChar = pActor ? pActor->GetAnimatedCharacter() : nullptr;
	if (pAnimChar)
	{
		pAnimChar->SetMovementControlMethods(eMCM_Animation, eMCM_Animation);
		pAnimChar->RequestPhysicalColliderMode(eColliderMode_Disabled, eColliderModeLayer_Game, "CVehicleSeatAnimActionExit::Enter");
	}
}

void CVehicleSeatAnimActionExit::Exit()
{
	BaseAction::Exit();

	// Leave the heli mid-air and this will trigger.
	IActor* pActor = m_pSeat->GetPassengerActor();
	DRX_ASSERT(gEnv->bMultiplayer || pActor);

	m_pSeat->StandUp();
}
