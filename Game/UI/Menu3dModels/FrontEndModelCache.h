// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FRONTENDMODELCACHE_H__
#define __FRONTENDMODELCACHE_H__

// Includes
#include <drx3D/Game/ItemResourceCache.h>

// Defines
#ifndef _RELEASE
	#define FEMC_CACHE_FILE_ACCESSES	1  // Set this to 1 to capture files loaded during the caching
#else
	#define FEMC_CACHE_FILE_ACCESSES	0
#endif

//==================================================================================================
// Name: CFrontEndModelCache
// Desc: Cache for front end models
// Created by: Tim Furnish
// Refactored by: James Chilvers
//==================================================================================================
class CFrontEndModelCache
#if FEMC_CACHE_FILE_ACCESSES
	: public IDrxPakFileAcesssSink
#endif // #if FE_CACHE_FILE_ACCESSES
{
public:

	// Static functions
	static ILINE bool				Exists()
	{
		return (s_pSingletonInstance) ? true : false;
	}

	static void Allow3dFrontEndAssets(bool bAllowed, bool bUpdateImmediate);
	static bool IsAllowed3dFrontEndAssets();
	static void UpdateNeed3dFrontEndAssets();

protected:
	CFrontEndModelCache();
	virtual ~CFrontEndModelCache();

private:
	static ILINE void				Create()
	{
		if((s_pSingletonInstance==NULL) && (!gEnv->IsEditor()))
		{
			s_pSingletonInstance = new CFrontEndModelCache;
		}
	}

	static ILINE void				Release()
	{
		SAFE_DELETE(s_pSingletonInstance);
	}

	void CacheItemModel(tukk pItemName);
	void CacheCharacterModel(tukk pModelFileName);
	void CreateSupportForFrontEnd3dModels();
	void ReleaseSupportForFrontEnd3dModels();

	void SwitchToLevelHeap();
	void SwitchToGlobalHeap();

#if FEMC_CACHE_FILE_ACCESSES
	ILINE static void SetReasonForLoggingFileOpens(tukk pNewReason)
	{
		if(s_pSingletonInstance)
		{
			s_pSingletonInstance->m_pReasonForReportingFileOpen = pNewReason;
		}
	}

	void ReportFileOpen(FILE *pFile, tukk pFullPath);
	tukk m_pReasonForReportingFileOpen;
	std::vector<string> m_recordedFiles;
#endif // #if FE_CACHE_FILE_ACCESSES

	// Static vars
	static CFrontEndModelCache*		s_pSingletonInstance;
	static bool										s_bAllowedToLoad3dFrontEndAssets;
	static bool										s_bNeed3dFrontEndAssets;

	// Member vars
	CItemGeometryCache m_myGeometryCache;

	typedef _smart_ptr<IMaterial> TMaterialPtr;
	typedef std::vector<TMaterialPtr>	TMaterialVec;
	TMaterialVec m_materialCache;
	i32  m_prevHeap;
	bool m_bIsMultiplayerCache;
};//------------------------------------------------------------------------------------------------

#endif // __FRONTENDMODELCACHE_H__
