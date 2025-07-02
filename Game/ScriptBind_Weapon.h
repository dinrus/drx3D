// Разработка 2018-2025 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Описание: Script Binding for Weapon
  
 -------------------------------------------------------------------------
  История:
  - 25:11:2004   11:30 : Created by M�rcio Martins

*************************************************************************/
#ifndef __SCRIPTBIND_WEAPON_H__
#define __SCRIPTBIND_WEAPON_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>


struct IItemSystem;
struct IGameFramework;
class CItem;
class CWeapon;


class CScriptBind_Weapon :
	public CScriptableBase
{
public:
	CScriptBind_Weapon(ISystem *pSystem, IGameFramework *pGameFramework);
	virtual ~CScriptBind_Weapon();

	virtual void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	void AttachTo(CWeapon *pWeapon);

	i32 SetAmmoCount(IFunctionHandler *pH);
	i32 GetAmmoCount(IFunctionHandler *pH);
	i32 GetClipSize(IFunctionHandler *pH);

	i32 GetAmmoType(IFunctionHandler *pH);

	i32 SupportsAccessory(IFunctionHandler *pH, tukk accessoryName);
	i32 GetAccessory(IFunctionHandler *pH, tukk accessoryName);
	i32 AttachAccessory(IFunctionHandler *pH, tukk className, bool attach, bool force);
	i32 SwitchAccessory(IFunctionHandler *pH, tukk className);

	i32 SetCurrentFireMode(IFunctionHandler *pH, tukk name);

	i32 Reload(IFunctionHandler *pH);

private:
	void RegisterGlobals();
	void RegisterMethods();

	CItem *GetItem(IFunctionHandler *pH);
	CWeapon *GetWeapon(IFunctionHandler *pH);

	ISystem						*m_pSystem;
	IGameFramework		*m_pGameFW;
};


#endif //__SCRIPTBIND_ITEM_H__