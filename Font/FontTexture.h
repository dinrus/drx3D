// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "GlyphCache.h"
#include "GlyphBitmap.h"

typedef u8 FONT_TEXTURE_TYPE;

// the number of slots in the glyph cache
// each slot ocupies ((glyph_bitmap_width * glyph_bitmap_height) + 24) bytes
#define FONT_GLYPH_CACHE_SIZE (1)

// the glyph spacing in font texels between characters in proportional font mode (more correct would be to take the value in the character)
#define FONT_GLYPH_PROP_SPACING (1)

// the size of a rendered space, this value gets multiplied by the default characted width
#define FONT_SPACE_SIZE (0.5f)

// don't draw this char (used to avoid drawing color codes)
#define FONT_NOT_DRAWABLE_CHAR (0xffff)

// smoothing methods
#define FONT_SMOOTH_NONE        0
#define FONT_SMOOTH_BLUR        1
#define FONT_SMOOTH_SUPERSAMPLE 2

// smoothing amounts
#define FONT_SMOOTH_AMOUNT_NONE 0
#define FONT_SMOOTH_AMOUNT_2X   1
#define FONT_SMOOTH_AMOUNT_4X   2

typedef struct CTextureSlot
{
	u16 wSlotUsage;                // for LRU strategy, 0xffff is never released
	u32 cCurrentChar;              // ~0 if not used for characters
	i32    iTextureSlot;
	float  vTexCoord[2];              // character position in the texture (not yet half texel corrected)
	u8  iCharWidth;                // size in pixel
	u8  iCharHeight;               // size in pixel
	char   iCharOffsetX;
	char   iCharOffsetY;

	void Reset()
	{
		wSlotUsage = 0;
		cCurrentChar = ~0;
		iCharWidth = 0;
		iCharHeight = 0;
		iCharOffsetX = 0;
		iCharOffsetY = 0;
	}

	void SetNotReusable()
	{
		wSlotUsage = 0xffff;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const {}

} CTextureSlot;

typedef std::vector<CTextureSlot*>                                  CTextureSlotList;
typedef std::unordered_map<u32, CTextureSlot*, stl::hash_uint32> CTextureSlotTable;

#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
	#undef GetCharWidth
	#undef GetCharHeight
#endif

class CFontTexture
{
public:
	CFontTexture();
	~CFontTexture();

	i32                CreateFromFile(const string& szFileName, i32 iWidth, i32 iHeight, i32 iSmoothMethod, i32 iSmoothAmount, float fSizeRatio = 0.8f, i32 iWidthCharCount = 16, i32 iHeightCharCount = 16);
	i32                CreateFromMemory(u8* pFileData, i32 iDataSize, i32 iWidth, i32 iHeight, i32 iSmoothMethod, i32 iSmoothAmount, float fSizeRatio = 0.875f, i32 iWidthCharCount = 16, i32 iHeightCharCount = 16);
	i32                Create(i32 iWidth, i32 iHeight, i32 iSmoothMethod, i32 iSmoothAmount, float fSizeRatio = 0.8f, i32 iWidthCharCount = 16, i32 iHeightCharCount = 16);
	i32                Release();

	i32                SetEncoding(FT_Encoding pEncoding) { return m_pGlyphCache.SetEncoding(pEncoding); }
	FT_Encoding        GetEncoding()                      { return m_pGlyphCache.GetEncoding(); }

	i32                GetCellWidth()                     { return m_iCellWidth; }
	i32                GetCellHeight()                    { return m_iCellHeight; }

	i32                GetWidth()                         { return m_iWidth; }
	i32                GetHeight()                        { return m_iHeight; }

	i32                GetWidthCellCount()                { return m_iWidthCellCount; }
	i32                GetHeightCellCount()               { return m_iHeightCellCount; }

	float              GetTextureCellWidth()              { return m_fTextureCellWidth; }
	float              GetTextureCellHeight()             { return m_fTextureCellHeight; }

	FONT_TEXTURE_TYPE* GetBuffer()                        { return m_pBuffer; }

	u32             GetSlotChar(i32 iSlot) const;
	CTextureSlot*      GetCharSlot(u32 cChar);
	CTextureSlot*      GetGradientSlot();

	CTextureSlot*      GetLRUSlot();
	CTextureSlot*      GetMRUSlot();

	// returns 1 if texture updated, returns 2 if texture not updated, returns 0 on error
	// pUpdated is the number of slots updated
	i32 PreCacheString(tukk szString, i32* pUpdated = 0);
	// Arguments:
	//   pSlot - function does nothing if this pointer is 0
	void GetTextureCoord(CTextureSlot* pSlot, float texCoords[4], i32& iCharSizeX, i32& iCharSizeY, i32& iCharOffsetX, i32& iCharOffsetY) const;
	i32  GetCharacterWidth(u32 cChar) const;
	//	i32 GetCharHeightByChar(wchar_t cChar);

	// useful for special feature rendering interleaved with fonts (e.g. box behind the text)
	void CreateGradientSlot();

	i32  WriteToFile(const string& szFileName);

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_pGlyphCache);
		pSizer->AddObject(m_pSlotList);
		//pSizer->AddContainer(m_pSlotTable);
		pSizer->AddObject(m_pBuffer, m_iWidth * m_iHeight * sizeof(FONT_TEXTURE_TYPE));
	}

private: // ---------------------------------------------------------------

	i32 CreateSlotList(i32 iListSize);
	i32 ReleaseSlotList();
	i32 UpdateSlot(i32 iSlot, u16 wSlotUsage, u32 cChar);

	// --------------------------------

	i32                m_iWidth;                  // whole texture cache width
	i32                m_iHeight;                 // whole texture cache height

	float              m_fInvWidth;
	float              m_fInvHeight;

	i32                m_iCellWidth;
	i32                m_iCellHeight;

	float              m_fTextureCellWidth;
	float              m_fTextureCellHeight;

	i32                m_iWidthCellCount;
	i32                m_iHeightCellCount;

	i32                m_nTextureSlotCount;

	i32                m_iSmoothMethod;
	i32                m_iSmoothAmount;

	CGlyphCache        m_pGlyphCache;
	CTextureSlotList   m_pSlotList;
	CTextureSlotTable  m_pSlotTable;

	FONT_TEXTURE_TYPE* m_pBuffer;                 // [y*iWidth * x] x=0..iWidth-1, y=0..iHeight-1

	u16             m_wSlotUsage;
};
