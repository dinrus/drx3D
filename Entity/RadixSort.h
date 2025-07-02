// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/**
 *	Contains source code from the article "Radix Sort Revisited".
 *	\file		IceRevisitedRadix.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#if defined(NEED_ENDIAN_SWAP)
// The radix sort code below does not work (yet) on big-endian machines.
	#define RADIX_USE_STDSORT 1
#else
	#undef RADIX_USE_STDSORT
#endif

#if !defined RADIX_USE_STDSORT

//! Allocate histograms & offsets locally
	#define RADIX_LOCAL_RAM

enum RadixHint
{
	RADIX_SIGNED,     //!< Input values are signed
	RADIX_UNSIGNED,   //!< Input values are unsigned

	RADIX_FORCE_DWORD = 0x7fffffff
};

class RadixSort
{
public:
	// Constructor/Destructor
	RadixSort();
	~RadixSort();
	// Sorting methods
	RadixSort& Sort(u32k* input, u32 nb, RadixHint hint = RADIX_SIGNED);
	RadixSort& Sort(const float* input, u32 nb);

	//! Access to results. mRanks is a list of indices in sorted order, i.e. in the order you may further process your data
	ILINE u32k* GetRanks()      const { return mRanks;    }

	//! mIndices2 gets trashed on calling the sort routine, but otherwise you can recycle it the way you want.
	ILINE u32* GetRecyclable()   const { return mRanks2;   }

	// Stats
	u32       GetUsedRam()    const;
	//! Returns the total number of calls to the radix sorter.
	ILINE u32 GetNbTotalCalls() const { return mTotalCalls; }
	//! Returns the number of eraly exits due to temporal coherence.
	ILINE u32 GetNbHits()     const   { return mNbHits;   }

	void         GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}
private:
	#ifndef RADIX_LOCAL_RAM
	u32 * mHistogram;            //!< Counters for each byte
	u32* mOffset;                //!< Offsets (nearly a cumulative distribution function)
	#endif
	u32 mCurrentSize;            //!< Current size of the indices list
	u32  mAllocatedSize;         //!< Current size of the indices list in memory
	u32* mRanks;                 //!< Two lists, swapped each pass
	u32* mRanks2;
	// Stats
	u32  mTotalCalls;            //!< Total number of calls to the sort routine
	u32  mNbHits;                //!< Number of early exits due to coherence
	// Internal methods
	void CheckResize(u32 nb);
	bool Resize(u32 nb);
};

#else // !RADIX_USE_STDSORT

enum RadixHint { RADIX_SIGNED, RADIX_UNSIGNED };

class RadixSort
{
	u32*             mRanks[2];
	u32              mAllocatedSize[2];
	u32              mAllocatedSize2;
	static u32k cStaticSize = 1024;
	u32              sRanks[2][cStaticSize];
	u32              mRanksIndex;

	u32* Swap(u32 nb);

public:
	RadixSort();
	~RadixSort();

	RadixSort&    Sort(u32k* input, u32 nb, RadixHint hint = RADIX_SIGNED);
	RadixSort&    Sort(const float* input, u32 nb);

	u32k* GetRanks() const { return mRanks[mRanksIndex]; }

	void          GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}
};

#endif // else !RADIX_USE_STDSORT
