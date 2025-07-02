// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
-------------------------------------------------------------------------
Имя файла:   GameCodeCoverageGUI.h
$Id$
Описание: 

-------------------------------------------------------------------------
//  История:     Tim Furnish, 11/11/2009:
//               Moved into game DLL from AI system
//               Wrapped contents in ENABLE_GAME_CODE_COVERAGE
*********************************************************************/

#ifndef __GAME_CODE_COVERAGE_GUI_H_
#define __GAME_CODE_COVERAGE_GUI_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageEnabled.h>
#include <drx3D/Game/GameMechanismBase.h>

#if ENABLE_GAME_CODE_COVERAGE

class CGameCodeCoverageGUI : public CGameMechanismBase
{
public:		// Construction & destruction
	CGameCodeCoverageGUI(void);
	~CGameCodeCoverageGUI(void);

	static ILINE CGameCodeCoverageGUI * GetInstance()
	{
		return s_instance;
	}

public:		// Operations
	void Draw();

private:	// Member data
	static CGameCodeCoverageGUI * s_instance;

	virtual void Update(float dt) {}

	i32 m_showListWhenNumUnhitCheckpointsIs;
};

#endif	// ENABLE_GAME_CODE_COVERAGE

#endif	// __GAME_CODE_COVERAGE_GUI_H_