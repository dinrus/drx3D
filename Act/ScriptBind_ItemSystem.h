// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:

   -------------------------------------------------------------------------
   История:
   - 30:9:2004   14:21 : Created by Márcio Martins

*************************************************************************/
#ifndef __SCRIPTBIND_ITEMSYSTEM_H__
#define __SCRIPTBIND_ITEMSYSTEM_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>

class CItemSystem;
class CEquipmentUpr;

class CScriptBind_ItemSystem :
	public CScriptableBase
{
public:
	CScriptBind_ItemSystem(ISystem* pSystem, CItemSystem* pItemSystem, IGameFramework* pGameFramework);
	virtual ~CScriptBind_ItemSystem();

	void         Release() { delete this; };

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	//! <code>ItemSystem.Reset()</code>
	//! <description>Resets the item system.</description>
	i32 Reset(IFunctionHandler* pH);

	//! <code>ItemSystem.GiveItem( itemName )</code>
	//!		<param name="itemName">Item name.</param>
	//! <description>Gives the specified item.</description>
	i32 GiveItem(IFunctionHandler* pH, tukk itemName);

	//! <code>ItemSystem.GiveItemPack( actorId, packName )</code>
	//!		<param name="actorId">Actor identifier.</param>
	//!		<param name="packName">Pack name.</param>
	//! <description>Gives the item pack to the specified actor.</description>
	i32 GiveItemPack(IFunctionHandler* pH, ScriptHandle actorId, tukk packName);

	//! <code>ItemSystem.GetPackPrimaryItem( packName )</code>
	//!		<param name="packName">Pack name.</param>
	//! <description>Gets the primary item of the specified pack.</description>
	i32 GetPackPrimaryItem(IFunctionHandler* pH, tukk packName);

	//! <code>ItemSystem.GetPackNumItems()</code>
	//! <description>Get the number of items in the specified pack.</description>
	//!		<param name="packName">Pack name.</param>
	i32 GetPackNumItems(IFunctionHandler* pH, tukk packName);

	//! <code>ItemSystem.GetPackItemByIndex( packName, index )</code>
	//!		<param name="packName">Pack name.</param>
	//!		<param name="index">Pack index.</param>
	//! <description>Gets a pack item from its index.</description>
	i32 GetPackItemByIndex(IFunctionHandler* pH, tukk packName, i32 index);

	//! <code>ItemSystem.SetActorItem( actorId, itemId, keepHistory )</code>
	//!		<param name="actorId">Actor identifier.</param>
	//!		<param name="itemId">Item identifier.</param>
	//!		<param name="keepHistory">True to keep history, false otherwise.</param>
	//! <description>Sets an actor item.</description>
	i32 SetActorItem(IFunctionHandler* pH, ScriptHandle actorId, ScriptHandle itemId, bool keepHistory);

	//! <code>ItemSystem.SetActorItemByName( actorId, name, keepHistory )</code>
	//!		<param name="actorId">Actor identifier.</param>
	//!		<param name="name">Actor item name.</param>
	//!		<param name="keepHistory">True to keep history, false otherwise.</param>
	//! <description>Sets an actor item by name.</description>
	i32 SetActorItemByName(IFunctionHandler* pH, ScriptHandle actorId, tukk name, bool keepHistory);

	//! <code>ItemSystem.SerializePlayerLTLInfo( reading )</code>
	//!		<param name="reading">Boolean value.</param>
	//! <description>Serializes player LTL info.</description>
	i32 SerializePlayerLTLInfo(IFunctionHandler* pH, bool reading);

private:
	void RegisterGlobals();
	void RegisterMethods();

	CItemSystem*       m_pItemSystem;
	IGameFramework*    m_pGameFramework;
	CEquipmentUpr* m_pEquipmentUpr;
};

#endif //__SCRIPTBIND_ITEMSYSTEM_H__
