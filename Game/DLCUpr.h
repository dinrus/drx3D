// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Responsible for loading downloadable content into the game
and informing the game of what content is loaded.

-------------------------------------------------------------------------
История:
- 18:05:2010  : Created by Martin Sherburn
- 21:08:2012  : Maintained by Andrew Blackwell

*************************************************************************/

#ifndef __DLCMANAGER_H__
#define __DLCMANAGER_H__

class CMenuData;

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//reserve 8 flags for DLC and 8 flags for presale so we're really future proof
#define MAX_DLC_COUNT 16
#define MAX_DLC_ITEM_UNLOCKS 16

#if defined(DEDICATED_SERVER)
	#define DLC_LOAD_ON_CONSTRUCTION
#endif

#include <drx3D/Game/AutoEnum.h>
#include <drx3D/CoreX/Containers/DrxFixedArray.h>

#include <drx3D/Act/IPlayerProfiles.h>
#include <drx3D/Game/ProgressionUnlocks.h>

#include <drx3D/Game/UI/HUD/HUDUtils.h>

#define ENTITLEMENTS_AUTHORATIVE 0

#define NOTDLCLevelList(f) \
	f(eNOTDLC_c3mp_airport) \
	f(eNOTDLC_mpTestLevel) \
	f(eNOTDLC_c3mp_tanker) \
	
AUTOENUM_BUILDENUMWITHTYPE_WITHINVALID_WITHNUM(eNOTDLCLevelList, NOTDLCLevelList, eNOTDLC_INVALID_LEVEL, eNOTDLC_NUM_LEVELS);


#define NOTDLCGameModeList(f) \
	f(eNOTDLC_Assault) \
	f(eNOTDLC_CaptureTheFlag) \
	f(eNOTDLC_CrashSite) \
	f(eNOTDLC_Extraction) \
	f(eNOTDLC_InstantAction) \
	f(eNOTDLC_TeamInstantAction) \
	f(eNOTDLC_Gladiator) \
	f(eNOTDLC_Spears) \
	
AUTOENUM_BUILDENUMWITHTYPE_WITHINVALID_WITHNUM(eNOTDLCGameModeList, NOTDLCGameModeList, eNOTDLC_INVALID_GAMEMODE, eNOTDLC_NUM_GAMEMODES);


// Pre-Declarations
class CCheckPurchaseHistoryTask;

#define MAX_DLC_NAME 32
#define MAX_ROOT_LENGTH 32
#define MAX_TAG_LENGTH 32
#define MAX_DESC_LENGTH 32

struct DLCContents
{
	DLCContents(): bonusID( 0 ), uniqueID( 0 )	{ name [0 ] = '\0';  }
	std::vector<string>										levels;
	u32																bonusID;	//0 indicates no bonuses/pre-sale promotions included
	wchar_t																name[ MAX_DLC_NAME ];
	DrxFixedStringT<MAX_ROOT_LENGTH>			root;
	DrxFixedStringT<IDrxPak::g_nMaxPath>	scoreRewardsPath;
	DrxFixedStringT<IDrxPak::g_nMaxPath>	playlistsPath;
	DrxFixedStringT<MAX_TAG_LENGTH>				uniqueTag;
	DrxFixedStringT<MAX_DESC_LENGTH>			descriptionStr;
	u32																uniqueID;
	u32																messageID;
};

class CDLCUpr : IPlatformOS::IDLCListener, IPlatformOS::IPlatformListener, IGameWarningsListener
{
public:
	CDLCUpr();
	~CDLCUpr();

	void LoadDownloadableContent( u32 userIdOveride = INVALID_CONTROLLER_INDEX );
	void DisableDownloadableContent();
	void PatchMenu(CMenuData& menu);


	bool		IsDLCLoaded(i32 index) const { return ( (m_loadedDLCs & m_allowedDLCs) & BIT(index)) != 0; }
	u32	GetLoadedDLCs() const { return m_loadedDLCs & m_allowedDLCs; }

	u32 GetSquadCommonDLCs();
	u32 GetRequiredDLCsForLevel(tukk pLevelName);
	u32 GetRequiredDLCs();

	static bool MeetsDLCRequirements(u32 requiredDLCs, u32 loadedDLCs) { return (requiredDLCs & loadedDLCs) == requiredDLCs; }

	bool IsLevelStandard( tukk  levelname);	// Note: This function returns false for anything else than the shipped levels
	bool IsGameModeStandard( tukk  gamemode );	// Note: This function returns false for anything else than the shipped game modes

	i32 GetNamesStringOfPlayersMissingDLCsForLevel(tukk pLevelName, stack_string* pPlayersString);

	void ActivatePreSaleBonuses( bool showPopup, bool fromSuitReboot = false );

	tukk ScoreRewardsFilename( tukk pLevelName );

	void AddPlaylists();

	void OnlineAttributesRead();
	void SetAppliedEntitlements( u32 entitlements ) { m_appliedDLCStat = entitlements; }

	// IPlatformOS::IDLCListener interface implementation
	virtual void OnDLCMounted(const XmlNodeRef &rootNode, tukk sDLCRootFolder);
	virtual void OnDLCMountFailed(IPlatformOS::EDLCMountFail reason);
	virtual void OnDLCMountFinished(i32 nPacksFound );
	// IPlatformOS::IDLCListener interface implementation

	// IPlatformOS::IPlatformListener interface implementation
	virtual void OnPlatformEvent(const IPlatformOS::SPlatformEvent& event);
	// ~IPlatformOS::IPlatformListener interface implementation

	// IGameWarningsListener
	virtual bool OnWarningReturn(THUDWarningId id, tukk returnValue);
	virtual void OnWarningRemoved(THUDWarningId id) {}
	// ~IGameWarningsListener

	void OnDLCRemoved(tukk sDLCRootFolder);

	void ProcessDelayedWarnings();

	bool HaveUnlocked( EUnlockType type, tukk name, SPPHaveUnlockedQuery & results );

	void Update();

	u32 GetAppliedDLCBitfield() const { return m_appliedDLCStat; }
	u32 GetDLCXPToAward() const { return m_DLCXPToAward; }
	void ClearDLCXPToAward() { m_DLCXPToAward = 0; }

	bool IsPIIEntitlementSet() const;

private:
	bool VerifyCRCs(const XmlNodeRef &crcNode, tukk sDLCRootFolder);
	bool CheckLevels( i32 dlcId, tukk sDLCRootFolder );
	void PopulateDLCContents(const XmlNodeRef &rootNode, i32 dlcId, tukk name );
	void ClearDLCContents(i32 dlcId);
	bool LevelExists(tukk pLevelName);

	bool IsDLCReallyLoaded(i32 index) { return ( m_loadedDLCs & BIT(index)) != 0; }

	void RequestDLCWarning( tukk warningName, u32 priority, bool alwaysDelay = false );

	//keep private, external callers should continue to use GetRequiredDLCsForLevel
	i32 DlcIdForLevel( tukk pLevelName );

	void DoDLCUnlocks( XmlNodeRef unlocksXml, i32 dlcId );

	void AddNewDLCApplied(i32k index);
	bool IsNewDLC(i32k index) const;

	bool IsEntitlementSet( i32 entitlementIndex, bool presale ) const;
	i32 SetEntitlement( i32 entitlementIndex, bool presale );

private:
	DLCContents											m_dlcContents[MAX_DLC_COUNT];
	DrxFixedArray<SUnlock, MAX_DLC_ITEM_UNLOCKS >			m_itemUnlocks;
	DrxFixedArray<u32, MAX_DLC_ITEM_UNLOCKS >			m_itemUnlockDLCids;

	DrxFixedStringT<128>						m_prevLevelName;
	DrxFixedStringT<128>						m_dlcWarning;


	u32													m_loadedDLCs;
	u32													m_allowedDLCs;
	u32													m_entitledDLCs;

	u32													m_warningPriority;
	i32															m_requiredDLCs;
	bool														m_dlcLoaded;
	
	bool														m_allowedDLCUpToDate;
	bool														m_allowedDLCCheckFailed;
	bool														m_onlineAttributesRead;
	bool														m_bContentRemoved;
	bool														m_bContentAvailable;

	u32													m_appliedDLCStat;
	u32													m_DLCXPToAward;
	bool														m_bNewDLCAdded;
	bool														m_queueEventEntitlement;

	DrxLobbyTaskID									m_entitlementTask;
	

};

#endif // __DLCMANAGER_H__
