// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Script Binding for the Vehicle System

   -------------------------------------------------------------------------
   История:
   - 26:04:2005   : Created by Mathieu Pinard

*************************************************************************/
#ifndef __SCRIPTBIND_VEHICLESYSTEM_H__
#define __SCRIPTBIND_VEHICLESYSTEM_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>

struct IVehicleSystem;
struct IGameFramework;
class CVehicleSystem;

class CScriptBind_VehicleSystem :
	public CScriptableBase
{
public:
	CScriptBind_VehicleSystem(ISystem* pSystem, CVehicleSystem* vehicleSystem);
	virtual ~CScriptBind_VehicleSystem();

	void Release() { delete this; };

	//! <code>VehicleSystem.GetVehicleImplementations()</code>
	//! <description>Get a table of all implemented vehicles.</description>
	i32 GetVehicleImplementations(IFunctionHandler* pH);

	//! <code>VehicleSystem.GetOptionalScript(vehicleName)</code>
	//! <description>Get an (optional) script for the named vehicle.</description>
	i32 GetOptionalScript(IFunctionHandler* pH, tuk vehicleName);

	//! <code>VehicleSystem.SetTpvDistance(distance)</code>
	//! <description>Distance of camera in third person view.</description>
	i32 SetTpvDistance(IFunctionHandler* pH, float distance);

	//! <code>VehicleSystem.SetTpvHeight(height)</code>
	//! <description>Height of camera in third person view.</description>
	i32 SetTpvHeight(IFunctionHandler* pH, float height);

	//! <code>VehicleSystem.ReloadSystem()</code>
	//! <description>Reloads the vehicle system with default values.</description>
	i32          ReloadSystem(IFunctionHandler* pH);

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

private:

	void RegisterGlobals();
	void RegisterMethods();

	CVehicleSystem* m_pVehicleSystem;
};

#endif //__SCRIPTBIND_VEHICLESYSTEM_H__
