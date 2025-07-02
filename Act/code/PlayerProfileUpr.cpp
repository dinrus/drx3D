// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/PlayerProfileUpr.h>
#include <drx3D/Act/PlayerProfile.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/IActionMapUpr.h>
#include <drx3D/CoreX/Platform/IPlatformOS.h>
#include <drx3D/CoreX/DrxCrc32.h>
#include <drx3D/CoreX/Platform/DrxWindows.h>

#define SHARED_SAVEGAME_FOLDER            "%USER%/SaveGames"

#define ONLINE_ATTRIBUTES_DEFINITION_FILE "Scripts/Network/OnlineAttributes.xml"
#define ONLINE_VERSION_ATTRIBUTE_NAME     "OnlineAttributes/version"

tukk CPlayerProfileUpr::FACTORY_DEFAULT_NAME = "default";
tukk CPlayerProfileUpr::PLAYER_DEFAULT_PROFILE_NAME = "default";

namespace TESTING
{
void DumpProfiles(IPlayerProfileUpr* pFS, tukk userId)
{
	i32 nProfiles = pFS->GetProfileCount(userId);
	IPlayerProfileUpr::SProfileDescription desc;
	DrxLogAlways("User %s has %d profiles", userId, nProfiles);
	for (i32 i = 0; i < nProfiles; ++i)
	{
		pFS->GetProfileInfo(userId, i, desc);
		DrxLogAlways("#%d: '%s'", i, desc.name);
	}
}

void DumpAttrs(IPlayerProfile* pProfile)
{
	if (pProfile == 0)
		return;
	IAttributeEnumeratorPtr pEnum = pProfile->CreateAttributeEnumerator();
	IAttributeEnumerator::SAttributeDescription desc;
	DrxLogAlways("Attributes of profile %s", pProfile->GetName());
	i32 i = 0;
	TFlowInputData val;
	while (pEnum->Next(desc))
	{
		pProfile->GetAttribute(desc.name, val);
		string sVal;
		val.GetValueWithConversion(sVal);
		DrxLogAlways("Attr %d: %s=%s", i, desc.name, sVal.c_str());
		++i;
	}
}

void DumpSaveGames(IPlayerProfile* pProfile)
{
	ISaveGameEnumeratorPtr pSGE = pProfile->CreateSaveGameEnumerator();
	ISaveGameEnumerator::SGameDescription desc;
	DrxLogAlways("SaveGames for Profile '%s'", pProfile->GetName());
	char timeBuf[256];
	struct tm* timePtr;
	for (i32 i = 0; i < pSGE->GetCount(); ++i)
	{
		pSGE->GetDescription(i, desc);
		timePtr = localtime(&desc.metaData.saveTime);
		tukk timeString = timeBuf;
		if (strftime(timeBuf, sizeof(timeBuf), "%#c", timePtr) == 0)
			timeString = asctime(timePtr);
		DrxLogAlways("SaveGame %d/%d: name='%s' humanName='%s' desc='%s'", i, pSGE->GetCount() - 1, desc.name, desc.humanName, desc.description);
		DrxLogAlways("MetaData: level=%s gr=%s version=%d build=%s savetime=%s",
		             desc.metaData.levelName, desc.metaData.gameRules, desc.metaData.fileVersion, desc.metaData.buildVersion,
		             timeString);
	}
}

void DumpActionMap(IPlayerProfile* pProfile, tukk name)
{
	IActionMap* pMap = pProfile->GetActionMap(name);
	if (pMap)
	{
		i32 iAction = 0;
		IActionMapActionIteratorPtr pIter = pMap->CreateActionIterator();
		while (const IActionMapAction* pAction = pIter->Next())
		{
			DrxLogAlways("Action %d: '%s'", iAction++, pAction->GetActionId().c_str());

			i32 iNumInputData = pAction->GetNumActionInputs();
			for (i32 i = 0; i < iNumInputData; ++i)
			{
				const SActionInput* pActionInput = pAction->GetActionInput(i);
				DRX_ASSERT(pActionInput != NULL);
				DrxLogAlways("Key %d/%d: '%s'", i, iNumInputData - 1, pActionInput->input.c_str());
			}
		}
	}
}

void DumpMap(IConsoleCmdArgs* args)
{
	IActionMapUpr* pAM = CDrxAction::GetDrxAction()->GetIActionMapUpr();
	IActionMapIteratorPtr iter = pAM->CreateActionMapIterator();
	while (IActionMap* pMap = iter->Next())
	{
		DrxLogAlways("ActionMap: '%s' 0x%p", pMap->GetName(), pMap);
		i32 iAction = 0;
		IActionMapActionIteratorPtr pIter = pMap->CreateActionIterator();
		while (const IActionMapAction* pAction = pIter->Next())
		{
			DrxLogAlways("Action %d: '%s'", iAction++, pAction->GetActionId().c_str());

			i32 iNumInputData = pAction->GetNumActionInputs();
			for (i32 i = 0; i < iNumInputData; ++i)
			{
				const SActionInput* pActionInput = pAction->GetActionInput(i);
				DRX_ASSERT(pActionInput != NULL);
				DrxLogAlways("Key %d/%d: '%s'", i, iNumInputData - 1, pActionInput->input.c_str());
			}
		}
	}
}

void TestProfile(IConsoleCmdArgs* args)
{
	tukk userName = GetISystem()->GetUserName();
	IPlayerProfileUpr::EProfileOperationResult result;
	IPlayerProfileUpr* pFS = CDrxAction::GetDrxAction()->GetIPlayerProfileUpr();

	// test renaming current profile
#if 0
	pFS->RenameProfile(userName, "newOne4", result);
	return;
#endif

	bool bFirstTime = false;
	pFS->LoginUser(userName, bFirstTime);
	DumpProfiles(pFS, userName);
	pFS->DeleteProfile(userName, "PlayerCool", result);
	IPlayerProfile* pProfile = pFS->ActivateProfile(userName, "default");
	if (pProfile)
	{
		DumpActionMap(pProfile, "default");
		DumpAttrs(pProfile);
		pProfile->SetAttribute("hallo2", TFlowInputData(222));
		pProfile->SetAttribute("newAddedAttribute", TFlowInputData(24.10f));
		DumpAttrs(pProfile);
		pProfile->SetAttribute("newAddedAttribute", TFlowInputData(25.10f));
		DumpAttrs(pProfile);
		pProfile->ResetAttribute("newAddedAttribute");
		pProfile->ResetAttribute("hallo2");
		DumpAttrs(pProfile);
		DumpSaveGames(pProfile);
		float fVal;
		pProfile->GetAttribute("newAddedAttribute", fVal);
		pProfile->SetAttribute("newAddedAttribute", 2.22f);
	}
	else
	{
		DrxLogAlways("Can't activate profile 'default'");
	}

	const IPlayerProfile* pPreviewProfile = pFS->PreviewProfile(userName, "previewTest");
	if (pPreviewProfile)
	{
		float fVal;
		pPreviewProfile->GetAttribute("previewData", fVal);
		pPreviewProfile->GetAttribute("previewData2", fVal, true);
	}

	DumpProfiles(pFS, userName);
	// pFS->RenameProfile(userName, "new new profile");
	pFS->LogoutUser(userName);
}
}

i32 CPlayerProfileUpr::sUseRichSaveGames = 0;
i32 CPlayerProfileUpr::sRSFDebugWrite = 0;
i32 CPlayerProfileUpr::sRSFDebugWriteOnLoad = 0;
i32 CPlayerProfileUpr::sLoadOnlineAttributes = 1;

//------------------------------------------------------------------------
CPlayerProfileUpr::CPlayerProfileUpr(CPlayerProfileUpr::IPlatformImpl* pImpl)
	: m_curUserIndex(IPlatformOS::Unknown_User)
	, m_exclusiveControllerDeviceIndex(INVALID_CONTROLLER_INDEX) // start uninitialized
	, m_onlineDataCount(0)
	, m_onlineDataByteCount(0)
	, m_onlineAttributeAutoGeneratedVersion(0)
	, m_pImpl(pImpl)
	, m_pDefaultProfile(nullptr)
	, m_pReadingProfile(nullptr)
	, m_onlineAttributesListener(nullptr)
	, m_onlineAttributeDefinedVersion(0)
	, m_lobbyTaskId(DrxLobbyInvalidTaskID)
	, m_registered(false)
	, m_bInitialized(false)
	, m_enableOnlineAttributes(false)
	, m_allowedToProcessOnlineAttributes(true)
	, m_loadingProfile(false)
	, m_savingProfile(false)
{
	assert(m_pImpl != 0);

	// FIXME: TODO: temp stuff
	static bool testInit = false;
	if (testInit == false)
	{
		testInit = true;
		REGISTER_CVAR2("pp_RichSaveGames", &CPlayerProfileUpr::sUseRichSaveGames, 0, 0, "Enable RichSaveGame Format for SaveGames");
		REGISTER_CVAR2("pp_RSFDebugWrite", &CPlayerProfileUpr::sRSFDebugWrite, gEnv->pSystem->IsDevMode() ? 1 : 0, 0, "When RichSaveGames are enabled, save plain XML Data alongside for debugging");
		REGISTER_CVAR2("pp_RSFDebugWriteOnLoad", &CPlayerProfileUpr::sRSFDebugWriteOnLoad, 0, 0, "When RichSaveGames are enabled, save plain XML Data alongside for debugging when loading a savegame");
		REGISTER_CVAR2("pp_LoadOnlineAttributes", &CPlayerProfileUpr::sLoadOnlineAttributes, CPlayerProfileUpr::sLoadOnlineAttributes, VF_REQUIRE_APP_RESTART, "Load online attributes");

		REGISTER_COMMAND("test_profile", TESTING::TestProfile, VF_NULL, "");
		REGISTER_COMMAND("dump_action_maps", TESTING::DumpMap, VF_NULL, "Prints all action map bindings to console");

#if PROFILE_DEBUG_COMMANDS
		REGISTER_COMMAND("loadOnlineAttributes", &CPlayerProfileUpr::DbgLoadOnlineAttributes, VF_NULL, "Loads online attributes");
		REGISTER_COMMAND("saveOnlineAttributes", &CPlayerProfileUpr::DbgSaveOnlineAttributes, VF_NULL, "Saves online attributes");
		REGISTER_COMMAND("testOnlineAttributes", &CPlayerProfileUpr::DbgTestOnlineAttributes, VF_NULL, "Tests online attributes");

		REGISTER_COMMAND("loadProfile", &CPlayerProfileUpr::DbgLoadProfile, VF_NULL, "Load current user profile");
		REGISTER_COMMAND("saveProfile", &CPlayerProfileUpr::DbgSaveProfile, VF_NULL, "Save current user profile");
#endif
	}

	m_sharedSaveGameFolder = SHARED_SAVEGAME_FOLDER; // by default, use a shared savegame folder (Games For Windows Requirement)
	m_sharedSaveGameFolder.TrimRight("/\\");

	memset(m_onlineOnlyData, 0, sizeof(m_onlineOnlyData));
	memset(m_onlineAttributesState, IOnlineAttributesListener::eOAS_None, sizeof(m_onlineAttributesState));
#if PROFILE_DEBUG_COMMANDS
	m_testingPhase = 0;

	for (i32 i = 0; i < k_maxOnlineDataCount; ++i)
	{
		m_testFlowData[i] = 0;
	}
#endif
}

//------------------------------------------------------------------------
CPlayerProfileUpr::~CPlayerProfileUpr()
{
	// don't call virtual Shutdown or any other virtual function,
	// but delete things directly
	if (m_bInitialized)
	{
		GameWarning("[PlayerProfiles] CPlayerProfileUpr::~CPlayerProfileUpr Shutdown not called!");
	}
	std::for_each(m_userVec.begin(), m_userVec.end(), stl::container_object_deleter());
	SAFE_DELETE(m_pDefaultProfile);
	if (m_pImpl)
	{
		m_pImpl->Release();
		m_pImpl = 0;
	}

	IConsole* pC = ::gEnv->pConsole;
	pC->UnregisterVariable("pp_RichSaveGames", true);
	pC->RemoveCommand("test_profile");
	pC->RemoveCommand("dump_action_maps");
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::Initialize()
{
	LOADING_TIME_PROFILE_SECTION;
	if (m_bInitialized)
		return true;

	m_pImpl->Initialize(this);

	CPlayerProfile* pDefaultProfile = new CPlayerProfile(this, FACTORY_DEFAULT_NAME, "", false);
	bool ok = LoadProfile(0, pDefaultProfile, pDefaultProfile->GetName());
	if (!ok)
	{
		GameWarning("[PlayerProfiles] Cannot load factory default profile '%s'", FACTORY_DEFAULT_NAME);
		SAFE_DELETE(pDefaultProfile);
	}

	m_pDefaultProfile = pDefaultProfile;

	m_bInitialized = ok;
	return m_bInitialized;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::Shutdown()
{
	while (m_userVec.empty() == false)
	{
		SUserEntry* pEntry = m_userVec.back();
		LogoutUser(pEntry->userId);
	}
	SAFE_DELETE(m_pDefaultProfile);
	m_bInitialized = false;
	return true;
}

//------------------------------------------------------------------------
i32 CPlayerProfileUpr::GetUserCount()
{
	return m_userVec.size();
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::GetUserInfo(i32 index, IPlayerProfileUpr::SUserInfo& outInfo)
{
	if (index < 0 || index >= m_userVec.size())
	{
		assert(index >= 0 && index < m_userVec.size());
		return false;
	}

	SUserEntry* pEntry = m_userVec[index];
	outInfo.userId = pEntry->userId;
	return true;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::LoginUser(tukk userId, bool& bOutFirstTime)
{
	if (m_bInitialized == false)
		return false;

	bOutFirstTime = false;

	m_curUserID = userId;

	// user already logged in
	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry)
	{
		m_curUserIndex = pEntry->userIndex;
		return true;
	}

	SUserEntry* newEntry = new SUserEntry(userId);
	newEntry->pCurrentProfile = 0;
	newEntry->userIndex = m_userVec.size();
	// login the user and fill SUserEntry
	bool ok = m_pImpl->LoginUser(newEntry);

	if (!ok)
	{
		delete newEntry;
		GameWarning("[PlayerProfiles] Cannot login user '%s'", userId);
		return false;
	}

	// no entries yet -> create default profile by copying/saving factory default
	if (m_pDefaultProfile && newEntry->profileDesc.empty())
	{
		// save the default profile
		ok = m_pImpl->SaveProfile(newEntry, m_pDefaultProfile, PLAYER_DEFAULT_PROFILE_NAME);
		if (!ok)
		{
			GameWarning("[PlayerProfiles] Login of user '%s' failed because default profile '%s' cannot be created", userId, PLAYER_DEFAULT_PROFILE_NAME);

			delete newEntry;
			// TODO: maybe assign use factory default in this case, but then
			// cannot save anything incl. save-games
			return false;
		}
		DrxLog("[PlayerProfiles] Login of user '%s': First Time Login - Successfully created default profile '%s'", userId, PLAYER_DEFAULT_PROFILE_NAME);
		SLocalProfileInfo info(PLAYER_DEFAULT_PROFILE_NAME);
		time_t lastLoginTime;
		time(&lastLoginTime);
		info.SetLastLoginTime(lastLoginTime);
		newEntry->profileDesc.push_back(info);
		bOutFirstTime = true;
	}

	DrxLog("[PlayerProfiles] Login of user '%s' successful.", userId);
	if (bOutFirstTime == false)
	{
		DrxLog("[PlayerProfiles] Found %" PRISIZE_T " profiles.", newEntry->profileDesc.size());
		for (i32 i = 0; i < newEntry->profileDesc.size(); ++i)
			DrxLog("   Profile %d : '%s'", i, newEntry->profileDesc[i].GetName().c_str());
	}

	m_curUserIndex = newEntry->userIndex;
	m_userVec.push_back(newEntry);
	return true;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::LogoutUser(tukk userId)
{
	if (!m_bInitialized)
		return false;

	// check if user already logged in
	TUserVec::iterator iter;
	SUserEntry* pEntry = FindEntry(userId, iter);
	if (pEntry == 0)
	{
		GameWarning("[PlayerProfiles] Logout of user '%s' failed [was not logged in]", userId);
		return false;
	}

#if !DRX_PLATFORM_DURANGO // We don't want to auto save profile on durango, all settings are save to profile at the appropriate time
	// auto-save profile
	if (pEntry->pCurrentProfile && (gEnv && gEnv->pSystem && (!gEnv->pSystem->IsQuitting())))
	{
		EProfileOperationResult result;
		bool ok = SaveProfile(userId, result, ePR_Options);
		if (!ok)
		{
			GameWarning("[PlayerProfiles] Logout of user '%s': Couldn't save profile '%s'", userId, pEntry->pCurrentProfile->GetName());
		}
	}
#endif

	m_pImpl->LogoutUser(pEntry);

	if (m_curUserIndex == std::distance(m_userVec.begin(), iter))
	{
		m_curUserIndex = IPlatformOS::Unknown_User;
		m_curUserID.clear();
	}

	// delete entry from vector
	m_userVec.erase(iter);

	ClearOnlineAttributes();

	SAFE_DELETE(pEntry->pCurrentProfile);
	SAFE_DELETE(pEntry->pCurrentPreviewProfile);

	// delete entry
	delete pEntry;

	return true;
}

//------------------------------------------------------------------------
i32 CPlayerProfileUpr::GetProfileCount(tukk userId)
{
	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0)
		return 0;
	return pEntry->profileDesc.size();
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::GetProfileInfo(tukk userId, i32 index, IPlayerProfileUpr::SProfileDescription& outInfo)
{
	if (!m_bInitialized)
		return false;

	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0)
	{
		GameWarning("[PlayerProfiles] GetProfileInfo: User '%s' not logged in", userId);
		return false;
	}
	i32 count = pEntry->profileDesc.size();
	if (index >= count)
	{
		assert(index < count);
		return false;
	}
	SLocalProfileInfo& info = pEntry->profileDesc[index];
	outInfo.name = info.GetName().c_str();
	outInfo.lastLoginTime = info.GetLastLoginTime();
	return true;
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::SetProfileLastLoginTime(tukk userId, i32 index, time_t lastLoginTime)
{
	if (!m_bInitialized)
		return;

	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0)
	{
		GameWarning("[PlayerProfiles] GetProfileInfo: User '%s' not logged in", userId);
		return;
	}
	i32 count = pEntry->profileDesc.size();
	if (index >= count)
	{
		assert(index < count);
		return;
	}
	SLocalProfileInfo& info = pEntry->profileDesc[index];
	info.SetLastLoginTime(lastLoginTime);
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::CreateProfile(tukk userId, tukk profileName, bool bOverride, IPlayerProfileUpr::EProfileOperationResult& result)
{
	if (!m_bInitialized)
	{
		result = ePOR_NotInitialized;
		return false;
	}
	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0)
	{
		GameWarning("[PlayerProfiles] CreateProfile: User '%s' not logged in", userId);
		result = ePOR_UserNotLoggedIn;
		return false;
	}

	SLocalProfileInfo* pInfo = GetLocalProfileInfo(pEntry, profileName);
	if (pInfo != 0 && !bOverride) // profile with this name already exists
	{
		GameWarning("[PlayerProfiles] CreateProfile: User '%s' already has a profile with name '%s'", userId, profileName);
		result = ePOR_NameInUse;
		return false;
	}

	result = ePOR_Unknown;
	bool ok = true;
	if (pEntry->pCurrentProfile == 0)
	{
		// save the default profile
		ok = m_pImpl->SaveProfile(pEntry, m_pDefaultProfile, profileName, bOverride);
	}
	if (ok)
	{
		result = ePOR_Success;

		if (pInfo == 0) // if we override, it's already present
		{
			SLocalProfileInfo info(profileName);
			time_t lastPlayTime;
			time(&lastPlayTime);
			info.SetLastLoginTime(lastPlayTime);
			pEntry->profileDesc.push_back(info);

			// create default profile by copying/saving factory default
			ok = m_pImpl->SaveProfile(pEntry, m_pDefaultProfile, profileName);

			if (!ok)
			{
				pEntry->profileDesc.pop_back();
				result = ePOR_Unknown;
			}
		}
	}
	if (!ok)
	{
		GameWarning("[PlayerProfiles] CreateProfile: User '%s' cannot create profile '%s'", userId, profileName);
	}
	return ok;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::DeleteProfile(tukk userId, tukk profileName, IPlayerProfileUpr::EProfileOperationResult& result)
{
	if (!m_bInitialized)
	{
		result = ePOR_NotInitialized;
		return false;
	}
	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0)
	{
		GameWarning("[PlayerProfiles] DeleteProfile: User '%s' not logged in", userId);
		result = ePOR_UserNotLoggedIn;
		return false;
	}

	// cannot delete last profile
	if (pEntry->profileDesc.size() <= 1)
	{
		GameWarning("[PlayerProfiles] DeleteProfile: User '%s' cannot delete last profile", userId);
		result = ePOR_ProfileInUse;
		return false;
	}

	// make sure there is such a profile
	std::vector<SLocalProfileInfo>::iterator iter;
	SLocalProfileInfo* info = GetLocalProfileInfo(pEntry, profileName, iter);
	if (info)
	{
		bool ok = m_pImpl->DeleteProfile(pEntry, profileName);
		if (ok)
		{
			pEntry->profileDesc.erase(iter);
			// if the profile was the current profile, delete it
			if (pEntry->pCurrentProfile != 0 && stricmp(profileName, pEntry->pCurrentProfile->GetName()) == 0)
			{
				delete pEntry->pCurrentProfile;
				pEntry->pCurrentProfile = 0;
				// TODO: Maybe auto-select a profile
			}
			result = ePOR_Success;
			return true;
		}
		result = ePOR_Unknown;
	}
	else
		result = ePOR_NoSuchProfile;

	return false;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::RenameProfile(tukk userId, tukk newName, IPlayerProfileUpr::EProfileOperationResult& result)
{
	if (!m_bInitialized)
	{
		result = ePOR_NotInitialized;
		return false;
	}

	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0)
	{
		GameWarning("[PlayerProfiles] RenameProfile: User '%s' not logged in", userId);
		result = ePOR_UserNotLoggedIn;
		return false;
	}

	// make sure there is no such profile
	if (GetLocalProfileInfo(pEntry, newName) != 0)
	{
		result = ePOR_NoSuchProfile;
		return false;
	}

	// can only rename current active profile
	if (pEntry->pCurrentProfile == 0)
	{
		result = ePOR_NoActiveProfile;
		return false;
	}

	if (stricmp(pEntry->pCurrentProfile->GetName(), PLAYER_DEFAULT_PROFILE_NAME) == 0)
	{
		GameWarning("[PlayerProfiles] RenameProfile: User '%s' cannot rename default profile", userId);
		result = ePOR_DefaultProfile;
		return false;
	}

	result = ePOR_Unknown;
	bool ok = m_pImpl->RenameProfile(pEntry, newName);

	if (ok)
	{
		// assign a new name in the info DB
		SLocalProfileInfo* info = GetLocalProfileInfo(pEntry, pEntry->pCurrentProfile->GetName());
		info->SetName(newName);

		// assign a new name in the profile itself
		pEntry->pCurrentProfile->SetName(newName);

		result = ePOR_Success;
	}
	else
	{
		GameWarning("[PlayerProfiles] RenameProfile: Failed to rename profile to '%s' for user '%s' ", newName, userId);
	}
	return ok;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::SaveProfile(tukk userId, IPlayerProfileUpr::EProfileOperationResult& result, u32 reason)
{
	if (!m_bInitialized)
	{
		result = ePOR_NotInitialized;
		return false;
	}

	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0)
	{
		GameWarning("[PlayerProfiles] SaveProfile: User '%s' not logged in", userId);
		result = ePOR_UserNotLoggedIn;
		return false;
	}

	if (pEntry->pCurrentProfile == 0)
	{
		GameWarning("[PlayerProfiles] SaveProfile: User '%s' not logged in", userId);
		result = ePOR_NoActiveProfile;
		return false;
	}

	if (m_loadingProfile || m_savingProfile)
	{
		result = ePOR_LoadingProfile;

		return false;
	}
	else
	{
		m_savingProfile = true;
	}

	result = ePOR_Success;

	// ignore invalid file access for now since we do not support async save games yet - Feb. 2017
	SCOPED_ALLOW_FILE_ACCESS_FROM_THIS_THREAD();

	// notify game systems that the profile is about to be saved
	i32k listenerSize = m_listeners.size();
	for (i32 i = 0; i < listenerSize; i++)
	{
		m_listeners[i]->SaveToProfile(pEntry->pCurrentProfile, false, reason);
	}

	bool ok = m_pImpl->SaveProfile(pEntry, pEntry->pCurrentProfile, pEntry->pCurrentProfile->GetName());

	if (!ok)
	{
		GameWarning("[PlayerProfiles] SaveProfile: User '%s' cannot save profile '%s'", userId, pEntry->pCurrentProfile->GetName());
		result = ePOR_Unknown;
	}

	m_savingProfile = false;

	return ok;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::SaveInactiveProfile(tukk userId, tukk profileName, EProfileOperationResult& result, u32 reason)
{
	if (!m_bInitialized)
	{
		result = ePOR_NotInitialized;
		return false;
	}

	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0)
	{
		GameWarning("[PlayerProfiles] SaveProfile: User '%s' not logged in", userId);
		result = ePOR_UserNotLoggedIn;
		return false;
	}

	CPlayerProfile* pProfile = profileName ? new CPlayerProfile(this, profileName, userId, false) : 0;

	if (!pProfile || !LoadProfile(pEntry, pProfile, profileName, true))
	{
		result = ePOR_NoSuchProfile;
		return false;
	}

	result = ePOR_Success;

	// notify game systems that the profile is about to be saved
	i32k listenerSize = m_listeners.size();
	for (i32 i = 0; i < listenerSize; i++)
	{
		m_listeners[i]->SaveToProfile(pProfile, false, reason);
	}

	bool ok = m_pImpl->SaveProfile(pEntry, pProfile, pProfile->GetName());

	if (!ok)
	{
		GameWarning("[PlayerProfiles] SaveProfile: User '%s' cannot save profile '%s'", userId, pProfile->GetName());
		result = ePOR_Unknown;
	}

	delete pProfile;

	return ok;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::IsLoadingProfile() const
{
	return m_loadingProfile;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::IsSavingProfile() const
{
	return m_savingProfile;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::LoadProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name, bool bPreview /*false*/)
{
	if (m_loadingProfile || m_savingProfile)
	{
		return false;
	}
	else
	{
		m_loadingProfile = true;
	}

	bool ok = m_pImpl->LoadProfile(pEntry, pProfile, name);

	if (ok && !bPreview)
	{
		// notify game systems about the new profile data
		i32k listenerSize = m_listeners.size();
		for (i32 i = 0; i < listenerSize; i++)
		{
			m_listeners[i]->LoadFromProfile(pProfile, false, ePR_All);
		}
	}

	m_loadingProfile = false;

	return ok;
}

//------------------------------------------------------------------------
const IPlayerProfile* CPlayerProfileUpr::PreviewProfile(tukk userId, tukk profileName)
{
	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0) // no such user
	{
		GameWarning("[PlayerProfiles] ActivateProfile: User '%s' not logged in", userId);
		return 0;
	}

	// if this is the current profile, do nothing
	if (pEntry->pCurrentPreviewProfile != 0 && profileName && stricmp(profileName, pEntry->pCurrentPreviewProfile->GetName()) == 0)
	{
		return pEntry->pCurrentPreviewProfile;
	}

	if (pEntry->pCurrentPreviewProfile != 0)
	{
		delete pEntry->pCurrentPreviewProfile;
		pEntry->pCurrentPreviewProfile = 0;
	}

	if (profileName == 0 || *profileName == '\0')
		return 0;

	SLocalProfileInfo* info = GetLocalProfileInfo(pEntry, profileName);
	if (info == 0) // no such profile
	{
		GameWarning("[PlayerProfiles] PreviewProfile: User '%s' has no profile '%s'", userId, profileName);
		return 0;
	}

	pEntry->pCurrentPreviewProfile = new CPlayerProfile(this, profileName, userId, true);
	const bool ok = LoadProfile(pEntry, pEntry->pCurrentPreviewProfile, profileName, true);
	if (!ok)
	{
		GameWarning("[PlayerProfiles] PreviewProfile: User '%s' cannot load profile '%s'", userId, profileName);
		delete pEntry->pCurrentPreviewProfile;
		pEntry->pCurrentPreviewProfile = 0;
	}

	return pEntry->pCurrentPreviewProfile;
}

//------------------------------------------------------------------------
IPlayerProfile* CPlayerProfileUpr::ActivateProfile(tukk userId, tukk profileName)
{
	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0) // no such user
	{
		GameWarning("[PlayerProfiles] ActivateProfile: User '%s' not logged in", userId);
		return 0;
	}

	// if this is the current profile, do nothing
	if (pEntry->pCurrentProfile != 0 && stricmp(profileName, pEntry->pCurrentProfile->GetName()) == 0)
	{
		return pEntry->pCurrentProfile;
	}

	SLocalProfileInfo* info = GetLocalProfileInfo(pEntry, profileName);
	time_t lastPlayTime;
	time(&lastPlayTime);
	if (info == 0) // no such profile
	{
		GameWarning("[PlayerProfiles] ActivateProfile: User '%s' has no profile '%s'", userId, profileName);
		return 0;
	}
	info->SetLastLoginTime(lastPlayTime);
	CPlayerProfile* pNewProfile = new CPlayerProfile(this, profileName, userId, false);

	const bool ok = LoadProfile(pEntry, pNewProfile, profileName);
	if (ok)
	{
		delete pEntry->pCurrentProfile;
		pEntry->pCurrentProfile = pNewProfile;
	}
	else
	{
		GameWarning("[PlayerProfiles] ActivateProfile: User '%s' cannot load profile '%s'", userId, profileName);
		delete pNewProfile;
	}
	return pEntry->pCurrentProfile;
}

//------------------------------------------------------------------------
IPlayerProfile* CPlayerProfileUpr::GetCurrentProfile(tukk userId)
{
	SUserEntry* pEntry = FindEntry(userId);
	return pEntry ? pEntry->pCurrentProfile : 0;
}

//------------------------------------------------------------------------
tukk CPlayerProfileUpr::GetCurrentUser()
{
	return m_curUserID.c_str();
}

//------------------------------------------------------------------------
i32 CPlayerProfileUpr::GetCurrentUserIndex()
{
	return m_curUserIndex;
}

void CPlayerProfileUpr::SetExclusiveControllerDeviceIndex(u32 exclusiveDeviceIndex)
{
	m_exclusiveControllerDeviceIndex = exclusiveDeviceIndex;
}

u32 CPlayerProfileUpr::GetExclusiveControllerDeviceIndex() const
{
	return m_exclusiveControllerDeviceIndex;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::ResetProfile(tukk userId)
{
	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0) // no such user
	{
		GameWarning("[PlayerProfiles] ResetProfile: User '%s' not logged in", userId);
		return false;
	}

	if (pEntry->pCurrentProfile == 0)
		return false;

	pEntry->pCurrentProfile->Reset();
	return true;
}

//------------------------------------------------------------------------
IPlayerProfile* CPlayerProfileUpr::GetDefaultProfile()
{
	return m_pDefaultProfile;
}

//------------------------------------------------------------------------
CPlayerProfileUpr::SUserEntry*
CPlayerProfileUpr::FindEntry(tukk userId) const
{
	TUserVec::const_iterator iter = m_userVec.begin();
	while (iter != m_userVec.end())
	{
		const SUserEntry* pEntry = *iter;
		if (pEntry->userId.compare(userId) == 0)
		{
			return (const_cast<SUserEntry*>(pEntry));
		}
		++iter;
	}
	return 0;
}

//------------------------------------------------------------------------
CPlayerProfileUpr::SUserEntry*
CPlayerProfileUpr::FindEntry(tukk userId, CPlayerProfileUpr::TUserVec::iterator& outIter)
{
	TUserVec::iterator iter = m_userVec.begin();
	while (iter != m_userVec.end())
	{
		SUserEntry* pEntry = *iter;
		if (pEntry->userId.compare(userId) == 0)
		{
			outIter = iter;
			return pEntry;
		}
		++iter;
	}
	outIter = iter;
	return 0;
}

//------------------------------------------------------------------------
CPlayerProfileUpr::SLocalProfileInfo*
CPlayerProfileUpr::GetLocalProfileInfo(SUserEntry* pEntry, tukk profileName) const
{
	std::vector<SLocalProfileInfo>::iterator iter = pEntry->profileDesc.begin();
	while (iter != pEntry->profileDesc.end())
	{
		SLocalProfileInfo& info = *iter;
		if (info.compare(profileName) == 0)
		{
			return &(const_cast<SLocalProfileInfo&>(info));
		}
		++iter;
	}
	return 0;
}

//------------------------------------------------------------------------
CPlayerProfileUpr::SLocalProfileInfo*
CPlayerProfileUpr::GetLocalProfileInfo(SUserEntry* pEntry, tukk profileName, std::vector<SLocalProfileInfo>::iterator& outIter) const
{
	std::vector<SLocalProfileInfo>::iterator iter = pEntry->profileDesc.begin();
	while (iter != pEntry->profileDesc.end())
	{
		SLocalProfileInfo& info = *iter;
		if (info.compare(profileName) == 0)
		{
			outIter = iter;
			return &(const_cast<SLocalProfileInfo&>(info));
		}
		++iter;
	}
	outIter = iter;
	return 0;
}

struct CSaveGameThumbnail : public ISaveGameThumbnail
{
	CSaveGameThumbnail(const string& name) : m_nRefs(0), m_name(name)
	{
	}

	void AddRef()
	{
		++m_nRefs;
	}

	void Release()
	{
		if (0 == --m_nRefs)
			delete this;
	}

	void GetImageInfo(i32& width, i32& height, i32& depth)
	{
		width = m_image.width;
		height = m_image.height;
		depth = m_image.depth;
	}

	i32 GetWidth()
	{
		return m_image.width;
	}

	i32 GetHeight()
	{
		return m_image.height;
	}

	i32 GetDepth()
	{
		return m_image.depth;
	}

	u8k* GetImageData()
	{
		return m_image.data.begin();
	}

	tukk GetSaveGameName()
	{
		return m_name;
	}

	CPlayerProfileUpr::SThumbnail m_image;
	string                            m_name;
	i32                               m_nRefs;
};

// TODO: currently we don't cache thumbnails.
// every entry in CPlayerProfileUpr::TSaveGameInfoVec could store the thumbnail
// or we store ISaveGameThumbailPtr there.
// current access pattern (only one thumbnail at a time) doesn't call for optimization/caching
// but: maybe store also image format, so we don't need to swap RB
class CSaveGameEnumerator : public ISaveGameEnumerator
{
public:
	CSaveGameEnumerator(CPlayerProfileUpr::IPlatformImpl* pImpl, CPlayerProfileUpr::SUserEntry* pEntry) : m_nRefs(0), m_pImpl(pImpl), m_pEntry(pEntry)
	{
		assert(m_pImpl != 0);
		assert(m_pEntry != 0);
		pImpl->GetSaveGames(m_pEntry, m_saveGameInfoVec);
	}

	i32 GetCount()
	{
		return m_saveGameInfoVec.size();
	}

	bool GetDescription(i32 index, SGameDescription& desc)
	{
		if (index < 0 || index >= m_saveGameInfoVec.size())
			return false;

		CPlayerProfileUpr::SSaveGameInfo& info = m_saveGameInfoVec[index];
		info.CopyTo(desc);
		return true;
	}

	virtual ISaveGameThumbailPtr GetThumbnail(i32 index)
	{
		if (index >= 0 && index < m_saveGameInfoVec.size())
		{
			ISaveGameThumbailPtr pThumbnail = new CSaveGameThumbnail(m_saveGameInfoVec[index].name);
			CSaveGameThumbnail* pCThumbnail = static_cast<CSaveGameThumbnail*>(pThumbnail.get());
			if (m_pImpl->GetSaveGameThumbnail(m_pEntry, m_saveGameInfoVec[index].name, pCThumbnail->m_image))
			{
				return pThumbnail;
			}
		}
		return 0;
	}

	virtual ISaveGameThumbailPtr GetThumbnail(tukk saveGameName)
	{
		CPlayerProfileUpr::TSaveGameInfoVec::const_iterator iter = m_saveGameInfoVec.begin();
		CPlayerProfileUpr::TSaveGameInfoVec::const_iterator iterEnd = m_saveGameInfoVec.end();
		while (iter != iterEnd)
		{
			if (iter->name.compareNoCase(saveGameName) == 0)
			{
				ISaveGameThumbailPtr pThumbnail = new CSaveGameThumbnail(iter->name);
				CSaveGameThumbnail* pCThumbnail = static_cast<CSaveGameThumbnail*>(pThumbnail.get());
				if (m_pImpl->GetSaveGameThumbnail(m_pEntry, pCThumbnail->m_name, pCThumbnail->m_image))
				{
					return pThumbnail;
				}
				return 0;
			}
			++iter;
		}
		return 0;
	}

	void AddRef()
	{
		++m_nRefs;
	}

	void Release()
	{
		if (0 == --m_nRefs)
			delete this;
	}

private:
	i32 m_nRefs;
	CPlayerProfileUpr::IPlatformImpl*   m_pImpl;
	CPlayerProfileUpr::SUserEntry*      m_pEntry;
	CPlayerProfileUpr::TSaveGameInfoVec m_saveGameInfoVec;
};

//------------------------------------------------------------------------
ISaveGameEnumeratorPtr CPlayerProfileUpr::CreateSaveGameEnumerator(tukk userId, CPlayerProfile* pProfile)
{
	if (!m_bInitialized)
		return 0;

	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0) // no such user
	{
		GameWarning("[PlayerProfiles] CreateSaveGameEnumerator: User '%s' not logged in", userId);
		return 0;
	}

	if (pEntry->pCurrentProfile == 0 || pEntry->pCurrentProfile != pProfile)
		return 0;

	return new CSaveGameEnumerator(m_pImpl, pEntry);
}

//------------------------------------------------------------------------
ISaveGame* CPlayerProfileUpr::CreateSaveGame(tukk userId, CPlayerProfile* pProfile)
{
	if (!m_bInitialized)
		return 0;

	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0) // no such user
	{
		GameWarning("[PlayerProfiles] CreateSaveGame: User '%s' not logged in", userId);
		return 0;
	}

	if (pEntry->pCurrentProfile == 0 || pEntry->pCurrentProfile != pProfile)
		return 0;

	return m_pImpl->CreateSaveGame(pEntry);
}

//------------------------------------------------------------------------
ILoadGame* CPlayerProfileUpr::CreateLoadGame(tukk userId, CPlayerProfile* pProfile)
{
	if (!m_bInitialized)
		return 0;

	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0) // no such user
	{
		GameWarning("[PlayerProfiles] CreateLoadGame: User '%s' not logged in", userId);
		return 0;
	}

	if (pEntry->pCurrentProfile == 0 || pEntry->pCurrentProfile != pProfile)
		return 0;

	return m_pImpl->CreateLoadGame(pEntry);
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::DeleteSaveGame(tukk userId, CPlayerProfile* pProfile, tukk name)
{
	if (!m_bInitialized)
		return false;

	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0) // no such user
	{
		GameWarning("[PlayerProfiles] DeleteSaveGame: User '%s' not logged in", userId);
		return false;
	}

	if (pEntry->pCurrentProfile == 0 || pEntry->pCurrentProfile != pProfile)
		return false;

	return m_pImpl->DeleteSaveGame(pEntry, name);
}

//------------------------------------------------------------------------
ILevelRotationFile* CPlayerProfileUpr::GetLevelRotationFile(tukk userId, CPlayerProfile* pProfile, tukk name)
{
	if (!m_bInitialized)
		return 0;

	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0) // no such user
	{
		GameWarning("[PlayerProfiles] GetLevelRotationFile: User '%s' not logged in", userId);
		return NULL;
	}

	if (pEntry->pCurrentProfile == 0 || pEntry->pCurrentProfile != pProfile)
		return NULL;

	return m_pImpl->GetLevelRotationFile(pEntry, name);
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::ResolveAttributeBlock(tukk userId, tukk attrBlockName, IResolveAttributeBlockListener* pListener, i32 reason)
{
	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0) // no such user
	{
		GameWarning("[PlayerProfiles] ResolveAttributeBlock: User '%s' not logged in", userId);
		if (pListener)
		{
			pListener->OnFailure();
		}

		return false;
	}

	if (pEntry->pCurrentProfile == NULL)
	{
		if (pListener)
		{
			pListener->OnFailure();
		}

		return false;
	}

	return m_pImpl->ResolveAttributeBlock(pEntry, pEntry->pCurrentProfile, attrBlockName, pListener, reason);
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::ResolveAttributeBlock(tukk userId, const SResolveAttributeRequest& attrBlockNameRequest, IResolveAttributeBlockListener* pListener, i32 reason)
{
	SUserEntry* pEntry = FindEntry(userId);
	if (pEntry == 0) // no such user
	{
		GameWarning("[PlayerProfiles] ResolveAttributeBlock: User '%s' not logged in", userId);
		if (pListener)
		{
			pListener->OnFailure();
		}

		return false;
	}

	if (pEntry->pCurrentProfile == NULL)
	{
		GameWarning("[PlayerProfiles] ResolveAttributeBlock: User '%s' doesn't have a profile", userId);
		if (pListener)
		{
			pListener->OnFailure();
		}

		return false;
	}

	return m_pImpl->ResolveAttributeBlock(pEntry, pEntry->pCurrentProfile, attrBlockNameRequest, pListener, reason);
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::SetSharedSaveGameFolder(tukk sharedSaveGameFolder)
{
	m_sharedSaveGameFolder = sharedSaveGameFolder;
	m_sharedSaveGameFolder.TrimRight("/\\");
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::AddListener(IPlayerProfileListener* pListener, bool updateNow)
{
	m_listeners.reserve(8);
	stl::push_back_unique(m_listeners, pListener);

	if (updateNow)
	{
		// allow new listener to init from the current profile
		if (IPlayerProfile* pCurrentProfile = GetCurrentProfile(GetCurrentUser()))
		{
			pListener->LoadFromProfile(pCurrentProfile, false, ePR_All);
		}
	}
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::RemoveListener(IPlayerProfileListener* pListener)
{
	stl::find_and_erase(m_listeners, pListener);
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::AddOnlineAttributesListener(IOnlineAttributesListener* pListener)
{
	DRX_ASSERT_MESSAGE(m_onlineAttributesListener == NULL, "PlayerProfileUpr only handles a single OnlineAttributes Listener");
	m_onlineAttributesListener = pListener;
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::RemoveOnlineAttributesListener(IOnlineAttributesListener* pListener)
{
	DRX_ASSERT_MESSAGE(m_onlineAttributesListener == pListener, "Can't remove listener that hasn't been added!");
	if (m_onlineAttributesListener == pListener)
	{
		m_onlineAttributesListener = NULL;
	}
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::SetOnlineAttributesState(const IOnlineAttributesListener::EEvent event, const IOnlineAttributesListener::EState newState)
{
	if (m_onlineAttributesState[event] != newState)
	{
		m_onlineAttributesState[event] = newState;
		SendOnlineAttributeState(event, newState);
	}
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::SendOnlineAttributeState(const IOnlineAttributesListener::EEvent event, const IOnlineAttributesListener::EState newState)
{
	if (m_onlineAttributesListener)
	{
		m_onlineAttributesListener->OnlineAttributeState(event, newState);
	}
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::GetOnlineAttributesState(const IOnlineAttributesListener::EEvent event, IOnlineAttributesListener::EState& state) const
{
	DRX_ASSERT(event > IOnlineAttributesListener::eOAE_Invalid && event < IOnlineAttributesListener::eOAE_Max);
	state = m_onlineAttributesState[event];
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::EnableOnlineAttributes(bool enable)
{
	m_enableOnlineAttributes = enable;
}

//------------------------------------------------------------------------
// if saving and loading online is enabled
bool CPlayerProfileUpr::HasEnabledOnlineAttributes() const
{
	return m_enableOnlineAttributes;
}

//------------------------------------------------------------------------
// if saving and loading online is possible
bool CPlayerProfileUpr::CanProcessOnlineAttributes() const
{
	return m_allowedToProcessOnlineAttributes;
}

//------------------------------------------------------------------------
// whether the game is in a state that it can save/load online attributes - e.g. when in a mp game session is in progress saving online is disabled
void CPlayerProfileUpr::SetCanProcessOnlineAttributes(bool enable)
{
	m_allowedToProcessOnlineAttributes = enable;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::IsOnlineOnlyAttribute(tukk name)
{
	if (m_enableOnlineAttributes && m_registered)
	{
		TOnlineAttributeMap::const_iterator end = m_onlineAttributeMap.end();

		TOnlineAttributeMap::const_iterator attr = m_onlineAttributeMap.find(name);
		if (attr != end)
		{
			u32 index = attr->second;
			DRX_ASSERT(index >= 0 && index <= m_onlineDataCount);
			return m_onlineOnlyData[index];
		}
	}

	return false;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::RegisterOnlineAttributes()
{
	LOADING_TIME_PROFILE_SECTION;
	EDrxLobbyError error = eCLE_ServiceNotSupported;
	IDrxStats* stats = gEnv->pLobby ? gEnv->pLobby->GetStats() : nullptr;

	IOnlineAttributesListener::EState currentState = IOnlineAttributesListener::eOAS_None;
	GetOnlineAttributesState(IOnlineAttributesListener::eOAE_Register, currentState);

	if (stats && !m_registered)
	{
		if (currentState != IOnlineAttributesListener::eOAS_Processing)
		{
			XmlNodeRef xmlData = GetISystem()->LoadXmlFromFile(ONLINE_ATTRIBUTES_DEFINITION_FILE);
			if (xmlData)
			{
				xmlData->getAttr("version", m_onlineAttributeDefinedVersion);

				CCrc32 crc;

				i32k attributeCount = xmlData->getChildCount();

				m_onlineAttributeMap.reserve(2 + attributeCount);

				{
					SDrxLobbyUserData defaultData;
					defaultData.m_type = eCLUDT_Int32;
					defaultData.m_id = m_onlineDataCount;
					RegisterOnlineAttribute("checksum0", "i32", true, defaultData, crc);
					defaultData.m_id = m_onlineDataCount;
					RegisterOnlineAttribute("checksum1", "i32", true, defaultData, crc);
					DRX_ASSERT(0 == GetOnlineAttributeIndexByName("checksum0"));
					DRX_ASSERT(1 == GetOnlineAttributeIndexByName("checksum1"));
					DRX_ASSERT(m_onlineDataCount == k_onlineChecksums);
				}

				for (i32 i = 0; i < attributeCount; i++)
				{
					XmlNodeRef attributeData = xmlData->getChild(i);
					tukk name = attributeData->getAttr("name");
					tukk type = attributeData->getAttr("type");
					bool onlineOnly = true;
					attributeData->getAttr("onlineOnly", onlineOnly);

					SDrxLobbyUserData defaultValue;
					GetDefaultValue(type, attributeData, &defaultValue);

					if (!RegisterOnlineAttribute(name, type, onlineOnly, defaultValue, crc))
					{
						DRX_ASSERT_TRACE(false, ("Fail to register attribute %s - not enough space DataSlots %d/%d and Bytes %d/%d", name, m_onlineDataCount, k_maxOnlineDataCount, m_onlineDataByteCount, k_maxOnlineDataBytes));
						SetOnlineAttributesState(IOnlineAttributesListener::eOAE_Register, IOnlineAttributesListener::eOAS_Failed);
						return false;
					}
				}

				DrxLog("RegisterOnlineAttributes '%u' values in '%u' bytes", m_onlineDataCount, m_onlineDataByteCount);
				error = stats->StatsRegisterUserData(m_onlineData, m_onlineDataCount, NULL, CPlayerProfileUpr::RegisterUserDataCallback, this);
				DRX_ASSERT(error == eCLE_Success);

				m_onlineAttributeAutoGeneratedVersion = crc.Get();
			}
			else
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Unable to load '%s'", ONLINE_ATTRIBUTES_DEFINITION_FILE);
			}
		}
		else
		{
			error = eCLE_Success;
		}
	}
	const bool success = (error == eCLE_Success);
	if (success)
	{
		SetOnlineAttributesState(IOnlineAttributesListener::eOAE_Register, IOnlineAttributesListener::eOAS_Processing);
	}
	else
	{
		SetOnlineAttributesState(IOnlineAttributesListener::eOAE_Register, IOnlineAttributesListener::eOAS_Failed);
	}
	return success;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::RegisterOnlineAttribute(tukk name, tukk type, const bool onlineOnly, const SDrxLobbyUserData& defaultValue, CCrc32& crc)
{
	m_onlineOnlyData[m_onlineDataCount] = onlineOnly;

	if (SetUserDataType(&m_onlineData[m_onlineDataCount], type))
	{
		m_defaultOnlineData[m_onlineDataCount] = defaultValue;
		m_onlineDataByteCount += UserDataSize(&m_onlineData[m_onlineDataCount]);
		if (m_onlineDataCount >= k_maxOnlineDataCount || m_onlineDataByteCount >= k_maxOnlineDataBytes)
		{
			return false;  // fail because it doesn't fit
		}
#if USE_STEAM // on steam we look up by name not by index
		m_onlineData[m_onlineDataCount].m_id = string(name);
#endif
		m_onlineAttributeMap[name] = m_onlineDataCount;
		m_onlineDataCount++;
		crc.Add(name);
	}

	return true;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::SetUserDataType(SDrxLobbyUserData* data, tukk type)
{
	if (strcmpi(type, "i32") == 0)
	{
		data->m_type = eCLUDT_Int32;
		data->m_int32 = 0;
	}
	else if (strcmpi(type, "i16") == 0)
	{
		data->m_type = eCLUDT_Int16;
		data->m_int16 = 0;
	}
	else if (strcmpi(type, "int8") == 0)
	{
		data->m_type = eCLUDT_Int8;
		data->m_int8 = 0;
	}
	else if (strcmpi(type, "float") == 0)
	{
		data->m_type = eCLUDT_Float32;
		data->m_f32 = 0.0f;
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Unrecognized UserDataType - '%s'", type);
		return false;
	}

	data->m_id = m_onlineDataCount;
	return true;
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::GetDefaultValue(tukk type, XmlNodeRef attributeNode, SDrxLobbyUserData* pOutData)
{
	if (strcmpi(type, "i32") == 0)
	{
		pOutData->m_type = eCLUDT_Int32;
		pOutData->m_int32 = 0;
		attributeNode->getAttr("defaultValue", pOutData->m_int32);
	}
	else if (strcmpi(type, "i16") == 0)
	{
		pOutData->m_type = eCLUDT_Int16;
		pOutData->m_int16 = 0;
		i32 iValue = 0;
		attributeNode->getAttr("defaultValue", iValue);
		pOutData->m_int16 = (i16) iValue;
	}
	else if (strcmpi(type, "int8") == 0)
	{
		pOutData->m_type = eCLUDT_Int8;
		pOutData->m_int8 = 0;
		i32 iValue = 0;
		attributeNode->getAttr("defaultValue", iValue);
		pOutData->m_int8 = (int8) iValue;
	}
	else if (strcmpi(type, "float") == 0)
	{
		pOutData->m_type = eCLUDT_Float32;
		pOutData->m_f32 = 0.0f;
		attributeNode->getAttr("defaultValue", pOutData->m_f32);
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Unrecognized UserDataType - '%s'", type);
	}

	pOutData->m_id = m_onlineDataCount;
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::LoadOnlineAttributes(IPlayerProfile* pProfile)
{
	bool success = false;

	DRX_ASSERT(m_allowedToProcessOnlineAttributes);

	SetOnlineAttributesState(IOnlineAttributesListener::eOAE_Load, IOnlineAttributesListener::eOAS_Processing);

	if (m_enableOnlineAttributes)
	{
		if (m_registered && m_pReadingProfile == NULL)
		{
			IDrxStats* pStats = gEnv->pLobby ? gEnv->pLobby->GetStats() : nullptr;
			if (pStats)
			{
				m_pReadingProfile = pProfile;
				EDrxLobbyError error = pStats->StatsReadUserData(GetExclusiveControllerDeviceIndex(), &m_lobbyTaskId, CPlayerProfileUpr::ReadUserDataCallback, this);
				success = (error == eCLE_Success);
			}
#if !defined(_RELEASE) && !defined(DEDICATED_SERVER) && !defined(PURE_CLIENT)
			else
			{
				DrxLog("[profile] no stats module, defaulting to success");
				success = true;
				SetOnlineAttributesState(IOnlineAttributesListener::eOAE_Load, IOnlineAttributesListener::eOAS_Success);
			}
#endif
		}
	}

	if (!success)
	{
		SetOnlineAttributesState(IOnlineAttributesListener::eOAE_Load, IOnlineAttributesListener::eOAS_Failed);
		m_pReadingProfile = NULL;
		m_lobbyTaskId = DrxLobbyInvalidTaskID;
	}
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::SaveOnlineAttributes(IPlayerProfile* pProfile)
{
	DRX_ASSERT(m_allowedToProcessOnlineAttributes);

	if (m_enableOnlineAttributes)
	{
		if (m_registered)
		{
			TFlowInputData versionData(GetOnlineAttributesVersion());
			pProfile->SetAttribute(ONLINE_VERSION_ATTRIBUTE_NAME, versionData);

			i32k listenerSize = m_listeners.size();
			for (i32 i = 0; i < listenerSize; i++)
			{
				m_listeners[i]->SaveToProfile(pProfile, true, ePR_All); // TODO: perhaps this wants to only be ePR_Options, or needs an extra check?
			}

			//update online attributes from current profile
			TOnlineAttributeMap::const_iterator end = m_onlineAttributeMap.end();
			TOnlineAttributeMap::const_iterator iter = m_onlineAttributeMap.begin();
			while (iter != end)
			{
				if (iter->second >= k_onlineChecksums)
				{
					TFlowInputData data;
					bool hasAttr = pProfile->GetAttribute(iter->first.c_str(), data);
					DRX_ASSERT_MESSAGE(hasAttr, ("Expected %s to be set by SavingOnlineAttributes but wasn't", iter->first.c_str()));
					SetUserData(&m_onlineData[iter->second], data);
				}

				++iter;
			}

			ApplyChecksums(m_onlineData, m_onlineDataCount);

			//upload results
			IDrxStats* pStats = gEnv->pLobby ? gEnv->pLobby->GetStats() : nullptr;
			if (pStats && pStats->GetLeaderboardType() == eCLLT_P2P)
			{
				EDrxLobbyError error = pStats->StatsWriteUserData(GetExclusiveControllerDeviceIndex(), m_onlineData, m_onlineDataCount, NULL, CPlayerProfileUpr::WriteUserDataCallback, this);
				DRX_ASSERT(error == eCLE_Success);
			}
		}
	}
}

void CPlayerProfileUpr::ClearOnlineAttributes()
{
	// network cleanup
	if (m_lobbyTaskId != DrxLobbyInvalidTaskID)
	{
		IDrxStats* pStats = gEnv->pLobby ? gEnv->pLobby->GetStats() : nullptr;
		if (pStats)
		{
			pStats->CancelTask(m_lobbyTaskId);
		}

		m_lobbyTaskId = DrxLobbyInvalidTaskID;
	}

	m_pReadingProfile = NULL;
	m_onlineAttributesState[IOnlineAttributesListener::eOAE_Load] = IOnlineAttributesListener::eOAS_None;
}

void CPlayerProfileUpr::LoadGamerProfileDefaults(IPlayerProfile* pProfile)
{
	CPlayerProfile* pDefaultProfile = m_pDefaultProfile;
	m_pDefaultProfile = NULL;
	pProfile->LoadGamerProfileDefaults();
	m_pDefaultProfile = pDefaultProfile;
}

void CPlayerProfileUpr::ReloadProfile(IPlayerProfile* pProfile, u32 reason)
{
	TListeners::const_iterator it = m_listeners.begin(), itend = m_listeners.end();
	for (; it != itend; ++it)
	{
		(*it)->LoadFromProfile(pProfile, false, reason);
	}
}

void CPlayerProfileUpr::SetOnlineAttributes(IPlayerProfile* pProfile, const SDrxLobbyUserData* pData, i32k onlineDataCount)
{
	DRX_ASSERT(pProfile);

	if (pProfile)
	{
		ReadOnlineAttributes(pProfile, pData, onlineDataCount);
	}
}

//------------------------------------------------------------------------
u32 CPlayerProfileUpr::GetOnlineAttributes(SDrxLobbyUserData* pData, u32 numData)
{
	u32 numCopied = 0;

	if (pData && numData > 0)
	{
		numCopied = (numData > m_onlineDataCount) ? m_onlineDataCount : numData;
		memcpy(pData, m_onlineData, (sizeof(SDrxLobbyUserData) * numCopied));
	}

	return numCopied;
}

//------------------------------------------------------------------------
u32 CPlayerProfileUpr::GetDefaultOnlineAttributes(SDrxLobbyUserData* pData, u32 numData)
{
	u32 numCopied = 0;

	if (pData && numData > 0)
	{
		numCopied = (numData > m_onlineDataCount) ? m_onlineDataCount : numData;
		memcpy(pData, m_defaultOnlineData, (sizeof(SDrxLobbyUserData) * numCopied));
	}

	return numCopied;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::SetUserData(SDrxLobbyUserData* data, const TFlowInputData& value)
{
	switch (data->m_type)
	{
	case eCLUDT_Int8:
	case eCLUDT_Int16:
	case eCLUDT_Int32:
		{
			i32k* pInt = NULL;
			i32 stringValue = 0;
			if (const string* pString = value.GetPtr<string>())
			{
				stringValue = atoi(pString->c_str());
				pInt = &stringValue;
			}
			else
			{
				pInt = value.GetPtr<i32>();
			}

			if (pInt)
			{
				if (data->m_type == eCLUDT_Int32)
				{
					data->m_int32 = (*pInt);
				}
				else if (data->m_type == eCLUDT_Int16)
				{
					data->m_int16 = (*pInt);
				}
				else if (data->m_type == eCLUDT_Int8)
				{
					data->m_int8 = (*pInt);
				}
				else
				{
					DRX_ASSERT_MESSAGE(false, "TFlowInputData i32 didn't match UserData types");
					return false;
				}
				return true;
			}
			//This can be removed when DinrusXNetwork adds support for bools in user data
			else if (const bool* pBool = value.GetPtr<bool>())
			{
				if (data->m_type == eCLUDT_Int8)
				{
					data->m_int8 = (*pBool);
				}
				else
				{
					DRX_ASSERT_MESSAGE(false, "TFlowInputData i32 didn't match UserData types");
					return false;
				}
				return true;
			}
			else
			{
				DRX_ASSERT_MESSAGE(false, "TFlowInputData didn't contain expected i32/bool");
				return false;
			}
		}
		break;
	case eCLUDT_Float32:
		{
			const float* pFloat = NULL;
			float stringValue = 0.0f;
			if (const string* pString = value.GetPtr<string>())
			{
				stringValue = (float) atof(pString->c_str());
				pFloat = &stringValue;
			}
			else
			{
				pFloat = value.GetPtr<float>();
			}

			if (pFloat)
			{
				data->m_f32 = (*pFloat);
				return true;
			}
			else
			{
				DRX_ASSERT_MESSAGE(false, "TFlowInputData didn't contain expected float");
				return false;
			}
		}
		break;
	}

	DRX_ASSERT_MESSAGE(false, "Unable to store data size");
	return false;
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::ReadUserData(const SDrxLobbyUserData* data, TFlowInputData& val)
{
	switch (data->m_type)
	{
	case eCLUDT_Int32:
		{
			val.Set(data->m_int32);
			return true;
		}
		break;
	case eCLUDT_Int16:
		{
			val.Set((i32) data->m_int16);
			return true;
		}
		break;
	case eCLUDT_Int8:
		{
			val.Set((i32) data->m_int8);
			return true;
		}
		break;
	case eCLUDT_Float32:
		{
			val.Set(data->m_f32);
			return true;
		}
		break;
	}

	DRX_ASSERT_MESSAGE(false, "Unable to read data size");
	return false;
}

//------------------------------------------------------------------------
u32 CPlayerProfileUpr::UserDataSize(const SDrxLobbyUserData* data)
{
	switch (data->m_type)
	{
	case eCLUDT_Int32:
		{
			return sizeof(data->m_int32);
		}
	case eCLUDT_Int16:
		{
			return sizeof(data->m_int16);
		}
	case eCLUDT_Int8:
		{
			return sizeof(data->m_int8);
		}
	case eCLUDT_Float32:
		{
			return sizeof(data->m_f32);
		}
	}

	DRX_ASSERT_MESSAGE(0, "Unsupported LobbyUserData type");
	return 0;
}

//static------------------------------------------------------------------------
void CPlayerProfileUpr::ReadUserDataCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, SDrxLobbyUserData* pData, u32 numData, uk pArg)
{
	CPlayerProfileUpr* pUpr = static_cast<CPlayerProfileUpr*>(pArg);
	DRX_ASSERT(pUpr);

	if (error == eCLE_Success && pUpr->m_pReadingProfile)
	{
		pUpr->ReadOnlineAttributes(pUpr->m_pReadingProfile, pData, numData);
		pUpr->SetOnlineAttributesState(IOnlineAttributesListener::eOAE_Load, IOnlineAttributesListener::eOAS_Success);

#if PROFILE_DEBUG_COMMANDS
		if (pUpr->m_testingPhase == 2)
		{
			DbgTestOnlineAttributes(NULL);
		}
#endif
	}
	else if ((error == eCLE_ReadDataNotWritten) || (error == eCLE_ReadDataCorrupt))
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_COMMENT, "OnlineAttributes - ReadDataNotWritten");
		if (pUpr->m_pReadingProfile)
		{
			TOnlineAttributeMap::const_iterator iter = pUpr->m_onlineAttributeMap.begin();
			TOnlineAttributeMap::const_iterator end = pUpr->m_onlineAttributeMap.end();
			TFlowInputData blankData;
			while (iter != end)
			{
				pUpr->m_pReadingProfile->DeleteAttribute(iter->first);
				++iter;
			}

			pUpr->ReloadProfile(pUpr->m_pReadingProfile, ePR_All);
		}
		pUpr->SetOnlineAttributesState(IOnlineAttributesListener::eOAE_Load, IOnlineAttributesListener::eOAS_Success);
	}
	else
	{
		pUpr->SetOnlineAttributesState(IOnlineAttributesListener::eOAE_Load, IOnlineAttributesListener::eOAS_Failed);
	}

	pUpr->m_pReadingProfile = NULL;
	pUpr->m_lobbyTaskId = DrxLobbyInvalidTaskID;
}

//-----------------------------------------------------------------------------
void CPlayerProfileUpr::ReadOnlineAttributes(IPlayerProfile* pProfile, const SDrxLobbyUserData* pData, u32k numData)
{
	DRX_ASSERT(numData == m_onlineDataCount);

#if PROFILE_DEBUG_COMMANDS
	if (m_testingPhase == 2)
	{
		memcpy(m_onlineData, pData, sizeof(m_onlineData));
	}
#endif

	if (sLoadOnlineAttributes)
	{
		TOnlineAttributeMap::const_iterator end = m_onlineAttributeMap.end();

		TOnlineAttributeMap::const_iterator versionIter = m_onlineAttributeMap.find(ONLINE_VERSION_ATTRIBUTE_NAME);
		if (versionIter != end)
		{
			TFlowInputData versionData;
			ReadUserData(&pData[versionIter->second], versionData);

			i32k* pInt = versionData.GetPtr<i32>();
			if (pInt)
			{
				i32k version = *pInt;
				if (version == GetOnlineAttributesVersion())
				{

					if (ValidChecksums(pData, m_onlineDataCount))
					{
						TOnlineAttributeMap::const_iterator iter = m_onlineAttributeMap.begin();

						while (iter != end)
						{
							TFlowInputData data;
							ReadUserData(&pData[iter->second], data);
							pProfile->SetAttribute(iter->first, data);
							++iter;
						}

						i32k listenerSize = m_listeners.size();
						for (i32 i = 0; i < listenerSize; i++)
						{
							m_listeners[i]->LoadFromProfile(pProfile, true, ePR_All); // TODO: perhaps this only wants to be ePR_Options or needs an extra check?
						}
					}
					else
					{
						DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "OnlineAttributes - Checksum was invalid (Not reading online attributes)");
					}
				}
				else
				{
					DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "OnlineAttributes - version mismatch (Not reading online attributes)");
				}
			}
			else
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "OnlineAttributes - version data invalid (Not reading online attributes)");
			}
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "OnlineAttributes - version missing (Not reading online attributes)");
		}
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_COMMENT, "OnlineAttributes - not loading by request!");
	}
}

//------------------------------------------------------------------------
i32 CPlayerProfileUpr::GetOnlineAttributesVersion() const
{
	return m_onlineAttributeAutoGeneratedVersion + m_onlineAttributeDefinedVersion;
}

//------------------------------------------------------------------------
i32 CPlayerProfileUpr::GetOnlineAttributeIndexByName(tukk name)
{
	TOnlineAttributeMap::const_iterator end = m_onlineAttributeMap.end();
	TOnlineAttributeMap::const_iterator versionIter = m_onlineAttributeMap.find(name);
	i32 index = -1;

	if (versionIter != end)
	{
		index = versionIter->second;
	}

	return index;
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::GetOnlineAttributesDataFormat(SDrxLobbyUserData* pData, u32 numData)
{
	u32 numToGet = (numData > m_onlineDataCount) ? m_onlineDataCount : numData;

	for (i32 i = 0; i < numToGet; i++)
	{
		pData[i].m_id = m_onlineData[i].m_id;
		pData[i].m_type = m_onlineData[i].m_type;
	}
}

//------------------------------------------------------------------------
u32 CPlayerProfileUpr::GetOnlineAttributeCount()
{
	return m_onlineDataCount;
}

//------------------------------------------------------------------------
i32 CPlayerProfileUpr::ChecksumConvertValueToInt(const SDrxLobbyUserData* pData)
{
	i32 value = 0;

	switch (pData->m_type)
	{
	case eCLUDT_Int8:
		value = (i32)pData->m_int8;
		break;
	case eCLUDT_Int16:
		value = (i32)pData->m_int16;
		break;
	case eCLUDT_Int32:
		value = (i32)pData->m_int32;
		break;
	case eCLUDT_Float32:
		value = (i32)pData->m_f32;
		break;
	default:
		DRX_ASSERT_MESSAGE(0, string().Format("Unknown data type in online attribute data", pData->m_type));
		break;
	}

	return value;
}

//------------------------------------------------------------------------
void CPlayerProfileUpr::ApplyChecksums(SDrxLobbyUserData* pData, u32 numData)
{
	for (i32 i = 0; i < k_onlineChecksums; i++)
	{
		pData[i].m_int32 = Checksum(i, pData, numData);
	}
}

//------------------------------------------------------------------------
bool CPlayerProfileUpr::ValidChecksums(const SDrxLobbyUserData* pData, u32 numData)
{
	for (i32 i = 0; i < k_onlineChecksums; i++)
	{
		if (pData[i].m_int32 != Checksum(i, pData, numData))
		{
			return false;
		}
	}

	return true;
}

//------------------------------------------------------------------------
i32 CPlayerProfileUpr::Checksum(i32k checksum, const SDrxLobbyUserData* pData, u32 numData)
{
	switch (checksum)
	{
	case 0:
		{
			i32 hash = 40503;

			for (i32 i = k_onlineChecksums; i < numData; i++)
			{
				i32 value = ChecksumConvertValueToInt(&pData[i]);
				hash = ChecksumHash(hash, value);
			}

			return hash;
		}
	case 1:
		{
			i32 hash = 514229;

			for (i32 i = k_onlineChecksums; i < numData - 4; i += 4)
			{
				i32 value = ChecksumConvertValueToInt(&pData[i]);
				i32 value1 = ChecksumConvertValueToInt(&pData[i + 1]);
				i32 value2 = ChecksumConvertValueToInt(&pData[i + 2]);
				i32 value3 = ChecksumConvertValueToInt(&pData[i + 3]);

				hash = ChecksumHash(hash, value, value1, value2, value3);
			}

			return hash;
		}
	default:
		DRX_ASSERT(0);
		return 0;
	}
}

//------------------------------------------------------------------------
i32 CPlayerProfileUpr::ChecksumHash(i32k seed, i32k value) const
{
	i32 hash = seed + value;
	hash += (hash << 10);
	hash ^= (hash >> 6);
	hash += (hash << 3);

	return hash;
}

//------------------------------------------------------------------------
i32 CPlayerProfileUpr::ChecksumHash(i32k value0, i32k value1, i32k value2, i32k value3, i32k value4) const
{
	u32 a = (u32) value0;
	u32 b = (u32) value1;
	u32 c = (u32) value2;
	u32 d = (u32) value3;
	u32 e = (u32) value4;
	for (i32 i = 0; i < 16; i++)
	{
		a += (e >> 11);
		b += (a ^ d);
		c += (b << 3);
		d ^= (c >> 5);
		e += (d + 233);
	}

	return a + b + c + d + e;
}

//static------------------------------------------------------------------------
void CPlayerProfileUpr::RegisterUserDataCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg)
{
	CPlayerProfileUpr* pUpr = static_cast<CPlayerProfileUpr*>(pArg);
	DRX_ASSERT(pUpr);

	DRX_ASSERT(error == eCLE_Success);
	if (error == eCLE_Success)
	{
		pUpr->m_registered = true;
		pUpr->SetOnlineAttributesState(IOnlineAttributesListener::eOAE_Register, IOnlineAttributesListener::eOAS_Success);
	}
	else
	{
		pUpr->SetOnlineAttributesState(IOnlineAttributesListener::eOAE_Register, IOnlineAttributesListener::eOAS_Failed);
	}
}

//static------------------------------------------------------------------------
void CPlayerProfileUpr::WriteUserDataCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg)
{
	DRX_ASSERT(error == eCLE_Success);

#if PROFILE_DEBUG_COMMANDS
	CPlayerProfileUpr* pUpr = static_cast<CPlayerProfileUpr*>(pArg);
	DRX_ASSERT(pUpr);

	if (pUpr->m_testingPhase == 1)
	{
		DbgTestOnlineAttributes(NULL);
	}
#endif
}

#if PROFILE_DEBUG_COMMANDS
//static------------------------------------------------------------------------
void CPlayerProfileUpr::DbgLoadProfile(IConsoleCmdArgs* args)
{
	CPlayerProfileUpr* pPlayerProfMan = (CPlayerProfileUpr*) CDrxAction::GetDrxAction()->GetIPlayerProfileUpr();
	DrxLogAlways("Loading profile '%s'", pPlayerProfMan->GetCurrentUser());

	SUserEntry* pEntry = pPlayerProfMan->FindEntry(pPlayerProfMan->GetCurrentUser());

	CPlayerProfile* pProfile = (CPlayerProfile*) pPlayerProfMan->GetCurrentProfile(pPlayerProfMan->GetCurrentUser());

	pPlayerProfMan->LoadProfile(pEntry, pProfile, pPlayerProfMan->GetCurrentUser());
}

//static------------------------------------------------------------------------
void CPlayerProfileUpr::DbgSaveProfile(IConsoleCmdArgs* args)
{
	CPlayerProfileUpr* pPlayerProfMan = (CPlayerProfileUpr*) CDrxAction::GetDrxAction()->GetIPlayerProfileUpr();
	DrxLogAlways("Saving profile '%s'", pPlayerProfMan->GetCurrentUser());

	EProfileOperationResult result;
	pPlayerProfMan->SaveProfile(pPlayerProfMan->GetCurrentUser(), result, ePR_All);
	DrxLogAlways("Result %d", result);
}

//static------------------------------------------------------------------------
void CPlayerProfileUpr::DbgLoadOnlineAttributes(IConsoleCmdArgs* args)
{
	CPlayerProfileUpr* pPlayerProfMan = (CPlayerProfileUpr*) CDrxAction::GetDrxAction()->GetIPlayerProfileUpr();
	CPlayerProfile* pProfile = (CPlayerProfile*) pPlayerProfMan->GetCurrentProfile(pPlayerProfMan->GetCurrentUser());
	DrxLogAlways("%s", pProfile->GetName());
	pPlayerProfMan->LoadOnlineAttributes(pProfile);
}

//static------------------------------------------------------------------------
void CPlayerProfileUpr::DbgSaveOnlineAttributes(IConsoleCmdArgs* args)
{
	CPlayerProfileUpr* pPlayerProfMan = (CPlayerProfileUpr*) CDrxAction::GetDrxAction()->GetIPlayerProfileUpr();
	CPlayerProfile* pProfile = (CPlayerProfile*) pPlayerProfMan->GetCurrentProfile(pPlayerProfMan->GetCurrentUser());
	DrxLogAlways("%s", pProfile->GetName());
	pPlayerProfMan->SaveOnlineAttributes(pProfile);
}

//static------------------------------------------------------------------------
void CPlayerProfileUpr::DbgTestOnlineAttributes(IConsoleCmdArgs* args)
{
	CPlayerProfileUpr* pPlayerProfMan = (CPlayerProfileUpr*) CDrxAction::GetDrxAction()->GetIPlayerProfileUpr();

	if (pPlayerProfMan->m_enableOnlineAttributes)
	{
		if (pPlayerProfMan->m_registered)
		{
			DrxLogAlways("---Testing Phase %d---", pPlayerProfMan->m_testingPhase);
			switch (pPlayerProfMan->m_testingPhase)
			{
			case 0:
				{
					DrxLogAlways("Version %d", pPlayerProfMan->GetOnlineAttributesVersion());
					DrxLogAlways("DefinedVersion %d", pPlayerProfMan->m_onlineAttributeDefinedVersion);
					DrxLogAlways("AutoVersion %u", pPlayerProfMan->m_onlineAttributeAutoGeneratedVersion);
					DrxLogAlways("OnlineDataCount %u/%d", pPlayerProfMan->m_onlineDataCount, pPlayerProfMan->k_maxOnlineDataCount);
					DrxLogAlways("OnlineDataBytes %u/%d", pPlayerProfMan->m_onlineDataByteCount, pPlayerProfMan->k_maxOnlineDataBytes);

					memset(pPlayerProfMan->m_testData, 0, sizeof(pPlayerProfMan->m_testData));

					pPlayerProfMan->m_testingPhase = 1;
					DbgSaveOnlineAttributes(args);
				}
				break;

			case 1:
				{
					DrxLogAlways("Saved Online Data!");

					DrxLogAlways("Copying Original Data!");
					memcpy(pPlayerProfMan->m_testData, pPlayerProfMan->m_onlineData, sizeof(pPlayerProfMan->m_testData));

					DrxLogAlways("Memset Original Data!");
					memset(pPlayerProfMan->m_onlineData, -1, sizeof(pPlayerProfMan->m_onlineData));

					DrxLogAlways("Copying Profile FlowData");
					i32 arrayIndex = 0;
					CPlayerProfile* pProfile = (CPlayerProfile*) pPlayerProfMan->GetCurrentProfile(pPlayerProfMan->GetCurrentUser());

					TOnlineAttributeMap::const_iterator end = pPlayerProfMan->m_onlineAttributeMap.end();
					TOnlineAttributeMap::const_iterator iter = pPlayerProfMan->m_onlineAttributeMap.begin();
					while (iter != end)
					{
						TFlowInputData inputData;
						pProfile->GetAttribute(iter->first.c_str(), inputData);
						i32 inputIntData;
						inputData.GetValueWithConversion(inputIntData);

						pPlayerProfMan->m_testFlowData[arrayIndex] = inputIntData;

						arrayIndex++;
						++iter;
					}

					pPlayerProfMan->m_testingPhase = 2;
					DbgLoadOnlineAttributes(args);
				}
				break;

			case 2:
				{
					DrxLogAlways("Loaded Online Data!");

					i32 returned = memcmp(pPlayerProfMan->m_testData, pPlayerProfMan->m_onlineData, pPlayerProfMan->m_onlineDataCount * sizeof(SDrxLobbyUserData));
					DrxLogAlways("memcmp returned %d", returned);
					if (returned == 0)
					{
						DrxLogAlways("Online Data Returned SUCCESSFULLY!!!!!!!!!");
					}
					else
					{
						for (i32 i = 0; i < k_maxOnlineDataCount; i++)
						{
							if (pPlayerProfMan->m_testData[i].m_int32 != pPlayerProfMan->m_onlineData[i].m_int32)
							{
								DrxLogAlways("Expected %d (got %d) at %d", pPlayerProfMan->m_testData[i].m_int32, pPlayerProfMan->m_onlineData[i].m_int32, i);
							}
						}

						DrxLogAlways("Online Data Returned FAILED!!!!!!!!!!!!!!!!!!");
						pPlayerProfMan->m_testingPhase = 0;
						return;
					}

					DrxLogAlways("Checking Profile FlowData");
					i32 arrayIndex = 0;
					CPlayerProfile* pProfile = (CPlayerProfile*) pPlayerProfMan->GetCurrentProfile(pPlayerProfMan->GetCurrentUser());

					TOnlineAttributeMap::const_iterator end = pPlayerProfMan->m_onlineAttributeMap.end();
					TOnlineAttributeMap::const_iterator iter = pPlayerProfMan->m_onlineAttributeMap.begin();
					while (iter != end)
					{
						if (iter->second >= k_onlineChecksums)
						{
							TFlowInputData data;
							pProfile->GetAttribute(iter->first.c_str(), data);
							i32 intValue = -1;
							data.GetValueWithConversion(intValue);
							if (pPlayerProfMan->m_testFlowData[arrayIndex] != intValue)
							{
								DrxLogAlways("Non-matching Flow Data! FAILED!!!!!");
								pPlayerProfMan->m_testingPhase = 0;
								return;
							}
						}
						arrayIndex++;
						++iter;
					}

					DrxLogAlways("Complete Success!!!!!");

					pPlayerProfMan->m_testingPhase = 0;
					return;
				}
				break;
			}
		}
		else
		{
			DrxLogAlways("Online Attributes aren't not registered");
		}
	}
	else
	{
		DrxLogAlways("Online Attributes aren't not enabled");
	}
}

#endif

//-----------------------------------------------------------------------------
// FIXME: need something in dinrussystem or drxpak to move files or directories
#if DRX_PLATFORM_WINDOWS
bool CPlayerProfileUpr::MoveFileHelper(tukk existingFileName, tukk newFileName)
{
	char oldPath[IDrxPak::g_nMaxPath];
	char newPath[IDrxPak::g_nMaxPath];
	// need to adjust aliases and paths (use FLAGS_FOR_WRITING)
	gEnv->pDrxPak->AdjustFileName(existingFileName, oldPath, IDrxPak::FLAGS_FOR_WRITING);
	gEnv->pDrxPak->AdjustFileName(newFileName, newPath, IDrxPak::FLAGS_FOR_WRITING);
	return ::MoveFile(oldPath, newPath) != 0;
}
#else
// on all other platforms, just a warning
bool CPlayerProfileUpr::MoveFileHelper(tukk existingFileName, tukk newFileName)
{
	char oldPath[IDrxPak::g_nMaxPath];
	gEnv->pDrxPak->AdjustFileName(existingFileName, oldPath, IDrxPak::FLAGS_FOR_WRITING);
	string msg;
	msg.Format("CPlayerProfileUpr::MoveFileHelper for this Platform not implemented yet.\nOriginal '%s' will be lost!", oldPath);
	DRX_ASSERT_MESSAGE(0, msg.c_str());
	GameWarning("%s", msg.c_str());
	return false;
}
#endif

void CPlayerProfileUpr::GetMemoryStatistics(IDrxSizer* s)
{
	SIZER_SUBCOMPONENT_NAME(s, "PlayerProfiles");
	s->Add(*this);
	if (m_pDefaultProfile)
		m_pDefaultProfile->GetMemoryStatistics(s);
	s->AddContainer(m_userVec);
	m_pImpl->GetMemoryStatistics(s);
	s->AddObject(m_curUserID);
}
