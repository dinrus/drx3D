// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ParticleMemory.h
//  Version:     v1.00
//  Created:     18/03/2010 by Corey Spink
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: All the particle system's specific memory needs
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __particlememory_h__
#define __particlememory_h__
#pragma once

#include <drx3D/Eng3D/ParticleFixedSizeElementPool.h>

class ParticleObjectPool;
ParticleObjectPool& ParticleObjectAllocator();

#endif // __particlememory_h__
