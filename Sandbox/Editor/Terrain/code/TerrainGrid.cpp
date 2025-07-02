// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "TerrainGrid.h"

#include "Terrain/Heightmap.h"
#include "TerrainTexGen.h"

#include <Drx3DEngine/I3DEngine.h>

//////////////////////////////////////////////////////////////////////////
CTerrainGrid::CTerrainGrid()
{
	m_numSectors = 0;
	m_resolution = 0;
	m_sectorSize = 0;
	m_sectorResolution = 0;
	m_pixelsPerMeter = 0;
}

//////////////////////////////////////////////////////////////////////////
CTerrainGrid::~CTerrainGrid()
{
	ReleaseSectorGrid();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainGrid::InitSectorGrid(i32 numSectors)
{
	ReleaseSectorGrid();
	m_numSectors = numSectors;
	m_sectorGrid.resize(m_numSectors * m_numSectors);
	for (i32 i = 0; i < m_numSectors * m_numSectors; i++)
	{
		m_sectorGrid[i] = 0;
	}

	SSectorInfo si;
	GetIEditorImpl()->GetHeightmap()->GetSectorsInfo(si);
	m_sectorSize = si.sectorSize;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainGrid::ReleaseSectorGrid()
{
	IRenderer* pRenderer = GetIEditorImpl()->GetRenderer();

	for (i32 i = 0; i < m_numSectors * m_numSectors; i++)
	{
		if (m_sectorGrid[i])
		{
			pRenderer->RemoveTexture(m_sectorGrid[i]->textureId);

			delete m_sectorGrid[i];
		}
	}
	m_sectorGrid.clear();
	m_numSectors = 0;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainGrid::SetResolution(i32 resolution)
{
	m_resolution = resolution;

	if (abs(m_numSectors) > 0)
	{
		m_sectorResolution = m_resolution / m_numSectors;
	}
	else
	{
		m_sectorResolution = 0;
	}

	if (abs(m_sectorSize) > 0)
	{
		m_pixelsPerMeter = ((float)m_sectorResolution) / m_sectorSize;
	}
	else
	{
		m_pixelsPerMeter = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
CTerrainSector* CTerrainGrid::GetSector(CPoint sector)
{
	assert(sector.x >= 0 && sector.x < m_numSectors && sector.y >= 0 && sector.y < m_numSectors);
	i32 index = sector.x + sector.y * m_numSectors;
	if (!m_sectorGrid[index])
		m_sectorGrid[index] = new CTerrainSector;
	return m_sectorGrid[index];
}

//////////////////////////////////////////////////////////////////////////
CPoint CTerrainGrid::SectorToTexture(CPoint sector)
{
	return CPoint(sector.x * m_sectorResolution, sector.y * m_sectorResolution);
}

//////////////////////////////////////////////////////////////////////////
CPoint CTerrainGrid::WorldToSector(const Vec3& wp)
{
	//swap x/y
	return CPoint(i32(wp.y / m_sectorSize), i32(wp.x / m_sectorSize));
}

//////////////////////////////////////////////////////////////////////////
Vec3 CTerrainGrid::SectorToWorld(CPoint sector)
{
	//swap x/y
	return Vec3(sector.y * m_sectorSize, sector.x * m_sectorSize, 0);
}

//////////////////////////////////////////////////////////////////////////
Vec2 CTerrainGrid::WorldToTexture(const Vec3& wp)
{
	//swap x/y
	return Vec2(wp.y * m_pixelsPerMeter, wp.x * m_pixelsPerMeter);
}

//////////////////////////////////////////////////////////////////////////
void CTerrainGrid::GetSectorRect(CPoint sector, CRect& rect)
{
	rect.left = sector.x * m_sectorResolution;
	rect.top = sector.y * m_sectorResolution;
	rect.right = rect.left + m_sectorResolution;
	rect.bottom = rect.top + m_sectorResolution;
}

//////////////////////////////////////////////////////////////////////////
i32 CTerrainGrid::LockSectorTexture(CPoint sector, u32k dwTextureResolution, bool& bTextureWasRecreated)
{
	CTerrainSector* st = GetSector(sector);
	assert(st);
	GetIEditorImpl()->GetHeightmap()->AddModSector(sector.x, sector.y);
	bTextureWasRecreated = false;

	I3DEngine* p3Engine = GetIEditorImpl()->Get3DEngine();
	IRenderer* pRenderer = GetIEditorImpl()->GetRenderer();

	// if the texture existis already - make sure the size fits
	{
		ITexture* pTex = pRenderer->EF_GetTextureByID(st->textureId);

		if (pTex)
			if (pTex->GetWidth() != dwTextureResolution || pTex->GetHeight() != dwTextureResolution)
			{
				pRenderer->RemoveTexture(st->textureId);
				pTex = 0;

				p3Engine->SetTerrainSectorTexture(sector.y, sector.x, 0);
				st->textureId = 0;
				bTextureWasRecreated = true;
			}
	}

	if (!st->textureId)
	{
		st->textureId = pRenderer->UploadToVideoMemory(0, dwTextureResolution, dwTextureResolution,
		                                                 eTF_R8G8B8A8, eTF_R8G8B8A8, 0, false, FILTER_LINEAR, 0, 0, FT_USAGE_ALLOWREADSRGB);
	}

	p3Engine->SetTerrainSectorTexture(sector.y, sector.x, st->textureId);

	return st->textureId;
}

void CTerrainGrid::UnlockSectorTexture(CPoint sector)
{
	CTerrainSector* st = GetSector(sector);
	assert(st);

	I3DEngine* p3Engine = GetIEditorImpl()->Get3DEngine();
	IRenderer* pRenderer = GetIEditorImpl()->GetRenderer();
	ITexture* pTex = pRenderer->EF_GetTextureByID(st->textureId);

	if (pTex)
	{
		pRenderer->RemoveTexture(st->textureId);
		pTex = 0;

		p3Engine->SetTerrainSectorTexture(sector.y, sector.x, 0);
		st->textureId = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainGrid::GetMemoryUsage(IDrxSizer* pSizer)
{
	pSizer->Add(*this);

	pSizer->Add(m_sectorGrid);

	for (i32 i = 0; i < m_numSectors * m_numSectors; i++)
		if (m_sectorGrid[i])
			pSizer->Add(*m_sectorGrid[i]);
}

