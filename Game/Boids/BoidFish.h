// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   boidfish.h
//  Version:     v1.00
//  Created:     8/2010 by Luciano Morpurgo (refactored from flock.h)
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual C++ 7.0
//  Описание: 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __boidfish_h__
#define __boidfish_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/AI/IAISystem.h>
#include <drx3D/Game/BoidObject.h>

//////////////////////////////////////////////////////////////////////////
//! Boid object with fish behavior.
//////////////////////////////////////////////////////////////////////////

class CBoidFish : public CBoidObject
{
public:
	CBoidFish( SBoidContext &bc );
	~CBoidFish();

	virtual void Update( float dt,SBoidContext &bc );
	virtual void Kill( const Vec3 &hitPoint,const Vec3 &force );
	virtual void Physicalize( SBoidContext &bc );

protected:
	void SpawnParticleEffect( const Vec3 &pos,SBoidContext &bc,i32 nEffect );

	float m_dyingTime; // Deisred height this birds want to fly at.
	SmartScriptTable vec_Bubble;

	enum EScriptFunc {
		SPAWN_BUBBLE,
		SPAWN_SPLASH,
	};
	HSCRIPTFUNCTION m_pOnSpawnBubbleFunc;
	HSCRIPTFUNCTION m_pOnSpawnSplashFunc;
};

#endif // __boidfish_h__
