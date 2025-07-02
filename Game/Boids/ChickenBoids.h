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

#ifndef __chickenboids_h__
#define __chickenboids_h__
#pragma once

#include <drx3D/Game/Flock.h>
#include <drx3D/Game/BoidBird.h>

//////////////////////////////////////////////////////////////////////////
class CChickenBoid : public CBoidBird
{
public:
	CChickenBoid( SBoidContext &bc );
	virtual void Update( float dt,SBoidContext &bc );
	virtual void Think( float dt,SBoidContext &bc );
	virtual void Kill( const Vec3 &hitPoint,const Vec3 &force );
	virtual void Physicalize( SBoidContext &bc );
	virtual void OnPickup( bool bPickup,float fSpeed );
	virtual void OnCollision( SEntityEvent &event );
	virtual void CalcOrientation( Quat &qOrient );

protected:
	float m_maxIdleTime;
	float m_maxNonIdleTime;

	Vec3 m_avoidanceAccel;
	uint m_lastRayCastFrame;
	u32 m_bThrown : 1;
	u32 m_bScared : 1;
	u32 m_landing : 1;			//! TODO: replace with m_status

};

//////////////////////////////////////////////////////////////////////////
// Chicken Flock, is a specialized flock type for chickens.
//////////////////////////////////////////////////////////////////////////
class CChickenFlock : public CFlock
{
public:
	CChickenFlock( IEntity *pEntity );
	virtual void CreateBoids( SBoidsCreateContext &ctx );
	virtual CBoidObject* CreateBoid() { return new CChickenBoid(m_bc); };
	virtual void OnAIEvent(EAIStimulusType type, const Vec3& pos, float radius, float threat, EntityId sender);	
};

//////////////////////////////////////////////////////////////////////////
class CTurtleBoid : public CChickenBoid
{
public:
	CTurtleBoid( SBoidContext &bc ) : CChickenBoid(bc) {};
	virtual void Think( float dt,SBoidContext &bc );
};

//////////////////////////////////////////////////////////////////////////
class CTurtleFlock : public CChickenFlock
{
public:
	CTurtleFlock( IEntity *pEntity );
	virtual CBoidObject* CreateBoid() { return new CTurtleBoid(m_bc); };
};

#endif // __chickenboids_h__
