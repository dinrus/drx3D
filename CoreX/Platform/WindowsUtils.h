// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#if DRX_PLATFORM_WINDOWS
	#include "DrxWindows.h"
	#include <drx3D/CoreX/Renderer/IRenderer.h>
	#include <drx3D/CoreX/Renderer/IImage.h>
	#include <drx3D/CoreX/smartptr.h>
	#include <drx3D/Eng3D/ImageExtensionHelper.h>

//! Types supported by CreateResourceFromTexture.
enum EResourceType
{
	eResourceType_IconBig,
	eResourceType_IconSmall,
	eResourceType_Cursor,
};

//! Loads a (DDS) texture resource from the renderer into a Win32 GDI resource.
//! \return HICON that must be closed with DestroyIcon(), or NULL on failure.
static HICON CreateResourceFromTexture(IRenderer* pRenderer, tukk path, EResourceType type)
{
	if (!pRenderer || !path || !*path)
	{
		// Invalid parameter passed
		return NULL;
	}

	// Find the target dimensions of the GDI resource
	i32k nRequestedWidth = GetSystemMetrics(type == eResourceType_IconBig ? SM_CXICON : type == eResourceType_IconSmall ? SM_CXSMICON : SM_CXCURSOR);
	i32k nRequestedHeight = GetSystemMetrics(type == eResourceType_IconBig ? SM_CYICON : type == eResourceType_IconSmall ? SM_CYSMICON : SM_CYCURSOR);
	if (nRequestedWidth != nRequestedHeight || nRequestedHeight <= 0 || nRequestedWidth <= 0)
	{
		// Don't support non-squares icons or cursors
		return NULL;
	}

	// Load texture
	_smart_ptr<IImageFile> pImage = pRenderer->EF_LoadImage(path, 0);
	if (!pImage || pImage->mfGet_depth() != 1 || pImage->mfGet_NumSides() != 1 || pImage->mfGetTileMode() != eTM_None)
	{
		// Can't load texture, or texture is a cube-map or volume texture, or uses tiling of some kind
		return NULL;
	}
	byte* pImgRaw = pImage->mfGet_image(0);
	const ETEX_Format texFormat = pImage->mfGetFormat();

	// Pick first mip level smaller than the GDI resource
	i32 nMip = 0;
	size_t offset = 0;
	i32 nMipWidth = pImage->mfGet_width();
	i32 nMipHeight = pImage->mfGet_height();
	while (nMipWidth > nRequestedWidth)
	{
		++nMip;
		offset += pRenderer->GetTextureFormatDataSize(nMipWidth, nMipHeight, 1, 1, texFormat);
		nMipWidth = max(nMipWidth / 2, 1);
		nMipHeight = max(nMipHeight / 2, 1);
	}
	pImgRaw += offset;
	if (nMip >= pImage->mfGet_numMips())
	{
		// No appropriate mip in the texture
		// Note: Consider creating a full mip-chain on the texture so this can't happen
		return NULL;
	}
	const size_t nRawSize = pRenderer->GetTextureFormatDataSize(nMipWidth, nMipHeight, 1, 1, texFormat);

	#if 0
	// Check that DDS indexing is correct here
	offset += nRawSize;
	for (i32 nIdx = nMip + 1; nIdx < pImage->mfGet_numMips(); ++nIdx)
	{
		i32k w = max(nMipWidth >> (nIdx - nMip), 1);
		i32k h = max(nMipHeight >> (nIdx - nMip), 1);
		offset += pRenderer->GetTextureFormatDataSize(w, h, 1, 1, texFormat);
	}
	assert(offset == pImage->mfGet_ImageSize());
	#endif

	// Type of bitmap to create
	BITMAPV5HEADER bi = { 0 };
	bi.bV5Size = sizeof(BITMAPV5HEADER);
	bi.bV5Width = nRequestedWidth;
	bi.bV5Height = -nRequestedHeight;
	bi.bV5Planes = 1;
	bi.bV5BitCount = 32;
	bi.bV5Compression = BI_BITFIELDS;
	// The following mask specification specifies a supported 32 BPP alpha format for Windows XP+
	bi.bV5AlphaMask = 0xFF000000;
	bi.bV5RedMask = 0x00FF0000;
	bi.bV5GreenMask = 0x0000FF00;
	bi.bV5BlueMask = 0x000000FF;

	// Create the DIB section with an alpha channel
	const HDC hdc = GetDC(NULL);
	uk lpBits;
	const HBITMAP hBitmap = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &lpBits, NULL, 0);
	ReleaseDC(NULL, hdc);
	if (!hBitmap || !lpBits)
	{
		// Can't allocate OS bitmap
		return NULL;
	}
	GdiFlush();

	// Decompress texture
	const bool bTryDecompress = texFormat != eTF_R8G8B8A8 && texFormat != eTF_B8G8R8A8 && texFormat != eTF_B8G8R8X8;
	byte* const pImgDecomp = bTryDecompress ? new byte[nMipWidth * nMipHeight * sizeof(i32)] : pImgRaw;
	const bool bImgDecompressed = bTryDecompress && pRenderer->DXTDecompress(pImgRaw, nRawSize, pImgDecomp, nMipWidth, nMipHeight, 1, texFormat, false, sizeof(i32));

	// Check which conversions need to be performed
	const bool bSRGB = (pImage->mfGet_Flags() & FIM_SRGB_READ) != 0;
	const bool bSwap = (texFormat == eTF_R8G8B8A8) || bTryDecompress;

	HICON result = NULL;
	if (!bTryDecompress || bImgDecompressed)
	{
		// Assign texture data with mismatching sizes
		// Note: Any pixels not in the selected mip will become 100% transparent black, no resizing is performed
		const size_t sourceRowStride = nMipWidth;
		const size_t targetRowStride = nRequestedWidth;
		u32k* pSourceRow = (u32*)pImgDecomp;
		u32* pTargetRow = (u32*)lpBits;

		assert(sourceRowStride <= targetRowStride);
		assert(nMipHeight <= nRequestedHeight);
		assert(pSourceRow != pTargetRow);
		assert(pSourceRow && pTargetRow);

		u8 gammaTable[256];
		if (!bSRGB)
		{
			// Linear to sRGB table
			const float gamma = 1.0f / 2.2f;
			const float gammaMultiplier = 255.0f / powf(255.0f, gamma);
			for (i32 i = 0; i < 256; ++i)
			{
				gammaTable[i] = u8(powf(float(i), gamma) * gammaMultiplier);
			}
		}
		else if (bSwap)
		{
			// sRGB to sRGB table (no change)
			for (i32 i = 0; i < 256; ++i)
			{
				gammaTable[i] = u8(i);
			}
		}

		for (i32 y = 0; y < nMipHeight; ++y, pSourceRow += sourceRowStride, pTargetRow += targetRowStride)
		{
			if (!bSwap && bSRGB)
			{
				memcpy(pTargetRow, pSourceRow, sourceRowStride * sizeof(u32));
			}
			else
			{
				u32k* pSourceTexel = pSourceRow;
				u32* pTargetTexel = pTargetRow;
				for (i32 x = 0; x < nMipWidth; ++x, ++pSourceTexel, ++pTargetTexel)
				{
					u8k red = gammaTable[u8(*pSourceTexel >> 0)];
					u8k green = gammaTable[u8(*pSourceTexel >> 8)];
					u8k blue = gammaTable[u8(*pSourceTexel >> 16)];
					u32k alpha = *pSourceTexel & 0xFF000000;
					*pTargetTexel = (red << (bSwap ? 16 : 0)) | (green << 8) | (blue << (bSwap ? 0 : 16)) | alpha;
				}
			}

			// Fill remaining columns with zeros
			memset(pTargetRow + sourceRowStride, 0, (targetRowStride - sourceRowStride) * sizeof(u32));
		}

		// Fill remaining rows with zeros
		for (i32 y = nMipHeight; y < nRequestedHeight; ++y)
		{
			memset(pTargetRow, 0, targetRowStride * sizeof(u32));
			pTargetRow += targetRowStride;
		}

		// Convert to GDI icon
		ICONINFO iconinfo = { 0 };
		iconinfo.fIcon = type == eResourceType_Cursor ? FALSE : TRUE;
		iconinfo.hbmMask = ::CreateBitmap(nRequestedWidth, nRequestedHeight, 1, 1, NULL);
		iconinfo.hbmColor = hBitmap;
		result = ::CreateIconIndirect(&iconinfo);
		DeleteObject(iconinfo.hbmMask);
	}

	// Clean up
	DeleteObject(hBitmap);
	if (bTryDecompress)
	{
		delete[] pImgDecomp;
	}
	pImage.reset();

	return result;
}

#endif
