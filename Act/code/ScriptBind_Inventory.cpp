// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   История:
   - 4:9:2005   15:32 : Created by Márcio Martins

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/ScriptBind_Inventory.h>
#include <drx3D/Act/Inventory.h>
#include <drx3D/Act/IGameObject.h>

//------------------------------------------------------------------------
CScriptBind_Inventory::CScriptBind_Inventory(ISystem* pSystem, IGameFramework* pGameFramework)
	: m_pSystem(pSystem),
	m_pEntitySystem(gEnv->pEntitySystem),
	m_pGameFramework(pGameFramework)
{
	Init(m_pSystem->GetIScriptSystem(), m_pSystem, 1);

	RegisterMethods();
	RegisterGlobals();
}

//------------------------------------------------------------------------
CScriptBind_Inventory::~CScriptBind_Inventory()
{
}

//------------------------------------------------------------------------
void CScriptBind_Inventory::AttachTo(CInventory* pInventory)
{
	IScriptTable* pScriptTable = pInventory->GetEntity()->GetScriptTable();

	if (pScriptTable)
	{
		SmartScriptTable thisTable(m_pSS);

		thisTable->SetValue("__this", ScriptHandle(pInventory));
		thisTable->Delegate(GetMethodsTable());

		pScriptTable->SetValue("inventory", thisTable);
	}
}

//------------------------------------------------------------------------
void CScriptBind_Inventory::DetachFrom(CInventory* pInventory)
{
	IScriptTable* pScriptTable = pInventory->GetEntity()->GetScriptTable();

	if (pScriptTable)
		pScriptTable->SetToNull("inventory");
}

//------------------------------------------------------------------------
void CScriptBind_Inventory::RegisterGlobals()
{
}

//------------------------------------------------------------------------
void CScriptBind_Inventory::RegisterMethods()
{
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_Inventory::

	SCRIPT_REG_TEMPLFUNC(Destroy, "");
	SCRIPT_REG_TEMPLFUNC(Clear, "");

	SCRIPT_REG_TEMPLFUNC(Dump, "");

	SCRIPT_REG_TEMPLFUNC(GetItemByClass, "className");
	SCRIPT_REG_TEMPLFUNC(GetGrenadeWeaponByClass, "className");
	SCRIPT_REG_TEMPLFUNC(HasAccessory, "accessoryName");

	SCRIPT_REG_TEMPLFUNC(GetCurrentItemId, "");
	SCRIPT_REG_TEMPLFUNC(GetCurrentItem, "");
	SCRIPT_REG_TEMPLFUNC(GetAmmoCapacity, "ammoName");
	SCRIPT_REG_TEMPLFUNC(GetAmmoCount, "ammoName");
	SCRIPT_REG_TEMPLFUNC(SetAmmoCount, "ammoName, count");
}

//------------------------------------------------------------------------
CInventory* CScriptBind_Inventory::GetInventory(IFunctionHandler* pH)
{
	return (CInventory*)pH->GetThis();
}

//------------------------------------------------------------------------
i32 CScriptBind_Inventory::Destroy(IFunctionHandler* pH)
{
	CInventory* pInventory = GetInventory(pH);
	DRX_ASSERT(pInventory);

	pInventory->Destroy();
	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Inventory::Clear(IFunctionHandler* pH)
{
	CInventory* pInventory = GetInventory(pH);
	DRX_ASSERT(pInventory);

	pInventory->Clear();
	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Inventory::Dump(IFunctionHandler* pH)
{

#if DEBUG_INVENTORY_ENABLED
	CInventory* pInventory = GetInventory(pH);
	DRX_ASSERT(pInventory);

	pInventory->Dump();
#endif

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Inventory::GetItemByClass(IFunctionHandler* pH, tukk className)
{
	CInventory* pInventory = GetInventory(pH);
	DRX_ASSERT(pInventory);

	IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(className);
	EntityId itemId = pInventory->GetItemByClass(pClass);
	if (itemId)
		return pH->EndFunction(ScriptHandle(itemId));

	return pH->EndFunction();
}

//----------------------------------------------------------------------
i32 CScriptBind_Inventory::GetGrenadeWeaponByClass(IFunctionHandler* pH, tukk className)
{
	CInventory* pInventory = GetInventory(pH);
	DRX_ASSERT(pInventory);

	IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(className);
	EntityId itemId = pInventory->GetItemByClass(pClass, NULL);
	if (itemId)
		return pH->EndFunction(ScriptHandle(itemId));

	return pH->EndFunction();
}

//-------------------------------------------------------------------
i32 CScriptBind_Inventory::HasAccessory(IFunctionHandler* pH, tukk accessoryName)
{
	CInventory* pInventory = GetInventory(pH);
	DRX_ASSERT(pInventory);

	IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(accessoryName);
	if (pClass)
		return pH->EndFunction(pInventory->HasAccessory(pClass));

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Inventory::GetCurrentItemId(IFunctionHandler* pH)
{
	CInventory* pInventory = GetInventory(pH);
	DRX_ASSERT(pInventory);

	EntityId currentItemId = pInventory->GetCurrentItem();
	if (currentItemId)
		return pH->EndFunction(ScriptHandle(currentItemId));
	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Inventory::GetCurrentItem(IFunctionHandler* pH)
{
	CInventory* pInventory = GetInventory(pH);
	DRX_ASSERT(pInventory);

	EntityId currentItemId = pInventory->GetCurrentItem();
	if (currentItemId)
	{
		IEntity* pEntity = m_pEntitySystem->GetEntity(currentItemId);
		if (pEntity)
			return pH->EndFunction(pEntity->GetScriptTable());
	}
	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Inventory::GetAmmoCapacity(IFunctionHandler* pH, tukk ammoName)
{
	CInventory* pInventory = GetInventory(pH);
	DRX_ASSERT(pInventory);

	IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(ammoName);
	DRX_ASSERT(pClass);
	return pH->EndFunction(pInventory->GetAmmoCapacity(pClass));
}

//------------------------------------------------------------------------
i32 CScriptBind_Inventory::GetAmmoCount(IFunctionHandler* pH, tukk ammoName)
{
	CInventory* pInventory = GetInventory(pH);
	DRX_ASSERT(pInventory);

	IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(ammoName);
	DRX_ASSERT(pClass);
	return pH->EndFunction(pInventory->GetAmmoCount(pClass));
}

//------------------------------------------------------------------------
i32 CScriptBind_Inventory::SetAmmoCount(IFunctionHandler* pH, tukk ammoName, i32 count)
{
	CInventory* pInventory = GetInventory(pH);
	DRX_ASSERT(pInventory);

	IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(ammoName);
	DRX_ASSERT(pClass);
	if (pClass)
	{
		pInventory->SetAmmoCount(pClass, count);
		if (gEnv->bServer)
		{
			pInventory->GetActor()->GetGameObject()->InvokeRMI(CInventory::Cl_SetAmmoCount(),
			                                                   TRMIInventory_Ammo(ammoName, count),
			                                                   eRMI_ToRemoteClients);
		}
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Ammo class %s not found!", ammoName);
	}

	return pH->EndFunction();
}
