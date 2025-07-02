// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   fishflock.h
//  Version:     v1.00
//  Created:     8/2010 by Luciano Morpurgo (refactored from flock.h)
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual C++ 7.0
//  Описание: 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __fishflock_h__
#define __fishflock_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/AI/IAISystem.h>
#include <drx3D/Game/Flock.h>

//////////////////////////////////////////////////////////////////////////
class CFishFlock : public CFlock
{
public:
	CFishFlock( IEntity *pEntity ) : CFlock( pEntity,EFLOCK_FISH ) { m_boidEntityName = "FishBoid"; m_boidDefaultAnimName = "swim_loop"; };
	virtual void CreateBoids( SBoidsCreateContext &ctx );
};

#endif // __fishflock_h__
