#ifndef D3_ALIGNED_ALLOCATOR
#define D3_ALIGNED_ALLOCATOR

///we probably replace this with our own aligned memory allocator
///so we replace _aligned_malloc and _aligned_free with our own
///that is better portable and more predictable

#include <drx3D/Common/b3Scalar.h>
//#define D3_DEBUG_MEMORY_ALLOCATIONS 1
#ifdef D3_DEBUG_MEMORY_ALLOCATIONS

#define b3AlignedAlloc(a, b) \
	b3AlignedAllocInternal(a, b, __LINE__, __FILE__)

#define b3AlignedFree(ptr) \
	b3AlignedFreeInternal(ptr, __LINE__, __FILE__)

uk b3AlignedAllocInternal(size_t size, i32 alignment, i32 line, tuk filename);

void b3AlignedFreeInternal(uk ptr, i32 line, tuk filename);

#else
uk b3AlignedAllocInternal(size_t size, i32 alignment);
void b3AlignedFreeInternal(uk ptr);

#define b3AlignedAlloc(size, alignment) b3AlignedAllocInternal(size, alignment)
#define b3AlignedFree(ptr) b3AlignedFreeInternal(ptr)

#endif
typedef i32 SizeType;

typedef uk (b3AlignedAllocFunc)(size_t size, i32 alignment);
typedef void(b3AlignedFreeFunc)(uk memblock);
typedef uk (b3AllocFunc)(size_t size);
typedef void(b3FreeFunc)(uk memblock);

///The developer can let all drx3D memory allocations go through a custom memory allocator, using b3AlignedAllocSetCustom
void b3AlignedAllocSetCustom(b3AllocFunc* allocFunc, b3FreeFunc* freeFunc);
///If the developer has already an custom aligned allocator, then b3AlignedAllocSetCustomAligned can be used. The default aligned allocator pre-allocates extra memory using the non-aligned allocator, and instruments it.
void b3AlignedAllocSetCustomAligned(b3AlignedAllocFunc* allocFunc, b3AlignedFreeFunc* freeFunc);

///The b3AlignedAllocator is a portable class for aligned memory allocations.
///Default implementations for unaligned and aligned allocations can be overridden by a custom allocator using b3AlignedAllocSetCustom and b3AlignedAllocSetCustomAligned.
template <typename T, unsigned Alignment>
class b3AlignedAllocator
{
	typedef b3AlignedAllocator<T, Alignment> self_type;

public:
	//just going down a list:
	b3AlignedAllocator() {}
	/*
	b3AlignedAllocator( const self_type & ) {}
	*/

	template <typename Other>
	b3AlignedAllocator(const b3AlignedAllocator<Other, Alignment>&)
	{
	}

	typedef const T* const_pointer;
	typedef const T& const_reference;
	typedef T* pointer;
	typedef T& reference;
	typedef T value_type;

	pointer address(reference ref) const { return &ref; }
	const_pointer address(const_reference ref) const { return &ref; }
	pointer allocate(SizeType n, const_pointer* hint = 0)
	{
		(void)hint;
		return reinterpret_cast<pointer>(b3AlignedAlloc(sizeof(value_type) * n, Alignment));
	}
	void construct(pointer ptr, const value_type& value) { new (ptr) value_type(value); }
	void deallocate(pointer ptr)
	{
		b3AlignedFree(reinterpret_cast<uk>(ptr));
	}
	void destroy(pointer ptr) { ptr->~value_type(); }

	template <typename O>
	struct rebind
	{
		typedef b3AlignedAllocator<O, Alignment> other;
	};
	template <typename O>
	self_type& operator=(const b3AlignedAllocator<O, Alignment>&)
	{
		return *this;
	}

	friend bool operator==(const self_type&, const self_type&) { return true; }
};

#endif  //D3_ALIGNED_ALLOCATOR
