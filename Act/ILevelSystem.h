// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Gathers level information. Loads a level.

   -------------------------------------------------------------------------
   История:
   - 18:8:2004   11:22 : Created by Márcio Martins

*************************************************************************/

#ifndef __ILEVELSYSTEM_H__
#define __ILEVELSYSTEM_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/CoreX/Memory/DrxSizer.h>
#include <drx3D/FlowGraph/IFlowSystem.h>

struct ILevelRotationFile;
struct IConsoleCmdArgs;

struct ILevelRotation
{
	virtual ~ILevelRotation(){}
	typedef u32 TExtInfoId;

	struct IExtendedInfo
	{
	};

	typedef u8 TRandomisationFlags;

	enum EPlaylistRandomisationFlags
	{
		ePRF_None          = 0,
		ePRF_Shuffle       = 1 << 0,
		ePRF_MaintainPairs = 1 << 1,
	};

	virtual bool Load(ILevelRotationFile* file) = 0;
	virtual bool LoadFromXmlRootNode(const XmlNodeRef rootNode, tukk altRootTag) = 0;

	virtual void Reset() = 0;
	virtual i32  AddLevel(tukk level) = 0;
	virtual void AddGameMode(i32 level, tukk gameMode) = 0;

	virtual i32  AddLevel(tukk level, tukk gameMode) = 0;

	//call to set the playlist ready for a new session
	virtual void                                Initialise(i32 nSeed) = 0;

	virtual bool                                First() = 0;
	virtual bool                                Advance() = 0;
	virtual bool                                AdvanceAndLoopIfNeeded() = 0;

	virtual tukk                         GetNextLevel() const = 0;
	virtual tukk                         GetNextGameRules() const = 0;
	virtual i32                                 GetLength() const = 0;
	virtual i32                                 GetTotalGameModeEntries() const = 0;
	virtual i32                                 GetNext() const = 0;

	virtual tukk                         GetLevel(u32 idx, bool accessShuffled = true) const = 0;
	virtual i32                                 GetNGameRulesForEntry(u32 idx, bool accessShuffled = true) const = 0;
	virtual tukk                         GetGameRules(u32 idx, u32 iMode, bool accessShuffled = true) const = 0;

	virtual tukk                         GetNextGameRulesForEntry(i32 idx) const = 0;

	virtual i32k                           NumAdvancesTaken() const = 0;
	virtual void                                ResetAdvancement() = 0;

	virtual bool                                IsRandom() const = 0;

	virtual ILevelRotation::TRandomisationFlags GetRandomisationFlags() const = 0;
	virtual void                                SetRandomisationFlags(TRandomisationFlags flags) = 0;

	virtual void                                ChangeLevel(IConsoleCmdArgs* pArgs = NULL) = 0;

	virtual bool                                NextPairMatch() const = 0;

};

struct ILevelInfo
{
	virtual ~ILevelInfo(){}

	typedef struct 
	{
		string name;
		string xmlFile;
		i32    cgfCount;
		void   GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(name);
			pSizer->AddObject(xmlFile);
		}
	} TGameTypeInfo;

	struct SMinimapInfo
	{
		SMinimapInfo() : fStartX(0), fStartY(0), fEndX(1), fEndY(1), fDimX(1), fDimY(1), iWidth(1024), iHeight(1024) {}

		string sMinimapName;
		i32    iWidth;
		i32    iHeight;
		float  fStartX;
		float  fStartY;
		float  fEndX;
		float  fEndY;
		float  fDimX;
		float  fDimY;
	};

	virtual tukk                      GetName() const = 0;
	virtual const bool                       IsOfType(tukk sType) const = 0;
	virtual tukk                      GetPath() const = 0;
	virtual tukk                      GetPaks() const = 0;
	virtual tukk                      GetDisplayName() const = 0;
	virtual tukk                      GetPreviewImagePath() const = 0;
	virtual tukk                      GetBackgroundImagePath() const = 0;
	virtual tukk                      GetMinimapImagePath() const = 0;
	virtual i32                              GetHeightmapSize() const = 0;
	virtual const bool                       MetadataLoaded() const = 0;
	virtual bool                             GetIsModLevel() const = 0;
	virtual u32k                     GetScanTag() const = 0;
	virtual u32k                     GetLevelTag() const = 0;

	virtual i32                              GetGameTypeCount() const = 0;
	virtual const ILevelInfo::TGameTypeInfo* GetGameType(i32 gameType) const = 0;
	virtual bool                             SupportsGameType(tukk gameTypeName) const = 0;
	virtual const ILevelInfo::TGameTypeInfo* GetDefaultGameType() const = 0;
	virtual size_t                           GetGameRulesCount() const = 0;
	virtual size_t                           GetGameRules(tukk* pszGameRules, size_t numGameRules) const = 0;
	virtual bool                             HasGameRules() const = 0;

	virtual const ILevelInfo::SMinimapInfo&  GetMinimapInfo() const = 0;

	virtual tukk                      GetDefaultGameRules() const = 0;

	virtual bool                             GetAttribute(tukk name, TFlowInputData& val) const = 0;

	template<typename T> bool                GetAttribute(tukk name, T& outVal) const
	{
		TFlowInputData val;
		if (GetAttribute(name, val) == false)
			return false;
		return val.GetValueWithConversion(outVal);
	}
};

struct ILevelSystemListener
{
	virtual ~ILevelSystemListener(){}
	virtual void OnLevelNotFound(tukk levelName) = 0;
	virtual void OnLoadingStart(ILevelInfo* pLevel) = 0;
	virtual void OnLoadingLevelEntitiesStart(ILevelInfo* pLevel) = 0;
	virtual void OnLoadingComplete(ILevelInfo* pLevel) = 0;
	virtual void OnLoadingError(ILevelInfo* pLevel, tukk error) = 0;
	virtual void OnLoadingProgress(ILevelInfo* pLevel, i32 progressAmount) = 0;
	virtual void OnUnloadComplete(ILevelInfo* pLevel) = 0;

	void         GetMemoryUsage(IDrxSizer* pSizer) const { /*nothing*/ }
};

struct ILevelSystem :
	public ILevelSystemListener
{
	enum
	{
		TAG_MAIN    = 'MAIN',
		TAG_UNKNOWN = 'ZZZZ'
	};

	virtual void              Rescan(tukk levelsFolder, u32k tag) = 0;
	virtual void              LoadRotation() = 0;
	virtual i32               GetLevelCount() = 0;
	virtual DynArray<string>* GetLevelTypeList() = 0;
	virtual ILevelInfo*       GetLevelInfo(i32 level) = 0;
	virtual ILevelInfo*       GetLevelInfo(tukk levelName) = 0;

	virtual void              AddListener(ILevelSystemListener* pListener) = 0;
	virtual void              RemoveListener(ILevelSystemListener* pListener) = 0;

	virtual ILevelInfo*       GetCurrentLevel() const = 0;
	virtual ILevelInfo*       LoadLevel(tukk levelName) = 0;
	virtual void              UnLoadLevel() = 0;
	virtual ILevelInfo*       SetEditorLoadedLevel(tukk levelName, bool bReadLevelInfoMetaData = false) = 0;
	virtual bool              IsLevelLoaded() = 0;
	virtual void              PrepareNextLevel(tukk levelName) = 0;

	virtual ILevelRotation*   GetLevelRotation() = 0;

	virtual ILevelRotation*   FindLevelRotationForExtInfoId(const ILevelRotation::TExtInfoId findId) = 0;

	virtual bool              AddExtendedLevelRotationFromXmlRootNode(const XmlNodeRef rootNode, tukk altRootTag, const ILevelRotation::TExtInfoId extInfoId) = 0;
	virtual void              ClearExtendedLevelRotations() = 0;
	virtual ILevelRotation*   CreateNewRotation(const ILevelRotation::TExtInfoId id) = 0;

	// Retrieve`s last level level loading time.
	virtual float GetLastLevelLoadTime() = 0;
};

#endif //__ILEVELSYSTEM_H__
