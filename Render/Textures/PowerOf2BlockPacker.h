// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __POWEROF2BLOCKPACKER_H__
#define __POWEROF2BLOCKPACKER_H__

#include <vector>           // STL vector<>

class CPowerOf2BlockPacker
{
public:
	CTexture* m_pTexture;
	CTimeValue m_timeLastUsed;

public:
	// constructor
	// Arguments:
	//   dwLogHeight - e.g. specify 5 for 32, keep is small like ~ 5 or 6, don't use pixel size
	//   dwLogHeight - e.g. specify 5 for 32, keep is small like ~ 5 or 6, don't use pixel size
	CPowerOf2BlockPacker(u32k dwLogWidth, u32k dwLogHeight);

	~CPowerOf2BlockPacker();

	// Arguments:
	//   dwLogHeight - e.g. specify 5 for 32
	//   dwLogHeight - e.g. specify 5 for 32
	// Returns:
	//   dwBlockID (to remove later), 0xffffffff if there was no free space
	u32 AddBlock(u32k dwLogWidth, u32k dwLogHeight);

	// Arguments:
	//   dwBlockID - as it was returned from AddBlock()
	u32 GetBlockInfo(u32k dwBlockID, u32& dwMinX, u32& dwMinY, u32& dwMaxX, u32& dwMaxY);

	void   UpdateSize(i32 nW, i32 nH);

	// Arguments:
	//   dwBlockID - as it was returned from AddBlock()
	void RemoveBlock(u32k dwBlockID);

	// used for debugging
	// Return
	//   0xffffffff if not block was found
	u32 GetRandomBlock() const;

	u32 GetNumUsedBlocks() const
	{
		return m_nUsedBlocks;
	}

	void Clear();

	void FreeContainers();

private: // ----------------------------------------------------------

	struct SBlockMinMax
	{
		u32 m_dwMinX;      // 0xffffffff if free, included
		u32 m_dwMinY;      // not defined if free, included
		u32 m_dwMaxX;      // not defined if free, not included
		u32 m_dwMaxY;      // not defined if free, not included

		bool   IsFree() const
		{
			return m_dwMinX == 0xffffffff;
		}

		void MarkFree()
		{
			m_dwMinX = 0xffffffff;
		}

		~SBlockMinMax()
		{
			MarkFree();
		}
	};

	// -----------------------------------------------------------------

	std::vector<SBlockMinMax> m_Blocks;                 //
	std::vector<u32>       m_BlockBitmap;            // [m_dwWidth*m_dwHeight], elements are 0xffffffff if not used
	u32                    m_dwWidth;                // >0
	u32                    m_dwHeight;               // >0
	u32                    m_nUsedBlocks;

	// -----------------------------------------------------------------

	//
	void FillRect(const SBlockMinMax& rect, u32 dwValue);

	bool IsFree(const SBlockMinMax& rect);

	//
	u32 FindFreeBlockIDOrCreateNew();
};

#endif
