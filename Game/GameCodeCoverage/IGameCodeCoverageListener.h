// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
// -------------------------------------------------------------------------
//
//  Имя файла:   IGameCodeCoverageListener.h
//  Created:     18/06/2008 by Tim Furnish
//  Описание: Interface class for anything which wants to be informed of
//               code coverage checkpoints being hit
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __I_GAME_CODE_COVERAGE_LISTENER_H_
#define __I_GAME_CODE_COVERAGE_LISTENER_H_

#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageEnabled.h>

#if ENABLE_GAME_CODE_COVERAGE

class CGameCodeCoverageCheckPoint;

class IGameCodeCoverageListener
{
	public:
	virtual void InformCodeCoverageCheckpointHit(CGameCodeCoverageCheckPoint * cp) = 0;
};

#endif // ENABLE_GAME_CODE_COVERAGE

#endif // __I_GAME_CODE_COVERAGE_LISTENER_H_