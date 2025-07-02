// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#ifdef INCLUDE_SCALEFORM_SDK

#include <drx3D/Sys/IScaleformHelper.h>
#include <drx3D/Sys/ISystem.h>

#include <drx3D/Sys/ConfigScaleform.h>
#include <drx3D/Sys/SharedResources.h>
#include <drx3D/Sys/SharedStates.h>
#include <drx3D/Sys/FlashPlayerInstance.h>

#if defined(GFX_AMP_SERVER)
#include <GFxAmpServer.h>
#endif

class CScaleformHelper final : public IScaleformHelper
{
public:
	virtual bool Init() override
	{
		CFlashPlayer::InitCVars();
		CSharedFlashPlayerResources::Init();
		return true;
	}

	virtual void Destroy() override
	{
		CSharedFlashPlayerResources::Shutdown();
		delete this;
	}

	virtual void SetAmpEnabled(bool bEnabled) override
	{
#if defined(GFX_AMP_SERVER)
		GFxAmpServer::GetInstance().SetState(Amp_Disabled, !bEnabled);
#endif
	}

	virtual void AmpAdvanceFrame() override
	{
#if defined(GFX_AMP_SERVER)
		GFxAmpServer::GetInstance().AdvanceFrame();
#endif
	}
	
	virtual void SetTranslatorWordWrappingMode(tukk szLanguage) override
	{
		DrxGFxTranslator::GetAccess().SetWordWrappingMode(szLanguage);
	}

	virtual void SetTranslatorDirty(bool bDirty) override
	{
		DrxGFxTranslator::GetAccess().SetDirty(true);
	}

	virtual void ResetMeshCache() override
	{
		CSharedFlashPlayerResources::GetAccess().ResetMeshCache();
	}

	virtual std::shared_ptr<IFlashPlayer> CreateFlashPlayerInstance() override
	{
		return std::make_shared<CFlashPlayer>();
	}

	virtual IFlashPlayerBootStrapper* CreateFlashPlayerBootStrapper() override
	{
		return CFlashPlayer::CreateBootstrapper();
	}

	virtual void RenderFlashInfo() override
	{
		CFlashPlayer::RenderFlashInfo();
	}

	virtual void GetFlashMemoryUsage(IDrxSizer* pSizer) override
	{
		CSharedFlashPlayerResources::GetAccess().GetMemoryUsage(pSizer);
	}

	virtual void SetFlashLoadMovieHandler(IFlashLoadMovieHandler* pHandler) override
	{
		CFlashPlayer::SetFlashLoadMovieHandler(pHandler);
	}

	virtual float GetFlashProfileResults() override
	{
		float accumTime;
		CFlashPlayer::GetFlashProfileResults(accumTime);
		return accumTime;
	}

	virtual void GetFlashRenderStats(unsigned& numDPs, u32& numTris) override
	{
		numDPs = 0;
		numTris = 0;
#ifndef _RELEASE
		IScaleformRecording* pFlashRenderer(CSharedFlashPlayerResources::GetAccess().GetRenderer(true));

		if (pFlashRenderer)
		{
			GRenderer::Stats stats;
			pFlashRenderer->GetRenderStats(&stats, false);

			numDPs = stats.Primitives;
			numTris = stats.Triangles;
		}
#endif
	}

	virtual void SetRenderThreadIDs(threadID main, threadID render) override
	{
		IScaleformRecording* pRenderer = CSharedFlashPlayerResources::GetAccess().GetRenderer(true);
		if (pRenderer)
		{
			pRenderer->SetThreadIDs(main, render);
		}
	}
};

#endif
