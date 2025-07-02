// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>

#ifdef INCLUDE_SCALEFORM_SDK

#include <drx3D/Sys/GTexture_Impl.h>

	#include <drx3D/Sys/GMemorySTLAlloc.h>
	#include <drx3D/CoreX/String/StringUtils.h>
	#include <GImage.h>

	#include <drx3D/CoreX/Renderer/IRenderer.h>
	#include <drx3D/Sys/ISystem.h>
	#include <drx3D/Sys/ILog.h>
	#include <vector>

	#include <drx3D/CoreX/DrxEndian.h>

	#ifndef RELEASE
		#include <drx3D/Sys/IFlashUI.h>
	#endif

static inline ETEX_Format MapImageType(const GImageBase::ImageFormat& fmt)
{
	switch (fmt)
	{
	// MapImageType returns BGR/BGRA for RGB/RGBA for lack of RGB-enum
	case GImageBase::Image_ARGB_8888:
		return eTF_B8G8R8A8;
	case GImageBase::Image_RGB_888:
		return eTF_B8G8R8;
	case GImageBase::Image_L_8:
		return eTF_L8;
	case GImageBase::Image_A_8:
		return eTF_A8;
	case GImageBase::Image_DXT1:
		return eTF_BC1;
	case GImageBase::Image_DXT3:
		return eTF_BC2;
	case GImageBase::Image_DXT5:
		return eTF_BC3;
	default:
		return eTF_Unknown;
	}
}

//////////////////////////////////////////////////////////////////////////
// GTextureXRenderBase

	#if defined(ENABLE_FLASH_INFO)
 i32 GTextureXRenderBase::ms_textureMemoryUsed(0);
 i32 GTextureXRenderBase::ms_numTexturesCreated(0);
 i32 GTextureXRenderBase::ms_numTexturesCreatedTotal(0);
 i32 GTextureXRenderBase::ms_numTexturesCreatedTemp(0);
 i32 GTextureXRenderBase::ms_fontCacheTextureID(0);
	#endif

GTextureXRenderBase::GTextureXRenderBase(GRenderer* pRenderer, i32 texID /*= -1*/, ETEX_Format fmt /*= eTF_Unknown*/, bool bIsTemp /*= false*/)
	: m_texID(texID)
	, m_fmt(fmt)
	, m_userData(0)
	, m_pRenderer(pRenderer)
	#if defined(ENABLE_FLASH_INFO)
	, m_isTemp(bIsTemp)
	#endif
{
	assert(m_pRenderer);
	#if defined(ENABLE_FLASH_INFO)
	DrxInterlockedIncrement(&ms_numTexturesCreated);
	if (bIsTemp)
		DrxInterlockedIncrement(&ms_numTexturesCreatedTemp);
	else
		DrxInterlockedIncrement(&ms_numTexturesCreatedTotal);
	#endif

	m_pRenderer->AddRef();
}

GTextureXRenderBase::~GTextureXRenderBase()
{
	#if defined(ENABLE_FLASH_INFO)
	DrxInterlockedDecrement(&ms_numTexturesCreated);
	if (m_isTemp)
		DrxInterlockedDecrement(&ms_numTexturesCreatedTemp);
	#endif

	m_pRenderer->Release();
}

GRenderer* GTextureXRenderBase::GetRenderer() const
{
	return m_pRenderer;
}

//////////////////////////////////////////////////////////////////////////
// GTextureXRender

GTextureXRender::GTextureXRender(GRenderer* pRenderer, i32 texID /*= -1*/, ETEX_Format fmt /*= eTF_Unknown*/) : GTextureXRenderBase(pRenderer, texID, fmt)
{
}

GTextureXRender::~GTextureXRender()
{
	if (m_texID > 0)
	{
		IRenderer* pRenderer(gEnv->pRenderer);
		ITexture* pTexture(pRenderer->EF_GetTextureByID(m_texID));
		assert(pTexture);
	#if defined(ENABLE_FLASH_INFO)
		if (!pTexture->IsShared())
		{
			i32 size = pTexture->GetDeviceDataSize();
			DrxInterlockedAdd(&ms_textureMemoryUsed, -size);
		}
		if (ms_fontCacheTextureID == m_texID)
			ms_fontCacheTextureID = 0;
	#endif
		pRenderer->FlashRemoveTexture(pTexture);
	}
}

bool GTextureXRender::InitTexture(GImageBase* pIm, UInt /*usage*/)
{
	assert(m_texID == -1);
	if (InitTextureInternal(pIm))
	{
		IRenderer* pRenderer(gEnv->pRenderer);
		ITexture* pTexture(pRenderer->EF_GetTextureByID(m_texID));
		assert(pTexture && !pTexture->IsShared());
	#if defined(ENABLE_FLASH_INFO)
		i32 size = pTexture->GetDeviceDataSize();
		DrxInterlockedAdd(&ms_textureMemoryUsed, size);
	#endif
		m_fmt = pTexture->GetTextureSrcFormat();
	}
	return m_texID > 0;
}

bool GTextureXRender::InitTextureFromFile(tukk pFilename)
{
	assert(m_texID == -1);

	string sFile = PathUtil::ToUnixPath(pFilename);
	sFile.replace("//", "/");

	IRenderer* pRenderer(gEnv->pRenderer);
	ICVar* pMipmapsCVar = gEnv->pConsole->GetCVar("sys_flash_mipmaps");
	u32 mips_flag = pMipmapsCVar && pMipmapsCVar->GetIVal() ? 0 : FT_NOMIPS;

	ITexture* pTexture(pRenderer->EF_LoadTexture(sFile.c_str(), FT_DONT_STREAM | mips_flag));
	if (pTexture)
	{
	#ifndef RELEASE
		if (gEnv->pFlashUI)
		{
			gEnv->pFlashUI->CheckPreloadedTexture(pTexture);
		}
	#endif
		m_texID = pTexture->GetTextureID();
	#if defined(ENABLE_FLASH_INFO)
		if (!pTexture->IsShared())
		{
			i32 size = pTexture->GetDeviceDataSize();
			DrxInterlockedAdd(&ms_textureMemoryUsed, size);
		}
	#endif
		m_fmt = pTexture->GetTextureSrcFormat();
	}
	return m_texID > 0;
}

bool GTextureXRender::InitTextureFromTexId(i32 texid)
{
	assert(m_texID == -1);
	IRenderer* pRenderer(gEnv->pRenderer);
	ITexture* pTexture(pRenderer->EF_GetTextureByID(texid));
	if (pTexture)
	{
		pTexture->AddRef();
		m_texID = pTexture->GetTextureID();
	#if defined(ENABLE_FLASH_INFO)
		if (!pTexture->IsShared())
		{
			i32 size = pTexture->GetDeviceDataSize();
			DrxInterlockedAdd(&ms_textureMemoryUsed, size);
		}
	#endif
		m_fmt = pTexture->GetTextureSrcFormat();
	}
	return m_texID > 0;
}

bool GTextureXRender::InitTextureInternal(const GImageBase* pIm)
{
	ETEX_Format texFmt(MapImageType(pIm->Format));
	if (texFmt != eTF_Unknown)
	{
		return InitTextureInternal(texFmt, pIm->Width, pIm->Height, pIm->Pitch, pIm->pData);
	}
	else
	{
		gEnv->pLog->LogWarning("<Flash> GTextureXRender::InitTextureInternal( ... ) -- Attempted to load texture with unsupported texture format!\n");
		return false;
	}
}

bool GTextureXRender::InitTextureInternal(ETEX_Format texFmt, i32 width, i32 height, i32 pitch, u8* pData)
{
	IRenderer* pRenderer(gEnv->pRenderer);

	bool bRGBA((pRenderer->GetFeatures() & RFT_RGBA) != 0 || pRenderer->GetRenderType() >= ERenderType::Direct3D11);
	bool bSwapRB(texFmt == eTF_B8G8R8 || texFmt == eTF_B8G8R8X8 || texFmt == eTF_B8G8R8A8);
	ETEX_Format texFmtOrig(texFmt);

	// expand RGB to RGBX if necessary
	std::vector<u8, GMemorySTLAlloc<u8>> expandRGB8;
	if (texFmt == eTF_B8G8R8)
	{
		expandRGB8.reserve(width * height * 4);
		for (i32 y(0); y < height; ++y)
		{
			u8* pCol(&pData[pitch * y]);
			for (i32 x(0); x < width; ++x, pCol += 3)
			{
				expandRGB8.push_back(pCol[0]);
				expandRGB8.push_back(pCol[1]);
				expandRGB8.push_back(pCol[2]);
				expandRGB8.push_back(255);
			}
		}

		pData = &expandRGB8[0];
		pitch = width * 4;
		texFmt = eTF_B8G8R8X8;
	}

	if (bSwapRB)
	{
		// software-swap if no RGBA layout supported
		if (!bRGBA)
			SwapRB(pData, width, height, pitch);
		// otherwise swap by casting to swizzled format
		else if (texFmt == eTF_B8G8R8X8 || texFmt == eTF_B8G8R8A8)
			texFmt = eTF_R8G8B8A8;
		else if (texFmt == eTF_R8G8B8A8)
			texFmt = eTF_B8G8R8A8;
	}

	#if defined(NEED_ENDIAN_SWAP)
	if (texFmt == eTF_R8G8B8A8 || texFmt == eTF_B8G8R8A8 || texFmt == eTF_B8G8R8X8)
		SwapEndian(pData, width, height, pitch);
	#endif

	// Create texture...
	// Note that mip-maps should be generated for font textures (A8/L8) as
	// Scaleform generates font textures only once and relies on mip-mapping
	// to implement various font sizes.
	m_texID = pRenderer->SF_CreateTexture(width, height, (texFmt == eTF_A8 || texFmt == eTF_L8) ? 0 : 1, pData, texFmt, FT_DONT_STREAM);

	if (m_texID <= 0)
	{
		static tukk s_fomats[] =
		{
			"Unknown",
			"RGBA8",
			"RGB8->RGBA8",
			"L8",
			"A8",
			"DXT1",
			"DXT3",
			"DXT5",
			0
		};

		i32 fmtIdx(0);
		switch (texFmtOrig)
		{
		case eTF_B8G8R8A8:
			fmtIdx = 1;
			break;
		case eTF_B8G8R8:
			fmtIdx = 2;
			break;
		case eTF_L8:
			fmtIdx = 3;
			break;
		case eTF_A8:
			fmtIdx = 4;
			break;
		case eTF_BC1:
			fmtIdx = 5;
			break;
		case eTF_BC2:
			fmtIdx = 6;
			break;
		case eTF_BC3:
			fmtIdx = 7;
			break;
		}

		gEnv->pLog->LogWarning("<Flash> GTextureXRender::InitTextureInternal( ... ) "
		                       "-- Texture creation failed (%dx%d, %s)\n", width, height, s_fomats[fmtIdx]);
	}

	#if defined(NEED_ENDIAN_SWAP)
	if (texFmt == eTF_R8G8B8A8 || texFmt == eTF_B8G8R8A8 || texFmt == eTF_B8G8R8X8)
		SwapEndian(pData, width, height, pitch);
	#endif

	if (bSwapRB && !bRGBA)
		SwapRB(pData, width, height, pitch);

	return m_texID > 0;
}

bool GTextureXRender::InitDynamicTexture(i32 width, i32 height, GImage::ImageFormat format, i32 mipmaps, UInt usage)
{
	// Currently only used for font cache texture.
	if (Usage_Update == usage)
	{
		IRenderer* pRenderer(gEnv->pRenderer);
		assert(m_texID == -1);
		m_texID = pRenderer->SF_CreateTexture(width, height, mipmaps + 1, 0, MapImageType(format), FT_DONT_STREAM);
		if (m_texID > 0)
		{
			ITexture* pTexture(pRenderer->EF_GetTextureByID(m_texID));
			assert(pTexture && !pTexture->IsShared());
	#if defined(ENABLE_FLASH_INFO)
			i32 size = pTexture->GetDeviceDataSize();
			DrxInterlockedAdd(&ms_textureMemoryUsed, size);
			assert(!ms_fontCacheTextureID);
			ms_fontCacheTextureID = m_texID;
	#endif
			m_fmt = pTexture->GetTextureSrcFormat();
		}
	}
	else
	{
		assert(0 && "GTextureXRender::InitDynamicTexture() -- Unsupported usage flag!");
	}
	return m_texID > 0;
}

void GTextureXRender::Update(i32 level, i32 n, const UpdateRect* pRects, const GImageBase* pIm)
{
	DRX_PROFILE_FUNCTION(PROFILE_SYSTEM);

	assert(m_texID > 0);
	if (!pRects || !n || !pIm || m_texID <= 0)
		return;

	PREFAST_SUPPRESS_WARNING(6255)
	IRenderer::SUpdateRect * pSrcRects((IRenderer::SUpdateRect*) alloca(n * sizeof(IRenderer::SUpdateRect)));
	if (pSrcRects)
	{
		for (i32 i(0); i < n; ++i)
			pSrcRects[i].Set(pRects[i].dest.x, pRects[i].dest.y, pRects[i].src.Left, pRects[i].src.Top, pRects[i].src.Width(), pRects[i].src.Height());

		IRenderer* pRenderer(gEnv->pRenderer);
		pRenderer->SF_UpdateTexture(m_texID, level, n, pSrcRects, pIm->pData, pIm->Pitch, pIm->DataSize, MapImageType(pIm->Format));
	}
}

i32 GTextureXRender::Map(i32 /*level*/, i32 /*n*/, MapRect* /*maps*/, i32 /*flags*/)
{
	assert(0 && "GTextureXRender::Map() -- Not implemented!");
	return 0;
}

bool GTextureXRender::Unmap(i32 /*level*/, i32 /*n*/, MapRect* /*maps*/, i32 /*flags*/)
{
	assert(0 && "GTextureXRender::Unmap() -- Not implemented!");
	return false;
}

void GTextureXRender::SwapRB(u8* pImageData, u32 width, u32 height, u32 pitch) const
{
	for (u32 y(0); y < height; ++y)
	{
		u8* pCol(&pImageData[pitch * y]);
		for (u32 x(0); x < width; ++x, pCol += 4)
		{
			u8 s(pCol[0]);
			pCol[0] = pCol[2];
			pCol[2] = s;
		}
	}
}

	#if defined(NEED_ENDIAN_SWAP)
void GTextureXRender::SwapEndian(u8* pImageData, u32 width, u32 height, u32 pitch) const
{
	for (u32 y = 0; y < height; y++)
	{
		u32* pCol((u32*) &pImageData[pitch * y]);
		::SwapEndian(pCol, width);
	}
}
	#endif

//////////////////////////////////////////////////////////////////////////
// GTextureXRenderTempRT
GTextureXRenderTempRT::GTextureXRenderTempRT(GRenderer* pRenderer, i32 texID /*= -1*/, ETEX_Format fmt /*= eTF_Unknown*/) : GTextureXRender(pRenderer, texID, fmt)
{
}

GTextureXRenderTempRT::~GTextureXRenderTempRT()
{
}

//////////////////////////////////////////////////////////////////////////
// GTextureXRenderTempRTLockless
GTextureXRenderTempRTLockless::GTextureXRenderTempRTLockless(GRenderer* pRenderer) : GTextureXRenderBase(pRenderer, -1, eTF_Unknown, true)
	, m_pRT(0)
{
}

GTextureXRenderTempRTLockless::~GTextureXRenderTempRTLockless()
{
	m_pRT = 0;
}

//////////////////////////////////////////////////////////////////////////
// GTextureXRenderYUV

GTextureXRenderYUV::GTextureXRenderYUV(GRenderer* pRenderer) : GTextureXRenderBase(pRenderer)
	//, m_texIDs()
	, m_numIDs(0)
	, m_width(0)
	, m_height(0)
	, m_isStereoContent(false)
{
	m_texID = 0;
	m_texIDs[0] = m_texIDs[1] = m_texIDs[2] = m_texIDs[3] = -1;
}

GTextureXRenderYUV::~GTextureXRenderYUV()
{
	IRenderer* pRenderer = gEnv->pRenderer;
	for (i32 i = 0; i < m_numIDs; ++i)
	{
		i32 texId = m_texIDs[i];
		if (texId > 0)
		{
			ITexture* pTexture = pRenderer->EF_GetTextureByID(texId);
			assert(pTexture);
	#if defined(ENABLE_FLASH_INFO)
			if (!pTexture->IsShared())
			{
				i32 size = pTexture->GetDeviceDataSize();
				DrxInterlockedAdd(&ms_textureMemoryUsed, -size);
			}
	#endif
			pRenderer->FlashRemoveTexture(pTexture);
		}
	}
}

bool GTextureXRenderYUV::InitTexture(GImageBase* /*pIm*/, UInt /*usage*/)
{
	assert(0 && "GTextureXRenderYUV::InitTexture() -- Not implemented!");
	return false;
}

bool GTextureXRenderYUV::InitTextureFromFile(tukk /*pFilename*/)
{
	assert(0 && "GTextureXRenderYUV::InitTextureFromFile() -- Not implemented!");
	return false;
}

bool GTextureXRenderYUV::InitTextureFromTexId(i32 /*texid*/)
{
	assert(0 && "GTextureXRenderYUV::InitTextureFromTexId() -- Not implemented!");
	return false;
}

void GTextureXRenderYUV::Clear()
{
	static u8k clearVal[][4] = { {0,0,0,0}, {128,128,128,128}, {128,128,128,128}, {255,255,255,255} };

	i32k level = 0; // currently don't have mips so only clear level 0

	IRenderer* pRenderer(gEnv->pRenderer);
	for (i32 i = 0; i < m_numIDs; ++i)
	{
		pRenderer->SF_ClearTexture(m_texIDs[i], level, 1, nullptr, clearVal[i]);
	}
}

bool GTextureXRenderYUV::InitDynamicTexture(i32 width, i32 height, GImage::ImageFormat format, i32 mipmaps, UInt usage)
{
	assert(format == GImageBase::Image_RGB_888 || format == GImageBase::Image_ARGB_8888);
	if (Usage_Map == usage && (format == GImageBase::Image_RGB_888 || format == GImageBase::Image_ARGB_8888))
	{
		IRenderer* pRenderer(gEnv->pRenderer);
		bool ok = true;

		ITexture* pTextures[4];
		i32 textureMemoryUsed = 0;
		i32k numTex = format == GImage::Image_ARGB_8888 ? 4 : 3;
		for (i32 i = 0; i < numTex; ++i)
		{
			assert(m_texIDs[i] == -1);
			i32 w = Res(i, (u32) width);
			i32 h = Res(i, (u32) height);
			i32 texId = pRenderer->SF_CreateTexture(w, h, mipmaps + 1, 0, eTF_A8, FT_DONT_STREAM);
			ok = texId > 0;
			if (ok)
			{
				pTextures[i] = pRenderer->EF_GetTextureByID(texId);
				assert(pTextures[i] && !pTextures[i]->IsShared());
				textureMemoryUsed += pTextures[i]->GetDeviceDataSize();
				m_texIDs[i] = texId;
			}
			else
				break;
		}

		if (ok)
		{
			// Link YUVA planes circularly together (Y->U->V->A-> Y...)
			for (i32 i = 0; i < numTex; ++i)
				if (pTextures[i])
					pTextures[(i + (numTex - 1)) % numTex]->SetCustomID(m_texIDs[i]);

			m_width = (u32) width;
			m_height = (u32) height;
			m_numIDs = numTex;
			m_fmt = GImage::Image_ARGB_8888 ? eTF_YUVA : eTF_YUV;
	#if defined(ENABLE_FLASH_INFO)
			DrxInterlockedAdd(&ms_textureMemoryUsed, textureMemoryUsed);
	#endif
			Clear();
		}
		else
		{
			for (i32 i = 0; i < numTex; ++i)
			{
				i32 texId = m_texIDs[i];
				if (texId > 0)
				{
					ITexture* pTexture = pRenderer->EF_GetTextureByID(texId);
					assert(pTexture);
					pRenderer->FlashRemoveTexture(pTexture);
				}
				else
					break;
			}
		}
	}
	else
	{
		assert(0 && "GTextureXRenderYUV::InitDynamicTexture() -- Unsupported usage flag or format!");
	}

	return m_numIDs > 0;
}

void GTextureXRenderYUV::Update(i32 /*level*/, i32 /*n*/, const UpdateRect* /*pRects*/, const GImageBase* /*pIm*/)
{
	assert(0 && "GTextureXRenderYUV::Update() -- Not implemented!");
}

i32 GTextureXRenderYUV::Map(i32 level, i32 n, MapRect* maps, i32 /*flags*/)
{
	assert(m_numIDs > 0 && maps && !level && n >= m_numIDs);
	if (m_numIDs <= 0 || !maps || level > 0 || n < m_numIDs)
		return 0;

	IRenderer* pRenderer(gEnv->pRenderer);
	bool ok = true;
	for (i32 i = 0; i < m_numIDs; ++i)
	{
		uk pBits = 0;
		u32 pitch = m_width;
		ok = !!(pBits = DrxModuleMemalign(m_width * m_height * sizeof(u8), DRX_PLATFORM_ALIGNMENT));
		IF(ok, 1)
		{
			maps[i].width = Res(i, m_width);
			maps[i].height = Res(i, m_height);
			maps[i].pData = (UByte*) pBits;
			maps[i].pitch = pitch;
		}
		else
		{
			memset(&maps[i], 0, sizeof(MapRect) * (m_numIDs - i));
			break;
		}
	}
	return ok ? m_numIDs : 0;
}

bool GTextureXRenderYUV::Unmap(i32 level, i32 n, MapRect* maps, i32 /*flags*/)
{
	assert(m_numIDs > 0);
	if (m_numIDs <= 0)
		return false;

	if (!maps)
		return false;

	#if 0
	// 2x2 pixel checker board pattern to test 1:1 texel to pixel mapping for unscaled videos (i.e. video res = screen res)
	for (i32 i = 0; i < n; ++i)
	{
		for (u32 y = 0; y < maps[i].height; ++y)
		{
			u8* p = maps[i].pData + y * maps[i].pitch;
			if (i == 0)
			{
				for (u32 x = 0; x < maps[i].width; ++x)
					*p++ = (u8) (-(char) ((x + y) & 1));
			}
			else if (i == 1 || i == 2)
			{
				for (u32 x = 0; x < maps[i].width; ++x)
					*p++ = 128;
			}
			else
			{
				for (u32 x = 0; x < maps[i].width; ++x)
					*p++ = 255;
			}
		}
	}
	#endif

	IRenderer::SUpdateRect SrcRect;
	IRenderer* pRenderer(gEnv->pRenderer);
	for (i32 i = 0; i < m_numIDs; ++i)
	{
		IF(maps[i].pData, 1)
		{
			SrcRect.Set(0, 0, 0, 0, maps[i].width, maps[i].height);

			pRenderer->SF_UpdateTexture(m_texIDs[i], level, 1, &SrcRect, maps[i].pData, maps[i].pitch, maps[i].width * maps[i].height * sizeof(u8), eTF_A8);

			DrxModuleMemalignFree(maps[i].pData);
			maps[i].pData = nullptr;
		}
	}
	return true;
}

#endif // #ifdef INCLUDE_SCALEFORM_SDK
