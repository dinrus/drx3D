// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DriverD3D.h>

//=========================================================================================

#include <drx3D/Render/DinrusXFont/FBitmap.h>

bool CD3D9Renderer::FontUpdateTexture(i32 nTexId, i32 nX, i32 nY, i32 USize, i32 VSize, byte* pSrcData)
{
	CTexture* tp = CTexture::GetByID(nTexId);
	assert(tp);

	if (tp)
	{
		tp->UpdateTextureRegion(pSrcData, nX, nY, 0, USize, VSize, 1, eTF_A8);

		return true;
	}
	return false;
}

bool CD3D9Renderer::FontUploadTexture(class CFBitmap* pBmp, ETEX_Format eSrcFormat)
{
	if (!pBmp)
	{
		return false;
	}

	u32* pSrcData = new u32[pBmp->GetWidth() * pBmp->GetHeight()];

	if (!pSrcData)
	{
		return false;
	}

	pBmp->Get32Bpp(&pSrcData);

	char szName[128];
	drx_sprintf(szName, "$AutoFont_%d", m_TexGenID++);

	i32 iFlags = FT_TEX_FONT | FT_DONT_STREAM | FT_DONT_RELEASE;
	CTexture* tp = CTexture::GetOrCreate2DTexture(szName, pBmp->GetWidth(), pBmp->GetHeight(), 1, iFlags, (u8*)pSrcData, eSrcFormat);

	SAFE_DELETE_ARRAY(pSrcData);

	pBmp->SetRenderData((uk )tp);

	return true;
}

i32 CD3D9Renderer::FontCreateTexture(i32 Width, i32 Height, byte* pSrcData, ETEX_Format eSrcFormat, bool genMips)
{
	if (!pSrcData)
		return -1;

	char szName[128];
	drx_sprintf(szName, "$AutoFont_%d", m_TexGenID++);

	i32 iFlags = FT_TEX_FONT | FT_DONT_STREAM | FT_DONT_RELEASE;
	if (genMips)
		iFlags |= FT_FORCE_MIPS;
	CTexture* tp = CTexture::GetOrCreate2DTexture(szName, Width, Height, 1, iFlags, (u8*)pSrcData, eSrcFormat);

	return tp->GetID();
}

void CD3D9Renderer::FontReleaseTexture(class CFBitmap* pBmp)
{
	if (!pBmp)
	{
		return;
	}

	CTexture* tp = (CTexture*)pBmp->GetRenderData();

	SAFE_RELEASE(tp);
}
