// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CAlphaBitmap;

/*!
 *	Utility Class to manipulate images.
 */
class PLUGIN_API CImageUtil
{
public:
	//////////////////////////////////////////////////////////////////////////
	// Image loading.
	//////////////////////////////////////////////////////////////////////////
	//! Load image, detect image type by file extension.
	// Arguments:
	//   pQualityLoss - 0 if info is not needed, pointer to the result otherwise - not need to preinitialize
	static bool LoadImage(const string& fileName, CImageEx& image, bool* pQualityLoss = 0);
	static bool LoadImage(tukk fileName, CImageEx& image, bool* pQualityLoss = 0);
	//! Save image, detect image type by file extension.
	static bool SaveImage(const string& fileName, CImageEx& image);

	// General image fucntions
	static bool LoadJPEG(const string& strFileName, CImageEx& image);
	static bool SaveJPEG(const string& strFileName, CImageEx& image);

	static bool SaveBitmap(const string& szFileName, CImageEx& image, bool inverseY = true);
	static bool SaveBitmap(LPCSTR szFileName, DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, HDC hdc);
	static bool LoadBmp(const string& file, CImageEx& image);
	static bool LoadBmp(const string& fileName, CImageEx& image, const RECT& rc);

	//! Save image in PGM format.
	static bool SavePGM(const string& fileName, u32 dwWidth, u32 dwHeight, u32* pData);
	//! Load image in PGM format.
	static bool LoadPGM(const string& fileName, u32* pWidthOut, u32* pHeightOut, u32** pImageDataOut);

	//////////////////////////////////////////////////////////////////////////
	// Image scaling.
	//////////////////////////////////////////////////////////////////////////
	//! Scale source image to fit size of target image.
	static void ScaleToFit(const CByteImage& srcImage, CByteImage& trgImage);
	//! Scale source image to fit size of target image.
	static void ScaleToFit(const CImageEx& srcImage, CImageEx& trgImage);
	//! Scale source image to fit twice side by side in target image.
	static void ScaleToDoubleFit(const CImageEx& srcImage, CImageEx& trgImage);
	//! Scale source image twice down image with filering
	enum _EAddrMode {WRAP, CLAMP};
	static void DownScaleSquareTextureTwice(const CImageEx& srcImage, CImageEx& trgImage, _EAddrMode eAddressingMode = WRAP);

	//! Smooth image.
	static void SmoothImage(CByteImage& image, i32 numSteps);

	//////////////////////////////////////////////////////////////////////////
	// filtered lookup
	//////////////////////////////////////////////////////////////////////////

	//! behaviour outside of the texture is not defined
	//! \param iniX in fix point 24.8
	//! \param iniY in fix point 24.8
	//! \return 0..255
	static u8 GetBilinearFilteredAt(i32k iniX256, i32k iniY256, const CByteImage& image);

private:

	static bool LoadImageWithGDIPlus(const string& fileName, CImageEx& image);
	static bool FillFromBITMAPObj(const BITMAP* bitmap, CImageEx& image);
	static bool CreateBitmapFromImage(const CImageEx& image, CBitmap& bitmapObj);
	static bool Save(const string& strFileName, CImageEx& inImage);
};

