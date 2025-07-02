// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Font/StdAfx.h>
#include <drx3D/Font/GlyphCache.h>
#include <drx3D/Font/FontTexture.h>

//-------------------------------------------------------------------------------------------------
CGlyphCache::CGlyphCache()
	: m_iGlyphBitmapWidth(0)
	, m_iGlyphBitmapHeight(0)
	, m_fSizeRatio(0.8f)
	, m_iSmoothMethod(0)
	, m_iSmoothAmount(0)
	, m_pScaleBitmap(nullptr)
	, m_dwUsage(1)
{
}

//-------------------------------------------------------------------------------------------------
CGlyphCache::~CGlyphCache()
{
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphCache::Create(i32 iCacheSize, i32 iGlyphBitmapWidth, i32 iGlyphBitmapHeight, i32 iSmoothMethod, i32 iSmoothAmount, float fSizeRatio)
{
	m_fSizeRatio = fSizeRatio;

	m_iSmoothMethod = iSmoothMethod;
	m_iSmoothAmount = iSmoothAmount;

	m_iGlyphBitmapWidth = iGlyphBitmapWidth;
	m_iGlyphBitmapHeight = iGlyphBitmapHeight;

	if (!CreateSlotList(iCacheSize))
	{
		ReleaseSlotList();

		return 0;
	}

	i32 iScaledGlyphWidth = 0;
	i32 iScaledGlyphHeight = 0;

	switch (m_iSmoothMethod)
	{
	case FONT_SMOOTH_SUPERSAMPLE:
		{
			switch (m_iSmoothAmount)
			{
			case FONT_SMOOTH_AMOUNT_2X:
				iScaledGlyphWidth = m_iGlyphBitmapWidth << 1;
				iScaledGlyphHeight = m_iGlyphBitmapHeight << 1;
				break;
			case FONT_SMOOTH_AMOUNT_4X:
				iScaledGlyphWidth = m_iGlyphBitmapWidth << 2;
				iScaledGlyphHeight = m_iGlyphBitmapHeight << 2;
				break;
			}
		}
		break;
	}

	if (iScaledGlyphWidth)
	{
		m_pScaleBitmap = new CGlyphBitmap;

		if (!m_pScaleBitmap)
		{
			Release();

			return 0;
		}

		if (!m_pScaleBitmap->Create(iScaledGlyphWidth, iScaledGlyphHeight))
		{
			Release();

			return 0;
		}

		m_pFontRenderer.SetGlyphBitmapSize(iScaledGlyphWidth, iScaledGlyphHeight);
	}
	else
	{
		//		m_pFontRenderer.SetGlyphBitmapSize(m_iGlyphBitmapWidth, m_iGlyphBitmapHeight);
		// we assume the font is square - but the texture we want to store it might not be
		//		m_pFontRenderer.SetGlyphBitmapSize(m_iGlyphBitmapHeight, m_iGlyphBitmapHeight);
		m_pFontRenderer.SetGlyphBitmapSize(32, 32);
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphCache::Release()
{
	ReleaseSlotList();

	m_pCacheTable.clear();

	if (m_pScaleBitmap)
	{
		m_pScaleBitmap->Release();

		delete m_pScaleBitmap;

		m_pScaleBitmap = 0;
	}

	m_iGlyphBitmapWidth = 0;
	m_iGlyphBitmapHeight = 0;
	m_fSizeRatio = 0.8f;

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphCache::LoadFontFromFile(const string& szFileName)
{
	return m_pFontRenderer.LoadFromFile(szFileName);
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphCache::LoadFontFromMemory(u8* pFileBuffer, i32 iDataSize)
{
	return m_pFontRenderer.LoadFromMemory(pFileBuffer, iDataSize);
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphCache::ReleaseFont()
{
	m_pFontRenderer.Release();

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphCache::GetGlyphBitmapSize(i32* pWidth, i32* pHeight)
{
	if (pWidth)
	{
		*pWidth = m_iGlyphBitmapWidth;
	}

	if (pHeight)
	{
		*pHeight = m_iGlyphBitmapWidth;
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphCache::PreCacheGlyph(u32 cChar)
{
	CCacheTable::iterator pItor = m_pCacheTable.find(cChar);

	if (pItor != m_pCacheTable.end())
	{
		pItor->second->dwUsage = m_dwUsage;

		return 1;
	}

	CCacheSlot* pSlot = GetLRUSlot();

	if (!pSlot)
	{
		return 0;
	}

	if (pSlot->dwUsage > 0)
	{
		UnCacheGlyph(pSlot->cCurrentChar);
	}

	if (m_pScaleBitmap)
	{
		i32 iOffsetMult = 1;

		switch (m_iSmoothAmount)
		{
		case FONT_SMOOTH_AMOUNT_2X:
			iOffsetMult = 2;
			break;
		case FONT_SMOOTH_AMOUNT_4X:
			iOffsetMult = 4;
			break;
		}

		m_pScaleBitmap->Clear();

		if (!m_pFontRenderer.GetGlyph(m_pScaleBitmap, &pSlot->iCharWidth, &pSlot->iCharHeight, pSlot->iCharOffsetX, pSlot->iCharOffsetY, 0, 0, cChar))
		{
			return 0;
		}

		pSlot->iCharWidth >>= iOffsetMult >> 1;
		pSlot->iCharHeight >>= iOffsetMult >> 1;

		m_pScaleBitmap->BlitScaledTo8(pSlot->pGlyphBitmap.GetBuffer(), 0, 0, m_pScaleBitmap->GetWidth(), m_pScaleBitmap->GetHeight(), 0, 0, pSlot->pGlyphBitmap.GetWidth(), pSlot->pGlyphBitmap.GetHeight(), pSlot->pGlyphBitmap.GetWidth());
	}
	else
	{
		if (!m_pFontRenderer.GetGlyph(&pSlot->pGlyphBitmap, &pSlot->iCharWidth, &pSlot->iCharHeight, pSlot->iCharOffsetX, pSlot->iCharOffsetY, 0, 0, cChar))
		{
			return 0;
		}
	}

	if (m_iSmoothMethod == FONT_SMOOTH_BLUR)
	{
		pSlot->pGlyphBitmap.Blur(m_iSmoothAmount);
	}

	pSlot->dwUsage = m_dwUsage;
	pSlot->cCurrentChar = cChar;

	m_pCacheTable.insert(std::pair<u32, CCacheSlot*>(cChar, pSlot));

	return 1;
}

i32 CGlyphCache::UnCacheGlyph(u32 cChar)
{
	CCacheTable::iterator pItor = m_pCacheTable.find(cChar);

	if (pItor != m_pCacheTable.end())
	{
		CCacheSlot* pSlot = pItor->second;

		pSlot->Reset();

		m_pCacheTable.erase(pItor);

		return 1;
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphCache::GlyphCached(u32 cChar)
{
	return (m_pCacheTable.find(cChar) != m_pCacheTable.end());
}

//-------------------------------------------------------------------------------------------------
CCacheSlot* CGlyphCache::GetLRUSlot()
{
	u32 dwMinUsage = 0xffffffff;
	CCacheSlot* pLRUSlot = 0;
	CCacheSlot* pSlot;

	CCacheSlotListItor pItor = m_pSlotList.begin();

	while (pItor != m_pSlotList.end())
	{
		pSlot = *pItor;

		if (pSlot->dwUsage == 0)
		{
			return pSlot;
		}
		else
		{
			if (pSlot->dwUsage < dwMinUsage)
			{
				pLRUSlot = pSlot;
				dwMinUsage = pSlot->dwUsage;
			}
		}

		++pItor;
	}

	return pLRUSlot;
}

//-------------------------------------------------------------------------------------------------
CCacheSlot* CGlyphCache::GetMRUSlot()
{
	u32 dwMaxUsage = 0;
	CCacheSlot* pMRUSlot = 0;
	CCacheSlot* pSlot;

	CCacheSlotListItor pItor = m_pSlotList.begin();

	while (pItor != m_pSlotList.end())
	{
		pSlot = *pItor;

		if (pSlot->dwUsage != 0)
		{
			if (pSlot->dwUsage > dwMaxUsage)
			{
				pMRUSlot = pSlot;
				dwMaxUsage = pSlot->dwUsage;
			}
		}

		++pItor;
	}

	return pMRUSlot;
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphCache::GetGlyph(CGlyphBitmap** pGlyph, i32* piWidth, i32* piHeight, char& iCharOffsetX, char& iCharOffsetY, u32 cChar)
{
	CCacheTable::iterator pItor = m_pCacheTable.find(cChar);

	if (pItor == m_pCacheTable.end())
	{
		if (!PreCacheGlyph(cChar))
		{
			return 0;
		}
	}

	pItor = m_pCacheTable.find(cChar);

	pItor->second->dwUsage = m_dwUsage++;
	(*pGlyph) = &pItor->second->pGlyphBitmap;

	if (piWidth)
	{
		*piWidth = pItor->second->iCharWidth;
	}

	if (piHeight)
	{
		*piHeight = pItor->second->iCharHeight;
	}

	iCharOffsetX = pItor->second->iCharOffsetX;
	iCharOffsetY = pItor->second->iCharOffsetY;

	return 1;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
i32 CGlyphCache::CreateSlotList(i32 iListSize)
{
	for (i32 i = 0; i < iListSize; i++)
	{
		CCacheSlot* pCacheSlot = new CCacheSlot;

		if (!pCacheSlot)
		{
			return 0;
		}

		if (!pCacheSlot->pGlyphBitmap.Create(m_iGlyphBitmapWidth, m_iGlyphBitmapHeight))
		{
			delete pCacheSlot;

			return 0;
		}

		pCacheSlot->Reset();

		pCacheSlot->iCacheSlot = i;

		m_pSlotList.push_back(pCacheSlot);
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphCache::ReleaseSlotList()
{
	CCacheSlotListItor pItor = m_pSlotList.begin();

	while (pItor != m_pSlotList.end())
	{
		(*pItor)->pGlyphBitmap.Release();

		delete (*pItor);

		pItor = m_pSlotList.erase(pItor);
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------
