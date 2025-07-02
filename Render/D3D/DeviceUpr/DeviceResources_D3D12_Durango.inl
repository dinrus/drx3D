// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

u32 CDeviceTexture::TextureDataSize(u32 nWidth, u32 nHeight, u32 nDepth, u32 nMips, u32 nSlices, const ETEX_Format eTF, ETEX_TileMode eTM, u32 eFlags)
{
	// Fallback to the default texture size function
	return CTexture::TextureDataSize(nWidth, nHeight, nDepth, nMips, nSlices, eTF, eTM_None);
}
