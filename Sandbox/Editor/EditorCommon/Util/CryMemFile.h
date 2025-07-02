// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _CRYMEMFILE_HEADER_
#define _CRYMEMFILE_HEADER_

// derived class to get correct memory allocation/deallocation with custom memory manager - and to avoid memory leaks from calling Detach()
class CDrxMemFile : public CMemFile
{
	virtual BYTE* Alloc(SIZE_T nBytes)
	{
		return (BYTE*) malloc(nBytes);
	}

	virtual void Free(BYTE* lpMem)
	{
		/*return*/ free(lpMem);
	}

	virtual BYTE* Realloc(BYTE* lpMem, SIZE_T nBytes)
	{
		return (BYTE*)realloc(lpMem, nBytes);
	}

public: // ---------------------------------------------------------------

	CDrxMemFile(UINT nGrowBytes = 1024) : CMemFile(nGrowBytes) {}
	CDrxMemFile(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0) : CMemFile(lpBuffer, nBufferSize, nGrowBytes) {}

	virtual ~CDrxMemFile()
	{
		Close();    // call Close() to make sure the Free() is using my v-table
	}

	// only for temporary use
	BYTE* GetMemPtr() const
	{
		return m_lpBuffer;
	}

	BYTE* Detach()
	{
		assert(0);    // dangerous - most likely we cause memory leak - better use GetMemPtr
		return 0;
	}
};

#endif // _CRYMEMFILE_HEADER_

