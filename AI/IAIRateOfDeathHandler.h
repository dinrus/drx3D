// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Interface to apply modification to an AI's handling of the
   Rate of Death balancing system

   -------------------------------------------------------------------------
   История:
   - 07:09:2009: Created by Kevin Kirst

*************************************************************************/

#ifndef __IAI_RATEOFDEATH_HANDLER_H__
#define __IAI_RATEOFDEATH_HANDLER_H__

#include <drx3D/AI/IAgent.h> // <> required for Interfuscator

struct IAIRateOfDeathHandler
{
	// <interfuscator:shuffle>
	virtual ~IAIRateOfDeathHandler() {}

	//! Calculate and return how long the target should stay alive from now.
	virtual float GetTargetAliveTime(const IAIObject* pAI, const IAIObject* pTarget, EAITargetZone eTargetZone, float& fFireDazzleTime) = 0;

	//! Calculate and return how long the AI should take to react to firing at the target from now.
	virtual float GetFiringReactionTime(const IAIObject* pAI, const IAIObject* pTarget, const Vec3& vTargetPos) = 0;

	//! Calculate and return the zone the target is currently in.
	virtual EAITargetZone GetTargetZone(const IAIObject* pAI, const IAIObject* pTarget) = 0;
	// </interfuscator:shuffle>

	void GetMemoryUsage(IDrxSizer* pSizer) const { /*LATER*/ }
};

#endif //__IAI_RATEOFDEATH_HANDLER_H__
