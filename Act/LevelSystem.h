// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __LEVELSYSTEM_H__
#define __LEVELSYSTEM_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Act/ILevelSystem.h>

#define LEVEL_ROTATION_DEBUG 0

#if LEVEL_ROTATION_DEBUG
	#define Log_LevelRotation(...) DrxLog("CLevelRotation" __VA_ARGS__)
#else
	#define Log_LevelRotation(...)
#endif

class CLevelInfo :
	public ILevelInfo
{
	friend class CLevelSystem;
public:
	CLevelInfo() : m_heightmapSize(0), m_bMetaDataRead(false), m_isModLevel(false), m_scanTag(ILevelSystem::TAG_UNKNOWN), m_levelTag(ILevelSystem::TAG_UNKNOWN)
	{
		SwapEndian(m_scanTag, eBigEndian);
		SwapEndian(m_levelTag, eBigEndian);
	};
	virtual ~CLevelInfo() {};

	// ILevelInfo
	virtual tukk                      GetName() const override                 { return m_levelName.c_str(); };
	virtual const bool                       IsOfType(tukk sType) const override;
	virtual tukk                      GetPath() const override                 { return m_levelPath.c_str(); };
	virtual tukk                      GetPaks() const override                 { return m_levelPaks.c_str(); };
	virtual bool                             GetIsModLevel() const override           { return m_isModLevel; }
	virtual u32k                     GetScanTag() const override              { return m_scanTag; }
	virtual u32k                     GetLevelTag() const override             { return m_levelTag; }
	virtual tukk                      GetDisplayName() const override;
	virtual tukk                      GetPreviewImagePath() const override     { return m_previewImagePath.c_str(); }
	virtual tukk                      GetBackgroundImagePath() const override  { return m_backgroundImagePath.c_str(); }
	virtual tukk                      GetMinimapImagePath() const override     { return m_minimapImagePath.c_str(); }
	virtual i32                              GetHeightmapSize() const override        { return m_heightmapSize; };
	virtual const bool                       MetadataLoaded() const override          { return m_bMetaDataRead; }

	virtual i32                              GetGameTypeCount() const override        { return m_gameTypes.size(); };
	virtual const ILevelInfo::TGameTypeInfo* GetGameType(i32 gameType) const override { return &m_gameTypes[gameType]; };
	virtual bool                             SupportsGameType(tukk gameTypeName) const override;
	virtual const ILevelInfo::TGameTypeInfo* GetDefaultGameType() const override;
	virtual bool                             HasGameRules() const override            { return !m_gamerules.empty(); }

	virtual const ILevelInfo::SMinimapInfo&  GetMinimapInfo() const override          { return m_minimapInfo; }

	virtual tukk                      GetDefaultGameRules() const override     { return m_gamerules.empty() ? nullptr : m_gamerules[0].c_str(); }
	virtual size_t                           GetGameRulesCount() const override       { return m_gamerules.size(); }
	virtual size_t                           GetGameRules(tukk* pszGameRules, size_t numGameRules) const override;
	virtual bool                             GetAttribute(tukk name, TFlowInputData& val) const override;
	// ~ILevelInfo

	void GetMemoryUsage(IDrxSizer*) const;

private:
	void ReadMetaData();
	bool ReadInfo();

	bool OpenLevelPak();
	void CloseLevelPak();

	string                                 m_levelName;
	string                                 m_levelPath;
	string                                 m_levelPaks;
	string                                 m_levelDisplayName;
	string                                 m_previewImagePath;
	string                                 m_backgroundImagePath;
	string                                 m_minimapImagePath;

	string                                 m_levelPakFullPath;
	string                                 m_levelMMPakFullPath;
	string                                 m_levelSvoPakFullPath;

	std::vector<string>                    m_gamerules;
	i32                                    m_heightmapSize;
	u32                                 m_scanTag;
	u32                                 m_levelTag;
	bool                                   m_bMetaDataRead;
	std::vector<ILevelInfo::TGameTypeInfo> m_gameTypes;
	bool                                   m_isModLevel;
	SMinimapInfo                           m_minimapInfo;
	typedef std::map<string, TFlowInputData, stl::less_stricmp<string>> TAttributeList;
	TAttributeList                         m_levelAttributes;

	DynArray<string>                       m_levelTypeList;
};

class CLevelRotation : public ILevelRotation
{
public:
	CLevelRotation();
	virtual ~CLevelRotation();

	typedef struct SLevelRotationEntry
	{
		string              levelName;
		std::vector<string> gameRulesNames;
		std::vector<i32>    gameRulesShuffle;
		i32                 currentModeIndex;

		tukk GameModeName() const; //will return next game mode/rules according to shuffle
	} SLevelRotationEntry;

	typedef std::vector<SLevelRotationEntry> TLevelRotationVector;

	// ILevelRotation
	virtual bool Load(ILevelRotationFile* file);
	virtual bool LoadFromXmlRootNode(const XmlNodeRef rootNode, tukk altRootTag);

	virtual void Reset();
	virtual i32  AddLevel(tukk level);
	virtual void AddGameMode(i32 level, tukk gameMode);

	virtual i32  AddLevel(tukk level, tukk gameMode);

	//call to set the playlist ready for a new session
	virtual void                Initialise(i32 nSeed);

	virtual bool                First();
	virtual bool                Advance();

	virtual bool                AdvanceAndLoopIfNeeded();

	virtual tukk         GetNextLevel() const;
	virtual tukk         GetNextGameRules() const;

	virtual tukk         GetLevel(u32 idx, bool accessShuffled = true) const;
	virtual i32                 GetNGameRulesForEntry(u32 idx, bool accessShuffled = true) const;
	virtual tukk         GetGameRules(u32 idx, u32 iMode, bool accessShuffled = true) const;

	virtual tukk         GetNextGameRulesForEntry(i32 idx) const; //always matches shuffling

	virtual i32k           NumAdvancesTaken() const;
	virtual void                ResetAdvancement();

	virtual i32                 GetLength() const;
	virtual i32                 GetTotalGameModeEntries() const;
	virtual i32                 GetNext() const;

	virtual void                ChangeLevel(IConsoleCmdArgs* pArgs = NULL);

	virtual bool                NextPairMatch() const;

	virtual TRandomisationFlags GetRandomisationFlags() const { return m_randFlags; }
	virtual void                SetRandomisationFlags(TRandomisationFlags flags);

	virtual bool                IsRandom() const;
	//~ILevelRotation

	ILINE void                       SetExtendedInfoId(const ILevelRotation::TExtInfoId id) { m_extInfoId = id; }
	ILINE ILevelRotation::TExtInfoId GetExtendedInfoId()                                    { return m_extInfoId; }

protected:

	void      ModeShuffle();
	void      ShallowShuffle();
	void      AddGameMode(SLevelRotationEntry& level, tukk gameMode);
	void      AddLevelFromNode(tukk levelName, XmlNodeRef& levelNode);

	ILINE i32 GetRealRotationIndex(uint idx, bool accessShuffled) const
	{
		i32 realId = idx;
		if (idx < m_rotation.size())
		{
			if (accessShuffled && (m_randFlags & ePRF_Shuffle))
			{
				realId = m_shuffle[idx];
			}
		}
		else
		{
			realId = -1;
		}

		Log_LevelRotation(" GetRealRotationIndex passed %d returning %d", idx, realId);
		return realId;
	}

	TLevelRotationVector       m_rotation;
	TRandomisationFlags        m_randFlags;
	i32                        m_next;
	std::vector<uint>          m_shuffle;
	i32                        m_advancesTaken;

	ILevelRotation::TExtInfoId m_extInfoId;   //if this ID is set, we disable implicit shuffling
	bool                       m_hasGameModesDecks;
};

class CLevelSystem :
	public ILevelSystem,
	public ISystem::ILoadingProgressListener
{
public:
	CLevelSystem(ISystem* pSystem);
	virtual ~CLevelSystem();

	void Release() { delete this; };

	// ILevelSystem
	virtual DynArray<string>* GetLevelTypeList();
	virtual void              Rescan(tukk levelsFolder, u32k tag);
	virtual void              LoadRotation();
	virtual i32               GetLevelCount();
	virtual ILevelInfo*       GetLevelInfo(i32 level);
	virtual ILevelInfo*       GetLevelInfo(tukk levelName);

	virtual void              AddListener(ILevelSystemListener* pListener);
	virtual void              RemoveListener(ILevelSystemListener* pListener);

	virtual ILevelInfo*       GetCurrentLevel() const { return m_pCurrentLevelInfo; }
	virtual ILevelInfo*       LoadLevel(tukk levelName);
	virtual void              UnLoadLevel();
	virtual ILevelInfo*       SetEditorLoadedLevel(tukk levelName, bool bReadLevelInfoMetaData = false);
	virtual void              PrepareNextLevel(tukk levelName);
	virtual float             GetLastLevelLoadTime() { return m_fLastLevelLoadTime; };
	virtual bool              IsLevelLoaded()        { return m_bLevelLoaded; }

	virtual ILevelRotation*   GetLevelRotation()     { return &m_levelRotation; };

	virtual ILevelRotation*   FindLevelRotationForExtInfoId(const ILevelRotation::TExtInfoId findId);

	virtual bool              AddExtendedLevelRotationFromXmlRootNode(const XmlNodeRef rootNode, tukk altRootTag, const ILevelRotation::TExtInfoId extInfoId);
	virtual void              ClearExtendedLevelRotations();
	virtual ILevelRotation*   CreateNewRotation(const ILevelRotation::TExtInfoId id);
	// ~ILevelSystem

	// ILoadingProgessListener
	virtual void OnLoadingProgress(i32 steps);
	// ~ILoadingProgessListener

	void PrecacheLevelRenderData();
	void GetMemoryUsage(IDrxSizer* s) const;

	void SaveOpenedFilesList();

private:

	// ILevelSystemListener events notification
	void OnLevelNotFound(tukk levelName);
	void OnLoadingStart(ILevelInfo* pLevel);
	void OnLoadingLevelEntitiesStart(ILevelInfo* pLevelInfo);
	void OnLoadingComplete(ILevelInfo* pLevel);
	void OnLoadingError(ILevelInfo* pLevel, tukk error);
	void OnLoadingProgress(ILevelInfo* pLevel, i32 progressAmount);
	void OnUnloadComplete(ILevelInfo* pLevel);

	void    ScanFolder(const string& rootFolder, bool modFolder, u32k tag);
	void    LogLoadingTime();
	bool    LoadLevelInfo(CLevelInfo& levelInfo);

	// internal get functions for the level infos ... they preserve the type and don't
	// directly cast to the interface
	CLevelInfo* GetLevelInfoInternal(i32 level);
	CLevelInfo* GetLevelInfoInternal(tukk levelName);
	CLevelInfo* GetLevelInfoByPathInternal(tukk szLevelPath);

	typedef std::vector<CLevelRotation> TExtendedLevelRotations;

	ISystem*                           m_pSystem;
	std::vector<CLevelInfo>            m_levelInfos;
	string                             m_levelsFolder;
	ILevelInfo*                        m_pCurrentLevelInfo;
	ILevelInfo*                        m_pLoadingLevelInfo;

	TExtendedLevelRotations            m_extLevelRotations;
	CLevelRotation                     m_levelRotation;

	string                             m_lastLevelName;
	float                              m_fLastLevelLoadTime;
	float                              m_fFilteredProgress;
	float                              m_fLastTime;

	bool                               m_bLevelLoaded;
	bool                               m_bRecordingFileOpens;

	i32                                m_nLoadedLevelsCount;

	CTimeValue                         m_levelLoadStartTime;

	static i32                         s_loadCount;

	std::vector<ILevelSystemListener*> m_listeners;

	DynArray<string>                   m_levelTypeList;
};

#endif //__LEVELSYSTEM_H__
