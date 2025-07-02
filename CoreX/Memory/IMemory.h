// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:

   -------------------------------------------------------------------------
   История:
   - 11:5:2009   : Created by Andrey Honich

*************************************************************************/

#pragma once

#include <drx3D/CoreX/Memory/IDefragAllocator.h>   // <> required for Interfuscator
#include <drx3D/CoreX/Memory/IGeneralMemoryHeap.h> // <> required for Interfuscator

struct IMemoryBlock : public CMultiThreadRefCount
{
	// <interfuscator:shuffle>
	virtual uk GetData() = 0;
	virtual i32   GetSize() = 0;
	// </interfuscator:shuffle>
};
TYPEDEF_AUTOPTR(IMemoryBlock);

//////////////////////////////////////////////////////////////////////////
struct ICustomMemoryBlock : public IMemoryBlock
{
	//! Копирует участок из исходной памяти в указанный буфер вывода.
	virtual void CopyMemoryRegion(uk pOutputBuffer, size_t nOffset, size_t nSize) = 0;
};

//////////////////////////////////////////////////////////////////////////
struct ICustomMemoryHeap : public CMultiThreadRefCount
{
	// <interfuscator:shuffle>
	virtual ICustomMemoryBlock* AllocateBlock(size_t const nAllocateSize, char const* const sUsage, size_t const nAlignment = 16) = 0;
	virtual void                GetMemoryUsage(IDrxSizer* pSizer) = 0;
	virtual size_t              GetAllocated() = 0;
	// </interfuscator:shuffle>
};

class IMemoryAddressRange
{
public:
	// <interfuscator:shuffle>
	virtual void   Release() = 0;

	virtual tuk  GetBaseAddress() const = 0;
	virtual size_t GetPageCount() const = 0;
	virtual size_t GetPageSize() const = 0;

	virtual uk  MapPage(size_t pageIdx) = 0;
	virtual void   UnmapPage(size_t pageIdx) = 0;
	// </interfuscator:shuffle>
protected:
	virtual ~IMemoryAddressRange() {}
};

class IPageMappingHeap
{
public:
	// <interfuscator:shuffle>
	virtual void   Release() = 0;

	virtual size_t GetGranularity() const = 0;
	virtual bool   IsInAddressRange(uk ptr) const = 0;

	virtual size_t FindLargestFreeBlockSize() const = 0;

	virtual uk  Map(size_t sz) = 0;
	virtual void   Unmap(uk ptr, size_t sz) = 0;
	// </interfuscator:shuffle>
protected:
	virtual ~IPageMappingHeap() {}
};
