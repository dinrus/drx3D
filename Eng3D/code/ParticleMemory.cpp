// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ParticleMemory.cpp
//  Version:     v1.00
//  Created:     18/03/2010 by Corey Spink
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Всё необходимое для памяти системы частиц.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/ParticleMemory.h>
#include <drx3D/Eng3D/Particle.h>
#include <drx3D/Eng3D/DinrusX3dEngBase.h>

bool g_bParticleObjectPoolInitialized = false;
DRX_ALIGN(128) char sStorageParticleObjectPool[sizeof(ParticleObjectPool)];
DrxCriticalSection g_ParticlePoolInitLock;

///////////////////////////////////////////////////////////////////////////////
ParticleObjectPool& ParticleObjectAllocator()
{
	IF (g_bParticleObjectPoolInitialized == false, 0)
	{
		AUTO_LOCK(g_ParticlePoolInitLock);
		IF (g_bParticleObjectPoolInitialized == false, 0)
		{
			new(sStorageParticleObjectPool) ParticleObjectPool();
			alias_cast<ParticleObjectPool*>(sStorageParticleObjectPool)->Init(DinrusX3dEngBase::GetCVars()->e_ParticlesPoolSize << 10);
			MemoryBarrier();
			g_bParticleObjectPoolInitialized = true;
		}
	}

	return *alias_cast<ParticleObjectPool*>(sStorageParticleObjectPool);
}
