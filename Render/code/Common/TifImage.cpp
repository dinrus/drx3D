// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>

#ifdef DRX_PLATFORM_WINDOWS
#include <tiffio.h>   // TIFF library
#endif

bool WriteTIF(ukk pData, i32 width, i32 height, i32 bytesPerChannel, i32 numChannels, bool bFloat, tukk szPreset, tukk szFileName)
{
#ifdef DRX_PLATFORM_WINDOWS
	if (bFloat && (bytesPerChannel != 2 && bytesPerChannel != 4))
	{
		bFloat = false;
	}

	bool bRet = false;

	TIFF* tif = TIFFOpen(szFileName, "wb");
	if (tif)
	{
		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, numChannels);
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bytesPerChannel * 8);
		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
		TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		if (bFloat)
		{
			TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
		}

		if (szPreset && szPreset[0])
		{
			string tiffphotoshopdata, valueheader;
			string presetkeyvalue = string("/preset=") + string(szPreset);

			valueheader.push_back('\x1C');
			valueheader.push_back('\x02');
			valueheader.push_back('\x28');
			valueheader.push_back((presetkeyvalue.size() >> 8) & 0xFF);
			valueheader.push_back((presetkeyvalue.size()) & 0xFF);
			valueheader.append(presetkeyvalue);

			tiffphotoshopdata.push_back('8');
			tiffphotoshopdata.push_back('B');
			tiffphotoshopdata.push_back('I');
			tiffphotoshopdata.push_back('M');
			tiffphotoshopdata.push_back('\x04');
			tiffphotoshopdata.push_back('\x04');
			tiffphotoshopdata.push_back('\x00');
			tiffphotoshopdata.push_back('\x00');

			tiffphotoshopdata.push_back((valueheader.size() >> 24) & 0xFF);
			tiffphotoshopdata.push_back((valueheader.size() >> 16) & 0xFF);
			tiffphotoshopdata.push_back((valueheader.size() >> 8) & 0xFF);
			tiffphotoshopdata.push_back((valueheader.size()) & 0xFF);
			tiffphotoshopdata.append(valueheader);

			TIFFSetField(tif, TIFFTAG_PHOTOSHOP, tiffphotoshopdata.size(), tiffphotoshopdata.c_str());
		}

		size_t pitch = width * bytesPerChannel * numChannels;
		tuk raster = (tuk)_TIFFmalloc((tsize_t)(pitch * height));
		memcpy(raster, pData, pitch * height);

		bRet = true;
		for (i32 h = 0; h < height; ++h)
		{
			size_t offset = h * pitch;
			i32 err = TIFFWriteScanline(tif, raster + offset, h, 0);
			assert(IsHeapValid());
			if (err < 0)
			{
				bRet = false;
				break;
			}
}
		_TIFFfree(raster);
		TIFFClose(tif);
	}
	return bRet;
#else
	return false;
#endif
}

