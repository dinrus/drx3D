// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Upr for playlists

-------------------------------------------------------------------------
История:
- 06:03:2010 : Created by Tom Houghton

*************************************************************************/

#ifndef ___PLAYLISTMANAGER_H___
#define ___PLAYLISTMANAGER_H___

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Act/ILevelSystem.h>

//------------------------------------------------------------------------
#define GAME_VARIANT_NAME_MAX_LENGTH	32
#define USE_DEDICATED_LEVELROTATION 1
#define PLAYLIST_MANAGER_CUSTOM_NAME "Custom"

//------------------------------------------------------------------------
struct SSupportedVariantInfo
{
	SSupportedVariantInfo(tukk pName) : m_name(pName), m_id(0) {}
	SSupportedVariantInfo() : m_name("Unknown"), m_id(0) {}
	SSupportedVariantInfo(tukk pName, i32 id) : m_name(pName), m_id(id) {}

	typedef DrxFixedStringT<GAME_VARIANT_NAME_MAX_LENGTH> TFixedString;

	TFixedString	m_name;
	i32						m_id;
};

//------------------------------------------------------------------------
struct SPlaylistRotationExtension
{
	typedef std::vector<SSupportedVariantInfo> TVariantsVec;
	typedef DrxFixedStringT<32>  TLocaliNameStr;
	typedef DrxFixedStringT<64>  TLocaliDescStr;

	string  uniqueName;
	TLocaliNameStr localName;			// if @ string key for localized name, or just already localized name
	TLocaliDescStr localDescription;			// if @ string key for localized description, or just already localized description
	DrxFixedStringT<64> imagePath;
	TVariantsVec m_supportedVariants;
	i32		m_maxPlayers;
	bool  m_enabled;
	bool  m_requiresUnlocking;
	bool  m_hidden;
	bool  m_resolvedVariants;
	bool  m_allowDefaultVariant;
	bool	m_newC3Playlist;

	void Reset()
	{
		uniqueName.resize(0);
		localName.clear();
		localDescription.clear();
		m_supportedVariants.clear();
		m_maxPlayers = MAX_PLAYER_LIMIT;
		m_enabled = false;
		m_requiresUnlocking = false;
		m_hidden = false;
		m_resolvedVariants = false;
		m_allowDefaultVariant = true;
		m_newC3Playlist = false;
	}

	// Suppress passedByValue for smart pointers like XmlNodeRef
	// cppcheck-suppress passedByValue
	bool LoadFromXmlNode(const XmlNodeRef xmlNode);
};

//------------------------------------------------------------------------
struct SPlaylist
{
	void Reset()
	{
		rotExtInfo.Reset();
		id = 0;
		bSynchedFromServer = false;
#if USE_DEDICATED_LEVELROTATION
		m_bIsCustomPlaylist = false;
#endif
	}
	ILINE tukk GetName() const { return rotExtInfo.localName.c_str(); }
	ILINE tukk GetDescription() const { return rotExtInfo.localDescription.c_str(); }
	ILINE tukk GetUniqueName() const { return rotExtInfo.uniqueName.c_str(); }
	ILINE bool  IsEnabled() const { return rotExtInfo.m_enabled; }
	ILINE bool  IsHidden() const { return rotExtInfo.m_hidden; }
	ILINE void  SetEnabled(bool e) { rotExtInfo.m_enabled = e; }
	ILINE bool	RequiresUnlocking() const { return rotExtInfo.m_requiresUnlocking; }
	ILINE bool  IsSelectable() const { return (!bSynchedFromServer); }
	ILINE const SPlaylistRotationExtension::TVariantsVec &GetSupportedVariants() const
	{
		DRX_ASSERT(rotExtInfo.m_resolvedVariants);
		return rotExtInfo.m_supportedVariants;
	}
	void ResolveVariants();

	SPlaylistRotationExtension  rotExtInfo;
	ILevelRotation::TExtInfoId  id;
	bool												bSynchedFromServer;
#if USE_DEDICATED_LEVELROTATION
	bool												m_bIsCustomPlaylist;
#endif
};

//------------------------------------------------------------------------
struct SGameVariant
{
	SGameVariant() : m_id(0), m_restrictRank(0), m_requireRank(0), m_allowInCustomGames(false), m_enabled(false), m_requiresUnlock(false), m_supportsAllModes(true), m_allowSquads(true), m_allowClans(true) {}

	typedef DrxFixedStringT<8> TFixedString8;
	typedef DrxFixedStringT<GAME_VARIANT_NAME_MAX_LENGTH> TFixedString;
	typedef DrxFixedStringT<64> TLongFixedString;

	typedef DrxFixedArray<TLongFixedString, 32> TLongStringArray;
	typedef DrxFixedArray<i32, 16> TIntArray;

	TFixedString			m_name;
	// For m_local* string params : If the first character is @ it'll be localized in flash, otherwise it should have been localized already.
	TFixedString			m_localName;
	TLongFixedString		m_localDescription;
	TLongFixedString		m_localDescriptionUpper;
	TFixedString8				m_suffix;

	TLongStringArray	m_options;
	TIntArray					m_supportedModes;			// Only populated if m_supportsAllModes is not set, 
																					// contains values from EGameMode enum
	i32								m_id;
	i32								m_restrictRank;
	i32								m_requireRank;
	bool							m_allowInCustomGames;
	bool							m_enabled;
	bool							m_requiresUnlock;
	bool							m_supportsAllModes;
	bool							m_allowSquads;
	bool							m_allowClans;
};

//------------------------------------------------------------------------
struct SGameModeOption
{
public:
	typedef DrxFixedStringT<32> TFixedString;

	SGameModeOption() : m_pCVar(NULL), m_netMultiplyer(1.f), m_profileMultiplyer(1.f), m_fDefault(0.f), m_iDefault(0), m_gameModeSpecific(false), m_floatPrecision(0) {}

	void SetInt(ICVar *pCvar, tukk pProfileOption, bool gameModeSpecific, i32 iDefault);
	void SetFloat(ICVar *pCvar, tukk pProfileOption, bool gameModeSpecific, float netMultiplyer, float profileMultiplyer, float fDefault, i32 floatPrecision);
	void SetString(ICVar *pCvar, tukk pProfileOption, bool gameModeSpecific);
	void CopyToCVar(CProfileOptions *pProfileOptions, bool useDefault, tukk pGameModeName);
	void CopyToProfile(CProfileOptions *pProfileOptions, tukk pGameModeName);
	bool IsGameModeSpecific() { return m_gameModeSpecific; }

private:
	void SetCommon(ICVar *pCvar, tukk pProfileOption, bool gameModeSpecific, float netMultiplyer, float profileMultiplyer);

public:
	TFixedString m_profileOption;
	TFixedString m_strValue;
	ICVar *m_pCVar;
	float m_netMultiplyer;			// Value to multiply the cvar value to send across the network.
	float m_profileMultiplyer;	// Value to multiply the cvar value to get the profile one.
	float m_fDefault;
	i32 m_iDefault;
	i32 m_floatPrecision;
	bool m_gameModeSpecific;
};

//------------------------------------------------------------------------
struct SConfigVar
{
	ICVar *m_pCVar;
	bool m_bNetSynched;
};

//------------------------------------------------------------------------
struct SOptionRestriction
{
	enum EOperationType
	{
		eOT_Equal,
		eOT_NotEqual,
		eOT_LessThan,
		eOT_GreaterThan,
	};

	struct SOperand
	{
		struct SValue
		{
			DrxFixedStringT<16> m_string;
			float m_float;
			i32 m_int;
		};

		ICVar *m_pVar;
		SValue m_comparisionValue;
		SValue m_fallbackValue;
		EOperationType m_type;
	};

	SOperand m_operand1;
	SOperand m_operand2;
};

//------------------------------------------------------------------------
class CPlaylistUpr :	public IConsoleVarSink
{
private:
	typedef std::vector<SPlaylist>					TPlaylists;
	typedef std::vector<SGameModeOption>		TOptionsVec;
	typedef std::vector<SConfigVar>					TConfigVarsVec;
	typedef std::vector<SOptionRestriction> TOptionRestrictionsVec;

public:
	typedef std::vector<SGameVariant>	TVariantsVec;
	typedef std::vector<SGameVariant*>	TVariantsPtrVec;

	// IConsoleVarSink
	virtual bool OnBeforeVarChange( ICVar *pVar,tukk sNewValue ) { return true; }
	virtual void OnAfterVarChange( ICVar *pVar );
	// ~IConsoleVarSink

	CPlaylistUpr();
	~CPlaylistUpr();

	void  Init(tukk pPath);

	void  AddPlaylistsFromPath(tukk pPath);
	void  AddPlaylistFromXmlNode(XmlNodeRef xmlNode);

	void  AddVariantsFromPath(tukk pPath);

	ILevelRotation*  GetLevelRotation();

	bool  DisablePlaylist(tukk uniqueName);

	ILINE bool  HavePlaylistSet() const { return (m_currentPlaylistId != 0); } 

	bool ChoosePlaylist(i32k chooseIdx);
	bool ChoosePlaylistById(const ILevelRotation::TExtInfoId playlistId);
	bool ChooseVariant(i32k selectIdx);

	bool HaveUnlockedPlaylist(const SPlaylist *pPlaylist, DrxFixedStringT<128>* pUnlockString=NULL);
	bool HaveUnlockedPlaylistVariant(const SPlaylist *pPlaylist, i32k variantId, DrxFixedStringT<128>* pUnlockString=NULL);

	tukk GetActiveVariant();
	i32k GetActiveVariantIndex() const;
	ILINE bool IsUsingCustomVariant() { return GetActiveVariantIndex() == m_customVariantIndex; }
	tukk GetVariantName(i32k index);
	i32 GetVariantIndex(tukk pName);
	i32k GetNumVariants() const;
	i32k GetNumPlaylists() const;

	const SPlaylist* GetPlaylist(i32k index);
	const SPlaylist* GetCurrentPlaylist();
	const SPlaylist* GetPlaylistByUniqueName(tukk pInUniqueName)			{ return FindPlaylistByUniqueName(pInUniqueName); }
	tukk 			 GetPlaylistNameById(ILevelRotation::TExtInfoId findId)			{ return FindPlaylistById(findId)->GetName(); }
	i32              GetPlaylistIndex( ILevelRotation::TExtInfoId findId ) const;

	void ClearCurrentPlaylist() { SetCurrentPlaylist(0); }

	ILevelRotation::TExtInfoId CreateCustomPlaylist(tukk pName);

	void SetModeOptions();

	const TVariantsVec &GetVariants() const { return m_variants; }
	void GetVariantsForGameMode(tukk pGameMode, TVariantsPtrVec &result);

	ILINE i32 GetDefaultVariant() const { return m_defaultVariantIndex; }
	ILINE i32 GetCustomVariant() const { return m_customVariantIndex; }
	ILINE const SGameVariant *GetVariant(i32 index) const
	{
		if ((index >= 0) && (index < (i32)m_variants.size()))
		{
			return &m_variants[index];
		}
		else
		{
			return NULL;
		}
	}

	void GetTotalPlaylistCounts(u32 *outUnlocked, u32 *outTotal);

	void SetGameModeOption(tukk pOption, tukk pValue);
	void GetGameModeOption(tukk pOption, DrxFixedStringT<32> &result);
	u32 GetGameModeOptionCount()
	{
		return (u32)m_options.size();
	}
	void GetGameModeProfileOptionName(u32k index, DrxFixedStringT<32> &result);

	u16 PackCustomVariantOption(u32 index);
	i32 UnpackCustomVariantOption(u16 value, u32 index, i32* pIntValue, float* pFloatValue);
	i32 UnpackCustomVariantOptionProfileValues(u16 value, u32 index, i32* pIntValue, float* pFloatValue, i32* pFloatPrecision);
	void ReadDetailedServerInfo(u16 *pOptions, u32 numOptions);

	void WriteSetCustomVariantOptions(CDrxLobbyPacket* pPacket, CPlaylistUpr::TOptionsVec pOptions, u32 numOptions);
	void WriteSetVariantPacket(CDrxLobbyPacket *pPacket);

	void ReadSetCustomVariantOptions(CDrxLobbyPacket* pPacket, CPlaylistUpr::TOptionsVec pOptions, u32 numOptions);
	void ReadSetVariantPacket(CDrxLobbyPacket *pPacket);

	bool AdvanceRotationUntil(i32k newNextIdx);

	void OnPromoteToServer();
	
	template<class T> static void GetLocalisationSpecificAttr(const XmlNodeRef& rNode, tukk pAttrKey, tukk pLang, T* pOutVal);

#if USE_DEDICATED_LEVELROTATION
	bool IsUsingCustomRotation();
#endif

	void IgnoreCVarChanges(bool enabled) { m_bIsSettingOptions = enabled; }

private:
	void  ClearAllVariantCVars();
	void  SaveCurrentSettingsToProfile();

	void  SetCurrentPlaylist(const ILevelRotation::TExtInfoId id);
	void  SetActiveVariant(i32 variantIndex);

	ILevelRotation*  GetRotationForCurrentPlaylist();

	SPlaylist*  FindPlaylistById(ILevelRotation::TExtInfoId findId);
	SPlaylist*  FindPlaylistByUniqueName(tukk uniqueName);

	void	LoadVariantsFile(tukk pPath);
	void  AddVariantFromXmlNode(XmlNodeRef xmlNode);
	bool  DisableVariant(tukk pName);
	SGameVariant *FindVariantByName(tukk pName);

	void AddGameModeOptionInt(tukk pCVarName, tukk pProfileOption, bool gameModeSpecific, i32 iDefault);
	void AddGameModeOptionFloat(tukk pCVarName, tukk pProfileOption, bool gameModeSpecific, float netMultiplyer, float profileMultiplyer, float fDefault, i32 floatPrecision);
	void AddGameModeOptionString(tukk pCVarName, tukk pProfileOption, bool gameModeSpecific);
	SGameModeOption *GetGameModeOptionStruct(tukk pOptionName);

	void AddConfigVar(tukk pCVarName, bool bNetSynched);

	ILevelRotation::TExtInfoId GetPlaylistId(tukk pUniqueName) const;

	void  Deinit();

#if USE_DEDICATED_LEVELROTATION
	void LoadLevelRotation();
	void ReadServerInfo(XmlNodeRef xmlNode, i32 &outVariantIndex, i32 &outMaxPlayers);
	void SetOptionsFromXml(XmlNodeRef xmlNode, SGameVariant *pVariant);
#endif

#if !defined(_RELEASE)
	void DbgPlaylistsList();
	void DbgPlaylistsChoose(i32k chooseIdx);
	void DbgPlaylistsUnchoose();
	void DbgPlaylistsShowVariants();
	void DbgPlaylistsSelectVariant(i32k selectIdx);

	static void CmdPlaylistsList(IConsoleCmdArgs* pCmdArgs);
	static void CmdPlaylistsChoose(IConsoleCmdArgs* pCmdArgs);
	static void CmdPlaylistsUnchoose(IConsoleCmdArgs* pCmdArgs);
	static void CmdPlaylistsShowVariants(IConsoleCmdArgs* pCmdArgs);
	static void CmdPlaylistsSelectVariant(IConsoleCmdArgs* pCmdArgs);
#endif

	static void CmdStartPlaylist(IConsoleCmdArgs* pCmdArgs);
	static bool DoStartPlaylistCommand(tukk pPlaylistArg);

private:
	i32 GetSynchedVarsSize();
	void WriteSynchedVars(CDrxLobbyPacket* pPacket);
	void ReadSynchedVars(CDrxLobbyPacket* pPacket);

	void LoadOptionRestrictions();
	bool LoadOperand(XmlNodeRef operandXml, SOptionRestriction::SOperand &outResult);
	void CheckRestrictions(ICVar *pChangedVar);
	void ApplyRestriction(SOptionRestriction::SOperand &operand1, SOptionRestriction::SOperand &operand2);
	bool CheckOperation(SOptionRestriction::SOperand &operand, bool bEnsureFalse);

	static void OnCustomOptionCVarChanged(ICVar *pCVar);

	TPlaylists  m_playlists;
	TVariantsVec m_variants;
	TOptionsVec m_options;
	TConfigVarsVec m_configVars;
	TOptionRestrictionsVec m_optionRestrictions;

	ILevelRotation::TExtInfoId  m_currentPlaylistId;
	i32 m_activeVariantIndex;
	i32 m_defaultVariantIndex;
	i32 m_customVariantIndex;

	bool m_inited;
	bool m_bIsSettingOptions;
};


#endif  // ___PLAYLISTMANAGER_H___
