// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PLAYERPROFILEMANAGER_H__
#define __PLAYERPROFILEMANAGER_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Act/IPlayerProfiles.h>
#include <drx3D/CoreX/DrxCrc32.h>

class CPlayerProfile;
struct ISaveGame;
struct ILoadGame;

#if defined(_DEBUG)
	#define PROFILE_DEBUG_COMMANDS 1
#else
	#define PROFILE_DEBUG_COMMANDS 0
#endif

class CPlayerProfileUpr : public IPlayerProfileUpr
{
public:
	struct IPlatformImpl;

	// profile description
	struct SLocalProfileInfo
	{
		SLocalProfileInfo()
			: m_lastLoginTime(0)
		{
			SetName(PLAYER_DEFAULT_PROFILE_NAME);
		}
		SLocalProfileInfo(const string& name)
			: m_lastLoginTime(0)
		{
			SetName(name);
		}
		SLocalProfileInfo(tukk name)
			: m_lastLoginTime(0)
		{
			SetName(name);
		}

		void SetName(tukk name)
		{
			m_name = name;
		}

		void SetName(const string& name)
		{
			m_name = name;
		}
		void SetLastLoginTime(time_t lastLoginTime)
		{
			m_lastLoginTime = lastLoginTime;
		}

		const string& GetName() const
		{
			return m_name;
		}

		const time_t& GetLastLoginTime() const
		{
			return m_lastLoginTime;
		}

		i32 compare(tukk name) const
		{
			return m_name.compareNoCase(name);
		}

	private:
		string m_name; // name of the profile
		time_t m_lastLoginTime;
	};

	// per user data
	struct SUserEntry
	{
		SUserEntry(const string& inUserId) : userId(inUserId), pCurrentProfile(0), pCurrentPreviewProfile(0), userIndex(0), unknownUser(false) {}
		string                         userId;
		CPlayerProfile*                pCurrentProfile;
		CPlayerProfile*                pCurrentPreviewProfile;
		u32                   userIndex;
		bool                           unknownUser;
		std::vector<SLocalProfileInfo> profileDesc;
	};
	typedef std::vector<SUserEntry*> TUserVec;

	struct SSaveGameMetaData
	{
		SSaveGameMetaData()
		{
			levelName = gameRules = buildVersion = SAVEGAME_LEVEL_NAME_UNDEFINED;
			saveTime = 0;
			loadTime = 0;
			fileVersion = -1;
		}
		void CopyTo(ISaveGameEnumerator::SGameMetaData& data)
		{
			data.levelName = levelName;
			data.gameRules = gameRules;
			data.fileVersion = fileVersion;
			data.buildVersion = buildVersion;
			data.saveTime = saveTime;
			data.loadTime = loadTime;
			data.xmlMetaDataNode = xmlMetaDataNode;
		}
		string     levelName;
		string     gameRules;
		i32        fileVersion;
		string     buildVersion;
		time_t     saveTime; // original time of save
		time_t     loadTime; // most recent time loaded, from file modified timestamp
		XmlNodeRef xmlMetaDataNode;
	};

	struct SThumbnail
	{
		SThumbnail() : width(0), height(0), depth(0) {}
		DynArray<u8> data;
		i32             width;
		i32             height;
		i32             depth;
		bool IsValid() const { return data.size() && width && height && depth; }
		void ReleaseData()
		{
			data.clear();
			width = height = depth = 0;
		}
	};

	struct SSaveGameInfo
	{
		string            name;
		string            humanName;
		string            description;
		SSaveGameMetaData metaData;
		SThumbnail        thumbnail;
		void              CopyTo(ISaveGameEnumerator::SGameDescription& desc)
		{
			desc.name = name;
			desc.humanName = humanName;
			desc.description = description;
			metaData.CopyTo(desc.metaData);
		}
	};
	typedef std::vector<SSaveGameInfo> TSaveGameInfoVec;

	enum EPlayerProfileSection
	{
		ePPS_Attribute = 0,
		ePPS_Actionmap,
		ePPS_Num  // Last
	};

	// members

	CPlayerProfileUpr(CPlayerProfileUpr::IPlatformImpl* pImpl); // CPlayerProfileUpr takes ownership of IPlatformImpl*
	virtual ~CPlayerProfileUpr();

	// IPlayerProfileUpr
	virtual bool                  Initialize();
	virtual bool                  Shutdown();
	virtual i32                   GetUserCount();
	virtual bool                  GetUserInfo(i32 index, IPlayerProfileUpr::SUserInfo& outInfo);
	virtual tukk           GetCurrentUser();
	virtual i32                   GetCurrentUserIndex();
	virtual void                  SetExclusiveControllerDeviceIndex(u32 exclusiveDeviceIndex);
	virtual u32          GetExclusiveControllerDeviceIndex() const;
	virtual bool                  LoginUser(tukk userId, bool& bOutFirstTime);
	virtual bool                  LogoutUser(tukk userId);
	virtual i32                   GetProfileCount(tukk userId);
	virtual bool                  GetProfileInfo(tukk userId, i32 index, IPlayerProfileUpr::SProfileDescription& outInfo);
	virtual bool                  CreateProfile(tukk userId, tukk profileName, bool bOverrideIfPresent, IPlayerProfileUpr::EProfileOperationResult& result);
	virtual bool                  DeleteProfile(tukk userId, tukk profileName, IPlayerProfileUpr::EProfileOperationResult& result);
	virtual bool                  RenameProfile(tukk userId, tukk newName, IPlayerProfileUpr::EProfileOperationResult& result);
	virtual bool                  SaveProfile(tukk userId, IPlayerProfileUpr::EProfileOperationResult& result, u32 reason);
	virtual bool                  SaveInactiveProfile(tukk userId, tukk profileName, EProfileOperationResult& result, u32 reason);
	virtual bool                  IsLoadingProfile() const;
	virtual bool                  IsSavingProfile() const;
	virtual IPlayerProfile*       ActivateProfile(tukk userId, tukk profileName);
	virtual IPlayerProfile*       GetCurrentProfile(tukk userId);
	virtual bool                  ResetProfile(tukk userId);
	virtual void                  ReloadProfile(IPlayerProfile* pProfile, u32 reason);
	virtual IPlayerProfile*       GetDefaultProfile();
	virtual const IPlayerProfile* PreviewProfile(tukk userId, tukk profileName);
	virtual void                  SetSharedSaveGameFolder(tukk sharedSaveGameFolder);
	virtual tukk           GetSharedSaveGameFolder()
	{
		return m_sharedSaveGameFolder.c_str();
	}
	virtual void   GetMemoryStatistics(IDrxSizer* s);

	virtual void   AddListener(IPlayerProfileListener* pListener, bool updateNow);
	virtual void   RemoveListener(IPlayerProfileListener* pListener);
	virtual void   AddOnlineAttributesListener(IOnlineAttributesListener* pListener);
	virtual void   RemoveOnlineAttributesListener(IOnlineAttributesListener* pListener);
	virtual void   EnableOnlineAttributes(bool enable);
	virtual bool   HasEnabledOnlineAttributes() const;
	virtual bool   CanProcessOnlineAttributes() const;
	virtual void   SetCanProcessOnlineAttributes(bool enable);
	virtual bool   RegisterOnlineAttributes();
	virtual void   GetOnlineAttributesState(const IOnlineAttributesListener::EEvent event, IOnlineAttributesListener::EState& state) const;
	virtual bool   IsOnlineOnlyAttribute(tukk name);
	virtual void   LoadOnlineAttributes(IPlayerProfile* pProfile);
	virtual void   SaveOnlineAttributes(IPlayerProfile* pProfile);
	virtual i32    GetOnlineAttributesVersion() const;
	virtual i32    GetOnlineAttributeIndexByName(tukk name);
	virtual void   GetOnlineAttributesDataFormat(SDrxLobbyUserData* pData, u32 numData);
	virtual u32 GetOnlineAttributeCount();
	virtual void   ClearOnlineAttributes();
	virtual void   SetProfileLastLoginTime(tukk userId, i32 index, time_t lastLoginTime);
	// ~IPlayerProfileUpr

	void LoadGamerProfileDefaults(IPlayerProfile* pProfile);

	// maybe move to IPlayerProfileUpr i/f
	virtual ISaveGameEnumeratorPtr CreateSaveGameEnumerator(tukk userId, CPlayerProfile* pProfile);
	virtual ISaveGame*             CreateSaveGame(tukk userId, CPlayerProfile* pProfile);
	virtual ILoadGame*             CreateLoadGame(tukk userId, CPlayerProfile* pProfile);
	virtual bool                   DeleteSaveGame(tukk userId, CPlayerProfile* pProfile, tukk name);

	virtual ILevelRotationFile*    GetLevelRotationFile(tukk userId, CPlayerProfile* pProfile, tukk name);

	virtual bool                   ResolveAttributeBlock(tukk userId, tukk attrBlockName, IResolveAttributeBlockListener* pListener, i32 reason);
	virtual bool                   ResolveAttributeBlock(tukk userId, const SResolveAttributeRequest& attrBlockNameRequest, IResolveAttributeBlockListener* pListener, i32 reason);

	const string&                  GetSharedSaveGameFolder() const
	{
		return m_sharedSaveGameFolder;
	}

	bool IsSaveGameFolderShared() const
	{
#if DRX_PLATFORM_DESKTOP
		return m_sharedSaveGameFolder.empty() == false;
#else
		// TCR/TRC - don't use profile name in file names.
		// Also, console saves are already keyed to a user profile.
		return false;
#endif
	}

	ILINE CPlayerProfile* GetDefaultCPlayerProfile() const
	{
		return m_pDefaultProfile;
	}

	// helper to move file or directory -> should eventually go into DinrusXSys/DrxPak
	// only implemented for Windows
	bool           MoveFileHelper(tukk existingFileName, tukk newFileName);

	virtual void   SetOnlineAttributes(IPlayerProfile* pProfile, const SDrxLobbyUserData* pData, i32k onlineDataCount);
	virtual u32 GetOnlineAttributes(SDrxLobbyUserData* pData, u32 numData);
	virtual u32 GetDefaultOnlineAttributes(SDrxLobbyUserData* pData, u32 numData);

	void           ApplyChecksums(SDrxLobbyUserData* pData, u32 numData);
	bool           ValidChecksums(const SDrxLobbyUserData* pData, u32 numData);

public:
	struct IProfileXMLSerializer
	{
		virtual ~IProfileXMLSerializer(){}
		virtual bool       IsLoading() = 0;

		virtual void       SetSection(CPlayerProfileUpr::EPlayerProfileSection section, XmlNodeRef& node) = 0;
		virtual XmlNodeRef CreateNewSection(CPlayerProfileUpr::EPlayerProfileSection section, tukk name) = 0;
		virtual XmlNodeRef GetSection(CPlayerProfileUpr::EPlayerProfileSection section) = 0;

		/*
		   virtual XmlNodeRef AddSection(tukk name) = 0;
		   virtual XmlNodeRef GetSection(tukk name) = 0;
		 */
	};

	struct IPlatformImpl
	{
		typedef CPlayerProfileUpr::SUserEntry               SUserEntry;
		typedef CPlayerProfileUpr::SLocalProfileInfo        SLocalProfileInfo;
		typedef CPlayerProfileUpr::SThumbnail               SThumbnail;
		typedef CPlayerProfileUpr::SResolveAttributeRequest SResolveAttributeRequest;
		virtual ~IPlatformImpl(){}
		virtual bool                Initialize(CPlayerProfileUpr* pMgr) = 0;
		virtual void                Release() = 0; // must delete itself
		virtual bool                LoginUser(SUserEntry* pEntry) = 0;
		virtual bool                LogoutUser(SUserEntry* pEntry) = 0;
		virtual bool                SaveProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name, bool initialSave = false, i32 reason = ePR_All) = 0;
		virtual bool                LoadProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name) = 0;
		virtual bool                DeleteProfile(SUserEntry* pEntry, tukk name) = 0;
		virtual bool                RenameProfile(SUserEntry* pEntry, tukk newName) = 0;
		virtual bool                GetSaveGames(SUserEntry* pEntry, TSaveGameInfoVec& outVec, tukk altProfileName = "") = 0; // if altProfileName == "", uses current profile of SUserEntry
		virtual ISaveGame*          CreateSaveGame(SUserEntry* pEntry) = 0;
		virtual ILoadGame*          CreateLoadGame(SUserEntry* pEntry) = 0;
		virtual bool                DeleteSaveGame(SUserEntry* pEntry, tukk name) = 0;
		virtual ILevelRotationFile* GetLevelRotationFile(SUserEntry* pEntry, tukk name) = 0;
		virtual bool                GetSaveGameThumbnail(SUserEntry* pEntry, tukk saveGameName, SThumbnail& thumbnail) = 0;
		virtual void                GetMemoryStatistics(IDrxSizer* s) = 0;
		virtual bool                ResolveAttributeBlock(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk attrBlockName, IResolveAttributeBlockListener* pListener, i32 reason)                            { return false; }
		virtual bool                ResolveAttributeBlock(SUserEntry* pEntry, CPlayerProfile* pProfile, const SResolveAttributeRequest& attrBlockNameRequest, IResolveAttributeBlockListener* pListener, i32 reason) { return false; }

	};

	SUserEntry* FindEntry(tukk userId) const;

protected:

	SUserEntry*        FindEntry(tukk userId, TUserVec::iterator& outIter);

	SLocalProfileInfo* GetLocalProfileInfo(SUserEntry* pEntry, tukk profileName) const;
	SLocalProfileInfo* GetLocalProfileInfo(SUserEntry* pEntry, tukk profileName, std::vector<SLocalProfileInfo>::iterator& outIter) const;

public:
	static i32         sUseRichSaveGames;
	static i32         sRSFDebugWrite;
	static i32         sRSFDebugWriteOnLoad;
	static i32         sLoadOnlineAttributes;

	static tukk FACTORY_DEFAULT_NAME;
	static tukk PLAYER_DEFAULT_PROFILE_NAME;

protected:
	bool        SetUserData(SDrxLobbyUserData* data, const TFlowInputData& value);
	bool        ReadUserData(const SDrxLobbyUserData* data, TFlowInputData& val);
	u32      UserDataSize(const SDrxLobbyUserData* data);
	bool        SetUserDataType(SDrxLobbyUserData* data, tukk type);
	void        GetDefaultValue(tukk type, XmlNodeRef attributeNode, SDrxLobbyUserData* pOutData);

	bool        RegisterOnlineAttribute(tukk name, tukk type, const bool onlineOnly, const SDrxLobbyUserData& defaultValue, CCrc32& crc);
	void        ReadOnlineAttributes(IPlayerProfile* pProfile, const SDrxLobbyUserData* pData, u32k numData);
	static void ReadUserDataCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, SDrxLobbyUserData* pData, u32 numData, uk pArg);
	static void RegisterUserDataCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg);
	static void WriteUserDataCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg);

	void        SetOnlineAttributesState(const IOnlineAttributesListener::EEvent event, const IOnlineAttributesListener::EState newState);
	void        SendOnlineAttributeState(const IOnlineAttributesListener::EEvent event, const IOnlineAttributesListener::EState newState);

	//Online attributes use checksums to check the validity of data loaded
	i32  Checksum(i32k checksum, const SDrxLobbyUserData* pData, u32 numData);
	i32  ChecksumHash(i32k hash, i32k value) const;
	i32  ChecksumHash(i32k value0, i32k value1, i32k value2, i32k value3, i32k value4) const;
	i32  ChecksumConvertValueToInt(const SDrxLobbyUserData* pData);

	bool LoadProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name, bool bPreview = false);

	const static i32 k_maxOnlineDataCount = 1500;
	const static i32 k_maxOnlineDataBytes = 2976; //reduced from 3000 for encryption
	const static i32 k_onlineChecksums = 2;

#if PROFILE_DEBUG_COMMANDS
	static void DbgLoadOnlineAttributes(IConsoleCmdArgs* args);
	static void DbgSaveOnlineAttributes(IConsoleCmdArgs* args);
	static void DbgTestOnlineAttributes(IConsoleCmdArgs* args);

	static void DbgLoadProfile(IConsoleCmdArgs* args);
	static void DbgSaveProfile(IConsoleCmdArgs* args);

	SDrxLobbyUserData m_testData[k_maxOnlineDataCount];
	i32               m_testFlowData[k_maxOnlineDataCount];
	i32               m_testingPhase;
#endif

	typedef VectorMap<string, u32>            TOnlineAttributeMap;
	typedef std::vector<IPlayerProfileListener*> TListeners;

	SDrxLobbyUserData                 m_defaultOnlineData[k_maxOnlineDataCount];
	SDrxLobbyUserData                 m_onlineData[k_maxOnlineDataCount];
	bool                              m_onlineOnlyData[k_maxOnlineDataCount];
	TOnlineAttributeMap               m_onlineAttributeMap;
	TListeners                        m_listeners;
	TUserVec                          m_userVec;
	string                            m_curUserID;
	string                            m_sharedSaveGameFolder;
	i32                               m_curUserIndex;
	u32                      m_exclusiveControllerDeviceIndex;
	u32                            m_onlineDataCount;
	u32                            m_onlineDataByteCount;
	u32                            m_onlineAttributeAutoGeneratedVersion;
	IPlatformImpl*                    m_pImpl;
	CPlayerProfile*                   m_pDefaultProfile;
	IPlayerProfile*                   m_pReadingProfile;
	IOnlineAttributesListener::EState m_onlineAttributesState[IOnlineAttributesListener::eOAE_Max];
	IOnlineAttributesListener*        m_onlineAttributesListener;
	i32                               m_onlineAttributeDefinedVersion;
	DrxLobbyTaskID                    m_lobbyTaskId;
	bool                              m_registered;
	bool                              m_bInitialized;
	bool                              m_enableOnlineAttributes;
	bool                              m_allowedToProcessOnlineAttributes;
	bool                              m_loadingProfile;
	bool                              m_savingProfile;
};

#endif
