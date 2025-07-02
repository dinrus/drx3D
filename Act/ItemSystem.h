// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Manages items.

   -------------------------------------------------------------------------
   История:
   - 29:9:2004   18:01 : Created by Márcio Martins

*************************************************************************/
#ifndef __ITEMSYSTEM_H__
#define __ITEMSYSTEM_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/CoreX/Game/IGameFramework.h>
#include "IItemSystem.h"
#include <drx3D/Act/ILevelSystem.h>
#include "ItemParams.h"
#include "EquipmentUpr.h"

#include <drx3D/CoreX/Containers/DrxListenerSet.h>

#define DEFAULT_ITEM_SCRIPT     "Scripts/Entities/Items/Item.lua"
#define DEFAULT_ITEM_CREATEFUNC "CreateItemTable"
//#define USE_LTL_PRECACHING

//#if defined(USER_benito)
//#define ITEM_SYSTEM_DEBUG_MEMSTATS
//#endif

class CItemSystem :
	public ILevelSystemListener,
	public IItemSystem
{
public:
	CItemSystem(IGameFramework* pGameFramework, ISystem* pSystem);
	virtual ~CItemSystem();

	void Release() { delete this; };
	void Update();

	// IItemSystem
	virtual void                   Reload();
	virtual void                   Reset();
	virtual void                   Scan(tukk folderName);
	virtual IItemParamsNode*       CreateParams() { return new CItemParamsNode; };
	virtual const IItemParamsNode* GetItemParams(tukk itemName) const;
	virtual i32                    GetItemParamsCount() const;
	virtual tukk            GetItemParamName(i32 index) const;
	virtual tukk            GetItemParamsDescriptionFile(tukk itemName) const;
	virtual u8                  GetItemPriority(tukk item) const;
	virtual tukk            GetItemCategory(tukk item) const;
	virtual u8                  GetItemUniqueId(tukk item) const;

	virtual bool                   IsItemClass(tukk name) const;

	virtual tukk            GetFirstItemClass();
	virtual tukk            GetNextItemClass();

	virtual void                   RegisterForCollection(EntityId itemId);
	virtual void                   UnregisterForCollection(EntityId itemId);

	virtual void                   AddItem(EntityId itemId, IItem* pItem);
	virtual void                   RemoveItem(EntityId itemId, tukk itemName = NULL);
	virtual IItem*                 GetItem(EntityId itemId) const;

	virtual ICharacterInstance*    GetCachedCharacter(tukk fileName);
	virtual IStatObj*              GetCachedObject(tukk fileName);
	virtual void                   CacheObject(tukk fileName, bool useCgfStreaming);
	virtual void                   CacheGeometry(const IItemParamsNode* geometry);
	virtual void                   CacheItemGeometry(tukk className);
	virtual void                   ClearGeometryCache();

	virtual void                   CacheItemSound(tukk className);
	virtual void                   ClearSoundCache();

	virtual EntityId               GiveItem(IActor* pActor, tukk item, bool sound, bool select = true, bool keepHistory = true, tukk setup = NULL, EEntityFlags entityFlags = (EEntityFlags)0);
	virtual void                   SetActorItem(IActor* pActor, EntityId itemId, bool keepHistory);
	virtual void                   SetActorItem(IActor* pActor, tukk name, bool keepHistory);
	virtual void                   SetActorAccessory(IActor* pActor, EntityId itemId, bool keepHistory);
	virtual void                   DropActorItem(IActor* pActor, EntityId itemId);
	virtual void                   DropActorAccessory(IActor* pActor, EntityId itemId);

	virtual void                   SetConfiguration(tukk name) { m_config = name; };
	virtual tukk            GetConfiguration() const           { return m_config.c_str(); };

	virtual uk                  Query(IItemSystemQuery query, ukk param = NULL);

	virtual void                   RegisterListener(IItemSystemListener* pListener);
	virtual void                   UnregisterListener(IItemSystemListener* pListener);

	virtual void                   Serialize(TSerialize ser);
	virtual void                   SerializePlayerLTLInfo(bool bReading);

	virtual IEquipmentUpr*     GetIEquipmentUpr() { return m_pEquipmentUpr; }

	virtual u32                 GetItemSocketCount(tukk item) const;
	virtual tukk            GetItemSocketName(tukk item, i32 idx) const;
	virtual bool                   IsCompatible(tukk item, tukk attachment) const;
	virtual bool                   GetItemSocketCompatibility(tukk item, tukk socket) const;
	virtual bool                   CanSocketBeEmpty(tukk item, tukk socket) const;

	// ~IItemSystem

	// ILevelSystemListener
	virtual void OnLevelNotFound(tukk levelName)          {};
	virtual void OnLoadingLevelEntitiesStart(ILevelInfo* pLevel) {}
	virtual void OnLoadingStart(ILevelInfo* pLevel);
	virtual void OnLoadingComplete(ILevelInfo* pLevel);
	virtual void OnLoadingError(ILevelInfo* pLevel, tukk error)     {};
	virtual void OnLoadingProgress(ILevelInfo* pLevel, i32 progressAmount) {};
	virtual void OnUnloadComplete(ILevelInfo* pLevel);
	//~ILevelSystemListener

	bool ScanXML(XmlNodeRef& root, tukk xmlFile);

	void RegisterItemClass(tukk name, IGameFramework::IItemCreator* pCreator);
	void PrecacheLevel();

	void GetMemoryUsage(IDrxSizer* s) const;

	void PreReload();
	void PostReload();

private:
	void        CreateItemTable(tukk name);
	void        RegisterCVars();
	void        UnregisterCVars();
	void        InsertFolder(tukk folder);

	static void GiveItemCmd(IConsoleCmdArgs* args);
	static void DropItemCmd(IConsoleCmdArgs* args);
	static void GiveItemsHelper(IConsoleCmdArgs* args, bool useGiveable, bool useDebug);
	static void GiveAllItemsCmd(IConsoleCmdArgs* args);
	static void GiveAmmoCmd(IConsoleCmdArgs* args);
	static void GiveDebugItemsCmd(IConsoleCmdArgs* args);
	static void ListItemNames(IConsoleCmdArgs* args);
	static void SaveWeaponPositionCmd(IConsoleCmdArgs* args);

	static ICVar* m_pPrecache;
	static ICVar* m_pItemLimitMP;
	static ICVar* m_pItemLimitSP;

	void DisplayItemSystemStats();
	void DumpItemList(tukk filter);
	void FindItemName(tukk nameFilter);
	void ItemSystemErrorMessage(tukk fileName, tukk errorInfo, bool displayErrorDialog);

#ifdef USE_LTL_PRECACHING
	void PreCacheLevelToLevelLoadout();

	typedef DrxFixedArray<const IEntityClass*, 64> TLevelToLevelItemArray;
	TLevelToLevelItemArray m_precacheLevelToLevelItemList;
	enum ELTLPreCacheState
	{
		LTLCS_FORNEXTLEVELLOAD, // indicates that the caching has to be done right after the next level is loaded
		LTLCS_FORSAVEGAME       // indicates that the list is only used for creating saveloads. If a level has LTL items coming from a previous level, we need
		                        // to save them on every savegame created in that level, because we need to recache them in case the player quits and then resume game.
	};
	ELTLPreCacheState m_LTLPrecacheState;
#endif

	struct SSpawnUserData
	{
		SSpawnUserData(tukk cls, u16 channel) : className(cls), channelId(channel) {}
		tukk className;
		u16      channelId;
	};

	typedef std::map<string, IStatObj*>           TObjectCache;
	typedef TObjectCache::iterator                TObjectCacheIt;

	typedef std::map<string, ICharacterInstance*> TCharacterCache;
	typedef TCharacterCache::iterator             TCharacterCacheIt;

	typedef struct SItemParamsDesc
	{
		enum ItemFlags
		{
			eIF_PreCached_Sound    = 1 << 0,
			eIF_PreCached_Geometry = 1 << 1
		};

		SItemParamsDesc() : precacheFlags(0), params(0) {};
		~SItemParamsDesc()
		{
			SAFE_RELEASE(params);

			if (!configurations.empty())
			{
				for (std::map<string, CItemParamsNode*>::iterator it = configurations.begin(); it != configurations.end(); ++it)
				{
					SAFE_RELEASE(it->second);
				}
			}
			configurations.clear();
		};

		void GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(filePath);
			pSizer->AddObject(category);
			pSizer->AddObject(params);
			pSizer->AddObject(configurations);
		}

		CItemParamsNode*                   params;
		string                             filePath;
		string                             category;
		u8                              uniqueId;
		u8                              priority;
		u8                              precacheFlags;

		std::map<string, CItemParamsNode*> configurations;

	} SItemParamsDesc;

	typedef struct SItemClassDesc
	{
		SItemClassDesc() : pCreator(0) {};
		SItemClassDesc(IGameObjectExtensionCreatorBase* pCrtr) : pCreator(pCrtr) {};

		IGameObjectExtensionCreatorBase* pCreator;
		void GetMemoryUsage(IDrxSizer* pSizer) const { /*nothing*/ }
	} SItemClassDesc;

	typedef std::map<string, SItemClassDesc>   TItemClassMap;
	typedef std::map<string, SItemParamsDesc>  TItemParamsMap;

	typedef std::map<string, i32>              TItemSystemSpawnPolicy;

	typedef std::map<EntityId, IItem*>         TItemMap;
	typedef CListenerSet<IItemSystemListener*> TListeners;

	typedef std::vector<string>                TFolderList;

	typedef std::map<EntityId, CTimeValue>     TCollectionMap;

	CTimeValue                     m_precacheTime;

	TObjectCache                   m_objectCache;
	TCharacterCache                m_characterCache;

	ISystem*                       m_pSystem;
	IGameFramework*                m_pGameFramework;
	IEntitySystem*                 m_pEntitySystem;

	XmlNodeRef                     m_playerLevelToLevelSave;

	TItemClassMap                  m_classes;
	TItemParamsMap                 m_params;
	TItemMap                       m_items;
	TListeners                     m_listeners;
	u32                         m_spawnCount;

	TCollectionMap                 m_collectionmap;

	TFolderList                    m_folders;
	bool                           m_reloading;
	bool                           m_recursing;
	bool                           m_itemParamsFlushed;

	string                         m_config;

	CEquipmentUpr*             m_pEquipmentUpr;

	TItemParamsMap::const_iterator m_itemClassIterator;

	i32                            i_inventory_capacity;
};

#endif //__ITEMSYSTEM_H__
