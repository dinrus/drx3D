// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Memory/DrxSizer.h>
//#include <platform.h> // Needed for LARGE_INTEGER (for consoles).

class XmlNodeRef;

// Localized strings manager interface.

//! \cond INTERNAL
//! Localization Info structure.
struct SLocalizedInfoGame
{

	SLocalizedInfoGame()
		: szCharacterName(NULL)
		, bUseSubtitle(false)
	{

	}

	tukk szCharacterName;
	string      sUtf8TranslatedText;

	bool        bUseSubtitle;
};

struct SLocalizedAdvancesSoundEntry
{
	string sName;
	float  fValue;
	void   GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(sName);
	}
};

//! Localization Sound Info structure, containing sound related parameters.
struct SLocalizedSoundInfoGame : public SLocalizedInfoGame
{
	SLocalizedSoundInfoGame()
		: sSoundEvent(NULL)
		, fVolume(0.f)
		, fRadioRatio(0.f)
		, bIsDirectRadio(false)
		, bIsIntercepted(false)
		, nNumSoundMoods(0)
		, pSoundMoods(NULL)
		, nNumEventParameters(0)
		, pEventParameters(NULL)
	{

	}

	tukk sSoundEvent;
	float       fVolume;
	float       fRadioRatio;
	bool        bIsDirectRadio;
	bool        bIsIntercepted;

	// SoundMoods.
	i32                           nNumSoundMoods;
	SLocalizedAdvancesSoundEntry* pSoundMoods;

	// EventParameters.
	i32                           nNumEventParameters;
	SLocalizedAdvancesSoundEntry* pEventParameters;
};

//! Localization Sound Info structure, containing sound related parameters.
struct SLocalizedInfoEditor : public SLocalizedInfoGame
{
	SLocalizedInfoEditor()
		: sKey(NULL)
		, sOriginalCharacterName(NULL)
		, sOriginalActorLine(NULL)
		, sUtf8TranslatedActorLine(NULL)
		, nRow(0)
	{

	}

	tukk  sKey;
	tukk  sOriginalCharacterName;
	tukk  sOriginalActorLine;
	tukk  sUtf8TranslatedActorLine;
	u32 nRow;
};

struct ILocalizationPostProcessor
{
	virtual ~ILocalizationPostProcessor() {}
	virtual void PostProcessString(string& inout) const = 0;
};
//! \endcond

//! Interface to the Localization Upr.
struct ILocalizationUpr
{
	//! Platform independent language IDs. Used to map the platform specific language codes to localization pakfiles.
	//! Please ensure that each entry in this enum has a corresponding entry in the PLATFORM_INDEPENDENT_LANGUAGE_NAMES array which is defined in LocalizedStringUpr.cpp currently.
	enum EPlatformIndependentLanguageID
	{
		ePILID_Japanese,
		ePILID_English,
		ePILID_French,
		ePILID_Spanish,
		ePILID_German,
		ePILID_Italian,
		ePILID_Dutch,
		ePILID_Portuguese,
		ePILID_Russian,
		ePILID_Korean,
		ePILID_ChineseT,        //!< Traditional Chinese.
		ePILID_ChineseS,        //!< Simplified Chinese.
		ePILID_Finnish,
		ePILID_Swedish,
		ePILID_Danish,
		ePILID_Norwegian,
		ePILID_Polish,
		ePILID_Arabic,          //!< Test value for PS3. Not currently supported by Sony on the PS3 as a system language.
		ePILID_Czech,
		ePILID_Turkish,
		ePILID_MAX_OR_INVALID   //!< Not a language, denotes the maximum number of languages or an unknown language.
	};

	typedef u32 TLocalizationBitfield;

	// <interfuscator:shuffle>
	virtual ~ILocalizationUpr(){}
	virtual tukk                                 LangNameFromPILID(const ILocalizationUpr::EPlatformIndependentLanguageID id) = 0;
	virtual ILocalizationUpr::TLocalizationBitfield MaskSystemLanguagesFromSupportedLocalizations(const ILocalizationUpr::TLocalizationBitfield systemLanguages) = 0;
	virtual ILocalizationUpr::TLocalizationBitfield IsLanguageSupported(const ILocalizationUpr::EPlatformIndependentLanguageID id) = 0;
	virtual bool        SetLanguage(tukk sLanguage) = 0;
	virtual tukk GetLanguage() = 0;

	//! Load the descriptor file with tag information.
	virtual bool InitLocalizationData(tukk sFileName, bool bReload = false) = 0;

	//! Request to load loca data by tag. Actual loading will happen during next level load begin event.
	virtual bool RequestLoadLocalizationDataByTag(tukk sTag) = 0;

	//! Direct load of loca data by tag.
	virtual bool LoadLocalizationDataByTag(tukk sTag, bool bReload = false) = 0;
	virtual bool ReleaseLocalizationDataByTag(tukk sTag) = 0;

	virtual void RegisterPostProcessor(ILocalizationPostProcessor* pPostProcessor) = 0;
	virtual void UnregisterPostProcessor(ILocalizationPostProcessor* pPostProcessor) = 0;

	virtual bool LoadExcelXmlSpreadsheet(tukk sFileName, bool bReload = false) = 0;
	virtual void ReloadData() = 0;

	//! Free localization data.
	virtual void FreeData() = 0;

	//! Translate a string into the currently selected language.
	//! Processes the input string and translates all labels contained into the currently selected language.
	//! \param[in]  sString             String to be translated.
	//! \param[out] outLocalizedString  Translated version of the string.
	//! \param[in]  bEnglish            If true, translates the string into the always present English language.
	//! \return true if localization was successful, false otherwise.
	virtual bool LocalizeString(tukk sString, string& outLocalizedString, bool bEnglish = false) = 0;

	//! Same as LocalizeString( tukk sString, string& outLocalizedString, bool bEnglish=false ) but at the moment this is faster.
	virtual bool LocalizeString(const string& sString, string& outLocalizedString, bool bEnglish = false) = 0;

	//! Provides the localized version corresponding to a label.
	//! A label has to start with '@' sign.
	//! \param[in] sLabel             Label to be translated, must start with '@' sign.
	//! \param[out]outLocalizedString Localized version of the label.
	//! \param[in] bEnglish           If true, returns the always present English version of the label.
	//! \return true if localization was successful, false otherwise.
	virtual bool LocalizeLabel(tukk sLabel, string& outLocalizedString, bool bEnglish = false) = 0;

	//! Get localization info structure corresponding to a key (key=label without the '@' sign).
	//! \param[in] sKey         Key to be looked up. Key = Label without '@' sign.
	//! \param[out] outGameInfo Reference to localization info structure to be filled in.
	//! \return true if info for key was found, false otherwise.
	virtual bool GetLocalizedInfoByKey(tukk sKey, SLocalizedInfoGame& outGameInfo) = 0;

	//! Get the sound localization info structure corresponding to a key.
	//! \param[in] sKey          Key to be looked up. Key = Label without '@' sign.
	//! \param[out] outSoundInfo Reference to sound info structure to be filled in.
	//! outSoundInfo.pSoundMoods requires nNumSoundMoods-times allocated memory.
	//! On return outSoundInfo.nNumSoundMoods will hold how many SoundsMood entries are needed.
	//! On return outSoundInfo.nNumEventParameters will hold how many EventParameter entries are needed.
	//! outSoundInfo.pEventParameters requires nNumEventParameters-times allocated memory.
	//! Passing 0 in either the Num fields will make the query ignore checking for allocated memory.
	//! \return true if successful, false otherwise (key not found, or not enough memory provided to write additional info).
	virtual bool GetLocalizedInfoByKey(tukk sKey, SLocalizedSoundInfoGame* pOutSoundInfo) = 0;

	//! Return number of localization entries.
	virtual i32 GetLocalizedStringCount() = 0;

	//! Get the localization info structure at index nIndex.
	//! \param[in] nIndex Index.
	//! \param[out] outEditorInfo Reference to localization info structure to be filled in.
	//! \return true if successful, false otherwise (out of bounds).
	virtual bool GetLocalizedInfoByIndex(i32 nIndex, SLocalizedInfoEditor& outEditorInfo) = 0;

	//! Get the localization info structure at index nIndex.
	//! \param[in] nIndex Index.
	//! \param[out] outGameInfo Reference to localization info structure to be filled in.
	//! \return true if successful, false otherwise (out of bounds).
	virtual bool GetLocalizedInfoByIndex(i32 nIndex, SLocalizedInfoGame& outGameInfo) = 0;

	//! Get the english localization info structure corresponding to a key.
	//! \param[in] sKey Key to be looked up. Key = Label without '@' sign.
	//! \param[out] sLocalizedString Corresponding english language string.
	//! \return true if successful, false otherwise (key not found).
	virtual bool GetEnglishString(tukk sKey, string& sLocalizedString) = 0;

	//! Get Subtitle for Key or Label.
	//! \param[in] sKeyOrLabel    Key or Label to be used for subtitle lookup. Key = Label without '@' sign.
	//! \param[out] outSubtitle   Subtitle (untouched if Key/Label not found).
	//! \param[in] bForceSubtitle If true, get subtitle (sLocalized or sEnglish) even if not specified in Data file.
	//! \return true if subtitle found (and outSubtitle filled in), false otherwise.
	virtual bool GetSubtitle(tukk sKeyOrLabel, string& outSubtitle, bool bForceSubtitle = false) = 0;

	//! These methods format outString depending on sString with ordered arguments.
	//! FormatStringMessage(outString, "This is %2 and this is %1", "second", "first");
	//! \param[out] outString This is first and this is second.
	virtual void FormatStringMessage(string& outString, const string& sString, tukk* sParams, i32 nParams) = 0;
	virtual void FormatStringMessage(string& outString, const string& sString, tukk param1, tukk param2 = 0, tukk param3 = 0, tukk param4 = 0) = 0;

	virtual void LocalizeTime(time_t t, bool bMakeLocalTime, bool bShowSeconds, string& outTimeString) = 0;
	virtual void LocalizeDate(time_t t, bool bMakeLocalTime, bool bShort, bool bIncludeWeekday, string& outDateString) = 0;
	virtual void LocalizeDuration(i32 seconds, string& outDurationString) = 0;
	virtual void LocalizeNumber(i32 number, string& outNumberString) = 0;
	virtual void LocalizeNumber(float number, i32 decimals, string& outNumberString) = 0;
	// </interfuscator:shuffle>

	static ILINE TLocalizationBitfield LocalizationBitfieldFromPILID(EPlatformIndependentLanguageID pilid)
	{
		assert(pilid >= 0 && pilid < ePILID_MAX_OR_INVALID);
		return (1 << pilid);
	}
};
