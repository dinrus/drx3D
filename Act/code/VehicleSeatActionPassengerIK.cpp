// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a seat action to handle IK on the passenger body

   -------------------------------------------------------------------------
   История:
   - 10:01:2006: Created by Mathieu Pinard

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/IActorSystem.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Act/Vehicle.h>
#include <drx3D/Act/VehicleSeatActionPassengerIK.h>

#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>

//------------------------------------------------------------------------
bool CVehicleSeatActionPassengerIK::Init(IVehicle* pVehicle, IVehicleSeat* pSeat, const CVehicleParams& table)
{
	m_pVehicle = pVehicle;
	m_passengerId = 0;

	CVehicleParams ikTable = table.findChild("PassengerIK");
	if (!ikTable)
		return false;

	CVehicleParams limbsTable = ikTable.findChild("Limbs");
	if (!limbsTable)
		return false;

	if (!ikTable.getAttr("waitShortlyBeforeStarting", m_waitShortlyBeforeStarting))
		m_waitShortlyBeforeStarting = false;

	i32 i = 0;
	i32 c = limbsTable.getChildCount();
	m_ikLimbs.reserve(c);

	for (; i < c; i++)
	{
		if (CVehicleParams limbTable = limbsTable.getChild(i))
		{
			SIKLimb limb;

			if (limbTable.haveAttr("limb"))
			{
				limb.limbName = limbTable.getAttr("limb");
				if (limbTable.haveAttr("helper"))
				{
					if (limb.pHelper = m_pVehicle->GetHelper(limbTable.getAttr("helper")))
						m_ikLimbs.push_back(limb);
				}
			}
		}
	}

	return !m_ikLimbs.empty();
}

//------------------------------------------------------------------------
void CVehicleSeatActionPassengerIK::Reset()
{

}

//------------------------------------------------------------------------
void CVehicleSeatActionPassengerIK::StartUsing(EntityId passengerId)
{
	m_passengerId = passengerId;

	m_pVehicle->SetObjectUpdate(this, IVehicle::eVOU_Visible);
}

//------------------------------------------------------------------------
void CVehicleSeatActionPassengerIK::StopUsing()
{
	m_passengerId = 0;

	m_pVehicle->SetObjectUpdate(this, IVehicle::eVOU_NoUpdate);
}

//------------------------------------------------------------------------
void CVehicleSeatActionPassengerIK::Update(float frameTime)
{
	if (!m_passengerId)
		return;

	IActor* pActor = CDrxAction::GetDrxAction()->GetIActorSystem()->GetActor(m_passengerId);
	if (!pActor)
	{
		DRX_ASSERT(!"Invalid entity id for the actor id, Actor System didn't know it");
		return;
	}

	if (ICharacterInstance* pCharInstance = pActor->GetEntity()->GetCharacter(0))
	{
		ISkeletonAnim* pSkeletonAnim = pCharInstance->GetISkeletonAnim();
		DRX_ASSERT(pSkeletonAnim);

		if (pSkeletonAnim->GetNumAnimsInFIFO(0) >= 1)
		{
			const CAnimation& anim = pSkeletonAnim->GetAnimFromFIFO(0, 0);
			u16k loopCount = anim.GetLoop();
			const f32 animationTime = pSkeletonAnim->GetAnimationNormalizedTime(&anim);
			if (!m_waitShortlyBeforeStarting || (loopCount > 0 || animationTime > 0.9f))
			{
				for (TIKLimbVector::iterator ite = m_ikLimbs.begin(); ite != m_ikLimbs.end(); ++ite)
				{
					const SIKLimb& limb = *ite;

					Vec3 helperPos = limb.pHelper->GetWorldSpaceTranslation();
					pActor->SetIKPos(limb.limbName.c_str(), helperPos, 1);
				}
			}
		}
	}

}

void CVehicleSeatActionPassengerIK::GetMemoryUsage(IDrxSizer* s) const
{
	s->Add(*this);
	s->AddObject(m_ikLimbs);
}

DEFINE_VEHICLEOBJECT(CVehicleSeatActionPassengerIK);
