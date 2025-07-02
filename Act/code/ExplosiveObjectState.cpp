// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  network breakability: state of an object pre-explosion
   -------------------------------------------------------------------------
   История:
   - 22/01/2007   10:34 : Created by Craig Tiller
*************************************************************************/

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/ExplosiveObjectState.h>
#include <drx3D/Act/ObjectSelector.h>
#include <drx3D/Act/DebugBreakage.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/GameContext.h>

void SDeclareExplosiveObjectState::SerializeWith(TSerialize ser)
{
	LOGBREAK("SDeclareExplosiveObjectState: %s", ser.IsReading() ? "Reading:" : "Writing");

	ser.Value("breakId", breakId, 'brId');
	ser.Value("isEnt", isEnt);
	if (isEnt)
	{
		if (ser.IsWriting())
			DRX_ASSERT(CDrxAction::GetDrxAction()->GetGameContext()->GetNetContext()->IsBound(entId));
		ser.Value("entid", entId, 'eid');
		ser.Value("entpos", entPos);
		ser.Value("entrot", entRot);
		ser.Value("entscale", entScale);
	}
	else
	{
		ser.Value("eventPos", eventPos);
		ser.Value("hash", hash);
	}
}

bool ExplosiveObjectStateFromPhysicalEntity(SExplosiveObjectState& s, IPhysicalEntity* pPEnt)
{
	bool ok = false;
	switch (pPEnt->GetiForeignData())
	{
	case PHYS_FOREIGN_ID_ENTITY:
		if (IEntity* pEnt = (IEntity*) pPEnt->GetForeignData(PHYS_FOREIGN_ID_ENTITY))
		{
			s.isEnt = true;
			s.entId = pEnt->GetId();
			s.entPos = pEnt->GetWorldPos();
			s.entRot = pEnt->GetWorldRotation();
			s.entScale = pEnt->GetScale();
			ok = true;
		}
		break;
	case PHYS_FOREIGN_ID_STATIC:
		if (IRenderNode* rn = (IRenderNode*)pPEnt->GetForeignData(PHYS_FOREIGN_ID_STATIC))
		{
			s.isEnt = false;
			s.eventPos = rn->GetBBox().GetCenter();
			s.hash = CObjectSelector::CalculateHash(rn);
			ok = true;
		}
		break;
	}
	return ok;
}
