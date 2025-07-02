// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ImageTIF.h"

#include <tiffio.h>   // TIFF library

#include <DrxSystem/File/DrxFile.h>
#include "Util/FileUtil.h"

#include <drx3D/CoreX/Renderer/IRenderer.h>


// Function prototypes
static tsize_t libtiffDummyReadProc(thandle_t fd, tdata_t buf, tsize_t size);
static tsize_t libtiffDummyWriteProc(thandle_t fd, tdata_t buf, tsize_t size);
static toff_t  libtiffDummySizeProc(thandle_t fd);
static toff_t  libtiffDummySeekProc(thandle_t fd, toff_t off, i32 i);
//static i32 libtiffDummyCloseProc (thandle_t fd);

// We need globals because of the callbacks (they don't allow us to pass state)
static DrxMutex globalImageMutex;
static tuk globalImageBuffer = 0;
static u64 globalImageBufferOffset = 0;
static u64 globalImageBufferSize = 0;

/////////////////// Callbacks to libtiff

static i32 libtiffDummyMapFileProc(thandle_t, tdata_t*, toff_t*)
{
	return 0;
}

static void libtiffDummyUnmapFileProc(thandle_t, tdata_t, toff_t)
{
}

static toff_t libtiffDummySizeProc(thandle_t fd)
{
	return globalImageBufferSize;
}

static tsize_t
libtiffDummyReadProc(thandle_t fd, tdata_t buf, tsize_t size)
{
	tsize_t nBytesLeft = globalImageBufferSize - globalImageBufferOffset;

	if (size > nBytesLeft)
		size = nBytesLeft;

	memcpy(buf, &globalImageBuffer[globalImageBufferOffset], size);

	globalImageBufferOffset += size;

	// Return the amount of data read
	return size;
}

static tsize_t
libtiffDummyWriteProc(thandle_t fd, tdata_t buf, tsize_t size)
{
	return (size);
}

static toff_t
libtiffDummySeekProc(thandle_t fd, toff_t off, i32 i)
{
	switch (i)
	{
	case SEEK_SET:
		globalImageBufferOffset = off;
		break;

	case SEEK_CUR:
		globalImageBufferOffset += off;
		break;

	case SEEK_END:
		globalImageBufferOffset = globalImageBufferSize - off;
		break;

	default:
		globalImageBufferOffset = off;
		break;
	}

	// This appears to return the location that it went to
	return globalImageBufferOffset;
}

static i32
libtiffDummyCloseProc(thandle_t fd)
{
	// Return a zero meaning all is well
	return 0;
}

bool CImageTIF::Load(const string& fileName, CImageEx& outImage)
{
	CDrxFile file;
	if (!file.Open(fileName, "rb"))
	{
		DrxLog("File not found %s", (tukk)fileName);
		return false;
	}

	// We use some global variables in callbacks, so we must
	// prevent multithread access to the data
	DrxAutoLock<DrxMutex> tifAutoLock(globalImageMutex);

	std::vector<u8> data;

	globalImageBufferSize = file.GetLength();

	data.resize(globalImageBufferSize);
	globalImageBuffer = (tuk)&data[0];
	globalImageBufferOffset = 0;

	file.ReadRaw(globalImageBuffer, globalImageBufferSize);

	// Open the dummy document (which actually only exists in memory)
	TIFF* tif = TIFFClientOpen(fileName, "rm", (thandle_t) -1, libtiffDummyReadProc,
	                           libtiffDummyWriteProc, libtiffDummySeekProc,
	                           libtiffDummyCloseProc, libtiffDummySizeProc, libtiffDummyMapFileProc, libtiffDummyUnmapFileProc);

	//	TIFF* tif = TIFFOpen(fileName,"r");

	bool bRet = false;

	if (tif)
	{
		u32 dwWidth, dwHeight;
		size_t npixels;
		u32* raster;
		tuk dccfilename = NULL;

		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &dwWidth);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &dwHeight);
		TIFFGetField(tif, TIFFTAG_IMAGEDESCRIPTION, &dccfilename);

		npixels = dwWidth * dwHeight;
		raster = (u32*) _TIFFmalloc((tsize_t)(npixels * sizeof(u32)));

		if (raster)
		{
			if (TIFFReadRGBAImage(tif, dwWidth, dwHeight, raster, 0))
			{
				if (outImage.Allocate(dwWidth, dwHeight))
				{
					tuk dest = (tuk)outImage.GetData();
					u32 dwPitch = dwWidth * 4;

					for (u32 dwY = 0; dwY < dwHeight; ++dwY)
					{
						tuk src2 = (tuk)&raster[(dwHeight - 1 - dwY) * dwWidth];
						tuk dest2 = &dest[dwPitch * dwY];

						memcpy(dest2, src2, dwWidth * 4);
					}

					if (dccfilename)
						outImage.SetDccFilename(dccfilename);

					bRet = true;

					/*
					   {
					   string sSpecialInstructions = GetSpecialInstructionsFromTIFF(tif);

					   char *p = sSpecialInstructions.GetBuffer();

					   while(*p)
					   {
					    string sKey,sValue;

					    JumpOverWhitespace(p);

					    if(*p != '/')								// /
					    {
					      m_pCC->pLog->LogError("Special instructions in TIFF '%s' are broken (/ expected)",lpszPathName);
					      break;
					    }
					    p++;												// jump over /

					    while(IsValidNameChar(*p))	// key
					      sKey+=*p++;

					    JumpOverWhitespace(p);

					    if(*p != '=')								// =
					    {
					      m_pCC->pLog->LogError("Special instructions in TIFF '%s' are broken (= expected)",lpszPathName);
					      break;
					    }
					    p++;												// jump over =

					    JumpOverWhitespace(p);

					    while(IsValidNameChar(*p))	// value
					      sValue+=*p++;

					    JumpOverWhitespace(p);

					    sKey.Trim();sValue.Trim();

					    m_pCC->pFileSpecificConfig->UpdateOrCreateEntry("",sKey,sValue);
					   }
					   }
					 */
				}
			}

			_TIFFfree(raster);
		}
		TIFFClose(tif);
	}

	if (!bRet)
		outImage.Detach();

	return bRet;
}

//////////////////////////////////////////////////////////////////////////
bool CImageTIF::SaveRAW(const string& fileName, ukk pData, i32 width, i32 height, i32 bytesPerChannel, i32 numChannels, bool bFloat, tukk preset)
{
	CFileUtil::OverwriteFile(fileName);

	return GetIEditor()->GetRenderer()->WriteTIFToDisk(pData, width, height, bytesPerChannel, numChannels, bFloat, preset, fileName);
}

tukk CImageTIF::GetPreset(const string& fileName)
{
	std::vector<u8> data;
	CDrxFile file;
	if (!file.Open(fileName, "rb"))
	{
		DrxLog("File not found %s", (tukk)fileName);
		return NULL;
	}
	globalImageBufferSize = file.GetLength();

	data.resize(globalImageBufferSize);
	globalImageBuffer = (tuk)&data[0];
	globalImageBufferOffset = 0;

	file.ReadRaw(globalImageBuffer, globalImageBufferSize);

	TIFF* tif = TIFFClientOpen(fileName, "rm", (thandle_t) -1, libtiffDummyReadProc,
	                           libtiffDummyWriteProc, libtiffDummySeekProc,
	                           libtiffDummyCloseProc, libtiffDummySizeProc, libtiffDummyMapFileProc, libtiffDummyUnmapFileProc);

	string strReturn;
	tuk preset = NULL;
	i32 size;
	if (tif)
	{
		TIFFGetField(tif, TIFFTAG_PHOTOSHOP, &size, &preset);
		for (i32 i = 0; i < size; ++i)
		{
			if (!strncmp((preset + i), "preset", 6))
			{
				tuk presetoffset = preset + i;
				strReturn = presetoffset;
				if (strReturn.find('/') != -1)
					strReturn = strReturn.substr(0, strReturn.find('/'));

				break;
			}

		}
		TIFFClose(tif);
	}
	return strReturn.c_str();
}

