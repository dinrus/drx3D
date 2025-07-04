// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Font/StdAfx.h>
#include <drx3D/Font/FontTexture.h>
#include <drx3D/CoreX/String/UnicodeIterator.h>
#include <drx3D/CoreX/Platform/DrxWindows.h>

#if DRX_PLATFORM_WINDOWS
	#undef GetCharWidth
#endif

//-------------------------------------------------------------------------------------------------
CFontTexture::CFontTexture()
	: m_wSlotUsage(1), m_iWidth(0), m_iHeight(0), m_fInvWidth(0.0f), m_fInvHeight(0.0f), m_iCellWidth(0), m_iCellHeight(0),
	m_fTextureCellWidth(0), m_fTextureCellHeight(0), m_iWidthCellCount(0), m_iHeightCellCount(0), m_nTextureSlotCount(0),
	m_pBuffer(0), m_iSmoothMethod(FONT_SMOOTH_NONE), m_iSmoothAmount(FONT_SMOOTH_AMOUNT_NONE)
{
}

//-------------------------------------------------------------------------------------------------
CFontTexture::~CFontTexture()
{
	Release();
}

//-------------------------------------------------------------------------------------------------
i32 CFontTexture::CreateFromFile(const string& szFileName, i32 iWidth, i32 iHeight, i32 iSmoothMethod, i32 iSmoothAmount, float fSizeRatio, i32 iWidthCellCount, i32 iHeightCellCount)
{
	if (!m_pGlyphCache.LoadFontFromFile(szFileName))
	{
		Release();

		return 0;
	}

	if (!Create(iWidth, iHeight, iSmoothMethod, iSmoothAmount, fSizeRatio, iWidthCellCount, iHeightCellCount))
		return 0;

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CFontTexture::CreateFromMemory(u8* pFileData, i32 iDataSize, i32 iWidth, i32 iHeight, i32 iSmoothMethod, i32 iSmoothAmount, float fSizeRatio, i32 iWidthCellCount, i32 iHeightCellCount)
{
	if (!m_pGlyphCache.LoadFontFromMemory(pFileData, iDataSize))
	{
		Release();

		return 0;
	}

	if (!Create(iWidth, iHeight, iSmoothMethod, iSmoothAmount, fSizeRatio, iWidthCellCount, iHeightCellCount))
		return 0;

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CFontTexture::Create(i32 iWidth, i32 iHeight, i32 iSmoothMethod, i32 iSmoothAmount, float fSizeRatio, i32 iWidthCellCount, i32 iHeightCellCount)
{
	m_pBuffer = new FONT_TEXTURE_TYPE[iWidth * iHeight];
	if (!m_pBuffer)
		return 0;

	memset(m_pBuffer, 0, iWidth * iHeight * sizeof(FONT_TEXTURE_TYPE));

	if (!(iWidthCellCount * iHeightCellCount))
	{
		return 0;
	}

	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_fInvWidth = 1.0f / (float)iWidth;
	m_fInvHeight = 1.0f / (float)iHeight;

	m_iWidthCellCount = iWidthCellCount;
	m_iHeightCellCount = iHeightCellCount;
	m_nTextureSlotCount = m_iWidthCellCount * m_iHeightCellCount;

	m_iSmoothMethod = iSmoothMethod;
	m_iSmoothAmount = iSmoothAmount;

	m_iCellWidth = m_iWidth / m_iWidthCellCount;
	m_iCellHeight = m_iHeight / m_iHeightCellCount;

	m_fTextureCellWidth = m_iCellWidth * m_fInvWidth;
	m_fTextureCellHeight = m_iCellHeight * m_fInvHeight;

	if (!m_pGlyphCache.Create(FONT_GLYPH_CACHE_SIZE, m_iCellWidth, m_iCellHeight, iSmoothMethod, iSmoothAmount, fSizeRatio))
	{
		Release();

		return 0;
	}

	if (!CreateSlotList(m_nTextureSlotCount))
	{
		Release();

		return 0;
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CFontTexture::Release()
{
	delete[] m_pBuffer;
	m_pBuffer = 0;

	ReleaseSlotList();

	m_pSlotTable.clear();

	m_pGlyphCache.Release();

	m_iWidthCellCount = 0;
	m_iHeightCellCount = 0;
	m_nTextureSlotCount = 0;

	m_iWidth = 0;
	m_iHeight = 0;
	m_fInvWidth = 0.0f;
	m_fInvHeight = 0.0f;

	m_iCellWidth = 0;
	m_iCellHeight = 0;

	m_iSmoothMethod = 0;
	m_iSmoothAmount = 0;

	m_fTextureCellWidth = 0.0f;
	m_fTextureCellHeight = 0.0f;

	m_wSlotUsage = 1;

	return 1;
}

//-------------------------------------------------------------------------------------------------
u32 CFontTexture::GetSlotChar(i32 iSlot) const
{
	return m_pSlotList[iSlot]->cCurrentChar;
}

//-------------------------------------------------------------------------------------------------
CTextureSlot* CFontTexture::GetCharSlot(u32 cChar)
{
	CTextureSlotTable::iterator pItor = m_pSlotTable.find(cChar);

	if (pItor != m_pSlotTable.end())
		return pItor->second;

	return 0;
}

//-------------------------------------------------------------------------------------------------
CTextureSlot* CFontTexture::GetLRUSlot()
{
	u16 wMinSlotUsage = 0xffff;
	CTextureSlot* pLRUSlot = 0;
	CTextureSlot* pSlot;

	CTextureSlotList::iterator pItor = m_pSlotList.begin();

	while (pItor != m_pSlotList.end())
	{
		pSlot = *pItor;

		if (pSlot->wSlotUsage == 0)
		{
			return pSlot;
		}
		else
		{
			if (pSlot->wSlotUsage < wMinSlotUsage)
			{
				pLRUSlot = pSlot;
				wMinSlotUsage = pSlot->wSlotUsage;
			}
		}

		++pItor;
	}

	return pLRUSlot;
}

//-------------------------------------------------------------------------------------------------
CTextureSlot* CFontTexture::GetMRUSlot()
{
	u16 wMaxSlotUsage = 0;
	CTextureSlot* pMRUSlot = 0;
	CTextureSlot* pSlot;

	CTextureSlotList::iterator pItor = m_pSlotList.begin();

	while (pItor != m_pSlotList.end())
	{
		pSlot = *pItor;

		if (pSlot->wSlotUsage != 0)
		{
			if (pSlot->wSlotUsage > wMaxSlotUsage)
			{
				pMRUSlot = pSlot;
				wMaxSlotUsage = pSlot->wSlotUsage;
			}
		}

		++pItor;
	}

	return pMRUSlot;
}

//-------------------------------------------------------------------------------------------------
i32 CFontTexture::PreCacheString(tukk szString, i32* pUpdated)
{
	u16 wSlotUsage = m_wSlotUsage++;
	i32 iUpdated = 0;

	u32 cChar;
	for (Unicode::CIterator<tukk , false> it(szString); cChar = *it; ++it)
	{
		CTextureSlot* pSlot = GetCharSlot(cChar);

		if (!pSlot)
		{
			pSlot = GetLRUSlot();

			if (!pSlot)
				return 0;

			if (!UpdateSlot(pSlot->iTextureSlot, wSlotUsage, cChar))
				return 0;

			++iUpdated;
		}
		else
		{
			pSlot->wSlotUsage = wSlotUsage;
		}
	}

	if (pUpdated)
		*pUpdated = iUpdated;

	if (iUpdated)
		return 1;

	return 2;
}

//-------------------------------------------------------------------------------------------------
void CFontTexture::GetTextureCoord(CTextureSlot* pSlot, float texCoords[4],
                                   i32& iCharSizeX, i32& iCharSizeY, i32& iCharOffsetX, i32& iCharOffsetY) const
{
	if (!pSlot)
		return;     // expected behavior

	i32 iChWidth = pSlot->iCharWidth;
	i32 iChHeight = pSlot->iCharHeight;
	float slotCoord0 = pSlot->vTexCoord[0];
	float slotCoord1 = pSlot->vTexCoord[1];

	texCoords[0] = slotCoord0 - m_fInvWidth;                    // extra pixel for nicer bilinear filter
	texCoords[1] = slotCoord1 - m_fInvHeight;                   // extra pixel for nicer bilinear filter
	texCoords[2] = slotCoord0 + (float)iChWidth * m_fInvWidth;
	texCoords[3] = slotCoord1 + (float)iChHeight * m_fInvHeight;

	iCharSizeX = iChWidth + 1;                  // extra pixel for nicer bilinear filter
	iCharSizeY = iChHeight + 1;                 // extra pixel for nicer bilinear filter
	iCharOffsetX = pSlot->iCharOffsetX;
	iCharOffsetY = pSlot->iCharOffsetY;
}

//-------------------------------------------------------------------------------------------------
i32 CFontTexture::GetCharacterWidth(u32 cChar) const
{
	CTextureSlotTable::const_iterator pItor = m_pSlotTable.find(cChar);

	if (pItor == m_pSlotTable.end())
		return 0;

	const CTextureSlot& rSlot = *pItor->second;

	return rSlot.iCharWidth + 1;                    // extra pixel for nicer bilinear filter
}

//-------------------------------------------------------------------------------------------------
/*
   i32 CFontTexture::GetCharHeightByChar(wchar_t cChar)
   {
   CTextureSlotTable::iterator pItor = m_pSlotTable.find(cChar);

   if (pItor != m_pSlotTable.end())
   {
    return pItor->second->iCharHeight;
   }

   return 0;
   }
 */

//-------------------------------------------------------------------------------------------------
i32 CFontTexture::WriteToFile(const string& szFileName)
{
#if DRX_PLATFORM_WINDOWS
	FILE* hFile = fopen(szFileName.c_str(), "wb");

	if (!hFile)
		return 0;

	BITMAPFILEHEADER pHeader;
	BITMAPINFOHEADER pInfoHeader;

	memset(&pHeader, 0, sizeof(BITMAPFILEHEADER));
	memset(&pInfoHeader, 0, sizeof(BITMAPINFOHEADER));

	pHeader.bfType = 0x4D42;
	pHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_iWidth * m_iHeight * 3;
	pHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	pInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	pInfoHeader.biWidth = m_iWidth;
	pInfoHeader.biHeight = m_iHeight;
	pInfoHeader.biPlanes = 1;
	pInfoHeader.biBitCount = 24;
	pInfoHeader.biCompression = 0;
	pInfoHeader.biSizeImage = m_iWidth * m_iHeight * 3;

	fwrite(&pHeader, 1, sizeof(BITMAPFILEHEADER), hFile);
	fwrite(&pInfoHeader, 1, sizeof(BITMAPINFOHEADER), hFile);

	u8 cRGB[3];

	for (i32 i = m_iHeight - 1; i >= 0; i--)
	{
		for (i32 j = 0; j < m_iWidth; j++)
		{
			cRGB[0] = m_pBuffer[(i * m_iWidth) + j];
			cRGB[1] = *cRGB;

			cRGB[2] = *cRGB;

			fwrite(cRGB, 1, 3, hFile);
		}
	}

	fclose(hFile);
#endif
	return 1;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
i32 CFontTexture::CreateSlotList(i32 iListSize)
{
	i32 y, x;

	for (i32 i = 0; i < iListSize; i++)
	{
		CTextureSlot* pTextureSlot = new CTextureSlot;

		if (!pTextureSlot)
			return 0;

		pTextureSlot->iTextureSlot = i;
		pTextureSlot->Reset();

		y = i / m_iWidthCellCount;
		x = i % m_iWidthCellCount;

		pTextureSlot->vTexCoord[0] = (float)(x * m_fTextureCellWidth) + (0.5f / (float)m_iWidth);
		pTextureSlot->vTexCoord[1] = (float)(y * m_fTextureCellHeight) + (0.5f / (float)m_iHeight);

		m_pSlotList.push_back(pTextureSlot);
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CFontTexture::ReleaseSlotList()
{
	CTextureSlotList::iterator pItor = m_pSlotList.begin();

	while (pItor != m_pSlotList.end())
	{
		delete (*pItor);

		pItor = m_pSlotList.erase(pItor);
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CFontTexture::UpdateSlot(i32 iSlot, u16 wSlotUsage, u32 cChar)
{
	CTextureSlot* pSlot = m_pSlotList[iSlot];

	if (!pSlot)
		return 0;

	CTextureSlotTable::iterator pItor = m_pSlotTable.find(pSlot->cCurrentChar);

	if (pItor != m_pSlotTable.end())
		m_pSlotTable.erase(pItor);

	m_pSlotTable.insert(std::pair<wchar_t, CTextureSlot*>(cChar, pSlot));

	pSlot->wSlotUsage = wSlotUsage;
	pSlot->cCurrentChar = cChar;

	i32 iWidth = 0;
	i32 iHeight = 0;

	// blit the char glyph into the texture
	i32 x = pSlot->iTextureSlot % m_iWidthCellCount;
	i32 y = pSlot->iTextureSlot / m_iWidthCellCount;

	CGlyphBitmap* pGlyphBitmap;

	if (!m_pGlyphCache.GetGlyph(&pGlyphBitmap, &iWidth, &iHeight, pSlot->iCharOffsetX, pSlot->iCharOffsetY, cChar))
		return 0;

	pSlot->iCharWidth = iWidth;
	pSlot->iCharHeight = iHeight;

	pGlyphBitmap->BlitTo8(m_pBuffer, 0, 0,
	                      iWidth, iHeight, x * m_iCellWidth, y * m_iCellHeight, m_iWidth);

	return 1;
}

//-------------------------------------------------------------------------------------------------
void CFontTexture::CreateGradientSlot()
{
	CTextureSlot* pSlot = GetGradientSlot();
	assert(pSlot->cCurrentChar == (u32) ~0);    // 0 needs to be unused spot

	pSlot->Reset();
	pSlot->iCharWidth = m_iCellWidth - 2;
	pSlot->iCharHeight = m_iCellHeight - 2;
	pSlot->SetNotReusable();

	i32 x = pSlot->iTextureSlot % m_iWidthCellCount;
	i32 y = pSlot->iTextureSlot / m_iWidthCellCount;

	assert(sizeof(*m_pBuffer) == sizeof(u8));
	u8* pBuffer = &m_pBuffer[x * m_iCellWidth + y * m_iCellHeight * m_iWidth];

	for (u32 dwY = 0; dwY < pSlot->iCharHeight; ++dwY)
		for (u32 dwX = 0; dwX < pSlot->iCharWidth; ++dwX)
			pBuffer[dwX + dwY * m_iWidth] = dwY * 255 / (pSlot->iCharHeight - 1);
}

//-------------------------------------------------------------------------------------------------
CTextureSlot* CFontTexture::GetGradientSlot()
{
	return m_pSlotList[0];
}
