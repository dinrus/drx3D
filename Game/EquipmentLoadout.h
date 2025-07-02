// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Equipment Loadouts for C2MP

-------------------------------------------------------------------------
История:
- 18:09:2009  : Created by Ben Johnson

*************************************************************************/

#ifndef __EQUIPMENT_LOADOUT_H__
#define __EQUIPMENT_LOADOUT_H__

# pragma once

#include <drx3D/Game/ItemString.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/GameRulesModules/IGameRulesClientConnectionListener.h>
#include <drx3D/Game/ProgressionUnlocks.h>
#include <drx3D/Game/AutoEnum.h>
#include <drx3D/Act/IPlayerProfiles.h>
#include <drx3D/CoreX/Containers/VectorMap.h>
#include <drx3D/Game/UI/UITypes.h>

#ifndef _RELEASE
	#define LIST_LOADOUT_CONTENTS_ON_SCREEN 0 // Feel free to turn this on locally, but please don't commit it as anything but 0 [TF]
#else
	#define LIST_LOADOUT_CONTENTS_ON_SCREEN 0
#endif

#define MAX_DISPLAYNAME_LENGTH 15
#define MAX_SERVER_OVERRIDE_PACKAGES 10

#define MAX_DISPLAY_ATTACHMENTS 32
#define MAX_DISPLAY_DEFAULT_ATTACHMENTS 3
#define INVALID_PACKAGE -1

struct IFlashPlayer;
struct IItemSystem;
class CGameRules;
struct SEquipmentItem; 

class CEquipmentLoadout : public IGameRulesClientConnectionListener
	, public IPlayerProfileListener
{
public:

#define FPWEAPONCACHEITEMS 1
	enum EPrecacheGeometryEvent
	{
		ePGE_OnDeath=0,
		ePGE_OnTabSwap,

		ePGE_All,
		
		ePGE_Total = ePGE_All*FPWEAPONCACHEITEMS,
	};

	enum EEquipmentLoadoutFlags
	{
		ELOF_NONE											= 0,
		ELOF_HAS_CHANGED							= (1<<0),		// Var to indicate a loadout has changed since last being sent.
		ELOF_HAS_BEEN_SET_ON_SERVER		= (1<<1),		// On the server. Has the loadout ever been set for a client - for use in m_clientLoadouts 
		ELOF_CAN_BE_CUSTOMIZED				= (1<<2),		// Whether the loadout can be customized in the menus
		ELOF_HIDE_WHEN_LOCKED					= (1<<3),		// Hide this package when it is locked
	};

	// Categories of loadout slots
	enum EEquipmentLoadoutCategory
	{
		eELC_NONE             = 0,
		eELC_WEAPON						= 1,
		eELC_EXPLOSIVE        = 2,
		eELC_ATTACHMENT       = 3,
		eELC_SIZE             = 4
	};

	// Categories used by the UI when customising
	enum EEquipmentLoadoutUICategories
	{
		eELUIC_None										= 0,
		eELUIC_PrimaryWeapon          = 1,
		eELUIC_SecondaryWeapon				= 2,
		eELUIC_ExplosiveWeapon        = 3,
		eELUIC_BowAccessory						= 4,
	};

#define EQUIPMENT_PACKAGE_TYPES(f) \
	f(SDK)					

	AUTOENUM_BUILDENUMWITHTYPE_WITHNUM(EEquipmentPackageGroup, EQUIPMENT_PACKAGE_TYPES, eEPG_NumGroups);

	struct SLoadoutWeapon
	{
		SLoadoutWeapon() {}
		SLoadoutWeapon(tukk pDisplayName, i32 id, bool showSwitchWeapon) : m_displayName(pDisplayName), m_id(id), m_showSwitchWeapon(showSwitchWeapon) {}

		DrxFixedStringT<32> m_displayName;
		i32 m_id;
		bool m_showSwitchWeapon;
	};
	typedef std::vector<SLoadoutWeapon> TWeaponsVec;

	struct SClassicModel
	{
		enum EModelType
		{
			eMT_Rebel=0,
			eMT_Cell,
			eMT_Total
		};

		SClassicModel()
		{
			memset(&m_modelIndex, MP_MODEL_INDEX_DEFAULT, sizeof(m_modelIndex[0])*eMT_Total);
		}
		u8 m_modelIndex[eMT_Total];
	};

	CEquipmentLoadout();
	~CEquipmentLoadout();

	// IGameRulesClientConnectionListener
	virtual void OnClientConnect(i32 channelId, bool isReset, EntityId playerId);
	virtual void OnClientDisconnect(i32 channelId, EntityId playerId);
	virtual void OnClientEnteredGame(i32 channelId, bool isReset, EntityId playerId) {}
	virtual void OnOwnClientEnteredGame() {};
	// ~IGameRulesClientConnectionListener

	void PrecacheLevel();

	//IPlayerProfileListener
	virtual void SaveToProfile(IPlayerProfile* pProfile, bool online, u32 reason);
	virtual void LoadFromProfile(IPlayerProfile* pProfile, bool online, u32 reason);
	//~IPlayerProfileListener
	
	void SelectProfileLoadout();

	void ClSendCurrentEquipmentLoadout(i32 channelId);
	
	void SvSetClientLoadout(i32 channelId, const CGameRules::EquipmentLoadoutParams &params);

	void SvAssignClientEquipmentLoadout(i32 channelId, i32 playerId);
	void ClAssignLastSentEquipmentLoadout(CActor *pActor);

	bool SvHasClientEquipmentLoadout(i32 channelId);
	void SvRemoveClientEquipmentLoadout(i32 channelId);

	void FlagItemAsNew(tukk itemName);
	void FlagItemAttachmentAsNew(tukk itemName);
	void ClearItemNewFlags(i32k itemId);

	// Init
	void UpdateSpawnStatusMessage(); // Soon to be depreciated by equipment menu object CMenuMPEquipmentSelect
	void SendSpawnStatusMessageToFlash(tukk pMessage); // Soon to be depreciated by equipment menu object CMenuMPEquipmentSelect

	void SetPackageGroup( EEquipmentPackageGroup loadoutSet ) { m_currentPackageGroup = loadoutSet; }
	EEquipmentPackageGroup GetPackageGroup() const { return m_currentPackageGroup; }

	// Customizing
	bool SetPackageCustomizeItem(i32 packageId, i32 itemId, i32 category, i32 Nth);

	void SetCurrentCustomizePackage();
	void SetCurrentCustomizeItem(i32 itemId);
	const bool HasCurrentCustomizePackageChanged() const;
	bool HasLastSavedPackageChanged();

	void SetSelectedPackage(i32 index);
	i32 GetSelectedPackage() const;
	tukk GetCurrentSelectPackageName();

	void ResetLoadoutsToDefault();
	void CheckPresale();

	// Loadout select highlights
	void SetHighlightedPackage(i32 packageIndex);
	i32 GetCurrentHighlightPackageIndex() const;
	tukk GetCurrentHighlightPackageName();

	void SetHasPreGameLoadoutSent(bool hasSent) { m_hasPreGamePackageSent = hasSent; }
	bool GetHasPreGameLoadoutSent() { return m_hasPreGamePackageSent; }

	bool CanCustomizePackage(i32 index);
	bool IsPackageLocked(i32 index);

	bool TokenUnlockPackage();
	void AddPackageToBeUnlocked(i32 index) { m_unlockPack = index; }
	i32 GetPackageToBeUnlocked() const { return m_unlockPack; }
	
	EUnlockType GetUnlockTypeForItem(i32 index) const;
	
	bool IsItemLocked(i32 index, bool bNeedUnlockDescriptions, SPPHaveUnlockedQuery* pResults=NULL) const;
	bool TokenUnlockItem();
	void AddItemToBeUnlocked(i32 index) { m_unlockItem = index; }
	i32 GetTokenUnlockItem() { return m_unlockItem; }

	void SetCurrentCategory(i32 category) { m_currentCategory = category; }
	void SetCurrentNth(i32 nth) { m_currentNth = nth; }

	// HostMigration
	void ClGetCurrentLoadoutParams(CGameRules::EquipmentLoadoutParams &params);

	void OnGameEnded();
	void OnRoundEnded();
	void OnGameReset();

	const TWeaponsVec &GetWeaponsList();

	void InvalidateLastSentLoadout();
	void ForceSelectLastSelectedLoadout();

	void Display3DItem(u8 itemNumber);

#if LIST_LOADOUT_CONTENTS_ON_SCREEN
	void ListLoadoutContentsOnScreen();
#endif

  tukk GetItemNameFromIndex(i32 idx);

	tukk GetLastSelectedWeaponName() const { return m_lastSelectedWeaponName.c_str(); }
	ILINE void InformNoLongerCustomizingWeapon()  { m_currentlyCustomizingWeapon = false;    }
	ILINE bool IsCustomizingWeapon() const        { return m_currentlyCustomizingWeapon;     }

	tukk GetPackageDisplayFromIndex(i32 index);
	tukk GetPackageDisplayFromName(tukk name);

	tukk GetPlayerModel(i32k index);
	EEquipmentLoadoutCategory GetSlotCategory(i32k slot);
	i32 GetCurrentSelectedPackageIndex();
	bool IsLocked(EUnlockType type, tukk name, SPPHaveUnlockedQuery &results) const;
	tukk GetCurrentPackageGroupName();

	void SetCurrentUICategory(EEquipmentLoadoutUICategories uiCategory) { m_curUICategory = uiCategory; }
	EEquipmentLoadoutUICategories GetCurrentUICategory() const { return m_curUICategory; }

	void SetUICustomiseAttachments(bool bCustomiseAttachments) { m_curUICustomiseAttachments = bCustomiseAttachments; }
	bool GetUICustomiseAttachments() const { return m_curUICustomiseAttachments; }

	void NewAttachmentInLoadout(const ItemString &newAttachmentName, i32 weapon);
	void DetachInLoadout(const ItemString &oldAttachmentName, i32 weapon);

	i32 IsWeaponInCurrentLoadout(const ItemString &weaponName);
	bool IsItemInCurrentLoadout(i32k index) const;

	void ApplyAttachmentOverrides(u8 * contents) const;

	tukk GetModelName(u8 modelIndex) const;
	u8 GetModelIndexOverride(u16 channelId) const;
	u8 AddModel(tukk pModelName);

	void UpdateClassicModeModel(u16 channelId, i32 teamId);

	void StreamFPGeometry( const EPrecacheGeometryEvent event, i32k package);
	void ReleaseStreamedFPGeometry( const EPrecacheGeometryEvent event );
	void Debug();

	void InitUnlockedAttachmentFlags();
	void SetCurrentAttachmentsToUnlocked();
	void UpdateWeaponAttachments( IEntityClass* pWeaponClass, IEntityClass* pAccessory );
	u32 GetWeaponAttachmentFlags( tukk pWeaponName );
	void SpawnedWithLoadout( u8 loadoutIdx );

public:

	typedef u8 TEquipmentPackageContents[EQUIPMENT_LOADOUT_NUM_SLOTS];
	typedef DrxFixedStringT<32> TFixedString;
	typedef DrxFixedStringT<64> TFixedString64;
	typedef DrxFixedArray<i32, MAX_DISPLAY_DEFAULT_ATTACHMENTS> TDefaultAttachmentsArr;
	typedef std::map<i32, SClassicModel> TClassicMap;

	// A local equipment package representation.
	struct SEquipmentPackage
	{
		SEquipmentPackage(tukk name, tukk displayName, tukk displayDescription)
			:	m_name(name), 
				m_displayName(displayName), 
				m_displayDescription(displayDescription), 
				m_flags(ELOF_NONE), 
				m_id(0),
				m_bUseIcon(false),
				m_modelIndex(MP_MODEL_INDEX_DEFAULT)
		{
			memset(&m_contents,0,sizeof(m_contents));
			memset(&m_defaultContents,0,sizeof(m_defaultContents));
		}

		const bool GetDisplayString(string& outDisplayStr, tukk szPostfix) const;
		u8k GetSlotItem(const EEquipmentLoadoutCategory slotCategory, i32k Nth, const bool bDefault=false) const;
		void GetAttachmentItems(i32 weaponNth, CEquipmentLoadout::TDefaultAttachmentsArr& attachmentsArr) const;

		TEquipmentPackageContents m_contents;
		TEquipmentPackageContents m_defaultContents;
		TFixedString m_name;
		TFixedString m_displayName;
		TFixedString64 m_displayDescription;
		u32 m_flags;
		i32 m_id;
		bool m_bUseIcon;
		u8 m_modelIndex;
	};

	// Loadout Item definition.
	struct SEquipmentItem
	{
		SEquipmentItem(tukk name)
			: m_name(name), m_category(eELC_NONE), m_uniqueIndex(0), m_parentItemId(0), m_subcategory(0), m_allowedPackageGroups(0),
			  m_hideWhenLocked(false), m_bShowNew(false), m_bShowAttachmentsNew(false), 
				m_bUseIcon(false),m_showSwitchWeapon(true)
		{
		}

		TFixedString m_name;
		TFixedString m_displayName;
		TFixedString m_displayTypeName;
		TFixedString m_description;

		EEquipmentLoadoutCategory m_category;
		u32 m_uniqueIndex;
		u32 m_parentItemId;	// If not zero this item is a skin as the id is it's base item id

		u8 m_subcategory;
		u8 m_allowedPackageGroups;

		bool m_hideWhenLocked;
		bool m_bShowNew;							// Show as new in the UI
		bool m_bShowAttachmentsNew;		// Show as having new attachments in the UI
		bool m_bUseIcon;							// Use icon instead of 3D model in the loadouts
		bool m_showSwitchWeapon;
	};

	struct SAttachmentInfo
	{
		SAttachmentInfo(i32 itemId)
			:	m_itemId(itemId), m_locked(false), m_bShowNew(false)
		{
		}

		i32 m_itemId;
		bool m_locked;
		bool m_bShowNew;
	};
	typedef DrxFixedArray<SAttachmentInfo, MAX_DISPLAY_ATTACHMENTS> TAttachmentsArr;

	typedef std::vector<SEquipmentItem> TEquipmentItems;
	typedef std::vector<SEquipmentPackage> TEquipmentPackages;

	struct SPackageGroup
	{
		SPackageGroup()
		{
			Reset();
		}

		void Reset()
		{
			m_packages.clear();
			m_selectedPackage = 0;
			m_profilePackage = 0;
			m_lastSentPackage = INVALID_PACKAGE;
			m_highlightedPackage = 0;
		}

		TEquipmentPackages m_packages;
		i32 m_selectedPackage;
		i32 m_profilePackage;
		i32 m_lastSentPackage;
		i32 m_highlightedPackage;
	};

	const CEquipmentLoadout::SPackageGroup& GetCurrentPackageGroup();
	const SEquipmentPackage* GetPackageFromId(i32k index);
	const TEquipmentItems& GetAllItems() { return m_allItems; }
	const SEquipmentItem* GetItem(i32k index);
	const SEquipmentItem* GetItemByName(tukk name) const;

	void GetWeaponSkinsForWeapon(i32 itemId, TAttachmentsArr& arrayAttachments);
	void GetAvailableAttachmentsForWeapon(i32 itemId, TAttachmentsArr* arrayAttachments, TDefaultAttachmentsArr* arrayDefaultAttachments, bool bNeedUnlockDescriptions = true);
	void GetListedAttachmentsForWeapon(i32 itemId, TAttachmentsArr* arrayAttachments );
	bool SetPackageNthWeaponAttachments(i32k packageId, u32 Nth, TDefaultAttachmentsArr& arrayAttachments);
	i32  GetItemIndexFromName(tukk name);

	void RandomChoiceModel ();

	void GoingBack(){m_bGoingBack=true;}

	const bool ClIsAttachmentIncluded(const SEquipmentItem* pAttachmentItem, tukk pWeaponItemName,const TEquipmentPackageContents& contents) const;


private:
	// A clients equipment package representation on the server.
	struct SClientEquipmentPackage
	{
		SClientEquipmentPackage()
			:	m_flags(ELOF_NONE), m_modelIndex(MP_MODEL_INDEX_DEFAULT), m_loadoutIdx(0)
		{
			memset(&m_contents,0,sizeof(m_contents));
			m_weaponAttachmentFlags = 0;
		}

		u32 m_flags;
		// Specify which attachments the player has access to. 
		u32 m_weaponAttachmentFlags;

		TEquipmentPackageContents m_contents;
		u8 m_modelIndex;
		u8 m_loadoutIdx;
	};

	struct SAttachmentOverride
	{
		u8 m_id;
		bool m_bInUse;

		SAttachmentOverride()
			: m_id(0)
			, m_bInUse(false)
		{
		}

		void Reset()
		{
			m_bInUse = false;
		}
	};

	typedef std::vector<SAttachmentOverride> TAttachmentsOverridesVector;
	typedef std::vector<TAttachmentsOverridesVector> TAttachmentsOverridesWeapons;

  void Reset();

	SEquipmentPackage* GetPackageFromIdInternal(i32k index);
	SEquipmentPackage* GetPackageFromName(tukk name);
	i32         GetPackageIndexFromName(EEquipmentPackageGroup group, tukk name);
	i32         GetPackageIndexFromId(EEquipmentPackageGroup group, i32 id);

	i32  CountOfCategoryUptoIndex(EEquipmentLoadoutCategory category, i32 Nth, i32 upToIndex);
	i32  GetSlotIndex(EEquipmentLoadoutCategory category, i32 Nth);

	void SetNthWeaponAttachments(u32 Nth, TDefaultAttachmentsArr& arrayAttachments);
	void SetDefaultAttachmentsForWeapon(i32 itemId, u32 Nth);
	void ApplyWeaponAttachment(const SEquipmentItem *pItem, const IItemSystem *pItemSystem, EntityId lastWeaponId);

	void LoadDefaultEquipmentPackages();

	void LoadSavedEquipmentPackages(EEquipmentPackageGroup group, IPlayerProfile* pProfile, bool online);

	void LoadItemDefinitions();
	void LoadItemDefinitionParams(XmlNodeRef itemXML, SEquipmentItem &item);
	void LoadItemDefinitionFromParent(tukk parentItemName, SEquipmentItem &item);

	bool SaveEquipmentPackage(IPlayerProfile* pProfile, EEquipmentPackageGroup group, i32 packIndex, i32 count);
	
	EMenuButtonFlags GetNewFlagForItemCategory(EEquipmentLoadoutCategory category, i32 subcategory);

	static bool CompareWeapons(const SLoadoutWeapon &elem1, const SLoadoutWeapon &elem2);

	void	   ClSetAttachmentInclusionFlags(const TEquipmentPackageContents &contents, u32& flags);
	const bool SvIsAttachmentIncluded(const SEquipmentItem* pAttachmentItem,u32k& inclusionFlags) const;
	void	   SvAssignWeaponAttachments(u8k itemId, const SClientEquipmentPackage& package, CActor * pActor, i32 playerId );
	const bool AttachmentIncludedInPackageContents(const SEquipmentItem* pAttachmentItem,  const TEquipmentPackageContents& contents) const;
	void	   SetAttachmentInclusionFlag(u32& inclusionFlags, i32k attachmentItemId);
	const bool IsAttachmentInclusionFlagSet(u32k& inclusionFlags, i32k attachmentItemId) const;
	u8 GetClassicModeModel(i32 itemId, SClassicModel::EModelType modelType) const;

	void ResetOverrides();

	typedef std::map<i32, SClientEquipmentPackage> TClientEquipmentPackages;
	typedef DrxFixedArray<TFixedString64, MP_MODEL_INDEX_DEFAULT> TModelNamesArray;
	typedef DrxFixedStringT<128> TStreamedGeometry[ePGE_Total];

	TEquipmentItems m_allItems;
	TWeaponsVec m_allWeapons;
	TModelNamesArray m_modelNames;

	u8 m_weaponsStartIndex;
	u8 m_attachmentsStartIndex;
	u8 m_slotCategories[EQUIPMENT_LOADOUT_NUM_SLOTS];

	// Local Client
	SPackageGroup m_packageGroups[eEPG_NumGroups];
	TClassicMap m_classicMap;
	TEquipmentPackageContents m_currentCustomizePackage;
	TFixedString m_lastSelectedWeaponName;
	EEquipmentPackageGroup m_currentPackageGroup;
	TAttachmentsOverridesWeapons m_attachmentsOverridesWeapon;
	TStreamedGeometry m_streamedGeometry;

	bool m_haveSortedWeapons;
	bool m_hasPreGamePackageSent;
	bool m_currentlyCustomizingWeapon;

	i32 m_unlockPack;
	i32 m_unlockItem;

	i32 m_currentCategory;
	i32 m_currentNth;

	// UI
	EEquipmentLoadoutUICategories m_curUICategory;
	bool m_curUICustomiseAttachments;

	bool m_currentCustomizeNameChanged;
	bool m_bShowCellModel;
	bool m_bGoingBack;

	TClientEquipmentPackages m_clientLoadouts;

	typedef VectorMap<u32, u32>	TWeaponAttachmentFlagMap;	//weapon class name crc to attach flags
	TWeaponAttachmentFlagMap	m_unlockedAttachments;
	TWeaponAttachmentFlagMap	m_currentAvailableAttachments;
};

#endif // ~__EQUIPMENT_LOADOUT_H__
