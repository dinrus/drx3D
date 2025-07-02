// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   LocalizedStringUpr.h
//  Version:     v1.00
//  Created:     22/9/2005 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Sys/StdAfx.h>

#include <drx3D/Sys/LocalizedStringUpr.h>
#include <drx3D/Input/IInput.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/System.h> // to access InitLocalization()
#include <drx3D/CoreX/String/Path.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include <locale.h>
#include <time.h>

#if defined(INCLUDE_SCALEFORM_SDK) || defined(DRX_FEATURE_SCALEFORM_HELPER)
	#include <drx3D/Sys/IScaleformHelper.h>
#endif

#include <drx/Core/lib/z/zlib.h>

#if !defined(_RELEASE)
	#include <drx3D/Sys/DrxSizerImpl.h>
#endif //#if !defined(_RELEASE)

#define MAX_CELL_COUNT 32

// CVAR names
const char c_sys_localization_debug[] = "sys_localization_debug";
const char c_sys_localization_encode[] = "sys_localization_encode";

enum ELocalizedXmlColumns
{
	ELOCALIZED_COLUMN_SKIP = 0,
	ELOCALIZED_COLUMN_KEY,
	ELOCALIZED_COLUMN_AUDIOFILE,
	ELOCALIZED_COLUMN_CHARACTER_NAME,
	ELOCALIZED_COLUMN_SUBTITLE_TEXT,
	ELOCALIZED_COLUMN_ACTOR_LINE,
	ELOCALIZED_COLUMN_USE_SUBTITLE,
	ELOCALIZED_COLUMN_VOLUME,
	ELOCALIZED_COLUMN_SOUNDEVENT,
	ELOCALIZED_COLUMN_RADIO_RATIO,
	ELOCALIZED_COLUMN_EVENTPARAMETER,
	ELOCALIZED_COLUMN_SOUNDMOOD,
	ELOCALIZED_COLUMN_IS_DIRECT_RADIO,
	// legacy names
	ELOCALIZED_COLUMN_LEGACY_PERSON,
	ELOCALIZED_COLUMN_LEGACY_CHARACTERNAME,
	ELOCALIZED_COLUMN_LEGACY_TRANSLATED_CHARACTERNAME,
	ELOCALIZED_COLUMN_LEGACY_ENGLISH_DIALOGUE,
	ELOCALIZED_COLUMN_LEGACY_TRANSLATION,
	ELOCALIZED_COLUMN_LEGACY_YOUR_TRANSLATION,
	ELOCALIZED_COLUMN_LEGACY_ENGLISH_SUBTITLE,
	ELOCALIZED_COLUMN_LEGACY_TRANSLATED_SUBTITLE,
	ELOCALIZED_COLUMN_LEGACY_ORIGINAL_CHARACTER_NAME,
	ELOCALIZED_COLUMN_LEGACY_TRANSLATED_CHARACTER_NAME,
	ELOCALIZED_COLUMN_LEGACY_ORIGINAL_TEXT,
	ELOCALIZED_COLUMN_LEGACY_TRANSLATED_TEXT,
	ELOCALIZED_COLUMN_LEGACY_ORIGINAL_ACTOR_LINE,
	ELOCALIZED_COLUMN_LEGACY_TRANSLATED_ACTOR_LINE,
	ELOCALIZED_COLUMN_LAST,
};

// The order must match to the order of the ELocalizedXmlColumns
static tukk sLocalizedColumnNames[] =
{
	// everyhing read by the file will be convert to lower cases
	"skip",
	"key",
	"audio_filename",
	"character name",
	"subtitle text",
	"actor line",
	"use subtitle",
	"volume",
	"prototype event",
	"radio ratio",
	"eventparameter",
	"soundmood",
	"is direct radio",
	// legacy names
	"person",
	"character name",
	"translated character name",
	"english dialogue",
	"translation",
	"your translation",
	"english subtitle",
	"translated subtitle",
	"original character name",
	"translated character name",
	"original text",
	"translated text",
	"original actor line",
	"translated actor line",
};

//Please ensure that this array matches the contents of EPlatformIndependentLanguageID in ILocalizationUpr.h
static tukk PLATFORM_INDEPENDENT_LANGUAGE_NAMES[ILocalizationUpr::ePILID_MAX_OR_INVALID] =
{
	"japanese",
	"english",
	"french",
	"spanish",
	"german",
	"italian",
	"dutch",
	"portuguese",
	"russian",
	"korean",
	"chineset",   // Traditional Chinese
	"chineses",   // Simplified Chinese
	"finnish",
	"swedish",
	"danish",
	"norwegian",
	"polish",
	"arabic",
	"czech",
	"turkish"
};

//////////////////////////////////////////////////////////////////////////
#if !defined(_RELEASE)
static void ReloadDialogData(IConsoleCmdArgs* pArgs)
{
	//gEnv->pSystem->GetLocalizationUpr()->FreeData();
	//CSystem *pSystem = (CSystem*) gEnv->pSystem;
	//pSystem->InitLocalization();
	//pSystem->OpenBasicPaks();
	gEnv->pSystem->GetLocalizationUpr()->ReloadData();
}
#endif //#if !defined(_RELEASE)

//////////////////////////////////////////////////////////////////////////
#if !defined(_RELEASE)
static void TestFormatMessage(IConsoleCmdArgs* pArgs)
{
	string fmt1("abc %1 def % gh%2i %");
	string fmt2("abc %[action:abc] %2 def % gh%1i %1");
	string out1, out2;
	gEnv->pSystem->GetLocalizationUpr()->FormatStringMessage(out1, fmt1, "first", "second", "third");
	DrxLogAlways("%s", out1.c_str());
	gEnv->pSystem->GetLocalizationUpr()->FormatStringMessage(out2, fmt2, "second");
	DrxLogAlways("%s", out2.c_str());
}
#endif //#if !defined(_RELEASE)

//////////////////////////////////////////////////////////////////////////
#if !defined(_RELEASE)
void CLocalizedStringsUpr::LocalizationDumpLoadedInfo(IConsoleCmdArgs* pArgs)
{
	CLocalizedStringsUpr* pLoca = (CLocalizedStringsUpr*) gEnv->pSystem->GetLocalizationUpr();

	tukk pTagName = "";
	for (TTagFileNames::iterator tagit = pLoca->m_tagFileNames.begin(); tagit != pLoca->m_tagFileNames.end(); ++tagit)
	{
		DrxLogAlways("Tag %s (%u)", tagit->first.c_str(), tagit->second.id);

		i32 entries = 0;
		DrxSizerImpl* pSizer = new DrxSizerImpl();

		for (tmapFilenames::iterator it = pLoca->m_loadedTables.begin(); it != pLoca->m_loadedTables.end(); ++it)
		{
			if (tagit->second.id == it->second.nTagID)
			{
				DrxLogAlways("	%s", it->first.c_str());
			}

			if (pLoca->m_pLanguage)
			{
				u32k numEntries = pLoca->m_pLanguage->m_vLocalizedStrings.size();
				for (i32 i = numEntries - 1; i >= 0; i--)
				{
					SLocalizedStringEntry* entry = pLoca->m_pLanguage->m_vLocalizedStrings[i];
					if (tagit->second.id == entry->nTagID)
					{
						entries++;
						entry->GetMemoryUsage((IDrxSizer*) pSizer);
					}
				}
			}
		}

		// *INDENT-OFF* - Space between closing quote and PRISIZE_T needs to be preserved
		DrxLogAlways("		Entries %d, Approx Size %" PRISIZE_T "Kb", entries, pSizer->GetTotalSize() / 1024);
		// *INDENT-ON*

		SAFE_RELEASE(pSizer);
	}
}

#endif //#if !defined(_RELEASE)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CLocalizedStringsUpr::CLocalizedStringsUpr(ISystem* pSystem)
	: m_postProcessors(1)
	, m_cvarLocalizationDebug(0)
	, m_cvarLocalizationEncode(1)
	, m_availableLocalizations(0)
{
	m_pSystem = pSystem;
	m_pSystem->GetISystemEventDispatcher()->RegisterListener(this, "CLocalizedStringsUpr");

	m_languages.reserve(4);
	m_pLanguage = 0;

#if !defined(_RELEASE)
	m_haveWarnedAboutAtLeastOneLabel = false;

	REGISTER_COMMAND("ReloadDialogData", ReloadDialogData, VF_NULL,
	                 "Reloads all localization dependent XML sheets for the currently set language.");

	REGISTER_COMMAND("_TestFormatMessage", TestFormatMessage, VF_NULL, "");

	REGISTER_CVAR2(c_sys_localization_debug, &m_cvarLocalizationDebug, m_cvarLocalizationDebug, VF_CHEAT,
	               "Toggles debugging of the Localization Upr.\n"
	               "Usage: sys_localization_debug [0..3]\n"
	               "1: outputs warnings\n"
	               "2: outputs extended information and warnings\n"
	               "3: outputs CRC32 hashes and strings to help detect clashes\n"
	               "Default is 0 (off).");

	REGISTER_CVAR2(c_sys_localization_encode, &m_cvarLocalizationEncode, m_cvarLocalizationEncode, VF_REQUIRE_APP_RESTART,
	               "Toggles encoding of translated text to save memory. REQUIRES RESTART.\n"
	               "Usage: sys_localization_encode [0..1]\n"
	               "0: No encoding, store as wide strings\n"
	               "1: Huffman encode translated text, saves approx 30% with a small runtime performance cost\n"
	               "Default is 1.");

	REGISTER_COMMAND("LocalizationDumpLoadedInfo", LocalizationDumpLoadedInfo, VF_NULL, "Dump out into about the loaded localization files");
#endif //#if !defined(_RELEASE)

	//Check that someone hasn't added a language ID without a language name
	assert(PLATFORM_INDEPENDENT_LANGUAGE_NAMES[ILocalizationUpr::ePILID_MAX_OR_INVALID - 1] != 0);

	// Populate available languages by scanning the localization directory for paks
	IDrxPak* pPak = gEnv->pDrxPak;
	ILocalizationUpr::TLocalizationBitfield availableLanguages = 0;
	if (pPak)
	{
		stack_string const szLocalizationFolderPath(PathUtil::GetGameFolder() + DRX_NATIVE_PATH_SEPSTR + PathUtil::GetLocalizationFolder() + DRX_NATIVE_PATH_SEPSTR);
		stack_string const szLocalizationSearch(szLocalizationFolderPath + "*.pak");
		size_t const extensionLength = 4; // for ".pak"
		_finddata_t fd;
		intptr_t const handle = pPak->FindFirst(szLocalizationSearch.c_str(), &fd);

		if (handle > -1)
		{
			const char szLocalizationXmlPostfix[] = "_xml";
			const size_t localizationXmlPostfixLength = sizeof(szLocalizationXmlPostfix) - 1; // -1 for null-terminator
			do
			{
				// drop files with too short name
				stack_string const szLanguageName = fd.name;

				if (szLanguageName.length() <= localizationXmlPostfixLength + extensionLength)
				{
					continue;
				}

				// drop files that don't match the pattern "name_xml.pak"
				size_t const languageLength = szLanguageName.length() - localizationXmlPostfixLength - extensionLength;

				if (szLanguageName.compareNoCase(languageLength, localizationXmlPostfixLength, szLocalizationXmlPostfix))
				{
					continue;
				}

				// test language name against supported languages
				for (i32 i = 0; i < ILocalizationUpr::ePILID_MAX_OR_INVALID; i++)
				{
					char const* const szCurrentLanguage = LangNameFromPILID((ILocalizationUpr::EPlatformIndependentLanguageID)i);

					if (szLanguageName.compareNoCase(0, languageLength, szCurrentLanguage) == 0)
					{
						availableLanguages |= ILocalizationUpr::LocalizationBitfieldFromPILID((ILocalizationUpr::EPlatformIndependentLanguageID)i);

						if (m_cvarLocalizationDebug >= 2)
						{
							DrxLogAlways("<Localization> Detected language support for %s (id %d)", szCurrentLanguage, i);
						}
					}
				}
			}
			while (pPak->FindNext(handle, &fd) >= 0);

			pPak->FindClose(handle);
		}
	}

	if (availableLanguages == 0)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR_DBGBRK, "<Localization> No localization files found!");
	}

	SetAvailableLocalizationsBitfield(availableLanguages);
}

//////////////////////////////////////////////////////////////////////
CLocalizedStringsUpr::~CLocalizedStringsUpr()
{
	m_pSystem->GetISystemEventDispatcher()->RemoveListener(this);
	FreeData();
}

//////////////////////////////////////////////////////////////////////
void CLocalizedStringsUpr::GetLoadedTags(TLocalizationTagVec& tags)
{
	for (auto const& t : m_tagFileNames)
	{
		if (t.second.loaded)
		{
			tags.push_back(t.first);
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CLocalizedStringsUpr::FreeLocalizationData()
{
	AutoLock lock(m_cs); // Make sure to lock as this is a modifying operation
	ListAndClearProblemLabels();

	for (auto const& pLanguage : m_languages)
	{
		std::for_each(pLanguage->m_vLocalizedStrings.begin(), pLanguage->m_vLocalizedStrings.end(), stl::container_object_deleter());
		pLanguage->m_vLocalizedStrings.clear();
		pLanguage->m_keysMap.clear();
	}

	m_loadedTables.clear();

	for (auto& t : m_tagFileNames)
	{
		t.second.loaded = false;
	}
}

//////////////////////////////////////////////////////////////////////
void CLocalizedStringsUpr::FreeData()
{
	FreeLocalizationData();

	for (auto const& pLanguage : m_languages)
	{
		delete pLanguage;
	}

	m_languages.resize(0);
	m_pLanguage = nullptr;
}

//////////////////////////////////////////////////////////////////////////
tukk CLocalizedStringsUpr::LangNameFromPILID(const ILocalizationUpr::EPlatformIndependentLanguageID id)
{
	assert(id >= 0 && id < ILocalizationUpr::ePILID_MAX_OR_INVALID);
	return PLATFORM_INDEPENDENT_LANGUAGE_NAMES[id];
}

//Uses bitwise operations to compare the localizations we provide in this SKU and the languages that the platform supports.
//Returns !0 if we provide more localizations than are available as system languages
ILocalizationUpr::TLocalizationBitfield CLocalizedStringsUpr::MaskSystemLanguagesFromSupportedLocalizations(const ILocalizationUpr::TLocalizationBitfield systemLanguages)
{
	assert(m_availableLocalizations != 0);    //Make sure to set the available localizations!
	return (~systemLanguages) & m_availableLocalizations;
}

//Returns !0 if the language is supported.
ILocalizationUpr::TLocalizationBitfield CLocalizedStringsUpr::IsLanguageSupported(const ILocalizationUpr::EPlatformIndependentLanguageID id)
{
	assert(m_availableLocalizations != 0);    //Make sure to set the available localizations!
	return m_availableLocalizations & (1 << id);
}

//////////////////////////////////////////////////////////////////////////
void CLocalizedStringsUpr::SetAvailableLocalizationsBitfield(const ILocalizationUpr::TLocalizationBitfield availableLocalizations)
{
	m_availableLocalizations = availableLocalizations;
}

//////////////////////////////////////////////////////////////////////
tukk CLocalizedStringsUpr::GetLanguage()
{
	if (m_pLanguage == 0)
		return "";
	return m_pLanguage->sLanguage.c_str();
}

//////////////////////////////////////////////////////////////////////
bool CLocalizedStringsUpr::SetLanguage(tukk sLanguage)
{
	if (m_cvarLocalizationDebug >= 2)
		DrxLog("<Localization> Set language to %s", sLanguage);

#if defined(INCLUDE_SCALEFORM_SDK) || defined(DRX_FEATURE_SCALEFORM_HELPER)
	if (gEnv->pScaleformHelper)
	{
		gEnv->pScaleformHelper->SetTranslatorWordWrappingMode(sLanguage);
	}
#endif

	// Check if already language loaded.
	for (u32 i = 0; i < m_languages.size(); i++)
	{
		if (stricmp(sLanguage, m_languages[i]->sLanguage) == 0)
		{
			InternalSetCurrentLanguage(m_languages[i]);
			return true;
		}
	}

	SLanguage* pLanguage = new SLanguage;
	m_languages.push_back(pLanguage);

	if (m_cvarLocalizationDebug >= 2)
		DrxLog("<Localization> Insert new language to %s", sLanguage);

	pLanguage->sLanguage = sLanguage;
	InternalSetCurrentLanguage(pLanguage);

	//-------------------------------------------------------------------------------------------------
	// input localization
	//-------------------------------------------------------------------------------------------------
	// keyboard
	for (i32 i = 0; i <= 0x80; i++)
	{
		AddControl(i);
	}

	// mouse
	for (i32 i = 1; i <= 0x0f; i++)
	{
		AddControl(i * 0x10000);
	}

	return (true);
}

//////////////////////////////////////////////////////////////////////////
void CLocalizedStringsUpr::AddControl(i32 nKey)
{
	//marcok: currently not implemented ... will get fixed soon
	/*IInput *pInput = m_pSystem->GetIInput();

	   if (!pInput)
	   {
	   return;
	   }

	   wchar_t szwKeyName[256] = {0};
	   char		szKey[256] = {0};

	   if (!IS_MOUSE_KEY(nKey))
	   {
	   if (pInput->GetOSKeyName(nKey, szwKeyName, 255))
	   {
	    drx_sprintf(szKey, "control%d", nKey);

	    SLocalizedStringEntry loc;
	    loc.sEnglish.Format( "%S",szwKeyName );
	    loc.sLocalized = szwKeyName;
	    loc.sKey = szKey;
	    AddLocalizedString( m_pLanguage,loc );
	   }
	   }*/
}

//////////////////////////////////////////////////////////////////////////
void CLocalizedStringsUpr::ParseFirstLine(IXmlTableReader* pXmlTableReader, tuk nCellIndexToType, std::map<i32, string>& SoundMoodIndex, std::map<i32, string>& EventParameterIndex)
{
	string sCellContent;

	for (;; )
	{
		i32 nCellIndex = 0;
		tukk pContent = 0;
		size_t contentSize = 0;
		if (!pXmlTableReader->ReadCell(nCellIndex, pContent, contentSize))
			break;

		if (nCellIndex >= MAX_CELL_COUNT)
			break;

		if (contentSize <= 0)
			continue;

		sCellContent.assign(pContent, contentSize);
		sCellContent.MakeLower();

		for (i32 i = 0; i < DRX_ARRAY_COUNT(sLocalizedColumnNames); ++i)
		{
			tukk pFind = strstr(sCellContent.c_str(), sLocalizedColumnNames[i]);
			if (pFind != 0)
			{
				nCellIndexToType[nCellIndex] = i;

				// find SoundMood
				if (i == ELOCALIZED_COLUMN_SOUNDMOOD)
				{
					tukk pSoundMoodName = pFind + strlen(sLocalizedColumnNames[i]) + 1;
					i32 nSoundMoodNameLength = sCellContent.length() - strlen(sLocalizedColumnNames[i]) - 1;
					if (nSoundMoodNameLength > 0)
					{
						SoundMoodIndex[nCellIndex] = pSoundMoodName;
					}
				}

				// find EventParameter
				if (i == ELOCALIZED_COLUMN_EVENTPARAMETER)
				{
					tukk pParameterName = pFind + strlen(sLocalizedColumnNames[i]) + 1;
					i32 nParameterNameLength = sCellContent.length() - strlen(sLocalizedColumnNames[i]) - 1;
					if (nParameterNameLength > 0)
					{
						EventParameterIndex[nCellIndex] = pParameterName;
					}
				}

				break;
			}
			// HACK until all columns are renamed to "Translation"
			//if (stricmp(sCellContent, "Your Translation") == 0)
			//{
			//	nCellIndexToType[nCellIndex] = ELOCALIZED_COLUMN_TRANSLATED_ACTOR_LINE;
			//	break;
			//}
		}
	}
}

// copy characters to lower-case 0-terminated buffer
static void CopyLowercase(tuk dst, size_t dstSize, tukk src, size_t srcCount)
{
	if (dstSize > 0)
	{
		if (srcCount > dstSize - 1)
		{
			srcCount = dstSize - 1;
		}
		while (srcCount--)
		{
			const char c = *src++;
			*dst++ = (c <= 'Z' && c >= 'A') ? c + ('a' - 'A') : c;
		}
		*dst = '\0';
	}
}

static void ReplaceEndOfLine(DrxFixedStringT<CLocalizedStringsUpr::LOADING_FIXED_STRING_LENGTH>& s)
{
	const string oldSubstr("\\n");
	const string newSubstr(" \n");
	size_t pos = 0;
	for (;; )
	{
		pos = s.find(oldSubstr, pos);
		if (pos == string::npos)
		{
			return;
		}
		s.replace(pos, oldSubstr.length(), newSubstr);
	}
}

//////////////////////////////////////////////////////////////////////////

void CLocalizedStringsUpr::OnSystemEvent(
  ESystemEvent eEvent, UINT_PTR wparam, UINT_PTR lparam)
{
	// might want to add an event which tells us that we are loading the main menu
	// so everything can be unloaded and init files reloaded so safe some memory
	switch (eEvent)
	{
	case ESYSTEM_EVENT_LEVEL_LOAD_START:
		{
			// This event is here not of interest while we're in the Editor.
			if (!gEnv->IsEditor())
			{
				if (m_cvarLocalizationDebug >= 2)
				{
					DrxLog("<Localization> Loading Requested Tags");
				}

				for (TStringVec::iterator it = m_tagLoadRequests.begin(); it != m_tagLoadRequests.end(); ++it)
				{
					LoadLocalizationDataByTag(*it);
				}
			}

			m_tagLoadRequests.clear();
			break;
		}
	case ESYSTEM_EVENT_EDITOR_ON_INIT:
		{
			// Load all tags after the Editor has finished initialization.
			for (TTagFileNames::iterator it = m_tagFileNames.begin(); it != m_tagFileNames.end(); ++it)
			{
				LoadLocalizationDataByTag(it->first);
			}

			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void CLocalizedStringsUpr::RegisterPostProcessor(ILocalizationPostProcessor* pPostProcessor)
{
	assert(pPostProcessor);
	m_postProcessors.Add(pPostProcessor);
}

void CLocalizedStringsUpr::UnregisterPostProcessor(ILocalizationPostProcessor* pPostProcessor)
{
	assert(pPostProcessor);
	m_postProcessors.Remove(pPostProcessor);
}

//////////////////////////////////////////////////////////////////////////

bool CLocalizedStringsUpr::InitLocalizationData(
  tukk sFileName, bool bReload)
{
	XmlNodeRef root = m_pSystem->LoadXmlFromFile(sFileName);

	if (!root)
	{
		DrxLog("Loading Localization File %s failed!", sFileName);
		return false;
	}

	for (i32 i = 0; i < root->getChildCount(); i++)
	{
		XmlNodeRef typeNode = root->getChild(i);
		string sType = typeNode->getTag();

		// tags should be unique
		if (m_tagFileNames.find(sType) != m_tagFileNames.end())
			continue;

		TStringVec vEntries;
		for (i32 j = 0; j < typeNode->getChildCount(); j++)
		{
			XmlNodeRef entry = typeNode->getChild(j);
			if (!entry->isTag("entry"))
				continue;

			vEntries.push_back(entry->getContent());
		}

		DRX_ASSERT(m_tagFileNames.size() < 255);

		size_t curNumTags = m_tagFileNames.size();

		m_tagFileNames[sType].filenames = vEntries;
		m_tagFileNames[sType].id = static_cast<u8>(curNumTags + 1);
		m_tagFileNames[sType].loaded = false;

	}

	return true;
}

bool CLocalizedStringsUpr::RequestLoadLocalizationDataByTag(tukk sTag)
{
	TTagFileNames::iterator it = m_tagFileNames.find(sTag);
	if (it == m_tagFileNames.end())
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "[LocError] RequestLoadLocalizationDataByTag - Localization tag '%s' not found", sTag);
		return false;
	}

	if (m_cvarLocalizationDebug >= 2)
	{
		DrxLog("<Localization> RequestLoadLocalizationDataByTag %s", sTag);
	}

	m_tagLoadRequests.push_back(sTag);

	return true;
}

bool CLocalizedStringsUpr::LoadLocalizationDataByTag(
  tukk sTag, bool bReload)
{
	TTagFileNames::iterator it = m_tagFileNames.find(sTag);
	if (it == m_tagFileNames.end())
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "[LocError] LoadLocalizationDataByTag - Localization tag '%s' not found", sTag);
		return false;
	}

	if (it->second.loaded)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "[LocError] LoadLocalizationDataByTag - Already loaded tag '%s'", sTag);
		return false;
	}

	bool bResult = true;
	stack_string const szLocalizationFolderPath(PathUtil::GetLocalizationFolder() + DRX_NATIVE_PATH_SEPSTR + m_pLanguage->sLanguage + "_xml" + DRX_NATIVE_PATH_SEPSTR);
	TStringVec& vEntries = it->second.filenames;

	for (TStringVec::iterator it2 = vEntries.begin(); it2 != vEntries.end(); ++it2)
	{
		bResult &= DoLoadExcelXmlSpreadsheet(szLocalizationFolderPath.c_str() + *it2, it->second.id, bReload);
	}

	if (m_cvarLocalizationDebug >= 2)
	{
		DrxLog("<Localization> LoadLocalizationDataByTag %s with result %d", sTag, bResult);
	}

	it->second.loaded = true;

	return bResult;
}

bool CLocalizedStringsUpr::ReleaseLocalizationDataByTag(
  tukk sTag)
{
	INDENT_LOG_DURING_SCOPE(true, "Releasing localization data with the tag '%s'", sTag);
	ListAndClearProblemLabels();

	TTagFileNames::iterator it = m_tagFileNames.find(sTag);
	if (it == m_tagFileNames.end())
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "[LocError] ReleaseLocalizationDataByTag - Localization tag '%s' not found", sTag);
		return false;
	}

	if (it->second.loaded == false)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "[LocError] ReleaseLocalizationDataByTag - tag '%s' not loaded", sTag);
		return false;
	}

	u8k nTagID = it->second.id;

	tmapFilenames newLoadedTables;
	for (tmapFilenames::iterator iter = m_loadedTables.begin(); iter != m_loadedTables.end(); ++iter)
	{
		if (iter->second.nTagID != nTagID)
		{
			newLoadedTables[iter->first] = iter->second;
		}
	}
	m_loadedTables = newLoadedTables;

	if (m_pLanguage)
	{
		//LARGE_INTEGER liStart;
		//QueryPerformanceCounter(&liStart);
		AutoLock lock(m_cs);  //Make sure to lock, as this is a modifying operation

		bool bMapEntryErased = false;
		//First, remove entries from the map
		for (StringsKeyMap::iterator keyMapIt = m_pLanguage->m_keysMap.begin(); keyMapIt != m_pLanguage->m_keysMap.end(); )
		{
			if (keyMapIt->second->nTagID == nTagID)
			{
				//VECTORMAP ONLY
				keyMapIt = m_pLanguage->m_keysMap.erase(keyMapIt);
				bMapEntryErased = true;
			}
			else
			{
				++keyMapIt;
			}
		}

		if (bMapEntryErased == true)
		{
			StringsKeyMap newMap = m_pLanguage->m_keysMap;
			m_pLanguage->m_keysMap.clearAndFreeMemory();
			m_pLanguage->m_keysMap = newMap;
		}

		bool bVecEntryErased = false;
		//Then remove the entries in the storage vector
		u32k numEntries = m_pLanguage->m_vLocalizedStrings.size();
		for (i32 i = numEntries - 1; i >= 0; i--)
		{
			SLocalizedStringEntry* entry = m_pLanguage->m_vLocalizedStrings[i];
			PREFAST_ASSUME(entry);
			if (entry->nTagID == nTagID)
			{
				if (m_cvarLocalizationEncode == 1)
				{
					if (entry->huffmanTreeIndex != -1)
					{
						HuffmanCoder* pCoder = m_pLanguage->m_vEncoders[entry->huffmanTreeIndex];
						if (pCoder != NULL)
						{
							pCoder->DecRef();
							if (pCoder->RefCount() == 0)
							{
								if (m_cvarLocalizationDebug >= 2)
								{
									DrxLog("<Localization> Releasing coder %d as it no longer has associated strings", entry->huffmanTreeIndex);
								}
								//This coding table no longer needed, it has no more associated strings
								SAFE_DELETE(m_pLanguage->m_vEncoders[entry->huffmanTreeIndex]);
							}
						}
					}
				}
				bVecEntryErased = true;
				delete(entry);
				m_pLanguage->m_vLocalizedStrings.erase(m_pLanguage->m_vLocalizedStrings.begin() + i);
			}
		}

		//Shrink the vector if necessary
		if (bVecEntryErased == true)
		{
			SLanguage::TLocalizedStringEntries newVec = m_pLanguage->m_vLocalizedStrings;
			m_pLanguage->m_vLocalizedStrings.clear();
			m_pLanguage->m_vLocalizedStrings = newVec;
		}

		/*LARGE_INTEGER liEnd, liFreq;
		   QueryPerformanceCounter(&liEnd);
		   QueryPerformanceFrequency(&liFreq);

		   CTimeValue lockTime = CTimeValue((liEnd.QuadPart - liStart.QuadPart) * CTimeValue::TIMEVALUE_PRECISION / liFreq.QuadPart);
		   if (m_cvarLocalizationDebug >= 2)
		   {
		   DrxLog("<Localization> ReleaseLocalizationDataByTag %s lock time %fMS", sTag, lockTime.GetMilliSeconds());
		   }*/
	}

	if (m_cvarLocalizationDebug >= 2)
	{
		DrxLog("<Localization> ReleaseLocalizationDataByTag %s", sTag);
	}

	it->second.loaded = false;

	return true;
}

bool CLocalizedStringsUpr::LoadExcelXmlSpreadsheet(tukk sFileName, bool bReload)
{
	return DoLoadExcelXmlSpreadsheet(sFileName, 0, bReload);
}

//////////////////////////////////////////////////////////////////////
// Loads a string-table from a Excel XML Spreadsheet file.
bool CLocalizedStringsUpr::DoLoadExcelXmlSpreadsheet(tukk sFileName, u8 nTagID, bool bReload)
{
	LOADING_TIME_PROFILE_SECTION_ARGS(sFileName)
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Localization XMLs");
	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Other, 0, "Localization XML (%s)", sFileName);

	if (!m_pLanguage)
		return false;

	//check if this table has already been loaded
	if (!bReload)
	{
		if (m_loadedTables.find(CONST_TEMP_STRING(sFileName)) != m_loadedTables.end())
			return (true);
	}

	ListAndClearProblemLabels();

	IXmlTableReader* const pXmlTableReader = m_pSystem->GetXmlUtils()->CreateXmlTableReader();
	if (!pXmlTableReader)
	{
		DrxLog("Loading Localization File %s failed (XML system failure)!", sFileName);
		return false;
	}

	XmlNodeRef root;
	{
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "LoadXmlFromFile");
		root = m_pSystem->LoadXmlFromFile(sFileName);
		if (!root)
		{
			DrxLog("Loading Localization File %s failed!", sFileName);
			pXmlTableReader->Release();
			return false;
		}
	}

	// bug search, re-export to a file to compare it
	//string sReExport = sFileName;
	//sReExport += ".re";
	//root->saveToFile(sReExport.c_str());

	DrxLog("Loading Localization File %s", sFileName);
	INDENT_LOG_DURING_SCOPE();

	//Create a huffman coding table for these strings - if they're going to be encoded or compressed
	HuffmanCoder* pEncoder = NULL;
	u8 iEncoder = 0;
	size_t startOfStringsToCompress = 0;
	if (m_cvarLocalizationEncode == 1)
	{
		{
			MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Add Encoder");
			for (iEncoder = 0; iEncoder < m_pLanguage->m_vEncoders.size(); iEncoder++)
			{
				if (m_pLanguage->m_vEncoders[iEncoder] == NULL)
				{
					m_pLanguage->m_vEncoders[iEncoder] = pEncoder = new HuffmanCoder();
					break;
				}
			}
			if (iEncoder == m_pLanguage->m_vEncoders.size())
			{
				pEncoder = new HuffmanCoder();
				m_pLanguage->m_vEncoders.push_back(pEncoder);
			}
			//Make a note of the current end of the loc strings array, as encoding is done in two passes.
			//One pass to build the code table, another to apply it
			pEncoder->Init();
		}
		startOfStringsToCompress = m_pLanguage->m_vLocalizedStrings.size();
	}

	{
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "pXmlTableReader->Begin");
		if (!pXmlTableReader->Begin(root))
		{
			DrxLog("Loading Localization File %s failed! The file is in an unsupported format.", sFileName);
			pXmlTableReader->Release();
			return false;
		}
	}

	i32 rowCount = pXmlTableReader->GetEstimatedRowCount();
	{
		AutoLock lock(m_cs);  //Make sure to lock, as this is a modifying operation
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "m_vLocalizedStrings reserve");
		m_pLanguage->m_vLocalizedStrings.reserve(m_pLanguage->m_vLocalizedStrings.size() + rowCount);
	}
	{
		AutoLock lock(m_cs);  //Make sure to lock, as this is a modifying operation
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "m_keysMap reserve");
		//VectorMap only, not applicable to std::map
		m_pLanguage->m_keysMap.reserve(m_pLanguage->m_keysMap.size() + rowCount);
	}

	{
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "m_loadedTables.insert");
		pairFileName sNewFile;
		sNewFile.first = sFileName;
		sNewFile.second.bDataStripping = false; // this is off for now
		sNewFile.second.nTagID = nTagID;
		m_loadedTables.insert(sNewFile);
	}

	// Cell Index
	char nCellIndexToType[MAX_CELL_COUNT];
	memset(nCellIndexToType, 0, sizeof(nCellIndexToType));

	// SoundMood Index
	std::map<i32, string> SoundMoodIndex;

	// EventParameter Index
	std::map<i32, string> EventParameterIndex;

	bool bFirstRow = true;

	DrxFixedStringT<LOADING_FIXED_STRING_LENGTH> sTmp;

	// lower case event name
	char szLowerCaseEvent[128];
	// lower case key
	char szLowerCaseKey[1024];
	// key CRC
	u32 keyCRC;

	size_t nMemSize = 0;

	for (;; )
	{
		i32 nRowIndex = -1;
		{
			MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "pXmlTableReader->ReadRow");
			if (!pXmlTableReader->ReadRow(nRowIndex))
				break;
		}

		if (bFirstRow)
		{
			MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "ParseFirstLine");
			bFirstRow = false;
			ParseFirstLine(pXmlTableReader, nCellIndexToType, SoundMoodIndex, EventParameterIndex);
			// Skip first row, it contains description only.
			continue;
		}

		bool bValidKey = false;
		bool bValidTranslatedText = false;
		bool bValidTranslatedCharacterName = false;
		bool bValidTranslatedActorLine = false;
		bool bUseSubtitle = true;
		bool bIsDirectRadio = false;
		bool bIsIntercepted = false;

		struct CConstCharArray
		{
			tukk ptr;
			size_t      count;

			CConstCharArray()
			{
				clear();
			}
			void clear()
			{
				ptr = "";
				count = 0;
			}
			bool empty() const
			{
				return count == 0;
			}
		};

		CConstCharArray sKeyString;
		CConstCharArray sCharacterName;
		CConstCharArray sTranslatedCharacterName; // Legacy, to be removed some day...
		CConstCharArray sSubtitleText;
		CConstCharArray sTranslatedText; // Legacy, to be removed some day...
		CConstCharArray sActorLine;
		CConstCharArray sTranslatedActorLine; // Legacy, to be removed some day...
		CConstCharArray sSoundEvent;

		float fVolume = 1.0f;
		float fRadioRatio = 1.0f;
		float fEventParameterValue = 0.0f;
		float fSoundMoodValue = 0.0f;

		i32 nItems = 0;
		std::map<i32, float> SoundMoodValues;
		std::map<i32, float> EventParameterValues;

		for (;; )
		{
			i32 nCellIndex = -1;
			CConstCharArray cell;

			{
				MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "pXmlTableReader->ReadCell");
				if (!pXmlTableReader->ReadCell(nCellIndex, cell.ptr, cell.count))
					break;
			}

			if (nCellIndex >= MAX_CELL_COUNT)
				break;

			// skip empty cells
			if (cell.count <= 0)
				continue;

			const char nCellType = nCellIndexToType[nCellIndex];

			switch (nCellType)
			{
			case ELOCALIZED_COLUMN_SKIP:
				break;
			case ELOCALIZED_COLUMN_KEY:
				sKeyString = cell;
				bValidKey = true;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_AUDIOFILE:
				sKeyString = cell;
				bValidKey = true;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_CHARACTER_NAME:
				sCharacterName = cell;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_SUBTITLE_TEXT:
				sSubtitleText = cell;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_ACTOR_LINE:
				sActorLine = cell;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_USE_SUBTITLE:
				sTmp.assign(cell.ptr, cell.count);
				bUseSubtitle = DrxStringUtils::toYesNoType(sTmp.c_str()) == DrxStringUtils::nYNT_No ? false : true; // favor yes (yes and invalid -> yes)
				break;
			case ELOCALIZED_COLUMN_VOLUME:
				sTmp.assign(cell.ptr, cell.count);
				fVolume = (float)atof(sTmp.c_str());
				++nItems;
				break;
			case ELOCALIZED_COLUMN_SOUNDEVENT:
				sSoundEvent = cell;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_RADIO_RATIO:
				sTmp.assign(cell.ptr, cell.count);
				fRadioRatio = (float)atof(sTmp.c_str());
				++nItems;
				break;
			case ELOCALIZED_COLUMN_EVENTPARAMETER:
				sTmp.assign(cell.ptr, cell.count);
				fEventParameterValue = (float)atof(sTmp.c_str());
				{
					MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "EventParameterValues map insert");
					EventParameterValues[nCellIndex] = fEventParameterValue;
				}
				++nItems;
				break;
			case ELOCALIZED_COLUMN_SOUNDMOOD:
				sTmp.assign(cell.ptr, cell.count);
				fSoundMoodValue = (float)atof(sTmp.c_str());
				{
					MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "SoundMoodValues map insert");
					SoundMoodValues[nCellIndex] = fSoundMoodValue;
				}
				++nItems;
				break;
			case ELOCALIZED_COLUMN_IS_DIRECT_RADIO:
				sTmp.assign(cell.ptr, cell.count);
				if (!stricmp(sTmp.c_str(), "intercept"))
				{
					bIsIntercepted = true;
				}
				bIsDirectRadio = bIsIntercepted || (DrxStringUtils::toYesNoType(sTmp.c_str()) == DrxStringUtils::nYNT_Yes ? true : false); // favor no (no and invalid -> no)
				++nItems;
				break;
			// legacy names
			case ELOCALIZED_COLUMN_LEGACY_PERSON:
				// old file often only have content in this column
				if (!cell.empty())
				{
					sCharacterName = cell;
					sTranslatedCharacterName = cell;
					bValidTranslatedCharacterName = true;
				}
				++nItems;
				break;
			case ELOCALIZED_COLUMN_LEGACY_CHARACTERNAME:
				sCharacterName = cell;
				sTranslatedCharacterName = cell;
				bValidTranslatedCharacterName = true;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_LEGACY_TRANSLATED_CHARACTERNAME:
				sTranslatedCharacterName = cell;
				bValidTranslatedCharacterName = true;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_LEGACY_ENGLISH_DIALOGUE:
				// old file often only have content in this column
				sActorLine = cell;
				sSubtitleText = cell;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_LEGACY_TRANSLATION:
				sTranslatedActorLine = cell;
				sTranslatedText = cell;
				bValidTranslatedText = true;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_LEGACY_YOUR_TRANSLATION:
				sTranslatedActorLine = cell;
				sTranslatedText = cell;
				bValidTranslatedText = true;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_LEGACY_ENGLISH_SUBTITLE:
				sSubtitleText = cell;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_LEGACY_TRANSLATED_SUBTITLE:
				sTranslatedText = cell;
				sTranslatedActorLine = cell;
				bValidTranslatedText = true;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_LEGACY_ORIGINAL_CHARACTER_NAME:
				sCharacterName = cell;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_LEGACY_TRANSLATED_CHARACTER_NAME:
				sTranslatedCharacterName = cell;
				bValidTranslatedCharacterName = true;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_LEGACY_ORIGINAL_TEXT:
				sSubtitleText = cell;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_LEGACY_TRANSLATED_TEXT:
				sTranslatedText = cell;
				bValidTranslatedText = true;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_LEGACY_ORIGINAL_ACTOR_LINE:
				sActorLine = cell;
				++nItems;
				break;
			case ELOCALIZED_COLUMN_LEGACY_TRANSLATED_ACTOR_LINE:
				sTranslatedActorLine = cell;
				bValidTranslatedActorLine = true;
				++nItems;
				break;
			}
		}

		if (!bValidKey)
			continue;

		if (!bValidTranslatedText)
		{
			// if this is a dialog entry with a soundevent and with subtitles then a warning should be issued
			if (m_cvarLocalizationDebug && !sSoundEvent.empty() && bUseSubtitle)
			{
				sTmp.assign(sKeyString.ptr, sKeyString.count);
				DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "[LocError] Key '%s' in file <%s> has no translated text", sTmp.c_str(), sFileName);
			}

			// use translated actor line entry if available before falling back to original entry
			if (!sTranslatedActorLine.empty())
				sTranslatedText = sTranslatedActorLine;
			else
				sTranslatedText = sSubtitleText;
		}

		if (!bValidTranslatedActorLine)
		{
			// if this is a dialog entry with a soundevent then a warning should be issued
			if (m_cvarLocalizationDebug && !sSoundEvent.empty())
			{
				sTmp.assign(sKeyString.ptr, sKeyString.count);
				DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "[LocError] Key '%s' in file <%s> has no translated actor line", sTmp.c_str(), sFileName);
			}

			// use translated text entry if available before falling back to original entry
			if (!sTranslatedText.empty())
				sTranslatedActorLine = sTranslatedText;
			else
				sTranslatedActorLine = sSubtitleText;
		}

		if (!sSoundEvent.empty() && !bValidTranslatedCharacterName)
		{
			if (m_cvarLocalizationDebug)
			{
				sTmp.assign(sKeyString.ptr, sKeyString.count);
				DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "[LocError] Key '%s' in file <%s> has no translated character name", sTmp.c_str(), sFileName);
			}

			sTranslatedCharacterName = sCharacterName;
		}

		if (nItems == 1) // skip lines which contain just one item in the key
			continue;

		// reject to store text if line was marked with no subtitles in game mode
		if (!gEnv->IsEditor())
		{
			if (!bUseSubtitle)
			{
				sSubtitleText.clear();
				sTranslatedText.clear();
			}
		}

		// Skip @ character in the key string.
		if (!sKeyString.empty() && sKeyString.ptr[0] == '@')
		{
			sKeyString.ptr++;
			sKeyString.count--;
		}

		{
			MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "CopyLowercase");
			CopyLowercase(szLowerCaseEvent, sizeof(szLowerCaseEvent), sSoundEvent.ptr, sSoundEvent.count);
			CopyLowercase(szLowerCaseKey, sizeof(szLowerCaseKey), sKeyString.ptr, sKeyString.count);
		}

		//Compute the CRC32 of the key
		keyCRC = CCrc32::Compute(szLowerCaseKey);
		if (m_cvarLocalizationDebug >= 3)
		{
			DrxLogAlways("<localization dupe/clash detection> CRC32: %8X, Key: %s", keyCRC, szLowerCaseKey);
		}

		if (m_pLanguage->m_keysMap.find(keyCRC) != m_pLanguage->m_keysMap.end())
		{
			sTmp.assign(sKeyString.ptr, sKeyString.count);
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "[LocError] Localized String '%s' Already Loaded for Language %s OR there is a CRC hash clash", sTmp.c_str(), m_pLanguage->sLanguage.c_str());
			continue;
		}

		SLocalizedStringEntry* pEntry;
		{
			MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "New SLocalizedStringEntry");
			pEntry = new SLocalizedStringEntry;
			pEntry->flags = 0;
		}
		if (bUseSubtitle == true)
		{
			pEntry->flags |= SLocalizedStringEntry::USE_SUBTITLE;
		}
		pEntry->nTagID = nTagID;

		if (gEnv->IsEditor())
		{
			pEntry->pEditorExtension = new SLocalizedStringEntryEditorExtension();

			pEntry->pEditorExtension->sKey = szLowerCaseKey;

			pEntry->pEditorExtension->nRow = nRowIndex;

			if (!sActorLine.empty())
			{
				MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "pEntry->sOriginalActorLine");
				sTmp.assign(sActorLine.ptr, sActorLine.count);
				ReplaceEndOfLine(sTmp);
				pEntry->pEditorExtension->sOriginalActorLine.assign(sTmp.c_str());
			}
			if (!sTranslatedActorLine.empty())
			{
				MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "pEntry->sUtf8TranslatedActorLine");
				sTmp.assign(sTranslatedActorLine.ptr, sTranslatedActorLine.count);
				ReplaceEndOfLine(sTmp);
				pEntry->pEditorExtension->sUtf8TranslatedActorLine.append(sTmp.c_str());
			}
			if (bUseSubtitle && !sSubtitleText.empty())
			{
				MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "pEntry->sOriginalText");
				sTmp.assign(sSubtitleText.ptr, sSubtitleText.count);
				ReplaceEndOfLine(sTmp);
				pEntry->pEditorExtension->sOriginalText.assign(sTmp.c_str());
			}
			// only use the translated character name
			{
				MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "pEntry->sOriginalCharacterName");
				pEntry->pEditorExtension->sOriginalCharacterName.assign(sCharacterName.ptr, sCharacterName.count);
			}
		}

		if (bUseSubtitle && !sTranslatedText.empty())
		{
			MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "pEntry->sUtf8TranslatedText");
			sTmp.assign(sTranslatedText.ptr, sTranslatedText.count);
			ReplaceEndOfLine(sTmp);
			if (m_cvarLocalizationEncode == 1)
			{
				pEncoder->Update((u8k*)(sTmp.c_str()), sTmp.length());
				//DrxLogAlways("%u Storing %s (%u)", m_pLanguage->m_vLocalizedStrings.size(), sTmp.c_str(), sTmp.length());
				pEntry->TranslatedText.szCompressed = new u8[sTmp.length() + 1];
				pEntry->flags |= SLocalizedStringEntry::IS_COMPRESSED;
				//Store the raw string. It'll be compressed later
				memcpy(pEntry->TranslatedText.szCompressed, sTmp.c_str(), sTmp.length());
				pEntry->TranslatedText.szCompressed[sTmp.length()] = '\0';  //Null terminate
			}
			else
			{
				pEntry->TranslatedText.psUtf8Uncompressed = new string(sTmp.c_str(), sTmp.c_str() + sTmp.length());
			}
		}

		// the following is used to cleverly assign strings
		// we store all known string into the m_prototypeEvents set and assign known entries from there
		// the DrxString makes sure, that only the ref-count is increment on assignment
		if (*szLowerCaseEvent)
		{
			PrototypeSoundEvents::iterator it = m_prototypeEvents.find(CONST_TEMP_STRING(szLowerCaseEvent));
			if (it != m_prototypeEvents.end())
				pEntry->sPrototypeSoundEvent = *it;
			else
			{
				MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "pEntry->sPrototypeSoundEvent");
				pEntry->sPrototypeSoundEvent = szLowerCaseEvent;
				m_prototypeEvents.insert(pEntry->sPrototypeSoundEvent);
			}
		}

		const CConstCharArray sWho = sTranslatedCharacterName.empty() ? sCharacterName : sTranslatedCharacterName;
		if (!sWho.empty())
		{
			sTmp.assign(sWho.ptr, sWho.count);
			ReplaceEndOfLine(sTmp);
			sTmp.replace(" ", "_");
			string tmp;
			{
				MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Allocating tmp for pEntry->sCharacterName");
				tmp = sTmp.c_str();
			}
			CharacterNameSet::iterator it = m_characterNameSet.find(tmp);
			if (it != m_characterNameSet.end())
				pEntry->sCharacterName = *it;
			else
			{
				{
					MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Assigning pEntry->sCharacterName");
					pEntry->sCharacterName = tmp;
				}
				{
					MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "m_characterNameSet.insert(pEntry->sCharacterName)");
					m_characterNameSet.insert(pEntry->sCharacterName);
				}
			}
		}

		pEntry->fVolume = DrxConvertFloatToHalf(fVolume);

		// SoundMood Entries
		{
			MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "pEntry->SoundMoods");
			pEntry->SoundMoods.resize(SoundMoodValues.size());
			if (SoundMoodValues.size() > 0)
			{
				std::map<i32, float>::const_iterator itEnd = SoundMoodValues.end();
				i32 nSoundMoodCount = 0;
				for (std::map<i32, float>::const_iterator it = SoundMoodValues.begin(); it != itEnd; ++it)
				{
					pEntry->SoundMoods[nSoundMoodCount].fValue = (*it).second;
					pEntry->SoundMoods[nSoundMoodCount].sName = SoundMoodIndex[(*it).first];
					++nSoundMoodCount;
				}
			}
		}

		// EventParameter Entries
		{
			MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "pEntry->EventParameters");
			pEntry->EventParameters.resize(EventParameterValues.size());
			if (EventParameterValues.size() > 0)
			{
				std::map<i32, float>::const_iterator itEnd = EventParameterValues.end();
				i32 nEventParameterCount = 0;
				for (std::map<i32, float>::const_iterator it = EventParameterValues.begin(); it != itEnd; ++it)
				{
					pEntry->EventParameters[nEventParameterCount].fValue = (*it).second;
					pEntry->EventParameters[nEventParameterCount].sName = EventParameterIndex[(*it).first];
					++nEventParameterCount;
				}
			}
		}

		pEntry->fRadioRatio = DrxConvertFloatToHalf(fRadioRatio);

		if (bIsDirectRadio == true)
		{
			pEntry->flags |= SLocalizedStringEntry::IS_DIRECTED_RADIO;
		}
		if (bIsIntercepted == true)
		{
			pEntry->flags |= SLocalizedStringEntry::IS_INTERCEPTED;
		}

		nMemSize += sizeof(*pEntry) + pEntry->sCharacterName.length() * sizeof(char);
		if (m_cvarLocalizationEncode == 0)
		{
			//Note that this isn't accurate if we're using encoding/compression to shrink the string as the encoding step hasn't happened yet
			if (pEntry->TranslatedText.psUtf8Uncompressed)
			{
				nMemSize += pEntry->TranslatedText.psUtf8Uncompressed->length() * sizeof(char);
			}
		}
		if (pEntry->pEditorExtension != NULL)
		{
			nMemSize += pEntry->pEditorExtension->sKey.length()
			            + pEntry->pEditorExtension->sOriginalActorLine.length()
			            + pEntry->pEditorExtension->sUtf8TranslatedActorLine.length() * sizeof(char)
			            + pEntry->pEditorExtension->sOriginalText.length()
			            + pEntry->pEditorExtension->sOriginalCharacterName.length();
		}

		// Compression Preparation
		//u32 nSourceSize = pEntry->swTranslatedText.length()*sizeof(wchar_t);
		//if (nSourceSize)
		//	i32 zResult = Compress(pDest, nDestLen, pEntry->swTranslatedText.c_str(), nSourceSize);

		{
			MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "AddLocalizedString");
			AddLocalizedString(m_pLanguage, pEntry, keyCRC);
		}
	}

	if (m_cvarLocalizationEncode == 1)
	{
		{
			MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Finalizing encoder");
			pEncoder->Finalize();
		}

		{
			MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Compressing Strings (Alloc Storage)");
			u8 compressionBuffer[COMPRESSION_FIXED_BUFFER_LENGTH];
			//u8 decompressionBuffer[COMPRESSION_FIXED_BUFFER_LENGTH];
			size_t uncompressedTotal = 0, compressedTotal = 0;
			for (size_t stringToCompress = startOfStringsToCompress; stringToCompress < m_pLanguage->m_vLocalizedStrings.size(); stringToCompress++)
			{
				SLocalizedStringEntry* pStringToCompress = m_pLanguage->m_vLocalizedStrings[stringToCompress];
				if (pStringToCompress->TranslatedText.szCompressed != NULL)
				{
					size_t compBufSize = COMPRESSION_FIXED_BUFFER_LENGTH;
					memset(compressionBuffer, 0, COMPRESSION_FIXED_BUFFER_LENGTH);
					//DrxLogAlways("%u Compressing %s (%p)", stringToCompress, pStringToCompress->szCompressedTranslatedText, pStringToCompress->szCompressedTranslatedText);
					size_t inputStringLength = strlen((tukk)(pStringToCompress->TranslatedText.szCompressed));
					pEncoder->CompressInput(pStringToCompress->TranslatedText.szCompressed, inputStringLength, compressionBuffer, &compBufSize);
					// cppcheck-suppress arrayIndexOutOfBounds
					compressionBuffer[compBufSize] = 0;
					pStringToCompress->huffmanTreeIndex = iEncoder;
					pEncoder->AddRef();
					//DrxLogAlways("Compressed %s (%u) to %s (%u)", pStringToCompress->szCompressedTranslatedText, strlen((tukk)pStringToCompress->szCompressedTranslatedText), compressionBuffer, compBufSize);
					uncompressedTotal += inputStringLength;
					compressedTotal += compBufSize;

					u8* szCompressedString = new u8[compBufSize];
					SAFE_DELETE_ARRAY(pStringToCompress->TranslatedText.szCompressed);
					memcpy(szCompressedString, compressionBuffer, compBufSize);
					pStringToCompress->TranslatedText.szCompressed = szCompressedString;

					//Testing code
					//memset( decompressionBuffer, 0, COMPRESSION_FIXED_BUFFER_LENGTH );
					//size_t decompBufSize = pEncoder->UncompressInput(compressionBuffer, COMPRESSION_FIXED_BUFFER_LENGTH, decompressionBuffer, COMPRESSION_FIXED_BUFFER_LENGTH);
					//DrxLogAlways("Decompressed %s (%u) to %s (%u)", compressionBuffer, compBufSize, decompressionBuffer, decompBufSize);
				}
			}
			//DrxLogAlways("[LOC PROFILING] %s, %u, Uncompressed %u, Compressed %u", sFileName, m_pLanguage->m_vLocalizedStrings.size() - startOfStringsToCompress, uncompressedTotal, compressedTotal);
		}
	}

	pXmlTableReader->Release();

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CLocalizedStringsUpr::ReloadData()
{
	tmapFilenames temp = m_loadedTables;

	FreeLocalizationData();
	for (tmapFilenames::iterator it = temp.begin(); it != temp.end(); ++it)
	{
		DoLoadExcelXmlSpreadsheet((*it).first, (*it).second.nTagID, true);
	}
}

//////////////////////////////////////////////////////////////////////////
void CLocalizedStringsUpr::AddLocalizedString(SLanguage* pLanguage, SLocalizedStringEntry* pEntry, u32k keyCRC32)
{
	{
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "m_vLocalizedStrings push back");
		pLanguage->m_vLocalizedStrings.push_back(pEntry);
	}
	i32 nId = (i32)pLanguage->m_vLocalizedStrings.size() - 1;
	{
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Language Key Map Entry");
		pLanguage->m_keysMap[keyCRC32] = pEntry;
	}

	if (m_cvarLocalizationDebug >= 2)
		DrxLog("<Localization> Add new string <%u> with ID %d to <%s>", keyCRC32, nId, pLanguage->sLanguage.c_str());
}

//////////////////////////////////////////////////////////////////////////
bool CLocalizedStringsUpr::LocalizeString(tukk sString, string& outLocalizedString, bool bEnglish)
{
	return LocalizeStringInternal(sString, strlen(sString), outLocalizedString, bEnglish);
}

//////////////////////////////////////////////////////////////////////////
bool CLocalizedStringsUpr::LocalizeString(const string& sString, string& outLocalizedString, bool bEnglish)
{
	return LocalizeStringInternal(sString.c_str(), sString.length(), outLocalizedString, bEnglish);
}

//////////////////////////////////////////////////////////////////////////
bool CLocalizedStringsUpr::LocalizeStringInternal(tukk pStr, size_t len, string& outLocalizedString, bool bEnglish)
{
	assert(m_pLanguage);
	if (m_pLanguage == 0)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "LocalizeString: No language set.");
		outLocalizedString.assign(pStr, pStr + len);
		return false;
	}

	// note: we don't write directly to outLocalizedString, in case it aliases pStr
	string out;

	// scan the string
	bool done = false;
	i32 curpos = 0;
	tukk pPos = pStr;
	tukk pEnd = pStr + len;
	while (true)
	{
		tukk pLabel = strchr(pPos, '@');
		if (!pLabel) break;
		// found an occurrence

		// we have skipped a few characters, so copy them over
		if (pLabel != pPos)
		{
			out.append(pPos, pLabel);
		}

		// find the end of the label
		tukk pFirstAngleBracket = strchr(pLabel, '<');
		tukk pFirstSpace = strchr(pLabel, ' ');
		tukk pLabelEnd = pEnd;
		if (pFirstAngleBracket && pFirstSpace)
		{
			pLabelEnd = min(pFirstSpace, pFirstAngleBracket);
		}
		else if (pFirstAngleBracket)
		{
			pLabelEnd = pFirstAngleBracket;
		}
		else if (pFirstSpace)
		{
			pLabelEnd = pFirstSpace;
		}

		// localize token
		string token(pLabel, pLabelEnd);
		string sLocalizedToken;
		if (bEnglish)
		{
			GetEnglishString(token.c_str(), sLocalizedToken);
		}
		else
		{
			LocalizeLabel(token.c_str(), sLocalizedToken);
		}

		for (PostProcessors::Notifier notifier(m_postProcessors); notifier.IsValid(); notifier.Next())
		{
			notifier->PostProcessString(sLocalizedToken);
		}

		out.append(sLocalizedToken);
		pPos = pLabelEnd;
	}
	out.append(pPos, pEnd);
	out.swap(outLocalizedString);
	return true;
}

#if defined(LOG_DECOMP_TIMES)
static double g_fSecondsPerTick = 0.0;
static FILE* pDecompLog = NULL;
// engine independent game timer since gEnv/pSystem isn't available yet
static void LogDecompTimer(__int64 nTotalTicks, __int64 nDecompTicks, __int64 nAllocTicks)
{
	if (g_fSecondsPerTick == 0.0)
	{
		__int64 nPerfFreq;
		QueryPerformanceFrequency((LARGE_INTEGER*)&nPerfFreq);
		g_fSecondsPerTick = 1.0 / (double)nPerfFreq;
	}

	if (!pDecompLog)
	{
		char szFilenameBuffer[MAX_PATH];
		time_t rawTime;
		time(&rawTime);
		struct tm* pTimeInfo = localtime(&rawTime);

		CreateDirectory("TestResults\\", 0);
		strftime(szFilenameBuffer, sizeof(szFilenameBuffer), "TestResults\\Decomp_%Y_%m_%d-%H_%M_%S.csv", pTimeInfo);
		pDecompLog = fopen(szFilenameBuffer, "wb");
		fprintf(pDecompLog, "Total,Decomp,Alloc\n");
	}
	float nTotalMillis = float(g_fSecondsPerTick * 1000.0 * nTotalTicks);
	float nDecompMillis = float(g_fSecondsPerTick * 1000.0 * nDecompTicks);
	float nAllocMillis = float(g_fSecondsPerTick * 1000.0 * nAllocTicks);
	fprintf(pDecompLog, "%f,%f,%f\n", nTotalMillis, nDecompMillis, nAllocMillis);
	fflush(pDecompLog);
}
#endif

string CLocalizedStringsUpr::SLocalizedStringEntry::GetTranslatedText(const SLanguage* pLanguage) const
{
	DRX_PROFILE_FUNCTION(PROFILE_SYSTEM);
	if ((flags & IS_COMPRESSED) != 0)
	{
#if defined(LOG_DECOMP_TIMES)
		__int64 nTotalTicks, nDecompTicks, nAllocTicks;
		nTotalTicks = DrxGetTicks();
#endif  //LOG_DECOMP_TIMES

		string outputString;
		if (TranslatedText.szCompressed != NULL)
		{
			u8 decompressionBuffer[COMPRESSION_FIXED_BUFFER_LENGTH];
			HuffmanCoder* pEncoder = pLanguage->m_vEncoders[huffmanTreeIndex];

#if defined(LOG_DECOMP_TIMES)
			nDecompTicks = DrxGetTicks();
#endif  //LOG_DECOMP_TIMES

			//We don't actually know how much memory was allocated for this string, but the maximum compression buffer size is known
			size_t decompBufSize = pEncoder->UncompressInput(TranslatedText.szCompressed, COMPRESSION_FIXED_BUFFER_LENGTH, decompressionBuffer, COMPRESSION_FIXED_BUFFER_LENGTH);
			assert(decompBufSize < COMPRESSION_FIXED_BUFFER_LENGTH && "Buffer overflow");

#if defined(LOG_DECOMP_TIMES)
			nDecompTicks = DrxGetTicks() - nDecompTicks;
#endif  //LOG_DECOMP_TIMES

			size_t len = strnlen((tukk)decompressionBuffer, COMPRESSION_FIXED_BUFFER_LENGTH);
			assert(len < COMPRESSION_FIXED_BUFFER_LENGTH && "Buffer not null-terminated");

#if defined(LOG_DECOMP_TIMES)
			nAllocTicks = DrxGetTicks();
#endif  //LOG_DECOMP_TIMES

			outputString.assign((tukk)decompressionBuffer, (tukk)decompressionBuffer + decompBufSize);

#if defined(LOG_DECOMP_TIMES)
			nAllocTicks = DrxGetTicks() - nAllocTicks;
			nTotalTicks = DrxGetTicks() - nTotalTicks;
			LogDecompTimer(nTotalTicks, nDecompTicks, nAllocTicks);
#endif  //LOG_DECOMP_TIMES
		}
		return outputString;
	}
	else
	{
		if (TranslatedText.psUtf8Uncompressed != NULL)
		{
			return *TranslatedText.psUtf8Uncompressed;
		}
		else
		{
			string emptyOutputString;
			return emptyOutputString;
		}
	}
}

#ifndef _RELEASE
//////////////////////////////////////////////////////////////////////////
void CLocalizedStringsUpr::LocalizedStringsUprWarning(tukk label, tukk message)
{
	if (!m_warnedAboutLabels[label])
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "Failed to localize label '%s' - %s", label, message);
		m_warnedAboutLabels[label] = true;
		m_haveWarnedAboutAtLeastOneLabel = true;
	}
}

//////////////////////////////////////////////////////////////////////////
void CLocalizedStringsUpr::ListAndClearProblemLabels()
{
	if (m_haveWarnedAboutAtLeastOneLabel)
	{
		DrxLog("These labels caused localization problems:");
		INDENT_LOG_DURING_SCOPE();

		for (std::map<string, bool>::iterator iter = m_warnedAboutLabels.begin(); iter != m_warnedAboutLabels.end(); ++iter)
		{
			DrxLog("%s", iter->first.c_str());
		}

		m_warnedAboutLabels.clear();
		m_haveWarnedAboutAtLeastOneLabel = false;
	}
}
#endif

//////////////////////////////////////////////////////////////////////////
bool CLocalizedStringsUpr::LocalizeLabel(tukk sLabel, string& outLocalString, bool bEnglish)
{
	assert(sLabel);
	if (!m_pLanguage || !sLabel)
		return false;

	// Label sign.
	if (sLabel[0] == '@')
	{
		u32 labelCRC32 = CCrc32::ComputeLowercase(sLabel + 1);
		{
			AutoLock lock(m_cs);                                                                       //Lock here, to prevent strings etc being modified underneath this lookup
			SLocalizedStringEntry* entry = stl::find_in_map(m_pLanguage->m_keysMap, labelCRC32, NULL); // skip @ character.

			// if it continues with cc_ it's a control code
			if (sLabel[1] && (sLabel[1] == 'c' || sLabel[1] == 'C') &&
			    sLabel[2] && (sLabel[2] == 'c' || sLabel[2] == 'C') &&
			    sLabel[3] && sLabel[3] == '_')
			{
				if (entry == NULL)
				{
					// controlcode
					// lookup KeyName
					IInput* pInput = gEnv->pInput;
					SInputEvent ev;
					ev.deviceType = eIDT_Keyboard;
					ev.keyName = sLabel + 4; // skip @cc_
					u32 inputCharUTF32 = pInput->GetInputCharUnicode(ev);

					if (inputCharUTF32 == 0)
					{
						// try OS key
						tukk keyName = pInput->GetOSKeyName(ev);
						if (keyName && *keyName)
						{
							outLocalString.assign(keyName);
							return true;
						}
						// if we got some empty, try non-keyboard as well
						ev.deviceType = eIDT_Unknown;
						inputCharUTF32 = pInput->GetInputCharUnicode(ev);
					}

					if (inputCharUTF32)
					{
						// Note: Since the input char is assumed to be ASCII, the uppercasing is trivial
						if (inputCharUTF32 >= 'a' && inputCharUTF32 <= 'z') inputCharUTF32 = inputCharUTF32 - 'a' + 'A';
						Unicode::Convert(outLocalString, &inputCharUTF32, &inputCharUTF32 + 1);
						return true;
					}
				}
				// we found an localized entry (e.g. @cc_space), use normal translation -> Space/Leertaste
			}

			if (entry != NULL)
			{
				string translatedText = entry->GetTranslatedText(m_pLanguage);
				if ((bEnglish || translatedText.empty()) && entry->pEditorExtension != NULL)
				{
					//assert(!"No Localization Text available!");
					outLocalString.assign(entry->pEditorExtension->sOriginalText);
					return true;
				}
				else
				{
					outLocalString.swap(translatedText);
				}
				return true;
			}
			else
			{
				LocalizedStringsUprWarning(sLabel, "entry not found in string table");
			}
		}
	}
	else
	{
		LocalizedStringsUprWarning(sLabel, "must start with @ symbol");
	}

	outLocalString.assign(sLabel);

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CLocalizedStringsUpr::GetEnglishString(tukk sKey, string& sLocalizedString)
{
	assert(sKey);
	if (!m_pLanguage || !sKey)
		return false;

	// Label sign.
	if (sKey[0] == '@')
	{
		u32 keyCRC32 = CCrc32::ComputeLowercase(sKey + 1);
		{
			AutoLock lock(m_cs);                                                                     //Lock here, to prevent strings etc being modified underneath this lookup
			SLocalizedStringEntry* entry = stl::find_in_map(m_pLanguage->m_keysMap, keyCRC32, NULL); // skip @ character.
			if (entry != NULL && entry->pEditorExtension != NULL)
			{
				sLocalizedString = entry->pEditorExtension->sOriginalText;
				return true;
			}
			else
			{
				keyCRC32 = CCrc32::ComputeLowercase(sKey);
				entry = stl::find_in_map(m_pLanguage->m_keysMap, keyCRC32, NULL);
				if (entry != NULL && entry->pEditorExtension != NULL)
				{
					sLocalizedString = entry->pEditorExtension->sOriginalText;
					return true;
				}
				else
				{
					// DrxWarning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,"Localized string for Label <%s> not found", sKey );
					sLocalizedString = sKey;
					return false;
				}
			}
		}
	}
	else
	{
		// DrxWarning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,"Not a valid localized string Label <%s>, must start with @ symbol", sKey );
	}

	sLocalizedString = sKey;
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CLocalizedStringsUpr::GetLocalizedInfoByKey(tukk sKey, SLocalizedInfoGame& outGameInfo)
{
	if (!m_pLanguage)
		return false;

	u32 keyCRC32 = CCrc32::ComputeLowercase(sKey);
	{
		AutoLock lock(m_cs);  //Lock here, to prevent strings etc being modified underneath this lookup
		const SLocalizedStringEntry* entry = stl::find_in_map(m_pLanguage->m_keysMap, keyCRC32, NULL);
		if (entry != NULL)
		{
			outGameInfo.szCharacterName = entry->sCharacterName;
			outGameInfo.sUtf8TranslatedText = entry->GetTranslatedText(m_pLanguage);

			outGameInfo.bUseSubtitle = (entry->flags & SLocalizedStringEntry::USE_SUBTITLE);

			return true;
		}
		else
			return false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CLocalizedStringsUpr::GetLocalizedInfoByKey(tukk sKey, SLocalizedSoundInfoGame* pOutSoundInfo)
{
	assert(sKey);
	if (!m_pLanguage || !sKey || !pOutSoundInfo)
		return false;

	bool bResult = false;

	u32 keyCRC32 = CCrc32::ComputeLowercase(sKey);
	{
		AutoLock lock(m_cs);  //Lock here, to prevent strings etc being modified underneath this lookup
		const SLocalizedStringEntry* pEntry = stl::find_in_map(m_pLanguage->m_keysMap, keyCRC32, NULL);
		if (pEntry != NULL)
		{
			bResult = true;

			pOutSoundInfo->szCharacterName = pEntry->sCharacterName;
			pOutSoundInfo->sUtf8TranslatedText = pEntry->GetTranslatedText(m_pLanguage);

			//pOutSoundInfo->sOriginalActorLine = pEntry->sOriginalActorLine.c_str();
			//pOutSoundInfo->sTranslatedActorLine = pEntry->swTranslatedActorLine.c_str();
			//pOutSoundInfo->sOriginalText = pEntry->sOriginalText;
			// original Character

			pOutSoundInfo->sSoundEvent = pEntry->sPrototypeSoundEvent.c_str();
			pOutSoundInfo->fVolume = DrxConvertHalfToFloat(pEntry->fVolume);
			pOutSoundInfo->fRadioRatio = DrxConvertHalfToFloat(pEntry->fRadioRatio);
			pOutSoundInfo->bUseSubtitle = (pEntry->flags & SLocalizedStringEntry::USE_SUBTITLE) != 0;
			pOutSoundInfo->bIsDirectRadio = (pEntry->flags & SLocalizedStringEntry::IS_DIRECTED_RADIO) != 0;
			pOutSoundInfo->bIsIntercepted = (pEntry->flags & SLocalizedStringEntry::IS_INTERCEPTED) != 0;

			//SoundMoods
			if (pOutSoundInfo->nNumSoundMoods >= pEntry->SoundMoods.size())
			{
				// enough space to copy data
				i32 i = 0;
				for (; i < pEntry->SoundMoods.size(); ++i)
				{
					pOutSoundInfo->pSoundMoods[i].sName = pEntry->SoundMoods[i].sName;
					pOutSoundInfo->pSoundMoods[i].fValue = pEntry->SoundMoods[i].fValue;
				}
				// if mode is available fill it with default
				for (; i < pOutSoundInfo->nNumSoundMoods; ++i)
				{
					pOutSoundInfo->pSoundMoods[i].sName = "";
					pOutSoundInfo->pSoundMoods[i].fValue = 0.0f;
				}
				pOutSoundInfo->nNumSoundMoods = pEntry->SoundMoods.size();
			}
			else
			{
				// not enough memory, say what is needed
				pOutSoundInfo->nNumSoundMoods = pEntry->SoundMoods.size();
				bResult = (pOutSoundInfo->pSoundMoods == NULL); // only report error if memory was provided but is too small
			}

			//EventParameters
			if (pOutSoundInfo->nNumEventParameters >= pEntry->EventParameters.size())
			{
				// enough space to copy data
				i32 i = 0;
				for (; i < pEntry->EventParameters.size(); ++i)
				{
					pOutSoundInfo->pEventParameters[i].sName = pEntry->EventParameters[i].sName;
					pOutSoundInfo->pEventParameters[i].fValue = pEntry->EventParameters[i].fValue;
				}
				// if mode is available fill it with default
				for (; i < pOutSoundInfo->nNumEventParameters; ++i)
				{
					pOutSoundInfo->pEventParameters[i].sName = "";
					pOutSoundInfo->pEventParameters[i].fValue = 0.0f;
				}
				pOutSoundInfo->nNumEventParameters = pEntry->EventParameters.size();
			}
			else
			{
				// not enough memory, say what is needed
				pOutSoundInfo->nNumEventParameters = pEntry->EventParameters.size();
				bResult = (pOutSoundInfo->pSoundMoods == NULL); // only report error if memory was provided but is too small
			}
		}
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////////
i32 CLocalizedStringsUpr::GetLocalizedStringCount()
{
	if (!m_pLanguage)
		return 0;
	return m_pLanguage->m_vLocalizedStrings.size();
}

//////////////////////////////////////////////////////////////////////////
bool CLocalizedStringsUpr::GetLocalizedInfoByIndex(i32 nIndex, SLocalizedInfoGame& outGameInfo)
{
	if (!m_pLanguage)
		return false;
	const std::vector<SLocalizedStringEntry*>& entryVec = m_pLanguage->m_vLocalizedStrings;
	if (nIndex < 0 || nIndex >= (i32)entryVec.size())
		return false;
	const SLocalizedStringEntry* pEntry = entryVec[nIndex];

	outGameInfo.szCharacterName = pEntry->sCharacterName;
	outGameInfo.sUtf8TranslatedText = pEntry->GetTranslatedText(m_pLanguage);

	outGameInfo.bUseSubtitle = (pEntry->flags & SLocalizedStringEntry::USE_SUBTITLE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CLocalizedStringsUpr::GetLocalizedInfoByIndex(i32 nIndex, SLocalizedInfoEditor& outEditorInfo)
{
	if (!m_pLanguage)
		return false;
	const std::vector<SLocalizedStringEntry*>& entryVec = m_pLanguage->m_vLocalizedStrings;
	if (nIndex < 0 || nIndex >= (i32)entryVec.size())
		return false;
	const SLocalizedStringEntry* pEntry = entryVec[nIndex];
	outEditorInfo.szCharacterName = pEntry->sCharacterName;
	outEditorInfo.sUtf8TranslatedText = pEntry->GetTranslatedText(m_pLanguage);

	assert(pEntry->pEditorExtension != NULL);

	outEditorInfo.sKey = pEntry->pEditorExtension->sKey;

	outEditorInfo.sOriginalActorLine = pEntry->pEditorExtension->sOriginalActorLine;
	outEditorInfo.sUtf8TranslatedActorLine = pEntry->pEditorExtension->sUtf8TranslatedActorLine;

	//outEditorInfo.sOriginalText = pEntry->sOriginalText;
	outEditorInfo.sOriginalCharacterName = pEntry->pEditorExtension->sOriginalCharacterName;

	outEditorInfo.nRow = pEntry->pEditorExtension->nRow;
	outEditorInfo.bUseSubtitle = (pEntry->flags & SLocalizedStringEntry::USE_SUBTITLE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CLocalizedStringsUpr::GetSubtitle(tukk sKeyOrLabel, string& outSubtitle, bool bForceSubtitle)
{
	assert(sKeyOrLabel);
	if (!m_pLanguage || !sKeyOrLabel || !*sKeyOrLabel)
		return false;
	if (*sKeyOrLabel == '@')
		++sKeyOrLabel;

	u32 keyCRC32 = CCrc32::ComputeLowercase(sKeyOrLabel);
	{
		AutoLock lock(m_cs);  //Lock here, to prevent strings etc being modified underneath this lookup
		const SLocalizedStringEntry* pEntry = stl::find_in_map(m_pLanguage->m_keysMap, keyCRC32, NULL);
		if (pEntry != NULL)
		{
			if ((pEntry->flags & SLocalizedStringEntry::USE_SUBTITLE) == false && !bForceSubtitle)
				return false;

			// TODO verify that no fallback is needed

			outSubtitle = pEntry->GetTranslatedText(m_pLanguage);

			if (outSubtitle.empty() == true)
			{
				if (pEntry->pEditorExtension != NULL && pEntry->pEditorExtension->sUtf8TranslatedActorLine.empty() == false)
				{
					outSubtitle = pEntry->pEditorExtension->sUtf8TranslatedActorLine;
				}
				else if (pEntry->pEditorExtension != NULL && pEntry->pEditorExtension->sOriginalText.empty() == false)
				{
					outSubtitle = pEntry->pEditorExtension->sOriginalText;
				}
				else if (pEntry->pEditorExtension != NULL && pEntry->pEditorExtension->sOriginalActorLine.empty() == false)
				{
					outSubtitle = pEntry->pEditorExtension->sOriginalActorLine;
				}
			}
			return true;
		}
		return false;
	}
}

template<typename StringClass, typename CharType>
void _InternalFormatStringMessage(StringClass& outString, const StringClass& sString, const CharType** sParams, i32 nParams)
{
	static const CharType token = (CharType) '%';
	static const CharType tokens1[2] = { token, (CharType) '\0' };
	static const CharType tokens2[3] = { token, token, (CharType) '\0' };

	i32 maxArgUsed = 0;
	i32 lastPos = 0;
	i32 curPos = 0;
	i32k sourceLen = sString.length();
	while (true)
	{
		size_t foundPos = sString.find(token, curPos);
		if (foundPos != string::npos)
		{
			if (foundPos + 1 < sourceLen)
			{
				i32k nArg = sString[foundPos + 1] - '1';
				if (nArg >= 0 && nArg <= 9)
				{
					if (nArg < nParams)
					{
						outString.append(sString, lastPos, foundPos - lastPos);
						outString.append(sParams[nArg]);
						curPos = foundPos + 2;
						lastPos = curPos;
						maxArgUsed = std::max(maxArgUsed, nArg);
					}
					else
					{
						StringClass tmp(sString);
						tmp.replace(tokens1, tokens2);
						if (sizeof(*tmp.c_str()) == sizeof(char))
							DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "Parameter for argument %d is missing. [%s]", nArg + 1, (tukk)tmp.c_str());
						else
							DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "Parameter for argument %d is missing. [%S]", nArg + 1, (const wchar_t*)tmp.c_str());
						curPos = foundPos + 1;
					}
				}
				else
					curPos = foundPos + 1;
			}
			else
				curPos = foundPos + 1;
		}
		else
		{
			outString.append(sString, lastPos, sourceLen - lastPos);
			break;
		}
	}
}

template<typename StringClass, typename CharType>
void _InternalFormatStringMessage(StringClass& outString, const StringClass& sString, const CharType* param1, const CharType* param2 = 0, const CharType* param3 = 0, const CharType* param4 = 0)
{
	static i32k MAX_PARAMS = 4;
	const CharType* params[MAX_PARAMS] = { param1, param2, param3, param4 };
	i32 nParams = 0;
	while (nParams < MAX_PARAMS && params[nParams])
		++nParams;
	_InternalFormatStringMessage(outString, sString, params, nParams);
}

//////////////////////////////////////////////////////////////////////////
void CLocalizedStringsUpr::FormatStringMessage(string& outString, const string& sString, tukk* sParams, i32 nParams)
{
	_InternalFormatStringMessage(outString, sString, sParams, nParams);
}

//////////////////////////////////////////////////////////////////////////
void CLocalizedStringsUpr::FormatStringMessage(string& outString, const string& sString, tukk param1, tukk param2, tukk param3, tukk param4)
{
	_InternalFormatStringMessage(outString, sString, param1, param2, param3, param4);
}

//////////////////////////////////////////////////////////////////////////
i32 CLocalizedStringsUpr::GetMemoryUsage(IDrxSizer* pSizer)
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_languages);
	pSizer->AddObject(m_prototypeEvents);
	pSizer->AddObject(m_characterNameSet);
	pSizer->AddObject(m_pLanguage);

	return 0;
}

#if DRX_PLATFORM_WINDOWS
namespace
{
struct LanguageID
{
	tukk   language;
	u64 lcID;
};

LanguageID languageIDArray[] =
{
	{ "ChineseT",  0x404 },   // 1028
	{ "Czech",     0x405 },   // 1029
	{ "English",   0x409 },   // 1033
	{ "French",    0x40C },   // 1036
	{ "German",    0x407 },   // 1031
	{ "Hungarian", 0x40E },   // 1038
	{ "Italian",   0x410 },   // 1040
	{ "Japanese",  0x411 },   // 1041
	{ "Korean",    0x412 },   // 1042
	{ "Polish",    0x415 },   // 1045
	{ "Russian",   0x419 },   // 1049
	{ "Spanish",   0x40A },   // 1034
	{ "Thai",      0x41E },   // 1054
	{ "Turkish",   0x41F },   // 1055
};

const size_t numLanguagesIDs = DRX_ARRAY_COUNT(languageIDArray);

LanguageID GetLanguageID(tukk language)
{
	// default is English (US)
	const LanguageID defaultLanguage = { "English", 0x409 };
	for (i32 i = 0; i < numLanguagesIDs; ++i)
	{
		if (stricmp(language, languageIDArray[i].language) == 0)
			return languageIDArray[i];
	}
	return defaultLanguage;
}

LanguageID g_currentLanguageID = { 0, 0 };
};
#endif

void CLocalizedStringsUpr::InternalSetCurrentLanguage(CLocalizedStringsUpr::SLanguage* pLanguage)
{
	m_pLanguage = pLanguage;
#if DRX_PLATFORM_WINDOWS
	if (m_pLanguage != 0)
		g_currentLanguageID = GetLanguageID(m_pLanguage->sLanguage);
	else
	{
		g_currentLanguageID.language = 0;
		g_currentLanguageID.lcID = 0;
	}
#endif
	// TODO: on Linux based systems we should now set the locale
	// Enabling this on windows something seems to get corrupted...
	// on Windows we always use Windows Platform Functions, which use the lcid
#if 0
	if (g_currentLanguageID.language)
	{
		tukk currentLocale = setlocale(LC_ALL, 0);
		if (0 == setlocale(LC_ALL, g_currentLanguageID.language))
			setlocale(LC_ALL, currentLocale);
	}
	else
		setlocale(LC_ALL, "C");
#endif
}

void CLocalizedStringsUpr::LocalizeDuration(i32 seconds, string& outDurationString)
{
	i32 s = seconds;
	i32 d, h, m;
	d = s / 86400;
	s -= d * 86400;
	h = s / 3600;
	s -= h * 3600;
	m = s / 60;
	s = s - m * 60;
	string str;
	if (d > 1)
		str.Format("%d @ui_days %02d:%02d:%02d", d, h, m, s);
	else if (d > 0)
		str.Format("%d @ui_day %02d:%02d:%02d", d, h, m, s);
	else if (h > 0)
		str.Format("%02d:%02d:%02d", h, m, s);
	else
		str.Format("%02d:%02d", m, s);
	LocalizeString(str, outDurationString);
}

void CLocalizedStringsUpr::LocalizeNumber(i32 number, string& outNumberString)
{
	if (number == 0)
	{
		outNumberString.assign("0");
		return;
	}

	outNumberString.assign("");

	i32 n = abs(number);
	string separator;
	DrxFixedStringT<64> tmp;
	LocalizeString("@ui_thousand_separator", separator);
	while (n > 0)
	{
		i32 a = n / 1000;
		i32 b = n - (a * 1000);
		if (a > 0)
		{
			tmp.Format("%s%03d%s", separator.c_str(), b, tmp.c_str());
		}
		else
		{
			tmp.Format("%d%s", b, tmp.c_str());
		}
		n = a;
	}

	if (number < 0)
	{
		tmp.Format("-%s", tmp.c_str());
	}

	outNumberString.assign(tmp.c_str());
}

void CLocalizedStringsUpr::LocalizeNumber(float number, i32 decimals, string& outNumberString)
{
	if (number == 0.0f)
	{
		DrxFixedStringT<64> tmp;
		tmp.Format("%.*f", decimals, number);
		outNumberString.assign(tmp.c_str());
		return;
	}

	outNumberString.assign("");

	string commaSeparator;
	LocalizeString("@ui_decimal_separator", commaSeparator);
	float f = number > 0.0f ? number : -number;
	i32 d = (i32)f;

	string intPart;
	LocalizeNumber(d, intPart);

	float decimalsOnly = f - (float)d;

	i32 decimalsAsInt = int_round(decimalsOnly * pow(10.0f, decimals));

	DrxFixedStringT<64> tmp;
	tmp.Format("%s%s%0*d", intPart.c_str(), commaSeparator.c_str(), decimals, decimalsAsInt);

	outNumberString.assign(tmp.c_str());
}

#if DRX_PLATFORM_WINDOWS
namespace
{
void UnixTimeToFileTime(time_t unixtime, FILETIME* filetime)
{
	LONGLONG longlong = Int32x32To64(unixtime, 10000000) + 116444736000000000;
	filetime->dwLowDateTime = (DWORD) longlong;
	filetime->dwHighDateTime = (DWORD)(longlong >> 32);
}

void UnixTimeToSystemTime(time_t unixtime, SYSTEMTIME* systemtime)
{
	FILETIME filetime;
	UnixTimeToFileTime(unixtime, &filetime);
	FileTimeToSystemTime(&filetime, systemtime);
}

time_t UnixTimeFromFileTime(const FILETIME* filetime)
{
	LONGLONG longlong = filetime->dwHighDateTime;
	longlong <<= 32;
	longlong |= filetime->dwLowDateTime;
	longlong -= 116444736000000000;
	return longlong / 10000000;
}

time_t UnixTimeFromSystemTime(const SYSTEMTIME* systemtime)
{
	// convert systemtime to filetime
	FILETIME filetime;
	SystemTimeToFileTime(systemtime, &filetime);
	// convert filetime to unixtime
	time_t unixtime = UnixTimeFromFileTime(&filetime);
	return unixtime;
}
};

void CLocalizedStringsUpr::LocalizeTime(time_t t, bool bMakeLocalTime, bool bShowSeconds, string& outTimeString)
{
	if (bMakeLocalTime)
	{
		struct tm thetime;
		thetime = *localtime(&t);
		t = gEnv->pTimer->DateToSecondsUTC(thetime);
	}
	outTimeString.resize(0);
	LCID lcID = g_currentLanguageID.lcID ? g_currentLanguageID.lcID : LOCALE_USER_DEFAULT;
	DWORD flags = bShowSeconds == false ? TIME_NOSECONDS : 0;
	SYSTEMTIME systemTime;
	UnixTimeToSystemTime(t, &systemTime);
	i32 len = ::GetTimeFormatW(lcID, flags, &systemTime, 0, 0, 0);
	if (len > 0)
	{
		// len includes terminating null!
		DrxFixedWStringT<256> tmpString;
		tmpString.resize(len);
		::GetTimeFormatW(lcID, flags, &systemTime, 0, (wchar_t*) tmpString.c_str(), len);
		Unicode::Convert(outTimeString, tmpString);
	}
}

void CLocalizedStringsUpr::LocalizeDate(time_t t, bool bMakeLocalTime, bool bShort, bool bIncludeWeekday, string& outDateString)
{
	if (bMakeLocalTime)
	{
		struct tm thetime;
		thetime = *localtime(&t);
		t = gEnv->pTimer->DateToSecondsUTC(thetime);
	}
	outDateString.resize(0);
	LCID lcID = g_currentLanguageID.lcID ? g_currentLanguageID.lcID : LOCALE_USER_DEFAULT;
	SYSTEMTIME systemTime;
	UnixTimeToSystemTime(t, &systemTime);

	// len includes terminating null!
	DrxFixedWStringT<256> tmpString;

	if (bIncludeWeekday)
	{
		// Get name of day
		i32 len = ::GetDateFormatW(lcID, 0, &systemTime, L"ddd", 0, 0);
		if (len > 0)
		{
			// len includes terminating null!
			tmpString.resize(len);
			::GetDateFormatW(lcID, 0, &systemTime, L"ddd", (wchar_t*) tmpString.c_str(), len);
			string utf8;
			Unicode::Convert(utf8, tmpString);
			outDateString.append(utf8);
			outDateString.append(" ");
		}
	}
	DWORD flags = bShort ? DATE_SHORTDATE : DATE_LONGDATE;
	i32 len = ::GetDateFormatW(lcID, flags, &systemTime, 0, 0, 0);
	if (len > 0)
	{
		// len includes terminating null!
		tmpString.resize(len);
		::GetDateFormatW(lcID, flags, &systemTime, 0, (wchar_t*) tmpString.c_str(), len);
		string utf8;
		Unicode::Convert(utf8, tmpString);
		outDateString.append(utf8);
	}
}

#else // #if DRX_PLATFORM_WINDOWS

void CLocalizedStringsUpr::LocalizeTime(time_t t, bool bMakeLocalTime, bool bShowSeconds, string& outTimeString)
{
	struct tm theTime;
	if (bMakeLocalTime)
		theTime = *localtime(&t);
	else
		theTime = *gmtime(&t);

	wchar_t buf[256];
	const size_t bufSize = DRX_ARRAY_COUNT(buf);
	wcsftime(buf, bufSize, bShowSeconds ? L"%#X" : L"%X", &theTime);
	buf[bufSize - 1] = 0;
	Unicode::Convert(outTimeString, buf);
}

void CLocalizedStringsUpr::LocalizeDate(time_t t, bool bMakeLocalTime, bool bShort, bool bIncludeWeekday, string& outDateString)
{
	struct tm theTime;
	if (bMakeLocalTime)
		theTime = *localtime(&t);
	else
		theTime = *gmtime(&t);

	wchar_t buf[256];
	const size_t bufSize = DRX_ARRAY_COUNT(buf);
	const wchar_t* format = bShort ? (bIncludeWeekday ? L"%a %x" : L"%x") : L"%#x"; // long format always contains Weekday name
	wcsftime(buf, bufSize, format, &theTime);
	buf[bufSize - 1] = 0;
	Unicode::Convert(outDateString, buf);
}

#endif
