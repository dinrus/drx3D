// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   EquipmentSystem.cpp
//  Version:     v1.00
//  Created:     07/07/2006 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Interface for Editor to access DrxAction/Game specific equipments
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/EquipmentSystemInterface.h>

#include <drx3D/CoreX/Game/IGame.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Act/IItemSystem.h>

#include <drx3D/Game/GameXmlParamReader.h>

CEquipmentSystemInterface::CEquipmentSystemInterface(CEditorGame* pEditorGame, IGameToEditorInterface *pGameToEditorInterface)
: m_pEditorGame(pEditorGame)
{
	m_pIItemSystem = gEnv->pGame->GetIGameFramework()->GetIItemSystem();
	m_pIEquipmentUpr = m_pIItemSystem->GetIEquipmentUpr();
	InitItems(pGameToEditorInterface);
}

CEquipmentSystemInterface::~CEquipmentSystemInterface()
{
}

class CEquipmentSystemInterface::CIterator : public IEquipmentSystemInterface::IEquipmentItemIterator
{
public:
	CIterator(const TItemMap& equipmentMap, tukk type)
	{
		m_nRefs = 0;
		m_type  = type;
		if (m_type.empty())
		{
			m_mapIterCur = equipmentMap.begin();
			m_mapIterEnd = equipmentMap.end();
		}
		else
		{
			m_mapIterCur = equipmentMap.find(type);
			m_mapIterEnd = m_mapIterCur;
			if (m_mapIterEnd != equipmentMap.end())
				++m_mapIterEnd;
		}
		if (m_mapIterCur != m_mapIterEnd)
		{
			m_itemIterCur = m_mapIterCur->second.begin();
			m_itemIterEnd = m_mapIterCur->second.end();
		}
	}
	void AddRef()
	{
		++m_nRefs;
	}
	void Release()
	{
		if (--m_nRefs <= 0)
			delete this;
	}
	bool Next(SEquipmentItem& outItem)
	{
		if (m_mapIterCur != m_mapIterEnd)
		{
			if (m_itemIterCur != m_itemIterEnd)
			{
				outItem.name = (*m_itemIterCur).c_str();
				outItem.type = m_mapIterCur->first.c_str();
				++m_itemIterCur;
				if (m_itemIterCur == m_itemIterEnd)
				{
					++m_mapIterCur;
					if (m_mapIterCur != m_mapIterEnd)
					{
						m_itemIterCur = m_mapIterCur->second.begin();
						m_itemIterEnd = m_mapIterCur->second.end();
					}
				}
				return true;
			}
		}
		outItem.name = "";
		outItem.type = "";
		return false;
	}

	i32 m_nRefs;
	string m_type;
	TItemMap::const_iterator m_mapIterCur;
	TItemMap::const_iterator m_mapIterEnd;
	TNameArray::const_iterator m_itemIterCur;
	TNameArray::const_iterator m_itemIterEnd;
};

// return iterator with all available equipment items
IEquipmentSystemInterface::IEquipmentItemIteratorPtr 
CEquipmentSystemInterface::CreateEquipmentItemIterator(tukk type)
{
	return new CIterator(m_itemMap, type);
}

// return iterator with all available accessories for an item
IEquipmentSystemInterface::IEquipmentItemIteratorPtr 
CEquipmentSystemInterface::CreateEquipmentAccessoryIterator(tukk type)
{
	TAccessoryMap::const_iterator itemAccessoryMap = m_accessoryMap.find(type);

	if(itemAccessoryMap != m_accessoryMap.end())
	{
		return new CIterator(itemAccessoryMap->second, "");
	}
	
	return NULL;
}

// delete all equipment packs
void CEquipmentSystemInterface::DeleteAllEquipmentPacks()
{
	m_pIEquipmentUpr->DeleteAllEquipmentPacks();
}

bool CEquipmentSystemInterface::LoadEquipmentPack(const XmlNodeRef& rootNode)
{
	return m_pIEquipmentUpr->LoadEquipmentPack(rootNode);
}

namespace
{
	template <class Container> void ToContainer(tukk* names, i32 nameCount, Container& container)
	{
		while (nameCount > 0)
		{
			container.push_back(*names);
			++names;
			--nameCount;
		}
	}
}

void CEquipmentSystemInterface::InitItems(IGameToEditorInterface* pGTE)
{
	// Get ItemParams from ItemSystem
	// Creates the following entries
	// "item"               All Item classes
	// "item_selectable"    All Item classes which can be selected
	// "item_givable",      All Item classes which can be given
	// "weapon"             All Weapon classes (an Item of class 'Weapon' or an Item which has ammo)
	// "weapon_selectable"  All Weapon classes which can be selected
	// "weapon_givable"     All Weapon classes which can be given
	// and for any weapon which has ammo
	// "ammo_WEAPONNAME"    All Ammos for this weapon

	IItemSystem* pItemSys = m_pIItemSystem;
	i32 maxCountItems = pItemSys->GetItemParamsCount();
	i32 maxAllocItems = maxCountItems+1; // allocate one more to store empty
	tukk* allItemClasses = new tukk [maxAllocItems];
	tukk* givableItemClasses = new tukk [maxAllocItems];
	tukk* selectableItemClasses = new tukk [maxAllocItems];
	tukk* allWeaponClasses = new tukk [maxAllocItems];
	tukk* givableWeaponClasses = new tukk [maxAllocItems];
	tukk* selectableWeaponClasses = new tukk [maxAllocItems];

	i32 numAllItems = 0;
	i32 numAllWeapons = 0;
	i32 numGivableItems = 0;
	i32 numSelectableItems = 0;
	i32 numSelectableWeapons = 0;
	i32 numGivableWeapons = 0;
	std::set<string> allAmmosSet;

	// store default "---" -> "" value
	{
		tukk empty = "";
		selectableWeaponClasses[numSelectableWeapons++] = empty;
		givableWeaponClasses[numGivableWeapons++] = empty;
		allWeaponClasses[numAllWeapons++] = empty;
		selectableItemClasses[numSelectableItems++] = empty;
		givableItemClasses[numGivableItems++] = empty;
		allItemClasses[numAllItems++] = empty;
		allAmmosSet.insert(empty);
	}

	for (i32 i=0; i<maxCountItems; ++i)
	{
		tukk itemName = pItemSys->GetItemParamName(i);
		allItemClasses[numAllItems++] = itemName;

		tukk itemDescriptionFile = pItemSys->GetItemParamsDescriptionFile(itemName);
		
		DRX_ASSERT(itemDescriptionFile);
		XmlNodeRef itemRootParams = gEnv->pSystem->LoadXmlFromFile(itemDescriptionFile);

		if (!itemRootParams)
		{
			GameWarning("Item description file %s doesn't exist for item %s", itemDescriptionFile, itemName);
			continue;
		}

		CGameXmlParamReader rootReader(itemRootParams);
		tukk inheritItem = itemRootParams->getAttr("inherit");
		bool isInherited = (inheritItem && inheritItem[0] != 0);
		if (isInherited)
		{
			tukk baseItemFile = pItemSys->GetItemParamsDescriptionFile(inheritItem);
			itemRootParams = gEnv->pSystem->LoadXmlFromFile(baseItemFile);
		}

		if (itemRootParams)
		{
			bool givable = false;
			bool selectable = false;
			bool uiWeapon = false;
			XmlNodeRef childItemParams = itemRootParams->findChild("params");
			if (childItemParams) 
			{
				CGameXmlParamReader reader(childItemParams);

				//the equipeable flag is supposed to be used for weapons that are not givable
				//but that are still needed to be in the equipment weapon list.
				bool equipeable = false;
				reader.ReadParamValue<bool>("equipeable", equipeable);
				reader.ReadParamValue<bool>("giveable", givable);
				givable |= equipeable;

				reader.ReadParamValue<bool>("selectable", selectable);
				reader.ReadParamValue<bool>("ui_weapon", uiWeapon);
			}

			if (givable)
				givableItemClasses[numGivableItems++] = itemName;
			if (selectable)
				selectableItemClasses[numSelectableItems++] = itemName;

			XmlNodeRef childAccessoriesNode = itemRootParams->findChild("accessories");

			if(childAccessoriesNode)
			{
				i32k numAccessories = childAccessoriesNode->getChildCount();

				for(i32 childIdx = 0; childIdx < numAccessories; childIdx++)
				{
					XmlNodeRef childAccessory = childAccessoriesNode->getChild(childIdx);

					if(childAccessory && stricmp(childAccessory->getTag(), "accessory") == 0)
					{
						tukk accessoryName = childAccessory->getAttr("name");
						tukk categoryName = childAccessory->getAttr("category");			

						if(categoryName && accessoryName)
						{
							m_accessoryMap[itemName][categoryName].push_back(accessoryName);
						}
					}
				}
			}

			XmlNodeRef ammosNode = itemRootParams->findChild("ammos");
			if (ammosNode)
			{
				CGameXmlParamReader reader(ammosNode);

				i32k maxAmmos = reader.GetUnfilteredChildCount();
				i32 numAmmos = 0;
				tukk* ammoNames = new tukk [maxAmmos];
				for (i32 j=0; j<maxAmmos; ++j)
				{
					XmlNodeRef ammoChildNode = reader.GetFilteredChildAt(j);
					if ((ammoChildNode != NULL) && stricmp(ammoChildNode->getTag(), "ammo") == 0)
					{
						tukk ammoName = ammoChildNode->getAttr("name");
						if (ammoName && ammoName[0])
						{
							ammoNames[numAmmos] = ammoName;
							++numAmmos;
							allAmmosSet.insert(ammoName);
						}
					}
				}
				if (numAmmos > 0)
				{
					// make it a weapon when there's ammo
					allWeaponClasses[numAllWeapons++] = itemName;
					if (selectable)
						selectableWeaponClasses[numSelectableWeapons++] = itemName;
					if (givable)
						givableWeaponClasses[numGivableWeapons++] = itemName;

					string ammoEntryName = "ammo_";
					ammoEntryName+=itemName;
					pGTE->SetUIEnums(ammoEntryName.c_str(), ammoNames, numAmmos);
				}
				delete[] ammoNames;
			}
			else
			{
				tukk itemClass = itemRootParams->getAttr("class");
				if (uiWeapon || (itemClass != 0 && stricmp(itemClass, "weapon") == 0))
				{
					// make it a weapon when there's ammo
					allWeaponClasses[numAllWeapons++] = itemName;
					if (selectable)
						selectableWeaponClasses[numSelectableWeapons++] = itemName;
					if (givable)
						givableWeaponClasses[numGivableWeapons++] = itemName;
				}
			}
		}
	}

	i32 numAllAmmos = 0;
	i32k allAmmoCount = allAmmosSet.size();
	tukk* allAmmos = new tukk [allAmmoCount];
	std::set<string>::const_iterator iter (allAmmosSet.begin());
	while (iter != allAmmosSet.end())
	{
		PREFAST_ASSUME(numAllAmmos > 0 && numAllAmmos < allAmmoCount);
		allAmmos[numAllAmmos++] = iter->c_str();
		++iter;
	}
	pGTE->SetUIEnums("ammos", allAmmos, numAllAmmos);
	ToContainer(allAmmos+1, numAllAmmos-1, m_itemMap["Ammo"]);
	delete[] allAmmos;

	pGTE->SetUIEnums("weapon_selectable", selectableWeaponClasses, numSelectableWeapons);
	pGTE->SetUIEnums("weapon_givable", givableWeaponClasses, numGivableWeapons);
	pGTE->SetUIEnums("weapon", allWeaponClasses, numAllWeapons);
	pGTE->SetUIEnums("item_selectable", selectableItemClasses, numSelectableItems);
	pGTE->SetUIEnums("item_givable", givableItemClasses, numGivableItems);
	pGTE->SetUIEnums("item", allItemClasses, numAllItems);

	ToContainer(allItemClasses+1,numAllItems-1,m_itemMap["Item"]);
	ToContainer(givableItemClasses+1,numGivableItems-1,m_itemMap["ItemGivable"]);
	ToContainer(allWeaponClasses+1,numAllWeapons-1,m_itemMap["Weapon"]);

	delete[] selectableWeaponClasses;
	delete[] givableWeaponClasses;
	delete[] allWeaponClasses;
	delete[] selectableItemClasses;
	delete[] givableItemClasses; 
	delete[] allItemClasses;
}
