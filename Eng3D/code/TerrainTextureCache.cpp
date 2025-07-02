// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   TerrainTextureCache.cpp
//  Version:     v1.00
//  Created:     28/1/2007 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Управление текстурой ландшафта.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/terrain_sector.h>
#include <drx3D/Eng3D/terrain.h>

CTextureCache::CTextureCache()
{
	m_eTexFormat = eTF_Unknown;
	m_nDim = 0;
	m_nPoolTexId = 0;
	m_nPoolDim = 0;
}

CTextureCache::~CTextureCache()
{
	if (GetPoolSize())
		assert(m_FreeTextures.Count() + m_Quarantine.Count() + m_UsedTextures.Count() == GetPoolItemsNum());

	ResetTexturePool();
}

void CTextureCache::ResetTexturePool()
{
	m_FreeTextures.AddList(m_UsedTextures);
	m_UsedTextures.Clear();
	m_FreeTextures.AddList(m_Quarantine);
	m_Quarantine.Clear();
	if (GetRenderer())
	{
		GetRenderer()->RemoveTexture(m_nPoolTexId);
	}
	m_FreeTextures.Clear();
	m_eTexFormat = eTF_Unknown;
	m_nDim = 0;
	m_nPoolTexId = 0;
	m_nPoolDim = 0;
}

u16 CTextureCache::GetTexture(byte* pData, u16& nSlotId)
{
	if (!m_FreeTextures.Count())
		Update();

	if (!m_FreeTextures.Count())
	{
		Error("CTextureCache::GetTexture: !m_FreeTextures.Count()");
		return 0;
	}

	nSlotId = m_FreeTextures.Last();

	m_FreeTextures.DeleteLast();

	RectI region;
	GetSlotRegion(region, nSlotId);

	GetRenderer()->UploadToVideoMemory(pData, m_nDim, m_nDim, m_eTexFormat, m_eTexFormat, 0, false, FILTER_NONE, m_nPoolTexId,
		NULL, FT_USAGE_ALLOWREADSRGB, GetTerrain()->GetEndianOfTexture(), &region);

	m_UsedTextures.Add(nSlotId);

	assert(m_FreeTextures.Count() + m_Quarantine.Count() + m_UsedTextures.Count() == GetPoolItemsNum());

	return m_nPoolTexId;
}

void CTextureCache::UpdateTexture(byte* pData, u16k& nSlotId)
{
	FUNCTION_PROFILER_3DENGINE;

	assert(nSlotId >= 0 && nSlotId < GetPoolSize());

	RectI region;
	GetSlotRegion(region, nSlotId);

	GetRenderer()->UploadToVideoMemory(pData, m_nDim, m_nDim, m_eTexFormat, m_eTexFormat, 0, false, FILTER_NONE, m_nPoolTexId,
	                                     NULL, FT_USAGE_ALLOWREADSRGB, GetTerrain()->GetEndianOfTexture(), &region);
}

void CTextureCache::ReleaseTexture(u16& nTexId, u16& nTexSlotId)
{
	assert(nTexId == m_nPoolTexId);

	if (m_UsedTextures.Delete(nTexSlotId))
	{
		m_Quarantine.Add(nTexSlotId);

		nTexId = 0;
		nTexSlotId = ~0;
	}
	else
		assert(!"Attempt to release non pooled texture");

	assert(m_FreeTextures.Count() + m_Quarantine.Count() + m_UsedTextures.Count() == GetPoolItemsNum());
}

bool CTextureCache::Update()
{
	m_FreeTextures.AddList(m_Quarantine);
	m_Quarantine.Clear();

	return GetPoolSize() == GetPoolItemsNum();
}

i32 CTextureCache::GetPoolSize()
{
	return (m_FreeTextures.Count() + m_Quarantine.Count() + m_UsedTextures.Count());
}

void CTextureCache::InitPool(byte* pData, i32 nDim, ETEX_Format eTexFormat)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Texture, 0, "Terrain texture cache");

	ResetTexturePool();

	m_eTexFormat = eTexFormat;
	m_nDim = nDim;

	m_nPoolDim = (i32)sqrt(GetPoolItemsNum());
	i32 nPoolTexDim = m_nPoolDim * m_nDim;

	stack_string sPoolTextureName;
	sPoolTextureName.Format("$TERRAIN_TEX_POOL_%p", this);

	m_nPoolTexId = GetRenderer()->UploadToVideoMemory(NULL, nPoolTexDim, nPoolTexDim, eTexFormat, eTexFormat, 0, false, FILTER_NONE, 0,
		                                                  sPoolTextureName, (eTexFormat == eTF_R32F) ? 0 : FT_USAGE_ALLOWREADSRGB, GetPlatformEndian(), NULL, false);

	if (m_nPoolTexId <= 0)
	{
		if (!gEnv->IsDedicated())
			Error("Debug: CTextureCache::InitPool: GetRenderer()->DownLoadToVideoMemory returned %d", m_nPoolTexId);

		if (!gEnv->IsDedicated())
			Error("Debug: DownLoadToVideoMemory() params: dim=%d, eTexFormat=%d", nDim, eTexFormat);
	}

	for (i32 i = GetPoolItemsNum() - 1; i >= 0; i--)
	{
		m_FreeTextures.Add(i);
	}

	assert(m_FreeTextures.Count() + m_Quarantine.Count() + m_UsedTextures.Count() == GetPoolItemsNum());
}

void CTextureCache::GetSlotRegion(RectI& region, i32 nSlotId)
{
	region.w = region.h = m_nDim;
	region.x = (nSlotId / m_nPoolDim) * m_nDim;
	region.y = (nSlotId % m_nPoolDim) * m_nDim;
}

i32 CTextureCache::GetPoolItemsNum()
{
	if (m_eTexFormat == eTF_R8G8B8A8)
	{
		// allocate less items if texture compression is not used
		return GetCVars()->e_TerrainTextureStreamingPoolItemsNum / 4;
	}

	return GetCVars()->e_TerrainTextureStreamingPoolItemsNum;
}