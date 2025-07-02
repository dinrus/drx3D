// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   BMPHelper.cpp
//  Version:     v1.00
//  Created:     28/11/2006 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: BMPHelper
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Act/StdAfx.h>
#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/Act/BMPHelper.h>
#include <drx3D/Act/RichSaveGameTypes.h>

#if defined(NEED_ENDIAN_SWAP)
// big endian
static const unsigned short BF_TYPE = 0x424D;            /* 0x42 == 'B', 0x4D == 'M' */
#else
// little endian
static const unsigned short BF_TYPE = 0x4D42;            /* 0x42 == 'B', 0x4D == 'M' */
#endif

#define COMPRESSION_BI_RGB 0

namespace BMPHelper
{
size_t CalcBMPSize(i32 width, i32 height, i32 depth)
{
	i32 bytesPerLine = width * depth;
	if (bytesPerLine & 0x0003)
	{
		bytesPerLine |= 0x0003;
		++bytesPerLine;
	}
	size_t totalSize = bytesPerLine * height + sizeof(DrxBitmapFileHeader) + sizeof(DrxBitmapInfoHeader);
	return totalSize;
}

// pByteData: B G R A
// if pFile != 0 writes into file. otherwise opens a new file with filename 'filename'
// returns size of written bytes in outFileSize
// can be optimized quite a bit
bool InternalSaveBMP(tukk filename, FILE* pFile, u8* pByteData, i32 width, i32 height, i32 depth, bool inverseY)
{
	DrxBitmapFileHeader bitmapfileheader;
	DrxBitmapInfoHeader bitmapinfoheader;

	IDrxPak* pDrxPak = gEnv->pDrxPak;
	const bool bFileGiven = pFile != 0;
	if (bFileGiven == false)
		pFile = pDrxPak->FOpen(filename, "wb");
	if (pFile == 0)
		return false;

	i32 origBytesPerLine = width * depth;
	i32 bytesPerLine = origBytesPerLine;
	if (bytesPerLine & 0x0003)
	{
		bytesPerLine |= 0x0003;
		++bytesPerLine;
	}
	i32 padBytes = bytesPerLine - origBytesPerLine;   // how many pad-bytes

	u8* pTempImage = new u8[bytesPerLine * height]; // optimize and write one line only

	for (i32 y = 0; y < height; y++)
	{
		i32 src_y = y;
		if (inverseY)
			src_y = (height - 1) - y;

		u8* pDstLine = pTempImage + y * bytesPerLine;
		u8* pSrcLine = pByteData + src_y * origBytesPerLine;
		if (depth == 3)
		{
			u8 r, g, b;
			for (i32 x = 0; x < width; x++)
			{
				b = *pSrcLine++;
				g = *pSrcLine++;
				r = *pSrcLine++;
				*pDstLine++ = b;
				*pDstLine++ = g;
				*pDstLine++ = r;
			}
		}
		else
		{
			u8 r, g, b, a;
			for (i32 x = 0; x < width; x++)
			{
				b = *pSrcLine++;
				g = *pSrcLine++;
				r = *pSrcLine++;
				a = *pSrcLine++;
				*pDstLine++ = b;
				*pDstLine++ = g;
				*pDstLine++ = r;
				*pDstLine++ = a;
			}
		}
		for (i32 n = 0; n < padBytes; ++n)
		{
			*pDstLine++ = 0;
		}
	}

	// Fill in bitmap structures
	bitmapfileheader.bfType = BF_TYPE;
	bitmapfileheader.bfSize = bytesPerLine * height + sizeof(DrxBitmapFileHeader) + sizeof(DrxBitmapInfoHeader);
	bitmapfileheader.bfReserved1 = 0;
	bitmapfileheader.bfReserved2 = 0;
	bitmapfileheader.bfOffBits = sizeof(DrxBitmapFileHeader) + sizeof(DrxBitmapInfoHeader);
	bitmapinfoheader.biSize = sizeof(DrxBitmapInfoHeader);
	bitmapinfoheader.biWidth = width;
	bitmapinfoheader.biHeight = height;
	bitmapinfoheader.biPlanes = 1;
	bitmapinfoheader.biBitCount = depth * 8;
	bitmapinfoheader.biCompression = COMPRESSION_BI_RGB;
	bitmapinfoheader.biSizeImage = 0;
	bitmapinfoheader.biXPelsPerMeter = 0;
	bitmapinfoheader.biYPelsPerMeter = 0;
	bitmapinfoheader.biClrUsed = 0;
	bitmapinfoheader.biClrImportant = 0;

	pDrxPak->FWrite(&bitmapfileheader, 1, pFile);
	pDrxPak->FWrite(&bitmapinfoheader, 1, pFile);
	pDrxPak->FWrite(pTempImage, bytesPerLine * height, 1, pFile);

	if (bFileGiven == false)
		pDrxPak->FClose(pFile);

	delete[] pTempImage;

	// Success
	return true;

}

// fills data with BGR or BGRA
// always top->bottom
bool InternalLoadBMP(tukk filename, FILE* pFile, u8* pByteData, i32& width, i32& height, i32& depth, bool bForceInverseY)
{
	// todo: cleanup for pFile
	struct Cleanup
	{
		Cleanup() : m_pFile(0), m_bCloseFile(false), m_bRestoreCur(false), m_offset(0) {}
		~Cleanup()
		{
			if (m_pFile)
			{
				if (m_bRestoreCur)
					gEnv->pDrxPak->FSeek(m_pFile, m_offset, SEEK_SET);
				if (m_bCloseFile)
					gEnv->pDrxPak->FClose(m_pFile);
			}
		}
		void SetFile(FILE* pFile, bool bClose, bool bRestoreCur)
		{
			m_pFile = pFile;
			m_bCloseFile = bClose;
			m_bRestoreCur = bRestoreCur;
			if (m_pFile && m_bRestoreCur)
				m_offset = gEnv->pDrxPak->FTell(m_pFile);
		}
	protected:
		FILE* m_pFile;
		bool  m_bCloseFile;
		bool  m_bRestoreCur;
		long  m_offset;
	};

	Cleanup cleanup;   // potentially close the file on return (if pFile was given, do nothing)
	DrxBitmapFileHeader bitmapfileheader;
	DrxBitmapInfoHeader bitmapinfoheader;

	memset(&bitmapfileheader, 0, sizeof(DrxBitmapFileHeader));
	memset(&bitmapinfoheader, 0, sizeof(DrxBitmapInfoHeader));

	IDrxPak* pDrxPak = gEnv->pDrxPak;
	const bool bFileGiven = pFile != 0;
	if (bFileGiven == false)
		pFile = pDrxPak->FOpen(filename, "rb");
	if (pFile == 0)
		return false;

	cleanup.SetFile(pFile, !bFileGiven, bFileGiven);

	size_t els = pDrxPak->FRead(&bitmapfileheader, 1, pFile);
	if (els != 1)
		return false;

	els = pDrxPak->FRead(&bitmapinfoheader, 1, pFile);
	if (els != 1)
		return false;

	if (bitmapfileheader.bfType != BF_TYPE)
		return false;

	if (bitmapinfoheader.biCompression != COMPRESSION_BI_RGB)
		return false;

	if (bitmapinfoheader.biBitCount != 24 && bitmapinfoheader.biBitCount != 32)
		return false;

	if (bitmapinfoheader.biPlanes != 1)
		return false;

	i32 imgWidth = bitmapinfoheader.biWidth;
	i32 imgHeight = bitmapinfoheader.biHeight;
	i32 imgDepth = bitmapinfoheader.biBitCount == 24 ? 3 : 4;

	i32 imgBytesPerLine = imgWidth * imgDepth;
	i32 fileBytesPerLine = imgBytesPerLine;
	if (fileBytesPerLine & 0x0003)
	{
		fileBytesPerLine |= 0x0003;
		++fileBytesPerLine;
	}
	size_t prologSize = sizeof(DrxBitmapFileHeader) + sizeof(DrxBitmapInfoHeader);
	size_t bufSize = bitmapfileheader.bfSize - prologSize;

	// some BMPs from Adobe PhotoShop seem 2 bytes longer
	if (bufSize != fileBytesPerLine * imgHeight && bufSize != (fileBytesPerLine * imgHeight) + 2)
		return false;

	bool bFlipY = true;
	width = imgWidth;
	height = imgHeight;
	if (height < 0)     // height > 0: bottom->top, height < 0: top->bottom, no flip necessary
	{
		height = -height;
		bFlipY = false;
	}
	depth = imgDepth;

	if (pByteData == 0)   // only requested dimensions
		return true;

	u8* pTempImage = new u8[bufSize];
	if (pTempImage == 0)
		return false;

	els = pDrxPak->FReadRaw(pTempImage, 1, bufSize, pFile);
	if (els != bufSize)
	{
		delete[] pTempImage;
		return false;
	}

	if (bForceInverseY)
		bFlipY = !bFlipY;

	// read all rows
	u8* pSrcLine = pTempImage;
	for (i32 y = 0; y < imgHeight; ++y)
	{
		i32 dstY = y;
		if (bFlipY)
			dstY = imgHeight - y - 1;

		u8* pDstLine = pByteData + dstY * imgBytesPerLine;
		memcpy(pDstLine, pSrcLine, imgBytesPerLine);
		pSrcLine += fileBytesPerLine;
	}

	delete[] pTempImage;

	// Success
	return true;
}

bool LoadBMP(tukk filename, u8* pByteData, i32& width, i32& height, i32& depth, bool bForceInverseY)
{
	return InternalLoadBMP(filename, 0, pByteData, width, height, depth, bForceInverseY);
}

bool LoadBMP(FILE* pFile, u8* pByteData, i32& width, i32& height, i32& depth, bool bForceInverseY)
{
	return InternalLoadBMP("", pFile, pByteData, width, height, depth, bForceInverseY);
}

bool SaveBMP(tukk filename, u8* pByteData, i32 width, i32 height, i32 depth, bool flipY)
{
	return InternalSaveBMP(filename, 0, pByteData, width, height, depth, flipY);
}

bool SaveBMP(FILE* pFile, u8* pByteData, i32 width, i32 height, i32 depth, bool flipY)
{
	return InternalSaveBMP("", pFile, pByteData, width, height, depth, flipY);
}
};
