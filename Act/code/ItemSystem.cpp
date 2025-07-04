// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   История:
   - 29:9:2004   18:02 : Created by Mrcio Martins

 *************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/ItemSystem.h>
#include <drx3D/CoreX/String/Path.h>
//#include <drx3D/Act/ScriptBind_ItemSystem.h>
#include <drx3D/Act/IActionMapUpr.h>
#include <drx3D/Act/GameObject.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/IGameObjectSystem.h>
#include <drx3D/Act/IActorSystem.h>

#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Act/ItemParams.h>
#include <drx3D/Act/EquipmentUpr.h>
#include <drx3D/Act/DrxActionCVars.h>

#include <drx3D/Act/IGameRulesSystem.h>

ICVar* CItemSystem::m_pPrecache = 0;
ICVar* CItemSystem::m_pItemLimitMP = 0;
ICVar* CItemSystem::m_pItemLimitSP = 0;

//This is used for AI offhands, if we want more classes, perhaps it's better
//if every class can specify the size of its shared pool
#define ITEM_SHARED_POOL_MAX_SIZE 5

//------------------------------------------------------------------------

#if !defined(_RELEASE)
// Auto complete for i_giveitem cmd.
struct SItemListAutoComplete : public IConsoleArgumentAutoComplete
{
	// Gets number of matches for the argument to auto complete.
	virtual i32 GetCount() const
	{
		IGameFramework* pGameFramework = gEnv->pGameFramework;
		IItemSystem* pItemSystem = pGameFramework->GetIItemSystem();
		return pItemSystem->GetItemParamsCount();
	}

	// Gets argument value by index, nIndex must be in 0 <= nIndex < GetCount()
	virtual tukk GetValue(i32 nIndex) const
	{
		IGameFramework* pGameFramework = gEnv->pGameFramework;
		IItemSystem* pItemSystem = pGameFramework->GetIItemSystem();
		return pItemSystem->GetItemParamName(nIndex);
	}
};

static SItemListAutoComplete s_itemListAutoComplete;
#endif // !defined(_RELEASE)

//------------------------------------------------------------------------
CItemSystem::CItemSystem(IGameFramework* pGameFramework, ISystem* pSystem)
	: m_pGameFramework(pGameFramework),
	m_pSystem(pSystem),
	m_pEntitySystem(gEnv->pEntitySystem),
	m_spawnCount(0),
	m_precacheTime(0.0f),
	m_reloading(false),
	m_recursing(false),
	m_listeners(2)
{
#ifndef USE_LTL_PRECACHING
	m_itemParamsFlushed = false;
#else
	m_itemParamsFlushed = true;
	m_LTLPrecacheState = LTLCS_FORSAVEGAME; // sanity. Irrelevant as the list is empty anyway at this point
#endif

	RegisterCVars();
	m_pEquipmentUpr = new CEquipmentUpr(this);
	m_itemClassIterator = m_params.begin();
}

//------------------------------------------------------------------------
CItemSystem::~CItemSystem()
{
	SAFE_DELETE(m_pEquipmentUpr);
	UnregisterCVars();

	ClearGeometryCache();
	ClearSoundCache();
}

//------------------------------------------------------------------------
void CItemSystem::OnLoadingStart(ILevelInfo* pLevelInfo)
{
	Reset();

	ClearGeometryCache();
	ClearSoundCache();
}

//------------------------------------------------------------------------
void CItemSystem::OnLoadingComplete(ILevelInfo* pLevel)
{
	// marcio: precaching of items enabled by default for now
	//	ICVar *sys_preload=gEnv->pConsole->GetCVar("sys_preload");
	//	if ((!sys_preload || sys_preload->GetIVal()) && m_pPrecache->GetIVal())
	{
		PrecacheLevel();
	}
}

//------------------------------------------------------------------------
void CItemSystem::OnUnloadComplete(ILevelInfo* pLevel)
{
	m_listeners.Clear(true);
#ifdef USE_LTL_PRECACHING
	if (m_LTLPrecacheState == LTLCS_FORSAVEGAME)
		m_precacheLevelToLevelItemList.clear(); // just for sanity reasons.
#endif
}

//------------------------------------------------------------------------
void CItemSystem::RegisterItemClass(tukk name, IGameFramework::IItemCreator* pCreator)
{
	m_classes.insert(TItemClassMap::value_type(name, SItemClassDesc(pCreator)));
}

//------------------------------------------------------------------------
void CItemSystem::Update()
{
	DRX_PROFILE_FUNCTION(PROFILE_ACTION);

	ITimer* pTimer = gEnv->pTimer;
	IActor* pActor = m_pGameFramework->GetClientActor();

	if (pActor)
	{
		IInventory* pInventory = pActor->GetInventory();
		if (pInventory)
		{
			IItem* pCurrentItem = GetItem(pInventory->GetCurrentItem());
			if (pCurrentItem && pCurrentItem->IsSelected())
				pCurrentItem->UpdateFPView(pTimer->GetFrameTime());

			IItem* pLastItem = GetItem(pInventory->GetLastItem());
			if (pLastItem && pLastItem != pCurrentItem && pLastItem->IsSelected())
				pLastItem->UpdateFPView(pTimer->GetFrameTime());

			// marcok: argh ... we depend on Drxsis game stuff here ... REMOVE
			IEntityClass* pOffHandClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("OffHand");
			IItem* pOffHandItem = GetItem(pInventory->GetItemByClass(pOffHandClass));

			// HAX: update offhand - shouldn't be here
			if (pOffHandItem)
				pOffHandItem->UpdateFPView(pTimer->GetFrameTime());
		}
	}

	if (CDrxActionCVars::Get().debugItemMemStats != 0)
	{
		DisplayItemSystemStats();
	}

}

//------------------------------------------------------------------------
const IItemParamsNode* CItemSystem::GetItemParams(tukk itemName) const
{
	TItemParamsMap::const_iterator it = m_params.find(CONST_TEMP_STRING(itemName));
	if (it != m_params.end())
	{
		if (m_config.empty())
		{
			return it->second.params;
		}

		std::map<string, CItemParamsNode*>::const_iterator cit = it->second.configurations.find(m_config);
		if (cit != it->second.configurations.end())
		{
			return cit->second;
		}

		return it->second.params;
	}

	GameWarning("Trying to get xml description for item '%s'! Something very wrong has happened!", itemName);
	return 0;
}

//------------------------------------------------------------------------
u8 CItemSystem::GetItemPriority(tukk itemName) const
{
	TItemParamsMap::const_iterator it = m_params.find(CONST_TEMP_STRING(itemName));
	if (it != m_params.end())
		return it->second.priority;

	GameWarning("Trying to get priority for item '%s'! Something very wrong has happened!", itemName);
	return 0;
}

//------------------------------------------------------------------------
tukk CItemSystem::GetItemCategory(tukk itemName) const
{
	TItemParamsMap::const_iterator it = m_params.find(CONST_TEMP_STRING(itemName));
	if (it != m_params.end())
		return it->second.category.c_str();

	GameWarning("Trying to get category for item '%s'! Something very wrong has happened!", itemName);
	return 0;
}

//-------------------------------------------------------------------------
u8 CItemSystem::GetItemUniqueId(tukk itemName) const
{
	TItemParamsMap::const_iterator it = m_params.find(CONST_TEMP_STRING(itemName));
	if (it != m_params.end())
		return it->second.uniqueId;

	GameWarning("Trying to get uniqueId for item '%s'! Something very wrong has happened!", itemName);
	return 0;
}

//------------------------------------------------------------------------
bool CItemSystem::IsItemClass(tukk name) const
{
	TItemParamsMap::const_iterator it = m_params.find(CONST_TEMP_STRING(name));
	return (it != m_params.end());
}

//------------------------------------------------------------------------
tukk CItemSystem::GetFirstItemClass()
{
	m_itemClassIterator = m_params.begin();

	if (m_itemClassIterator != m_params.end())
		return m_itemClassIterator->first;
	return 0;
}

//------------------------------------------------------------------------
tukk CItemSystem::GetNextItemClass()
{
	if (m_itemClassIterator != m_params.end())
	{
		++m_itemClassIterator;
		if (m_itemClassIterator != m_params.end())
			return m_itemClassIterator->first;
	}
	return 0;
}

//------------------------------------------------------------------------
void CItemSystem::RegisterForCollection(EntityId itemId)
{
#if _DEBUG
	IItem* pItem = GetItem(itemId);
	DRX_ASSERT(pItem);

	if (pItem)
	{
		DRX_ASSERT(!pItem->GetOwnerId());
	}
#endif

	CTimeValue now = gEnv->pTimer->GetFrameStartTime();

	TCollectionMap::iterator cit = m_collectionmap.find(itemId);
	if (cit == m_collectionmap.end())
		m_collectionmap.insert(TCollectionMap::value_type(itemId, now));
	else
		cit->second = now;

	if (gEnv->bServer)
	{
		i32 limit = 0;
		const ICVar* pItemLimit = gEnv->bMultiplayer ? m_pItemLimitMP : m_pItemLimitSP;
		if (pItemLimit)
			limit = pItemLimit->GetIVal();

		if (limit > 0)
		{
			i32 size = (i32)m_collectionmap.size();
			if (size > limit)
			{
				EntityId nItemId = 0;
				for (TCollectionMap::const_iterator it = m_collectionmap.begin(); it != m_collectionmap.end(); ++it)
				{
					if (it->second < now)
					{
						now = it->second;
						nItemId = it->first;
					}
				}

				if (IItem* pItem = GetItem(nItemId))
				{
					DRX_ASSERT(!pItem->GetOwnerId());

					DrxLogAlways("[game] Removing item %s due to i_lying_item_limit!", pItem->GetEntity()->GetName());
					UnregisterForCollection(nItemId);
					gEnv->pEntitySystem->RemoveEntity(nItemId);
				}
			}
		}
	}
}

//------------------------------------------------------------------------
void CItemSystem::UnregisterForCollection(EntityId itemId)
{
	stl::member_find_and_erase(m_collectionmap, itemId);
}

//------------------------------------------------------------------------
void CItemSystem::Reload()
{
	m_reloading = true;

	m_params.clear();

	for (TFolderList::iterator fit = m_folders.begin(); fit != m_folders.end(); ++fit)
		Scan(fit->c_str());

	SEntityEvent event(ENTITY_EVENT_RESET);

	for (TItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		IEntity* pEntity = m_pEntitySystem->GetEntity(it->first);
		if (pEntity)
			pEntity->SendEvent(event);
	}

	m_reloading = false;
}

//------------------------------------------------------------------------
void CItemSystem::PreReload()
{
	m_reloading = true;

	m_params.clear();

	for (TFolderList::iterator fit = m_folders.begin(); fit != m_folders.end(); ++fit)
		Scan(fit->c_str());

	for (TItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		IItem* pItem = it->second;
		if (pItem)
			pItem->PreResetParams();
	}
}

//------------------------------------------------------------------------
void CItemSystem::PostReload()
{
	SEntityEvent event(ENTITY_EVENT_RESET);

	for (TItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		IItem* pItem = it->second;
		if (pItem)
			pItem->ResetParams();
	}

	for (TItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		IEntity* pEntity = m_pEntitySystem->GetEntity(it->first);
		if (pEntity)
			pEntity->SendEvent(event);
	}

	m_reloading = false;
}

//------------------------------------------------------------------------
void CItemSystem::Reset()
{
	if (GetISystem()->IsSerializingFile() == 1)
	{
		IEntitySystem* pEntitySystem = gEnv->pEntitySystem;
		TItemMap::iterator it = m_items.begin();
		TItemMap::iterator endIt = m_items.end();

		for (; it != endIt; )
		{
			EntityId id = it->first;
			IEntity* pEntity = pEntitySystem->GetEntity(id);
			it->second->Reset();

			if (!pEntity)
			{
				TItemMap::iterator here = it++;
				m_items.erase(here);
			}
			else
			{
				++it;
			}
		}
	}

	m_pEquipmentUpr->Reset();
}

//------------------------------------------------------------------------
void CItemSystem::Scan(tukk folderName)
{
	string folder = folderName;
	string search = folder;
	search += "/*.*";

	IDrxPak* pPak = gEnv->pDrxPak;

	_finddata_t fd;
	intptr_t handle = pPak->FindFirst(search.c_str(), &fd);

	INDENT_LOG_DURING_SCOPE(!m_recursing, "Scanning '%s' for item files...", folderName);

	if (!m_recursing)
		DrxComment("Loading item XML definitions from '%s'!", folderName);

	if (handle > -1)
	{
		do
		{
			if (!strcmp(fd.name, ".") || !strcmp(fd.name, ".."))
				continue;

			if (fd.attrib & _A_SUBDIR)
			{
				string subName = folder + "/" + fd.name;
				if (m_recursing)
					Scan(subName.c_str());
				else
				{
					m_recursing = true;
					Scan(subName.c_str());
					m_recursing = false;
				}
				continue;
			}

			tukk fileExtension = PathUtil::GetExt(fd.name);
			if (stricmp(fileExtension, "xml"))
			{
				continue;
			}

			string xmlFile = folder + string("/") + string(fd.name);
			XmlNodeRef rootNode = m_pSystem->LoadXmlFromFile(xmlFile.c_str());

			if (!rootNode)
			{
				ItemSystemErrorMessage(xmlFile.c_str(), "Root xml node couldn't be loaded", true);
				continue;
			}

			if (!ScanXML(rootNode, xmlFile.c_str()))
			{
				continue;
			}

		}
		while (pPak->FindNext(handle, &fd) >= 0);
	}

	if (!m_recursing)
		DrxLog("Finished loading item XML definitions from '%s'!", folderName);

	if (!m_reloading && !m_recursing)
	{
		InsertFolder(folderName);
	}
}

//------------------------------------------------------------------------
bool CItemSystem::ScanXML(XmlNodeRef& root, tukk xmlFile)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "ItemSystem");
	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Other, 0, "Item XML (%s)", xmlFile);

	if (strcmpi(root->getTag(), "item"))
	{
		// We don't want to report error here, we have other files in the same folder like ammo with different tag
		return false;
	}

	tukk name = root->getAttr("name");
	if (!name)
	{
		ItemSystemErrorMessage(xmlFile, "Item definition file does not contain attribute 'name'! Skipping...", true);
		return false;
	}

	tukk className = root->getAttr("class");

	if (!className)
	{
		ItemSystemErrorMessage(xmlFile, "Item definition file does not contain attribute 'class'! Skipping...", true);
		return false;
	}

	INDENT_LOG_DURING_SCOPE(true, "Item system parsing '%s' file (name='%s' class='%s')", xmlFile, name, className);

	TItemClassMap::iterator it = m_classes.find(CONST_TEMP_STRING(className));
	if (it == m_classes.end())
	{
		DrxFixedStringT<128> errorBuffer;
		errorBuffer.Format("Unknown item class '%s'! Skipping...", className);
		ItemSystemErrorMessage(xmlFile, errorBuffer.c_str(), true);
		return false;
	}

	TItemParamsMap::iterator dit = m_params.find(CONST_TEMP_STRING(name));

	if (dit == m_params.end())
	{
		tukk scriptName = root->getAttr("script");
		IEntityClassRegistry::SEntityClassDesc classDesc;
		classDesc.sName = name;
		if (scriptName && scriptName[0])
			classDesc.sScriptFile = scriptName;
		else
		{
			classDesc.sScriptFile = DEFAULT_ITEM_SCRIPT;
			CreateItemTable(name);
		}

		i32 invisible = 0;
		root->getAttr("invisible", invisible);

		classDesc.pUserProxyCreateFunc = (IEntityClass::UserProxyCreateFunc)it->second.pCreator;
		classDesc.flags |= invisible ? ECLF_INVISIBLE : 0;

		if (!m_reloading)
			CDrxAction::GetDrxAction()->GetIGameObjectSystem()->RegisterExtension(name, it->second.pCreator, &classDesc);

		std::pair<TItemParamsMap::iterator, bool> result = m_params.insert(TItemParamsMap::value_type(name, SItemParamsDesc()));
		dit = result.first;
	}

	SItemParamsDesc& desc = dit->second;

#if 0
	// deprecated and won't compile at all...
	if (!configName || !configName[0])
	{
		SAFE_RELEASE(desc.params);

		desc.params = new CItemParamsNode();
		bool filterMP = desc.params->ConvertFromXMLWithFiltering(root, "SP");

		desc.priority = 0;
		i32 p = 0;
		if (desc.params->GetAttribute("priority", p))
		{
			desc.priority = (u8)p;
		}
		desc.category = desc.params->GetAttribute("category");
		desc.uniqueId = 0;
		i32 id = 0;
		if (desc.params->GetAttribute("uniqueId", id))
		{
			desc.uniqueId = (u8)id;
		}

		if (filterMP)
		{
			CItemParamsNode* params = new CItemParamsNode();
			params->ConvertFromXMLWithFiltering(root, "MP");
			desc.configurations.insert(std::make_pair<string, CItemParamsNode*>("mp", params));
		}
	}
	else
	{
		// to check that a weapon has multiple configuration (we support that for different game modes)
		//		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Non-empty configuration found in \"%s\"!!!", xmlFile);

		CItemParamsNode* params = new CItemParamsNode();
		params->ConvertFromXML(root);
		desc.configurations.insert(std::make_pair<string, CItemParamsNode*>(configName, params));
	}
#else
	{
		DRX_ASSERT(desc.params == NULL);

		i32 priority = 0;
		if (root->getAttr("priority", priority))
		{
			desc.priority = (u8)priority;
		}
		else
		{
			desc.priority = 0;
		}

		desc.category = root->getAttr("category");

		i32 uniqueId = 0;
		if (root->getAttr("uniqueId", uniqueId))
		{
			desc.uniqueId = (u8)uniqueId;
		}
		else
		{
			desc.uniqueId = 0;
		}

		desc.filePath = xmlFile;
	}
#endif

	return true;
}

//------------------------------------------------------------------------
void CItemSystem::AddItem(EntityId itemId, IItem* pItem)
{
	m_items.insert(TItemMap::value_type(itemId, pItem));

}

//------------------------------------------------------------------------
void CItemSystem::RemoveItem(EntityId itemId, tukk itemName)
{
	stl::member_find_and_erase(m_items, itemId);
	stl::member_find_and_erase(m_collectionmap, itemId);
}

//------------------------------------------------------------------------
IItem* CItemSystem::GetItem(EntityId itemId) const
{
	TItemMap::const_iterator it = m_items.find(itemId);

	if (it != m_items.end())
	{
		return it->second;
	}
	return 0;
}

//------------------------------------------------------------------------
EntityId CItemSystem::GiveItem(IActor* pActor, tukk item, bool sound, bool select, bool keepHistory, tukk setup, EEntityFlags entityFlags)
{
	if (!gEnv->bServer && !(entityFlags & ENTITY_FLAG_CLIENT_ONLY))
	{
		GameWarning("Trying to spawn item of class '%s' on a process which is not server!", item);
		return 0;
	}

	if (gEnv->pSystem->IsSerializingFile())
		return 0;

	DRX_ASSERT(item && pActor);

	INDENT_LOG_DURING_SCOPE(true, "Giving %s a new item of class %s (sound=%u select=%u keepHistory=%u setup='%s')", pActor->GetEntity()->GetEntityTextDescription().c_str(), item, sound, select, keepHistory, setup ? setup : "N/A");

	static char itemName[65];
	drx_sprintf(itemName, "%s%.03u", item, ++m_spawnCount);
	SEntitySpawnParams params;
	params.sName = itemName;
	params.pClass = m_pEntitySystem->GetClassRegistry()->FindClass(item);
	params.nFlags |= (ENTITY_FLAG_NO_PROXIMITY | ENTITY_FLAG_NEVER_NETWORK_STATIC | entityFlags);
	if (!params.pClass)
	{
		GameWarning("Trying to spawn item of class '%s' which is unknown!", item);
		return 0;
	}

	if (IEntity* pItemEnt = m_pEntitySystem->SpawnEntity(params))
	{
		EntityId itemEntId = pItemEnt->GetId();
		IItem* pItem = GetItem(itemEntId);
		DRX_ASSERT_MESSAGE(pItem, "Just spawned an entity assuming it was an item but it isn't");
		if (pItem)
		{
			// this may remove the entity
			pItem->PickUp(pActor->GetEntityId(), sound, select, keepHistory, setup);

			//[kirill] make sure AI gets notified about new item
			if (gEnv->pAISystem)
			{
				if (IAIObject* pActorAI = pActor->GetEntity()->GetAI())
					gEnv->pAISystem->SendSignal(SIGNALFILTER_SENDER, 0, "OnUpdateItems", pActorAI);
			}

			if ((pItemEnt = gEnv->pEntitySystem->GetEntity(itemEntId)) && !pItemEnt->IsGarbage())
			{
				// set properties table to null, since they'll not be used [timur's request]
				if (pItemEnt->GetScriptTable())
					pItemEnt->GetScriptTable()->SetToNull("Properties");

				return pItemEnt->GetId();
			}
		}
	}

	return 0;
}

//------------------------------------------------------------------------
void CItemSystem::SetActorItem(IActor* pActor, EntityId itemId, bool keepHistory)
{
	if (!pActor)
		return;
	IInventory* pInventory = pActor->GetInventory();
	if (!pInventory)
		return;

	EntityId currentItemId = pInventory->GetCurrentItem();

	IItem* pItem = GetItem(itemId);

	if (currentItemId == itemId)
	{
		if (keepHistory)
			pInventory->SetLastItem(currentItemId);

		if (pItem)
			pItem->Select(true);
		return;
	}

	if (currentItemId)
	{
		IItem* pCurrentItem = GetItem(currentItemId);
		if (pCurrentItem)
		{
			pInventory->SetCurrentItem(0);
			pCurrentItem->Select(false);
			if (keepHistory)
				pInventory->SetLastItem(currentItemId);
		}
	}

	for (TListeners::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
	{
		notifier->OnSetActorItem(pActor, pItem);
	}

	if (pItem == nullptr && itemId != INVALID_ENTITYID)
	{
#ifndef _RELEASE
		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(itemId);
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "%s tried to select item ID %u but that ID does not belong to an item, it belongs to %s", pActor->GetEntity()->GetName(), itemId, pEntity ? pEntity->GetEntityTextDescription().c_str() : "nothing");
#endif
		return;
	}

	CDrxAction::GetDrxAction()->GetIGameplayRecorder()->Event(pActor->GetEntity(), GameplayEvent(eGE_ItemSelected, 0, 0, (uk )(EXPAND_PTR)itemId));
	pActor->NotifyCurrentItemChanged(pItem);

	if (!pItem)
		return;

	pInventory->SetCurrentItem(itemId);

	pItem->SetHand(IItem::eIH_Right);
	pItem->Select(true);
}

//------------------------------------------------------------------------
void CItemSystem::SetActorAccessory(IActor* pActor, EntityId itemId, bool keepHistory)
{
	if (!pActor)
		return;

	IInventory* pInventory = pActor->GetInventory();
	if (!pInventory)
		return;

	IItem* pItem = GetItem(itemId);
	for (TListeners::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
	{
		notifier->OnSetActorAccessory(pActor, pItem);
	}
}

//------------------------------------------------------------------------

void CItemSystem::DropActorItem(IActor* pActor, EntityId itemId)
{
	if (!pActor)
		return;

	IInventory* pInventory = pActor->GetInventory();
	if (!pInventory)
		return;

	IItem* pItem = GetItem(itemId);
	if (!pItem)
		return;

	for (TListeners::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
	{
		notifier->OnDropActorItem(pActor, pItem);
	}
}

//------------------------------------------------------------------------

void CItemSystem::DropActorAccessory(IActor* pActor, EntityId itemId)
{
	if (!pActor)
		return;

	IInventory* pInventory = pActor->GetInventory();
	if (!pInventory)
		return;

	IItem* pItem = GetItem(itemId);
	if (!pItem) return;

	for (TListeners::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
	{
		notifier->OnDropActorAccessory(pActor, pItem);
	}
}

//------------------------------------------------------------------------
void CItemSystem::SetActorItem(IActor* pActor, tukk name, bool keepHistory)
{
	IInventory* pInventory = (pActor != NULL) ? pActor->GetInventory() : NULL;
	if (!pInventory)
		return;

	IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(name);
	EntityId itemId = pInventory->GetItemByClass(pClass);
	if (!itemId)
		return;

	SetActorItem(pActor, itemId, keepHistory);
}

//------------------------------------------------------------------------
void CItemSystem::CacheGeometry(const IItemParamsNode* geometry)
{
	if (!geometry)
		return;

	i32 n = geometry->GetChildCount();
	if (n > 0)
	{
		for (i32 i = 0; i < n; i++)
		{
			const IItemParamsNode* slot = geometry->GetChild(i);
			tukk name = slot->GetAttribute("name");
			i32 useStreaming = 1;
			slot->GetAttribute("useStreaming", useStreaming);

			if (name && name[0])
				CacheObject(name, (useStreaming != 0));
		}
	}
}

//------------------------------------------------------------------------
void CItemSystem::CacheItemGeometry(tukk className)
{
	LOADING_TIME_PROFILE_SECTION(gEnv->pSystem);
	if (m_itemParamsFlushed)
		return;

	TItemParamsMap::iterator it = m_params.find(CONST_TEMP_STRING(className));
	if (it == m_params.end())
		return;

	if ((it->second.precacheFlags & SItemParamsDesc::eIF_PreCached_Geometry) == 0)
	{
		if (const IItemParamsNode* root = GetItemParams(className))
		{
			const IItemParamsNode* geometry = root->GetChild("geometry");
			if (geometry)
				CacheGeometry(geometry);
		}

		it->second.precacheFlags |= SItemParamsDesc::eIF_PreCached_Geometry;
	}
}

//------------------------------------------------------------------------
void CItemSystem::CacheItemSound(tukk className)
{
	LOADING_TIME_PROFILE_SECTION(gEnv->pSystem);
	if (m_itemParamsFlushed)
		return;

	TItemParamsMap::iterator it = m_params.find(CONST_TEMP_STRING(className));
	if (it == m_params.end())
		return;

	if ((it->second.precacheFlags & SItemParamsDesc::eIF_PreCached_Sound) == 0)
	{
		if (const IItemParamsNode* root = GetItemParams(className))
		{
			const IItemParamsNode* actions = root->GetChild("actions");
			if (actions)
			{
				i32 n = actions->GetChildCount();
				for (i32 i = 0; i < n; i++)
				{
					const IItemParamsNode* action = actions->GetChild(i);
					if (!stricmp(action->GetName(), "action"))
					{
						i32 na = action->GetChildCount();
						for (i32 a = 0; a < na; a++)
						{
							const IItemParamsNode* sound = actions->GetChild(i);
							if (!stricmp(sound->GetName(), "sound"))
							{
								tukk soundName = sound->GetNameAttribute();
								REINST("do we still need this type of data priming?")
								//gEnv->pSoundSystem->Precache(soundName, 0, FLAG_SOUND_PRECACHE_EVENT_DEFAULT);
							}
						}
					}
				}

			}
		}

		it->second.precacheFlags |= SItemParamsDesc::eIF_PreCached_Sound;
	}
}

//------------------------------------------------------------------------
void CItemSystem::ClearSoundCache()
{
	for (TItemParamsMap::iterator it = m_params.begin(); it != m_params.end(); ++it)
		it->second.precacheFlags &= ~SItemParamsDesc::eIF_PreCached_Sound;
}

//------------------------------------------------------------------------
ICharacterInstance* CItemSystem::GetCachedCharacter(tukk fileName)
{
	DrxFixedStringT<256> name(fileName);
	name.replace('\\', '/');
	name.MakeLower();

	TCharacterCacheIt cit = m_characterCache.find(CONST_TEMP_STRING(name.c_str()));

	if (cit == m_characterCache.end())
		return 0;

	return cit->second;
}

//------------------------------------------------------------------------
IStatObj* CItemSystem::GetCachedObject(tukk fileName)
{
	DrxFixedStringT<256> name = fileName;
	name.replace('\\', '/');
	name.MakeLower();

	TObjectCacheIt oit = m_objectCache.find(CONST_TEMP_STRING(name.c_str()));

	if (oit == m_objectCache.end())
		return 0;

	return oit->second;
}

//------------------------------------------------------------------------
void CItemSystem::CacheObject(tukk fileName, bool useCgfStreaming)
{
	string name = PathUtil::ToUnixPath(string(fileName));
	name.MakeLower();

	TObjectCacheIt oit = m_objectCache.find(name);
	TCharacterCacheIt cit = m_characterCache.find(name);

	if ((oit != m_objectCache.end()) || (cit != m_characterCache.end()))
	{
		return;
	}

	stack_string ext(PathUtil::GetExt(name.c_str()));

	if ((ext == "cdf") || (ext == "chr") || (ext == "cga"))
	{
		ICharacterInstance* pChar = gEnv->pCharacterUpr->CreateInstance(fileName);
		if (pChar)
		{
			pChar->AddRef();
			m_characterCache.insert(TCharacterCache::value_type(name, pChar));
		}
	}
	else
	{
		IStatObj* pStatObj = gEnv->p3DEngine->LoadStatObj(fileName, 0, 0, useCgfStreaming);
		if (pStatObj)
		{
			pStatObj->AddRef();
			m_objectCache.insert(TObjectCache::value_type(name, pStatObj));
		}
	}
}

//------------------------------------------------------------------------
void CItemSystem::ClearGeometryCache()
{
	for (TObjectCacheIt oit = m_objectCache.begin(); oit != m_objectCache.end(); ++oit)
	{
		if (oit->second)
			oit->second->Release();
	}
	m_objectCache.clear();

	for (TCharacterCacheIt cit = m_characterCache.begin(); cit != m_characterCache.end(); ++cit)
	{
		cit->second->Release();
	}
	m_characterCache.clear();

	for (TItemParamsMap::iterator it = m_params.begin(); it != m_params.end(); ++it)
	{
		it->second.precacheFlags &= ~SItemParamsDesc::eIF_PreCached_Geometry;
	}
}

//------------------------------------------------------------------------
void CItemSystem::PrecacheLevel()
{
	DRX_PROFILE_FUNCTION(PROFILE_ACTION);

	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;

	TItemMap::iterator it = m_items.begin();
	TItemMap::iterator end = m_items.end();

	for (; it != end; ++it)
	{
		if (IEntity* pEntity = pEntitySystem->GetEntity(it->first))
		{
			CacheItemGeometry(pEntity->GetClass()->GetName());
			CacheItemSound(pEntity->GetClass()->GetName());
		}
	}

#ifdef USE_LTL_PRECACHING
	if (m_LTLPrecacheState == LTLCS_FORNEXTLEVELLOAD)
	{
		PreCacheLevelToLevelLoadout();
		m_LTLPrecacheState = LTLCS_FORSAVEGAME;
	}
	else
		m_precacheLevelToLevelItemList.clear();

#endif

}

#ifdef USE_LTL_PRECACHING
void CItemSystem::PreCacheLevelToLevelLoadout()
{
	IGameRules* pGameRules = CDrxAction::GetDrxAction()->GetIGameRulesSystem()->GetCurrentGameRules();

	DRX_ASSERT_MESSAGE(pGameRules != NULL, "No game rules active, can not preload resources");

	if (pGameRules)
	{
		//Level to level inventory resource pre-loading
		u32 numberOfElements = m_precacheLevelToLevelItemList.size();
		if (numberOfElements > 0)
		{
			for (u32 i = 0; i < numberOfElements; ++i)
			{
				DRX_ASSERT(m_precacheLevelToLevelItemList[i]);

				pGameRules->PrecacheLevelResource(m_precacheLevelToLevelItemList[i]->GetName(), eGameResourceType_Item);
			}
		}
	}
}
#endif
//------------------------------------------------------------------------
void CItemSystem::CreateItemTable(tukk name)
{
	IScriptSystem* pScriptSystem = gEnv->pScriptSystem;
	pScriptSystem->ExecuteFile(DEFAULT_ITEM_SCRIPT);
	pScriptSystem->BeginCall(DEFAULT_ITEM_CREATEFUNC);
	pScriptSystem->PushFuncParam(name);
	pScriptSystem->EndCall();
}

//------------------------------------------------------------------------
void CItemSystem::RegisterCVars()
{
	REGISTER_CVAR2("i_inventory_capacity", &i_inventory_capacity, 64, VF_CHEAT, "Players inventory capacity");
	REGISTER_COMMAND("i_giveitem", (ConsoleCommandFunc)GiveItemCmd, VF_CHEAT, "Gives specified item to the player!");
	REGISTER_COMMAND("i_dropitem", (ConsoleCommandFunc)DropItemCmd, VF_CHEAT, "Drops the current selected item!");
	REGISTER_COMMAND("i_giveallitems", (ConsoleCommandFunc)GiveAllItemsCmd, VF_CHEAT, "Gives all available items to the player!");
	REGISTER_COMMAND("i_givedebugitems", (ConsoleCommandFunc)GiveDebugItemsCmd, VF_CHEAT, "Gives special debug items to the player!");
	REGISTER_COMMAND("i_listitems", ListItemNames, VF_CHEAT, "List all item names matching the string provided as parameter.");
	REGISTER_COMMAND("i_saveweaponposition", (ConsoleCommandFunc)SaveWeaponPositionCmd, VF_CHEAT, "Saves weapon offset");
	REGISTER_COMMAND("i_giveammo", (ConsoleCommandFunc)GiveAmmoCmd, VF_CHEAT,
	                 "Sets specified ammo to the specified amount in the player's inventory.\n"
	                 "Usage: i_giveammo PistolBullet 999");

	// Auto complete
#if !defined(_RELEASE)
	assert(gEnv->pConsole);
	PREFAST_ASSUME(gEnv->pConsole);
	gEnv->pConsole->RegisterAutoComplete("i_giveitem", &s_itemListAutoComplete);
#endif // !defined(_RELEASE)

	m_pPrecache = REGISTER_INT("i_precache", 1, VF_DUMPTODISK, "Enables precaching of items during level loading.");
	m_pItemLimitMP = REGISTER_INT("i_lying_item_limit_mp", 64, 0, "Max number of items lying around in a level (<= 0 means no limit). Only works in multiplayer.");
	m_pItemLimitSP = REGISTER_INT("i_lying_item_limit_sp", 64, 0, "Max number of items lying around in a level (<= 0 means no limit). Only works in singleplayer.");
}

//------------------------------------------------------------------------
void CItemSystem::UnregisterCVars()
{
	gEnv->pConsole->RemoveCommand("i_giveitem");
	gEnv->pConsole->RemoveCommand("i_dropitem");
	gEnv->pConsole->RemoveCommand("i_giveallitems");
	gEnv->pConsole->RemoveCommand("i_givedebugitems");
	gEnv->pConsole->RemoveCommand("i_listitems");
	gEnv->pConsole->RemoveCommand("i_giveammo");
	gEnv->pConsole->UnregisterVariable("i_precache", true);
	gEnv->pConsole->UnregisterVariable("i_inventory_capacity", true);
}

//------------------------------------------------------------------------
void CItemSystem::GiveItemCmd(IConsoleCmdArgs* args)
{
	if (args->GetArgCount() < 2)
		return;

	IGameFramework* pGameFramework = gEnv->pGameFramework;
	IActorSystem* pActorSystem = pGameFramework->GetIActorSystem();
	IItemSystem* pItemSystem = pGameFramework->GetIItemSystem();

	tukk itemName = args->GetArg(1);
	tukk actorName = 0;

	if (args->GetArgCount() > 2)
		actorName = args->GetArg(2);

	IActor* pActor = 0;

	if (actorName)
	{
		IEntity* pEntity = gEnv->pEntitySystem->FindEntityByName(actorName);
		if (pEntity)
		{
			pActor = pGameFramework->GetIActorSystem()->GetActor(pEntity->GetId());
		}
	}
	else
		pActor = pGameFramework->GetClientActor();

	if (!pActor)
		return;

	//Check item name before giving (it will resolve case sensitive 'issues')
	itemName = (tukk)(pItemSystem->Query(eISQ_Find_Item_By_Name, itemName));

	if (itemName != NULL)
		pItemSystem->GiveItem(pActor, itemName, true, true, true);
	else
		GameWarning("Trying to spawn item of class '%s' which is unknown!", args->GetArg(1));
}

//------------------------------------------------------------------------
void CItemSystem::DropItemCmd(IConsoleCmdArgs* args)
{
	IGameFramework* pGameFramework = gEnv->pGameFramework;
	IActorSystem* pActorSystem = pGameFramework->GetIActorSystem();
	IItemSystem* pItemSystem = pGameFramework->GetIItemSystem();

	tukk actorName = 0;

	if (args->GetArgCount() > 1)
		actorName = args->GetArg(1);

	IActor* pActor = 0;

	if (actorName)
	{
		IEntity* pEntity = gEnv->pEntitySystem->FindEntityByName(actorName);
		if (pEntity)
		{
			pActor = pGameFramework->GetIActorSystem()->GetActor(pEntity->GetId());
		}
	}
	else
		pActor = pGameFramework->GetClientActor();

	if (!pActor)
		return;

	IItem* pCurrentItem = pActor->GetCurrentItem();
	if (!pCurrentItem)
		return;

	pCurrentItem->Drop();
}

//------------------------------------------------------------------------
void CItemSystem::ListItemNames(IConsoleCmdArgs* args)
{
	IGameFramework* pGameFramework = gEnv->pGameFramework;
	IItemSystem* pItemSystem = pGameFramework->GetIItemSystem();

	tukk itemName = NULL;

	if (args->GetArgCount() > 0)
		itemName = args->GetArg(1);
	;

	pItemSystem->Query(eISQ_Dump_Item_Names, itemName);

}

//--------------------------------------------------------------------
void CItemSystem::GiveItemsHelper(IConsoleCmdArgs* args, bool useGiveable, bool useDebug)
{
	if (args->GetArgCount() < 1)
		return;

	IGameFramework* pGameFramework = gEnv->pGameFramework;
	IActorSystem* pActorSystem = pGameFramework->GetIActorSystem();
	CItemSystem* pItemSystem = static_cast<CItemSystem*>(pGameFramework->GetIItemSystem());

	tukk actorName = 0;

	if (args->GetArgCount() > 1)
		actorName = args->GetArg(1);

	IActor* pActor = 0;

	if (actorName)
	{
		IEntity* pEntity = gEnv->pEntitySystem->FindEntityByName(actorName);
		if (pEntity)
		{
			pActor = pGameFramework->GetIActorSystem()->GetActor(pEntity->GetId());
		}
	}
	else
		pActor = pGameFramework->GetClientActor();

	if (!pActor)
		return;

	for (TItemParamsMap::iterator it = pItemSystem->m_params.begin(); it != pItemSystem->m_params.end(); ++it)
	{
		CItemParamsNode* params = it->second.params;
		if (!params)
			continue;

		const IItemParamsNode* itemParams = params->GetChild("params");

		bool give = false;
		bool debug = false;
		if (itemParams)
		{
			i32 n = itemParams->GetChildCount();
			for (i32 i = 0; i < n; i++)
			{
				const IItemParamsNode* param = itemParams->GetChild(i);
				tukk name = param->GetAttribute("name");
				if (!stricmp(name ? name : "", "giveable") && useGiveable)
				{
					i32 val = 0;
					param->GetAttribute("value", val);
					give = val != 0;
				}
				if (!stricmp(name ? name : "", "debug") && useDebug)
				{
					i32 val = 0;
					param->GetAttribute("value", val);
					debug = val != 0;
				}
			}
		}

		if (give || debug)
		{
			pItemSystem->GiveItem(pActor, it->first.c_str(), false);
		}
	}
}

//------------------------------------------------------------------------
void CItemSystem::GiveAllItemsCmd(IConsoleCmdArgs* args)
{
	GiveItemsHelper(args, true, false);
}

//------------------------------------------------------------------------
void CItemSystem::GiveDebugItemsCmd(IConsoleCmdArgs* args)
{
	GiveItemsHelper(args, false, true);
}

//------------------------------------------------------------------------
void CItemSystem::SaveWeaponPositionCmd(IConsoleCmdArgs* args)
{
	IGameFramework* pGameFramework = gEnv->pGameFramework;
	if (IActor* pActor = pGameFramework->GetClientActor())
	{
		if (IItem* pItem = pActor->GetCurrentItem())
			if (IWeapon* pWeapon = pItem->GetIWeapon())
				pWeapon->SaveWeaponPosition();
	}
}

//------------------------------------------------------------------------
void CItemSystem::GiveAmmoCmd(IConsoleCmdArgs* args)
{
	if (args->GetArgCount() < 3)
		return;

	tukk ammo = args->GetArg(1);
	i32 amount = atoi(args->GetArg(2));

	IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(ammo);
	if (!pClass)
		return;

	IActor* pActor = gEnv->pGameFramework->GetClientActor();
	if (!pActor)
		return;

	if (IInventory* pInventory = pActor->GetInventory())
		pInventory->SetAmmoCount(pClass, amount);
}

//------------------------------------------------------------------------
void CItemSystem::RegisterListener(IItemSystemListener* pListener)
{
	m_listeners.Add(pListener);
}

//------------------------------------------------------------------------
void CItemSystem::UnregisterListener(IItemSystemListener* pListener)
{
	m_listeners.Remove(pListener);
}

void CItemSystem::Serialize(TSerialize ser)
{
	if (ser.GetSerializationTarget() != eST_Network)
	{
		if (ser.IsReading())
			m_playerLevelToLevelSave = 0;

		bool hasInventoryNode = m_playerLevelToLevelSave ? true : false;
		ser.Value("hasInventoryNode", hasInventoryNode);

		if (hasInventoryNode)
		{
			if (ser.IsWriting())
			{
				IXmlStringData* xmlData = m_playerLevelToLevelSave->getXMLData(50000);
				string data(xmlData->GetString());
				ser.Value("LTLInventoryData", data);
				xmlData->Release();

#ifdef USE_LTL_PRECACHING
				ser.BeginGroup("LTLPrecacheClasses");
				if (m_LTLPrecacheState == LTLCS_FORSAVEGAME)
				{
					u32 numClasses = m_precacheLevelToLevelItemList.size();
					ser.Value("numClasses", numClasses);
					for (u32 i = 0; i < numClasses; ++i)
					{
						ser.BeginGroup("class");
						string name = m_precacheLevelToLevelItemList[i]->GetName();
						ser.Value("name", name);
						ser.EndGroup();
					}
				}
				ser.EndGroup();
#endif

			}
			else
			{
				string data;
				ser.Value("LTLInventoryData", data);
				m_playerLevelToLevelSave = gEnv->pSystem->LoadXmlFromBuffer(data.c_str(), data.length());

#ifdef USE_LTL_PRECACHING
				m_precacheLevelToLevelItemList.clear();
				ser.BeginGroup("LTLPrecacheClasses");
				u32 numClasses = 0;
				ser.Value("numClasses", numClasses);
				for (u32 i = 0; i < numClasses; ++i)
				{
					ser.BeginGroup("class");
					string name;
					ser.Value("name", name);
					const IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(name.c_str());
					if (pClass)
						m_precacheLevelToLevelItemList.push_back(pClass);
					ser.EndGroup();
				}
				ser.EndGroup();
				PreCacheLevelToLevelLoadout();
				m_LTLPrecacheState = LTLCS_FORSAVEGAME;
#endif
			}
		}
	}
}

void CItemSystem::SerializePlayerLTLInfo(bool bReading)
{
	if (bReading && !m_playerLevelToLevelSave)
		return;

	if (gEnv->bMultiplayer)
		return;

	if (!bReading)
	{
		m_playerLevelToLevelSave = gEnv->pSystem->CreateXmlNode("Inventory");
	}

	IXmlSerializer* pSerializer = gEnv->pSystem->GetXmlUtils()->CreateXmlSerializer();
	IActor* pActor = gEnv->pGameFramework->GetClientActor();
	ISerialize* pSer = NULL;
	if (!bReading)
		pSer = pSerializer->GetWriter(m_playerLevelToLevelSave);
	else
	{
		pSer = pSerializer->GetReader(m_playerLevelToLevelSave);
		m_pEquipmentUpr->OnBeginGiveEquipmentPack();
	}
	TSerialize ser = TSerialize(pSer);
	if (pActor)
	{
		pActor->SerializeLevelToLevel(ser);
	}
	pSerializer->Release();

	if (bReading)
		m_pEquipmentUpr->OnEndGiveEquipmentPack();

#ifdef USE_LTL_PRECACHING
	if (!bReading)
	{
		m_precacheLevelToLevelItemList.clear();
		m_LTLPrecacheState = LTLCS_FORNEXTLEVELLOAD;

		IInventory* pInventory = pActor ? pActor->GetInventory() : NULL;
		if (pInventory)
		{
			//Items
			i32k numberOfItems = pInventory->GetCount();
			for (i32 i = 0; i < numberOfItems; ++i)
			{
				const EntityId itemId = pInventory->GetItem(i);
				IEntity* pEntity = gEnv->pEntitySystem->GetEntity(itemId);

				if (pEntity)
				{
					m_precacheLevelToLevelItemList.push_back(pEntity->GetClass());
				}
			}

			//Accessories
			i32k numberOfAccessories = pInventory->GetAccessoryCount();
			for (i32 i = 0; i < numberOfAccessories; ++i)
			{
				const IEntityClass* pAccessoryClass = pInventory->GetAccessoryClass(i);
				if (pAccessoryClass)
				{
					m_precacheLevelToLevelItemList.push_back(pAccessoryClass);
				}
			}
		}
	}
#endif
}

//------------------------------------------------------------------------
i32 CItemSystem::GetItemParamsCount() const
{
	return m_params.size();
}

//------------------------------------------------------------------------
tukk CItemSystem::GetItemParamName(i32 index) const
{
	// FIXME: maybe return an iterator class, so get rid of advance (it's a map, argh)
	assert(index >= 0 && index < m_params.size());
	TItemParamsMap::const_iterator iter = m_params.begin();
	std::advance(iter, index);
	return iter->first.c_str();
}

//------------------------------------------------------------------------
tukk CItemSystem::GetItemParamsDescriptionFile(tukk itemName) const
{
	TItemParamsMap::const_iterator it = m_params.find(CONST_TEMP_STRING(itemName));
	if (it != m_params.end())
	{
		return it->second.filePath.c_str();
	}

	GameWarning("Trying to get xml description file for item '%s'! Something very wrong has happened!", itemName);
	return NULL;
}

//----------------------------------------------------------------------
uk CItemSystem::Query(IItemSystemQuery query, ukk param /* = NULL */)
{
	static DrxFixedStringT<64> lastItemName = "";

	switch (query)
	{
	case eISQ_Dump_Item_Names:
		{
			tukk filterName = param ? (tukk)(param) : NULL;
			DumpItemList(filterName);
		}
		break;

	case eISQ_Find_Item_By_Name:
		{
			DrxFixedStringT<32> filterName = param ? (tukk)(param) : "";
			if (filterName.empty())
				return NULL;

			TItemParamsMap::const_iterator cit = m_params.find(CONST_TEMP_STRING(filterName.c_str()));

			//If not found, make lower case and make a non-case sensitive search
			if (cit == m_params.end())
			{
				filterName.MakeLower();
				cit = m_params.begin();
				while (cit != m_params.end())
				{
					DrxFixedStringT<32> itemName = cit->first.c_str();
					itemName.MakeLower();
					if (itemName == filterName)
					{
						break;
					}
					++cit;
				}
			}

			//We don't find any match
			if (cit == m_params.end())
				return NULL;

			//Found a match, return its name
			lastItemName = cit->first.c_str();
			return (uk )(lastItemName.c_str());
		}
		break;

	default:
		break;
	}

	return NULL;
}

//--------------------------------------------------------------------
void CItemSystem::DumpItemList(tukk filter)
{
	DrxFixedStringT<32> filterName = "";

	if (filter != NULL)
	{
		filterName = filter;
		filterName.MakeLower();
	}

	TItemParamsMap::const_iterator cit = m_params.begin();
	if (filterName.empty())
	{
		while (cit != m_params.end())
		{
			DrxLog("\t- %s", cit->first.c_str());
			++cit;
		}
	}
	else
	{
		while (cit != m_params.end())
		{
			DrxFixedStringT<32> itemName = cit->first.c_str();
			itemName.MakeLower();
			if (itemName.find(CONST_TEMP_STRING(filterName.c_str())) != string::npos)
			{
				DrxLog("\t- %s", cit->first.c_str());
			}
			++cit;
		}
	}

}

//----------------------------------------------------------------------
typedef struct classMemData
{
	u32 countOfClass;
	u32 memSize;

	classMemData() : countOfClass(0), memSize(0){}
}t_classMemData;

typedef std::map<IEntityClass*, t_classMemData> TDebugClassesMem;

void CItemSystem::DisplayItemSystemStats()
{
	const float kbInvert = 1.0f / 1024.0f;

	TDebugClassesMem debugClassesMap;
	t_classMemData tempData;

	IDrxSizer* pSizer = gEnv->pSystem->CreateSizer();

	GetMemoryUsage(pSizer);

	i32 itemSystemMem = pSizer->GetTotalSize();

	u32 itemCount = 0;
	u32 weaponCount = 0;

	u32 weaponMemSize = 0;
	u32 lastSizerSize = 0;

	TItemMap::const_iterator cit = m_items.begin();
	TItemMap::const_iterator citEnd = m_items.end();
	while (cit != citEnd)
	{
		IItem* pItem = GetItem(cit->first);
		lastSizerSize = pSizer->GetTotalSize();
		pItem->GetMemoryUsage(pSizer);
		u32 itemSize = (pSizer->GetTotalSize() - lastSizerSize);

		if (pItem->GetIWeapon())
		{
			itemCount++;
			weaponCount++;
			weaponMemSize += itemSize;
		}
		else
		{
			itemCount++;
		}

		TDebugClassesMem::iterator it = debugClassesMap.find(pItem->GetEntity()->GetClass());
		if (it != debugClassesMap.end())
		{
			it->second.countOfClass++;
			it->second.memSize += itemSize;
		}
		else
		{
			tempData.countOfClass = 1;
			tempData.memSize = itemSize;
			debugClassesMap.insert(TDebugClassesMem::value_type(pItem->GetEntity()->GetClass(), tempData));
		}

		++cit;
	}
	float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	//Global
	IRenderAuxText::Draw2dLabel(50.0f, 50.0f, 1.5f, white, false, "Item system mem:		%.2f Kb.", itemSystemMem * kbInvert);

	IRenderAuxText::Draw2dLabel(50.0f, 65.0f, 1.5f, white, false, "Num. Item instances:		%d.		Total mem: %.2f Kb", itemCount, (float)(pSizer->GetTotalSize() - itemSystemMem) * kbInvert);
	IRenderAuxText::Draw2dLabel(50.0f, 80.0f, 1.5f, white, false, "Num. Weapon instances:	%d.		Total mem: %.2f Kb", weaponCount, (float)weaponMemSize * kbInvert);

	//Per item class
	TDebugClassesMem::const_iterator cit2 = debugClassesMap.begin();
	TDebugClassesMem::const_iterator citEnd2 = debugClassesMap.end();

	i32 i = 0;
	while (cit2 != citEnd2)
	{
		float grey[4] = { 0.7f, 0.7f, 0.7f, 1.0f };

		float midSize = (float)cit2->second.memSize / (float)cit2->second.countOfClass;
		IRenderAuxText::Draw2dLabel(50.0f, 100.0f + (10.0f * i), 1.2f, grey, false, "Class %s:	Instances: %d.	MemSize: %.2f Kb.	Instance Size:%.3f Kb", cit2->first->GetName(), cit2->second.countOfClass, (float)cit2->second.memSize * kbInvert, midSize * kbInvert);

		++cit2;
		i++;
	}

	pSizer->Release();
}

//-----------------------------------------------------------------------

void CItemSystem::GetMemoryUsage(IDrxSizer* pSizer) const
{
	SIZER_COMPONENT_NAME(pSizer, "ItemSystem");

	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddContainer(m_objectCache);
	pSizer->AddContainer(m_characterCache);
	pSizer->AddContainer(m_classes);
	pSizer->AddContainer(m_params);
	pSizer->AddContainer(m_items);
	m_listeners.GetMemoryUsage(pSizer);
	pSizer->AddContainer(m_folders);
	pSizer->AddObject(m_pEquipmentUpr);
}

u32 CItemSystem::GetItemSocketCount(tukk item) const
{
	if (m_itemParamsFlushed)
		return 0;

	const IItemParamsNode* root = GetItemParams(item);
	if (!root)
		return 0;

	const IItemParamsNode* sockets = root->GetChild("sockets");
	if (!sockets)
		return 0;

	i32 count = 0;
	for (i32 i = 0; i < sockets->GetChildCount(); ++i)
	{
		tukk name = sockets->GetChildName(i);
		if (name != 0 && !stricmp(name, "socket"))
			++count;
	}
	return count;
}

tukk CItemSystem::GetItemSocketName(tukk item, i32 idx) const
{
	if (m_itemParamsFlushed)
		return NULL;

	const IItemParamsNode* root = GetItemParams(item);
	if (!root)
		return NULL;

	const IItemParamsNode* sockets = root->GetChild("sockets");
	if (!sockets)
		return NULL;

	return sockets->GetChild(idx)->GetAttribute("name");
}

bool CItemSystem::IsCompatible(tukk item, tukk attachment) const
{
	if (m_itemParamsFlushed)
		return false;

	const IItemParamsNode* rootItem = GetItemParams(item);
	if (!rootItem)
		return false;

	const IItemParamsNode* sockets = rootItem->GetChild("sockets");
	if (!sockets)
		return false;

	const IItemParamsNode* rootAtt = GetItemParams(attachment);
	if (!rootAtt)
		return false;

	const IItemParamsNode* types = rootAtt->GetChild("types");
	if (!types)
		return false;

	for (i32 i = 0; i < sockets->GetChildCount(); ++i)
	{
		const IItemParamsNode* socket = sockets->GetChild(i);
		for (i32 j = 0; j < socket->GetChildCount(); ++j)
		{
			tukk socketName = socket->GetChild(j)->GetAttribute("type");
			if (socketName)
				for (i32 k = 0; k < types->GetChildCount(); ++k)
				{
					tukk typeName = types->GetChild(k)->GetAttribute("name");
					if (typeName != 0 && !stricmp(socketName, typeName))
						return true;
				}
		}
	}
	return false;
}

bool CItemSystem::GetItemSocketCompatibility(tukk item, tukk socket) const
{
	if (m_itemParamsFlushed)
		return false;

	const IItemParamsNode* itemAtt = GetItemParams(item);
	if (!itemAtt)
		return false;

	const IItemParamsNode* itemTypes = itemAtt->GetChild("types");
	if (!itemTypes)
		return false;

	std::set<string> itemSockets;
	for (i32 i = 0; i < itemTypes->GetChildCount(); ++i)
	{
		tukk socketName = itemTypes->GetChild(i)->GetAttribute("name");
		itemSockets.insert(socketName);
	}

	for (TItemParamsMap::const_iterator it = m_params.begin(), eIt = m_params.end(); it != eIt; ++it)
	{
		if (it->first == item)
			continue;

		if (const IItemParamsNode* itemParams = it->second.params)
		{
			if (const IItemParamsNode* socketsNode = itemParams->GetChild("sockets"))
			{
				for (i32 i = 0; i < socketsNode->GetChildCount(); ++i)
				{
					const IItemParamsNode* childNode = socketsNode->GetChild(i);
					if (tukk childName = childNode->GetAttribute("name"))
						if (CONST_TEMP_STRING(childName) == socket)
							for (i32 j = 0; j < childNode->GetChildCount(); ++j)
								if (const IItemParamsNode* innerNode = childNode->GetChild(j))
									if (tukk supportType = innerNode->GetAttribute("type"))
										if (itemSockets.find(supportType) != itemSockets.end())
											return true;
				}
			}
		}
	}

	return false;
}

bool CItemSystem::CanSocketBeEmpty(tukk item, tukk socket) const
{
	if (m_itemParamsFlushed)
		return true;

	const IItemParamsNode* root = GetItemParams(item);
	if (!root)
		return true;

	const IItemParamsNode* sockets = root->GetChild("sockets");
	if (!sockets)
		return true;

	for (i32 i = 0; i < sockets->GetChildCount(); ++i)
	{
		tukk socketName = sockets->GetChild(i)->GetAttribute("name");
		if (socketName != 0 && !stricmp(socketName, socket))
		{
			i32 canBeEmpty = 1;
			sockets->GetChild(i)->GetAttribute("can_be_empty", canBeEmpty);
			return canBeEmpty != 0;
		}
	}
	return true;
}

void CItemSystem::ItemSystemErrorMessage(tukk fileName, tukk errorInfo, bool displayErrorDialog)
{
	if ((fileName == NULL) || (errorInfo == NULL))
		return;

	DrxFixedStringT<1024> messageBuffer;
	messageBuffer.Format("Failed to load '%s'. Required data missing, which could lead to un-expected game behavior or crashes. (%s)", fileName, errorInfo);

	DrxLogAlways("%s", messageBuffer.c_str());

	if (displayErrorDialog)
	{
		DrxMessageBox(messageBuffer.c_str(), "Error", eMB_Error);
	}
}

void CItemSystem::InsertFolder(tukk folder)
{
	for (TFolderList::const_iterator folderCit = m_folders.begin(); folderCit != m_folders.end(); ++folderCit)
	{
		if (stricmp(folderCit->c_str(), folder) == 0)
			return;
	}

	m_folders.push_back(folder);
}
