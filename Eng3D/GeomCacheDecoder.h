// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// ------------------------------------------------------------------------
//  Имя файла:   GeomCacheDecoder.h
//  Created:     23/8/2012 by Axel Gneiting
//  Описание: Decodes geom cache data
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef _GEOMCACHE_DECODER_
#define _GEOMCACHE_DECODER_

#pragma once

#if defined(USE_GEOM_CACHES)

	#include <drx3D/Eng3D/GeomCacheFileFormat.h>

class CGeomCache;
struct SGeomCacheRenderMeshUpdateContext;
struct SGeomCacheStaticMeshData;

struct SGeomCacheFrameHeader
{
	enum EFrameHeaderState
	{
		eFHS_Uninitialized = 0,
		eFHS_Undecoded     = 1,
		eFHS_Decoded       = 2
	};

	EFrameHeaderState m_state;
	u32            m_offset;
};

namespace GeomCacheDecoder
{
// Decodes an index frame
void DecodeIFrame(const CGeomCache* pGeomCache, tuk pData);

// Decodes a bi-directional predicted frame
void DecodeBFrame(const CGeomCache* pGeomCache, tuk pData, tuk pPrevFramesData[2],
                  tuk pFloorIndexFrameData, tuk pCeilIndexFrameData);

bool PrepareFillMeshData(SGeomCacheRenderMeshUpdateContext& updateContext, const SGeomCacheStaticMeshData& staticMeshData,
                         tukk & pFloorFrameMeshData, tukk & pCeilFrameMeshData, size_t& offsetToNextMesh, float& lerpFactor);

void FillMeshDataFromDecodedFrame(const bool bMotionBlur, SGeomCacheRenderMeshUpdateContext& updateContext,
                                  const SGeomCacheStaticMeshData& staticMeshData, tukk pFloorFrameMeshData,
                                  tukk pCeilFrameMeshData, float lerpFactor);

// Gets total needed space for uncompressing successive blocks
u32 GetDecompressBufferSize(tukk const pStartBlock, const uint numFrames);

// Decompresses one block of compressed data with header for input
bool DecompressBlock(const GeomCacheFile::EBlockCompressionFormat compressionFormat, tuk const pDest, tukk const pSource);

// Decompresses blocks of compressed data with headers for input and output
bool DecompressBlocks(const GeomCacheFile::EBlockCompressionFormat compressionFormat, tuk const pDest,
                      tukk const pSource, const uint blockOffset, const uint numBlocks, const uint numHandleFrames);

Vec3 DecodePosition(const Vec3& aabbMin, const Vec3& aabbSize, const GeomCacheFile::Position& inPosition, const Vec3& convertFactor);
Vec2 DecodeTexcoord(const GeomCacheFile::Texcoords& inTexcoords);
Quat DecodeQTangent(const GeomCacheFile::QTangent& inQTangent);

void TransformAndConvertToTangentAndBitangent(const Quat& rotation, const Quat& inQTangent, SPipTangents& outTangents);
void ConvertToTangentAndBitangent(const Quat& inQTangent, SPipTangents& outTangents);
};

#endif
#endif
