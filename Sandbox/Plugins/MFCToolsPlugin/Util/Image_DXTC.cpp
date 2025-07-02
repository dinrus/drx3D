// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "Image_DXTC.h"
#include <DrxSystem/File/DrxFile.h>
#include <drx3D/CoreX/BitFiddling.h>

#ifndef MAKEFOURCC
	#define MAKEFOURCC(ch0, ch1, ch2, ch3)              \
	  ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | \
	   ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))
#endif //defined(MAKEFOURCC)

#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/CoreX/Renderer/IImage.h>
#include <Drx3DEngine/ImageExtensionHelper.h>

//////////////////////////////////////////////////////////////////////////
// HDR_UPPERNORM -> factor used when converting from [0,32768] high dynamic range images
//                  to [0,1] low dynamic range images; 32768 = 2^(2^4-1), 4 exponent bits
// LDR_UPPERNORM -> factor used when converting from [0,1] low dynamic range images
//                  to 8bit outputs

#define HDR_UPPERNORM 1.0f  // factor set to 1.0, to be able to see content in our rather dark HDR images
#define LDR_UPPERNORM 255.0f

static float GammaToLinear(float x)
{
	return (x <= 0.04045f) ? x / 12.92f : powf((x + 0.055f) / 1.055f, 2.4f);
}

static float LinearToGamma(float x)
{
	return (x <= 0.0031308f) ? x * 12.92f : 1.055f * powf(x, 1.0f / 2.4f) - 0.055f;
}

//////////////////////////////////////////////////////////////////////////
// preserve the ability to use the old squish code in parallel
#define squish          squishccr
#define SQUISH_USE_CPP
#define SQUISH_USE_SSE  3
#define SQUISH_USE_XSSE 0
#define SQUISH_USE_CCR

// Squish uses non-standard inline friend templates which Recode cannot parse
#ifndef __RECODE__
	#include "squish.h"
	#include "squish.inl"
#endif

// number of bytes per block per type
#define BLOCKSIZE_BC1 8
#define BLOCKSIZE_BC2 16
#define BLOCKSIZE_BC3 16
#define BLOCKSIZE_BC4 8
#define BLOCKSIZE_BC5 16
#define BLOCKSIZE_BC6 16
#define BLOCKSIZE_BC7 16

CImage_DXTC::COMPRESSOR_ERROR CImage_DXTC::DecompressTextureBTC(
  i32 width,
  i32 height,
  ETEX_Format sourceFormat,
  CImage_DXTC::UNCOMPRESSED_FORMAT destinationFormat,
  i32k imageFlags,
  ukk sourceData,
  uk destinationData,
  i32 destinationDataSize,
  i32 destinationPageOffset)
{
	// Squish uses non-standard inline friend templates which Recode cannot parse
#ifndef __RECODE__
	{
		const COMPRESSOR_ERROR result = CheckParameters(
		  width,
		  height,
		  destinationFormat,
		  sourceData,
		  destinationData,
		  destinationDataSize);

		if (result != COMPRESSOR_ERROR_NONE)
		{
			return result;
		}
	}

	i32 flags = 0;
	i32 offs = 0;
	i32 sourceChannels = 4;
	switch (sourceFormat)
	{
	case eTF_BC1:
		sourceChannels = 4;
		flags = squish::kBtc1;
		break;
	case eTF_BC2:
		sourceChannels = 4;
		flags = squish::kBtc2;
		break;
	case eTF_BC3:
		sourceChannels = 4;
		flags = squish::kBtc3;
		break;
	case eTF_BC4U:
		sourceChannels = 1;
		flags = squish::kBtc4;
		break;
	case eTF_BC5U:
		sourceChannels = 2;
		flags = squish::kBtc5 + squish::kColourMetricUnit;
		break;
	case eTF_BC6UH:
		sourceChannels = 3;
		flags = squish::kBtc6;
		break;
	case eTF_BC7:
		sourceChannels = 4;
		flags = squish::kBtc7;
		break;

	case eTF_BC4S:
		sourceChannels = 1;
		flags = squish::kBtc4 + squish::kSignedInternal + squish::kSignedExternal;
		offs = 0x80;
		break;
	case eTF_BC5S:
		sourceChannels = 2;
		flags = squish::kBtc5 + squish::kSignedInternal + squish::kSignedExternal + squish::kColourMetricUnit;
		offs = 0x80;
		break;
	case eTF_BC6SH:
		sourceChannels = 3;
		flags = squish::kBtc6 + squish::kSignedInternal + squish::kSignedExternal;
		offs = 0x80;
		break;

	default:
		return COMPRESSOR_ERROR_UNSUPPORTED_SOURCE_FORMAT;
	}

	squish::sqio::dtp datatype = !IsLimitedHDR(sourceFormat) ? squish::sqio::dtp::DT_U8 : squish::sqio::dtp::DT_F23;
	switch (destinationFormat)
	{
	case FORMAT_ARGB_8888:   /*datatype = squish::sqio::dtp::DT_U8;*/
		break;
	//	case FORMAT_ARGB_16161616: datatype = squish::sqio::dtp::DT_U16; break;
	//	case FORMAT_ARGB_32323232F: datatype = squish::sqio::dtp::DT_F23; break;
	default:
		return COMPRESSOR_ERROR_UNSUPPORTED_DESTINATION_FORMAT;
	}

	struct squish::sqio sqio = squish::GetSquishIO(width, height, datatype, flags);

	i32k blockChannels = 4;
	i32k blockWidth = 4;
	i32k blockHeight = 4;

	i32k pixelStride = blockChannels * sizeof(u8);
	i32k rowStride = (destinationPageOffset ? destinationPageOffset : pixelStride * width);

	if ((datatype == squish::sqio::dtp::DT_U8) && (destinationFormat == FORMAT_ARGB_8888))
	{
		tukk src = (tukk)sourceData;
		for (i32 y = 0; y < height; y += blockHeight)
		{
			u8* dst = ((u8*)destinationData) + (y * rowStride);

			for (i32 x = 0; x < width; x += blockWidth)
			{
				u8 values[blockHeight][blockWidth][blockChannels] = { 0 };

				// decode
				sqio.decoder((u8*)values, src, sqio.flags);

				// transfer
				for (i32 by = 0; by < blockHeight; by += 1)
				{
					u8* bdst = ((u8*)dst) + (by * rowStride);

					for (i32 bx = 0; bx < blockWidth; bx += 1)
					{
						bdst[bx * pixelStride + 0] = sourceChannels <= 0 ? 0U : (values[by][bx][0] + offs);
						bdst[bx * pixelStride + 1] = sourceChannels <= 1 ? bdst[bx * pixelStride + 0] : (values[by][bx][1] + offs);
						bdst[bx * pixelStride + 2] = sourceChannels <= 1 ? bdst[bx * pixelStride + 0] : (values[by][bx][2] + offs);
						bdst[bx * pixelStride + 3] = sourceChannels <= 3 ? 255U : (values[by][bx][3]);
					}
				}

				dst += blockWidth * pixelStride;
				src += sqio.blocksize;
			}
		}
	}
	else if ((datatype == squish::sqio::dtp::DT_F23) && (destinationFormat == FORMAT_ARGB_8888))
	{
		tukk src = (tukk)sourceData;
		for (i32 y = 0; y < height; y += blockHeight)
		{
			u8* dst = ((u8*)destinationData) + (y * rowStride);

			for (i32 x = 0; x < width; x += blockWidth)
			{
				float values[blockHeight][blockWidth][blockChannels] = { 0 };

				// decode
				sqio.decoder((float*)values, src, sqio.flags);

				// transfer
				for (i32 by = 0; by < blockHeight; by += 1)
				{
					u8* bdst = ((u8*)dst) + (by * rowStride);

					for (i32 bx = 0; bx < blockWidth; bx += 1)
					{
						bdst[bx * pixelStride + 0] = sourceChannels <= 0 ? 0U : std::min((u8)255, (u8)floorf(values[by][bx][0] * LDR_UPPERNORM / HDR_UPPERNORM + 0.5f));
						bdst[bx * pixelStride + 1] = sourceChannels <= 1 ? bdst[bx * pixelStride + 0] : std::min((u8)255, (u8)floorf(values[by][bx][1] * LDR_UPPERNORM / HDR_UPPERNORM + 0.5f));
						bdst[bx * pixelStride + 2] = sourceChannels <= 1 ? bdst[bx * pixelStride + 0] : std::min((u8)255, (u8)floorf(values[by][bx][2] * LDR_UPPERNORM / HDR_UPPERNORM + 0.5f));
						bdst[bx * pixelStride + 3] = sourceChannels <= 3 ? 255U : 255U;
					}
				}

				dst += blockWidth * pixelStride;
				src += sqio.blocksize;
			}
		}
	}
#endif

	return COMPRESSOR_ERROR_NONE;
}

//////////////////////////////////////////////////////////////////////////
CImage_DXTC::CImage_DXTC()
{
}
//////////////////////////////////////////////////////////////////////////
CImage_DXTC::~CImage_DXTC()
{
}
//////////////////////////////////////////////////////////////////////////
bool CImage_DXTC::Load(tukk filename, CImageEx& outImage, bool* pQualityLoss)
{
	if (pQualityLoss)
		*pQualityLoss = false;

	_smart_ptr<IImageFile> pImage = gEnv->pRenderer->EF_LoadImage(filename, 0);

	if (!pImage)
	{
		return(false);
	}

	BYTE* pDecompBytes;

	// Does texture have mipmaps?
	bool bMipTexture = pImage->mfGet_numMips() > 1;

	ETEX_Format eFormat = pImage->mfGetFormat();
	i32 imageFlags = pImage->mfGet_Flags();

	if (eFormat == eTF_Unknown)
	{
		return false;
	}

	_smart_ptr<IImageFile> pAlphaImage;

	ETEX_Format eAttachedFormat = eTF_Unknown;

	if (imageFlags & FIM_HAS_ATTACHED_ALPHA)
	{
		if (pAlphaImage = gEnv->pRenderer->EF_LoadImage(filename, FIM_ALPHA))
		{
			eAttachedFormat = pAlphaImage->mfGetFormat();
		}
	}

	const bool bIsSRGB = (imageFlags & FIM_SRGB_READ) != 0;
	outImage.SetSRGB(bIsSRGB);

	u32k imageWidth = pImage->mfGet_width();
	u32k imageHeight = pImage->mfGet_height();
	u32k numMips = pImage->mfGet_numMips();

	i32 nHorizontalFaces(1);
	i32 nVerticalFaces(1);
	i32 nNumFaces(1);
	i32 nMipMapsSize(0);
	i32 nMipMapCount(numMips);
	i32 nTargetPitch(imageWidth * 4);
	i32 nTargetPageSize(nTargetPitch * imageHeight);

	i32 nHorizontalPageOffset(nTargetPitch);
	i32 nVerticalPageOffset(0);
	bool boIsCubemap = pImage->mfGet_NumSides() == 6;
	if (boIsCubemap)
	{
		nHorizontalFaces = 3;
		nVerticalFaces = 2;
		nHorizontalPageOffset = nTargetPitch * nHorizontalFaces;
		nVerticalPageOffset = nTargetPageSize * nHorizontalFaces;
	}

	outImage.Allocate(imageWidth * nHorizontalFaces, imageHeight * nVerticalFaces);
	pDecompBytes = (BYTE*)outImage.GetData();
	if (!pDecompBytes)
	{
		DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Cannot allocate image %dx%d, Out of memory", imageWidth, imageHeight);
		return false;
	}

	if (pQualityLoss)
		*pQualityLoss = CImageExtensionHelper::IsQuantized(eFormat);

	bool bOk = true;

	i32 nCurrentFace(0);
	i32 nCurrentHorizontalFace(0);
	i32 nCurrentVerticalFace(0);
	u8* dest(NULL);
	u8k* src(NULL);

	u8* basedest(NULL);
	u8k* basesrc(NULL);

	for (nCurrentHorizontalFace = 0; nCurrentHorizontalFace < nHorizontalFaces; ++nCurrentHorizontalFace)
	{
		basedest = &pDecompBytes[nTargetPitch * nCurrentHorizontalFace]; // Horizontal offset.
		for (nCurrentVerticalFace = 0; nCurrentVerticalFace < nVerticalFaces; ++nCurrentVerticalFace, ++nCurrentFace)
		{
			basedest += nVerticalPageOffset * nCurrentVerticalFace; // Vertical offset.

			basesrc = src = pImage->mfGet_image(nCurrentFace);

			if (eFormat == eTF_R8G8B8A8 || eFormat == eTF_R8G8B8A8S)
			{
				for (i32 y = 0; y < imageHeight; y++)
				{
					dest = basedest + nHorizontalPageOffset * y; // Pixel position.

					for (i32 x = 0; x < imageWidth; x++)
					{
						dest[0] = src[0];
						dest[1] = src[1];
						dest[2] = src[2];
						dest[3] = src[3];

						dest += 4;
						src += 4;
					}
				}
			}
			else if (eFormat == eTF_B8G8R8A8)
			{
				for (i32 y = 0; y < imageHeight; y++)
				{
					dest = basedest + nHorizontalPageOffset * y; // Pixel position.

					for (i32 x = 0; x < imageWidth; x++)
					{
						dest[0] = src[2];
						dest[1] = src[1];
						dest[2] = src[0];
						dest[3] = src[3];

						dest += 4;
						src += 4;
					}
				}
			}
			else if (eFormat == eTF_B8G8R8X8)
			{
				for (i32 y = 0; y < imageHeight; y++)
				{
					dest = basedest + nHorizontalPageOffset * y; // Pixel position.

					for (i32 x = 0; x < imageWidth; x++)
					{
						dest[0] = src[2];
						dest[1] = src[1];
						dest[2] = src[0];
						dest[3] = 255;

						dest += 4;
						src += 4;
					}
				}
			}
			else if (eFormat == eTF_B8G8R8)
			{
				for (i32 y = 0; y < imageHeight; y++)
				{
					dest = basedest + nHorizontalPageOffset * y; // Pixel position.

					for (i32 x = 0; x < imageWidth; x++)
					{
						dest[0] = src[2];
						dest[1] = src[1];
						dest[2] = src[0];
						dest[3] = 255;

						dest += 4;
						src += 3;
					}
				}
			}
			else if (eFormat == eTF_L8)
			{
				for (i32 y = 0; y < imageHeight; y++)
				{
					dest = basedest + nHorizontalPageOffset * y; // Pixel position.

					for (i32 x = 0; x < imageWidth; x++)
					{
						dest[0] = *src;
						dest[1] = *src;
						dest[2] = *src;
						dest[3] = 255;

						dest += 4;
						src += 1;
					}
				}
			}
			else if (eFormat == eTF_A8)
			{
				for (i32 y = 0; y < imageHeight; y++)
				{
					dest = basedest + nHorizontalPageOffset * y; // Pixel position.

					for (i32 x = 0; x < imageWidth; x++)
					{
						dest[0] = 0;
						dest[1] = 0;
						dest[2] = 0;
						dest[3] = *src;

						dest += 4;
						src += 1;
					}
				}
			}
			else if (eFormat == eTF_A8L8)
			{
				for (i32 y = 0; y < imageHeight; y++)
				{
					dest = basedest + nHorizontalPageOffset * y; // Pixel position.

					for (i32 x = 0; x < imageWidth; x++)
					{
						dest[0] = src[0];
						dest[1] = src[0];
						dest[2] = src[0];
						dest[3] = src[1];

						dest += 4;
						src += 2;
					}
				}
			}
			else if (eFormat == eTF_R9G9B9E5)
			{
				i32k nSourcePitch = imageWidth * 4;

				for (i32 y = 0; y < imageHeight; y++)
				{
					src = basesrc + nSourcePitch * y;            //Scanline position.
					dest = basedest + nHorizontalPageOffset * y; // Pixel position.

					for (i32 x = 0; x < imageWidth; x++)
					{
						const struct RgbE
						{
							u32 r : 9, g : 9, b : 9, e : 5;
						}* srcv = (const struct RgbE*)src;

						const float escale = powf(2.0f, i32(srcv->e) - 15 - 9) * LDR_UPPERNORM / HDR_UPPERNORM;

						dest[0] = std::min((u8)255, (u8)floorf(srcv->r * escale + 0.5f));
						dest[1] = std::min((u8)255, (u8)floorf(srcv->g * escale + 0.5f));
						dest[2] = std::min((u8)255, (u8)floorf(srcv->b * escale + 0.5f));
						dest[3] = 255U;

						dest += 4;
						src += 4;
					}
				}
			}
			else
			{
				i32k pixelCount = imageWidth * imageHeight;
				i32k outputBufferSize = pixelCount * 4;
				i32k mipCount = numMips;

				const COMPRESSOR_ERROR err = DecompressTextureBTC(imageWidth, imageHeight, eFormat, FORMAT_ARGB_8888, imageFlags, basesrc, basedest, outputBufferSize, nHorizontalPageOffset);
				if (err != COMPRESSOR_ERROR_NONE)
				{
					return false;
				}
			}

			// alpha channel might be attached
			if (imageFlags & FIM_HAS_ATTACHED_ALPHA)
			{
				if (IsBlockCompressed(eAttachedFormat))
				{
					const byte* const basealpha = pAlphaImage->mfGet_image(0);
					i32k alphaImageWidth = pAlphaImage->mfGet_width();
					i32k alphaImageHeight = pAlphaImage->mfGet_height();
					i32k alphaImageFlags = pAlphaImage->mfGet_Flags();
					i32k tmpOutputBufferSize = imageWidth * imageHeight * 4;
					u8* tmpOutputBuffer = new u8[tmpOutputBufferSize];

					const COMPRESSOR_ERROR err = DecompressTextureBTC(alphaImageWidth, alphaImageHeight, eAttachedFormat, FORMAT_ARGB_8888, alphaImageFlags, basealpha, tmpOutputBuffer, tmpOutputBufferSize, 0);
					if (err != COMPRESSOR_ERROR_NONE)
					{
						delete[]tmpOutputBuffer;
						return false;
					}

					// assuming attached image can have lower res and difference is power of two
					u32k reducex = IntegerLog2((u32)(imageWidth / alphaImageWidth));
					u32k reducey = IntegerLog2((u32)(imageHeight / alphaImageHeight));

					for (i32 y = 0; y < imageHeight; ++y)
					{
						dest = basedest + nHorizontalPageOffset * y; // Pixel position.

						for (i32 x = 0; x < imageWidth; ++x)
						{
							dest[3] = tmpOutputBuffer[((x >> reducex) + (y >> reducey) * alphaImageWidth) * 4];
							dest += 4;
						}
					}

					delete[]tmpOutputBuffer;
				}
				else if (eAttachedFormat != eTF_Unknown)
				{
					const byte* const basealpha = pAlphaImage->mfGet_image(0); // assuming it's A8 format (ensured with assets when loading)
					i32k alphaImageWidth = pAlphaImage->mfGet_width();
					i32k alphaImageHeight = pAlphaImage->mfGet_height();
					i32k alphaImageFlags = pAlphaImage->mfGet_Flags();

					// assuming attached image can have lower res and difference is power of two
					u32k reducex = IntegerLog2((u32)(imageWidth / alphaImageWidth));
					u32k reducey = IntegerLog2((u32)(imageHeight / alphaImageHeight));

					for (i32 y = 0; y < imageHeight; ++y)
					{
						dest = basedest + nHorizontalPageOffset * y; // Pixel position.

						for (i32 x = 0; x < imageWidth; ++x)
						{
							dest[3] = basealpha[(x >> reducex) + (y >> reducey) * alphaImageWidth];
							dest += 4;
						}
					}
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// destination range is 8bits
	// rescale in linear space
	float cScaleR = 1.0f;
	float cScaleG = 1.0f;
	float cScaleB = 1.0f;
	float cScaleA = 1.0f;
	float cLowR = 0.0f;
	float cLowG = 0.0f;
	float cLowB = 0.0f;
	float cLowA = 0.0f;

	if (imageFlags & FIM_RENORMALIZED_TEXTURE)
	{
		const ColorF cMinColor = pImage->mfGet_minColor();
		const ColorF cMaxColor = pImage->mfGet_maxColor();

		// base range after normalization, fe. [0,1] for 8bit images, or [0,2^15] for RGBE/HDR data
		float cUprValue = 1.0f;
		if (CImageExtensionHelper::IsDynamicRange(eFormat))
			cUprValue = cMaxColor.a / HDR_UPPERNORM;

		// original range before normalization, fe. [0,1.83567]
		cScaleR = (cMaxColor.r - cMinColor.r) / cUprValue;
		cScaleG = (cMaxColor.g - cMinColor.g) / cUprValue;
		cScaleB = (cMaxColor.b - cMinColor.b) / cUprValue;
		// original offset before normalization, fe. [0.0001204]
		cLowR = cMinColor.r;
		cLowG = cMinColor.g;
		cLowB = cMinColor.b;
	}

	if (imageFlags & FIM_HAS_ATTACHED_ALPHA)
	{
		if (pAlphaImage)
		{
			i32k alphaImageFlags = pAlphaImage->mfGet_Flags();

			if (alphaImageFlags & FIM_RENORMALIZED_TEXTURE)
			{
				const ColorF cMinColor = pAlphaImage->mfGet_minColor();
				const ColorF cMaxColor = pAlphaImage->mfGet_maxColor();

				// base range after normalization, fe. [0,1] for 8bit images, or [0,2^15] for RGBE/HDR data
				float cUprValue = 1.0f;
				if (CImageExtensionHelper::IsDynamicRange(eFormat))
					cUprValue = cMaxColor.a / HDR_UPPERNORM;

				// original range before normalization, fe. [0,1.83567]
				cScaleA = (cMaxColor.r - cMinColor.r) / cUprValue;
				// original offset before normalization, fe. [0.0001204]
				cLowA = cMinColor.r;
			}
		}
	}

	if (cScaleR != 1.0f || cScaleG != 1.0f || cScaleB != 1.0f || cScaleA != 1.0f ||
	    cLowR != 0.0f || cLowG != 0.0f || cLowB != 0.0f || cLowA != 0.0f)
	{
		if (CImageExtensionHelper::IsDynamicRange(eFormat))
			imageFlags &= ~FIM_SRGB_READ;

		if (imageFlags & FIM_SRGB_READ)
		{
			for (i32 s = 0; s < (imageWidth * nHorizontalFaces * imageHeight * nVerticalFaces * 4); s += 4)
			{
				pDecompBytes[s + 0] = std::min((u8)255, u8(LinearToGamma(GammaToLinear(pDecompBytes[s + 0] / LDR_UPPERNORM) * cScaleR + cLowR) * LDR_UPPERNORM + 0.5f));
				pDecompBytes[s + 1] = std::min((u8)255, u8(LinearToGamma(GammaToLinear(pDecompBytes[s + 1] / LDR_UPPERNORM) * cScaleG + cLowG) * LDR_UPPERNORM + 0.5f));
				pDecompBytes[s + 2] = std::min((u8)255, u8(LinearToGamma(GammaToLinear(pDecompBytes[s + 2] / LDR_UPPERNORM) * cScaleB + cLowB) * LDR_UPPERNORM + 0.5f));
				pDecompBytes[s + 3] = std::min((u8)255, u8(LinearToGamma(GammaToLinear(pDecompBytes[s + 3] / LDR_UPPERNORM) * cScaleA + cLowA) * LDR_UPPERNORM + 0.5f));
			}
		}
		else
		{
			for (i32 s = 0; s < (imageWidth * nHorizontalFaces * imageHeight * nVerticalFaces * 4); s += 4)
			{
				pDecompBytes[s + 0] = std::min((u8)255, u8(pDecompBytes[s + 0] * cScaleR + cLowR * LDR_UPPERNORM + 0.5f));
				pDecompBytes[s + 1] = std::min((u8)255, u8(pDecompBytes[s + 1] * cScaleG + cLowG * LDR_UPPERNORM + 0.5f));
				pDecompBytes[s + 2] = std::min((u8)255, u8(pDecompBytes[s + 2] * cScaleB + cLowB * LDR_UPPERNORM + 0.5f));
				pDecompBytes[s + 3] = std::min((u8)255, u8(pDecompBytes[s + 3] * cScaleA + cLowA * LDR_UPPERNORM + 0.5f));
			}
		}
	}

	bool hasAlpha = (eAttachedFormat != eTF_Unknown) /*|| CImageExtensionHelper::HasAlphaForTextureFormat(eFormat)*/;
	for (i32 s = 0; s < (imageWidth * nHorizontalFaces * imageHeight * nVerticalFaces); s += 4)
		hasAlpha |= (pDecompBytes[s + 3] != 0xFF);

	//////////////////////////////////////////////////////////////////////////
	string strFormat = NameForTextureFormat(eFormat);
	string mips;
	mips.Format(" Mips:%d", numMips);

	if (eAttachedFormat != eTF_Unknown)
	{
		strFormat += " + ";
		strFormat += NameForTextureFormat(eAttachedFormat);
	}

	strFormat += mips;

	// Check whether it's gamma-corrected or not and add a description accordingly.
	if (imageFlags & FIM_SRGB_READ)
		strFormat += ", SRGB/Gamma corrected";
	if (imageFlags & FIM_RENORMALIZED_TEXTURE)
		strFormat += ", Renormalized";
	if (IsLimitedHDR(eFormat))
		strFormat += ", HDR";

	outImage.SetFormatDescription(strFormat);
	outImage.SetNumberOfMipMaps(numMips);
	outImage.SetHasAlphaChannel(hasAlpha);
	outImage.SetIsLimitedHDR(IsLimitedHDR(eFormat));
	outImage.SetIsCubemap(boIsCubemap);
	outImage.SetFormat(eFormat);
	outImage.SetSRGB(imageFlags & FIM_SRGB_READ);

	// done reading file
	return bOk;
}

//////////////////////////////////////////////////////////////////////////
i32 CImage_DXTC::TextureDataSize(i32 nWidth, i32 nHeight, i32 nDepth, i32 nMips, ETEX_Format eTF)
{
	if (eTF == eTF_Unknown)
		return 0;

	if (nMips <= 0)
		nMips = 0;
	i32 nSize = 0;
	i32 nM = 0;
	while (nWidth || nHeight || nDepth)
	{
		if (!nWidth)
			nWidth = 1;
		if (!nHeight)
			nHeight = 1;
		if (!nDepth)
			nDepth = 1;
		nM++;

		i32 nSingleMipSize;
		if (IsBlockCompressed(eTF))
		{
			i32 blockSize = CImageExtensionHelper::BytesPerBlock(eTF);
			nSingleMipSize = ((nWidth + 3) / 4) * ((nHeight + 3) / 4) * nDepth * blockSize;
		}
		else
			nSingleMipSize = nWidth * nHeight * nDepth * BitsPerPixel(eTF) / 8;
		nSize += nSingleMipSize;

		nWidth >>= 1;
		nHeight >>= 1;
		nDepth >>= 1;
		if (nMips == nM)
			break;
	}
	//assert (nM == nMips);

	return nSize;

}

//////////////////////////////////////////////////////////////////////////
CImage_DXTC::COMPRESSOR_ERROR CImage_DXTC::CheckParameters(
  i32 width,
  i32 height,
  CImage_DXTC::UNCOMPRESSED_FORMAT destinationFormat,
  ukk sourceData,
  uk destinationData,
  i32 destinationDataSize)
{
	i32k blockWidth = 4;
	i32k blockHeight = 4;

	i32k bgraPixelSize = 4 * sizeof(u8);
	i32k bgraRowSize = bgraPixelSize * width;

	if ((width <= 0) || (height <= 0) || (!sourceData))
	{
		return COMPRESSOR_ERROR_NO_INPUT_DATA;
	}

	if ((width % blockWidth) || (height % blockHeight))
	{
		return COMPRESSOR_ERROR_GENERIC;
	}

	if ((destinationData == 0) || (destinationDataSize <= 0))
	{
		return COMPRESSOR_ERROR_NO_OUTPUT_POINTER;
	}

	if (destinationFormat != FORMAT_ARGB_8888)
	{
		return COMPRESSOR_ERROR_UNSUPPORTED_DESTINATION_FORMAT;
	}

	if ((height * bgraRowSize <= 0) || (height * bgraRowSize > destinationDataSize))
	{
		return COMPRESSOR_ERROR_GENERIC;
	}

	return COMPRESSOR_ERROR_NONE;
}

