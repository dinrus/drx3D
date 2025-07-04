/*
 * Частично код заимствован из STLPort alloc
 *
 * Разработка (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Разработка (c) 1997
 * Moscow Center for SPARC Technology
 *
 * Разработка (c) 1999
 * Boris Fomitchev
 *
 *
 */

#ifndef _DRX_MEMORY_ALLOCATOR
#define _DRX_MEMORY_ALLOCATOR

#pragma once
#include <algorithm>

#define TRACK_NODE_ALLOC_WITH_MEMREPLAY 1
#define DRX_STL_ALLOC

#if (DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT) || DRX_PLATFORM_APPLE
	#include <sys/mman.h>
#endif

#include <string.h>   // memset

//! Don't use _MAX_BYTES as identifier for Max Bytes, STLPORT defines the same enum.
//! This leads to situation where the wrong enum is choosen in different compilation units,
//! which in case leads to errors(The stlport one is defined as 128).
#if defined(__OS400__) || DRX_PLATFORM_DURANGO || (DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT) || DRX_PLATFORM_MAC || (DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT)
enum {_ALIGNMENT = 16, _ALIGN_SHIFT = 4, __MAX_BYTES = 512, NFREELISTS = 32, ADDRESSSPACE = 2 * 1024 * 1024, ADDRESS_SHIFT = 40};
#else
enum {_ALIGNMENT = 8, _ALIGN_SHIFT = 3, __MAX_BYTES = 512, NFREELISTS = 64, ADDRESSSPACE = 2 * 1024 * 1024, ADDRESS_SHIFT = 20};
#endif /* __OS400__ */

#define DRX_MEMORY_ALLOCATOR

#define S_FREELIST_INDEX(__bytes) ((__bytes - size_t(1)) >> (i32)_ALIGN_SHIFT)

class _Node_alloc_obj
{
public:
	_Node_alloc_obj* _M_next;
};

#if DRX_PLATFORM_DURANGO || (DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT) || DRX_PLATFORM_APPLE || (DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT)
	#define MASK_COUNT 0x000000FFFFFFFFFF
	#define MASK_VALUE 0xFFFFFF
	#define MASK_NEXT  0xFFFFFFFFFF000000
	#define MASK_SHIFT 24
#else
	#define MASK_COUNT 0x000FFFFF
	#define MASK_VALUE 0xFFF
	#define MASK_NEXT  0xFFFFF000
	#define MASK_SHIFT 12
#endif

#define NUM_OBJ 64

struct _Obj_Address
{
	//	i16 * _M_next;
	//	i16
	size_t GetNext(size_t pBase)
	{
		return pBase + (size_t)(_M_value >> MASK_SHIFT);
	}

	//size_t GetNext() {
	//	return (size_t)(_M_value >> 20);
	//}

	size_t GetCount()
	{
		return _M_value & MASK_VALUE;
	}

	void SetNext(/*uk */ size_t pNext)
	{
		_M_value &= MASK_COUNT;
		_M_value |= (size_t)pNext << MASK_SHIFT;
	}

	void SetCount(size_t count)
	{
		_M_value &= MASK_NEXT;
		_M_value |= count & MASK_VALUE;
	}
private:
	size_t _M_value;
	//	i16 * _M_end;
};

//struct _Node_Allocations_Tree {
//	enum { eListSize = _Size / (sizeof(uk ) * _Num_obj); };
//	_Obj_Address * _M_allocations_list[eListSize];
//	i32 _M_Count;
//	_Node_Allocations_Tree * _M_next;
//};

template<i32 _Size>
struct _Node_Allocations_Tree
{
	//! Указатель на конец блока памяти.
	tuk _M_end;

	enum { eListSize = _Size / (sizeof(uk ) * NUM_OBJ) };
	//! Список аллокаций.
	_Obj_Address            _M_allocations_list[eListSize];
	i32                     _M_allocations_count;
	//! Указатель на следующий блок памяти.
	_Node_Allocations_Tree* _M_Block_next;
};

struct _Node_alloc_Mem_block_Huge
{
	//! Указатель на конец блока памяти.
	tuk                       _M_end;
	//! Число.
	i32                         _M_count;
	_Node_alloc_Mem_block_Huge* _M_next;
};

template<i32 _Size>
struct _Node_alloc_Mem_block
{
	//! Указатель на конец блока памяти.
	tuk                       _M_end;
	//! Указатель на следующий блок памяти.
	_Node_alloc_Mem_block_Huge* _M_huge_block;
	_Node_alloc_Mem_block*      _M_next;
};

//! Аллокаторы!.
enum EAllocFreeType
{
	eDrxDefaultMalloc,
	eDrxMallocDrxFreeCRTCleanup,
	eDrxLinuxMalloc
};

template<EAllocFreeType type>
struct Node_Allocator
{
	inline uk pool_alloc(size_t size)
	{
		return DrxModuleMalloc(size);
	};
	inline uk cleanup_alloc(size_t size)
	{
		return DrxCrtMalloc(size);
	};
	inline size_t pool_free(uk ptr)
	{
		DrxModuleFree(ptr);
		return 0;
	};
	inline void cleanup_free(uk ptr)
	{
		DrxCrtFree(ptr);
	};

	inline size_t getSize(uk ptr)
	{
		return DrxCrtSize(ptr);
	}
};

//! partial.
template<>
struct Node_Allocator<eDrxDefaultMalloc>
{
	inline uk pool_alloc(size_t size)
	{
		return malloc(size);
	};
	inline uk cleanup_alloc(size_t size)
	{
		return malloc(size);
	};
	inline size_t pool_free(uk ptr)
	{
		size_t n = DrxCrtSize(ptr);
		free(ptr);
		return n;
	};
	inline void cleanup_free(uk ptr)
	{
		free(ptr);
	};
	inline size_t getSize(uk ptr)
	{
		return DrxCrtSize(ptr);
	}

};

// partial
template<>
struct Node_Allocator<eDrxMallocDrxFreeCRTCleanup>
{
	inline uk pool_alloc(size_t size)
	{
		return DrxCrtMalloc(size);
	};
	inline uk cleanup_alloc(size_t size)
	{
		return DrxCrtMalloc(size);
	};
	inline size_t pool_free(uk ptr)
	{
		return DrxCrtFree(ptr);
	};
	inline void cleanup_free(uk ptr)
	{
		DrxCrtFree(ptr);
	};
	inline size_t getSize(uk ptr)
	{
		return DrxCrtSize(ptr);
	}

};

#if (DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT) || DRX_PLATFORM_APPLE
template<>
struct Node_Allocator<eDrxLinuxMalloc>
{
	struct _MemHead
	{
		uk  ptr;
		size_t size;
	};

	inline uk pool_alloc(size_t size)
	{
	#if (DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT)
		tuk p = (tuk)mmap(NULL, size + sizeof(_MemHead), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_32BIT | MAP_ANONYMOUS, -1, 0);
	#else
		// Mac OS X does not have the MAP_32BIT since it's BSD based, compiling with -fPIC should solve the issue
		tuk p = (tuk)mmap(NULL, size + sizeof(_MemHead), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	#endif
		_MemHead* pH = (_MemHead*)p;
		pH->ptr = p;
		pH->size = size;
		p += sizeof(_MemHead);
		return p;
	};
	inline uk cleanup_alloc(size_t size)
	{
	#if (DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT)
		tuk p = (tuk)mmap(NULL, size + sizeof(_MemHead), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_32BIT | MAP_ANONYMOUS, -1, 0);
	#else
		// Mac OS X does not have the MAP_32BIT since it's BSD based, compiling with -fPIC should solve the issue
		tuk p = (tuk)mmap(NULL, size + sizeof(_MemHead), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	#endif
		_MemHead* pH = (_MemHead*)p;
		pH->ptr = p;
		pH->size = size;
		p += sizeof(_MemHead);
		return p;
	};
	inline size_t pool_free(uk ptr)
	{
		_MemHead* pH = (_MemHead*)((tuk)ptr - sizeof(_MemHead));
		size_t n = pH->size;
		munmap(pH->ptr, pH->size + sizeof(_MemHead));
		return n;
	};
	inline void cleanup_free(uk ptr)
	{
		_MemHead* pH = (_MemHead*)((tuk)ptr - sizeof(_MemHead));
		munmap(pH->ptr, pH->size + sizeof(_MemHead));
	};
	inline size_t getSize(uk ptr)
	{
		_MemHead* pH = (_MemHead*)((tuk)ptr - sizeof(_MemHead));
		return pH->size;
	}
};
#endif

#include <drx3D/CoreX/Thread/DrxAtomics.h>

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	#define USE_WRITELOCKS
#endif

struct InternalCriticalSectionDummy
{
	char padding[128];
};

inline void DrxInternalCreateCriticalSection(uk pCS)
{
	#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	DrxCreateCriticalSectionInplace(pCS);
	#endif
}

template<bool __threads, i32 _Size>
class Node_Alloc_Lock
{
public:

#ifndef USE_WRITELOCKS
	Node_Alloc_Lock(LONG nAllocSize)// : lock(g_lockMemMan)
	{
		if (!m_bInitialized)
		{
			Initialize();
		}

		m_nAllocSection = nAllocSize;
		DrxEnterCriticalSection(&g_globalSections[nAllocSize]);
	}

	~Node_Alloc_Lock()
	{
		DrxLeaveCriticalSection(&g_globalSections[m_nAllocSection]);
	}
#else
	Node_Alloc_Lock(LONG nAllocSize) : lock(g_lockMemMan)
	{
		m_nAllocSection = nAllocSize;
	}

	~Node_Alloc_Lock()
	{
	}

#endif
	inline static bool ATOMIC_CAS(LONG* dst, LONG val, LONG old_val)
	{
		return (*dst = val) == val;
	}

	inline static bool ATOMIC_CAS_PTR(uk * dst, uk val, uk old_val)
	{
		return (*dst = val) == val;
	}

	inline static LONG ATOMIC_INCREMENT(LONG* averyverylongnamex)
	{
		return ++(*averyverylongnamex);
	}

	inline static LONG ATOMIC_DECREMENT(LONG* averyverylongnamex)
	{
		return --(*averyverylongnamex);
	}

	inline static LONG ATOMIC_EXCHANGE_ADD(LONG* averyverylongnamex, LONG averyverylongnamey)
	{
		return *averyverylongnamex += averyverylongnamey;
	}

	//#define ATOMIC_CAS(__dst, __val, __old_val) (DrxInterlockedCompareExchange((LONG*)__dst, (LONG)__val, (LONG)__old_val) == (LONG)__old_val)/*(InterlockedCompareExchange64((LONG *)__dst, (LONG)__val, (LONG)__old_val) == (LONG)__old_val)*/
	//#  ifndef ATOMIC_INCREMENT
	//#    define ATOMIC_INCREMENT(__x)           DrxInterlockedIncrement((i32*)__x)
	//#    define ATOMIC_DECREMENT(__x)           DrxInterlockedDecrement((i32*)__x)
	//#    define ATOMIC_EXCHANGE(__x, __y)       DrxInterlockedExchangeAdd((LONG*)__x, (LONG)__y)
	//#    define ATOMIC_EXCHANGE_ADD(__x, __y)   DrxInterlockedExchangeAdd((LONG*)__x, (LONG)__y)
	//#	 endif

#ifndef USE_WRITELOCKS
	static InternalCriticalSectionDummy g_globalSections[NFREELISTS + 1];
	static bool                         m_bInitialized;
#else
	WriteLock                           lock;
	static  i32                 g_lockMemMan;
#endif

	LONG m_nAllocSection;

#ifndef USE_WRITELOCKS
	static void Initialize()
	{
		if (!m_bInitialized)
		{
			m_bInitialized = true;
			for (i32 i = 0; i < DRX_ARRAY_COUNT(g_globalSections); ++i)
			{
				DrxInternalCreateCriticalSection(&g_globalSections[i]);
			}
		}
	}
	static void DeInitialize()
	{
		if (m_bInitialized)
		{
			m_bInitialized = false;
			for (i32 i = 0; i < DRX_ARRAY_COUNT(g_globalSections); ++i)
			{
				if (&g_globalSections[i])
					#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
					DrxDeleteCriticalSectionInplace(&g_globalSections[i]);
				#else
				 ;//Для Винды надо ещё придумать))
				 #endif
			}

			memset(&g_globalSections[0], 0, sizeof(g_globalSections));
		}
	}

#else

	static void Initialize()
	{
	}

	static void DeInitialize()
	{
	}

#endif
};

#ifndef USE_WRITELOCKS
template<bool __threads, i32 _Size>
InternalCriticalSectionDummy Node_Alloc_Lock<__threads, _Size>::g_globalSections[NFREELISTS + 1];
template<bool __threads, i32 _Size>
bool Node_Alloc_Lock<__threads, _Size>::m_bInitialized = false;
#else
template<bool __threads, i32 _Size>
 i32 Node_Alloc_Lock<__threads, _Size>::g_lockMemMan = 0;
#endif

struct AutoStaticInitialize
{
	AutoStaticInitialize()
	{
		Node_Alloc_Lock<true, 1>::Initialize();
	}

	~AutoStaticInitialize()
	{
		Node_Alloc_Lock<true, 1>::DeInitialize();
	}

};

static AutoStaticInitialize autoInitializer;

template<i32 _Size>
class Node_Alloc_Lock<false, _Size>
{
public:
	Node_Alloc_Lock(LONG) {}
	~Node_Alloc_Lock() {}

	inline static bool ATOMIC_CAS(LONG * dst, LONG val, LONG old_val)
	{
		return DrxInterlockedCompareExchange(dst, val, old_val) == old_val;
	}

	inline static bool ATOMIC_CAS_PTR(uk * dst, uk val, uk old_val)
	{
		return DrxInterlockedCompareExchangePointer(dst, val, old_val) == old_val;
	}

	inline static LONG ATOMIC_INCREMENT(LONG * x)
	{
		return DrxInterlockedIncrement((i32 *)x);
	}

	inline static LONG ATOMIC_DECREMENT(LONG * x)
	{
		return DrxInterlockedDecrement((i32 *)x);
	}

	inline static LONG ATOMIC_EXCHANGE_ADD(LONG * x, LONG y)
	{
		return DrxInterlockedExchangeAdd(x, y);
	}

	//#  ifndef ATOMIC_INCREMENT
	//#    define ATOMIC_INCREMENT(__x)           (++(*__x))
	//#    define ATOMIC_DECREMENT(__x)           (--(*__x))
	//#    define ATOMIC_EXCHANGE(__x, __y)       (*__x = *__y)
	//#    define ATOMIC_EXCHANGE_ADD(__x, __y)   (*__x += __y)
	//#	 endif
};

#define _malloc ::malloc
#define _free   ::free

//! A class that forward node allocator calls directly to CRT.
struct drx_crt_node_allocator
{
	static const size_t MaxSize = ~0;

	static uk        alloc(size_t __n)
	{
		return DrxCrtMalloc(__n);
	}
	static size_t dealloc(uk p)
	{
		return DrxCrtFree(p);
	}
	static uk allocate(size_t __n)
	{
		return alloc(__n);
	}
	static uk allocate(size_t __n, size_t nAlignment)
	{
		return alloc(__n);
	}
	static size_t deallocate(uk __p)
	{
		return dealloc(__p);
	}
	void cleanup() {}
};

template<EAllocFreeType _alloc, bool __threads, i32 _Size>
class node_alloc
{
private:
	static inline size_t _S_round_up(size_t __bytes) { return (((__bytes) + (size_t)_ALIGNMENT - 1) & ~((size_t)_ALIGNMENT - 1)); }
	typedef _Node_alloc_obj _Obj;

public:
	enum
	{
		MaxSize      = __MAX_BYTES,
		MaxAlignment = _ALIGNMENT,
	};

public /*private*/ :

	typedef _Node_alloc_Mem_block<_Size> _Mem_block;

	//! \return Object of size __n, and optionally adds to size __n free list.
	static _Obj* _S_refill(size_t __n);

	//! Allocates a chunk for nobjs of size __p_size.
	//! nobjs may be reduced if it is inconvenient to allocate the requested number.
	static tuk _S_chunk_alloc(size_t __p_size, i32& __nobjs);
	// Chunk allocation state.

	static uk  _M_allocate(size_t __n);
	/* __p may not be 0 */
	static void   _M_deallocate(uk __p, size_t __n);

	static size_t _S_freelist_count(i32 num);
	static size_t _S_freelist_fill(i32 num, uk * _ptr);
	static size_t _S_freelist_count_inside(i32 num, _Obj* first, _Obj* last);
	static size_t _S_fill_from_freelist(i32 num, _Obj* first, _Obj* last, uk *);
	static void   _S_freelist_delete_inside(i32 num, _Obj* first, _Obj* last);
	static void   _S_freememblocks_delete_inside(i32 num, uk first, uk last);
	static size_t _S_freememblocks_count_inside(i32 num, uk first, uk last);

	static size_t _S_get_free(void)
	{
		size_t total = 0;
		for (i32 i = 0; i < NFREELISTS; i++)
		{
			size_t size = _ALIGNMENT * i;
			total += _S_freelist_count(i) * size;
		}
		return total;
	}

#if defined(DO_CLEAN_NODE_ALLOC)
	//A helper class to guaranty the memory pool management:
	//friend struct _Node_alloc_helper;
	//Some compilers (MSVC) seems to have trouble with friend declarations:
public:
	//! Methods to report alloc/dealloc calls to the counter system.
	static size_t& _S_alloc_call(size_t incr = 1);
	static void    _S_dealloc_call();

private:
	//! Free all the allocated chuncks of memory.
	static void _S_chunk_dealloc();

	//! Beginning of the linked list of allocated chunks of memory.
#endif /* DO_CLEAN_NODE_ALLOC */

	//! static _Obj *_S_chunks;.
	static _Node_alloc_Mem_block_Huge*     _S_chunks;
	static _Obj*                           _S_free_list[NFREELISTS];
	static LONG                            _S_freelist_counter[NFREELISTS];
	enum { NTREESCOUNT = 1024 * (ADDRESSSPACE / _Size) }; //!< 2GB.
	static _Node_Allocations_Tree<_Size>*  _S_Node_tree[NTREESCOUNT + 1];

	//! Amount of total allocated memory.
	static LONG                         _S_heap_size;
#ifndef _RELEASE
	static LONG                                 _S_wasted_in_allocation;
	static LONG                                 _S_wasted_in_blocks;
#endif
	static _Mem_block*                  _S_free_mem_blocks;
	static _Node_alloc_Mem_block_Huge*  __pCurrentHugeBlock;

	static void _Register_Huge_Block(_Node_Allocations_Tree<_Size>* pBlock)
	{

		size_t startBlock = (size_t)pBlock >> ADDRESS_SHIFT;
		//		__cas_new_head<_Node_alloc_Mem_block_Huge<_Size>, __threads, _Size>(&_S_Node_tree[startBlock], pBlock);
		size_t endBlock = ((size_t)pBlock + _Size) >> ADDRESS_SHIFT;

		for (; startBlock <= endBlock; ++startBlock)
		{

			_Node_Allocations_Tree<_Size>* pFirstNotZeroOrNotpBlock = _S_Node_tree[startBlock];

			if (!pFirstNotZeroOrNotpBlock)
			{
				_S_Node_tree[startBlock] = pBlock;
				continue;
			}

			while (pFirstNotZeroOrNotpBlock->_M_Block_next && pFirstNotZeroOrNotpBlock->_M_Block_next != pBlock)
			{
				pFirstNotZeroOrNotpBlock = pFirstNotZeroOrNotpBlock->_M_Block_next;
			}

			if (!pFirstNotZeroOrNotpBlock->_M_Block_next && pFirstNotZeroOrNotpBlock != pBlock)
				pFirstNotZeroOrNotpBlock->_M_Block_next = pBlock;
			//	//__cas_new_head_block<_Node_alloc_Mem_block_Huge<_Size>, __threads, _Size>(&_S_Node_tree[startBlock], pBlock);
		}
	}

	static void _Unregister_Huge_Block(_Node_Allocations_Tree<_Size>* pBlock)
	{

		size_t startBlock = (size_t)pBlock >> ADDRESS_SHIFT;
		//		__cas_new_head<_Node_alloc_Mem_block_Huge<_Size>, __threads, _Size>(&_S_Node_tree[startBlock], pBlock);
		size_t endBlock = ((size_t)pBlock + _Size) >> ADDRESS_SHIFT;
		for (; startBlock <= endBlock; ++startBlock)
		{

			_Node_Allocations_Tree<_Size>* startNode = _S_Node_tree[startBlock];

			if (startNode == pBlock)
			{
				_S_Node_tree[startBlock] = pBlock->_M_Block_next;
				continue;
			}
			while (startNode && startNode->_M_Block_next != pBlock)
			{
				startNode = startNode->_M_Block_next;
			}

			if (startNode && startNode->_M_Block_next == pBlock)
			{
				startNode->_M_Block_next = pBlock->_M_Block_next;
			}
		}
	}

	static void Register_Small_Block(uk pObj, size_t count, size_t _n)
	{

#if CAPTURE_REPLAY_LOG && TRACK_NODE_ALLOC_WITH_MEMREPLAY
		DrxGetIMemReplay()->MarkBucket(_n, _ALIGNMENT, pObj, _n * count);
#endif

		//return;
		_Node_Allocations_Tree<_Size>* pBlock = _S_Node_tree[(size_t)pObj >> ADDRESS_SHIFT];

		while ((pBlock->_M_end /*+ sizeof(_Node_alloc_Mem_block_Huge<_Size>)*/<pObj || pBlock> pObj))
			pBlock = pBlock->_M_Block_next;

		//while(pBlock && (pBlock->_M_end /*+ sizeof(_Node_alloc_Mem_block_Huge<_Size>)*/ < pObj || pBlock > pObj))
		//pBlock = pBlock->_M_Block_next;
		//if (!pBlock) {
		//i32 a = 0;
		//}

		pBlock->_M_allocations_list[pBlock->_M_allocations_count].SetNext((tuk)pObj - (tuk)pBlock);
		pBlock->_M_allocations_list[pBlock->_M_allocations_count++].SetCount(count);
		pBlock->_M_allocations_list[pBlock->_M_allocations_count].SetNext((tuk)pObj + count * _n - (tuk)pBlock);

		if (count >= NTREESCOUNT + 1)
		{
			i32 b = 0;
		}
	}

public:
	//! This is needed for proper simple_alloc wrapping.
	static LONG  _S_allocations;
	typedef char value_type;

	static uk allocate(size_t __n, size_t nAlignment)
	{
		// forward to the core function (alignment is checked in calling function)
		return allocate(__n);
	}

	/* __n must be > 0      */
	static uk allocate(size_t __n)
	{
		MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);

		uk ret;

		if (__n > (size_t)__MAX_BYTES)
		{
#if !(DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT) && !DRX_PLATFORM_APPLE
			Node_Allocator<_alloc> all;
#else
			Node_Allocator<eDrxLinuxMalloc> all;
#endif
			ret = all.pool_alloc(__n);
		}
		else
			ret = _M_allocate(__n);

		MEMREPLAY_SCOPE_ALLOC(ret, __n, _ALIGNMENT);

		return ret;
	}

	static uk alloc(size_t __n)
	{
		return allocate(__n);
	}

	static size_t _Find_binary_search(uk p, _Node_Allocations_Tree<_Size>* pBlock)
	{

		size_t size = __MAX_BYTES + 1;

		size_t pBase = (size_t)pBlock;// & 0xFFFF0000;
		_Obj_Address* pAddress = pBlock->_M_allocations_list; //[(size_t)p & 0xFFFF];
		size_t pSubBlock = (size_t)p;//& 0x1FFFF;

		size_t pInBlock;
		size_t low = 0;
		size_t high = pBlock->_M_allocations_count;
		size_t mid = 0;
		while (low <= high)
		{
			mid = (low + high) / 2;
			pInBlock = pAddress[mid].GetNext(pBase);
			if (pInBlock > pSubBlock)
			{
				high = mid - 1;
			}
			else
			{
				if (pInBlock < pSubBlock)
				{
					low = mid + 1;
				}
				else
					break;
			}
		}

		if (mid > high)
			mid = high;

		size = ((size_t)pAddress[mid + 1].GetNext(pBase) - (size_t)pAddress[mid].GetNext(pBase)) / pAddress[mid].GetCount();

		return size;

	}

	static size_t _Find_right_size(uk p)
	{
		size_t size = __MAX_BYTES + 1;
		size_t NodeBlock = (size_t)p >> ADDRESS_SHIFT;

		_Node_Allocations_Tree<_Size>* pNode;
		pNode = _S_Node_tree[NodeBlock];
		if (pNode)
		{
			// check tree
			do
			{
				if (p >= pNode && p <= pNode->_M_end /* + sizeof(_Node_alloc_Mem_block_Huge<_Size>)*/)
				{
					// binary search for exact number
					return _Find_binary_search(p, pNode);
				}
				else
					pNode = pNode->_M_Block_next;
			}
			while (pNode);
		}

		return size;
	}
	/* __p may not be 0 */
	static size_t deallocate(uk __p) //, size_t __n)
	{
		MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);

		size_t ret;

		size_t __n = _Find_right_size(__p);
		if (__n > (size_t)__MAX_BYTES)
		{
#if !(DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT) && !DRX_PLATFORM_APPLE
			Node_Allocator<_alloc> all;
#else
			Node_Allocator<eDrxLinuxMalloc> all;
#endif
			__n = all.pool_free(__p);
		}
		else
			_M_deallocate(__p, __n);

		ret = __n;

		MEMREPLAY_SCOPE_FREE(__p);

		return ret;
	}

	static size_t deallocate(uk __p, size_t __n)
	{
		MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);

		size_t ret;

		if (__n > (size_t)__MAX_BYTES)
		{
#if !(DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT) && !DRX_PLATFORM_APPLE
			Node_Allocator<_alloc> all;
#else
			Node_Allocator<eDrxLinuxMalloc> all;
#endif
			__n = all.pool_free(__p);
		}
		else
			_M_deallocate(__p, __n);

		ret = __n;

		MEMREPLAY_SCOPE_FREE(__p);

		return ret;
	}

	static bool IsInAddressRange(uk p)
	{
		return getSizeEx(p) > 0;
	}

	static bool CanGuaranteeAlignment(size_t nSize, size_t nAlignment)
	{
		if (nSize > (size_t)__MAX_BYTES)
			return false;

		if (nAlignment > (size_t)_ALIGNMENT)
			return false;

		return true;
	}

	//! Helper method to retrieve the size of allocations only within the node allocator.
	//! If the memory block does not originate from within the node allocator, 0 is returned.
	static size_t getSizeEx(uk __p)
	{
		size_t __n = _Find_right_size(__p);
		return __n > (size_t)__MAX_BYTES ? 0 : __n;
	}

	static size_t getSize(uk __p)
	{
		size_t __n = _Find_right_size(__p);
		if (__n > (size_t)__MAX_BYTES)
		{
#if !(DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT) && !DRX_PLATFORM_APPLE
			Node_Allocator<_alloc> all;
#else
			Node_Allocator<eDrxLinuxMalloc> all;
#endif
			__n = all.getSize(__p);
		}
		return __n;
	}

	static void releaseMemory()
	{
		_Node_alloc_Mem_block_Huge* __pcur = _S_chunks, * __pnext = 0;
		while (__pcur)
		{
			__pnext = __pcur->_M_next;
			delete __pcur;
			__pcur = __pnext;
		}
	}

	static size_t dealloc(uk __p) //, size_t __n)
	{
		return deallocate(__p);
	}

	static size_t dealloc(uk __p, size_t __n)
	{
		return deallocate(__p, __n);
	}

	/* __n must be > 0      */
	//	static uk   alloc(size_t __n) { return (__n > (size_t)__MAX_BYTES) ?  Node_Allocator<_alloc>.pool_alloc(__n) : _M_allocate(__n); }
	/* __p may not be 0 */
	//static void  dealloc(uk __p, size_t __n) { if (__n > (size_t)__MAX_BYTES) Node_Allocator<_alloc>.pool_free(__p); else _M_deallocate(__p, __n); }

	static size_t get_heap_size()            { return _S_heap_size; };
#if defined(_RELEASE)
	static size_t get_wasted_in_allocation() { return 0; };
	static size_t get_wasted_in_blocks()     { return 0; };
#else
	static size_t get_wasted_in_allocation() { return _S_wasted_in_allocation; };
	static size_t get_wasted_in_blocks()     { return _S_wasted_in_blocks; };
#endif
	static void   cleanup();
};

template<class _ListElem, bool __threads, i32 __size>
inline void __cas_new_head(_ListElem* * __free_list, _ListElem* __new_head)
{
	_ListElem* __old_head;
	do
	{
		__old_head = *__free_list;
		__new_head->_M_next = __old_head;
	}
	while (!Node_Alloc_Lock<__threads, __size>::ATOMIC_CAS((LONG *)__free_list, (LONG)__new_head, (LONG)__old_head));
}

template<class _ListElem, bool __threads, i32 __size>
inline void __cas_new_head_block(_ListElem* * __free_list, _ListElem* __new_head)
{
	_ListElem* __old_head;
	do
	{
		__old_head = *__free_list;
		__new_head->_M_Block_next = __old_head;
	}
	while (!Node_Alloc_Lock<__threads, __size>::ATOMIC_CAS((LONG *)__free_list, (LONG)__new_head, (LONG)__old_head));
}

template<EAllocFreeType _alloc, bool __threads, i32 _Size>
uk node_alloc<_alloc, __threads, _Size >::_M_allocate(size_t __n)
{

	_Obj* __r;
	LONG allocated = (LONG)S_FREELIST_INDEX(__n);
	Node_Alloc_Lock<__threads, _Size> lock(allocated);
	_Obj* * __my_free_list = _S_free_list + allocated;

	do
	{
		__r = *__my_free_list;
		if (__r == 0)
		{
			__r = _S_refill(__n);
			break;
		}
	}
	while (!Node_Alloc_Lock<__threads, _Size>::ATOMIC_CAS_PTR((uk *)__my_free_list, __r->_M_next, __r));
#if !defined(_RELEASE)
	Node_Alloc_Lock<__threads, _Size>::ATOMIC_EXCHANGE_ADD(&_S_wasted_in_allocation, ((allocated + 1) << _ALIGN_SHIFT) - __n);
	Node_Alloc_Lock<__threads, _Size>::ATOMIC_EXCHANGE_ADD(&_S_wasted_in_blocks, -((allocated + 1) << _ALIGN_SHIFT));
#endif
	LONG * counter = _S_freelist_counter + S_FREELIST_INDEX(__n);
	Node_Alloc_Lock<__threads, _Size>::ATOMIC_DECREMENT(counter);

	//_S_wasted_in_allocation += allocated - __n;
	//_S_wasted_in_blocks -= allocated;
	//--_S_freelist_counter[S_FREELIST_INDEX(__n)];

	return __r;
}

#ifndef __STATIC_CAST
	#define __STATIC_CAST(x, y) static_cast<x>(y)
#endif

#ifndef __REINTERPRET_CAST
	#define __REINTERPRET_CAST(x, y) reinterpret_cast<x>(y)
#endif

template<EAllocFreeType _alloc, bool __threads, i32 _Size>
void node_alloc<_alloc, __threads, _Size >::_M_deallocate(uk __p, size_t __n)
{

	LONG allocated = S_FREELIST_INDEX(__n);
	Node_Alloc_Lock<__threads, _Size> lock(allocated);
	_Obj* * __my_free_list = _S_free_list + allocated;
	_Obj* __pobj = __STATIC_CAST(_Obj*, __p);
	__cas_new_head<_Obj, __threads, _Size>(__my_free_list, __pobj);

#if GARBAGE_MEMORY_ON_FREE
	u8* __pdata = (u8*)(__pobj);
	#if GARBAGE_MEMORY_RANDOM
	for (size_t i = sizeof(_Obj); i < __n; ++i)
	{
		__pdata[i] = rand() & 0xff;
	}
	#else
	size_t a = 0, b = 1;
	for (size_t i = sizeof(_Obj); i < __n; ++i)
	{
		__pdata[i] = a & 0xff;
		a += b;
		if (a > 0xFF)
		{
			b++;
			a &= 0xFF;
		}
	}
	#endif
#endif

#ifndef _RELEASE
	Node_Alloc_Lock<__threads, _Size>::ATOMIC_EXCHANGE_ADD(&_S_wasted_in_allocation, -((allocated + 1) << _ALIGN_SHIFT) + __n);
	Node_Alloc_Lock<__threads, _Size>::ATOMIC_EXCHANGE_ADD(&_S_wasted_in_blocks, ((allocated + 1) << _ALIGN_SHIFT));
#endif
	LONG * counter = _S_freelist_counter + S_FREELIST_INDEX(__n);
	Node_Alloc_Lock<__threads, _Size>::ATOMIC_INCREMENT(counter);

	//_S_wasted_in_allocation -= allocated - __n;
	//_S_wasted_in_blocks += allocated;
	//++_S_freelist_counter[S_FREELIST_INDEX(__n)];
}

/* Returns an object of size __n, and optionally adds to size __n free list.*/
/* We assume that __n is properly aligned.                                  */
/* We hold the allocation lock.                                             */
//template <bool __threads, i32 __inst>

//#define NUM_OBJ 20

template<EAllocFreeType _alloc, bool __threads, i32 _Size>
_Node_alloc_obj* node_alloc<_alloc, __threads, _Size >::_S_refill(size_t __n)
{
	i32 __nobjs = NUM_OBJ;
	__n = _S_round_up(__n);

	Node_Alloc_Lock<__threads, _Size> globalLock(NFREELISTS);
	tuk __chunk = _S_chunk_alloc(__n, __nobjs);

	Register_Small_Block(__chunk, __nobjs, __n);
	if (1 == __nobjs)
	{
		//++_S_freelist_counter[S_FREELIST_INDEX(__n)];
		LONG * counter = _S_freelist_counter + S_FREELIST_INDEX(__n);
		Node_Alloc_Lock<__threads, _Size>::ATOMIC_INCREMENT(counter);
		__pCurrentHugeBlock->_M_count += 1;
		return (_Obj*)__chunk;
	}

	_Obj* * __my_free_list = _S_free_list + S_FREELIST_INDEX(__n);
	_Obj* __result;
	_Obj* __current_obj;
	_Obj* __new_head;
	_Obj* __next_obj;

	/* Build free list in chunk */

	//	_S_freelist_counter[S_FREELIST_INDEX(__n)] += __nobjs;
	LONG * counter = _S_freelist_counter + S_FREELIST_INDEX(__n);
	Node_Alloc_Lock<__threads, _Size>::ATOMIC_EXCHANGE_ADD(counter, __nobjs);
	Node_Alloc_Lock<__threads, _Size>::ATOMIC_EXCHANGE_ADD(alias_cast<LONG *>(&__pCurrentHugeBlock->_M_count), __nobjs);
	//__pCurrentHugeBlock->_M_count += __nobjs;
	__result = (_Obj*)__chunk;
	__new_head = __next_obj = (_Obj*)(__chunk + __n);
	for (--__nobjs; --__nobjs; )
	{
		__current_obj = __next_obj;
		__next_obj = (_Obj*)((tuk)__next_obj + __n);
		__current_obj->_M_next = __next_obj;

	}

	//Link the new free list to the global free list
	do
	{
		__next_obj->_M_next = *__my_free_list;
		// !!!!

	}
	while (!Node_Alloc_Lock<__threads, _Size>::ATOMIC_CAS_PTR((uk *)__my_free_list, __new_head, __next_obj->_M_next));

	return __result;
}

/* We allocate memory in large chunks in order to avoid fragmenting     */
/* the malloc heap too much.                                            */
/* We assume that size is properly aligned.                             */
/* We hold the allocation lock.                                         */

template<EAllocFreeType _alloc, bool __threads, i32 _Size>
tuk node_alloc<_alloc, __threads, _Size >::_S_chunk_alloc(size_t _p_size,
                                                            i32& __nobjs)
{
	tuk __result = 0;
	size_t __total_bytes = _p_size * __nobjs;
#if !(DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT) && !DRX_PLATFORM_APPLE
	Node_Allocator<_alloc> allocator;
#else
	Node_Allocator<eDrxLinuxMalloc> allocator;
#endif

	//We get the first available free block:
	_Mem_block* __pblock;
	LONG * pMemBlock = alias_cast<LONG *>(&_S_free_mem_blocks);

	do
	{
		__pblock = _S_free_mem_blocks;
	}
	while (__pblock != 0 && !Node_Alloc_Lock<__threads, _Size>::ATOMIC_CAS(pMemBlock, (LONG)__pblock->_M_next, (LONG)__pblock));

	if (__pblock != 0)
	{
		size_t __bytes_left = __pblock->_M_end - (tuk)__pblock;
		tuk __new_buf_pos = (tuk)__pblock;

		if ((__bytes_left < __total_bytes) && (__bytes_left >= _p_size))
		{
			//We won't get all the requested objs:
			__nobjs = (i32)(__bytes_left / _p_size);
			__total_bytes = _p_size * __nobjs;
		}

		if (__bytes_left >= __total_bytes)
		{
			__result = (tuk)__pblock;
			__bytes_left -= __total_bytes;
			__new_buf_pos += __total_bytes;
		}

		//We get rid of the leaving part of the mem block:
		if (__bytes_left != 0)
		{
			if ((__result != 0) && (__bytes_left > sizeof(_Mem_block)))
			{
				//We use part of the block and there is enough mem left to put the block back to
				//the free mem blocks:
				_Mem_block* __pnew_block = (_Mem_block*)(__new_buf_pos);
				__pnew_block->_M_end = __pblock->_M_end;
				__cas_new_head<_Node_alloc_Mem_block<_Size>, __threads, _Size>(&_S_free_mem_blocks, __pnew_block);
				//__pCurrentHugeBlock->_M_count += __nobjs;
			}
			else
			{
				if (__result != 0)
					i32 a = 0;
				//A too small block, we put it in the main free list elements:
				/*

				   _Obj* __my_free_list = _S_free_list + S_FREELIST_INDEX(__bytes_left);
				   _Obj* __pobj = __REINTERPRET_CAST(_Obj*, __new_buf_pos);
				   ++_S_freelist_counter[S_FREELIST_INDEX(__bytes_left)];
				   __cas_new_head(__my_free_list, __pobj);
				   if (__result == 0)
				   __pCurrentHugeBlock->_M_count +=  1;

				 */
			}
		}

		if (__result != 0)
		{
			return __result;
		}
	}

	//#define _ALLOCATION_SIZE 4 * 1024 * 1024
#define _ALLOCATION_SIZE 512 * 1024
	//We haven't found a free block or not enough place, we ask memory to the the system:
	size_t __bytes_to_get = _Size; //_ALLOCATION_SIZE;////
	__result = (tuk)allocator.pool_alloc(__bytes_to_get);//(tuk)_malloc(__bytes_to_get);

#if CAPTURE_REPLAY_LOG && TRACK_NODE_ALLOC_WITH_MEMREPLAY
	// Make sure that the memory allocated as an owner that memReplay can track
	DrxGetIMemReplay()->MarkBucket(-2, 4, __result, __bytes_to_get);
#endif

	//++_S_allocations;
	Node_Alloc_Lock<__threads, _Size>::ATOMIC_INCREMENT(&_S_allocations);
	//_S_wasted_in_blocks += __bytes_to_get;
#ifndef _RELEASE
	Node_Alloc_Lock<__threads, _Size>::ATOMIC_EXCHANGE_ADD(&_S_wasted_in_blocks, __bytes_to_get);
#endif
	//this is gonna be optimized away in profile builds
	if (0 == __result)
	{
		// BOOOOOOM!
		__result = 0;
		return 0;
	}

	{
		size_t newHeapSize, oldHeapSize;
		do
		{
			oldHeapSize = _S_heap_size;
			newHeapSize = oldHeapSize + __bytes_to_get;
		}
		while (!Node_Alloc_Lock<__threads, _Size>::ATOMIC_CAS(&_S_heap_size, newHeapSize, oldHeapSize));
	}

	//#  ifdef DO_CLEAN_NODE_ALLOC
	_Node_alloc_Mem_block_Huge* __pBlock = (_Node_alloc_Mem_block_Huge*)(__result);// + __total_bytes);

	_Node_Allocations_Tree<_Size>* _pTreeBlock = (_Node_Allocations_Tree<_Size>*)(__result + sizeof(_Node_alloc_Mem_block_Huge));

	memset(_pTreeBlock->_M_allocations_list, 0, sizeof(_pTreeBlock->_M_allocations_list));

	_pTreeBlock->_M_end = __result + __bytes_to_get;

	__pBlock->_M_end = __result + __bytes_to_get;

	size_t blockSize = _S_round_up(sizeof(_Node_alloc_Mem_block_Huge) + sizeof(_Node_Allocations_Tree<_Size> ));
	__bytes_to_get -= blockSize;

	__pBlock->_M_count = 0;
	_pTreeBlock->_M_Block_next = 0;
	_pTreeBlock->_M_allocations_count = 0;
	_Register_Huge_Block(_pTreeBlock);

#if CAPTURE_REPLAY_LOG && TRACK_NODE_ALLOC_WITH_MEMREPLAY
	DrxGetIMemReplay()->MarkBucket(-1, _ALIGNMENT, __result, blockSize);
#endif

	__cas_new_head<_Node_alloc_Mem_block_Huge, __threads, _Size>(&_S_chunks, __pBlock);
	__result += _S_round_up(blockSize);
	//__pCurrentHugeBlock = __pBlock;

	_Node_alloc_Mem_block_Huge*  __old_block;
	do
	{
		__old_block = __pCurrentHugeBlock;
	}
	while (!Node_Alloc_Lock<__threads, _Size>::ATOMIC_CAS_PTR(alias_cast<uk *>(&__pCurrentHugeBlock), __pBlock, __old_block));

	//__cas_new_head(__pCurrentHugeBlock)

	//#  endif
	_Mem_block* __pnewBlock = (_Mem_block*)(__result + __total_bytes);
	__pnewBlock->_M_end = __result + __bytes_to_get;
	__cas_new_head<_Node_alloc_Mem_block<_Size>, __threads, _Size>(&_S_free_mem_blocks, __pnewBlock);
	return __result;
}

#if defined(DO_CLEAN_NODE_ALLOC)
//template <bool __threads, i32 __inst>
template<EAllocFreeType _alloc, bool __threads, i32 _Size>
size_t& node_alloc<_alloc, __threads, _Size >::_S_alloc_call(size_t incr)
{
	static size_t _S_counter = 0;
	if (incr != 0)
	{
		Node_Alloc_Lock<__threads, _Size>::ATOMIC_INCREMENT(&_S_counter);
	}
	return _S_counter;
}

template<EAllocFreeType _alloc, bool __threads, i32 _Size>
void node_alloc<_alloc, __threads, _Size >::_S_dealloc_call()
{

	size_t* pcounter = &_S_alloc_call(0);
	Node_Alloc_Lock<__threads, _Size>::ATOMIC_DECREMENT(pcounter);
	//As we are only releasing memory on shared library unload, counter
	//can only reach 0 once threads has been stopped so we do not have to
	//check atomic_decrement result.
	if (*pcounter == 0)
	{
		_S_chunk_dealloc();
	}
}
#endif   /* DO_CLEAN_NODE_ALLOC */

/*#endif / * _STLP_USE_LOCK_FREE_IMPLEMENTATION * /*/
template<EAllocFreeType _alloc, bool __threads, i32 _Size>
size_t node_alloc<_alloc, __threads, _Size >::_S_freelist_count(i32 num)
{

	_Obj* * __pList = _S_free_list + num;
	_Obj* __pcur = __REINTERPRET_CAST(_Obj*, *__pList), * __pnext;
	size_t count(0);
	while (__pcur != 0)
	{
		__pnext = __pcur->_M_next;
		__pcur = __pnext;
		++count;
	}
	return count;
}

template<EAllocFreeType _alloc, bool __threads, i32 _Size>
size_t node_alloc<_alloc, __threads, _Size >::_S_freelist_fill(i32 num, uk * _ptr)
{

	_Obj* * __pList = _S_free_list + num;
	_Obj* __pcur = __REINTERPRET_CAST(_Obj*, *__pList), * __pnext;
	size_t count(0);
	while (__pcur != 0)
	{
		__pnext = __pcur->_M_next;
		_ptr[count] = __pcur;
		__pcur = __pnext;
		++count;
	}
	return count;
}

template<EAllocFreeType _alloc, bool __threads, i32 _Size>
size_t node_alloc<_alloc, __threads, _Size >::_S_freelist_count_inside(i32 num, _Obj* first, _Obj* last)
{

	_Obj* * __pList = _S_free_list + num;
	_Obj* __pcur = __REINTERPRET_CAST(_Obj*, *__pList), * __pnext;
	size_t count(0);
	size_t totalcount(0);
	while (__pcur != 0)
	{
		__pnext = __pcur->_M_next;
		if (__pcur >= first && __pcur <= last)
			++count;
		__pcur = __pnext;
		++totalcount;
	}
	return count;
}

template<EAllocFreeType _alloc, bool __threads, i32 _Size>
size_t node_alloc<_alloc, __threads, _Size >::_S_fill_from_freelist(i32 num, _Obj* first, _Obj* last, uk * _ptr)
{

	_Obj* * __pList = _S_free_list + num;
	_Obj* __pcur = __REINTERPRET_CAST(_Obj*, *__pList), * __pnext;
	size_t count(0);
	//size_t totalcount(0);
	while (__pcur != 0)
	{
		__pnext = __pcur->_M_next;
		if (__pcur >= first && __pcur <= last)
		{
			_ptr[count++] = __pcur;
		}
		//			++count;
		__pcur = __pnext;
		//	++totalcount;
	}
	return count;
}

template<EAllocFreeType _alloc, bool __threads, i32 _Size>
void node_alloc<_alloc, __threads, _Size >::_S_freelist_delete_inside(i32 num, _Obj* first, _Obj* last)
{

	_Obj* * __pList = _S_free_list + num;
	_Obj* __pnext, * __pfirst, * __plast, * __r;

	do
	{
		__r = *__pList;
		if (__r < first || __r > last)
		{
			break;
		}
	}
	while (Node_Alloc_Lock<__threads, _Size>::ATOMIC_CAS(__pList, __r->_M_next, __r));

	_Obj* __pcur = __REINTERPRET_CAST(_Obj*, *__pList);
	__pfirst = __pcur;

	size_t count(0);
	while (__pcur != 0)
	{
		__pnext = __pcur->_M_next;
		if (__pcur >= first && __pcur <= last)
		{
			//if (__pnext)
			{
				__cas_new_head<_Obj, __threads, _Size>(&__pnext, __pfirst);
				++count;
			}
		}
		else
		{
			__pfirst = __pcur;
		}
		__pcur = __pnext;
	}

	__pcur = __REINTERPRET_CAST(_Obj*, *__pList);

	// for testing purposes
	/*
	   #ifdef _DEBUG
	   count = 0;
	   while (__pcur != 0) {
	   __pnext = __pcur->_M_next;
	   if (__pcur >= first && __pcur < last)
	   {
	   ++count;
	   }
	   __pcur = __pnext;
	   }
	   #endif
	 */

}

template<EAllocFreeType _alloc, bool __threads, i32 _Size>
size_t node_alloc<_alloc, __threads, _Size >::_S_freememblocks_count_inside(i32 num, uk first, uk last)
{

	_Mem_block** __pList = &_S_free_mem_blocks;
	_Mem_block* __pcur = __REINTERPRET_CAST(_Mem_block*, *__pList), * __pnext;
	size_t count(0);
	size_t totalcount(0);
	while (__pcur != 0)
	{
		__pnext = __pcur->_M_next;
		if ((uk )__pcur >= first && (uk )__pcur <= last)
			count += (tuk)__pcur->_M_end - (tuk)__pcur;
		__pcur = __pnext;
		++totalcount;
	}
	return count;
}

template<EAllocFreeType _alloc, bool __threads, i32 _Size>
void node_alloc<_alloc, __threads, _Size >::_S_freememblocks_delete_inside(i32 num, uk first, uk last)
{

	_Mem_block** __pList = &_S_free_mem_blocks;
	_Mem_block* __r;

	do
	{
		__r = *__pList;
		if (__r < first || __r > last)
		{
			break;
		}
	}
	while (Node_Alloc_Lock<__threads, _Size>::ATOMIC_CAS((LONG*)__pList, (LONG)__r->_M_next, (LONG)__r));

	_Mem_block* __pcur = __REINTERPRET_CAST(_Mem_block*, *__pList), * __pnext, * __pfirst;
	__pfirst = __pcur;

	size_t count(0);
	while (__pcur != 0)
	{
		__pnext = __pcur->_M_next;
		if ((uk )__pcur >= (uk )first && (uk )__pcur <= (uk )last)
		{
			//if (__pnext)
			{
				__cas_new_head<_Mem_block, __threads, _Size>(&__pnext, __pfirst);
				++count;
			}
		}
		else
		{
			__pfirst = __pcur;
		}
		__pcur = __pnext;
	}

	// For testing purposes
	/*
	   #ifdef _DEBUG
	   __pcur = __REINTERPRET_CAST(_Mem_block*, *__pList);

	   count = 0;
	   while (__pcur != 0) {
	   __pnext = __pcur->_M_next;
	   if ((uk )__pcur >= (uk )first && (uk )__pcur <= (uk )last)
	   {
	   ++count;
	   }
	   __pcur = __pnext;
	   }

	   #endif // _DEBUG
	 */

}

#if defined(DO_CLEAN_NODE_ALLOC)
/* We deallocate all the memory chunks      */
template<EAllocFreeType _alloc, bool __threads, i32 _Size>
void node_alloc<_alloc, __threads, _Size >::_S_chunk_dealloc()
{
	_Mem_block* __pcur = _S_chunks, * __pnext;
	while (__pcur != 0)
	{
		__pnext = __pcur->_M_next;
		_free(__pcur);
		__pcur = __pnext;
	}
	_S_chunks = 0;
	_S_free_mem_blocks = 0;
	_S_heap_size = 0;
	memset((tuk)_S_free_list, 0, NFREELISTS * sizeof(_Obj*));
}
#endif /* DO_CLEAN_NODE_ALLOC */

template<EAllocFreeType _alloc, bool __threads, i32 _Size>
_Node_alloc_obj * node_alloc<_alloc, __threads, _Size>::_S_free_list[NFREELISTS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
template<EAllocFreeType _alloc, bool __threads, i32 _Size>
LONG node_alloc<_alloc, __threads, _Size>::_S_freelist_counter[NFREELISTS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
template<EAllocFreeType _alloc, bool __threads, i32 _Size>
_Node_alloc_Mem_block<_Size>* node_alloc<_alloc, __threads, _Size>::_S_free_mem_blocks = 0;
template<EAllocFreeType _alloc, bool __threads, i32 _Size>
LONG node_alloc<_alloc, __threads, _Size>::_S_heap_size = 0;
template<EAllocFreeType _alloc, bool __threads, i32 _Size>
LONG node_alloc<_alloc, __threads, _Size>::_S_allocations = 0;
template<EAllocFreeType _alloc, bool __threads, i32 _Size>
_Node_alloc_Mem_block_Huge* node_alloc<_alloc, __threads, _Size>::__pCurrentHugeBlock = 0;
template<EAllocFreeType _alloc, bool __threads, i32 _Size>
_Node_alloc_Mem_block_Huge* node_alloc<_alloc, __threads, _Size>::_S_chunks = 0;
#ifndef _RELEASE
template<EAllocFreeType _alloc, bool __threads, i32 _Size>
LONG node_alloc<_alloc, __threads, _Size>::_S_wasted_in_allocation = 0;
template<EAllocFreeType _alloc, bool __threads, i32 _Size>
LONG node_alloc<_alloc, __threads, _Size>::_S_wasted_in_blocks = 0;
#endif
template<EAllocFreeType _alloc, bool __threads, i32 _Size>
_Node_Allocations_Tree<_Size>* node_alloc<_alloc, __threads, _Size>::_S_Node_tree[node_alloc < _alloc, __threads, _Size > ::NTREESCOUNT + 1] = { 0 };

template<EAllocFreeType _alloc, bool __threads, i32 _Size>
void node_alloc<_alloc, __threads, _Size >::cleanup()
{

	return;
#if !DRX_PLATFORM_LINUX && !DRX_PLATFORM_ANDROID && !DRX_PLATFORM_APPLE

	#ifndef USE_WRITELOCKS
	Node_Alloc_Lock<true, _Size>::Initialize();
	for (i32 i = 0; i < NFREELISTS; ++i)
	{
		DrxEnterCriticalSection(&Node_Alloc_Lock<true, _Size>::g_globalSections[i]);
	}
	#else

	Node_Alloc_Lock<true, _Size> lock(0);
	#endif

	Node_Allocator<_alloc> allocator;

	_Node_alloc_Mem_block_Huge* __pcur = _S_chunks, * __pnext, * __pfirst;
	__pfirst = __pcur;

	typedef _Obj* ptrtype;

	ptrtype* tmp_free_list[NFREELISTS];
	ptrtype* tmp_low[NFREELISTS];
	ptrtype* tmp_high[NFREELISTS];
	size_t tmp_free_list_size[NFREELISTS];
	size_t tmp_count[NFREELISTS];

	for (size_t i = 0; i < NFREELISTS; ++i)
	{

		size_t count = _S_freelist_counter[i];

		// For testing purposes
		//#ifdef _DEBUG
		//		size_t count1 = _S_freelist_count(i);
		//		if (count != count1)
		//			count = count1;
		//#endif

		tmp_free_list_size[i] = count;
		tmp_free_list[i] = 0;
		if (count)
		{
			tmp_free_list[i] = (ptrtype*)allocator.cleanup_alloc((count + 1) * sizeof(ptrtype)); //::new ptrtype [count];
			_S_freelist_fill(i, (uk *)tmp_free_list[i]);
			//qsort(tmp_free_list[i][0], count, sizeof(_Obj*), compare);

			//ptrtype _min = tmp_free_list[i][0];
			//size_t _minind = 0;
			//for (size_t k = 0; k < count; ++k)
			//{
			//	for (size_t j = k; j < count; ++j)
			//	{
			//		if ((size_t)tmp_free_list[i][j] < (size_t)_min)
			//		{
			//			_min = tmp_free_list[i][j];
			//			_minind = j;
			//		}
			//	}
			//	std::swap(tmp_free_list[i][k], tmp_free_list[i][_minind]);
			//}

			std::sort(&(tmp_free_list[i][0]), &(tmp_free_list[i][count]));
		}
	}

	while (__pcur != 0)
	{
		bool bdelete(false);
		__pnext = __pcur->_M_next;

		size_t freelistsize = 0;
		size_t cursize = __pcur->_M_count; //(size_t)(__pcur->_M_end - (tuk)__pcur);// - sizeof(_Node_alloc_Mem_block_Huge));
		for (size_t i = 0; i < NFREELISTS; ++i)
		{
			size_t count = tmp_free_list_size[i];//- 1;
			tmp_count[i] = 0;
			if (tmp_free_list_size[i] > 0)
			{
				_Obj** low = std::lower_bound(&(tmp_free_list[i][0]), &(tmp_free_list[i][count]), (_Obj*)__pcur);
				_Obj** high = std::upper_bound(&(tmp_free_list[i][0]), &(tmp_free_list[i][count]), (_Obj*)__pcur->_M_end);//*/ + sizeof(_Node_alloc_Mem_block_Huge<_Size>));
				tmp_low[i] = low;
				tmp_high[i] = high;

				if (low == high && ((uk )low < (uk )__pcur || (uk ) high > (uk )__pcur->_M_end))
				{
					//size_t ttt = _S_freelist_count_inside(i, (_Obj*)__pcur, (_Obj*)__pcur->_M_end);// + sizeof(_Node_alloc_Mem_block_Huge));
					//if (ttt != tmp_count[i])
					//	i32 a = 0;

					//For testing purposes
					i32 b = 0;
				}
				else
				{
					tmp_count[i] = high - low;// + 1;
					freelistsize += tmp_count[i];//  * (i + 1) << _ALIGN_SHIFT;
					//For testing purposes
					//#ifdef _DEBUG
					//size_t ttt = _S_freelist_count_inside(i, (_Obj*)__pcur, (_Obj*)__pcur->_M_end);// + sizeof(_Node_alloc_Mem_block_Huge));
					//if (ttt != tmp_count[i])
					//	i32 a = 0;
					//#endif
				}
			}
		}

		if (freelistsize)
		{

			if (cursize != freelistsize)
			{
				i32 a = freelistsize - cursize;
			}
			else
			{
				bdelete = true;

				for (size_t i = 0; i < NFREELISTS; ++i)
				{
					if (tmp_count[i])
					{
						//For testing purposes
						//_S_freelist_delete_inside(i, (_Obj*)__pcur, (_Obj*)__pcur->_M_end /* + sizeof(_Node_alloc_Mem_block)*/);

						if (tmp_count[i] > 1)
						{

							size_t copied = &(tmp_free_list[i][tmp_free_list_size[i]]) - tmp_high[i];// + 1;
							tmp_free_list_size[i] -= tmp_count[i];
							memmove(tmp_low[i], tmp_high[i] /* + 1*/, copied * sizeof(_Obj*));

						}
						else
						{
							tmp_free_list_size[i] = 0;
						}
					}
				}

				_S_freememblocks_delete_inside(0, (_Obj*)__pcur, (_Obj*)__pcur->_M_end);

				// delete from S_chunks
				if (__pfirst == __pcur)
				{
					//ATOMIC_CAS(_S_chunks, __pcur->_M_next, __pcur);
					_S_chunks = __pcur->_M_next;
					__pfirst = _S_chunks;
				}
				else
				{

					__cas_new_head<_Node_alloc_Mem_block_Huge, __threads, _Size>(&__pnext, __pfirst);
				}
				_S_heap_size -= _Size /*cursize*/;
	#ifndef _RELEASE
				_S_wasted_in_blocks -= _Size;
	#endif
				_Unregister_Huge_Block((_Node_Allocations_Tree<_Size>*)((tuk)__pcur + sizeof(_Node_alloc_Mem_block_Huge)));
				// free memory
				//_free(__pcur);
				allocator.pool_free(__pcur);
				--_S_allocations;
			}

			if (!bdelete)
			{
				__pfirst = __pcur;
			}
		}
		__pcur = __pnext;
	}

	for (size_t i = 0; i < NFREELISTS; ++i)
	{
		if (tmp_free_list[i])
		{
			// restore bew free_list
			_Obj* * __my_free_list = _S_free_list + i;
			_S_freelist_counter[i] = tmp_free_list_size[i];

			if (tmp_free_list_size[i] > 0)
			{
				_Obj* _pcur = tmp_free_list[i][0];
				*__my_free_list = _pcur;
				for (size_t j = 1; j < tmp_free_list_size[i]; ++j)
				{
					_pcur->_M_next = tmp_free_list[i][j];
					_pcur = _pcur->_M_next;
				}
				_pcur->_M_next = 0;
			}
			else
			{
				*__my_free_list = 0;
			}

			allocator.cleanup_free(tmp_free_list[i]);

			//::delete [] tmp_free_list[i];
		}
	}

	#ifndef USE_WRITELOCKS
	for (i32 i = 0; i < NFREELISTS; ++i)
	{
		DrxLeaveCriticalSection(&Node_Alloc_Lock<true, _Size>::g_globalSections[i]);
	}
	#endif
#endif
}

#undef S_FREELIST_INDEX

#endif
