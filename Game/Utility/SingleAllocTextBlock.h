// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
SingleAllocTextBlock.h
Created December 2009 by Tim Furnish
*************************************************************************/

// A class which calculates how much memory is needed to duplicate a series of char buffers,
// allocates the memory in a single chunk and then uses the allocated memory to store copies
// of the buffers. Memory is freed up when class is destroyed or by calling Reset().

// NB: No real reason why this couldn't also be used for doing a single memory allocation
// for several objects of types other than char buffers, but that would require some memory
// alignment code adding, so it's not here yet.

#ifndef __SINGLEALLOCTEXTBLOCK_H__
#define __SINGLEALLOCTEXTBLOCK_H__

class CSingleAllocTextBlock
{
	public:
	struct SReuseDuplicatedStrings
	{
		tukk  m_charPtr;
	};

	CSingleAllocTextBlock();
	~CSingleAllocTextBlock();
	void Reset();
	void EmptyWithoutFreeing();
	void IncreaseSizeNeeded(size_t theSize);
	void IncreaseSizeNeeded(tukk  textIn, bool doDuplicateCheck = true);
	tukk  StoreText(tukk  textIn, bool doDuplicateCheck = true);
	void Allocate();
	void Lock();

	ILINE size_t GetSizeNeeded() const
	{
		return m_sizeNeeded;
	}

	ILINE tukk  GetMem() const
	{
		return m_mem;
	}

	ILINE size_t GetNumBytesUsed() const
	{
		return m_numBytesUsed;
	}

	ILINE void SetDuplicatedStringWorkspace(SReuseDuplicatedStrings * theArray, i32 arraySize)
	{
		assert (m_reuseDuplicatedStringsArray == NULL);
		assert (m_reuseDuplicatedStringsArraySize == 0);
		assert (m_reuseDuplicatedStringsNumUsed == 0);

		m_reuseDuplicatedStringsArray = theArray;
		m_reuseDuplicatedStringsArraySize = arraySize;
	}

	private:
	tukk  FindDuplicate(tukk  textIn);
	void RememberPossibleDuplicate(tukk  textIn);

	char *                     m_mem;
	size_t                     m_sizeNeeded;
	size_t                     m_sizeNeededWithoutUsingDuplicates;
	size_t                     m_numBytesUsed;
	SReuseDuplicatedStrings *  m_reuseDuplicatedStringsArray;
	i32                        m_reuseDuplicatedStringsArraySize;
	i32                        m_reuseDuplicatedStringsNumUsed;
};

#endif // __SINGLEALLOCTEXTBLOCK_H__