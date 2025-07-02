// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FLY_HELPERS__TACTICAL__H__
#define __FLY_HELPERS__TACTICAL__H__

#include <drx3D/AI/AIPlayer.h>

namespace FlyHelpers
{
ILINE bool IsVisible(CPipeUser* pPipeUser, IEntity* pTargetEntity)
{
	DRX_ASSERT(pPipeUser);
	DRX_ASSERT(pTargetEntity);

	const IAIObject* pTargetAiObject = pTargetEntity->GetAI();
	IF_UNLIKELY (!pTargetAiObject)
	{
		// This case is not properly handled because we don't care about it for the moment.
		return false;
	}

	const IVisionMap* pVisionMap = gEnv->pAISystem->GetVisionMap();
	DRX_ASSERT(pVisionMap);

	const VisionID observerVisionId = pPipeUser->GetVisionID();
	const VisionID targetVisitonId = pTargetAiObject->GetVisionID();
	const bool hasLineOfSight = pVisionMap->IsVisible(observerVisionId, targetVisitonId);

	bool isCloaked = false;
	const CAIPlayer* pTargetAiPlayer = pTargetAiObject->CastToCAIPlayer();
	if (pTargetAiPlayer)
	{
		isCloaked = pTargetAiPlayer->IsInvisibleFrom(pPipeUser->GetPos(), true, true);
	}

	const bool isVisible = (hasLineOfSight && !isCloaked);
	return isVisible;
}
}

#endif
