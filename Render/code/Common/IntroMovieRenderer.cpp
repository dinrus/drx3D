// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>

#include <drx3D/Render/IntroMovieRenderer.h>
#include <DinrusXSys/ILocalizationUpr.h>
#include <DinrusXSys/Scaleform/IScaleformHelper.h>

//////////////////////////////////////////////////////////////////////////

// this maps languages to subtitle channels
static tukk g_subtitleMapping[] =
{
	"main",       // channel 0, music and sound effects
	"english",    // channel 1
	"french",     // channel 2
	"german",     // channel 3
	"italian",    // channel 4
	"russian",    // channel 5
	"polish",     // channel 6
	"spanish",    // channel 7
	"turkish",    // channel 8
	"japanese",   // channel 9
	"czech",      // channel 10
	"chineset"    // channel 11
};

static const size_t g_subtitleMappingCount = (DRX_ARRAY_COUNT(g_subtitleMapping));

//////////////////////////////////////////////////////////////////////////

bool CIntroMovieRenderer::Initialize()
{
	m_pFlashPlayer = gEnv->pScaleformHelper ? gEnv->pScaleformHelper->CreateFlashPlayerInstance() : nullptr;

	if (m_pFlashPlayer)
	{
		m_pFlashPlayer->Load("libs/ui/usm_player_intro.gfx");
		m_pFlashPlayer->Advance(0.f);

		m_pFlashPlayer->SetFSCommandHandler(this);
		m_pFlashPlayer->SetBackgroundAlpha(0.0f);
		m_pFlashPlayer->Advance(0.0f);

		m_pFlashPlayer->SetVariable("showSubtitle", true);
		m_pFlashPlayer->SetVariable("selectedSubtitle", GetSubtitleChannelForSystemLanguage());

		m_pFlashPlayer->SetVariable("canSkip", SFlashVarValue(false));

		m_pFlashPlayer->Invoke1("setMoviePath", "/Videos/IntroMovies.usm");

		m_pFlashPlayer->Invoke1("setFollowUpAnimation", false);
		m_pFlashPlayer->Invoke1("setBlackBackground", true);

		m_pFlashPlayer->Invoke0("vPlay");
	}

	return true;
}

CIntroMovieRenderer::EVideoStatus CIntroMovieRenderer::GetCurrentStatus()
{
	if (m_pFlashPlayer)
	{
		SFlashVarValue val(0.0);
		m_pFlashPlayer->GetVariable("m_status", val);
		EVideoStatus status = (EVideoStatus)int_round(val.GetDouble());
		return status;
	}
	return eVideoStatus_Error;
}

void CIntroMovieRenderer::WaitForCompletion()
{
	bool bFinished = false;
	while (!bFinished)
	{
		EVideoStatus status = GetCurrentStatus();

		switch (status)
		{
		case eVideoStatus_PrePlaying:
		case eVideoStatus_Stopped:
		case eVideoStatus_Error:
		case eVideoStatus_Finished:
			{
				bFinished = true;
			}
			break;
		}
		gEnv->pLog->UpdateLoadingScreen(0);
		DrxSleep(1);
	}
}

i32 CIntroMovieRenderer::GetSubtitleChannelForSystemLanguage()
{
	tukk curLanguage = NULL;
	if (ICVar* pCVar_g_language = gEnv->pConsole->GetCVar("g_language"))
	{
		curLanguage = pCVar_g_language->GetString();
	}
	if (curLanguage != NULL && *curLanguage)
	{
		for (i32 i = 0; i < g_subtitleMappingCount; ++i)
		{
			if (stricmp(g_subtitleMapping[i], curLanguage) == 0)
				return i;
		}
	}

	// show english subtitles
	return 1;
}

//////////////////////////////////////////////////////////////////////////

void CIntroMovieRenderer::LoadtimeUpdate(float deltaTime)
{
	auto pPlayer = m_pFlashPlayer;
	if (pPlayer)
	{
		UpdateViewport();
		pPlayer->Advance(deltaTime);
	}
}

void CIntroMovieRenderer::LoadtimeRender()
{
	auto pPlayer = m_pFlashPlayer;
	if (pPlayer)
		pPlayer->Render();
}

void CIntroMovieRenderer::UpdateViewport()
{
	auto pPlayer = m_pFlashPlayer;
	if (!pPlayer)
		return;

	i32 videoWidth (pPlayer->GetWidth());
	i32 videoHeight(pPlayer->GetHeight());

	i32k screenWidth (gEnv->pRenderer->GetWidth ());
	i32k screenHeight(gEnv->pRenderer->GetHeight());

	const float pixelAR = gEnv->pRenderer->GetPixelAspectRatio();

	const float scaleX((float)screenWidth  / (float)videoWidth);
	const float scaleY((float)screenHeight / (float)videoHeight);

	float scale(scaleY);

	/*
	   if(flags&eVideoFlag_KeepAspectRatio)
	   {
	   if (flags&eVideoFlag_CoverFullScreen)
	   {
	    float videoRatio((float)videoWidth / (float)videoHeight);
	    float screenRatio((float)screenWidth / (float)screenHeight);

	    if (videoRatio < screenRatio)
	      scale = scaleX;
	   }
	   else
	   {
	    if (scaleY * videoWidth > screenWidth)
	      scale = scaleX;
	   }
	   }
	   else
	 */
	{
		scale = 1.0f;
		videoWidth = screenWidth;
		videoHeight = screenHeight;
	}

	i32 w(int_round(videoWidth * scale));
	i32 h(int_round(videoHeight * scale));
	i32 x((screenWidth - w) / 2);
	i32 y((screenHeight - h) / 2);

	SetViewportIfChanged(x, y, w, h, 1.0f);
}

void CIntroMovieRenderer::SetViewportIfChanged(i32k x, i32k y, i32k width, i32k height, const float pixelAR)
{
	i32 oldX, oldY, oldWidth, oldHeight = 0;
	float oldAR = 1.0f;
	m_pFlashPlayer->GetViewport(oldX, oldY, oldWidth, oldHeight, oldAR);
	if (x != oldX || y != oldY || width != oldWidth || height != oldHeight || fabs(oldAR - pixelAR) > FLT_EPSILON)
	{
		m_pFlashPlayer->SetViewport(x, y, width, height, pixelAR);
	}
}

//////////////////////////////////////////////////////////////////////////
