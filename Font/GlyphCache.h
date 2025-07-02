// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <vector>
#include "GlyphBitmap.h"
#include "FontRenderer.h"
#include <drx3D/CoreX/StlUtils.h>

typedef struct CCacheSlot
{
	u32 dwUsage;
	i32          iCacheSlot;
	u32       cCurrentChar;

	u8        iCharWidth;          // size in pixel
	u8        iCharHeight;         // size in pixel
	char         iCharOffsetX;
	char         iCharOffsetY;

	CGlyphBitmap pGlyphBitmap;

	void Reset()
	{
		dwUsage = 0;
		cCurrentChar = ~0;

		iCharWidth = 0;
		iCharHeight = 0;
		iCharOffsetX = 0;
		iCharOffsetY = 0;

		pGlyphBitmap.Clear();
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(pGlyphBitmap);
	}

} CCacheSlot;

typedef std::unordered_map<u32, CCacheSlot*, stl::hash_uint32> CCacheTable;

typedef std::vector<CCacheSlot*>                                  CCacheSlotList;
typedef std::vector<CCacheSlot*>::iterator                        CCacheSlotListItor;

#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
	#undef GetCharWidth
	#undef GetCharHeight
#endif

class CGlyphCache
{
public:
	CGlyphCache();
	~CGlyphCache();

	i32         Create(i32 iCacheSize, i32 iGlyphBitmapWidth, i32 iGlyphBitmapHeight, i32 iSmoothMethod, i32 iSmoothAmount, float fSizeRatio = 0.8f);
	i32         Release();

	i32         LoadFontFromFile(const string& szFileName);
	i32         LoadFontFromMemory(u8* pFileBuffer, i32 iDataSize);
	i32         ReleaseFont();

	i32         SetEncoding(FT_Encoding pEncoding) { return m_pFontRenderer.SetEncoding(pEncoding); };
	FT_Encoding GetEncoding()                      { return m_pFontRenderer.GetEncoding(); };

	i32         GetGlyphBitmapSize(i32* pWidth, i32* pHeight);

	i32         PreCacheGlyph(u32 cChar);
	i32         UnCacheGlyph(u32 cChar);
	i32         GlyphCached(u32 cChar);

	CCacheSlot* GetLRUSlot();
	CCacheSlot* GetMRUSlot();

	i32         GetGlyph(CGlyphBitmap** pGlyph, i32* piWidth, i32* piHeight, char& iCharOffsetX, char& iCharOffsetY, u32 cChar);

	void        GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_pSlotList);
		//pSizer->AddContainer(m_pCacheTable);
		pSizer->AddObject(m_pScaleBitmap);
		pSizer->AddObject(m_pFontRenderer);
	}

private:

	i32 CreateSlotList(i32 iListSize);
	i32 ReleaseSlotList();

	CCacheSlotList m_pSlotList;
	CCacheTable    m_pCacheTable;

	i32            m_iGlyphBitmapWidth;
	i32            m_iGlyphBitmapHeight;
	float          m_fSizeRatio;

	i32            m_iSmoothMethod;
	i32            m_iSmoothAmount;

	CGlyphBitmap*  m_pScaleBitmap;

	CFontRenderer  m_pFontRenderer;

	u32   m_dwUsage;
};
