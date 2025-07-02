// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct SEditorPaintBrush;

// RGB terrain texture layer
// internal structure (tiled based, loading tiles on demand) is hidden from outside
// texture does not have to be square
// cache uses "last recently used" strategy to remain within set memory bounds (during terrain texture generation)
class SANDBOX_API CRGBLayer
{
public:
	// constructor
	CRGBLayer(tukk szFilename);
	// destructor
	virtual ~CRGBLayer();

	// Serialization
	void Serialize(XmlNodeRef& node, bool bLoading);

	// might throw an exception if memory is low but we only need very few bytes
	// Arguments:
	//   dwTileCountX - must not be 0
	//   dwTileCountY - must not be 0
	//   dwTileResolution must be power of two
	void AllocateTiles(u32k dwTileCountX, u32k dwTileCountY, u32k dwTileResolution);

	//
	void FreeData();

	// calculated the texture resolution needed to capture all details in the texture
	u32 CalcMinRequiredTextureExtend();

	u32 GetTileCountX() const { return m_dwTileCountX; }
	u32 GetTileCountY() const { return m_dwTileCountY; }

	// might need to load the tile once
	// Return:
	//   might be 0 if no tile exists at this point
	u32 GetTileResolution(u32k dwTileX, u32k dwTileY);

	// Arguments:
	//   dwTileX -  0..GetTileCountX()-1
	//   dwTileY -  0..GetTileCountY()-1
	//   bRaise - true=raise, flase=lower
	bool ChangeTileResolution(u32k dwTileX, u32k dwTileY, u32 dwNewSize);

	// 1:1
	void SetSubImageRGBLayer(u32k dwDstX, u32k dwDstY, const CImageEx& rTileImage);

	void SetSubImageStretched(const float fSrcLeft, const float fSrcTop, const float fSrcRight, const float fSrcBottom, CImageEx& rInImage, const bool bFiltered = false);

	void SetSubImageTransformed(const CImageEx* pImage, const Matrix34& transform);

	// stretched (dwSrcWidth,dwSrcHeight)->rOutImage
	// Arguments:
	//   fSrcLeft - 0..1
	//   fSrcTop - 0..1
	//   fSrcRight - 0..1, <fSrcLeft
	//   fSrcBottom - 0..1, <fSrcTop
	//	Notice: is build as follows: blue | green << 8 | red << 16, alpha unused
	void GetSubImageStretched(const float fSrcLeft, const float fSrcTop, const float fSrcRight, const float fSrcBottom, CImageEx& rOutImage, const bool bFiltered = false);

	// Extracts a stretched subimage from the layer to an image with the same pixel format. Faster, no filtering.
	// Arguments:
	//   fSrcLeft - 0..1
	//   fSrcTop - 0..1
	//   fSrcRight - 0..1, < fSrcLeft
	//   fSrcBottom - 0..1, < fSrcTop
	void GetSubimageFast(const float fSrcLeft, const float fSrcTop, const float fSrcRight, const float fSrcBottom, CImageEx& rOutImage);

	// Stretches an image with the same pixel format as the layer into a subrect of the layer. Faster, no filtering.
	// Arguments:
	//   fDestLeft - 0..1
	//   fDestTop - 0..1
	//   fDestRight - 0..1, < fSrcLeft
	//   fDestBottom - 0..1, < fSrcTop
	void SetSubimageFast(const float fDestLeft, const float fDestTop, const float fDestRight, const float fDestBottom, CImageEx& rSrcImage);

	// Paint spot with pattern (to an RGB image)
	// Arguments:
	//   fpx - 0..1 in the texture
	//   fpy - 0..1 in the texture
	void PaintBrushWithPatternTiled(const float fpx, const float fpy, SEditorPaintBrush& brush, const CImageEx& imgPattern);

	// needed for Layer Painting
	// Arguments:
	//   fSrcLeft - usual range: [0..1] but full range is valid
	//   fSrcTop - usual range: [0..1] but full range is valid
	//   fSrcRight - usual range: [0..1] but full range is valid
	//   fSrcBottom - usual range: [0..1] but full range is valid
	// Return:
	//   might be 0 if no tile was touched
	u32 CalcMaxLocalResolution(const float fSrcLeft, const float fSrcTop, const float fSrcRight, const float fSrcBottom);

	bool   ImportExportBlock(tukk pFileName, i32 nSrcLeft, i32 nSrcTop, i32 nSrcRight, i32 nSrcBottom, u32* outSquare, bool bIsImport = false);
	bool   ExportSegment(CMemoryBlock& mem, u32 dwTileX, u32 dwTileY, bool bCompress = true);

	//
	bool IsAllocated() const;

	void GetMemoryUsage(IDrxSizer* pSizer);

	// useful to detect problems before saving (e.g. file is read-only)
	bool WouldSaveSucceed();

	// offsets texture by (x,y) tiles
	void Offset(i32 iTilesX, i32 iTilesY);

	// forces all tiles to be loaded into memory, violating max mem limit if necessary
	// used when converting normal level into segmented format
	void LoadAll();

	// similar to AllocTiles() but preserves the image
	// if dwTileCountX and dwTileCountY already match, will do nothing
	void Resize(u32 dwTileCountX, u32 dwTileCountY, u32 dwTileResolution);

	// can be called from outside to save memory
	void CleanupCache();

	// writes out a debug images to the current directory
	void Debug();

	// rarely needed editor operation - n terrain texture tiles become 4*n
	// slow
	// Returns:
	//   success
	bool RefineTiles();

	// slow but convenient (will be filtered one day)
	// Arguments:
	//   fpx - 0..1
	//   fpx - 0..1
	u32 GetFilteredValueAt(const float fpx, const float fpy);
	u32 GetUnfilteredValueAt(const float fpx, const float fpy);

	// slow but convenient (will be filtered one day)
	// Arguments:
	//   fpx - 0..1
	//   fpy - 0..1
	u32 GetValueAt(const float fpx, const float fpy);

	// Arguments:
	//   dwX - [0..GetTextureWidth()]
	//   dwY - [0..GetTextureHeight()]
	void SetValueAt(u32k dwX, u32k dwY, u32k dwValue);

	// slow but convenient (will be filtered one day)
	// Arguments:
	//   fpx - 0..1
	//   fpy - 0..1
	void      SetValueAt(const float fpx, const float fpy, u32k dwValue);

	CImageEx* GetTileImage(i32 tileX, i32 tileY, bool setDirtyFlag = true);
	void      UnloadTile(i32 tileX, i32 tileY);

private:
	class CTerrainTextureTiles
	{
	public:
		// default constructor
		CTerrainTextureTiles() : m_pTileImage(0), m_bDirty(false), m_dwSize(0)
		{
		}

		CImageEx*  m_pTileImage;          // 0 if not loaded, otherwise pointer to the image (m_dwTileResolution,m_dwTileResolution)
		CTimeValue m_timeLastUsed;        // needed for "last recently used" strategy
		bool       m_bDirty;              // true=tile needs to be written to disk, needed for "last recently used" strategy
		u32     m_dwSize;              // only valid if m_dwSize!=0, if not valid you need to call LoadTileIfNeeded()
	};

private:
	bool OpenPakForLoading();
	bool ClosePakForLoading();
	bool SaveAndFreeMemory(const bool bForceFileCreation = false);

	bool GetValueAt(const float fpx, const float fpy, u32*& value);

	// Arguments:
	//   dwTileX - 0..m_dwTileCountX
	//   dwTileY - 0..m_dwTileCountY
	//   bNoGarbageCollection - do not garbage collect (used by LoadAll())
	// Return:
	//   might be 0 if no tile exists at this point
	CTerrainTextureTiles* LoadTileIfNeeded(u32k dwTileX, u32k dwTileY, bool bNoGarbageCollection = false);

	// Arguments:
	//   dwTileX - 0..m_dwTileCountX
	//   dwTileY - 0..m_dwTileCountY
	// Return:
	//   might be 0 if no tile exists at this point
	CTerrainTextureTiles* GetTilePtr(u32k dwTileX, u32k dwTileY);

	void                  FreeTile(CTerrainTextureTiles& rTile);

	// Return:
	//   might be 0
	CTerrainTextureTiles* FindOldestTileToFree();

	// removed tiles till we reach the limit
	void ConsiderGarbageCollection();

	void ClipTileRect(CRect& inoutRect) const;

	// Return:
	//   true = save needed
	bool   IsDirty() const;

	string GetFullFileName();

private:
	std::vector<CTerrainTextureTiles> m_TerrainTextureTiles;            // [x+y*m_dwTileCountX]

	u32                            m_dwTileCountX;                   //
	u32                            m_dwTileCountY;                   //
	u32                            m_dwTileResolution;               // must be power of two, tiles are square in size
	u32                            m_dwCurrentTileMemory;            // to detect if GarbageCollect is needed
	bool                              m_bPakOpened;                     // to optimize redundant OpenPak an ClosePak
	bool                              m_bInfoDirty;                     // true=save needed e.g. internal properties changed
	bool                              m_bNextSerializeForceSizeSave;    //

	static u32k               m_dwMaxTileMemory = 1024 * 1024 * 1024; // Stall free support for up to 16k x 16k terrain texture
	string                            m_TerrainRGBFileName;
};

