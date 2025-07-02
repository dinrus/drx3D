// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#ifndef RESOURCE_COMPILER
#include <drx3D/Animation/GlobalAnimationHeaderLMG.h>
#include <drx3D/Animation/GlobalAnimationHeaderCAF.h>
#include <drx3D/Animation/GlobalAnimationHeaderAIM.h>
#include <drx3D/Animation/LoaderDBA.h>

class CAnimationSet;
class DrxCGALoader;
class CAnimationUpr;
class CAnimationSet;
struct CInternalSkinningInfo;

struct AnimSearchHelper
{

	typedef std::vector<i32>                                                     TIndexVector;
	typedef std::map<u32 /*crc*/, TIndexVector* /*vector of indexes*/>        TFoldersVector;

	typedef std::vector<u32>                                                  TSubFolderCrCVector;
	typedef std::map<u32 /*crc*/, TSubFolderCrCVector* /*vector of indexes*/> TSubFoldersMap;

	TFoldersVector       m_AnimationsMap;
	TSubFoldersMap       m_SubFoldersMap;

	bool                 AddAnimation(u32 crc, u32 gahIndex);
	void                 AddAnimation(const string& path, u32 gahIndex);

	TSubFolderCrCVector* GetSubFoldersVector(u32 crc);

	TIndexVector*        GetAnimationsVector(u32 crc);
	TIndexVector*        GetAnimationsVector(const string& path);
	void                 Clear();

	AnimSearchHelper() {}
	~AnimSearchHelper() { Clear(); };

private:
	// Ensure non copyable
	AnimSearchHelper(const AnimSearchHelper&);
	AnimSearchHelper& operator=(const AnimSearchHelper&);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
// class CAnimationUpr
// Ответственен за создание множества анимаций, последовательно привязанных к "костям персонажа".
// На игру имеется только один экземпляр этого класса.
/////////////////////////////////////////////////////////////////////////////////////////////////////
class CAnimationUpr : public IAnimEvents
{
public:

	virtual IAnimEventList*       GetAnimEventList(tukk animationFilePath);
	virtual const IAnimEventList* GetAnimEventList(tukk animationFilePath) const;

	virtual bool                  SaveAnimEventToXml(const CAnimEventData& dataIn, XmlNodeRef& dataOut);
	virtual bool                  LoadAnimEventFromXml(const XmlNodeRef& dataIn, CAnimEventData& dataOut);
	virtual void                  InitializeSegmentationDataFromAnimEvents(tukk animationFilePath);

	void                          InitialiseRunTimePools();
	void                          DestroyRunTimePools();

	/////////////////////////////////////////////////////////////////////
	// finds the animation-asset by path-name.
	// Returns -1 if no animation was found.
	// Returns the animation ID if it was found.
	/////////////////////////////////////////////////////////////////////

	//! Searches for a CAF animation asset by its name.
	//! \param szFilePath Name of the animation to search for.
	//! \return Global ID of the animation or -1 if no animation was found.
	i32 GetGlobalIDbyFilePath_CAF(tukk szFilePath) const;

	//! Searches for an AIM animation asset by its name.
	//! \param szFilePath Name of the animation to search for.
	//! \return Global ID of the animation or -1 if no animation was found.
	i32 GetGlobalIDbyFilePath_AIM(tukk szFilePath) const;

	//! Searches for a blend-space (locomotion group) asset by its name.
	//! \param szFilePath Name of the blend-space to search for.
	//! \return Global ID of the blend-space or -1 if no blend-space was found.
	i32 GetGlobalIDbyFilePath_LMG(tukk szFilePath) const;

	//! Searches for a blend-space (locomotion group) asset by a crc32 hash of its name.
	//! \param szFilePath Lowercase crc32 hash of the blend-space name to search for.
	//! \return Global ID of the blend-space or -1 if no blend-space was found.
	i32 GetGlobalIDbyFilePathCRC_LMG(u32 crc32) const;

	// create header with the specified name. If the header already exists, it return the index
	i32               CreateGAH_CAF(const string& strFileName);
	i32               CreateGAH_AIM(const string& strFileName);
	i32               CreateGAH_LMG(const string& strFileName);

	CGlobalHeaderDBA* FindGlobalHeaderByCRC_DBA(u32 crc);

	// loads existing animation record, returns false on error
	bool LoadAnimationTCB(i32 nGlobalAnimId, DynArray<CControllerTCB>& m_LoadCurrAnimation, DrxCGALoader* pCGA, const IDefaultSkeleton* pIDefaultSkeleton);

	void UnloadAnimationCAF(GlobalAnimationHeaderCAF& rCAF);
	void UnloadAnimationAIM(i32 nGLobalAnimID);

	// returns the total number of animations hold in memory (for statistics)
	size_t GetGlobalAnimCount() { return m_arrGlobalCAF.size();  }

	// notifies the controller manager that this client doesn't use the given animation any more.
	// these calls must be balanced with AnimationAddRef() calls
	void AnimationReleaseCAF(GlobalAnimationHeaderCAF& rCAF);
	void AnimationReleaseAIM(GlobalAnimationHeaderAIM& rAIM);
	void AnimationReleaseLMG(GlobalAnimationHeaderLMG& rLMG);

	// puts the size of the whole subsystem into this sizer object, classified, according to the flags set in the sizer
	void GetMemoryUsage(class IDrxSizer* pSizer) const;
	void             DebugAnimUsage(u32 printtxt);

	size_t           GetSizeOfDBA();
	bool             CreateGlobalHeaderDBA(DynArray<string>& name);
	bool             DBA_PreLoad(tukk pFilePathDBA, bool highPriority);
	bool             DBA_LockStatus(tukk pFilePathDBA, u32 status, bool highPriority);
	bool             DBA_Unload(tukk pFilePathDBA);
	bool             DBA_Unload_All();
	bool             Unload_All_Animation();

	EReloadCAFResult ReloadCAF(tukk szFilePathCAF);
	i32              ReloadLMG(tukk szFilePathLMG);

	u32           GetDBACRC32fromFilePath(tukk szFilePathCAF);
	bool             IsDatabaseInMemory(u32 nDBACRC32);

	DynArray<CGlobalHeaderDBA>         m_arrGlobalHeaderDBA;
	CNameCRCMap                        m_AnimationMapCAF;
	DynArray<GlobalAnimationHeaderCAF> m_arrGlobalCAF;
	//	CNameCRCMap		m_AnimationMapAIM;
	DynArray<GlobalAnimationHeaderAIM> m_arrGlobalAIM;
	//	CNameCRCMap		m_AnimationMapLMG;
	DynArray<GlobalAnimationHeaderLMG> m_arrGlobalLMG;

#ifdef EDITOR_PCDEBUGCODE
	typedef std::map<string, DynArray<char>> CachedBSPACES;
	CachedBSPACES m_cachedBSPACES;
#endif

	AnimSearchHelper   m_animSearchHelper;
	SAnimMemoryTracker m_AnimMemoryTracker;

	bool               m_shuttingDown;

	CAnimationUpr(const CAnimationUpr&);
	void operator=(const CAnimationUpr&);

	CAnimationUpr() : m_shuttingDown(false)
	{
		//We reserve the place for future animations.
		//The best performance will be archived when this number is >= actual number of animations that can be used, and not much greater
		m_arrGlobalHeaderDBA.reserve(128);
		m_arrGlobalCAF.reserve(0);
		m_arrGlobalAIM.reserve(0);
		m_arrGlobalLMG.reserve(128);

#if BLENDSPACE_VISUALIZATION
		//the first LMG must be an Internal Type (it is a pre-initalized 1D blend-space). We use it just for debugging on PC
		i32 nGlobalAnimID = CreateGAH_LMG("InternalPara1D.LMG");
		m_arrGlobalLMG[nGlobalAnimID].CreateInternalType_Para1D();
#endif

		InitialiseRunTimePools();
	}

	~CAnimationUpr()
	{
		m_shuttingDown = true;
		DestroyRunTimePools();
	}

private:
};
#endif