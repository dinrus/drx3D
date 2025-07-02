// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   statobjman.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Загрузка деревьев, построение, регистрация/отрегистрация
//               сущностей для рендеринга.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>

#include <drx3D/Eng3D/StatObj.h>
#include <drx3D/Eng3D/ObjMan.h>
#include <drx3D/Eng3D/VisAreas.h>
#include <drx3D/Eng3D/terrain_sector.h>
#include <drx3D/Eng3D/PolygonClipContext.h>
#include <drx3D/Eng3D/3dEngine.h>
#include <drx3D/Eng3D/IndexedMesh.h>
#include <drx3D/Eng3D/Brush.h>
#include <drx3D/Eng3D/Vegetation.h>
#include <drx3D/Eng3D/terrain.h>
#include <drx3D/Eng3D/ObjectsTree.h>
#include <drx3D/Sys/IResourceUpr.h>
#include <drx3D/Eng3D/DecalRenderNode.h>

#define BRUSH_LIST_FILE     "brushlist.txt"
#define CGF_LEVEL_CACHE_PAK "cgf.pak"

//Platform specific includes MemoryBarrier
#if defined(DRX_PLATFORM_WINDOWS) || defined(DRX_PLATFORM_DURANGO)
	#include <drx3D/CoreX/Platform/DrxWindows.h>
#endif

//////////////////////////////////////////////////////////////////////////
IStatObj* CObjUpr::GetStaticObjectByTypeID(i32 nTypeID)
{
	if (nTypeID >= 0 && nTypeID < m_lstStaticTypes.Count())
		return m_lstStaticTypes[nTypeID].pStatObj;

	return 0;
}

IStatObj* CObjUpr::FindStaticObjectByFilename(tukk filename)
{
	return stl::find_in_map(m_nameToObjectMap, CONST_TEMP_STRING(filename), NULL);
}

void CObjUpr::UnloadVegetationModels(bool bDeleteAll)
{
	PodArray<StatInstGroup>& rGroupTable = m_lstStaticTypes;
	for (u32 nGroupId = 0; nGroupId < rGroupTable.size(); nGroupId++)
	{
		StatInstGroup& rGroup = rGroupTable[nGroupId];

		rGroup.pStatObj = NULL;
		rGroup.pMaterial = NULL;

		for (i32 j = 0; j < FAR_TEX_COUNT; ++j)
		{
			SVegetationSpriteLightInfo& rLightInfo = rGroup.m_arrSSpriteLightInfo[j];
			SAFE_RELEASE(rLightInfo.m_pDynTexture);
		}
	}

	if (bDeleteAll)
		rGroupTable.Free();
}

//////////////////////////////////////////////////////////////////////////
void CObjUpr::UnloadObjects(bool bDeleteAll)
{
	UnloadVegetationModels(bDeleteAll);
	UnloadFarObjects();

	CleanStreamingData();

	m_pRMBox = 0;

	m_decalsToPrecreate.resize(0);

	// Clear all objects that are in the garbage collector.
	ClearStatObjGarbage();

	stl::free_container(m_checkForGarbage);
	m_bGarbageCollectionEnabled = false;

	if (bDeleteAll)
	{
		m_lockedObjects.clear(); // Lock/Unlock resources will not work with this.

		// Release default stat obj.
		m_pDefaultCGF = 0;

		m_nameToObjectMap.clear();
		m_lstLoadedObjects.clear();

		i32 nNumLeaks = 0;
		std::vector<CStatObj*> garbage;
		for (CStatObj* pStatObj = CStatObj::get_intrusive_list_root(); pStatObj; pStatObj = pStatObj->get_next_intrusive())
		{
			garbage.push_back(pStatObj);

#if !defined(_RELEASE)
			if (!pStatObj->IsDefaultObject())
			{
				nNumLeaks++;
				Warning("StatObj not deleted: %s (%s)  RefCount: %d", pStatObj->m_szFileName.c_str(), pStatObj->m_szGeomName.c_str(), pStatObj->m_nUsers);
			}
#endif //_RELEASE
		}

#ifndef _RELEASE
		// deleting leaked objects
		if (nNumLeaks > 0)
		{
			Warning("CObjUpr::CheckObjectLeaks: %d object(s) found in memory", nNumLeaks);
		}
#endif //_RELEASE

		for (i32 i = 0, num = (i32)garbage.size(); i < num; i++)
		{
			CStatObj* pStatObj = garbage[i];
			pStatObj->ShutDown();
		}
		for (i32 i = 0, num = (i32)garbage.size(); i < num; i++)
		{
			CStatObj* pStatObj = garbage[i];
			delete pStatObj;
		}

#ifdef POOL_STATOBJ_ALLOCS
		assert(m_statObjPool->GetTotalMemory().nUsed == 0);
#endif
	}
	m_bGarbageCollectionEnabled = true;

#ifdef POOL_STATOBJ_ALLOCS
	m_statObjPool->FreeMemoryIfEmpty();
#endif

	stl::free_container(m_lstTmpCastingNodes);
	stl::free_container(m_decalsToPrecreate);
	stl::free_container(m_tmpAreas0);
	stl::free_container(m_tmpAreas1);
	for (size_t rl = 0; rl < MAX_RECURSION_LEVELS; ++rl)
	{
		for (size_t ti = 0; ti < nThreadsNum; ++ti)
			stl::free_container(m_arrVegetationSprites[rl][ti]);
	}

	m_lstStaticTypes.Free();
}

//////////////////////////////////////////////////////////////////////////
void CObjUpr::CleanStreamingData()
{
	stl::free_container(m_arrStreamingNodeStack);

	stl::free_container(m_arrStreamableToRelease);
	stl::free_container(m_arrStreamableToLoad);
	stl::free_container(m_arrStreamableToDelete);
}

//////////////////////////////////////////////////////////////////////////
// class for asyncronous preloading of level CGF's
//////////////////////////////////////////////////////////////////////////
struct CLevelStatObjLoader : public IStreamCallback, public DinrusX3dEngBase
{
	i32 m_nTasksNum;

	CLevelStatObjLoader()
	{
		m_nTasksNum = 0;
	}

	void StartStreaming(tukk pFileName)
	{
		m_nTasksNum++;

		// request the file
		StreamReadParams params;
		params.dwUserData = 0;
		params.nSize = 0;
		params.pBuffer = NULL;
		params.nLoadTime = 0;
		params.nMaxLoadTime = 0;
		params.ePriority = estpUrgent;
		GetSystem()->GetStreamEngine()->StartRead(eStreamTaskTypeGeometry, pFileName, this, &params);
	}

	virtual void StreamOnComplete(IReadStream* pStream, unsigned nError)
	{
		if (!nError)
		{
			string szName = pStream->GetName();
			// remove game folder from path
			tukk szInGameName = strstr(szName, "\\");
			// load CGF from memory
			GetObjUpr()->LoadStatObj(szInGameName + 1, NULL, NULL, true, 0, pStream->GetBuffer(), pStream->GetBytesRead());
		}

		m_nTasksNum--;
	}
};

//////////////////////////////////////////////////////////////////////////
// Preload in efficient way all CGF's used in level
//////////////////////////////////////////////////////////////////////////
void CObjUpr::PreloadLevelObjects()
{
	LOADING_TIME_PROFILE_SECTION;

	// Starting a new level, so make sure the round ids are ahead of what they were in the last level
	m_nUpdateStreamingPrioriryRoundId += 8;
	m_nUpdateStreamingPrioriryRoundIdFast += 8;

	PrintMessage("Starting loading level CGF's ...");
	INDENT_LOG_DURING_SCOPE();

	float fStartTime = GetCurAsyncTimeSec();

	bool bCgfCacheExist = false;
	if (GetCVars()->e_StreamCgf != 0)
	{
		// Only when streaming enable use no-mesh cgf pak.
		//bCgfCacheExist = GetISystem()->GetIResourceUpr()->LoadLevelCachePak( CGF_LEVEL_CACHE_PAK,"" );
	}
	IResourceList* pResList = GetISystem()->GetIResourceUpr()->GetLevelResourceList();

	// Construct streamer object
	CLevelStatObjLoader cgfStreamer;

	DrxPathString cgfFilename;
	i32 nCgfCounter = 0;
	i32 nInLevelCacheCount = 0;

	bool bVerboseLogging = GetCVars()->e_StatObjPreload > 1;

	//////////////////////////////////////////////////////////////////////////
	// Enumerate all .CGF inside level from the "brushlist.txt" file.
	{
		string brushListFilename = Get3DEngine()->GetLevelFilePath(BRUSH_LIST_FILE);
		CDrxFile file;
		if (file.Open(brushListFilename.c_str(), "rb") && file.GetLength() > 0)
		{
			i32 nFileLength = file.GetLength();
			tuk buf = new char[nFileLength + 1];
			buf[nFileLength] = 0; // Null terminate
			file.ReadRaw(buf, nFileLength);

			// Parse file, every line in a file represents a resource filename.
			char seps[] = "\r\n";
			tuk token = strtok(buf, seps);
			while (token != NULL)
			{
				i32 nAliasLen = sizeof("%level%") - 1;
				if (strncmp(token, "%level%", nAliasLen) == 0)
				{
					cgfFilename = Get3DEngine()->GetLevelFilePath(token + nAliasLen);
				}
				else
				{
					cgfFilename = token;
				}

				if (bVerboseLogging)
				{
					DrxLog("%s", cgfFilename.c_str());
				}
				// Do not use streaming for the Brushes from level.pak.
				GetObjUpr()->LoadStatObj(cgfFilename.c_str(), NULL, 0, false, 0);
				//cgfStreamer.StartStreaming(cgfFilename.c_str());
				nCgfCounter++;

				token = strtok(NULL, seps);

				//This loop can take a few seconds, so we should refresh the loading screen and call the loading tick functions to ensure that no big gaps in coverage occur.
				SYNCHRONOUS_LOADING_TICK();
			}
			delete[]buf;
		}
	}
	//////////////////////////////////////////////////////////////////////////

	// Request objects loading from Streaming System.
	if (tukk pCgfName = pResList->GetFirst())
	{
		while (pCgfName)
		{
			if (strstr(pCgfName, ".cgf"))
			{
				tukk sLodName = strstr(pCgfName, "_lod");
				if (sLodName && (sLodName[4] >= '0' && sLodName[4] <= '9'))
				{
					// Ignore Lod files.
					pCgfName = pResList->GetNext();
					continue;
				}

				cgfFilename = pCgfName;

				if (bVerboseLogging)
				{
					DrxLog("%s", cgfFilename.c_str());
				}
				CStatObj* pStatObj = GetObjUpr()->LoadStatObj(cgfFilename.c_str(), NULL, 0, true, 0);
				if (pStatObj)
				{
					if (pStatObj->m_bMeshStrippedCGF)
					{
						nInLevelCacheCount++;
					}
				}
				//cgfStreamer.StartStreaming(cgfFilename.c_str());
				nCgfCounter++;

				//This loop can take a few seconds, so we should refresh the loading screen and call the loading tick functions to ensure that no big gaps in coverage occur.
				SYNCHRONOUS_LOADING_TICK();
			}

			pCgfName = pResList->GetNext();
		}
	}

	//  PrintMessage("Finished requesting level CGF's: %d objects in %.1f sec", nCgfCounter, GetCurAsyncTimeSec()-fStartTime);

	// Continue updating streaming system until all CGF's are loaded
	if (cgfStreamer.m_nTasksNum > 0)
	{
		LOADING_TIME_PROFILE_SECTION_NAMED("CObjUpr::PreloadLevelObjects_StreamEngine_Update");
		GetSystem()->GetStreamEngine()->UpdateAndWait();
	}

	if (bCgfCacheExist)
	{
		//GetISystem()->GetIResourceUpr()->UnloadLevelCachePak( CGF_LEVEL_CACHE_PAK );
	}

	float dt = GetCurAsyncTimeSec() - fStartTime;
	PrintMessage("Finished loading level CGF's: %d objects loaded (%d from LevelCache) in %.1f sec", nCgfCounter, nInLevelCacheCount, dt);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create / delete object
//////////////////////////////////////////////////////////////////////////////////////////////////////////
CStatObj* CObjUpr::LoadStatObj(tukk __szFileName
                                   , tukk _szGeomName, IStatObj::SSubObject** ppSubObject
                                   , bool bUseStreaming
                                   , u64 nLoadingFlags
                                   , ukk pData
                                   , i32 nDataSize
                                   , tukk szBlockName)
{
	if (!m_pDefaultCGF && strcmp(__szFileName, DEFAULT_CGF_NAME) != 0)
	{
		// Load default object if not yet loaded.
		tukk sDefaulObjFilename = DEFAULT_CGF_NAME;
		// prepare default object
		m_pDefaultCGF = LoadStatObj(sDefaulObjFilename, NULL, NULL, false, nLoadingFlags);
		if (!m_pDefaultCGF)
		{
			Error("CObjUpr::LoadStatObj: Default object not found (%s)", sDefaulObjFilename);
			m_pDefaultCGF = new CStatObj();
		}
		m_pDefaultCGF->m_bDefaultObject = true;
	}

	if (DrxStringUtils::stristr(__szFileName, "_lod"))
	{
		Warning("Failed to load cgf: %s, '_lod' meshes can be loaded only internally as part of multi-lod CGF loading", __szFileName);
		return m_pDefaultCGF;
	}

	LOADING_TIME_PROFILE_SECTION_ARGS(__szFileName);
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Static Geometry");

	if (ppSubObject)
		*ppSubObject = NULL;

	if (!strcmp(__szFileName, "NOFILE"))
	{
		// make empty object to be filled from outside
		CStatObj* pObject = new CStatObj();
		m_lstLoadedObjects.insert(pObject);
		return pObject;
	}

	// Normalize file name
	char sFilename[_MAX_PATH];

	//////////////////////////////////////////////////////////////////////////
	// Remap %level% alias if needed an unify filename
	{
		i32 nAliasNameLen = sizeof("%level%") - 1;
		if (strncmp(__szFileName, "%level%", nAliasNameLen) == 0)
		{
			drx_strcpy(sFilename, Get3DEngine()->GetLevelFilePath(__szFileName + nAliasNameLen));
		}
		else
		{
			drx_strcpy(sFilename, __szFileName);
		}

		PREFAST_SUPPRESS_WARNING(6054)                                     // sFilename is null terminated
		std::replace(sFilename, sFilename + strlen(sFilename), '\\', '/'); // To Unix Path
	}
	//////////////////////////////////////////////////////////////////////////

	bool bForceBreakable = strstr(sFilename, "break") != 0;
	if (_szGeomName && !strcmp(_szGeomName, "#ForceBreakable"))
	{
		bForceBreakable = true;
		_szGeomName = 0;
	}

	// Try to find already loaded object
	CStatObj* pObject = 0;

	i32 flagCloth = 0;
	if (_szGeomName && !strcmp(_szGeomName, "cloth"))
		_szGeomName = 0, flagCloth = STATIC_OBJECT_DYNAMIC | STATIC_OBJECT_CLONE;
	else
	{
		pObject = stl::find_in_map(m_nameToObjectMap, CONST_TEMP_STRING(sFilename), NULL);
		if (pObject)
		{
			if (!bUseStreaming && pObject->m_bCanUnload)
			{
				pObject->DisableStreaming();
			}

			assert(!pData);
			if (!pObject->m_bLodsLoaded && !pData)
			{
				pObject->LoadLowLODs(bUseStreaming, nLoadingFlags);
			}

			if (_szGeomName && _szGeomName[0])
			{
				// Return SubObject.
				CStatObj::SSubObject* pSubObject = pObject->FindSubObject(_szGeomName);
				if (!pSubObject || !pSubObject->pStatObj)
					return 0;
				if (pSubObject->pStatObj)
				{
					if (ppSubObject)
						*ppSubObject = pSubObject;
					return (CStatObj*)pSubObject->pStatObj;
				}
			}
			return pObject;
		}
	}

	// Load new CGF
	pObject = new CStatObj();
	pObject->m_nFlags |= flagCloth;

	bUseStreaming &= (GetCVars()->e_StreamCgf != 0);

	if (bUseStreaming)
		pObject->m_bCanUnload = true;
	if (bForceBreakable)
		nLoadingFlags |= IStatObj::ELoadingFlagsForceBreakable;

	if (!pObject->LoadCGF(sFilename, strstr(sFilename, "_lod") != NULL, nLoadingFlags, pData, nDataSize))
	{
		if (!(nLoadingFlags & IStatObj::ELoadingFlagsNoErrorIfFail))
			Error("Failed to load cgf: %s", __szFileName);
		// object not found
		// if geom name is specified - just return 0
		if (_szGeomName && _szGeomName[0])
		{
			delete pObject;
			return 0;
		}

		// make unique default CGF for every case of missing CGF, this will make export process more reliable and help finding missing CGF's in pure game
		/*		pObject->m_bCanUnload = false;
		    if (m_bEditor && pObject->LoadCGF( DEFAULT_CGF_NAME, false, nLoadingFlags, pData, nDataSize ))
		    {
		      pObject->m_szFileName = sFilename;
		      pObject->m_bDefaultObject = true;
		    }
		    else*/
		{
			delete pObject;
			return m_pDefaultCGF;
		}
	}

	// now try to load lods
	if (!pData)
	{
		pObject->LoadLowLODs(bUseStreaming, nLoadingFlags);
	}

	if (!pObject->m_bCanUnload)
	{
		// even if streaming is disabled we register object for potential streaming (streaming system will never unload it)
		pObject->DisableStreaming();
	}

	// sub meshes merging
	pObject->TryMergeSubObjects(false);

	m_lstLoadedObjects.insert(pObject);
	m_nameToObjectMap[pObject->m_szFileName] = pObject;

	if (_szGeomName && _szGeomName[0])
	{
		// Return SubObject.
		CStatObj::SSubObject* pSubObject = pObject->FindSubObject(_szGeomName);
		if (!pSubObject || !pSubObject->pStatObj)
			return 0;
		if (pSubObject->pStatObj)
		{
			if (ppSubObject)
				*ppSubObject = pSubObject;
			return (CStatObj*)pSubObject->pStatObj;
		}
	}

	return pObject;
}

//////////////////////////////////////////////////////////////////////////
bool CObjUpr::InternalDeleteObject(CStatObj* pObject)
{
	assert(pObject);

	if (!m_bLockCGFResources && !IsResourceLocked(pObject->m_szFileName))
	{
		LoadedObjects::iterator it = m_lstLoadedObjects.find(pObject);
		if (it != m_lstLoadedObjects.end())
		{
			m_lstLoadedObjects.erase(it);
			m_nameToObjectMap.erase(pObject->m_szFileName);
		}
		else
		{
			//Warning( "CObjUpr::ReleaseObject called on object not loaded in ObjectUpr %s",pObject->m_szFileName.c_str() );
			//return false;
		}

		delete pObject;
		return true;
	}
	else if (m_bLockCGFResources)
	{
		// Put them to locked stat obj list.
		stl::push_back_unique(m_lockedObjects, pObject);
	}

	return false;
}

CStatObj* CObjUpr::AllocateStatObj()
{
#ifdef POOL_STATOBJ_ALLOCS
	return (CStatObj*)m_statObjPool->Allocate();
#else
	return (CStatObj*)malloc(sizeof(CStatObj));
#endif
}

void CObjUpr::FreeStatObj(CStatObj* pObj)
{
#ifdef POOL_STATOBJ_ALLOCS
	m_statObjPool->Deallocate(pObj);
#else
	free(pObj);
#endif
}

CObjUpr::CObjUpr() :
	m_pDefaultCGF(NULL),
	m_decalsToPrecreate(),
	m_bNeedProcessObjectsStreaming_Finish(false),
	m_CullThread()
{
#ifdef POOL_STATOBJ_ALLOCS
	m_statObjPool = new stl::PoolAllocator<sizeof(CStatObj), stl::PSyncMultiThread, alignof(CStatObj)>(stl::FHeap().PageSize(64)); // 20Kb per page
#endif

	m_vStreamPreCachePointDefs.Add(SObjManPrecachePoint());
	m_vStreamPreCacheCameras.Add(SObjManPrecacheCamera());
	m_nNextPrecachePointId = 0;
	m_bCameraPrecacheOverridden = false;

	m_pObjUpr = this;

	m_fCurrTime = 0.0f;

	m_vSkyColor.Set(0, 0, 0);
	m_fSunSkyRel = 0;
	m_vSunColor.Set(0, 0, 0);
	m_fILMul = 1.0f;
	m_fSkyBrightMul = 0.3f;
	m_fSSAOAmount = 1.f;
	m_fSSAOContrast = 1.f;
	m_fGIAmount = 1.f;
	m_rainParams.nUpdateFrameID = -1;
	m_rainParams.fAmount = 0.f;
	m_rainParams.fRadius = 1.f;
	m_rainParams.vWorldPos.Set(0, 0, 0);
	m_rainParams.vColor.Set(1, 1, 1);
	m_rainParams.fFakeGlossiness = 0.5f;
	m_rainParams.fFakeReflectionAmount = 1.5f;
	m_rainParams.fDiffuseDarkening = 0.5f;
	m_rainParams.fRainDropsAmount = 0.5f;
	m_rainParams.fRainDropsSpeed = 1.f;
	m_rainParams.fRainDropsLighting = 1.f;
	m_rainParams.fMistAmount = 3.f;
	m_rainParams.fMistHeight = 8.f;
	m_rainParams.fPuddlesAmount = 1.5f;
	m_rainParams.fPuddlesMaskAmount = 1.0f;
	m_rainParams.fPuddlesRippleAmount = 2.0f;
	m_rainParams.fSplashesAmount = 1.3f;
	m_rainParams.bIgnoreVisareas = false;
	m_rainParams.bDisableOcclusion = false;

	m_fMaxViewDistanceScale = 1.f;
	m_fGSMMaxDistance = 0;
	m_bLockCGFResources = false;

	m_pRMBox = NULL;
	m_bGarbageCollectionEnabled = true;

	m_decalsToPrecreate.reserve(128);

	// init queue for check occlusion
	m_CheckOcclusionQueue.Init(GetCVars()->e_CheckOcclusionQueueSize);
	m_CheckOcclusionOutputQueue.Init(GetCVars()->e_CheckOcclusionOutputQueueSize);
}

// make unit box for occlusion test
void CObjUpr::MakeUnitCube()
{
	if (m_pRMBox)
		return;

	SVF_P3F_C4B_T2F arrVerts[8];
	arrVerts[0].xyz = Vec3(0, 0, 0);
	arrVerts[1].xyz = Vec3(1, 0, 0);
	arrVerts[2].xyz = Vec3(0, 0, 1);
	arrVerts[3].xyz = Vec3(1, 0, 1);
	arrVerts[4].xyz = Vec3(0, 1, 0);
	arrVerts[5].xyz = Vec3(1, 1, 0);
	arrVerts[6].xyz = Vec3(0, 1, 1);
	arrVerts[7].xyz = Vec3(1, 1, 1);

	//		6-------7
	//	 /		   /|
	//	2-------3	|
	//	|	      |	|
	//	|	4			| 5
	//	|				|/
	//	0-------1

	static const vtx_idx arrIndices[] =
	{
		// front + back
		1, 0, 2,
		2, 3, 1,
		5, 6, 4,
		5, 7, 6,
		// left + right
		0, 6, 2,
		0, 4, 6,
		1, 3, 7,
		1, 7, 5,
		// top + bottom
		3, 2, 6,
		6, 7, 3,
		1, 4, 0,
		1, 5, 4
	};

	m_pRMBox = GetRenderer()->CreateRenderMeshInitialized(
	  arrVerts,
	  DRX_ARRAY_COUNT(arrVerts),
	  EDefaultInputLayouts::P3F_C4B_T2F,
	  arrIndices,
	  DRX_ARRAY_COUNT(arrIndices),
	  prtTriangleList,
	  "OcclusionQueryCube", "OcclusionQueryCube",
	  eRMT_Static);

	m_pRMBox->SetChunk(NULL, 0, DRX_ARRAY_COUNT(arrVerts), 0, DRX_ARRAY_COUNT(arrIndices), 1.0f, 0);

	m_bGarbageCollectionEnabled = true;
}

CObjUpr::~CObjUpr()
{
	// free default object
	m_pDefaultCGF = 0;

	// free brushes
	/*  assert(!m_lstBrushContainer.Count());
	   for(i32 i=0; i<m_lstBrushContainer.Count(); i++)
	   {
	    if(m_lstBrushContainer[i]->GetEntityStatObj())
	      ReleaseObject((CStatObj*)m_lstBrushContainer[i]->GetEntityStatObj());
	    delete m_lstBrushContainer[i];
	   }
	   m_lstBrushContainer.Reset();
	 */
	UnloadObjects(true);
#ifdef POOL_STATOBJ_ALLOCS
	delete m_statObjPool;
#endif
}

i32 CObjUpr::ComputeDissolve(const CLodValue &lodValueIn, SRenderNodeTempData* pTempData, IRenderNode* pEnt, float fEntDistance, CLodValue arrlodValuesOut[2])
{
	i32 nLodMain = CLAMP(0, lodValueIn.LodA(), MAX_STATOBJ_LODS_NUM - 1);
	i32 nLodMin = std::max(nLodMain - 1, 0);
	i32 nLodMax = std::min(nLodMain + 1, MAX_STATOBJ_LODS_NUM - 1);

	float prevLodLastTimeUsed = 0;
	float* arrLodLastTimeUsed = pTempData->userData.arrLodLastTimeUsed;

	// Find when previous lod was used as primary lod last time and update last time used for current primary lod
	arrLodLastTimeUsed[nLodMain] = GetCurTimeSec();
	for (i32 nLO = nLodMin; nLO <= nLodMax; nLO++)
	{
		if (nLO != nLodMain)
			prevLodLastTimeUsed = std::max(prevLodLastTimeUsed, arrLodLastTimeUsed[nLO]);
	}

	float fDissolveRef = 1.f - SATURATE((GetCurTimeSec() - prevLodLastTimeUsed) / GetCVars()->e_LodTransitionTime);
	prevLodLastTimeUsed = std::max(prevLodLastTimeUsed, GetCurTimeSec() - GetCVars()->e_LodTransitionTime);

	// Compute also max view distance fading
	const float fDistFadeInterval = 2.f;
	float fDistFadeRef = SATURATE(min(fEntDistance / pEnt->m_fWSMaxViewDist * 5.f - 4.f, ((fEntDistance - pEnt->m_fWSMaxViewDist) / fDistFadeInterval + 1.f)));

	CLodValue lodSubValue;
	i32 nLodsNum = 0;

	// Render current lod and (if needed) previous lod
	for (i32 nLO = nLodMin; nLO <= nLodMax && nLodsNum < 2; nLO++)
	{
		if (arrLodLastTimeUsed[nLO] < prevLodLastTimeUsed)
			continue;

		if (nLodMain == nLO)
		{
			// Incoming LOD
			float fDissolveMaxDistRef = std::max(fDissolveRef, fDistFadeRef);
			lodSubValue = CLodValue(nLO, i32(fDissolveMaxDistRef * 255.f), -1);
		}
		else
		{
			// Outgoing LOD
			float fDissolveMaxDistRef = std::min(fDissolveRef, 1.f - fDistFadeRef);
			lodSubValue = CLodValue(-1, i32(fDissolveMaxDistRef * 255.f), nLO);
		}

		arrlodValuesOut[nLodsNum] = lodSubValue;
		nLodsNum++;
	}

	return nLodsNum;
}

// mostly xy size
float CObjUpr::GetXYRadius(i32 type)
{
	if ((m_lstStaticTypes.Count() <= type || !m_lstStaticTypes[type].pStatObj))
		return 0;

	Vec3 vSize = m_lstStaticTypes[type].pStatObj->GetBoxMax() - m_lstStaticTypes[type].pStatObj->GetBoxMin();
	vSize.z *= 0.5f;

	float fXYRadius = vSize.GetLength() * 0.5f;

	return fXYRadius;
}

bool CObjUpr::GetStaticObjectBBox(i32 nType, Vec3& vBoxMin, Vec3& vBoxMax)
{
	if ((m_lstStaticTypes.Count() <= nType || !m_lstStaticTypes[nType].pStatObj))
		return 0;

	vBoxMin = m_lstStaticTypes[nType].pStatObj->GetBoxMin();
	vBoxMax = m_lstStaticTypes[nType].pStatObj->GetBoxMax();

	return true;
}

void CObjUpr::GetMemoryUsage(class IDrxSizer* pSizer) const
{
	{
		SIZER_COMPONENT_NAME(pSizer, "Self");
		pSizer->AddObject(this, sizeof(*this));
	}

	{
		SIZER_COMPONENT_NAME(pSizer, "StaticTypes");
		pSizer->AddObject(m_lstStaticTypes);
	}

	for (i32 i = 0; i < MAX_RECURSION_LEVELS; i++)
	{
		SIZER_COMPONENT_NAME(pSizer, "VegetationSprites");
		for (i32 t = 0; t < nThreadsNum; t++)
		{
			pSizer->AddObject(m_arrVegetationSprites[i][t]);
		}
	}

	{
		SIZER_COMPONENT_NAME(pSizer, "CMesh");
		CIndexedMesh* pMesh = CIndexedMesh::get_intrusive_list_root();
		while (pMesh)
		{
			pSizer->AddObject(pMesh);
			pMesh = pMesh->get_next_intrusive();
		}
	}

	{
		SIZER_COMPONENT_NAME(pSizer, "StatObj");
		for (CStatObj* pStatObj = CStatObj::get_intrusive_list_root(); pStatObj; pStatObj = pStatObj->get_next_intrusive())
		{
			pStatObj->GetMemoryUsage(pSizer);
		}
	}

	{
		SIZER_COMPONENT_NAME(pSizer, "EmptyNodes");
		pSizer->AddObject(COctreeNode::m_arrEmptyNodes);
	}
}

// retrieves the bandwidth calculations for the audio streaming
void CObjUpr::GetBandwidthStats(float* fBandwidthRequested)
{
#if !defined (_RELEASE) || defined(ENABLE_STATOSCOPE_RELEASE)
	if (fBandwidthRequested && CStatObj::s_fStreamingTime != 0.0f)
	{
		*fBandwidthRequested = (CStatObj::s_nBandwidth / CStatObj::s_fStreamingTime) / 1024.0f;
	}
#endif
}

void CObjUpr::ReregisterEntitiesInArea(AABB * pBox, bool bCleanUpTree)
{
	PodArray<SRNInfo> lstEntitiesInArea;

	Get3DEngine()->MoveObjectsIntoListGlobal(&lstEntitiesInArea, pBox, true);

	if (GetVisAreaUpr())
		GetVisAreaUpr()->MoveObjectsIntoList(&lstEntitiesInArea, pBox, true);

	for (i32 i = 0; i < lstEntitiesInArea.Count(); i++)
	{
		IVisArea* pPrevArea = lstEntitiesInArea[i].pNode->GetEntityVisArea();
		Get3DEngine()->UnRegisterEntityDirect(lstEntitiesInArea[i].pNode);

		if (lstEntitiesInArea[i].pNode->GetRenderNodeType() == eERType_Decal)
			((CDecalRenderNode*)lstEntitiesInArea[i].pNode)->RequestUpdate();
	}

	if (bCleanUpTree)
	{
		Get3DEngine()->GetObjectsTree()->CleanUpTree();
		if (GetVisAreaUpr())
			GetVisAreaUpr()->CleanUpTrees();
	}

	for (i32 i = 0; i < lstEntitiesInArea.Count(); i++)
	{
		Get3DEngine()->RegisterEntity(lstEntitiesInArea[i].pNode);
	}
}

void CObjUpr::FreeNotUsedCGFs()
{
	//assert(!m_bLockCGFResources);
	m_lockedObjects.clear();

	if (!m_bLockCGFResources)
	{
		//Timur, You MUST use next here, or with erase you invalidating
		LoadedObjects::iterator next;
		for (LoadedObjects::iterator it = m_lstLoadedObjects.begin(); it != m_lstLoadedObjects.end(); it = next)
		{
			next = it;
			++next;
			CStatObj* p = (CStatObj*)(*it);
			if (p->m_nUsers <= 0)
			{
				CheckForGarbage(p);
			}
		}
	}

	ClearStatObjGarbage();
}

//////////////////////////////////////////////////////////////////////////
void CObjUpr::GetLoadedStatObjArray(IStatObj** pObjectsArray, i32& nCount)
{
	if (!pObjectsArray)
	{
		nCount = m_lstLoadedObjects.size();
		return;
	}

	CObjUpr::LoadedObjects::iterator it = m_lstLoadedObjects.begin();
	for (i32 i = 0; i < nCount && it != m_lstLoadedObjects.end(); ++i, ++it)
	{
		pObjectsArray[i] = *it;
	}
}

void StatInstGroup::Update(CVars* pCVars, i32 nGeomDetailScreenRes)
{
	m_dwRndFlags = 0;

	static ICVar* pObjShadowCastSpec = gEnv->pConsole->GetCVar("e_ObjShadowCastSpec");
	if (nCastShadowMinSpec <= pObjShadowCastSpec->GetIVal())
	{
		m_dwRndFlags |= ERF_CASTSHADOWMAPS | ERF_HAS_CASTSHADOWMAPS;
	}

	if (bDynamicDistanceShadows)
		m_dwRndFlags |= ERF_DYNAMIC_DISTANCESHADOWS;
	if (bHideability)
		m_dwRndFlags |= ERF_HIDABLE;
	if (bHideabilitySecondary)
		m_dwRndFlags |= ERF_HIDABLE_SECONDARY;
	if (bPickable)
		m_dwRndFlags |= ERF_PICKABLE;
	if (!bAllowIndoor)
		m_dwRndFlags |= ERF_OUTDOORONLY;
	if (bGIMode)
		m_dwRndFlags |= ERF_GI_MODE_BIT0; // corresponds to IRenderNode::eGM_StaticVoxelization

	u32 nSpec = (u32)minConfigSpec;
	if (nSpec != 0)
	{
		m_dwRndFlags &= ~ERF_SPEC_BITS_MASK;
		m_dwRndFlags |= (nSpec << ERF_SPEC_BITS_SHIFT) & ERF_SPEC_BITS_MASK;
	}

	if (GetStatObj())
	{
		fVegRadiusVert = GetStatObj()->GetRadiusVert();
		fVegRadiusHor = GetStatObj()->GetRadiusHors();
		fVegRadius = max(fVegRadiusVert, fVegRadiusHor);
	}
	else
	{
		fVegRadiusHor = fVegRadius = fVegRadiusVert = 0;
	}

	if (bUseSprites && fVegRadius > 0)
	{
		m_fSpriteSwitchDist = 18.f * fVegRadius * fSize *
		                      max(pCVars->e_VegetationSpritesDistanceCustomRatioMin, fSpriteDistRatio);
		m_fSpriteSwitchDist *= pCVars->e_VegetationSpritesDistanceRatio;
		m_fSpriteSwitchDist *= max(1.f, (float)nGeomDetailScreenRes / 1024.f);
		if (m_fSpriteSwitchDist < pCVars->e_VegetationSpritesMinDistance)
			m_fSpriteSwitchDist = pCVars->e_VegetationSpritesMinDistance;
	}
	else
		m_fSpriteSwitchDist = 1000000.f;

	for (i32 j = 0; j < FAR_TEX_COUNT; ++j)
	{
		SVegetationSpriteLightInfo& rLightInfo = m_arrSSpriteLightInfo[j];
		if (rLightInfo.m_pDynTexture)
			rLightInfo.m_pDynTexture->SetFlags(rLightInfo.m_pDynTexture->GetFlags() | IDynTexture::fNeedRegenerate);
	}

#if defined(FEATURE_SVO_GI)
	IMaterial* pMat = pMaterial ? pMaterial.get() : (pStatObj ? pStatObj->GetMaterial() : 0);
	if (pMat && (DinrusX3dEngBase::GetCVars()->e_svoTI_Active >= 0) && (gEnv->IsEditor() || DinrusX3dEngBase::GetCVars()->e_svoTI_Apply))
		pMat->SetKeepLowResSysCopyForDiffTex();
#endif
}

float StatInstGroup::GetAlignToTerrainAmount() const
{
	return fAlignToTerrainCoefficient;
}

bool CObjUpr::SphereRenderMeshIntersection(IRenderMesh* pRenderMesh, const Vec3& vInPos, const float fRadius, IMaterial* pMat)
{
	FUNCTION_PROFILER_3DENGINE;

	// get position offset and stride
	i32 nPosStride = 0;
	byte* pPos = pRenderMesh->GetPosPtr(nPosStride, FSL_READ);

	// get indices
	vtx_idx* pInds = pRenderMesh->GetIndexPtr(FSL_READ);
	i32 nInds = pRenderMesh->GetIndicesCount();
	assert(nInds % 3 == 0);

	// test tris
	TRenderChunkArray& Chunks = pRenderMesh->GetChunks();
	for (i32 nChunkId = 0; nChunkId < Chunks.size(); nChunkId++)
	{
		CRenderChunk* pChunk = &Chunks[nChunkId];
		if (pChunk->m_nMatFlags & MTL_FLAG_NODRAW || !pChunk->pRE)
			continue;

		// skip transparent and alpha test
		if (pMat)
		{
			const SShaderItem& shaderItem = pMat->GetShaderItem(pChunk->m_nMatID);
			if (!shaderItem.m_pShader || shaderItem.m_pShader->GetFlags() & EF_NODRAW)
				continue;
		}

		i32 nLastIndexId = pChunk->nFirstIndexId + pChunk->nNumIndices;
		for (i32 i = pChunk->nFirstIndexId; i < nLastIndexId; i += 3)
		{
			assert((i32)pInds[i + 0] < pRenderMesh->GetVerticesCount());
			assert((i32)pInds[i + 1] < pRenderMesh->GetVerticesCount());
			assert((i32)pInds[i + 2] < pRenderMesh->GetVerticesCount());

			// get triangle vertices
			Vec3 v0 = (*(Vec3*)&pPos[nPosStride * pInds[i + 0]]);
			Vec3 v1 = (*(Vec3*)&pPos[nPosStride * pInds[i + 1]]);
			Vec3 v2 = (*(Vec3*)&pPos[nPosStride * pInds[i + 2]]);

			AABB triBox;
			triBox.min = v0;
			triBox.max = v0;
			triBox.Add(v1);
			triBox.Add(v2);

			if (Overlap::Sphere_AABB(Sphere(vInPos, fRadius), triBox))
				return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
void CObjUpr::ClearStatObjGarbage()
{
	FUNCTION_PROFILER_3DENGINE;

	std::vector<CStatObj*> garbage;

	DrxMT::vector<CStatObj*>::AutoLock lock(m_checkForGarbage.get_lock());

	while (!m_checkForGarbage.empty())
	{
		garbage.resize(0);

		// Make sure all stat objects inside this array are unique.
		if (!m_checkForGarbage.empty())
		{
			// Only check explicitly added objects.
			// First ShutDown object clearing all pointers.
			CStatObj* pStatObj;
			while (m_checkForGarbage.try_pop_back(pStatObj))
			{
				if (pStatObj->m_bCheckGarbage)
				{
					// Check if it must be released.
					i32 nChildRefs = pStatObj->CountChildReferences();
					if (pStatObj->m_nUsers <= 0 && nChildRefs <= 0)
					{
						garbage.push_back(pStatObj);
					}
					else
					{
						pStatObj->m_bCheckGarbage = false;
					}
				}
			}
		}

		// First ShutDown object clearing all pointers.
		for (i32 i = 0, num = (i32)garbage.size(); i < num; i++)
		{
			CStatObj* pStatObj = garbage[i];

			if (!m_bLockCGFResources && !IsResourceLocked(pStatObj->m_szFileName))
			{
				// only shutdown object if it can be deleted by InternalDeleteObject()
				pStatObj->ShutDown();
			}
		}

		// Then delete all garbage objects.
		for (i32 i = 0, num = (i32)garbage.size(); i < num; i++)
		{
			CStatObj* pStatObj = garbage[i];
			InternalDeleteObject(pStatObj);
		}

	}
}

//////////////////////////////////////////////////////////////////////////
IRenderMesh* CObjUpr::GetRenderMeshBox()
{
	if (!m_pRMBox)
	{
		MakeUnitCube();
	}
	return m_pRMBox;
}

//////////////////////////////////////////////////////////////////////////
void CObjUpr::CheckForGarbage(CStatObj* pObject)
{
	if (m_bGarbageCollectionEnabled &&
	    !pObject->m_bCheckGarbage)
	{
		pObject->m_bCheckGarbage = true;
		m_checkForGarbage.push_back(pObject);
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjUpr::UnregisterForGarbage(CStatObj* pObject)
{
	DRX_ASSERT(pObject);

	if (m_bGarbageCollectionEnabled &&
	    pObject->m_bCheckGarbage)
	{
		m_checkForGarbage.try_remove(pObject);
		pObject->m_bCheckGarbage = false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CObjUpr::AddOrCreatePersistentRenderObject(SRenderNodeTempData* pTempData, CRenderObject*& pRenderObject, const CLodValue* pLodValue, const Matrix34& transformationMatrix, const SRenderingPassInfo& passInfo) const
{
	DRX_ASSERT(pRenderObject == nullptr);
	const bool shouldGetOrCreatePermanentObject = (GetCVars()->e_PermanentRenderObjects && (pTempData || pRenderObject) && GetCVars()->e_DebugDraw == 0 && (!pLodValue || !pLodValue->DissolveRefA())) && 
		!(passInfo.IsRecursivePass() || (pTempData && (pTempData->userData.m_pFoliage || (pTempData->userData.pOwnerNode && (pTempData->userData.pOwnerNode->GetRndFlags() & ERF_SELECTED)))));
	if (shouldGetOrCreatePermanentObject)
	{
		if (pLodValue && pLodValue->LodA() == -1 && pLodValue->LodB() == -1)
			return true;

		if (pLodValue && pLodValue->LodA() == -1 && pLodValue->DissolveRefB() == 255)
			return true;

		if (pLodValue && pLodValue->LodB() == -1 && pLodValue->DissolveRefA() == 255)
			return true;

		i32 nLod = pLodValue ? CLAMP(0, pLodValue->LodA(), MAX_STATOBJ_LODS_NUM - 1) : 0;

		u32 passId = passInfo.IsShadowPass() ? 1 : 0;
		u32 passMask = BIT(passId);

		pRenderObject = pTempData->GetRenderObject(nLod);

		// Update instance only for dirty objects
		const auto instanceDataDirty = pRenderObject->m_bInstanceDataDirty[passId];
		passInfo.GetIRenderView()->AddPermanentObject(pRenderObject, passInfo);

		// Has this object already been filled?
		i32 previousMask = DrxInterlockedExchangeOr(reinterpret_cast< LONG*>(&pRenderObject->m_passReadyMask), passMask);
		if (previousMask & passMask) // Object drawn once => fast path.
		{
			if (instanceDataDirty)
			{
				// Update instance matrix
				pRenderObject->SetMatrix(transformationMatrix, passInfo);
				pRenderObject->m_bInstanceDataDirty[passId] = false;
			}

			if (GetCVars()->e_BBoxes && pTempData && pTempData->userData.pOwnerNode)
				GetObjUpr()->RenderObjectDebugInfo(pTempData->userData.pOwnerNode, pRenderObject->m_fDistance, passInfo);

			return true;
		}

		// Permanent object needs to be filled first time,
		if (pTempData && pTempData->userData.pOwnerNode)
			pTempData->userData.nStatObjLastModificationId = GetResourcesModificationChecksum(pTempData->userData.pOwnerNode);
	}
	else
	{
		// Fallback to temporary render object
		pRenderObject = passInfo.GetIRenderView()->AllocateTemporaryRenderObject();
	}

	// We do not have a persistant render object
	// Always update instance matrix
	pRenderObject->SetMatrix(transformationMatrix, passInfo);

	return false;
}

//////////////////////////////////////////////////////////////////////////
u32 CObjUpr::GetResourcesModificationChecksum(IRenderNode* pOwnerNode) const
{
	u32 nModificationId = 1;

	if (CStatObj* pStatObj = (CStatObj*)pOwnerNode->GetEntityStatObj())
		nModificationId += pStatObj->GetModificationId();

	if (CMatInfo* pMatInfo = (CMatInfo*)pOwnerNode->GetMaterial())
		nModificationId += pMatInfo->GetModificationId();

	if (pOwnerNode->GetRenderNodeType() == eERType_TerrainSector)
		nModificationId += ((CTerrainNode*)pOwnerNode)->GetMaterialsModificationId();

	return nModificationId;
}

//////////////////////////////////////////////////////////////////////////
IRenderMesh* CObjUpr::GetBillboardRenderMesh(IMaterial* pMaterial)
{
	if(!m_pBillboardMesh)
	{
		PodArray<SVF_P3F_C4B_T2F> arrVertices;
		PodArray<SPipTangents> arrTangents;
		PodArray<vtx_idx> arrIndices;

		SVF_P3F_C4B_T2F vert;
		ZeroStruct(vert);
		vert.st = Vec2(0.0f, 0.0f);
		vert.color.dcolor = -1;

		// verts
		vert.xyz.Set(+.5, 0, -.5);
		vert.st = Vec2(1, 1);
		arrVertices.Add(vert);
		vert.xyz.Set(-.5, 0, -.5);
		vert.st = Vec2(0, 1);
		arrVertices.Add(vert);
		vert.xyz.Set(+.5, 0, +.5);
		vert.st = Vec2(1, 0);
		arrVertices.Add(vert);
		vert.xyz.Set(-.5, 0, +.5);
		vert.st = Vec2(0, 0);
		arrVertices.Add(vert);

		// tangents
		arrTangents.Add(SPipTangents(Vec3(1, 0, 0), Vec3(0, 0, 1), 1));
		arrTangents.Add(SPipTangents(Vec3(1, 0, 0), Vec3(0, 0, 1), 1));
		arrTangents.Add(SPipTangents(Vec3(1, 0, 0), Vec3(0, 0, 1), 1));
		arrTangents.Add(SPipTangents(Vec3(1, 0, 0), Vec3(0, 0, 1), 1));

		// indices
		arrIndices.Add(0);
		arrIndices.Add(1);
		arrIndices.Add(2);
		arrIndices.Add(1);
		arrIndices.Add(3);
		arrIndices.Add(2);

		m_pBillboardMesh = DinrusX3dEngBase::GetRenderer()->CreateRenderMeshInitialized(
			arrVertices.GetElements(), arrVertices.Count(), EDefaultInputLayouts::P3F_C4B_T2F,
			arrIndices.GetElements(), arrIndices.Count(), prtTriangleList,
			"Billboard", "Billboard", eRMT_Static, 1, 0, NULL, NULL, false, true,
			arrTangents.GetElements());

		m_pBillboardMesh->SetChunk(pMaterial, 0, arrVertices.Count(), 0, arrIndices.Count(), 1);
	}

	return m_pBillboardMesh;
}
