// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   GameCodeCoverageTracker.cpp
//  Created:     18/06/2008 by Matthew
//  Описание: Defines code coverage check points
//               and a central class to track their registration
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageTracker.h>
#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageUpr.h>

#if ENABLE_GAME_CODE_COVERAGE

CGameCodeCoverageCheckPoint::CGameCodeCoverageCheckPoint( tukk  label ) : m_nCount(0), m_psLabel(label)
{
	assert(label);
	CGameCodeCoverageUpr::GetInstance()->Register(this);
}

void CGameCodeCoverageCheckPoint::Touch()
{
	++ m_nCount;
	CGameCodeCoverageUpr::GetInstance()->Hit(this);
}

#endif