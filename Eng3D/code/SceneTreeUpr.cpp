// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   3dengine.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Реализвция методов интерфейса I3DEngine.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>

#if defined(FEATURE_SVO_GI)

	#include <drx3D/Eng3D/VoxelSegment.h>
	#include <drx3D/Eng3D/SceneTree.h>
	#include <drx3D/Eng3D/SceneTreeUpr.h>

extern CSvoEnv* gSvoEnv;

void CSvoUpr::CheckAllocateGlobalCloud()
{
	LOADING_TIME_PROFILE_SECTION;
	if (!gSvoEnv && DinrusX3dEngBase::GetCVars()->e_svoEnabled)
	{
		float mapSize = (float)DinrusX3dEngBase::Get3DEngine()->GetTerrainSize();
		AABB areaBox(Vec3(0, 0, 0), Vec3(mapSize, mapSize, mapSize));
		gSvoEnv = new CSvoEnv(areaBox);
	}
}

tuk CSvoUpr::GetStatusString(i32 lineId)
{
	static char szText[256] = "";

	i32 slotId = 0;

	if (lineId == (slotId++) && (CVoxelSegment::m_addPolygonToSceneCounter || GetCVars()->e_svoEnabled))
	{
		i32 allSlotsNum = SVO_ATLAS_DIM_BRICKS_XY * SVO_ATLAS_DIM_BRICKS_XY * SVO_ATLAS_DIM_BRICKS_Z;
		drx_sprintf(szText, "SVO pool: %2d of %dMB x %d, %3d/%3d of %4d = %.1f Async: %2d, Post: %2d, Loaded: %4d",
		            CVoxelSegment::m_poolUsageBytes / 1024 / 1024,
		            i32(CVoxelSegment::m_voxTexPoolDimXY * CVoxelSegment::m_voxTexPoolDimXY * CVoxelSegment::m_voxTexPoolDimZ / 1024 / 1024) * (gSvoEnv->m_voxTexFormat == eTF_BC3 ? 1 : 4),
		            CVoxelSegment::m_svoDataPoolsCounter,
		            CVoxelSegment::m_addPolygonToSceneCounter, CVoxelSegment::m_poolUsageItems, allSlotsNum,
		            (float)CVoxelSegment::m_poolUsageItems / allSlotsNum,
		            CVoxelSegment::m_streamingTasksInProgress, CVoxelSegment::m_postponedCounter, CVoxelSegment::m_arrLoadedSegments.Count());
		return szText;
	}

	static ICVar* pDisplayInfo = GetConsole()->GetCVar("r_DisplayInfo");
	if (pDisplayInfo->GetIVal() < 2)
	{
		return nullptr;
	}

	if (lineId == (slotId++))
	{
		static i32 s_texUpdatesInProgressLast = 0;
		s_texUpdatesInProgressLast = max(s_texUpdatesInProgressLast, CVoxelSegment::m_updatesInProgressTex);

		static i32 s_briUpdatesInProgressLast = 0;
		s_briUpdatesInProgressLast = max(s_briUpdatesInProgressLast, CVoxelSegment::m_updatesInProgressBri);

		drx_sprintf(szText, "Brick updates: %2d / %3d, bs: %d  GPU nodes: %2d / %2d",
		            s_briUpdatesInProgressLast, s_texUpdatesInProgressLast, SVO_VOX_BRICK_MAX_SIZE,
		            gSvoEnv->m_dynNodeCounter, gSvoEnv->m_dynNodeCounter_DYNL);

		float curTime = DinrusX3dEngBase::Get3DEngine()->GetCurTimeSec();
		static float s_lastTime = DinrusX3dEngBase::Get3DEngine()->GetCurTimeSec();
		if (curTime > s_lastTime + 0.1f)
		{
			if (s_texUpdatesInProgressLast)
				s_texUpdatesInProgressLast -= 4;

			if (s_briUpdatesInProgressLast)
				s_briUpdatesInProgressLast--;

			s_lastTime = curTime;
		}

		return szText;
	}

	if (lineId == (slotId++))
	{
		drx_sprintf(szText, "cpu bricks pool: el = %d of %d, %d MB",
		            gSvoEnv->m_brickSubSetAllocator.GetCount(),
		            gSvoEnv->m_brickSubSetAllocator.GetCapacity(),
		            gSvoEnv->m_brickSubSetAllocator.GetCapacityBytes() / 1024 / 1024);
		return szText;
	}

	if (lineId == (slotId++))
	{
		drx_sprintf(szText, "cpu node pool: el = %d of %d, %d KB",
		            gSvoEnv->m_nodeAllocator.GetCount(),
		            gSvoEnv->m_nodeAllocator.GetCapacity(),
		            gSvoEnv->m_nodeAllocator.GetCapacityBytes() / 1024);
		return szText;
	}

	if (lineId == (slotId++))
	{
		drx_sprintf(szText, "bouncing lights: dynamic: %d, static: %d", gSvoEnv->m_lightsTI_D.Count(), gSvoEnv->m_lightsTI_S.Count());
		return szText;
	}

	if (gSvoEnv->m_pSvoRoot && lineId == (slotId++))
	{
		i32 trisCount = 0, vertCount = 0, trisBytes = 0, vertBytes = 0, maxVertPerArea = 0, matsCount = 0;
		gSvoEnv->m_pSvoRoot->GetTrisInAreaStats(trisCount, vertCount, trisBytes, vertBytes, maxVertPerArea, matsCount);

		i32 voxSegAllocatedBytes = 0;
		gSvoEnv->m_pSvoRoot->GetVoxSegMemUsage(voxSegAllocatedBytes);

		drx_sprintf(szText, "VoxSeg: %d MB, Tris/Verts %d / %d K, %d / %d MB, avmax %d K, Mats %d",
		            voxSegAllocatedBytes / 1024 / 1024,
		            trisCount / 1000, vertCount / 1000, trisBytes / 1024 / 1024, vertBytes / 1024 / 1024, maxVertPerArea / 1000, matsCount);
		return szText;
	}

	#ifdef FEATURE_SVO_GI_USE_MESH_RT
	if (lineId == (slotId++) && gSvoEnv->m_arrRTPoolInds.Count())
	{
		drx_sprintf(szText, "RT pools: tex %.2f, verts %.2f, inds %.2f",
		            (float)gSvoEnv->m_arrRTPoolTexs.Count() / max((float)gSvoEnv->m_arrRTPoolTexs.capacity(), 1.f),
		            (float)gSvoEnv->m_arrRTPoolTris.Count() / max((float)gSvoEnv->m_arrRTPoolTris.capacity(), 1.f),
		            (float)gSvoEnv->m_arrRTPoolInds.Count() / max((float)gSvoEnv->m_arrRTPoolInds.capacity(), 1.f));
		return szText;
	}
	#endif

	return nullptr;
}

void CSvoUpr::Update(const SRenderingPassInfo& passInfo, CCamera& newCam)
{
	if (/*Get3DEngine()->GetRenderFramesSinceLevelStart()>30 && */ passInfo.IsGeneralPass())
	{
		CSvoUpr::UpdateSubSystems(passInfo.GetCamera(), newCam);
	}
}

void CSvoUpr::UpdateSubSystems(const CCamera& _newCam, CCamera& newCam)
{

}

void CSvoUpr::OnFrameStart(const SRenderingPassInfo& passInfo)
{
	CVoxelSegment::m_currPassMainFrameID = passInfo.GetMainFrameID();
	//	if(GetCVars()->e_rsMode != RS_RENDERFARM)
	CVoxelSegment::SetVoxCamera(passInfo.GetCamera());

	CVoxelSegment::m_pCurrPassInfo = (SRenderingPassInfo*)&passInfo;
}

void CSvoUpr::OnFrameComplete()
{
	//	if(gSSRSystem)
	//	gSSRSystem->OnFrameComplete();
	CVoxelSegment::m_pCurrPassInfo = 0;
}

void CSvoUpr::Release()
{
	SAFE_DELETE(gSvoEnv);
}

void CSvoUpr::Render(bool bSyncUpdate)
{
	LOADING_TIME_PROFILE_SECTION;
	if (GetCVars()->e_svoTI_Apply && (!m_bLevelLoadingInProgress || gEnv->IsEditor()) && !GetCVars()->e_svoTI_Active)
	{
		GetCVars()->e_svoTI_Active = 1;
		GetCVars()->e_svoLoadTree = 1;
		GetCVars()->e_svoRender = 1;

		if (GetCVars()->e_svoTI_Troposphere_Active)
		{
			GetCVars()->e_Clouds = 0;
			if (ICVar* pV = gEnv->pConsole->GetCVar("r_UseAlphaBlend"))
				pV->Set(0);
		}
	}

	if (GetCVars()->e_svoLoadTree)
	{
		LOADING_TIME_PROFILE_SECTION_NAMED("SVO Load Tree");
		SAFE_DELETE(gSvoEnv);

		GetCVars()->e_svoEnabled = 1;

		float mapSize = (float)DinrusX3dEngBase::Get3DEngine()->GetTerrainSize();
		AABB areaBox(Vec3(0, 0, 0), Vec3(mapSize, mapSize, mapSize));
		gSvoEnv = new CSvoEnv(areaBox);
		gSvoEnv->ReconstructTree(0 /*m_pGamePlayArea && (m_pGamePlayArea->GetPoints().Count()>0)*/);

		GetCVars()->e_svoLoadTree = 0;
	}

	if (GetCVars()->e_svoEnabled && GetCVars()->e_svoRender)
	{
		LOADING_TIME_PROFILE_SECTION_NAMED("SVO Render");

		CheckAllocateGlobalCloud();

		if (gSvoEnv && !CVoxelSegment::m_bExportMode)
		{
			if (bSyncUpdate)
				gSvoEnv->m_svoFreezeTime = gEnv->pTimer->GetAsyncCurTime();

			if (gSvoEnv->m_bFirst_SvoFreezeTime)
				gSvoEnv->m_svoFreezeTime = gEnv->pTimer->GetAsyncCurTime();
			gSvoEnv->m_bFirst_SvoFreezeTime = false;

			if (gSvoEnv->m_svoFreezeTime > 0)
			{
				// perform synchronous SVO update, usually happens in first frames after level loading
				while (gSvoEnv->m_svoFreezeTime > 0)
				{
					gSvoEnv->Render();
					gEnv->pSystem->GetStreamEngine()->Update();

					if ((gEnv->pTimer->GetAsyncCurTime() - gSvoEnv->m_svoFreezeTime) > GetCVars()->e_svoTI_MaxSyncUpdateTime)
					{
						// prevent possible freeze in case of SVO pool overflow
						break;
					}

					DrxSleep(5);
				}

				gSvoEnv->m_svoFreezeTime = -1;
			}
			else
			{
				gSvoEnv->Render();
			}
		}
	}
}

void CSvoUpr::OnDisplayInfo(float& textPosX, float& textPosY, float& textStepY, float textScale)
{
	if (GetCVars()->e_svoEnabled)
	{
		i32 lineId = 0;
		while (tuk szStatus = CSvoUpr::GetStatusString(lineId))
		{
			Get3DEngine()->DrawTextRightAligned(textPosX, textPosY += textStepY, textScale, ColorF(0.5f, 1.0f, 1.0f), szStatus);
			lineId++;
		}
	}
}

bool CSvoUpr::GetSvoStaticTextures(I3DEngine::SSvoStaticTexInfo& svoInfo, PodArray<I3DEngine::SLightTI>* pLightsTI_S, PodArray<I3DEngine::SLightTI>* pLightsTI_D)
{
	return gSvoEnv ? gSvoEnv->GetSvoStaticTextures(svoInfo, pLightsTI_S, pLightsTI_D) : false;
}

void CSvoUpr::GetSvoBricksForUpdate(PodArray<I3DEngine::SSvoNodeInfo>& arrNodeInfo, float nodeSize, PodArray<SVF_P3F_C4B_T2F>* pVertsOut)
{
	if (gSvoEnv)
		gSvoEnv->GetSvoBricksForUpdate(arrNodeInfo, nodeSize, pVertsOut);
}

i32 CSvoUpr::ExportSvo(IDrxArchive* pArchive)
{
	if (GetCVars()->e_StreamCgf)
	{
		assert(!"e_StreamCgf must be 0 for successful SVO export");
		PrintMessage("SVO export error: e_StreamCgf must be 0 for successful SVO export");
		return 0;
	}

	CVoxelSegment::m_bExportMode = true;
	CVoxelSegment::m_exportVisitedAreasCounter = 0;
	CVoxelSegment::m_exportVisitedNodesCounter = 0;
	CVoxelSegment::m_bExportAbortRequested = false;

	if (!gSvoEnv)
	{
		CSvoUpr::Render(false);
	}

	i32 result = 0;

	if (gSvoEnv)
	{
		result = gSvoEnv->ExportSvo(pArchive);
	}

	CVoxelSegment::m_bExportMode = false;

	return result;
}

void CSvoUpr::RegisterMovement(const AABB& objBox)
{
	if (gSvoEnv && gSvoEnv->m_pSvoRoot)
		gSvoEnv->m_pSvoRoot->RegisterMovement(objBox);
}

#endif
