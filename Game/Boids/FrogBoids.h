// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   bugsflock.h
//  Version:     v1.00
//  Created:     11/4/2003 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __Frogboids_h__
#define __Frogboids_h__
#pragma once

#include <drx3D/Game/Flock.h>
#include <drx3D/Game/BoidBird.h>

//////////////////////////////////////////////////////////////////////////
class CFrogBoid : public CBoidBird
{
public:
	CFrogBoid( SBoidContext &bc );
	~CFrogBoid();
	virtual void Update( float dt,SBoidContext &bc );
	virtual void Think( float dt,SBoidContext &bc );
	virtual void Kill( const Vec3 &hitPoint,const Vec3 &force );
	virtual void Physicalize( SBoidContext &bc );
	virtual void OnPickup( bool bPickup,float fSpeed );
	virtual void OnCollision( SEntityEvent &event );

protected:

	void OnEnteringWater(const SBoidContext &bc);
	void UpdateWaterInteraction(const SBoidContext &bc);

	float m_maxIdleTime;
	Vec3 m_avoidanceAccel;
	float m_fTimeToNextJump;
	unsigned m_bThrown : 1;
	unsigned m_onGround : 1;

	bool	m_bIsInsideWater;
	HSCRIPTFUNCTION m_pOnWaterSplashFunc;
};

//////////////////////////////////////////////////////////////////////////
// Frog Flock, is a specialized flock type for Frogs.
//////////////////////////////////////////////////////////////////////////
class CFrogFlock : public CFlock
{
public:
	CFrogFlock( IEntity *pEntity );
	virtual void CreateBoids( SBoidsCreateContext &ctx );
	virtual bool CreateEntities();
};

#endif // __Frogboids_h__
