// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/CharacterUpr.h>

#include <drx3D/Animation/CharacterInstance.h>
#include <drx3D/Animation/LoaderCHR.h>
#include <drx3D/Animation/LoaderCGA.h>
#include <drx3D/Animation/AnimationUpr.h>
#include<drx3D/Animation/FaceAnimation.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Sys/IResourceUpr.h>
#include <drx3D/Animation/ParametricSampler.h>
#include <drx3D/Animation/AttachmentVCloth.h>
#include <drx3D/Animation/AttachmentMerger.h>
#include <drx3D/Animation/SerializationCommon.h>

float g_YLine = 0.0f;

#define CDF_LEVEL_CACHE_PAK     "cdf.pak"
#define CHR_LEVEL_CACHE_PAK     "chr.pak"
#define CGA_LEVEL_CACHE_PAK     "cga.pak"

#define DBA_UNLOAD_MAX_DELTA_MS 2000

#define INVALID_CDF_ID          ~0
/*
   CharacterUpr
 */

bool CharacterUpr::s_bPaused = false;
u32 CharacterUpr::s_renderFrameIdLocal = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////
// CharacterUpr
///////////////////////////////////////////////////////////////////////////////////////////////////////

CharacterUpr::CharacterUpr()
{
	m_AllowStartOfAnimation = 1;
	m_arrFrameTimes.resize(200);
	for (u32 i = 0; i < 200; i++)
		m_arrFrameTimes[i] = 0.014f;

	m_pFacialAnimation = new CFacialAnimation();
	m_nUpdateCounter = 0;
	m_InitializedByIMG = 0;
	m_StartGAH_Iterator = 0;

	m_pStreamingListener = NULL;

	m_arrModelCacheSKEL.reserve(100);
	m_arrModelCacheSKIN.reserve(100);
	g_SkeletonUpdates = 0;
	g_AnimationUpdates = 0;
	m_nFrameSyncTicks = 0;
	m_nFrameTicks = 0;
	m_nActiveCharactersLastFrame = 0;

	UpdateDatabaseUnloadTimeStamp();
	m_arrCacheForCDF.reserve(1024);

	memset(m_nStreamUpdateRoundId, 0, sizeof(m_nStreamUpdateRoundId));
}

void CharacterUpr::PostInit()
{
	if (g_pISystem == 0)
		DrxFatalError("DinrusXAnimation: ISystem not initialized");

	PREFAST_ASSUME(g_pISystem);
	g_pIRenderer = g_pISystem->GetIRenderer();

	g_pIPhysicalWorld = g_pISystem->GetIPhysicalWorld();
	if (g_pIPhysicalWorld == 0)
		DrxFatalError("DinrusXAnimation: failed to initialize pIPhysicalWorld");

	g_pI3DEngine = g_pISystem->GetI3DEngine();
	if (g_pI3DEngine == 0)
		DrxFatalError("DinrusXAnimation: failed to initialize pI3DEngine");
}

CharacterUpr::~CharacterUpr()
{
	u32 numModels = m_arrModelCacheSKEL.size();
	if (numModels)
	{
		g_pILog->LogToFile("*ERROR* CharacterUpr: %" PRISIZE_T " body instances still not deleted. Forcing deletion (though some instances may still refer to them)", m_arrModelCacheSKEL.size());
		CleanupModelCache(true);
	}

	delete m_pFacialAnimation;

	//g_DeleteInterfaces();
}

//////////////////////////////////////////////////////////////////////////
void CharacterUpr::PreloadLevelModels()
{
	LOADING_TIME_PROFILE_SECTION;
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Preload Characters");

	//bool bCdfCacheExist = GetISystem()->GetIResourceUpr()->LoadLevelCachePak( CDF_LEVEL_CACHE_PAK,"" ); // Keep it open until level end.

	PreloadModelsCHR();

	PreloadModelsCGA();

	PreloadModelsCDF();

	if (Console::GetInst().ca_PreloadAllCAFs)
	{
		//This is a blocking call to load all global animation headers rather than waiting for them to stream in
		//Alternative is to use the DBA system in asset pipeline
		i32k numGlobalCAF = GetAnimationUpr().m_arrGlobalCAF.size();

		for (i32 i = 0; i < numGlobalCAF; ++i)
		{
			GlobalAnimationHeaderCAF& rCAF = GetAnimationUpr().m_arrGlobalCAF[i];
			if (rCAF.IsAssetLoaded() == 0 && rCAF.IsAssetRequested() == 0)
				rCAF.LoadCAF();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CharacterUpr::PreloadModelsCHR()
{
	LOADING_TIME_PROFILE_SECTION;
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Preload CHR");

	DrxLog("===== Preloading Characters ====");

	CTimeValue startTime = gEnv->pTimer->GetAsyncTime();

	//bool bChrCacheExist = GetISystem()->GetIResourceUpr()->LoadLevelCachePak( CHR_LEVEL_CACHE_PAK,"" );

	IResourceList* pResList = GetISystem()->GetIResourceUpr()->GetLevelResourceList();

	// Construct streamer object
	//CLevelStatObjLoader cgfStreamer;

	i32 nChrCounter = 0;
	i32 nInLevelCacheCount = 0;

	// Request objects loading from Streaming System.
	DrxPathString filename;
	tukk sFilenameInResource = pResList->GetFirst();
	while (sFilenameInResource)
	{
		tukk fileExt = PathUtil::GetExt(sFilenameInResource);
		u32 IsSKIN = stricmp(fileExt, DRX_SKIN_FILE_EXT) == 0;
		u32 IsSKEL = stricmp(fileExt, DRX_SKEL_FILE_EXT) == 0;

		if (IsSKEL)
		{
			tukk sLodName = strstr(sFilenameInResource, "_lod");
			if (sLodName && (sLodName[4] >= '0' && sLodName[4] <= '9'))
			{
				// Ignore Lod files.
				sFilenameInResource = pResList->GetNext();
				continue;
			}
			filename = sFilenameInResource;
			u32 nFileOnDisk = gEnv->pDrxPak->IsFileExist(filename);
			if (nFileOnDisk)
			{
				CDefaultSkeleton* pSkel = FetchModelSKEL(filename.c_str(), 0);
				if (pSkel)
					pSkel->SetKeepInMemory(true), nChrCounter++;
			}
		}

		if (IsSKIN)  //we don't want ".chrparams" in the list
		{
			tukk sLodName = strstr(sFilenameInResource, "_lod");
			if (sLodName && (sLodName[4] >= '0' && sLodName[4] <= '9'))
			{
				// Ignore Lod files.
				sFilenameInResource = pResList->GetNext();
				continue;
			}
			filename = sFilenameInResource;
			u32 nFileOnDisk = gEnv->pDrxPak->IsFileExist(filename);
			assert(nFileOnDisk);
			if (nFileOnDisk)
			{
				CSkin* pSkin = FetchModelSKIN(filename.c_str(), 0);
				if (pSkin)
					pSkin->SetKeepInMemory(true), nChrCounter++;
			}
		}

		sFilenameInResource = pResList->GetNext();
		SLICE_AND_SLEEP();
	}

	//all reference must be 0 after loading
	u32 numSKELs = m_arrModelCacheSKEL.size();
	for (u32 i = 0; i < numSKELs; i++)
	{
		u32 numRefs = m_arrModelCacheSKEL[i].m_pDefaultSkeleton->GetRefCounter();
		if (numRefs)
			DrxFatalError("SKEL NumRefs: %d", numRefs);
	}
	u32 numSKINs = m_arrModelCacheSKIN.size();
	for (u32 i = 0; i < numSKINs; i++)
	{
		u32 numRefs = m_arrModelCacheSKIN[i].m_pDefaultSkinning->GetRefCounter();
		if (numRefs)
			DrxFatalError("SKIN NumRefs: %d", numRefs);
	}

	CTimeValue endTime = gEnv->pTimer->GetAsyncTime();

	float dt = (endTime - startTime).GetSeconds();

	DrxLog("===== Finished Preloading %d Characters in %.2f seconds ====", nChrCounter, dt);

	//GetISystem()->GetIResourceUpr()->UnloadLevelCachePak( CHR_LEVEL_CACHE_PAK );
}

void CharacterUpr::PreloadModelsCGA()
{
	LOADING_TIME_PROFILE_SECTION;
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Preload CGA");

	DrxLog("===== Preloading CGAs ====");

	CTimeValue startTime = gEnv->pTimer->GetAsyncTime();

	//bool bCGACacheExist = false;

	bool bCgfCacheExist = false;
	static ICVar* pCVar_e_StreamCgf = gEnv->pConsole->GetCVar("e_StreamCgf");
	if (pCVar_e_StreamCgf && pCVar_e_StreamCgf->GetIVal() != 0)
	{
		// Only when streaming enable use no-mesh cga pak.
		//bCGACacheExist = GetISystem()->GetIResourceUpr()->LoadLevelCachePak( CGA_LEVEL_CACHE_PAK,"" );
	}

	i32 nChrCounter = 0;
	IResourceList* pResList = GetISystem()->GetIResourceUpr()->GetLevelResourceList();

	DrxCGALoader CGALoader;

	// Request objects loading from Streaming System.
	DrxPathString filename;
	tukk sFilenameInResource = pResList->GetFirst();
	u32k nLoadingFlags = 0;
	while (sFilenameInResource)
	{
		if (strstr(sFilenameInResource, ".cga"))
		{
			tukk sLodName = strstr(sFilenameInResource, "_lod");
			if (sLodName && (sLodName[4] >= '0' && sLodName[4] <= '9'))
			{
				// Ignore Lod files.
				sFilenameInResource = pResList->GetNext();
				continue;
			}

			filename = sFilenameInResource;
			u32 nFileOnDisk = gEnv->pDrxPak->IsFileExist(filename);
			assert(nFileOnDisk);
			CDefaultSkeleton* pSkelTemp = 0;    //temp-pointer to access the refcounter, without calling the destructor
			if (nFileOnDisk)
			{
				LoadAnimationImageFile("animations/animations.img", "animations/DirectionalBlends.img");
				CGALoader.Reset();
				CDefaultSkeleton* pDefaultSkeleton = CGALoader.LoadNewCGA(filename, this, nLoadingFlags);
				if (pDefaultSkeleton)
				{
					pDefaultSkeleton->SetKeepInMemory(true);
					RegisterModelSKEL(pDefaultSkeleton, nLoadingFlags);
					nChrCounter++;
				}
			}
		}
		sFilenameInResource = pResList->GetNext();

		SLICE_AND_SLEEP();
	}

	//all reference must be 0 after loading
	u32 numSKELs = m_arrModelCacheSKEL.size();
	for (u32 i = 0; i < numSKELs; i++)
	{
		u32 numRefs = m_arrModelCacheSKEL[i].m_pDefaultSkeleton->GetRefCounter();
		if (numRefs)
			DrxFatalError("SKEL NumRefs: %d", numRefs);
	}

	//GetISystem()->GetIResourceUpr()->UnloadLevelCachePak( CGA_LEVEL_CACHE_PAK );

	CTimeValue endTime = gEnv->pTimer->GetAsyncTime();

	float dt = (endTime - startTime).GetSeconds();
	DrxLog("===== Finished Preloading %d CGAs in %.2f seconds ====", nChrCounter, dt);
}

//////////////////////////////////////////////////////////////////////////
void CharacterUpr::PreloadModelsCDF()
{
	LOADING_TIME_PROFILE_SECTION;
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Preload CDF");

	DrxLog("===== Preloading CDFs ====");

	CTimeValue startTime = gEnv->pTimer->GetAsyncTime();

	i32 nCounter = 0;

	IResourceList* pResList = GetISystem()->GetIResourceUpr()->GetLevelResourceList();

	// Request objects loading from Streaming System.
	DrxPathString filename;
	tukk sFilenameInResource = pResList->GetFirst();
	while (sFilenameInResource)
	{
		if (strstr(sFilenameInResource, ".cdf"))
		{
			filename = sFilenameInResource;

			if (gEnv->pDrxPak->IsFileExist(filename))
			{
				u32k nCdfIndex = LoadCDF(filename);
				if (nCdfIndex != INVALID_CDF_ID)
				{
					m_arrCacheForCDF[nCdfIndex].AddRef(); // Keep reference to it.
					nCounter++;
				}
			}
		}
		sFilenameInResource = pResList->GetNext();

		SLICE_AND_SLEEP();
	}

	CTimeValue endTime = gEnv->pTimer->GetAsyncTime();

	float dt = (endTime - startTime).GetSeconds();
	DrxLog("===== Finished Preloading %d CDFs in %.2f seconds ====", nCounter, dt);
}

//////////////////////////////////////////////////////////////////////////
// Loads a cgf and the corresponding caf file and creates an animated object,
// or returns an existing object.
ICharacterInstance* CharacterUpr::CreateInstance(tukk szFilePath, u32 nLoadingFlags)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Characters");
	if (szFilePath == 0)
		return (NULL);                                                      // to prevent a crash in the frequent case the designers will mess

	stack_string strFilePath = szFilePath;
	PathUtil::UnifyFilePath(strFilePath);

#if !DRX_PLATFORM_DESKTOP
	//there is no char-edit on consoles
	nLoadingFlags = nLoadingFlags & (CA_CharEditModel ^ -1);
#endif

	tukk fileExt = PathUtil::GetExt(strFilePath.c_str());
	u32 isSKIN = stricmp(fileExt, DRX_SKIN_FILE_EXT) == 0;
	u32 isSKEL = stricmp(fileExt, DRX_SKEL_FILE_EXT) == 0;
	u32 isCGA = stricmp(fileExt, "cga") == 0;
	u32 isCDF = stricmp(fileExt, "cdf") == 0;

#ifdef EDITOR_PCDEBUGCODE
	if (isSKIN)
		return CreateSKELInstance(strFilePath.c_str(), nLoadingFlags);       //Load SKIN and install it as a SKEL file (this is only needed in CharEdit-Mode to preview the model)
#endif

	if (isSKEL)
		return CreateSKELInstance(strFilePath.c_str(), nLoadingFlags);       //Loading SKEL file.

	if (isCGA)
		return CreateCGAInstance(strFilePath.c_str(), nLoadingFlags);        //Loading CGA file.

	if (isCDF)
		return LoadCharacterDefinition(strFilePath.c_str(), nLoadingFlags);  //Loading CDF file.

	g_pILog->LogError("DinrusXAnimation: no valid character file-format a SKEL-Instance: %s", strFilePath.c_str());
	return 0;                                                             //if it ends here, then we have no valid file-format
}

ICharacterInstance* CharacterUpr::CreateSKELInstance(tukk strFilePath, u32 nLoadingFlags)
{
	LOADING_TIME_PROFILE_SECTION(g_pISystem);
	CDefaultSkeleton* pModelSKEL = CheckIfModelSKELLoaded(strFilePath, nLoadingFlags);
	if (pModelSKEL == 0)
	{
		pModelSKEL = FetchModelSKEL(strFilePath, nLoadingFlags);               //SKIN not in memory, so load it
		if (pModelSKEL == 0)
			return NULL;                                                      // the model has not been loaded
	}
	else
	{
		u32 isPrevMode = nLoadingFlags & CA_PreviewMode;
		if (isPrevMode == 0)
		{
			u32 numAnims = pModelSKEL->m_pAnimationSet->GetAnimationCount();
			if (numAnims == 0)
			{
				tukk szExt = PathUtil::GetExt(strFilePath);
				stack_string strGeomFileNameNoExt;
				strGeomFileNameNoExt.assign(strFilePath, *szExt ? szExt - 1 : szExt);
				stack_string paramFileName = strGeomFileNameNoExt + "." + DRX_CHARACTER_PARAM_FILE_EXT;
				pModelSKEL->LoadCHRPARAMS(paramFileName);
			}
		}
	}
	return new CCharInstance(strFilePath, pModelSKEL);
}

//////////////////////////////////////////////////////////////////////////

ISkin* CharacterUpr::LoadModelSKIN(tukk szFilePath, u32 nLoadingFlags)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Characters");
	if (szFilePath == 0)
		return 0;                                                                           //to prevent a crash in the frequent case the designers will mess bad filenames

#if !DRX_PLATFORM_DESKTOP
	//there is no char-edit on consoles
	nLoadingFlags = nLoadingFlags & (CA_CharEditModel ^ -1);
#endif

	stack_string strFilePath = szFilePath;
	PathUtil::UnifyFilePath(strFilePath);

	tukk fileExt = PathUtil::GetExt(strFilePath.c_str());
	u32 isSKIN = stricmp(fileExt, DRX_SKIN_FILE_EXT) == 0;
	if (isSKIN)
	{
		LOADING_TIME_PROFILE_SECTION(g_pISystem);
		CSkin* pModelSKIN = FetchModelSKIN(strFilePath.c_str(), nLoadingFlags);
		return pModelSKIN;
	}

	g_pILog->LogError("DinrusXAnimation: no valid character file-format to create a skin-attachment: %s", strFilePath.c_str());
	return 0;                                                                             //if it ends here, then we have no valid file-format
}

//////////////////////////////////////////////////////////////////////////

IDefaultSkeleton* CharacterUpr::LoadModelSKEL(tukk szFilePath, u32 nLoadingFlags)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Characters");
	if (szFilePath == 0)
		return 0;                                                                           //to prevent a crash in the frequent case the designers will mess bad filenames

#if !DRX_PLATFORM_DESKTOP
	//there is no char-edit on consoles
	nLoadingFlags = nLoadingFlags & (CA_CharEditModel ^ -1);
#endif

	stack_string strFilePath = szFilePath;
	PathUtil::UnifyFilePath(strFilePath);

	tukk fileExt = PathUtil::GetExt(strFilePath.c_str());
	u32 isSKEL = stricmp(fileExt, DRX_SKEL_FILE_EXT) == 0;
	if (isSKEL)
	{
		LOADING_TIME_PROFILE_SECTION(g_pISystem);
		CDefaultSkeleton* pModelSKEL = FetchModelSKEL(strFilePath.c_str(), nLoadingFlags);  //SKEL not in memory, so load it
		return pModelSKEL;                                                                  //SKIN not in memory, so load it
	}

	g_pILog->LogError("DinrusXAnimation: no valid character file-format to create a skin-attachment: %s", strFilePath.c_str());
	return 0;                                                                             //if it ends here, then we have no valid file-format
}

//////////////////////////////////////////////////////////////////////////
CDefaultSkeleton* CharacterUpr::CheckIfModelSKELLoaded(const string& strFilePath, u32 nLoadingFlags)
{
	if ((nLoadingFlags & CA_CharEditModel) == 0)
	{
		u32 numModels = m_arrModelCacheSKEL.size();
		for (u32 m = 0; m < numModels; m++)
		{
			tukk strFilePath1 = strFilePath.c_str();
			tukk strFilePath2 = m_arrModelCacheSKEL[m].m_pDefaultSkeleton->GetModelFilePath();
			bool IsIdentical = stricmp(strFilePath1, strFilePath2) == 0;
			if (IsIdentical)
				return m_arrModelCacheSKEL[m].m_pDefaultSkeleton;                               //model already loaded
		}
	}
	else
	{
#ifdef EDITOR_PCDEBUGCODE
		u32 numModels = m_arrModelCacheSKEL_CharEdit.size();
		for (u32 m = 0; m < numModels; m++)
		{
			tukk strFilePath1 = strFilePath.c_str();
			tukk strFilePath2 = m_arrModelCacheSKEL_CharEdit[m].m_pDefaultSkeleton->GetModelFilePath();
			bool IsIdentical = stricmp(strFilePath1, strFilePath2) == 0;
			if (IsIdentical)
				return m_arrModelCacheSKEL_CharEdit[m].m_pDefaultSkeleton;                      //model already loaded
		}
#endif
	}
	return 0;

}

CDefaultSkeleton* CharacterUpr::CheckIfModelExtSKELCreated(const uint64 nCRC64, u32 nLoadingFlags)
{
	if ((nLoadingFlags & CA_CharEditModel) == 0)
	{
		u32 numModels = m_arrModelCacheSKEL.size();
		for (u32 m = 0; m < numModels; m++)
		{
			uint64 crc64 = m_arrModelCacheSKEL[m].m_pDefaultSkeleton->GetModelFilePathCRC64();
			if (nCRC64 == crc64)
				return m_arrModelCacheSKEL[m].m_pDefaultSkeleton;                               //model already loaded
		}
	}
	else
	{
#ifdef EDITOR_PCDEBUGCODE
		u32 numModels = m_arrModelCacheSKEL_CharEdit.size();
		for (u32 m = 0; m < numModels; m++)
		{
			uint64 crc64 = m_arrModelCacheSKEL_CharEdit[m].m_pDefaultSkeleton->GetModelFilePathCRC64();
			if (nCRC64 == crc64)
				return m_arrModelCacheSKEL_CharEdit[m].m_pDefaultSkeleton;                      //model already loaded
		}
#endif
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CSkin* CharacterUpr::CheckIfModelSKINLoaded(const string& strFilePath, u32 nLoadingFlags)
{
	if ((nLoadingFlags & CA_CharEditModel) == 0)
	{
		u32 numModels = m_arrModelCacheSKIN.size();
		for (u32 m = 0; m < numModels; m++)
		{
			tukk strBodyFilePath = m_arrModelCacheSKIN[m].m_pDefaultSkinning->GetModelFilePath();
			if (strFilePath.compareNoCase(strBodyFilePath) == 0)
				return m_arrModelCacheSKIN[m].m_pDefaultSkinning;                               //model already loaded
		}
	}
	else
	{
#ifdef EDITOR_PCDEBUGCODE
		u32 numModels = m_arrModelCacheSKIN_CharEdit.size();
		for (u32 m = 0; m < numModels; m++)
		{
			tukk strBodyFilePath = m_arrModelCacheSKIN_CharEdit[m].m_pDefaultSkinning->GetModelFilePath();
			if (strFilePath.compareNoCase(strBodyFilePath) == 0)
				return m_arrModelCacheSKIN_CharEdit[m].m_pDefaultSkinning;                      //model already loaded
		}
#endif
	}
	return 0;

}

//////////////////////////////////////////////////////////////////////////
CDefaultSkinningReferences* CharacterUpr::GetDefaultSkinningReferences(CSkin* pDefaultSkinning)
{
	if ((pDefaultSkinning->m_nLoadingFlags & CA_CharEditModel) == 0)
	{
		u32 numModels = m_arrModelCacheSKIN.size();
		for (u32 m = 0; m < numModels; m++)
		{
			if (m_arrModelCacheSKIN[m].m_pDefaultSkinning == pDefaultSkinning)
				return &m_arrModelCacheSKIN[m];
		}
	}
	else
	{
#ifdef EDITOR_PCDEBUGCODE
		u32 numModels = m_arrModelCacheSKIN_CharEdit.size();
		for (u32 m = 0; m < numModels; m++)
		{
			if (m_arrModelCacheSKIN_CharEdit[m].m_pDefaultSkinning == pDefaultSkinning)
				return &m_arrModelCacheSKIN_CharEdit[m];
		}
#endif
	}
	return 0;

}

//////////////////////////////////////////////////////////////////////////
// Finds a cached or creates a new CDefaultSkeleton instance and returns it
// returns NULL if the construction failed
CDefaultSkeleton* CharacterUpr::FetchModelSKEL(tukk szFilePath, u32 nLoadingFlags)
{
	CDefaultSkeleton* pModelSKEL = CheckIfModelSKELLoaded(szFilePath, nLoadingFlags);
	if (pModelSKEL)
		return pModelSKEL;

	pModelSKEL = new CDefaultSkeleton(szFilePath, CHR);
	if (pModelSKEL)
	{
		pModelSKEL->m_pAnimationSet = new CAnimationSet(szFilePath);

		LoadAnimationImageFile("animations/animations.img", "animations/DirectionalBlends.img");
		bool IsLoaded = pModelSKEL->LoadNewSKEL(szFilePath, nLoadingFlags);
		if (IsLoaded == 0)
		{
			delete pModelSKEL;
			return 0; //loading error
		}
		RegisterModelSKEL(pModelSKEL, nLoadingFlags);
		return pModelSKEL;
	}
	return 0;     //some error
}

//////////////////////////////////////////////////////////////////////////

CDefaultSkeleton* CharacterUpr::FetchModelSKELForGCA(tukk szFilePath, u32 nLoadingFlags)
{
	CDefaultSkeleton* pModelSKEL = CheckIfModelSKELLoaded(szFilePath, nLoadingFlags);
	if (pModelSKEL == 0)
	{
		LoadAnimationImageFile("animations/animations.img", "animations/DirectionalBlends.img");
		DrxCGALoader CGALoader;
		pModelSKEL = CGALoader.LoadNewCGA(szFilePath, this, nLoadingFlags);
		if (pModelSKEL)
			RegisterModelSKEL(pModelSKEL, nLoadingFlags);
	}
	return pModelSKEL;
}

//////////////////////////////////////////////////////////////////////////

CSkin* CharacterUpr::FetchModelSKIN(tukk szFilePath, u32 nLoadingFlags)
{
	CSkin* pModelSKIN = CheckIfModelSKINLoaded(szFilePath, nLoadingFlags);
	if (pModelSKIN)
		return pModelSKIN;

	pModelSKIN = new CSkin(szFilePath, nLoadingFlags);    // Wrap the model in a smart pointer to guard against early-exit memory leaks due to a bad asset
	if (pModelSKIN)
	{
		bool IsLoaded = pModelSKIN->LoadNewSKIN(szFilePath, nLoadingFlags);
		if (IsLoaded == 0)
		{
			delete pModelSKIN;
			return 0;                                      //loading error
		}
		RegisterModelSKIN(pModelSKIN, nLoadingFlags);
		return pModelSKIN;
	}
	return 0;                                          //some error
}

bool CharacterUpr::LoadAndLockResources(tukk szFilePath, u32 nLoadingFlags)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Characters");
	if (szFilePath == 0)
		return false;

	//Note: This initialization is copied from CreateInstance( ), it could be that this pointers haven't been initialized yet at this point?
	g_pIRenderer = g_pISystem->GetIRenderer();
	g_pIPhysicalWorld = g_pISystem->GetIPhysicalWorld();
	g_pI3DEngine = g_pISystem->GetI3DEngine();

	stack_string strFilePath = szFilePath;
	PathUtil::UnifyFilePath(strFilePath);
	tukk strFileExt = PathUtil::GetExt(strFilePath.c_str());

	const bool isCHR = stricmp(strFileExt, DRX_SKEL_FILE_EXT) == 0;
	if (isCHR)
	{
		CDefaultSkeleton* pModelSKEL = FetchModelSKEL(strFilePath, nLoadingFlags);
		if (pModelSKEL)
		{
			pModelSKEL->SetKeepInMemory(true);
			return true;
		}
		return false;                                                         //fail
	}

	const bool isCGA = stricmp(strFileExt, DRX_ANIM_GEOMETRY_FILE_EXT) == 0;
	if (isCGA)
	{
		CDefaultSkeleton* pModelSKEL = FetchModelSKELForGCA(strFilePath, nLoadingFlags);
		if (pModelSKEL)
		{
			pModelSKEL->SetKeepInMemory(true);
			return true;
		}
		return false;                                                         //fail
	}

	const bool isSKIN = stricmp(strFileExt, DRX_SKIN_FILE_EXT) == 0;
	if (isSKIN)
	{
		CSkin* pModelSKIN = FetchModelSKIN(strFilePath, nLoadingFlags);
		if (pModelSKIN)
		{
			pModelSKIN->SetKeepInMemory(true);
			return true;
		}
		return false;                                                         //fail
	}

	const bool isCDF = stricmp(strFileExt, DRX_CHARACTER_DEFINITION_FILE_EXT) == 0;
	if (isCDF)
	{
		i32 cdfId = GetOrLoadCDFId(strFilePath);
		if (cdfId == INVALID_CDF_ID)
			return false;                                                       //fail

		m_arrCacheForCDF[cdfId].m_nKeepInMemory = true;                         //always lock in memory
		tukk strBaseModelFilePath = m_arrCacheForCDF[cdfId].m_strBaseModelFilePath.c_str();
		CDefaultSkeleton* pBaseModelSKEL = FetchModelSKEL(strBaseModelFilePath, nLoadingFlags);
		if (pBaseModelSKEL)
		{
			pBaseModelSKEL->SetKeepInMemory(true);
			bool bKeep = false;
			i32 nKeepInMemory = m_arrCacheForCDF[cdfId].m_nKeepModelsInMemory;  //by default we keep all model-headers in memory
			if (nKeepInMemory)
				bKeep = true;

			u32 numAttachments = m_arrCacheForCDF[cdfId].m_arrAttachments.size();
			for (u32 a = 0; a < numAttachments; a++)
			{
				const CharacterAttachment& attachment = m_arrCacheForCDF[cdfId].m_arrAttachments[a];
				tukk strAttFilePath = attachment.m_strBindingPath.c_str();
				tukk strAttFileExt = PathUtil::GetExt(szFilePath);
				tukk strSimFilePath = attachment.m_strSimBindingPath.c_str();

				const bool isAttSKEL = stricmp(strAttFileExt, DRX_SKEL_FILE_EXT) == 0;
				if (isAttSKEL)
				{
					CDefaultSkeleton* pModelSKEL = FetchModelSKEL(strAttFilePath, nLoadingFlags);
					if (pModelSKEL)
						pModelSKEL->SetKeepInMemory(bKeep);
				}

				TryLoadModelSkin(strAttFilePath, nLoadingFlags, bKeep);
				TryLoadModelSkin(strSimFilePath, nLoadingFlags, bKeep);
			}

			return true; //success
		}
	}

	return false;
}

void CharacterUpr::TryLoadModelSkin(tukk szFilePath, u32 nLoadingFlags, bool bKeep)
{
	tukk strAttFileExt = PathUtil::GetExt(szFilePath);
	const bool isAttSKIN = stricmp(strAttFileExt, DRX_SKIN_FILE_EXT) == 0;
	if (isAttSKIN)
	{
		CSkin* pModelSKIN = FetchModelSKIN(szFilePath, nLoadingFlags);
		if (pModelSKIN)
			pModelSKIN->SetKeepInMemory(bKeep);
	}
}

void CharacterUpr::StreamKeepCharacterResourcesResident(tukk szFilePath, i32 nLod, bool bKeep, bool bUrgent)
{
	tukk fileExt = PathUtil::GetExt(szFilePath);

	i32 const nRefAdj = bKeep ? 1 : -1;

	const bool isSKIN = stricmp(fileExt, DRX_SKIN_FILE_EXT) == 0;
	if (isSKIN)
	{
		CSkin* pSkin = CheckIfModelSKINLoaded(szFilePath, 0);
		if (pSkin != NULL)
		{
			if (!pSkin->m_arrModelMeshes.empty())
			{
				i32 nSkinLod = clamp_tpl(nLod, 0, pSkin->m_arrModelMeshes.size() - 1);
				MeshStreamInfo& msi = pSkin->m_arrModelMeshes[nSkinLod].m_stream;

				msi.nKeepResidentRefs += nRefAdj;
				msi.bIsUrgent = msi.bIsUrgent || bUrgent;
			}
		}
		return;
	}

	const bool isSKEL = stricmp(fileExt, DRX_SKEL_FILE_EXT) == 0;
	const bool isCGA = stricmp(fileExt, DRX_ANIM_GEOMETRY_FILE_EXT) == 0;
	if (isSKEL || isCGA)
	{
		if (CDefaultSkeleton* pSkeleton = CheckIfModelSKELLoaded(szFilePath, 0))
		{
			if (CModelMesh* pModelMesh = pSkeleton->GetModelMesh())
			{
				MeshStreamInfo& msi = pModelMesh->m_stream;
				msi.nKeepResidentRefs += nRefAdj;
				msi.bIsUrgent = msi.bIsUrgent || bUrgent;
			}
		}
		return;
	}

	const bool isCDF = stricmp(fileExt, DRX_CHARACTER_DEFINITION_FILE_EXT) == 0;
	if (isCDF)
	{
		StreamKeepCDFResident(szFilePath, nLod, nRefAdj, bUrgent);
		return;
	}
}

bool CharacterUpr::StreamHasCharacterResources(tukk szFilePath, i32 nLod)
{
	tukk fileExt = PathUtil::GetExt(szFilePath);

	const bool isSKIN = stricmp(fileExt, DRX_SKIN_FILE_EXT) == 0;
	if (isSKIN)
	{
		CSkin* pSkin = CheckIfModelSKINLoaded(szFilePath, 0);
		if (pSkin != NULL)
		{
			if (!pSkin->m_arrModelMeshes.empty())
			{
				i32 nSkinLod = clamp_tpl(nLod, 0, pSkin->m_arrModelMeshes.size() - 1);
				return pSkin->m_arrModelMeshes[nSkinLod].m_pIRenderMesh != NULL;
			}
		}

		return false;
	}

	const bool isSKEL = stricmp(fileExt, DRX_SKEL_FILE_EXT) == 0;
	const bool isCGA = stricmp(fileExt, DRX_ANIM_GEOMETRY_FILE_EXT) == 0;
	if (isSKEL || isCGA)
	{
		if (CDefaultSkeleton* pSkeleton = CheckIfModelSKELLoaded(szFilePath, 0))
		{
			if (CModelMesh* pModelMesh = pSkeleton->GetModelMesh())
			{
				return (pModelMesh->m_pIRenderMesh != nullptr);
			}
		}

		return false;
	}

	const bool isCDF = stricmp(fileExt, DRX_CHARACTER_DEFINITION_FILE_EXT) == 0;
	if (isCDF)
	{
		return StreamKeepCDFResident(szFilePath, nLod, 0, false);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
ICharacterInstance* CharacterUpr::CreateCGAInstance(tukk strPath, u32 nLoadingFlags)
{
	LOADING_TIME_PROFILE_SECTION(g_pISystem);

	uint64 membegin = 0;
	if (Console::GetInst().ca_MemoryUsageLog)
	{
		//char tmp[4096];
		DrxModuleMemoryInfo info;
		DrxGetMemoryInfoForModule(&info);
		membegin = info.allocated - info.freed;
	}

	CDefaultSkeleton* pDefaultSkeleton = (CDefaultSkeleton*)CheckIfModelSKELLoaded(strPath, nLoadingFlags);
	if (pDefaultSkeleton == 0)
	{
		g_pILog->UpdateLoadingScreen("Loading CGA %s", strPath);
		INDENT_LOG_DURING_SCOPE();
		pDefaultSkeleton = FetchModelSKELForGCA(strPath, nLoadingFlags);
	}

	if (pDefaultSkeleton == 0)
		return NULL; // the model has not been loaded

	CCharInstance* pDrxCharInstance = new CCharInstance(strPath, pDefaultSkeleton);
	pDrxCharInstance->m_SkeletonPose.InitCGASkeleton();

	pDrxCharInstance->m_SkeletonPose.UpdateBBox(1);

	if (Console::GetInst().ca_MemoryUsageLog)
	{
		DrxModuleMemoryInfo info;
		DrxGetMemoryInfoForModule(&info);
		g_pILog->UpdateLoadingScreen("InitCGAInstance %s. Memstat %i", strPath, (i32)(info.allocated - info.freed));
		g_AnimStatisticsInfo.m_iInstancesSizes += info.allocated - info.freed - membegin;
		g_pILog->UpdateLoadingScreen("Instances Memstat %i", g_AnimStatisticsInfo.GetInstancesSize());
	}

	return pDrxCharInstance;
}

void CharacterUpr::RegisterModelSKEL(CDefaultSkeleton* pDefaultSkeleton, u32 nLoadingFlags)
{
	LOADING_TIME_PROFILE_SECTION(g_pISystem);
	CDefaultSkeletonReferences dsr;
	dsr.m_pDefaultSkeleton = pDefaultSkeleton;
	u32 size = m_arrModelCacheSKEL.size();
	if ((nLoadingFlags & CA_CharEditModel) == 0)
		m_arrModelCacheSKEL.push_back(dsr);
#ifdef EDITOR_PCDEBUGCODE
	else
		m_arrModelCacheSKEL_CharEdit.push_back(dsr);
#endif
}

// Deletes the given body from the model cache. This is done when there are no instances using this body info.
void CharacterUpr::UnregisterModelSKEL(CDefaultSkeleton* pDefaultSkeleton)
{
	u32 numModels1 = m_arrModelCacheSKEL.size();
	for (u32 m = 0; m < numModels1; m++)
	{
		if (m_arrModelCacheSKEL[m].m_pDefaultSkeleton == pDefaultSkeleton)
		{
			for (u32 e = m; e < (numModels1 - 1); e++)
				m_arrModelCacheSKEL[e] = m_arrModelCacheSKEL[e + 1];
			m_arrModelCacheSKEL.pop_back();
			break;
		}
	}

#ifdef EDITOR_PCDEBUGCODE
	u32 numModels2 = m_arrModelCacheSKEL_CharEdit.size();
	for (u32 m = 0; m < numModels2; m++)
	{
		if (m_arrModelCacheSKEL_CharEdit[m].m_pDefaultSkeleton == pDefaultSkeleton)
		{
			for (u32 e = m; e < (numModels2 - 1); e++)
				m_arrModelCacheSKEL_CharEdit[e] = m_arrModelCacheSKEL_CharEdit[e + 1];
			m_arrModelCacheSKEL_CharEdit.pop_back();
			break;
		}
	}
#endif
}

void CharacterUpr::RegisterInstanceSkel(CDefaultSkeleton* pDefaultSkeleton, CCharInstance* pInstance)
{
	u32 numModelCacheSKEL = m_arrModelCacheSKEL.size();
	for (u32 i = 0; i < numModelCacheSKEL; i++)
	{
		if (m_arrModelCacheSKEL[i].m_pDefaultSkeleton == pDefaultSkeleton)
			stl::push_back_unique(m_arrModelCacheSKEL[i].m_RefByInstances, pInstance), pDefaultSkeleton->SetInstanceCounter(m_arrModelCacheSKEL[i].m_RefByInstances.size());
	}
#ifdef EDITOR_PCDEBUGCODE
	u32 numModelCacheSKEL_CharEdit = m_arrModelCacheSKEL_CharEdit.size();
	for (u32 i = 0; i < numModelCacheSKEL_CharEdit; i++)
	{
		if (m_arrModelCacheSKEL_CharEdit[i].m_pDefaultSkeleton == pDefaultSkeleton)
			stl::push_back_unique(m_arrModelCacheSKEL_CharEdit[i].m_RefByInstances, pInstance), pDefaultSkeleton->SetInstanceCounter(m_arrModelCacheSKEL_CharEdit[i].m_RefByInstances.size());
	}
#endif
}

void CharacterUpr::UnregisterInstanceSkel(CDefaultSkeleton* pDefaultSkeleton, CCharInstance* pInstance)
{
	u32 numModelCacheSKEL = m_arrModelCacheSKEL.size();
	for (u32 i = 0; i < numModelCacheSKEL; i++)
	{
		if (m_arrModelCacheSKEL[i].m_pDefaultSkeleton == pDefaultSkeleton)
			stl::find_and_erase(m_arrModelCacheSKEL[i].m_RefByInstances, pInstance), pDefaultSkeleton->SetInstanceCounter(m_arrModelCacheSKEL[i].m_RefByInstances.size());
	}
#ifdef EDITOR_PCDEBUGCODE
	u32 numModelCacheSKEL_CharEdit = m_arrModelCacheSKEL_CharEdit.size();
	for (u32 i = 0; i < numModelCacheSKEL_CharEdit; i++)
	{
		if (m_arrModelCacheSKEL_CharEdit[i].m_pDefaultSkeleton == pDefaultSkeleton)
			stl::find_and_erase(m_arrModelCacheSKEL_CharEdit[i].m_RefByInstances, pInstance), pDefaultSkeleton->SetInstanceCounter(m_arrModelCacheSKEL_CharEdit[i].m_RefByInstances.size());
	}
#endif
}

u32 CharacterUpr::GetNumInstancesPerModel(const IDefaultSkeleton& rIDefaultSkeleton) const
{
	u32 numDefaultSkeletons = m_arrModelCacheSKEL.size();
	for (u32 m = 0; m < numDefaultSkeletons; m++)
	{
		if (m_arrModelCacheSKEL[m].m_pDefaultSkeleton == &rIDefaultSkeleton)
			return m_arrModelCacheSKEL[m].m_RefByInstances.size();
	}
	return 0;
}
ICharacterInstance* CharacterUpr::GetICharInstanceFromModel(const IDefaultSkeleton& rIDefaultSkeleton, u32 num) const
{
	u32 numDefaultSkeletons = m_arrModelCacheSKEL.size();
	for (u32 m = 0; m < numDefaultSkeletons; m++)
	{
		if (m_arrModelCacheSKEL[m].m_pDefaultSkeleton == &rIDefaultSkeleton)
		{
			u32 numInstance = m_arrModelCacheSKEL[m].m_RefByInstances.size();
			return (num >= numInstance) ? 0 : m_arrModelCacheSKEL[m].m_RefByInstances[num];
		}
	}
	return 0;
}

//---------------------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------------------
void CharacterUpr::RegisterModelSKIN(CSkin* pDefaultSkinning, u32 nLoadingFlags)
{
	LOADING_TIME_PROFILE_SECTION(g_pISystem);
	CDefaultSkinningReferences dsr;
	dsr.m_pDefaultSkinning = pDefaultSkinning;
	u32 size = m_arrModelCacheSKIN.size();
	if ((nLoadingFlags & CA_CharEditModel) == 0)
		m_arrModelCacheSKIN.push_back(dsr);
#ifdef EDITOR_PCDEBUGCODE
	else
		m_arrModelCacheSKIN_CharEdit.push_back(dsr);
#endif
}
// Deletes the given body from the model cache. This is done when there are no instances using this body info.
void CharacterUpr::UnregisterModelSKIN(CSkin* pDefaultSkeleton)
{
	u32 numModels1 = m_arrModelCacheSKIN.size();
	for (u32 m = 0; m < numModels1; m++)
	{
		if (m_arrModelCacheSKIN[m].m_pDefaultSkinning == pDefaultSkeleton)
		{
			for (u32 e = m; e < (numModels1 - 1); e++)
				m_arrModelCacheSKIN[e] = m_arrModelCacheSKIN[e + 1];
			m_arrModelCacheSKIN.pop_back();
			break;
		}
	}

#ifdef EDITOR_PCDEBUGCODE
	u32 numModels2 = m_arrModelCacheSKIN_CharEdit.size();
	for (u32 m = 0; m < numModels2; m++)
	{
		if (m_arrModelCacheSKIN_CharEdit[m].m_pDefaultSkinning == pDefaultSkeleton)
		{
			for (u32 e = m; e < (numModels2 - 1); e++)
				m_arrModelCacheSKIN_CharEdit[e] = m_arrModelCacheSKIN_CharEdit[e + 1];
			m_arrModelCacheSKIN_CharEdit.pop_back();
			break;
		}
	}
#endif
}

void CharacterUpr::RegisterInstanceSkin(CSkin* pDefaultSkinning, CAttachmentSKIN* pInstance)
{
	u32 numModelCacheSKIN = m_arrModelCacheSKIN.size();
	for (u32 i = 0; i < numModelCacheSKIN; i++)
	{
		if (m_arrModelCacheSKIN[i].m_pDefaultSkinning == pDefaultSkinning)
			stl::push_back_unique(m_arrModelCacheSKIN[i].m_RefByInstances, pInstance);
	}
#ifdef EDITOR_PCDEBUGCODE
	u32 numModelCacheSKIN_CharEdit = m_arrModelCacheSKIN_CharEdit.size();
	for (u32 i = 0; i < numModelCacheSKIN_CharEdit; i++)
	{
		if (m_arrModelCacheSKIN_CharEdit[i].m_pDefaultSkinning == pDefaultSkinning)
			stl::push_back_unique(m_arrModelCacheSKIN_CharEdit[i].m_RefByInstances, pInstance);
	}
#endif
}

void CharacterUpr::UnregisterInstanceSkin(CSkin* pDefaultSkinning, CAttachmentSKIN* pInstance)
{
	u32 numModelCacheSKIN = m_arrModelCacheSKIN.size();
	for (u32 i = 0; i < numModelCacheSKIN; i++)
	{
		if (m_arrModelCacheSKIN[i].m_pDefaultSkinning == pDefaultSkinning)
			stl::find_and_erase(m_arrModelCacheSKIN[i].m_RefByInstances, pInstance);
	}
#ifdef EDITOR_PCDEBUGCODE
	u32 numModelCacheSKIN_CharEdit = m_arrModelCacheSKIN_CharEdit.size();
	for (u32 i = 0; i < numModelCacheSKIN_CharEdit; i++)
	{
		if (m_arrModelCacheSKIN_CharEdit[i].m_pDefaultSkinning == pDefaultSkinning)
			stl::find_and_erase(m_arrModelCacheSKIN_CharEdit[i].m_RefByInstances, pInstance);
	}
#endif
}

void CharacterUpr::RegisterInstanceVCloth(CSkin* pDefaultSkinning, CAttachmentVCLOTH* pInstance)
{
	u32 numModelCacheSKIN = m_arrModelCacheSKIN.size();
	for (u32 i = 0; i < numModelCacheSKIN; i++)
	{
		if (m_arrModelCacheSKIN[i].m_pDefaultSkinning == pDefaultSkinning)
			stl::push_back_unique(m_arrModelCacheSKIN[i].m_RefByInstancesVCloth, pInstance);
	}
#ifdef EDITOR_PCDEBUGCODE
	u32 numModelCacheSKIN_CharEdit = m_arrModelCacheSKIN_CharEdit.size();
	for (u32 i = 0; i < numModelCacheSKIN_CharEdit; i++)
	{
		if (m_arrModelCacheSKIN_CharEdit[i].m_pDefaultSkinning == pDefaultSkinning)
			stl::push_back_unique(m_arrModelCacheSKIN_CharEdit[i].m_RefByInstancesVCloth, pInstance);
	}
#endif
}

void CharacterUpr::UnregisterInstanceVCloth(CSkin* pDefaultSkinning, CAttachmentVCLOTH* pInstance)
{
	u32 numModelCacheSKIN = m_arrModelCacheSKIN.size();
	for (u32 i = 0; i < numModelCacheSKIN; i++)
	{
		if (m_arrModelCacheSKIN[i].m_pDefaultSkinning == pDefaultSkinning)
			stl::find_and_erase(m_arrModelCacheSKIN[i].m_RefByInstancesVCloth, pInstance);
	}
#ifdef EDITOR_PCDEBUGCODE
	u32 numModelCacheSKIN_CharEdit = m_arrModelCacheSKIN_CharEdit.size();
	for (u32 i = 0; i < numModelCacheSKIN_CharEdit; i++)
	{
		if (m_arrModelCacheSKIN_CharEdit[i].m_pDefaultSkinning == pDefaultSkinning)
			stl::find_and_erase(m_arrModelCacheSKIN_CharEdit[i].m_RefByInstancesVCloth, pInstance);
	}
#endif
}

// find wrapping projection for render mesh to simulationmesh
static inline void WrapRenderVertexToSimMesh(
  mesh_data const& simMesh,
  std::vector<std::vector<i32>> const& simAdjTris,
  Vec3 const& renderVtxPos,
  SSkinMapEntry& renderVtxMapOut)
{
	// search closest particle in simMesh
	i32 closestIdx = -1;
	float closestDistance2 = FLT_MAX;
	for (i32 i = 0; i < simMesh.nVertices; i++)
	{
		Vec3 delta = renderVtxPos - simMesh.pVertices[i];
		float len2 = delta.len2();
		if (len2 < closestDistance2) { closestIdx = i; closestDistance2 = len2; }
	}

	// if render particle is very close to sim particle, then use direct mapping to that position
	const float threshold2 = 0.001f * 0.001f;
	if (closestDistance2 < threshold2)
	{
		renderVtxMapOut.iMap = closestIdx;
		renderVtxMapOut.s = 0.f;
		renderVtxMapOut.t = 0.f;
		renderVtxMapOut.h = 0.f;
		return;
	}

	// search triangle with positive uv - this is the ideal case
	// go through all the adjacent triangles and find the best fit
	for (size_t j = 0; j < simAdjTris[closestIdx].size(); j++)
	{
		i32k tri = simAdjTris[closestIdx][j];

		// get triangles side/neighbor vertices to closest point
		i32 i2 = 0;
		for (i32 k = 0; k < 3; k++)
		{
			i32 idx = simMesh.pIndices[tri * 3 + k];
			if (idx == closestIdx)
				i2 = k;
		}
		i32k idx0 = simMesh.pIndices[tri * 3 + inc_mod3[i2]];
		i32k idx1 = simMesh.pIndices[tri * 3 + dec_mod3[i2]];

		// barycentric and distance to render mesh
		float s, t, h;
		const Vec3 u = simMesh.pVertices[idx0] - simMesh.pVertices[closestIdx];
		const Vec3 v = simMesh.pVertices[idx1] - simMesh.pVertices[closestIdx];
		Vec3 w = renderVtxPos - simMesh.pVertices[closestIdx];
		Vec3 n = (u ^ v).normalized();
		h = w * n;
		w -= h * n;
		const float d00 = u * u;
		const float d01 = u * v;
		const float d11 = v * v;
		const float d20 = w * u;
		const float d21 = w * v;
		const float denom = d00 * d11 - d01 * d01;
		s = (d11 * d20 - d01 * d21) / denom;
		t = (d00 * d21 - d01 * d20) / denom;

		// if s,t are positive, keep this value, as it is a good mapping
		// if either s or t is < 0 - set this only, if no other tri (e.g., with positive uv) has been found, not ideal but in worst case better than nothing...
		if ((s >= 0 && t >= 0) || (renderVtxMapOut.iTri < 0))
		{
			renderVtxMapOut.iMap = i2;
			renderVtxMapOut.iTri = tri;
			renderVtxMapOut.s = s;
			renderVtxMapOut.t = t;
			renderVtxMapOut.h = h;
		}
	}
}

SClothGeometry* CharacterUpr::LoadVClothGeometry(const CAttachmentVCLOTH& pAttachementVCloth, _smart_ptr<IRenderMesh> pRenderMeshes[])
{
	assert(pAttachementVCloth.GetClothCacheKey() > 0);
	SClothGeometry& ret = m_clothGeometries[pAttachementVCloth.GetClothCacheKey()];
	if (ret.weldMap != NULL)
	{
		ret.AllocateBuffer();
		return &ret;
	}

	IRenderMesh* pSimRenderMesh = pAttachementVCloth.m_pSimSkin->GetIRenderMesh(0);
	ret.nVtx = pSimRenderMesh->GetVerticesCount();
	i32 nSimIndices = pSimRenderMesh->GetIndicesCount();

	pSimRenderMesh->LockForThreadAccess();
	strided_pointer<Vec3> pVertices;
	pVertices.data = (Vec3*)pSimRenderMesh->GetPosPtr(pVertices.iStride, FSL_READ);
	vtx_idx* pSimIndices = pSimRenderMesh->GetIndexPtr(FSL_READ);
	strided_pointer<ColorB> pColors(0);
	pColors.data = (ColorB*)pSimRenderMesh->GetColorPtr(pColors.iStride, FSL_READ);

	// look for welded vertices and prune them
	i32 nWelded = 0;
	std::vector<Vec3> unweldedVerts;
	unweldedVerts.reserve(ret.nVtx);
	if (!ret.weldMap)
		ret.weldMap = new vtx_idx[ret.nVtx];
	// TODO: faster welded vertices detector - this is O(n^2)
	for (i32 i = 0; i < ret.nVtx; i++)
	{
		i32 iMap = -1;
		for (i32 j = i - 1; j >= 0; j--)
		{
			Vec3 delta = pVertices[i] - pVertices[j];
			if (delta.len() < 0.001f)
			{
				iMap = ret.weldMap[j];
				nWelded++;
				break;
			}
		}
		if (iMap >= 0)
			ret.weldMap[i] = iMap;
		else
		{
			ret.weldMap[i] = unweldedVerts.size();
			unweldedVerts.push_back(pVertices[i]);
		}
	}
	if (nWelded)
		DrxLog("[Character Cloth] Found %d welded vertices out of %d in attachment %s", nWelded, ret.nVtx, pAttachementVCloth.GetName());
	// reconstruct indices
	vtx_idx* unweldedIndices = new vtx_idx[nSimIndices];
	ret.nUniqueVtx = unweldedVerts.size();
	for (i32 i = 0; i < nSimIndices; i++)
		unweldedIndices[i] = ret.weldMap[pSimIndices[i]];

	// create the physics geometry (CTriMesh)
	IGeometry* pSimPhysMesh = g_pIPhysicalWorld->GetGeomUpr()->CreateMesh(&unweldedVerts[0], unweldedIndices, 0, 0, nSimIndices / 3, 0);

	delete[] unweldedIndices;

	// register phys geometry
	ret.pPhysGeom = g_pIPhysicalWorld->GetGeomUpr()->RegisterGeometry(pSimPhysMesh);
	pSimPhysMesh->Release();

	// compute blending weights from vertex colors
	ret.weights = new float[ret.nUniqueVtx];
	for (i32 i = 0; i < ret.nVtx; i++)
	{
		float alpha = 1.f - (float)pColors[i].r / 255.f;
		ret.weights[ret.weldMap[i]] = alpha;
	}

	pSimRenderMesh->UnlockStream(VSF_TANGENTS);
	pSimRenderMesh->UnlockStream(VSF_HWSKIN_INFO);
	pSimRenderMesh->UnlockStream(VSF_GENERAL);
	pSimRenderMesh->UnLockForThreadAccess();

	// build adjacent triangles list
	mesh_data* md = (mesh_data*)pSimPhysMesh->GetData();
	std::vector<std::vector<i32>> adjTris(md->nVertices);
	for (i32 i = 0; i < md->nTris; i++)
	{
		for (i32 j = 0; j < 3; j++)
		{
			i32 idx = md->pIndices[i * 3 + j];
			adjTris[idx].push_back(i);
		}
	}

	ret.maxVertices = 0;
	for (i32 lod = 0; lod < SClothGeometry::MAX_LODS; lod++)
	{
		IRenderMesh* pRenderMesh = pRenderMeshes[lod];
		if (!pRenderMesh)
			continue;

		i32 nVtx = pRenderMesh->GetVerticesCount();
		ret.maxVertices = max(ret.maxVertices, nVtx);
		i32 nIndices = pRenderMesh->GetIndicesCount();
		ret.numIndices[lod] = nIndices;
		i32 nTris = nIndices / 3;

		pRenderMesh->LockForThreadAccess();
		vtx_idx* pIndices = pRenderMesh->GetIndexPtr(FSL_READ);
		ret.pIndices[lod] = new vtx_idx[nIndices];
		memcpy(ret.pIndices[lod], pIndices, nIndices * sizeof(vtx_idx));

		strided_pointer<Vec3> pVtx;
		pVtx.data = (Vec3*)pRenderMesh->GetPosPtr(pVtx.iStride, FSL_READ);
		strided_pointer<Vec2> pUVs;
		pUVs.data = (Vec2*)pRenderMesh->GetUVPtr(pUVs.iStride, FSL_READ);

		SSkinMapEntry* skinMap = new SSkinMapEntry[nVtx];
		ret.skinMap[lod] = skinMap;
		i32 numUnmapped = 0;
		for (i32 i = 0; i < nVtx; i++)
		{
			// init as 'no skinning/triangle-mapping found', this is used below to detect the number of unmapped vertices
			skinMap[i].iTri = -1;

			// wrap render mesh to sim mesh
			mesh_data const& simMesh = *md;
			std::vector<std::vector<i32>> const& simAdjTris = adjTris;
			Vec3 const& renderVtx = pVtx[i];
			SSkinMapEntry& renderVtxMap = skinMap[i];
			WrapRenderVertexToSimMesh(simMesh, simAdjTris, renderVtx, renderVtxMap); // determine best wrapping in 'renderVtxMap', i.e. skinMap[i]

			if (skinMap[i].iTri < 0)
			{
				numUnmapped++;
			}
		}
		if (numUnmapped)
			DrxLog("[Character cloth] Unmapped vertices: %d", numUnmapped);

		// prepare tangent data
		ret.tangentData[lod] = new STangentData[nTris];
		for (i32 i = 0; i < nTris; i++)
		{
			i32 base = i * 3;
			i32 i1 = pIndices[base++];
			i32 i2 = pIndices[base++];
			i32 i3 = pIndices[base];

			const Vec3& v1 = pVtx[i1];
			const Vec3& v2 = pVtx[i2];
			const Vec3& v3 = pVtx[i3];

			Vec3 u = v2 - v1;
			Vec3 v = v3 - v1;

			float w1x = pUVs[i1].x;
			float w1y = pUVs[i1].y;
			float w2x = pUVs[i2].x;
			float w2y = pUVs[i2].y;
			float w3x = pUVs[i3].x;
			float w3y = pUVs[i3].y;

			float s1 = w2x - w1x;
			float s2 = w3x - w1x;
			float t1 = w2y - w1y;
			float t2 = w3y - w1y;

			const float denom = s1 * t2 - s2 * t1;
			float r = fabsf(denom) > FLT_EPSILON ? 1.0f / denom : 1.0f;

			ret.tangentData[lod][i].t1 = t1;
			ret.tangentData[lod][i].t2 = t2;
			ret.tangentData[lod][i].r = r;
		}

		pRenderMesh->UnlockStream(VSF_GENERAL);
		pRenderMesh->UnLockForThreadAccess();
	}

	// allocate working buffers
	ret.AllocateBuffer();

	return &ret;
}

#ifdef EDITOR_PCDEBUGCODE
void CharacterUpr::ClearAllKeepInMemFlags()
{
	u32 numModelCacheSKEL_CharEdit = m_arrModelCacheSKEL_CharEdit.size();
	for (u32 i = 0; i < numModelCacheSKEL_CharEdit; i++)
		m_arrModelCacheSKEL_CharEdit[i].m_pDefaultSkeleton->SetKeepInMemory(false);

	u32 numModelCacheSKIN_CharEdit = m_arrModelCacheSKIN_CharEdit.size();
	for (u32 i = 0; i < numModelCacheSKIN_CharEdit; i++)
		m_arrModelCacheSKIN_CharEdit[i].m_pDefaultSkinning->SetKeepInMemory(false);

	for (i32 i = (numModelCacheSKEL_CharEdit - 1); i > -1; i--)
		m_arrModelCacheSKEL_CharEdit[i].m_pDefaultSkeleton->DeleteIfNotReferenced();
	for (i32 i = (numModelCacheSKIN_CharEdit - 1); i > -1; i--)
		m_arrModelCacheSKIN_CharEdit[i].m_pDefaultSkinning->DeleteIfNotReferenced();
}
#endif

//////////////////////////////////////////////////////////////////////////

// Deletes itself
void CharacterUpr::Release()
{
	delete this;
}

//////////////////////////////////////////////////////////////////////////
IFacialAnimation* CharacterUpr::GetIFacialAnimation()
{
	return m_pFacialAnimation;
}

//////////////////////////////////////////////////////////////////////////
const IFacialAnimation* CharacterUpr::GetIFacialAnimation() const
{
	return m_pFacialAnimation;
}

const IAttachmentMerger& CharacterUpr::GetIAttachmentMerger() const
{
	return CAttachmentMerger::Instance();
}

// returns statistics about this instance of character animation manager
// don't call this too frequently
void CharacterUpr::GetStatistics(Statistics& rStats) const
{
	memset(&rStats, 0, sizeof(rStats));
	rStats.numCharModels = m_arrModelCacheSKEL.size();
	for (u32 m = 0; m < rStats.numCharModels; m++)
		rStats.numCharacters += m_arrModelCacheSKEL[m].m_RefByInstances.size();
}

// puts the size of the whole subsystem into this sizer object, classified,
// according to the flags set in the sizer
void CharacterUpr::GetMemoryUsage(class IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_AnimationUpr);
	pSizer->AddObject(m_arrCacheForCDF);
	pSizer->AddObject(g_AnimationUpr);
	{
		SIZER_SUBCOMPONENT_NAME(pSizer, "Facial Animation");
		pSizer->AddObject(m_pFacialAnimation);
	}
	{
		SIZER_SUBCOMPONENT_NAME(pSizer, "Model Data");
		pSizer->AddObject(m_arrModelCacheSKEL);
	}

	GetCharacterInstancesSize(pSizer);

#ifndef _LIB // Only when compiling as dynamic library
	{
		//SIZER_COMPONENT_NAME(pSizer,"Strings");
		//pSizer->AddObject( (this+1),string::_usedMemory(0) );
	}
	{
		SIZER_COMPONENT_NAME(pSizer, "STL Allocator Waste");
		DrxModuleMemoryInfo meminfo;
		ZeroStruct(meminfo);
		DrxGetMemoryInfoForModule(&meminfo);
		pSizer->AddObject((this + 2), (u32)meminfo.STL_wasted);
	}
#endif

	{
		SIZER_SUBCOMPONENT_NAME(pSizer, "Animation Context");
		pSizer->AddObject(CAnimationContext::Instance());
	}
}

void CharacterUpr::GetModelCacheSize() const
{
	u32 numModels = m_arrModelCacheSKEL.size();
	for (u32 i = 0; i < numModels; i++)
		m_arrModelCacheSKEL[i].m_pDefaultSkeleton->SizeOfDefaultSkeleton();
}

void CharacterUpr::GetCharacterInstancesSize(class IDrxSizer* pSizer) const
{
	SIZER_SUBCOMPONENT_NAME(pSizer, "Character instances");

	u32 nSize = 0;
	u32 numModelCache = m_arrModelCacheSKEL.size();
	for (u32 m = 0; m < numModelCache; ++m)
	{
		u32 numInstances = m_arrModelCacheSKEL[m].m_RefByInstances.size();
		for (u32 i = 0; i < numInstances; i++)
			pSizer->AddObject(m_arrModelCacheSKEL[m].m_RefByInstances[i]);
	}

}

//! Cleans up all resources - currently deletes all bodies and characters (even if there are references on them)
void CharacterUpr::ClearResources(bool bForceCleanup)
{
	ClearPoseModifiersFromSynchQueue();
	SyncAllAnimations();
	StopAnimationsOnAllInstances();

	// clean animations cache and statistics
	if (m_pFacialAnimation)
		m_pFacialAnimation->ClearAllCaches();

	if (!gEnv->IsEditor())
		CleanupModelCache(bForceCleanup);
	GetAnimationUpr().Unload_All_Animation();
}

//////////////////////////////////////////////////////////////////////////
// Deletes all the cached bodies and their associated character instances
void CharacterUpr::CleanupModelCache(bool bForceCleanup)
{

#if PLACEHOLDER_CHARACTER_DEBUG_ENABLE
	DrxLogAlways("CharacterUpr:CleanModelCache - Placeholder objects '%d'", CPlaceholderCharacter::GetAllocatedObjects());
#endif

	u32 numSKIN1 = m_arrModelCacheSKIN.size();
	for (u32 i = 0; i < numSKIN1; i++)
		m_arrModelCacheSKIN[i].m_pDefaultSkinning->SetKeepInMemory(true);   // Make sure nothing gets deleted.

	i32 numSKIN2 = m_arrModelCacheSKIN.size();                            // Clean all instances.
	for (i32 s = (numSKIN2 - 1); s > -1; s--)
	{
		i32 numInstances = m_arrModelCacheSKIN[s].m_RefByInstances.size();
		if (numInstances)
			g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, 0, "CharacterUpr.CleanupSkinInstances", "Forcing deletion of %d instances for body %s. CRASH POSSIBLE because other subsystems may have stored dangling pointer(s).", m_arrModelCacheSKIN[s].m_pDefaultSkinning->GetRefCounter(), m_arrModelCacheSKIN[s].m_pDefaultSkinning->GetModelFilePath());
		for (i32 i = (numInstances - 1); i > -1; i--)
			delete m_arrModelCacheSKIN[s].m_RefByInstances[i];

		i32 numVClothInstances = m_arrModelCacheSKIN[s].m_RefByInstancesVCloth.size();
		if (numVClothInstances)
			g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, 0, "CharacterUpr.CleanupVClothInstances", "Forcing deletion of %d instances for body %s. CRASH POSSIBLE because other subsystems may have stored dangling pointer(s).", m_arrModelCacheSKIN[s].m_pDefaultSkinning->GetRefCounter(), m_arrModelCacheSKIN[s].m_pDefaultSkinning->GetModelFilePath());
		for (i32 i = (numVClothInstances - 1); i > -1; i--)
			delete m_arrModelCacheSKIN[s].m_RefByInstancesVCloth[i];
	}

	u32 numSKIN3 = m_arrModelCacheSKIN.size();                           //count backwards, because the array is decreased after each delete.
	for (i32 i = (numSKIN3 - 1); i > -1; i--)
		m_arrModelCacheSKIN[i].m_pDefaultSkinning->DeleteIfNotReferenced(); //even if locked in memory

	u32 numSKIN4 = m_arrModelCacheSKIN.size();                           //if we still have instances, then something went wrong
	if (numSKIN4)
	{
		for (u32 i = 0; i < numSKIN4; i++)
			DrxLogAlways("m_arrModelCacheSKIN[%i] = %s", i, m_arrModelCacheSKIN[i].m_pDefaultSkinning->GetModelFilePath());
		DrxFatalError("m_arrModelCacheSKIN is supposed to be empty, but there are still %d CSkins in memory", numSKIN4);
	}

	//----------------------------------------------------------------------------------------------------------------------

	u32 numSKEL1 = m_arrModelCacheSKEL.size();
	for (u32 i = 0; i < numSKEL1; i++)
		m_arrModelCacheSKEL[i].m_pDefaultSkeleton->SetKeepInMemory(true);   // Make sure nothing gets deleted.

	i32 numSKEL2 = m_arrModelCacheSKEL.size();                            // Clean all instances.
	for (i32 s = (numSKEL2 - 1); s > -1; s--)
	{
		i32 numInstances = m_arrModelCacheSKEL[s].m_RefByInstances.size();
		if (numInstances)
			g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, 0, "CharacterUpr.CleanupInstances", "Forcing deletion of %d instances for body %s. CRASH POSSIBLE because other subsystems may have stored dangling pointer(s).", m_arrModelCacheSKEL[s].m_pDefaultSkeleton->GetRefCounter(), m_arrModelCacheSKEL[s].m_pDefaultSkeleton->m_strFilePath.c_str());
		for (i32 i = (numInstances - 1); i > -1; i--)
			delete m_arrModelCacheSKEL[s].m_RefByInstances[i];
	}

	u32 numSKEL3 = m_arrModelCacheSKEL.size();                           //count backwards, because the array is decreased after each delete.
	for (i32 i = (numSKEL3 - 1); i > -1; i--)
		m_arrModelCacheSKEL[i].m_pDefaultSkeleton->DeleteIfNotReferenced(); //even if locked in memory

	u32 numSKEL4 = m_arrModelCacheSKEL.size();                           //if we still have instances, then something went wrong
	if (numSKEL4)
	{
		for (u32 i = 0; i < numSKEL4; i++)
			DrxLogAlways("m_arrModelCacheSKEL[%i] = %s", i, m_arrModelCacheSKEL[i].m_pDefaultSkeleton->GetModelFilePath());
		DrxFatalError("m_arrModelCacheSKEL is supposed to be empty, but there are still %d CSkels in memory", numSKEL4);
	}

	DrxCHRLoader::ClearCachedLodSkeletonResults();

	if (bForceCleanup)
	{
		m_arrCacheForCDF.clear();
		for (TClothGeomCache::iterator it = m_clothGeometries.begin(); it != m_clothGeometries.end(); ++it)
		{
			it->second.Cleanup();
		}
	}

	CFacialModel::ClearResources();
}

//------------------------------------------------------------------------
//--  average frame-times to avoid stalls and peaks in framerate
//------------------------------------------------------------------------
f32 CharacterUpr::GetAverageFrameTime(f32 sec, f32 FrameTime, f32 fTimeScale, f32 LastAverageFrameTime)
{
	u32 numFT = m_arrFrameTimes.size();
	for (i32 i = (numFT - 2); i > -1; i--)
		m_arrFrameTimes[i + 1] = m_arrFrameTimes[i];

	m_arrFrameTimes[0] = FrameTime;

	//get smoothed frame
	u32 FrameAmount = 1;
	if (LastAverageFrameTime)
	{
		FrameAmount = u32(sec / LastAverageFrameTime * fTimeScale + 0.5f);         //average the frame-times for a certain time-period (sec)
		if (FrameAmount > numFT)  FrameAmount = numFT;
		if (FrameAmount < 1)  FrameAmount = 1;
	}

	f32 AverageFrameTime = 0;
	for (u32 i = 0; i < FrameAmount; i++)
		AverageFrameTime += m_arrFrameTimes[i];
	AverageFrameTime /= FrameAmount;

	//don't smooth if we pase the game
	if (FrameTime < 0.0001f)
		AverageFrameTime = FrameTime;

	//	g_YLine+=66.0f;
	//	float fColor[4] = {1,0,1,1};
	//	g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"AverageFrameTime:  Frames:%d  FrameTime:%f  AverageTime:%f", FrameAmount, FrameTime, AverageFrameTime);
	//	g_YLine+=16.0f;

	return AverageFrameTime;
}

u32 CharacterUpr::GetRendererThreadId()
{
	bool bIsMultithreadedRenderer = false;
	g_pIRenderer->EF_Query(EFQ_RenderMultithreaded, bIsMultithreadedRenderer);
	if (bIsMultithreadedRenderer)
	{
		static u32 frameId = 0;
		if (!s_bPaused)
			frameId = (u32)g_pIRenderer->GetFrameID(false);
		return frameId;
	}

	return s_renderFrameIdLocal;
}

u32 CharacterUpr::GetRendererMainThreadId()
{
	bool bIsMultithreadedRenderer = false;
	g_pIRenderer->EF_Query(EFQ_RenderMultithreaded, bIsMultithreadedRenderer);
	if (bIsMultithreadedRenderer)
	{
		static u32 frameId = 0;
		if (!s_bPaused)
			frameId = (u32)g_pIRenderer->GetFrameID(false);
		return frameId;
	}

	return s_renderFrameIdLocal;
}

void CharacterUpr::UpdateRendererFrame()
{
	if (!s_bPaused)
		s_renderFrameIdLocal++;
}

// should be called every frame
void CharacterUpr::Update(bool bPaused)
{
	s_bPaused = bPaused;
	DRX_PROFILE_REGION(PROFILE_ANIMATION, "CharacterUpr::Update");
	DRXPROFILE_SCOPE_PROFILE_MARKER("CharacterUpr::Update");
	ANIMATION_LIGHT_PROFILER();

	CAnimationContext::Instance().Update();

	m_nUpdateCounter++;

	//update interfaces every frame
	g_YLine = 16.0f;
	g_pIRenderer = g_pISystem->GetIRenderer();
	g_pIPhysicalWorld = g_pISystem->GetIPhysicalWorld();
	g_pI3DEngine = g_pISystem->GetI3DEngine();
	g_bProfilerOn = g_pISystem->GetIProfileSystem()->IsProfiling();

	bool bIsMultithreadedRenderer = false;
	if (g_pIRenderer)
		g_pIRenderer->EF_Query(EFQ_RenderMultithreaded, bIsMultithreadedRenderer);

	if (!bIsMultithreadedRenderer && !s_bPaused)
		s_renderFrameIdLocal++;

	g_fCurrTime = g_pITimer->GetCurrTime();

	if (m_InitializedByIMG == 0)
	{
		//This initialization must happen as early as possible. In the ideal case we should do it as soon as we initialize DinrusXAnimation.
		//Doing it when we fetch the CHR is to late, because it will conflict with manually loaded DBAs (MP can load DBA and lock DBAs without a CHR)
		//LoadAnimationImageFile( "animations/animations.img","animations/DirectionalBlends.img" );
	}

	f32 fTimeScale = g_pITimer->GetTimeScale();
	f32 fFrameTime = g_pITimer->GetFrameTime();
	if (fFrameTime > 0.2f) fFrameTime = 0.2f;
	if (fFrameTime < 0.0f) fFrameTime = 0.0f;
	g_AverageFrameTime = GetAverageFrameTime(0.25f, fFrameTime, fTimeScale, g_AverageFrameTime);

	CVertexAnimation::ClearSoftwareRenderMeshes();

	/*
	   #ifndef _RELEASE
	   u32 num=g_DataMismatch.size();
	   for (u32 i=0; i<num; i++)
	   {
	    float fColor[4] = {0,1,1,1};
	    g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.4f, fColor, false,"FatalError: Data-Mismatch for Streaming File %s",g_DataMismatch[i].c_str() );
	    g_YLine+=14.0f;
	   }
	   #endif
	 */


	UpdateInstances(bPaused);

	if (Console::GetInst().ca_DebugModelCache)
	{
		float fWhite[4] = { 1, 1, 1, 1 };
#ifdef EDITOR_PCDEBUGCODE
		u32 numMCSkelCE = m_arrModelCacheSKEL_CharEdit.size();
		u32 numMCSkinCE = m_arrModelCacheSKIN_CharEdit.size();
		if (numMCSkelCE + numMCSkinCE)
		{
			g_pAuxGeom->Draw2dLabel(1, g_YLine, 2.0f, fWhite, false, "arrModelCache_CharEdit: %4d", numMCSkelCE), g_YLine += 20.0f;
			DebugModelCache(1, m_arrModelCacheSKEL_CharEdit, m_arrModelCacheSKIN_CharEdit);
			g_pAuxGeom->Draw2dLabel(1, g_YLine, 2.0f, fWhite, false, "-----------------------------"), g_YLine += 20.0f;
		}
#endif
		DebugModelCache(1, m_arrModelCacheSKEL, m_arrModelCacheSKIN);
#ifndef _RELEASE
		Console::GetInst().ca_DebugModelCache &= 0x17;   //disable logging to console
#endif
	}
	if (Console::GetInst().ca_ReloadAllCHRPARAMS)
		ReloadAllCHRPARAMS();
	if (Console::GetInst().ca_DebugAnimUsage)
		GetAnimationUpr().DebugAnimUsage(1);
	if (Console::GetInst().ca_UnloadAnimationDBA)
		DatabaseUnloading();

	if (Console::GetInst().ca_DebugAnimMemTracking)
	{
		//Test if memory tracking works
		float fBlue[4] = { 0, 0, 1, 1 };
		g_YLine += 10.0f;
		if (g_AnimationUpr.m_AnimMemoryTracker.m_nAnimsCounter)
		{
			uint64 average = g_AnimationUpr.m_AnimMemoryTracker.m_nAnimsAdd / g_AnimationUpr.m_AnimMemoryTracker.m_nAnimsCounter;
			g_pAuxGeom->Draw2dLabel(1, g_YLine, 2.0f, fBlue, false, "nAnimsPerFrame: %4d  nAnimsMax: %4d nAnimsAvrg: %4d", g_AnimationUpr.m_AnimMemoryTracker.m_nAnimsCurrent / 1024, g_AnimationUpr.m_AnimMemoryTracker.m_nAnimsMax / 1024, (u32)(average / 1024));
			g_YLine += 16.0f;
		}
	}

	if (Console::GetInst().ca_DebugAnimUpdates)
	{
		float fColor[4] = { 0, 1, 1, 1 };
		g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.3f, fColor, false, "AnimationUpdates: %d", g_AnimationUpdates);
		g_YLine += 16.0f;
		g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.3f, fColor, false, "SkeletonUpdates: %d", g_SkeletonUpdates);
		g_YLine += 16.0f;

		size_t numFSU = std::count_if(begin(m_arrForceSkeletonUpdates), end(m_arrForceSkeletonUpdates), [](i32 i) {return i > 0; });
		g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.3f, fColor, false, "Instances with 'Force Skeleton Update': %d", numFSU);

		g_YLine += 16.0f;
		u32k numInstances = m_arrForceSkeletonUpdates.size();
		for (u32 i = 0; i < numInstances; i++)
		{
			g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.2f, fColor, false, "Anim:(%d)  Force:(%d)  Visible:(%d)  ModelPath: %s", m_arrAnimPlaying[i], m_arrForceSkeletonUpdates[i], m_arrVisible[i], m_arrSkeletonUpdates[i].c_str());
			g_YLine += 14.0f;
		}
	}
	g_AnimationUpdates = 0;
	g_SkeletonUpdates = 0;

	m_arrSkeletonUpdates.resize(0);
	m_arrAnimPlaying.resize(0);
	m_arrForceSkeletonUpdates.resize(0);
	m_arrVisible.resize(0);

#ifndef CONSOLE_CONST_CVAR_MODE
	if (Console::GetInst().ca_DumpUsedAnims)
	{
		DumpAssetStatistics();
		Console::GetInst().ca_DumpUsedAnims = 0;
	}
#endif

#ifndef _RELEASE
	if (Console::GetInst().ca_vaProfile != 0)
	{
		g_vertexAnimationProfiler.DrawVertexAnimationStats(Console::GetInst().ca_vaProfile);
	}
	g_vertexAnimationProfiler.Clear();
#endif
}

void CharacterUpr::UpdateStreaming(i32 nFullUpdateRoundId, i32 nFastUpdateRoundId)
{
	DRX_PROFILE_FUNCTION(PROFILE_ANIMATION)

	if (nFastUpdateRoundId >= 0)
		m_nStreamUpdateRoundId[0] = static_cast<u32>(nFastUpdateRoundId);
	if (nFullUpdateRoundId >= 0)
		m_nStreamUpdateRoundId[1] = static_cast<u32>(nFullUpdateRoundId);

	if (Console::GetInst().ca_StreamCHR)
	{
		u32 nRenderFrameId = gEnv->pRenderer->GetFrameID(false);
		u32* nRoundIds = m_nStreamUpdateRoundId;

		UpdateStreaming_SKEL(m_arrModelCacheSKEL, nRenderFrameId, nRoundIds);
		UpdateStreaming_SKIN(m_arrModelCacheSKIN, nRenderFrameId, nRoundIds);

#ifdef EDITOR_PCDEBUGCODE
		UpdateStreaming_SKEL(m_arrModelCacheSKEL_CharEdit, nRenderFrameId, nRoundIds);
		UpdateStreaming_SKIN(m_arrModelCacheSKIN_CharEdit, nRenderFrameId, nRoundIds);
#endif
	}
}

void CharacterUpr::UpdateStreaming_SKEL(std::vector<CDefaultSkeletonReferences>& skels, u32 nRenderFrameId, u32k* nRoundIds)
{
	for (std::vector<CDefaultSkeletonReferences>::iterator it = skels.begin(), itEnd = skels.end(); it != itEnd; ++it)
	{
		CDefaultSkeleton* pSkel = it->m_pDefaultSkeleton;
		if (pSkel->m_ObjectType != CHR)
			continue;

		CModelMesh* pSkelMesh = pSkel->GetModelMesh();
		if (!pSkelMesh)
			continue;

		MeshStreamInfo& si = pSkelMesh->m_stream;
		if (!si.pStreamer)
		{
			bool bShouldBeInMemRender = si.nFrameId > nRenderFrameId - 4;
			bool bShouldBeInMemPrecache = false;
			for (i32 j = 0; j < MAX_STREAM_PREDICTION_ZONES; ++j)
				bShouldBeInMemPrecache = bShouldBeInMemPrecache || (si.nRoundIds[j] >= nRoundIds[j] - 2);

			bool bShouldBeInMemRefs = si.nKeepResidentRefs > 0;
			bool bShouldBeInMem = bShouldBeInMemRefs || bShouldBeInMemRender || bShouldBeInMemPrecache;

			if (bShouldBeInMem)
			{
				if (!pSkelMesh->m_pIRenderMesh)
				{
					EStreamTaskPriority estp;
					if (si.bIsUrgent)
						estp = estpUrgent;
					else if (bShouldBeInMemRefs)
						estp = estpAboveNormal;
					else
						estp = estpNormal;

					si.pStreamer = new DrxCHRLoader;
					si.pStreamer->BeginLoadCHRRenderMesh(pSkel, it->m_RefByInstances, estp);
				}
			}
			else if (pSkelMesh->m_pIRenderMesh)
			{
				pSkelMesh->m_pIRenderMesh = NULL;
			}

			si.bIsUrgent = false;
		}
	}
}

void CharacterUpr::UpdateStreaming_SKIN(std::vector<CDefaultSkinningReferences>& skins, u32 nRenderFrameId, u32k* nRoundIds)
{
	for (std::vector<CDefaultSkinningReferences>::iterator it = skins.begin(), itEnd = skins.end(); it != itEnd; ++it)
	{
		CSkin* pSkin = it->m_pDefaultSkinning;

		for (i32 i = 0, c = pSkin->m_arrModelMeshes.size(); i < c; ++i)
		{
			CModelMesh& mm = pSkin->m_arrModelMeshes[i];
			MeshStreamInfo& si = mm.m_stream;

			if (!si.pStreamer)
			{
				bool bShouldBeInMemRender = si.nFrameId > nRenderFrameId - 4;
				bool bShouldBeInMemPrecache = false;
				for (i32 j = 0; j < MAX_STREAM_PREDICTION_ZONES; ++j)
					bShouldBeInMemPrecache = bShouldBeInMemPrecache || (si.nRoundIds[j] >= nRoundIds[j] - 2);

				bool bShouldBeInMemRefs = si.nKeepResidentRefs > 0;
				bool bShouldBeInMem = bShouldBeInMemRefs || bShouldBeInMemRender || bShouldBeInMemPrecache;

				if (bShouldBeInMem)
				{
					if (!mm.m_pIRenderMesh)
					{
						EStreamTaskPriority estp;
						if (si.bIsUrgent)
							estp = estpUrgent;
						else if (bShouldBeInMemRefs)
							estp = estpAboveNormal;
						else
							estp = estpNormal;

						si.pStreamer = new DrxCHRLoader;
						si.pStreamer->BeginLoadSkinRenderMesh(pSkin, i, estp);
					}
				}
				else if (mm.m_pIRenderMesh)
				{
					CDefaultSkinningReferences* pSkinRef = GetDefaultSkinningReferences(pSkin);
					if (pSkinRef == 0)
						return;
					u32 nSkinInstances = pSkinRef->m_RefByInstances.size();
					for (i32 i = 0; i < nSkinInstances; i++)
					{
						CAttachmentSKIN* pAttachmentSkin = pSkinRef->m_RefByInstances[i];
						i32 guid = pAttachmentSkin->GetGuid();

						if (guid > 0)
						{
							mm.m_pIRenderMesh->ReleaseRemappedBoneIndicesPair(guid);
						}
					}

					u32 nVClothInstances = pSkinRef->m_RefByInstancesVCloth.size();
					for (i32 i = 0; i < nVClothInstances; i++)
					{
						CAttachmentVCLOTH* pAttachmentVCloth = pSkinRef->m_RefByInstancesVCloth[i];
						i32 guid = pAttachmentVCloth->GetGuid();

						if (guid > 0)
						{
							if (pAttachmentVCloth->m_pSimSkin)
							{
								IRenderMesh* pVCSimRenderMesh = pAttachmentVCloth->m_pSimSkin->GetIRenderMesh(0);
								if (pVCSimRenderMesh && (pVCSimRenderMesh == mm.m_pIRenderMesh))
								{
									pVCSimRenderMesh->ReleaseRemappedBoneIndicesPair(guid);
								}
							}

							if (pAttachmentVCloth->m_pRenderSkin)
							{
								IRenderMesh* pVCRenderMesh = pAttachmentVCloth->m_pRenderSkin->GetIRenderMesh(0);
								if (pVCRenderMesh && (pVCRenderMesh == mm.m_pIRenderMesh))
								{
									pVCRenderMesh->ReleaseRemappedBoneIndicesPair(guid);
								}
							}
						}
					}

					mm.m_pIRenderMesh = NULL;
				}

				si.bIsUrgent = false;
			}
		}
	}
}

//----------------------------------------------------------------------
CDefaultSkeleton* CharacterUpr::GetModelByAimPoseID(u32 nGlobalIDAimPose)
{
	u32 numModels = m_arrModelCacheSKEL.size();
	for (u32 m = 0; m < numModels; m++)
	{
		CDefaultSkeleton* pModelBase = m_arrModelCacheSKEL[m].m_pDefaultSkeleton;
		tukk strFilePath = pModelBase->GetModelFilePath();
		tukk fileExt = PathUtil::GetExt(strFilePath);
		u32 isSKEL = stricmp(fileExt, DRX_SKEL_FILE_EXT) == 0;
		if (isSKEL)
		{
			CDefaultSkeleton* pDefaultSkeleton = (CDefaultSkeleton*)pModelBase;
			if (pDefaultSkeleton->m_ObjectType == CHR)
			{
				tukk PathName = pDefaultSkeleton->GetModelFilePath();
				CAnimationSet* pAnimationSet = pDefaultSkeleton->m_pAnimationSet;
				i32 status = pAnimationSet->FindAimposeByGlobalID(nGlobalIDAimPose);
				if (status > 0)
					return pDefaultSkeleton;
			}
		}
	}

#ifdef EDITOR_PCDEBUGCODE
	u32 numModels2 = m_arrModelCacheSKEL_CharEdit.size();
	for (u32 m = 0; m < numModels2; m++)
	{
		CDefaultSkeleton* pModelBase = m_arrModelCacheSKEL_CharEdit[m].m_pDefaultSkeleton;
		tukk strFilePath = pModelBase->GetModelFilePath();
		tukk fileExt = PathUtil::GetExt(strFilePath);
		u32 isSKEL = stricmp(fileExt, DRX_SKEL_FILE_EXT) == 0;
		if (isSKEL)
		{
			CDefaultSkeleton* pDefaultSkeleton = (CDefaultSkeleton*)pModelBase;
			if (pDefaultSkeleton->m_ObjectType == CHR)
			{
				tukk PathName = pDefaultSkeleton->GetModelFilePath();
				CAnimationSet* pAnimationSet = pDefaultSkeleton->m_pAnimationSet;
				i32 status = pAnimationSet->FindAimposeByGlobalID(nGlobalIDAimPose);
				if (status > 0)
					return pDefaultSkeleton;
			}
		}
	}
#endif

	return 0;
}

void CharacterUpr::ReloadAllCHRPARAMS()
{
#ifdef EDITOR_PCDEBUGCODE
	StopAnimationsOnAllInstances();
	ClearPoseModifiersFromSynchQueue();
	SyncAllAnimations();
	Console::GetInst().ca_ReloadAllCHRPARAMS = 0;

	u32 numModels = m_arrModelCacheSKEL.size();
	for (u32 m = 0; m < numModels; m++)
	{
		CDefaultSkeleton* pDefaultSkeleton = m_arrModelCacheSKEL[m].m_pDefaultSkeleton;

		string paramFileNameBase(pDefaultSkeleton->GetModelFilePath());
		paramFileNameBase.replace(".chr", ".chrparams");
		paramFileNameBase.replace(".cga", ".chrparams");

		if (g_pIPak->IsFileExist(paramFileNameBase))
		{
			g_pCharacterUpr->GetParamLoader().ClearLists();
			pDefaultSkeleton->m_pAnimationSet->ReleaseAnimationData();

			DrxLog("Reloading %s", paramFileNameBase.c_str());

			DrxLogAlways("DinrusXAnimation: Reloading CHRPARAMS for model: %s", pDefaultSkeleton->GetModelFilePath());
			pDefaultSkeleton->m_pAnimationSet->NotifyListenersAboutReloadStarted();
			pDefaultSkeleton->LoadCHRPARAMS(paramFileNameBase.c_str());
			pDefaultSkeleton->m_pAnimationSet->NotifyListenersAboutReload();
		}
	}

	u32 numModelsCharEdit = m_arrModelCacheSKEL_CharEdit.size();
	for (u32 m = 0; m < numModelsCharEdit; m++)
	{
		CDefaultSkeleton* pDefaultSkeleton = m_arrModelCacheSKEL_CharEdit[m].m_pDefaultSkeleton;

		string paramFileNameBase(pDefaultSkeleton->GetModelFilePath());
		paramFileNameBase.replace(".chr", ".chrparams");
		paramFileNameBase.replace(".cga", ".chrparams");

		if (g_pIPak->IsFileExist(paramFileNameBase))
		{
			g_pCharacterUpr->GetParamLoader().ClearLists();
			pDefaultSkeleton->m_pAnimationSet->ReleaseAnimationData();

			DrxLog("Reloading %s", paramFileNameBase.c_str());

			DrxLogAlways("DinrusXAnimation: Reloading CHRPARAMS for model: %s", pDefaultSkeleton->GetModelFilePath());
			pDefaultSkeleton->m_pAnimationSet->NotifyListenersAboutReloadStarted();
			pDefaultSkeleton->LoadCHRPARAMS(paramFileNameBase.c_str());
			pDefaultSkeleton->m_pAnimationSet->NotifyListenersAboutReload();
		}
	}

#endif
}

void CharacterUpr::TrackMemoryOfModels()
{
#ifndef _RELEASE
	u32 tmp = Console::GetInst().ca_DebugModelCache;
	Console::GetInst().ca_DebugModelCache = 7;
	DebugModelCache(0, m_arrModelCacheSKEL, m_arrModelCacheSKIN);
	Console::GetInst().ca_DebugModelCache = tmp;
#endif
}

void CharacterUpr::DebugModelCache(u32 printtxt, std::vector<CDefaultSkeletonReferences>& parrModelCacheSKEL, std::vector<CDefaultSkinningReferences>& parrModelCacheSKIN)
{
	float fWhite[4] = { 1, 1, 1, 1 };
	m_AnimationUpr.m_AnimMemoryTracker.m_numTCharInstances = 0;
	m_AnimationUpr.m_AnimMemoryTracker.m_nTotalCharMemory = 0;
	m_AnimationUpr.m_AnimMemoryTracker.m_numModels = 0;

	u32 nTotalModelMemory = 0;
	u32 nTotalModelSkeletonMemory = 0;
	u32 nTotalModelPhysics = 0;
	u32 nTotalUsedPhysics = 0;
	u32 nTotalModelAnimationSet = 0;
	u32 nTotalAttachmentMemory = 0;
	u32 nTotalAttachmentCount = 0;

	u32 nPrintFlags = Console::GetInst().ca_DebugModelCache;
	if (nPrintFlags & 1)
	{
		u32 nTotalSkelMemory = 0;
		u32 nTotalSkeletonMemory = 0;
		u32 numTSkelInstances = 0;

		u32 numModelSKELs = parrModelCacheSKEL.size();
		g_pAuxGeom->Draw2dLabel(1, g_YLine, 2.0f, fWhite, false, "arrModelCacheSKEL: %4d", numModelSKELs), g_YLine += 20.0f;
		for (u32 m = 0; m < numModelSKELs; m++)
		{
			CDefaultSkeleton* pDefaultSkeleton = parrModelCacheSKEL[m].m_pDefaultSkeleton;
			if (pDefaultSkeleton->m_pCGA_Object)
				continue;
			u32 numSkelInstances = parrModelCacheSKEL[m].m_RefByInstances.size();
			u32 nRefCounter = pDefaultSkeleton->GetRefCounter();
			numTSkelInstances += numSkelInstances;
			u32 numAnims = pDefaultSkeleton->m_pAnimationSet->GetAnimationCount();
			u32 nIMemory = 0;
			for (u32 i = 0; i < numSkelInstances; i++)
			{
				CCharInstance* pCharInstance = parrModelCacheSKEL[m].m_RefByInstances[i];
				nTotalSkelMemory += pCharInstance->SizeOfCharInstance();
				nTotalAttachmentMemory += pCharInstance->m_AttachmentUpr.SizeOfAllAttachments();
				nTotalAttachmentCount += pCharInstance->m_AttachmentUpr.GetAttachmentCount();
				nIMemory += pCharInstance->SizeOfCharInstance();
			}

			nTotalModelMemory += pDefaultSkeleton->SizeOfDefaultSkeleton();
			nTotalModelSkeletonMemory += pDefaultSkeleton->SizeOfSkeleton();
			nTotalModelAnimationSet += pDefaultSkeleton->m_pAnimationSet->SizeOfAnimationSet();
			if (printtxt)
			{
				float fColor[4] = { 0, 1, 0, 1 };
				tukk PathName = pDefaultSkeleton->GetModelFilePath();
				u32 nKiM = pDefaultSkeleton->GetKeepInMemory();
				u32 nMMemory = pDefaultSkeleton->SizeOfDefaultSkeleton();
				if ((nPrintFlags & 0x10) == 0)
				{
					g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.3f, fColor, false, "Model: %3d   SkelInst: %2d  RefCount: %2d imem: %7d  mmem: %7d anim: %4d  KiM: %2d %s", m, numSkelInstances, nRefCounter, nIMemory, nMMemory, numAnims, nKiM, PathName),
					g_YLine += 16.0f;
				}
				if (nPrintFlags & 0x8)
				{
					DrxLogAlways("Model: %3u   SkelInst: %2u  RefCount: %2u  imem: %7u  mmem: %7u anim: %4u  KiM: %2u %s", m, numSkelInstances, nRefCounter, nIMemory, nMMemory, numAnims, nKiM, PathName), g_YLine += 16.0f;
				}
			}
		}
		float fColorRed[4] = { 1, 0, 0, 1 };
		g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.6f, fColorRed, false, "SKELInstancesCounter:%3d (Mem: %4dKB)     ModelsCounter:%3d (Mem: %4dKB)", numTSkelInstances, nTotalSkelMemory / 1024, numModelSKELs, nTotalModelMemory / 1024);
		g_YLine += 32.0f;
		if (nPrintFlags & 0x8)
		{
			DrxLogAlways("SKELInstancesCounter:%3u (Mem: %4uKB)     ModelsCounter:%3u (Mem: %4uKB)", numTSkelInstances, nTotalSkelMemory / 1024, numModelSKELs, nTotalModelMemory / 1024), g_YLine += 16.0f;
		}

		m_AnimationUpr.m_AnimMemoryTracker.m_numTCharInstances += numTSkelInstances;
		m_AnimationUpr.m_AnimMemoryTracker.m_nTotalCharMemory += nTotalSkelMemory;
		m_AnimationUpr.m_AnimMemoryTracker.m_numModels += numModelSKELs;
	}

	//-----------------------------------------------------------------------------------------------------------------------------
	if (nPrintFlags & 2)
	{
		u32 nTotalSkelMemory = 0;
		u32 numTSkelInstances = 0;
		u32 numModelSKELs = parrModelCacheSKEL.size();
		for (u32 m = 0; m < numModelSKELs; m++)
		{
			CDefaultSkeleton* pDefaultSkeleton = parrModelCacheSKEL[m].m_pDefaultSkeleton;
			if (pDefaultSkeleton->m_pCGA_Object == 0)
				continue;
			u32 numSkelInstances = parrModelCacheSKEL[m].m_RefByInstances.size();
			u32 nRefCounter = pDefaultSkeleton->GetRefCounter();
			numTSkelInstances += numSkelInstances;
			u32 numAnims = pDefaultSkeleton->m_pAnimationSet->GetAnimationCount();
			u32 nIMemory = 0;
			for (u32 i = 0; i < numSkelInstances; i++)
			{
				CCharInstance* pCharInstance = parrModelCacheSKEL[m].m_RefByInstances[i];
				nTotalSkelMemory += pCharInstance->SizeOfCharInstance();
				nTotalAttachmentMemory += pCharInstance->m_AttachmentUpr.SizeOfAllAttachments();
				nTotalAttachmentCount += pCharInstance->m_AttachmentUpr.GetAttachmentCount();
				nIMemory += pCharInstance->SizeOfCharInstance();
			}

			nTotalModelMemory += pDefaultSkeleton->SizeOfDefaultSkeleton();
			nTotalModelSkeletonMemory += pDefaultSkeleton->SizeOfSkeleton();
			nTotalModelAnimationSet += pDefaultSkeleton->m_pAnimationSet->SizeOfAnimationSet();
			if (printtxt)
			{
				float fColor[4] = { 0, 1, 0, 1 };
				tukk PathName = pDefaultSkeleton->GetModelFilePath();
				u32 nKiM = pDefaultSkeleton->GetKeepInMemory();
				u32 nMMemory = pDefaultSkeleton->SizeOfDefaultSkeleton();
				if ((nPrintFlags & 0x10) == 0)
					g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.3f, fColor, false, "Model: %3d   SkelInst: %2d  RefCount: %2d imem: %7d  mmem: %7d anim: %4d  Kim: %2d %s", m, numSkelInstances, nRefCounter, nIMemory, nMMemory, numAnims, nKiM, PathName), g_YLine += 16.0f;
				if (nPrintFlags & 0x08)
					DrxLogAlways("Model: %3u   SkelInst: %2u  RefCount: %2u  imem: %7u  mmem: %7u anim: %4u  KiM: %2u %s", m, numSkelInstances, nRefCounter, nIMemory, nMMemory, numAnims, nKiM, PathName), g_YLine += 16.0f;
			}
		}
		float fColorRed[4] = { 1, 0, 0, 1 };
		g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.6f, fColorRed, false, "SKELInstancesCounter:%3d (Mem: %4dKB)     ModelsCounter:%3d (Mem: %4dKB)", numTSkelInstances, nTotalSkelMemory / 1024, numModelSKELs, nTotalModelMemory / 1024);
		g_YLine += 32.0f;
		if (nPrintFlags & 0x08)
			DrxLogAlways("SKELInstancesCounter:%3u (Mem: %4uKB)     ModelsCounter:%3u (Mem: %4uKB)", numTSkelInstances, nTotalSkelMemory / 1024, numModelSKELs, nTotalModelMemory / 1024), g_YLine += 16.0f;
		m_AnimationUpr.m_AnimMemoryTracker.m_numTCharInstances += numTSkelInstances;
		m_AnimationUpr.m_AnimMemoryTracker.m_nTotalCharMemory += nTotalSkelMemory;
		m_AnimationUpr.m_AnimMemoryTracker.m_numModels += numModelSKELs;
	}

	//-----------------------------------------------------------------------------------------------------------------------------

	if (nPrintFlags & 4)
	{
		u32 nTotalSkinMemory = 0;
		u32 numTSkinInstances = 0;
		u32 numModelSKINs = parrModelCacheSKIN.size();
		g_pAuxGeom->Draw2dLabel(1, g_YLine, 2.0f, fWhite, false, "arrModelCacheSKIN: %4d", numModelSKINs), g_YLine += 20.0f;
		for (u32 m = 0; m < numModelSKINs; m++)
		{
			CSkin* pModelSKIN = parrModelCacheSKIN[m].m_pDefaultSkinning;
			u32 nRefCounter = pModelSKIN->GetRefCounter();
			u32 numSkinInstances = parrModelCacheSKIN[m].m_RefByInstances.size();
			numTSkinInstances += numSkinInstances;
			u32 nIMemory = 0;
			for (u32 i = 0; i < numSkinInstances; i++)
			{
				CAttachmentSKIN* pSkinInstance = parrModelCacheSKIN[m].m_RefByInstances[i];
				nIMemory += pSkinInstance->SizeOfThis();
				nTotalSkinMemory += pSkinInstance->SizeOfThis();
			}
			u32 numVClothInstances = parrModelCacheSKIN[m].m_RefByInstancesVCloth.size();
			numTSkinInstances += numVClothInstances;
			for (u32 i = 0; i < numVClothInstances; i++)
			{
				CAttachmentVCLOTH* pVClothInstance = parrModelCacheSKIN[m].m_RefByInstancesVCloth[i];
				nIMemory += pVClothInstance->SizeOfThis();
				nTotalSkinMemory += pVClothInstance->SizeOfThis();
			}

			nTotalModelMemory += pModelSKIN->SizeOfModelData();
			if (printtxt)
			{
				tukk PathName = pModelSKIN->GetModelFilePath();

				u32 numLODs = pModelSKIN->GetNumLODs();
				u32 nKiM = pModelSKIN->GetKeepInMemory();

				u32 nMMemory = pModelSKIN->SizeOfModelData();
				float fColor[4] = { 0, 0.5f, 0, 0.7f };
				if ((nPrintFlags & 0x10) == 0)
					g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.3f, fColor, false, "Model: %3d   SkinInst: %2d  RefCount: %2d imem: %7d  mmem: %7d  LOD: %2d KiM: %2d  %s", m, numSkinInstances + numVClothInstances, nRefCounter, nIMemory, nMMemory, numLODs, nKiM, PathName), g_YLine += 16.0f;
				if (nPrintFlags & 0x08)
					DrxLogAlways("Model: %3u   SkinInst: %2u  RefCount: %2u imem: %7u  mmem: %7u  LOD: %2u KiM: %2u %s", m, numSkinInstances + numVClothInstances, nRefCounter, nIMemory, nMMemory, numLODs, nKiM, PathName), g_YLine += 16.0f;
			}
		}
		float fColorRed[4] = { 1, 0, 0, 1 };
		g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.6f, fColorRed, false, "SKINInstancesCounter:%3d (Mem: %4dKB)     ModelsCounter:%3d (Mem: %4dKB)", numTSkinInstances, nTotalSkinMemory / 1024, numModelSKINs, nTotalModelMemory / 1024);
		g_YLine += 32.0f;
		m_AnimationUpr.m_AnimMemoryTracker.m_numTSkinInstances = numTSkinInstances;
		m_AnimationUpr.m_AnimMemoryTracker.m_nTotalSkinMemory = nTotalSkinMemory;
	}

	//-----------------------------------------------------------------------------------------------------------------------------

	if (printtxt)
	{
		float fColorRed[4] = { 1, 0, 0, 1 };
		g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.6f, fColorRed, false, "-----------------------------------------------------------------------------------------");
		g_YLine += 16.0f;

		g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.6f, fColorRed, false, "nTotalAttachmentMemory:  %4dKB  nTotalAttachmentCount:  %4dKB", nTotalAttachmentMemory / 1024, nTotalAttachmentCount);
		g_YLine += 32.0f;

		g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.6f, fColorRed, false, "nAnimationSet:  %4dKB", nTotalModelAnimationSet / 1024);
		g_YLine += 16.0f;
		if (nPrintFlags & 0x08)
			DrxLogAlways("nAnimationSet:  %4uKB", nTotalModelAnimationSet / 1024);

		g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.6f, fColorRed, false, "nTotalModelSkeletonMemory:  %4dKB     nTotalModelPhysics: %4dKB   nUsedPhysics: %4dKB   nEmptyPhysics: %4dKB", nTotalModelSkeletonMemory / 1024, nTotalModelPhysics / 1024, nTotalUsedPhysics / 1024, nTotalModelPhysics / 1024 - nTotalUsedPhysics / 1024);
		g_YLine += 16.0f;
		if (nPrintFlags & 0x08)
			DrxLogAlways("nTotalModelSkeletonMemory:  %4uKB     nTotalModelPhysics: %4uKB   nUsedPhysics: %4uKB   nEmptyPhysics: %4uKB", nTotalModelSkeletonMemory / 1024, nTotalModelPhysics / 1024, nTotalUsedPhysics / 1024, nTotalModelPhysics / 1024 - nTotalUsedPhysics / 1024);

		g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.6f, fColorRed, false, "TotalModelMemory (Mem: %4dKB)", nTotalModelMemory / 1024);
		g_YLine += 32.0f;
	}

	m_AnimationUpr.m_AnimMemoryTracker.m_nTotalMMemory = nTotalModelMemory;
}

//---------------------------------
//just for reloading
CAnimationSet* CharacterUpr::GetAnimationSetUsedInCharEdit()
{
#ifdef EDITOR_PCDEBUGCODE
	u32 numModels = m_arrModelCacheSKEL_CharEdit.size();
	for (u32 m = 0; m < numModels; m++)
	{
		float fColor[4] = { 0, 1, 0, 1 };
		CDefaultSkeleton* pDefaultSkeleton = m_arrModelCacheSKEL_CharEdit[m].m_pDefaultSkeleton;
		tukk strFilepath = pDefaultSkeleton->GetModelFilePath();
		tukk fileExt = PathUtil::GetExt(strFilepath);
		u32 isSKEL = stricmp(fileExt, DRX_SKEL_FILE_EXT) == 0;
		if (!isSKEL)
			continue;

		u32 numInstances = m_arrModelCacheSKEL_CharEdit[m].m_RefByInstances.size();
		for (u32 i = 0; i < numInstances; i++)
		{
			CCharInstance* pCharInstance = m_arrModelCacheSKEL_CharEdit[m].m_RefByInstances[i];
			if (pCharInstance->GetCharEditMode())
			{
				CDefaultSkeleton* pInstModel = pCharInstance->m_pDefaultSkeleton;
				if (pDefaultSkeleton != pInstModel)
					DrxFatalError("DinrusXAnimation: mem corruption");
				return (CAnimationSet*)pInstModel->GetIAnimationSet();
			}
		}
	}
#endif

	return 0;
}

void CharacterUpr::StopAnimationsOnAllInstances()
{
	InternalStopAnimationsOnAllInstances(nullptr);
}

void CharacterUpr::StopAnimationsOnAllInstances(const IAnimationSet& animationSet)
{
	InternalStopAnimationsOnAllInstances(&animationSet);
}

void CharacterUpr::InternalStopAnimationsOnAllInstances(const IAnimationSet* pAnimationSet)
{
	u32 numModels = m_arrModelCacheSKEL.size();
	for (u32 m = 0; m < numModels; m++)
	{
		CDefaultSkeleton* pDefaultSkeleton = m_arrModelCacheSKEL[m].m_pDefaultSkeleton;
		if (pAnimationSet && (pDefaultSkeleton->GetIAnimationSet() != pAnimationSet))
			continue;

		tukk strFilepath = pDefaultSkeleton->GetModelFilePath();
		tukk fileExt = PathUtil::GetExt(strFilepath);
		u32 isSKEL = stricmp(fileExt, DRX_SKEL_FILE_EXT) == 0;
		if (isSKEL == 0)
			continue;

		u32 numInstances = m_arrModelCacheSKEL[m].m_RefByInstances.size();
		for (u32 i = 0; i < numInstances; i++)
		{
			CCharInstance* pCharInstance = m_arrModelCacheSKEL[m].m_RefByInstances[i];
			pCharInstance->m_SkeletonAnim.StopAnimationsAllLayers();
			pCharInstance->m_SkeletonAnim.FinishAnimationComputations();
		}
	}

#ifdef EDITOR_PCDEBUGCODE
	u32 numModelsCharEdit = m_arrModelCacheSKEL_CharEdit.size();
	for (u32 m = 0; m < numModelsCharEdit; m++)
	{
		CDefaultSkeleton* pDefaultSkeleton = m_arrModelCacheSKEL_CharEdit[m].m_pDefaultSkeleton;
		if (pAnimationSet && (pDefaultSkeleton->GetIAnimationSet() != pAnimationSet))
			continue;

		tukk strFilepath = pDefaultSkeleton->GetModelFilePath();
		tukk fileExt = PathUtil::GetExt(strFilepath);
		u32 isSKEL = stricmp(fileExt, DRX_SKEL_FILE_EXT) == 0;
		if (isSKEL == 0)
			continue;

		u32 numInstances = m_arrModelCacheSKEL_CharEdit[m].m_RefByInstances.size();
		for (u32 i = 0; i < numInstances; i++)
		{
			CCharInstance* pCharInstance = m_arrModelCacheSKEL_CharEdit[m].m_RefByInstances[i];
			pCharInstance->m_SkeletonAnim.StopAnimationsAllLayers();
			pCharInstance->m_SkeletonAnim.FinishAnimationComputations();
		}
	}
#endif

	u32 numLMGs = g_AnimationUpr.m_arrGlobalLMG.size();
	for (u32 l = 0; l < numLMGs; l++)
	{
		GlobalAnimationHeaderLMG& rLMG = g_AnimationUpr.m_arrGlobalLMG[l];
		rLMG.m_DimPara[0].m_nInitialized = 0;
		rLMG.m_DimPara[1].m_nInitialized = 0;
		rLMG.m_DimPara[2].m_nInitialized = 0;
		rLMG.m_DimPara[3].m_nInitialized = 0;
	}

	ClearPoseModifiersFromSynchQueue();
	SyncAllAnimations();
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
void CharacterUpr::DatabaseUnloading()
{
	if (Console::GetInst().ca_DisableAnimationUnloading)
		return;

	u32 numHeadersCAF = g_AnimationUpr.m_arrGlobalCAF.size();
	if (numHeadersCAF == 0)
		return;
	u32 numHeadersDBA = g_AnimationUpr.m_arrGlobalHeaderDBA.size();
	if (numHeadersDBA == 0)
		return;

	GlobalAnimationHeaderCAF* parrGlobalCAF = &g_AnimationUpr.m_arrGlobalCAF[0];
	CGlobalHeaderDBA* parrGlobalDBA = &g_AnimationUpr.m_arrGlobalHeaderDBA[0];

	u32 s = m_StartGAH_Iterator;
	u32 e = min(numHeadersCAF, m_StartGAH_Iterator + 100);

	if (s == 0)
	{
		for (u32 d = 0; d < numHeadersDBA; d++)
			parrGlobalDBA[d].m_nUsedAnimations = 0;
	}

	for (u32 i = s; i < e; i++)
	{
		if (parrGlobalCAF[i].m_nRef_at_Runtime == 0)
		{
			if (parrGlobalCAF[i].IsAssetOnDemand() && parrGlobalCAF[i].IsAssetLoaded())
				g_AnimationUpr.UnloadAnimationCAF(parrGlobalCAF[i]);
			continue;
		}
		u32 nCRC32 = parrGlobalCAF[i].m_FilePathDBACRC32;
		if (nCRC32)
		{
			for (u32 d = 0; d < numHeadersDBA; d++)
			{
				if (nCRC32 == parrGlobalDBA[d].m_FilePathDBACRC32)
				{
					parrGlobalDBA[d].m_nUsedAnimations++;
					parrGlobalDBA[d].m_nLastUsedTimeDelta = 0;
				}
			}
		}
	}

	m_StartGAH_Iterator += 100;
	if (m_StartGAH_Iterator < numHeadersCAF)
		return;

	//----------------------------------------------------------------------------------------------------

	m_StartGAH_Iterator = 0;
	float fColor[4] = { 1, 0, 0, 1 };
	//	g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.2f, fColor, false,"DatabaseUnloading");
	//	g_YLine+=12.0f;

	u32 timeDelta = GetDatabaseUnloadTimeDelta();

	for (u32 d = 0; d < numHeadersDBA; d++)
	{
		CGlobalHeaderDBA& rGHDBA = g_AnimationUpr.m_arrGlobalHeaderDBA[d];
		if (rGHDBA.m_pDatabaseInfo == 0)
			continue;

		tukk pName = rGHDBA.m_strFilePathDBA;
		u32 nDBACRC32 = rGHDBA.m_FilePathDBACRC32;
		if (rGHDBA.m_nUsedAnimations || rGHDBA.m_bDBALock)
		{
			rGHDBA.m_nLastUsedTimeDelta = 0;
			continue;
		}

		rGHDBA.m_nLastUsedTimeDelta += timeDelta;

		if (rGHDBA.m_nLastUsedTimeDelta <= (u32) Console::GetInst().ca_DBAUnloadUnregisterTime * 1000)
			continue;

		//	g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.2f, fColor, false,"Scanning: %s",pName );
		//	g_YLine+=12.0f;

		for (u32 i = 0; i < numHeadersCAF; i++)
		{
			GlobalAnimationHeaderCAF& rGAH = g_AnimationUpr.m_arrGlobalCAF[i];
			if (rGAH.m_FilePathDBACRC32 != nDBACRC32)
				continue;
			rGAH.m_nControllers = 0;    //just mark as unloaded
		}
	}

	//check if the time  has come to remove DBA
	for (u32 d = 0; d < numHeadersDBA; d++)
	{
		CGlobalHeaderDBA& rGHDBA = g_AnimationUpr.m_arrGlobalHeaderDBA[d];
		if (rGHDBA.m_pDatabaseInfo == 0)
			continue;
		if (rGHDBA.m_nLastUsedTimeDelta <= (u32) Console::GetInst().ca_DBAUnloadRemoveTime * 1000)
			continue;
		if (rGHDBA.m_nUsedAnimations)
			continue;
#ifdef _DEBUG
		tukk pName = rGHDBA.m_strFilePathDBA;
#endif

		m_AllowStartOfAnimation = 0;
		u32 numModels = m_arrModelCacheSKEL.size();
		for (u32 m = 0; m < numModels; m++)
		{
			CDefaultSkeleton* pDefaultSkeleton = m_arrModelCacheSKEL[m].m_pDefaultSkeleton;
			tukk strFilepath = pDefaultSkeleton->GetModelFilePath();
			tukk fileExt = PathUtil::GetExt(strFilepath);
			u32 isSKEL = stricmp(fileExt, DRX_SKEL_FILE_EXT) == 0;
			if (isSKEL == 0)
				continue;
			u32 numInstances = m_arrModelCacheSKEL[m].m_RefByInstances.size();
			for (u32 i = 0; i < numInstances; i++)
			{
				CCharInstance* pCharInstance = m_arrModelCacheSKEL[m].m_RefByInstances[i];
				pCharInstance->m_SkeletonAnim.FinishAnimationComputations();
			}
		}
		rGHDBA.DeleteDatabaseDBA(); //remove entire DBA from memory
		m_AllowStartOfAnimation = 1;
	}

	UpdateDatabaseUnloadTimeStamp();
}

//////////////////////////////////////////////////////////////////////////
class CAnimationStatsResourceList
{
public:
	stack_string UnifyFilename(tukk sResourceFile)
	{
		stack_string filename = sResourceFile;
		filename.replace('\\', '/');
		filename.MakeLower();
		return filename;
	}

	void Add(tukk sResourceFile)
	{
		stack_string filename = UnifyFilename(sResourceFile);
		m_set.insert(filename);
	}
	void Remove(tukk sResourceFile)
	{
		stack_string filename = UnifyFilename(sResourceFile);
		m_set.erase(filename);
	}
	void Clear()
	{
		m_set.clear();
	}
	bool IsExist(tukk sResourceFile)
	{
		stack_string filename = UnifyFilename(sResourceFile);
		if (m_set.find(filename) != m_set.end())
			return true;
		return false;
	}
	void LoadCAF(tukk sResourceListFilename)
	{
		m_filename = sResourceListFilename;
		FILE* file = fxopen(sResourceListFilename, "rb");
		if (file)
		{
			PREFAST_SUPPRESS_WARNING(6031) fseek(file, 0, SEEK_END);
			i32 nFileLentgh = ftell(file);
			PREFAST_SUPPRESS_WARNING(6031) fseek(file, 0, SEEK_SET);
			tuk buf = new char[nFileLentgh + 16];
			(void)fread(buf, nFileLentgh, 1, file);
			buf[nFileLentgh] = 0;

			// Parse file, every line in a file represents a resource filename.
			char seps[] = "\r\n";
			tuk token = strtok(buf, seps);
			while (token != NULL)
			{
				token += 44;
				Add(token);
				token = strtok(NULL, seps);
			}
			delete[]buf;
		}
	}
	void LoadLMG(tukk sResourceListFilename)
	{
		m_filename = sResourceListFilename;
		FILE* file = fxopen(sResourceListFilename, "rb");
		if (file)
		{
			PREFAST_SUPPRESS_WARNING(6031) fseek(file, 0, SEEK_END);
			i32 nFileLentgh = ftell(file);
			PREFAST_SUPPRESS_WARNING(6031) fseek(file, 0, SEEK_SET);
			tuk buf = new char[nFileLentgh + 16];
			(void)fread(buf, nFileLentgh, 1, file);
			buf[nFileLentgh] = 0;

			// Parse file, every line in a file represents a resource filename.
			char seps[] = "\r\n";
			tuk token = strtok(buf, seps);
			while (token != NULL)
			{
				token += 20;
				Add(token);
				token = strtok(NULL, seps);
			}
			delete[]buf;
		}
	}

	void SaveCAF()
	{
		FILE* file = fxopen(m_filename, "wt");
		u32 num = 0;
		for (ResourceSet::const_iterator it = m_set.begin(); it != m_set.end(); ++it)
		{
			tukk pFilePath = (*it).c_str();
			u32 dbacrc32 = g_AnimationUpr.GetDBACRC32fromFilePath(pFilePath);
			u32 cafcrc32 = CCrc32::ComputeLowercase(pFilePath);

			u32 pos = 0;
			u32 rot = 0;
			u32 use = 0;
			u32 nNumHeadersCAF = g_AnimationUpr.m_arrGlobalCAF.size();
			for (u32 i = 0; i < nNumHeadersCAF; i++)
			{
				GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[i];
				if (rCAF.GetFilePathCRC32() != cafcrc32)
					continue;

				pos = rCAF.GetTotalPosKeys();
				rot = rCAF.GetTotalRotKeys();
				use = rCAF.m_nTouchedCounter;
				fprintf(file, "No:%04u DBA: %08x p:%02x q:%02x use:%05u   %s\n", num, dbacrc32, pos, rot, use, (*it).c_str());
				num++;
				break;
			}

		}
		fclose(file);
	}

	void SaveLMG()
	{
		FILE* file = fxopen(m_filename, "wt");
		u32 num = 0;
		for (ResourceSet::const_iterator it = m_set.begin(); it != m_set.end(); ++it)
		{
			tukk pFilePath = (*it).c_str();
			u32 lmgcrc32 = CCrc32::ComputeLowercase(pFilePath);

			u32 nNumHeadersLMG = g_AnimationUpr.m_arrGlobalLMG.size();
			for (u32 i = 0; i < nNumHeadersLMG; i++)
			{
				GlobalAnimationHeaderLMG& rLMG = g_AnimationUpr.m_arrGlobalLMG[i];
				if (rLMG.GetFilePathCRC32() != lmgcrc32)
					continue;

				fprintf(file, "No:%04u use:%05u   %s\n", num, rLMG.m_nTouchedCounter, (*it).c_str());
				num++;
				break;
			}

		}
		fclose(file);
	}

private:
	typedef std::set<string> ResourceSet;
	ResourceSet           m_set;
	ResourceSet::iterator m_iter;
	string                m_filename;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void CharacterUpr::DumpAssetStatistics()
{
	CAnimationStatsResourceList caf_usedList;
	CAnimationStatsResourceList caf_unusedList;
	CAnimationStatsResourceList lmg_usedList;
	CAnimationStatsResourceList lmg_unusedList;

	caf_usedList.LoadCAF("AnimStats_Used_CAF.txt");
	caf_unusedList.LoadCAF("AnimStats_NotUsed_CAF.txt");

	lmg_usedList.LoadLMG("AnimStats_Used_LMG.txt");
	lmg_unusedList.LoadLMG("AnimStats_NotUsed_LMG.txt");

	u32 nNumHeadersCAF = g_AnimationUpr.m_arrGlobalCAF.size();
	for (u32 i = 0; i < nNumHeadersCAF; i++)
	{
		GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[i];
		tukk strPathName = rCAF.GetFilePath();
		if (caf_usedList.IsExist(strPathName))
			rCAF.m_nTouchedCounter++;

		if (rCAF.m_nTouchedCounter)
		{
			caf_usedList.Add(strPathName);
			caf_unusedList.Remove(strPathName);
		}
		else
		{
			if (!caf_usedList.IsExist(strPathName))
				caf_unusedList.Add(strPathName);
		}
	}

	u32 nNumHeadersLMG = g_AnimationUpr.m_arrGlobalLMG.size();
	for (u32 i = 0; i < nNumHeadersLMG; i++)
	{
		GlobalAnimationHeaderLMG& rLMG = g_AnimationUpr.m_arrGlobalLMG[i];
		tukk strPathName = rLMG.GetFilePath();
		if (lmg_usedList.IsExist(strPathName))
			rLMG.m_nTouchedCounter++;

		if (rLMG.m_nTouchedCounter)
		{
			lmg_usedList.Add(strPathName);
			lmg_unusedList.Remove(strPathName);
		}
		else
		{
			if (!lmg_usedList.IsExist(strPathName))
				lmg_unusedList.Add(strPathName);
		}
	}

	caf_usedList.SaveCAF();
	caf_unusedList.SaveCAF();
	lmg_usedList.SaveLMG();
	lmg_unusedList.SaveLMG();
}

// can be called instead of Update() for UI purposes (such as in preview viewports, etc).
void CharacterUpr::DummyUpdate()
{
	// Note: DummyUpdate() is mutually exclusive with Update(). Either one or the other will be called, exactly once per frame, depending on whether we have a level loaded in Sandbox or not.

	CAnimationContext::Instance().Update();
	// We need to update the animation context here in order to flush the animation memory pool, regardless of if the character manager is fully running or not.
	// The reason for that is, even if character manager updates are suspended (i.e. no level is loaded), external tools (Character Tool, Mannequin, etc) can still
	// orchestrate animation updates of the character instances they own. If the memory pool is not flushed, this results in perpetual growth of memory usage.
	// TODO: Memory pooling in the animation system needs to be streamlined.

	m_nUpdateCounter++;
}

void CharacterUpr::GetMotionParameterDetails(SMotionParameterDetails& outDetails, EMotionParamID paramId) const
{
	static SMotionParameterDetails details[eMotionParamID_COUNT] = {
		{ "MoveSpeed",    "Travel Speed",    SMotionParameterDetails::ADDITIONAL_EXTRACTION },
		{ "TurnSpeed",    "Turn Speed",      SMotionParameterDetails::ADDITIONAL_EXTRACTION },
		{ "TravelAngle",  "Travel Angle",    SMotionParameterDetails::ADDITIONAL_EXTRACTION },
		{ "TravelSlope",  "Travel Slope",    SMotionParameterDetails::ADDITIONAL_EXTRACTION },
		{ "TurnAngle",    "Turn Angle",      SMotionParameterDetails::ADDITIONAL_EXTRACTION },
		{ "TravelDist",   "Travel Distance", SMotionParameterDetails::ADDITIONAL_EXTRACTION },
		{ "StopLeg",      "Stop Leg",        0                                              },
		{ "BlendWeight",  "Blend Weight",    0                                              },
		{ "BlendWeight2", "Blend Weight2",   0                                              },
		{ "BlendWeight3", "Blend Weight3",   0                                              },
		{ "BlendWeight4", "Blend Weight4",   0                                              },
		{ "BlendWeight5", "Blend Weight5",   0                                              },
		{ "BlendWeight6", "Blend Weight6",   0                                              },
		{ "BlendWeight7", "Blend Weight7",   0                                              }
	};
	if (paramId >= 0 && paramId < eMotionParamID_COUNT)
		outDetails = details[i32(paramId)];
	else
		outDetails = SMotionParameterDetails();
}

#ifdef EDITOR_PCDEBUGCODE
bool CharacterUpr::InjectCDF(tukk pathname, tukk fileContent, size_t fileLength)
{
	u32 id = GetOrLoadCDFId(pathname);
	if (id != INVALID_CDF_ID)
	{
		m_arrCacheForCDF.erase(m_arrCacheForCDF.begin() + id);
	}

	XmlNodeRef root = g_pISystem->LoadXmlFromBuffer(fileContent, fileLength);
	if (root == 0)
	{
		g_pILog->LogError("DinrusXAnimation: failed to parse injected XML file: %s", pathname);
		return false;
	}

	return LoadCDFFromXML(root, pathname) != INVALID_CDF_ID;
}

void CharacterUpr::InjectCHRPARAMS(tukk pathname, tukk content, size_t contentLength)
{
	m_ParamLoader.InjectCHRPARAMS(pathname, content, contentLength);
}
void CharacterUpr::ClearCHRPARAMSCache()
{
	m_ParamLoader.ClearCHRPARAMSCache();
}

 void CharacterUpr::InjectBSPACE(tukk pathname, tukk content, size_t contentLength)
{
	stack_string path = pathname;
	path.MakeLower();
	m_AnimationUpr.m_cachedBSPACES[path].assign(content, content + contentLength);
}

void CharacterUpr::ClearBSPACECache()
{
	m_AnimationUpr.m_cachedBSPACES.clear();
}

#endif

i32 CharacterUpr::LoadCDF(tukk pathname)
{
	LOADING_TIME_PROFILE_SECTION_ARGS(pathname);
	XmlNodeRef root = g_pISystem->LoadXmlFromFile(pathname);
	if (root == 0)
	{
		g_pILog->LogError("DinrusXAnimation: failed to parse XML file: %s", pathname);
		return INVALID_CDF_ID;
	}

	return LoadCDFFromXML(root, pathname);
}

i32 CharacterUpr::LoadCDFFromXML(XmlNodeRef root, tukk pathname)
{
	LOADING_TIME_PROFILE_SECTION(g_pISystem);

	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_CDF, EMemStatContextFlags::MSF_Instance, "%s", pathname);

	DRX_DEFINE_ASSET_SCOPE("CDF", pathname);

	CharacterDefinition def;
	def.m_strFilePath = pathname;

	//-----------------------------------------------------------
	//load model
	//-----------------------------------------------------------
	u32 numChilds = root->getChildCount();
	if (numChilds == 0)
		return INVALID_CDF_ID;

	for (u32 xmlnode = 0; xmlnode < numChilds; xmlnode++)
	{
		XmlNodeRef node = root->getChild(xmlnode);
		tukk tag = node->getTag();

		//-----------------------------------------------------------
		//load base model
		//-----------------------------------------------------------
		if (strcmp(tag, "Model") == 0)
		{
			def.m_strBaseModelFilePath = node->getAttr("File");

			if (node->haveAttr("Material"))
			{
				tukk material = node->getAttr("Material");
				def.m_pBaseModelMaterial = g_pISystem->GetI3DEngine()->GetMaterialUpr()->LoadMaterial(material, false);
			}

			node->getAttr("KeepModelsInMemory", def.m_nKeepModelsInMemory);
		}

		//-----------------------------------------------------------
		//load attachment-list
		//-----------------------------------------------------------
		if (strcmp(tag, "AttachmentList") == 0)
		{
			u32 numChildren = node->getChildCount();
			if (numChildren)
			{
				def.m_arrAttachments.resize(numChildren);
				u32 numValidAttachments = CAttachmentUpr::ParseXMLAttachmentList(&def.m_arrAttachments[0], numChildren, node);
				def.m_arrAttachments.resize(numValidAttachments);
			}
		}

		def.m_pPoseModifierSetup = CPoseModifierSetup::CreateClassInstance();
		if (def.m_pPoseModifierSetup)
		{
			Serialization::LoadXmlNode(*def.m_pPoseModifierSetup, root);
		}
	}
	m_arrCacheForCDF.push_back(def);
	return m_arrCacheForCDF.size() - 1;
}

ICharacterInstance* CharacterUpr::LoadCharacterDefinition(const string pathname, u32 nLoadingFlags)
{
	LOADING_TIME_PROFILE_SECTION_ARGS(pathname.c_str());
	DRX_DEFINE_ASSET_SCOPE("CDF", pathname.c_str());

	u32 nLogWarnings = (nLoadingFlags & CA_DisableLogWarnings) == 0;

	CCharInstance* pCharInstance = NULL;

	//	__sgi_alloc::get_wasted_in_blocks();
	if (Console::GetInst().ca_MemoryUsageLog)
	{
		DrxModuleMemoryInfo info;
		DrxGetMemoryInfoForModule(&info);
		g_pILog->UpdateLoadingScreen("CDF %s. Start. Memstat %i", pathname.c_str(), (i32)(info.allocated - info.freed));
	}

	u32k cdfId = GetOrLoadCDFId(pathname);
	if (cdfId == INVALID_CDF_ID)
		return NULL;

	m_arrCacheForCDF[cdfId].AddRef();
	u32k numRefs = m_arrCacheForCDF[cdfId].m_nRefCounter;

	// It is a bad idea to store reference to m_cdfFiles[ind], since this vector
	// is going to be changed in nested call to LoadCharacterDefinition / LoadCDF
	u32 nSizeOfPath = m_arrCacheForCDF[cdfId].m_strBaseModelFilePath.size();
	if (nSizeOfPath)
	{
		tukk pFilepathSKEL = m_arrCacheForCDF[cdfId].m_strBaseModelFilePath.c_str();
		i32 nKeepModelInMemory = m_arrCacheForCDF[cdfId].m_nKeepModelsInMemory;
		bool IsBaseSKEL = (0 == stricmp(PathUtil::GetExt(pFilepathSKEL), DRX_SKEL_FILE_EXT));
		bool IsBaseCGA = (0 == stricmp(PathUtil::GetExt(pFilepathSKEL), DRX_ANIM_GEOMETRY_FILE_EXT));
		if (IsBaseSKEL == 0 && IsBaseCGA == 0 && nLogWarnings && !gEnv->IsEditor())
			DrxFatalError("DinrusXAnimation: base-character must be a DRX_SKEL_FILE_EXT or a CGA: %s", pathname.c_str());

		pCharInstance = static_cast<CCharInstance*>(CreateInstance(pFilepathSKEL, nLoadingFlags));
		if (!pCharInstance)
		{
			if (nLogWarnings)
			{
				g_pILog->LogError("Couldn't create base-character: %s", pFilepathSKEL);
			}
			return nullptr;
		}

		if (m_arrCacheForCDF[cdfId].m_pBaseModelMaterial)
			pCharInstance->SetIMaterial_Instance(m_arrCacheForCDF[cdfId].m_pBaseModelMaterial);

		if (nKeepModelInMemory == 0 || nKeepModelInMemory == 1)
		{
			CDefaultSkeleton* pDefaultSkeleton = pCharInstance->m_pDefaultSkeleton;
			if (nKeepModelInMemory)
				pDefaultSkeleton->SetKeepInMemory(true);
			else
				pDefaultSkeleton->SetKeepInMemory(false);
		}

		pCharInstance->SetFilePath(pathname); //store the CDF-pathname inside the instance

		SkelExtension(pCharInstance, pFilepathSKEL, cdfId, nLoadingFlags);

		//-----------------------------------------------------------
		//init attachment-list
		u32 numLoadedAttachments = m_arrCacheForCDF.size();
		if (numLoadedAttachments)
		{
			CAttachmentUpr* pAttachmentUpr = (CAttachmentUpr*)pCharInstance->GetIAttachmentUpr();
			pAttachmentUpr->InitAttachmentList(m_arrCacheForCDF[cdfId].m_arrAttachments.size() > 0 ? &m_arrCacheForCDF[cdfId].m_arrAttachments[0] : NULL, m_arrCacheForCDF[cdfId].m_arrAttachments.size(), pathname, nLoadingFlags, nKeepModelInMemory);
		}

		CharacterDefinition& cdf = m_arrCacheForCDF[cdfId];
		if (cdf.m_pPoseModifierSetup)
		{
			CPoseModifierSetupPtr& pPoseModifierSetup = ((CSkeletonAnim*)pCharInstance->GetISkeletonAnim())->m_pPoseModifierSetup;
			if (!pPoseModifierSetup)
				pPoseModifierSetup = CPoseModifierSetup::CreateClassInstance();
			if (pPoseModifierSetup)
				cdf.m_pPoseModifierSetup->Create(*pPoseModifierSetup.get());
		}
	}

	if (Console::GetInst().ca_MemoryUsageLog)
	{
		DrxModuleMemoryInfo info;
		DrxGetMemoryInfoForModule(&info);
		g_pILog->UpdateLoadingScreen("CDF. Finish. Memstat %i", (i32)(info.allocated - info.freed));
	}

	return pCharInstance;
}

//----------------------------------------------------------------------------------

void CharacterUpr::SkelExtension(CCharInstance* pCharInstance, tukk pFilepathSKEL, u32k cdfId, u32k nLoadingFlags)
{
	CDefaultSkeleton* const pDefaultSkeleton = pCharInstance->m_pDefaultSkeleton;

	std::vector<tukk > mismatchingSkins;
	uint64 nExtendedCRC64 = CCrc32::ComputeLowercase(pFilepathSKEL);

	for (auto && attachment : m_arrCacheForCDF[cdfId].m_arrAttachments)
	{
		if (attachment.m_Type != CA_SKIN)
		{
			continue;
		}

		tukk const szSkinPath = attachment.m_strBindingPath.c_str();
		if (!szSkinPath)
		{
			continue;
		}

		CSkin* const pModelSKIN = static_cast<CSkin*>(LoadModelSKIN(szSkinPath, nLoadingFlags));
		if (!pModelSKIN)
		{
			continue;
		}

		u32k mismatchingJointsCount = CompatibilityTest(pDefaultSkeleton, pModelSKIN);
		if (mismatchingJointsCount > 0)
		{
			mismatchingSkins.push_back(szSkinPath);
		}

		nExtendedCRC64 += CCrc32::ComputeLowercase(szSkinPath);
	}

	if (!mismatchingSkins.empty())
	{
		CDefaultSkeleton* pExtDefaultSkeleton = CheckIfModelExtSKELCreated(nExtendedCRC64, nLoadingFlags);
		if (!pExtDefaultSkeleton)
		{
			pExtDefaultSkeleton = CreateExtendedSkel(pCharInstance, pDefaultSkeleton, nExtendedCRC64, mismatchingSkins, nLoadingFlags);
		}
		if (pExtDefaultSkeleton)
		{
			pDefaultSkeleton->SetKeepInMemory(true);
			pCharInstance->RuntimeInit(pExtDefaultSkeleton);
		}
	}
}

void CharacterUpr::ExtendDefaultSkeletonWithSkinAttachments(ICharacterInstance* pCharInst, tukk szFilepathSKEL, tukk* szSkinAttachments, u32k skinsCount, u32k nLoadingFlags)
{
	CCharInstance* pCharInstance = static_cast<CCharInstance*>(pCharInst);
	CDefaultSkeleton* const pDefaultSkeleton = pCharInstance->m_pDefaultSkeleton;

	std::vector<tukk > mismatchingSkins;
	uint64 nExtendedCRC64 = CCrc32::ComputeLowercase(szFilepathSKEL);

	for (u32 i = 0; i < skinsCount; ++i)
	{
		tukk const szSkinPath = szSkinAttachments[i];

		CSkin* const pModelSKIN = static_cast<CSkin*>(LoadModelSKIN(szSkinPath, nLoadingFlags));
		if (!pModelSKIN)
		{
			continue;
		}

		u32k mismatchingJointsCount = CompatibilityTest(pDefaultSkeleton, pModelSKIN);
		if (mismatchingJointsCount > 0)
		{
			mismatchingSkins.push_back(szSkinPath);
		}

		nExtendedCRC64 += CCrc32::ComputeLowercase(szSkinPath);
	}

	if (!mismatchingSkins.empty())
	{
		CDefaultSkeleton* pExtDefaultSkeleton = CheckIfModelExtSKELCreated(nExtendedCRC64, nLoadingFlags);
		if (!pExtDefaultSkeleton)
		{
			pExtDefaultSkeleton = CreateExtendedSkel(pCharInstance, pDefaultSkeleton, nExtendedCRC64, mismatchingSkins, nLoadingFlags);
		}
		if (pExtDefaultSkeleton)
		{
			pDefaultSkeleton->SetKeepInMemory(true);
			pCharInstance->RuntimeInit(pExtDefaultSkeleton);
		}
	}
}

u32 CharacterUpr::CompatibilityTest(CDefaultSkeleton* pCDefaultSkeleton, CSkin* pCSkinModel)
{
	u32 numJointsSkel = pCDefaultSkeleton->m_arrModelJoints.size();
	u32 numJointsSkin = pCSkinModel->m_arrModelJoints.size();
	if (numJointsSkin > numJointsSkel)
		return numJointsSkin - numJointsSkel;
	u32 NotMatchingNames = 0;
	for (u32 js = 0; js < numJointsSkin; js++)
	{
		u32k sHash = pCSkinModel->m_arrModelJoints[js].m_nJointCRC32Lower;
		i32k jid = pCDefaultSkeleton->GetJointIDByCRC32(sHash);
		NotMatchingNames += jid == -1;         //Bone-names of SLAVE-skeleton don't match with bone-names of MASTER-skeleton
	}
	return NotMatchingNames;
}

static struct
{
	struct SNode
	{
		QuatT          m_DefaultAbsolute;
		tukk    m_strJointName;
		u32         m_nJointCRC32Lower; // case insensitive CRC32 of joint-name. Used to match joint name in skeleton/skin files, but NOT controller names in CAF.
		u32         m_nCRC32Parent;
		i32          m_idxParent;        //index of parent-joint. if the idx==-1 then this joint is the root. Usually this values are > 0
		i32          m_idxNext;          //sibling of this joint
		i32          m_idxFirst;         //first child of this joint
		f32            m_fMass;
		DrxBonePhysics m_PhysInfo;
	};

	//find a joint in the hierarchy by crc32
	i32 find(u32 numJoints, u32 nCRC32)
	{
		for (u32 j = 0; j < numJoints; j++)
			if (m_arrHierarchy[j].m_nJointCRC32Lower == nCRC32) return j;
		return -1;
	}

	void parse(i32 idx, SNode* pModelJoints, u32 numJoints)
	{
		assert(m_cidx != numJoints);
		pModelJoints[m_cidx++] = m_arrHierarchy[idx];
		i32 s = m_arrHierarchy[idx].m_idxNext;
		if (s) parse(s, pModelJoints, numJoints);
		i32 c = m_arrHierarchy[idx].m_idxFirst;
		if (c) parse(c, pModelJoints, numJoints);
	}

	void insert(u32 numJoints, u32 j, const CSkin::SJointInfo* pSkinJoints)
	{
		i32 p = pSkinJoints[j].m_idxParent;
		if (p < 0)
			DrxFatalError("ModelError: inserting root joint");
		tukk pParentJoint = pSkinJoints[p].m_NameModelSkin.c_str();
		i32 np = -1;
		m_arrHierarchy[numJoints].m_nCRC32Parent = pSkinJoints[p].m_nJointCRC32Lower;
		for (i32 b = (numJoints - 1); b > -1; b--)
			if (pSkinJoints[p].m_nJointCRC32Lower == m_arrHierarchy[b].m_nJointCRC32Lower) { np = b; break; }
		if (np < 0)
			DrxFatalError("ModelError: parent joint not found: %s", pParentJoint);
		m_arrHierarchy[numJoints].m_DefaultAbsolute = pSkinJoints[j].m_DefaultAbsolute;
		m_arrHierarchy[numJoints].m_idxParent = np;
		m_arrHierarchy[numJoints].m_idxNext = 0;
		m_arrHierarchy[numJoints].m_idxFirst = 0;
		m_arrHierarchy[numJoints].m_fMass = 0;
		m_arrHierarchy[numJoints].m_PhysInfo.pPhysGeom = 0;
		m_arrHierarchy[numJoints].m_strJointName = pSkinJoints[j].m_NameModelSkin.c_str();
		m_arrHierarchy[numJoints].m_nJointCRC32Lower = pSkinJoints[j].m_nJointCRC32Lower;
	}

	//use the hierarchy to build a new linearized skeleton
	void rebuild(u32 numJoints)
	{
		memset(m_arrNumChildren, 0, sizeof(m_arrNumChildren));
		for (u32 j = 1; j < numJoints; j++)
			if (m_arrHierarchy[m_arrHierarchy[j].m_idxParent].m_idxFirst == 0) m_arrHierarchy[m_arrHierarchy[j].m_idxParent].m_idxFirst = j;
		for (u32 p, j = 1; j < numJoints; j++)
			p = m_arrHierarchy[j].m_idxParent, m_arrIdxChildren[p * MAX_JOINT_AMOUNT + m_arrNumChildren[p]++] = j;
		for (u32 p = 0; p < numJoints; p++)
			for (u32 s = 1; s < m_arrNumChildren[p]; s++)
				m_arrHierarchy[m_arrIdxChildren[p * MAX_JOINT_AMOUNT + s - 1]].m_idxNext = m_arrIdxChildren[p * MAX_JOINT_AMOUNT + s];
		m_cidx = 0;
		parse(0, &m_arrExtModelJoints[0], numJoints);
		if (m_cidx != numJoints)
			DrxFatalError("ModelError: joint count not identical");
		for (u32 j = 1; j < numJoints; j++)
		{
			i32 p = -1, pcrc32 = m_arrExtModelJoints[j].m_nCRC32Parent;
			for (i32 i = i32(j - 1); i > -1; i--)
				if (pcrc32 == m_arrExtModelJoints[i].m_nJointCRC32Lower) { p = i; break; }
			if (p < 0)
				DrxFatalError("ModelError: parent joint not found");
			m_arrExtModelJoints[j].m_idxParent = p;
		}
	}

	u32 m_cidx;
	SNode  m_arrHierarchy[MAX_JOINT_AMOUNT * 2];
	u16 m_arrNumChildren[MAX_JOINT_AMOUNT];
	u16 m_arrIdxChildren[MAX_JOINT_AMOUNT * MAX_JOINT_AMOUNT];
	SNode  m_arrExtModelJoints[MAX_JOINT_AMOUNT * 2]; // This is the new extended skeleton
} lh;

CDefaultSkeleton* CharacterUpr::CreateExtendedSkel(CCharInstance* pCharInstance, CDefaultSkeleton* pDefaultSkeleton, uint64 nExtendedCRC64, const std::vector<tukk >& mismatchingSkins, u32k nLoadingFlags)
{
	tukk pFilepathSKEL = pDefaultSkeleton->GetModelFilePath();

	stack_string tmp = pFilepathSKEL;
	stack_string strextended = "_" + tmp;
	CDefaultSkeleton* pExtDefaultSkeleton = new CDefaultSkeleton(strextended.c_str(), CHR, nExtendedCRC64);
	pExtDefaultSkeleton->SetModelAnimEventDatabase(pDefaultSkeleton->GetModelAnimEventDatabaseCStr());
	RegisterModelSKEL(pExtDefaultSkeleton, nLoadingFlags);

	//create a real hierarchy
	u32 numJoints = pDefaultSkeleton->m_arrModelJoints.size();
	for (u32 j = 0; j < numJoints; j++)
	{
		i32 p = pDefaultSkeleton->m_arrModelJoints[j].m_idxParent;
		lh.m_arrHierarchy[j].m_DefaultAbsolute = pDefaultSkeleton->m_poseDefaultData.GetJointsAbsolute()[j];
		lh.m_arrHierarchy[j].m_idxParent = p;
		lh.m_arrHierarchy[j].m_idxNext = 0;
		lh.m_arrHierarchy[j].m_idxFirst = 0;
		lh.m_arrHierarchy[j].m_fMass = pDefaultSkeleton->m_arrModelJoints[j].m_fMass;
		lh.m_arrHierarchy[j].m_PhysInfo = pDefaultSkeleton->m_arrBackupPhysInfo[j];
		lh.m_arrHierarchy[j].m_strJointName = pDefaultSkeleton->m_arrModelJoints[j].m_strJointName;
		lh.m_arrHierarchy[j].m_nJointCRC32Lower = pDefaultSkeleton->m_arrModelJoints[j].m_nJointCRC32Lower;
		lh.m_arrHierarchy[j].m_nCRC32Parent = p < 0 ? 0 : pDefaultSkeleton->m_arrModelJoints[p].m_nJointCRC32Lower;
	}

	u32 nExtensionNeeded = mismatchingSkins.size();
	for (u32 e = 0; e < nExtensionNeeded; e++)
	{
		tukk ppath = mismatchingSkins[e];
		const CSkin* pCModelSKIN = CheckIfModelSKINLoaded(ppath, nLoadingFlags);
		if (pCModelSKIN == 0)
			DrxFatalError("ModelError: skin not loaded");
		PREFAST_ASSUME(pCModelSKIN);
		const CSkin::SJointInfo* parrModelJoints = &pCModelSKIN->m_arrModelJoints[0];
		if (lh.find(numJoints, parrModelJoints->m_nJointCRC32Lower) < 0)
		{
			DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_ERROR, "DinrusXAnimation: Skin root joint '%s' was not found in skeleton (with root '%s'), while attaching skin '%s' to skeleton '%s'.",
			           parrModelJoints->m_NameModelSkin.c_str(), lh.m_arrHierarchy[0].m_strJointName, pCModelSKIN->GetModelFilePath(), pDefaultSkeleton->GetModelFilePath());
			lh.m_arrHierarchy[numJoints].m_DefaultAbsolute = parrModelJoints->m_DefaultAbsolute;
			lh.m_arrHierarchy[numJoints].m_idxParent = 0;
			lh.m_arrHierarchy[numJoints].m_idxNext = 0;
			lh.m_arrHierarchy[numJoints].m_idxFirst = 0;
			lh.m_arrHierarchy[numJoints].m_fMass = 0;
			lh.m_arrHierarchy[numJoints].m_PhysInfo.pPhysGeom = 0;
			lh.m_arrHierarchy[numJoints].m_strJointName = parrModelJoints->m_NameModelSkin;
			lh.m_arrHierarchy[numJoints].m_nJointCRC32Lower = parrModelJoints->m_nJointCRC32Lower;
			lh.m_arrHierarchy[numJoints].m_nCRC32Parent = pDefaultSkeleton->m_arrModelJoints[0].m_nJointCRC32Lower;
			numJoints++;
		}
		u32 numSkinJoints = pCModelSKIN->m_arrModelJoints.size();
		for (u32 s = 0; s < numSkinJoints; s++)
		{
			tukk pskinname = parrModelJoints[s].m_NameModelSkin.c_str();
			u32 found = 0, scrc32 = parrModelJoints[s].m_nJointCRC32Lower;
			for (u32 m = 0; m < numJoints; m++)
				if (scrc32 == lh.m_arrHierarchy[m].m_nJointCRC32Lower)  { found = 1; break; }
			if (found == 0)
				lh.insert(numJoints++, s, parrModelJoints);
		}
		if (numJoints >= MAX_JOINT_AMOUNT)
			DrxFatalError("ModelError: bone count over limit");
	}

	//rebuild the linear skeleton from the hierarchy
	lh.rebuild(numJoints);
	pExtDefaultSkeleton->m_arrModelJoints.resize(numJoints);
	pExtDefaultSkeleton->m_poseDefaultData.Initialize(numJoints);
	pExtDefaultSkeleton->m_arrModelJoints[0].m_idxParent = lh.m_arrExtModelJoints[0].m_idxParent;
	pExtDefaultSkeleton->m_arrModelJoints[0].m_strJointName = lh.m_arrExtModelJoints[0].m_strJointName;
	pExtDefaultSkeleton->m_arrModelJoints[0].m_nJointCRC32Lower = lh.m_arrExtModelJoints[0].m_nJointCRC32Lower;
	pExtDefaultSkeleton->m_arrModelJoints[0].m_nJointCRC32 = CCrc32::Compute(lh.m_arrExtModelJoints[0].m_strJointName);
	pExtDefaultSkeleton->m_arrModelJoints[0].m_PhysInfo = lh.m_arrExtModelJoints[0].m_PhysInfo;
	pExtDefaultSkeleton->m_arrModelJoints[0].m_fMass = lh.m_arrExtModelJoints[0].m_fMass;
	pExtDefaultSkeleton->m_poseDefaultData.GetJointsRelative()[0] = lh.m_arrExtModelJoints[0].m_DefaultAbsolute;
	pExtDefaultSkeleton->m_poseDefaultData.GetJointsAbsolute()[0] = lh.m_arrExtModelJoints[0].m_DefaultAbsolute;
	for (u32 i = 1; i < numJoints; i++)
	{
		i32 p = lh.m_arrExtModelJoints[i].m_idxParent;
		pExtDefaultSkeleton->m_arrModelJoints[i].m_idxParent = p;
		pExtDefaultSkeleton->m_arrModelJoints[i].m_strJointName = lh.m_arrExtModelJoints[i].m_strJointName;
		pExtDefaultSkeleton->m_arrModelJoints[i].m_nJointCRC32Lower = lh.m_arrExtModelJoints[i].m_nJointCRC32Lower;
		pExtDefaultSkeleton->m_arrModelJoints[i].m_nJointCRC32 = CCrc32::Compute(lh.m_arrExtModelJoints[i].m_strJointName);
		pExtDefaultSkeleton->m_arrModelJoints[i].m_PhysInfo = lh.m_arrExtModelJoints[i].m_PhysInfo;
		pExtDefaultSkeleton->m_arrModelJoints[i].m_fMass = lh.m_arrExtModelJoints[i].m_fMass;
		pExtDefaultSkeleton->m_poseDefaultData.GetJointsAbsolute()[i] = lh.m_arrExtModelJoints[i].m_DefaultAbsolute;
		pExtDefaultSkeleton->m_poseDefaultData.GetJointsRelative()[i] = pExtDefaultSkeleton->m_poseDefaultData.GetJointsAbsolute()[p].GetInverted() * lh.m_arrExtModelJoints[i].m_DefaultAbsolute;
	}
	pExtDefaultSkeleton->RebuildJointLookupCaches();
	pExtDefaultSkeleton->CopyAndAdjustSkeletonParams(pDefaultSkeleton);
	pExtDefaultSkeleton->SetupPhysicalProxies(pDefaultSkeleton->m_arrBackupPhyBoneMeshes, pDefaultSkeleton->m_arrBackupBoneEntities, pDefaultSkeleton->GetIMaterial(), pFilepathSKEL);
	pExtDefaultSkeleton->VerifyHierarchy();
	return pExtDefaultSkeleton;
}

void CharacterUpr::ReleaseCDF(tukk pathname)
{
	u32 ind = INVALID_CDF_ID;
	u32 cdfCount = m_arrCacheForCDF.size();
	for (u32 i = 0; i < cdfCount; i++)
	{
		if (m_arrCacheForCDF[i].m_strFilePath.compareNoCase(pathname) == 0)
		{
			ind = i;
			break;
		}
	}

	if (ind != INVALID_CDF_ID)
	{
		//found CDF-name
		assert(m_arrCacheForCDF[ind].m_nRefCounter);
		m_arrCacheForCDF[ind].m_nRefCounter--;
		if (m_arrCacheForCDF[ind].m_nRefCounter == 0 && m_arrCacheForCDF[ind].m_nKeepInMemory == 0)
		{
			for (u32 i = ind + 1; i < cdfCount; i++)
				m_arrCacheForCDF[i - 1] = m_arrCacheForCDF[i];
			m_arrCacheForCDF.pop_back();
		}
	}

}

u32 CharacterUpr::GetCDFId(const string& pathname)
{
	u32 cdfId = INVALID_CDF_ID;
	u32 cdfCount = m_arrCacheForCDF.size();
	for (u32 i = 0; i < cdfCount; i++)
	{
		if (m_arrCacheForCDF[i].m_strFilePath.compareNoCase(pathname) == 0)
		{
			cdfId = i;
			break;
		}
	}

	return cdfId;
}

u32 CharacterUpr::GetOrLoadCDFId(const string& pathname)
{
	u32 cdfId = GetCDFId(pathname);

	if (cdfId == INVALID_CDF_ID)
	{
		cdfId = LoadCDF(pathname);
	}

	if (cdfId == INVALID_CDF_ID)
	{
		DrxLogAlways("DinrusXAnimation: character-definition not found: %s", pathname.c_str());
	}

	return cdfId;
}

//////////////////////////////////////////////////////////////////////////
void CharacterUpr::GetLoadedModels(IDefaultSkeleton** pIDefaultSkeletons, u32& nCount) const
{
	if (pIDefaultSkeletons == 0)
	{
		nCount = m_arrModelCacheSKEL.size();
		return;
	}
	u32 numDefaultSkeletons = min(u32(m_arrModelCacheSKEL.size()), nCount);
	for (u32 i = 0; i < nCount; i++)
		pIDefaultSkeletons[i] = m_arrModelCacheSKEL[i].m_pDefaultSkeleton;
}

//////////////////////////////////////////////////////////////////////////
void CharacterUpr::ReloadAllModels()
{
	//Created too many mem-leaks. Deactivated for now
}

void CharacterUpr::SyncAllAnimations()
{
	DRX_PROFILE_REGION(PROFILE_ANIMATION, "CharacterUpr::SyncAllAnimations");
	DRXPROFILE_SCOPE_PROFILE_MARKER("CharacterUpr::SyncAllAnimations");
	ANIMATION_LIGHT_PROFILER();

	m_ContextSyncQueue.ExecuteForAll(
	  [](CharacterInstanceProcessing::SContext& ctx)
	{
		ctx.job.Wait();
		return CharacterInstanceProcessing::SFinishAnimationComputations()(ctx);
	});

	m_nActiveCharactersLastFrame = m_ContextSyncQueue.GetNumberOfContexts();
	m_ContextSyncQueue.ClearContexts();

	if (Console::GetInst().ca_MemoryDefragEnabled)
		g_controllerHeap.Update();
}

void CharacterUpr::ClearPoseModifiersFromSynchQueue()
{
	u32 count = u32(m_ContextSyncQueue.GetNumberOfContexts());
	for (u32 i = 0; i < count; ++i)
	{
		CharacterInstanceProcessing::SContext& ctx = m_ContextSyncQueue.GetContext(i);
		CSkeletonPose* pSkeletonPose = (CSkeletonPose*)ctx.pInstance->GetISkeletonPose();
		CSkeletonAnim* pSkeletonAnim = (CSkeletonAnim*)ctx.pInstance->GetISkeletonAnim();
		for (uint j = 0; j < numVIRTUALLAYERS; ++j)
		{
			pSkeletonAnim->m_layers[j].m_poseModifierQueue.ClearAllPoseModifiers();
		}
		pSkeletonAnim->m_poseModifierQueue.ClearAllPoseModifiers();
		pSkeletonAnim->m_transformPinningPoseModifier.reset();
	}
}

//////////////////////////////////////////////////////////////////////////
void CharacterUpr::UpdateInstances(bool bPause)
{
	if (bPause)
		return;

	// Go through all registered character instances and check if they need to be updated.
	DRX_PROFILE_FUNCTION(PROFILE_ANIMATION)

	for (auto& modelRef : m_arrModelCacheSKEL)
	{
		for (CCharInstance* pCharInstance : modelRef.m_RefByInstances)
		{
			pCharInstance->PerFrameUpdate();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CharacterUpr::LoadAnimationImageFile(tukk filenameCAF, tukk filenameAIM)
{
	LOADING_TIME_PROFILE_SECTION;

	if (m_InitializedByIMG)
	{
		return;
	}

	m_InitializedByIMG = 1;

	g_pI3DEngine = g_pISystem->GetI3DEngine();   // TODO: Why is this initialization perfomed here?

	LoadAnimationImageFileCAF(filenameCAF);
	LoadAnimationImageFileAIM(filenameAIM);
}

//////////////////////////////////////////////////////////////////////////
bool CharacterUpr::LoadAnimationImageFileCAF(tukk filenameCAF)
{
	LOADING_TIME_PROFILE_SECTION;

	if (Console::GetInst().ca_UseIMG_CAF == 0)
	{
		// This in just for the "Production-Mode" to avoid to resize the a array at loading-time and runtime
		// In "Release-Mode" we have an IMG file which tells us the exact amount of assets
		g_AnimationUpr.m_arrGlobalCAF.reserve(NUM_CAF_FILES);
		return false;
	}

	stack_string strPath = filenameCAF;
	PathUtil::UnifyFilePath(strPath);

	_smart_ptr<IChunkFile> pChunkFile = g_pI3DEngine->CreateChunkFile(true);
	if (!pChunkFile->Read(strPath))
	{
		pChunkFile->GetLastError();
		return false;
	}

	u32k numHeaders = g_AnimationUpr.m_arrGlobalCAF.size();
	if (numHeaders > 0)
	{
		DrxFatalError("DinrusXAnimation: the list with GlobalCAFs is already initialized");
	}

	u32k numChunk = pChunkFile->NumChunks();   //the max amount of CAF files is precomputed in the RC
	g_AnimationUpr.m_arrGlobalCAF.reserve(numChunk + APX_NUM_OF_CGA_ANIMATIONS);
	g_AnimationUpr.m_arrGlobalCAF.resize(numChunk);

	for (u32 i = 0; i < numChunk; ++i)
	{
		const IChunkFile::ChunkDesc* const pChunkDesc = pChunkFile->GetChunk(i);

		if (pChunkDesc->chunkType != ChunkType_GlobalAnimationHeaderCAF || pChunkDesc->chunkVersion != CHUNK_GAHCAF_INFO::VERSION)
		{
			g_AnimationUpr.m_arrGlobalCAF.clear();
			return false;
		}

		if (pChunkDesc->bSwapEndian)
		{
			DrxFatalError("%s: data are stored in non-native endian format", __FUNCTION__);
		}

		const CHUNK_GAHCAF_INFO* const pChunk = static_cast<const CHUNK_GAHCAF_INFO*>(pChunkDesc->data);

		static u32k nValidFlags = (CA_ASSET_BIG_ENDIAN | CA_ASSET_CREATED | CA_ASSET_ONDEMAND | CA_ASSET_LOADED | CA_ASSET_ADDITIVE | CA_ASSET_CYCLE);
		if (pChunk->m_Flags & ~nValidFlags)
		{
			DrxWarning(VALIDATOR_MODULE_3DENGINE, VALIDATOR_WARNING, "Badly exported animation-asset: flags: %08x %s", pChunk->m_Flags, pChunk->m_FilePath);
		}

		if (pChunk->m_FilePathCRC32 != CCrc32::ComputeLowercase(pChunk->m_FilePath))
		{
			DrxFatalError("DinrusXAnimation: CRC32 Invalid! Most likely the endian conversion from RC is wrong");
		}

		if (pChunk->m_nControllers == 0)
		{
			DrxWarning(VALIDATOR_MODULE_3DENGINE, VALIDATOR_WARNING, "DinrusXAnimation: Assets has no controllers. Probably compressed to death: %s", pChunk->m_FilePath);
		}

		GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[i];
		rCAF.SetFlags(pChunk->m_Flags & nValidFlags);
		rCAF.m_FilePathDBACRC32 = pChunk->m_FilePathDBACRC32;                        // TODO: investigate this
		rCAF.SetFilePath(pChunk->m_FilePath);
		rCAF.m_fStartSec = pChunk->m_fStartSec;
		rCAF.m_fEndSec = pChunk->m_fEndSec;
		rCAF.m_fTotalDuration = pChunk->m_fTotalDuration;
		rCAF.m_StartLocation = pChunk->m_StartLocation;                            // asset-feature: the original location of the animation in world-space
		rCAF.m_nControllers = 0;
		rCAF.m_nControllers2 = pChunk->m_nControllers;

		assert(rCAF.GetFilePathCRC32() == pChunk->m_FilePathCRC32);

		if (rCAF.m_arrController.size() > 0)
		{
			DrxFatalError("DinrusXAnimation: m_arrController ready initialized. possible mem-leak");
		}

		g_AnimationUpr.m_animSearchHelper.AddAnimation(pChunk->m_FilePath, i);  // TODO: investigate this, looks like a duplicated feature
		m_AnimationUpr.m_AnimationMapCAF.InsertValue(rCAF.GetFilePathCRC32(), i);

		rCAF.OnAssetCreated();
	}

	m_InitializedByIMG |= 2;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CharacterUpr::LoadAnimationImageFileAIM(tukk filenameAIM)
{
	if (Console::GetInst().ca_UseIMG_AIM == 0)
	{
		// This in just for the "Production-Mode" to avoid to resize the a array at loading-time and runtime
		// In "Release-Mode" we have an IMG file which tells us the exact amount of assets
		g_AnimationUpr.m_arrGlobalAIM.reserve(NUM_AIM_FILES);
		return false;
	}

	stack_string strPath = filenameAIM;
	PathUtil::UnifyFilePath(strPath);

	_smart_ptr<IChunkFile> pChunkFile = g_pI3DEngine->CreateChunkFile(true);
	if (!pChunkFile->Read(strPath))
	{
		pChunkFile->GetLastError();                                                                         // TODO: Is it necessary to call this method here?
		return false;
	}

	u32k chunkCount = pChunkFile->NumChunks();
	g_AnimationUpr.m_arrGlobalAIM.resize(chunkCount);
	for (u32 i = 0; i < chunkCount; ++i)
	{
		const IChunkFile::ChunkDesc* const pChunkDesc = pChunkFile->GetChunk(i);

		if (pChunkDesc->chunkType != ChunkType_GlobalAnimationHeaderAIM || pChunkDesc->chunkVersion != CHUNK_GAHAIM_INFO::VERSION)
		{
			// TODO: Issue w warning.
			g_AnimationUpr.m_arrGlobalAIM.clear();
			return false;                                                                                     // TODO: Should this even be an unrecoverable case? We can't we simply ignore unknown chunk types?
		}

		if (pChunkDesc->bSwapEndian)
		{
			DrxFatalError("%s: data are stored in non-native endian format", __FUNCTION__);                    // TODO: Replace fatal error with graceful asset validation.
		}

		const CHUNK_GAHAIM_INFO* const pChunk = (const CHUNK_GAHAIM_INFO*)pChunkDesc->data;

		static u32k nValidFlags = (CA_ASSET_BIG_ENDIAN | CA_ASSET_LOADED | CA_ASSET_CREATED | CA_AIMPOSE);
		if (pChunk->m_Flags & ~nValidFlags)
		{
			DrxWarning(VALIDATOR_MODULE_3DENGINE, VALIDATOR_WARNING, "Badly exported aim-pose: flags: %08x %s", pChunk->m_Flags, pChunk->m_FilePath);
		}

		if (pChunk->m_FilePathCRC32 != CCrc32::ComputeLowercase(pChunk->m_FilePath)) // TODO: Data duplication, could be removed
		{
			DrxFatalError("DinrusXAnimation: CRC32 Invalid! Most likely the endian conversion from RC is wrong"); // TODO: Replace fatal error with graceful asset validation.
		}

		GlobalAnimationHeaderAIM& rAIM = g_AnimationUpr.m_arrGlobalAIM[i];
		rAIM.SetFlags(pChunk->m_Flags & nValidFlags);
		rAIM.SetFilePath(pChunk->m_FilePath);
		rAIM.m_fStartSec = pChunk->m_fStartSec;
		rAIM.m_fEndSec = pChunk->m_fEndSec;
		rAIM.m_fTotalDuration = pChunk->m_fTotalDuration;
		rAIM.m_AnimTokenCRC32 = pChunk->m_AnimTokenCRC32;
		rAIM.m_nExist = pChunk->m_nExist;
		rAIM.m_MiddleAimPoseRot = pChunk->m_MiddleAimPoseRot;
		rAIM.m_MiddleAimPose = pChunk->m_MiddleAimPose;

		assert(rAIM.GetFilePathCRC32() == pChunk->m_FilePathCRC32);

		assert(rAIM.m_MiddleAimPoseRot.IsValid());                                                          // TODO: Replace asserts with graceful asset validation
		assert(rAIM.m_MiddleAimPose.IsValid());

		for (u32 v = 0; v < (CHUNK_GAHAIM_INFO::XGRID * CHUNK_GAHAIM_INFO::YGRID); ++v)
		{
			rAIM.m_PolarGrid[v] = pChunk->m_PolarGrid[v];
		}

		tukk pAimPoseMem = reinterpret_cast<tukk >(&pChunk->m_numAimPoses);

		u32k numAimPoses = *reinterpret_cast<u32k*>(pAimPoseMem);
		pAimPoseMem += sizeof(u32);

		rAIM.m_arrAimIKPosesAIM.resize(numAimPoses);
		for (u32 a = 0; a < numAimPoses; ++a)
		{
			u32k numRot = *reinterpret_cast<u32k*>(pAimPoseMem);
			pAimPoseMem += sizeof(u32);

			rAIM.m_arrAimIKPosesAIM[a].m_arrRotation.resize(numRot);
			for (u32 r = 0; r < numRot; ++r)
			{
				const Quat quat = *reinterpret_cast<const Quat*>(pAimPoseMem);
				pAimPoseMem += sizeof(Quat);
				assert(quat.IsValid()); // TODO: Replace asserts with graceful asset validation

				rAIM.m_arrAimIKPosesAIM[a].m_arrRotation[r] = quat;
			}

			u32k numPos = *reinterpret_cast<u32k*>(pAimPoseMem);
			pAimPoseMem += sizeof(u32);

			rAIM.m_arrAimIKPosesAIM[a].m_arrPosition.resize(numPos);
			for (u32 p = 0; p < numPos; ++p)
			{
				const Vec3 pos = *(Vec3*)(pAimPoseMem);
				pAimPoseMem += sizeof(Vec3);
				assert(pos.IsValid());  // TODO: Replace asserts with graceful asset validation

				rAIM.m_arrAimIKPosesAIM[a].m_arrPosition[p] = pos;
			}
		}

		rAIM.OnAssetCreated();
	}

	m_InitializedByIMG |= 4;        // TODO: Get rid of this hack (there's a special case somewhere around which expects an undocumented 0x02 flag).
	return true;
}

bool CharacterUpr::DBA_PreLoad(tukk pFilePathDBA, ICharacterUpr::EStreamingDBAPriority priority)
{
	const bool highPriority = (priority == ICharacterUpr::eStreamingDBAPriority_Urgent);

	return g_AnimationUpr.DBA_PreLoad(pFilePathDBA, highPriority);
}

bool CharacterUpr::DBA_LockStatus(tukk pFilePathDBA, u32 status, ICharacterUpr::EStreamingDBAPriority priority)
{
	const bool highPriority = (priority == ICharacterUpr::eStreamingDBAPriority_Urgent);

	return g_AnimationUpr.DBA_LockStatus(pFilePathDBA, status, highPriority);
}

bool CharacterUpr::DBA_Unload(tukk pFilePathDBA)
{
	m_AllowStartOfAnimation = 0;
	//StopAnimationsOnAllInstances();  //Not necessary! This was stopping animation on all characters.
	g_AnimationUpr.DBA_Unload(pFilePathDBA);
	m_AllowStartOfAnimation = 1;
	return true;
}

bool CharacterUpr::DBA_Unload_All()
{
	m_AllowStartOfAnimation = 0;
	StopAnimationsOnAllInstances();
	g_AnimationUpr.DBA_Unload_All();
	m_AllowStartOfAnimation = 1;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CharacterUpr::CAF_AddRef(u32 filePathCRC)
{
	i32 globalID = g_AnimationUpr.m_AnimationMapCAF.GetValueCRC(filePathCRC);

	// Either the file path crc is invalid or it links to an asset of type AIM or LMG (which we don't cache)
	if (globalID == -1)
	{
		return false;
	}

	return CAF_AddRefByGlobalId(globalID);
}

//////////////////////////////////////////////////////////////////////////
bool CharacterUpr::CAF_IsLoaded(u32 filePathCRC) const
{
	bool bRet = false;

	i32 globalID = g_AnimationUpr.m_AnimationMapCAF.GetValueCRC(filePathCRC);
	DRX_ASSERT((globalID >= 0) && (globalID < g_AnimationUpr.m_arrGlobalCAF.size()));
	if ((globalID >= 0) && (globalID < g_AnimationUpr.m_arrGlobalCAF.size()))
	{
		GlobalAnimationHeaderCAF& headerCAF = g_AnimationUpr.m_arrGlobalCAF[globalID];

		bRet = (headerCAF.IsAssetLoaded() != 0);
	}

	return bRet;
}

//////////////////////////////////////////////////////////////////////////
bool CharacterUpr::CAF_Release(u32 filePathCRC)
{
	i32 globalID = g_AnimationUpr.m_AnimationMapCAF.GetValueCRC(filePathCRC);

	return CAF_ReleaseByGlobalId(globalID);
}

//////////////////////////////////////////////////////////////////////////
bool CharacterUpr::CAF_AddRefByGlobalId(i32 globalID)
{
	bool bRet = false;

	DRX_ASSERT((globalID >= 0) && (globalID < g_AnimationUpr.m_arrGlobalCAF.size()));
	if ((globalID >= 0) && (globalID < g_AnimationUpr.m_arrGlobalCAF.size()))
	{
		GlobalAnimationHeaderCAF& headerCAF = g_AnimationUpr.m_arrGlobalCAF[globalID];
		if (!headerCAF.IsAssetLoaded())
		{
			headerCAF.StartStreamingCAF();
		}

		++headerCAF.m_nRef_at_Runtime;

		bRet = true;
	}

	return bRet;
}

//////////////////////////////////////////////////////////////////////////
bool CharacterUpr::CAF_ReleaseByGlobalId(i32 globalID)
{
	bool bRet = false;

	DRX_ASSERT((globalID >= 0) && (globalID < g_AnimationUpr.m_arrGlobalCAF.size()));
	if ((globalID >= 0) && (globalID < g_AnimationUpr.m_arrGlobalCAF.size()))
	{
		GlobalAnimationHeaderCAF& headerCAF = g_AnimationUpr.m_arrGlobalCAF[globalID];

		DRX_ASSERT(headerCAF.m_nRef_at_Runtime > 0);
		if (headerCAF.m_nRef_at_Runtime > 0)
			--headerCAF.m_nRef_at_Runtime;

		bRet = true;
	}

	return bRet;
}

bool CharacterUpr::StreamKeepCDFResident(tukk szFilePath, i32 nLod, i32 nRefAdj, bool bUrgent)
{
	bool bRet = true;

	u32k cdfId = GetCDFId(szFilePath);
	if (cdfId == INVALID_CDF_ID)
		return bRet;

	if (!m_arrCacheForCDF[cdfId].m_strBaseModelFilePath.empty())
	{
		tukk pFilepathSKEL = m_arrCacheForCDF[cdfId].m_strBaseModelFilePath.c_str();
		const bool isBaseSKEL = (0 == stricmp(PathUtil::GetExt(pFilepathSKEL), DRX_SKEL_FILE_EXT));
		const bool isBaseCGA = (0 == stricmp(PathUtil::GetExt(pFilepathSKEL), DRX_ANIM_GEOMETRY_FILE_EXT));

		// First prefetch base model
		if (isBaseSKEL || isBaseCGA)
		{
			if (CDefaultSkeleton* pSkeleton = CheckIfModelSKELLoaded(pFilepathSKEL, 0))
			{
				if (CModelMesh* pModelMesh = pSkeleton->GetModelMesh())
				{
					MeshStreamInfo& msi = pModelMesh->m_stream;
					msi.nKeepResidentRefs += nRefAdj;
					msi.bIsUrgent = msi.bIsUrgent || bUrgent;
					bRet = bRet && (pModelMesh->m_pIRenderMesh != NULL);
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//Prefetch attachments
		u32k numAttachments = m_arrCacheForCDF[cdfId].m_arrAttachments.size();
		for (u32 attachmentIdx = 0; attachmentIdx < numAttachments; attachmentIdx++)
		{
			const CharacterAttachment attachmentInfo = m_arrCacheForCDF[cdfId].m_arrAttachments[attachmentIdx];

			if (attachmentInfo.m_Type == CA_BONE)
			{
				tukk fileExt = PathUtil::GetExt(attachmentInfo.m_strBindingPath);
				const bool isCDF = (0 == stricmp(fileExt, DRX_CHARACTER_DEFINITION_FILE_EXT));
				const bool isSKEL = (0 == stricmp(fileExt, DRX_SKEL_FILE_EXT));
				const bool isCGA = (0 == stricmp(fileExt, DRX_ANIM_GEOMETRY_FILE_EXT));
				const bool isCGF = (0 == stricmp(fileExt, DRX_GEOMETRY_FILE_EXT));

				if (isSKEL || isCGA)
				{
					if (CDefaultSkeleton* pSkeleton = CheckIfModelSKELLoaded(attachmentInfo.m_strBindingPath.c_str(), 0))
					{
						if (CModelMesh* pModelMesh = pSkeleton->GetModelMesh())
						{
							MeshStreamInfo& msi = pModelMesh->m_stream;
							msi.nKeepResidentRefs += nRefAdj;
							msi.bIsUrgent = msi.bIsUrgent || bUrgent;
							bRet = bRet && (pModelMesh->m_pIRenderMesh != NULL);
						}
					}
				}
				else if (isCDF)
				{
					bool bKeepRes = StreamKeepCDFResident(attachmentInfo.m_strBindingPath.c_str(), nLod, nRefAdj, bUrgent);
					bRet = bRet && bKeepRes;
				}
			}

			if (attachmentInfo.m_Type == CA_FACE)
			{
				tukk fileExt = PathUtil::GetExt(attachmentInfo.m_strBindingPath);

				const bool isCDF = (0 == stricmp(fileExt, DRX_CHARACTER_DEFINITION_FILE_EXT));
				const bool isCGF = (0 == stricmp(fileExt, DRX_GEOMETRY_FILE_EXT));
				const bool isCHR = (0 == stricmp(fileExt, DRX_SKEL_FILE_EXT));      //will not make sense in then future, because you can't see anything

				if (isCHR)
				{
					if (CDefaultSkeleton* pSkeleton = CheckIfModelSKELLoaded(attachmentInfo.m_strBindingPath.c_str(), 0))
					{
						if (CModelMesh* pModelMesh = pSkeleton->GetModelMesh())
						{
							MeshStreamInfo& msi = pModelMesh->m_stream;
							msi.nKeepResidentRefs += nRefAdj;
							msi.bIsUrgent = msi.bIsUrgent || bUrgent;

							bRet = bRet && (pModelMesh->m_pIRenderMesh != NULL);
						}
					}
				}
				else if (isCDF)
				{
					bool bKeepRes = StreamKeepCDFResident(attachmentInfo.m_strBindingPath.c_str(), nLod, nRefAdj, bUrgent);
					bRet = bRet && bKeepRes;
				}
			}

			if (attachmentInfo.m_Type == CA_SKIN)
			{
				tukk fileExt = PathUtil::GetExt(attachmentInfo.m_strBindingPath);
				const bool isSKIN = (0 == stricmp(fileExt, DRX_SKIN_FILE_EXT));

				if (isSKIN)
				{
					CSkin* pSkin = CheckIfModelSKINLoaded(attachmentInfo.m_strBindingPath.c_str(), 0);
					if (pSkin)
					{
						if (!pSkin->m_arrModelMeshes.empty())
						{
							i32 nSkinLod = clamp_tpl(nLod, 0, pSkin->m_arrModelMeshes.size() - 1);
							MeshStreamInfo& msi = pSkin->m_arrModelMeshes[nSkinLod].m_stream;
							msi.nKeepResidentRefs += nRefAdj;
							msi.bIsUrgent = msi.bIsUrgent || bUrgent;

							bRet = bRet && (pSkin->m_arrModelMeshes[nSkinLod].m_pIRenderMesh != NULL);
						}
					}
				}
			}
		}
	}

	return bRet;
}

tukk CharacterUpr::GetDBAFilePathByGlobalID(i32 globalID) const
{
	if (globalID >= 0)
	{
		i32 num = i32(m_AnimationUpr.m_arrGlobalCAF.size());
		assert(globalID < num);
		if (globalID < num)
		{
			const GlobalAnimationHeaderCAF& rCAF = m_AnimationUpr.m_arrGlobalCAF[globalID];
			for (i32 i = 0; i < m_AnimationUpr.m_arrGlobalHeaderDBA.size(); i++)
			{
				const CGlobalHeaderDBA& rDBA = m_AnimationUpr.m_arrGlobalHeaderDBA[i];
				if (rDBA.m_FilePathDBACRC32 == rCAF.m_FilePathDBACRC32)
					return rDBA.m_strFilePathDBA;
			}
		}
	}
	return NULL;
}

void CharacterUpr::UpdateDatabaseUnloadTimeStamp()
{
	// sets a timestep in milliseconds from game startup
	m_lastDatabaseUnloadTimeStamp = u32(1000.0f * gEnv->pTimer->GetCurrTime(ITimer::ETIMER_UI));
}

u32 CharacterUpr::GetDatabaseUnloadTimeDelta() const
{
	u32 currTimeStamp = u32(1000.0f * gEnv->pTimer->GetCurrTime(ITimer::ETIMER_UI));
	assert(m_lastDatabaseUnloadTimeStamp <= currTimeStamp);
	u32 delta = currTimeStamp - m_lastDatabaseUnloadTimeStamp;
	if (delta > DBA_UNLOAD_MAX_DELTA_MS)
		delta = DBA_UNLOAD_MAX_DELTA_MS;
	// return time delta in milliseconds from the last call to UpdateDatabseUnloadTimeStep()
	// we cap this at a maximum delta, to prevent skipping of CAF Unregister events if a frame
	// takes too long
	return delta;
}

bool CharacterUpr::CAF_LoadSynchronously(u32 filePathCRC)
{
	bool bRet = false;

	i32 globalID = g_AnimationUpr.m_AnimationMapCAF.GetValueCRC(filePathCRC);
	DRX_ASSERT((globalID >= 0) && (globalID < g_AnimationUpr.m_arrGlobalCAF.size()));
	if ((globalID >= 0) && (globalID < g_AnimationUpr.m_arrGlobalCAF.size()))
	{
		GlobalAnimationHeaderCAF& headerCAF = g_AnimationUpr.m_arrGlobalCAF[globalID];
		if (!headerCAF.IsAssetLoaded())
		{
			headerCAF.LoadControllersCAF();
			bRet = (headerCAF.IsAssetLoaded() != 0);
		}
		else
		{
			bRet = true;
		}
	}

	return bRet;
}

bool CharacterUpr::LMG_LoadSynchronously(u32 filePathCRC, const IAnimationSet* pAnimationSet)
{
	i32 globalID = GetAnimationUpr().GetGlobalIDbyFilePathCRC_LMG(filePathCRC);

	if ((globalID >= 0) && (globalID < GetAnimationUpr().m_arrGlobalLMG.size()))
	{
		GlobalAnimationHeaderLMG& header = GetAnimationUpr().m_arrGlobalLMG[globalID];

		for (size_t i = 0; i < header.m_numExamples; ++i)
		{
			const BSParameter& parameter = header.m_arrParameter[i];
			i32 animationId = pAnimationSet->GetAnimIDByCRC(parameter.m_animName.m_CRC32);
			i32 filePathCRC = pAnimationSet->GetFilePathCRCByAnimID(animationId);
			if (!CAF_LoadSynchronously(filePathCRC))
				return false;
		}

		size_t numCombinedBlendspaces = header.m_arrCombinedBlendSpaces.size();
		for (size_t i = 0; i < numCombinedBlendspaces; ++i)
		{
			const BlendSpaceInfo& bspace = header.m_arrCombinedBlendSpaces[i];
			if (!LMG_LoadSynchronously(bspace.m_FilePathCRC32, pAnimationSet))
				return false;
		}
		return true;
	}

	return false;
}

#if BLENDSPACE_VISUALIZATION
void CharacterUpr::CreateDebugInstances(tukk szCharacterFileName)
{
	u32 nMaxDebugInstances = 128;
	m_arrCharacterBase.resize(nMaxDebugInstances);
	for (u32 i = 0; i < nMaxDebugInstances; i++)
	{
		m_arrCharacterBase[i].m_pInst = CreateInstance(szCharacterFileName, CA_CharEditModel | CA_DisableLogWarnings);
		m_arrCharacterBase[i].m_GridLocation.q.SetRotationZ(-gf_PI * 0.5f);
		m_arrCharacterBase[i].m_GridLocation.t = Vec3(ZERO);
		m_arrCharacterBase[i].m_AmbientColor.r = 0;
		m_arrCharacterBase[i].m_AmbientColor.g = 0;
		m_arrCharacterBase[i].m_AmbientColor.b = 0;
		m_arrCharacterBase[i].m_AmbientColor.a = 0;
		m_arrCharacterBase[i].m_fExtrapolation = -0.25f;
	}
}

bool CharacterUpr::HasAnyDebugInstancesCreated() const
{
	if (m_arrCharacterBase.empty())
		return false;
	if (!m_arrCharacterBase[0].m_pInst)
		return false;

	return true;
}

bool CharacterUpr::HasDebugInstancesCreated(tukk szCharacterFileName) const
{
	return HasAnyDebugInstancesCreated() && stricmp(m_arrCharacterBase[0].m_pInst->GetFilePath(), szCharacterFileName) == 0;
}

void CharacterUpr::DeleteDebugInstances()
{
	u32 numDebugInstances = m_arrCharacterBase.size();
	for (u32 i = 0; i < numDebugInstances; i++)
		m_arrCharacterBase[i].m_pInst = 0;                                  //_smart_ptr  > decrease refcount and release model
}

void CharacterUpr::RenderDebugInstances(const SRenderingPassInfo& passInfo)
{
	//render all examples as mini-characters
	QuatTS qts;

	u32 numDebugInstances = m_arrCharacterBase.size();
	for (u32 i = 0; i < numDebugInstances; i++)
	{
		if (m_arrCharacterBase[i].m_AmbientColor.a == 0)
			continue;
		m_arrCharacterBase[i].m_AmbientColor.a = 0;

		ICharacterInstance* pExampleInst = m_arrCharacterBase[i].m_pInst;    //_smart_ptr  > decrease refcount and release model

		qts.q = m_arrCharacterBase[i].m_GridLocation.q;
		qts.t = m_arrCharacterBase[i].m_GridLocation.t;
		qts.s = m_arrCharacterBase[i].m_GridLocation.s;
		Matrix34 rEntityMat = Matrix34(qts);

		SRendParams rp;
		rp.fDistance = 3.5f;
		rp.AmbientColor.r = m_arrCharacterBase[i].m_AmbientColor.r;
		rp.AmbientColor.g = m_arrCharacterBase[i].m_AmbientColor.g;
		rp.AmbientColor.b = m_arrCharacterBase[i].m_AmbientColor.b;
		rp.AmbientColor.a = 1;
		rp.dwFObjFlags = 0;
		rp.dwFObjFlags |= FOB_TRANS_MASK;
		rp.pMatrix = &rEntityMat;
		rp.pPrevMatrix = &rEntityMat;
		IMaterial* pMtl = pExampleInst->GetIMaterial();
		if (pMtl)
			rp.pMaterial = pMtl;

		pExampleInst->Render(rp, passInfo);
	}
}

void CharacterUpr::RenderBlendSpace(const SRenderingPassInfo& passInfo, ICharacterInstance* pCharacterInstance, float fCharacterScale, u32 debugFlags)
{
	ISkeletonAnim* pSkeletonAnim = pCharacterInstance->GetISkeletonAnim();
	i32 layerId = 0;
	CAnimation* pAnimation = &pSkeletonAnim->GetAnimFromFIFO(layerId, 0);

	i32 nAnimID = pAnimation->GetAnimationId();

	//check if ParaGroup is invalid
	CAnimationSet* pAnimationSet = (CAnimationSet*)pCharacterInstance->GetIAnimationSet();
	const ModelAnimationHeader* pAnim = pAnimationSet->GetModelAnimationHeader(nAnimID);
	assert(pAnim->m_nAssetType == LMG_File);
	GlobalAnimationHeaderLMG& rLMG = g_AnimationUpr.m_arrGlobalLMG[pAnim->m_nGlobalAnimId];
	u32 nError = rLMG.m_Status.size();

	SParametricSamplerInternal* pSampler = (SParametricSamplerInternal*)pAnimation->GetParametricSampler();
	if (pSampler)
	{
		if (pSampler->m_numDimensions == 1)
			pSampler->BlendSpace1DVisualize(rLMG, *pAnimation, pAnimationSet, 1.0f, 0, debugFlags, fCharacterScale);
		else if (pSampler->m_numDimensions == 2)
			pSampler->BlendSpace2DVisualize(rLMG, *pAnimation, pAnimationSet, 1.0f, 0, debugFlags, fCharacterScale);
		else if (pSampler->m_numDimensions == 3)
			pSampler->BlendSpace3DVisualize(rLMG, *pAnimation, pAnimationSet, 1.0f, 0, debugFlags, fCharacterScale);
		else
			pSampler->VisualizeBlendSpace(pAnimationSet, *pAnimation, 1.0f, 0, rLMG, ZERO, 0, fCharacterScale);
	}

	if (fCharacterScale != 0.0f)
		RenderDebugInstances(passInfo);
}

#endif
