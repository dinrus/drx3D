// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   GaneCodeCoverageTracker.h
//  Created:     18/06/2008 by Matthew
//  Описание: Defines code coverage check points
//               and a central class to track their registration
// -------------------------------------------------------------------------
//  История:     Tim Furnish, 11/11/2009:
//               Moved into game DLL from AI system
//               Wrapped contents in ENABLE_GAME_CODE_COVERAGE
//
////////////////////////////////////////////////////////////////////////////

/**
* Design notes:
*   See the CGameCodeCoverageUpr class that acts as the high-level API to the system
*   A (separate) tracking GUI is crucial to this system - an efficient interface to service this is not trivial
*
* Technical notes:
*   The manager would appear to lend itself to a map of names to checkpoint pointers for quick lookup
*   However I really want to keep overhead to a minimum even in registering and I guess we might have 1000 CCCPoints
*   A vector, with some sorting or heapifying might be better when the code matures
*/

#ifndef __GAME_CODE_COVERAGE_TRACKER_H_
#define __GAME_CODE_COVERAGE_TRACKER_H_

#pragma once

#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageEnabled.h>

#if ENABLE_SHARED_CODE_COVERAGE

#define CCCPOINT(x) CODECHECKPOINT(x)
#define CCCPOINT_IF(check, x) if(check) CCCPOINT(x)

#elif ENABLE_GAME_CODE_COVERAGE

#define CCCPOINT(x) do { static CGameCodeCoverageCheckPoint autoReg##x(#x); autoReg##x.Touch(); } while(0)
#define CCCPOINT_IF(check, x) if(check) CCCPOINT(x)

class CGameCodeCoverageCheckPoint
{
	public:
	CGameCodeCoverageCheckPoint( tukk  label );

	void Touch();
	ILINE i32 GetCount() const { return m_nCount; }
	ILINE tukk  GetLabel() const { return m_psLabel; }

	protected:
	i32 m_nCount;
	tukk  m_psLabel;
};

#else

#define CCCPOINT(x) (void)(0)
#define CCCPOINT_IF(check, x) (void)(0)

#endif // ENABLE_GAME_CODE_COVERAGE

#endif // __GAME_CODE_COVERAGE_TRACKER_H_