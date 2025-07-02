// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Script Binding for the Vehicle System

   -------------------------------------------------------------------------
   История:
   - 26:04:2005   : Created by Mathieu Pinard

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Act/ActorSystem.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Act/VehicleSystem.h>
#include <drx3D/Act/ScriptBind_VehicleSystem.h>
#include <drx3D/Act/VehicleViewThirdPerson.h>

//------------------------------------------------------------------------
CScriptBind_VehicleSystem::CScriptBind_VehicleSystem(ISystem* pSystem, CVehicleSystem* vehicleSystem)
{
	m_pVehicleSystem = vehicleSystem;

	Init(gEnv->pScriptSystem, pSystem);
	SetGlobalName("Vehicle");

	RegisterGlobals();
	RegisterMethods();

	gEnv->pScriptSystem->ExecuteFile("Scripts/Entities/Vehicles/VehicleSystem.lua");
}

//------------------------------------------------------------------------
CScriptBind_VehicleSystem::~CScriptBind_VehicleSystem()
{
}

//------------------------------------------------------------------------
void CScriptBind_VehicleSystem::RegisterGlobals()
{
}

//------------------------------------------------------------------------
void CScriptBind_VehicleSystem::RegisterMethods()
{
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_VehicleSystem::

	SCRIPT_REG_FUNC(GetVehicleImplementations);
	SCRIPT_REG_TEMPLFUNC(GetOptionalScript, "vehicleName");
	SCRIPT_REG_TEMPLFUNC(ReloadSystem, "");
}

//------------------------------------------------------------------------
i32 CScriptBind_VehicleSystem::GetVehicleImplementations(IFunctionHandler* pH)
{
	SmartScriptTable tableImpls(m_pSS->CreateTable());
	SVehicleImpls impls;

	m_pVehicleSystem->GetVehicleImplementations(impls);

	//DrxLog("Scriptbind got %i vehicles", impls.Count());
	for (i32 i = 0; i < impls.Count(); ++i)
	{
		tableImpls->SetAt(i + 1, impls.GetAt(i).c_str());
	}
	return pH->EndFunction(tableImpls);
}

//------------------------------------------------------------------------
i32 CScriptBind_VehicleSystem::GetOptionalScript(IFunctionHandler* pH, tuk vehicleName)
{
	char scriptName[1024] = { 0 };

	if (m_pVehicleSystem->GetOptionalScript(vehicleName, scriptName, sizeof(scriptName)))
		return pH->EndFunction(scriptName);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_VehicleSystem::ReloadSystem(IFunctionHandler* pH)
{
	m_pVehicleSystem->ReloadSystem();

	return pH->EndFunction();
}
