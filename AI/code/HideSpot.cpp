// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   HideSpot.cpp
   $Id$
   Описание: Hidespot-related structures

   -------------------------------------------------------------------------
   История:

 *********************************************************************/

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/HideSpot.h>

SHideSpot::SHideSpot() : pAnchorObject(0), pObstacle(0), entityId(0)
{
}

SHideSpot::SHideSpot(SHideSpotInfo::EHideSpotType type, const Vec3& pos, const Vec3& dir)
	: info(type, pos, dir), pAnchorObject(0), pObstacle(0), entityId(0)
{
}

bool SHideSpot::IsSecondary() const
{
	switch (info.type)
	{
	case SHideSpotInfo::eHST_TRIANGULAR:
		return pObstacle && !pObstacle->IsCollidable();
	case SHideSpotInfo::eHST_ANCHOR:
		return pAnchorObject && (pAnchorObject->GetType() == AIANCHOR_COMBAT_HIDESPOT_SECONDARY);
	}

	return false;
}
