// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __raster_h__
#define __raster_h__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/CoreX/Math/Drx_Math.h>
#include "../LightmapCompiler/SimpleTriangleRasterizer.h"

struct SMatChunk
{
	Matrix34 localTM;
	u32   startIndex;
	i32    num;
	i32    matIndex;
	u32   posStride;
	vtx_idx* pIndices;
	u8*   pPositions;
	SMatChunk()
		: startIndex(0), num(0), matIndex(-1), posStride(sizeof(Vec3)), pIndices(NULL), pPositions(NULL)
	{
		localTM.SetIdentity();
	}
};

//interface for an optional triangle validator getting the triangle world positions passed
struct ITriangleValidator
{
	virtual const bool ValidateTriangle(const Vec3 crA, const Vec3 crB, const Vec3 crC) const { return true; }
};

//class containing a XY, ZX and ZY raster-bit-table and the matrix which transforms into it
//object is transformed into world space without offset
//a non uniform scale is applied to fit all 3 raster tables
class CBBoxRaster
{
public:
	static u32k scRasterRes = 32;
	enum EBufferSel
	{
		eBS_XY,
		eBS_ZY,
		eBS_ZX
	};

	CBBoxRaster();  //default ctor
	~CBBoxRaster(); //dtor

	//project once, each subsequent call will invalidate results from before
	void ExecuteProjectTrianglesOS
	(
	  const Vec3& crWorldMin,
	  const Vec3& crWorldMax,
	  const Matrix34& crOSToWorldRotMatrix,
	  const std::vector<SMatChunk>& crChunks,
	  bool useConservativeFill,
	  const ITriangleValidator* cpTriangleValidator
	);//projects triangles into scaled object space raster tables

	//returns result of ray-material query
	const bool    RayTriangleIntersection(const Vec3& crWSOrigin, const Vec3& crWSDir, const bool cOffsetXY, const float cStepWidth = 1.5f) const;

	const bool    GetEntry(u32k* const cpBuffer, u32k cX, u32k cY) const; //returns the value in the corresponding table

	u32k  GetEntryExact(u32k* const cpBuffer, u32k cX, u32k cY) const;  //returns the exact bit in the corresponding table

	void          SetEntry(u32* pBuffer, u32k cX, u32k cY, const bool cFillModeNegative); //sets the value in the corresponding table

	void          SaveTGA(tukk cpName) const;

	u32k* GetBuffer(const EBufferSel cBufSel);

	const bool    RetrieveXYBBox(AABB& rXYBox, const float cHeight = 1.f, const bool cUseDefault = false) const;//retrieves height on bottom of object

	//uses ray aabb intersection to find entry point into bounding box
	//return false if failed
	static void OffsetSampleAABB(const Vec3& crOrigin, const AABB& crAABB, Vec3& rNewOrigin);

	//project once, each subsequent call will invalidate results from before
	void ProjectTrianglesOS
	(
	  const Vec3 cWorldMin,
	  const Vec3 cWorldMax,
	  const Matrix34& crOSToWorldRotMatrix,
	  const std::vector<SMatChunk>& crChunks,
	  bool useConservativeFill,
	  const ITriangleValidator* cpTriangleValidator
	);//projects triangles into scaled object space raster tables

private:
	u32             m_pRasterXY[(scRasterRes * scRasterRes) >> 2]; //XY raster bit table, 2 bit per entry
	u32             m_pRasterZY[(scRasterRes * scRasterRes) >> 2]; //ZY raster bit table, 2 bit per entry
	u32             m_pRasterZX[(scRasterRes * scRasterRes) >> 2]; //ZX raster bit table, 2 bit per entry
	Vec3               m_WorldToOSTrans;//subtract that
	Vec3               m_OSScale;//scale to fit the whole raster tables

	static const float cMaxExt;
};

inline CBBoxRaster::CBBoxRaster()
{}

inline CBBoxRaster::~CBBoxRaster()
{}

inline const bool CBBoxRaster::GetEntry(u32k* const cpBuffer, u32k cX, u32k cY) const
{
	//return true if on either negative or positive side
	assert(cX < scRasterRes && cY < scRasterRes);
	u32k cIndex = (cY * scRasterRes + cX);
	u32k cMask = (1 << ((cIndex & 15) << 1)) | (1 << (((cIndex & 15) << 1) + 1));
	return ((cpBuffer[cIndex >> 4] & cMask) != 0);
}

inline u32k CBBoxRaster::GetEntryExact(u32k* const cpBuffer, u32k cX, u32k cY) const
{
	//return 0 if not entry, 1 if negative and 2 if positive
	assert(cX < scRasterRes && cY < scRasterRes);
	u32k cIndex = (cY * scRasterRes + cX);
	u32k cMask0 = (1 << ((cIndex & 15) << 1));
	u32k cMask1 = (1 << (((cIndex & 15) << 1) + 1));
	return (((cpBuffer[cIndex >> 4] & cMask0) != 0) ?
	        ((cpBuffer[cIndex >> 4] & cMask1) != 0) ? (1 + 2) : 1 : ((cpBuffer[cIndex >> 4] & cMask1) != 0) ? 2 : 0);
}

inline void CBBoxRaster::SetEntry(u32* pBuffer, u32k cX, u32k cY, const bool cFillModeNegative)
{
	assert(cX < scRasterRes && cY < scRasterRes);
	u32k cIndex = (cY * scRasterRes + cX);
	u32k cBufIndex = cIndex >> 4;
	if (cFillModeNegative)
		pBuffer[cBufIndex] |= (1 << ((cIndex & 15) << 1));
	else
		pBuffer[cBufIndex] |= (1 << (((cIndex & 15) << 1) + 1));
}

inline u32k* CBBoxRaster::GetBuffer(const CBBoxRaster::EBufferSel cBufSel)
{
	return (cBufSel == eBS_XY) ? m_pRasterXY : (cBufSel == eBS_ZY) ? m_pRasterZY : m_pRasterZX;
}

//-------------------------------------------------------------------------------------------------------

class CRasterTriangleSink : public CSimpleTriangleRasterizer::IRasterizeSink
{
public:
	CRasterTriangleSink(u32* pBuf, CBBoxRaster& rBBoxRaster);
	virtual void Line(const float, const float, i32k cIniLeft, i32k cIniRight, i32k cLineY);
	void         SetFillMode(const bool cNegative);

private:
	u32*      m_pBuf;        //pointer to buffer where to set the entries
	CBBoxRaster& m_rBBoxRaster; //reference to box raster setting the bit entries
	bool         m_FillMode;    //fill mode: either 1 or 2 according to position of triangle
};

inline CRasterTriangleSink::CRasterTriangleSink(u32* pBuf, CBBoxRaster& rBBoxRaster)
	: m_pBuf(pBuf), m_rBBoxRaster(rBBoxRaster), m_FillMode(false)
{}

inline void CRasterTriangleSink::Line
(
  const float,
  const float,
  i32k cIniLeft,
  i32k cIniRight,
  i32k cLineY
)
{
	for (i32 x = cIniLeft; x < cIniRight; ++x)
		m_rBBoxRaster.SetEntry(m_pBuf, x, cLineY, m_FillMode);
}

inline void CRasterTriangleSink::SetFillMode(const bool cNegative)
{
	m_FillMode = cNegative;
}

#endif // __raster_h__

