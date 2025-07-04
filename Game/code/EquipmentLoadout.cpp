// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Equipment Loadouts for C2MP

-------------------------------------------------------------------------
История:
- 18:09:2009  : Created by Ben Johnson

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/EquipmentLoadout.h>
#include <drx3D/Game/Actor.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/Item.h>
#include <drx3D/Act/IItemSystem.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/Game/UI/UIUpr.h>
#include <drx3D/Game/UI/HUD/HUDEventWrapper.h>
#include <drx3D/Game/UI/HUD/HUDUtils.h>
#include <drx3D/Game/PersistantStats.h>
#include <drx3D/CoreX/TypeInfo_impl.h>
#include <drx3D/Game/WeaponSystem.h>
#include <drx3D/Game/Utility/DrxWatch.h>
#include <drx3D/Game/Utility/DesignerWarning.h>
#include <drx3D/Game/LocalPlayerComponent.h>

#include <drx3D/Game/UI/Menu3dModels/MenuRender3DModelMgr.h>
#include <drx3D/Game/UI/Menu3dModels/FrontEndModelCache.h>

#include <drx3D/Game/GameRulesModules/IGameRulesSpawningModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesStateModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesObjectivesModule.h>

#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageTracker.h>

#include <drx3D/Game/GameRulesTypes.h>

#include <drx3D/Game/Network/Lobby/GameAchievements.h>
#include <drx3D/Game/Network/Lobby/GameLobbyData.h>
#include <drx3D/Game/Network/Lobby/GameLobby.h>

//------------------------------------------------------------------------
#define EQUIPMENT_LOADOUT_ITEMS_XML_PATH		"Scripts/GameRules/EquipmentLoadoutItems.xml"
#define EQUIPMENT_LOADOUT_PACKAGES_XML_PATH		"Scripts/GameRules/EquipmentLoadoutPackages.xml"
#define EQUIPMENT_LOADOUT_PROFILE_PATH		"Multiplayer.EquipmentLoadout"

#define INVALID_INDEX -1

#define EQUIPMENT_LOADOUT_WEAPON_SUBCATEGORY_SECONDARY 0

static TKeyValuePair<CEquipmentLoadout::EEquipmentLoadoutCategory,tukk >
s_equipmentLoadoutCategories[] = {  
	{CEquipmentLoadout::eELC_NONE,""},
	{CEquipmentLoadout::eELC_WEAPON,"Weapon"},
	{CEquipmentLoadout::eELC_EXPLOSIVE,"Explosive"},
	{CEquipmentLoadout::eELC_ATTACHMENT,"Attachment"},
};

#define EQUIP_PAIR(a) { CEquipmentLoadout::a, #a },

static TKeyValuePair<CEquipmentLoadout::EEquipmentPackageGroup, tukk > s_equipmentPackageGroups[] =
{
	EQUIPMENT_PACKAGE_TYPES(EQUIP_PAIR)
};

#undef EQUIP_PAIR

AUTOENUM_BUILDNAMEARRAY(s_packageGroupNames, EQUIPMENT_PACKAGE_TYPES);



//------------------------------------------------------------------------
// CEquipmentLoadout::SEquipmentPackage
//------------------------------------------------------------------------

// Takes the display name string and tries to get a string with an added postfix
const bool CEquipmentLoadout::SEquipmentPackage::GetDisplayString(string& outDisplayStr, tukk szPostfix) const
{
	// Don't clear outDisplayStr, a default might be set.

	if (!m_displayName.empty() && m_displayName.at(0) == '@')
	{
		if (ILocalizationUpr* pLocMgr = gEnv->pSystem->GetLocalizationUpr())
		{
			string tempString;
			tempString.Format("%s%s", m_displayName.c_str(), szPostfix);
			tukk tempStringPtr = tempString.c_str();

			SLocalizedInfoGame outInfo;
			if (pLocMgr->GetLocalizedInfoByKey((tempStringPtr + 1), outInfo)) // check if the string exists (minus @)
			{
				outDisplayStr = outInfo.sUtf8TranslatedText;
				return true;
			}
		}
	}

	return false;
}



u8k CEquipmentLoadout::SEquipmentPackage::GetSlotItem(const EEquipmentLoadoutCategory slotCategory, i32k Nth, const bool bDefault/*=false*/) const
{
	CEquipmentLoadout *pEquipmentLoadout = g_pGame->GetEquipmentLoadout();

	// Deal with any overridden special cases first
	const bool bGameActive = g_pGame->IsGameActive();
	if (bGameActive)
	{
		switch(slotCategory)
		{
		case CEquipmentLoadout::eELC_EXPLOSIVE:
			{
				// Explosives disabled
				if (g_pGameCVars->g_allowExplosives == 0 || strlen(g_pGameCVars->g_forceHeavyWeapon->GetString()) > 0)
				{
					return 0;
				}
			}
			break;
		case CEquipmentLoadout::eELC_WEAPON:
			{
				if(strlen(g_pGameCVars->g_forceHeavyWeapon->GetString()) > 0)
				{
					return 0; //No Weapons of any kind
				}

				// PrimaryWeapon overridden
				if (Nth == 0 && g_pGameCVars->g_forceWeapon != -1)
				{
					return g_pGameCVars->g_forceWeapon;
				}
			}
			break;
		}
	}

	// Get Nth slot contents
	i32 slotCount = 0;
	for (i32 i=0; i<EQUIPMENT_LOADOUT_NUM_SLOTS; i++)
	{
		u8 contentsValue = bDefault ? m_defaultContents[i] : m_contents[i];
		CEquipmentLoadout::EEquipmentLoadoutCategory curSlotCategory = pEquipmentLoadout->GetSlotCategory(i);
		if (slotCategory == curSlotCategory)	
		{
			if (slotCount == Nth)
			{
				return contentsValue;
			}

			++slotCount;
		}
	}

	// Not found
	return 0;
}

void CEquipmentLoadout::SEquipmentPackage::GetAttachmentItems(i32 weaponNth, CEquipmentLoadout::TDefaultAttachmentsArr& attachmentsArr) const
{
	attachmentsArr.clear();

	CEquipmentLoadout *pEquipmentLoadout = g_pGame->GetEquipmentLoadout();
	if (weaponNth == 0 && g_pGameCVars->g_forceWeapon != -1)
	{
		// When forcing weapon, set the attachments to be default.
		pEquipmentLoadout->GetAvailableAttachmentsForWeapon(g_pGameCVars->g_forceWeapon, NULL, &attachmentsArr);

		return;
	}

	if(strlen(g_pGameCVars->g_forceHeavyWeapon->GetString()) > 0)
	{
		//No attachments on heavy weapons
		return;
	}

	u8 weaponId = 0; // Store the weapon to get attachments for

	// Get Nth slot contents
	i32 weaponCount = 0;
	i32 attachmentCount = 0;
	for (i32 i=0; i<EQUIPMENT_LOADOUT_NUM_SLOTS; i++)
	{
		u8 contentsValue = m_contents[i];
		CEquipmentLoadout::EEquipmentLoadoutCategory curSlotCategory = pEquipmentLoadout->GetSlotCategory(i);
		if (curSlotCategory == CEquipmentLoadout::eELC_WEAPON)
		{
			if (weaponCount == weaponNth)
			{
				weaponId = contentsValue;
			}

			if (weaponCount>weaponNth)
			{
				// Moved onto next Nth weapon, so return
				return;
			}
			++weaponCount;
		}
		else if (curSlotCategory == CEquipmentLoadout::eELC_ATTACHMENT)
		{
			if (weaponId)
			{
				attachmentsArr.push_back(contentsValue);
			}
		}
	}
}


//------------------------------------------------------------------------
// CEquipmentLoadout
//------------------------------------------------------------------------
CEquipmentLoadout::CEquipmentLoadout()
{
	m_currentlyCustomizingWeapon = false;
	m_currentPackageGroup = SDK;

	m_weaponsStartIndex = 0;
	m_attachmentsStartIndex = 0;

	m_currentCustomizeNameChanged = false;
	m_haveSortedWeapons = false;
	m_hasPreGamePackageSent = false;

	m_unlockPack = 0;
	m_unlockItem = 0;

	m_currentCategory = 0;
	m_currentNth = 0;
	m_curUICategory = eELUIC_None;
	m_curUICustomiseAttachments = false;

	m_lastSelectedWeaponName = "";

	m_bShowCellModel = false;

	memset(&m_currentCustomizePackage,0,sizeof(m_currentCustomizePackage));

	m_allItems.reserve(64);
	m_allWeapons.reserve(18);

	// Item 0 used for invalid/null item.
	SEquipmentItem item("");
	item.m_uniqueIndex = 0;
	m_allItems.push_back(item);
	
	memset(&m_slotCategories,0,sizeof(m_slotCategories));
	m_slotCategories[0] = eELC_WEAPON;
	m_slotCategories[1] = eELC_ATTACHMENT;
	m_slotCategories[2] = eELC_ATTACHMENT;
	m_slotCategories[3] = eELC_ATTACHMENT;
	m_slotCategories[4] = eELC_WEAPON;
	m_slotCategories[5] = eELC_ATTACHMENT;
	m_slotCategories[6] = eELC_ATTACHMENT;
	m_slotCategories[7] = eELC_ATTACHMENT;
	m_slotCategories[8] = eELC_EXPLOSIVE;
	m_slotCategories[9] = eELC_EXPLOSIVE; // Unused
	COMPILE_TIME_ASSERT(10 == EQUIPMENT_LOADOUT_NUM_SLOTS);

	i32 currentWeapon = -1;
	for (i32 i=0; i<EQUIPMENT_LOADOUT_NUM_SLOTS; ++i)
	{
		if (m_slotCategories[i] == eELC_WEAPON)
		{
			++currentWeapon;
			m_attachmentsOverridesWeapon.push_back(TAttachmentsOverridesVector());
		}
		else if (m_slotCategories[i] == eELC_ATTACHMENT)
		{
			m_attachmentsOverridesWeapon[currentWeapon].push_back(SAttachmentOverride());
		}
	}

	LoadItemDefinitions();					// Load item definitions from xml.
	LoadDefaultEquipmentPackages(); // Load default from xml.

	if(IPlayerProfileUpr *pProfileMan = gEnv->pGame->GetIGameFramework()->GetIPlayerProfileUpr())
	{
		pProfileMan->AddListener(this, true);
	}

	for(i32 i=0; i<ePGE_Total; i++)
	{
		m_streamedGeometry[i] = "";
	}

	m_bGoingBack = false;
}

//------------------------------------------------------------------------
const CEquipmentLoadout::TWeaponsVec & CEquipmentLoadout::GetWeaponsList()
{
	if (!m_haveSortedWeapons)
	{	
		TWeaponsVec::iterator itBegin = m_allWeapons.begin();
		TWeaponsVec::iterator itEnd = m_allWeapons.end();

		for (TWeaponsVec::iterator it = itBegin; it != itEnd; ++ it)
		{
			it->m_displayName = CHUDUtils::LocalizeString(it->m_displayName.c_str());
		}

		std::sort(itBegin, itEnd, CompareWeapons);
		m_haveSortedWeapons = true;
	}
	return m_allWeapons;
}

//------------------------------------------------------------------------
bool CEquipmentLoadout::CompareWeapons(const SLoadoutWeapon &elem1, const SLoadoutWeapon &elem2)
{
	i32 result = elem1.m_displayName.compare(elem2.m_displayName);
	return (result < 0);
}

//------------------------------------------------------------------------
CEquipmentLoadout::~CEquipmentLoadout()
{
	if(IPlayerProfileUpr *pProfileMan = gEnv->pGame->GetIGameFramework()->GetIPlayerProfileUpr())
	{
		pProfileMan->RemoveListener(this);
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::Reset()
{
	for (i32 i = 0; i < eEPG_NumGroups; ++ i)
	{
	  m_packageGroups[i].Reset();
	}
  LoadDefaultEquipmentPackages();
}

//------------------------------------------------------------------------
void CEquipmentLoadout::LoadFromProfile(IPlayerProfile* pProfile, bool online, u32 reason)
{
	if (reason & ePR_Game)
	{
		Reset();
	  
		for (i32 i = 0; i < eEPG_NumGroups; ++ i)
		{
			LoadSavedEquipmentPackages((EEquipmentPackageGroup)i, pProfile, online);		// Load saved changes from profile.

			SPackageGroup &group = m_packageGroups[i];

			// Load default choice from profile.
			i32 selectedId = 0;
			DrxFixedStringT<128> pathName;
			pathName.Format("%s.%d.SelectedId", EQUIPMENT_LOADOUT_PROFILE_PATH, i);

			if (pProfile->GetAttribute(pathName.c_str(), selectedId))
			{
				i32 index = GetPackageIndexFromId((EEquipmentPackageGroup)i, selectedId);
				if ((index == INVALID_INDEX) || (index >= (i32)group.m_packages.size()))
				{
					index = 0;
				}

				group.m_profilePackage = index;
				group.m_selectedPackage = index;

				TEquipmentPackageContents &contents = group.m_packages[index].m_contents;
				memcpy( (uk )m_currentCustomizePackage, (uk )contents, sizeof(contents));
			}
		}
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::SelectProfileLoadout()
{
	tukk forcedPackage = g_pGameCVars->g_forceLoadoutPackage->GetString();
	if(strlen(forcedPackage) > 0)
	{
		for(i32 i = 0; i < eEPG_NumGroups; ++i)
		{
			i32 packageCount = m_packageGroups[i].m_packages.size();
			for(i32 j = 0; j < packageCount; ++j)
			{
				if(strcmpi(m_packageGroups[i].m_packages[j].m_name, forcedPackage) == 0)
				{
					m_currentPackageGroup = (EEquipmentPackageGroup)i;
					m_packageGroups[(EEquipmentPackageGroup)i].m_selectedPackage = j;

					return;
				}
			}
		}
	}

	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	group.m_selectedPackage = group.m_profilePackage;
}

//------------------------------------------------------------------------
void CEquipmentLoadout::LoadItemDefinitions()
{
	DrxLog ("Loading item definitions from \"%s\"", EQUIPMENT_LOADOUT_ITEMS_XML_PATH);
	INDENT_LOG_DURING_SCOPE();

	bool hasSetAttachmentStart = false, hasSetWeaponStart = false;

	XmlNodeRef root = gEnv->pSystem->LoadXmlFromFile( EQUIPMENT_LOADOUT_ITEMS_XML_PATH );
	if (root && !stricmp(root->getTag(), "loadouts"))
	{
		i32 numCategories = root->getChildCount();
		for (i32 i = 0; i < numCategories; ++ i)
		{
			XmlNodeRef categoryXml = root->getChild(i);
			tukk categoryName = categoryXml->getTag();
			EEquipmentLoadoutCategory category = KEY_BY_VALUE(categoryName, s_equipmentLoadoutCategories);

			u32 allItemsSize = m_allItems.size();
			if (category==eELC_ATTACHMENT && hasSetAttachmentStart==false)
			{
				m_attachmentsStartIndex = allItemsSize;
				hasSetAttachmentStart = true;
			}
			else if (category==eELC_WEAPON && hasSetWeaponStart==false)
			{
				m_weaponsStartIndex = allItemsSize;
				hasSetWeaponStart = true;
			}

			i32 numItems = categoryXml->getChildCount();
			for (i32 j = 0; j < numItems; ++ j)
			{
				XmlNodeRef itemXml = categoryXml->getChild(j);

				if (!stricmp(itemXml->getTag(), "item"))
				{
					if (g_pGameCVars->g_EPD > 0)
					{
						i32  epd = 0;
						const bool  gotEPD = itemXml->getAttr("EPD", epd);
						if (!gotEPD || (epd != g_pGameCVars->g_EPD))
						{
							continue;
						}
					}
				
					tukk strValue = NULL;
					itemXml->getAttr("name", &strValue);

					SEquipmentItem item(strValue);

					if (itemXml->getAttr("parentName", &strValue))
					{
						// It's a skin item with a parent, copy parent's members.
						LoadItemDefinitionFromParent(strValue, item);
					}

					LoadItemDefinitionParams(itemXml, item);

					item.m_category = category;
					item.m_uniqueIndex = m_allItems.size(); // unique index must match index in array.

					m_allItems.push_back(item);

					if ((category == eELC_WEAPON) && (item.m_parentItemId==0))
					{
						m_allWeapons.push_back(SLoadoutWeapon(item.m_displayName.c_str(), item.m_uniqueIndex, item.m_showSwitchWeapon));
					}
				}
			} // ~for categoryXml
		} // ~for root
	}
	else
	{
		DrxLogAlways("CEquipmentLoadout Failed to get root node for xml %s", EQUIPMENT_LOADOUT_ITEMS_XML_PATH);
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::LoadItemDefinitionFromParent(tukk parentItemName, SEquipmentItem &item)
{
	i32 itemId = GetItemIndexFromName(parentItemName);
	item.m_parentItemId = itemId;
	
	DRX_ASSERT_TRACE((itemId > 0), ("Couldn't find parent '%s' for item '%s', make sure it has been declared above it.", parentItemName, item.m_name.c_str()));

	if (itemId > 0 && itemId < (i32)m_allItems.size())
	{
		SEquipmentItem &parentItem = m_allItems[itemId];

		item.m_displayName = parentItem.m_displayName.c_str();
		item.m_displayTypeName = parentItem.m_displayTypeName.c_str();
		item.m_description = parentItem.m_description.c_str();

		item.m_category = parentItem.m_category;
		item.m_subcategory = parentItem.m_subcategory;
		item.m_allowedPackageGroups = parentItem.m_allowedPackageGroups;
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::LoadItemDefinitionParams(XmlNodeRef itemXML, SEquipmentItem &item)
{
	i32 numParams = itemXML->getChildCount();
	for (i32 i = 0; i < numParams; ++ i)
	{
		XmlNodeRef paramXml = itemXML->getChild(i);

		tukk name = paramXml->getTag();

		i32 value = 0;
		tukk strValue = 0;
		if (!stricmp(name, "HideWhenLocked"))
		{
			if (paramXml->getAttr("value", value))
				item.m_hideWhenLocked = (value != 0);
		}
		if (!stricmp(name, "ShowSwitchWeapon"))
		{
			if (paramXml->getAttr("value", value))
				item.m_showSwitchWeapon = (value != 0);
		}
		else if (!stricmp(name, "UseIcon"))
		{
			if (paramXml->getAttr("value", value))
				item.m_bUseIcon = (value != 0);
		}
		else if (!stricmp(name, "Name"))
		{
			if (paramXml->getAttr("value", &strValue))
				item.m_displayName = strValue;
		}
		else if (!stricmp(name, "TypeName"))
		{
			if (paramXml->getAttr("value", &strValue))
				item.m_displayTypeName = strValue;
		}
		else if (!stricmp(name, "Description"))
		{
			if (paramXml->getAttr("value", &strValue))
				item.m_description = strValue;
		}
		else if (!stricmp(name, "Subcategory"))
		{
			if (paramXml->getAttr("value", value))
				item.m_subcategory = value;
		}
		else if (!stricmp(name, "AllowedPackageGroups"))
		{
			tukk pAllowedGroups;
			if (paramXml->getAttr("value", &pAllowedGroups))
			{
				u32 bitfield = AutoEnum_GetBitfieldFromString(pAllowedGroups, s_packageGroupNames, eEPG_NumGroups);
				item.m_allowedPackageGroups = (u8) bitfield;
			}
		}
	} // ~for
}

//------------------------------------------------------------------------
void CEquipmentLoadout::LoadDefaultEquipmentPackages()
{
	DrxLog ("[LOADOUT] Loading default equipment packages");
	INDENT_LOG_DURING_SCOPE();

	XmlNodeRef root = gEnv->pSystem->LoadXmlFromFile( EQUIPMENT_LOADOUT_PACKAGES_XML_PATH );
	if (root)
	{
		if (!stricmp(root->getTag(), "default"))
		{
			i32 numGroups = root->getChildCount();
			for (i32 groupIndex = 0; groupIndex < numGroups; ++ groupIndex)
			{
				XmlNodeRef groupXml = root->getChild(groupIndex);
				tukk pTag = groupXml->getTag();

				if (!stricmp(pTag, "Group"))
				{
					tukk pString = NULL;
					if (groupXml->getAttr("name", &pString))
					{
						EEquipmentPackageGroup groupId = KEY_BY_VALUE(pString, s_equipmentPackageGroups);
						SPackageGroup &group = m_packageGroups[groupId];
						i32 numLoadouts = groupXml->getChildCount();
						group.m_packages.reserve(numLoadouts);

						for (i32 loadoutIndex = 0; loadoutIndex < numLoadouts; ++ loadoutIndex)
						{
							XmlNodeRef loadoutXml = groupXml->getChild(loadoutIndex);
							pString = loadoutXml->getTag();

							if (!stricmp(pString, "loadout"))
							{
								tukk strName = loadoutXml->getAttr("name");
								tukk displayName = loadoutXml->getAttr("displayName"); // displayName string is also used for small name (by adding _small)
								tukk displayDescription = loadoutXml->getAttr("displayDescription");

								SEquipmentPackage package(strName, displayName, displayDescription);
								loadoutXml->getAttr("id", package.m_id);

								i32 attrIcon = 0;
								if (loadoutXml->getAttr("useIcon",attrIcon))
								{
									package.m_bUseIcon = (attrIcon!=0);
								}

								tukk pModelName;
								if (loadoutXml->getAttr("modelOverride", &pModelName))
								{
									package.m_modelIndex = AddModel(pModelName);
								}

								i32 slotCounts[eELC_SIZE];
								memset(&slotCounts,0,sizeof(slotCounts));

								i32 numItems = loadoutXml->getChildCount();
								for (i32 itemIndex = 0; itemIndex < numItems; ++ itemIndex)
								{
									XmlNodeRef itemXml = loadoutXml->getChild(itemIndex);
									tukk slotTag = itemXml->getTag();

									if (!stricmp(slotTag,"option"))
									{
										itemXml->getAttr("name", &strName);

										if (!stricmp(strName,"Customizable"))
										{
											package.m_flags |= ELOF_CAN_BE_CUSTOMIZED;
										}
										else if (!stricmp(strName,"HideWhenLocked"))
										{
											package.m_flags |= ELOF_HIDE_WHEN_LOCKED;
										}
									}
									else
									{
										itemXml->getAttr("name", &strName);

										EEquipmentLoadoutCategory category = KEY_BY_VALUE(slotTag, s_equipmentLoadoutCategories);

										i32 slotIndex = GetSlotIndex(category, slotCounts[category]);

										if (category == eELC_WEAPON)
										{
											slotCounts[eELC_ATTACHMENT] += CountOfCategoryUptoIndex(eELC_ATTACHMENT, slotCounts[eELC_ATTACHMENT], slotIndex);	// Add empty attachment slots
										}

										if (slotIndex >= 0)
										{
											i32 index = GetItemIndexFromName(strName);
											package.m_contents[slotIndex] = index;
											package.m_defaultContents[slotIndex] = index;
											++slotCounts[category];
										}
									}
								}
								
								group.m_packages.push_back(package);
							}
						}

#ifndef _RELEASE
						// Verify package ids
						i32 numPackages = group.m_packages.size();
						for (i32 i = 0; i < numPackages; ++ i)
						{
							i32 id = group.m_packages[i].m_id;
							for (i32 j = i + 1; j < numPackages; ++ j)
							{
								if (group.m_packages[j].m_id == id)
								{
									DesignerWarning(false, "Multiple packages with the same id (%d) in group %s", id, VALUE_BY_KEY(groupId, s_equipmentPackageGroups));
								}
							}
						}
#endif
					}
				}
				else if(!stricmp(pTag, "Classic"))
				{
					//<Classic rebel="Objects/characters/human/rebels/rebel_%s.cdf">
					//	<Map weapon="SCAR" rebel="01"/>
					//	<Map weapon="DSG1" rebel="03"/>
					//	<Map weapon="Typhoon" rebel="02"/>
					//</Classic>

					tukk pRebelBase = groupXml->getAttr("rebel");
					if(pRebelBase && !pRebelBase[0])
						pRebelBase = NULL;

					tukk pCellBase = groupXml->getAttr("cell");
					if(pCellBase && !pCellBase[0])
						pCellBase = NULL;

					// Load the Classic Mode Weapon/Model mappings.
					i32k numMappings = groupXml->getChildCount();
					for (i32 mappingIndex = 0; mappingIndex<numMappings; ++mappingIndex)
					{
						XmlNodeRef mappingXml = groupXml->getChild(mappingIndex);
						if (!stricmp(mappingXml->getTag(), "Map"))
						{
							tukk  pName = NULL;
							if(mappingXml->getAttr("weapon", &pName))
							{
								if(i32k mapKey = GetItemIndexFromName(pName))
								{
									SClassicModel& map = m_classicMap[mapKey];
									tukk pVariant = NULL;
									DrxFixedStringT<64> modelName;
									if(pRebelBase && mappingXml->getAttr("rebel", &pVariant))
									{
										modelName.Format(pRebelBase, pVariant);
										map.m_modelIndex[SClassicModel::eMT_Rebel] = AddModel(modelName.c_str());
									}
									if(pCellBase && mappingXml->getAttr("cell", &pVariant))
									{
										modelName.Format(pCellBase, pVariant);
										map.m_modelIndex[SClassicModel::eMT_Cell] = AddModel(modelName.c_str());
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		DrxLogAlways("CEquipmentLoadout Failed to get root node for xml %s", EQUIPMENT_LOADOUT_PACKAGES_XML_PATH);
	}
}

//------------------------------------------------------------------------
// 
i32 CEquipmentLoadout::CountOfCategoryUptoIndex(EEquipmentLoadoutCategory category, i32 Nth, i32 upToIndex)
{
	if (upToIndex<0)
		return 0;

	i32 result = 0;

	i32 currNth = 0;
	for (i32 i=0; i<upToIndex; i++)
	{
		if (m_slotCategories[i] == category)
		{
			if (currNth >= Nth)
			{
				result++;
			}

			currNth++;
		}
	}

	return result;
}

//------------------------------------------------------------------------
// Gets the slot index for the Nth in that category (Nth 0 is first)
i32 CEquipmentLoadout::GetSlotIndex(EEquipmentLoadoutCategory category, i32 Nth)
{
	i32 result = INVALID_INDEX;

	i32 currNth = 0;
	for (i32 i=0; i<EQUIPMENT_LOADOUT_NUM_SLOTS; i++)
	{
		if (m_slotCategories[i] == category)
		{
			if (currNth == Nth)
			{
				result = i;
				break;
			}
			else
			{
				currNth++;
			}
		}
	}

	return result;
}

//------------------------------------------------------------------------
// Load saved loadouts from profile.
void CEquipmentLoadout::LoadSavedEquipmentPackages(EEquipmentPackageGroup group, IPlayerProfile* pProfile, bool online)
{
	SPackageGroup &packageGroup = m_packageGroups[group];

	DrxFixedStringT<256> strPathRoot;
	strPathRoot.Format("%s.%d", EQUIPMENT_LOADOUT_PROFILE_PATH, group);

	DrxFixedStringT<256> strPathName;
	strPathName.Format("%s.NumPacks", strPathRoot.c_str());
	i32 numEntries = 0;
	if(pProfile->GetAttribute(strPathName,numEntries))
	{
		i32 loadoutId = 0;
		for(i32 i(0); i<numEntries; ++i)
		{
			strPathName.Format("%s.%d.Id", strPathRoot.c_str(), i);
			if (pProfile->GetAttribute(strPathName, loadoutId))
			{
				i32 loadoutIndex = GetPackageIndexFromId(group, loadoutId);
				if ((loadoutIndex != INVALID_INDEX) && (loadoutIndex < (i32)packageGroup.m_packages.size()))
				{
					SEquipmentPackage &package = packageGroup.m_packages[loadoutIndex];
					string displayName = "";
					strPathName.Format("%s.%d.DisplayName", strPathRoot.c_str(), i);
					
					if (pProfile->GetAttribute(strPathName, displayName))
					{
						package.m_displayName = displayName.c_str();
					}

					strPathName.Format("%s.%d.Num", strPathRoot.c_str(), i);
					i32 numItemEntries = 0;
					if(pProfile->GetAttribute(strPathName,numItemEntries))
					{
						i32 itemIndex = INVALID_INDEX;
						i32 slotIndex = INVALID_INDEX;
						for(i32 j(0); j<numItemEntries; ++j)
						{
							strPathName.Format("%s.%d.%d.SlotIndex", strPathRoot.c_str(), i, j);
							if(pProfile->GetAttribute(strPathName, slotIndex))
							{
								if (slotIndex >= 0 && slotIndex < EQUIPMENT_LOADOUT_NUM_SLOTS)
								{
									strPathName.Format("%s.%d.%d.Index", strPathRoot.c_str(), i, j);
									if(pProfile->GetAttribute(strPathName, itemIndex))
									{
										if(itemIndex > INVALID_INDEX && itemIndex < (i32)m_allItems.size())
										{
											package.m_contents[slotIndex] = itemIndex;
										}
									}
								}
							}
						} // ~for
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------
// Save any changes in loadout from the default packages to profile.
void CEquipmentLoadout::SaveToProfile(IPlayerProfile* pProfile, bool online, u32 reason)
{
	if(reason & ePR_Game)
	{
		for (i32 i = 0; i < eEPG_NumGroups; ++ i)
		{
			SPackageGroup &group = m_packageGroups[i];

			i32 count = 0;
			i32k numEntries = group.m_packages.size();
			for (i32 j = 0; j < numEntries; ++ j)
			{
				if (SaveEquipmentPackage(pProfile, (EEquipmentPackageGroup)i, j, count))
				{
					count++;
				}
			}
			DrxFixedStringT<128> strPath;
			strPath.Format("%s.%d.NumPacks", EQUIPMENT_LOADOUT_PROFILE_PATH, i);

			pProfile->SetAttribute(strPath.c_str(), count);

			if (group.m_packages.size() > 0)
			{
				strPath.Format("%s.%d.SelectedId", EQUIPMENT_LOADOUT_PROFILE_PATH, i);
				pProfile->SetAttribute(strPath.c_str(), group.m_packages[group.m_selectedPackage].m_id);
				group.m_profilePackage = group.m_selectedPackage;
			}
		}
  }
}

//------------------------------------------------------------------------
bool CEquipmentLoadout::SaveEquipmentPackage(IPlayerProfile* pProfile, EEquipmentPackageGroup group, i32 packIndex, i32 count)
{
	bool done = false;

	SEquipmentPackage &package = m_packageGroups[group].m_packages[packIndex];
	if (package.m_flags & ELOF_CAN_BE_CUSTOMIZED)
	{
		DrxFixedStringT<128> strPathRoot;
		strPathRoot.Format("%s.%d", EQUIPMENT_LOADOUT_PROFILE_PATH, group);

		DrxFixedStringT<128> strPathName;
		strPathName.Format("%s.%d.Id", strPathRoot.c_str(), count);
		pProfile->SetAttribute(strPathName, package.m_id);
		
		string displayName(package.m_displayName.c_str());
		strPathName.Format("%s.%d.DisplayName", strPathRoot.c_str(), count);
		pProfile->SetAttribute(strPathName, displayName);

		strPathName.Format("%s.%d.Num", strPathRoot.c_str(), count);
		pProfile->SetAttribute(strPathName, EQUIPMENT_LOADOUT_NUM_SLOTS);

		for (i32 i=0; i<EQUIPMENT_LOADOUT_NUM_SLOTS; ++i)
		{
			strPathName.Format("%s.%d.%d.Index", strPathRoot.c_str(), count, i);
			i32 itemIndex = package.m_contents[i];
			pProfile->SetAttribute(strPathName, itemIndex);

			strPathName.Format("%s.%d.%d.SlotIndex", strPathRoot.c_str(), count, i);
			pProfile->SetAttribute(strPathName, i);
		}

		done = true;
	}

	return done;
}

//------------------------------------------------------------------------
// SERVER: Create a client's loadout representation
void CEquipmentLoadout::OnClientConnect(i32 channelId, bool isReset, EntityId playerId)
{
	if (!isReset)
	{
		m_clientLoadouts.insert(TClientEquipmentPackages::value_type(channelId, SClientEquipmentPackage()));
	}
}

//------------------------------------------------------------------------
// SERVER: Remove a client's loadout representation
void CEquipmentLoadout::OnClientDisconnect(i32 channelId, EntityId playerId)
{
	stl::member_find_and_erase(m_clientLoadouts, channelId);
}

//------------------------------------------------------------------------
// SERVER: Set a client's current loadout.
void CEquipmentLoadout::SvSetClientLoadout(i32 channelId, const CGameRules::EquipmentLoadoutParams &params)
{
	SClientEquipmentPackage &package = m_clientLoadouts[channelId];
	package.m_flags |= (ELOF_HAS_CHANGED|ELOF_HAS_BEEN_SET_ON_SERVER);

	memcpy( (uk )package.m_contents, (uk )params.m_contents, sizeof(package.m_contents));
	package.m_weaponAttachmentFlags = params.m_weaponAttachmentFlags; 
	package.m_modelIndex = params.m_modelIndex;
	package.m_loadoutIdx = params.m_loadoutIndex;
}
//------------------------------------------------------------------------
void CEquipmentLoadout::ApplyAttachmentOverrides(u8 * contents) const
{
	i32 currWeapon = -1;
	i32 currAttachmentForWeapon = 0;
	for (i32 i=0; i<EQUIPMENT_LOADOUT_NUM_SLOTS; ++i)
	{
		if (m_slotCategories[i] == eELC_WEAPON)
		{
			++currWeapon;
			currAttachmentForWeapon = 0;
		}
		else if (m_slotCategories[i] == eELC_ATTACHMENT)
		{
			const SAttachmentOverride & attachmentOverride = m_attachmentsOverridesWeapon[currWeapon].at(currAttachmentForWeapon);
			if (attachmentOverride.m_bInUse)
			{
				contents[i] = attachmentOverride.m_id;
			}
			++currAttachmentForWeapon;
		}
	}
}


//------------------------------------------------------------------------
// CLIENT: Send the current loadout to the server
void CEquipmentLoadout::ClSendCurrentEquipmentLoadout(i32 channelId)
{
	CGameRules::EquipmentLoadoutParams params;

	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	i32k numPackage = group.m_packages.size();
	if ((numPackage > 0) && (numPackage > (i32)group.m_selectedPackage))
	{
		SEquipmentPackage& package = group.m_packages[group.m_selectedPackage];
		TEquipmentPackageContents &contents = package.m_contents;

		CGameRules *pGameRules = g_pGame->GetGameRules();

		memcpy( (uk )params.m_contents, (uk )contents, sizeof(contents));
		ApplyAttachmentOverrides(params.m_contents);
		params.m_modelIndex = package.m_modelIndex;
		params.m_loadoutIndex = group.m_selectedPackage;

		if(i32k itemId = package.GetSlotItem(eELC_WEAPON,0))
		{
			TClassicMap::const_iterator it = m_classicMap.find(itemId);
			if(it!=m_classicMap.end())
			{
				const SClassicModel& classicModel = it->second;
				SClassicModel::EModelType modelType = SClassicModel::eMT_Rebel;
				const EntityId localId = g_pGame->GetIGameFramework()->GetClientActorId();
				if(localId)
				{
					modelType = (g_pGame->GetGameRules()->GetTeam(localId)==2)?SClassicModel::eMT_Cell:SClassicModel::eMT_Rebel;
				}
				else if(!g_pGame->IsGameActive())
				{
					modelType = (m_bShowCellModel) ? SClassicModel::eMT_Cell : SClassicModel::eMT_Rebel;
				}
				u8k classicModelIndex = classicModel.m_modelIndex[modelType];
				if(classicModelIndex!=MP_MODEL_INDEX_DEFAULT)
				{
					params.m_modelIndex = classicModelIndex;
				}

				params.m_modelIndex = GetClassicModeModel(itemId, modelType);
			}
		}

		// Build attachment inclusion list here so server knows which attachments this
		// Package includes
		ClSetAttachmentInclusionFlags(contents, params.m_weaponAttachmentFlags); 

		group.m_lastSentPackage = group.m_selectedPackage;

		if (gEnv->bServer)
		{
			SvSetClientLoadout(channelId, params);
		}
		else
		{
			pGameRules->GetGameObject()->InvokeRMI(CGameRules::SvSetEquipmentLoadout(), params, eRMI_ToServer);
		}
	}
}

//------------------------------------------------------------------------
// Client: we need to tell the server which attachments we have got equipped
void CEquipmentLoadout::ClSetAttachmentInclusionFlags(const TEquipmentPackageContents &contents, u32& inclusionFlags)
{
	// Clear
	inclusionFlags = 0; 

	// For all weapons (primary and secondary) weapons in this equipment package
	IItemSystem *pItemSystem = g_pGame->GetIGameFramework()->GetIItemSystem();
	SEquipmentItem *item = NULL;
	for(i32 i = 0; i < EQUIPMENT_LOADOUT_NUM_SLOTS; ++i)
	{
		u8 itemId = contents[i];
		if (itemId >= m_allItems.size())
		{
			DrxLogAlways("CEquipmentLoadout::ClSetAttachmentInclusionFlags Invalid item id");
			continue;
		}

		if (itemId == 0) // nil item
		{
			continue;
		}

		// Grab next
		item = &m_allItems[itemId];

		// If weapon.. check which attachments are included/excluded
		if(item->m_category == eELC_WEAPON)
		{
			// Grab all attachments
			// Give the player all unlocked attachments for this weapon
			TDefaultAttachmentsArr arrayDefaultAttachments;
			TAttachmentsArr				  arrayAttachments;
			GetAvailableAttachmentsForWeapon(itemId, &arrayAttachments, &arrayDefaultAttachments);

			bool playSound    = false; 
			bool setAsDefault = false; 
			size_t size = arrayAttachments.size();
			i32 allItemsSize = static_cast<i32>(m_allItems.size()); 
			for (size_t a=0; a<size; ++a)
			{
				// Init reused data
				SPPHaveUnlockedQuery results; 
				SEquipmentItem* pAttachmentItem = NULL; 

				i32k attachmentItemId = arrayAttachments[a].m_itemId;
				if ((attachmentItemId > 0) && (attachmentItemId < allItemsSize))
				{
					if( !arrayAttachments[a].m_locked )
					{
						// Grab the attachment
						pAttachmentItem = &m_allItems[attachmentItemId];

						// check if we need to set inclusion flag
						if(ClIsAttachmentIncluded(pAttachmentItem, item->m_name, contents))
						{
							// Set attachment flag
							SetAttachmentInclusionFlag(inclusionFlags, attachmentItemId);
						}
					}
				}
			}

			// Now make sure the default attachments are also included
			setAsDefault = true;
			size = arrayDefaultAttachments.size();
			for (size_t a=0; a<size; ++a)
			{
				i32k attachmentItemId = arrayDefaultAttachments[a];
				if ((attachmentItemId > 0) && (attachmentItemId < static_cast<i32>(m_allItems.size())))
				{
					SEquipmentItem *pAttachmentItem = &m_allItems[attachmentItemId];

					if(ClIsAttachmentIncluded(pAttachmentItem, item->m_name, contents))
					{
						// Set attachment flag
						SetAttachmentInclusionFlag(inclusionFlags, attachmentItemId);
					}
				}
			}
		}
	} // end for(i32 i = 0; i < EQUIPMENT_LOADOUT_NUM_SLOTS; ++i)
}

//------------------------------------------------------------------------
// convert item id into a [0,31] flag and set
void CEquipmentLoadout::SetAttachmentInclusionFlag(u32& inclusionFlags, i32k attachmentItemId)
{
	u8 bitNum = static_cast<u8>(attachmentItemId - m_attachmentsStartIndex); 
	DRX_ASSERT_MESSAGE(bitNum < 32, "CEquipmentLoadout::SetAttachmentInclusionFlag < ERROR - attachment Flag >= 32 require more storage bits");
	
	//DrxLog("CEquipmentLoadout::SetAttachmentInclusionFlag() < Inclusion bit num %d set", bitNum);
	inclusionFlags |= (1<<bitNum); 
}

//------------------------------------------------------------------------
// convert item id into a [0,31] flag and check if set
const bool CEquipmentLoadout::IsAttachmentInclusionFlagSet(u32k& inclusionFlags, i32k attachmentItemId) const
{
	u8 bitNum = static_cast<u8>(attachmentItemId - m_attachmentsStartIndex); 
	DRX_ASSERT_MESSAGE(bitNum < 32, "CEquipmentLoadout::IsAttachmentInclusionFlagSet() < ERROR - attachment Flag >= 32 require more storage bits");
	//DrxLog("CEquipmentLoadout::IsAttachmentInclusionFlagSet() < Inclusion bit num %d checked", bitNum);

	return (inclusionFlags & (1<<bitNum)) > 0; 
}

//------------------------------------------------------------------------
// Allow easy checking of any attachment against inclusion flags data etc
const bool CEquipmentLoadout::ClIsAttachmentIncluded(const SEquipmentItem* pAttachmentItem, tukk pWeaponItemName, const TEquipmentPackageContents& contents) const
{
	IItemSystem *pItemSystem = g_pGame->GetIGameFramework()->GetIItemSystem();
	DRX_ASSERT_MESSAGE(pAttachmentItem != NULL, "CEquipmentLoadout::ClIsAttachmentIncluded < ERROR pAttachmentItem is NULL");
	DRX_ASSERT_MESSAGE(pWeaponItemName != NULL, "CEquipmentLoadout::ClIsAttachmentIncluded < ERROR pWeaponItemName is NULL");

	////////////////////////////////////////////////////////////////////////////////
	// a) Loadout - if explicitly included in loadout (e.g default ' 'Assault' loadout gives reflex sight.. even when haven't unlocked )
	////////////////////////////////////////////////////////////////////////////////
	// If item is locked we *may* still be able to give it to the player if it is an attachment that
	// was included in one of the default 'non-custom' load-outs.
	if(AttachmentIncludedInPackageContents(pAttachmentItem, contents))
	{
		// Explicitly included so not restricted. 
		return true; 
	}

	////////////////////////////////////////
	// b) Progression - is this unlocked?
	////////////////////////////////////////
	
	// Until UI sorted we give players all weapons they have unlocked. 

	string itemLockName(pWeaponItemName); // Need to append weapon + attachment name (e.g. Scar.Reflex)
	tukk   pWeaponName  = NULL;
	IEntityClass* pWeaponClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(pWeaponItemName);
	if(pWeaponClass)
	{
		g_pGame->GetWeaponSystem()->GetWeaponAlias().UpdateClass(&pWeaponClass);
		pWeaponName = pWeaponClass->GetName();
		itemLockName.Format("%s.%s", pWeaponName, pAttachmentItem->m_name.c_str());
	}

	SPPHaveUnlockedQuery results; 
	bool bLocked = IsLocked(eUT_Attachment,itemLockName,results);
	if ( bLocked)
	{
		// No.. this *really* is locked. Sorry you cant have it. 
		return false; 
	}

	////////////////////////////////////////
	// b) TODO - Explicit override
	////////////////////////////////////////
	// Design may want the ability for users to explicitly disable/enable attachments in the front end HUD equipment customize menu. TBD

	// Not locked 
	return true; 
}

//------------------------------------------------------------------------
// Checks the package contents for the specified attachment
const bool CEquipmentLoadout::AttachmentIncludedInPackageContents(const SEquipmentItem* pAttachmentItem,  const TEquipmentPackageContents& contents) const
{
	// How many attachments are there?
	i32 startIndex = m_attachmentsStartIndex;
	i32 endIndex   = m_attachmentsStartIndex;
	i32 allItemsSize = static_cast<i32>(m_allItems.size()); 
	for (i32 i = m_attachmentsStartIndex; i < allItemsSize; ++i)
	{
		const SEquipmentItem &item = m_allItems[i];

		if (item.m_category != eELC_ATTACHMENT)
		{
			break;
		}
		else
		{
			// We have one more valid attachment
			++endIndex;
		}
	}
	i32 numAttachments = endIndex - startIndex; 
	// TEST // 

	const SEquipmentItem *item = NULL;
	for (i32 i=0; i<EQUIPMENT_LOADOUT_NUM_SLOTS; i++)
	{
		u8 itemId = contents[i];
		if (itemId >= m_allItems.size())
		{
			continue;
		}

		if (itemId == 0)
		{
			continue;
		}

		item = &m_allItems[itemId];
		if(item->m_category == eELC_ATTACHMENT)
		{
			// Is this equal to the one we are checking?
			if(pAttachmentItem->m_uniqueIndex == itemId)
			{
				// Not Locked, explicitly included in loud-out
				return true; 
			}
		}
	}

	// not included
	return false; 
}

//------------------------------------------------------------------------
// Allow easy checking of any attachment against inclusion flags data etc
const bool CEquipmentLoadout::SvIsAttachmentIncluded(const SEquipmentItem* pAttachmentItem,u32k& inclusionFlags) const
{
	IItemSystem *pItemSystem = g_pGame->GetIGameFramework()->GetIItemSystem();
	DRX_ASSERT_MESSAGE(pAttachmentItem != NULL, "CEquipmentLoadout::SvIsAttachmentIncluded < ERROR pAttatchmentItem is NULL");
	
	// Check appropriate bit
	if(inclusionFlags > 0 && IsAttachmentInclusionFlagSet(inclusionFlags,pAttachmentItem->m_uniqueIndex))
	{
		// On list. Included.
		return true; 
	}

	// Not on inclusion list so we are good to go. 
	return false; 
}


//------------------------------------------------------------------------
// Client: On spawning, assign any local items. 
void CEquipmentLoadout::ClAssignLastSentEquipmentLoadout(CActor *pActor)
{
}

//------------------------------------------------------------------------
// Server: sets up correct weapon attachments for current player based on client equipment package
void CEquipmentLoadout::SvAssignWeaponAttachments( u8k itemId, const SClientEquipmentPackage& package, CActor * pActor, i32 playerId )
{
	DRX_ASSERT_MESSAGE(pActor != NULL, "CEquipmentLoadout::SvAssignWeaponAttachments < ERROR pActor is NULL");
	i32 numItems =  m_allItems.size();
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	IItemSystem *pItemSystem = pGameFramework->GetIItemSystem();

	// Give the player all unlocked attachments for this weapon
	// TEST- Add ALL attachments a player has unlocked to their inventory
	TDefaultAttachmentsArr arrayDefaultAttachments;
	TAttachmentsArr				  arrayAttachments;
	GetListedAttachmentsForWeapon(itemId, &arrayAttachments);

	CActor::AttachmentsParams attachmentParams;

	u16 networkClassId = ~u16(0);
	bool playSound    = false; 
	bool setAsDefault = false; 
	size_t size = arrayAttachments.size();
	for (size_t i=0; i<size; ++i)
	{
		// Init reused data
		SPPHaveUnlockedQuery results; 
		SEquipmentItem* pAttachmentItem = NULL; 

		i32k attachmentItemId = arrayAttachments[i].m_itemId;
		if ((attachmentItemId > 0) && (attachmentItemId < numItems) )
		{
			// Grab the attachment
			pAttachmentItem = &m_allItems[attachmentItemId];

			// Give to player if on inclusion list
			if(SvIsAttachmentIncluded(pAttachmentItem,package.m_weaponAttachmentFlags))
			{
				if(pItemSystem->GiveItem(pActor, pAttachmentItem->m_name, playSound, setAsDefault, true, NULL, EEntityFlags(ENTITY_FLAG_SERVER_ONLY)))
				{
					if(pGameFramework->GetNetworkSafeClassId(networkClassId, pAttachmentItem->m_name))
					{
						attachmentParams.m_attachments.push_back(CActor::AttachmentsParams::SWeaponAttachment(networkClassId, setAsDefault));
					}
#if !defined(_RELEASE)
					else
					{
						DrxLog("could not find network safe class id for attachment %s", pAttachmentItem->m_name.c_str());
					}
#endif
				}
			}
		}
	}

	// Now make sure the default attachments are created and attached when starting 
	setAsDefault = true;
	size = arrayDefaultAttachments.size();
	for (size_t i=0; i<size; ++i)
	{
		i32k attachmentItemId = arrayDefaultAttachments[i];
		if ((attachmentItemId > 0) && (attachmentItemId < numItems))
		{
			SEquipmentItem *pAttachmentItem = &m_allItems[attachmentItemId];
			// Give to player if not on inclusion list
			if(SvIsAttachmentIncluded(pAttachmentItem,package.m_weaponAttachmentFlags))
			{
				if(pItemSystem->GiveItem(pActor, pAttachmentItem->m_name, playSound, setAsDefault, true, NULL, EEntityFlags(ENTITY_FLAG_SERVER_ONLY)))
				{
					if(pGameFramework->GetNetworkSafeClassId(networkClassId, pAttachmentItem->m_name))
					{
						attachmentParams.m_attachments.push_back(CActor::AttachmentsParams::SWeaponAttachment(networkClassId, setAsDefault));
					}
#if !defined(_RELEASE)
					else
					{
						DrxLog("could not find network safe class id for attachment %s", pAttachmentItem->m_name.c_str());
					}
#endif
				}
			}
		}
	}

	attachmentParams.m_loadoutIdx = package.m_loadoutIdx;

	if( pActor->IsClient() )
	{
		SpawnedWithLoadout( package.m_loadoutIdx );
	}

	pActor->GetGameObject()->InvokeRMI(CActor::ClAssignWeaponAttachments(), attachmentParams, eRMI_ToRemoteClients);
	
}

//------------------------------------------------------------------------
// SERVER: On spawning, assign the current client's loadout. Assigns all server controlled items, not local ones (used to be vision modes).
void CEquipmentLoadout::SvAssignClientEquipmentLoadout(i32 channelId, i32 playerId)
{
	TClientEquipmentPackages::iterator it= m_clientLoadouts.find(channelId);
	if (it != m_clientLoadouts.end())
	{
		SClientEquipmentPackage &package = it->second;

		/*
		DrxLog("@@ SERVER SvAssignClientEquipmentLoadout for channel %d", channelId);
		for (i32 i=0; i<EQUIPMENT_LOADOUT_NUM_SLOTS; ++i)
		{
			DrxLog("     %d - %d", i, package.m_contents[i]);
		}
		*/

		IItemSystem *pItemSystem = g_pGame->GetIGameFramework()->GetIItemSystem();
		CActor *pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(playerId));

		DRX_ASSERT_MESSAGE(pActor->IsPlayer(), "Actor appling loadout to is not a player!");
		CPlayer *pPlayer = static_cast<CPlayer*>(pActor);
		if (pPlayer && pItemSystem != NULL)
		{
			SEquipmentItem *item = NULL;
			EntityId lastWeaponId = 0;
			i32 numItems = m_allItems.size();

			CGameRules *pGameRules = g_pGame->GetGameRules();

			bool  useWeaponLoadouts = (!pGameRules || pGameRules->RulesUseWeaponLoadouts());
			
			const bool onlyGetsHeavyWeapon = strlen(g_pGameCVars->g_forceHeavyWeapon->GetString()) > 0;
			if(!onlyGetsHeavyWeapon)
			{
				for (i32 i=0; i<=EQUIPMENT_LOADOUT_NUM_SLOTS-1; i++)
				{
					u8 itemId = package.m_contents[i];
					if (itemId >= numItems)
					{
						DrxLogAlways("CEquipmentLoadout::SvAssignClientEquipmentLoadout Invalid item id");
						continue;
					}

					if (itemId == 0) // nil item
					{
						continue;
					}

					item = &m_allItems[itemId];

					switch(item->m_category)
					{
						// Primary weapon should be selected immediately, others aren't
					case eELC_WEAPON:
						{
							if (useWeaponLoadouts)
							{
								if (g_pGameCVars->g_forceWeapon == -1)
								{
									lastWeaponId = pItemSystem->GiveItem(pActor, item->m_name, false, (lastWeaponId==0), true); // select first only 
									SvAssignWeaponAttachments(itemId, package, pActor, playerId);

								}
								else if (lastWeaponId == 0)
								{
									itemId = g_pGameCVars->g_forceWeapon;
									if ((itemId >= numItems) || (itemId == 0))
									{
										continue;
									}

									item = &m_allItems[itemId];
									lastWeaponId = pItemSystem->GiveItem(pActor, item->m_name, false, (lastWeaponId==0), true); // select first only

									// Because we are using a force weapon, we must override with the default attachments
									TDefaultAttachmentsArr arrayDefaultAttachments;
									GetAvailableAttachmentsForWeapon(itemId, NULL, &arrayDefaultAttachments);

									size_t size = arrayDefaultAttachments.size();
									for (size_t a=0; a<size; ++a)
									{
										i32k attachmentItemId = arrayDefaultAttachments[a];
										if ((attachmentItemId > 0) && (attachmentItemId < numItems))
										{
											SEquipmentItem *pAttachmentIteam = &m_allItems[attachmentItemId];
											ApplyWeaponAttachment(pAttachmentIteam, pItemSystem, lastWeaponId);
										}
									}
								}
							}
						}
						break;

					case eELC_EXPLOSIVE:
						{
							if (useWeaponLoadouts && g_pGameCVars->g_allowExplosives)
							{
								pItemSystem->GiveItem(pActor, item->m_name, false, false, true);
							}
						}
						break;

					case eELC_ATTACHMENT:
						{
							if (useWeaponLoadouts && (g_pGameCVars->g_forceWeapon == -1))
							{
								// In mp some loadouts have attachments that must begin detached
								bool bApply = true;
								IEntityClass* pAttachmentClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(item->m_name.c_str());
								if(pAttachmentClass)
								{
									IItem* pWeaponItem = pItemSystem->GetItem(lastWeaponId);
									if(pWeaponItem)
									{
										CItemSharedParams * pItemSharedParams = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(pWeaponItem->GetEntity()->GetClass()->GetName(), false);
										if(pItemSharedParams)
										{
											i32k numAccessoryParams = pItemSharedParams->accessoryparams.size();
											for(i32 a = 0; a < numAccessoryParams; a++)
											{
												SAccessoryParams& accessoryParams = pItemSharedParams->accessoryparams[a];

												if(pAttachmentClass == accessoryParams.pAccessoryClass && accessoryParams.beginsDetached)
												{
													bApply = false; 
													break; 
												}
											}
										}
									}
								}

								if(bApply)
								{
									ApplyWeaponAttachment(item, pItemSystem, lastWeaponId);
								}
							}
						}
						break;

						// All other items e.g. weapons, attachments (all given the same way)
					default:
						{
							pItemSystem->GiveItem(pActor, item->m_name, false, false, true);
						}
						break;
					}
				} // ~for

				//Need this weapon to allow environmental weapons to be used
				if(!g_pGameCVars->g_mpNoEnvironmentalWeapons)
					pItemSystem->GiveItem(pActor, "PickAndThrowWeapon", false, false, false);
			}

			// Release the cached resources because the player has now taken over.
			ReleaseStreamedFPGeometry( CEquipmentLoadout::ePGE_All );

			//pPlayer->EquipAttachmentsOnCarriedWeapons();
			pPlayer->OnReceivingLoadout();
		}
	}
}


//------------------------------------------------------------------------
void CEquipmentLoadout::ApplyWeaponAttachment(const SEquipmentItem *pItem, const IItemSystem *pItemSystem, EntityId lastWeaponId)
{
	IItem* pWeaponItem = pItemSystem->GetItem(lastWeaponId);

	DRX_ASSERT_MESSAGE(pWeaponItem, "No item found!");

	if (pWeaponItem)
	{
		if (IWeapon *pWeapon = pWeaponItem->GetIWeapon())
		{
			CItem * pCWeaponItem = static_cast<CItem*>(pWeaponItem);

			if (pCWeaponItem)
			{
				pCWeaponItem->AttachAccessory(pItem->m_name.c_str(), true, true, true, true, true);
				CItem* pAccessory = pCWeaponItem->GetAccessory(pItem->m_name.c_str());
				if(!pAccessory)
				{
					GameWarning("Failed to attach accessory %s on item %s", pItem->m_name.c_str(), pCWeaponItem->GetDisplayName());
				}
			}
		}
	}
	else
	{
		GameWarning("Failed to attach accessory %s because last weapon not found (id:%d).", pItem->m_name.c_str(), (i32)lastWeaponId);
	}
}

//------------------------------------------------------------------------
bool CEquipmentLoadout::SvHasClientEquipmentLoadout(i32 channelId)
{
	TClientEquipmentPackages::iterator it= m_clientLoadouts.find(channelId);
	if (it != m_clientLoadouts.end())
	{
		return ((it->second.m_flags & ELOF_HAS_BEEN_SET_ON_SERVER) != 0);
	}

	return false;
}

//------------------------------------------------------------------------
void CEquipmentLoadout::SvRemoveClientEquipmentLoadout(i32 channelId)
{
	TClientEquipmentPackages::iterator it= m_clientLoadouts.find(channelId);
	if (it != m_clientLoadouts.end())
	{
		it->second.m_flags = it->second.m_flags & ~ELOF_HAS_BEEN_SET_ON_SERVER;
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::Display3DItem(u8 itemNumber)
{
	CMenuRender3DModelMgr::Release();
	// Only allow 3D ModelMgr to be created if you're allowed to in the frontend
	if (!CFrontEndModelCache::IsAllowed3dFrontEndAssets())
	{
		return;
	}
	if (m_currentlyCustomizingWeapon)
	{
		i32 slot = GetSlotIndex((EEquipmentLoadoutCategory)m_currentCategory, m_currentNth);
		assert (itemNumber == m_currentCustomizePackage[slot]);
		itemNumber = m_currentCustomizePackage[slot];
	}

	EEquipmentLoadoutCategory category = (itemNumber < m_allItems.size()) ? m_allItems[itemNumber].m_category : eELC_NONE;
	u8 displayStartSlot = 255;
	u8 displayEndSlot = 255;
	const TEquipmentPackageContents& contents = m_currentCustomizePackage;

	switch (category)
	{
		case eELC_WEAPON:
		if (contents[0] == itemNumber || m_allItems[contents[0]].m_parentItemId == itemNumber)
		{
			displayStartSlot = 0;
			displayEndSlot = 3;
		}
		else if (contents[4] == itemNumber || m_allItems[contents[4]].m_parentItemId == itemNumber)
		{
			displayStartSlot = 4;
			displayEndSlot = 7;
		}
		break;

		case eELC_EXPLOSIVE:
		if (contents[8] == itemNumber || m_allItems[contents[8]].m_parentItemId == itemNumber)
		{
			displayStartSlot = displayEndSlot = 8;
		}
		break;
	}

	if (displayStartSlot < EQUIPMENT_LOADOUT_NUM_SLOTS)
	{
		CMenuRender3DModelMgr* pRender3DModelMgr = NULL;

		assert (displayEndSlot < EQUIPMENT_LOADOUT_NUM_SLOTS);
		CMenuRender3DModelMgr::TAddedModelIndex mainModelIndex = CMenuRender3DModelMgr::kAddedModelIndex_Invalid;
		CItemSharedParams * mountAccessoriesOnThis = NULL;

		for (u8 i = displayStartSlot; i <= displayEndSlot; ++ i)
		{
			tukk  itemName = GetItemNameFromIndex(contents[i]);
			if (itemName != NULL && itemName[0])
			{
				CItemSharedParams * pItemSharedParams = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(itemName, false);
				DRX_ASSERT_TRACE(pItemSharedParams, ("Failed to get item params for class called '%s'", itemName));

				if (pItemSharedParams)
				{
					if(const SGeometryDef* pGeomDef = pItemSharedParams->GetGeometryForSlot(eIGS_ThirdPerson))
					{
						if (i == displayStartSlot)
						{
							// TODO: Implement game code side of 3d models in front end
							/*pRender3DModelMgr = new CMenuRender3DModelMgr(menuRender3DModelMgrSettingsIndex);
							mainModelIndex = pRender3DModelMgr->AddModel(pItemSharedParams->geometry[1].modelPath.c_str(), pItemSharedParams->geometry[1].material.c_str(), false);*/
							mountAccessoriesOnThis = pItemSharedParams;
						}
						else if (pRender3DModelMgr)
						{
							i32k numAccessoryParams = mountAccessoriesOnThis->accessoryparams.size();
							i32 a;
							for(a = 0; a < numAccessoryParams; a++)
							{
								SAccessoryParams& accessoryParams = mountAccessoriesOnThis->accessoryparams[a];
								tukk accessoryName = accessoryParams.pAccessoryClass->GetName();
								bool heyThatsMe = (0 == stricmp(accessoryName, itemName));
								if (heyThatsMe)
								{
									tukk  mountPointName = accessoryParams.attach_helper.c_str();
									DrxLog ("Attaching '%s' to weapon - mount point name is '%s'", itemName, mountPointName);
									INDENT_LOG_DURING_SCOPE();
									// TODO: Implement game code side of 3d models in front end
									/*CMenuRender3DModelMgr::TAddedModelIndex attachmentModelIndex = pRender3DModelMgr->AddModel(pGeomDef->modelPath.c_str(), pGeomDef->material.c_str(), false);
								   pRender3DModelMgr->AttachToStaticObject(mainModelIndex, attachmentModelIndex, mountPointName);*/
									break;
								}
							}

							if (a == numAccessoryParams)
							{
								DrxLog ("Failed to attach '%s' to weapon - found no appropriate accessory params", itemName);
							}
						}
					}
				}
			}
		}
	}
	else if (category == eELC_WEAPON || category == eELC_EXPLOSIVE)
	{
		// TODO: Implement game code side of 3d models in front end
		// Display a weapon which we've highlighted but which doesn't appear in the current loadout...
		/*tukk  itemName = GetItemNameFromIndex(itemNumber);
		CMenuRender3DModelMgr * pRender3DModelMgr = new CMenuRender3DModelMgr(menuRender3DModelMgrSettingsIndex);
		if (itemName != NULL && itemName[0])
		{
			CItemSharedParams * pItemSharedParams = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(itemName, false);
			DRX_ASSERT_TRACE(pItemSharedParams, ("Failed to get item params for class called '%s'", itemName));

			if (pItemSharedParams)
			{
			if (pItemSharedParams && pItemSharedParams->geometry.size() >= 2)
			{
				pRender3DModelMgr->AddModel(pItemSharedParams->geometry[1].modelPath.c_str(), pItemSharedParams->geometry[1].material.c_str(), false);
				}
			}
		}*/
	}
}

//------------------------------------------------------------------------
tukk CEquipmentLoadout::GetCurrentPackageGroupName()
{
	return VALUE_BY_KEY(m_currentPackageGroup, s_equipmentPackageGroups);
}

//------------------------------------------------------------------------
void CEquipmentLoadout::DetachInLoadout(const ItemString &oldAttachmentName, i32 weapon)
{
	i32 deletingId = GetItemIndexFromName(oldAttachmentName.c_str());
	if (deletingId == 0)
		return;
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	i32k numPackage = group.m_packages.size();
	if ((numPackage > 0) && (numPackage > (i32)group.m_selectedPackage))
	{
		TEquipmentPackageContents &contents = group.m_packages[group.m_selectedPackage].m_contents;
		i32 startIdx = GetSlotIndex(eELC_WEAPON, weapon)+1;

		if (startIdx>0)
		{
			for (i32 i=0; (m_slotCategories[startIdx+i] == eELC_ATTACHMENT) && ((startIdx+i) < EQUIPMENT_LOADOUT_NUM_SLOTS); ++i)
			{
				i32 id;
				SAttachmentOverride & attachmentOverride = m_attachmentsOverridesWeapon[weapon].at(i);
				if (attachmentOverride.m_bInUse)
				{
					id = attachmentOverride.m_id;
				}
				else
				{
					id = contents[startIdx + i];
				}

				if (deletingId ==  id)
				{
					attachmentOverride.m_bInUse = true;
					attachmentOverride.m_id = 0;

					CGameRules *pGameRules = g_pGame->GetGameRules();
					if (pGameRules)
					{
						pGameRules->SetPendingLoadoutChange();
					}
					group.m_packages[group.m_selectedPackage].m_flags |= ELOF_HAS_CHANGED;
					return;
				}
			}
		}
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::NewAttachmentInLoadout(const ItemString &newAttachmentName, i32 weapon)
{
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	i32k numPackage = group.m_packages.size();
	i32 newId = GetItemIndexFromName(newAttachmentName.c_str());
	if ((numPackage > 0) && (numPackage > (i32)group.m_selectedPackage))
	{
		TEquipmentPackageContents &contents = group.m_packages[group.m_selectedPackage].m_contents;
		i32 startIdx = GetSlotIndex(eELC_WEAPON, weapon)+1;

		if (startIdx>0)
		{
			for (i32 i=0; (m_slotCategories[startIdx+i] == eELC_ATTACHMENT) && ((startIdx+i) < EQUIPMENT_LOADOUT_NUM_SLOTS); ++i)
			{
				SAttachmentOverride & attachmentOverride = m_attachmentsOverridesWeapon[weapon].at(i);
				if (attachmentOverride.m_bInUse)
				{
					if (attachmentOverride.m_id == 0)
					{
						attachmentOverride.m_id = newId;
						CGameRules *pGameRules = g_pGame->GetGameRules();
						if (pGameRules)
						{
							pGameRules->SetPendingLoadoutChange();
						}
						group.m_packages[group.m_selectedPackage].m_flags |= ELOF_HAS_CHANGED;
						return;
					}
				}
				else
				{
					if (contents[startIdx + i] == 0)
					{
						attachmentOverride.m_id = newId;
						attachmentOverride.m_bInUse = true;
						CGameRules *pGameRules = g_pGame->GetGameRules();
						if (pGameRules)
						{
							pGameRules->SetPendingLoadoutChange();
						}
						group.m_packages[group.m_selectedPackage].m_flags |= ELOF_HAS_CHANGED;
						return;
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------
bool CEquipmentLoadout::IsItemLocked(i32 index, bool bNeedUnlockDescriptions, SPPHaveUnlockedQuery* pResults/*=NULL*/) const
{
	bool result = false;
	i32k numItems = m_allItems.size();
	if (index < numItems)
	{
		const SEquipmentItem &item = m_allItems[index];
		SPPHaveUnlockedQuery results;

		string itemLockName(item.m_name.c_str());
		EUnlockType type = eUT_Weapon;
		if(item.m_category == eELC_ATTACHMENT)
		{
			type = eUT_Attachment;

			DRX_ASSERT(m_lastSelectedWeaponName != "");
			tukk weaponName = m_lastSelectedWeaponName.c_str();

			if (IEntityClass* pWeaponClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(weaponName))
			{
				g_pGame->GetWeaponSystem()->GetWeaponAlias().UpdateClass(&pWeaponClass);

				weaponName = pWeaponClass->GetName();
				itemLockName.Format("%s.%s", weaponName, item.m_name.c_str());
			}
		}

		SPPHaveUnlockedQuery* pReturnResults = (pResults ? pResults : &results);

		pReturnResults->getstring = bNeedUnlockDescriptions && (pResults != NULL);

		return (IsLocked(type, itemLockName.c_str(), *pReturnResults));
	}

	return false;
}

//------------------------------------------------------------------------
bool CEquipmentLoadout::IsLocked(EUnlockType type, tukk name, SPPHaveUnlockedQuery &results) const
{
	bool locked = false;
	CPlayerProgression *pPlayerProgression = CPlayerProgression::GetInstance();
	if (pPlayerProgression)
	{
		pPlayerProgression->HaveUnlocked(type, name, results);

		if (results.exists && results.unlocked == false)
		{
			locked = true;
		}
	}

	return locked;
}

//------------------------------------------------------------------------
i32 CEquipmentLoadout::GetCurrentSelectedPackageIndex()
{
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	return group.m_selectedPackage;
}

//------------------------------------------------------------------------
EMenuButtonFlags CEquipmentLoadout::GetNewFlagForItemCategory(EEquipmentLoadoutCategory category, i32 subcategory)
{
	EMenuButtonFlags newFlag = eMBF_None;
	switch (category)
	{
	case eELC_WEAPON:
		{
			if (subcategory==EQUIPMENT_LOADOUT_WEAPON_SUBCATEGORY_SECONDARY) // Secondary weapons
				newFlag = eMBF_SecondaryWeapon;
			else
				newFlag = eMBF_Weapon;
		}
		break;
	case eELC_EXPLOSIVE:
		newFlag = eMBF_ExplosiveWeapon;
		break;
	}

	return newFlag;
}

//------------------------------------------------------------------------
void CEquipmentLoadout::FlagItemAsNew(tukk itemName)
{
	size_t allItemsSize = m_allItems.size();
	for (size_t i=0; i<allItemsSize; ++i)
	{
		SEquipmentItem &item = m_allItems[i];
		if (item.m_name.compare(itemName)==0)
		{
			item.m_bShowNew = true;

			if (CPlayerProgression *pPlayerProgression = CPlayerProgression::GetInstance())
			{
				EMenuButtonFlags newFlag = GetNewFlagForItemCategory(item.m_category, item.m_subcategory);
				if (newFlag != eMBF_None)
				{
					pPlayerProgression->AddUINewDisplayFlags(newFlag);
				}
			}

			DrxLog("Flagged item!");
		}
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::FlagItemAttachmentAsNew(tukk itemName)
{
	size_t allItemsSize = m_allItems.size();
	for (size_t i=0; i<allItemsSize; ++i)
	{
		SEquipmentItem &item = m_allItems[i];
		if (item.m_name.compare(itemName)==0)
		{
			if (item.m_category==eELC_WEAPON)
			{
				item.m_bShowAttachmentsNew = true;

				i32 flags = eMBF_AttachmentToken;
				if (item.m_subcategory==EQUIPMENT_LOADOUT_WEAPON_SUBCATEGORY_SECONDARY) // Secondary weapons
				{
					flags |= eMBF_SecondaryWeaponAttachmentToken;
				}
				else
				{
					flags |= eMBF_WeaponAttachmentToken;
				}

				if (CPlayerProgression *pPlayerProgression = CPlayerProgression::GetInstance())
				{
					pPlayerProgression->AddUINewDisplayFlags(flags);
				}
			}
			else
			{
				DRX_ASSERT_TRACE(0, ("Trying to set attachment weapon new flag on %s which is not of weapon loadout type", item.m_name.c_str()));
			}
		}
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::ClearItemNewFlags(i32k itemId)
{
	if ((itemId >= 0) && ((i32)m_allItems.size() > itemId))
	{
		SEquipmentItem &item = m_allItems[itemId];
		if (item.m_bShowNew)
		{
			item.m_bShowNew = false;
		}
	}
}

//------------------------------------------------------------------------
bool CEquipmentLoadout::CanCustomizePackage(i32 index)
{
	bool result = false;
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	if ((index >= 0) && ((i32)group.m_packages.size() > index))
	{
		SEquipmentPackage &package = group.m_packages[index];
		result = ((package.m_flags&ELOF_CAN_BE_CUSTOMIZED)!=0);
	}

	return result;
}

//------------------------------------------------------------------------
bool CEquipmentLoadout::IsPackageLocked(i32 index)
{
	bool result = false;
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	if ((index >= 0) && ((i32)group.m_packages.size() > index))
	{
		SEquipmentPackage &package = group.m_packages[index];

		SPPHaveUnlockedQuery results;
		EUnlockType type = (package.m_flags&ELOF_CAN_BE_CUSTOMIZED) ? eUT_CreateCustomClass : eUT_Loadout;
		DrxFixedStringT<128> unlockName;
		unlockName.Format("%s_%s", VALUE_BY_KEY(m_currentPackageGroup, s_equipmentPackageGroups), package.m_name.c_str());
		result = IsLocked(type, unlockName.c_str(), results);
	}

	return result;
}

//------------------------------------------------------------------------
bool CEquipmentLoadout::TokenUnlockPackage()
{
	return true;
}

//------------------------------------------------------------------------
EUnlockType CEquipmentLoadout::GetUnlockTypeForItem(i32 index) const
{
	if ((index >= 0) && ((i32)m_allItems.size() > index))
	{
		const SEquipmentItem &item = m_allItems[index];

		switch (item.m_category)
		{
		case eELC_WEAPON:
    case eELC_EXPLOSIVE:
			return eUT_Weapon;

		case eELC_ATTACHMENT:
			return eUT_Attachment;
		};
	}

	return eUT_Invalid;
}

//------------------------------------------------------------------------
bool CEquipmentLoadout::TokenUnlockItem()
{
	return true;
}

//------------------------------------------------------------------------
void CEquipmentLoadout::SetSelectedPackage(i32 index)
{
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	i32k numPackages = group.m_packages.size();
	if (index < numPackages)
	{
		DrxLog("@@ CEquipmentLoadout::SetSelectedPackage new:%d (%s) from old:%d", index, group.m_packages.at(index).m_name.c_str(), group.m_selectedPackage);

		group.m_selectedPackage = index;
		TAttachmentsOverridesWeapons::iterator weaponIter = m_attachmentsOverridesWeapon.begin();
		TAttachmentsOverridesWeapons::iterator weaponEndIter = m_attachmentsOverridesWeapon.end();
		while(weaponIter != weaponEndIter)
		{
			TAttachmentsOverridesVector::iterator attachmentIter = weaponIter->begin();
			TAttachmentsOverridesVector::iterator attachmentEndIter = weaponIter->end();
			while (attachmentIter != attachmentEndIter)
			{
				attachmentIter->Reset();
				++attachmentIter;
			}
			++weaponIter;
		}

		TEquipmentPackageContents &contents = group.m_packages[group.m_selectedPackage].m_contents;
		memcpy( (uk )m_currentCustomizePackage, (uk )contents, sizeof(contents));
		m_currentCustomizeNameChanged = false;
	}
}


//------------------------------------------------------------------------
const bool CEquipmentLoadout::HasCurrentCustomizePackageChanged() const
{
	if (m_currentCustomizeNameChanged)
	{
		return true;
	}

	const SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	i32k numPackages = group.m_packages.size();
	if (group.m_selectedPackage < numPackages)
	{
		const SEquipmentPackage &package = group.m_packages[group.m_selectedPackage];

		for (i32 i=0; i<EQUIPMENT_LOADOUT_NUM_SLOTS; ++i)
		{
			if (package.m_contents[i] != m_currentCustomizePackage[i])
				return true;
		}
	}

	return false;
}


//------------------------------------------------------------------------
bool CEquipmentLoadout::HasLastSavedPackageChanged()
{
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	return (group.m_selectedPackage != group.m_profilePackage);
}

//------------------------------------------------------------------------
void CEquipmentLoadout::SetCurrentCustomizePackage()
{
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	i32k numPackages = group.m_packages.size();
	if (group.m_selectedPackage < numPackages)
	{
		SEquipmentPackage &package = group.m_packages[group.m_selectedPackage];

		for (i32 i=0; i<EQUIPMENT_LOADOUT_NUM_SLOTS; ++i)
		{
			if (package.m_contents[i] != m_currentCustomizePackage[i])
			{
				 package.m_contents[i] = m_currentCustomizePackage[i];
			}
		}
	}
}


//------------------------------------------------------------------------
bool CEquipmentLoadout::SetPackageCustomizeItem(i32 packageId, i32 itemId, i32 category, i32 Nth)
{
	bool bChanged = false;

	if (SEquipmentPackage *pPackage = GetPackageFromIdInternal(packageId))
	{
		i32 slot = GetSlotIndex((EEquipmentLoadoutCategory)category, Nth);
		if (slot != INVALID_INDEX)
		{
			if (pPackage->m_contents[slot] != itemId)
			{
				u8 oldItemId = pPackage->m_contents[slot];
				pPackage->m_contents[slot] = itemId;

				if ((EEquipmentLoadoutCategory)category == eELC_WEAPON)
				{
					if (m_allItems[itemId].m_parentItemId == 0 && m_allItems[oldItemId].m_parentItemId != itemId)
					{
						DrxLog ("Old item and new item are of different base types so setting default attachments");

						TDefaultAttachmentsArr arrayDefaultAttachments;
						GetAvailableAttachmentsForWeapon(itemId, NULL, &arrayDefaultAttachments);
						SetPackageNthWeaponAttachments(packageId, Nth, arrayDefaultAttachments);
					}
				}
				bChanged = true;
			}
		}
	}

	return bChanged;
}

//------------------------------------------------------------------------
bool CEquipmentLoadout::SetPackageNthWeaponAttachments(i32k packageId, u32 Nth, TDefaultAttachmentsArr& arrayAttachments)
{
	bool bChanged = false;

	if (SEquipmentPackage *pPackage = GetPackageFromIdInternal(packageId))
	{
		while (arrayAttachments.size()<MAX_DISPLAY_DEFAULT_ATTACHMENTS)	// Pad to max attachments to null any other slots
		{
			arrayAttachments.push_back(0);
		};

		i32 attachmentNth = 0;
		i32k slot = GetSlotIndex(eELC_WEAPON, Nth) + 1;	// after weapon slot
		i32k numAttachments = arrayAttachments.size();
		for (i32 i=slot; i<EQUIPMENT_LOADOUT_NUM_SLOTS && attachmentNth<numAttachments; ++i)
		{
			i32k itemId = pPackage->m_contents[i];
			if (m_slotCategories[i] == eELC_ATTACHMENT)
			{
				if (itemId != arrayAttachments[attachmentNth])
				{
					pPackage->m_contents[i] = arrayAttachments[attachmentNth];
					bChanged = true;
				}
				++attachmentNth;
			}
			else if (m_slotCategories[i] == eELC_WEAPON)	// Onto next weapon, bail
				break;
		}
	}

	return bChanged;
}

//------------------------------------------------------------------------
void CEquipmentLoadout::SetCurrentCustomizeItem(i32 itemId)
{
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
#ifndef _RELEASE
	DrxLog ("SetCurrentCustomizeItem: m_selectedPackage=%d m_currentNth=%d category=%d", group.m_selectedPackage, m_currentNth, m_currentCategory);
	INDENT_LOG_DURING_SCOPE();
#endif

	if ((group.m_selectedPackage >= 0) && (group.m_selectedPackage < (i32)group.m_packages.size()))
	{
		i32 slot = GetSlotIndex((EEquipmentLoadoutCategory)m_currentCategory, m_currentNth);

#ifndef _RELEASE
		DrxLog ("Changing contents[%d] from %u '%s' to %d '%s'", slot, m_currentCustomizePackage[slot], GetItemNameFromIndex(m_currentCustomizePackage[slot]), itemId, GetItemNameFromIndex(itemId));
		INDENT_LOG_DURING_SCOPE();
#endif

		if (m_currentCustomizePackage[slot] != itemId)
		{
			u8 oldItemId = m_currentCustomizePackage[slot];
			m_currentCustomizePackage[slot] = itemId;

			if ((EEquipmentLoadoutCategory)m_currentCategory == eELC_WEAPON)
			{
				if (m_allItems[itemId].m_parentItemId == 0 && m_allItems[oldItemId].m_parentItemId != itemId)
				{
					DrxLog ("Old item and new item are of different base types so setting default attachments");
					SetDefaultAttachmentsForWeapon(itemId, m_currentNth);
				}
				else
				{
					DrxLog ("Old item and new item are of the same base type so not setting default attachments");
					assert (m_currentlyCustomizingWeapon);
				}
			}
		}
	}
}

//------------------------------------------------------------------------
static tukk  s_playerModels_frontend[][2] =
{
	{
		"Objects/Characters/Human/sdk_player/sdk_player.cdf",
		"Objects/Characters/Human/sdk_player/sdk_player.cdf",
	},
	{
		"Objects/Characters/Human/sdk_player/sdk_player.cdf",
		"Objects/Characters/Human/sdk_player/sdk_player.cdf",
	},
};

static tukk  s_playerModels_ingame[][2] =
{
	{
		"Objects/Characters/Human/sdk_player/sdk_player.cdf",
		"Objects/Characters/Human/sdk_player/sdk_player.cdf",
	},
	{
		"Objects/Characters/Human/sdk_player/sdk_player.cdf",
		"Objects/Characters/Human/sdk_player/sdk_player.cdf",
	},
};

//------------------------------------------------------------------------
tukk CEquipmentLoadout::GetPlayerModel(i32k index)
{
	tukk  usePlayerModelName = NULL;
	if (const SEquipmentPackage* pPackage = GetPackageFromId(index))
	{
		IActor * localActor = g_pGame->GetIGameFramework()->GetClientActor();

		if(pPackage->m_modelIndex != MP_MODEL_INDEX_DEFAULT)
		{
			return GetModelName(pPackage->m_modelIndex);
		}

		i32 modelIndex = 0;
		i32 teamIndex = 0;

		bool bGameActive = true;
		if (localActor)
		{
			CGameRules *pGameRules = g_pGame->GetGameRules();
			if(pGameRules && pGameRules->GetGameMode() == eGM_Gladiator)
			{
				teamIndex = 0;
			}
			else
			{
				i32 teamId = pGameRules ? pGameRules->GetTeam(localActor->GetEntityId()) : 0;
				teamIndex = (teamId == 2) ? 1 : 0;
			}
		}
		else
		{
			bGameActive = g_pGame->IsGameActive();
			if(!bGameActive)
			{
				teamIndex = (m_bShowCellModel)? 1 : 0;
			}
			else
			{
				modelIndex = 0;
				teamIndex = 0;
			}
		}

		if(i32k itemId = pPackage->GetSlotItem(CEquipmentLoadout::eELC_WEAPON, 0))
		{
			TClassicMap::const_iterator it = m_classicMap.find(itemId);
			if(it!=m_classicMap.end())
			{
				const SClassicModel& classicModel = it->second;
				u8k classicModelIndex = classicModel.m_modelIndex[teamIndex];
				if(classicModelIndex!=MP_MODEL_INDEX_DEFAULT)
				{
					return GetModelName(classicModelIndex);
				}
			}
		}
		
		if(bGameActive)
			usePlayerModelName = s_playerModels_ingame[modelIndex][teamIndex];
		else
			usePlayerModelName = s_playerModels_frontend[modelIndex][teamIndex];
	}

	return usePlayerModelName;
}

//------------------------------------------------------------------------
void CEquipmentLoadout::SetHighlightedPackage(i32 packageIndex)
{
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	assert (packageIndex >= 0 && packageIndex < static_cast<i32>(group.m_packages.size()));
	SEquipmentPackage &package = group.m_packages[packageIndex];

	group.m_highlightedPackage = (i32) packageIndex;

	// Only allow 3D ModelMgr to be created if you're allowed to in the frontend
	if (CFrontEndModelCache::IsAllowed3dFrontEndAssets())
	{
		IActor * localActor = g_pGame->GetIGameFramework()->GetClientActor();
		tukk  usePlayerModelName = GetPlayerModel(packageIndex);
		tukk  primaryItemName = GetItemNameFromIndex(package.m_contents[0]);
		tukk  secondaryItemName = GetItemNameFromIndex(package.m_contents[4]);
		tukk  explosiveItemName = GetItemNameFromIndex(package.m_contents[8]);

		if (primaryItemName && primaryItemName[0])
		{
			CItemSharedParams * pItemSharedParams = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(primaryItemName, false);
			DRX_ASSERT_TRACE(pItemSharedParams, ("Failed to get item params for class called '%s'", primaryItemName));

			if (pItemSharedParams)
			{
				tukk  attachPos = pItemSharedParams->params.attachment[IItem::eIH_Right].c_str();
				const SGeometryDef* pGeomDef = pItemSharedParams->GetGeometryForSlot(eIGS_ThirdPerson);

				if (pGeomDef && attachPos[0])
				{
					bool hasUnderBarrelAttachment = false;

					// TODO: Implement game code side of 3d models in front end
					/*CMenuRender3DModelMgr::TAddedModelIndex weaponModelIndex = render3DModelMgr->AddModel(pItemSharedParams->geometry[1].modelPath.c_str(), pItemSharedParams->geometry[1].material.c_str(), false);
					render3DModelMgr->AttachToCharacter(characterModelIndex, weaponModelIndex, attachPos);*/

					for (u8 i = 1; i < 4; ++ i)
					{
						tukk  attachmentName = GetItemNameFromIndex(package.m_contents[i]);
						if (attachmentName && attachmentName[0])
						{
							CItemSharedParams * pSubItemSharedParams = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(attachmentName, false);
							DRX_ASSERT_TRACE(pSubItemSharedParams, ("Failed to get item params for class called '%s'", attachmentName));

							if (pSubItemSharedParams)
							{
								if(const SGeometryDef* pSubGeomDef = pSubItemSharedParams->GetGeometryForSlot(eIGS_ThirdPerson))
								{
									i32k numAccessoryParams = pItemSharedParams->accessoryparams.size();
									for(i32 a = 0; a < numAccessoryParams; a++)
									{
										SAccessoryParams& accessoryParams = pItemSharedParams->accessoryparams[a];
										tukk accessoryName = accessoryParams.pAccessoryClass->GetName();
										bool heyThatsMe = (0 == stricmp(accessoryName, attachmentName));
										if (heyThatsMe)
										{
											tukk  mountPointName = accessoryParams.attach_helper.c_str();
											DrxLog ("Attaching '%s' to weapon - mount point name is '%s'", attachmentName, mountPointName);
											INDENT_LOG_DURING_SCOPE();
											// TODO: Implement game code side of 3d models in front end
											/*CMenuRender3DModelMgr::TAddedModelIndex attachmentModelIndex = render3DModelMgr->AddModel(pSubGeomDef->modelPath.c_str(), pSubGeomDef->material.c_str(), false);
										  render3DModelMgr->AttachToStaticObject(weaponModelIndex, attachmentModelIndex, mountPointName);*/

											if (stricmp(mountPointName, "attachment_bottom") == 0)
											{
												hasUnderBarrelAttachment = true;
											}
											break;
										}
									}
								}								
							}
						}
					}

					if(hasUnderBarrelAttachment)
					{
						// TODO: Implement game code side of 3d models in front end
						//render3DModelMgr->SetAnim("stand_tac_idlePose_rifle_modifier_add_3p_01", characterModelIndex, 1.0f, 1);
					}
				}
			}
		}

		if (secondaryItemName && secondaryItemName[0])
		{
			CItemSharedParams * pItemSharedParams = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(secondaryItemName, false);
			DRX_ASSERT_TRACE(pItemSharedParams, ("Failed to get item params for class called '%s'", secondaryItemName));

			if (pItemSharedParams)
			{
				tukk  attachPos = pItemSharedParams->params.bone_attachment_01.c_str();
				const SGeometryDef* pGeomDef = pItemSharedParams->GetGeometryForSlot(eIGS_ThirdPerson);

				if (pGeomDef && attachPos[0])
				{
					// TODO: Implement game code side of 3d models in front end
					/*CMenuRender3DModelMgr::TAddedModelIndex weaponModelIndex = render3DModelMgr->AddModel(pItemSharedParams->geometry[1].modelPath.c_str(), pItemSharedParams->geometry[1].material.c_str(), false);
					render3DModelMgr->AttachToCharacter(characterModelIndex, weaponModelIndex, attachPos);*/
				}
			}
		}

		if (explosiveItemName && explosiveItemName[0])
		{
			CItemSharedParams * pItemSharedParams = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(explosiveItemName, false);
			DRX_ASSERT_TRACE(pItemSharedParams, ("Failed to get item params for class called '%s'", explosiveItemName));

			if (pItemSharedParams)
			{
				tukk  attachPos = pItemSharedParams->params.bone_attachment_02.c_str();
				const SGeometryDef* pGeomDef = pItemSharedParams->GetGeometryForSlot(eIGS_ThirdPerson);

				if (pGeomDef && attachPos[0])
				{
					// TODO: Implement game code side of 3d models in front end
					/*CMenuRender3DModelMgr::TAddedModelIndex weaponModelIndex = render3DModelMgr->AddModel(pItemSharedParams->geometry[1].modelPath.c_str(), pItemSharedParams->geometry[1].material.c_str(), false);
					render3DModelMgr->AttachToCharacter(characterModelIndex, weaponModelIndex, attachPos);*/
				}
			}
		}
	}
}

//------------------------------------------------------------------------
tukk CEquipmentLoadout::GetCurrentHighlightPackageName()
{
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	i32k numPackages = group.m_packages.size();
	if (group.m_highlightedPackage >= 0 && group.m_highlightedPackage < numPackages)
	{
		SEquipmentPackage &package = group.m_packages[group.m_highlightedPackage];

		DRX_ASSERT(package.m_flags & ELOF_CAN_BE_CUSTOMIZED);

		return package.m_displayName.c_str();
	}

	return "";
}

//------------------------------------------------------------------------
tukk CEquipmentLoadout::GetCurrentSelectPackageName()
{
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	i32k numPackages = group.m_packages.size();
	if (group.m_selectedPackage >= 0 && group.m_selectedPackage < numPackages)
	{
		SEquipmentPackage &package = group.m_packages[group.m_selectedPackage];

		DRX_ASSERT(package.m_flags & ELOF_CAN_BE_CUSTOMIZED);

		return package.m_displayName.c_str();
	}

	return "";
}

//------------------------------------------------------------------------
void CEquipmentLoadout::GetWeaponSkinsForWeapon(i32 itemId, TAttachmentsArr& arrayAttachments)
{
	i32k allItemsSize = (i32)m_allItems.size();
	if (itemId >= 0 && itemId < allItemsSize)
	{
		for (i32 i=m_weaponsStartIndex; i<allItemsSize; ++i)
		{
			SEquipmentItem &item = m_allItems[i];

			if (item.m_category==eELC_WEAPON && itemId!=0 && itemId==item.m_parentItemId)
			{
				const bool bLocked = IsItemLocked(i, true);

				// Dont add skin if it should be hidden
				if (bLocked && item.m_hideWhenLocked)
				{
					continue;
				}
				else
				{
					u8 currentGroupBit = BIT(m_currentPackageGroup); // Use current group, should be correct
					if ((item.m_allowedPackageGroups & currentGroupBit) == 0)
					{
						continue;
					}
				}

				SAttachmentInfo attachmentInfo(item.m_uniqueIndex);
				attachmentInfo.m_locked = (bLocked ? 1 : 0 );
				arrayAttachments.push_back(attachmentInfo);
			}
		}
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::GetAvailableAttachmentsForWeapon(i32 itemId, TAttachmentsArr* arrayAttachments, TDefaultAttachmentsArr* arrayDefaultAttachments, bool bNeedUnlockDescriptions /*= true*/)
{
	i32 allItemsSize = (i32)m_allItems.size();

	if (itemId >= 0 && itemId < allItemsSize)
	{
		CItemSharedParams* pItemShared = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(m_allItems[itemId].m_name, false);
		if (pItemShared)
		{
			i32k numAccessoryParams = pItemShared->accessoryparams.size();

			m_lastSelectedWeaponName = m_allItems[itemId].m_name.c_str();

			for(i32 i = 0; i < numAccessoryParams; i++)
			{
				SAccessoryParams& accessoryParams = pItemShared->accessoryparams[i];
				tukk accessoryName = accessoryParams.pAccessoryClass->GetName();

#if !defined(_RELEASE)
				if ( (stricmp(accessoryParams.category.c_str(), "bottom")==0)
					|| (stricmp(accessoryParams.category.c_str(), "scope")==0)
					|| (stricmp(accessoryParams.category.c_str(), "barrel")==0)
					|| (stricmp(accessoryParams.category.c_str(), "ammo")==0))
				{
#endif
					for (i32 j=m_attachmentsStartIndex; j<allItemsSize; ++j)
					{
						SEquipmentItem &item = m_allItems[j];

						if (item.m_category != eELC_ATTACHMENT)
							break;

						if (stricmp(accessoryName, item.m_name.c_str())==0)
						{
							const bool bLocked = IsItemLocked(j, bNeedUnlockDescriptions, NULL) && !IsItemInCurrentLoadout(j);

							// Dont add attachment if it should be hidden
							if (bLocked && item.m_hideWhenLocked)
							{
								continue;
							}
							else
							{
								u8 currentGroupBit = BIT(m_currentPackageGroup); // Use current group, should be correct
								if ((item.m_allowedPackageGroups & currentGroupBit) == 0)
								{
									continue;
								}
							}

							if (accessoryParams.defaultAccessory==false)
							{
								if (arrayAttachments)
								{
									SAttachmentInfo attachmentInfo(item.m_uniqueIndex);
									attachmentInfo.m_locked = (bLocked ? 1 : 0);
									arrayAttachments->push_back(attachmentInfo);
								}
							}
							else
							{
								if (arrayDefaultAttachments)
								{
									arrayDefaultAttachments->push_back(item.m_uniqueIndex);
								}
							}
							//DrxLog("Found loadout attachment %s", item.m_name.c_str());
						}
					}
#if !defined(_RELEASE)
				}
				else
				{
					DrxLog("Unknown loadout attachment <%s> category<%s>", accessoryName, accessoryParams.category.c_str());
					DesignerWarning(0, "Unknown loadout attachment <%s> category<%s>", accessoryName, accessoryParams.category.c_str());
				}
#endif
			}
		}
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::GetListedAttachmentsForWeapon(i32 itemId, TAttachmentsArr* arrayAttachments )
{
	i32 allItemsSize = (i32)m_allItems.size();

	if( itemId >= 0 && itemId < allItemsSize )
	{
		CItemSharedParams* pItemShared = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters( m_allItems[ itemId ].m_name, false );
		if( pItemShared )
		{
			i32k numAccessoryParams = pItemShared->accessoryparams.size();

			for( i32 i = 0; i < numAccessoryParams; i++ )
			{
				SAccessoryParams& accessoryParams = pItemShared->accessoryparams[ i ];
				tukk accessoryName = accessoryParams.pAccessoryClass->GetName();

				for( i32 j = m_attachmentsStartIndex; j < allItemsSize; ++j )
				{
					SEquipmentItem &item = m_allItems[ j ];

					if (item.m_category != eELC_ATTACHMENT)
						break;

					if (stricmp(accessoryName, item.m_name.c_str())==0)
					{
						if (arrayAttachments)
						{
							SAttachmentInfo attachmentInfo(item.m_uniqueIndex);
							arrayAttachments->push_back(attachmentInfo);
						}
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------
#if LIST_LOADOUT_CONTENTS_ON_SCREEN
void CEquipmentLoadout::ListLoadoutContentsOnScreen()
{
	DrxWatch ("Currently %s, last selected weapon '%s' (category=%d, Nth=%d)", m_currentlyCustomizingWeapon ? "customizing a weapon" : "not customising anything", m_lastSelectedWeaponName.c_str(), m_currentCategory, m_currentNth);

	for (u8 i=0; i<EQUIPMENT_LOADOUT_NUM_SLOTS; i++)
	{
		DrxWatch ("Contents[%u] = %u '%s'", i, m_currentCustomizePackage[i], GetItemNameFromIndex(m_currentCustomizePackage[i]));
	}
}
#endif

//------------------------------------------------------------------------
void CEquipmentLoadout::SetDefaultAttachmentsForWeapon(i32 itemId, u32 Nth)
{
	TDefaultAttachmentsArr arrayDefaultAttachments;

	GetAvailableAttachmentsForWeapon(itemId, NULL, &arrayDefaultAttachments);

	SetNthWeaponAttachments(Nth, arrayDefaultAttachments);

	Display3DItem(m_currentCustomizePackage[GetSlotIndex(eELC_WEAPON, Nth)]);
}

//------------------------------------------------------------------------
void CEquipmentLoadout::SetNthWeaponAttachments(u32 Nth, TDefaultAttachmentsArr& arrayAttachments)
{
	while (arrayAttachments.size()<MAX_DISPLAY_DEFAULT_ATTACHMENTS)	// Pad to max attachments to null any other slots
	{
		arrayAttachments.push_back(0);
	};

	i32 attachmentNth = 0;
	i32k slot = GetSlotIndex(eELC_WEAPON, Nth) + 1;	// after weapon slot
	i32k numAttachments = arrayAttachments.size();
	for (i32 i=slot; i<EQUIPMENT_LOADOUT_NUM_SLOTS && attachmentNth<numAttachments; ++i)
	{
		i32 itemId = m_currentCustomizePackage[i];
		if (m_slotCategories[i] == eELC_ATTACHMENT)
		{
			//DrxLogAlways("Save attachments slot:%d attachmentNow:%d, attachmentOld;%d", i, m_currentCustomizePackage[i], attachments[attachmentNth]);
			if (m_currentCustomizePackage[i] != arrayAttachments[attachmentNth])
			{
				m_currentCustomizePackage[i] = arrayAttachments[attachmentNth];
			}
			++attachmentNth;
		}
		else if (m_slotCategories[i] == eELC_WEAPON)	// Onto next weapon, bail
			break;
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::ResetLoadoutsToDefault()
{
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	u32k numPackages = group.m_packages.size();
	for (u32 i = 0; i < numPackages; ++ i)
	{
		SEquipmentPackage &package = group.m_packages[i];

		for (i32 a=0; a<EQUIPMENT_LOADOUT_NUM_SLOTS; ++a)
		{
			package.m_contents[a] = package.m_defaultContents[a];
		}
	}
	group.m_selectedPackage = 0;
}

//------------------------------------------------------------------------
void CEquipmentLoadout::CheckPresale()
{
	bool anyChangesMade = false;

	SPackageGroup &packageGroup = m_packageGroups[CEquipmentLoadout::SDK];
	i32k numItems = m_allItems.size();
	u32k numPackages = packageGroup.m_packages.size();
	for( u32 iPackage = 0; iPackage < numPackages; ++iPackage )
	{
		SEquipmentPackage &package = packageGroup.m_packages[ iPackage ];

		for( i32 iSlot = 0; iSlot < EQUIPMENT_LOADOUT_NUM_SLOTS; ++iSlot )
		{
			u8 itemId = package.m_contents[ iSlot ];
			if( itemId >= numItems )
			{
				DrxLogAlways("CEquipmentLoadout::CheckPresale Invalid item id");
				continue;
			}

			if( itemId == 0 ) // nil item
			{
				continue;
			}

			if( package.m_contents[ iSlot ] == package.m_defaultContents[ iSlot ] )
			{
				//no point checking if it's the default
				continue;
			}

			SEquipmentItem &item = m_allItems[ itemId ];
			EUnlockType unlockType = GetUnlockTypeForItem( itemId );

			SPPHaveUnlockedQuery results;

			bool locked = IsLocked( unlockType, item.m_name.c_str(), results );

			if( ! locked && item.m_parentItemId != 0 )
			{
				EUnlockType parentUnlockType = GetUnlockTypeForItem( item.m_parentItemId );

				SEquipmentItem &parentItem = m_allItems[ item.m_parentItemId ];

				locked = IsLocked( parentUnlockType, parentItem.m_name.c_str(), results );
			}

			if( locked && results.exists )
			{
				//don't have access to this thing, replace with default;
				package.m_contents[ iSlot ] = package.m_defaultContents[ iSlot ];

				anyChangesMade = true;

				//if we've reset a weapon, also reset the attachments on the weapon
				if( m_slotCategories[ iSlot ] == eELC_WEAPON )
				{
					i32 iAttachment = iSlot+1;
					while( m_slotCategories[ iAttachment ] == eELC_ATTACHMENT )
					{
						package.m_contents[ iAttachment ] = package.m_defaultContents[ iAttachment ];
						iAttachment++;
					}
				}
			}
		}
	}	
}

//------------------------------------------------------------------------
i32 CEquipmentLoadout::GetItemIndexFromName(tukk name)
{
	TEquipmentItems::iterator itCurrent = m_allItems.begin();
	TEquipmentItems::iterator itEnd = m_allItems.end();

	while (itCurrent != itEnd)
	{
		if (!stricmp(name, itCurrent->m_name.c_str()))
			return itCurrent->m_uniqueIndex;

		++itCurrent;
	}

	return 0;
}

//------------------------------------------------------------------------
tukk CEquipmentLoadout::GetItemNameFromIndex(i32 idx)
{
	i32k numItems = m_allItems.size();
	if (idx > 0  && idx < numItems)
	{
		return m_allItems.at(idx).m_name.c_str();
	}

	return "";
}

//------------------------------------------------------------------------
i32 CEquipmentLoadout::GetPackageIndexFromName(EEquipmentPackageGroup group, tukk name)
{
	if (name)
	{
		SPackageGroup &packageGroup = m_packageGroups[group];
		i32 num = packageGroup.m_packages.size();

		for (i32 i=0; i<num; ++i)
		{
			if (!stricmp(name, packageGroup.m_packages[i].m_name.c_str()))
			{
				return i;
			}
		}
	}

	return INVALID_INDEX;
}

//------------------------------------------------------------------------
i32 CEquipmentLoadout::GetPackageIndexFromId( EEquipmentPackageGroup group, i32 id )
{
	SPackageGroup &packageGroup = m_packageGroups[group];
	i32 numPackages = packageGroup.m_packages.size();

	for (i32 i = 0; i < numPackages; ++ i)
	{
		if (packageGroup.m_packages[i].m_id == id)
		{
			return i;
		}
	}
	return INVALID_INDEX;
}

//------------------------------------------------------------------------
tukk CEquipmentLoadout::GetPackageDisplayFromIndex(i32 index)
{
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	i32k numPackages = group.m_packages.size();
	if (index < numPackages)
	{
		SEquipmentPackage& pEquipment = group.m_packages[index];
		
		return pEquipment.m_displayName.c_str();
	}

	return NULL;
}

//------------------------------------------------------------------------
tukk CEquipmentLoadout::GetPackageDisplayFromName(tukk name)
{
	SEquipmentPackage* pEquipment = GetPackageFromName(name);
	if(pEquipment)
	{
		return pEquipment->m_displayName.c_str();
	}

	return NULL;
}

//------------------------------------------------------------------------
CEquipmentLoadout::SEquipmentPackage *CEquipmentLoadout::GetPackageFromName(tukk name)
{
	if (name)
	{
		char groupName[32];
		size_t skipChars = drx_copyStringUntilFindChar(groupName, name, sizeof(groupName), '_');
		EEquipmentPackageGroup group = KEY_BY_VALUE(groupName, s_equipmentPackageGroups);
		i32 index = GetPackageIndexFromName(group, name + skipChars);
		if (index >= 0)
		{
			return &m_packageGroups[group].m_packages[index];
		}
	}

	return NULL;
}

//------------------------------------------------------------------------
void CEquipmentLoadout::ClGetCurrentLoadoutParams( CGameRules::EquipmentLoadoutParams &params )
{
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	i32k numPackages = group.m_packages.size();
	if (group.m_selectedPackage < numPackages)
	{
		TEquipmentPackageContents &contents = group.m_packages[group.m_selectedPackage].m_contents;

		memcpy( (uk )params.m_contents, (uk )contents, sizeof(contents));
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::OnGameEnded()
{
	m_clientLoadouts.clear();

	m_currentPackageGroup = SDK;
}

//------------------------------------------------------------------------
void CEquipmentLoadout::OnRoundEnded()
{
	TClientEquipmentPackages::const_iterator end = m_clientLoadouts.end();
	for (TClientEquipmentPackages::iterator it = m_clientLoadouts.begin(); it != end; ++ it)
	{
//		IActor *pActor = g_pGame->GetGameRules()->GetActorByChannelId(it->first);
//		DrxLog("CEquipmentLoadout::OnRoundEnded() clearing set on server for actor=%s", pActor ? pActor->GetEntity()->GetName() : "NULL");
		it->second.m_flags &= ~ELOF_HAS_BEEN_SET_ON_SERVER;
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::PrecacheLevel()
{
	LOADING_TIME_PROFILE_SECTION;
	CGameSharedParametersStorage* pGameParamsStorage = g_pGame->GetGameSharedParametersStorage();
	std::vector<SEquipmentItem>::const_iterator it = m_allItems.begin();
	std::vector<SEquipmentItem>::const_iterator itEnd = m_allItems.end();
	for(; it != itEnd; ++it)
	{
		IEntityClass* pItemClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(it->m_name);
		if (pItemClass)
		{
			CItemSharedParams *pSharedParams = pGameParamsStorage->GetItemSharedParameters(pItemClass->GetName(), false);
			if(!pSharedParams)
			{
				GameWarning("Uninitialised item params. Has the xml been setup correctly for item %s?", pItemClass->GetName());
			}
			else
			{
				pSharedParams->CacheResourcesForLevelStartMP(pGameParamsStorage->GetItemResourceCache(), pItemClass);
			}
		}
	}


	IEntityClass* pBinocularsClass = CItem::sBinocularsClass;
	if (!pBinocularsClass)
	{
		pBinocularsClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Binoculars");
	}

	if (pBinocularsClass)
	{
		CItemSharedParams *pBinocularSharedParams = pGameParamsStorage->GetItemSharedParameters(pBinocularsClass->GetName(), false);
		if(!pBinocularSharedParams)
		{
			GameWarning("Uninitialised item params. Has the xml been setup correctly for item %s?", pBinocularsClass->GetName());
		}
		else
		{
			pBinocularSharedParams->CacheResourcesForLevelStartMP(pGameParamsStorage->GetItemResourceCache(), pBinocularsClass);
		}
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::InvalidateLastSentLoadout()
{
	DrxLog("CEquipmentLoadout::InvalidateLastSentLoadout()");

	for (i32 i = 0; i < eEPG_NumGroups; ++ i)
	{
		m_packageGroups[i].m_lastSentPackage = INVALID_PACKAGE;		
	}
}

//------------------------------------------------------------------------
void CEquipmentLoadout::ForceSelectLastSelectedLoadout()
{
	//This prevents any mis-setting of this from meaning that the force loadout select fails
	m_bGoingBack = false;

	// Select the currently highlighted package.
	SetSelectedPackage(GetCurrentHighlightPackageIndex());

	// Ensure we actually send the loadout
	InvalidateLastSentLoadout();

	g_pGame->GetUI()->ForceCompletePreGame();
}

//------------------------------------------------------------------------
i32 CEquipmentLoadout::GetSelectedPackage() const
{
	const SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	return group.m_selectedPackage;
}

//------------------------------------------------------------------------
i32 CEquipmentLoadout::GetCurrentHighlightPackageIndex() const
{
	const SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	return group.m_highlightedPackage;
}

//------------------------------------------------------------------------
void CEquipmentLoadout::OnGameReset()
{
	// Mark all loadouts as changed
	TClientEquipmentPackages::const_iterator end = m_clientLoadouts.end();
	for (TClientEquipmentPackages::iterator it = m_clientLoadouts.begin(); it != end; ++ it)
	{
		SClientEquipmentPackage &package = it->second;
		package.m_flags |= (ELOF_HAS_CHANGED);
	}
}

const CEquipmentLoadout::SPackageGroup& CEquipmentLoadout::GetCurrentPackageGroup()
{
	return m_packageGroups[m_currentPackageGroup];
}

const CEquipmentLoadout::SEquipmentPackage* CEquipmentLoadout::GetPackageFromId(i32k index)
{
	return GetPackageFromIdInternal(index);
}

CEquipmentLoadout::SEquipmentPackage* CEquipmentLoadout::GetPackageFromIdInternal(i32k index)
{
	if (index>=0 && index<(i32)m_packageGroups[m_currentPackageGroup].m_packages.size())
	{
		return &m_packageGroups[m_currentPackageGroup].m_packages[index];
	}
	return NULL;
}

const CEquipmentLoadout::SEquipmentItem* CEquipmentLoadout::GetItem(i32k index)
{
	if (index>=0 && index<(i32)m_allItems.size())
	{
		return &m_allItems[index];
	}
	return NULL;
}

const CEquipmentLoadout::SEquipmentItem* CEquipmentLoadout::GetItemByName(tukk name) const
{
	if (name && name[0]!=0)
	{
		TEquipmentItems::const_iterator itCurrent = m_allItems.begin();
		TEquipmentItems::const_iterator itEnd = m_allItems.end();

		while (itCurrent != itEnd)
		{
			if (stricmp(name, itCurrent->m_name.c_str())==0)
			{
				return &(*itCurrent);
			}

			++itCurrent;
		}
	}

	return NULL;
}

CEquipmentLoadout::EEquipmentLoadoutCategory CEquipmentLoadout::GetSlotCategory(i32k slot)
{
	if (slot>=0 && slot<EQUIPMENT_LOADOUT_NUM_SLOTS)
	{
		return (EEquipmentLoadoutCategory)m_slotCategories[slot];
	}
	return eELC_NONE;
}

tukk  CEquipmentLoadout::GetModelName( u8 modelIndex ) const
{
	return m_modelNames[modelIndex].c_str();
}

u8 CEquipmentLoadout::GetModelIndexOverride( u16 channelId ) const
{
	TClientEquipmentPackages::const_iterator it= m_clientLoadouts.find(channelId);
	if (it != m_clientLoadouts.end())
	{
		return it->second.m_modelIndex;
	}
	return MP_MODEL_INDEX_DEFAULT;
}

u8 CEquipmentLoadout::AddModel( tukk pModelName )
{
	u8 index = MP_MODEL_INDEX_DEFAULT;
	i32 numKnownModels = m_modelNames.size();
	for (i32 i = 0; i < numKnownModels; ++ i)
	{
		if (!stricmp(m_modelNames[i].c_str(), pModelName))
		{
			index = (u8)i;
			break;
		}
	}
	if (index == MP_MODEL_INDEX_DEFAULT)
	{
#ifndef _RELEASE
		if (m_modelNames.size() == (MP_MODEL_INDEX_DEFAULT - 1))
		{
			DrxFatalError("Too many player models");
		}
#endif
		m_modelNames.push_back(pModelName);
		index = m_modelNames.size() - 1;
	}
	return index;
}

i32 CEquipmentLoadout::IsWeaponInCurrentLoadout( const ItemString &weaponName )
{
	SPackageGroup &group = m_packageGroups[m_currentPackageGroup];
	i32k numPackage = group.m_packages.size();
	if ((numPackage > 0) && (numPackage > (i32)group.m_selectedPackage))
	{
		TEquipmentPackageContents &contents = group.m_packages[group.m_selectedPackage].m_contents;
		i32 idx = GetItemIndexFromName(weaponName.c_str());
		i32 weaponNumber = 0;
		i32 slotIdx = GetSlotIndex(eELC_WEAPON, weaponNumber);
		while (slotIdx != -1)
		{
			if (contents[slotIdx] == idx)
			{
				return weaponNumber;
			}
			++weaponNumber;
			slotIdx = GetSlotIndex(eELC_WEAPON, weaponNumber);
		}
	}
	return -1;
}

bool CEquipmentLoadout::IsItemInCurrentLoadout( i32k index ) const
{
	const SPackageGroup& group = m_packageGroups[m_currentPackageGroup];
	i32k numPackage = group.m_packages.size();
	if (group.m_selectedPackage>=0 && group.m_selectedPackage<numPackage)
	{
		const TEquipmentPackageContents &contents = group.m_packages[group.m_selectedPackage].m_contents;
		for (i32 i=0; i<EQUIPMENT_LOADOUT_NUM_SLOTS; i++)
		{
			if(contents[i]==index)
			{
				return true;
			}
		}
	}
	return false;
}

void CEquipmentLoadout::RandomChoiceModel()
{
	m_bShowCellModel = drx_random(0, 1) == 0;
}

void CEquipmentLoadout::UpdateClassicModeModel( u16 channelId, i32 teamId )
{
	TClientEquipmentPackages::iterator it= m_clientLoadouts.find(channelId);
	if (it != m_clientLoadouts.end())
	{
		SClientEquipmentPackage &package = it->second;

		i32 numItems = m_allItems.size();
		for (i32 i = 0; i < EQUIPMENT_LOADOUT_NUM_SLOTS; ++ i)
		{
			u8 itemId = package.m_contents[i];
			if ((itemId > 0) && (itemId < numItems))
			{
				SEquipmentItem &item = m_allItems[itemId];
				if (item.m_category == eELC_WEAPON)
				{
					package.m_modelIndex = GetClassicModeModel(itemId, (teamId == 2) ? SClassicModel::eMT_Cell : SClassicModel::eMT_Rebel);
					break;
				}
			}
		}
	}
}

u8 CEquipmentLoadout::GetClassicModeModel( i32 itemId, SClassicModel::EModelType modelType ) const
{
	TClassicMap::const_iterator it = m_classicMap.find(itemId);
	if (it != m_classicMap.end())
	{
		const SClassicModel& classicModel = it->second;
		return classicModel.m_modelIndex[modelType];
	}
	return MP_MODEL_INDEX_DEFAULT;
}

void CEquipmentLoadout::StreamFPGeometry( const EPrecacheGeometryEvent event, i32k package )
{
	const SPackageGroup& group = m_packageGroups[m_currentPackageGroup];
	const TEquipmentPackageContents& contents = group.m_packages[package].m_contents;
	CGameSharedParametersStorage* pStorage = g_pGame->GetGameSharedParametersStorage();

	i32 entry = 0;
	if(ICharacterUpr* pCharUpr = gEnv->pCharacterUpr)
	{
		for (i32 i=0; i<EQUIPMENT_LOADOUT_NUM_SLOTS; i++)
		{
			if( m_slotCategories[i] == eELC_WEAPON )
			{
				tukk  itemName = GetItemNameFromIndex(contents[i]);
				if (itemName != NULL && itemName[0])
				{
					if(CItemSharedParams* pItemSharedParams = pStorage->GetItemSharedParameters(itemName, false))
					{
						if(const SGeometryDef* pGeomDef = pItemSharedParams->GetGeometryForSlot(eIGS_FirstPerson))
						{
							pCharUpr->StreamKeepCharacterResourcesResident(pGeomDef->modelPath.c_str(), 0, true);
							m_streamedGeometry[(event*FPWEAPONCACHEITEMS)+entry] = pGeomDef->modelPath.c_str();
							DrxLog("Streaming in: %s", pGeomDef->modelPath.c_str());
							if(++entry==FPWEAPONCACHEITEMS)
								break;
						}
					}
				}
			}
		}
	}
}

void CEquipmentLoadout::ReleaseStreamedFPGeometry( const EPrecacheGeometryEvent event )
{
	ICharacterUpr* pCharUpr = gEnv->pCharacterUpr;
	for(i32 i=0; i<ePGE_Total; i++)
	{
		if( event==ePGE_All || event==(i/FPWEAPONCACHEITEMS) )
		{
			if(pCharUpr)
			{
				pCharUpr->StreamKeepCharacterResourcesResident(m_streamedGeometry[i].c_str(), 0, false);
			}
			m_streamedGeometry[i] = "";
		}
	}
}

void CEquipmentLoadout::Debug()
{
	for(i32 i=0; i<ePGE_Total; i++)
	{
		DrxWatch("[File: %s] %s", m_streamedGeometry[i].c_str(), gEnv->pCharacterUpr ? gEnv->pCharacterUpr->StreamHasCharacterResources(m_streamedGeometry[i].c_str(), 0) ? "LOADED" : "LOADING" : "NO CHARACTER MANAGER" );
	}
}

void CEquipmentLoadout::InitUnlockedAttachmentFlags()
{
	m_unlockedAttachments.clear();

	TWeaponsVec::iterator weaponsIter = m_allWeapons.begin();
	TWeaponsVec::iterator weaponsEnd  = m_allWeapons.end();

	CPlayerProgression* pPlayerProgression = CPlayerProgression::GetInstance();
	string fullAttachmentName;

	for( ; weaponsIter < weaponsEnd; ++weaponsIter )
	{
		SEquipmentItem& weapItem = m_allItems[ weaponsIter->m_id ];
		tukk pWeapName = weapItem.m_name.c_str();

		u32 unlockedAttachFlags = 0;

		if( CItemSharedParams* pItemSharedParams = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(pWeapName, false) )
		{
			TAccessoryParamsVector& accessories = pItemSharedParams->accessoryparams;
			i32k numAccessoryParams = accessories.size();

			u32 attachBit = 1;
		
			for( i32 iAttach = 0; iAttach < numAccessoryParams; iAttach++, attachBit <<= 1 )
			{
				const SAccessoryParams* accessoryParams = &accessories[iAttach];
				tukk itemName = accessoryParams->pAccessoryClass->GetName();

				tukk tempItemNameStr;

				// Make naming match up with existing names
				if( strstr( itemName, "Ironsight" ) != NULL )
				{
					tempItemNameStr = "Ironsight";
				}
				else if( stricmp( itemName, "Single" ) == 0 )
				{
					tempItemNameStr = "SemiAutomaticFire";
				}
				else
				{
					tempItemNameStr = itemName;
				}

				fullAttachmentName.Format( "%s.%s", pWeapName,  tempItemNameStr );

				SPPHaveUnlockedQuery results;
				bool found = pPlayerProgression->HaveUnlocked( eUT_Attachment, fullAttachmentName.c_str(), results);

				if( results.unlocked || found == false )
				{
					unlockedAttachFlags |= attachBit;
				}
			}

			u32 crc = CCrc32::ComputeLowercase( weapItem.m_name.c_str() );
			
			m_unlockedAttachments.insert( TWeaponAttachmentFlagMap::value_type( crc, unlockedAttachFlags ) );
		}
	}
}

u32 CEquipmentLoadout::GetWeaponAttachmentFlags( tukk pWeaponName )
{
	if( IEntityClass* pWeaponClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass( pWeaponName ) )
	{
		g_pGame->GetWeaponSystem()->GetWeaponAlias().UpdateClass( &pWeaponClass );

		tukk className = pWeaponClass->GetName();

		u32 crc = CCrc32::ComputeLowercase( className );

		TWeaponAttachmentFlagMap::iterator it = m_currentAvailableAttachments.find( crc );

		if( it != m_currentAvailableAttachments.end() )
		{
			return it->second;
		}
	}

	return 0;
}

void CEquipmentLoadout::UpdateWeaponAttachments( IEntityClass* pWeaponClass, IEntityClass* pAccessory )
{
	g_pGame->GetWeaponSystem()->GetWeaponAlias().UpdateClass( &pWeaponClass );

	tukk className = pWeaponClass->GetName();

	u32 crc = CCrc32::ComputeLowercase( className );

	DrxLog( "CEquipmentLoadout::UpdateWeaponAttachments weap %s attach %s", className, pAccessory->GetName() );

	TWeaponAttachmentFlagMap::iterator it = m_currentAvailableAttachments.find( crc );

	if( it != m_currentAvailableAttachments.end() )
	{
		if( CItemSharedParams * pItemSharedParams = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(className, false) )
		{
			TAccessoryParamsVector& accessories = pItemSharedParams->accessoryparams;
			i32k numAccessoryParams = accessories.size();

			u32 attachBit = 1;

			for( i32 iAttach = 0; iAttach < numAccessoryParams; iAttach++, attachBit <<= 1 )
			{
				//can direct compare pointers as only one instance of each class type
				if( accessories[ iAttach ].pAccessoryClass == pAccessory )
				{
					it->second |= attachBit;
					break;
				}
			}
		}
	}
}

void CEquipmentLoadout::SetCurrentAttachmentsToUnlocked()
{
	m_currentAvailableAttachments = m_unlockedAttachments;
}

void CEquipmentLoadout::SpawnedWithLoadout( u8 loadoutIdx )
{
	SPackageGroup &group = m_packageGroups[ m_currentPackageGroup ];
	i32k numPackage = group.m_packages.size();
	if( numPackage > loadoutIdx )
	{
		SEquipmentPackage& package = group.m_packages[loadoutIdx];

		//only needed for default loadouts
		if( (package.m_flags & ELOF_CAN_BE_CUSTOMIZED) == 0 )
		{
			TEquipmentPackageContents &contents = package.m_contents;

			tukk currentWeapon = NULL;
			IEntityClass* pWeaponClass = NULL;
			IEntityClassRegistry* pReg = gEnv->pEntitySystem->GetClassRegistry();

			for (i32 i=0; i<EQUIPMENT_LOADOUT_NUM_SLOTS; ++i)
			{
				if ( m_slotCategories[i] == eELC_WEAPON )
				{
					currentWeapon = m_allItems[ package.m_contents[ i ] ].m_name.c_str();
					pWeaponClass = pReg->FindClass( currentWeapon );
				}
				else if ( m_slotCategories[ i ] == eELC_ATTACHMENT )
				{
					//only non-zero contents IDs
					if( i32 contentindx = package.m_contents[ i ]  )
					{
						tukk attachmentName = m_allItems[ contentindx ].m_name.c_str();
						IEntityClass* pAttachmentClass = pReg->FindClass( attachmentName );
						UpdateWeaponAttachments( pWeaponClass, pAttachmentClass );
					}
				}
			}
		}
	}
}



 
