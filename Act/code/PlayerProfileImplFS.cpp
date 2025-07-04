// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Sys/IXml.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/Act/PlayerProfileImplFS.h>
#include <drx3D/Act/PlayerProfileImplRSFHelper.h>
#include <drx3D/Act/PlayerProfile.h>
#include <drx3D/Act/XmlSaveGame.h>
#include <drx3D/Act/XmlLoadGame.h>
#include <drx3D/Act/BMPHelper.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/CoreX/Platform/IPlatformOS.h>
#include <drx3D/Act/DrxActionCVars.h>
#include <drx3D/Act/SerializeWriterXMLCPBin.h>
#include <drx3D/Act/SerializeReaderXMLCPBin.h>
#include <drx3D/Act/XMLCPB_WriterInterface.h>
#include <drx3D/Act/XMLCPB_ReaderInterface.h>
#include <drx3D/Act/XMLCPB_Utils.h>

using namespace PlayerProfileImpl;

#define PLAYER_PROFILE_SAVE_AS_TEXT_IN_DEVMODE 1

//------------------------------------------------------------------------
CPlayerProfileImplFS::CPlayerProfileImplFS() : m_pMgr(0)
{
}

//------------------------------------------------------------------------
CPlayerProfileImplFS::~CPlayerProfileImplFS()
{
}

//------------------------------------------------------------------------
bool CPlayerProfileImplFS::Initialize(CPlayerProfileUpr* pMgr)
{
	m_pMgr = pMgr;
	return true;
}

//------------------------------------------------------------------------
void CPlayerProfileImplFS::InternalMakeFSPath(SUserEntry* pEntry, tukk profileName, string& outPath)
{
	if (pEntry)
	{
		outPath = ("%USER%/ProfilesSingle/");
	}
	else
	{
		// default profile always in game folder
		outPath = "Libs/Config/ProfilesSingle/";
	}
	if (profileName && *profileName)
	{
		outPath.append(profileName);
		outPath.append("/");
	}
}

//------------------------------------------------------------------------
void CPlayerProfileImplFS::InternalMakeFSSaveGamePath(SUserEntry* pEntry, tukk profileName, string& outPath, bool bNeedFolder)
{
	assert(pEntry != 0);
	assert(profileName != 0);

	if (m_pMgr->IsSaveGameFolderShared())
	{
		outPath = m_pMgr->GetSharedSaveGameFolder();
		outPath.append("/");
		if (!bNeedFolder)
		{
			outPath.append(profileName);
			outPath.append("_");
		}
	}
	else
	{
		outPath = "%USER%/ProfilesSingle/";
		outPath.append(profileName);
		outPath.append("/SaveGames/");
	}
}

//------------------------------------------------------------------------
bool CPlayerProfileImplFS::SaveProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name, bool initialSave, i32 reason /* = ePR_All*/)
{
	// save the profile into a specific location

	// check, if it's a valid filename
	if (IsValidFilename(name) == false)
		return false;

	// XML for now
	string filename;
	InternalMakeFSPath(pEntry, name, filename);

	// currently all (attributes, action maps, ...) in one file
	XmlNodeRef rootNode = GetISystem()->CreateXmlNode(PROFILE_ROOT_TAG);
	rootNode->setAttr(PROFILE_NAME_TAG, name);
	CSerializerXML serializer(rootNode, false); // SerializerXML no longer supports using sections in 1 file.

	bool ok = pProfile->SerializeXML(&serializer);
	if (ok)
	{
		ok = SaveXMLFile(filename, rootNode);
	}
	return ok;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplFS::LoadProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name)
{
	// load the profile from a specific location

	// XML for now
	// if pEntry==0 -> use factory default
	string filename;
	InternalMakeFSPath(pEntry, name, filename);

	// currently all (attributes, action maps, ...) in one file
	XmlNodeRef rootNode = LoadXMLFile(filename.c_str());
	if (rootNode == 0)
		return false;

	if (stricmp(rootNode->getTag(), PROFILE_ROOT_TAG) != 0)
	{
		GameWarning("CPlayerProfileImplFS::LoadProfile: Profile '%s' of user '%s' seems to exist, but is invalid (File '%s). Skipped", name, pEntry->userId.c_str(), filename.c_str());
		return false;
	}
	CSerializerXML serializer(rootNode, true); // SerializerXML no longer supports using sections in 1 file.

	bool ok = pProfile->SerializeXML(&serializer);
	return ok;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplFS::LoginUser(SUserEntry* pEntry)
{
	// lookup stored profiles of the user (pEntry->userId) and fill in the pEntry->profileDesc
	// vector
	pEntry->profileDesc.clear();

	// scan directory for profiles

	string path;
	InternalMakeFSPath(pEntry, "", path);  // no profile name -> only path

	IDrxPak* pDrxPak = gEnv->pDrxPak;
	_finddata_t fd;

	path.TrimRight("/\\");
	string search = path + "/*.xml";
	intptr_t handle = pDrxPak->FindFirst(search.c_str(), &fd);
	if (handle != -1)
	{
		do
		{
			// fd.name contains the profile name
			string filename = path;
			filename += "/";
			filename += fd.name;
			XmlNodeRef rootNode = LoadXMLFile(filename.c_str());

			// see if the root tag is o.k.
			if (rootNode && stricmp(rootNode->getTag(), PROFILE_ROOT_TAG) == 0)
			{
				string profileName = fd.name;
				PathUtil::RemoveExtension(profileName);
				if (rootNode->haveAttr(PROFILE_NAME_TAG))
				{
					tukk profileHumanName = rootNode->getAttr(PROFILE_NAME_TAG);
					if (profileHumanName != 0 && stricmp(profileHumanName, profileName) == 0)
					{
						profileName = profileHumanName;
					}
				}
				pEntry->profileDesc.push_back(SLocalProfileInfo(profileName));
			}
			else
			{
				GameWarning("CPlayerProfileImplFS::LoginUser: Profile '%s' of User '%s' seems to exist, but is invalid (File '%s). Skipped", fd.name, pEntry->userId.c_str(), filename.c_str());
			}
		}
		while (pDrxPak->FindNext(handle, &fd) >= 0);

		pDrxPak->FindClose(handle);
	}

	return true;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplFS::DeleteProfile(SUserEntry* pEntry, tukk name)
{
	string path;
	InternalMakeFSPath(pEntry, name, path);  // no profile name -> only path
	bool bOK = gEnv->pDrxPak->RemoveFile(path.c_str());
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
bool CPlayerProfileImplFS::RenameProfile(SUserEntry* pEntry, tukk newName)
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
bool CPlayerProfileImplFS::LogoutUser(SUserEntry* pEntry)
{
	return true;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplFS::GetSaveGames(SUserEntry* pEntry, CPlayerProfileUpr::TSaveGameInfoVec& outVec, tukk altProfileName)
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
ISaveGame* CPlayerProfileImplFS::CreateSaveGame(SUserEntry* pEntry)
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
ILoadGame* CPlayerProfileImplFS::CreateLoadGame(SUserEntry* pEntry)
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
bool CPlayerProfileImplFS::DeleteSaveGame(SUserEntry* pEntry, tukk name)
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
bool CPlayerProfileImplFS::GetSaveGameThumbnail(SUserEntry* pEntry, tukk saveGameName, SThumbnail& thumbnail)
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

// Profile Implementation which stores most of the stuff ActionMaps/Attributes in separate files

//------------------------------------------------------------------------
CPlayerProfileImplFSDir::CPlayerProfileImplFSDir() : m_pMgr(0)
{
}

//------------------------------------------------------------------------
CPlayerProfileImplFSDir::~CPlayerProfileImplFSDir()
{
}

//------------------------------------------------------------------------
bool CPlayerProfileImplFSDir::Initialize(CPlayerProfileUpr* pMgr)
{
	m_pMgr = pMgr;
	return true;
}

//------------------------------------------------------------------------
void CPlayerProfileImplFSDir::GetMemoryStatistics(IDrxSizer* s)
{
	s->Add(*this);
}

//------------------------------------------------------------------------
void CPlayerProfileImplFSDir::InternalMakeFSPath(SUserEntry* pEntry, tukk profileName, string& outPath)
{
	if (pEntry)
	{
		outPath = ("%USER%/Profiles/");
	}
	else
	{
		// default profile always in game folder
		outPath = "Libs/Config/Profiles/";
	}
	if (profileName && *profileName)
	{
		outPath.append(profileName);
		outPath.append("/");
	}
}

//------------------------------------------------------------------------
void CPlayerProfileImplFSDir::InternalMakeFSSaveGamePath(SUserEntry* pEntry, tukk profileName, string& outPath, bool bNeedFolder)
{
	assert(pEntry != 0);
	assert(profileName != 0);

	if (m_pMgr->IsSaveGameFolderShared())
	{
		outPath = m_pMgr->GetSharedSaveGameFolder();
		outPath.append("/");
		if (!bNeedFolder)
		{
			outPath.append(profileName);
			outPath.append("_");
		}
	}
	else
	{
		outPath = "%USER%/Profiles/";
		outPath.append(profileName);
		outPath.append("/SaveGames/");
	}
}

//------------------------------------------------------------------------
bool CPlayerProfileImplFSDir::SaveProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name, bool initialSave, i32 reason /* = ePR_All*/)
{
	// save the profile into a specific location

	// check if it's a valid filename
	if (IsValidFilename(name) == false)
		return false;

	string path;
	InternalMakeFSPath(pEntry, name, path);

	XmlNodeRef rootNode = GetISystem()->CreateXmlNode(PROFILE_ROOT_TAG);
	rootNode->setAttr(PROFILE_NAME_TAG, name);

	time_t lastPlayed = NULL;
	time(&lastPlayed);
	rootNode->setAttr(PROFILE_LAST_PLAYED, lastPlayed);

	// save profile.xml
	bool ok = SaveXMLFile(path + "profile.xml", rootNode);
	if (ok)
	{
		CSerializerXML serializer(rootNode, false);
		ok = pProfile->SerializeXML(&serializer);

		// save action map and attributes into separate files
		if (ok)
		{
			ok &= SaveXMLFile(path + "attributes.xml", serializer.GetSection(CPlayerProfileUpr::ePPS_Attribute));
			ok &= SaveXMLFile(path + "actionmaps.xml", serializer.GetSection(CPlayerProfileUpr::ePPS_Actionmap));
		}
	}
	return ok;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplFSDir::LoadProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name)
{
	// load the profile from a specific location

	// XML for now
	string path;
	InternalMakeFSPath(pEntry, name, path);

	XmlNodeRef rootNode = GetISystem()->CreateXmlNode(PROFILE_ROOT_TAG);
	CSerializerXML serializer(rootNode, true);
	XmlNodeRef attrNode = LoadXMLFile(path + "attributes.xml");
	XmlNodeRef actionNode = LoadXMLFile(path + "actionmaps.xml");
	//	serializer.AddSection(attrNode);
	//	serializer.AddSection(actionNode);
	serializer.SetSection(CPlayerProfileUpr::ePPS_Attribute, attrNode);
	serializer.SetSection(CPlayerProfileUpr::ePPS_Actionmap, actionNode);

	bool ok = pProfile->SerializeXML(&serializer);
	return ok;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplFSDir::LoginUser(SUserEntry* pEntry)
{
	// lookup stored profiles of the user (pEntry->userId) and fill in the pEntry->profileDesc
	// vector
	pEntry->profileDesc.clear();

	// scan directory for profiles

	string path;
	InternalMakeFSPath(pEntry, "", path);  // no profile name -> only path

	std::multimap<time_t, SLocalProfileInfo> profiles;

	IDrxPak* pDrxPak = gEnv->pDrxPak;
	_finddata_t fd;

	path.TrimRight("/\\");
	string search = path + "/*.*";

	IPlatformOS* pOS = GetISystem()->GetPlatformOS();
	IPlatformOS::IFileFinderPtr fileFinder = pOS->GetFileFinder(IPlatformOS::Unknown_User);
	intptr_t handle = fileFinder->FindFirst(search.c_str(), &fd);
	if (handle != -1)
	{
		do
		{
			if (strcmp(fd.name, ".") == 0 || strcmp(fd.name, "..") == 0)
				continue;

			if (fd.attrib & _A_SUBDIR)
			{
				// profile name = folder name
				// but make sure there is a profile.xml in it
				string filename = path + "/" + fd.name;
				filename += "/";
				filename += "profile.xml";
				XmlNodeRef rootNode = LoadXMLFile(filename.c_str());

				// see if the root tag is o.k.
				if (rootNode && stricmp(rootNode->getTag(), PROFILE_ROOT_TAG) == 0)
				{
					string profileName = fd.name;
					if (rootNode->haveAttr(PROFILE_NAME_TAG))
					{
						tukk profileHumanName = rootNode->getAttr(PROFILE_NAME_TAG);
						if (profileHumanName != 0 && stricmp(profileHumanName, profileName) == 0)
						{
							profileName = profileHumanName;
						}
					}
					time_t time = NULL;
					if (rootNode->haveAttr(PROFILE_LAST_PLAYED))
					{
						rootNode->getAttr(PROFILE_LAST_PLAYED, time);
					}
					SLocalProfileInfo info(profileName);
					info.SetLastLoginTime(time);
					profiles.insert(std::make_pair(time, info));
				}
				else
				{
					GameWarning("CPlayerProfileImplFSDir::LoginUser: Profile '%s' of User '%s' seems to exists but is invalid (File '%s). Skipped", fd.name, pEntry->userId.c_str(), filename.c_str());
				}
			}
		}
		while (fileFinder->FindNext(handle, &fd) >= 0);

		fileFinder->FindClose(handle);
	}

	// Insert in most recently played order
	std::multimap<time_t, SLocalProfileInfo>::const_reverse_iterator itend = profiles.rend();
	for (std::multimap<time_t, SLocalProfileInfo>::const_reverse_iterator it = profiles.rbegin(); it != itend; ++it)
		pEntry->profileDesc.push_back(it->second);

	return true;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplFSDir::DeleteProfile(SUserEntry* pEntry, tukk name)
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
bool CPlayerProfileImplFSDir::RenameProfile(SUserEntry* pEntry, tukk newName)
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
bool CPlayerProfileImplFSDir::LogoutUser(SUserEntry* pEntry)
{
	return true;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplFSDir::GetSaveGames(SUserEntry* pEntry, CPlayerProfileUpr::TSaveGameInfoVec& outVec, tukk altProfileName)
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
ISaveGame* CPlayerProfileImplFSDir::CreateSaveGame(SUserEntry* pEntry)
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
ILoadGame* CPlayerProfileImplFSDir::CreateLoadGame(SUserEntry* pEntry)
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
bool CPlayerProfileImplFSDir::DeleteSaveGame(SUserEntry* pEntry, tukk name)
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
bool CPlayerProfileImplFSDir::GetSaveGameThumbnail(SUserEntry* pEntry, tukk saveGameName, SThumbnail& thumbnail)
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

ILevelRotationFile* CPlayerProfileImplFSDir::GetLevelRotationFile(SUserEntry* pEntry, tukk name)
{
	CCommonSaveGameHelper sgHelper(this);
	return sgHelper.GetLevelRotationFile(pEntry, name);
}

//------------------------------------------------------------------------
// COMMON SaveGameHelper used by both CPlayerProfileImplFSDir and CPlayerProfileImplFS
//------------------------------------------------------------------------

bool CCommonSaveGameHelper::GetSaveGameThumbnail(CPlayerProfileUpr::SUserEntry* pEntry, tukk saveGameName, CPlayerProfileUpr::SThumbnail& thumbnail)
{
	assert(pEntry->pCurrentProfile != 0);
	if (pEntry->pCurrentProfile == 0)
	{
		GameWarning("CCommonSaveGameHelper:GetSaveGameThumbnail: Entry for user '%s' has no current profile", pEntry->userId.c_str());
		return false;
	}
	tukk name = saveGameName;
	string filename;
	m_pImpl->InternalMakeFSSaveGamePath(pEntry, pEntry->pCurrentProfile->GetName(), filename, true);
	string strippedName = PathUtil::ReplaceExtension(PathUtil::GetFile(name), ".bmp");
	filename.append(strippedName);

	i32 width = 0;
	i32 height = 0;
	i32 depth = 0;
	bool bSuccess = BMPHelper::LoadBMP(filename, 0, width, height, depth);
	if (bSuccess)
	{
		thumbnail.data.resize(width * height * depth);
		bSuccess = BMPHelper::LoadBMP(filename, thumbnail.data.begin(), width, height, depth);
		if (bSuccess)
		{
			thumbnail.height = height;
			thumbnail.width = width;
			thumbnail.depth = depth;
		}
	}
	if (!bSuccess)
		thumbnail.ReleaseData();
	return bSuccess;
}

bool CCommonSaveGameHelper::FetchMetaData(XmlNodeRef& root, CPlayerProfileUpr::SSaveGameMetaData& metaData)
{
	// TODO: use CXmlLoadGame for this
	XmlNodeRef metaDataNode = root;
	if (metaDataNode->isTag("Metadata") == false)
		metaDataNode = root->findChild("Metadata");
	if (metaDataNode == 0)
		return false;
	bool ok = true;
	ok &= GetAttr(metaDataNode, "level", metaData.levelName);
	ok &= GetAttr(metaDataNode, "gameRules", metaData.gameRules);
	ok &= GetAttr(metaDataNode, "version", metaData.fileVersion);
	ok &= GetAttr(metaDataNode, "build", metaData.buildVersion);
	ok &= GetTimeAttr(metaDataNode, "saveTime", metaData.saveTime);
	metaData.loadTime = metaData.saveTime;
	metaData.xmlMetaDataNode = metaDataNode;
	return ok;
}

bool CCommonSaveGameHelper::GetSaveGames(CPlayerProfileUpr::SUserEntry* pEntry, CPlayerProfileUpr::TSaveGameInfoVec& outVec, tukk altProfileName = "")
{
	// Scan savegames directory for XML files
	// we scan only for save game meta information
	string path;
	string profileName = (altProfileName && *altProfileName) ? altProfileName : pEntry->pCurrentProfile->GetName();
	m_pImpl->InternalMakeFSSaveGamePath(pEntry, profileName, path, true);

	const bool bNeedProfilePrefix = m_pImpl->GetUpr()->IsSaveGameFolderShared();
	string profilePrefix = profileName;
	profilePrefix += '_';
	size_t profilePrefixLen = profilePrefix.length();

	path.TrimRight("/\\");
	string search;
	search.Format("%s/*%s", path.c_str(), DRX_SAVEGAME_FILE_EXT);

	IPlatformOS* os = GetISystem()->GetPlatformOS();
	IPlatformOS::IFileFinderPtr fileFinder = os->GetFileFinder(IPlatformOS::Unknown_User);
	_finddata_t fd;
	intptr_t handle = fileFinder->FindFirst(search.c_str(), &fd);
	if (handle != -1)
	{
		CPlayerProfileUpr::SSaveGameInfo sgInfo;
		do
		{
			if (strcmp(fd.name, ".") == 0 || strcmp(fd.name, "..") == 0)
				continue;

			if (bNeedProfilePrefix)
			{
				if (strnicmp(profilePrefix, fd.name, profilePrefixLen) != 0)
					continue;
			}

			sgInfo.name = fd.name;
			if (bNeedProfilePrefix) // skip profile_ prefix (we made sure this is valid by comparism above)
				sgInfo.humanName = fd.name + profilePrefixLen;
			else
				sgInfo.humanName = fd.name;
			PathUtil::RemoveExtension(sgInfo.humanName);
			sgInfo.description = "no description";

			bool ok = false;

			// filename construction
			string metaFilename = path;
			metaFilename.append("/");
			metaFilename.append(fd.name);
			metaFilename = PathUtil::ReplaceExtension(metaFilename, ".meta");
			XmlNodeRef rootNode = LoadXMLFile(metaFilename.c_str());
			// see if the root tag is o.k.
			if (rootNode && stricmp(rootNode->getTag(), "Metadata") == 0)
			{
				// get meta data
				ok = FetchMetaData(rootNode, sgInfo.metaData);
			}
			else
			{
				// when there is no meta information, we currently reject the savegame
				//ok = true; // un-comment this, if you want to accept savegames without meta
			}
			// Use the file modified time for the load time as we touch the saves when they are used to keep most recent checkpoint
			sgInfo.metaData.loadTime = static_cast<time_t>(fd.time_write);

			if (ok)
			{
				outVec.push_back(sgInfo);
			}
			else
			{
				GameWarning("CPlayerProfileImplFS::GetSaveGames: SaveGame '%s' of user '%s' is invalid", fd.name, pEntry->userId.c_str());
			}

		}
		while (fileFinder->FindNext(handle, &fd) >= 0);

		fileFinder->FindClose(handle);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

class CXMLSaveGameFSDir : public CXmlSaveGame
{
	typedef CXmlSaveGame inherited;

public:
	CXMLSaveGameFSDir(ICommonProfileImpl* pImpl, CPlayerProfileImplFSDir::SUserEntry* pEntry)
	{
		m_pProfileImpl = pImpl;
		m_pEntry = pEntry;
		assert(m_pProfileImpl != 0);
		assert(m_pEntry != 0);
	}

	// ILoadGame
	virtual bool Init(tukk name)
	{
		assert(m_pEntry->pCurrentProfile != 0);
		if (m_pEntry->pCurrentProfile == 0)
		{
			GameWarning("CXMLSaveGameFSDir: Entry for user '%s' has no current profile", m_pEntry->userId.c_str());
			return false;
		}
		string path;
		m_pProfileImpl->InternalMakeFSSaveGamePath(m_pEntry, m_pEntry->pCurrentProfile->GetName(), path, false);
		// make directory or use the SaveXMLFile helper function
		// DrxCreateDirectory(...)
		string strippedName = PathUtil::GetFile(name);
		path.append(strippedName);
		return inherited::Init(path.c_str());
	}

	virtual bool Write(tukk filename, XmlNodeRef data)
	{
		const string fname(filename);
		bool bOK = inherited::Write(fname, data);
		if (bOK)
			bOK = WriteAdditionalData(filename, data);

		return bOK;
	}

	bool WriteAdditionalData(tukk filename, XmlNodeRef data)
	{
		bool bOK = true;
		// we save the meta information also in a separate file
		XmlNodeRef meta = data->findChild("Metadata");
		const string fname(filename);
		if (meta)
		{
			const string metaPath = PathUtil::ReplaceExtension(fname, ".meta");
			bOK = ::SaveXMLFile(metaPath, meta);
		}
		if (m_thumbnail.data.size() > 0)
		{
			const string bmpPath = PathUtil::ReplaceExtension(fname, ".bmp");
			BMPHelper::SaveBMP(bmpPath, m_thumbnail.data.begin(), m_thumbnail.width, m_thumbnail.height, m_thumbnail.depth, false);
		}
		return bOK;
	}

	virtual u8* SetThumbnail(u8k* imageData, i32 width, i32 height, i32 depth)
	{
		m_thumbnail.width = width;
		m_thumbnail.height = height;
		m_thumbnail.depth = depth;
		size_t size = width * height * depth;
		m_thumbnail.data.resize(size);
		if (imageData)
			memcpy(m_thumbnail.data.begin(), imageData, size);
		else
		{
			if (m_thumbnail.depth == 3)
			{
				u8* p = (u8*) m_thumbnail.data.begin();
				size_t n = size;
				while (n)
				{
					*p++ = 0x00; // B
					*p++ = 0x00; // G
					*p++ = 0x00; // R
					n -= 3;
				}
			}
			else if (m_thumbnail.depth == 4)
			{
				u32k col = RGBA8(0x00, 0x00, 0x00, 0x00); // alpha see through
				u32* p = (u32*) m_thumbnail.data.begin();
				size_t n = size >> 2;
				while (n--)
					*p++ = col;
			}
			else
			{
				memset(m_thumbnail.data.begin(), 0, size);
			}
		}
		return m_thumbnail.data.begin();
	}

	virtual bool SetThumbnailFromBMP(tukk filename)
	{
		i32 width = 0;
		i32 height = 0;
		i32 depth = 0;
		bool bSuccess = BMPHelper::LoadBMP(filename, 0, width, height, depth, true);
		if (bSuccess)
		{
			CPlayerProfileUpr::SThumbnail thumbnail;
			thumbnail.data.resize(width * height * depth);
			bSuccess = BMPHelper::LoadBMP(filename, thumbnail.data.begin(), width, height, depth, true);
			if (bSuccess)
			{
				SetThumbnail(thumbnail.data.begin(), width, height, depth);
			}
		}
		return bSuccess;
	}

	ICommonProfileImpl*                  m_pProfileImpl;
	CPlayerProfileImplFSDir::SUserEntry* m_pEntry;
	CPlayerProfileImplFSDir::SThumbnail  m_thumbnail;
};

//////////////////////////////////////////////////////////////////////////

class CXMLCPBinSaveGameFSDir : public CXMLSaveGameFSDir
{
	typedef CXMLSaveGameFSDir inherited;

	class CSerializeCtx : public _reference_target_t
	{
	public:
		CSerializeCtx(XMLCPB::CNodeLiveWriterRef node, XMLCPB::CWriterInterface& binWriter)
			: m_pWriterXMLCPBin(std::unique_ptr<CSerializeWriterXMLCPBin>(new CSerializeWriterXMLCPBin(node, binWriter)))
			, m_pWriter(std::unique_ptr<ISerialize>(new CSimpleSerializeWithDefaults<CSerializeWriterXMLCPBin>(*m_pWriterXMLCPBin)))
		{
		}

		TSerialize GetTSerialize() { return TSerialize(m_pWriter.get()); }

	private:
		std::unique_ptr<CSerializeWriterXMLCPBin> m_pWriterXMLCPBin;
		std::unique_ptr<ISerialize>               m_pWriter;
	};

public:
	CXMLCPBinSaveGameFSDir(ICommonProfileImpl* pImpl, CPlayerProfileImplFSDir::SUserEntry* pEntry) : inherited(pImpl, pEntry)
	{
	}

	virtual bool Init(tukk name)
	{
		bool ok = inherited::Init(name);
		if (ok)
			m_binXmlWriter.Init("SaveGame", GetFileName());
		return ok;
	}

	// adds a node with only attributes to the root
	void AddSimpleXmlToRoot(XmlNodeRef node)
	{
		XMLCPB::CNodeLiveWriterRef binNode = m_binXmlWriter.GetRoot();
		assert(binNode.IsValid());
		assert(node->getChildCount() == 0);
		binNode = binNode->AddChildNode(node->getTag());
		for (i32 i = 0; i < node->getNumAttributes(); ++i)
		{
			tukk pKey;
			tukk pVal;
			node->getAttributeByIndex(i, &pKey, &pVal);
			binNode->AddAttr(pKey, pVal);
		}
		binNode->Done();
	}

	virtual bool Write(tukk filename, XmlNodeRef data)
	{
		bool bResult = false;

		AddSimpleXmlToRoot(GetMetadataXmlNode());

		if (m_binXmlWriter.FinishWritingFile())
		{
			bResult = inherited::WriteAdditionalData(filename, data);
		}

		return bResult;
	}

	TSerialize AddSection(tukk section)
	{
		XMLCPB::CNodeLiveWriterRef nodeRef = m_binXmlWriter.GetRoot()->AddChildNode(section);
		_smart_ptr<CSerializeCtx> pCtx = new CSerializeCtx(nodeRef, m_binXmlWriter);
		m_sections.push_back(pCtx);
		return pCtx->GetTSerialize();
	}

	XMLCPB::CWriterInterface               m_binXmlWriter; // XMLCPB -> Compressed Binary XML  TODO: change all those XMLCPB names...
	std::vector<_smart_ptr<CSerializeCtx>> m_sections;
};

ISaveGame* CCommonSaveGameHelper::CreateSaveGame(CPlayerProfileUpr::SUserEntry* pEntry)
{
	bool useXMLCPBin = CDrxActionCVars::Get().g_useXMLCPBinForSaveLoad == 1;

	if (useXMLCPBin)
		return new CXMLCPBinSaveGameFSDir(m_pImpl, pEntry);
	else
		return new CXMLSaveGameFSDir(m_pImpl, pEntry);
}

//////////////////////////////////////////////////////////////////////////

class CXMLLoadGameFSDir : public CXmlLoadGame
{
	typedef CXmlLoadGame inherited;

public:
	CXMLLoadGameFSDir(ICommonProfileImpl* pImpl, CPlayerProfileImplFSDir::SUserEntry* pEntry)
	{
		m_pImpl = pImpl;
		m_pEntry = pEntry;
		assert(m_pImpl != 0);
		assert(m_pEntry != 0);
	}

	// ILoadGame
	virtual bool Init(tukk name)
	{
		bool ok = ObtainFinalFileName(name);
		if (ok)
			ok = inherited::Init(m_filename.c_str());

		return ok;
	}

	bool ObtainFinalFileName(tukk name)
	{
		assert(m_pEntry->pCurrentProfile != 0);
		if (m_pEntry->pCurrentProfile == 0)
		{
			GameWarning("CXMLLoadGameFSDir: Entry for user '%s' has no current profile", m_pEntry->userId.c_str());
			return false;
		}

		// figure out, if 'name' is an absolute path or a profile-relative path
		if (gEnv->pDrxPak->IsAbsPath(name) == false)
		{
			// no full path, assume 'name' is local to profile directory
			bool bNeedFolder = true;
			if (m_pImpl->GetUpr()->IsSaveGameFolderShared())
			{
				// if the savegame's name doesn't start with a profile_ prefix
				// add one (for quickload)
				string profilePrefix = m_pEntry->pCurrentProfile->GetName();
				profilePrefix.append("_");
				size_t profilePrefixLen = profilePrefix.length();
				if (strnicmp(name, profilePrefix, profilePrefixLen) != 0)
					bNeedFolder = false;
			}
			m_pImpl->InternalMakeFSSaveGamePath(m_pEntry, m_pEntry->pCurrentProfile->GetName(), m_filename, bNeedFolder);
			string strippedName = PathUtil::GetFile(name);
			m_filename.append(strippedName);
		}
		else
		{
			// it's an abs path, assign it
			m_filename.assign(name);
		}
		return true;
	}

	string                               m_filename;
	ICommonProfileImpl*                  m_pImpl;
	CPlayerProfileImplFSDir::SUserEntry* m_pEntry;
};

//////////////////////////////////////////////////////////////////////////

class CXMLCPBinLoadGameFSDir : public CXMLLoadGameFSDir
{
	typedef CXMLLoadGameFSDir inherited;

	class CSerializeCtx : public _reference_target_t
	{
	public:
		CSerializeCtx(XMLCPB::CNodeLiveReaderRef node, XMLCPB::CReaderInterface& binReader)
			: m_pReaderXMLCPBin(std::unique_ptr<CSerializeReaderXMLCPBin>(new CSerializeReaderXMLCPBin(node, binReader)))
			, m_pReader(std::unique_ptr<ISerialize>(new CSimpleSerializeWithDefaults<CSerializeReaderXMLCPBin>(*m_pReaderXMLCPBin)))
		{}

		TSerialize GetTSerialize() { return TSerialize(m_pReader.get()); }

	private:
		std::unique_ptr<CSerializeReaderXMLCPBin> m_pReaderXMLCPBin;
		std::unique_ptr<ISerialize>               m_pReader;
	};

public:
	CXMLCPBinLoadGameFSDir(ICommonProfileImpl* pImpl, CPlayerProfileImplFSDir::SUserEntry* pEntry)
		: inherited(pImpl, pEntry)
		, m_pHeap(XMLCPB::CReader::CreateHeap())
		, m_binXmlReader(m_pHeap)
		, m_metadataNode(m_binXmlReader.CreateNodeRef())
	{
	}

	virtual bool Init(tukk name)
	{
		bool ok = ObtainFinalFileName(name);

		if (ok)
		{
			ok = m_binXmlReader.ReadBinaryFile(m_filename.c_str());
			if (ok)
			{
#ifdef XMLCPB_DEBUGUTILS
				if (CDrxActionCVars::Get().g_XMLCPBGenerateXmlDebugFiles == 1)
					XMLCPB::CDebugUtils::DumpToXmlFile(m_binXmlReader.GetRoot(), "LastBinaryLoaded.xml");
#endif
				m_metadataNode = m_binXmlReader.GetRoot()->GetChildNode("Metadata");
			}
		}

		return ok;
	}

	IGeneralMemoryHeap* GetHeap()
	{
		return m_pHeap;
	}

	tukk GetMetadata(tukk pTag)
	{
		tukk pVal = NULL;
		m_metadataNode->ReadAttr(pTag, pVal);
		return pVal;
	}

	bool GetMetadata(tukk pTag, i32& value)
	{
		return m_metadataNode->ReadAttr(pTag, value);
	}

	bool HaveMetadata(tukk pTag)
	{
		return m_metadataNode->HaveAttr(pTag);
	}

	std::unique_ptr<TSerialize> GetSection(tukk pSection)
	{
		XMLCPB::CNodeLiveReaderRef node = m_binXmlReader.GetRoot()->GetChildNode(pSection);
		if (!node.IsValid())
			return std::unique_ptr<TSerialize>();
		_smart_ptr<CSerializeCtx> pCtx = new CSerializeCtx(node, m_binXmlReader);
		m_sections.push_back(pCtx);
		return std::unique_ptr<TSerialize>(new TSerialize(pCtx->GetTSerialize()));
	}

	bool HaveSection(tukk pSection)
	{
		return m_binXmlReader.GetRoot()->GetChildNode(pSection).IsValid();
	}

	tukk GetFileName() const
	{
		return m_filename.c_str();
	}

	_smart_ptr<IGeneralMemoryHeap>         m_pHeap;
	XMLCPB::CReaderInterface               m_binXmlReader; // XMLCPB -> Compressed Binary XML  TODO: change all those XMLCPB names...
	std::vector<_smart_ptr<CSerializeCtx>> m_sections;
	XMLCPB::CNodeLiveReaderRef             m_metadataNode;
};

ILoadGame* CCommonSaveGameHelper::CreateLoadGame(CPlayerProfileUpr::SUserEntry* pEntry)
{
	bool useXMLCPBin = CDrxActionCVars::Get().g_useXMLCPBinForSaveLoad == 1;

	if (useXMLCPBin)
		return new CXMLCPBinLoadGameFSDir(m_pImpl, pEntry);
	else
		return new CXMLLoadGameFSDir(m_pImpl, pEntry);
}

bool CCommonSaveGameHelper::DeleteSaveGame(CPlayerProfileUpr::SUserEntry* pEntry, tukk name)
{
	string filename;
	m_pImpl->InternalMakeFSSaveGamePath(pEntry, pEntry->pCurrentProfile->GetName(), filename, true);
	string strippedName = PathUtil::GetFile(name);
	filename.append(strippedName);
	bool ok = gEnv->pDrxPak->RemoveFile(filename.c_str()); // normal savegame
	filename = PathUtil::ReplaceExtension(filename, ".meta");
	gEnv->pDrxPak->RemoveFile(filename.c_str()); // meta data
	filename = PathUtil::ReplaceExtension(filename, ".bmp");
	gEnv->pDrxPak->RemoveFile(filename.c_str()); // thumbnail
	return ok;
}

bool CCommonSaveGameHelper::MoveSaveGames(CPlayerProfileUpr::SUserEntry* pEntry, tukk oldProfileName, tukk newProfileName)
{
	// move savegames or, if savegame folder is shared, rename them
	string oldSaveGamesPath;
	string newSaveGamesPath;
	m_pImpl->InternalMakeFSSaveGamePath(pEntry, oldProfileName, oldSaveGamesPath, true);
	m_pImpl->InternalMakeFSSaveGamePath(pEntry, newProfileName, newSaveGamesPath, true);

	CPlayerProfileUpr* pMgr = m_pImpl->GetUpr();
	if (pMgr->IsSaveGameFolderShared() == false)
	{
		// move complete folder
		pMgr->MoveFileHelper(oldSaveGamesPath, newSaveGamesPath);
	}
	else
	{
		// save game folder is shared, move file by file
		CPlayerProfileUpr::TSaveGameInfoVec saveGameInfoVec;
		if (GetSaveGames(pEntry, saveGameInfoVec, oldProfileName))
		{
			CPlayerProfileUpr::TSaveGameInfoVec::iterator iter = saveGameInfoVec.begin();
			CPlayerProfileUpr::TSaveGameInfoVec::iterator iterEnd = saveGameInfoVec.end();
			string oldPrefix = oldProfileName;
			oldPrefix += "_";
			size_t oldPrefixLen = oldPrefix.length();
			string newPrefix = newProfileName;
			newPrefix += "_";
			while (iter != iterEnd)
			{
				const string& oldSGName = iter->name;
				// begins with old profile's prefix?
				if (strnicmp(oldSGName, oldPrefix, oldPrefixLen) == 0)
				{
					string newSGName = newPrefix;
					newSGName.append(oldSGName, oldPrefixLen, oldSGName.length() - oldPrefixLen);
					string oldPath = oldSaveGamesPath + oldSGName;
					string newPath = newSaveGamesPath + newSGName;
					pMgr->MoveFileHelper(oldPath, newPath); // savegame

					oldPath = PathUtil::ReplaceExtension(oldPath, ".meta");
					newPath = PathUtil::ReplaceExtension(newPath, ".meta");
					pMgr->MoveFileHelper(oldPath, newPath); // meta

					oldPath = PathUtil::ReplaceExtension(oldPath, ".bmp");
					newPath = PathUtil::ReplaceExtension(newPath, ".bmp");
					pMgr->MoveFileHelper(oldPath, newPath); // thumbnail
				}
				++iter;
			}
		}
	}
	return true;
}

class CLevelRotationFile : public ILevelRotationFile
{
public:
	CLevelRotationFile(const string& fname) : m_filename(fname)
	{}
	~CLevelRotationFile()
	{}

	virtual bool Save(XmlNodeRef r)
	{
		return r->saveToFile(m_filename, 256 * 1024);
	}

	virtual XmlNodeRef Load()
	{
		if (FILE* f = gEnv->pDrxPak->FOpen(m_filename, "rb"))
		{
			gEnv->pDrxPak->FClose(f);
			return gEnv->pSystem->LoadXmlFromFile(m_filename);
		}
		else
			return gEnv->pSystem->LoadXmlFromFile("Libs/Config/Profiles/Default/levelrotation/levelrotation.xml");
	}

	virtual void Complete()
	{
		delete this;
	}
private:
	string m_filename;
};

ILevelRotationFile* CCommonSaveGameHelper::GetLevelRotationFile(CPlayerProfileUpr::SUserEntry* pEntry, tukk name)
{
	string rootFolder;
	m_pImpl->InternalMakeFSPath(pEntry, pEntry->pCurrentProfile->GetName(), rootFolder);
	rootFolder = PathUtil::Make(rootFolder, "levelrotation/");

	string strippedName = PathUtil::GetFile(name);
	strippedName = PathUtil::ReplaceExtension(strippedName, ".xml");
	string filename = PathUtil::Make(rootFolder, strippedName);
	return new CLevelRotationFile(filename);
}

namespace PlayerProfileImpl
{
tukk PROFILE_ROOT_TAG = "Profile";
tukk PROFILE_NAME_TAG = "Name";
tukk PROFILE_LAST_PLAYED = "LastPlayed";

// a simple serializer. manages sections (XmlNodes)
// is used during load and save
// CSerializerXML no longer supports sections being within the root node. In future it could be modified to but for now because of BinaryXML loading on consoles the ProfileImplFSDir is preferred.

CSerializerXML::CSerializerXML(XmlNodeRef& root, bool bLoading) : m_bLoading(bLoading), m_root(root)
{
	assert(m_root != 0);
}

XmlNodeRef CSerializerXML::CreateNewSection(CPlayerProfileUpr::EPlayerProfileSection section, tukk name)
{
	i32 sectionIdx = (i32)section;
	assert(sectionIdx >= 0 && sectionIdx < (i32)CPlayerProfileUpr::ePPS_Num);
	assert(m_sections[sectionIdx] == 0);

	XmlNodeRef newNode = GetISystem()->CreateXmlNode(name);
	m_sections[sectionIdx] = newNode;

	return newNode;
}

void CSerializerXML::SetSection(CPlayerProfileUpr::EPlayerProfileSection section, XmlNodeRef& node)
{
	i32 sectionIdx = (i32)section;
	assert(sectionIdx >= 0 && sectionIdx < (i32)CPlayerProfileUpr::ePPS_Num);
	assert(m_sections[sectionIdx] == 0);

	if (node != 0)
		m_sections[sectionIdx] = node;
}

XmlNodeRef CSerializerXML::GetSection(CPlayerProfileUpr::EPlayerProfileSection section)
{
	i32 sectionIdx = (i32)section;
	assert(sectionIdx >= 0 && sectionIdx < (i32)CPlayerProfileUpr::ePPS_Num);

	return m_sections[sectionIdx];
}

/*
   XmlNodeRef CSerializerXML::AddSection(tukk name)
   {
   XmlNodeRef child = m_root->findChild(name);
   if (child == 0)
   {
    child = m_root->newChild(name);
   }
   return child;
   }

   XmlNodeRef CSerializerXML::GetSection(tukk name)
   {
   XmlNodeRef child = m_root->findChild(name);
   return child;
   }
   // ~CPlayerProfileUpr::ISerializer

   // --- used by impl ---

   void CSerializerXML::AddSection(XmlNodeRef& node)
   {
   if (node != 0)
    m_root->addChild(node);
   }

   i32 CSerializerXML::GetSectionCount()
   {
   return m_root->getChildCount();
   }

   XmlNodeRef CSerializerXML::GetSectionByIndex(i32 index)
   {
   assert (index >=0 && index < m_root->getChildCount());
   return m_root->getChild(index);
   }
 */

// some helpers
bool SaveXMLFile(const string& filename, const XmlNodeRef& rootNode)
{
	LOADING_TIME_PROFILE_SECTION(gEnv->pSystem);

	bool ok = false;

	if (rootNode != 0)
	{
		ok = rootNode->saveToFile(filename.c_str());
	}

	if (!ok)
	{
		GameWarning("[PlayerProfiles] PlayerProfileImplFS: Cannot save XML file '%s'", filename.c_str());
	}
	return ok;
}

XmlNodeRef LoadXMLFile(const string& filename)
{
	XmlNodeRef rootNode = GetISystem()->LoadXmlFromFile(filename.c_str());
	if (rootNode == 0)
	{
		GameWarning("[PlayerProfiles] PlayerProfileImplFS: Cannot load XML file '%s'", filename.c_str());
	}
	return rootNode;
}

bool IsValidFilename(tukk filename)
{
	tukk invalidChars = "\\/:*?\"<>~|";
	return strpbrk(filename, invalidChars) == 0;
}

} //endns PlayerProfileImpl
