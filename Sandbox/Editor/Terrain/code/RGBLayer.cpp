// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "RGBLayer.h"
#include "GameEngine.h"
#include "Util/PakFile.h"
#include <drx3D/Sys/ITimer.h>
#include "Layer.h"
#include <drx3D/CoreX/Memory/DrxSizer.h>
#include "Util/Clipboard.h"

#include "Terrain/TerrainManager.h"
#include "Util/ImagePainter.h"

#include "QT/Widgets/QWaitProgress.h"
#include "Controls/QuestionDialog.h"

//////////////////////////////////////////////////////////////////////////
CRGBLayer::CRGBLayer(tukk szFilename) : m_TerrainRGBFileName(szFilename), m_dwTileResolution(0), m_dwTileCountX(0), m_dwTileCountY(0),
	m_dwCurrentTileMemory(0), m_bPakOpened(false), m_bInfoDirty(false), m_bNextSerializeForceSizeSave(false)
{
	assert(szFilename);

	FreeData();
}

//////////////////////////////////////////////////////////////////////////
CRGBLayer::~CRGBLayer()
{
	FreeData();
}

void CRGBLayer::SetSubImageStretched(const float fSrcLeft, const float fSrcTop, const float fSrcRight, const float fSrcBottom, CImageEx& rInImage, const bool bFiltered)
{
	assert(fSrcLeft >= 0.0f && fSrcLeft <= 1.0f);
	assert(fSrcTop >= 0.0f && fSrcTop <= 1.0f);
	assert(fSrcRight >= 0.0f && fSrcRight <= 1.0f);
	assert(fSrcBottom >= 0.0f && fSrcBottom <= 1.0f);
	assert(fSrcRight >= fSrcLeft);
	assert(fSrcBottom >= fSrcTop);

	float fSrcWidth = fSrcRight - fSrcLeft;
	float fSrcHeight = fSrcBottom - fSrcTop;

	u32 dwDestWidth = rInImage.GetWidth();
	u32 dwDestHeight = rInImage.GetHeight();

	u32 dwTileX1 = (u32)(fSrcLeft * m_dwTileCountX);
	u32 dwTileY1 = (u32)(fSrcTop * m_dwTileCountY);
	u32 dwTileX2 = (u32)ceil(fSrcRight * m_dwTileCountX);
	u32 dwTileY2 = (u32)ceil(fSrcBottom * m_dwTileCountY);

	for (u32 dwTileY = dwTileY1; dwTileY < dwTileY2; dwTileY++)
		for (u32 dwTileX = dwTileX1; dwTileX < dwTileX2; dwTileX++)
		{
			CTerrainTextureTiles* tile = LoadTileIfNeeded(dwTileX, dwTileY);
			assert(tile);
			assert(tile->m_pTileImage);
			u32 dwTileSize = tile->m_pTileImage->GetWidth();

			float fSx = (-fSrcLeft + float(dwTileX) / m_dwTileCountX) / fSrcWidth;
			float fSy = (-fSrcTop + float(dwTileY) / m_dwTileCountY) / fSrcHeight;

			for (u32 y = 0; y < dwTileSize; y++)
				for (u32 x = 0; x < dwTileSize; x++)
				{
					float fX = float(x) / dwTileSize / m_dwTileCountX / fSrcWidth + fSx;
					float fY = float(y) / dwTileSize / m_dwTileCountY / fSrcHeight + fSy;

					if (0.0f <= fX && fX < 1.0f && 0.0f <= fY && fY < 1.0f)
					{
						u32 dwX = (u32)(fX * dwDestWidth);
						u32 dwY = (u32)(fY * dwDestHeight);

						u32 dwX1 = dwX + 1;
						u32 dwY1 = dwY + 1;

						if (dwX1 < dwDestWidth && dwY1 < dwDestHeight)
						{
							float fLerpX = fX * dwDestWidth - dwX;
							float fLerpY = fY * dwDestHeight - dwY;

							ColorB colS[4];

							colS[0] = rInImage.ValueAt(dwX, dwY);
							colS[1] = rInImage.ValueAt(dwX + 1, dwY);
							colS[2] = rInImage.ValueAt(dwX, dwY + 1);
							colS[3] = rInImage.ValueAt(dwX + 1, dwY + 1);

							ColorB colTop, colBottom;

							colTop.lerpFloat(colS[0], colS[1], fLerpX);
							colBottom.lerpFloat(colS[2], colS[3], fLerpX);

							ColorB colRet;

							colRet.lerpFloat(colTop, colBottom, fLerpY);

							tile->m_pTileImage->ValueAt(x, y) = colRet.pack_abgr8888();

						}
						else
							tile->m_pTileImage->ValueAt(x, y) = rInImage.ValueAt(dwX, dwY);
					}
				}
			tile->m_bDirty = true;
		}
}

u32 SampleImage(const CImageEx* pImage, float x, float y);

void         CRGBLayer::SetSubImageTransformed(const CImageEx* pImage, const Matrix34& transform)
{
	Vec3 p0 = transform.TransformPoint(Vec3(0, 0, 0));
	Vec3 p1 = transform.TransformPoint(Vec3(1, 0, 0));
	Vec3 p2 = transform.TransformPoint(Vec3(0, 1, 0));
	Vec3 p3 = transform.TransformPoint(Vec3(1, 1, 0));

	float fSrcLeft = clamp_tpl(min(min(p0.x, p1.x), min(p2.x, p3.x)), 0.0f, 1.0f);
	float fSrcRight = clamp_tpl(max(max(p0.x, p1.x), max(p2.x, p3.x)), 0.0f, 1.0f);
	float fSrcTop = clamp_tpl(min(min(p0.y, p1.y), min(p2.y, p3.y)), 0.0f, 1.0f);
	float fSrcBottom = clamp_tpl(max(max(p0.y, p1.y), max(p2.y, p3.y)), 0.0f, 1.0f);

	float fSrcWidth = fSrcRight - fSrcLeft;
	float fSrcHeight = fSrcBottom - fSrcTop;

	u32 dwTileX1 = (u32)(fSrcLeft * m_dwTileCountX);
	u32 dwTileY1 = (u32)(fSrcTop * m_dwTileCountY);
	u32 dwTileX2 = (u32)ceil(fSrcRight * m_dwTileCountX);
	u32 dwTileY2 = (u32)ceil(fSrcBottom * m_dwTileCountY);

	Matrix34 invTransform = transform.GetInverted();

	for (u32 dwTileY = dwTileY1; dwTileY < dwTileY2; dwTileY++)
	{
		for (u32 dwTileX = dwTileX1; dwTileX < dwTileX2; dwTileX++)
		{
			CTerrainTextureTiles* tile = LoadTileIfNeeded(dwTileX, dwTileY);
			assert(tile);
			assert(tile->m_pTileImage);
			u32 dwTileSize = tile->m_pTileImage->GetWidth();

			for (u32 y = 0; y < dwTileSize; y++)
			{
				for (u32 x = 0; x < dwTileSize; x++)
				{
					float tx = ((float)x / dwTileSize + dwTileX) / m_dwTileCountX;
					float ty = ((float)y / dwTileSize + dwTileY) / m_dwTileCountY;
					Vec3 uvw = invTransform.TransformPoint(Vec3(tx, ty, 0));
					if (uvw.x < 0 || uvw.y < 0 || uvw.x > 1 || uvw.y > 1)
						continue;
					tile->m_pTileImage->ValueAt(x, y) = SampleImage(pImage, uvw.x, uvw.y);
				}
			}
			tile->m_bDirty = true;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CRGBLayer::GetSubImageStretched(const float fSrcLeft, const float fSrcTop, const float fSrcRight,
                                     const float fSrcBottom, CImageEx& rOutImage, const bool bFiltered)
{
	assert(fSrcLeft >= 0.0f && fSrcLeft <= 1.0f);
	assert(fSrcTop >= 0.0f && fSrcTop <= 1.0f);
	assert(fSrcRight >= 0.0f && fSrcRight <= 1.0f);
	assert(fSrcBottom >= 0.0f && fSrcBottom <= 1.0f);
	assert(fSrcRight >= fSrcLeft);
	assert(fSrcBottom >= fSrcTop);

	u32 dwDestWidth = rOutImage.GetWidth();
	u32 dwDestHeight = rOutImage.GetHeight();

	i32 iBorderIncluded = bFiltered ? 1 : 0;

	float fScaleX = (fSrcRight - fSrcLeft) / (float)(dwDestWidth - iBorderIncluded);
	float fScaleY = (fSrcBottom - fSrcTop) / (float)(dwDestHeight - iBorderIncluded);

	for (u32 dwDestY = 0; dwDestY < dwDestHeight; ++dwDestY)
	{
		float fSrcY = fSrcTop + dwDestY * fScaleY;

		if (bFiltered) // check is not in the inner loop because of performance reasons
		{
			for (u32 dwDestX = 0; dwDestX < dwDestWidth; ++dwDestX)
			{
				float fSrcX = fSrcLeft + dwDestX * fScaleX;

				rOutImage.ValueAt(dwDestX, dwDestY) = GetFilteredValueAt(fSrcX, fSrcY);
			}
		}
		else
		{
			for (u32 dwDestX = 0; dwDestX < dwDestWidth; ++dwDestX)
			{
				float fSrcX = fSrcLeft + dwDestX * fScaleX;

				rOutImage.ValueAt(dwDestX, dwDestY) = GetValueAt(fSrcX, fSrcY);
			}
		}
	}
}

bool CRGBLayer::ChangeTileResolution(u32k dwTileX, u32k dwTileY, u32 dwNewSize)
{
	assert(dwTileX < m_dwTileCountX);
	assert(dwTileY < m_dwTileCountY);

	CTerrainTextureTiles* tile = LoadTileIfNeeded(dwTileX, dwTileY);
	assert(tile);

	if (!tile)
		return false;   // to avoid crash

	assert(tile->m_pTileImage);

	CImageEx newImage;

	u32 dwOldSize = tile->m_pTileImage->GetWidth();
	//u32 dwNewSize = bRaise ? dwOldSize*2 : dwOldSize/2;

	if (dwNewSize < 8)
		return false;   // not lower than this

	if (!newImage.Allocate(dwNewSize, dwNewSize))
		return false;

	float fBorderX = 0; // 0.5f/(float)(dwOldSize*m_dwTileCountX);
	float fBorderY = 0; // 0.5f/(float)(dwOldSize*m_dwTileCountY);

	// copy over with filtering
	GetSubImageStretched((float)dwTileX / (float)m_dwTileCountX + fBorderX, (float)dwTileY / (float)m_dwTileCountY + fBorderY,
	                     (float)(dwTileX + 1) / (float)m_dwTileCountX - fBorderX, (float)(dwTileY + 1) / (float)m_dwTileCountY - fBorderY, newImage, true);

	tile->m_pTileImage->Attach(newImage);
	tile->m_bDirty = true;

	tile->m_dwSize = dwNewSize;

	return true;
}

u32 CRGBLayer::GetValueAt(const float fpx, const float fpy)
{
	u32* value = nullptr;
	if (GetValueAt(fpx, fpy, value))
	{
		return *value;
	}
	else
	{
		return 0;
	}
}

bool CRGBLayer::GetValueAt(const float fpx, const float fpy, u32*& value)
{
	assert(fpx >= 0.0f && fpx <= 1.0f);
	assert(fpy >= 0.0f && fpy <= 1.0f);

	u32 dwTileX = (u32)(fpx * m_dwTileCountX);
	u32 dwTileY = (u32)(fpy * m_dwTileCountY);

	CTerrainTextureTiles* tile = LoadTileIfNeeded(dwTileX, dwTileY);
	assert(tile);

	if (!tile)
		return false;   // to avoid crash

	assert(tile->m_pTileImage);

	u32 dwTileSize = tile->m_pTileImage->GetWidth();
	//	u32 dwTileSizeReduced = dwTileSize-1;
	u32 dwTileOffsetX = dwTileX * dwTileSize;
	u32 dwTileOffsetY = dwTileY * dwTileSize;

	float fX = fpx * (dwTileSize * m_dwTileCountX), fY = fpy * (dwTileSize * m_dwTileCountY);

	u32 dwLocalX = (u32)(fX - dwTileOffsetX);
	u32 dwLocalY = (u32)(fY - dwTileOffsetY);

	// no bilinear filter ------------------------
	assert(dwLocalX < tile->m_pTileImage->GetWidth());
	assert(dwLocalY < tile->m_pTileImage->GetHeight());
	value = &tile->m_pTileImage->ValueAt(dwLocalX, dwLocalY);
	return true;
}

u32 CRGBLayer::GetFilteredValueAt(const float fpx, const float fpy)
{
	assert(fpx >= 0.0f && fpx <= 1.0f);
	assert(fpy >= 0.0f && fpy <= 1.0f);

	u32 dwTileX = (u32)(fpx * m_dwTileCountX);
	u32 dwTileY = (u32)(fpy * m_dwTileCountY);

	float fX = (fpx * m_dwTileCountX - dwTileX), fY = (fpy * m_dwTileCountY - dwTileY);

	// adjust lookup to keep as few tiles in memory as possible
	if (dwTileX != 0 && fX <= 0.000001f)
	{
		--dwTileX;
		fX = 0.99999f;
	}

	if (dwTileY != 0 && fY <= 0.000001f)
	{
		--dwTileY;
		fY = 0.99999f;
	}

	CTerrainTextureTiles* tile = LoadTileIfNeeded(dwTileX, dwTileY);
	assert(tile);

	assert(tile->m_pTileImage);

	u32 dwTileSize = tile->m_pTileImage->GetWidth();

	fX *= (dwTileSize - 1);
	fY *= (dwTileSize - 1);

	u32 dwLocalX = (u32)(fX);
	u32 dwLocalY = (u32)(fY);

	float fLerpX = fX - dwLocalX;     // 0..1
	float fLerpY = fY - dwLocalY;     // 0..1

	// bilinear filter ----------------------

	assert(dwLocalX < tile->m_pTileImage->GetWidth() - 1);
	assert(dwLocalY < tile->m_pTileImage->GetHeight() - 1);

	ColorF colS[4];

	colS[0] = tile->m_pTileImage->ValueAt(dwLocalX, dwLocalY);
	colS[1] = tile->m_pTileImage->ValueAt(dwLocalX + 1, dwLocalY);
	colS[2] = tile->m_pTileImage->ValueAt(dwLocalX, dwLocalY + 1);
	colS[3] = tile->m_pTileImage->ValueAt(dwLocalX + 1, dwLocalY + 1);

	ColorF colTop, colBottom;

	colTop.lerpFloat(colS[0], colS[1], fLerpX);
	colBottom.lerpFloat(colS[2], colS[3], fLerpX);

	ColorF colRet;

	colRet.lerpFloat(colTop, colBottom, fLerpY);

	return colRet.pack_abgr8888();
}

u32 CRGBLayer::GetUnfilteredValueAt(float fpx, float fpy)
{
	assert(fpx >= 0.0f && fpx <= 1.0f);
	assert(fpy >= 0.0f && fpy <= 1.0f);

	u32 dwTileX = (u32)(fpx * m_dwTileCountX);
	u32 dwTileY = (u32)(fpy * m_dwTileCountY);

	float fX = (fpx * m_dwTileCountX - dwTileX), fY = (fpy * m_dwTileCountY - dwTileY);

	CTerrainTextureTiles* tile = LoadTileIfNeeded(dwTileX, dwTileY);
	assert(tile);

	assert(tile->m_pTileImage);

	u32 dwTileSize = tile->m_pTileImage->GetWidth();

	fX *= (dwTileSize - 1);
	fY *= (dwTileSize - 1);

	u32 dwLocalX = (u32)(fX);
	u32 dwLocalY = (u32)(fY);

	assert(dwLocalX < tile->m_pTileImage->GetWidth() - 1);
	assert(dwLocalY < tile->m_pTileImage->GetHeight() - 1);

	ColorB col = tile->m_pTileImage->ValueAt(dwLocalX, dwLocalY);
	return col.pack_abgr8888();
}

void CRGBLayer::SetValueAt(const float fpx, const float fpy, u32k dwValue)
{
	u32* value = nullptr;
	if (GetValueAt(fpx, fpy, value))
	{
		*value = dwValue;
	}
}

void CRGBLayer::SetValueAt(u32k dwX, u32k dwY, u32k dwValue)
{
	u32 dwTileX = (u32)dwX / m_dwTileResolution;
	u32 dwTileY = (u32)dwY / m_dwTileResolution;

	CTerrainTextureTiles* tile = LoadTileIfNeeded(dwTileX, dwTileY);
	assert(tile);

	if (!tile)
		return;       // should not be needed

	assert(tile->m_pTileImage);

	tile->m_bDirty = true;

	u32 dwLocalX = dwX - dwTileX * m_dwTileResolution;
	u32 dwLocalY = dwY - dwTileY * m_dwTileResolution;

	assert(dwLocalX < tile->m_pTileImage->GetWidth());
	assert(dwLocalY < tile->m_pTileImage->GetHeight());

	tile->m_pTileImage->ValueAt(dwLocalX, dwLocalY) = dwValue;
}

void CRGBLayer::FreeTile(CTerrainTextureTiles& rTile)
{
	delete rTile.m_pTileImage;
	rTile.m_pTileImage = 0;

	rTile.m_bDirty = false;
	rTile.m_timeLastUsed = CTimeValue();

	// re-calculate memory usage
	m_dwCurrentTileMemory = 0;
	for (std::vector<CTerrainTextureTiles>::iterator it = m_TerrainTextureTiles.begin(); it != m_TerrainTextureTiles.end(); ++it)
		m_dwCurrentTileMemory += ((*it).m_pTileImage) ? ((*it).m_pTileImage->GetSize()) : 0;
}

void CRGBLayer::ConsiderGarbageCollection()
{
	while (m_dwCurrentTileMemory > m_dwMaxTileMemory)
	{
		CTerrainTextureTiles* pOldestTile = FindOldestTileToFree();

		if (pOldestTile)
			FreeTile(*pOldestTile);
		else
			return;
	}
}

void CRGBLayer::FreeData()
{
	ClosePakForLoading();

	std::vector<CTerrainTextureTiles>::iterator it;

	for (it = m_TerrainTextureTiles.begin(); it != m_TerrainTextureTiles.end(); ++it)
	{
		CTerrainTextureTiles& ref = *it;

		FreeTile(ref);
	}

	m_TerrainTextureTiles.clear();

	m_dwTileCountX = 0;
	m_dwTileCountY = 0;
	m_dwTileResolution = 0;
	m_dwCurrentTileMemory = 0;
}

CRGBLayer::CTerrainTextureTiles* CRGBLayer::FindOldestTileToFree()
{
	std::vector<CTerrainTextureTiles>::iterator it;
	u32 dwI = 0;

	CTerrainTextureTiles* pRet = 0;

	for (it = m_TerrainTextureTiles.begin(); it != m_TerrainTextureTiles.end(); ++it, ++dwI)
	{
		CTerrainTextureTiles& ref = *it;

		if (ref.m_pTileImage)                 // something to free
			if (!ref.m_bDirty)                  // hasn't changed
			{
				if (pRet == 0 || ref.m_timeLastUsed < pRet->m_timeLastUsed)
					pRet = &ref;
			}
	}

	return pRet;
}

namespace
{
DrxCriticalSection csLoadTileIfNeeded;
}

bool CRGBLayer::OpenPakForLoading()
{
	AUTO_LOCK(csLoadTileIfNeeded);

	if (m_bPakOpened)
		return true;

	const string pakFilename = GetFullFileName();

	IDrxPak* pIPak = GetIEditorImpl()->GetSystem()->GetIPak();
	m_bPakOpened = pIPak->IsFileExist(pakFilename) && pIPak->OpenPack(pakFilename);

	// if the pak file wasn't created yet
	if (!m_bPakOpened)
	{
		// create PAK file so we don't get errors when loading
		SaveAndFreeMemory(true);

		m_bPakOpened = pIPak->OpenPack(pakFilename);
	}

	return m_bPakOpened;
}

bool CRGBLayer::ClosePakForLoading()
{
	DRX_ASSERT(GetIEditorImpl() && GetIEditorImpl()->GetSystem());

	AUTO_LOCK(csLoadTileIfNeeded);

	// Here we need to unconditionally close the pak file, even if it was opened somewhere else, e.g. see CGameEngine::LoadLevel.
	IDrxPak* pIPak = GetIEditorImpl()->GetSystem()->GetIPak();
	const string pakFilename = GetFullFileName();
	m_bPakOpened = !pIPak->ClosePack(pakFilename);

	if (m_bPakOpened)
	{
		DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, "Failed to close pak file %s", pakFilename.c_str());
	}
	return !m_bPakOpened;
}

CRGBLayer::CTerrainTextureTiles* CRGBLayer::GetTilePtr(u32k dwTileX, u32k dwTileY)
{
	assert(dwTileX < m_dwTileCountX);
	assert(dwTileY < m_dwTileCountY);

	if (dwTileX >= m_dwTileCountX || dwTileY >= m_dwTileCountY)
		return 0;     // to avoid crash

	return &m_TerrainTextureTiles[dwTileX + dwTileY * m_dwTileCountX];
}

CRGBLayer::CTerrainTextureTiles* CRGBLayer::LoadTileIfNeeded(u32k dwTileX, u32k dwTileY, bool bNoGarbageCollection)
{
	CTerrainTextureTiles* pTile = GetTilePtr(dwTileX, dwTileY);

	if (!pTile)
		return 0;

	if (!pTile->m_pTileImage)
	{
		AUTO_LOCK(csLoadTileIfNeeded);

		pTile->m_timeLastUsed = GetIEditorImpl()->GetSystem()->GetITimer()->GetAsyncCurTime();

		if (!pTile->m_pTileImage)
		{
			DrxLog("Loading RGB Layer Tile: TilePos=(%d, %d) MemUsage=%.1fMB", dwTileX, dwTileY, float(m_dwCurrentTileMemory) / 1024.f / 1024.f);   // debugging

			if (!bNoGarbageCollection)
				ConsiderGarbageCollection();

			string pathRel = GetIEditorImpl()->GetGameEngine()->GetLevelPath();
			CImageEx* pTileImage = pTile->m_pTileImage;

			if (OpenPakForLoading())
			{
				CMemoryBlock mem;
				u32 dwWidth = 0, dwHeight = 0;
				u8* pSrc8 = 0;
				i32 bpp = 3;

				CDrxFile file;

				char szTileName[128];

				drx_sprintf(szTileName, "Tile%d_%d.raw", dwTileX, dwTileY);

				if (file.Open(PathUtil::AddBackslash(pathRel.GetString()) + szTileName, "rb"))
				{
					if (file.ReadType(&dwWidth)
					    && file.ReadType(&dwHeight)
					    && dwWidth > 0 && dwHeight > 0)
					{
						assert(dwWidth == dwHeight);

						if (mem.Allocate(dwWidth * dwHeight * bpp))
						{
							if (!file.ReadRaw(mem.GetBuffer(), dwWidth * dwHeight * bpp))
							{
								DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, "Error reading tile raw data from %s!", szTileName);
								dwWidth = 0;
							}
							pSrc8 = (u8*)mem.GetBuffer();
						}
						else
						{
							// error
							assert(0);
						}
					}
					else
					{
						// error
						assert(0);
					}

					file.Close();   // need to close the file to be able tot close the pak
				}
				else
				{
					//				DrxLog("CRGBLayer::LoadTileIfNeeded not found in pak ...");		// debugging
				}

				if (pSrc8 && dwWidth > 0 && dwHeight > 0)
				{
					if (!pTileImage)
						pTileImage = new CImageEx();

					pTile->m_dwSize = dwWidth;

					if (pTileImage->Allocate(dwWidth, dwHeight))
					{
						u8* pDst8 = (u8*)pTileImage->GetData();

						for (u32 dwI = 0; dwI < dwWidth * dwHeight; ++dwI)
						{
							*pDst8++ = *pSrc8++;
							*pDst8++ = *pSrc8++;
							*pDst8++ = *pSrc8++;
							*pDst8++ = 0;
						}
					}
					else
					{
						// error
						assert(0);
					}
				}
			}

			// still not there - then create an empty tile
			if (!pTileImage)
			{
				pTileImage = new CImageEx();

				pTile->m_dwSize = m_dwTileResolution;

				pTileImage->Allocate(m_dwTileResolution, m_dwTileResolution);
				pTileImage->Fill(0xff);

				// for more convenience in the beginning:
				CLayer* pLayer = GetIEditorImpl()->GetTerrainManager()->GetLayer(0);

				if (pLayer)
				{
					pLayer->PrecacheTexture();

					CImagePainter painter;

					u32 dwTileSize = pTileImage->GetWidth();
					u32 dwTileSizeReduced = dwTileSize - 1;
					u32 dwTileOffsetX = dwTileX * dwTileSizeReduced;
					u32 dwTileOffsetY = dwTileY * dwTileSizeReduced;

					painter.FillWithPattern(*pTileImage, dwTileOffsetX, dwTileOffsetY, pLayer->m_texture);
				}

			}

			pTile->m_bDirty = false;

			pTile->m_pTileImage = pTileImage;

			// re-calculate memory usage
			m_dwCurrentTileMemory = 0;
			for (std::vector<CTerrainTextureTiles>::iterator it = m_TerrainTextureTiles.begin(); it != m_TerrainTextureTiles.end(); ++it)
				m_dwCurrentTileMemory += ((*it).m_pTileImage) ? ((*it).m_pTileImage->GetSize()) : 0;

			//		DrxLog("CRGBLayer::LoadTileIfNeeded ... %d",m_dwCurrentTileMemory);		// debugging
		}
	}

	return pTile;
}

bool CRGBLayer::IsDirty() const
{
	if (m_bInfoDirty)
		return true;

	std::vector<CTerrainTextureTiles>::const_iterator it;

	for (it = m_TerrainTextureTiles.begin(); it != m_TerrainTextureTiles.end(); ++it)
	{
		const CTerrainTextureTiles& ref = *it;

		if (ref.m_pTileImage)
			if (ref.m_bDirty)
				return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
string CRGBLayer::GetFullFileName()
{
	string pathRel = GetIEditorImpl()->GetGameEngine()->GetLevelPath();
	string pathPak = PathUtil::Make(pathRel, m_TerrainRGBFileName);

	return pathPak;
}

//////////////////////////////////////////////////////////////////////////
bool CRGBLayer::WouldSaveSucceed()
{
	if (!IsDirty())
	{
		return true;    // no need to save
	}

	if (!ClosePakForLoading())
	{
		return false;
	}

	const string pakFilename = GetFullFileName();
	if (!CFileUtil::OverwriteFile(pakFilename.c_str()))
	{
		return false;
	}

	// Create pak file
	{
		CPakFile pakFile;
		if (!pakFile.Open(pakFilename.c_str()))
		{
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, "Unable to create pak file %s.", pakFilename.c_str());
			return false;   // save would fail
		}
		pakFile.Close();
	}
	return true;      // save would work
}

//////////////////////////////////////////////////////////////////////////
void CRGBLayer::Offset(i32 iTilesX, i32 iTilesY)
{
	i32 yStart, yEnd, yStep;
	if (iTilesY < 0)
	{
		yStart = 0;
		yEnd = (i32) m_dwTileCountY;
		yStep = 1;
	}
	else
	{
		yStart = (i32) m_dwTileCountY - 1;
		yEnd = -1;
		yStep = -1;
	}
	i32 xStart, xEnd, xStep;
	if (iTilesX < 0)
	{
		xStart = 0;
		xEnd = (i32) m_dwTileCountX;
		xStep = 1;
	}
	else
	{
		xStart = (i32) m_dwTileCountX - 1;
		xEnd = -1;
		xStep = -1;
	}
	for (i32 y = yStart; y != yEnd; y += yStep)
	{
		i32 sy = y - iTilesY;
		for (i32 x = xStart; x != xEnd; x += xStep)
		{
			i32 sx = x - iTilesX;
			CTerrainTextureTiles* pTile = GetTilePtr(x, y);
			FreeTile(*pTile);
			CTerrainTextureTiles* pSrc = (sy >= 0 && sy < m_dwTileCountY && sx >= 0 && sx < m_dwTileCountX) ? GetTilePtr(sx, sy) : 0;
			if (pSrc)
			{
				*pTile = *pSrc;
				// not using FreeTile(pSrc) because we don't want the image to be deleted
				pSrc->m_pTileImage = 0;
				pSrc->m_bDirty = false;
				pSrc->m_timeLastUsed = CTimeValue();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CRGBLayer::LoadAll()
{
	if (!OpenPakForLoading())
		return;
	for (u32 y = 0; y < m_dwTileCountY; ++y)
	{
		for (u32 x = 0; x < m_dwTileCountX; ++x)
		{
			LoadTileIfNeeded(x, y, true);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CRGBLayer::Resize(u32 dwTileCountX, u32 dwTileCountY, u32 dwTileResolution)
{
	CImageEx** pImages = new CImageEx*[dwTileCountX * dwTileCountY];
	u32 x, y;
	for (y = 0; y < dwTileCountY; ++y)
	{
		for (x = 0; x < dwTileCountX; ++x)
		{
			CImageEx*& pImg = pImages[y * dwTileCountX + x];
			pImg = new CImageEx();
			pImg->Allocate(dwTileResolution, dwTileResolution);
			float l = ((float) x) / dwTileCountX;
			float t = ((float) y) / dwTileCountY;
			float r = ((float) x + 1) / dwTileCountX;
			float b = ((float) y + 1) / dwTileCountY;
			GetSubImageStretched(l, t, r, b, *pImg);
		}
	}
	FreeData();
	AllocateTiles(dwTileCountX, dwTileCountY, dwTileResolution);
	for (y = 0; y < dwTileCountY; ++y)
	{
		for (x = 0; x < dwTileCountX; ++x)
		{
			CTerrainTextureTiles* pTile = GetTilePtr(x, y);
			pTile->m_pTileImage = pImages[y * dwTileCountX + x];
			pTile->m_dwSize = dwTileResolution;
			pTile->m_bDirty = true;
		}
	}
	delete pImages;
}

//////////////////////////////////////////////////////////////////////////
void CRGBLayer::CleanupCache()
{
	ClosePakForLoading();

	std::vector<CTerrainTextureTiles>::iterator it;

	for (it = m_TerrainTextureTiles.begin(); it != m_TerrainTextureTiles.end(); ++it)
	{
		CTerrainTextureTiles& ref = *it;

		if (ref.m_pTileImage)
			if (!ref.m_bDirty)
				FreeTile(ref);
	}
}

//////////////////////////////////////////////////////////////////////////
bool CRGBLayer::SaveAndFreeMemory(const bool bForceFileCreation)
{
	if (!ClosePakForLoading())
	{
		return false;
	}

	if (!bForceFileCreation && !IsDirty())
	{
		return true;
	}

	// create pak file
	const string pakFilename = GetFullFileName();
	CPakFile pakFile;
	if (!pakFile.Open(pakFilename))
	{
		DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, "Failed to open pak file %s for writing.", pakFilename.c_str());
		return false;
	}

	std::vector<CTerrainTextureTiles>::iterator it;
	u32 dwI = 0;

	for (it = m_TerrainTextureTiles.begin(); it != m_TerrainTextureTiles.end(); ++it, ++dwI)
	{
		CTerrainTextureTiles& ref = *it;

		u32 dwTileX = dwI % m_dwTileCountX;
		u32 dwTileY = dwI / m_dwTileCountX;

		if (ref.m_pTileImage)
		{
			if (ref.m_bDirty)
			{
				CMemoryBlock mem;
				ExportSegment(mem, dwTileX, dwTileY, false);

				char szTileName[20];

				drx_sprintf(szTileName, "Tile%d_%d.raw", dwTileX, dwTileY);

				if (!pakFile.UpdateFile(PathUtil::AddBackslash(GetIEditorImpl()->GetGameEngine()->GetLevelPath().GetString()) + szTileName, mem))
				{
					DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, "Failed to update tile file %s.", szTileName);
				}
			}

			// free tile
			FreeTile(ref);
		}
	}

	pakFile.Close();

	return true;
}

void CRGBLayer::ClipTileRect(CRect& inoutRect) const
{
	if ((i32)(inoutRect.left) < 0)
		inoutRect.left = 0;

	if ((i32)(inoutRect.top) < 0)
		inoutRect.top = 0;

	if ((i32)(inoutRect.right) > m_dwTileCountX)
		inoutRect.right = m_dwTileCountX;

	if ((i32)(inoutRect.bottom) > m_dwTileCountY)
		inoutRect.bottom = m_dwTileCountY;
}

//////////////////////////////////////////////////////////////////////////
void CRGBLayer::PaintBrushWithPatternTiled(const float fpx, const float fpy, SEditorPaintBrush& brush, const CImageEx& imgPattern)
{
	assert(brush.fRadius >= 0.0f && brush.fRadius <= 1.0f);

	float fOldRadius = brush.fRadius;

	u32 dwMaxRes = CalcMaxLocalResolution(0, 0, 1, 1);

	CRect recTiles = CRect(
	  (u32)floor((fpx - brush.fRadius - 2.0f / dwMaxRes) * m_dwTileCountX),
	  (u32)floor((fpy - brush.fRadius - 2.0f / dwMaxRes) * m_dwTileCountY),
	  (u32)ceil((fpx + brush.fRadius + 2.0f / dwMaxRes) * m_dwTileCountX),
	  (u32)ceil((fpy + brush.fRadius + 2.0f / dwMaxRes) * m_dwTileCountY));

	ClipTileRect(recTiles);

	CImagePainter painter;

	for (u32 dwTileY = recTiles.top; dwTileY < recTiles.bottom; ++dwTileY)
		for (u32 dwTileX = recTiles.left; dwTileX < recTiles.right; ++dwTileX)
		{
			CTerrainTextureTiles* tile = LoadTileIfNeeded(dwTileX, dwTileY);
			assert(tile);

			assert(tile->m_pTileImage);

			tile->m_bDirty = true;

			u32 dwTileSize = tile->m_pTileImage->GetWidth();
			u32 dwTileSizeReduced = dwTileSize - 1;    // usable area in tile is limited because of bilinear filtering
			u32 dwTileOffsetX = dwTileX * dwTileSizeReduced;
			u32 dwTileOffsetY = dwTileY * dwTileSizeReduced;

			//			float fX=fpx*(dwTileSizeReduced*m_dwTileCountX)+0.5f, fY=fpy*(dwTileSizeReduced*m_dwTileCountY)+0.5f;
			float fScaleX = (dwTileSizeReduced * m_dwTileCountX), fScaleY = (dwTileSizeReduced * m_dwTileCountY);

			brush.fRadius = fOldRadius * (dwTileSize * m_dwTileCountX);

			painter.PaintBrushWithPattern(fpx, fpy, *tile->m_pTileImage, dwTileOffsetX, dwTileOffsetY, fScaleX, fScaleY, brush, imgPattern);
			/*
			      // debug edges
			      tile->m_pTileImage->ValueAt(0,0)=0x00ff0000;
			      tile->m_pTileImage->ValueAt(dwTileSize-1,0)=0x00ff00ff;
			      tile->m_pTileImage->ValueAt(0,dwTileSize-1)=0x0000ff00;
			      tile->m_pTileImage->ValueAt(dwTileSize-1,dwTileSize-1)=0x000000ff;
			 */
		}

	// fix borders - minor quality improvement (can be optimized)
	for (u32 dwTileY = recTiles.top; dwTileY < recTiles.bottom; ++dwTileY)
		for (u32 dwTileX = recTiles.left; dwTileX < recTiles.right; ++dwTileX)
		{
			assert(dwTileX < m_dwTileCountX);
			assert(dwTileY < m_dwTileCountY);

			CTerrainTextureTiles* tile1 = LoadTileIfNeeded(dwTileX, dwTileY);
			assert(tile1);
			assert(tile1->m_pTileImage);
			assert(tile1->m_bDirty);

			u32 dwTileSize1 = tile1->m_pTileImage->GetWidth();

			if (dwTileX != recTiles.left)  // vertical border between tile2 and tile 1
			{
				CTerrainTextureTiles* tile2 = LoadTileIfNeeded(dwTileX - 1, dwTileY);
				assert(tile2);
				assert(tile2->m_pTileImage);
				assert(tile2->m_bDirty);

				u32 dwTileSize2 = tile2->m_pTileImage->GetWidth();
				u32 dwTileSizeMax = max(dwTileSize1, dwTileSize2);

				for (u32 dwI = 0; dwI < dwTileSizeMax; ++dwI)
				{
					u32& dwC2 = tile2->m_pTileImage->ValueAt(dwTileSize2 - 1, (dwI * (dwTileSize2 - 1)) / (dwTileSizeMax - 1));
					u32& dwC1 = tile1->m_pTileImage->ValueAt(0, (dwI * (dwTileSize1 - 1)) / (dwTileSizeMax - 1));

					u32 dwAvg = ColorB::ComputeAvgCol_Fast(dwC1, dwC2);

					dwC1 = dwAvg;
					dwC2 = dwAvg;
				}
			}

			if (dwTileY != recTiles.top) // horizontal border between tile2 and tile 1
			{
				CTerrainTextureTiles* tile2 = LoadTileIfNeeded(dwTileX, dwTileY - 1);
				assert(tile2);
				assert(tile2->m_pTileImage);
				assert(tile2->m_bDirty);

				u32 dwTileSize2 = tile2->m_pTileImage->GetWidth();
				u32 dwTileSizeMax = max(dwTileSize1, dwTileSize2);

				for (u32 dwI = 0; dwI < dwTileSizeMax; ++dwI)
				{
					u32& dwC2 = tile2->m_pTileImage->ValueAt((dwI * (dwTileSize2 - 1)) / (dwTileSizeMax - 1), dwTileSize2 - 1);
					u32& dwC1 = tile1->m_pTileImage->ValueAt((dwI * (dwTileSize1 - 1)) / (dwTileSizeMax - 1), 0);

					u32 dwAvg = ColorB::ComputeAvgCol_Fast(dwC1, dwC2);

					dwC1 = dwAvg;
					dwC2 = dwAvg;
				}
			}
		}

	brush.fRadius = fOldRadius;
}

u32 CRGBLayer::GetTileResolution(u32k dwTileX, u32k dwTileY)
{
	CTerrainTextureTiles* tile = GetTilePtr(dwTileX, dwTileY);
	assert(tile);

	if (!tile->m_dwSize)   // not size info yet - load the tile
	{
		tile = LoadTileIfNeeded(dwTileX, dwTileY);
		assert(tile);

		m_bInfoDirty = true;    // save is required to update the dwSize

		assert(tile->m_pTileImage);
	}

	assert(tile->m_dwSize);

	return tile->m_dwSize;
}

u32 CRGBLayer::CalcMaxLocalResolution(const float fSrcLeft, const float fSrcTop, const float fSrcRight, const float fSrcBottom)
{
	assert(fSrcRight >= fSrcLeft);
	assert(fSrcBottom >= fSrcTop);

	CRect recTiles = CRect((u32)floor(fSrcLeft * m_dwTileCountX), (u32)floor(fSrcTop * m_dwTileCountY),
	                       (u32)ceil(fSrcRight * m_dwTileCountX), (u32)ceil(fSrcBottom * m_dwTileCountY));

	ClipTileRect(recTiles);

	u32 dwRet = 0;

	for (u32 dwTileY = recTiles.top; dwTileY < recTiles.bottom; ++dwTileY)
		for (u32 dwTileX = recTiles.left; dwTileX < recTiles.right; ++dwTileX)
		{
			u32 dwSize = GetTileResolution(dwTileX, dwTileY);
			assert(dwSize);

			u32 dwLocalWidth = dwSize * m_dwTileCountX;
			u32 dwLocalHeight = dwSize * m_dwTileCountY;

			if (dwRet < dwLocalWidth)
				dwRet = dwLocalWidth;
			if (dwRet < dwLocalHeight)
				dwRet = dwLocalHeight;
		}

	return dwRet;
}

bool CRGBLayer::IsAllocated() const
{
	return m_TerrainTextureTiles.size() != 0;
}

//////////////////////////////////////////////////////////////////////////
void CRGBLayer::Serialize(XmlNodeRef& node, bool bLoading)
{
	////////////////////////////////////////////////////////////////////////
	// Save or restore the class
	////////////////////////////////////////////////////////////////////////
	if (bLoading)
	{
		m_dwTileCountX = m_dwTileCountY = m_dwTileResolution = 0;

		// Texture
		node->getAttr("TileCountX", m_dwTileCountX);
		node->getAttr("TileCountY", m_dwTileCountY);
		node->getAttr("TileResolution", m_dwTileResolution);

		if (m_dwTileCountX && m_dwTileCountY && m_dwTileResolution)          // if info is valid
			AllocateTiles(m_dwTileCountX, m_dwTileCountY, m_dwTileResolution);
		else
			FreeData();

		XmlNodeRef mainNode = node->findChild("RGBLayer");

		if (mainNode)    // old nodes might not have this info
		{
			XmlNodeRef tiles = mainNode->findChild("Tiles");
			if (tiles)
			{
				i32 numObjects = tiles->getChildCount();

				for (i32 i = 0; i < numObjects; i++)
				{
					XmlNodeRef tile = tiles->getChild(i);

					u32 dwX = 0, dwY = 0, dwSize = 0;

					tile->getAttr("X", dwX);
					tile->getAttr("Y", dwY);
					tile->getAttr("Size", dwSize);

					CTerrainTextureTiles* pPtr = GetTilePtr(dwX, dwY);
					assert(pPtr);

					if (pPtr)
						pPtr->m_dwSize = dwSize;
				}
			}
		}
	}
	else
	{
		// Storing
		node = XmlHelpers::CreateXmlNode("TerrainTexture");

		// Texture
		node->setAttr("TileCountX", m_dwTileCountX);
		node->setAttr("TileCountY", m_dwTileCountY);
		node->setAttr("TileResolution", m_dwTileResolution);

		SaveAndFreeMemory();

		XmlNodeRef mainNode = node->newChild("RGBLayer");
		XmlNodeRef tiles = mainNode->newChild("Tiles");

		for (u32 dwY = 0; dwY < m_dwTileCountY; ++dwY)
			for (u32 dwX = 0; dwX < m_dwTileCountX; ++dwX)
			{
				XmlNodeRef obj = tiles->newChild("tile");

				CTerrainTextureTiles* pPtr = GetTilePtr(dwX, dwY);
				assert(pPtr);

				u32 dwSize = pPtr->m_dwSize;

				if (dwSize || m_bNextSerializeForceSizeSave)
				{
					obj->setAttr("X", dwX);
					obj->setAttr("Y", dwY);
					obj->setAttr("Size", dwSize);
				}
			}

		m_bNextSerializeForceSizeSave = false;

		m_bInfoDirty = false;
	}
}

void CRGBLayer::AllocateTiles(u32k dwTileCountX, u32k dwTileCountY, u32k dwTileResolution)
{
	assert(dwTileCountX);
	assert(dwTileCountY);

	// free
	m_TerrainTextureTiles.clear();
	m_TerrainTextureTiles.resize(dwTileCountX * dwTileCountY);

	m_dwTileCountX = dwTileCountX;
	m_dwTileCountY = dwTileCountY;
	m_dwTileResolution = dwTileResolution;

	DrxLog("CRGBLayer::AllocateTiles %dx%d tiles %d => %dx%d texture", dwTileCountX, dwTileCountY, dwTileResolution, dwTileCountX * dwTileResolution, dwTileCountY * dwTileResolution);    // debugging
}

void CRGBLayer::SetSubImageRGBLayer(u32k dwDstX, u32k dwDstY, const CImageEx& rTileImage)
{
	u32 dwWidth = rTileImage.GetWidth();
	u32 dwHeight = rTileImage.GetHeight();

	for (u32 dwTexelY = 0; dwTexelY < dwHeight; ++dwTexelY)
		for (u32 dwTexelX = 0; dwTexelX < dwWidth; ++dwTexelX)
			SetValueAt(dwDstX + dwTexelX, dwDstY + dwTexelY, rTileImage.ValueAt(dwTexelX, dwTexelY));
}

//////////////////////////////////////////////////////////////////////////
void CRGBLayer::GetMemoryUsage(IDrxSizer* pSizer)
{
	std::vector<CTerrainTextureTiles>::iterator it;

	for (it = m_TerrainTextureTiles.begin(); it != m_TerrainTextureTiles.end(); ++it)
	{
		CTerrainTextureTiles& ref = *it;

		if (ref.m_pTileImage)                  // something to free
			pSizer->Add((tuk)ref.m_pTileImage->GetData(), ref.m_pTileImage->GetSize());
	}

	pSizer->Add(m_TerrainTextureTiles);

	pSizer->Add(*this);
}

//////////////////////////////////////////////////////////////////////////
u32 CRGBLayer::CalcMinRequiredTextureExtend()
{
	u32 dwMaxLocalExtend = 0;

	for (u32 dwTileY = 0; dwTileY < m_dwTileCountY; ++dwTileY)
		for (u32 dwTileX = 0; dwTileX < m_dwTileCountX; ++dwTileX)
			dwMaxLocalExtend = max(dwMaxLocalExtend, GetTileResolution(dwTileX, dwTileY));

	return max(m_dwTileCountX, m_dwTileCountY) * dwMaxLocalExtend;
}

//////////////////////////////////////////////////////////////////////////
void CRGBLayer::Debug()
{
	std::vector<CTerrainTextureTiles>::iterator it;

	u32 dwI = 0;

	for (it = m_TerrainTextureTiles.begin(); it != m_TerrainTextureTiles.end(); ++it, ++dwI)
	{
		CTerrainTextureTiles& ref = *it;

		if (ref.m_pTileImage)
		{
			char szName[256];

			drx_sprintf(szName, "RGBLayerTile%dx%d.bmp", dwI % m_dwTileCountX, dwI / m_dwTileCountX);
			CImageUtil::SaveBitmap(szName, *ref.m_pTileImage);
		}
	}
}

/*
   bool CRGBLayer::IsSaveNeeded() const
   {
   std::vector<CTerrainTextureTiles>::const_iterator it, end=m_TerrainTextureTiles.end();

   for(it=m_TerrainTextureTiles.begin();it!=end;++it,++dwI)
   {
    const CTerrainTextureTiles &ref = *it;

    if(ref->m_bDirty)
      return true;
   }

   return false;
   }
 */

//////////////////////////////////////////////////////////////////////////
bool CRGBLayer::RefineTiles()
{
	CRGBLayer out("TerrainTexture2.pak");

	assert(m_dwTileCountX);
	assert(m_dwTileCountY);
	assert(m_dwTileResolution / 2);

	out.AllocateTiles(m_dwTileCountX * 2, m_dwTileCountY * 2, m_dwTileResolution / 2);

	std::vector<CTerrainTextureTiles>::iterator it, end = m_TerrainTextureTiles.end();
	u32 dwI = 0;

	for (it = m_TerrainTextureTiles.begin(); it != end; ++it, ++dwI)
	{
		CTerrainTextureTiles& ref = *it;

		u32 dwTileX = dwI % m_dwTileCountX;
		u32 dwTileY = dwI / m_dwTileCountY;

		LoadTileIfNeeded(dwTileX, dwTileY);

		assert(ref.m_dwSize);

		for (u32 dwY = 0; dwY < 2; ++dwY)
			for (u32 dwX = 0; dwX < 2; ++dwX)
			{
				CTerrainTextureTiles* pOutTile = out.GetTilePtr(dwTileX * 2 + dwX, dwTileY * 2 + dwY);
				assert(pOutTile);

				pOutTile->m_dwSize = ref.m_dwSize / 2;
				pOutTile->m_pTileImage = new CImageEx();
				pOutTile->m_pTileImage->Allocate(pOutTile->m_dwSize, pOutTile->m_dwSize);
				pOutTile->m_timeLastUsed = GetIEditorImpl()->GetSystem()->GetITimer()->GetAsyncCurTime();
				pOutTile->m_bDirty = true;
				m_bInfoDirty = true;

				float fSrcLeft = (float)(dwTileX * 2 + dwX) / (float)(m_dwTileCountX * 2);
				float fSrcTop = (float)(dwTileY * 2 + dwY) / (float)(m_dwTileCountY * 2);
				float fSrcRight = (float)(dwTileX * 2 + dwX + 1) / (float)(m_dwTileCountX * 2);
				float fSrcBottom = (float)(dwTileY * 2 + dwY + 1) / (float)(m_dwTileCountY * 2);

				GetSubImageStretched(fSrcLeft, fSrcTop, fSrcRight, fSrcBottom, *pOutTile->m_pTileImage, false);
			}

		if (!out.SaveAndFreeMemory(true))
		{
			assert(0);
			return false;
		}
	}

	if (!ClosePakForLoading())
	{
		return false;
	}

	string path = GetFullFileName();
	string path2 = out.GetFullFileName();

	MoveFile(path, PathUtil::ReplaceExtension((tukk)path, "bak"));
	MoveFile(path2, path);

	AllocateTiles(m_dwTileCountX * 2, m_dwTileCountY * 2, m_dwTileResolution);
	m_bNextSerializeForceSizeSave = true;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CRGBLayer::ImportExportBlock(tukk pFileName, i32 nSrcLeft, i32 nSrcTop, i32 nSrcRight, i32 nSrcBottom, u32* outSquare, bool bIsImport)
{
	u32 dwCountX = GetTileCountX();
	u32 dwCountY = GetTileCountY();

	i32 iX1 = nSrcLeft;
	i32 iY1 = nSrcTop;
	i32 iX2 = nSrcRight;
	i32 iY2 = nSrcBottom;

	if (iX2 >= dwCountX)
		iX2 = dwCountX - 1;
	if (iY2 >= dwCountY)
		iY2 = dwCountY - 1;

	if (0.001f + iX2 > nSrcRight && iX2 > iX1)
		iX2--;
	if (0.001f + iY2 > nSrcBottom && iY2 > iY1)
		iY2--;

	u32 maxRes = 0;
	for (i32 iX = iX1; iX <= iX2; iX++)
		for (i32 iY = iY1; iY <= iY2; iY++)
		{
			if (iX < 0 || iY < 0)
				continue;

			u32 dwRes = GetTileResolution((u32)iX, (u32)iY);
			if (dwRes > maxRes)
				maxRes = dwRes;
		}

	i32 x1 = i32 (nSrcLeft * maxRes);
	i32 y1 = i32 (nSrcTop * maxRes);
	i32 x2 = i32 (nSrcRight * maxRes);
	i32 y2 = i32 (nSrcBottom * maxRes);

	// loading big image by tile
	bool bIsPerParts = false;

	*outSquare = (x2 - x1) * (y2 - y1);

	CImageEx img;
	if (bIsImport)
	{
		if (pFileName && *pFileName)
		{
			// if image is big load by parts
			if (x2 - x1 > 2048 || y2 - y1 > 2048)
				bIsPerParts = true;

			if (!bIsPerParts)
			{
				CImageEx newImage;
				if (!CImageUtil::LoadBmp(pFileName, newImage))
				{
					CQuestionDialog::SCritical(QObject::tr(""), QObject::tr("Error: Can't load BMP file. Probably out of memory."));
					return false;
				}
				img.RotateOrt(newImage, 3);
				img.SwapRedAndBlue();
			}

		}
		else
		{
			CClipboard cb;
			if (!cb.GetImage(img))
				return false;
		}
	}
	else
		img.Allocate(x2 - x1, y2 - y1);

	i32 ststx1;
	i32 ststy1;
	bool bIsStst = true;

	i32 nTotalSectors = (iX2 - iX1 + 1) * (iY2 - iY1 + 1);
	i32 nCurrSectorNum = 0;
	CWaitProgress progress("Import/Export Terrain Surface Texture");

	for (i32 iX = iX1; iX <= iX2; iX++)
		for (i32 iY = iY1; iY <= iY2; iY++)
		{
			if (iX < 0 || iY < 0 || iX >= dwCountX || iY >= dwCountY)
				continue;

			if (!progress.Step((100 * nCurrSectorNum) / nTotalSectors))
				return false;

			// need for big areas
			if (!nCurrSectorNum && !(nCurrSectorNum % 16))
				SaveAndFreeMemory(true);

			nCurrSectorNum++;

			CTerrainTextureTiles* pTile = LoadTileIfNeeded((u32)iX, (u32)iY);
			if (!pTile)
				continue;

			pTile->m_bDirty = true;

			u32 dwRes = GetTileResolution((u32)iX, (u32)iY);

			i32 stx1 = x1 - iX * maxRes;
			if (stx1 < 0) stx1 = 0;
			i32 sty1 = y1 - iY * maxRes;
			if (sty1 < 0) sty1 = 0;

			i32 stx2 = x2 - iX * maxRes;
			if (stx2 > maxRes) stx2 = maxRes;
			i32 sty2 = y2 - iY * maxRes;
			if (sty2 > maxRes) sty2 = maxRes;

			if (bIsPerParts)
			{
				if (bIsStst)
				{
					ststx1 = stx1;
					ststy1 = sty1;
					bIsStst = false;
				}
				RECT rc = { iX* maxRes + stx1 - iX1 * maxRes - ststx1, iY * maxRes + sty1 - iY1 * maxRes - ststy1, iX * maxRes + stx2 - iX1 * maxRes - ststx1, iY * maxRes + sty2 - iY1 * maxRes - ststy1 };
				if (!CImageUtil::LoadBmp(pFileName, img, rc))
				{
					CQuestionDialog::SCritical(QObject::tr(""), QObject::tr("Error: Can't load BMP file. Probably out of memory."));
					return false;
				}
				img.SwapRedAndBlue();
			}

			if (dwRes == maxRes)
			{
				for (i32 x = stx1; x < stx2; x++)
					for (i32 y = sty1; y < sty2; y++)
					{
						if (bIsImport)
						{
							if (bIsPerParts)
								pTile->m_pTileImage->ValueAt(x, y) = img.ValueAt(x - stx1, y - sty1);
							else
							{
								if (x + iX * maxRes - x1 < img.GetWidth() && y + iY * maxRes - y1 < img.GetHeight())
									pTile->m_pTileImage->ValueAt(x, y) = img.ValueAt(x + iX * maxRes - x1, y + iY * maxRes - y1);
							}
						}
						else
							img.ValueAt(x + iX * maxRes - x1, y + iY * maxRes - y1) = pTile->m_pTileImage->ValueAt(x, y);
					}
			}
			else
			{
				CImageEx part;
				part.Allocate(stx2 - stx1, sty2 - sty1);
				if (!bIsImport)
					GetSubImageStretched(((float)stx1 / maxRes + iX) / dwCountX, ((float)sty1 / maxRes + iY) / dwCountY,
					                     ((float)stx2 / maxRes + iX) / dwCountX, ((float)sty2 / maxRes + iY) / dwCountY, part, true);

				for (i32 x = stx1; x < stx2; x++)
					for (i32 y = sty1; y < sty2; y++)
					{
						if (bIsImport)
						{
							if (bIsPerParts)
								part.ValueAt(x - stx1, y - sty1) = img.ValueAt(x - stx1, y - sty1);
							else
								part.ValueAt(x - stx1, y - sty1) = img.ValueAt(x + iX * maxRes - x1, y + iY * maxRes - y1);
						}
						else
							img.ValueAt(x + iX * maxRes - x1, y + iY * maxRes - y1) = part.ValueAt(x - stx1, y - sty1);
					}
				if (bIsImport)
					SetSubImageStretched(((float)stx1 / maxRes + iX) / dwCountX, ((float)sty1 / maxRes + iY) / dwCountY,
					                     ((float)stx2 / maxRes + iX) / dwCountX, ((float)sty2 / maxRes + iY) / dwCountY, part, true);

			}
		}

	if (!bIsImport)
	{

		if (pFileName && *pFileName)
		{
			img.SwapRedAndBlue();
			CImageEx newImage;
			newImage.RotateOrt(img, 1);
			CImageUtil::SaveBitmap(pFileName, newImage);
		}
		else
		{
			CClipboard cb;
			cb.PutImage(img);
		}
	}

	return true;
}

bool CRGBLayer::ExportSegment(CMemoryBlock& mem, u32 dwTileX, u32 dwTileY, bool bCompress)
{
	CTerrainTextureTiles* pTile = LoadTileIfNeeded(dwTileX, dwTileY);
	if (!pTile || !pTile->m_pTileImage)
	{
		assert(0);
		return false;
	}

	u32 dwWidth = pTile->m_pTileImage->GetWidth(), dwHeight = pTile->m_pTileImage->GetHeight();

	assert(dwWidth);
	assert(dwHeight);

	CMemoryBlock tmpMem;
	CMemoryBlock& tmp = bCompress ? tmpMem : mem;

	i32k bpp = 3;
	tmp.Allocate(sizeof(u32) + sizeof(u32) + dwWidth * dwHeight * bpp);

	u32* pMem32 = (u32*)tmp.GetBuffer();
	u8* pDst8 = (u8*)tmp.GetBuffer() + sizeof(u32) + sizeof(u32);
	u8* pSrc8 = (u8*)pTile->m_pTileImage->GetData();

	pMem32[0] = dwWidth;
	pMem32[1] = dwHeight;

	for (u32 j = 0; j < dwWidth * dwHeight; ++j)
	{
		*pDst8++ = *pSrc8++;
		*pDst8++ = *pSrc8++;
		*pDst8++ = *pSrc8++;
		pSrc8++;
	}

	if (bCompress)
		tmp.Compress(mem);

	return true;
}

CImageEx* CRGBLayer::GetTileImage(i32 tileX, i32 tileY, bool setDirtyFlag /* = true */)
{
	CTerrainTextureTiles* tile = LoadTileIfNeeded(tileX, tileY);
	assert(tile != 0);
	if (setDirtyFlag)
		tile->m_bDirty = true;
	return tile->m_pTileImage;
}

void CRGBLayer::UnloadTile(i32 tileX, i32 tileY)
{
	CTerrainTextureTiles* pTile = GetTilePtr(tileX, tileY);
	if (!pTile)
		return;
	FreeTile(*pTile);
}

void TransferSubimageFast(CImageEx& source, const Rectf& srcRect, CImageEx& dest, const Rectf& destRect)
{
	// Images are expected to be in the same format, square and with size = 2^n, rectangles are also square and in relative coordinates.
	Recti srcRectI, destRectI;
	srcRectI.Min.x = (i32)(srcRect.Min.x * source.GetWidth());  // Exchange x and y in for correct subtile coordinates
	srcRectI.Min.y = (i32)(srcRect.Min.y * source.GetHeight());
	srcRectI.Max.x = (i32)(srcRect.Max.x * source.GetWidth());
	srcRectI.Max.y = (i32)(srcRect.Max.y * source.GetHeight());
	destRectI.Min.x = (i32)(destRect.Min.x * dest.GetWidth());
	destRectI.Min.y = (i32)(destRect.Min.y * dest.GetHeight());
	destRectI.Max.x = (i32)(destRect.Max.x * dest.GetWidth());
	destRectI.Max.y = (i32)(destRect.Max.y * dest.GetHeight());
	i32 nSrcRectSize = srcRectI.GetWidth();
	i32 nDestRectSize = destRectI.GetWidth();
	i32 nSrcImageSize = source.GetWidth();
	i32 nDestImageSize = dest.GetWidth();

	u32* pSrcData = source.GetData() + srcRectI.Min.x + srcRectI.Min.y * nSrcImageSize;
	u32* pDestData = dest.GetData() + destRectI.Min.x + destRectI.Min.y * nDestImageSize;

	if (nSrcRectSize > nDestRectSize)
	{
		// Downscaling
		i32 nStride = nSrcRectSize / nDestRectSize;
		for (i32 row = 0; row < nDestRectSize; ++row)
		{
			u32* pSrcRow = pSrcData + row * nSrcImageSize * nStride;
			u32* pDestRow = pDestData + row * nDestImageSize;
			for (i32 col = 0; col < nDestRectSize; ++col)
			{
				*(pDestRow++) = *pSrcRow;
				pSrcRow += nStride;
			}
		}
	}
	else if (nDestRectSize > nSrcRectSize)
	{
		// Upscaling
		i32 nScale = nDestRectSize / nSrcRectSize;
		for (i32 row = 0; row < nSrcRectSize; ++row)
		{
			u32* pSrcRow = pSrcData + row * nSrcImageSize;
			for (i32 y = 0; y < nScale; ++y)
			{
				u32* pDestRow = pDestData + (row * nScale + y) * nDestImageSize;
				for (i32 col = 0; col < nSrcRectSize; ++col)
				{
					for (i32 x = 0; x < nScale; ++x)
						*(pDestRow++) = *pSrcRow;
					++pSrcRow;
				}
			}
		}
	}
	else
	{
		// Copying
		for (i32 row = 0; row < nDestRectSize; ++row)
			memmove(pDestData + row * nDestImageSize, pSrcData + row * nSrcImageSize, nSrcRectSize * 4);
	}
}

void CRGBLayer::GetSubimageFast(const float fSrcLeft, const float fSrcTop, const float fSrcRight, const float fSrcBottom, CImageEx& rOutImage)
{
	Rectf rectTiles(fSrcLeft * GetTileCountX(), fSrcTop * GetTileCountY(), fSrcRight * GetTileCountX(), fSrcBottom * GetTileCountY());
	float fTilesToRect = 1.0f / rectTiles.GetWidth();

	i32 nMinXTiles = (i32)rectTiles.Min.x;
	i32 nMinYTiles = (i32)rectTiles.Min.y;
	i32 nMaxXTiles = (i32)(rectTiles.Max.x - 0.00001); // Don't consider the last incident tile to the right
	i32 nMaxYTiles = (i32)(rectTiles.Max.y - 0.00001); // and to the bottom if only its boundary is touched.

	for (i32 nTileX = nMinXTiles; nTileX <= nMaxXTiles; ++nTileX)
		for (i32 nTileY = nMinYTiles; nTileY <= nMaxYTiles; ++nTileY)
		{
			CImageEx* pTileImage = GetTileImage(nTileX, nTileY, false);
			Rectf::Vec origin(nTileX, nTileY);
			Rectf srcRect;
			srcRect.Min.x = MAX(0.0f, rectTiles.Min.x - origin.x);
			srcRect.Min.y = MAX(0.0f, rectTiles.Min.y - origin.y);
			srcRect.Max.x = MIN(1.0f, rectTiles.Max.x - origin.x);
			srcRect.Max.y = MIN(1.0f, rectTiles.Max.y - origin.y);
			Rectf destRect;
			destRect.Min.x = MAX(0.0f, (origin.x - rectTiles.Min.x) * fTilesToRect);
			destRect.Min.y = MAX(0.0f, (origin.y - rectTiles.Min.y) * fTilesToRect);
			destRect.Max.x = MIN(1.0f, (origin.x + 1.0f - rectTiles.Min.x) * fTilesToRect);
			destRect.Max.y = MIN(1.0f, (origin.y + 1.0f - rectTiles.Min.y) * fTilesToRect);
			TransferSubimageFast(*pTileImage, srcRect, rOutImage, destRect);
		}
}

void CRGBLayer::SetSubimageFast(const float fDestLeft, const float fDestTop, const float fDestRight, const float fDestBottom, CImageEx& rSrcImage)
{
	Rectf rectTiles(fDestLeft * GetTileCountX(), fDestTop * GetTileCountY(), fDestRight * GetTileCountX(), fDestBottom * GetTileCountY());
	float fTilesToRect = 1.0f / rectTiles.GetWidth();

	i32 nMinXTiles = (i32)rectTiles.Min.x;
	i32 nMinYTiles = (i32)rectTiles.Min.y;
	i32 nMaxXTiles = (i32)(rectTiles.Max.x - 0.00001); // Don't consider the last incident tile to the right
	i32 nMaxYTiles = (i32)(rectTiles.Max.y - 0.00001); // and to the bottom if only its boundary is touched.

	for (i32 nTileX = nMinXTiles; nTileX <= nMaxXTiles; ++nTileX)
		for (i32 nTileY = nMinYTiles; nTileY <= nMaxYTiles; ++nTileY)
		{
			CImageEx* pTileImage = GetTileImage(nTileX, nTileY, false);
			Rectf::Vec origin(nTileX, nTileY);
			Rectf destRect;
			destRect.Min.x = MAX(0.0f, rectTiles.Min.x - origin.x);
			destRect.Min.y = MAX(0.0f, rectTiles.Min.y - origin.y);
			destRect.Max.x = MIN(1.0f, rectTiles.Max.x - origin.x);
			destRect.Max.y = MIN(1.0f, rectTiles.Max.y - origin.y);
			Rectf srcRect;
			srcRect.Min.x = MAX(0.0f, (origin.x - rectTiles.Min.x) * fTilesToRect);
			srcRect.Min.y = MAX(0.0f, (origin.y - rectTiles.Min.y) * fTilesToRect);
			srcRect.Max.x = MIN(1.0f, (origin.x + 1.0f - rectTiles.Min.x) * fTilesToRect);
			srcRect.Max.y = MIN(1.0f, (origin.y + 1.0f - rectTiles.Min.y) * fTilesToRect);
			TransferSubimageFast(rSrcImage, srcRect, *pTileImage, destRect);
		}
}

