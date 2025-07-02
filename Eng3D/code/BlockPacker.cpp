// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   BlockPacker.cpp
//  Created:     2012 by Vladimir Kajalin.
//  Описание:    атлас блока 3D.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>

#if defined(FEATURE_SVO_GI)

	#include <drx3D/Eng3D/BlockPacker.h>

i32k nHS = 4;

CBlockPacker3D::CBlockPacker3D(u32k dwLogWidth, u32k dwLogHeight, u32k dwLogDepth, const bool bNonPow)
	: m_fLastUsed(0.0f)
	, m_nUsedBlocks(0)
{
	if (bNonPow)
	{
		m_dwWidth = dwLogWidth;
		m_dwHeight = dwLogHeight;
		m_dwDepth = dwLogDepth;
	}
	else
	{
		m_dwWidth = 1 << dwLogWidth;
		m_dwHeight = 1 << dwLogHeight;
		m_dwDepth = 1 << dwLogDepth;
	}

	m_BlockBitmap.resize(m_dwWidth * m_dwHeight * m_dwDepth, 0xffffffff);
	m_BlockUsageGrid.resize(m_dwWidth * m_dwHeight * m_dwDepth / nHS / nHS / nHS, 0);
	m_Blocks.reserve(m_dwWidth * m_dwHeight * m_dwDepth);
}

SBlockMinMax* CBlockPacker3D::GetBlockInfo(u32k dwBlockID)
{
	u32 dwSize = (u32)m_Blocks.size();

	assert(dwBlockID < dwSize);
	if (dwBlockID >= dwSize)
		return NULL;

	SBlockMinMax& ref = m_Blocks[dwBlockID];

	if (ref.IsFree())
		return NULL;

	return &m_Blocks[dwBlockID];
}

void CBlockPacker3D::UpdateSize(i32 nW, i32 nH, i32 nD)
{
	assert(m_nUsedBlocks == 0);

	m_dwWidth = nW;
	m_dwHeight = nH;
	m_dwDepth = nD;

	m_nUsedBlocks = 0;

	m_BlockBitmap.resize(m_dwWidth * m_dwHeight * m_dwDepth, 0xffffffff);
}

void CBlockPacker3D::RemoveBlock(u32k dwBlockID)
{
	u32 dwSize = (u32)m_Blocks.size();

	assert(dwBlockID < dwSize);
	if (dwBlockID >= dwSize)
		return;     // to avoid crash

	SBlockMinMax& ref = m_Blocks[dwBlockID];

	assert(!ref.IsFree());

	FillRect(ref, 0xffffffff);
	m_nUsedBlocks -= (ref.m_dwMaxX - ref.m_dwMinX) * (ref.m_dwMaxY - ref.m_dwMinY) * (ref.m_dwMaxZ - ref.m_dwMinZ);

	ref.MarkFree();

	//	m_BlocksList.remove(&ref);
}

void CBlockPacker3D::RemoveBlock(SBlockMinMax* pInfo)
{
	SBlockMinMax& ref = *pInfo;

	assert(!ref.IsFree());

	FillRect(ref, 0xffffffff);
	m_nUsedBlocks -= (ref.m_dwMaxX - ref.m_dwMinX) * (ref.m_dwMaxY - ref.m_dwMinY) * (ref.m_dwMaxZ - ref.m_dwMinZ);

	ref.MarkFree();

	//	m_BlocksList.remove(&ref);
}

SBlockMinMax* CBlockPacker3D::AddBlock(u32k dwLogWidth, u32k dwLogHeight, u32k dwLogDepth, uk pUserData, u32 nCreateFrameId, u32 nDataSize)
{
	if (!dwLogWidth || !dwLogHeight || !dwLogDepth)
		assert(!"Empty block");

	u32 dwLocalWidth = dwLogWidth;
	u32 dwLocalHeight = dwLogHeight;
	u32 dwLocalDepth = dwLogDepth;

	i32 nCountNeeded = dwLocalWidth * dwLocalHeight * dwLocalDepth;

	i32 dwW = m_dwWidth / nHS;
	i32 dwH = m_dwHeight / nHS;
	i32 dwD = m_dwDepth / nHS;

	for (i32 nZ = 0; nZ < dwD; nZ++)
		for (i32 nY = 0; nY < dwH; nY++)
			for (i32 nX = 0; nX < dwW; nX++)
			{
				u32 dwMinX = nX * nHS;
				u32 dwMinY = nY * nHS;
				u32 dwMinZ = nZ * nHS;

				u32 dwMaxX = (nX + 1) * nHS;
				u32 dwMaxY = (nY + 1) * nHS;
				u32 dwMaxZ = (nZ + 1) * nHS;

				i32 nCountFree = nHS * nHS * nHS - m_BlockUsageGrid[nX + nY * dwW + nZ * dwW * dwH];

				if (nCountNeeded <= nCountFree)
				{
					SBlockMinMax testblock;
					testblock.m_pUserData = pUserData;
					testblock.m_nLastVisFrameId = nCreateFrameId;
					testblock.m_nDataSize = nDataSize;

					for (u32 dwZ = dwMinZ; dwZ < dwMaxZ; dwZ += dwLocalDepth)
					{
						for (u32 dwY = dwMinY; dwY < dwMaxY; dwY += dwLocalHeight)
						{
							for (u32 dwX = dwMinX; dwX < dwMaxX; dwX += dwLocalWidth)
							{
								testblock.m_dwMinX = dwX;
								testblock.m_dwMaxX = dwX + dwLocalWidth;
								testblock.m_dwMinY = dwY;
								testblock.m_dwMaxY = dwY + dwLocalHeight;
								testblock.m_dwMinZ = dwZ;
								testblock.m_dwMaxZ = dwZ + dwLocalDepth;

								if (IsFree(testblock))
								{
									u32 dwBlockID = FindFreeBlockIDOrCreateNew();

									m_Blocks[dwBlockID] = testblock;

									FillRect(testblock, dwBlockID);

									m_nUsedBlocks += dwLocalWidth * dwLocalHeight * dwLocalDepth;

									//							m_BlocksList.insertBeginning(&m_Blocks[dwBlockID]);

									return &m_Blocks[dwBlockID];
								}
							}
						}
					}
				}
			}

	return NULL;  // no space left to this block
}

void CBlockPacker3D::UpdateUsageGrid(const SBlockMinMax& rectIn)
{
	SBlockMinMax rectUM;

	rectUM.m_dwMinX = rectIn.m_dwMinX / nHS;
	rectUM.m_dwMinY = rectIn.m_dwMinY / nHS;
	rectUM.m_dwMinZ = rectIn.m_dwMinZ / nHS;

	rectUM.m_dwMaxX = (rectIn.m_dwMaxX - 1) / nHS + 1;
	rectUM.m_dwMaxY = (rectIn.m_dwMaxY - 1) / nHS + 1;
	rectUM.m_dwMaxZ = (rectIn.m_dwMaxZ - 1) / nHS + 1;

	i32 dwW = m_dwWidth / nHS;
	i32 dwH = m_dwHeight / nHS;
	i32 dwD = m_dwDepth / nHS;

	for (u32 dwZ = rectUM.m_dwMinZ; dwZ < rectUM.m_dwMaxZ; ++dwZ)
	{
		for (u32 dwY = rectUM.m_dwMinY; dwY < rectUM.m_dwMaxY; ++dwY)
		{
			for (u32 dwX = rectUM.m_dwMinX; dwX < rectUM.m_dwMaxX; ++dwX)
			{
				SBlockMinMax rectTest = rectUM;

				rectTest.m_dwMinX = dwX * nHS;
				rectTest.m_dwMinY = dwY * nHS;
				rectTest.m_dwMinZ = dwZ * nHS;

				rectTest.m_dwMaxX = (dwX + 1) * nHS;
				rectTest.m_dwMaxY = (dwY + 1) * nHS;
				rectTest.m_dwMaxZ = (dwZ + 1) * nHS;

				m_BlockUsageGrid[dwX + dwY * dwW + dwZ * dwW * dwH] = GetUsedSlotsCount(rectTest);
			}
		}
	}
}

void CBlockPacker3D::FillRect(const SBlockMinMax& rect, u32 dwValue)
{
	for (u32 dwZ = rect.m_dwMinZ; dwZ < rect.m_dwMaxZ; ++dwZ)
		for (u32 dwY = rect.m_dwMinY; dwY < rect.m_dwMaxY; ++dwY)
			for (u32 dwX = rect.m_dwMinX; dwX < rect.m_dwMaxX; ++dwX)
				m_BlockBitmap[dwX + dwY * m_dwWidth + dwZ * m_dwWidth * m_dwHeight] = dwValue;

	UpdateUsageGrid(rect);
}

i32 CBlockPacker3D::GetUsedSlotsCount(const SBlockMinMax& rect)
{
	i32 nCount = 0;

	for (u32 dwZ = rect.m_dwMinZ; dwZ < rect.m_dwMaxZ; ++dwZ)
		for (u32 dwY = rect.m_dwMinY; dwY < rect.m_dwMaxY; ++dwY)
			for (u32 dwX = rect.m_dwMinX; dwX < rect.m_dwMaxX; ++dwX)
				if (m_BlockBitmap[dwX + dwY * m_dwWidth + dwZ * m_dwWidth * m_dwHeight] != 0xffffffff)
					nCount++;

	return nCount;
}

bool CBlockPacker3D::IsFree(const SBlockMinMax& rect)
{
	for (u32 dwZ = rect.m_dwMinZ; dwZ < rect.m_dwMaxZ; ++dwZ)
		for (u32 dwY = rect.m_dwMinY; dwY < rect.m_dwMaxY; ++dwY)
			for (u32 dwX = rect.m_dwMinX; dwX < rect.m_dwMaxX; ++dwX)
				if (m_BlockBitmap[dwX + dwY * m_dwWidth + dwZ * m_dwWidth * m_dwHeight] != 0xffffffff)
					return false;

	return true;
}

u32 CBlockPacker3D::FindFreeBlockIDOrCreateNew()
{
	std::vector<SBlockMinMax>::const_iterator it, end = m_Blocks.end();
	u32 dwI = 0;

	for (it = m_Blocks.begin(); it != end; ++it, ++dwI)
	{
		const SBlockMinMax& ref = *it;

		if (ref.IsFree())
			return dwI;
	}

	m_Blocks.push_back(SBlockMinMax());

	return (u32)m_Blocks.size() - 1;
}

#endif
