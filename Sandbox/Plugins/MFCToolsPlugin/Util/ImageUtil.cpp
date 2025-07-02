// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "ImageUtil.h"
#include "ImageGif.h"
#include "ImageTIF.h"
#include "ImageHDR.h"
#include "Image_DXTC.h"
#include <DrxSystem/File/DrxFile.h>
#include "GdiUtil.h"
#include "FilePathUtil.h"

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::SaveBitmap(const string& szFileName, CImageEx& inImage, bool inverseY)
{
	////////////////////////////////////////////////////////////////////////
	// Simple DIB save code
	////////////////////////////////////////////////////////////////////////

	HANDLE hfile;
	DWORD dwBytes;
	u32 i;
	DWORD* pLine1 = NULL;
	DWORD* pLine2 = NULL;
	DWORD* pTemp = NULL;
	BITMAPFILEHEADER bitmapfileheader;
	BITMAPINFOHEADER bitmapinfoheader;

	DrxLog("Saving data to bitmap... %s", (tukk)szFileName);

	i32 dwWidth = inImage.GetWidth();
	i32 dwHeight = inImage.GetHeight();
	DWORD* pData = (DWORD*)inImage.GetData();

	i32 iAddToPitch = (dwWidth * 3) % 4;
	if (iAddToPitch)
		iAddToPitch = 4 - iAddToPitch;

	u8* pImage = new u8[(dwWidth * 3 + iAddToPitch) * dwHeight];

	i = 0;
	for (i32 y = 0; y < dwHeight; y++)
	{
		i32 src_y = y;

		if (inverseY)
			src_y = (dwHeight - 1) - y;

		for (i32 x = 0; x < dwWidth; x++)
		{
			DWORD c = pData[x + src_y * dwWidth];
			pImage[i] = GetBValue(c);
			pImage[i + 1] = GetGValue(c);
			pImage[i + 2] = GetRValue(c);
			i += 3;
		}

		for (i32 j = 0; j < iAddToPitch; j++)
			pImage[i++] = 0;
	}

	// Fill in bitmap structures
	bitmapfileheader.bfType = 0x4D42;
	bitmapfileheader.bfSize = (dwWidth * dwHeight * 3) + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bitmapfileheader.bfReserved1 = 0;
	bitmapfileheader.bfReserved2 = 0;
	bitmapfileheader.bfOffBits = sizeof(BITMAPFILEHEADER) +
	                             sizeof(BITMAPINFOHEADER) + (0 * sizeof(RGBQUAD));
	bitmapinfoheader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfoheader.biWidth = dwWidth;
	bitmapinfoheader.biHeight = dwHeight;
	bitmapinfoheader.biPlanes = 1;
	bitmapinfoheader.biBitCount = (WORD) 24;
	bitmapinfoheader.biCompression = BI_RGB;
	bitmapinfoheader.biSizeImage = 0;
	bitmapinfoheader.biXPelsPerMeter = 0;
	bitmapinfoheader.biYPelsPerMeter = 0;
	bitmapinfoheader.biClrUsed = 0;
	bitmapinfoheader.biClrImportant = 0;

	// Write bitmap to disk
	hfile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hfile == INVALID_HANDLE_VALUE)
	{
		delete[]pImage;
		return false;
	}

	// Write the headers to the file
	WriteFile(hfile, &bitmapfileheader, sizeof(BITMAPFILEHEADER), &dwBytes, NULL);
	WriteFile(hfile, &bitmapinfoheader, sizeof(BITMAPINFOHEADER), &dwBytes, NULL);

	// Write the data
	DWORD written;
	WriteFile(hfile, pImage, ((dwWidth * 3 + iAddToPitch) * dwHeight), &written, NULL);

	CloseHandle(hfile);

	delete[]pImage;

	// Success
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::Save(const string& strFileName, CImageEx& inImage)
{
	CAlphaBitmap imgBitmap;

	if (imgBitmap.Create((uk )inImage.GetData(), inImage.GetWidth(), inImage.GetHeight()) == false)
		return false;

	CreateBitmapFromImage(inImage, imgBitmap.GetBitmap());

	return imgBitmap.Save(strFileName);
}

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::SaveJPEG(const string& strFileName, CImageEx& inImage)
{
	return Save(strFileName, inImage);
}

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::LoadImageWithGDIPlus(const string& fileName, CImageEx& image)
{
	CAlphaBitmap imgBitmap;

	if (imgBitmap.Load(fileName) == false)
		return false;

	BITMAP bitmap;
	if (imgBitmap.GetBitmap().GetBitmap(&bitmap) == 0)
		return false;

	return CImageUtil::FillFromBITMAPObj(&bitmap, image);
}

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::LoadJPEG(const string& strFileName, CImageEx& outImage)
{
	return CImageUtil::LoadImageWithGDIPlus(strFileName, outImage);
}

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::SavePGM(const string& fileName, u32 dwWidth, u32 dwHeight, u32* pData)
{
	FILE* file = fopen(fileName, "wt");
	if (!file)
		return false;

	fprintf(file, "P2\n");
	fprintf(file, "%d %d\n", dwWidth, dwHeight);
	fprintf(file, "65535\n");
	for (i32 y = 0; y < dwHeight; y++)
	{
		for (i32 x = 0; x < dwWidth; x++)
		{
			fprintf(file, "%d ", (u32)pData[x + y * dwWidth]);
		}
		fprintf(file, "\n");
	}

	fclose(file);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::LoadPGM(const string& fileName, u32* pWidthOut, u32* pHeightOut, u32** pImageDataOut)
{
	FILE* file = fopen(fileName, "rt");
	if (!file)
		return false;

	const char seps[] = " \n\t";
	tuk token;

	i32 width = 0;
	i32 height = 0;
	i32 numColors = 1;

	fseek(file, 0, SEEK_END);
	i32 fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	tuk str = new char[fileSize];
	fread(str, fileSize, 1, file);

	token = strtok(str, seps);

	while (token != NULL && token[0] == '#')
	{
		if (token != NULL && token[0] == '#')
			strtok(NULL, "\n");
		token = strtok(NULL, seps);
	}
	if (stricmp(token, "P2") != 0)
	{
		// Bad file. not supported pgm.
		delete[]str;
		fclose(file);
		return false;
	}

	do
	{
		token = strtok(NULL, seps);
		if (token != NULL && token[0] == '#')
		{
			strtok(NULL, "\n");
		}
	}
	while (token != NULL && token[0] == '#');
	width = atoi(token);

	do
	{
		token = strtok(NULL, seps);
		if (token != NULL && token[0] == '#')
			strtok(NULL, "\n");
	}
	while (token != NULL && token[0] == '#');
	height = atoi(token);

	do
	{
		token = strtok(NULL, seps);
		if (token != NULL && token[0] == '#')
			strtok(NULL, "\n");
	}
	while (token != NULL && token[0] == '#');
	numColors = atoi(token);

	*pWidthOut = width;
	*pHeightOut = height;

	u32* pImage = new u32[width * height];
	*pImageDataOut = pImage;

	u32* p = pImage;
	i32 size = width * height;
	i32 i = 0;
	while (token != NULL && i < size)
	{
		do
		{
			token = strtok(NULL, seps);
		}
		while (token != NULL && token[0] == '#');
		*p++ = atoi(token);
		i++;
	}

	delete[]str;

	fclose(file);
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static inline u16 us_endian(const byte* ptr)
{
	short n;
	memcpy(&n, ptr, sizeof(n));
	return n;
}

static inline u64 ul_endian(const byte* ptr)
{
	long n;
	memcpy(&n, ptr, sizeof(n));
	return n;
}

static inline long l_endian(const byte* ptr)
{
	long n;
	memcpy(&n, ptr, sizeof(n));
	return n;
}

#define BFTYPE(x)    us_endian((x) + 0)
#define BFSIZE(x)    ul_endian((x) + 2)
#define BFOFFBITS(x) ul_endian((x) + 10)
#define BISIZE(x)    ul_endian((x) + 14)
#define BIWIDTH(x)   l_endian((x) + 18)
#define BIHEIGHT(x)  l_endian((x) + 22)
#define BITCOUNT(x)  us_endian((x) + 28)
#define BICOMP(x)    ul_endian((x) + 30)
#define IMAGESIZE(x) ul_endian((x) + 34)
#define BICLRUSED(x) ul_endian((x) + 46)
#define BICLRIMP(x)  ul_endian((x) + 50)
#define BIPALETTE(x) ((x) + 54)

// Type ID
#define BM "BM" // Windows 3.1x, 95, NT, ...
#define BA "BA" // OS/2 Bitmap Array
#define CI "CI" // OS/2 Color Icon
#define CP "CP" // OS/2 Color Pointer
#define IC "IC" // OS/2 Icon
#define PT "PT" // OS/2 Pointer

// Possible values for the header size
#define WinHSize   0x28
#define OS21xHSize 0x0C
#define OS22xHSize 0xF0

// Possible values for the BPP setting
#define Monochrome  1    // Monochrome bitmap
#define _16Color    4    // 16 color bitmap
#define _256Color   8    // 256 color bitmap
#define HIGHCOLOR   16   // 16bit (high color) bitmap
#define TRUECOLOR24 24   // 24bit (true color) bitmap
#define TRUECOLOR32 32   // 32bit (true color) bitmap

// Compression Types
#ifndef BI_RGB
	#define BI_RGB       0 // none
	#define BI_RLE8      1 // RLE 8-bit / pixel
	#define BI_RLE4      2 // RLE 4-bit / pixel
	#define BI_BITFIELDS 3 // Bitfields
#endif

#pragma pack(push,1)
struct SRGBcolor
{
	u8 red, green, blue;
};
struct SRGBPixel
{
	u8 red, green, blue, alpha;
};
#pragma pack(pop)

//===========================================================================
bool CImageUtil::LoadBmp(const string& fileName, CImageEx& image)
{
	std::vector<u8> data;

	CDrxFile file;
	if (!file.Open(fileName, "rb"))
	{
		DrxLog("File not found %s", (tukk)fileName);
		return false;
	}

	long iSize = file.GetLength();

	data.resize(iSize);
	u8* iBuffer = &data[0];
	file.ReadRaw(iBuffer, iSize);

	if (!((memcmp(iBuffer, BM, 2) == 0) && BISIZE(iBuffer) == WinHSize))
	{
		// Not bmp file.
		DrxLog("Invalid BMP file format %s", (tukk)fileName);
		return false;
	}

	i32 mWidth = BIWIDTH(iBuffer);
	i32 mHeight = BIHEIGHT(iBuffer);
	image.Allocate(mWidth, mHeight);
	i32k bmp_size = mWidth * mHeight;

	byte* iPtr = iBuffer + BFOFFBITS(iBuffer);

	// The last scanline in BMP corresponds to the top line in the image
	i32 buffer_y = mWidth * (mHeight - 1);
	bool blip = false;

	if (BITCOUNT(iBuffer) == _256Color)
	{
		//mpIndexImage = mfGet_IndexImage();
		byte* buffer = new byte[mWidth * mHeight];
		SRGBcolor mspPal[256];
		SRGBcolor* pwork = mspPal;
		byte* inpal = BIPALETTE(iBuffer);

		for (i32 color = 0; color < 256; color++, pwork++)
		{
			// Whacky BMP palette is in BGR order.
			pwork->blue = *inpal++;
			pwork->green = *inpal++;
			pwork->red = *inpal++;
			inpal++; // Skip unused byte.
		}

		if (BICOMP(iBuffer) == BI_RGB)
		{
			// Read the pixels from "top" to "bottom"
			while (iPtr < iBuffer + iSize && buffer_y >= 0)
			{
				memcpy(buffer + buffer_y, iPtr, mWidth);
				iPtr += mWidth;
				buffer_y -= mWidth;
			} /* endwhile */
		}
		else if (BICOMP(iBuffer) == BI_RLE8)
		{
			// Decompress pixel data
			byte rl, rl1, i;      // runlength
			byte clridx, clridx1; // colorindex
			i32 buffer_x = 0;
			while (iPtr < iBuffer + iSize && buffer_y >= 0)
			{
				rl = rl1 = *iPtr++;
				clridx = clridx1 = *iPtr++;
				if (rl == 0)
					if (clridx == 0)
					{
						// new scanline
						if (!blip)
						{
							// if we didnt already jumped to the new line, do it now
							buffer_x = 0;
							buffer_y -= mWidth;
						}
						continue;
					}
					else if (clridx == 1)
						// end of bitmap
						break;
					else if (clridx == 2)
					{
						// next 2 bytes mean column- and scanline- offset
						buffer_x += *iPtr++;
						buffer_y -= (mWidth * (*iPtr++));
						continue;
					}
					else if (clridx > 2)
						rl1 = clridx;

				for (i = 0; i < rl1; i++)
				{
					if (!rl)
						clridx1 = *iPtr++;
					buffer[buffer_y + buffer_x] = clridx1;

					if (++buffer_x >= mWidth)
					{
						buffer_x = 0;
						buffer_y -= mWidth;
						blip = true;
					}
					else
						blip = false;
				}
				// pad in case rl == 0 and clridx in [3..255]
				if (rl == 0 && (clridx & 0x01))
					iPtr++;
			}
		}

		// Convert indexed to RGBA
		for (i32 y = 0; y < mHeight; y++)
		{
			for (i32 x = 0; x < mWidth; x++)
			{
				SRGBcolor& entry = mspPal[buffer[x + y * mWidth]];
				image.ValueAt(x, y) = 0xFF000000 | RGB(entry.red, entry.green, entry.blue);
			}
		}

		delete[]buffer;
		return true;
	}
	else if (!BICLRUSED(iBuffer) && BITCOUNT(iBuffer) == TRUECOLOR24)
	{
		i32 iAddToPitch = (mWidth * 3) % 4;
		if (iAddToPitch)
			iAddToPitch = 4 - iAddToPitch;

		SRGBPixel* buffer = (SRGBPixel*)image.GetData();

		while (iPtr < iBuffer + iSize && buffer_y >= 0)
		{
			SRGBPixel* d = buffer + buffer_y;
			for (i32 x = mWidth; x; x--)
			{
				d->blue = *iPtr++;
				d->green = *iPtr++;
				d->red = *iPtr++;
				d->alpha = 255;
				d++;
			} /* endfor */

			iPtr += iAddToPitch;

			buffer_y -= mWidth;
		}
		return true;
	}
	else if (!BICLRUSED(iBuffer) && BITCOUNT(iBuffer) == TRUECOLOR32)
	{
		SRGBPixel* buffer = (SRGBPixel*)image.GetData();

		while (iPtr < iBuffer + iSize && buffer_y >= 0)
		{
			SRGBPixel* d = buffer + buffer_y;
			for (i32 x = mWidth; x; x--)
			{
				d->blue = *iPtr++;
				d->green = *iPtr++;
				d->red = *iPtr++;
				d->alpha = *iPtr++;
				d++;
			} /* endfor */

			buffer_y -= mWidth;
		}
		return true;
	}

	DrxLog("Unknown BMP image format %s", (tukk)fileName);

	return false;
}

//===========================================================================
bool CImageUtil::LoadBmp(const string& fileName, CImageEx& image, const RECT& rc)
{
#pragma pack(push,1)
	struct SRGBcolor
	{
		u8 red, green, blue;
	};
	struct SRGBPixel
	{
		u8 red, green, blue, alpha;
	};
#pragma pack(pop)

	std::vector<u8> header;

	CDrxFile file;
	if (!file.Open(fileName, "rb"))
	{
		DrxLog("File not found %s", (tukk)fileName);
		return false;
	}

	long iSizeHeader = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	header.resize(iSizeHeader);

	//u8* iBuffer = &data[0];
	u8* iBuffer = &header[0];

	file.ReadRaw(iBuffer, iSizeHeader);

	if (!((memcmp(iBuffer, BM, 2) == 0) && BISIZE(iBuffer) == WinHSize))
	{
		// Not bmp file.
		DrxLog("Invalid BMP file format %s", (tukk)fileName);
		return false;
	}

	i32 mWidth = BIWIDTH(iBuffer);
	i32 mHeight = BIHEIGHT(iBuffer);
	i32k bmp_size = mWidth * mHeight;

	long offset = BFOFFBITS(iBuffer) - iSizeHeader;
	if (offset > 0)
	{
		std::vector<u8> data;
		data.resize(offset);
		file.ReadRaw(&data[0], offset);
	}

	i32 w = rc.right - rc.left;
	i32 h = rc.bottom - rc.top;

	std::vector<u8> data;

	i32 st = 4;
	if (BITCOUNT(iBuffer) == TRUECOLOR24)
		st = 3;

	i32 iAddToPitch = (mWidth * st) % 4;
	if (iAddToPitch)
		iAddToPitch = 4 - iAddToPitch;

	long iSize = mWidth * st + iAddToPitch;

	data.resize(iSize);

	//for (i32 y = mHeight-1; y > rc.bottom; y--)
	//file.ReadRaw( &data[0], iSize );

	file.Seek(file.GetPosition() + (mHeight - rc.bottom) * iSize, SEEK_SET);

	if (!BICLRUSED(iBuffer) && (BITCOUNT(iBuffer) == TRUECOLOR24 || BITCOUNT(iBuffer) == TRUECOLOR32))
	{
		image.Allocate(w, h);
		SRGBPixel* buffer = (SRGBPixel*)image.GetData();

		for (i32 y = h - 1; y >= 0; y--)
		{
			file.ReadRaw(&data[0], iSize);

			for (i32 x = 0; x < w; x++)
			{
				buffer[x + y * w].blue = data[(x + rc.left) * st];
				buffer[x + y * w].green = data[(x + rc.left) * st + 1];
				buffer[x + y * w].red = data[(x + rc.left) * st + 2];
				if (BITCOUNT(iBuffer) == TRUECOLOR24)
					buffer[x + y * w].alpha = 255;
				else
					buffer[x + y * w].alpha = data[(x + rc.left) * st + 3];
			}
		}
		return true;
	}

	DrxLog("Unknown BMP image format %s", (tukk)fileName);
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::LoadImage(const string& fileName, CImageEx& image, bool* pQualityLoss)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	if (pQualityLoss)
		*pQualityLoss = false;

	_splitpath(fileName, drive, dir, fname, ext);

	// Only DDS has explicit sRGB flag - we'll assume by default all formats are stored in gamma space
	image.SetSRGB(true);

	if (stricmp(ext, ".bmp") == 0)
	{
		return LoadBmp(fileName, image);
	}
	else if (stricmp(ext, ".tif") == 0)
	{
		return CImageTIF().Load(fileName, image);
	}
	else if (stricmp(ext, ".jpg") == 0)
	{
		if (pQualityLoss)
			*pQualityLoss = true;   // we assume JPG has quality loss

		return LoadJPEG(fileName, image);
	}
	else if (stricmp(ext, ".gif") == 0)
	{
		return CImageGif().Load(fileName, image);
	}
	else if (stricmp(ext, ".pgm") == 0)
	{
		UINT w, h;
		u32* pData;
		bool res = LoadPGM(fileName, &w, &h, &pData);
		if (!res)
			return false;
		image.Allocate(w, h);
		memcpy(image.GetData(), pData, image.GetSize());
		delete[]pData;
		return res;
	}
	else if (stricmp(ext, ".dds") == 0)
	{
		return CImage_DXTC().Load(fileName, image, pQualityLoss);
	}
	else if (stricmp(ext, ".png") == 0)
	{
		return CImageUtil::LoadImageWithGDIPlus(fileName, image);
	}
	else if (stricmp(ext, ".hdr") == 0)
	{
		return CImageHDR().Load(fileName, image);
	}

	return false;
}

bool CImageUtil::LoadImage(tukk fileName, CImageEx& image, bool* pQualityLoss)
{
	return LoadImage(string(fileName), image, pQualityLoss);
}

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::SaveImage(const string& fileName, CImageEx& image)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	// Remove the read-only attribute so the file can be overwritten.
	SetFileAttributes(fileName, FILE_ATTRIBUTE_NORMAL);

	_splitpath(fileName, drive, dir, fname, ext);
	if (stricmp(ext, ".bmp") == 0)
	{
		return SaveBitmap(fileName, image);
	}
	else if (stricmp(ext, ".jpg") == 0)
	{
		return SaveJPEG(fileName, image);
	}
	else if (stricmp(ext, ".pgm") == 0)
	{
		return SavePGM(fileName, image.GetWidth(), image.GetHeight(), image.GetData());
	}
	else if (stricmp(ext, ".png") == 0)
	{
		return Save(fileName, image);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CImageUtil::ScaleToFit(const CByteImage& srcImage, CByteImage& trgImage)
{
	u32 x, y, u, v;
	u8* destRow, * dest, * src, * sourceRow;

	u32 srcW = srcImage.GetWidth();
	u32 srcH = srcImage.GetHeight();

	u32 trgW = trgImage.GetWidth();
	u32 trgH = trgImage.GetHeight();

	u32 xratio = (srcW << 16) / trgW;
	u32 yratio = (srcH << 16) / trgH;

	src = srcImage.GetData();
	destRow = trgImage.GetData();

	v = 0;
	for (y = 0; y < trgH; y++)
	{
		u = 0;
		sourceRow = src + (v >> 16) * srcW;
		dest = destRow;
		for (x = 0; x < trgW; x++)
		{
			*dest++ = sourceRow[u >> 16];
			u += xratio;
		}
		v += yratio;
		destRow += trgW;
	}
}

//////////////////////////////////////////////////////////////////////////
void CImageUtil::DownScaleSquareTextureTwice(const CImageEx& srcImage, CImageEx& trgImage, _EAddrMode eAddressingMode)
{
	u32* pSrcData = srcImage.GetData();
	i32 nSrcWidth = srcImage.GetWidth();
	i32 nSrcHeight = srcImage.GetHeight();
	i32 nTrgWidth = srcImage.GetWidth() >> 1;
	i32 nTrgHeight = srcImage.GetHeight() >> 1;

	// reallocate target
	trgImage.Release();
	trgImage.Allocate(nTrgWidth, nTrgHeight);
	u32* pDstData = trgImage.GetData();

	// values in this filter are the log2 of the actual multiplicative values .. see DXCFILTER_BLUR3X3 for the used 3x3 filter
	static i32 filter[3][3] =
	{
		{ 0, 1, 0 },
		{ 1, 2, 1 },
		{ 0, 1, 0 }
	};

	for (i32 i = 0; i < nTrgHeight; i++)
		for (i32 j = 0; j < nTrgWidth; j++)
		{
			// filter3x3
			i32 x = j << 1;
			i32 y = i << 1;

			i32 r, g, b, a;
			r = b = g = a = 0;
			u32 col;

			if (eAddressingMode == WRAP) // TODO: this condition could be compile-time static by making it a template arg
			{
				for (i32 i = 0; i < 3; i++)
				{
					for (i32 j = 0; j < 3; j++)
					{
						col = pSrcData[((y + nSrcHeight + i - 1) % nSrcHeight) * nSrcWidth + ((x + nSrcWidth + j - 1) % nSrcWidth)];

						r += (col & 0xff) << filter[i][j];
						g += ((col >> 8) & 0xff) << filter[i][j];
						b += ((col >> 16) & 0xff) << filter[i][j];
						a += ((col >> 24) & 0xff) << filter[i][j];
					}
				}
			}
			else
			{
				assert(eAddressingMode == CLAMP);
				for (i32 i = 0; i < 3; i++)
				{
					for (i32 j = 0; j < 3; j++)
					{
						i32 x1 = clamp_tpl<i32>((x + j), 0, nSrcWidth - 1);
						i32 y1 = clamp_tpl<i32>((y + i), 0, nSrcHeight - 1);
						col = pSrcData[y1 * nSrcWidth + x1];

						r += (col & 0xff) << filter[i][j];
						g += ((col >> 8) & 0xff) << filter[i][j];
						b += ((col >> 16) & 0xff) << filter[i][j];
						a += ((col >> 24) & 0xff) << filter[i][j];
					}
				}
			}

			// the sum of the multiplicative values here is 16 so we shift by 4 bits
			r >>= 4;
			g >>= 4;
			b >>= 4;
			a >>= 4;

			u32 res = r + (g << 8) + (b << 16) + (a << 24);

			*pDstData++ = res;
		}
}

//////////////////////////////////////////////////////////////////////////
void CImageUtil::ScaleToFit(const CImageEx& srcImage, CImageEx& trgImage)
{
	u32 x, y, u, v;
	u32* destRow, * dest, * src, * sourceRow;

	u32 srcW = srcImage.GetWidth();
	u32 srcH = srcImage.GetHeight();

	u32 trgW = trgImage.GetWidth();
	u32 trgH = trgImage.GetHeight();

	u32 xratio = trgW > 0 ? (srcW << 16) / trgW : 1;
	u32 yratio = trgH > 0 ? (srcH << 16) / trgH : 1;

	src = srcImage.GetData();
	destRow = trgImage.GetData();

	v = 0;
	for (y = 0; y < trgH; y++)
	{
		u = 0;
		sourceRow = src + (v >> 16) * srcW;
		dest = destRow;
		for (x = 0; x < trgW; x++)
		{
			*dest++ = sourceRow[u >> 16];
			u += xratio;
		}
		v += yratio;
		destRow += trgW;
	}
}

//////////////////////////////////////////////////////////////////////////
void CImageUtil::ScaleToDoubleFit(const CImageEx& srcImage, CImageEx& trgImage)
{
	u32 x, y, u, v;
	u32* destRow, * dest, * src, * sourceRow;

	u32 srcW = srcImage.GetWidth();
	u32 srcH = srcImage.GetHeight();

	u32 trgHalfW = trgImage.GetWidth() / 2;
	u32 trgH = trgImage.GetHeight();

	u32 xratio = trgHalfW > 0 ? (srcW << 16) / trgHalfW : 1;
	u32 yratio = trgH > 0 ? (srcH << 16) / trgH : 1;

	src = srcImage.GetData();
	destRow = trgImage.GetData();

	v = 0;
	for (y = 0; y < trgH; y++)
	{
		u = 0;
		sourceRow = src + (v >> 16) * srcW;
		dest = destRow;
		for (x = 0; x < trgHalfW; x++)
		{
			*(dest + trgHalfW) = sourceRow[u >> 16];
			*dest++ = sourceRow[u >> 16];
			u += xratio;
		}
		v += yratio;
		destRow += trgHalfW * 2;
	}
}

//////////////////////////////////////////////////////////////////////////
void CImageUtil::SmoothImage(CByteImage& image, i32 numSteps)
{
	assert(numSteps > 0);
	u8* buf = image.GetData();
	i32 w = image.GetWidth();
	i32 h = image.GetHeight();

	for (i32 steps = 0; steps < numSteps; steps++)
	{
		// Smooth the image.
		for (i32 y = 1; y < h - 1; y++)
		{
			// Precalculate for better speed
			u8* ptr = &buf[y * w + 1];

			for (i32 x = 1; x < w - 1; x++)
			{
				// Smooth it out
				*ptr =
				  (
				    (u32)ptr[1] +
				    ptr[w] +
				    ptr[-1] +
				    ptr[-w] +
				    ptr[w + 1] +
				    ptr[w - 1] +
				    ptr[-w + 1] +
				    ptr[-w - 1]
				  ) >> 3;

				// Next pixel
				ptr++;
			}
		}
	}
}

u8 CImageUtil::GetBilinearFilteredAt(i32k iniX256, i32k iniY256, const CByteImage& image)
{
	//	assert(image.IsValid());		if(!image.IsValid())return(0);		// this shouldn't be

	DWORD x = (DWORD)(iniX256) >> 8;
	DWORD y = (DWORD)(iniY256) >> 8;

	if (x >= image.GetWidth() - 1 || y >= image.GetHeight() - 1)
		return image.ValueAt(x, y);                              // border is not filtered, 255 to get in range 0..1

	DWORD rx = (DWORD)(iniX256) & 0xff;   // fractional aprt
	DWORD ry = (DWORD)(iniY256) & 0xff;   // fractional aprt

	DWORD top = (DWORD)image.ValueAt((i32)x, (i32)y) * (256 - rx)         // left top
	            + (DWORD)image.ValueAt((i32)x + 1, (i32)y) * rx;          // right top

	DWORD bottom = (DWORD)image.ValueAt((i32)x, (i32)y + 1) * (256 - rx)  // left bottom
	               + (DWORD)image.ValueAt((i32)x + 1, (i32)y + 1) * rx;   // right bottom

	return (u8)((top * (256 - ry) + bottom * ry) >> 16);
}

bool CImageUtil::FillFromBITMAPObj(const BITMAP* bitmap, CImageEx& image)
{
	if (bitmap == NULL)
		return false;

	if (bitmap->bmBitsPixel != 32)
		return false;

	image.Allocate(bitmap->bmWidth, bitmap->bmHeight);

	BYTE* src_p = (BYTE*)bitmap->bmBits;

	for (i32 i = 0; i < bitmap->bmHeight; i++)
	{
		for (i32 k = 0; k < bitmap->bmWidth; k++)
		{
			SRGBPixel* dest_p = (SRGBPixel*)(&(image.ValueAt(k, i)));

			dest_p->blue = *src_p++;
			dest_p->green = *src_p++;
			dest_p->red = *src_p++;
			dest_p->alpha = *src_p++;
		}
	}

	return true;
}

bool CImageUtil::CreateBitmapFromImage(const CImageEx& image, CBitmap& bitmapObj)
{
	CImageEx tempImg;
	tempImg.Attach(image);
	tempImg.ReverseUpDown();
	tempImg.SwapRedAndBlue();
	i32 imgSize = image.GetWidth() * image.GetHeight() * 4;

	bitmapObj.SetBitmapBits(imgSize, tempImg.GetData());

	return true;
}

