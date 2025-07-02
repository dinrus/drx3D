// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

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

#ifndef __LocalizedStringUpr_h__
#define __LocalizedStringUpr_h__
#pragma once

#include <drx3D/Sys/ILocalizationUpr.h>
#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/CoreX/Containers/VectorMap.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>

#include <drx3D/Sys/Huffman.h>

//////////////////////////////////////////////////////////////////////////
/*
   Manage Localization Data
 */
class CLocalizedStringsUpr : public ILocalizationUpr, public ISystemEventListener
{
public:
	typedef std::vector<string> TLocalizationTagVec;

	const static size_t LOADING_FIXED_STRING_LENGTH = 1024;
	const static size_t COMPRESSION_FIXED_BUFFER_LENGTH = 3072;

	CLocalizedStringsUpr(ISystem* pSystem);
	virtual ~CLocalizedStringsUpr();

	// ILocalizationUpr
	virtual tukk                                 LangNameFromPILID(const ILocalizationUpr::EPlatformIndependentLanguageID id);
	virtual ILocalizationUpr::TLocalizationBitfield MaskSystemLanguagesFromSupportedLocalizations(const ILocalizationUpr::TLocalizationBitfield systemLanguages);
	virtual ILocalizationUpr::TLocalizationBitfield IsLanguageSupported(const ILocalizationUpr::EPlatformIndependentLanguageID id);

	virtual tukk                                 GetLanguage();
	virtual bool                                        SetLanguage(tukk sLanguage);

	virtual bool                                        InitLocalizationData(tukk sFileName, bool bReload = false);
	virtual bool                                        RequestLoadLocalizationDataByTag(tukk sTag);
	virtual bool                                        LoadLocalizationDataByTag(tukk sTag, bool bReload = false);
	virtual bool                                        ReleaseLocalizationDataByTag(tukk sTag);

	virtual void                                        RegisterPostProcessor(ILocalizationPostProcessor* pPostProcessor);
	virtual void                                        UnregisterPostProcessor(ILocalizationPostProcessor* pPostProcessor);

	virtual bool                                        LoadExcelXmlSpreadsheet(tukk sFileName, bool bReload = false);
	virtual void                                        ReloadData();
	virtual void                                        FreeData();

	virtual bool                                        LocalizeString(const string& sString, string& outLocalizedString, bool bEnglish = false);
	virtual bool                                        LocalizeString(tukk sString, string& outLocalizedString, bool bEnglish = false);
	virtual bool                                        LocalizeLabel(tukk sLabel, string& outLocalizedString, bool bEnglish = false);
	virtual bool                                        GetLocalizedInfoByKey(tukk sKey, SLocalizedInfoGame& outGameInfo);
	virtual bool                                        GetLocalizedInfoByKey(tukk sKey, SLocalizedSoundInfoGame* pOutSoundInfoGame);
	virtual i32                                         GetLocalizedStringCount();
	virtual bool                                        GetLocalizedInfoByIndex(i32 nIndex, SLocalizedInfoGame& outGameInfo);
	virtual bool                                        GetLocalizedInfoByIndex(i32 nIndex, SLocalizedInfoEditor& outEditorInfo);

	virtual bool                                        GetEnglishString(tukk sKey, string& sLocalizedString);
	virtual bool                                        GetSubtitle(tukk sKeyOrLabel, string& outSubtitle, bool bForceSubtitle = false);

	virtual void                                        FormatStringMessage(string& outString, const string& sString, tukk* sParams, i32 nParams);
	virtual void                                        FormatStringMessage(string& outString, const string& sString, tukk param1, tukk param2 = 0, tukk param3 = 0, tukk param4 = 0);

	virtual void                                        LocalizeTime(time_t t, bool bMakeLocalTime, bool bShowSeconds, string& outTimeString);
	virtual void                                        LocalizeDate(time_t t, bool bMakeLocalTime, bool bShort, bool bIncludeWeekday, string& outDateString);
	virtual void                                        LocalizeDuration(i32 seconds, string& outDurationString);
	virtual void                                        LocalizeNumber(i32 number, string& outNumberString);
	virtual void                                        LocalizeNumber(float number, i32 decimals, string& outNumberString);
	// ~ILocalizationUpr

	// ISystemEventUpr
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);
	// ~ISystemEventUpr

	i32  GetMemoryUsage(IDrxSizer* pSizer);

	void GetLoadedTags(TLocalizationTagVec& tags);
	void FreeLocalizationData();

#if !defined(_RELEASE)
	static void LocalizationDumpLoadedInfo(IConsoleCmdArgs* pArgs);
#endif //#if !defined(_RELEASE)

private:
	void SetAvailableLocalizationsBitfield(const ILocalizationUpr::TLocalizationBitfield availableLocalizations);

	bool LocalizeStringInternal(tukk pStr, size_t len, string& outLocalizedString, bool bEnglish);

	bool DoLoadExcelXmlSpreadsheet(tukk sFileName, u8 tagID, bool bReload);

	struct SLocalizedStringEntryEditorExtension
	{
		string       sKey;                     // Map key text equivalent (without @)
		string       sOriginalActorLine;       // english text
		string       sUtf8TranslatedActorLine; // localized text
		string       sOriginalText;            // subtitle. if empty, uses English text
		string       sOriginalCharacterName;   // english character name speaking via XML asset

		u32 nRow;                // Number of row in XML file

		void         GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(this, sizeof(*this));

			pSizer->AddObject(sKey);
			pSizer->AddObject(sOriginalActorLine);
			pSizer->AddObject(sUtf8TranslatedActorLine);
			pSizer->AddObject(sOriginalText);
			pSizer->AddObject(sOriginalCharacterName);
		}
	};

	struct SLanguage;

	//#define LOG_DECOMP_TIMES				//If defined, will log decompression times to a file

	struct SLocalizedStringEntry
	{
		//Flags
		enum
		{
			USE_SUBTITLE      = BIT(0), //should a subtitle displayed for this key?
			IS_DIRECTED_RADIO = BIT(1), //should the radio receiving hud be displayed?
			IS_INTERCEPTED    = BIT(2), //should the radio receiving hud show the interception display?
			IS_COMPRESSED     = BIT(3), //Translated text is compressed
		};

		union trans_text
		{
			string* psUtf8Uncompressed;
			u8*  szCompressed;     // Note that no size information is stored. This is for struct size optimization and unfortunately renders the size info inaccurate.
		};

		string     sCharacterName; // character name speaking via XML asset
		trans_text TranslatedText; // Subtitle of this line

		// audio specific part
		string  sPrototypeSoundEvent;       // associated sound event prototype (radio, ...)
		DrxHalf fVolume;
		DrxHalf fRadioRatio;
		// SoundMoods
		DynArray<SLocalizedAdvancesSoundEntry> SoundMoods;
		// EventParameters
		DynArray<SLocalizedAdvancesSoundEntry> EventParameters;
		// ~audio specific part

		// subtitle & radio flags
		u8 flags;

		// Index of Huffman tree for translated text. -1 = no tree assigned (error)
		int8  huffmanTreeIndex;

		u8 nTagID;

		// bool    bDependentTranslation; // if the english/localized text contains other localization labels

		//Additional information for Sandbox. Null in game
		SLocalizedStringEntryEditorExtension* pEditorExtension;

		SLocalizedStringEntry() :
			flags(0),
			huffmanTreeIndex(-1),
			pEditorExtension(NULL)
		{
			TranslatedText.psUtf8Uncompressed = NULL;
		};
		~SLocalizedStringEntry()
		{
			SAFE_DELETE(pEditorExtension);
			if ((flags & IS_COMPRESSED) == 0)
			{
				SAFE_DELETE(TranslatedText.psUtf8Uncompressed);
			}
			else
			{
				SAFE_DELETE_ARRAY(TranslatedText.szCompressed);
			}
		};

		string GetTranslatedText(const SLanguage* pLanguage) const;

		void   GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(this, sizeof(*this));

			pSizer->AddObject(sCharacterName);

			if ((flags & IS_COMPRESSED) == 0 && TranslatedText.psUtf8Uncompressed != NULL) //Number of bytes stored for compressed text is unknown, which throws this GetMemoryUsage off
			{
				pSizer->AddObject(*TranslatedText.psUtf8Uncompressed);
			}

			pSizer->AddObject(sPrototypeSoundEvent);

			pSizer->AddObject(SoundMoods);
			pSizer->AddObject(EventParameters);

			if (pEditorExtension != NULL)
			{
				pEditorExtension->GetMemoryUsage(pSizer);
			}
		}
	};

	//Keys as CRC32. Strings previously, but these proved too large
	typedef VectorMap<u32, SLocalizedStringEntry*> StringsKeyMap;

	struct SLanguage
	{
		typedef std::vector<SLocalizedStringEntry*> TLocalizedStringEntries;
		typedef std::vector<HuffmanCoder*>          THuffmanCoders;

		string                  sLanguage;
		StringsKeyMap           m_keysMap;
		TLocalizedStringEntries m_vLocalizedStrings;
		THuffmanCoders          m_vEncoders;

		void                    GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(this, sizeof(*this));
			pSizer->AddObject(sLanguage);
			pSizer->AddObject(m_vLocalizedStrings);
			pSizer->AddObject(m_keysMap);
			pSizer->AddObject(m_vEncoders);
		}
	};

	struct SFileInfo
	{
		bool  bDataStripping;
		u8 nTagID;
	};

#ifndef _RELEASE
	std::map<string, bool> m_warnedAboutLabels;
	bool                   m_haveWarnedAboutAtLeastOneLabel;

	void        LocalizedStringsUprWarning(tukk label, tukk message);
	void        ListAndClearProblemLabels();
#else
	inline void LocalizedStringsUprWarning(...) {};
	inline void ListAndClearProblemLabels()         {};
#endif

	void AddLocalizedString(SLanguage* pLanguage, SLocalizedStringEntry* pEntry, u32k keyCRC32);
	void AddControl(i32 nKey);
	//////////////////////////////////////////////////////////////////////////
	void ParseFirstLine(IXmlTableReader* pXmlTableReader, tuk nCellIndexToType, std::map<i32, string>& SoundMoodIndex, std::map<i32, string>& EventParameterIndex);
	void InternalSetCurrentLanguage(SLanguage* pLanguage);
	ISystem*   m_pSystem;
	// Pointer to the current language.
	SLanguage* m_pLanguage;

	typedef CListenerSet<ILocalizationPostProcessor*> PostProcessors;
	PostProcessors m_postProcessors;

	// all loaded Localization Files
	typedef std::pair<string, SFileInfo> pairFileName;
	typedef std::map<string, SFileInfo>  tmapFilenames;
	tmapFilenames m_loadedTables;

	// filenames per tag
	typedef std::vector<string> TStringVec;
	struct STag
	{
		TStringVec filenames;
		u8      id;
		bool       loaded;
	};
	typedef std::map<string, STag> TTagFileNames;
	TTagFileNames m_tagFileNames;
	TStringVec    m_tagLoadRequests;

	// Array of loaded languages.
	std::vector<SLanguage*> m_languages;

	typedef std::set<string> PrototypeSoundEvents;
	PrototypeSoundEvents m_prototypeEvents;  // this set is purely used for clever string/string assigning to save memory

	struct less_strcmp : public std::binary_function<const string&, const string&, bool>
	{
		bool operator()(const string& left, const string& right) const
		{
			return strcmp(left.c_str(), right.c_str()) < 0;
		}
	};

	typedef std::set<string, less_strcmp> CharacterNameSet;
	CharacterNameSet m_characterNameSet; // this set is purely used for clever string/string assigning to save memory

	// CVARs
	i32 m_cvarLocalizationDebug;
	i32 m_cvarLocalizationEncode; //Encode/Compress translated text to save memory

	//The localizations that are available for this SKU. Used for determining what to show on a language select screen or whether to show one at all
	TLocalizationBitfield m_availableLocalizations;

	//Lock for
	mutable DrxCriticalSection m_cs;
	typedef DrxAutoCriticalSection AutoLock;
};

#endif // __LocalizedStringUpr_h__
