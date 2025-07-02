// Разработка 2018-2025 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Описание: Script Binding for Item
  
 -------------------------------------------------------------------------
  История:
  - 27:10:2004   11:30 : Created by M�rcio Martins

*************************************************************************/
#ifndef __SCRIPTBIND_ITEM_H__
#define __SCRIPTBIND_ITEM_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>


struct IItemSystem;
struct IGameFramework;
class CItem;
class CActor;


class CScriptBind_Item :
	public CScriptableBase
{
public:
	CScriptBind_Item(ISystem *pSystem, IGameFramework *pGameFramework);
	virtual ~CScriptBind_Item();

	virtual void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	void AttachTo(CItem *pItem);

	i32 Reset(IFunctionHandler *pH);

	i32 CanPickUp(IFunctionHandler *pH, ScriptHandle userId);
	i32 CanUse(IFunctionHandler *pH, ScriptHandle userId);
	i32 CanUseVehicle(IFunctionHandler *pH, ScriptHandle userId);
	i32 IsPickable(IFunctionHandler *pH);
	i32 IsMounted(IFunctionHandler *pH);
	i32 GetUsableText(IFunctionHandler *pH);

	i32 GetOwnerId(IFunctionHandler *pH);
	i32 StartUse(IFunctionHandler *pH, ScriptHandle userId);
	i32 StopUse(IFunctionHandler *pH, ScriptHandle userId);
	i32 Use(IFunctionHandler *pH, ScriptHandle userId);
	i32 IsUsed(IFunctionHandler *pH);
	i32 OnUsed(IFunctionHandler *pH, ScriptHandle userId);

	i32 GetMountedDir(IFunctionHandler *pH);
	i32 SetMountedAngleLimits(IFunctionHandler *pH, float min_pitch, float max_pitch, float yaw_range);

  i32 OnHit(IFunctionHandler *pH, SmartScriptTable hitTable);
  i32 IsDestroyed(IFunctionHandler *pH);

	i32 HasAccessory(IFunctionHandler *pH, tukk accessoryName);

	i32 AllowDrop(IFunctionHandler *pH);
	i32 DisallowDrop(IFunctionHandler *pH);

private:
	void RegisterGlobals();
	void RegisterMethods();

	CItem *GetItem(IFunctionHandler *pH);
	CActor *GetActor(EntityId actorId);

	ISystem						*m_pSystem;
	IGameFramework		*m_pGameFW;

	SmartScriptTable	m_stats;
	SmartScriptTable	m_params;
};


#endif //__SCRIPTBIND_ITEM_H__