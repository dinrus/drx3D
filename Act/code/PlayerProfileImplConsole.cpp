// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   PlayerProfileImplConsole.cpp
//  Created:     21/12/2009 by Alex Weighell.
//  Описание: Player profile implementation for consoles which manage
//               the profile data via the OS and not via a file system
//               which may not be present.
//
//               Note:
//               Currently only the user login code is implemented though.
//               The save data is still (incorrectly) using the file system.
//
//               TODO:
//               Change the save data to use the platforms OS save data API.
//               Probably best to do this through the IPLatformOS interface.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Sys/IXml.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/CoreX/Platform/IPlatformOS.h>
#include <drx3D/Act/PlayerProfileImplConsole.h>
#include <drx3D/Act/PlayerProfile.h>
#include <drx3D/Act/PlayerProfileImplRSFHelper.h>
#include <drx3D/Act/DrxActionCVars.h>

using namespace PlayerProfileImpl;

void CPlayerProfileImplConsole::Release()
{
	delete this;
}

// Profile Implementation which stores most of the stuff ActionMaps/Attributes in separate files

//------------------------------------------------------------------------
CPlayerProfileImplConsole::CPlayerProfileImplConsole() : m_pMgr(0)
{
}

//------------------------------------------------------------------------
CPlayerProfileImplConsole::~CPlayerProfileImplConsole()
{
}

//------------------------------------------------------------------------
bool CPlayerProfileImplConsole::Initialize(CPlayerProfileUpr* pMgr)
{
	m_pMgr = pMgr;
	return true;
}

//------------------------------------------------------------------------
void CPlayerProfileImplConsole::GetMemoryStatistics(IDrxSizer* s)
{
	s->Add(*this);
}

//------------------------------------------------------------------------
void CPlayerProfileImplConsole::InternalMakeFSPath(SUserEntry* pEntry, tukk profileName, string& outPath)
{
	if (pEntry)
	{
		outPath = ("%USER%/Profiles/");
	}
	else
	{
		// default profile always in game folder
		outPath = PathUtil::GetGameFolder();
		outPath.append("/Libs/Config/Profiles/");
	}
	if (profileName && *profileName)
	{
		// [Ian:11/10/10] We currently need to pass the profile name through the filename so we can figure out which user in PlatformOS.
		//if(!pEntry) // only do this for the default profile
		{
			outPath.append(profileName);
			outPath.append("/");
		}
	}
}

//------------------------------------------------------------------------
void CPlayerProfileImplConsole::InternalMakeFSSaveGamePath(SUserEntry* pEntry, tukk profileName, string& outPath, bool bNeedFolder)
{
	assert(pEntry != 0);
	assert(profileName != 0);

	if (m_pMgr->IsSaveGameFolderShared())
	{
		outPath = m_pMgr->GetSharedSaveGameFolder();
		outPath.append("/");
		if (!bNeedFolder)
		{
			// [Ian:20/10/10] We currently need to pass the profile name through the filename so we can figure out which user in PlatformOS.
			outPath.append(profileName);
			outPath.append("_");
		}
	}
	else
	{
		outPath = "%USER%/Profiles/";
		// [Ian:20/10/10] We currently need to pass the profile name through the filename so we can figure out which user in PlatformOS.
		outPath.append(profileName);
		outPath.append("/SaveGames/");
	}
}

//------------------------------------------------------------------------
bool CPlayerProfileImplConsole::SaveProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name, bool initialSave, i32 /*reason = ePR_All*/)
{
	IPlatformOS* os = gEnv->pSystem->GetPlatformOS();

	// save the profile into a specific location

	// check if it's a valid filename
	if (IsValidFilename(name) == false)
		return false;

	string path;
	InternalMakeFSPath(pEntry, name, path);

	XmlNodeRef rootNode = GetISystem()->CreateXmlNode(PROFILE_ROOT_TAG);
	rootNode->setAttr(PROFILE_NAME_TAG, name);
	CSerializerXML serializer(rootNode, false);
	pProfile->SerializeXML(&serializer);

	XmlNodeRef attributes = serializer.GetSection(CPlayerProfileUpr::ePPS_Attribute);
	XmlNodeRef actionMap = serializer.GetSection(CPlayerProfileUpr::ePPS_Actionmap);

	if (!rootNode->findChild("Attributes"))
		rootNode->addChild(attributes);
	if (!rootNode->findChild("ActionMaps"))
		rootNode->addChild(actionMap);

	return SaveXMLFile(path + "profile.xml", rootNode);
}

//------------------------------------------------------------------------
bool CPlayerProfileImplConsole::LoadProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name)
{
	// load the profile from a specific location

	// XML for now
	string path;
	InternalMakeFSPath(pEntry, name, path);

	XmlNodeRef rootNode = GetISystem()->CreateXmlNode(PROFILE_ROOT_TAG);
	CSerializerXML serializer(rootNode, true);

	XmlNodeRef profile = LoadXMLFile(path + "profile.xml");

	bool ok = false;
	if (profile)
	{
		XmlNodeRef attrNode = profile->findChild("Attributes");
		XmlNodeRef actionNode = profile->findChild("ActionMaps");
		if (!(attrNode && actionNode)) //default (PC) profile?
		{
			attrNode = LoadXMLFile(path + "attributes.xml");
			actionNode = LoadXMLFile(path + "actionmaps.xml");
		}

		if (attrNode && actionNode)
		{
			serializer.SetSection(CPlayerProfileUpr::ePPS_Attribute, attrNode);
			serializer.SetSection(CPlayerProfileUpr::ePPS_Actionmap, actionNode);

			ok = pProfile->SerializeXML(&serializer);
		}
	}

	return ok;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplConsole::LoginUser(SUserEntry* pEntry)
{
	// lookup stored profiles of the user (pEntry->userId) and fill in the pEntry->profileDesc
	// vector
	pEntry->profileDesc.clear();

	IPlatformOS* os = gEnv->pSystem->GetPlatformOS();

	u32 userIndex;
	bool signedIn = os->UserIsSignedIn(pEntry->userId.c_str(), userIndex);
	DrxLogAlways("LoginUser::UserIsSignedIn %d\n", signedIn);

	if (signedIn)
	{
		pEntry->profileDesc.push_back(SLocalProfileInfo(pEntry->userId));

		// Check the profile data exists - if not create it
		string path;
		InternalMakeFSPath(pEntry, pEntry->userId, path);
		IPlatformOS::IFileFinderPtr fileFinder = os->GetFileFinder(userIndex);
		//this assumes there is a profile if a directory exists
		if (!fileFinder->FileExists(path))
		{
			// Create new profile based on the defaults
			CPlayerProfile* profile = m_pMgr->GetDefaultCPlayerProfile();
			string name = profile->GetName();
			profile->SetName(pEntry->userId);
			m_pMgr->LoadGamerProfileDefaults(profile);
#if !DRX_PLATFORM_DURANGO
			// Durango: no reason to overwrite user profile with default profile, mainly for that we're always online.
			SaveProfile(pEntry, profile, pEntry->userId.c_str(), true);
#endif
			profile->SetName(name);
		}
	}
	else
	{
		printf("OS No User signed in\n");
	}

	return signedIn;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplConsole::DeleteProfile(SUserEntry* pEntry, tukk name)
{
	string path;
	InternalMakeFSPath(pEntry, name, path);  // no profile name -> only path
	bool bOK = gEnv->pDrxPak->RemoveDir(path.c_str(), true);
	// in case the savegame folder is shared, we have to delete the savegames from the shared folder
	if (bOK && m_pMgr->IsSaveGameFolderShared())
	{
		CPlayerProfileUpr::TSaveGameInfoVec saveGameInfoVec;
		if (GetSaveGames(pEntry, saveGameInfoVec, name)) // provide alternate profileName, because pEntry->pCurrentProfile points to the active profile
		{
			CPlayerProfileUpr::TSaveGameInfoVec::iterator iter = saveGameInfoVec.begin();
			CPlayerProfileUpr::TSaveGameInfoVec::iterator iterEnd = saveGameInfoVec.end();
			while (iter != iterEnd)
			{
				DeleteSaveGame(pEntry, iter->name);
				++iter;
			}
		}
	}
	return bOK;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplConsole::RenameProfile(SUserEntry* pEntry, tukk newName)
{
	// TODO: only rename in the filesystem. for now save new and delete old
	tukk oldName = pEntry->pCurrentProfile->GetName();
	bool ok = SaveProfile(pEntry, pEntry->pCurrentProfile, newName);
	if (!ok)
		return false;

	// move the save games
	if (CPlayerProfileUpr::sUseRichSaveGames)
	{
		CRichSaveGameHelper sgHelper(this);
		sgHelper.MoveSaveGames(pEntry, oldName, newName);
	}
	else
	{
		CCommonSaveGameHelper sgHelper(this);
		sgHelper.MoveSaveGames(pEntry, oldName, newName);
	}

	DeleteProfile(pEntry, oldName);
	return true;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplConsole::LogoutUser(SUserEntry* pEntry)
{
	return true;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplConsole::GetSaveGames(SUserEntry* pEntry, CPlayerProfileUpr::TSaveGameInfoVec& outVec, tukk altProfileName)
{
	if (CPlayerProfileUpr::sUseRichSaveGames)
	{
		CRichSaveGameHelper sgHelper(this);
		return sgHelper.GetSaveGames(pEntry, outVec, altProfileName);
	}
	else
	{
		CCommonSaveGameHelper sgHelper(this);
		return sgHelper.GetSaveGames(pEntry, outVec, altProfileName);
	}
}

//------------------------------------------------------------------------
ISaveGame* CPlayerProfileImplConsole::CreateSaveGame(SUserEntry* pEntry)
{
	if (CPlayerProfileUpr::sUseRichSaveGames)
	{
		CRichSaveGameHelper sgHelper(this);
		return sgHelper.CreateSaveGame(pEntry);
	}
	else
	{
		CCommonSaveGameHelper sgHelper(this);
		return sgHelper.CreateSaveGame(pEntry);
	}
}

//------------------------------------------------------------------------
ILoadGame* CPlayerProfileImplConsole::CreateLoadGame(SUserEntry* pEntry)
{
	if (CPlayerProfileUpr::sUseRichSaveGames)
	{
		CRichSaveGameHelper sgHelper(this);
		return sgHelper.CreateLoadGame(pEntry);
	}
	else
	{
		CCommonSaveGameHelper sgHelper(this);
		return sgHelper.CreateLoadGame(pEntry);
	}
}

//------------------------------------------------------------------------
bool CPlayerProfileImplConsole::DeleteSaveGame(SUserEntry* pEntry, tukk name)
{
	if (CPlayerProfileUpr::sUseRichSaveGames)
	{
		CRichSaveGameHelper sgHelper(this);
		return sgHelper.DeleteSaveGame(pEntry, name);
	}
	else
	{
		CCommonSaveGameHelper sgHelper(this);
		return sgHelper.DeleteSaveGame(pEntry, name);
	}
}

//------------------------------------------------------------------------
bool CPlayerProfileImplConsole::GetSaveGameThumbnail(SUserEntry* pEntry, tukk saveGameName, SThumbnail& thumbnail)
{
	if (CPlayerProfileUpr::sUseRichSaveGames)
	{
		CRichSaveGameHelper sgHelper(this);
		return sgHelper.GetSaveGameThumbnail(pEntry, saveGameName, thumbnail);
	}
	else
	{
		CCommonSaveGameHelper sgHelper(this);
		return sgHelper.GetSaveGameThumbnail(pEntry, saveGameName, thumbnail);
	}
}

ILevelRotationFile* CPlayerProfileImplConsole::GetLevelRotationFile(SUserEntry* pEntry, tukk name)
{
	CCommonSaveGameHelper sgHelper(this);
	return sgHelper.GetLevelRotationFile(pEntry, name);
}
