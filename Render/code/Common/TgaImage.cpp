// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   TgaImage.cpp : TGA image file format implementation.

   Revision история:
* Created by Khonich Andrey

   =============================================================================*/

#include <drx3D/Render/StdAfx.h>

namespace ImageUtils
{
string OutputPath(tukk filename)
{
	string fullPath;
	if (strstr(filename, ":\\") == NULL)
	{
		// construct path with current folder - otherwise path would start at mastercd/game
		char szCurrDir[IDrxPak::g_nMaxPath];
		DrxGetCurrentDirectory(sizeof(szCurrDir), szCurrDir);

		fullPath = string(szCurrDir) + "/" + filename;
	}
	else
	{
		fullPath = filename;
	}
	return fullPath;
}
}

static FILE* sFileData;

static void bwrite(u8 data)
{
	byte d[2];
	d[0] = data;

	gEnv->pDrxPak->FWrite(d, 1, 1, sFileData);
}

void wwrite(unsigned short data)
{
	u8 h, l;

	l = data & 0xFF;
	h = data >> 8;
	bwrite(l);
	bwrite(h);
}

static void WritePixel(i32 depth, u64 a, u64 r, u64 g, u64 b)
{
	DWORD color16;

	switch (depth)
	{
	case 32:
		bwrite((byte)b);        // b
		bwrite((byte)g);        // g
		bwrite((byte)r);        // r
		bwrite((byte)a);        // a
		break;

	case 24:
		bwrite((byte)b);        // b
		bwrite((byte)g);        // g
		bwrite((byte)r);        // r
		break;

	case 16:
		r >>= 3;
		g >>= 3;
		b >>= 3;

		r &= 0x1F;
		g &= 0x1F;
		b &= 0x1F;

		color16 = (r << 10) | (g << 5) | b;

		wwrite((unsigned short)color16);
		break;
	}
}

static void GetPixel(u8* data, i32 depth, u64& a, u64& r, u64& g, u64& b)
{
	switch (depth)
	{
	case 32:
		r = *data++;
		g = *data++;
		b = *data++;
		a = *data++;
		break;

	case 24:
		r = *data++;
		g = *data++;
		b = *data++;
		a = 0xFF;
		break;

	default:
		assert(0);
		break;
	}
}

bool WriteTGA(byte* data, i32 width, i32 height, tukk filename, i32 src_bits_per_pixel, i32 dest_bits_per_pixel)
{
#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	return false;
#else
	i32 i;
	u64 r, g, b, a;

	string fullPath = ImageUtils::OutputPath(filename);

	if ((sFileData = gEnv->pDrxPak->FOpen(fullPath.c_str(), "wb")) == NULL)
		return false;

	//mdesc |= LR;   // left right
	//m_desc |= UL_TGA_BT;   // top

	i32 id_length = 0;
	i32 x_org = 0;
	i32 y_org = 0;
	i32 desc = 0x20; // origin is upper left

	// 32 bpp

	i32 cm_index = 0;
	i32 cm_length = 0;
	i32 cm_entry_size = 0;
	i32 color_map_type = 0;

	i32 type = 2;

	bwrite(id_length);
	bwrite(color_map_type);
	bwrite(type);

	wwrite(cm_index);
	wwrite(cm_length);

	bwrite(cm_entry_size);

	wwrite(x_org);
	wwrite(y_org);
	wwrite((unsigned short) width);
	wwrite((unsigned short) height);

	bwrite(dest_bits_per_pixel);

	bwrite(desc);

	i32 hxw = height * width;

	i32 right = 0;
	//  i32 top   = 1;

	DWORD* temp_dp = (DWORD*) data;     // data = input pointer

	DWORD* swap = 0;

	UINT src_bytes_per_pixel = src_bits_per_pixel / 8;

	UINT size_in_bytes = hxw * src_bytes_per_pixel;

	if (src_bits_per_pixel == dest_bits_per_pixel)
	{
	#if defined(NEED_ENDIAN_SWAP)
		byte* d = new byte[hxw * 4];
		memcpy(d, data, hxw * 4);
		SwapEndian((u32*)d, hxw);
		gEnv->pDrxPak->FWrite(d, hxw, src_bytes_per_pixel, sFileData);
		SAFE_DELETE_ARRAY(d);
	#else
		gEnv->pDrxPak->FWrite(data, hxw, src_bytes_per_pixel, sFileData);
	#endif
	}
	else
	{
		for (i = 0; i < hxw; i++)
		{
			GetPixel(data, src_bits_per_pixel, a, b, g, r);
			WritePixel(dest_bits_per_pixel, a, b, g, r);
			data += src_bytes_per_pixel;
		}
	}

	gEnv->pDrxPak->FClose(sFileData);

	SAFE_DELETE_ARRAY(swap);
	return true;
#endif
}
