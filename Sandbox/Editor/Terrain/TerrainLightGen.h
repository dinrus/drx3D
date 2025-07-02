// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __terrainlightgen_h__
#define __terrainlightgen_h__

#if _MSC_VER > 1000
	#pragma once
#endif

// forward declaration.
class CLayer;
struct LightingSettings;

/** Class that generates terrain surface texture for lighting
 */
class CTerrainLightGen
{
public: // -----------------------------------------------------------------------

	// default constructor
	CTerrainLightGen
	(
	  i32k cApplySS = 1,
	  CPakFile* m_pLevelPakFile = NULL,
	  const bool cUpdateIndirLighting = false
	);
	// destructor
	~CTerrainLightGen();

	// Generate terrain surface texture.
	// @param surfaceTexture Output image where texture will be stored, it must already be allocated.
	// to the size of surfaceTexture.
	// @param sector Terrain sector to generate texture for.
	// @param rect Region on the terrain where texture must be generated within sector..
	// @param pOcclusionSurfaceTexture optional occlusion surface texture
	// @return true if texture was generated.
	bool GenerateSectorTexture(CPoint sector, const CRect& rect, i32 flags, CImageEx& surfaceTexture);

	// might be valid (!=0, depending on mode) after GenerateSectorTexture(), release with ReleaseOcclusionSurfaceTexture()
	const CImageEx* GetOcclusionSurfaceTexture() const;

	//
	void ReleaseOcclusionSurfaceTexture();

	//! Invalidate all lighting valid flags for all sectors..
	void InvalidateLighting();

	//
	// Arguments:
	//   bFullInit - only needed (true) if m_hmap is used (not for exporting but for preview) - should be removed one day
	void Init(i32k resolution, const bool bFullInit);

	void GetSubImageStretched(const float fSrcLeft, const float fSrcTop, const float fSrcRight, const float fSrcBottom, CImageEx& rOutImage, i32k genFlags);

private: // ---------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// Sectors.
	//////////////////////////////////////////////////////////////////////////
	//! Get rectangle for this sector.
	void GetSectorRect(CPoint sector, CRect& rect);
	//! Get terrain sector info.
	i32& GetCLightGenSectorFlag(CPoint sector);
	//! Get terrain sector.
	i32  GetSectorFlags(CPoint sector);
	//! Add flags to sector.
	void SetSectorFlags(CPoint sector, i32 flags);

	//////////////////////////////////////////////////////////////////////////

	//! calculate the shadow (only hills)
	//! \param inpHeightmapData must not be 0
	//! \param iniResolution
	//! \param iniShift
	//! \param iniX [0..iniWidth-1]
	//! \param iniY [0..iniHeight-1]
	//! \param invSunVector normalized vector to the sun
	//! \param infShadowBlur defines the slope blurring, 0=no blurring, .. (angle would be better but is slower)
	//! \return 0..1
	float GetSunAmount(const float* inpHeightmapData, i32 iniX, i32 iniY, float infInvHeightScale,
	                   const Vec3& vSunShadowVector, const float infShadowBlur) const;

	//! use precalculated data, bilinear filtered
	//! \param iniX [0..m_resolution-1]
	//! \param iniY [0..m_resolution-1]
	//! \return 0.0f=no hemisphere visible .. 1.0f =full hemisphere visible
	float GetSkyAccessibilityFast(i32k iniX, i32k iniY) const;

	//! use precalculated data, bilinear filtered
	//! \param iniX [0..m_resolution-1]
	//! \param iniY [0..m_resolution-1]
	//! \return 0.0f=no sun visible .. 1.0f =full sun visible
	float GetSunAccessibilityFast(i32k iniX, i32k iniY) const;

	//! use precalculated data, bilinear filtered
	//! \param fX [0..1]
	//! \param fY [0..1]
	//! \return 0.0f=no hemisphere visible .. 1.0f =full hemisphere visible
	float GetSkyAccessibilityFloat(const float fX, const float fY) const;

	//! use precalculated data, bilinear filtered
	//! \param fX [0..1]
	//! \param fY [0..1]
	//! \return 0.0f=no sun visible .. 1.0f =full sun visible
	float GetSunAccessibilityFloat(const float fX, const float fY) const;

	// Calculate lightmap for sector.
	bool GenerateLightmap(CPoint sector, LightingSettings* ls, CImageEx& lightmap, i32 genFlags);
	void GenerateShadowmap(CPoint sector, CByteImage& shadowmap, float shadowAmmount, const Vec3& sunVector);
	void UpdateSectorHeightmap(CPoint sector);
	bool UpdateWholeHeightmap();
	//! Caluclate max terrain height (Optimizes calculation of shadows and sky accessibility).
	void CalcTerrainMaxZ();

	//! Log generation progress.
	void Log(tukk format, ...);

	//! you have to do it for the whole map and that can take a min but using the result is just a lookup
	//! upate m_SkyAccessiblity and m_SunAccessiblity
	//! \return true=success, false otherwise
	bool  RefreshAccessibility(const LightingSettings* inpLS, i32 genFlags);

	float CalcHeightScaleForLighting(const LightingSettings* pSettings, const DWORD indwTargetResolution) const;

	//////////////////////////////////////////////////////////////////////////
	// Vars.
	//////////////////////////////////////////////////////////////////////////

	bool             m_bLog;                            // true if ETTG_QUIET was not specified
	bool             m_bNotValid;
	u32     m_resolution;                      // target texture resolution

	u32     m_resolutionShift;                 // (1 << m_resolutionShift) == m_resolition.
	i32              m_sectorResolution;                // Resolution of sector.
	i32              m_numSectors;                      // Number of sectors per side.
	float            m_pixelSizeInMeters;               // Size of one pixel on generated texture in meters.
	float            m_terrainMaxZ;                     // Highest point on terrain. use CalcTerrainMaxZ() to update this value

	CHeightmap*      m_heightmap;                       // pointer to the editor heihgtmap (not scaled up to the target resolution)
	CImageEx         m_OcclusionSurfaceTexture;         // might be valid (!=0, depending on mode) after GenerateSectorTexture(), release with ReleaseOcclusionSurfaceTexture()

	std::vector<i32> m_sectorGrid;                // Sector grid.

	CFloatImage      m_hmap;                            // Local heightmap (scaled up to the target resolution)

	// Sky Accessiblity
	CByteImage m_SkyAccessiblity;                       // Amount of sky that is visible for each point (4MB for 2048x2048) (not scaled up from source, but might be scaled down), 0=no sky accessible, 255= full sky accessible
	i32        m_iCachedSkyAccessiblityQuality;         // to detect changes to refresh m_SkyAccessiblity
	//	i32								m_iCachedSkyBrightening;					// to detect changes to refresh m_SkyAccessiblity

	// Sun Accessibility
	CByteImage m_SunAccessiblity;                       // Amount of sun that is visible for each point (4MB for 2048x2048) (not scaled up from source, but might be scaled down), 0=no sun accessible, 255= full sun accessible
	//	i32								m_iCachedSunBlurLevel;						// to detect changes to refresh m_SunAccessiblity
	Vec3       m_vCachedSunDirection;                   // to detect changes to refresh m_SunAccessiblity

	CPakFile*  m_pLevelPakFile;                         // reference to PAK file

	u32     m_TerrainAccMapRes;                      // resolution of terrain accessibility map
	u8*     m_pTerrainAccMap;                        // terrain accessibility map generated by indirect lighting
	bool       m_UpdateIndirLighting;                   // true if to update indirect lighting
	bool       m_IndirLightingDataRetrieved;            // true if indirect lighting data have been retrieved
	i32        m_ApplySS;                               // quality for indirect lighting: apply super sampling to terrain occl
	//	float							m_TerrainGIClamp;									// clamp value for terrain IL, 0..1

	friend class CTerrainTexGen;
};

#endif // __terrainlightgen_h__

