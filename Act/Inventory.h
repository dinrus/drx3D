// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Inventory GameObject Extension

   -------------------------------------------------------------------------
   История:
   - 29:8:2005   14:27 : Created by Márcio Martins

*************************************************************************/
#ifndef __INVENTORY_H__
#define __INVENTORY_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#if !defined(_RELEASE)
	#define DEBUG_INVENTORY_ENABLED 1
#else
	#define DEBUG_INVENTORY_ENABLED 0
#endif

#include <drx3D/Act/DinrusAction.h>
#include "IGameObject.h"
#include "IItemSystem.h"
#include <drx3D/Entity/IEntitySystem.h>

// dummy struct needed to use the RMI macros without parameters
struct TRMIInventory_Dummy
{
	TRMIInventory_Dummy() {}

	virtual ~TRMIInventory_Dummy(){}
	virtual void SerializeWith(TSerialize ser)
	{
	}
};

// for RMI that need an item class parameter
struct TRMIInventory_Item
{
	string m_ItemClass;

	TRMIInventory_Item()
	{}

	TRMIInventory_Item(tukk _pItemClass)
		: m_ItemClass(_pItemClass)
	{}

	void SerializeWith(TSerialize ser)
	{
		ser.Value("ItemClass", m_ItemClass);
	}
};

// for RMI that need ammo class + amount
struct TRMIInventory_Ammo
{
	string m_AmmoClass;
	i32    m_iAmount;

	TRMIInventory_Ammo()
		: m_iAmount(0)
	{}

	TRMIInventory_Ammo(tukk _pAmmoClass, i32 _iAmount)
		: m_AmmoClass(_pAmmoClass),
		m_iAmount(_iAmount)
	{}

	void SerializeWith(TSerialize ser)
	{
		ser.Value("AmmoClass", m_AmmoClass);
		ser.Value("Amount", m_iAmount);
	}
};

// for RMI that need an equipment pack name
struct TRMIInventory_EquipmentPack
{
	string m_EquipmentPack;
	bool   m_bAdd;
	bool   m_bPrimary;

	TRMIInventory_EquipmentPack()
		: m_bAdd(false),
		m_bPrimary(false)
	{}

	TRMIInventory_EquipmentPack(tukk _pEquipmentPack, bool _bAdd, bool _bPrimary)
		: m_EquipmentPack(_pEquipmentPack),
		m_bAdd(_bAdd),
		m_bPrimary(_bPrimary)
	{}

	void SerializeWith(TSerialize ser)
	{
		ser.Value("EquipmentPack", m_EquipmentPack);
		ser.Value("SetMode", m_bAdd);
		ser.Value("SelectPrimary", m_bPrimary);
	}
};

class CInventory :
	public CGameObjectExtensionHelper<CInventory, IInventory>
{
	struct SAmmoInfo
	{
		SAmmoInfo()
			: count(0)
			, users(0)
			, capacity(0)
		{

		}

		SAmmoInfo(i32 _count, i32 _capacity)
			: count(_count)
			, users(0)
			, capacity(_capacity)
		{

		}

		ILINE void SetCount(i32 _count)                    { count = _count; }
		ILINE i32  GetCount() const                        { return count; }
		ILINE void SetCapacity(i32 _capacity)              { capacity = _capacity; }
		ILINE i32  GetCapacity() const                     { return capacity; }
		ILINE void AddUser()                               { users++; }
		ILINE void RemoveUser()                            { DRX_ASSERT(users > 0); users = max(users - 1, 0); }
		ILINE i32  GetUserCount() const                    { return users; }

		ILINE void ResetCount()                            { count = 0; }
		ILINE void ResetUsers()                            { users = 0; }

		void       GetMemoryUsage(IDrxSizer* pSizer) const { /*nothing*/ }
	private:
		i32        count;
		i32        users;
		i32        capacity;
	};

	typedef std::vector<EntityId>              TInventoryVector;
	typedef TInventoryVector::const_iterator   TInventoryCIt;
	typedef TInventoryVector::iterator         TInventoryIt;
	typedef std::map<IEntityClass*, SAmmoInfo> TAmmoInfoMap;
	typedef std::vector<IEntityClass*>         TInventoryVectorEx;
	typedef std::vector<IInventoryListener*>   TListenerVec;

public:
	CInventory();
	virtual ~CInventory();

	//IGameObjectExtension
	virtual bool                 Init(IGameObject* pGameObject);
	virtual void                 InitClient(i32 channelId)          {};
	virtual void                 PostInit(IGameObject* pGameObject) {};
	virtual void                 PostInitClient(i32 channelId)      {};
	virtual bool                 ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params);
	virtual void                 PostReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params);
	virtual void                 Release()                                                                     { delete this; };
	virtual void                 FullSerialize(TSerialize ser);
	virtual bool                 NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags) { return true; }
	virtual void                 PostSerialize();
	virtual void                 SerializeSpawnInfo(TSerialize ser)                                            {}
	virtual ISerializableInfoPtr GetSpawnInfo()                                                                { return 0; }
	virtual void                 Update(SEntityUpdateContext& ctx, i32)                                        {}
	virtual void                 PostUpdate(float frameTime)                                                   {};
	virtual void                 PostRemoteSpawn()                                                             {};
	virtual void                 HandleEvent(const SGameObjectEvent&)                                          {};
	virtual void                 ProcessEvent(const SEntityEvent&);
	virtual uint64               GetEventMask() const;
	virtual void                 SetChannelId(u16 id)                                                       {};
	//~IGameObjectExtension

	//IInventory
	virtual bool                AddItem(EntityId id);
	virtual bool                RemoveItem(EntityId id);
	virtual void                RemoveAllItems(bool forceClear = false);

	virtual void                RMIReqToServer_RemoveAllItems() const;
	virtual void                RMIReqToServer_AddItem(tukk _pszItemClass) const;
	virtual void                RMIReqToServer_RemoveItem(tukk _pszItemClass) const;
	virtual void                RMIReqToServer_SetAmmoCount(tukk _pszAmmoClass, i32 _iAmount) const;
	virtual void                RMIReqToServer_AddEquipmentPack(tukk _pszEquipmentPack, bool _bAdd, bool _bPrimary) const;

	virtual bool                AddAccessory(IEntityClass* itemClass);

	virtual void                Destroy();
	virtual void                Clear(bool forceClear = false);

	virtual i32                 GetCapacity() const;
	virtual i32                 GetCount() const;
	virtual i32                 GetCountOfClass(tukk className) const;
	virtual i32                 GetCountOfCategory(tukk categoryName) const;
	virtual i32                 GetCountOfUniqueId(u8 uniqueId) const;

	virtual i32                 GetSlotCount(i32 slotId) const;

	virtual EntityId            GetItem(i32 slotId) const;
	virtual tukk         GetItemString(i32 slotId) const;
	virtual EntityId            GetItemByClass(IEntityClass* pClass, IItem* pIgnoreItem = NULL) const;
	virtual IItem*              GetItemByName(tukk name) const;

	virtual i32                 GetAccessoryCount() const;
	virtual tukk         GetAccessory(i32 slotId) const;
	virtual const IEntityClass* GetAccessoryClass(i32 slotId) const;
	virtual bool                HasAccessory(IEntityClass* pClass) const;

	virtual i32                 FindItem(EntityId itemId) const;

	virtual i32                 FindNext(IEntityClass* pClass, tukk category, i32 firstSlot, bool wrap) const;
	virtual i32                 FindPrev(IEntityClass* pClass, tukk category, i32 firstSlot, bool wrap) const;

	virtual EntityId            GetCurrentItem() const;
	virtual EntityId            GetHolsteredItem() const;
	virtual void                SetCurrentItem(EntityId itemId);
	virtual void                SetHolsteredItem(EntityId itemId);

	virtual void                SetLastItem(EntityId itemId);
	virtual EntityId            GetLastItem() const;
	virtual EntityId            GetLastSelectedInSlot(IInventory::EInventorySlots slotId) const;
	virtual void                RemoveItemFromCategorySlot(EntityId entityId);

	virtual void                HolsterItem(bool holster);

	virtual void                SerializeInventoryForLevelChange(TSerialize ser);
	virtual bool                IsSerializingForLevelChange() const { return m_bSerializeLTL; }

	virtual i32                 GetAmmoTypesCount() const;
	virtual IEntityClass*       GetAmmoType(i32 idx) const;

	// these functions are not multiplayer safe..
	// any changes made using these functions will not be synchronized...
	virtual void         SetAmmoCount(IEntityClass* pAmmoType, i32 count);
	virtual i32          GetAmmoCount(IEntityClass* pAmmoType) const;
	virtual void         SetAmmoCapacity(IEntityClass* pAmmoType, i32 max);
	virtual i32          GetAmmoCapacity(IEntityClass* pAmmoType) const;
	virtual void         ResetAmmo();

	virtual void         AddAmmoUser(IEntityClass* pAmmoType);
	virtual void         RemoveAmmoUser(IEntityClass* pAmmoType);
	virtual i32          GetNumberOfUsersForAmmo(IEntityClass* pAmmoType) const;

	inline void          AmmoIteratorFirst() { m_stats.ammoIterator = m_stats.ammoInfo.begin(); }
	inline bool          AmmoIteratorEnd()   { return m_stats.ammoIterator == m_stats.ammoInfo.end(); }
	inline i32           GetAmmoPackCount()  { return m_stats.ammoInfo.size(); }

	inline IEntityClass* AmmoIteratorNext()
	{
		IEntityClass* pClass = m_stats.ammoIterator->first;
		++m_stats.ammoIterator;
		return pClass;
	}

	inline const IEntityClass* AmmoIteratorGetClass()
	{
		return m_stats.ammoIterator->first;
	}

	inline i32 AmmoIteratorGetCount()
	{
		return m_stats.ammoIterator->second.GetCount();
	}

	virtual IActor*                     GetActor() { return m_pActor; };

	virtual void                        SetInventorySlotCapacity(IInventory::EInventorySlots slotId, u32 capacity);
	virtual void                        AssociateItemCategoryToSlot(tukk itemCategory, IInventory::EInventorySlots slotId);
	virtual bool                        IsAvailableSlotForItemClass(tukk itemClass) const;
	virtual bool                        IsAvailableSlotForItemCategory(tukk itemCategory) const;
	virtual bool                        AreItemsInSameSlot(tukk itemClass1, tukk itemClass2) const;
	virtual IInventory::EInventorySlots GetSlotForItemCategory(tukk itemCategory) const;

	virtual void                        AddListener(IInventoryListener* pListener);
	virtual void                        RemoveListener(IInventoryListener* pListener);

	virtual void                        IgnoreNextClear();

	//~IInventory

	i32          GetAccessorySlotIndex(IEntityClass* accessoryClass) const;

	virtual void GetMemoryUsage(IDrxSizer* s) const
	{
		s->AddObject(this, sizeof(*this));
		s->AddObject(m_stats);
		s->AddObject(m_editorstats);
	}

#if DEBUG_INVENTORY_ENABLED
	void Dump() const;
#endif

	// used for clients to request changes to the server.
	DECLARE_SERVER_RMI_NOATTACH(SvReq_RemoveAllItems, TRMIInventory_Dummy, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_NOATTACH(SvReq_AddItem, TRMIInventory_Item, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_NOATTACH(SvReq_RemoveItem, TRMIInventory_Item, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_NOATTACH(SvReq_SetAmmoCount, TRMIInventory_Ammo, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_NOATTACH(SvReq_AddEquipmentPack, TRMIInventory_EquipmentPack, eNRT_ReliableOrdered);

	// used to set changes into the clients.
	DECLARE_CLIENT_RMI_NOATTACH(Cl_SetAmmoCount, TRMIInventory_Ammo, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(Cl_RemoveAllAmmo, TRMIInventory_Dummy, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(Cl_SetAmmoCapacity, TRMIInventory_Ammo, eNRT_ReliableOrdered);

private:

	i32      Validate();

	void     SetLastSelectedInSlot(EntityId entityId);
	void     AddItemToCategorySlot(EntityId entityId);
	void     ResetAmmoAndUsers();
	EInventorySlots GetSlotFromEntityID(EntityId) const;
	EntityId GetAnyEntityInSlot(i32 slot) const;

	struct compare_slots
	{
		compare_slots()
		{
			m_pEntitySystem = gEnv->pEntitySystem;
			m_pItemSystem = CDrxAction::GetDrxAction()->GetIItemSystem();
		}
		bool operator()(const EntityId lhs, const EntityId rhs) const
		{
			const IEntity* pLeft = m_pEntitySystem->GetEntity(lhs);
			const IEntity* pRight = m_pEntitySystem->GetEntity(rhs);
			u32k lprio = pLeft ? m_pItemSystem->GetItemPriority(pLeft->GetClass()->GetName()) : 0;
			u32k rprio = pRight ? m_pItemSystem->GetItemPriority(pRight->GetClass()->GetName()) : 0;

			if (lprio != rprio)
				return lprio < rprio;
			else
				return lhs < rhs;
		}

		IEntitySystem* m_pEntitySystem;
		IItemSystem*   m_pItemSystem;
	};

	struct compare_class_slots
	{
		compare_class_slots()
		{
			m_pItemSystem = CDrxAction::GetDrxAction()->GetIItemSystem();
		}
		bool operator()(const IEntityClass* lhs, const IEntityClass* rhs) const
		{
			u32k lprio = m_pItemSystem->GetItemPriority(lhs->GetName());
			u32k rprio = m_pItemSystem->GetItemPriority(rhs->GetName());

			return lprio < rprio;
		}
		IItemSystem* m_pItemSystem;
	};

	struct SSlotInfo
	{
		SSlotInfo()
			: maxCapacity(0)
			, count(0)
			, lastSelected(0)
		{

		}

		void Reset()
		{
			count = 0;
			lastSelected = 0;
		}

		u32 count;
		u32 maxCapacity;
		EntityId     lastSelected;
	};

	typedef std::map<string, IInventory::EInventorySlots> TCategoriesToSlot;

	struct SInventoryStats
	{
		SInventoryStats()
			: currentItemId(0),
			holsteredItemId(0),
			lastItemId(0)
		{};

		void GetMemoryUsage(IDrxSizer* s) const
		{
			s->AddContainer(ammoInfo);
			s->AddContainer(slots);

			for (TCategoriesToSlot::const_iterator it = categoriesToSlot.begin(); it != categoriesToSlot.end(); ++it)
			{
				s->AddObject(it->first);
			}
		}

		TInventoryVector       slots;
		TInventoryVectorEx     accessorySlots;    //Items in inventory, but the entity might not exit, or is in a shared pool
		TAmmoInfoMap           ammoInfo;
		SSlotInfo              slotsInfo[IInventory::eInventorySlot_Last];
		TCategoriesToSlot      categoriesToSlot;

		TAmmoInfoMap::iterator ammoIterator;

		EntityId               currentItemId;
		EntityId               holsteredItemId;
		EntityId               lastItemId;
	};

	SInventoryStats m_stats;
	SInventoryStats m_editorstats;
	TListenerVec    m_listeners;

	IGameFramework* m_pGameFrameWork;
	IActor*         m_pActor;
	bool            m_bSerializeLTL;
	bool            m_iteratingListeners;
	bool            m_ignoreNextClear;
};

#endif //__INVENTORY_H__
