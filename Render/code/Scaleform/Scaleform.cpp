// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>

#include <drx3D/Render/Renderer.h>
#include <memory>

//////////////////////////////////////////////////////////////////////////
void CRenderer::FlashRender(std::shared_ptr<IFlashPlayer_RenderProxy> &&pPlayer)
{
	m_pRT->RC_FlashRender(std::move(pPlayer));
}

void CRenderer::FlashRenderPlayer(std::shared_ptr<IFlashPlayer> &&pPlayer)
{
	m_pRT->RC_FlashRenderPlayer(std::move(pPlayer));
}

void CRenderer::FlashRenderPlaybackLockless(std::shared_ptr<IFlashPlayer_RenderProxy> &&pPlayer, i32 cbIdx, bool finalPlayback)
{
	m_pRT->RC_FlashRenderPlaybackLockless(std::move(pPlayer), cbIdx, finalPlayback);
}

void CRenderer::FlashRemoveTexture(ITexture* pTexture)
{
	pTexture->Release();
}
