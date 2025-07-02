// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//	File: SystemRender.cpp
//  Описание: DrxENGINE system core
//
//	История:
//	-Jan 6,2004: split from system.cpp
//
//////////////////////////////////////////////////////////////////////

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/System.h>
#include <drx3D/Entity/IEntity.h>
#include <drx3D/Sys/IStatoscope.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/CoreX/Renderer/IScaleform.h>
#include <drx3D/Sys/IProcess.h>
#include <drx3D/Sys/Log.h>
#include <drx3D/Sys/XConsole.h>
#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/AI/IAISystem.h>
#include <drx3D/CoreX/Platform/DrxLibrary.h>
#include <drx3D/Sys/IBudgetingSystem.h>
#include <drx3D/Sys/PhysRenderer.h>
#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/CoreX/ParticleSys/IParticles.h>
#include <drx3D/Movie/IMovieSystem.h>
#include <drx3D/CoreX/Thread/IJobUpr.h>
#include <drx3D/CoreX/Platform/IPlatformOS.h>

#include <drx3D/Sys/DrxSizerStats.h>
#include <drx3D/Sys/DrxSizerImpl.h>
#include <drx3D/Sys/ITestSystem.h>   // ITestSystem
#include <drx3D/Sys/VisRegTest.h>
#include <drx3D/Sys/ThreadProfiler.h>
#include <drx3D/Sys/IDiskProfiler.h>
#include <drx3D/Sys/ITextModeConsole.h>
#include <drx3D/Sys/HardwareMouse.h>
#include <drx3D/Entity/IEntitySystem.h> // <> required for Interfuscator
#include <drx3D/Sys/NULLRenderAuxGeom.h>

#include <drx3D/Sys/MiniGUI.h>
#include <drx3D/Sys/PerfHUD.h>
#include <drx3D/Sys/ThreadInfo.h>

#include <drx3D/Sys/IScaleformHelper.h>
#include<drx3D/Sys/IHMDUpr.h>

extern CMTSafeHeap* g_pPakHeap;

extern i32 DrxMemoryGetAllocatedSize();

// Remap r_fullscreen to keep legacy functionality, r_WindowType is new desired path
void OnFullscreenStateChanged(ICVar* pFullscreenCVar)
{
	if (ICVar* pCVar = gEnv->pConsole->GetCVar("r_WindowType"))
	{
		if (pFullscreenCVar->GetIVal() != 0)
		{
			pCVar->Set(3);
		}
		else if(pCVar->GetIVal() == 3)
		{
			pCVar->Set(0);
		}
	}
}

// Reflect r_windowType changes to the legacy r_fullscreen CVar
void OnWindowStateChanged(ICVar* pCVar)
{
	if (ICVar* pFullscreenCVar = gEnv->pConsole->GetCVar("r_Fullscreen"))
	{
		pFullscreenCVar->Set(pCVar->GetIVal() == 3 ? 1 : 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////
void CSystem::CreateRendererVars(const SSysInitParams& startupParams)
{
	i32 iFullScreenDefault  = 1;
	i32 iDisplayInfoDefault = 1;
	i32 iWidthDefault       = 1280;
	i32 iHeightDefault      = 720;
#if DRX_PLATFORM_DURANGO
	iWidthDefault  = 1600;
	iHeightDefault = 900;
#elif DRX_PLATFORM_ORBIS
	iWidthDefault  = 1920;
	iHeightDefault = 1080;
#elif DRX_PLATFORM_WINDOWS
	iFullScreenDefault = 0;
	iWidthDefault      = GetSystemMetrics(SM_CXFULLSCREEN) * 2 / 3;
	iHeightDefault     = GetSystemMetrics(SM_CYFULLSCREEN) * 2 / 3;
#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_APPLE
	iFullScreenDefault = 0;
#endif

#if defined(RELEASE)
	iDisplayInfoDefault = 0;
#endif

	// load renderer settings from engine.ini
	m_rWidth = REGISTER_INT("r_Width", iWidthDefault, VF_DUMPTODISK,
		"Sets the display width, in pixels.\n"
		"Usage: r_Width [800/1024/..]");
	m_rHeight = REGISTER_INT("r_Height", iHeightDefault, VF_DUMPTODISK,
		"Sets the display height, in pixels.\n"
		"Usage: r_Height [600/768/..]");
	m_rColorBits = REGISTER_INT("r_ColorBits", 32, VF_DUMPTODISK,
		"Sets the color resolution, in bits per pixel. Default is 32.\n"
		"Usage: r_ColorBits [32/24/16/8]");
	m_rDepthBits = REGISTER_INT("r_DepthBits", 24, VF_DUMPTODISK | VF_REQUIRE_APP_RESTART,
		"Sets the depth precision, in bits per pixel. Default is 24.\n"
		"Usage: r_DepthBits [32/24/16]");
	m_rStencilBits = REGISTER_INT("r_StencilBits", 8, VF_DUMPTODISK,
		"Sets the stencil precision, in bits per pixel. Default is 8.\n");

#if DRX_PLATFORM_WINDOWS
	REGISTER_INT("r_overrideDXGIAdapter", -1, VF_REQUIRE_APP_RESTART,
	  "Specifies index of the preferred video adapter to be used for rendering (-1=off, loops until first suitable adapter is found).\n"
	  "Use this to resolve which video card to use if more than one DX11 capable GPU is available in the system.");
#endif

#if DRX_PLATFORM_ANDROID
	tukk p_r_DriverDef = STR_VK_RENDERER;
#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_APPLE
		tukk p_r_DriverDef = STR_GL_RENDERER;
#elif DRX_PLATFORM_DURANGO
	tukk p_r_DriverDef = STR_DX11_RENDERER;
#elif DRX_PLATFORM_ORBIS
	tukk p_r_DriverDef = STR_DX11_RENDERER;
#else
	tukk p_r_DriverDef = STR_AUTO_RENDERER;
#endif

	if (startupParams.pCvarsDefault)
	{
		// hack to customize the default value of r_Driver
		SCvarsDefault* pCvarsDefault = startupParams.pCvarsDefault;
		if (pCvarsDefault->sz_r_DriverDef && pCvarsDefault->sz_r_DriverDef[0])
			p_r_DriverDef = startupParams.pCvarsDefault->sz_r_DriverDef;
	}

	m_rDriver = REGISTER_STRING("r_Driver", p_r_DriverDef, VF_DUMPTODISK,
		"Sets the renderer driver ( DX11/DX12/GL/VK/AUTO ).\n"
		"Specify in system.cfg like this: r_Driver = \"DX11\"");

	m_rFullscreen = REGISTER_INT_CB("r_Fullscreen", iFullScreenDefault, VF_DUMPTODISK,
		"Toggles fullscreen mode. Default is 1 in normal game and 0 in DevMode.\n"
		"Usage: r_Fullscreen [0=window/1=fullscreen]", OnFullscreenStateChanged);

	m_rWindowState = REGISTER_INT_CB("r_WindowType", m_rFullscreen->GetIVal() != 0 ? 3 : 0, VF_DUMPTODISK,
		"Changes the type of window for the rendered viewport.\n"
		"Usage: r_WindowType [0=normal window/1=borderless window/2=borderless full screen/3=exclusive full screen]", OnWindowStateChanged);

	m_rFullsceenNativeRes = REGISTER_INT("r_FullscreenNativeRes", 0, VF_DUMPTODISK,
		"Toggles native resolution upscaling.\n"
		"If enabled, scene gets upscaled from specified resolution while UI is rendered in native resolution.");

	m_rDisplayInfo = REGISTER_INT("r_DisplayInfo", iDisplayInfoDefault, VF_RESTRICTEDMODE | VF_DUMPTODISK,
		"Toggles debugging information display.\n"
		"Usage: r_DisplayInfo [0=off/1=show/2=enhanced/3=minimal/4=fps bar/5=heartbeat]");

	m_rDisplayInfoTargetFPS = REGISTER_FLOAT("r_displayinfoTargetFPS", 30.0f, VF_RESTRICTEDMODE | VF_DUMPTODISK,
		"Specifies the aimed number of FPS that is considered ideal for the game.\n"
		"The value must be positive and is used to display budgeting information with r_DisplayInfo=3");

	m_rOverscanBordersDrawDebugView = REGISTER_INT("r_OverscanBordersDrawDebugView", 0, VF_RESTRICTEDMODE | VF_DUMPTODISK,
		"Toggles drawing overscan borders.\n"
		"Usage: r_OverscanBordersDrawDebugView [0=off/1=show]");
}

//////////////////////////////////////////////////////////////////////////
void CSystem::RenderBegin(const SDisplayContextKey& displayContextKey)
{
	DRX_PROFILE_FUNCTION(PROFILE_SYSTEM);

	DRX_PROFILE_MARKER("CSystem::RenderBegin");

	if (m_bIgnoreUpdates)
		return;

	bool rndAvail = m_env.pRenderer != 0;

	//////////////////////////////////////////////////////////////////////
	//start the rendering pipeline
	if (rndAvail)
	{
		m_env.pRenderer->BeginFrame(displayContextKey);
		gEnv->nMainFrameID = m_env.pRenderer->GetFrameID(false);
	}
	else
	{
		if (m_pNULLRenderAuxGeom)
		{
			m_pNULLRenderAuxGeom->BeginFrame();
		}
		++gEnv->nMainFrameID;
	}

#if defined(INCLUDE_SCALEFORM_SDK) || defined(DRX_FEATURE_SCALEFORM_HELPER)
	if (rndAvail && !gEnv->IsDedicated() && gEnv->pScaleformHelper)
	{
		// render thread IDs might get updated in BeginFrame() so update GFx render layer
		threadID mainThread(0), renderThread(0);
		m_env.pRenderer->GetThreadIDs(mainThread, renderThread);
		gEnv->pScaleformHelper->SetRenderThreadIDs(mainThread, renderThread);
	}
#endif
}

tuk PhysHelpersToStr(i32 iHelpers, tuk strHelpers);
i32   StrToPhysHelpers(tukk strHelpers);

//////////////////////////////////////////////////////////////////////////
void CSystem::RenderEnd(bool bRenderStats)
{
	{
		DRX_PROFILE_FUNCTION(PROFILE_SYSTEM);
		DRX_PROFILE_MARKER("CSystem::RenderEnd");

		if (m_bIgnoreUpdates)
			return;

		if (!m_env.pRenderer)
		{
			if (bRenderStats)
			{
				RenderStatistics();
			}
			if (m_pNULLRenderAuxGeom)
			{
#if !defined (_RELEASE)
				if (m_pPhysRenderer)
				{
					RenderPhysicsHelpers();
				}
#endif
				m_pNULLRenderAuxGeom->EndFrame();
			}
			return;
		}
		/*
		    if(m_env.pMovieSystem)
		      m_env.pMovieSystem->Render();
		 */

		GetPlatformOS()->RenderEnd();

#if !defined (_RELEASE)
		// Flush render data and swap buffers.
		m_env.pRenderer->RenderDebug(bRenderStats);
#endif

		RenderJobStats();

#if defined(USE_PERFHUD)
		if (m_pPerfHUD)
			m_pPerfHUD->Draw();
		if (m_pMiniGUI)
			m_pMiniGUI->Draw();
#endif

		if (bRenderStats)
		{
			RenderStatistics();
		}

		if (IConsole* pConsole = GetIConsole())
			pConsole->Draw();

		m_env.pRenderer->ForceGC(); // XXX Rename this
		m_env.pRenderer->EndFrame();

		if (IConsole* pConsole = GetIConsole())
		{
			// if we have pending cvar calculation, execute it here
			// since we know cvars will be correct here after ->EndFrame().
			if (!pConsole->IsHashCalculated())
				pConsole->CalcCheatVarHash();
		}

	}
}

//////////////////////////////////////////////////////////////////////////
void CSystem::RenderPhysicsHelpers()
{
#if !defined (_RELEASE)
	if (m_bIgnoreUpdates)
		return;

	if (gEnv->pPhysicalWorld && m_pPhysRenderer)
	{
		char str[128];
		if (StrToPhysHelpers(m_p_draw_helpers_str->GetString()) != m_env.pPhysicalWorld->GetPhysVars()->iDrawHelpers)
			m_p_draw_helpers_str->Set(PhysHelpersToStr(m_env.pPhysicalWorld->GetPhysVars()->iDrawHelpers, str));

		m_pPhysRenderer->UpdateCamera(GetViewCamera());
		m_pPhysRenderer->DrawAllHelpers(m_env.pPhysicalWorld);
		m_pPhysRenderer->Flush(m_Time.GetFrameTime());

		RenderPhysicsStatistics(gEnv->pPhysicalWorld);
	}

	if (m_env.pAuxGeomRenderer)
		m_env.pAuxGeomRenderer->Submit();
#endif
}

//////////////////////////////////////////////////////////////////////////
void CSystem::RenderPhysicsStatistics(IPhysicalWorld* pWorld)
{
#if !defined(_RELEASE)
	//////////////////////////////////////////////////////////////////////
	// draw physics helpers
	if (pWorld && GetIRenderer())
	{
		const float renderMarginX = 10.0f;
		const float renderMarginY = 10.0f;
		const float lineSize      = 12.0f;
		const float fontSize      = 1.3f;
		ITextModeConsole*  pTMC   = GetITextModeConsole();
		phys_profile_info* pInfos;
		phys_job_info* pJobInfos;
		PhysicsVars* pVars = pWorld->GetPhysVars();
		i32  i             = -2;
		char msgbuf[512];

		if (pVars->bProfileEntities == 1)
		{
			pe_status_pos sp;
			float fColor[4]  = { 0.3f, 0.6f, 1.0f, 1.0f }, dt;
			i32 nMaxEntities = (GetIRenderer()->GetOverlayHeight() / (i32)lineSize) - 2;
			if (pVars->bProfileGroups)
			{
				nMaxEntities -= pWorld->GetGroupProfileInfo(pInfos) + 2;     // + extra lines to account for spacing between groups
			}
			if (pVars->bProfileFunx)
			{
				nMaxEntities -= pWorld->GetFuncProfileInfo(pInfos) + 1;
			}
			if (nMaxEntities < 0)
			{
				nMaxEntities = 0;
			}
			i32 j, mask, nEnts = pWorld->GetEntityProfileInfo(pInfos);
			if (nEnts > nMaxEntities)
			{
				nEnts = nMaxEntities;
			}
			for(; nEnts > 0 && pInfos[nEnts - 1].nCallsLast + pInfos[nEnts - 1].nCallsAvg < 0.2f; nEnts--)
				;

			if (!pVars->bSingleStepMode)
			{
				for (i = 0; i < nEnts; i++)
				{
					pInfos[i].nTicksAvg = (i32)(((int64)pInfos[i].nTicksAvg * 15 + pInfos[i].nTicksLast) >> 4);
					pInfos[i].nCallsAvg = pInfos[i].nCallsAvg * (15.0f / 16) + pInfos[i].nCallsLast * (1.0f / 16);
				}
				phys_profile_info ppi;
				for (i = 0; i < nEnts - 1; i++)
					for (j = nEnts - 1; j > i; j--)
						if (pInfos[j - 1].nTicksAvg < pInfos[j].nTicksAvg)
						{
							ppi           = pInfos[j - 1];
							pInfos[j - 1] = pInfos[j];
							pInfos[j]     = ppi;
						}
					}
			for (i = 0; i < nEnts; i++)
			{
				mask                  = (pInfos[i].nTicksPeak - pInfos[i].nTicksLast) >> 31;
				mask                 |= (70 - pInfos[i].peakAge) >> 31;
				mask                 &= (pVars->bSingleStepMode - 1) >> 31;
				pInfos[i].nTicksPeak += pInfos[i].nTicksLast - pInfos[i].nTicksPeak & mask;
				pInfos[i].nCallsPeak += pInfos[i].nCallsLast - pInfos[i].nCallsPeak & mask;
				drx_sprintf(msgbuf, "%.2fms/%.1f (peak %.2fms/%d) %s (id %d)",
				  dt = gEnv->pTimer->TicksToSeconds(pInfos[i].nTicksAvg) * 1000.0f, pInfos[i].nCallsAvg,
				  gEnv->pTimer->TicksToSeconds(pInfos[i].nTicksPeak) * 1000.0f, pInfos[i].nCallsPeak,
				  pInfos[i].pName ? pInfos[i].pName : "", pInfos[i].id);
				IRenderAuxText::Draw2dLabel(renderMarginX, renderMarginY + i * lineSize, fontSize, fColor, false, "%s", msgbuf);
				if (pTMC) pTMC->PutText(0, i, msgbuf);
				IPhysicalEntity* pent = pWorld->GetPhysicalEntityById(pInfos[i].id);
				if (dt > 0.1f && pInfos[i].pName && pent && pent->GetStatus(&sp))
					IRenderAuxText::DrawLabelExF(sp.pos + Vec3(0, 0, sp.BBox[1].z), 1.4f, fColor, true, true, "%s %.2fms", pInfos[i].pName, dt);
				pInfos[i].peakAge = pInfos[i].peakAge + 1 & ~mask;
				//pInfos[i].nCalls=pInfos[i].nTicks = 0;
			}

			if (inrange(m_iJumpToPhysProfileEnt, 0, nEnts))
			{
				ScriptHandle hdl;
				hdl.n = ~0;
				m_env.pScriptSystem->GetGlobalValue("g_localActorId", hdl);
				IEntity* pPlayerEnt   = m_env.pEntitySystem->GetEntity((EntityId)hdl.n);
				IPhysicalEntity* pent = pWorld->GetPhysicalEntityById(pInfos[m_iJumpToPhysProfileEnt - 1].id);
				if (pPlayerEnt && pent)
				{
					pe_params_bbox pbb;
					pent->GetParams(&pbb);
					pPlayerEnt->SetPos((pbb.BBox[0] + pbb.BBox[1]) * 0.5f + Vec3(0, -3.0f, 1.5f));
					pPlayerEnt->SetRotation(Quat(IDENTITY));
				}
				m_iJumpToPhysProfileEnt = 0;
			}
		}
		if (pVars->bProfileFunx)
		{
			i32 j, mask, nFunx = pWorld->GetFuncProfileInfo(pInfos);
			float fColor[4] = { 0.75f, 0.08f, 0.85f, 1.0f };
			for (j = 0, ++i; j < nFunx; j++, i++)
			{
				mask                  = (pInfos[j].nTicksPeak - pInfos[j].nTicks) >> 31;
				mask                 |= (70 - pInfos[j].peakAge) >> 31;
				pInfos[j].nTicksPeak += pInfos[j].nTicks - pInfos[j].nTicksPeak & mask;
				pInfos[j].nCallsPeak += pInfos[j].nCalls - pInfos[j].nCallsPeak & mask;
				IRenderAuxText::Draw2dLabel(renderMarginX, renderMarginY + i * lineSize, fontSize, fColor, false,
				  "%s %.2fms/%d (peak %.2fms/%d)", pInfos[j].pName, gEnv->pTimer->TicksToSeconds(pInfos[j].nTicks) * 1000.0f, pInfos[j].nCalls,
				  gEnv->pTimer->TicksToSeconds(pInfos[j].nTicksPeak) * 1000.0f, pInfos[j].nCallsPeak);
				pInfos[j].peakAge = pInfos[j].peakAge + 1 & ~mask;
				pInfos[j].nCalls  = pInfos[j].nTicks = 0;
			}
		}
		if (pVars->bProfileGroups)
		{
			i32 j           = 0, mask, nGroups = pWorld->GetGroupProfileInfo(pInfos), nJobs = pWorld->GetJobProfileInfo(pJobInfos);
			float fColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			if (!pVars->bProfileEntities)
				j = 12;

			for (++i; j < nGroups; j++, i++)
			{
				pInfos[j].nTicksAvg   = (i32)(((int64)pInfos[j].nTicksAvg * 15 + pInfos[j].nTicksLast) >> 4);
				mask                  = (pInfos[j].nTicksPeak - pInfos[j].nTicksLast) >> 31;
				mask                 |= (70 - pInfos[j].peakAge) >> 31;
				pInfos[j].nTicksPeak += pInfos[j].nTicksLast - pInfos[j].nTicksPeak & mask;
				pInfos[j].nCallsPeak += pInfos[j].nCallsLast - pInfos[j].nCallsPeak & mask;
				float time     = gEnv->pTimer->TicksToSeconds(pInfos[j].nTicksAvg) * 1000.0f;
				float timeNorm = time * (1.0f / 32);
				fColor[1] = fColor[2] = 1.0f - (max(0.7f, min(1.0f, timeNorm)) - 0.7f) * (1.0f / 0.3f);
				IRenderAuxText::Draw2dLabel(renderMarginX, renderMarginY + i * lineSize, fontSize, fColor, false,
				  "%s %.2fms/%d (peak %.2fms/%d)", pInfos[j].pName, time, pInfos[j].nCallsLast,
				  gEnv->pTimer->TicksToSeconds(pInfos[j].nTicksPeak) * 1000.0f, pInfos[j].nCallsPeak);
				pInfos[j].peakAge = pInfos[j].peakAge + 1 & ~mask;
				if (j == nGroups - 3) ++i;
			}
		}
		if (pVars->bProfileEntities == 2)
		{
			i32 nEnts = m_env.pPhysicalWorld->GetEntityProfileInfo(pInfos);

			for (i32 j = 0; j < nEnts; j++)
			{
				gEnv->pStatoscope->AddPhysEntity(&pInfos[j]);
			}
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
void CSystem::RenderJobStats()
{
	//enable job system filtering to disable async execution at runtime
	tukk const cpFilterName = m_sys_job_system_filter->GetString();
	if (cpFilterName && *cpFilterName != 0 && *cpFilterName != '0')
	{
		gEnv->GetJobUpr()->SetJobFilter(cpFilterName);
	}
	else
	{
		gEnv->GetJobUpr()->SetJobFilter(NULL);
	}

	gEnv->GetJobUpr()->Update(m_sys_job_system_profiler->GetIVal());
	gEnv->GetJobUpr()->SetJobSystemEnabled(m_sys_job_system_enable->GetIVal());

	JobUpr::IBackend* const __restrict pThreadBackEnd   = gEnv->GetJobUpr()->GetBackEnd(JobUpr::eBET_Thread);
	JobUpr::IBackend* const __restrict pBlockingBackEnd = gEnv->GetJobUpr()->GetBackEnd(JobUpr::eBET_Blocking);

#if defined(ENABLE_PROFILING_CODE)
	if (m_FrameProfileSystem.IsEnabled())
	{
#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)

		// Get none-blocking job & worker profile stats
		if (pThreadBackEnd)
		{
			JobUpr::IWorkerBackEndProfiler* pWorkerProfiler = pThreadBackEnd->GetBackEndWorkerProfiler();

			if (pWorkerProfiler)
			{
				m_FrameProfileSystem.ValThreadFrameStatsCapacity(pWorkerProfiler->GetNumWorkers());
				pWorkerProfiler->GetFrameStats(*m_FrameProfileSystem.m_ThreadFrameStats, m_FrameProfileSystem.m_ThreadJobFrameStats, JobUpr::IWorkerBackEndProfiler::eJobSortOrder_Lexical);
			}
		}

		// Get blocking job & worker profile stats
		if (pBlockingBackEnd)
		{
			JobUpr::IWorkerBackEndProfiler* pWorkerProfiler = pBlockingBackEnd->GetBackEndWorkerProfiler();

			if (pWorkerProfiler)
			{
				m_FrameProfileSystem.ValBlockingFrameStatsCapacity(pWorkerProfiler->GetNumWorkers());
				pWorkerProfiler->GetFrameStats(*m_FrameProfileSystem.m_BlockingFrameStats, m_FrameProfileSystem.m_BlockingJobFrameStats, JobUpr::IWorkerBackEndProfiler::eJobSortOrder_Lexical);
			}
		}
#endif
	}
#endif
}

//! Update screen and call some important tick functions during loading.
void CSystem::SynchronousLoadingTick(tukk pFunc, i32 line)
{
	LOADING_TIME_PROFILE_SECTION;
	if (gEnv && gEnv->bMultiplayer && !gEnv->IsEditor())
	{
		//UpdateLoadingScreen currently contains a couple of tick functions that need to be called regularly during the synchronous level loading,
		//when the usual engine and game ticks are suspended.
		UpdateLoadingScreen();

#if defined(MAP_LOADING_SLICING)
		GetISystemScheduler()->SliceAndSleep(pFunc, line);
#endif
	}
}

//////////////////////////////////////////////////////////////////////////
#define CHECK_UPDATE_TIMES
void CSystem::UpdateLoadingScreen()
{
	// Do not update the network thread from here - it will cause context corruption - use the NetworkStallTicker thread system

	if (GetCurrentThreadId() != gEnv->mMainThreadId)
	{
		return;
	}

	// Take this opportunity to update streaming engine.
	static CTimeValue t0;
	CTimeValue t    = gEnv->pTimer->GetAsyncTime();
	float timeDelta = fabs((t - t0).GetSeconds());

#if DRX_PLATFORM_DURANGO
	//XBOX ONE TCR - XR-004-01: 'The title should return from its suspending handler within one second'
	static float s_updateFrequencyInSeconds = 0.5f;
#else
	static float s_updateFrequencyInSeconds = 1.0f;   // Update screen not frequent,not more than once in 1 second
#endif

	if (m_env.pConsole)
	{
		static ICVar* con_showonload = m_env.pConsole->GetCVar("con_showonload");
		if (con_showonload && con_showonload->GetIVal() != 0)
		{
			s_updateFrequencyInSeconds = 0.01f;           // Much more frequent when console is visible on load
		}
	}

	if (timeDelta < s_updateFrequencyInSeconds)   // Update screen not frequently
	{
		return;
	}
#if defined(CHECK_UPDATE_TIMES)
#if DRX_PLATFORM_DURANGO
	else if (timeDelta > 1.0f)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "CSystem::UpdateLoadingScreen %f seconds since last tick: this is a long delay and a serious risk for failing XBOX ONE TCR - XR-004-01 \n", timeDelta);
	}
#endif
#endif
	t0 = t;

	if (!m_bEditor && !m_bQuit)
	{
		if (m_pProgressListener)
		{
			m_pProgressListener->OnLoadingProgress(0);
		}
	}
	// This happens during loading, give windows opportunity to process window messages.
#if DRX_PLATFORM_WINDOWS
	if (m_hWnd && ::IsWindow((HWND)m_hWnd))
	{
		gEnv->pSystem->PumpWindowMessage(true, m_hWnd);
	}

#endif

#if DRX_PLATFORM_DURANGO
	// poll system events on durango
	Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);
#endif
}

//////////////////////////////////////////////////////////////////////////

void CSystem::DisplayErrorMessage(tukk acMessage,
  float                                       fTime,
  const float*                                pfColor,
  bool                                        bHardError)
{
	SErrorMessage message;
	message.m_Message = acMessage;
	if (pfColor)
	{
		memcpy(message.m_Color, pfColor, 4 * sizeof(float));
	}
	else
	{
		message.m_Color[0] = 1.0f;
		message.m_Color[1] = 0.0f;
		message.m_Color[2] = 0.0f;
		message.m_Color[3] = 1.0f;
	}
	message.m_HardFailure = bHardError;
	message.m_fTimeToShow = fTime;
	m_ErrorMessages.push_back(message);
}

//! Renders the statistics; this is called from RenderEnd, but if the
//! Host application (Editor) doesn't employ the Render cycle in ISystem,
//! it may call this method to render the essential statistics
//////////////////////////////////////////////////////////////////////////
void CSystem::RenderStatistics()
{
	RenderStats();
#if defined(USE_FRAME_PROFILER)
	// Render profile info.
	m_FrameProfileSystem.Render();
	if (m_pThreadProfiler)
		m_pThreadProfiler->Render();

#if defined(USE_DISK_PROFILER)
	if (m_pDiskProfiler)
		m_pDiskProfiler->Update();
#endif

	RenderMemStats();

#endif
	if (gEnv->pScaleformHelper)
	{
		gEnv->pScaleformHelper->RenderFlashInfo();
	}

	// [VR]
	// Update debug info
	if (m_sys_vr_support && m_sys_vr_support->GetIVal())
	{
		GetHmdUpr()->Action(IHmdUpr::EHmdAction::eHmdAction_DrawInfo);
	}

	RenderThreadInfo();
#if !defined(_RELEASE)
	if (m_sys_enable_budgetmonitoring->GetIVal())
		m_pIBudgetingSystem->MonitorBudget();
#endif
}

//////////////////////////////////////////////////////////////////////
void CSystem::Render()
{
	if (m_bIgnoreUpdates)
		return;

	//check what is the current process
	if (!m_pProcess)
		return; //should never happen

	DRX_PROFILE_FUNCTION(PROFILE_SYSTEM);
	DRX_PROFILE_MARKER("CSystem::Render");

	//////////////////////////////////////////////////////////////////////
	//draw
	m_env.p3DEngine->PreWorldStreamUpdate(m_ViewCamera);

	if (m_env.pRenderer)
	{
		if (m_pProcess->GetFlags() & PROC_3DENGINE)
		{
			if (!m_env.IsEditing())  // Editor calls it's own rendering update
			{

#if !defined(_RELEASE)
				if (m_pTestSystem)
					m_pTestSystem->BeforeRender();
#endif

				if (m_env.p3DEngine && !m_env.IsFMVPlaying())
				{
					if ((!IsEquivalent(m_ViewCamera.GetPosition(), Vec3(0, 0, 0), VEC_EPSILON) && (!IsLoading())) || // never pass undefined camera to p3DEngine->RenderWorld()
						m_env.IsDedicated() || m_env.pRenderer->IsPost3DRendererEnabled())
					{
						m_env.p3DEngine->RenderWorld(SHDF_ALLOW_WATER | SHDF_ALLOWPOSTPROCESS | SHDF_ALLOWHDR | SHDF_ZPASS | SHDF_ALLOW_AO, SRenderingPassInfo::CreateGeneralPassRenderingInfo(m_ViewCamera), __FUNCTION__);
					}
					else
					{
						m_env.pRenderer->FillFrame(Col_Black);
					}
				}

#if !defined(_RELEASE)
				if (m_pVisRegTest)
					m_pVisRegTest->AfterRender();
				if (m_pTestSystem)
					m_pTestSystem->AfterRender();

				//			m_pProcess->Draw();

				if (m_env.pAISystem)
					m_env.pAISystem->DebugDraw();
#endif

				if (m_env.pMovieSystem)
					m_env.pMovieSystem->Render();
			}
		}
		else
		{
			m_pProcess->RenderWorld(SHDF_ALLOW_WATER | SHDF_ALLOWPOSTPROCESS | SHDF_ALLOWHDR | SHDF_ZPASS | SHDF_ALLOW_AO, SRenderingPassInfo::CreateGeneralPassRenderingInfo(m_ViewCamera), __FUNCTION__);
		}
	}

	m_env.p3DEngine->WorldStreamUpdate();
#if !defined (_RELEASE) && DRX_PLATFORM_DURANGO
	RenderPhysicsHelpers();
#endif
}

//////////////////////////////////////////////////////////////////////////
void CSystem::RenderMemStats()
{
#if !defined(_RELEASE)
	// check for the presence of the system
	if (!m_env.pConsole || !m_env.pRenderer || !m_cvMemStats->GetIVal())
	{
		SAFE_DELETE(m_pMemStats);
		return;
	}

	TickMemStats();

	assert (m_pMemStats);
	m_pMemStats->updateKeys();
	// render the statistics
	{
		DrxSizerStatsRenderer StatsRenderer (this, m_pMemStats, m_cvMemStatsMaxDepth->GetIVal(), m_cvMemStatsThreshold->GetIVal());
		StatsRenderer.render((m_env.pRenderer->GetFrameID(false) + 2) % m_cvMemStats->GetIVal() <= 1);
	}
#endif

}

//////////////////////////////////////////////////////////////////////////
void CSystem::RenderStats()
{
	if (!m_env.pRenderer)
	{
		return;
	}

#if defined(ENABLE_PROFILING_CODE)
#ifndef _RELEASE
	// if we rendered an error message on screen during the last frame, then sleep now
	// to force hard stall for 3sec
	if (m_bHasRenderedErrorMessage && !gEnv->IsEditor() && !IsLoading())
	{
		// DO NOT REMOVE OR COMMENT THIS OUT!
		// If you hit this, then you most likely have invalid (synchronous) file accesses
		// which must be fixed in order to not stall the entire game.
		DrxSleep(3000);
		m_bHasRenderedErrorMessage = false;
	}
#endif

	// render info messages on screen
	float fTextPosX  = 5.0f;
	float fTextPosY  = -10;
	float fTextStepY = 13;

	float fFrameTime = gEnv->pTimer->GetRealFrameTime();
	TErrorMessages::iterator itnext;
	for (TErrorMessages::iterator it = m_ErrorMessages.begin(); it != m_ErrorMessages.end(); it = itnext)
	{
		itnext = it;
		++itnext;
		SErrorMessage& message = *it;

		IRenderAuxText::DrawText(Vec3(fTextPosX, fTextPosY += fTextStepY, 1.0f), 1.4f, message.m_Color, eDrawText_FixedSize | eDrawText_2D | eDrawText_Monospace, message.m_Message.c_str());

		message.m_fTimeToShow -= fFrameTime;

		if (message.m_HardFailure)
			m_bHasRenderedErrorMessage = true;

		if (message.m_fTimeToShow < 0.0f)
		{
			m_ErrorMessages.erase(it);
		}
	}
#endif

	if (!m_env.pConsole)
		return;

#ifndef _RELEASE
	if (m_rOverscanBordersDrawDebugView)
	{
		RenderOverscanBorders();
	}
#endif

	i32 iDisplayInfo = m_rDisplayInfo->GetIVal();
	if (iDisplayInfo == 0)
		return;

	// Draw engine stats
	if (m_env.p3DEngine)
	{
		// Draw 3dengine stats and get last text cursor position
		float nTextPosX = 101 - 20, nTextPosY = -2, nTextStepY = 3;
		m_env.p3DEngine->DisplayInfo(nTextPosX, nTextPosY, nTextStepY, iDisplayInfo != 1);

#if defined(ENABLE_LW_PROFILERS)
		if (m_rDisplayInfo->GetIVal() == 2)
		{
			IRenderAuxText::TextToScreenF(nTextPosX, nTextPosY += nTextStepY, "SysMem %.1f mb",
			  float(DumpMMStats(false)) / 1024.f);

			//if (m_env.pAudioSystem)
			//{
			//	SSoundMemoryInfo SoundMemInfo;
			//	m_env.pAudioSystem->GetInterfaceExtended()->GetMemoryInfo(&SoundMemInfo);
			//	m_env.pRenderer->TextToScreen( nTextPosX-18, nTextPosY+=nTextStepY, "------------Sound %2.1f/%2.1f mb",
			//		SoundMemInfo.fSoundBucketPoolUsedInMB + SoundMemInfo.fSoundPrimaryPoolUsedInMB + SoundMemInfo.fSoundSecondaryPoolUsedInMB,
			//		/*SoundMemInfo.fSoundBucketPoolSizeInMB +*/ SoundMemInfo.fSoundPrimaryPoolSizeInMB + SoundMemInfo.fSoundSecondaryPoolSizeInMB);
			//}
		}
#endif

#if 0
		for (i32 i = 0; i < NUM_POOLS; ++i)
		{
			i32 used     = (g_pPakHeap->m_iBigPoolUsed[i] ? (i32)g_pPakHeap->m_iBigPoolSize[i] : 0);
			i32 size     = (i32)g_pPakHeap->m_iBigPoolSize[i];
			float fC1[4] = {1, 1, 0, 1};
			m_env.pRenderer->Draw2dLabel(10, 100.0f + i * 16, 2.1f, fC1, false, "BigPool %d: %d bytes of %d bytes used", i, used, size);
		}
#endif
	}
}

void CSystem::RenderOverscanBorders()
{
#ifndef _RELEASE

	if (m_env.pRenderer && m_rOverscanBordersDrawDebugView)
	{
		i32 iOverscanBordersDrawDebugView = m_rOverscanBordersDrawDebugView->GetIVal();
		if (iOverscanBordersDrawDebugView)
		{
			i32k texId          = -1;
			const float uv           = 0.0f;
			const float rot          = 0.0f;
			i32k whiteTextureId = m_env.pRenderer->GetWhiteTextureId();

			const float r = 1.0f;
			const float g = 1.0f;
			const float b = 1.0f;
			const float a = 0.2f;

			Vec2 overscanBorders = Vec2(0.0f, 0.0f);
			m_env.pRenderer->EF_Query(EFQ_OverscanBorders, overscanBorders);

			const float overscanBorderWidth  = overscanBorders.x * VIRTUAL_SCREEN_WIDTH;
			const float overscanBorderHeight = overscanBorders.y * VIRTUAL_SCREEN_HEIGHT;

			const float xPos   = overscanBorderWidth;
			const float yPos   = overscanBorderHeight;
			const float width  = VIRTUAL_SCREEN_WIDTH - (2.0f * overscanBorderWidth);
			const float height = VIRTUAL_SCREEN_HEIGHT - (2.0f * overscanBorderHeight);

			//m_env.pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
			IRenderAuxImage::Draw2dImage(xPos, yPos,
			  width, height,
			  whiteTextureId,
			  uv, uv, uv, uv,
			  rot,
			  r, g, b, a);
		}
	}
#endif
}

void CSystem::RenderThreadInfo()
{
#if (DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO) && !defined(RELEASE)
	if (g_cvars.sys_display_threads)
	{
		static i32 maxCPU           = 0;
		i32k  maxAvailCPU      = 6;
		static u32 maxMask = 0;

		if (maxCPU == 0)
		{
			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);
			maxCPU  = sysinfo.dwNumberOfProcessors;
			maxMask = (1 << maxCPU) - 1;
		}

		struct SThreadProcessorInfo
		{
			SThreadProcessorInfo(const string& _name, u32 _id) : name(_name), id(_id) {}
			string name;
			u32 id;
		};

		static std::multimap<DWORD, SThreadProcessorInfo> sortetThreads;
		static SThreadInfo::TThreads threads;
		static SThreadInfo::TThreadInfo threadInfo;
		static i32 frame = 0;

		if ((frame++ % 100) == 0) // update thread info every 100 frame
		{
			threads.clear();
			threadInfo.clear();
			sortetThreads.clear();
			SThreadInfo::OpenThreadHandles(threads, SThreadInfo::TThreadIds(), false);
			SThreadInfo::GetCurrentThreads(threadInfo);
			for (SThreadInfo::TThreads::const_iterator it = threads.begin(); it != threads.end(); ++it)
			{
				DWORD mask = (DWORD)SetThreadAffinityMask(it->Handle, maxMask);
				SetThreadAffinityMask(it->Handle, mask);
				sortetThreads.insert(std::make_pair(mask, SThreadProcessorInfo(threadInfo[it->Id], it->Id)));
			}
			SThreadInfo::CloseThreadHandles(threads);
		}

		float  nX     = 5, nY = 10, dY = 12, dX = 10;
		float  fFSize = 1.2;
		ColorF col1   = Col_Yellow;
		ColorF col2   = Col_Red;

		for (i32 i = 0; i < maxCPU; ++i)
			IRenderAuxText::Draw2dLabel(nX + i * dX, nY, fFSize, i < maxAvailCPU ? &col1.r : &col2.r, false, "%i", i + 1);

		nY += dY;
		for (std::multimap<DWORD, SThreadProcessorInfo>::const_iterator it = sortetThreads.begin(); it != sortetThreads.end(); ++it)
		{
			for (i32 i = 0; i < maxCPU; ++i)
				IRenderAuxText::Draw2dLabel(nX + i * dX, nY, fFSize, i < maxAvailCPU ? &col1.r : &col2.r, false, "%s", it->first & BIT(i) ? "X" : "-");

			IRenderAuxText::Draw2dLabel(nX + dX * maxCPU, nY, fFSize, &col1.r, false, "Thread: %s (0x%X)", it->second.name.c_str(), it->second.id);
			nY += dY;
		}
	}
#endif
}
