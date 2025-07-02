// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   3dengineload.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Загрузка уровней.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>

#include <drx3D/Eng3D/3dEngine.h>
#include <drx3D/Eng3D/terrain.h>
#include <drx3D/Eng3D/ObjMan.h>
#include <drx3D/Eng3D/VisAreas.h>
#include <drx3D/CoreX/ParticleSys/IParticles.h>
#include <drx3D/Eng3D/ParticleSystem.h>
#include <drx3D/Eng3D/DecalUpr.h>
#include <drx3D/Eng3D/Vegetation.h>
#include <drx3D/Eng3D/Brush.h>
#include <drx3D/Eng3D/MatMan.h>
#include <drx3D/Eng3D/IndexedMesh.h>
#include <drx3D/Eng3D/SkyLightUpr.h>
#include <drx3D/Eng3D/ObjectsTree.h>
#include <drx3D/Eng3D/LightEntity.h>
#include <drx3D/Eng3D/WaterWaveRenderNode.h>
#include <drx3D/Eng3D/RoadRenderNode.h>
#include <drx3D/Eng3D/PhysCallbacks.h>
#include <drx3D/Eng3D/TimeOfDay.h>
#include <drx3D/Eng3D/LightEntity.h>
#include <drx3D/Eng3D/RenderMeshMerger.h>
#include <drx3D/Eng3D/FogVolumeRenderNode.h>
#include <drx3D/Eng3D/RopeRenderNode.h>
#include <drx3D/Eng3D/MergedMeshRenderNode.h>
#include <drx3D/Eng3D/BreezeGenerator.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Act/IMaterialEffects.h>
#include <drx3D/Eng3D/ClipVolumeUpr.h>

#if defined(FEATURE_SVO_GI)
	#include <drx3D/Eng3D/SceneTreeUpr.h>
#endif

#define LEVEL_DATA_FILE          "LevelData.xml"
#define CUSTOM_MATERIALS_FILE    "Materials.xml"
#define PARTICLES_FILE           "LevelParticles.xml"
#define SHADER_LIST_FILE         "ShadersList.txt"
#define LEVEL_CONFIG_FILE        "Level.cfg"
#define LEVEL_EDITOR_CONFIG_FILE "Editor.cfg"

inline Vec3 StringToVector(tukk str)
{
	Vec3 vTemp(0, 0, 0);
	float x, y, z;
	if (sscanf(str, "%f,%f,%f", &x, &y, &z) == 3)
	{
		vTemp(x, y, z);
	}
	else
	{
		vTemp(0, 0, 0);
	}
	return vTemp;
}

//////////////////////////////////////////////////////////////////////////
void C3DEngine::SetLevelPath(tukk szFolderName)
{
	// make folder path
	assert(strlen(szFolderName) < 1024);
	drx_strcpy(m_szLevelFolder, szFolderName);
	if (strlen(m_szLevelFolder) > 0)
	{
		if (m_szLevelFolder[strlen(m_szLevelFolder) - 1] != '/')
			drx_strcat(m_szLevelFolder, "/");
	}
}

//////////////////////////////////////////////////////////////////////////
void C3DEngine::LoadDefaultAssets()
{
	if (GetRenderer())
	{
		GetRenderer()->InitSystemResources(FRR_SYSTEM_RESOURCES);
		m_nBlackTexID = GetRenderer()->EF_LoadTexture("%ENGINE%/EngineAssets/Textures/black.dds", FT_DONT_STREAM)->GetTextureID();
		m_nBlackCMTexID = GetRenderer()->EF_LoadTexture("%ENGINE%/EngineAssets/Textures/BlackCM.dds", FT_DONT_RELEASE | FT_DONT_STREAM)->GetTextureID();
		m_nWhiteTexID = GetRenderer()->EF_LoadTexture("%ENGINE%/EngineAssets/Textures/white.dds", FT_DONT_STREAM)->GetTextureID();
	}

	//Add a call to refresh the loading screen and call the loading tick functions to ensure that no big gaps in coverage occur.
	SYNCHRONOUS_LOADING_TICK();

#if !defined(SYS_ENV_AS_STRUCT)
	PREFAST_ASSUME(gEnv);
#endif

	GetMatMan()->InitDefaults();

	m_pMatFogVolEllipsoid = GetMatMan()->LoadMaterial("%ENGINE%/EngineAssets/Materials/Fog/FogVolumeEllipsoid", false);
	m_pMatFogVolBox = GetMatMan()->LoadMaterial("%ENGINE%/EngineAssets/Materials/Fog/FogVolumeBox", false);

	if (GetRenderer())
	{
		if (!m_pRESky)
		{
			m_pRESky = (CRESky*)GetRenderer()->EF_CreateRE(eDATA_Sky); //m_pRESky->m_fAlpha = 1.f;
		}
		if (!m_pREHDRSky)
		{
			m_pREHDRSky = (CREHDRSky*)GetRenderer()->EF_CreateRE(eDATA_HDRSky);
		}

		if (!m_ptexIconLowMemoryUsage)
		{
			m_ptexIconLowMemoryUsage = GetRenderer()->EF_LoadTexture("%ENGINE%/EngineAssets/Icons/LowMemoryUsage.tif", FT_DONT_STREAM);
		}

		if (!m_ptexIconAverageMemoryUsage)
		{
			m_ptexIconAverageMemoryUsage = GetRenderer()->EF_LoadTexture("%ENGINE%/EngineAssets/Icons/AverageMemoryUsage.tif", FT_DONT_STREAM);
		}

		if (!m_ptexIconHighMemoryUsage)
		{
			m_ptexIconHighMemoryUsage = GetRenderer()->EF_LoadTexture("%ENGINE%/EngineAssets/Icons/HighMemoryUsage.tif", FT_DONT_STREAM);
		}

		if (!m_ptexIconEditorConnectedToConsole)
		{
			m_ptexIconEditorConnectedToConsole = GetRenderer()->EF_LoadTexture("%ENGINE%/EngineAssets/Icons/LivePreview.TIF", FT_DONT_STREAM);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool C3DEngine::InitLevelForEditor(tukk szFolderName, tukk szMissionName)
{
#if !DRX_PLATFORM_DESKTOP
	DRX_ASSERT_MESSAGE(0, "InitLevelForEditor not supported on consoles yet");
	return false;
#else
	LOADING_TIME_PROFILE_SECTION;

	m_bInUnload = false;
	m_bEditor = true;
	m_bAreaActivationInUse = false;
	m_bLayersActivated = true;

	ClearDebugFPSInfo();

	gEnv->pPhysicalWorld->DeactivateOnDemandGrid();

	gEnv->pEntitySystem->RegisterPhysicCallbacks();

	if (!szFolderName || !szFolderName[0])
	{ Warning("%s: Level name is not specified", __FUNCTION__); return 0; }

	if (!szMissionName || !szMissionName[0])
	{ Warning("%s: Mission name is not specified", __FUNCTION__); }

	SetLevelPath(szFolderName);

	// Load console vars specific to this level.
	if (GetPak()->IsFileExist(GetLevelFilePath(LEVEL_CONFIG_FILE)))
		GetISystem()->LoadConfiguration(GetLevelFilePath(LEVEL_CONFIG_FILE), 0, eLoadConfigLevel);

	if (GetPak()->IsFileExist(GetLevelFilePath(LEVEL_EDITOR_CONFIG_FILE)))
		GetISystem()->LoadConfiguration(GetLevelFilePath(LEVEL_EDITOR_CONFIG_FILE), 0, eLoadConfigLevel);

	if (!m_pObjUpr)
		m_pObjUpr = DrxAlignedNew<CObjUpr>();

	if (!m_pVisAreaUpr)
		m_pVisAreaUpr = new CVisAreaUpr();

	DRX_ASSERT(m_pClipVolumeUpr->GetClipVolumeCount() == 0);
	assert(gEnv->pCharacterUpr);

	//////////////////////////////////////////////////////////////////////////
	DrxComment("initializing merged mesh manager");
	m_pMergedMeshesUpr->Init();

	m_pBreezeGenerator->Initialize();

	if (m_pSkyLightUpr)
		m_pSkyLightUpr->InitSkyDomeMesh();

	// recreate particles and decals
	if (m_pPartUpr)
		m_pPartUpr->Reset();

	// recreate decals
	SAFE_DELETE(m_pDecalUpr);
	m_pDecalUpr = new CDecalUpr();

	// restore game state
	EnableOceanRendering(true);
	m_pObjUpr->m_bLockCGFResources = 0;

	LoadDefaultAssets();

	LoadParticleEffects(szFolderName);

	if (!m_pWaterWaveUpr)
	{
		m_pWaterWaveUpr = new CWaterWaveUpr();
	}

	{
		string SettingsFileName = GetLevelFilePath("ScreenshotMap.Settings");

		FILE* metaFile = gEnv->pDrxPak->FOpen(SettingsFileName, "r");
		if (metaFile)
		{
			char Data[1024 * 8];
			gEnv->pDrxPak->FRead(Data, sizeof(Data), metaFile);
			sscanf(Data, "<Map CenterX=\"%f\" CenterY=\"%f\" SizeX=\"%f\" SizeY=\"%f\" Height=\"%f\"  Quality=\"%d\" Orientation=\"%d\" />",
			       &GetCVars()->e_ScreenShotMapCenterX,
			       &GetCVars()->e_ScreenShotMapCenterY,
			       &GetCVars()->e_ScreenShotMapSizeX,
			       &GetCVars()->e_ScreenShotMapSizeY,
			       &GetCVars()->e_ScreenShotMapCamHeight,
			       &GetCVars()->e_ScreenShotQuality,
			       &GetCVars()->e_ScreenShotMapOrientation);
			gEnv->pDrxPak->FClose(metaFile);
		}
	}

	LoadPhysicsData();

	GetObjUpr()->LoadOcclusionMesh(szFolderName);

	return (true);
#endif
}

bool C3DEngine::LoadTerrain(XmlNodeRef pDoc, std::vector<struct IStatObj*>** ppStatObjTable, std::vector<IMaterial*>** ppMatTable)
{
	LOADING_TIME_PROFILE_SECTION;

	PrintMessage("===== Loading %s =====", COMPILED_HEIGHT_MAP_FILE_NAME);

	// open file
	FILE* f = GetPak()->FOpen(GetLevelFilePath(COMPILED_HEIGHT_MAP_FILE_NAME), "rbx");
	if (!f)
		return 0;

	// read header
	STerrainChunkHeader header;
	if (!GetPak()->FRead(&header, 1, f, false))
	{
		GetPak()->FClose(f);
		return 0;
	}

	SwapEndian(header, (header.nFlags & SERIALIZATION_FLAG_BIG_ENDIAN) ? eBigEndian : eLittleEndian);
	m_bLevelFilesEndian = (header.nFlags & SERIALIZATION_FLAG_BIG_ENDIAN) ? eBigEndian : eLittleEndian;

	// detect old header format where unitSize_InMeters was an i32 (now it is float)
	if (header.TerrainInfo.unitSize_InMeters < 0.25f || header.TerrainInfo.unitSize_InMeters > 64.f)
	{
		i32 unitSize_InMeters = *(i32*)&header.TerrainInfo.unitSize_InMeters;
		header.TerrainInfo.unitSize_InMeters = (float)unitSize_InMeters;
	}

	if (header.nChunkSize)
	{
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Terrain, 0, "Terrain");

		if (!m_pTerrain)
			m_pTerrain = (CTerrain*)CreateTerrain(header.TerrainInfo);

		m_pTerrain->LoadSurfaceTypesFromXML(pDoc);

		if (!m_pTerrain->Load(f, header.nChunkSize - sizeof(STerrainChunkHeader), &header, ppStatObjTable, ppMatTable))
		{
			delete m_pTerrain;
			m_pTerrain = NULL;
		}
	}

	assert(GetPak()->FEof(f));

	GetPak()->FClose(f);

	return m_pTerrain != NULL;
}

bool C3DEngine::LoadVisAreas(std::vector<struct IStatObj*>** ppStatObjTable, std::vector<IMaterial*>** ppMatTable)
{
	LOADING_TIME_PROFILE_SECTION;

	PrintMessage("===== Loading %s =====", COMPILED_VISAREA_MAP_FILE_NAME);

	// open file
	FILE* f = GetPak()->FOpen(GetLevelFilePath(COMPILED_VISAREA_MAP_FILE_NAME), "rbx");
	if (!f)
		return false;

	// read header
	SVisAreaManChunkHeader header;
	if (!GetPak()->FRead(&header, 1, f, false))
	{
		GetPak()->FClose(f);
		return 0;
	}

	SwapEndian(header, (header.nFlags & SERIALIZATION_FLAG_BIG_ENDIAN) ? eBigEndian : eLittleEndian);

	if (header.nChunkSize)
	{
		assert(!m_pVisAreaUpr);
		m_pVisAreaUpr = new CVisAreaUpr();
		if (!m_pVisAreaUpr->Load(f, header.nChunkSize, &header, *ppStatObjTable, *ppMatTable))
		{
			delete m_pVisAreaUpr;
			m_pVisAreaUpr = NULL;
		}
	}

	assert(GetPak()->FEof(f));

	GetPak()->FClose(f);

	return m_pVisAreaUpr != NULL;
}

//////////////////////////////////////////////////////////////////////////
void C3DEngine::UnloadLevel()
{
	LOADING_TIME_PROFILE_SECTION;
	if (GetRenderer())
	{
		GetRenderer()->EnableLevelUnloading(true);
	}
	GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_LEVEL_UNLOAD, 0, 0);

	if (GetRenderer())
	{
		GetRenderer()->EnableLevelUnloading(false);
	}

	m_bInUnload = true;
	m_szLevelFolder[0] = 0;

	if (GetRenderer())
	{
		GetRenderer()->FlushRTCommands(true, true, true);
	}

	//////////////////////////////////////////////////////////////////////////
	// delete all rendernodes marked for deletion
	{
		DrxComment("Deleting render nodes");
		for (i32 i = 0; i < DRX_ARRAY_COUNT(m_renderNodesToDelete); ++i)
			TickDelayedRenderNodeDeletion();
		DrxComment("done");
	}

	// release CGF and materials table
	for (u32 i = 0; m_pLevelStatObjTable && i < m_pLevelStatObjTable->size(); i++)
	{
		SAFE_RELEASE((*m_pLevelStatObjTable)[i]);
	}
	SAFE_DELETE(m_pLevelStatObjTable);

	for (u32 i = 0; m_pLevelMaterialsTable && i < m_pLevelMaterialsTable->size(); i++)
	{
		SAFE_RELEASE((*m_pLevelMaterialsTable)[i]);
	}
	SAFE_DELETE(m_pLevelMaterialsTable);

	m_arrObjectLayersActivity.Reset();
	COctreeNode::m_nNodesCounterStreamable = 0;

#if defined(FEATURE_SVO_GI)
	CSvoUpr::Release();
#endif

	m_visibleNodesUpr.ClearAll();

	FreeFoliages();

	if (m_pSkyLightUpr)
		m_pSkyLightUpr->ReleaseSkyDomeMesh();

	// free vegetation and brush CGF's
	m_lstKilledVegetations.Reset();

	ResetPostEffects();

	if (m_pPartUpr)
	{
		// Force clear of all deferred release operations
		m_pPartUpr->ClearDeferredReleaseResources();
	}

	// delete decal manager
	if (m_pDecalUpr)
	{
		DrxComment("Deleting Decals");
		SAFE_DELETE(m_pDecalUpr);
		DrxComment("done");
	}

	if (m_pTerrain)
	{
		DrxComment("Deleting Terain");
		SAFE_DELETE(m_pTerrain);
		DrxComment("done");
	}

	// delete outdoor objects
	DrxComment("Deleting Octree");
	SAFE_DELETE(m_pObjectsTree);

	// delete indoors
	if (m_pVisAreaUpr)
	{
		DrxComment("Deleting VisAreas");
		SAFE_DELETE(m_pVisAreaUpr);
		DrxComment("done");
	}

	// free all clip volumes marked for delete
	{
		m_pClipVolumeUpr->TrimDeletedClipVolumes();
		DRX_ASSERT(m_pClipVolumeUpr->GetClipVolumeCount() == 0);
	}

	DRX_ASSERT(!COctreeNode::m_nNodesCounterAll);

	if (m_pWaterWaveUpr)
	{
		DrxComment("Deleting WaterWaves");
		SAFE_DELETE(m_pWaterWaveUpr);
		DrxComment("done");
	}

	m_LightVolumesMgr.Reset();

	m_pTerrainWaterMat = 0;
	m_nWaterBottomTexId = 0;

	//////////////////////////////////////////////////////////////////////////
	DrxComment("Removing Lights ...");
	for (i32 i = 0; i < m_lstDynLights.Count(); i++)
	{
		SRenderLight* pLight = m_lstDynLights[i];
		FreeLightSourceComponents(pLight);
	}
	m_lstDynLights.Reset();
	SAFE_DELETE(m_pSun);
	DrxComment("done");
	//////////////////////////////////////////////////////////////////////////

	CleanLevelShaders();

	if (m_pRESky)
		m_pRESky->Release(true);
	if (m_pREHDRSky)
		m_pREHDRSky->Release(true);
	m_pRESky = 0;
	m_pREHDRSky = 0;
	stl::free_container(m_skyMatName);
	stl::free_container(m_skyLowSpecMatName);

	if (m_nCloudShadowTexId)
	{
		ITexture* tex = GetRenderer()->EF_GetTextureByID(m_nCloudShadowTexId);
		if (tex)
			tex->Release();

		m_nCloudShadowTexId = 0;
		GetRenderer()->SetCloudShadowsParams(0, Vec3(0, 0, 0), 1, false, 1);
		SetGlobalParameter(E3DPARAM_VOLFOG_SHADOW_ENABLE, Vec3(0, 0, 0));
	}

	// reset volumetric cloud parameters
	{
		// reset wind influence, tiling size, and tiling offset
		SetGlobalParameter(E3DPARAM_VOLCLOUD_WIND_ATMOSPHERIC, Vec3(0.0f, 0.0f, 0.0f));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_TILING_SIZE, Vec3(64000.0f, 64000.0f, 4000.0f));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_TILING_OFFSET, Vec3(0.0f, 0.0f, 0.0f));

		SetGlobalParameter(E3DPARAM_VOLCLOUD_ENV_PARAMS, Vec3(6360000.0f, 0.0f, -1000.0f));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_GLOBAL_NOISE_SCALE, Vec3(3, 3, 5));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_RENDER_PARAMS, Vec3(100000.0f, 35000.0f, 64.0f));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_TURBULENCE_NOISE_SCALE, Vec3(1.0f, 1.0f, 1.0f));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_TURBULENCE_NOISE_PARAMS, Vec3(1.0f, 1.0f, 0.0f));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_DENSITY_PARAMS, Vec3(0.04f, 0.0f, 1.0f));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_MISC_PARAM, Vec3(0.2f, 0.1f, 16000.0f));

		// reset texture
		if (GetRenderer())
		{
			GetRenderer()->SetVolumetricCloudParams(0);
			GetRenderer()->SetVolumetricCloudNoiseTex(0, 0);
		}
	}

	if (m_nNightMoonTexId)
	{
		ITexture* tex = GetRenderer()->EF_GetTextureByID(m_nNightMoonTexId);
		if (tex)
			tex->Release();

		m_nNightMoonTexId = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	if (m_pPartUpr)
	{
		DrxComment("Purge particles");
		// Force to clean all particles that are left, even if still referenced.
		m_pPartUpr->ClearRenderResources(true);
		DrxComment("done");
	}

	//////////////////////////////////////////////////////////////////////////
	if (gEnv->pCharacterUpr)
	{
		// Moved here as the particles need to be torn down before the char instances
		DrxComment("Purge Characters");
		// Delete all character instances and models.
		gEnv->pCharacterUpr->ClearResources(true);
		DrxComment("done");
	}

	//////////////////////////////////////////////////////////////////////////
	if (m_pObjUpr)
	{
		bool bDeleteAll = !m_bEditor || m_bInShutDown;
		DrxComment("Deleting Static Objects");
		m_pObjUpr->UnloadObjects(bDeleteAll);
		m_pObjUpr->m_CullThread.UnloadLevel();
		DrxComment("done");
	}

	assert(m_pObjectsTree == NULL);

	COctreeNode::StaticReset();

	// Now that all meshes and objects are deleted we final release permanent renderobjects
	// as they hold references to materials.
	if (GetRenderer())
	{
		GetRenderer()->FreeSystemResources(FRR_PERMANENT_RENDER_OBJECTS);
	}

	//////////////////////////////////////////////////////////////////////////
	// Force delete all materials.
	//////////////////////////////////////////////////////////////////////////
	if (GetMatMan() && !m_bEditor)
	{
		// Should be after deleting all meshes.
		// We force delete all materials.
		DrxComment("Deleting Materials");
		GetMatMan()->ShutDown();
		DrxComment("done");
	}

	if (!gEnv->IsDedicated())
	{
		SAFE_RELEASE_FORCE(m_ptexIconLowMemoryUsage);
		SAFE_RELEASE_FORCE(m_ptexIconAverageMemoryUsage);
		SAFE_RELEASE_FORCE(m_ptexIconHighMemoryUsage);
		SAFE_RELEASE_FORCE(m_ptexIconEditorConnectedToConsole);

		//////////////////////////////////////////////////////////////////////////
		// Relases loaded default loaded textures.
		//////////////////////////////////////////////////////////////////////////
		{
			SAFE_RELEASE(m_ptexIconAverageMemoryUsage);
			SAFE_RELEASE(m_ptexIconLowMemoryUsage);
			SAFE_RELEASE(m_ptexIconHighMemoryUsage);
			SAFE_RELEASE(m_ptexIconEditorConnectedToConsole);
		}
	}
	else
	{
		// loading a texture will return default on the dedicated server,
		// therefore we just NULL the pointers here as it would be bad to delete them,
		// bad things happen if we do
		m_ptexIconAverageMemoryUsage = NULL;
		m_ptexIconLowMemoryUsage = NULL;
		m_ptexIconHighMemoryUsage = NULL;
		m_ptexIconEditorConnectedToConsole = NULL;
	}

	if (m_pOpticsUpr && !gEnv->IsEditor())
	{
		m_pOpticsUpr->Reset();
	}

	//////////////////////////////////////////////////////////////////////////
	m_pBreezeGenerator->Shutdown();

	//////////////////////////////////////////////////////////////////////////
	// Delete physics related things.
	//////////////////////////////////////////////////////////////////////////
	if (gEnv->pEntitySystem)
		gEnv->pEntitySystem->UnregisterPhysicCallbacks();
	UnloadPhysicsData();

	stl::free_container(m_lstRoadRenderNodesForUpdate);
	stl::free_container(m_lstAlwaysVisible);
	if (m_decalRenderNodes.empty())
		stl::free_container(m_decalRenderNodes);
	stl::free_container(m_tmpLstLights);
	stl::free_container(m_tmpLstAffectingLights);
	stl::free_container(m_lstPerObjectShadows);
	m_nCustomShadowFrustumCount = 0;

	DinrusX3dEngBase::m_pRenderMeshMerger->Reset();

	SAFE_DELETE(m_pTimeOfDay);
	CLightEntity::StaticReset();
	CVisArea::StaticReset();
	CRoadRenderNode::FreeStaticMemoryUsage();
	CFogVolumeRenderNode::StaticReset();
	CRopeRenderNode::StaticReset();

	if (GetRenderer())
	{
		GetRenderer()->FlushRTCommands(true, true, true);
	}

	IDeferredPhysicsEventUpr* pPhysEventUpr = GetDeferredPhysicsEventUpr();
	if (pPhysEventUpr)
	{
		pPhysEventUpr->ClearDeferredEvents();
	}

	//////////////////////////////////////////////////////////////////////////
	DrxComment("Shutting down merged mesh manager");
	m_pMergedMeshesUpr->Shutdown();

	//////////////////////////////////////////////////////////////////////////
	// clear data used for SRenderingPass
	stl::free_container(m_RenderingPassCameras[0]);
	stl::free_container(m_RenderingPassCameras[1]);
	stl::free_container(m_deferredRenderProxyStreamingPriorityUpdates);

	for (auto& pFr : m_lstCustomShadowFrustums)
	{
		DRX_ASSERT(pFr->Unique());
		SAFE_RELEASE(pFr);
	}
	stl::free_container(m_lstCustomShadowFrustums);

	stl::free_container(m_collisionClasses);

	DRX_ASSERT(m_lstStaticLights.empty());
	for (auto& renderNodes : m_renderNodesToDelete)
		DRX_ASSERT(renderNodes.empty());
}

//////////////////////////////////////////////////////////////////////////
void C3DEngine::LoadFlaresData()
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Flare data");

	string flareExportListPath = gEnv->p3DEngine->GetLevelFilePath(FLARE_EXPORT_FILE);
	XmlNodeRef pFlareRootNode = gEnv->pSystem->LoadXmlFromFile(flareExportListPath);

	if (pFlareRootNode == NULL)
		return;

	i32 nFlareExportFileVer = 0;
	pFlareRootNode->getAttr("Version", nFlareExportFileVer);

	for (i32 i = 0, iCount(pFlareRootNode->getChildCount()); i < iCount; ++i)
	{
		XmlNodeRef pFlareNode = pFlareRootNode->getChild(i);
		if (pFlareNode == NULL)
			continue;
		tukk flareName = NULL;
		if (!pFlareNode->getAttr("name", &flareName))
			continue;
		i32 nOutIndex(-1);

		if (nFlareExportFileVer == 0)
		{
			gEnv->pOpticsUpr->Load(flareName, nOutIndex);
		}
		else if (nFlareExportFileVer == 1)
		{
			if (pFlareNode->getChildCount() == 0)
			{
				gEnv->pOpticsUpr->Load(flareName, nOutIndex);
			}
			else if (pFlareNode->getChildCount() > 0)
			{
				gEnv->pOpticsUpr->Load(pFlareNode, nOutIndex);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool C3DEngine::LoadLevel(tukk szFolderName, tukk szMissionName)
{
	LOADING_TIME_PROFILE_SECTION;

#if !defined(SYS_ENV_AS_STRUCT)
	PREFAST_ASSUME(gEnv);
#endif

	stl::scoped_set<bool> setInLoad(m_bInLoad, true);

	m_bInUnload = false;
	m_bAreaActivationInUse = false;
	m_bLayersActivated = false;
	m_eShadowMode = ESM_NORMAL;

	m_vPrevMainFrameCamPos.Set(-1000000.f, -1000000.f, -1000000.f);
	m_vAverageCameraMoveDir = Vec3(0);
	m_fAverageCameraSpeed = 0;

	ClearDebugFPSInfo();

#if DRX_PLATFORM_DESKTOP
	m_bEditor = false;
#endif

	gEnv->pEntitySystem->RegisterPhysicCallbacks();

	assert(!m_bEditor);

	//////////////////////////////////////////////////////////////////////////
	DrxComment("initializing merged mesh manager");
	m_pMergedMeshesUpr->Init();

	//////////////////////////////////////////////////////////////////////////
	m_pBreezeGenerator->Initialize();

	if (!szFolderName || !szFolderName[0])
	{
		Warning("%s: Level name is not specified", __FUNCTION__);
		return 0;
	}

	if (!szMissionName || !szMissionName[0])
	{ Warning("%s: Mission name is not specified", __FUNCTION__); }

	char szMissionNameBody[256] = "NoMission";
	if (!szMissionName)
		szMissionName = szMissionNameBody;

	SetLevelPath(szFolderName);

	if (GetPak()->IsFileExist(GetLevelFilePath(LEVEL_CONFIG_FILE)))
		GetISystem()->LoadConfiguration(GetLevelFilePath(LEVEL_CONFIG_FILE), 0, eLoadConfigLevel);

	{
		// check is LevelData.xml file exist
		char sMapFileName[_MAX_PATH];
		drx_strcpy(sMapFileName, m_szLevelFolder);
		drx_strcat(sMapFileName, LEVEL_DATA_FILE);
		if (!IsValidFile(sMapFileName))
		{
			PrintMessage("Error: Level not found: %s", sMapFileName);
			return 0;
		}
	}

	if (!m_pObjUpr)
	{
		m_pObjUpr = DrxAlignedNew<CObjUpr>();
	}

	DRX_ASSERT(m_pClipVolumeUpr->GetClipVolumeCount() == 0);
	assert(gEnv->pCharacterUpr);

	// Load and activate all shaders used by the level before activating any shaders
	if (!m_bEditor && GetRenderer())
	{
		LoadUsedShadersList();

		GetRenderer()->EF_Query(EFQ_SetDynTexSourceLayerInfo);
	}

	LoadDefaultAssets();

	if (m_pSkyLightUpr)
		m_pSkyLightUpr->InitSkyDomeMesh();

	// Load LevelData.xml File.
	XmlNodeRef xmlLevelData = GetSystem()->LoadXmlFromFile(GetLevelFilePath(LEVEL_DATA_FILE));

	if (xmlLevelData == 0)
	{
		Error("%s: xml file not found (files missing?)", __FUNCTION__);  // files missing ?
		return false;
	}

	// re-create decal manager
	SAFE_DELETE(m_pDecalUpr);
	m_pDecalUpr = new CDecalUpr();

	SAFE_DELETE(m_pWaterWaveUpr);
	m_pWaterWaveUpr = new CWaterWaveUpr();

	gEnv->pSystem->SetSystemGlobalState(ESYSTEM_GLOBAL_STATE_LEVEL_LOAD_START_MATERIALS);
	if (GetCVars()->e_PreloadMaterials)
	{
		// Preload materials.
		GetMatMan()->PreloadLevelMaterials();
	}
	if (GetCVars()->e_PreloadDecals)
	{
		// Preload materials.
		GetMatMan()->PreloadDecalMaterials();
	}

	// Preload any geometry used by merged meshes
	m_pMergedMeshesUpr->PreloadMeshes();

	gEnv->pSystem->SetSystemGlobalState(ESYSTEM_GLOBAL_STATE_LEVEL_LOAD_START_OBJECTS);
	// preload level cgfs
	if (GetCVars()->e_StatObjPreload && !gEnv->IsEditor())
	{
		if (GetCVars()->e_StatObjPreload == 2)
			GetSystem()->OutputLoadingTimeStats();

		m_pObjUpr->PreloadLevelObjects();

		if (GetCVars()->e_StatObjPreload == 2)
		{
			GetSystem()->OutputLoadingTimeStats();
		}

		gEnv->pSystem->SetSystemGlobalState(ESYSTEM_GLOBAL_STATE_LEVEL_LOAD_START_CHARACTERS);
		if (gEnv->pCharacterUpr)
		{
			PrintMessage("Starting loading level characters ...");
			INDENT_LOG_DURING_SCOPE();
			float fStartTime = GetCurAsyncTimeSec();

			gEnv->pCharacterUpr->PreloadLevelModels();

			float dt = GetCurAsyncTimeSec() - fStartTime;
			PrintMessage("Finished loading level characters (%.1f sec)", dt);
		}
	}

	assert(!m_pLevelStatObjTable);
	assert(!m_pLevelMaterialsTable);
	assert(!m_arrObjectLayersActivity.Count());

	// load terrain
	XmlNodeRef nodeRef = xmlLevelData->findChild("SurfaceTypes");

	LoadCollisionClasses(xmlLevelData->findChild("CollisionClasses"));

	gEnv->pSystem->SetSystemGlobalState(ESYSTEM_GLOBAL_STATE_LEVEL_LOAD_START_STATIC_WORLD);

#if defined(FEATURE_SVO_GI)
	if (GetCVars()->e_svoTI_Active >= 0)
	{
		// Load SVOGI settings (must be called before loading of brushes, vegetation and textures)
		char szFileName[256];
		drx_sprintf(szFileName, "mission_%s.xml", szMissionName);
		XmlNodeRef xmlMission = GetSystem()->LoadXmlFromFile(Get3DEngine()->GetLevelFilePath(szFileName));
		if (xmlMission)
			LoadTISettings(xmlMission->findChild("Environment"));
	}
#endif

	if (!LoadTerrain(nodeRef, &m_pLevelStatObjTable, &m_pLevelMaterialsTable))
	{
		Error("Terrain file (%s) not found or file version error, please try to re-export the level", COMPILED_HEIGHT_MAP_FILE_NAME);
		return false;
	}

	// load indoors
	if (!LoadVisAreas(&m_pLevelStatObjTable, &m_pLevelMaterialsTable))
	{
		Error("VisAreas file (%s) not found or file version error, please try to re-export the level", COMPILED_VISAREA_MAP_FILE_NAME);
		return false;
	}

	COctreeNode::FreeLoadingCache();

	// Preload any geometry used by merged meshes
	if (m_pMergedMeshesUpr->SyncPreparationStep() == false)
	{
		Error("some merged meshes failed to prepare properly (missing cgfs, re-export?!)");
	}

	// re-create particles and decals
	if (m_pPartUpr)
		m_pPartUpr->Reset();

	//Update loading screen and important tick functions
	SYNCHRONOUS_LOADING_TICK();

	LoadParticleEffects(szFolderName);

	PrintMessage("===== Loading mission settings from XML =====");

	//Update loading screen and important tick functions
	SYNCHRONOUS_LOADING_TICK();

	// load leveldata.xml
	m_pTerrainWaterMat = 0;
	m_nWaterBottomTexId = 0;
	LoadMissionDataFromXMLNode(szMissionName);

	//Update loading screen and important tick functions
	SYNCHRONOUS_LOADING_TICK();

	if (!m_bShowTerrainSurface)
	{
		gEnv->pPhysicalWorld->SetHeightfieldData(NULL);
	}

	// init water if not initialized already (if no mission was found)
	if (m_pTerrain && !m_pTerrain->GetOcean())
	{
		PrintMessage("===== Creating Ocean =====");
		m_pTerrain->InitTerrainWater(m_pTerrainWaterMat, m_nWaterBottomTexId);
	}

	PrintMessage("===== Load level physics data =====");
	LoadPhysicsData();
	LoadFlaresData();

	// restore game state
	EnableOceanRendering(true);
	m_pObjUpr->m_bLockCGFResources = 0;

	PrintMessage("===== loading occlusion mesh =====");

	GetObjUpr()->LoadOcclusionMesh(szFolderName);

	PrintMessage("===== Finished loading static world =====");

	m_skipedLayers.clear();

	if (gEnv->pMaterialEffects)
	{
		gEnv->pMaterialEffects->CompleteInit();
	}

	return (true);
}

//////////////////////////////////////////////////////////////////////////
void C3DEngine::LoadPhysicsData()
{
	CPhysCallbacks::Init();
}

static void OnReleaseGeom(IGeometry* pGeom)
{
	if (IStatObj* pStatObj = (IStatObj*)pGeom->GetForeignData())
		pStatObj->Release();
}

void C3DEngine::UnloadPhysicsData()
{
	if (m_pGlobalWind != 0)
	{
		gEnv->pPhysicalWorld->DestroyPhysicalEntity(m_pGlobalWind);
		m_pGlobalWind = 0;
	}
	if (gEnv->pPhysicalWorld)
	{
		// Pause and wait for the physics
		gEnv->pSystem->SetThreadState(ESubsys_Physics, false);

		gEnv->pPhysicalWorld->RemoveAllExplosionShapes(OnReleaseGeom);
		gEnv->pPhysicalWorld->GetGeomUpr()->UnregisterAllCracks(OnReleaseGeom);

		DrxComment("Physics PurgeDeletedEntities...");
		gEnv->pPhysicalWorld->DeactivateOnDemandGrid();
		gEnv->pPhysicalWorld->PurgeDeletedEntities();
		DrxComment("done");

		CPhysCallbacks::Done();

		if (!gEnv->IsEditor())
		{
			DrxComment("Physics Cleanup...");
			gEnv->pPhysicalWorld->Cleanup();
			DrxComment("Physics Cleanup done");
		}
	}
	//////////////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////////////
void C3DEngine::LoadCollisionClasses(XmlNodeRef node)
{
	m_collisionClasses.clear();
	if (node)
	{
		i32 count = node->getChildCount();
		m_collisionClasses.reserve(count);
		for (i32 i = 0; i < count; i++)
		{
			SCollisionClass cc(0, 0);
			XmlNodeRef xmlCC = node->getChild(i);
			xmlCC->getAttr("type", cc.type);
			xmlCC->getAttr("ignore", cc.ignore);
			m_collisionClasses.push_back(cc);
		}
	}
}

void C3DEngine::FreeFoliages()
{
	if (m_pFirstFoliage)
	{
		DrxComment("Removing physicalized foliages ...");
		CStatObjFoliage* pFoliage, * pFoliageNext;
		for (pFoliage = m_pFirstFoliage; &pFoliage->m_next != &m_pFirstFoliage; pFoliage = pFoliageNext)
		{
			pFoliageNext = pFoliage->m_next;
			delete pFoliage;
		}
		DrxComment("done");
	}
	m_arrEntsInFoliage.Reset();
}

void C3DEngine::LoadTerrainSurfacesFromXML(XmlNodeRef pDoc, bool bUpdateTerrain)
{
	if (!m_pTerrain)
		return;

	m_pTerrain->LoadSurfaceTypesFromXML(pDoc);

	m_pTerrain->UpdateSurfaceTypes();

	m_pTerrain->InitHeightfieldPhysics();
}

void C3DEngine::LoadMissionDataFromXMLNode(tukk szMissionName)
{
	LOADING_TIME_PROFILE_SECTION;

	if (!m_pTerrain)
	{
		Warning("Calling %s while level is not loaded", __FUNCTION__);
		return;
	}

	/*
	   if (GetRenderer())
	   {
	   GetRenderer()->MakeMainContextActive();
	   }
	 */

	// set default values
	m_vFogColor(1, 1, 1);
	m_fMaxViewDistHighSpec = 8000;
	m_fMaxViewDistLowSpec = 1000;
	m_fTerrainDetailMaterialsViewDistRatio = 1;
	m_vDefFogColor = m_vFogColor;

	// mission environment
	if (szMissionName && szMissionName[0])
	{
		char szFileName[256];
		drx_sprintf(szFileName, "mission_%s.xml", szMissionName);
		XmlNodeRef xmlMission = GetSystem()->LoadXmlFromFile(Get3DEngine()->GetLevelFilePath(szFileName));
		if (xmlMission)
		{
			LoadEnvironmentSettingsFromXML(xmlMission->findChild("Environment"));
			LoadTimeOfDaySettingsFromXML(xmlMission->findChild("TimeOfDay"));
		}
		else
			Error("%s: Mission file not found: %s", __FUNCTION__, szFileName);
	}
	else
		Error("%s: Mission name is not defined", __FUNCTION__);
}

tuk C3DEngine::GetXMLAttribText(XmlNodeRef pInputNode, tukk szLevel1, tukk szLevel2, tukk szDefaultValue)
{
	static char szResText[128];

	drx_strcpy(szResText, szDefaultValue);

	XmlNodeRef nodeLevel = pInputNode->findChild(szLevel1);
	if (nodeLevel && nodeLevel->haveAttr(szLevel2))
	{
		drx_strcpy(szResText, nodeLevel->getAttr(szLevel2));
	}

	return szResText;
}

tuk C3DEngine::GetXMLAttribText(XmlNodeRef pInputNode, tukk szLevel1, tukk szLevel2, tukk szLevel3, tukk szDefaultValue)
{
	static char szResText[128];

	drx_strcpy(szResText, szDefaultValue);

	XmlNodeRef nodeLevel = pInputNode->findChild(szLevel1);
	if (nodeLevel)
	{
		nodeLevel = nodeLevel->findChild(szLevel2);
		if (nodeLevel)
		{
			drx_strcpy(szResText, nodeLevel->getAttr(szLevel3));
		}
	}

	return szResText;
}

bool C3DEngine::GetXMLAttribBool(XmlNodeRef pInputNode, tukk szLevel1, tukk szLevel2, bool bDefaultValue)
{
	bool bResult = bDefaultValue;

	XmlNodeRef nodeLevel = pInputNode->findChild(szLevel1);
	if (nodeLevel && nodeLevel->haveAttr(szLevel2))
	{
		nodeLevel->getAttr(szLevel2, bResult);
	}

	return bResult;
}

void C3DEngine::UpdateMoonDirection()
{
	float moonLati(-gf_PI + gf_PI * m_moonRotationLatitude / 180.0f);
	float moonLong(0.5f * gf_PI - gf_PI * m_moonRotationLongitude / 180.0f);

	float sinLon(sinf(moonLong));
	float cosLon(cosf(moonLong));
	float sinLat(sinf(moonLati));
	float cosLat(cosf(moonLati));

	m_moonDirection = Vec3(sinLon * cosLat, sinLon * sinLat, cosLon);
}

void C3DEngine::LoadEnvironmentSettingsFromXML(XmlNodeRef pInputNode)
{
	PrintComment("Loading environment settings from XML ...");

	// set start and end time for dawn/dusk (to fade moon/sun light in and out)
	float dawnTime = (float) atof(GetXMLAttribText(pInputNode, "Lighting", "DawnTime", "355"));
	float dawnDuration = (float) atof(GetXMLAttribText(pInputNode, "Lighting", "DawnDuration", "10"));
	float duskTime = (float) atof(GetXMLAttribText(pInputNode, "Lighting", "DuskTime", "365"));
	float duskDuration = (float) atof(GetXMLAttribText(pInputNode, "Lighting", "DuskDuration", "10"));

	m_dawnStart = (dawnTime - dawnDuration * 0.5f) / 60.0f;
	m_dawnEnd = (dawnTime + dawnDuration * 0.5f) / 60.0f;
	m_duskStart = 12.0f + (duskTime - duskDuration * 0.5f) / 60.0f;
	m_duskEnd = 12.0f + (duskTime + duskDuration * 0.5f) / 60.0f;

	if (m_dawnEnd > m_duskStart)
	{
		m_duskEnd += m_dawnEnd - m_duskStart;
		m_duskStart = m_dawnEnd;
	}

	// get moon info
	m_moonRotationLatitude = (float) atof(GetXMLAttribText(pInputNode, "Moon", "Latitude", "240"));
	m_moonRotationLongitude = (float) atof(GetXMLAttribText(pInputNode, "Moon", "Longitude", "45"));
	UpdateMoonDirection();

	m_nightMoonSize = (float) atof(GetXMLAttribText(pInputNode, "Moon", "Size", "0.5"));

	{
		char moonTexture[256];
		drx_strcpy(moonTexture, GetXMLAttribText(pInputNode, "Moon", "Texture", ""));

		ITexture* pTex(0);
		if (moonTexture[0] != '\0' && GetRenderer())
		{
			pTex = GetRenderer()->EF_LoadTexture(moonTexture, FT_DONT_STREAM);
		}
		m_nNightMoonTexId = pTex ? pTex->GetTextureID() : 0;
	}

	// max view distance
	m_fMaxViewDistHighSpec = (float)atol(GetXMLAttribText(pInputNode, "Fog", "ViewDistance", "8000"));
	m_fMaxViewDistLowSpec = (float)atol(GetXMLAttribText(pInputNode, "Fog", "ViewDistanceLowSpec", "1000"));
	m_fMaxViewDistScale = 1.f;

	m_volFogGlobalDensityMultiplierLDR = (float) max(atof(GetXMLAttribText(pInputNode, "Fog", "LDRGlobalDensMult", "1.0")), 0.0);

	float fTerrainDetailMaterialsViewDistRatio = (float)atof(GetXMLAttribText(pInputNode, "Terrain", "DetailLayersViewDistRatio", "1.0"));
	if (m_fTerrainDetailMaterialsViewDistRatio != fTerrainDetailMaterialsViewDistRatio && GetTerrain())
		GetTerrain()->ResetTerrainVertBuffers(NULL);
	m_fTerrainDetailMaterialsViewDistRatio = fTerrainDetailMaterialsViewDistRatio;

	// SkyBox
	m_skyMatName = GetXMLAttribText(pInputNode, "SkyBox", "Material", "%ENGINE%/EngineAssets/Materials/sky/Sky");
	m_skyLowSpecMatName = GetXMLAttribText(pInputNode, "SkyBox", "MaterialLowSpec", "%ENGINE%/EngineAssets/Materials/sky/Sky");

	// Forces the engine to reload the material of the skybox in the next time it renders it.
	m_pSkyMat = NULL;
	m_pSkyLowSpecMat = NULL;

	m_fSkyBoxAngle = (float)atof(GetXMLAttribText(pInputNode, "SkyBox", "Angle", "0.0"));
	m_fSkyBoxStretching = (float)atof(GetXMLAttribText(pInputNode, "SkyBox", "Stretching", "1.0"));

	// set terrain water, sun road and bottom shaders
	char szTerrainWaterMatName[256];
	drx_strcpy(szTerrainWaterMatName, GetXMLAttribText(pInputNode, "Ocean", "Material", "%ENGINE%/EngineAssets/Materials/Water/Ocean_default"));
	m_pTerrainWaterMat = szTerrainWaterMatName[0] ? GetMatMan()->LoadMaterial(szTerrainWaterMatName, false) : NULL;

	if (m_pTerrain)
		m_pTerrain->InitTerrainWater(m_pTerrainWaterMat, 0);

	m_oceanCausticsMultiplier = (float) atof(GetXMLAttribText(pInputNode, "Ocean", "CausticsMultiplier", "0.85"));
	m_oceanCausticsDistanceAtten = (float) atof(GetXMLAttribText(pInputNode, "Ocean", "CausticsDistanceAtten", "100.0"));
	m_oceanCausticsTilling = (float) atof(GetXMLAttribText(pInputNode, "Ocean", "CausticsTilling", "1.0"));

	m_oceanWindDirection = (float) atof(GetXMLAttribText(pInputNode, "OceanAnimation", "WindDirection", "1.0"));
	m_oceanWindSpeed = (float) atof(GetXMLAttribText(pInputNode, "OceanAnimation", "WindSpeed", "4.0"));
	m_oceanWavesAmount = (float) atof(GetXMLAttribText(pInputNode, "OceanAnimation", "WavesAmount", "1.5"));
	m_oceanWavesAmount = clamp_tpl<float>(m_oceanWavesAmount, 0.4f, 3.0f);
	m_oceanWavesSize = (float) atof(GetXMLAttribText(pInputNode, "OceanAnimation", "WavesSize", "0.75"));
	m_oceanWavesSize = clamp_tpl<float>(m_oceanWavesSize, 0.0f, 3.0f);

	// disabled temporarily - we'll use fixed height instead with tweaked attenuation function
	m_oceanCausticHeight = 0.0f;
	//m_oceanCausticHeight = (float) atof( GetXMLAttribText(pInputNode, "Ocean", "CausticHeight", "2.5"));
	m_oceanCausticDepth = (float) atof(GetXMLAttribText(pInputNode, "Ocean", "CausticDepth", "8.0"));
	m_oceanCausticIntensity = (float) atof(GetXMLAttribText(pInputNode, "Ocean", "CausticIntensity", "1.0"));

	// get wind
	Vec3 vWindSpeed = StringToVector(GetXMLAttribText(pInputNode, "EnvState", "WindVector", "1,0,0"));
	SetWind(vWindSpeed);

	// Define breeze generation
	if (m_pBreezeGenerator)
	{
		m_pBreezeGenerator->Shutdown();

		m_pBreezeGenerator->m_enabled = GetXMLAttribBool(pInputNode, "EnvState", "BreezeGeneration", false);
		m_pBreezeGenerator->m_strength = (float)atof(GetXMLAttribText(pInputNode, "EnvState", "BreezeStrength", "1.f"));
		m_pBreezeGenerator->m_variance = (float)atof(GetXMLAttribText(pInputNode, "EnvState", "BreezeVariation", "1.f"));
		m_pBreezeGenerator->m_lifetime = (float)atof(GetXMLAttribText(pInputNode, "EnvState", "BreezeLifeTime", "15.f"));
		m_pBreezeGenerator->m_count = (u32)max(0, atoi(GetXMLAttribText(pInputNode, "EnvState", "BreezeCount", "4")));
		m_pBreezeGenerator->m_radius = (float)atof(GetXMLAttribText(pInputNode, "EnvState", "BreezeRadius", "5.f"));
		m_pBreezeGenerator->m_spawn_radius = (float)atof(GetXMLAttribText(pInputNode, "EnvState", "BreezeSpawnRadius", "25.f"));
		m_pBreezeGenerator->m_spread = (float)atof(GetXMLAttribText(pInputNode, "EnvState", "BreezeSpread", "0.f"));
		m_pBreezeGenerator->m_movement_speed = (float)atof(GetXMLAttribText(pInputNode, "EnvState", "BreezeMovementSpeed", "8.f"));
		m_pBreezeGenerator->m_awake_thresh = (float)atof(GetXMLAttribText(pInputNode, "EnvState", "BreezeAwakeThreshold", "0"));
		m_pBreezeGenerator->m_wind_speed = vWindSpeed;
		m_pBreezeGenerator->m_fixed_height = (float)atof(GetXMLAttribText(pInputNode, "EnvState", "BreezeFixedHeight", "-1.f"));

		m_pBreezeGenerator->Initialize();
	}

	// Per-level mergedmeshes pool size (on consoles)
	DinrusX3dEngBase::m_mergedMeshesPoolSize = atoi(GetXMLAttribText(pInputNode, "EnvState", "ConsoleMergedMeshesPool", MMRM_DEFAULT_POOLSIZE_STR));

	// update relevant time of day settings
	ITimeOfDay* pTimeOfDay(GetTimeOfDay());
	if (pTimeOfDay)
	{
		CTimeOfDay::SEnvironmentInfo envTODInfo;
		{
			envTODInfo.bSunLinkedToTOD = GetXMLAttribBool(pInputNode, "EnvState", "SunLinkedToTOD", true);
		}
		// get rotation of sun around z axis (needed to define an arbitrary path over zenit for day/night cycle position calculations)
		envTODInfo.sunRotationLatitude = (float) atof(GetXMLAttribText(pInputNode, "Lighting", "SunRotation", "240"));
		envTODInfo.sunRotationLongitude = (float) atof(GetXMLAttribText(pInputNode, "Lighting", "Longitude", "90"));

		pTimeOfDay->SetEnvironmentSettings(envTODInfo);
	}

	{
		m_bShowTerrainSurface = GetXMLAttribBool(pInputNode, "EnvState", "ShowTerrainSurface", true);
	}

	{
		tukk pText = GetXMLAttribText(pInputNode, "EnvState", "SunShadowsMinSpec", "1");
		i32 nMinSpec = atoi(pText);
		if (nMinSpec > 0 && CheckMinSpec(nMinSpec))
			m_bSunShadows = true;
		else
			m_bSunShadows = false;
	}

	{
		tukk pText = GetXMLAttribText(pInputNode, "EnvState", "SunShadowsAdditionalCascadeMinSpec", "0");
		i32 nMinSpec = atoi(pText);
		if (nMinSpec > 0 && CheckMinSpec(nMinSpec))
			m_nSunAdditionalCascades = 1;
		else
			m_nSunAdditionalCascades = 0;
	}

	{
		if (auto cvar = m_pConsole->GetCVar("r_ShadowsCache"))
		{
			m_nGsmCache = cvar->GetIVal();
		}
		else
		{
			m_nGsmCache = 0;
		}
	}

	{
		m_bHeightMapAoEnabled = GetXMLAttribBool(pInputNode, "Terrain", "HeightMapAO", false);
	}

	{
		bool bIntegrateObjectsIntoTerrain = GetCVars()->e_TerrainIntegrateObjectsMaxVertices && GetXMLAttribBool(pInputNode, "Terrain", "IntegrateObjects", false);

		if (bIntegrateObjectsIntoTerrain != m_bIntegrateObjectsIntoTerrain && GetTerrain())
		{
			GetTerrain()->ResetTerrainVertBuffers(NULL);
			m_bIntegrateObjectsIntoTerrain = bIntegrateObjectsIntoTerrain;
		}
	}

	// Enable automatic base texture update based on terrain detail materials info. This cvar used only by the editor for now.
	// TODO: support on-the-fly in-engine base texture generation (including roughness and normals) without exporting it from the editor.
	GetCVars()->e_TerrainAutoGenerateBaseTexture = GetXMLAttribBool(pInputNode, "Terrain", "AutoGenerateBaseTexture", false);

	{
		i32 nMinSpec = 3;//atoi(pText);
		m_fSunClipPlaneRange = 256.0;
		m_fSunClipPlaneRangeShift = 0.0f;

		if (nMinSpec > 0 && CheckMinSpec(nMinSpec))
		{
			m_fSunClipPlaneRange = (float)atof(GetXMLAttribText(pInputNode, "EnvState", "SunShadowsClipPlaneRange", "256.0"));

			float fSunClipPlaneRangeShift = (float)atof(GetXMLAttribText(pInputNode, "EnvState", "SunShadowsClipPlaneRangeShift", "0.0"));
			m_fSunClipPlaneRangeShift = clamp_tpl(fSunClipPlaneRangeShift / 100.0f, 0.0f, 1.0f);
		}
	}

	{
		m_bSunShadowsFromTerrain = GetXMLAttribBool(pInputNode, "EnvState", "SunShadowsFromTerrain", false) && GetCVars()->e_GsmCastFromTerrain;
	}

	{
		Get3DEngine()->m_bAreaActivationInUse = GetXMLAttribBool(pInputNode, "EnvState", "UseLayersActivation", false);
	}

	// load cloud shadow parameters
	{
		char cloudShadowTexture[256];
		drx_strcpy(cloudShadowTexture, GetXMLAttribText(pInputNode, "CloudShadows", "CloudShadowTexture", ""));

		ITexture* pTex = 0;
		if (cloudShadowTexture[0] != '\0' && GetRenderer())
			pTex = GetRenderer()->EF_LoadTexture(cloudShadowTexture, FT_DONT_STREAM);

		m_nCloudShadowTexId = pTex ? pTex->GetTextureID() : 0;

		// Get animation parameters
		const Vec3 cloudShadowSpeed = StringToVector(GetXMLAttribText(pInputNode, "CloudShadows", "CloudShadowSpeed", "0,0,0"));

		const float cloudShadowTiling = (float)atof(GetXMLAttribText(pInputNode, "CloudShadows", "CloudShadowTiling", "1.0"));
		const float cloudShadowBrightness = (float)atof(GetXMLAttribText(pInputNode, "CloudShadows", "CloudShadowBrightness", "1.0"));

		const bool cloudShadowInvert = GetXMLAttribBool(pInputNode, "CloudShadows", "CloudShadowInvert", false);

		if (GetRenderer())
		{
			GetRenderer()->SetCloudShadowsParams(m_nCloudShadowTexId, cloudShadowSpeed, cloudShadowTiling, cloudShadowInvert, cloudShadowBrightness);
		}
	}

	// load volumetric cloud parameters
	{
#if 0 //disable spherical cloud layer until it works correctly.
		const bool sphericalCloud = GetXMLAttribBool(pInputNode, "VolumetricCloud", "SphericalCloud", false);
		const float volCloudEarthRadius = max(0.0f, (float)atof(GetXMLAttribText(pInputNode, "VolumetricCloud", "EarthRadius", "6360000.0")));
#else
		const bool sphericalCloud = false;
		const float volCloudEarthRadius = 6360000.0f;
#endif
		const float horizonHeight = (float)atof(GetXMLAttribText(pInputNode, "VolumetricCloud", "HorizonHeight", "-1000.0"));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_ENV_PARAMS, Vec3(volCloudEarthRadius, sphericalCloud ? 1.0f : 0.0f, horizonHeight));

		const Vec3 cloudNoiseScale = StringToVector(GetXMLAttribText(pInputNode, "VolumetricCloud", "GlobalCloudNoiseScale", "3,3,5"));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_GLOBAL_NOISE_SCALE, cloudNoiseScale);

		const float maxViewableDistance = min(1000000.0f, max(0.0f, (float)atof(GetXMLAttribText(pInputNode, "VolumetricCloud", "MaxViewableDistance", "100000.0"))));
		const float maxRayMarchDistance = min(200000.0f, max(0.0f, (float)atof(GetXMLAttribText(pInputNode, "VolumetricCloud", "MaxRayMarchDistance", "20000.0"))));
		const float maxRayMarchStep = (float)min(512, max(16, atoi(GetXMLAttribText(pInputNode, "VolumetricCloud", "MaxRayMarchStep", "64"))));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_RENDER_PARAMS, Vec3(maxViewableDistance, maxRayMarchDistance, maxRayMarchStep));

		const Vec3 cloudTilingSize = StringToVector(GetXMLAttribText(pInputNode, "VolumetricCloud", "TilingSize", "64000,64000,4000"));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_TILING_SIZE, cloudTilingSize);

		const Vec3 cloudTilingOffset = StringToVector(GetXMLAttribText(pInputNode, "VolumetricCloud", "TilingOffset", "0,0,0"));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_TILING_OFFSET, -cloudTilingOffset);

		const Vec3 edgeTurbulenceNoiseScale = StringToVector(GetXMLAttribText(pInputNode, "VolumetricCloud", "EdgeTurbulenceNoiseScale", "1,1,1"));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_TURBULENCE_NOISE_SCALE, edgeTurbulenceNoiseScale);

		const float edgeTurbulenceNoiseErode = GetXMLAttribBool(pInputNode, "VolumetricCloud", "EdgeTurbulenceNoiseErode", true) ? 1.0f : 0.0f;
		const float edgeTurbulenceNoiseAbsolute = GetXMLAttribBool(pInputNode, "VolumetricCloud", "EdgeTurbulenceNoiseAbsolute", true) ? 1.0f : 0.0f;
		SetGlobalParameter(E3DPARAM_VOLCLOUD_TURBULENCE_NOISE_PARAMS, Vec3(edgeTurbulenceNoiseErode, edgeTurbulenceNoiseAbsolute, 0.0f));

		const float maxGlobalCloudDensity = min(1.0f, max(0.0f, (float)atof(GetXMLAttribText(pInputNode, "VolumetricCloud", "MaxGlobalCloudDensity", "0.04"))));
		const float minRescaleCloudTexDensity = (float)atof(GetXMLAttribText(pInputNode, "VolumetricCloud", "RemapCloudDensityMin", "0.0"));
		const float maxRescaleCloudTexDensity = (float)atof(GetXMLAttribText(pInputNode, "VolumetricCloud", "RemapCloudDensityMax", "1.0"));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_DENSITY_PARAMS, Vec3(maxGlobalCloudDensity, minRescaleCloudTexDensity, maxRescaleCloudTexDensity));

		const float additionalNoiseIntensity = (float)atof(GetXMLAttribText(pInputNode, "VolumetricCloud", "AdditionalNoiseIntensity", "0.2"));
		const float shadingLOD = min(1.0f, max(0.0f, (float)atof(GetXMLAttribText(pInputNode, "VolumetricCloud", "ShadingLOD", "0.7"))));
		const float cloudShadowTilingSize = max(1.0f, (float)atof(GetXMLAttribText(pInputNode, "VolumetricCloud", "ShadowTilingSize", "16000")));
		SetGlobalParameter(E3DPARAM_VOLCLOUD_MISC_PARAM, Vec3(additionalNoiseIntensity, shadingLOD, cloudShadowTilingSize));

		if (GetRenderer())
		{
			char cloudVolumeTexture[256];
			drx_strcpy(cloudVolumeTexture, GetXMLAttribText(pInputNode, "VolumetricCloud", "CloudVolumeTexture", ""));
			ITexture* pTex = nullptr;
			if (cloudVolumeTexture[0] != '\0')
				pTex = GetRenderer()->EF_LoadTexture(cloudVolumeTexture, FT_DONT_STREAM);
			i32 volCloudTexId = pTex ? pTex->GetTextureID() : 0;
			GetRenderer()->SetVolumetricCloudParams(volCloudTexId);

			drx_strcpy(cloudVolumeTexture, GetXMLAttribText(pInputNode, "VolumetricCloud", "GlobalCloudNoiseVolumeTexture", ""));
			pTex = nullptr;
			if (cloudVolumeTexture[0] != '\0')
				pTex = GetRenderer()->EF_LoadTexture(cloudVolumeTexture, FT_DONT_STREAM);
			i32 volCloudNoiseTexId = pTex ? pTex->GetTextureID() : 0;

			drx_strcpy(cloudVolumeTexture, GetXMLAttribText(pInputNode, "VolumetricCloud", "EdgeTurbulenceNoiseVolumeTexture", ""));
			pTex = nullptr;
			if (cloudVolumeTexture[0] != '\0')
				pTex = GetRenderer()->EF_LoadTexture(cloudVolumeTexture, FT_DONT_STREAM);
			i32 volCloudEdgeNoiseTexId = pTex ? pTex->GetTextureID() : 0;

			GetRenderer()->SetVolumetricCloudNoiseTex(volCloudNoiseTexId, volCloudEdgeNoiseTexId);
		}
	}

	{
		const bool enable = GetXMLAttribBool(pInputNode, "VolFogShadows", "Enable", false);
		const bool enableForClouds = GetXMLAttribBool(pInputNode, "VolFogShadows", "EnableForClouds", false);

		SetGlobalParameter(E3DPARAM_VOLFOG_SHADOW_ENABLE, Vec3(enable ? 1.0f : 0.0f, enableForClouds ? 1.0f : 0.0f, 0.0f));
	}

	if (GetRenderer())
	{
		i32 dim[2];

		tukk pWidth = GetXMLAttribText(pInputNode, "DynTexSource", "Width", "256");
		dim[0] = atoi(pWidth);

		tukk pHeight = GetXMLAttribText(pInputNode, "DynTexSource", "Height", "256");
		dim[1] = atoi(pHeight);

		GetRenderer()->EF_Query(EFQ_SetDynTexSourceSharedRTDim, dim[0], dim[1]);
	}

#if defined(FEATURE_SVO_GI)
	LoadTISettings(pInputNode);
#endif

	if (pTimeOfDay)
		pTimeOfDay->Update();

	if (GetSystem()->GetISystemEventDispatcher())
		GetSystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_ENVIRONMENT_SETTINGS_CHANGED, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
void C3DEngine::LoadTimeOfDaySettingsFromXML(XmlNodeRef node)
{
	if (node)
	{
		GetTimeOfDay()->Serialize(node, true);
	}
}

//////////////////////////////////////////////////////////////////////////
void C3DEngine::LoadParticleEffects(tukk szFolderName)
{
	LOADING_TIME_PROFILE_SECTION;

	if (m_pPartUpr)
	{
		PrintMessage("===== Loading Particle Effects =====");

		m_pPartUpr->LoadLibrary("Level", GetLevelFilePath(PARTICLES_FILE), true);

		if (GetCVars()->e_ParticlesPreload)
		{
			// Force loading all effects and assets, to ensure no runtime stalls.
			CTimeValue t0 = GetTimer()->GetAsyncTime();
			PrintMessage("Preloading Particle Effects...");
			m_pPartUpr->LoadLibrary("*", NULL, true);
			CTimeValue t1 = GetTimer()->GetAsyncTime();
			float dt = (t1 - t0).GetSeconds();
			PrintMessage("Particle Effects Loaded in %.2f seconds", dt);
		}
		else
		{
			// Load just specified libraries.
			m_pPartUpr->LoadLibrary("@PreloadLibs", szFolderName, true);
		}
	}
}

//! create static object containing empty IndexedMesh
IStatObj* C3DEngine::CreateStatObj()
{
	CStatObj* pStatObj = new CStatObj();
	pStatObj->m_pIndexedMesh = new CIndexedMesh();
	return pStatObj;
}

IStatObj* C3DEngine::CreateStatObjOptionalIndexedMesh(bool createIndexedMesh)
{
	CStatObj* pStatObj = new CStatObj();
	if (createIndexedMesh)
		pStatObj->m_pIndexedMesh = new CIndexedMesh();
	return pStatObj;
}

bool C3DEngine::RestoreTerrainFromDisk()
{
	if (m_pTerrain && m_pObjUpr && !m_bEditor && GetCVars()->e_TerrainDeformations)
	{
		m_pTerrain->ResetTerrainVertBuffers(NULL);

		if (FILE* f = GetPak()->FOpen(GetLevelFilePath(COMPILED_HEIGHT_MAP_FILE_NAME), "rbx"))
		{
			GetTerrain()->ReloadModifiedHMData(f);
			GetPak()->FClose(f);
		}
	}

	ResetParticlesAndDecals();

	// update roads
	if (m_pObjectsTree && GetCVars()->e_TerrainDeformations)
	{
		PodArray<IRenderNode*> lstRoads;
		m_pObjectsTree->GetObjectsByType(lstRoads, eERType_Road, NULL);
		for (i32 i = 0; i < lstRoads.Count(); i++)
		{
			CRoadRenderNode* pRoad = (CRoadRenderNode*)lstRoads[i];
			pRoad->OnTerrainChanged();
		}
	}

	ReRegisterKilledVegetationInstances();

	return true;
}

void C3DEngine::ReRegisterKilledVegetationInstances()
{
	for (i32 i = 0; i < m_lstKilledVegetations.Count(); i++)
	{
		IRenderNode* pObj = m_lstKilledVegetations[i];
		pObj->Physicalize();
		Get3DEngine()->RegisterEntity(pObj);
	}

	m_lstKilledVegetations.Clear();
}

//////////////////////////////////////////////////////////////////////////
bool C3DEngine::LoadUsedShadersList()
{
	LOADING_TIME_PROFILE_SECTION;

	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "LoadUsedShadersList");

	GetRenderer()->EF_Query(EFQ_SetShaderCombinations);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool C3DEngine::PrecreateDecals()
{
	LOADING_TIME_PROFILE_SECTION;
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "PrecreateDecals");

	CObjUpr::DecalsToPrecreate& decals(GetObjUpr()->m_decalsToPrecreate);
	// pre-create ...
	if (GetCVars()->e_DecalsPreCreate)
	{
		DrxLog("Pre-creating %d decals...", (i32)decals.size());

		CObjUpr::DecalsToPrecreate::iterator it(decals.begin());
		CObjUpr::DecalsToPrecreate::iterator itEnd(decals.end());
		for (; it != itEnd; ++it)
		{
			IDecalRenderNode* pDecalRenderNode(*it);
			pDecalRenderNode->Precache();
		}

		DrxLog(" done.\n");
	}
	else
		DrxLog("Skipped pre-creation of decals.\n");

	// ... and discard list (even if pre-creation was skipped!)
	decals.resize(0);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Called by game when everything needed for level is loaded.
//////////////////////////////////////////////////////////////////////////
void C3DEngine::PostLoadLevel()
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "PostLoadLevel");
	LOADING_TIME_PROFILE_SECTION;

	//////////////////////////////////////////////////////////////////////////
	// Submit water material to physics
	//////////////////////////////////////////////////////////////////////////
	{
		IMaterialUpr* pMatMan = GetMaterialUpr();
		IPhysicalWorld* pPhysicalWorld = gEnv->pPhysicalWorld;
		IPhysicalEntity* pGlobalArea = pPhysicalWorld->AddGlobalArea();

		pe_params_buoyancy pb;
		pb.waterPlane.n.Set(0, 0, 1);
		pb.waterPlane.origin.Set(0, 0, GetWaterLevel());
		pGlobalArea->SetParams(&pb);

		ISurfaceType* pSrfType = pMatMan->GetSurfaceTypeByName("mat_water");
		if (pSrfType)
			pPhysicalWorld->SetWaterMat(pSrfType->GetId());

		pe_params_waterman pwm;
		pwm.nExtraTiles = 3;
		pwm.nCells = 40;
		pwm.tileSize = 4;
		pwm.waveSpeed = 11.0f;
		pwm.dampingCenter = 0.6f;
		pwm.dampingRim = 2.3f;
		//pPhysicalWorld->SetWaterUprParams(&pwm);
	}

	if (GetCVars()->e_PrecacheLevel)
	{
		// pre-create decals
		PrecreateDecals();
	}

	CompleteObjectsGeometry();

	gEnv->pSystem->SetSystemGlobalState(ESYSTEM_GLOBAL_STATE_LEVEL_LOAD_START_TEXTURES);

	if (GetRenderer())
	{
		GetRenderer()->PostLevelLoading();
	}

	// refresh material constants pulled in from resources (such as textures)
	GetMatMan()->RefreshShaderResourceConstants();

	if (m_nGsmCache > 0)
	{
		m_CachedShadowsBounds.Reset();
		SetRecomputeCachedShadows(m_nCachedShadowsUpdateStrategy = ShadowMapFrustum::ShadowCacheData::eFullUpdate);
	}
}

i32 C3DEngine::SaveStatObj(IStatObj* pStatObj, TSerialize ser)
{
	if (!(pStatObj->GetFlags() & STATIC_OBJECT_GENERATED))
	{
		bool bVal = false;
		ser.Value("altered", bVal);
		ser.Value("file", pStatObj->GetFilePath());
		ser.Value("geom", pStatObj->GetGeoName());
	}
	else
	{
		bool bVal = true;
		ser.Value("altered", bVal);
		ser.Value("CloneSource", pStatObj->GetCloneSourceObject() ? pStatObj->GetCloneSourceObject()->GetFilePath() : "0");
		pStatObj->Serialize(ser);
	}

	return 1;
}

IStatObj* C3DEngine::LoadStatObj(TSerialize ser)
{
	bool bVal;
	IStatObj* pStatObj;
	ser.Value("altered", bVal);
	if (!bVal)
	{
		string fileName, geomName;
		ser.Value("file", fileName);
		ser.Value("geom", geomName);
		pStatObj = LoadStatObj(fileName, geomName);
	}
	else
	{
		string srcObjName;
		ser.Value("CloneSource", srcObjName);
		if (*(const unsigned short*)(tukk)srcObjName != '0')
			pStatObj = LoadStatObj(srcObjName)->Clone(false, false, true);
		else
			pStatObj = CreateStatObj();
		pStatObj->Serialize(ser);
	}
	return pStatObj;
}
