// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/AnimationDatabaseUpr.h>
#include <drx3D/CoreX/TypeInfo_impl.h>
#include <drx3D/Act/TagDefinitionXml.h>
#include <drx3D/Act/ProceduralClipConversion.h>
#include <drx3D/Act/MannequinUtils.h>

#include <drx3D/Act/Serialization.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/Sys/IResourceUpr.h>

const string MANNEQUIN_FOLDER = "Animations/Mannequin/ADB/";

CAnimationDatabaseUpr* CAnimationDatabaseUpr::s_Instance = NULL;

static void NormalizeFilename(char (&outFilename)[DEF_PATH_LENGTH], tukk inFilename)
{
	drx_strcpy(outFilename, DEF_PATH_LENGTH, inFilename);
	for (size_t i = 0; outFilename[i]; ++i)
	{
		if (outFilename[i] == '\\')
			outFilename[i] = '/';
	}
}

static bool IsValidNameIdentifier(tukk const name)
{
	tukk cit = name;
	if (!cit)
	{
		return false;
	}

	if (!isalpha(static_cast<u8>(*cit)) && !(*cit == '_'))
	{
		return false;
	}
	cit++;

	while (*cit)
	{
		const char c = *cit;
		if (!isalnum(static_cast<u8>(c)) && !(c == '_'))
		{
			return false;
		}
		cit++;
	}

	return true;
}

CAnimationDatabaseLibrary::CAnimationDatabaseLibrary()
{
	tukk ANIM_FLAGS[] =
	{
		"ManualUpdate",
		"Loop",
		"RepeatLast",
		"TimeWarping",
		"StartAtKeyTime",
		"StartAfter",
		"Idle2Move",
		"Move2Idle",
		"AllowRestart",
		"Sample30Hz",
		"DisableMultilayer",
		"ForceSkelUpdate",
		"TrackView",
		"RemoveFromFIFO",
		"FullRootPriority",
		"ForceTransitionToAnim"
	};
	i32k NUM_ANIM_FLAGS = DRX_ARRAY_COUNT(ANIM_FLAGS);

	for (i32 i = 0; i < NUM_ANIM_FLAGS; i++)
	{
		m_animFlags.AddTag(ANIM_FLAGS[i]);
	}

	m_animFlags.AssignBits();

	tukk TRANSITION_FLAGS[] =
	{
		"Cyclic",
		"CycleLocked",
		"Outro"
	};
	i32k NUM_TRANSITION_FLAGS = DRX_ARRAY_COUNT(TRANSITION_FLAGS);
	for (i32 i = 0; i < NUM_TRANSITION_FLAGS; i++)
	{
		m_transitionFlags.AddTag(TRANSITION_FLAGS[i]);
	}

	m_transitionFlags.AssignBits();
}

CAnimationDatabaseLibrary::~CAnimationDatabaseLibrary()
{
}

void CAnimationDatabaseLibrary::Insert(u32 nCrc, CAnimationDatabase* pDb)
{
	m_databases.insert(std::make_pair(nCrc, pDb));
}

CAnimationDatabaseLibrary::THandle CAnimationDatabaseLibrary::LoadResource(tukk name, u32 nFlags)
{
	THandle ret = TryOpenResource(name, nFlags);

	if (!ret)
	{
		CreateResource(ret, name, nFlags);
		if (ret)
			PublishResource(ret);
	}

	return ret;
}

CAnimationDatabaseLibrary::THandle CAnimationDatabaseLibrary::TryOpenResource(tukk name, u32 nFlags)
{
	char normalizedFilename[DEF_PATH_LENGTH];
	NormalizeFilename(normalizedFilename, name);

	u32 crc32 = CCrc32::ComputeLowercase(normalizedFilename);

	TAnimDatabaseList::const_iterator iter = m_databases.find(crc32);

	THandle res = NULL;
	if (iter != m_databases.end())
		res = iter->second;

	return res;
}

void CAnimationDatabaseLibrary::CreateResource(THandle& hdlOut, tukk name, u32 nFlags)
{
	char normalizedFilename[DEF_PATH_LENGTH];
	NormalizeFilename(normalizedFilename, name);

	CAnimationDatabase* pDb = new CAnimationDatabase(normalizedFilename);
	XmlNodeRef xml = gEnv->pSystem->GetXmlUtils()->LoadXmlFromFile(normalizedFilename);
	if (!xml || !LoadDatabase(xml, *pDb, true))
	{
		delete pDb;
		pDb = NULL;
	}

	hdlOut = pDb;
}

void CAnimationDatabaseLibrary::PublishResource(THandle& hdl)
{
#ifndef _RELEASE
	if (!hdl)
		__debugbreak();
#endif

	u32 crc = CCrc32::ComputeLowercase(hdl->GetFilename());
	Insert(crc, const_cast<CAnimationDatabase*>(static_cast<const CAnimationDatabase*>(hdl)));
}

CAnimationTagDefLibrary::CAnimationTagDefLibrary()
{
}

CAnimationTagDefLibrary::~CAnimationTagDefLibrary()
{
}

void CAnimationTagDefLibrary::Insert(u32 nCrc, CTagDefinition* pDef)
{
	m_tagDefs.insert(std::make_pair(nCrc, pDef));
}

CAnimationTagDefLibrary::THandle CAnimationTagDefLibrary::LoadResource(tukk name, u32 nFlags)
{
	THandle ret = TryOpenResource(name, nFlags);

	if (!ret)
	{
		CreateResource(ret, name, nFlags);
		if (ret)
			PublishResource(ret);
	}

	return ret;
}

CAnimationTagDefLibrary::THandle CAnimationTagDefLibrary::TryOpenResource(tukk name, u32 nFlags)
{
	THandle res = NULL;

	char normalizedFilename[DEF_PATH_LENGTH];
	NormalizeFilename(normalizedFilename, name);

	u32 crc32 = CCrc32::ComputeLowercase(normalizedFilename);

	TTagDefList::const_iterator iter = m_tagDefs.find(crc32);
	if (iter != m_tagDefs.end())
		res = iter->second;

	return res;
}

void CAnimationTagDefLibrary::CreateResource(THandle& hdlOut, tukk name, u32 nFlags)
{
	char normalizedFilename[DEF_PATH_LENGTH];
	NormalizeFilename(normalizedFilename, name);

	CTagDefinition* pDef = new CTagDefinition(normalizedFilename);

	if (!mannequin::LoadTagDefinition(normalizedFilename, *pDef, (nFlags & eTDF_Tags) != 0))
	{
		delete pDef;
		pDef = NULL;
	}

	hdlOut = pDef;
}

void CAnimationTagDefLibrary::PublishResource(THandle& hdlOut)
{
#ifndef _RELEASE
	if (!hdlOut)
		__debugbreak();
#endif

	u32 crc32 = CCrc32::ComputeLowercase(hdlOut->GetFilename());
	Insert(crc32, const_cast<CTagDefinition*>(hdlOut));
}

CAnimationControllerDefLibrary::CAnimationControllerDefLibrary()
{
}

CAnimationControllerDefLibrary::~CAnimationControllerDefLibrary()
{
}

void CAnimationControllerDefLibrary::Insert(u32 crc32, SControllerDef* pDef)
{
	m_controllerDefs.insert(std::make_pair(crc32, pDef));
}

CAnimationControllerDefLibrary::THandle CAnimationControllerDefLibrary::LoadResource(tukk name, u32 nFlags)
{
	THandle ret = TryOpenResource(name, nFlags);

	if (!ret)
	{
		CreateResource(ret, name, nFlags);
		if (ret)
			PublishResource(ret);
	}

	return ret;
}

CAnimationControllerDefLibrary::THandle CAnimationControllerDefLibrary::TryOpenResource(tukk name, u32 nFlags)
{
	THandle res = NULL;

	char normalizedFilename[DEF_PATH_LENGTH];
	NormalizeFilename(normalizedFilename, name);

	u32 crc32 = CCrc32::ComputeLowercase(normalizedFilename);

	TControllerDefList::const_iterator iter = m_controllerDefs.find(crc32);

	if (iter != m_controllerDefs.end())
		res = iter->second;

	return res;
}

void CAnimationControllerDefLibrary::CreateResource(THandle& hdlOut, tukk name, u32 nFlags)
{
	char normalizedFilename[DEF_PATH_LENGTH];
	NormalizeFilename(normalizedFilename, name);

	u32 crc32 = CCrc32::ComputeLowercase(normalizedFilename);

	SControllerDef* pDef = NULL;
	XmlNodeRef xml = gEnv->pSystem->GetXmlUtils()->LoadXmlFromFile(normalizedFilename);
	if (xml)
	{
		pDef = LoadControllerDef(xml, normalizedFilename);
		if (pDef)
			pDef->m_filename.SetByString(normalizedFilename);
	}

	hdlOut = pDef;
}

void CAnimationControllerDefLibrary::PublishResource(THandle& hdl)
{
#ifndef _RELEASE
	if (!hdl)
		__debugbreak();
#endif

	u32 crc32 = hdl->m_filename.crc;
	Insert(crc32, const_cast<SControllerDef*>(hdl));
}

CAnimationDatabaseUpr::CAnimationDatabaseUpr()
	: m_editorListenerSet(1)
{
	tukk FRAGMENT_FLAGS[] =
	{
		"Persistent",
		"AutoReinstall"
	};
	i32k NUM_FRAGMENT_FLAGS = DRX_ARRAY_COUNT(FRAGMENT_FLAGS);
	for (i32 i = 0; i < NUM_FRAGMENT_FLAGS; i++)
	{
		m_fragmentFlags.AddTag(FRAGMENT_FLAGS[i]);
	}

	m_fragmentFlags.AssignBits();
}

CAnimationDatabaseUpr::~CAnimationDatabaseUpr()
{
	UnloadAll();
}

const IAnimationDatabase* CAnimationDatabaseUpr::FindDatabase(u32 crcFilename) const
{
	return stl::find_in_map(m_databases, crcFilename, NULL);
}

const IAnimationDatabase* CAnimationDatabaseUpr::Load(tukk databaseName)
{
	//Don't load if there is no database
	if (databaseName == nullptr || strlen(databaseName) == 0)
	{
		return nullptr;
	}

	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Mannequin, 0, "Load ADB: %s", databaseName);

	const IAnimationDatabase* pDb = CAnimationDatabaseLibrary::LoadResource(databaseName, 0);
	return pDb;
}

IAnimationDatabase* CAnimationDatabaseUpr::Create(tukk filename, tukk defFilename)
{
	char normalizedFilename[DEF_PATH_LENGTH];
	NormalizeFilename(normalizedFilename, filename);

	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Mannequin, 0, "Create ADB: %s", normalizedFilename);

	u32 crc32 = CCrc32::ComputeLowercase(normalizedFilename);

	TAnimDatabaseList::const_iterator iter = m_databases.find(crc32);

	if (iter != m_databases.end())
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "!Attempting to create ADB file %s which already exists", normalizedFilename);
		return NULL;
	}
	else
	{
		char normalizedDefFilename[DEF_PATH_LENGTH];
		NormalizeFilename(normalizedDefFilename, defFilename);

		const SControllerDef* def = LoadControllerDef(normalizedDefFilename);

		if (def)
		{
			CAnimationDatabase* animDB = new CAnimationDatabase();
			m_databases.insert(std::make_pair(crc32, animDB));

			u32k numActions = def->m_fragmentIDs.GetNum();

			animDB->m_filename = normalizedFilename;
			animDB->m_pFragDef = &def->m_fragmentIDs;
			animDB->m_pTagDef = &def->m_tags;
			animDB->m_fragmentList.resize(numActions);
			for (u32 i = 0; i < numActions; ++i)
			{
				animDB->m_fragmentList[i] = new CAnimationDatabase::SFragmentEntry();
			}

			Save(*animDB, normalizedFilename);

			return animDB;
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "!Failed to load def file %s whilst creating ADB file %s", normalizedDefFilename, normalizedFilename);

			return NULL;
		}
	}
}

CTagDefinition* CAnimationDatabaseUpr::CreateTagDefinition(tukk filename)
{
	char normalizedFilename[DEF_PATH_LENGTH];
	NormalizeFilename(normalizedFilename, filename);

	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Mannequin, 0, "Create TagDefinition: %s", normalizedFilename);

	u32 crc32 = CCrc32::ComputeLowercase(normalizedFilename);

	CTagDefinition* tagDef = new CTagDefinition(normalizedFilename);

	mannequin::SaveTagDefinition(*tagDef);

	m_tagDefs.insert(std::make_pair(crc32, tagDef));

	return tagDef;
}

const SControllerDef* CAnimationDatabaseUpr::LoadControllerDef(tukk filename)
{
	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Mannequin, 0, "Load ControllerDef: %s", filename);

	const SControllerDef* const pDef = CAnimationControllerDefLibrary::LoadResource(filename, 0);
	return pDef;
}

const SControllerDef* CAnimationDatabaseUpr::FindControllerDef(u32k crcFilename) const
{
	return stl::find_in_map(m_controllerDefs, crcFilename, NULL);
}

const SControllerDef* CAnimationDatabaseUpr::FindControllerDef(tukk filename) const
{
	char normalizedFilename[DEF_PATH_LENGTH];
	NormalizeFilename(normalizedFilename, filename);

	u32 crc32 = CCrc32::ComputeLowercase(normalizedFilename);

	return FindControllerDef(crc32);
}

const CTagDefinition* CAnimationDatabaseUpr::LoadTagDefs(tukk filename, bool isTags)
{
	char normalizedFilename[DEF_PATH_LENGTH];
	NormalizeFilename(normalizedFilename, filename);

	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Mannequin, 0, "Load TagDefs: %s", normalizedFilename);

	const CTagDefinition* pTagDef = CAnimationTagDefLibrary::LoadResource(filename, isTags ? eTDF_Tags : 0);
	return pTagDef;
}

const CTagDefinition* CAnimationDatabaseUpr::FindTagDef(u32k crcFilename) const
{
	return stl::find_in_map(m_tagDefs, crcFilename, NULL);
}

const CTagDefinition* CAnimationDatabaseUpr::FindTagDef(tukk filename) const
{
	char normalizedFilename[DEF_PATH_LENGTH];
	NormalizeFilename(normalizedFilename, filename);

	u32 crc32 = CCrc32::ComputeLowercase(normalizedFilename);

	return FindTagDef(crc32);
}

void CAnimationDatabaseUpr::RemoveDataFromParent(CAnimationDatabase* parentADB, const CAnimationDatabase::SSubADB* subADB)
{
	// Clear out fragments that came from subAnimDB
	FragmentID fragID = 0;
	for (CAnimationDatabase::TFragmentList::const_iterator it = parentADB->m_fragmentList.begin(), itEnd = parentADB->m_fragmentList.end(); it != itEnd; ++it, ++fragID)
	{
		if (CanSaveFragmentID(fragID, *parentADB, subADB))
		{
			if (CAnimationDatabase::SFragmentEntry* entry = *it)
			{
				for (i32 k = entry->tagSetList.Size() - 1; k >= 0; k--)
				{
					bool bDummy;
					if (ShouldSaveFragment(fragID, entry->tagSetList.m_keys[k].globalTags, *parentADB, subADB, bDummy))
					{
						entry->tagSetList.Erase(k);
					}
				}
			}
		}
	}

	// Clear out blends that came from subAnimDB
	for (CAnimationDatabase::TFragmentBlendDatabase::iterator iter = parentADB->m_fragmentBlendDB.begin(), iterEnd = parentADB->m_fragmentBlendDB.end(); iter != iterEnd; ++iter)
	{
		if (CanSaveFragmentID(iter->first.fragFrom, *parentADB, subADB) && CanSaveFragmentID(iter->first.fragTo, *parentADB, subADB))
		{
			for (i32 v = iter->second.variantList.size() - 1; v >= 0; v--)
			{
				CAnimationDatabase::SFragmentBlendVariant& variant = iter->second.variantList[v];
				bool bDummy;
				if (ShouldSaveTransition(iter->first.fragFrom, iter->first.fragTo, variant.tagsFrom.globalTags, variant.tagsTo.globalTags, *parentADB, subADB, bDummy))
				{
					for (CAnimationDatabase::TFragmentBlendList::iterator fragIt = variant.blendList.begin(), fragItEnd = variant.blendList.end(); fragIt != fragItEnd; ++fragIt)
					{
						delete fragIt->pFragment;
					}
					iter->second.variantList.erase(iter->second.variantList.begin() + v);
				}
			}
		}
	}
}

void CAnimationDatabaseUpr::RevertSubADB(tukk szFilename, CAnimationDatabase* animDB, const CAnimationDatabase::SSubADB& subADB)
{
	// Check the subADBs of animDB
	if (stricmp(szFilename, subADB.filename) == 0)
	{
		RemoveDataFromParent(animDB, &subADB);

		XmlNodeRef xmlData = GetISystem()->LoadXmlFromFile(subADB.filename);
		LoadDatabaseData(*animDB, xmlData, animDB->GetFragmentDefs(), animDB->GetTagDefs(), subADB.tags, false);
	}
	for (CAnimationDatabase::TSubADBList::const_iterator adbIt = subADB.subADBs.begin(); adbIt != subADB.subADBs.end(); ++adbIt)
		RevertSubADB(szFilename, animDB, *adbIt);
}

void CAnimationDatabaseUpr::RevertDatabase(tukk szFilename)
{
	for (TAnimDatabaseList::const_iterator it = m_databases.begin(), itEnd = m_databases.end(); it != itEnd; ++it)
	{
		CAnimationDatabase* animDB = it->second;

		animDB->SetAutoSort(false);

		for (CAnimationDatabase::TSubADBList::const_iterator adbIt = animDB->m_subADBs.begin(); adbIt != animDB->m_subADBs.end(); ++adbIt)
		{
			const CAnimationDatabase::SSubADB& subADB = *adbIt;
			RevertSubADB(szFilename, animDB, subADB);
		}

		// Are we reverting animDB itself?
		if (stricmp(szFilename, animDB->GetFilename()) == 0)
		{
			RemoveDataFromParent(animDB, NULL);

			XmlNodeRef xmlData = GetISystem()->LoadXmlFromFile(animDB->GetFilename());
			LoadDatabaseData(*animDB, xmlData,
			                 animDB->GetFragmentDefs(), animDB->GetTagDefs(),
			                 TAG_STATE_EMPTY, false);
		}

		animDB->Sort();
		animDB->SetAutoSort(true);
	}
}

void CAnimationDatabaseUpr::ClearDatabase(CAnimationDatabase* pDatabase)
{
	// Clear out the fragment list
	std::for_each(pDatabase->m_fragmentList.begin(), pDatabase->m_fragmentList.end(), stl::container_object_deleter());
	pDatabase->m_fragmentList.clear();

	// Clear out the blends
	for (CAnimationDatabase::TFragmentBlendDatabase::iterator blDbIt = pDatabase->m_fragmentBlendDB.begin(), blDbItEnd = pDatabase->m_fragmentBlendDB.end(); blDbIt != blDbItEnd; ++blDbIt)
	{
		CAnimationDatabase::SFragmentBlendEntry& entry = blDbIt->second;
		for (CAnimationDatabase::TFragmentVariantList::iterator vrIt = entry.variantList.begin(), vrItEnd = entry.variantList.end(); vrIt != vrItEnd; ++vrIt)
		{
			CAnimationDatabase::SFragmentBlendVariant& variant = *vrIt;
			for (CAnimationDatabase::TFragmentBlendList::iterator fragIt = variant.blendList.begin(), fragItEnd = variant.blendList.end(); fragIt != fragItEnd; ++fragIt)
			{
				delete fragIt->pFragment;
			}
		}
	}
	pDatabase->m_fragmentBlendDB.clear();
}

void CAnimationDatabaseUpr::ReloadDatabase(IAnimationDatabase* pDatabase)
{
	CAnimationDatabase* animDB = (CAnimationDatabase*)pDatabase;

	ClearDatabase(animDB);

	// Clear out the subADBs
	stl::free_container(animDB->m_subADBs);

	// Reload the database and all subDB data
	if (XmlNodeRef xmlData = GetISystem()->LoadXmlFromFile(animDB->m_filename.c_str()))
	{
		LoadDatabase(xmlData, *animDB, true);
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Could not load xml file '%s'", animDB->m_filename.c_str());
	}
}

void CAnimationDatabaseUpr::RevertControllerDef(tukk szFilename)
{
	if (SControllerDef* controllerDef = const_cast<SControllerDef*>(FindControllerDef(szFilename)))
	{
		ReloadControllerDef(controllerDef);
	}
}

void CAnimationDatabaseUpr::ReloadControllerDef(SControllerDef* pControllerDef)
{
	TDefPathString defFilename = pControllerDef->m_filename;

	if (XmlNodeRef xmlData = GetISystem()->LoadXmlFromFile(defFilename.c_str()))
	{
		if (SControllerDef* newControllerDef = CAnimationControllerDefLibrary::LoadControllerDef(xmlData, defFilename.c_str()))
		{
			new(pControllerDef) SControllerDef(*newControllerDef);
			pControllerDef->m_filename = defFilename;
			delete newControllerDef;
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Error in reloading xml file %s", defFilename.c_str());
		}
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Could not load xml file %s", defFilename.c_str());
	}
}

void CAnimationDatabaseUpr::RevertTagDef(tukk szFilename)
{
	if (CTagDefinition* tagDef = const_cast<CTagDefinition*>(FindTagDef(szFilename)))
	{
		ReloadTagDefinition(tagDef);
	}
}

void CAnimationDatabaseUpr::ReloadTagDefinition(CTagDefinition* pTagDefinition)
{
	assert(pTagDefinition);
	pTagDefinition->Clear();

	tukk const filename = pTagDefinition->GetFilename();
	mannequin::LoadTagDefinition(filename, *pTagDefinition, pTagDefinition->HasMasks());

	NotifyListenersTagDefinitionChanged(*pTagDefinition);
}

void CAnimationDatabaseUpr::ReloadAll()
{
	for (TTagDefList::const_iterator iter = m_tagDefs.begin(), iterEnd = m_tagDefs.end(); iter != iterEnd; ++iter)
	{
		CTagDefinition* tagDef = iter->second;
		ReloadTagDefinition(tagDef);
	}

	for (TControllerDefList::iterator iter = m_controllerDefs.begin(), iterEnd = m_controllerDefs.end(); iter != iterEnd; ++iter)
	{
		SControllerDef* controllerDef = iter->second;
		ReloadControllerDef(controllerDef);
	}

	for (TAnimDatabaseList::const_iterator iter = m_databases.begin(), iterEnd = m_databases.end(); iter != iterEnd; ++iter)
	{
		CAnimationDatabase* animDB = iter->second;
		ReloadDatabase(animDB);
	}
}

void CAnimationDatabaseUpr::UnloadAll()
{
	for (TAnimDatabaseList::iterator it = m_databases.begin(), itEnd = m_databases.end(); it != itEnd; ++it)
		delete it->second;
	for (TControllerDefList::iterator it = m_controllerDefs.begin(), itEnd = m_controllerDefs.end(); it != itEnd; ++it)
		delete it->second;
	for (TTagDefList::iterator it = m_tagDefs.begin(), itEnd = m_tagDefs.end(); it != itEnd; ++it)
		delete it->second;

	m_databases.clear();
	m_controllerDefs.clear();
	m_tagDefs.clear();

	mannequin::OnDatabaseUprUnload();
}

void CAnimationDatabaseUpr::GetAffectedFragmentsString(const CTagDefinition* pQueryTagDef, TagID tagID, tuk buffer, i32 bufferSize)
{
	string filename;
	string temp;
	string matchingFragments;
	uint pos = 0;

	for (TAnimDatabaseList::const_iterator iter = m_databases.begin(); iter != m_databases.end(); ++iter)
	{
		CAnimationDatabase* pCurrentDatabase = iter->second;
		const CTagDefinition& fragmentDefs = pCurrentDatabase->GetFragmentDefs();
		bool isGlobalTag = (&pCurrentDatabase->GetTagDefs() == pQueryTagDef);
		bool filenameAdded = false;

		filename = pCurrentDatabase->GetFilename();
		filename = filename.substr(MANNEQUIN_FOLDER.length());

		u32k numFragmentDefs = fragmentDefs.GetNum();
		for (u32 fragIndex = 0; fragIndex < numFragmentDefs; ++fragIndex)
		{
			if (isGlobalTag || (fragmentDefs.GetSubTagDefinition(fragIndex) == pQueryTagDef))
			{
				u32k numTagSets = pCurrentDatabase->GetTotalTagSets(fragIndex);
				for (u32 tagSetIndex = 0; tagSetIndex < numTagSets; ++tagSetIndex)
				{
					SFragTagState tagState;
					u32 numOptions = pCurrentDatabase->GetTagSetInfo(fragIndex, tagSetIndex, tagState);
					if (isGlobalTag)
					{
						if (pQueryTagDef->IsSet(tagState.globalTags, tagID))
						{
							if (!filenameAdded)
							{
								matchingFragments.TrimRight(',');
								temp.Format("\n[%s]:", filename.c_str());
								matchingFragments += temp;
								filenameAdded = true;
							}
							DrxStackStringT<char, 128> sGlobalTags;
							pCurrentDatabase->GetTagDefs().FlagsToTagList(tagState.globalTags, sGlobalTags);
							temp.Format(" %s (%s),", fragmentDefs.GetTagName(fragIndex), sGlobalTags.c_str());
							matchingFragments += temp;
						}
					}
					else
					{
						if (pQueryTagDef->IsSet(tagState.fragmentTags, tagID))
						{
							if (!filenameAdded)
							{
								matchingFragments.TrimRight(',');
								temp.Format("\n[%s]:", filename.c_str());
								matchingFragments += temp;
								filenameAdded = true;
							}
							DrxStackStringT<char, 128> sFragmentTags;
							fragmentDefs.GetSubTagDefinition(fragIndex)->FlagsToTagList(tagState.fragmentTags, sFragmentTags);
							temp.Format(" %s (%s),", fragmentDefs.GetTagName(fragIndex), sFragmentTags.c_str());
							matchingFragments += temp;
						}
					}
				}
			}

			if (pos != matchingFragments.length())
			{
				// Log the fragments so there's a record
				DrxLog("[TAGDEF]: affected fragments: %s", matchingFragments.Mid(pos, matchingFragments.length() - 1).c_str());
				pos = matchingFragments.length();
			}
		}
	}

	if (!matchingFragments.empty())
	{
		matchingFragments.TrimRight(',');
	}
	else
	{
		matchingFragments = "\nno fragments";
	}

	drx_strcpy(buffer, bufferSize, matchingFragments.c_str());
	if (matchingFragments.length() + 1 > bufferSize)
	{
		// In case the string is truncated
		memcpy(buffer + bufferSize - 4, "...", 4);
	}
}

void CAnimationDatabaseUpr::ApplyTagDefChanges(const CTagDefinition* pOriginal, CTagDefinition* pModified)
{
	string filename;

	for (TAnimDatabaseList::const_iterator iter = m_databases.begin(); iter != m_databases.end(); ++iter)
	{
		CAnimationDatabase* pCurrentDatabase = iter->second;
		bool isGlobalTag = (&pCurrentDatabase->GetTagDefs() == pOriginal);

		if (isGlobalTag)
		{
			i32k numSubADBs = pCurrentDatabase->m_subADBs.size();
			TagState modified;

			for (i32 subADBIndex = numSubADBs - 1; subADBIndex >= 0; --subADBIndex)
			{
				CAnimationDatabase::SSubADB& subADB = pCurrentDatabase->m_subADBs[subADBIndex];
				if (pOriginal->MapTagState(subADB.tags, modified, *pModified))
				{
					subADB.tags = modified;
					subADB.comparisonMask = pModified->GenerateMask(modified);

					if (subADB.pTagDef == pOriginal)
					{
						subADB.knownTags = TAG_STATE_FULL;
					}
					else
					{
						subADB.knownTags = pModified->GetSharedTags(*subADB.pTagDef);
					}
				}
				else
				{
					RemoveDataFromParent(pCurrentDatabase, &pCurrentDatabase->m_subADBs[subADBIndex]);
					pCurrentDatabase->m_subADBs.erase(pCurrentDatabase->m_subADBs.begin() + subADBIndex);
				}
			}
		}
	}

	STagDefinitionImportsInfo& originalImports = mannequin::GetDefaultImportsInfo(pOriginal->GetFilename());
	originalImports.MapTags(*pOriginal, *pModified);

	// Do the save
	*(const_cast<CTagDefinition*>(pOriginal)) = *pModified;
}

void CAnimationDatabaseUpr::RenameTag(const CTagDefinition* pOriginal, i32 tagCRC, tukk newName)
{
	STagDefinitionImportsInfo& originalImports = mannequin::GetDefaultImportsInfo(pOriginal->GetFilename());

	// Determine which import (if any) contains the tag to rename
	TagID id = pOriginal->FindGroup(tagCRC);
	const STagDefinitionImportsInfo& root = originalImports.Find(id);
	//DrxLog("[TAGDEF]: root is %s", root.GetFilename());

	// For each tagdef we know about...
	for (TTagDefList::iterator it = m_tagDefs.begin(); it != m_tagDefs.end(); ++it)
	{
		std::vector<const STagDefinitionImportsInfo*> tagDefImportsInfo;
		STagDefinitionImportsInfo& tagDefImports = mannequin::GetDefaultImportsInfo(it->second->GetFilename());
		tagDefImports.FlattenImportsInfo(tagDefImportsInfo);

		// ...check if it imports the root...
		bool importsRoot = false;
		for (u32 index = 0, size = tagDefImportsInfo.size(); index < size; ++index)
		{
			if (stricmp(root.GetFilename(), tagDefImportsInfo[index]->GetFilename()) == 0)
			{
				importsRoot = true;
				break;
			}
		}

		// ...and if it does, rename all occurrences of the tag in the import "chain"
		if (importsRoot)
		{
			for (u32 index = 0, size = tagDefImportsInfo.size(); index < size; ++index)
			{
				const STagDefinitionImportsInfo* pImportsInfo = tagDefImportsInfo[index];
				CTagDefinition* pTagDef = stl::find_in_map(m_tagDefs, CCrc32::ComputeLowercase(pImportsInfo->GetFilename()), NULL);
				id = TAG_ID_INVALID;
				if (pTagDef != NULL)
				{
					id = pTagDef->Find(tagCRC);
				}
				if (id != TAG_ID_INVALID)
				{
					//DrxLog("[TAGDEF]: tag %p [%s]:[%s]->[%s]", pTagDef, pImportsInfo->GetFilename(), pTagDef->GetTagName(id), newName);
					pTagDef->SetTagName(id, newName);
				}
			}
		}
	}
};

void CAnimationDatabaseUpr::RenameTagGroup(const CTagDefinition* pOriginal, i32 tagGroupCRC, tukk newName)
{
	STagDefinitionImportsInfo& originalImports = mannequin::GetDefaultImportsInfo(pOriginal->GetFilename());

	// Determine which import (if any) contains the group to rename
	TagGroupID id = pOriginal->FindGroup(tagGroupCRC);
	const STagDefinitionImportsInfo& root = originalImports.Find(id);
	//DrxLog("[TAGDEF]: root is %s", root.GetFilename());

	// For each tagdef we know about...
	for (TTagDefList::iterator it = m_tagDefs.begin(); it != m_tagDefs.end(); ++it)
	{
		std::vector<const STagDefinitionImportsInfo*> tagDefImportsInfo;
		STagDefinitionImportsInfo& tagDefImports = mannequin::GetDefaultImportsInfo(it->second->GetFilename());
		tagDefImports.FlattenImportsInfo(tagDefImportsInfo);

		// ...check if it imports the root...
		bool importsRoot = false;
		for (u32 index = 0, size = tagDefImportsInfo.size(); index < size; ++index)
		{
			if (stricmp(root.GetFilename(), tagDefImportsInfo[index]->GetFilename()) == 0)
			{
				importsRoot = true;
				break;
			}
		}

		// ...and if it does, rename all occurrences of the group in the import "chain"
		if (importsRoot)
		{
			for (u32 index = 0, size = tagDefImportsInfo.size(); index < size; ++index)
			{
				const STagDefinitionImportsInfo* pImportsInfo = tagDefImportsInfo[index];
				CTagDefinition* pTagDef = stl::find_in_map(m_tagDefs, CCrc32::ComputeLowercase(pImportsInfo->GetFilename()), NULL);
				id = GROUP_ID_NONE;
				if (pTagDef != NULL)
				{
					id = pTagDef->FindGroup(tagGroupCRC);
				}
				if (id != GROUP_ID_NONE)
				{
					//DrxLog("[TAGDEF]: group %p [%s]:[%s]->[%s]", pTagDef, pImportsInfo->GetFilename(), pTagDef->GetGroupName(id), newName);
					pTagDef->SetGroupName(id, newName);
				}
			}
		}
	}
};

void CAnimationDatabaseUpr::GetIncludedTagDefs(const CTagDefinition* pQueriedTagDef, DynArray<CTagDefinition*>& outputTagDefs) const
{
	outputTagDefs.clear();

	if (!pQueriedTagDef)
	{
		return;
	}

	const STagDefinitionImportsInfo& queriedImportsInfo = mannequin::GetDefaultImportsInfo(pQueriedTagDef->GetFilename());
	std::vector<const STagDefinitionImportsInfo*> flatImportsInfoList;
	queriedImportsInfo.FlattenImportsInfo(flatImportsInfoList);

	for (u32 importIndex = 0, size = flatImportsInfoList.size(); importIndex < size; ++importIndex)
	{
		const STagDefinitionImportsInfo* pImportsInfo = flatImportsInfoList[importIndex];
		assert(pImportsInfo);
		CTagDefinition* pFoundTagDef = stl::find_in_map(m_tagDefs, CCrc32::ComputeLowercase(pImportsInfo->GetFilename()), NULL);

		if (pFoundTagDef && (pFoundTagDef != pQueriedTagDef))
		{
			outputTagDefs.push_back(pFoundTagDef);
		}
	}
}

void CAnimationDatabaseUpr::SaveSubADB(const CAnimationDatabase& animDB, const CAnimationDatabase::SSubADB& subAnimDB, const TFragmentSaveList& vFragSaveList, const TFragmentBlendSaveDatabase& mBlendSaveDatabase) const
{
	XmlNodeRef xmlSubNode = SaveDatabase(animDB, &subAnimDB, vFragSaveList, mBlendSaveDatabase);
	xmlSubNode->saveToFile(subAnimDB.filename.c_str());

	for (CAnimationDatabase::TSubADBList::const_iterator itSubADB = subAnimDB.subADBs.begin(); itSubADB != subAnimDB.subADBs.end(); ++itSubADB)
		SaveSubADB(animDB, *itSubADB, vFragSaveList, mBlendSaveDatabase);
}

void CAnimationDatabaseUpr::Save(const IAnimationDatabase& iAnimDB, tukk databaseName) const
{
	CAnimationDatabase& animDB = (CAnimationDatabase&)iAnimDB;
	TFragmentSaveList vFragSaveList;
	TFragmentBlendSaveDatabase mBlendSaveDatabase;

	XmlNodeRef xmlNode = SaveDatabase(animDB, NULL, vFragSaveList, mBlendSaveDatabase);
	xmlNode->saveToFile(databaseName);

	for (CAnimationDatabase::TSubADBList::const_iterator itSubADB = animDB.m_subADBs.begin(); itSubADB != animDB.m_subADBs.end(); ++itSubADB)
		SaveSubADB(animDB, *itSubADB, vFragSaveList, mBlendSaveDatabase);
}

//--- Helper functions

void CAnimationDatabaseLibrary::AnimEntryToXML(XmlNodeRef outXmlAnimEntry, const SAnimationEntry& animEntry, tukk name) const
{
	outXmlAnimEntry->setAttr("name", animEntry.animRef.c_str());
	if (animEntry.flags)
	{
		DrxStackStringT<char, 1024> sFlags;
		m_animFlags.IntegerFlagsToTagList(animEntry.flags, sFlags);
		outXmlAnimEntry->setAttr("flags", sFlags.c_str());
	}
	if (animEntry.playbackSpeed != 1.0f)
	{
		outXmlAnimEntry->setAttr("speed", animEntry.playbackSpeed);
	}
	if (animEntry.playbackWeight != 1.0f)
	{
		outXmlAnimEntry->setAttr("weight", animEntry.playbackWeight);
	}
	if (animEntry.weightList != 0)
	{
		outXmlAnimEntry->setAttr("weightList", animEntry.weightList);
	}
	char channelName[10];
	drx_strcpy(channelName, "channel0");
	for (u32 i = 0; i < MANN_NUMBER_BLEND_CHANNELS; i++)
	{
		if (animEntry.blendChannels[i] != 0.0f)
		{
			channelName[7] = '0' + i;
			outXmlAnimEntry->setAttr(channelName, animEntry.blendChannels[i]);
		}
	}
}

bool CAnimationDatabaseLibrary::XMLToAnimEntry(SAnimationEntry& animEntry, XmlNodeRef root) const
{
	bool success = true;
	animEntry.animRef.SetByString(root->getAttr("name"));
	success = success && m_animFlags.TagListToIntegerFlags(root->getAttr("flags"), animEntry.flags);
	animEntry.playbackSpeed = 1.0f;
	root->getAttr("speed", animEntry.playbackSpeed);
	animEntry.playbackWeight = 1.0f;
	root->getAttr("weight", animEntry.playbackWeight);
	root->getAttr("weightList", animEntry.weightList);

	char channelName[10];
	drx_strcpy(channelName, "channel0");
	for (u32 i = 0; i < MANN_NUMBER_BLEND_CHANNELS; i++)
	{
		channelName[7] = '0' + i;
		root->getAttr(channelName, animEntry.blendChannels[i]);
	}
	return success;
}

void CAnimationDatabaseLibrary::BlendToXML(XmlNodeRef outXmlBlend, const SAnimBlend& animBlend, tukk name) const
{
	outXmlBlend->setAttr("ExitTime", animBlend.exitTime);
	outXmlBlend->setAttr("StartTime", animBlend.startTime);
	outXmlBlend->setAttr("Duration", animBlend.duration);

	if (animBlend.flags)
	{
		DrxStackStringT<char, 1024> sFlags;
		m_animFlags.IntegerFlagsToTagList(animBlend.flags, sFlags);
		outXmlBlend->setAttr("flags", sFlags.c_str());
	}

	if (animBlend.terminal)
	{
		outXmlBlend->setAttr("terminal", true);
	}
}

void CAnimationDatabaseLibrary::XMLToBlend(SAnimBlend& animBlend, XmlNodeRef xmlBlend) const
{
	xmlBlend->getAttr("ExitTime", animBlend.exitTime);
	xmlBlend->getAttr("StartTime", animBlend.startTime);
	xmlBlend->getAttr("Duration", animBlend.duration);
	m_animFlags.TagListToIntegerFlags(xmlBlend->getAttr("flags"), animBlend.flags);
	xmlBlend->getAttr("terminal", animBlend.terminal);
}

void CAnimationDatabaseLibrary::ProceduralToXml(XmlNodeRef pOutNode, const SProceduralEntry& proceduralEntry) const
{
	const bool isNoneType = proceduralEntry.IsNoneType();
	tukk const typeName = isNoneType ? "" : mannequin::FindProcClipTypeName(proceduralEntry.typeNameHash);

	if (!typeName)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Unable to create xml node for procedural entry: Type '%u' is unknown!", proceduralEntry.typeNameHash.ToUInt32());
		return;
	}

	pOutNode->setAttr("type", typeName);

	if (proceduralEntry.pProceduralParams)
	{
		XmlNodeRef pXmlProceduralParams = Serialization::SaveXmlNode(*proceduralEntry.pProceduralParams, "ProceduralParams");
		if (pXmlProceduralParams)
		{
			pOutNode->addChild(pXmlProceduralParams);
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Unable to create 'ProceduralParams' node for procedural entry parameters for clip with type name '%s'", typeName);
		}
	}
}

bool CAnimationDatabaseLibrary::XmlToProcedural(XmlNodeRef pInNode, SProceduralEntry& outProceduralEntry) const
{
	DRX_ASSERT(gEnv && gEnv->pGameFramework);
	const IProceduralClipFactory& proceduralClipFactory = gEnv->pGameFramework->GetMannequinInterface().GetProceduralClipFactory();

	tukk const typeName = pInNode->getAttr("type");
	tukk const safeTypeName = typeName ? typeName : "";

	const IProceduralClipFactory::THash typeNameHash(safeTypeName);

	outProceduralEntry.typeNameHash = IProceduralClipFactory::THash();
	const bool isNoneType = typeNameHash.IsEmpty();
	if (!isNoneType)
	{
		outProceduralEntry.pProceduralParams = proceduralClipFactory.CreateProceduralClipParams(typeNameHash);
		if (!outProceduralEntry.pProceduralParams)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Could not create procedural clip parameters for procedural clip with type name '%s' and hash '%u' at line '%d'.", safeTypeName, typeNameHash.ToUInt32(), pInNode->getLine());
			return false;
		}

		XmlNodeRef pXmlProceduralParams = pInNode->findChild("ProceduralParams");

		{
			// Old procedural clip format conversion. Remove this whole block when support for old format is not desired anymore.
			if (!pXmlProceduralParams)
			{
				CProcClipConversionHelper helper;
				pXmlProceduralParams = helper.Convert(pInNode);
				if (pXmlProceduralParams)
				{
					DrxLogAlways("Converted old format procedural clip with type name '%s' at line '%d'.", safeTypeName, pInNode->getLine());
				}
				else
				{
					DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Failed to convert old format procedural clip with type name '%s' at line '%d'.", safeTypeName, pInNode->getLine());
					return false;
				}
			}
		}

		if (!pXmlProceduralParams)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Failed loading procedural parameters for clip with type name '%s' at line '%d': Unable to find 'ProceduralParams' xml node.", safeTypeName, pInNode->getLine());
			return false;
		}
		if (!Serialization::LoadXmlNode(*outProceduralEntry.pProceduralParams, pXmlProceduralParams))
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed loading procedural parameters for clip with type name '%s' at line '%d': Unable to create Xml Input Archive for serialization.", safeTypeName, pInNode->getLine());
			return false;
		}
	}

	outProceduralEntry.typeNameHash = typeNameHash;
	return true;
}

void CAnimationDatabaseLibrary::FragmentToXML(XmlNodeRef outXmlFrag, const CFragment* fragment, bool transition) const
{
	u32k numLayers = fragment->m_animLayers.size();
	for (u32 i = 0; i < numLayers; ++i)
	{
		const TAnimClipSequence& sequence = fragment->m_animLayers[i];

		XmlNodeRef xmlAnimLayer = outXmlFrag->createNode("AnimLayer");
		outXmlFrag->addChild(xmlAnimLayer);

		u32k numClips = sequence.size();
		for (u32 c = 0; c < numClips; c++)
		{
			const SAnimClip& animClip = sequence[c];

			XmlNodeRef xmlBlend = xmlAnimLayer->createNode("Blend");
			BlendToXML(xmlBlend, animClip.blend, "Blend");
			xmlAnimLayer->addChild(xmlBlend);

			const bool blendOnly = transition && (c == numClips - 1);
			if (!blendOnly)
			{
				XmlNodeRef xmlClip = xmlAnimLayer->createNode("Animation");
				AnimEntryToXML(xmlClip, animClip.animation, "Animation");
				xmlAnimLayer->addChild(xmlClip);
			}
		}
	}
	u32k numProcLayers = fragment->m_procLayers.size();
	for (u32 i = 0; i < numProcLayers; ++i)
	{
		const TProcClipSequence& sequence = fragment->m_procLayers[i];

		XmlNodeRef xmlProcLayer = outXmlFrag->createNode("ProcLayer");
		outXmlFrag->addChild(xmlProcLayer);

		u32k numClips = sequence.size();
		for (u32 c = 0; c < numClips; c++)
		{
			const SProceduralEntry& procClip = sequence[c];

			XmlNodeRef xmlBlend = xmlProcLayer->createNode("Blend");
			BlendToXML(xmlBlend, procClip.blend, "Blend");
			xmlProcLayer->addChild(xmlBlend);

			const bool blendOnly = transition && (c == numClips - 1);
			if (!blendOnly)
			{
				XmlNodeRef xmlClip = xmlProcLayer->createNode("Procedural");
				ProceduralToXml(xmlClip, procClip);
				xmlProcLayer->addChild(xmlClip);
			}
		}
	}

	outXmlFrag->setAttr("BlendOutDuration", fragment->m_blendOutDuration);
}

bool CAnimationDatabaseLibrary::XMLToFragment(CFragment& fragment, XmlNodeRef root, bool transition) const
{
	u32k numChildren = root->getChildCount();
	u32 numAnimLayers = 0;
	u32 numProcLayers = 0;

	root->getAttr("BlendOutDuration", fragment.m_blendOutDuration);

	for (u32 i = 0; i < numChildren; ++i)
	{
		XmlNodeRef xmlLayer = root->getChild(i);

		if (strcmp(xmlLayer->getTag(), "AnimLayer") == 0)
		{
			++numAnimLayers;
		}
		else if (strcmp(xmlLayer->getTag(), "ProcLayer") == 0)
		{
			++numProcLayers;
		}
		else
		{
			return false;
		}
	}

	fragment.m_animLayers.resize(numAnimLayers);
	fragment.m_procLayers.resize(numProcLayers);

	u32 animLayer = 0;
	u32 procLayer = 0;
	for (u32 i = 0; i < numChildren; ++i)
	{
		XmlNodeRef xmlLayer = root->getChild(i);

		if (strcmp(xmlLayer->getTag(), "AnimLayer") == 0)
		{
			u32k numEntries = xmlLayer->getChildCount();
			u32k transitionInc = transition ? 1 : 0;
			u32k numClips = (numEntries / 2) + transitionInc;

			TAnimClipSequence& sequence = fragment.m_animLayers[animLayer];
			sequence.resize(numClips);

			u32 clipNumber = 0;
			for (u32 e = 0; e < numEntries; ++e)
			{
				XmlNodeRef xmlChild = xmlLayer->getChild(e);
				SAnimClip& animClip = sequence[clipNumber];
				if (strcmp(xmlChild->getTag(), "Blend") == 0)
				{
					XMLToBlend(animClip.blend, xmlChild);
				}
				else if (strcmp(xmlChild->getTag(), "Animation") == 0)
				{
					XMLToAnimEntry(animClip.animation, xmlChild);
					++clipNumber;
				}
			}

			++animLayer;
		}
		else if (strcmp(xmlLayer->getTag(), "ProcLayer") == 0)
		{
			u32k numEntries = xmlLayer->getChildCount();
			u32k transitionInc = transition ? 1 : 0;
			u32k numClips = (numEntries / 2) + transitionInc;

			TProcClipSequence& sequence = fragment.m_procLayers[procLayer];
			sequence.resize(numClips);

			u32 clipNumber = 0;
			for (u32 e = 0; e < numEntries; ++e)
			{
				XmlNodeRef xmlChild = xmlLayer->getChild(e);
				SProceduralEntry& proceduralEntry = sequence[clipNumber];
				if (strcmp(xmlChild->getTag(), "Procedural") == 0)
				{
					XmlToProcedural(xmlChild, proceduralEntry);
					clipNumber++;
				}
				else if (strcmp(xmlChild->getTag(), "Blend") == 0)
				{
					XMLToBlend(proceduralEntry.blend, xmlChild);
				}
			}

			++procLayer;
		}
	}

	return true;
}

void CAnimationDatabaseLibrary::LoadDatabaseData
(
  CAnimationDatabase& animDB,
  const XmlNodeRef root,
  const CTagDefinition& fragIDs,
  const CTagDefinition& tagIDs,
  const TagState& adbFilter,
  bool recursive,
  CAnimationDatabase::SSubADB* subAnimDB
)
{
	if (recursive)
	{
		XmlNodeRef subADBs = root->findChild("SubADBs");
		u32k numSubADBs = subADBs ? subADBs->getChildCount() : 0;
		for (u32 i = 0; i < numSubADBs; ++i)
		{
			XmlNodeRef adbEntry = subADBs->getChild(i);
			tukk tags = adbEntry->getAttr("Tags");

			TagState subTags(TAG_STATE_EMPTY);
			bool tagsMatched = false;
			bool hasTags = (tags && strlen(tags) > 0);
			if (hasTags)
				tagsMatched = tagIDs.TagListToFlags(tags, subTags);

			char normalizedFilename[DEF_PATH_LENGTH];
			NormalizeFilename(normalizedFilename, adbEntry->getAttr("File"));

			CAnimationDatabase::SSubADB newSubADB;
			newSubADB.tags = tagIDs.GetUnion(adbFilter, subTags);
			newSubADB.comparisonMask = tagIDs.GenerateMask(newSubADB.tags);
			newSubADB.filename = normalizedFilename;

			DrxLog("Loading subADB %s", normalizedFilename);

			XmlNodeRef xmlData = gEnv->pSystem->GetXmlUtils()->LoadXmlFromFile(normalizedFilename);
			if (xmlData && LoadDatabaseDefinitions(xmlData, animDB.m_filename.c_str(), &newSubADB.pFragDef, &newSubADB.pTagDef))
			{
				//--- Create a mask of valid tags
				if (newSubADB.pTagDef == &tagIDs)
				{
					newSubADB.knownTags = TAG_STATE_FULL;
				}
				else
				{
					newSubADB.knownTags = tagIDs.GetSharedTags(*newSubADB.pTagDef);
				}

				i32 numFragIDs = adbEntry->getChildCount();
				for (i32 j = 0; j < numFragIDs; ++j)
				{
					XmlNodeRef FragIDEntry = adbEntry->getChild(j);
					tukk szFragmentName = FragIDEntry->getAttr("Name");
					if (szFragmentName && strlen(szFragmentName) > 0)
					{
						i32 fragmentID = newSubADB.pFragDef->Find(szFragmentName);
						if (fragmentID != TAG_ID_INVALID)
							newSubADB.vFragIDs.push_back(fragmentID);
					}
				}
				if (!tagsMatched && hasTags)
				{
					DrxLog("[CAnimationDatabaseUpr::LoadDatabase] Unknown tags %s for subADB %s", tags, adbEntry->getAttr("File"));
					continue;
				}

				LoadDatabaseData(animDB, xmlData, fragIDs, tagIDs, newSubADB.tags, recursive, &newSubADB);
			}
			else
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "!Could not load animation database (ADB) file '%s' (check the log for XML reader warnings)", normalizedFilename);
				continue;
			}

			if (!subAnimDB)
				animDB.m_subADBs.push_back(newSubADB);
			else
				subAnimDB->subADBs.push_back(newSubADB);

			SLICE_AND_SLEEP();
		}
	}

	if (root)
	{
		CFragment newFragment;
		XmlNodeRef fragmentList = root->findChild("FragmentList");
		u32 numChildren = fragmentList ? fragmentList->getChildCount() : 0;
		for (u32 i = 0; i < numChildren; ++i)
		{
			XmlNodeRef fragmentEntry = fragmentList->getChild(i);
			tukk szFragmentName = fragmentEntry->getTag();
			i32 fragmentID = fragIDs.Find(szFragmentName);

			if (fragmentID >= 0)
			{
				u32k numOptions = fragmentEntry->getChildCount();

				for (u32 k = 0; k < numOptions; ++k)
				{
					XmlNodeRef fragment = fragmentEntry->getChild(k);

					tukk globalTags = fragment->getAttr("Tags");
					tukk fragTags = fragment->getAttr("FragTags");
					SFragTagState tagState;
					bool tagsMatched = tagIDs.TagListToFlags(globalTags, tagState.globalTags);
					const CTagDefinition* pFragTagDef = fragIDs.GetSubTagDefinition(fragmentID);
					if (fragTags && pFragTagDef)
					{
						tagsMatched = tagsMatched && pFragTagDef->TagListToFlags(fragTags, tagState.fragmentTags);
					}

					if (tagsMatched)
					{
						newFragment.m_animLayers.clear();
						newFragment.m_procLayers.clear();
						if (!XMLToFragment(newFragment, fragment, false))
						{
							DrxLog("[CAnimationDatabaseUpr::LoadDatabase] Broken fragment entry for fragmentID %s tag %s fragTags %s", szFragmentName, globalTags, fragTags ? fragTags : "None");
						}
						else
						{
							animDB.AddEntry(fragmentID, tagState, newFragment);
						}
					}
					else
					{
						DrxLog("[CAnimationDatabaseUpr::LoadDatabase] Unknown tags for fragmentID %s tag %s fragTags %s", szFragmentName, globalTags, fragTags ? fragTags : "None");
					}
				}
			}
			else
			{
				DrxLog("[CAnimationDatabaseUpr::LoadDatabase] Warning missing fragmentID %s", szFragmentName);
			}
		}

		XmlNodeRef fragmentBlendList = root->findChild("FragmentBlendList");
		u32k numFragmentBlends = fragmentBlendList ? fragmentBlendList->getChildCount() : 0;
		for (u32 i = 0; i < numFragmentBlends; ++i)
		{
			XmlNodeRef xmlBlendEntry = fragmentBlendList->getChild(i);
			tukk fragFrom = xmlBlendEntry->getAttr("from");
			tukk fragTo = xmlBlendEntry->getAttr("to");
			i32 fragIDFrom = fragIDs.Find(fragFrom);
			i32 fragIDTo = fragIDs.Find(fragTo);

			const bool fragFromValid = (fragIDFrom != FRAGMENT_ID_INVALID) || (fragFrom[0] == '\0');
			const bool fragToValid = (fragIDTo != FRAGMENT_ID_INVALID) || (fragTo[0] == '\0');

			if (fragFromValid && fragToValid)
			{
				u32k numVariants = xmlBlendEntry->getChildCount();
				for (u32 v = 0; v < numVariants; ++v)
				{
					XmlNodeRef xmlVariant = xmlBlendEntry->getChild(v);
					tukk tagFrom = xmlVariant->getAttr("from");
					tukk tagTo = xmlVariant->getAttr("to");
					SFragTagState tsFrom, tsTo;
					bool tagStateMatched = tagIDs.TagListToFlags(tagFrom, tsFrom.globalTags);
					tagStateMatched = tagStateMatched && tagIDs.TagListToFlags(tagTo, tsTo.globalTags);

					const CTagDefinition* fragFromDef = (fragIDFrom != FRAGMENT_ID_INVALID) ? fragIDs.GetSubTagDefinition(fragIDFrom) : NULL;
					const CTagDefinition* fragToDef = (fragIDTo != FRAGMENT_ID_INVALID) ? fragIDs.GetSubTagDefinition(fragIDTo) : NULL;
					tukk fragTagFrom = xmlVariant->getAttr("fromFrag");
					tukk fragTagTo = xmlVariant->getAttr("toFrag");

					if (fragFromDef && fragTagFrom)
					{
						tagStateMatched = tagStateMatched && fragFromDef->TagListToFlags(fragTagFrom, tsFrom.fragmentTags);
					}
					if (fragToDef && fragTagTo)
					{
						tagStateMatched = tagStateMatched && fragToDef->TagListToFlags(fragTagTo, tsTo.fragmentTags);
					}

					if (tagStateMatched)
					{
						u32k numBlends = xmlVariant->getChildCount();
						for (u32 b = 0; b < numBlends; ++b)
						{
							TagState tempTags;
							XmlNodeRef xmlBlend = xmlVariant->getChild(b);
							SFragmentBlend fragBlend;
							xmlBlend->getAttr("selectTime", fragBlend.selectTime);
							xmlBlend->getAttr("startTime", fragBlend.startTime);
							xmlBlend->getAttr("enterTime", fragBlend.enterTime);
							tukk szBlendFlags = xmlBlend->getAttr("flags");

							if (szBlendFlags && szBlendFlags[0])
							{
								m_transitionFlags.TagListToIntegerFlags(szBlendFlags, fragBlend.flags);
							}

							fragBlend.pFragment = new CFragment();
							if ((xmlBlend->getChildCount() != 0) && !XMLToFragment(*fragBlend.pFragment, xmlBlend, true))
							{
								delete fragBlend.pFragment;
								DrxLog("[CAnimationDatabaseUpr::LoadDatabase] Broken fragment entry for blend");
							}
							else
							{
								animDB.AddBlendInternal(fragIDFrom, fragIDTo, tsFrom, tsTo, fragBlend);
							}
						}
					}
					else
					{
						DrxLog("[CAnimationDatabaseUpr::LoadDatabase] Unknown tags for blend %s to %s tag %s+%s to %s+%s", fragFrom, fragTo, tagFrom, fragTagFrom ? fragTagFrom : "None", tagTo, fragTagTo ? fragTagTo : "None");
					}
				}
			}
		}
	}
}

bool CAnimationDatabaseLibrary::LoadDatabaseDefinitions(const XmlNodeRef& root, tukk filename, const CTagDefinition** ppFragDefs, const CTagDefinition** ppTagDefs)
{
	tukk szBuffer;
	szBuffer = root->getAttr("Def");
	if (szBuffer && szBuffer[0])
	{
		const SControllerDef* def = gEnv->pGameFramework->GetMannequinInterface().GetAnimationDatabaseUpr().LoadControllerDef(szBuffer);
		if (def)
		{
			*ppFragDefs = &def->m_fragmentIDs;
			*ppTagDefs = &def->m_tags;
			return true;
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "!Missing def file for database %s - skipping load", filename);
			return false;
		}
	}

	szBuffer = root->getAttr("FragDef");
	if (szBuffer && szBuffer[0])
	{
		*ppFragDefs = gEnv->pGameFramework->GetMannequinInterface().GetAnimationDatabaseUpr().LoadTagDefs(szBuffer, false);
	}

	if (*ppFragDefs == NULL)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "!Missing fragment definition file for database %s - skipping load", filename);
		return false;
	}

	szBuffer = root->getAttr("TagDef");
	if (szBuffer && szBuffer[0])
	{
		*ppTagDefs = gEnv->pGameFramework->GetMannequinInterface().GetAnimationDatabaseUpr().LoadTagDefs(szBuffer, true);
	}

	if (*ppTagDefs == NULL)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "!Missing tag definition file for database %s - skipping load", filename);
		return false;
	}

	return true;
}

bool CAnimationDatabaseLibrary::LoadDatabase(const XmlNodeRef& root, CAnimationDatabase& animDB, bool recursive)
{
	if (stricmp(root->getTag(), "AnimDB") != 0)
	{
		return false;
	}

	if (LoadDatabaseDefinitions(root, animDB.m_filename.c_str(), &animDB.m_pFragDef, &animDB.m_pTagDef))
	{
		u32k numActions = animDB.m_pFragDef->GetNum();

		animDB.SetAutoSort(false);

		animDB.m_fragmentList.resize(numActions);
		for (u32 i = 0; i < numActions; ++i)
		{
			animDB.m_fragmentList[i] = new CAnimationDatabase::SFragmentEntry();
		}

		LoadDatabaseData(animDB, root, *animDB.m_pFragDef, *animDB.m_pTagDef, TAG_STATE_EMPTY, recursive);

		animDB.Sort();
		animDB.SetAutoSort(true);

		return true;
	}

	return false;
}

bool CAnimationDatabaseUpr::CanSaveFragmentID(FragmentID fragID, const CAnimationDatabase& animDB, const CAnimationDatabase::SSubADB* subAnimDB) const
{
	if (subAnimDB && (subAnimDB->pFragDef != animDB.m_pFragDef) && (fragID != FRAGMENT_ID_INVALID))
		return subAnimDB->pFragDef->Find(animDB.m_pFragDef->GetTagName(fragID)) != FRAGMENT_ID_INVALID;

	return true;
}

bool CAnimationDatabaseUpr::FragmentMatches(u32 fragCRC, const TagState& tagState, const CAnimationDatabase::SSubADB& subAnimDB, bool& bTagMatchFound) const
{
	//--- If this fragmentID is not known by this subADB, then it can't go here
	if ((fragCRC != 0) && (subAnimDB.pFragDef->Find(fragCRC) == FRAGMENT_ID_INVALID))
		return false;

	bTagMatchFound = false;
	bool bFragMatchFound = false;
	bool bMatchFound = false;
	if (!subAnimDB.tags.IsEmpty())
	{
		const TagState tagStateADB = subAnimDB.tags;
		const TagState maskADB = subAnimDB.comparisonMask;
		if (subAnimDB.pTagDef->Contains(tagState, tagStateADB, maskADB) && (subAnimDB.pFragDef->Find(fragCRC) != FRAGMENT_ID_INVALID))
			bTagMatchFound = ((subAnimDB.knownTags & tagState) == tagState);
	}

	if (!subAnimDB.vFragIDs.empty())
		for (CAnimationDatabase::SSubADB::TFragIDList::const_iterator itFID = subAnimDB.vFragIDs.begin(); itFID != subAnimDB.vFragIDs.end(); ++itFID)
			if (subAnimDB.pFragDef->GetTagCRC(*itFID) == fragCRC)
				bFragMatchFound = true;

	if ((subAnimDB.tags.IsEmpty() || bTagMatchFound) && (subAnimDB.vFragIDs.empty() || bFragMatchFound))
		bMatchFound = true;

	return bMatchFound;
}

bool CAnimationDatabaseUpr::ShouldSaveFragment(FragmentID fragID, const TagState& tagState, const CAnimationDatabase& animDB, const CAnimationDatabase::SSubADB* subAnimDB, bool& bTagMatchFound) const
{
	u32 fragCRC = animDB.m_pFragDef->GetTagCRC(fragID);
	bTagMatchFound = false;
	if (!subAnimDB || FragmentMatches(fragCRC, tagState, *subAnimDB, bTagMatchFound))
	{
		const CAnimationDatabase::TSubADBList& vSubADBs = subAnimDB ? subAnimDB->subADBs : animDB.m_subADBs;

		if (vSubADBs.empty())
		{
			return true;
		}

		bool bDummy;
		for (CAnimationDatabase::TSubADBList::const_iterator itSubADB = vSubADBs.begin(); itSubADB != vSubADBs.end(); ++itSubADB)
			if (FragmentMatches(fragCRC, tagState, *itSubADB, bDummy))
				return false;

		return true;
	}

	return !subAnimDB;
}

bool CAnimationDatabaseUpr::TransitionMatches(u32 fragCRCFrom, u32 fragCRCTo, const TagState& tagStateFrom, const TagState& tagStateTo, const CTagDefinition& parentTagDefs, const CAnimationDatabase::SSubADB& subAnimDB, bool& bTagMatchFound) const
{
	//--- If these tags are not known by this subADB, then it can't go here
	const TagState totalTags = parentTagDefs.GetUnion(tagStateFrom, tagStateTo);
	if (((subAnimDB.knownTags & totalTags) != totalTags)
	    || ((fragCRCFrom != 0) && (subAnimDB.pFragDef->Find(fragCRCFrom) == FRAGMENT_ID_INVALID))
	    || ((fragCRCTo != 0) && (subAnimDB.pFragDef->Find(fragCRCTo) == FRAGMENT_ID_INVALID)))
	{
		return false;
	}

	const TagState commonTags = parentTagDefs.GetIntersection(tagStateFrom, tagStateTo);
	bool bMatchFound = false;
	bTagMatchFound = false;
	bool bFragMatchFound = false;
	const TagState tagStateADB = subAnimDB.tags;
	const TagState maskADB = subAnimDB.comparisonMask;
	if (tagStateADB.IsEmpty() || parentTagDefs.Contains(commonTags, tagStateADB, maskADB))
	{
		bTagMatchFound = true;
	}

	if (!subAnimDB.vFragIDs.empty() && (fragCRCFrom != 0 || fragCRCTo != 0))
	{
		bool bFragMatchFoundFrom = false;
		bool bFragMatchFoundTo = false;
		for (CAnimationDatabase::SSubADB::TFragIDList::const_iterator itFID = subAnimDB.vFragIDs.begin(); itFID != subAnimDB.vFragIDs.end(); ++itFID)
		{
			if (subAnimDB.pFragDef->GetTagCRC(*itFID) == fragCRCFrom)
				bFragMatchFoundFrom = true;

			if (subAnimDB.pFragDef->GetTagCRC(*itFID) == fragCRCTo)
				bFragMatchFoundTo = true;
		}
		bFragMatchFound = (bFragMatchFoundFrom || !fragCRCFrom) && (bFragMatchFoundTo || !fragCRCTo);
	}

	if (bTagMatchFound && (subAnimDB.vFragIDs.empty() || bFragMatchFound))
		bMatchFound = true;

	return bMatchFound;
}

bool CAnimationDatabaseUpr::ShouldSaveTransition
(
  FragmentID fragIDFrom,
  FragmentID fragIDTo,
  const TagState& tagStateFrom,
  const TagState& tagStateTo,
  const CAnimationDatabase& animDB,
  const CAnimationDatabase::SSubADB* subAnimDB,
  bool& bTagMatchFound
) const
{
	//--- Special case rule: if this is an any to any then stick it at the top level, otherwise it'll go in the first match
	if ((fragIDFrom == FRAGMENT_ID_INVALID) && (fragIDTo == FRAGMENT_ID_INVALID))
	{
		return (subAnimDB == NULL);
	}

	u32 fragCRCFrom = (fragIDFrom == FRAGMENT_ID_INVALID) ? 0 : animDB.m_pFragDef->GetTagCRC(fragIDFrom);
	u32 fragCRCTo = (fragIDTo == FRAGMENT_ID_INVALID) ? 0 : animDB.m_pFragDef->GetTagCRC(fragIDTo);
	if (!subAnimDB || TransitionMatches(fragCRCFrom, fragCRCTo, tagStateFrom, tagStateTo, *animDB.m_pTagDef, *subAnimDB, bTagMatchFound))
	{
		const CAnimationDatabase::TSubADBList& vSubADBs = subAnimDB ? subAnimDB->subADBs : animDB.m_subADBs;

		if (vSubADBs.empty())
			return true;

		bool bSubTag;
		for (CAnimationDatabase::TSubADBList::const_iterator itSubADB = vSubADBs.begin(); itSubADB != vSubADBs.end(); ++itSubADB)
			if (TransitionMatches(fragCRCFrom, fragCRCTo, tagStateFrom, tagStateTo, *animDB.m_pTagDef, *itSubADB, bSubTag))
				return false;

		return true;
	}

	return !subAnimDB;
}

XmlNodeRef CAnimationDatabaseUpr::SaveDatabase
(
  const CAnimationDatabase& animDB,
  const CAnimationDatabase::SSubADB* subAnimDB,
  const TFragmentSaveList& vFragSaveList,
  const TFragmentBlendSaveDatabase& mBlendSaveDatabase,
  bool bFlatten
) const
{
	if (bFlatten && subAnimDB)
		return XmlNodeRef();

	string sMyFileName = subAnimDB ? subAnimDB->filename : animDB.m_filename;

	XmlNodeRef root = GetISystem()->CreateXmlNode("AnimDB");

	if (subAnimDB)
	{
		root->setAttr("FragDef", subAnimDB->pFragDef->GetFilename());
		root->setAttr("TagDef", subAnimDB->pTagDef->GetFilename());
	}
	else
	{
		root->setAttr("FragDef", animDB.m_pFragDef->GetFilename());
		root->setAttr("TagDef", animDB.m_pTagDef->GetFilename());
	}

	const CAnimationDatabase::TSubADBList& vSubADBs = subAnimDB ? subAnimDB->subADBs : animDB.m_subADBs;
	u32k numSubADBs = vSubADBs.size();
	if (!vSubADBs.empty())
	{
		XmlNodeRef subADBList;

		for (CAnimationDatabase::TSubADBList::const_iterator itSubADB = vSubADBs.begin(); itSubADB != vSubADBs.end(); ++itSubADB)
		{
			const CAnimationDatabase::SSubADB& subADB = *itSubADB;

			if (!subADBList)
			{
				subADBList = root->createNode("SubADBs");
				root->addChild(subADBList);
			}

			XmlNodeRef adbEntry = subADBList->createNode("SubADB");
			if (!subADB.tags.IsEmpty())
			{
				DrxStackStringT<char, 1024> sTags;
				animDB.m_pTagDef->FlagsToTagList(subADB.tags, sTags);
				adbEntry->setAttr("Tags", sTags.c_str());
			}
			adbEntry->setAttr("File", subADB.filename);
			subADBList->addChild(adbEntry);
			for (CAnimationDatabase::SSubADB::TFragIDList::const_iterator itFID = subADB.vFragIDs.begin(); itFID != subADB.vFragIDs.end(); ++itFID)
			{
				XmlNodeRef FragmentIDEntry = adbEntry->createNode("FragmentID");
				FragmentIDEntry->setAttr("Name", subADB.pFragDef->GetTagName(*itFID));
				adbEntry->addChild(FragmentIDEntry);
			}
		}
	}

	XmlNodeRef fragmentList = root->createNode("FragmentList");
	root->addChild(fragmentList);
	for (u32 i = 0; i < animDB.m_fragmentList.size(); ++i)
	{
		if (CanSaveFragmentID(i, animDB, subAnimDB))
		{
			CAnimationDatabase::SFragmentEntry* entry = animDB.m_fragmentList[i];
			u32k numTagSets = entry ? entry->tagSetList.Size() : 0;
			if (numTagSets > 0)
			{
				XmlNodeRef fragmentEntry;

				for (u32 k = 0; k < numTagSets; k++)
				{
					const string& saveFileName = vFragSaveList[i].vSaveStates[k].m_sFileName;
					if (saveFileName == sMyFileName || bFlatten)
					{
						if (!fragmentEntry)
						{
							fragmentEntry = root->createNode(animDB.m_pFragDef->GetTagName(i));
							fragmentList->addChild(fragmentEntry);
						}

						const SFragTagState& tagState = entry->tagSetList.m_keys[k];
						CAnimationDatabase::TFragmentOptionList& optionList = entry->tagSetList.m_values[k];
						u32k numOptions = optionList.size();
						for (u32 o = 0; o < numOptions; ++o)
						{
							XmlNodeRef fragment = fragmentEntry->createNode("Fragment");
							FragmentToXML(fragment, optionList[o].fragment, false);

							DrxStackStringT<char, 1024> sGlobalTags;
							animDB.m_pTagDef->FlagsToTagList(tagState.globalTags, sGlobalTags);
							fragment->setAttr("Tags", sGlobalTags.c_str());

							const CTagDefinition* fragTagDef = animDB.m_pFragDef->GetSubTagDefinition(i);
							if (fragTagDef && (tagState.fragmentTags != TAG_STATE_EMPTY))
							{
								DrxStackStringT<char, 1024> sFragmentTags;
								fragTagDef->FlagsToTagList(tagState.fragmentTags, sFragmentTags);
								fragment->setAttr("FragTags", sFragmentTags.c_str());
							}

							fragmentEntry->addChild(fragment);
						}
					}
				}
			}
		}
	}

	if (!animDB.m_fragmentBlendDB.empty())
	{
		XmlNodeRef fragmentBlendList = root->createNode("FragmentBlendList");
		bool hasAnyBlends = false;

		for (CAnimationDatabase::TFragmentBlendDatabase::const_iterator iter = animDB.m_fragmentBlendDB.begin(); iter != animDB.m_fragmentBlendDB.end(); ++iter)
		{
			const FragmentID fragIDFrom = iter->first.fragFrom;
			const FragmentID fragIDTo = iter->first.fragTo;

			if (CanSaveFragmentID(fragIDFrom, animDB, subAnimDB) && CanSaveFragmentID(fragIDTo, animDB, subAnimDB))
			{
				XmlNodeRef pFragment = root->createNode("Blend");
				pFragment->setAttr("from", (fragIDFrom == FRAGMENT_ID_INVALID) ? "" : animDB.m_pFragDef->GetTagName(fragIDFrom));
				pFragment->setAttr("to", (fragIDTo == FRAGMENT_ID_INVALID) ? "" : animDB.m_pFragDef->GetTagName(fragIDTo));

				const CTagDefinition* fragFromDef = (fragIDFrom != FRAGMENT_ID_INVALID) ? animDB.m_pFragDef->GetSubTagDefinition(fragIDFrom) : NULL;
				const CTagDefinition* fragToDef = (fragIDTo != FRAGMENT_ID_INVALID) ? animDB.m_pFragDef->GetSubTagDefinition(fragIDTo) : NULL;

				bool hasAnyEntries = false;

				u32k numVars = iter->second.variantList.size();
				for (u32 v = 0; v < numVars; ++v)
				{
					const CAnimationDatabase::SFragmentBlendVariant& variant = iter->second.variantList[v];

					TFragmentBlendSaveDatabase::const_iterator itSaveEntry = mBlendSaveDatabase.find(iter->first);
					if ((itSaveEntry != mBlendSaveDatabase.end() && itSaveEntry->second.vSaveStates[v].m_sFileName == sMyFileName) || bFlatten)
					{
						hasAnyEntries = true;
						hasAnyBlends = true;
						XmlNodeRef xmlFragmentVariant = root->createNode("Variant");
						pFragment->addChild(xmlFragmentVariant);

						DrxStackStringT<char, 1024> sGlobalTagsFrom;
						animDB.m_pTagDef->FlagsToTagList(variant.tagsFrom.globalTags, sGlobalTagsFrom);
						xmlFragmentVariant->setAttr("from", sGlobalTagsFrom.c_str());

						DrxStackStringT<char, 1024> sGlobalTagsTo;
						animDB.m_pTagDef->FlagsToTagList(variant.tagsTo.globalTags, sGlobalTagsTo);
						xmlFragmentVariant->setAttr("to", sGlobalTagsTo.c_str());

						if ((variant.tagsFrom.fragmentTags != TAG_STATE_EMPTY) && fragFromDef)
						{
							DrxStackStringT<char, 1024> sFragmentTags;
							fragFromDef->FlagsToTagList(variant.tagsFrom.fragmentTags, sFragmentTags);
							xmlFragmentVariant->setAttr("fromFrag", sFragmentTags.c_str());
						}

						if ((variant.tagsTo.fragmentTags != TAG_STATE_EMPTY) && fragToDef)
						{
							DrxStackStringT<char, 1024> sFragmentTags;
							fragToDef->FlagsToTagList(variant.tagsTo.fragmentTags, sFragmentTags);
							xmlFragmentVariant->setAttr("toFrag", sFragmentTags.c_str());
						}

						u32k numBlends = variant.blendList.size();
						for (u32 b = 0; b < numBlends; ++b)
						{
							const SFragmentBlend& blend = variant.blendList[b];

							XmlNodeRef xmlFragment = xmlFragmentVariant->createNode("Fragment");
							FragmentToXML(xmlFragment, blend.pFragment, true);
							xmlFragmentVariant->addChild(xmlFragment);
							xmlFragment->setAttr("selectTime", blend.selectTime);
							if (blend.startTime != 0.0f)
							{
								xmlFragment->setAttr("startTime", blend.startTime);
							}
							xmlFragment->setAttr("enterTime", blend.enterTime);

							if (blend.flags != 0)
							{
								DrxStackStringT<char, 1024> sFlags;
								m_transitionFlags.IntegerFlagsToTagList(blend.flags, sFlags);
								xmlFragment->setAttr("flags", sFlags.c_str());
							}
						}
					}
				}

				if (hasAnyEntries)
				{
					fragmentBlendList->addChild(pFragment);
				}
			}
		}

		if (hasAnyBlends)
		{
			root->addChild(fragmentBlendList);
		}
	}

	return root;
}

SControllerDef* CAnimationControllerDefLibrary::LoadControllerDef(const XmlNodeRef& root, tukk context)
{
	if (stricmp(root->getTag(), "ControllerDef") != 0)
	{
		return NULL;
	}

	tukk szBuffer;
	const CTagDefinition* tagTD = NULL;
	const CTagDefinition* fragmentTD = NULL;
	CTagDefinition scopeTD;

	XmlNodeRef tagList = root->findChild("Tags");
	szBuffer = tagList ? tagList->getAttr("filename") : NULL;
	if (szBuffer)
	{
		tagTD = gEnv->pGameFramework->GetMannequinInterface().GetAnimationDatabaseUpr().LoadTagDefs(szBuffer, true);
	}
	XmlNodeRef fragList = root->findChild("Fragments");
	szBuffer = fragList ? fragList->getAttr("filename") : NULL;
	if (szBuffer)
	{
		fragmentTD = gEnv->pGameFramework->GetMannequinInterface().GetAnimationDatabaseUpr().LoadTagDefs(szBuffer, false);
	}

	{
		XmlNodeRef scopeDefList = root->findChild("ScopeDefs");
		if (scopeDefList)
		{
			i32k scopeCount = scopeDefList->getChildCount();
			for (i32 i = 0; i < scopeCount; ++i)
			{
				XmlNodeRef xmlScopeNode = scopeDefList->getChild(i);
				tukk const scopeName = xmlScopeNode->getTag();
				scopeTD.AddTag(scopeName);
			}
			scopeTD.AssignBits();
		}
	}

	if (tagTD && fragmentTD)
	{
		SControllerDef* ret = new SControllerDef(*tagTD, *fragmentTD, scopeTD);

		ret->m_fragmentDef.resize(fragmentTD->GetNum());
		ret->m_scopeDef.resize(scopeTD.GetNum());

		XmlNodeRef subContextList = root->findChild("SubContexts");
		u32k numSubContexts = subContextList ? subContextList->getChildCount() : 0;
		for (u32 i = 0; i < numSubContexts; i++)
		{
			XmlNodeRef xmlSubContextEntry = subContextList->getChild(i);
			ret->m_subContextIDs.AddTag(xmlSubContextEntry->getTag());

			SSubContext subContext;
			tagTD->TagListToFlags(xmlSubContextEntry->getAttr("tags"), subContext.additionalTags);
			scopeTD.TagListToIntegerFlags(xmlSubContextEntry->getAttr("scopes"), subContext.scopeMask);
			ret->m_subContext.push_back(subContext);
		}

		XmlNodeRef fragDefList = root->findChild("FragmentDefs");
		DynArray<bool> loadedFragmentDefs(fragmentTD->GetNum(), false);
		u32k numFragDefs = fragDefList ? fragDefList->getChildCount() : 0;
		for (u32 i = 0; i < numFragDefs; ++i)
		{
			XmlNodeRef xmlFragDefEntry = fragDefList->getChild(i);

			u32 fragID = fragmentTD->Find(xmlFragDefEntry->getTag());
			if (fragID != FRAGMENT_ID_INVALID)
			{
				if (!loadedFragmentDefs[fragID])
				{
					loadedFragmentDefs[fragID] = true;
					tukk fragDefFlags = xmlFragDefEntry->getAttr("flags");
					ActionScopes scopeMask = 0;
					scopeTD.TagListToIntegerFlags(xmlFragDefEntry->getAttr("scopes"), scopeMask);
					SFragmentDef& fragmentDef = ret->m_fragmentDef[fragID];
					SFragTagState fragTagState;
					fragmentDef.scopeMaskList.Insert(fragTagState, scopeMask);
					if (fragDefFlags && fragDefFlags[0])
					{
						m_fragmentFlags.TagListToIntegerFlags(fragDefFlags, fragmentDef.flags);
					}
					const CTagDefinition* pFragTagDef = fragmentTD->GetSubTagDefinition(fragID);

					u32k numTags = xmlFragDefEntry->getChildCount();
					for (u32 t = 0; t < numTags; ++t)
					{
						XmlNodeRef xmlFragTag = xmlFragDefEntry->getChild(t);

						tukk tags = xmlFragTag->getAttr("tags");
						tukk fragTags = xmlFragTag->getAttr("fragTags");
						tukk scopes = xmlFragTag->getAttr("scopes");

						bool success = tagTD->TagListToFlags(tags, fragTagState.globalTags);
						fragTagState.fragmentTags = TAG_STATE_EMPTY;
						if (pFragTagDef && fragTags)
						{
							success = success && pFragTagDef->TagListToFlags(fragTags, fragTagState.fragmentTags);
						}
						success = success && scopeTD.TagListToIntegerFlags(scopes, scopeMask);

						if (success)
						{
							fragmentDef.scopeMaskList.Insert(fragTagState, scopeMask);
						}
					}

					fragmentDef.scopeMaskList.Sort(*tagTD, pFragTagDef);
				}
				else
				{
					DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Skipping duplicate fragment '%s' in %s", xmlFragDefEntry->getTag(), context);
				}
			}
			else
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Skipping unknown fragment '%s' in %s", xmlFragDefEntry->getTag(), context);
			}
		}

		//--- Load in explicit scope contexts if they exist
		XmlNodeRef scopeContextDefList = root->findChild("ScopeContextDefs");
		if (scopeContextDefList)
		{
			u32k numScopeContextDefs = scopeContextDefList->getChildCount();
			ret->m_scopeContextDef.reserve(numScopeContextDefs);
			for (u32 i = 0; i < numScopeContextDefs; ++i)
			{
				XmlNodeRef xmlScopeContextDefEntry = scopeContextDefList->getChild(i);
				tukk contextName = xmlScopeContextDefEntry->getTag();
				if (ret->m_scopeContexts.Find(contextName) >= 0)
				{
					DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Duplicate scope context '%s' referenced in scopecontextdefs in %s", contextName, context);
				}
				else
				{
					TagID contextID = ret->m_scopeContexts.AddTag(contextName);
					ret->m_scopeContextDef.push_back();
					SScopeContextDef& contextDef = ret->m_scopeContextDef[contextID];
					if (xmlScopeContextDefEntry->haveAttr("tags"))
					{
						tagTD->TagListToFlags(xmlScopeContextDefEntry->getAttr("tags"), contextDef.additionalTags);
					}
					if (xmlScopeContextDefEntry->haveAttr("sharedTags"))
					{
						tagTD->TagListToFlags(xmlScopeContextDefEntry->getAttr("sharedTags"), contextDef.sharedTags);
					}
				}
			}
		}

		XmlNodeRef scopeDefList = root->findChild("ScopeDefs");
		DynArray<bool> loadedScopeDefs(scopeTD.GetNum(), false);
		u32k numScopeDefs = scopeDefList ? scopeDefList->getChildCount() : 0;

		assert(numScopeDefs <= sizeof(ActionScopes) * 8);

		for (u32 i = 0; i < numScopeDefs; ++i)
		{
			XmlNodeRef xmlScopeDefEntry = scopeDefList->getChild(i);

			i32 scopeID = scopeTD.Find(xmlScopeDefEntry->getTag());
			if (scopeID < 0)
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Skipping unknown scope '%s' referenced in scopedefs in %s", xmlScopeDefEntry->getTag(), context);
			}
			else
			{
				if (!loadedScopeDefs[scopeID])
				{
					xmlScopeDefEntry->getAttr("layer", ret->m_scopeDef[scopeID].layer);
					xmlScopeDefEntry->getAttr("numLayers", ret->m_scopeDef[scopeID].numLayers);
					tukk szScopeAlias = xmlScopeDefEntry->getAttr("scopeAlias");
					ret->m_scopeDef[scopeID].scopeAlias = (szScopeAlias && szScopeAlias[0]) ? szScopeAlias : scopeTD.GetTagName(scopeID);
					tukk contextName = xmlScopeDefEntry->getAttr("context");
					tagTD->TagListToFlags(xmlScopeDefEntry->getAttr("Tags"), ret->m_scopeDef[scopeID].additionalTags);
					i32 contextID = ret->m_scopeContexts.Find(contextName);
					if (contextID < 0)
					{
						contextID = ret->m_scopeContexts.AddTag(contextName);
						ret->m_scopeContextDef.push_back();
					}
					ret->m_scopeDef[scopeID].context = contextID;
				}
				else
				{
					DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Skipping duplicate scope '%s' in %s", xmlScopeDefEntry->getTag(), context);
				}
			}
		}

		return ret;
	}

	DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Tags, Fragments or Scopes missing in %s", context);
	return NULL;
}

XmlNodeRef CAnimationDatabaseUpr::SaveControllerDef(const SControllerDef& controllerDef) const
{
	XmlNodeRef root = GetISystem()->CreateXmlNode("ControllerDef");

	XmlNodeRef tagList = root->createNode("Tags");
	root->addChild(tagList);
	tagList->setAttr("filename", controllerDef.m_tags.GetFilename());

	XmlNodeRef fragList = root->createNode("Fragments");
	root->addChild(fragList);
	fragList->setAttr("filename", controllerDef.m_fragmentIDs.GetFilename());

	u32k numTagContexts = controllerDef.m_subContextIDs.GetNum();
	if (numTagContexts > 0)
	{
		XmlNodeRef subContextList = root->createNode("SubContexts");
		root->addChild(subContextList);

		for (u32 i = 0; i < numTagContexts; i++)
		{
			const SSubContext& subContext = controllerDef.m_subContext[i];
			XmlNodeRef xmlSubContextEntry = root->createNode(controllerDef.m_subContextIDs.GetTagName(i));
			subContextList->addChild(xmlSubContextEntry);

			if (subContext.scopeMask != ACTION_SCOPES_NONE)
			{
				DrxStackStringT<char, 1024> sScopeMask;
				controllerDef.m_scopeIDs.IntegerFlagsToTagList(subContext.scopeMask, sScopeMask);
				xmlSubContextEntry->setAttr("scopes", sScopeMask.c_str());
			}

			if (subContext.additionalTags != TAG_STATE_EMPTY)
			{
				DrxStackStringT<char, 1024> sAdditionalTags;
				controllerDef.m_tags.FlagsToTagList(subContext.additionalTags, sAdditionalTags);
				xmlSubContextEntry->setAttr("tags", sAdditionalTags.c_str());
			}
		}
	}

	XmlNodeRef tagDefList = root->createNode("FragmentDefs");
	root->addChild(tagDefList);
	u32k numFrags = controllerDef.m_fragmentDef.size();
	for (u32 i = 0; i < numFrags; ++i)
	{
		const SFragmentDef& fragDef = controllerDef.m_fragmentDef[i];
		const CTagDefinition* pFragTagDef = controllerDef.GetFragmentTagDef(i);
		ActionScopes scopeMask = fragDef.scopeMaskList.GetDefault();

		XmlNodeRef xmlFragDef = tagDefList->createNode(controllerDef.m_fragmentIDs.GetTagName(i));
		tagDefList->addChild(xmlFragDef);

		if (scopeMask != 0)
		{
			DrxStackStringT<char, 1024> sScopeMask;
			controllerDef.m_scopeIDs.IntegerFlagsToTagList(scopeMask, sScopeMask);
			xmlFragDef->setAttr("scopes", sScopeMask.c_str());
		}

		//if (fragDef.tagDef)
		//{
		//	xmlFragDef->setAttr("tags", fragDef.tagDef->GetFilename());
		//}
		if (fragDef.flags)
		{
			DrxStackStringT<char, 1024> sFlags;
			m_fragmentFlags.IntegerFlagsToTagList(fragDef.flags, sFlags);
			xmlFragDef->setAttr("flags", sFlags.c_str());
		}

		u32k numTags = fragDef.scopeMaskList.Size();
		for (u32 t = 0; t < numTags - 1; ++t)
		{
			const SFragTagState& fragTagState = fragDef.scopeMaskList.m_keys[t];
			const ActionScopes& actionScope = fragDef.scopeMaskList.m_values[t];

			XmlNodeRef xmlOverrideDef = tagDefList->createNode("Override");
			xmlFragDef->addChild(xmlOverrideDef);

			DrxStackStringT<char, 1024> sGlobalTags;
			controllerDef.m_tags.FlagsToTagList(fragTagState.globalTags, sGlobalTags);
			xmlOverrideDef->setAttr("tags", sGlobalTags.c_str());

			if (pFragTagDef && (fragTagState.fragmentTags != TAG_STATE_EMPTY))
			{
				DrxStackStringT<char, 1024> sFragmentTags;
				pFragTagDef->FlagsToTagList(fragTagState.fragmentTags, sFragmentTags);
				xmlOverrideDef->setAttr("fragTags", sFragmentTags.c_str());
			}

			DrxStackStringT<char, 1024> sActionScope;
			controllerDef.m_scopeIDs.IntegerFlagsToTagList(actionScope, sActionScope);
			xmlOverrideDef->setAttr("scopes", sActionScope.c_str());
		}
	}

	XmlNodeRef scopeContextDefList = root->createNode("ScopeContextDefs");
	root->addChild(scopeContextDefList);
	u32k numScopeContexts = controllerDef.m_scopeContextDef.size();
	for (u32 i = 0; i < numScopeContexts; ++i)
	{
		const SScopeContextDef& scopeContextDef = controllerDef.m_scopeContextDef[i];
		XmlNodeRef xmlScopeContextDef = scopeContextDefList->createNode(controllerDef.m_scopeContexts.GetTagName(i));
		scopeContextDefList->addChild(xmlScopeContextDef);
		if (scopeContextDef.additionalTags != TAG_STATE_EMPTY)
		{
			DrxStackStringT<char, 1024> sAdditionalTags;
			controllerDef.m_tags.FlagsToTagList(scopeContextDef.additionalTags, sAdditionalTags);
			xmlScopeContextDef->setAttr("tags", sAdditionalTags.c_str());
		}
		if (scopeContextDef.sharedTags != TAG_STATE_FULL)
		{
			DrxStackStringT<char, 1024> sSharedTags;
			controllerDef.m_tags.FlagsToTagList(scopeContextDef.sharedTags, sSharedTags);
			xmlScopeContextDef->setAttr("sharedTags", sSharedTags.c_str());
		}
	}

	XmlNodeRef scopeDefList = root->createNode("ScopeDefs");
	root->addChild(scopeDefList);
	u32k numScopes = controllerDef.m_scopeDef.size();
	for (u32 i = 0; i < numScopes; ++i)
	{
		const SScopeDef& scopeDef = controllerDef.m_scopeDef[i];

		XmlNodeRef xmlScopeDef = scopeDefList->createNode(controllerDef.m_scopeIDs.GetTagName(i));
		scopeDefList->addChild(xmlScopeDef);
		xmlScopeDef->setAttr("layer", scopeDef.layer);
		xmlScopeDef->setAttr("numLayers", scopeDef.numLayers);
		if (0 != strcmp(scopeDef.scopeAlias.c_str(), controllerDef.m_scopeIDs.GetTagName(i)))
		{
			xmlScopeDef->setAttr("scopeAlias", scopeDef.scopeAlias.c_str());
		}
		tukk scopeContextName = controllerDef.m_scopeContexts.GetTagName(scopeDef.context);
		xmlScopeDef->setAttr("context", scopeContextName);
		if (scopeDef.additionalTags != TAG_STATE_EMPTY)
		{
			DrxStackStringT<char, 1024> sAdditionalTags;
			controllerDef.m_tags.FlagsToTagList(scopeDef.additionalTags, sAdditionalTags);
			xmlScopeDef->setAttr("Tags", sAdditionalTags.c_str());
		}
	}
	return root;
}

EModifyFragmentIdResult CAnimationDatabaseUpr::CreateFragmentID(const CTagDefinition& inFragmentIds, tukk szFragmentIdName)
{
	const bool validFragmentIdName = IsValidNameIdentifier(szFragmentIdName);
	if (!validFragmentIdName)
	{
		return eMFIR_InvalidNameIdentifier;
	}

	tukk fragmentDefFilename = inFragmentIds.GetFilename();
	u32k fragmentDefFilenameCrc = CCrc32::ComputeLowercase(fragmentDefFilename);

	CTagDefinition* fragmentDefs = stl::find_in_map(m_tagDefs, fragmentDefFilenameCrc, NULL);
	assert(fragmentDefs);
	if (!fragmentDefs)
	{
		return eMFIR_UnknownInputTagDefinition;
	}

	i32k fragmentTagId = fragmentDefs->Find(szFragmentIdName);
	const bool fragmentTagIdFound = (fragmentTagId != FRAGMENT_ID_INVALID);
	if (fragmentTagIdFound)
	{
		return eMFIR_DuplicateName;
	}

	i32k newFragmentTagId = fragmentDefs->AddTag(szFragmentIdName);
	assert(newFragmentTagId != FRAGMENT_ID_INVALID);

	for (TControllerDefList::const_iterator cit = m_controllerDefs.begin(); cit != m_controllerDefs.end(); ++cit)
	{
		SControllerDef* controllerDef = cit->second;
		tukk controllerFragmentDefFilename = controllerDef->m_fragmentIDs.GetFilename();
		u32k controllerFragmentDefFilenameCrc = CCrc32::ComputeLowercase(controllerFragmentDefFilename);
		const bool usingSameFragmentDef = (controllerFragmentDefFilenameCrc == fragmentDefFilenameCrc);
		if (usingSameFragmentDef)
		{
			controllerDef->m_fragmentDef.push_back(SFragmentDef());
		}
	}

	for (TAnimDatabaseList::const_iterator cit = m_databases.begin(); cit != m_databases.end(); ++cit)
	{
		CAnimationDatabase* otherAnimDB = cit->second;
		tukk otherDBFragmentDefFilename = otherAnimDB->GetFragmentDefs().GetFilename();
		u32k otherDBFragmentDefFilenameCrc = CCrc32::ComputeLowercase(otherDBFragmentDefFilename);
		const bool usingSameFragmentDef = (otherDBFragmentDefFilenameCrc == fragmentDefFilenameCrc);
		if (usingSameFragmentDef)
		{
			CAnimationDatabase::SFragmentEntry* fragmentEntry = new CAnimationDatabase::SFragmentEntry();
			assert(otherAnimDB->m_fragmentList.size() == newFragmentTagId);
			otherAnimDB->m_fragmentList.push_back(fragmentEntry);
		}
	}

	NotifyListenersTagDefinitionChanged(*fragmentDefs);
	return eMFIR_Success;
}

EModifyFragmentIdResult CAnimationDatabaseUpr::RenameFragmentID(const CTagDefinition& fragmentIds, FragmentID fragmentID, tukk szFragmentIdName)
{
	if (fragmentID == FRAGMENT_ID_INVALID)
	{
		return eMFIR_InvalidFragmentId;
	}

	const FragmentID fragmentIDCount = fragmentIds.GetNum();
	if (fragmentIDCount <= fragmentID)
	{
		return eMFIR_InvalidFragmentId;
	}

	const bool validFragmentIdName = IsValidNameIdentifier(szFragmentIdName);
	if (!validFragmentIdName)
	{
		return eMFIR_InvalidNameIdentifier;
	}

	const FragmentID foundFragmentIdWithDesiredName = fragmentIds.Find(szFragmentIdName);
	const bool duplicateFragmentIdName = ((foundFragmentIdWithDesiredName != FRAGMENT_ID_INVALID) && (foundFragmentIdWithDesiredName != fragmentID));
	if (duplicateFragmentIdName)
	{
		return eMFIR_DuplicateName;
	}

	tukk fragmentDefFilename = fragmentIds.GetFilename();
	u32k fragmentDefFilenameCrc = CCrc32::ComputeLowercase(fragmentDefFilename);

	CTagDefinition* fragmentDefs = stl::find_in_map(m_tagDefs, fragmentDefFilenameCrc, NULL);
	assert(fragmentDefs);
	assert(fragmentDefs == &fragmentIds);
	if (!fragmentDefs)
	{
		return eMFIR_UnknownInputTagDefinition;
	}

	fragmentDefs->SetTagName(fragmentID, szFragmentIdName);

	NotifyListenersTagDefinitionChanged(*fragmentDefs);
	return eMFIR_Success;
}

EModifyFragmentIdResult CAnimationDatabaseUpr::DeleteFragmentID(const CTagDefinition& fragmentIds, FragmentID fragmentID)
{
	if (fragmentID == FRAGMENT_ID_INVALID)
		return eMFIR_InvalidFragmentId;

	const FragmentID fragmentIDCount = fragmentIds.GetNum();
	if (fragmentIDCount <= fragmentID)
		return eMFIR_InvalidFragmentId;

	tukk fragmentDefFilename = fragmentIds.GetFilename();
	u32k fragmentDefFilenameCrc = CCrc32::ComputeLowercase(fragmentDefFilename);

	CTagDefinition* fragmentDefs = stl::find_in_map(m_tagDefs, fragmentDefFilenameCrc, NULL);
	assert(fragmentDefs);
	assert(fragmentDefs == &fragmentIds);
	if (!fragmentDefs)
		return eMFIR_UnknownInputTagDefinition;

	fragmentDefs->RemoveTag(fragmentID);

	for (TControllerDefList::const_iterator cit = m_controllerDefs.begin(); cit != m_controllerDefs.end(); ++cit)
	{
		SControllerDef* controllerDef = cit->second;
		tukk controllerFragmentDefFilename = controllerDef->m_fragmentIDs.GetFilename();
		u32k controllerFragmentDefFilenameCrc = CCrc32::ComputeLowercase(controllerFragmentDefFilename);
		const bool usingSameFragmentDef = (controllerFragmentDefFilenameCrc == fragmentDefFilenameCrc);
		if (usingSameFragmentDef)
		{
			controllerDef->m_fragmentDef.erase(fragmentID);
		}
	}

	for (TAnimDatabaseList::const_iterator it = m_databases.begin(), itEnd = m_databases.end(); it != itEnd; ++it)
	{
		CAnimationDatabase* database = it->second;
		tukk databaseFragmentDefFilename = database->GetFragmentDefs().GetFilename();
		u32k databaseFragmentDefFilenameCrc = CCrc32::ComputeLowercase(databaseFragmentDefFilename);
		const bool usingSameFragmentDef = (databaseFragmentDefFilenameCrc == fragmentDefFilenameCrc);
		if (usingSameFragmentDef)
		{
			database->DeleteFragmentID(fragmentID);
		}
	}

	NotifyListenersTagDefinitionChanged(*fragmentDefs);
	return eMFIR_Success;
}

bool CAnimationDatabaseUpr::SetFragmentTagDef(const CTagDefinition& fragmentIds, FragmentID fragmentID, const CTagDefinition* pFragTagDefs)
{
	if (fragmentID == FRAGMENT_ID_INVALID)
		return false;

	tukk fragmentDefFilename = fragmentIds.GetFilename();
	u32k fragmentDefFilenameCrc = CCrc32::ComputeLowercase(fragmentDefFilename);

	CTagDefinition* fragmentDefs = stl::find_in_map(m_tagDefs, fragmentDefFilenameCrc, NULL);
	assert(fragmentDefs);
	assert(fragmentDefs == &fragmentIds);
	if (fragmentDefs)
	{
		fragmentDefs->SetSubTagDefinition(fragmentID, pFragTagDefs);
		return true;
	}

	return false;
}

void CAnimationDatabaseUpr::SetFragmentDef(const SControllerDef& inControllerDef, FragmentID fragmentID, const SFragmentDef& fragmentDef)
{
	if (fragmentID == FRAGMENT_ID_INVALID)
	{
		return;
	}

	tukk controllerDefFilename = inControllerDef.m_filename.c_str();
	u32k controllerDefFilenameCrc = CCrc32::ComputeLowercase(controllerDefFilename);

	SControllerDef* controllerDef = stl::find_in_map(m_controllerDefs, controllerDefFilenameCrc, NULL);
	assert(controllerDef);
	if (!controllerDef)
	{
		return;
	}

	DynArray<SFragmentDef>& fragmentDefs = controllerDef->m_fragmentDef;
	if (fragmentDefs.size() <= fragmentID)
	{
		return;
	}

	fragmentDefs[fragmentID] = fragmentDef;
}

bool CAnimationDatabaseUpr::DeleteFragmentEntry(IAnimationDatabase* pDatabaseInterface, FragmentID fragmentID, const SFragTagState& tagState, u32 optionIdx, bool logWarning)
{
	assert(pDatabaseInterface);
	CAnimationDatabase* pDatabase = static_cast<CAnimationDatabase*>(pDatabaseInterface);

	bool successfullyDeleted = pDatabase->DeleteEntry(fragmentID, tagState, optionIdx);

	if (logWarning && !successfullyDeleted)
	{
		DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, "AnimDatabaseUpr: Invalid fragment entry: %d passed to %s", fragmentID, pDatabase->GetFilename());
	}

	i32 fragCRC = pDatabase->m_pFragDef->GetTagCRC(fragmentID);

	DrxStackStringT<char, 1024> sTaglist;
	pDatabase->m_pTagDef->FlagsToTagList(tagState.globalTags, sTaglist);
	SFragTagState targetTagState = tagState;

	const std::vector<CAnimationDatabase*>& impactedDatabases = FindImpactedDatabases(pDatabase, fragmentID, tagState.globalTags);
	for (std::vector<CAnimationDatabase*>::const_iterator itDatabases = impactedDatabases.begin(); itDatabases != impactedDatabases.end(); ++itDatabases)
	{
		CAnimationDatabase* pCurrDatabase = *itDatabases;
		if (pCurrDatabase != pDatabase)
		{
			pCurrDatabase->m_pTagDef->TagListToFlags(sTaglist.c_str(), targetTagState.globalTags);
			FragmentID targetFragID = pCurrDatabase->m_pFragDef->Find(fragCRC);

			bool success = pCurrDatabase->DeleteEntry(targetFragID, targetTagState, optionIdx);
			if (logWarning && !success)
			{
				DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, "AnimDatabaseUpr: Invalid fragment entry: %d passed to %s", fragmentID, pCurrDatabase->GetFilename());
			}
		}
	}

	return successfullyDeleted;
}

u32 CAnimationDatabaseUpr::AddFragmentEntry(IAnimationDatabase* pDatabaseInterface, FragmentID fragmentID, const SFragTagState& tagState, const CFragment& fragment)
{
	assert(pDatabaseInterface);
	CAnimationDatabase* pDatabase = static_cast<CAnimationDatabase*>(pDatabaseInterface);

	u32 idxRoot = pDatabase->AddEntry(fragmentID, tagState, fragment);

	i32 fragCRC = pDatabase->m_pFragDef->GetTagCRC(fragmentID);

	DrxStackStringT<char, 1024> sTaglist;
	pDatabase->m_pTagDef->FlagsToTagList(tagState.globalTags, sTaglist);
	SFragTagState targetTagState = tagState;

	const std::vector<CAnimationDatabase*>& impactedDatabases = FindImpactedDatabases(pDatabase, fragmentID, tagState.globalTags);
	for (std::vector<CAnimationDatabase*>::const_iterator itDatabases = impactedDatabases.begin(); itDatabases != impactedDatabases.end(); ++itDatabases)
	{
		CAnimationDatabase* pCurrDatabase = *itDatabases;
		if (pCurrDatabase != pDatabase)
		{
			pCurrDatabase->m_pTagDef->TagListToFlags(sTaglist.c_str(), targetTagState.globalTags);
			FragmentID targetFragID = pCurrDatabase->m_pFragDef->Find(fragCRC);

			u32 currIdx = pCurrDatabase->AddEntry(targetFragID, targetTagState, fragment);
			DRX_ASSERT(currIdx == idxRoot);
		}
	}

	return idxRoot;
}

void CAnimationDatabaseUpr::SetFragmentEntry(IAnimationDatabase* pDatabaseInterface, FragmentID fragmentID, const SFragTagState& tagState, u32 optionIdx, const CFragment& fragment)
{
	assert(pDatabaseInterface);
	CAnimationDatabase* pDatabase = static_cast<CAnimationDatabase*>(pDatabaseInterface);

	i32 fragCRC = pDatabase->m_pFragDef->GetTagCRC(fragmentID);
	DrxStackStringT<char, 1024> sTaglist;
	pDatabase->m_pTagDef->FlagsToTagList(tagState.globalTags, sTaglist);
	SFragTagState targetTagState = tagState;

	const std::vector<CAnimationDatabase*>& impactedDatabases = FindImpactedDatabases(pDatabase, fragmentID, tagState.globalTags);
	for (std::vector<CAnimationDatabase*>::const_iterator itDatabases = impactedDatabases.begin(); itDatabases != impactedDatabases.end(); ++itDatabases)
	{
		CAnimationDatabase* pCurrDatabase = *itDatabases;

		pCurrDatabase->m_pTagDef->TagListToFlags(sTaglist.c_str(), targetTagState.globalTags);
		FragmentID targetFragID = pCurrDatabase->m_pFragDef->Find(fragCRC);

		pCurrDatabase->SetEntry(targetFragID, targetTagState, optionIdx, fragment);
	}
}

std::vector<CAnimationDatabase*> CAnimationDatabaseUpr::FindImpactedDatabases(const CAnimationDatabase* pWorkingDatabase, FragmentID fragmentID, TagState globalTags) const
{
	// Find fragment owner database
	string ownerADB;
	FindRootSubADB(*pWorkingDatabase, NULL, fragmentID, globalTags, ownerADB);
	DRX_ASSERT(ownerADB.empty() == false);
	if (ownerADB.empty())
	{
		ownerADB = pWorkingDatabase->GetFilename();
	}

	// Add all databases including the root one
	std::vector<CAnimationDatabase*> databases;
	for (TAnimDatabaseList::const_iterator itDatabases = m_databases.begin(); itDatabases != m_databases.end(); ++itDatabases)
	{
		CAnimationDatabase* pCurrDatabase = itDatabases->second;
		if (HasAncestor(*pCurrDatabase, ownerADB))
		{
			databases.push_back(pCurrDatabase);
		}
	}

	DRX_ASSERT(false == databases.empty());
	IF_UNLIKELY (databases.empty())
	{
		databases.push_back(const_cast<CAnimationDatabase*>(pWorkingDatabase));
	}

	return databases;
}

bool CAnimationDatabaseUpr::HasAncestor(const CAnimationDatabase& database, const string& ancestorName) const
{
	if (database.GetFilename() == ancestorName)
		return true;

	for (CAnimationDatabase::TSubADBList::const_iterator itSubADBs = database.m_subADBs.begin(); itSubADBs != database.m_subADBs.end(); ++itSubADBs)
	{
		if (HasAncestor(*itSubADBs, ancestorName))
		{
			return true;
		}
	}
	return false;
}

bool CAnimationDatabaseUpr::HasAncestor(const CAnimationDatabase::SSubADB& database, const string& ancestorName) const
{
	if (database.filename == ancestorName)
	{
		return true;
	}

	for (CAnimationDatabase::TSubADBList::const_iterator itSubADBs = database.subADBs.begin(); itSubADBs != database.subADBs.end(); ++itSubADBs)
	{
		if (HasAncestor(*itSubADBs, ancestorName))
		{
			return true;
		}
	}
	return false;
}

void CAnimationDatabaseUpr::FindRootSubADB(const CAnimationDatabase& animDB, const CAnimationDatabase::SSubADB* pSubADB, FragmentID fragID, TagState tagState, string& outRootADB) const
{
	if (false == CanSaveFragmentID(fragID, animDB, pSubADB))
	{
		return;
	}

	bool tagStateMatch = false;
	if (ShouldSaveFragment(fragID, tagState, animDB, pSubADB, tagStateMatch))
	{
		if (tagStateMatch || outRootADB.empty())
		{
			outRootADB = pSubADB ? pSubADB->filename : animDB.m_filename;
		}
	}

	const CAnimationDatabase::TSubADBList& vSubADBs = pSubADB ? pSubADB->subADBs : animDB.m_subADBs;
	for (CAnimationDatabase::TSubADBList::const_iterator itSubADB = vSubADBs.begin(); itSubADB != vSubADBs.end(); ++itSubADB)
	{
		FindRootSubADB(animDB, &*itSubADB, fragID, tagState, outRootADB);
	}
}

void CAnimationDatabaseUpr::PrepareSave(const CAnimationDatabase& animDB, const CAnimationDatabase::SSubADB* subAnimDB, TFragmentSaveList& vFragSaveList, TFragmentBlendSaveDatabase& mBlendSaveDatabase) const
{
	vFragSaveList.resize(animDB.m_fragmentList.size());
	for (u32 i = 0; i < animDB.m_fragmentList.size(); ++i)
	{
		if (CanSaveFragmentID(i, animDB, subAnimDB))
		{
			CAnimationDatabase::SFragmentEntry* entry = animDB.m_fragmentList[i];
			u32 size = 0;
			if (entry)
				size = entry->tagSetList.Size();
			if (size > 0)
			{
				vFragSaveList[i].vSaveStates.resize(size);
				for (u32 k = 0; k < size; ++k)
				{
					const SFragTagState& tagState = entry->tagSetList.m_keys[k];
					bool bTagMatchFound = false;
					const bool bAddFragment = ShouldSaveFragment(i, tagState.globalTags, animDB, subAnimDB, bTagMatchFound);
					if (bAddFragment)
					{
						if (vFragSaveList[i].vSaveStates[k].m_sFileName.empty())
						{
							vFragSaveList[i].vSaveStates[k].m_bSavedByTags = bTagMatchFound;
							vFragSaveList[i].vSaveStates[k].m_sFileName = subAnimDB ? subAnimDB->filename : animDB.m_filename;
						}
					}
				}
			}
		}
	}

	if (!animDB.m_fragmentBlendDB.empty())
	{
		for (CAnimationDatabase::TFragmentBlendDatabase::const_iterator iter = animDB.m_fragmentBlendDB.begin(); iter != animDB.m_fragmentBlendDB.end(); ++iter)
		{
			const FragmentID fragIDFrom = iter->first.fragFrom;
			const FragmentID fragIDTo = iter->first.fragTo;

			if (CanSaveFragmentID(fragIDFrom, animDB, subAnimDB) && CanSaveFragmentID(fragIDTo, animDB, subAnimDB))
			{
				u32k numVars = iter->second.variantList.size();
				mBlendSaveDatabase[iter->first].vSaveStates.resize(numVars);
				for (u32 v = 0; v < numVars; ++v)
				{
					const CAnimationDatabase::SFragmentBlendVariant& variant = iter->second.variantList[v];

					bool bTagMatchFound = false;
					bool addBlend = ShouldSaveTransition(fragIDFrom, fragIDTo, variant.tagsFrom.globalTags, variant.tagsTo.globalTags, animDB, subAnimDB, bTagMatchFound);
					if (addBlend)
					{
						if (mBlendSaveDatabase[iter->first].vSaveStates[v].m_sFileName.empty() || bTagMatchFound)
						{
							mBlendSaveDatabase[iter->first].vSaveStates[v].m_bSavedByTags = bTagMatchFound;
							mBlendSaveDatabase[iter->first].vSaveStates[v].m_sFileName = subAnimDB ? subAnimDB->filename : animDB.m_filename;
						}
					}
				}
			}
		}
	}
	const CAnimationDatabase::TSubADBList& vSubADBs = subAnimDB ? subAnimDB->subADBs : animDB.m_subADBs;
	for (CAnimationDatabase::TSubADBList::const_iterator itSubADB = vSubADBs.begin(); itSubADB != vSubADBs.end(); ++itSubADB)
		PrepareSave(animDB, &(*itSubADB), vFragSaveList, mBlendSaveDatabase);

}

void CAnimationDatabaseUpr::SaveDatabasesSnapshot(SSnapshotCollection& snapshotCollection) const
{
	snapshotCollection.m_databases.clear();
	snapshotCollection.m_databases.reserve(m_databases.size());
	snapshotCollection.m_controllerDefs.clear();
	snapshotCollection.m_controllerDefs.reserve(m_controllerDefs.size());

	for (TAnimDatabaseList::const_iterator it = m_databases.begin(), itEnd = m_databases.end(); it != itEnd; ++it)
	{
		CAnimationDatabase* animDB = (CAnimationDatabase*)it->second;
		TFragmentSaveList vFragSaveList;
		TFragmentBlendSaveDatabase mBlendSaveDatabase;

		PrepareSave(*animDB, NULL, vFragSaveList, mBlendSaveDatabase);

		SAnimDBSnapshot snapshot;
		snapshot.pDatabase = animDB;
		snapshot.xmlData = SaveDatabase(*animDB, NULL, vFragSaveList, mBlendSaveDatabase, true);

		snapshotCollection.m_databases.push_back(snapshot);
	}

	for (TControllerDefList::const_iterator it = m_controllerDefs.begin(), itEnd = m_controllerDefs.end(); it != itEnd; ++it)
	{
		SControllerDef* pControllerDef = (SControllerDef*)it->second;

		SAnimControllerDefSnapshot snapshot;
		snapshot.pControllerDef = pControllerDef;
		snapshot.xmlData = SaveControllerDef(*pControllerDef);

		snapshotCollection.m_controllerDefs.push_back(snapshot);
	}
}

void CAnimationDatabaseUpr::LoadDatabasesSnapshot(const SSnapshotCollection& snapshotCollection)
{
	for (TAnimDBSnapshotCollection::const_iterator it = snapshotCollection.m_databases.begin(), itEnd = snapshotCollection.m_databases.end(); it != itEnd; ++it)
	{
		CAnimationDatabase* animDB = (CAnimationDatabase*)it->pDatabase;
		const XmlNodeRef& xmlData = it->xmlData;

		ClearDatabase(animDB);

		LoadDatabase(xmlData, *animDB, false);
	}

	for (TAnimControllerDefSnapshotCollection::const_iterator it = snapshotCollection.m_controllerDefs.begin(), itEnd = snapshotCollection.m_controllerDefs.end(); it != itEnd; ++it)
	{
		SControllerDef* pControllerDef = (SControllerDef*)it->pControllerDef;
		const XmlNodeRef& xmlData = it->xmlData;

		TDefPathString defFilename = pControllerDef->m_filename;
		SControllerDef* pNewControllerDef = CAnimationControllerDefLibrary::LoadControllerDef(xmlData, "");
		new(pControllerDef) SControllerDef(*pNewControllerDef);
		pControllerDef->m_filename = defFilename;
		delete pNewControllerDef;
	}
}

void CAnimationDatabaseUpr::GetLoadedTagDefs(DynArray<const CTagDefinition*>& tagDefs)
{
	tagDefs.clear();
	tagDefs.reserve(m_tagDefs.size());

	for (TTagDefList::const_iterator cit = m_tagDefs.begin(); cit != m_tagDefs.end(); ++cit)
	{
		const CTagDefinition* pTagDefinition = cit->second;
		tagDefs.push_back(pTagDefinition);
	}
}

void CAnimationDatabaseUpr::GetLoadedDatabases(DynArray<const IAnimationDatabase*>& animDatabases) const
{
	animDatabases.clear();
	animDatabases.reserve(m_databases.size());

	for (TAnimDatabaseList::const_iterator cit = m_databases.begin(); cit != m_databases.end(); ++cit)
	{
		CAnimationDatabase* pAnimationDatabase = cit->second;
		animDatabases.push_back(pAnimationDatabase);
	}
}

void CAnimationDatabaseUpr::GetLoadedControllerDefs(DynArray<const SControllerDef*>& controllerDefs) const
{
	controllerDefs.clear();
	controllerDefs.reserve(m_controllerDefs.size());

	for (TControllerDefList::const_iterator cit = m_controllerDefs.begin(); cit != m_controllerDefs.end(); ++cit)
	{
		const SControllerDef* pControllerDefs = cit->second;
		controllerDefs.push_back(pControllerDefs);
	}
}

void CAnimationDatabaseUpr::SaveAll(IMannequinWriter* pWriter) const
{
	if (!pWriter)
	{
		return;
	}

	for (TAnimDatabaseList::const_iterator cit = m_databases.begin(); cit != m_databases.end(); ++cit)
	{
		CAnimationDatabase* pAnimationDatabase = cit->second;
		SaveDatabase(pWriter, pAnimationDatabase);
	}

	for (TControllerDefList::const_iterator cit = m_controllerDefs.begin(); cit != m_controllerDefs.end(); ++cit)
	{
		SControllerDef* pControllerDef = cit->second;
		SaveControllerDef(pWriter, pControllerDef);
	}

	for (TTagDefList::const_iterator cit = m_tagDefs.begin(); cit != m_tagDefs.end(); ++cit)
	{
		CTagDefinition* pTagDefinition = cit->second;
		SaveTagDefinition(pWriter, pTagDefinition);
	}

	pWriter->WriteModifiedFiles();
}

void CAnimationDatabaseUpr::SaveSubADB
(
  IMannequinWriter* pWriter,
  const CAnimationDatabase& animationDatabase,
  const CAnimationDatabase::SSubADB& subADB,
  const TFragmentSaveList& vFragSaveList,
  const TFragmentBlendSaveDatabase& mBlendSaveDatabase

) const
{
	XmlNodeRef xmlAnimationDatabase = SaveDatabase(animationDatabase, &subADB, vFragSaveList, mBlendSaveDatabase);
	assert(xmlAnimationDatabase != 0);
	if (!xmlAnimationDatabase)
	{
		return;
	}

	tukk szFilename = subADB.filename;
	pWriter->SaveFile(szFilename, xmlAnimationDatabase, eFET_Database);

	for (CAnimationDatabase::TSubADBList::const_iterator itSubADB = subADB.subADBs.begin(); itSubADB != subADB.subADBs.end(); ++itSubADB)
		SaveSubADB(pWriter, animationDatabase, *itSubADB, vFragSaveList, mBlendSaveDatabase);
}

void CAnimationDatabaseUpr::SaveDatabase(IMannequinWriter* pWriter, const IAnimationDatabase* pIAnimationDatabase) const
{
	const CAnimationDatabase* pAnimationDatabase = static_cast<const CAnimationDatabase*>(pIAnimationDatabase);
	assert(pWriter);
	assert(pAnimationDatabase);

	TFragmentSaveList vFragSaveList;
	TFragmentBlendSaveDatabase mBlendSaveDatabase;

	PrepareSave(*pAnimationDatabase, NULL, vFragSaveList, mBlendSaveDatabase);

	u32 numSubADBs = pAnimationDatabase->m_subADBs.size();
	for (u32 i = 0; i < numSubADBs; ++i)
		SaveSubADB(pWriter, *pAnimationDatabase, pAnimationDatabase->m_subADBs[i], vFragSaveList, mBlendSaveDatabase);

	XmlNodeRef xmlAnimationDatabase = SaveDatabase(*pAnimationDatabase, NULL, vFragSaveList, mBlendSaveDatabase);
	assert(xmlAnimationDatabase != 0);
	if (!xmlAnimationDatabase)
	{
		return;
	}

	tukk szFilename = pAnimationDatabase->GetFilename();
	pWriter->SaveFile(szFilename, xmlAnimationDatabase, eFET_Database);
}

void CAnimationDatabaseUpr::SaveControllerDef(IMannequinWriter* pWriter, const SControllerDef* pControllerDef) const
{
	assert(pWriter);
	assert(pControllerDef);

	XmlNodeRef xmlControllerDef = SaveControllerDef(*pControllerDef);
	assert(xmlControllerDef != 0);
	if (!xmlControllerDef)
	{
		return;
	}

	tukk szFilename = pControllerDef->m_filename.c_str();
	pWriter->SaveFile(szFilename, xmlControllerDef, eFET_ControllerDef);
}

void CAnimationDatabaseUpr::SaveTagDefinition(IMannequinWriter* pWriter, const CTagDefinition* pTagDefinition) const
{
	assert(pWriter);
	assert(pTagDefinition);

	mannequin::TTagDefinitionSaveDataList saveList;
	mannequin::SaveTagDefinition(*pTagDefinition, saveList);

	for (size_t i = 0; i < saveList.size(); ++i)
	{
		mannequin::STagDefinitionSaveData& saveData = saveList[i];
		tukk szFilename = saveData.filename.c_str();
		XmlNodeRef xmlTagDefinition = saveData.pXmlNode;

		pWriter->SaveFile(szFilename, xmlTagDefinition, eFET_TagDef);
	}
}

bool IsFileUsedByTagDefinition(tukk normalizedFilename, const CTagDefinition* pTagDefinition)
{
	if (!pTagDefinition)
	{
		return false;
	}

	return (stricmp(normalizedFilename, pTagDefinition->GetFilename()) == 0);
}

bool CAnimationDatabaseUpr::IsFileUsedByControllerDef(const SControllerDef& controllerDef, tukk szFilename) const
{
	char normalizedFilename[DEF_PATH_LENGTH];
	NormalizeFilename(normalizedFilename, szFilename);

	if (stricmp(controllerDef.m_filename.c_str(), normalizedFilename) == 0)
	{
		return true;
	}

	if (IsFileUsedByTagDefinition(normalizedFilename, &controllerDef.m_tags))
	{
		return true;
	}

	if (IsFileUsedByTagDefinition(normalizedFilename, &controllerDef.m_fragmentIDs))
	{
		return true;
	}

	if (IsFileUsedByTagDefinition(normalizedFilename, &controllerDef.m_scopeContexts))
	{
		return true;
	}

	if (IsFileUsedByTagDefinition(normalizedFilename, &controllerDef.m_scopeIDs))
	{
		return true;
	}

	for (size_t i = 0; i < controllerDef.m_fragmentDef.size(); ++i)
	{
		const CTagDefinition* pFragTagDef = controllerDef.GetFragmentTagDef(i);
		if (IsFileUsedByTagDefinition(normalizedFilename, pFragTagDef))
		{
			return true;
		}
	}

	//for (TAnimDatabaseList::const_iterator cit = m_databases.begin(); cit != m_databases.end(); ++cit)
	//{
	//	const CAnimationDatabase* pAnimDB = cit->second;
	//	if (pAnimDB->m_def == &controllerDef)
	//	{
	//		if (stricmp(normalizedFilename, pAnimDB->GetFilename()) == 0)
	//		{
	//			return true;
	//		}
	//	}
	//}

	return false;
}

void CAnimationDatabaseUpr::RegisterListener(IMannequinEditorListener* pListener)
{
	m_editorListenerSet.Add(pListener);
}

void CAnimationDatabaseUpr::UnregisterListener(IMannequinEditorListener* pListener)
{
	m_editorListenerSet.Remove(pListener);
}

void CAnimationDatabaseUpr::NotifyListenersTagDefinitionChanged(const CTagDefinition& tagDef)
{
	for (TEditorListenerSet::Notifier notifier(m_editorListenerSet); notifier.IsValid(); notifier.Next())
	{
		notifier->OnMannequinTagDefInvalidated(tagDef);
	}

	CDrxAction::GetDrxAction()->GetMannequinInterface().GetMannequinUserParamsUpr().ReloadAll(*this);
}

void CAnimationDatabaseUpr::AddSubADBFragmentFilter(IAnimationDatabase* pDatabaseInterface, tukk szSubADBFilename, FragmentID fragmentID)
{
	DRX_ASSERT(szSubADBFilename);
	DRX_ASSERT(pDatabaseInterface);

	for (TAnimDatabaseList::iterator it = m_databases.begin(); it != m_databases.end(); ++it)
	{
		CAnimationDatabase* const pDatabase = it->second;
		CAnimationDatabase::SSubADB* pSubAdb = pDatabase->FindSubADB(szSubADBFilename, true);
		if (pSubAdb)
		{
			stl::push_back_unique(pSubAdb->vFragIDs, fragmentID);
		}
	}

	ReconcileSubDatabases(static_cast<CAnimationDatabase*>(pDatabaseInterface));
}

void CAnimationDatabaseUpr::RemoveSubADBFragmentFilter(IAnimationDatabase* pDatabaseInterface, tukk szSubADBFilename, FragmentID fragmentID)
{
	DRX_ASSERT(szSubADBFilename);
	DRX_ASSERT(pDatabaseInterface);

	for (TAnimDatabaseList::iterator it = m_databases.begin(); it != m_databases.end(); ++it)
	{
		CAnimationDatabase* const pDatabase = it->second;
		CAnimationDatabase::SSubADB* pSubAdb = pDatabase->FindSubADB(szSubADBFilename, true);
		if (pSubAdb)
		{
			stl::find_and_erase(pSubAdb->vFragIDs, fragmentID);
		}
	}

	ReconcileSubDatabases(static_cast<CAnimationDatabase*>(pDatabaseInterface));
}

u32 CAnimationDatabaseUpr::GetSubADBFragmentFilterCount(const IAnimationDatabase* pDatabaseInterface, tukk szSubADBFilename) const
{
	DRX_ASSERT(pDatabaseInterface);
	DRX_ASSERT(szSubADBFilename);

	const CAnimationDatabase* const pDatabase = static_cast<const CAnimationDatabase*>(pDatabaseInterface);
	const CAnimationDatabase::SSubADB* pSubADB = pDatabase->FindSubADB(szSubADBFilename, true);
	if (pSubADB)
	{
		return pSubADB->vFragIDs.size();
	}
	return 0;
}

FragmentID CAnimationDatabaseUpr::GetSubADBFragmentFilter(const IAnimationDatabase* pDatabaseInterface, tukk szSubADBFilename, u32 index) const
{
	DRX_ASSERT(pDatabaseInterface);
	DRX_ASSERT(szSubADBFilename);

	const CAnimationDatabase* const pDatabase = static_cast<const CAnimationDatabase*>(pDatabaseInterface);
	const CAnimationDatabase::SSubADB* pSubADB = pDatabase->FindSubADB(szSubADBFilename, true);
	DRX_ASSERT(pSubADB);
	return pSubADB->vFragIDs[index];
}

void CAnimationDatabaseUpr::SetSubADBTagFilter(IAnimationDatabase* pDatabaseInterface, tukk szSubADBFilename, TagState tagState)
{
	DRX_ASSERT(szSubADBFilename);
	DRX_ASSERT(pDatabaseInterface);

	for (TAnimDatabaseList::iterator it = m_databases.begin(); it != m_databases.end(); ++it)
	{
		CAnimationDatabase* const pDatabase = it->second;
		CAnimationDatabase::SSubADB* pSubAdb = pDatabase->FindSubADB(szSubADBFilename, true);
		if (pSubAdb)
		{
			pSubAdb->tags = tagState;
			pSubAdb->comparisonMask = pSubAdb->pTagDef->GenerateMask(tagState);
		}
	}

	ReconcileSubDatabases(static_cast<CAnimationDatabase*>(pDatabaseInterface));
}

TagState CAnimationDatabaseUpr::GetSubADBTagFilter(const IAnimationDatabase* pDatabaseInterface, tukk szSubADBFilename) const
{
	DRX_ASSERT(pDatabaseInterface);
	DRX_ASSERT(szSubADBFilename);

	const CAnimationDatabase* const pDatabase = static_cast<const CAnimationDatabase*>(pDatabaseInterface);
	const CAnimationDatabase::SSubADB* pSubADB = pDatabase->FindSubADB(szSubADBFilename, true);
	if (pSubADB)
	{
		return pSubADB->tags;
	}
	return TAG_STATE_EMPTY;
}

void CAnimationDatabaseUpr::ReconcileSubDatabases(const CAnimationDatabase* pSourceDatabase)
{
	for (TAnimDatabaseList::iterator it = m_databases.begin(); it != m_databases.end(); ++it)
	{
		CAnimationDatabase* const pDatabase = it->second;

		tukk const databaseFilename = pDatabase->GetFilename();
		const bool isSubDatabaseOfSourceDatabase = (pSourceDatabase->FindSubADB(databaseFilename, true) != NULL);
		if (isSubDatabaseOfSourceDatabase)
		{
			ReconcileSubDatabase(pSourceDatabase, pDatabase);
		}
	}
}

void CAnimationDatabaseUpr::ReconcileSubDatabase(const CAnimationDatabase* pSourceDatabase, CAnimationDatabase* pTargetSubDatabase)
{
	DRX_ASSERT(pSourceDatabase);
	DRX_ASSERT(pTargetSubDatabase);

	tukk const subADBFilename = pTargetSubDatabase->GetFilename();
	const CAnimationDatabase::SSubADB* pSubADB = pSourceDatabase->FindSubADB(subADBFilename, true);
	if (!pSubADB)
	{
		return;
	}

	TFragmentSaveList vFragSaveList;
	TFragmentBlendSaveDatabase mBlendSaveDatabase;
	PrepareSave(*pSourceDatabase, NULL, vFragSaveList, mBlendSaveDatabase);
	XmlNodeRef xmlSubNode = SaveDatabase(*pSourceDatabase, pSubADB, vFragSaveList, mBlendSaveDatabase);

	pTargetSubDatabase->m_subADBs.clear();
	for (size_t i = 0; i < pTargetSubDatabase->m_fragmentList.size(); ++i)
	{
		delete pTargetSubDatabase->m_fragmentList[i];
	}
	pTargetSubDatabase->m_fragmentList.clear();
	pTargetSubDatabase->m_fragmentBlendDB.clear();

	LoadDatabase(xmlSubNode, *pTargetSubDatabase, true);
}

void CAnimationDatabaseUpr::SetBlend(IAnimationDatabase* pIDatabase, FragmentID fragmentIDFrom, FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo, SFragmentBlendUid blendUid, const SFragmentBlend& fragBlend)
{
	DRX_ASSERT(pIDatabase);

	CAnimationDatabase* pDatabase = static_cast<CAnimationDatabase*>(pIDatabase);
	pDatabase->SetBlend(fragmentIDFrom, fragmentIDTo, tagFrom, tagTo, blendUid, fragBlend);
}

SFragmentBlendUid CAnimationDatabaseUpr::AddBlend(IAnimationDatabase* pIDatabase, FragmentID fragmentIDFrom, FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo, const SFragmentBlend& fragBlend)
{
	DRX_ASSERT(pIDatabase);

	CAnimationDatabase* pDatabase = static_cast<CAnimationDatabase*>(pIDatabase);
	return pDatabase->AddBlend(fragmentIDFrom, fragmentIDTo, tagFrom, tagTo, fragBlend);
}

void CAnimationDatabaseUpr::DeleteBlend(IAnimationDatabase* pIDatabase, FragmentID fragmentIDFrom, FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo, SFragmentBlendUid blendUid)
{
	DRX_ASSERT(pIDatabase);

	CAnimationDatabase* pDatabase = static_cast<CAnimationDatabase*>(pIDatabase);
	pDatabase->DeleteBlend(fragmentIDFrom, fragmentIDTo, tagFrom, tagTo, blendUid);
}

void CAnimationDatabaseUpr::GetFragmentBlends(const IAnimationDatabase* pIDatabase, SEditorFragmentBlendID::TEditorFragmentBlendIDArray& outBlendIDs) const
{
	DRX_ASSERT(pIDatabase);

	const CAnimationDatabase* pDatabase = static_cast<const CAnimationDatabase*>(pIDatabase);

	outBlendIDs.reserve(pDatabase->m_fragmentBlendDB.size());
	for (CAnimationDatabase::TFragmentBlendDatabase::const_iterator iter = pDatabase->m_fragmentBlendDB.begin(), end = pDatabase->m_fragmentBlendDB.end(); iter != end; ++iter)
	{
		outBlendIDs.push_back();
		SEditorFragmentBlendID& blend = outBlendIDs.back();
		blend.fragFrom = (*iter).first.fragFrom;
		blend.fragTo = (*iter).first.fragTo;
	}
}

void CAnimationDatabaseUpr::GetFragmentBlendVariants(const IAnimationDatabase* pIDatabase, const FragmentID fragmentIDFrom, const FragmentID fragmentIDTo,
                                                         SEditorFragmentBlendVariant::TEditorFragmentBlendVariantArray& outVariants) const
{
	DRX_ASSERT(pIDatabase);

	const CAnimationDatabase* pDatabase = static_cast<const CAnimationDatabase*>(pIDatabase);

	CAnimationDatabase::SFragmentBlendID query;
	query.fragFrom = fragmentIDFrom;
	query.fragTo = fragmentIDTo;
	CAnimationDatabase::TFragmentBlendDatabase::const_iterator iter = pDatabase->m_fragmentBlendDB.find(query);
	if (iter != pDatabase->m_fragmentBlendDB.end())
	{
		outVariants.reserve((*iter).second.variantList.size());
		for (CAnimationDatabase::TFragmentVariantList::const_iterator varIter = (*iter).second.variantList.begin(), end = (*iter).second.variantList.end(); varIter != end; ++varIter)
		{
			outVariants.push_back();
			SEditorFragmentBlendVariant& var = outVariants.back();
			var.tagsFrom = (*varIter).tagsFrom;
			var.tagsTo = (*varIter).tagsTo;
		}
	}
}

void CAnimationDatabaseUpr::GetFragmentBlend(const IAnimationDatabase* pIDatabase, const FragmentID fragmentIDFrom, const FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo, const SFragmentBlendUid& blendUid, SFragmentBlend& outFragmentBlend) const
{
	DRX_ASSERT(pIDatabase);

	const CAnimationDatabase* pDatabase = static_cast<const CAnimationDatabase*>(pIDatabase);
	const SFragmentBlend* pBlend = pDatabase->GetBlend(fragmentIDFrom, fragmentIDTo, tagFrom, tagTo, blendUid);

	if (pBlend == NULL)
		return;

	outFragmentBlend = *pBlend;
}
