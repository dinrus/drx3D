// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Script Binding for the Vehicle System

   -------------------------------------------------------------------------
   История:
   - 05:10:2004   12:05 : Created by Mathieu Pinard

*************************************************************************/
#ifndef __SCRIPTBIND_VEHICLE_H__
#define __SCRIPTBIND_VEHICLE_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>

struct IVehicleSystem;
struct IGameFramework;
class CVehicle;

class CScriptBind_Vehicle :
	public CScriptableBase
{
public:
	CScriptBind_Vehicle(ISystem* pSystem, IGameFramework* pGameFW);
	virtual ~CScriptBind_Vehicle();

	void         Release() { delete this; };

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	void AttachTo(IVehicle* pVehicle);

	//! <code>Vehicle.GetVehicle()</code>
	//! <description>Gets the vehicle identifier.</description>
	CVehicle* GetVehicle(IFunctionHandler* pH);

	//! <code>Vehicle.Reset()</code>
	//! <description>Resets the vehicle.</description>
	i32 Reset(IFunctionHandler* pH);

	//! <code>Vehicle.IsInsideRadius( pos, radius )</code>
	//! <description>Checks if the vehicle is inside the specified radius.</description>
	i32 IsInsideRadius(IFunctionHandler* pH, Vec3 pos, float radius);

	//! <code>Vehicle.MultiplyWithWorldTM( pos )</code>
	//!		<param name="pos">Position vector.</param>
	//! <description>Multiplies with the world transformation matrix.</description>
	i32 MultiplyWithWorldTM(IFunctionHandler* pH, Vec3 pos);

	//! <code>Vehicle.ResetSlotGeometry( slot, filename, geometry )</code>
	i32 ResetSlotGeometry(IFunctionHandler* pH, i32 slot, tukk filename, tukk geometry);

	//! <code>Vehicle.AddSeat( paramsTable )</code>
	//!		<param name="paramsTable">Seat parameters.</param>
	//! <description>Adds a seat to the vehicle.</description>
	i32 AddSeat(IFunctionHandler* pH, SmartScriptTable paramsTable);

	//! <code>Vehicle.HasHelper(name)</code>
	//!		<param name="name">Helper name.</param>
	//! <description>Checks if the vehicle has the specified helper.</description>
	i32 HasHelper(IFunctionHandler* pH, tukk name);

	//! <code>Vehicle.GetHelperPos(name, isInVehicleSpace)</code>
	//!		<param name="name">Helper name.</param>
	//!		<param name="isInVehicleSpace">.</param>
	//! <description>Gets the helper position.</description>
	i32 GetHelperPos(IFunctionHandler* pH, tukk name, bool isInVehicleSpace);

	//! <code>Vehicle.GetHelperDir( name, isInVehicleSpace )</code>
	//!		<param name="name">Helper name.</param>
	//!		<param name="isInVehicleSpace">.</param>
	//! <description>Gets the helper direction.</description>
	i32 GetHelperDir(IFunctionHandler* pH, tukk name, bool isInVehicleSpace);

	//! <code>Vehicle.GetHelperWorldPos( name )</code>
	//!		<param name="name">Helper name.</param>
	//! <description>Gets the helper position in the world coordinates.</description>
	i32 GetHelperWorldPos(IFunctionHandler* pH, tukk name);

	//! <code>Vehicle.EnableMovement( enable )</code>
	//!		<param name="enable">True to enable movement, false to disable.</param>
	//! <description>Enables/disables the movement of the vehicle.</description>
	i32 EnableMovement(IFunctionHandler* pH, bool enable);

	//! <code>Vehicle.DisableEngine( disable )</code>
	//!		<param name="disable">True to disable the engine, false to enable.</param>
	//! <description>Disables/enables the engine of the vehicle.</description>
	i32 DisableEngine(IFunctionHandler* pH, bool disable);

	//! <code>Vehicle.OnHit( targetId, shooterId, damage, position, radius, pHitClass, explosion )</code>
	//!		<param name="targetId">Target identifier.</param>
	//!		<param name="shooterId">Shooter identifier.</param>
	//!		<param name="damage">Damage amount.</param>
	//!		<param name="radius">Radius of the hit.</param>
	//!		<param name="hitTypeId">Hit type.</param>
	//!		<param name="explosion">True if the hit cause an explosion, false otherwise.</param>
	//! <description>Event that occurs after the vehicle is hit.</description>
	i32 OnHit(IFunctionHandler* pH, ScriptHandle targetId, ScriptHandle shooterId, float damage, Vec3 position, float radius, i32 hitTypeId, bool explosion);

	//! <code>Vehicle.ProcessPassengerDamage( passengerId, actorHealth, damage, pDamageClass, explosion )</code>
	//!		<param name="passengerId">Passenger identifier.</param>
	//!		<param name="actorHealth">Actor health amount.</param>
	//!		<param name="damage">Damage amount.</param>
	//!		<param name="hitTypeId">Damage type.</param>
	//!		<param name="explosion">True if there is an explosion, false otherwise.</param>
	//! <description>Processes passenger damages.</description>
	i32 ProcessPassengerDamage(IFunctionHandler* pH, ScriptHandle passengerId, float actorHealth, float damage, i32 hitTypeId, bool explosion);

	//! <code>Vehicle.Destroy()</code>
	//! <description>Destroys the vehicle.</description>
	i32 Destroy(IFunctionHandler* pH);

	//! <code>Vehicle.IsDestroyed()</code>
	//! <description>Checks if the vehicle is destroyed.</description>
	i32 IsDestroyed(IFunctionHandler* pH);

	//! <code>Vehicle.IsUsable( userHandle )</code>
	//!		<param name="userHandle">User identifier.</param>
	//! <description>Checks if the vehicle is usable by the user.</description>
	i32 IsUsable(IFunctionHandler* pH, ScriptHandle userHandle);

	//! <code>Vehicle.OnUsed( userHandle, index )</code>
	//!		<param name="userHandle">User identifier.</param>
	//!		<param name="index">Seat identifier.</param>
	//! <description>Events that occurs when the user uses the vehicle.</description>
	i32 OnUsed(IFunctionHandler* pH, ScriptHandle userHandle, i32 index);

	//! <code>Vehicle.EnterVehicle( actorHandle, seatId, isAnimationEnabled )</code>
	//!		<param name="actorHandle">Actor identifier.</param>
	//!		<param name="seatId">Seat identifier.</param>
	//!		<param name="isAnimationEnabled - True to enable the animation, false otherwise.</param>
	//! <description>Makes the actor entering the vehicle.</description>
	i32 EnterVehicle(IFunctionHandler* pH, ScriptHandle actorHandle, i32 seatId, bool isAnimationEnabled);

	//! <code>Vehicle.ChangeSeat(actorHandle, seatId, isAnimationEnabled)</code>
	//!		<param name="actorHandle">Actor identifier.</param>
	//!		<param name="seatId">Seat identifier.</param>
	//!		<param name="isAnimationEnabled - True to enable the animation, false otherwise.</param>
	//! <description>Makes the actor changing the seat inside the vehicle.</description>
	i32 ChangeSeat(IFunctionHandler* pH, ScriptHandle actorHandle, i32 seatId, bool isAnimationEnabled);

	//! <code>Vehicle.ExitVehicle( actorHandle )</code>
	//!		<param name="actorHandle">Actor identifier.</param>
	//! <description>Makes the actor going out from the vehicle.
	i32 ExitVehicle(IFunctionHandler* pH, ScriptHandle actorHandle);

	//! <code>Vehicle.GetComponentDamageRatio( componentName )</code>
	//! <description>Gets the damage ratio of the specified component.</description>
	i32 GetComponentDamageRatio(IFunctionHandler* pH, tukk pComponentName);

	//! <code>Vehicle.GetSeatForPassenger(id)</code>
	//! <returns>Vehicle seat id for the specified passenger.</returns>
	//!		<param name="id">passenger id.</param>
	i32 GetSeatForPassenger(IFunctionHandler* pH, ScriptHandle passengerId);

	//! <code>Vehicle.OnSpawnComplete()</code>
	//! <description>Callback into game code for OnSpawnComplete.</description>
	i32 OnSpawnComplete(IFunctionHandler* pH);

private:

	void RegisterMethods();

	IGameFramework* m_pGameFW;
	IVehicleSystem* m_pVehicleSystem;
};

#endif //__SCRIPTBIND_VEHICLE_H__
