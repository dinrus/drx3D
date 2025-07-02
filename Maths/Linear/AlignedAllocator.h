#ifndef DRX3D_ALIGNED_ALLOCATOR
#define DRX3D_ALIGNED_ALLOCATOR

///we probably replace this with our own aligned memory allocator
///so we replace _aligned_malloc and _aligned_free with our own
///that is better portable and more predictable

#include <drx3D/Maths/Linear/Scalar.h>

///Препроцессор DRX3D_DEBUG_MEMORY_ALLOCATIONS можно установить в системе построения
///для регрессивных тестов на обнаружение утечек памяти.
///#define DRX3D_DEBUG_MEMORY_ALLOCATIONS 1
#ifdef DRX3D_DEBUG_MEMORY_ALLOCATIONS

i32 DumpMemoryLeaks();

#define AlignedAlloc(a, b) \
	AlignedAllocInternal(a, b, __LINE__, __FILE__)

#define AlignedFree(ptr) \
	AlignedFreeInternal(ptr, __LINE__, __FILE__)

uk AlignedAllocInternal(size_t size, i32 alignment, i32 line, tukk filename);

void AlignedFreeInternal(uk ptr, i32 line, tukk filename);

#else
uk AlignedAllocInternal(size_t size, i32 alignment);
void AlignedFreeInternal(uk ptr);

#define AlignedAlloc(size, alignment) AlignedAllocInternal(size, alignment)
#define AlignedFree(ptr) AlignedFreeInternal(ptr)

#endif
typedef i32 size_type;

typedef uk (AlignedAllocFunc)(size_t size, i32 alignment);
typedef void(AlignedFreeFunc)(uk memblock);
typedef uk (AllocFunc)(size_t size);
typedef void(FreeFunc)(uk memblock);

///Разработчик может пропускать все размещения памяти drx3D через кастомный разместитель
/// памяти, используя AlignedAllocSetCustom.
void AlignedAllocSetCustom(AllocFunc* allocFunc, FreeFunc* freeFunc);
///Если у разработчика уже есть кастомный алинованный разместитель, то можно использовать
/// AlignedAllocSetCustomAligned. Дефолтный алинованный разместитель предразмещает дополнительную
/// память, используя неалинованный разместитель и инструментирует (оснащает) её.
void AlignedAllocSetCustomAligned(AlignedAllocFunc* allocFunc, AlignedFreeFunc* freeFunc);

///Класс AlignedAllocator = это портируемый класс для размещения алинованной памяти.
///Дефолтную реализацию для алинованных и неалинованных размещений можно переписать
///кастомным разместителем, используя AlignedAllocSetCustom и AlignedAllocSetCustomAligned.
template <typename T, unsigned Alignment>
class AlignedAllocator
{
	typedef AlignedAllocator<T, Alignment> self_type;

public:
	//just going down a list:
	AlignedAllocator() {}
	/*
	AlignedAllocator( const self_type & ) {}
	*/

	template <typename Other>
	AlignedAllocator(const AlignedAllocator<Other, Alignment>&)
	{
	}

	typedef const T* const_pointer;
	typedef const T& const_reference;
	typedef T* pointer;
	typedef T& reference;
	typedef T value_type;

	pointer address(reference ref) const { return &ref; }
	const_pointer address(const_reference ref) const { return &ref; }
	pointer allocate(size_type n, const_pointer* hint = 0)
	{
		(void)hint;
		return reinterpret_cast<pointer>(AlignedAlloc(sizeof(value_type) * n, Alignment));
	}
	void construct(pointer ptr, const value_type& value) { new (ptr) value_type(value); }
	void deallocate(pointer ptr)
	{
		AlignedFree(reinterpret_cast<uk>(ptr));
	}
	void destroy(pointer ptr) { ptr->~value_type(); }

	template <typename O>
	struct rebind
	{
		typedef AlignedAllocator<O, Alignment> other;
	};
	template <typename O>
	self_type& operator=(const AlignedAllocator<O, Alignment>&)
	{
		return *this;
	}

	friend bool operator==(const self_type&, const self_type&) { return true; }
};

#endif  //DRX3D_ALIGNED_ALLOCATOR
