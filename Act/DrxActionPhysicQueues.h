// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------

   Container for Raycast/Primitive deferred queues

   -------------------------------------------------------------------------
   История:
   - 20:10:20010   Created by Benito G.R.

*************************************************************************/

#ifndef __DRXACTION_PHYSICQUEUES_H__
#define __DRXACTION_PHYSICQUEUES_H__

#pragma once

#include <drx3D/Phys/RayCastQueue.h>
#include <drx3D/Phys/IntersectionTestQueue.h>

class CDrxActionPhysicQueues
{
public:
	typedef RayCastQueue<41>          DrxActionRayCaster;
	typedef IntersectionTestQueue<43> DrxActionIntersectionTester;

	CDrxActionPhysicQueues()
	{
		m_rayCaster.SetQuota(8);
		m_intersectionTester.SetQuota(6);
	}

	DrxActionRayCaster&          GetRayCaster()          { return m_rayCaster; }
	DrxActionIntersectionTester& GetIntersectionTester() { return m_intersectionTester; }

	void                         Update(float frameTime)
	{
		m_rayCaster.Update(frameTime);
		m_intersectionTester.Update(frameTime);
	}

private:
	DrxActionRayCaster          m_rayCaster;
	DrxActionIntersectionTester m_intersectionTester;
};

#endif //__DRXACTION_PHYSICQUEUES_H__
