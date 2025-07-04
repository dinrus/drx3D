// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __terraingrid_h__
#define __terraingrid_h__

#if _MSC_VER > 1000
	#pragma once
#endif

/** Represent single terrain sector.
 */
class SANDBOX_API CTerrainSector
{
public:
	CTerrainSector()
	{
		textureId = 0;
	}

public:
	//! Id of surface texture.
	u32 textureId;
};

/** Manages grid of terrain sectors.
 */
class SANDBOX_API CTerrainGrid
{
public:
	// constructor
	CTerrainGrid();
	// destructor
	~CTerrainGrid();

	//! Initialize grid.
	void InitSectorGrid(i32 numSectors);

	//! Set target texture resolution.
	void SetResolution(i32 resolution);

	//! Get terrain sector.
	CTerrainSector* GetSector(CPoint sector);

	//! Lock texture of sector and return texture id.
	i32  LockSectorTexture(CPoint sector, u32k dwTextureResolution, bool& bTextureWasRecreated);
	void UnlockSectorTexture(CPoint sector);

	//////////////////////////////////////////////////////////////////////////
	// Coordinate conversions.
	//////////////////////////////////////////////////////////////////////////
	void   GetSectorRect(CPoint sector, CRect& rect);
	//! Map from sector coordinates to texture coordinates.
	CPoint SectorToTexture(CPoint sector);
	//! Map world position to sector space.
	CPoint WorldToSector(const Vec3& wp);
	//! Map sector coordinates to world coordinates.
	Vec3   SectorToWorld(CPoint sector);
	//! Map world position to texture space.
	Vec2   WorldToTexture(const Vec3& wp);

	//! Return number of sectors per side.
	i32  GetNumSectors() const { return m_numSectors; }

	void GetMemoryUsage(IDrxSizer* pSizer);

private: // ---------------------------------------------------------------

	std::vector<CTerrainSector*> m_sectorGrid;            // Sector grid.
	i32                          m_numSectors;            // Number of sectors per side.
	i32                          m_resolution;            // Resolution of texture and heightmap.
	i32                          m_sectorResolution;      // Sector texture size.
	i32                          m_sectorSize;            // Size of sector in meters.
	float                        m_pixelsPerMeter;        // Texture Pixels per meter.

	//! Clear all sectors.
	void ReleaseSectorGrid();
};

#endif // __terraingrid_h__

