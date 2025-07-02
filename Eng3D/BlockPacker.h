// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if defined(FEATURE_SVO_GI)

	#include <vector>         // STL vector<>

struct SBlockMinMax
{
	SBlockMinMax()
	{
		ZeroStruct(*this);
		MarkFree();
	}

	u8         m_dwMinX; // 0xffffffff if free, included
	u8         m_dwMinY; // not defined if free, included
	u8         m_dwMinZ; // not defined if free, included
	u8         m_dwMaxX; // not defined if free, not included
	u8         m_dwMaxY; // not defined if free, not included
	u8         m_dwMaxZ; // not defined if free, not included
	u16        m_nDataSize;
	uk         m_pUserData;
	u32        m_nLastVisFrameId;

	SBlockMinMax* m_pNext, * m_pPrev;

	bool          IsFree() const
	{
		return m_dwMinX == 0xff;
	}

	void MarkFree()
	{
		m_dwMinX = 0xff;
		m_pUserData = 0;
		m_nDataSize = m_nLastVisFrameId = 0;
	}
};
      
class CBlockPacker3D
{
public:
	float m_fLastUsed;

public:
	// constructor
	// Arguments:
	//   dwLogHeight - e.g. specify 5 for 32, keep is small like ~ 5 or 6, don't use pixel size
	//   dwLogHeight - e.g. specify 5 for 32, keep is small like ~ 5 or 6, don't use pixel size
	CBlockPacker3D(u32k dwLogWidth, u32k dwLogHeight, u32k dwLogDepth, const bool bNonPow = false);

	// Arguments:
	//   dwLogHeight - e.g. specify 5 for 32
	//   dwLogHeight - e.g. specify 5 for 32
	// Returns:
	//   block * or 0 if there was no free space
	SBlockMinMax* AddBlock(u32k dwLogWidth, u32k dwLogHeight, u32k dwLogDepth, uk pUserData, u32 nCreateFrameId, u32 nDataSize);

	// Arguments:
	//   dwBlockID - as it was returned from AddBlock()
	SBlockMinMax* GetBlockInfo(u32k dwBlockID);

	void          UpdateSize(i32 nW, i32 nH, i32 nD);

	// Arguments:
	//   dwBlockID - as it was returned from AddBlock()
	void   RemoveBlock(u32k dwBlockID);
	void   RemoveBlock(SBlockMinMax* pInfo);

	u32 GetNumSubBlocks() const
	{
		return m_nUsedBlocks;
	}

	u32 GetNumBlocks() const
	{
		return m_Blocks.size();
	}

	//	void MarkAsInUse(SBlockMinMax * pInfo);

	//	SBlockMinMax * CBlockPacker3D::GetLruBlock();

private: // ----------------------------------------------------------

	// -----------------------------------------------------------------

	//	TDoublyLinkedList<SBlockMinMax> m_BlocksList;
	std::vector<SBlockMinMax> m_Blocks;                 //
	std::vector<u32>       m_BlockBitmap;            // [m_dwWidth*m_dwHeight*m_dwDepth], elements are 0xffffffff if not used
	std::vector<byte>         m_BlockUsageGrid;
	u32                    m_dwWidth;                // >0
	u32                    m_dwHeight;               // >0
	u32                    m_dwDepth;                // >0
	u32                    m_nUsedBlocks;
	// -----------------------------------------------------------------

	//
	void FillRect(const SBlockMinMax& rect, u32 dwValue);

	bool IsFree(const SBlockMinMax& rect);

	i32  GetUsedSlotsCount(const SBlockMinMax& rect);

	void UpdateUsageGrid(const SBlockMinMax& rectIn);

	//
	u32 FindFreeBlockIDOrCreateNew();
};

#endif
