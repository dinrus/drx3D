// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Script Binding for Inventory

   -------------------------------------------------------------------------
   История:
   - 4:9:2005   15:29 : Created by Márcio Martins

*************************************************************************/
#ifndef __SCRIPTBIND_INVENTORY_H__
#define __SCRIPTBIND_INVENTORY_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>

struct IItemSystem;
struct IGameFramework;
class CInventory;

class CScriptBind_Inventory :
	public CScriptableBase
{
public:
	CScriptBind_Inventory(ISystem* pSystem, IGameFramework* pGameFramework);
	virtual ~CScriptBind_Inventory();
	void         Release() { delete this; };
	void         AttachTo(CInventory* pInventory);
	void         DetachFrom(CInventory* pInventory);

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	//! <code>Inventory.Destroy()</code>
	//! <description>Destroys the inventory.</description>
	i32 Destroy(IFunctionHandler* pH);

	//! <code>Inventory.Clear()</code>
	//! <description>Clears the inventory.</description>
	i32 Clear(IFunctionHandler* pH);

	//! <code>Inventory.Dump()</code>
	//! <description>Dumps the inventory.</description>
	i32 Dump(IFunctionHandler* pH);

	//! <code>Inventory.GetItemByClass( className )</code>
	//!		<param name="className">Class name.</param>
	//! <description>Gets item by class name.</description>
	i32 GetItemByClass(IFunctionHandler* pH, tukk className);

	//! <code>Inventory.GetGrenadeWeaponByClass( className )</code>
	//!		<param name="className">Class name.</param>
	//! <description>Gets grenade weapon by class name.</description>
	i32 GetGrenadeWeaponByClass(IFunctionHandler* pH, tukk className);

	//! <code>Inventory.HasAccessory( accessoryName )</code>
	//!		<param name="accessoryName">Accessory name.</param>
	//! <description>Checks if the inventory contains the specified accessory.</description>
	i32 HasAccessory(IFunctionHandler* pH, tukk accessoryName);

	//! <code>Inventory.GetCurrentItemId()</code>
	//! <description>Gets the identifier of the current item.</description>
	i32 GetCurrentItemId(IFunctionHandler* pH);

	//! <code>Inventory.GetCurrentItem()</code>
	//! <description>Gets the current item.</description>
	i32 GetCurrentItem(IFunctionHandler* pH);

	//! <code>Inventory.GetAmmoCount(ammoName)</code>
	//!		<param name="ammoName">Ammunition name.</param>
	//! <description>Gets the amount of the specified ammunition name.</description>
	i32 GetAmmoCount(IFunctionHandler* pH, tukk ammoName);

	//! <code>Inventory.GetAmmoCapacity( ammoName )</code>
	//!		<param name="ammoName">Ammunition name.</param>
	//! <description>Gets the capacity for the specified ammunition.</description>
	i32 GetAmmoCapacity(IFunctionHandler* pH, tukk ammoName);

	//! <code>Inventory.SetAmmoCount( ammoName, count )</code>
	//!		<param name="ammoName">Ammunition name.</param>
	//!		<param name="count">Ammunition amount.</param>
	//! <description>Sets the amount of the specified ammunition.</description>
	i32 SetAmmoCount(IFunctionHandler* pH, tukk ammoName, i32 count);

private:
	void        RegisterGlobals();
	void        RegisterMethods();

	CInventory* GetInventory(IFunctionHandler* pH);

	ISystem*        m_pSystem;
	IEntitySystem*  m_pEntitySystem;
	IGameFramework* m_pGameFramework;
};

#endif //__SCRIPTBIND_INVENTORY_H__
