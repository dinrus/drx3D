// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   BucketAllocator.h
//  Version:     v1.00
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __BucketAllocator_h__
#define __BucketAllocator_h__
#pragma once

#define BUCKETQUANT 512

// Linux
#ifdef PTRBITS
	#undef PTRBITS
#endif

class PageBucketAllocatorForLua
{
	/*
	   Wouter,
	   Generic allocator that combines bucket allocation with reference counted 1 size object pages.
	   manages to perform well along each axis:
	   - very fast for small objects: only a few instructions in the general case for alloc/dealloc,
	   up to several orders of magnitude faster than traditional best fit allocators
	   - low per object memory overhead: 0 bytes overhead on small objects, small overhead for
	   pages that are partially in use (still significantly lower than other allocators).
	   - almost no fragmentation, reuse of pages is close to optimal
	   - very good cache locality (page aligned, same size objects)
	 */
	enum { EPAGESIZE = 4096 };
	enum { PAGEMASK = (~(EPAGESIZE - 1)) };
	//enum { PAGESATONCE = 64 };
	enum { PAGESATONCE = 32 };
	enum { PAGEBLOCKSIZE = EPAGESIZE * PAGESATONCE };
	enum { PTRSIZE = sizeof(tuk) };
	enum { MAXBUCKETS = BUCKETQUANT / 4 + 1 }; // meaning up to size 512 on 32bit pointer systems
	enum { MAXREUSESIZE = MAXBUCKETS * PTRSIZE - PTRSIZE };
	enum { PTRBITS = PTRSIZE == 2 ? 1 : PTRSIZE == 4 ? 2 : 3 };

	ILINE i32  bucket(i32 s)  { return (s + PTRSIZE - 1) >> PTRBITS; };
	ILINE i32* ppage(uk p) { return (i32*)(((INT_PTR)p) & PAGEMASK); };

	uk  reuse[MAXBUCKETS];
	uk * pages;
	size_t nAllocatedSize;

	void put_in_buckets(tuk start, tuk end, i32 bsize)
	{
		i32 size = bsize * PTRSIZE;
		for (end -= size; start <= end; start += size)
		{
			*((uk *)start) = reuse[bsize];
			reuse[bsize] = start;
		}
		;
	};

	void new_page_blocks()
	{
		tuk b = (tuk)malloc(PAGEBLOCKSIZE);  // if we could get page aligned memory here, that would be even better
		tuk first = ((tuk)ppage(b)) + EPAGESIZE;
		for (i32 i = 0; i < PAGESATONCE - 1; i++)
		{
			uk * p = (uk *)(first + i * EPAGESIZE);
			*p = pages;
			pages = p;
		}
		;
	};

	uk new_page(u32 bsize)
	{
		if (!pages) new_page_blocks();
		uk * page = pages;
		pages = (uk *)*pages;
		*page = 0;
		put_in_buckets((tuk)(page + 1), ((tuk)page) + EPAGESIZE, bsize);
		return alloc(bsize * PTRSIZE);
	};

	void free_page(i32* page, i32 bsize) // worst case if very large amounts of objects get deallocated in random order from when they were allocated
	{
		for (uk * r = &reuse[bsize]; *r; )
		{
			if (page == ppage(*r)) *r = *((uk *)*r);
			else r = (uk *)*r;
		}
		;
		uk * p = (uk *)page;
		*p = pages;
		pages = p;
	};

public:

	PageBucketAllocatorForLua()
	{
		nAllocatedSize = 0;
		pages = NULL;
		for (i32 i = 0; i < MAXBUCKETS; i++) reuse[i] = NULL;
	};

	uk alloc(size_t size)
	{
		nAllocatedSize += size;
		if (size == 0)
			return 0;

		if (size > MAXREUSESIZE)
		{
			return malloc(size);
		}
		size = bucket(size);
		uk * r = (uk *)reuse[size];
		if (!r)
		{
			return new_page(size);
		}
		reuse[size] = *r;
		i32* page = ppage(r);
		(*page)++;
		return (uk )r;
	};

	void dealloc(uk p, size_t size)
	{
		nAllocatedSize -= size;
		if (size > MAXREUSESIZE)
		{
			free(p);
		}
		else
		{
			size = bucket(size);
			*((uk *)p) = reuse[size];
			reuse[size] = p;
			i32* page = ppage(p);
			if (!--(*page)) free_page(page, size);
		};
	};

	uk re_alloc(uk ptr, size_t osize, size_t nsize)
	{
		if (NULL == ptr)
		{
			return alloc(nsize);
		}
		else if (osize > MAXREUSESIZE && nsize > MAXREUSESIZE)
		{
			nAllocatedSize += nsize;
			nAllocatedSize -= osize;
			return realloc(ptr, nsize);
		}
		else
		{
			uk nptr = 0;
			if (nsize)
			{
				nptr = (tuk)alloc(nsize);// + g_nPrecaution;
				memcpy(nptr, ptr, nsize > osize ? osize : nsize);
			}
			dealloc(ptr, osize);
			return nptr;
		}
	}

	size_t get_alloc_size() const { return nAllocatedSize; }

	void   stats()
	{
		i32 totalwaste = 0;
		for (i32 i = 0; i < MAXBUCKETS; i++)
		{
			i32 n = 0;
			for (uk * r = (uk *)reuse[i]; r; r = (uk *)*r) n++;
			if (n)
			{
				i32 waste = i * 4 * n / 1024;
				totalwaste += waste;
				DrxLogAlways("bucket %d -> %d (%d k)\n", i * 4, n, waste);
			}
			;
		}
		;
		DrxLogAlways("totalwaste %d k\n", totalwaste);
	};
};

#endif //__BucketAllocator_h__
