// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   scriptobjectboids.h
//  Version:     v1.00
//  Created:     17/5/2002 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __scriptobjectboids_h__
#define __scriptobjectboids_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include <drx3D/Script/IScriptSystem.h>

//  предварительные объявления.
class CFlock;
struct SBoidsCreateContext;

/*! <remarks>Provides access to boids flock functionality. These function will never be called from C-Code. They're script-exclusive.</remarks>*/
class CScriptBind_Boids  : public CScriptableBase
{
public:
	CScriptBind_Boids( ISystem *pSystem );
	virtual ~CScriptBind_Boids(void);

	virtual void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));	
	}
	//! <code>Boids.CreateFlock( entity,paramsTable )
	//! <description>Creates a flock of boids and binds it to the given entity.</description>
	//!		<param name="entity">Valid entity table.</param>
	//!		<param name="nType">Type of the flock, can be Boids.FLOCK_BIRDS,Boids.FLOCK_FISH,Boids.FLOCK_BUGS.</param>
	//!		<param name="paramTable">Table with parameters for flock (Look at sample scripts).
	i32 CreateFlock( IFunctionHandler *pH, SmartScriptTable entity,i32 nType,SmartScriptTable paramTable );

	//! <code>Boids.CreateFishFlock( entity,paramsTable )
	//! <description>Creates a fishes flock and binds it to the given entity.</description>
	//!		<param name="entity">Valid entity table.</param>
	//!		<param name="paramTable">Table with parameters for flock (Look at sample scripts).</param>
	i32 CreateFishFlock(IFunctionHandler *pH, SmartScriptTable entity,SmartScriptTable paramTable );

	//! <code>Boids.CreateBugsFlock( entity,paramsTable )
	//! <description>Creates a bugs flock and binds it to the given entity.</description>
	//!		<param name="entity">Valid entity table.</param>
	//!		<param name="paramTable">Table with parameters for flock (Look at sample scripts).</param>
	i32 CreateBugsFlock(IFunctionHandler *pH, SmartScriptTable entity,SmartScriptTable paramTable );

	//! <code>Boids.SetFlockParams( entity,paramsTable )
	//! <description>Modifies parameters of the existing flock in the specified entity.</description>
	//!		<param name="entity">Valid entity table containing flock.</param>
	//!		<param name="paramTable">Table with parameters for flock (Look at sample scripts).</param>
	i32 SetFlockParams(IFunctionHandler *pH, SmartScriptTable entity,SmartScriptTable paramTable );

	//! <code>Boids.EnableFlock( entity,paramsTable )
	//! <description>Enables/Disables flock in the entity.</description>
	//!		<param name="entity">Valid entity table containing flock.</param>
	//!		<param name="bEnable">true to enable or false to disable flock.</param>
	i32 EnableFlock(IFunctionHandler *pH,SmartScriptTable entity,bool bEnable );

	//! <code>Boids.SetFlockPercentEnabled( entity,paramsTable )
	//! <description>
	//! 	Used to gradually enable flock.
	//!    Depending on the percentage more or less boid objects will be rendered in flock.
	//! </description>
	//!		<param name="entity">Valid entity table containing flock.</param>
	//!		<param name="nPercent">In range 0 to 100, 0 mean no boids will be rendered,if 100 then all boids will be rendered.</param>
	i32 SetFlockPercentEnabled(IFunctionHandler *pH,SmartScriptTable entity,i32 nPercent );

	//! <code>Boids.SetAttractionPoint( entity,paramsTable )
	//! <description>Sets the one time attraction point for the boids</description>
	//!		<param name="entity">Valid entity table containing flock.</param>
	//!		<param name="point">The one time attraction point</param>
	i32 SetAttractionPoint(IFunctionHandler *pH,SmartScriptTable entity,Vec3 point );

	//! <code>Boids.OnBoidHit( flockEntity,boidEntity,hit )
	//! <description>Events that occurs on boid hit.</description>
	//!		<param name="flockEntity">Valid entity table containing flock.</param>
	//!		<param name="boidEntity">Valid entity table containing boid.</param>
	//!		<param name="hit">Valid entity table containing hit information.</param>
	i32 OnBoidHit(IFunctionHandler *pH, SmartScriptTable flockEntity, SmartScriptTable boidEntity, SmartScriptTable hit);

	//! <code>Boids.CanPickup( flockEntity, boidEntity )
	//! <description>Checks if the boid is pickable</description>
	//!		<param name="flockEntity">Valid entity table containing flock.</param>
	//!		<param name="boidEntity">Valid entity table containing boid.</param>
	i32 CanPickup(IFunctionHandler *pH, SmartScriptTable flockEntity, SmartScriptTable boidEntity);

	//! <code>Boids.GetUsableMessage( flockEntity )
	//! <description>Gets the appropriate localized UI message for this flock</description>
	//!		<param name="flockEntity">Valid entity table containing flock.</param>
	i32 GetUsableMessage(IFunctionHandler *pH, SmartScriptTable flockEntity);

	//! <code>Boids.OnPickup( flockEntity, boidEntity, bPickup, fThrowSpeed )
	//! <description>Forwards the appropriate pickup action to the boid object</description>
	//!		<param name="flockEntity">Valid entity table containing flock.</param>
	//!		<param name="boidEntity">Valid entity table containing boid.</param>
	//!		<param name="bPickup">Pickup or drop/throw?</param>
	//!		<param name="fThrowSpeed">value > 5.f will kill the boid by default (no effect on pickup action)</param>
	i32 OnPickup(IFunctionHandler *pH, SmartScriptTable flockEntity, SmartScriptTable boidEntity, bool bPickup, float fThrowSpeed);

private:
	bool ReadParamsTable( IScriptTable *pTable, struct SBoidContext &bc,SBoidsCreateContext &ctx );	
	IEntity* GetEntity( IScriptTable *pEntityTable );
	CFlock* GetFlock( IScriptTable *pEntityTable );

	i32 CommonCreateFlock( i32 type,IFunctionHandler *pH,SmartScriptTable entity,SmartScriptTable paramTable );

	ISystem *m_pSystem;
	IScriptSystem *m_pScriptSystem;

	DeclareConstIntCVar(boids_enabled, 1);
};

#endif // __scriptobjectboids_h__