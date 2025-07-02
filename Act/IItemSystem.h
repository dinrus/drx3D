// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Item System interfaces.

   -------------------------------------------------------------------------
   История:
   - 1:9:2004   19:52 : Created by Márcio Martins
   - 20:6:2005        : Changed IItem to be a game object extension by Craig Tiller

*************************************************************************/

#ifndef __IITEMSYSTEM_H__
#define __IITEMSYSTEM_H__

#pragma once

#include "IGameObject.h"
#include "IItem.h"
#include "IWeapon.h"
#include "IActorSystem.h"
#include <drx3D/FlowGraph/IFlowSystem.h>

enum EItemParamMapTypes
{
	eIPT_None   = -2,
	eIPT_Any    = -1,
	eIPT_Float  = EFlowDataTypes::eFDT_Float,
	eIPT_Int    = EFlowDataTypes::eFDT_Int,
	eIPT_Vec3   = EFlowDataTypes::eFDT_Vec3,
	eIPT_String = EFlowDataTypes::eFDT_String,
};

struct IItemParamsNode
{
	virtual ~IItemParamsNode() {}

	virtual void                   AddRef() const = 0;
	virtual u32                 GetRefCount() const = 0;
	virtual void                   Release() const = 0;

	virtual i32                    GetAttributeCount() const = 0;
	virtual tukk            GetAttributeName(i32 i) const = 0;
	virtual tukk            GetAttribute(i32 i) const = 0;
	virtual bool                   GetAttribute(i32 i, Vec3& attr) const = 0;
	virtual bool                   GetAttribute(i32 i, Ang3& attr) const = 0;
	virtual bool                   GetAttribute(i32 i, float& attr) const = 0;
	virtual bool                   GetAttribute(i32 i, i32& attr) const = 0;
	virtual i32                    GetAttributeType(i32 i) const = 0;

	virtual tukk            GetAttribute(tukk name) const = 0;
	virtual bool                   GetAttribute(tukk name, Vec3& attr) const = 0;
	virtual bool                   GetAttribute(tukk name, Ang3& attr) const = 0;
	virtual bool                   GetAttribute(tukk name, float& attr) const = 0;
	virtual bool                   GetAttribute(tukk name, i32& attr) const = 0;
	virtual i32                    GetAttributeType(tukk name) const = 0;

	virtual tukk            GetAttributeSafe(tukk name) const = 0;

	virtual tukk            GetNameAttribute() const = 0;

	virtual i32                    GetChildCount() const = 0;
	virtual tukk            GetChildName(i32 i) const = 0;
	virtual const IItemParamsNode* GetChild(i32 i) const = 0;
	virtual const IItemParamsNode* GetChild(tukk name) const = 0;

	virtual void                   SetAttribute(tukk name, tukk attr) = 0;
	virtual void                   SetAttribute(tukk name, const Vec3& attr) = 0;
	virtual void                   SetAttribute(tukk name, float attr) = 0;
	virtual void                   SetAttribute(tukk name, i32 attr) = 0;

	virtual void                   SetName(tukk name) = 0;
	virtual tukk            GetName() const = 0;

	virtual IItemParamsNode*       InsertChild(tukk name) = 0;
	virtual void                   ConvertFromXML(const XmlNodeRef& root) = 0;
	virtual bool                   ConvertFromXMLWithFiltering(const XmlNodeRef& root, tukk keepWithThisAttrValue) = 0;

	virtual void                   GetMemoryUsage(IDrxSizer* pSizer) const = 0;

	void                           Dump(tukk name = 0) const
	{
		i32 ident = 0;
		DumpNode(this, name ? name : "root", ident);
	}

private:
	void DumpNode(const IItemParamsNode* node, tukk name, i32& ident) const
	{
		string dump;
		dump.reserve(10000);
		for (i32 i = 0; i < ident; i++)
			dump += " ";

		dump += "<";
		dump += name;

		char snum[64];

		for (i32 a = 0; a < node->GetAttributeCount(); a++)
		{
			tukk attrName = node->GetAttributeName(a);
			dump += " ";
			dump += attrName;
			dump += "=\"";

			switch (node->GetAttributeType(a))
			{
			case eIPT_Int:
				{
					i32 attr;
					node->GetAttribute(a, attr);
					drx_sprintf(snum, "%d", attr);
					dump += snum;
				}
				break;
			case eIPT_Float:
				{
					float attr;
					node->GetAttribute(a, attr);
					drx_sprintf(snum, "%f", attr);
					dump += snum;
				}
				break;
			case eIPT_String:
				{
					tukk attr = node->GetAttribute(a);
					dump += attr;
				}
				break;
			case eIPT_Vec3:
				{
					Vec3 attr;
					node->GetAttribute(a, attr);
					drx_sprintf(snum, "%f,%f,%f", attr.x, attr.y, attr.z);
					dump += snum;
				}
				break;
			default:
				break;
			}

			dump += "\"";
		}
		if (node->GetChildCount())
		{
			dump += ">";
			DrxLogAlways("%s", dump.c_str());

			ident += 2;
			for (i32 c = 0; c < node->GetChildCount(); c++)
			{
				tukk childName = node->GetChildName(c);
				const IItemParamsNode* child = node->GetChild(c);
				DumpNode(child, childName, ident);
			}
			ident -= 2;

			string finale;
			for (i32 i = 0; i < ident; i++)
				finale += " ";
			finale += "</";
			finale += name;
			finale += ">";
			DrxLogAlways("%s", finale.c_str());
		}
		else
		{
			dump += " />";
			DrxLogAlways("%s", dump.c_str());
		}
	}
};

struct IInventory : public IGameObjectExtension
{
	enum EInventorySlots
	{
		eInventorySlot_Weapon = 0,
		eInventorySlot_Explosives,
		eInventorySlot_Grenades,
		eInventorySlot_Special,
		eInventorySlot_Last
	};

	virtual bool                AddItem(EntityId id) = 0;
	virtual bool                RemoveItem(EntityId id) = 0;
	virtual void                RemoveAllItems(bool forceClear = false) = 0;

	virtual bool                AddAccessory(IEntityClass* itemClass) = 0;

	virtual void                Destroy() = 0;
	virtual void                Clear(bool forceClear = false) = 0;

	virtual void                RMIReqToServer_RemoveAllItems() const = 0;
	virtual void                RMIReqToServer_AddItem(tukk _pszItemClass) const = 0;
	virtual void                RMIReqToServer_RemoveItem(tukk _pszItemClass) const = 0;
	virtual void                RMIReqToServer_SetAmmoCount(tukk _pszAmmoClass, i32 _iAmount) const = 0;
	virtual void                RMIReqToServer_AddEquipmentPack(tukk _pszEquipmentPack, bool _bAdd, bool _bPrimary) const = 0;

	virtual i32                 GetCapacity() const = 0;
	virtual i32                 GetCount() const = 0;
	virtual i32                 GetCountOfClass(tukk className) const = 0;
	virtual i32                 GetCountOfCategory(tukk categoryName) const = 0;
	virtual i32                 GetCountOfUniqueId(u8 uniqueId) const = 0;

	virtual i32                 GetSlotCount(i32 slotId) const = 0;

	virtual EntityId            GetItem(i32 slotId) const = 0;
	virtual tukk         GetItemString(i32 slotId) const = 0;
	virtual EntityId            GetItemByClass(IEntityClass* pClass, IItem* pIgnoreItem = NULL) const = 0;
	virtual IItem*              GetItemByName(tukk name) const = 0;

	virtual i32                 GetAccessoryCount() const = 0;
	virtual tukk         GetAccessory(i32 slotId) const = 0;
	virtual const IEntityClass* GetAccessoryClass(i32 slotId) const = 0;
	virtual bool                HasAccessory(IEntityClass* pClass) const = 0;

	virtual i32                 FindItem(EntityId itemId) const = 0;

	virtual i32                 FindNext(IEntityClass* pClass, tukk category, i32 firstSlot, bool wrap) const = 0;
	virtual i32                 FindPrev(IEntityClass* pClass, tukk category, i32 firstSlot, bool wrap) const = 0;

	virtual EntityId            GetCurrentItem() const = 0;
	virtual EntityId            GetHolsteredItem() const = 0;
	virtual void                SetCurrentItem(EntityId itemId) = 0;
	virtual void                SetHolsteredItem(EntityId itemId) = 0;

	virtual void                SetLastItem(EntityId itemId) = 0;
	virtual EntityId            GetLastItem() const = 0;
	virtual EntityId            GetLastSelectedInSlot(IInventory::EInventorySlots slotId) const = 0;

	virtual void                HolsterItem(bool holster) = 0;

	virtual void                SerializeInventoryForLevelChange(TSerialize ser) = 0;
	virtual bool                IsSerializingForLevelChange() const = 0;

	virtual i32                 GetAmmoTypesCount() const = 0;
	virtual IEntityClass*       GetAmmoType(i32 idx) const = 0;

	virtual void                SetAmmoCount(IEntityClass* pAmmoType, i32 count) = 0;
	virtual i32                 GetAmmoCount(IEntityClass* pAmmoType) const = 0;
	virtual void                SetAmmoCapacity(IEntityClass* pAmmoType, i32 max) = 0;
	virtual i32                 GetAmmoCapacity(IEntityClass* pAmmoType) const = 0;
	virtual void                ResetAmmo() = 0;

	virtual void                AddAmmoUser(IEntityClass* pAmmoType) = 0;
	virtual void                RemoveAmmoUser(IEntityClass* pAmmoType) = 0;
	virtual i32                 GetNumberOfUsersForAmmo(IEntityClass* pAmmoType) const = 0;

	virtual IActor*             GetActor() = 0;

	virtual void                SetInventorySlotCapacity(EInventorySlots slotId, u32 capacity) = 0;
	virtual void                AssociateItemCategoryToSlot(tukk itemCategory, EInventorySlots slotId) = 0;
	virtual bool                IsAvailableSlotForItemClass(tukk itemClass) const = 0;
	virtual bool                IsAvailableSlotForItemCategory(tukk itemCategory) const = 0;
	virtual bool                AreItemsInSameSlot(tukk itemClass1, tukk itemClass2) const = 0;
	virtual EInventorySlots     GetSlotForItemCategory(tukk itemCategory) const = 0;

	virtual void                AddListener(struct IInventoryListener* pListener) = 0;
	virtual void                RemoveListener(struct IInventoryListener* pListener) = 0;

	virtual void                IgnoreNextClear() = 0;
};

struct IInventoryListener
{
	virtual ~IInventoryListener(){}
	virtual void OnAddItem(EntityId entityId) = 0;
	virtual void OnSetAmmoCount(IEntityClass* pAmmoType, i32 count) = 0;
	virtual void OnAddAccessory(IEntityClass* pAccessoryClass) = 0;
	virtual void OnClearInventory() = 0;
};

struct IEquipmentPackPreCacheCallback
{
	virtual ~IEquipmentPackPreCacheCallback(){}
	virtual void PreCacheItemResources(tukk itemName) = 0;
};

// Summary
//   Used to give predefined inventory to actors
struct IEquipmentUpr
{
	virtual ~IEquipmentUpr(){}
	struct IListener
	{
		virtual ~IListener(){}
		virtual void OnBeginGiveEquipmentPack() {}
		virtual void OnEndGiveEquipmentPack()   {}
	};

	struct IEquipmentPackIterator
	{
		virtual ~IEquipmentPackIterator(){}
		virtual void        AddRef() = 0;
		virtual void        Release() = 0;
		virtual i32         GetCount() = 0;
		virtual tukk Next() = 0;
	};
	typedef _smart_ptr<IEquipmentPackIterator> IEquipmentPackIteratorPtr;

	// Clear all equipment packs
	virtual void DeleteAllEquipmentPacks() = 0;

	// Loads equipment packs from rootNode
	virtual void LoadEquipmentPacks(const XmlNodeRef& rootNode) = 0;

	// Load all equipment packs from a certain path
	virtual void LoadEquipmentPacksFromPath(tukk path) = 0;

	// Load an equipment pack from an XML node
	virtual bool LoadEquipmentPack(const XmlNodeRef& rootNode, bool bOverrideExisiting = true) = 0;

	// Give an equipment pack (resp. items/ammo) to an actor
	virtual bool GiveEquipmentPack(IActor* pActor, tukk packName, bool bAdd, bool bSelectPrimary) = 0;

	// Pre-cache all resources needed for the items included in the given pack
	virtual void PreCacheEquipmentPackResources(tukk packName, IEquipmentPackPreCacheCallback& preCacheCallback) = 0;

	// return iterator with all available equipment packs
	virtual IEquipmentUpr::IEquipmentPackIteratorPtr CreateEquipmentPackIterator() = 0;

	virtual void RegisterListener(IListener* pListener) = 0;
	virtual void UnregisterListener(IListener* pListener) = 0;
};

struct IActor;

struct IItemSystemListener
{
	virtual ~IItemSystemListener(){}
	virtual void OnSetActorItem(IActor* pActor, IItem* pItem) = 0;
	virtual void OnDropActorItem(IActor* pActor, IItem* pItem) = 0;
	virtual void OnSetActorAccessory(IActor* pActor, IItem* pItem) = 0;
	virtual void OnDropActorAccessory(IActor* pActor, IItem* pItem) = 0;

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const {};
};

// Summary
// Query that item system can handle
enum IItemSystemQuery
{
	eISQ_Dump_Item_Names = 0,
	eISQ_Find_Item_By_Name,
	eISQ_Last,
};

// Summary
//   Interface to the Item system
struct IItemSystem
{
	virtual ~IItemSystem(){}
	virtual void                   Reset() = 0;
	virtual void                   Reload() = 0;
	virtual void                   PreReload() = 0;
	virtual void                   PostReload() = 0;
	virtual void                   Scan(tukk folderName) = 0;
	virtual IItemParamsNode*       CreateParams() = 0;
	virtual const IItemParamsNode* GetItemParams(tukk itemName) const = 0;
	virtual tukk            GetItemParamsDescriptionFile(tukk itemName) const = 0;
	virtual i32                    GetItemParamsCount() const = 0;
	virtual tukk            GetItemParamName(i32 index) const = 0;
	virtual u8                  GetItemPriority(tukk item) const = 0;
	virtual tukk            GetItemCategory(tukk item) const = 0;
	virtual u8                  GetItemUniqueId(tukk item) const = 0;

	virtual bool                   IsItemClass(tukk name) const = 0;
	virtual tukk            GetFirstItemClass() = 0;
	virtual tukk            GetNextItemClass() = 0;

	virtual void                   RegisterForCollection(EntityId itemId) = 0;
	virtual void                   UnregisterForCollection(EntityId itemId) = 0;

	virtual void                   AddItem(EntityId itemId, IItem* pItem) = 0;
	virtual void                   RemoveItem(EntityId itemId, tukk itemName = NULL) = 0;
	virtual IItem*                 GetItem(EntityId itemId) const = 0;

	virtual void                   SetConfiguration(tukk name) = 0;
	virtual tukk            GetConfiguration() const = 0;

	virtual uk                  Query(IItemSystemQuery query, ukk param = NULL) = 0;

	virtual ICharacterInstance*    GetCachedCharacter(tukk fileName) = 0;
	virtual IStatObj*              GetCachedObject(tukk fileName) = 0;
	virtual void                   CacheObject(tukk fileName, bool useCgfStreaming) = 0;
	virtual void                   CacheGeometry(const IItemParamsNode* geometry) = 0;
	virtual void                   CacheItemGeometry(tukk className) = 0;
	virtual void                   ClearGeometryCache() = 0;

	virtual void                   CacheItemSound(tukk className) = 0;
	virtual void                   ClearSoundCache() = 0;

	virtual void                   Serialize(TSerialize ser) = 0;

	virtual EntityId               GiveItem(IActor* pActor, tukk item, bool sound, bool select, bool keepHistory, tukk setup = NULL, EEntityFlags entityFlags = (EEntityFlags)0) = 0;
	virtual void                   SetActorItem(IActor* pActor, EntityId itemId, bool keepHistory = true) = 0;
	virtual void                   SetActorItem(IActor* pActor, tukk name, bool keepHistory = true) = 0;
	virtual void                   DropActorItem(IActor* pActor, EntityId itemId) = 0;
	virtual void                   SetActorAccessory(IActor* pActor, EntityId itemId, bool keepHistory = true) = 0;
	virtual void                   DropActorAccessory(IActor* pActor, EntityId itemId) = 0;

	virtual void                   RegisterListener(IItemSystemListener* pListener) = 0;
	virtual void                   UnregisterListener(IItemSystemListener* pListener) = 0;

	virtual IEquipmentUpr*     GetIEquipmentUpr() = 0;

	virtual u32                 GetItemSocketCount(tukk item) const = 0;
	virtual tukk            GetItemSocketName(tukk item, i32 idx) const = 0;
	virtual bool                   IsCompatible(tukk item, tukk attachment) const = 0;
	virtual bool                   GetItemSocketCompatibility(tukk item, tukk socket) const = 0;
	virtual bool                   CanSocketBeEmpty(tukk item, tukk socket) const = 0;
};

#endif //__IITEMSYSTEM_H__
