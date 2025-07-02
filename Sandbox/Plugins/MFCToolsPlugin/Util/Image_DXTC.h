// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __image_dxtc_h__
#define __image_dxtc_h__
#pragma once

#include <Drx3DEngine/ImageExtensionHelper.h>

class CImage_DXTC
{
	// Typedefs
public:
protected:
	//////////////////////////////////////////////////////////////////////////
	// Extracted from Compressorlib.h on SDKs\CompressATI directory.
	// Added here because we are not really using the elements inside
	// this header file apart from those definitions as we are currently
	// loading the DLL CompressATI2.dll manually as recommended by the rendering
	// team.
	typedef enum
	{
		FORMAT_ARGB_8888,
		FORMAT_ARGB_TOOBIG
	} UNCOMPRESSED_FORMAT;

	typedef enum
	{
		COMPRESSOR_ERROR_NONE,
		COMPRESSOR_ERROR_NO_INPUT_DATA,
		COMPRESSOR_ERROR_NO_OUTPUT_POINTER,
		COMPRESSOR_ERROR_UNSUPPORTED_SOURCE_FORMAT,
		COMPRESSOR_ERROR_UNSUPPORTED_DESTINATION_FORMAT,
		COMPRESSOR_ERROR_UNABLE_TO_INIT_CODEC,
		COMPRESSOR_ERROR_GENERIC
	} COMPRESSOR_ERROR;
	//////////////////////////////////////////////////////////////////////////

	// Methods
public:
	CImage_DXTC();
	~CImage_DXTC();

	// Arguments:
	//   pQualityLoss - 0 if info is not needed, pointer to the result otherwise - not need to preinitialize
	bool                      Load(tukk filename, CImageEx& outImage, bool* pQualityLoss = 0); // true if success

	static inline tukk NameForTextureFormat(ETEX_Format ETF) { return CImageExtensionHelper::NameForTextureFormat(ETF); }
	static inline bool        IsBlockCompressed(ETEX_Format ETF)    { return CImageExtensionHelper::IsBlockCompressed(ETF); }
	static inline bool        IsLimitedHDR(ETEX_Format ETF)         { return CImageExtensionHelper::IsRangeless(ETF); }
	static inline i32         BitsPerPixel(ETEX_Format eTF)         { return CImageExtensionHelper::BitsPerPixel(eTF); }

	i32                       TextureDataSize(i32 nWidth, i32 nHeight, i32 nDepth, i32 nMips, ETEX_Format eTF);

private:
	static COMPRESSOR_ERROR CheckParameters(
	  i32 width,
	  i32 height,
	  UNCOMPRESSED_FORMAT destinationFormat,
	  ukk sourceData,
	  uk destinationData,
	  i32 destinationDataSize);

	static COMPRESSOR_ERROR DecompressTextureBTC(
	  i32 width,
	  i32 height,
	  ETEX_Format sourceFormat,
	  UNCOMPRESSED_FORMAT destinationFormat,
	  i32k imageFlags,
	  ukk sourceData,
	  uk destinationData,
	  i32 destinationDataSize,
	  i32 destinationPageOffset);
};

#endif // __image_dxtc_h__

