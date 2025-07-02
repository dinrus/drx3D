#include <drx3D/Maths/Linear/AlignedAllocator.h>
#include <string.h>

#ifdef DRX3D_DEBUG_MEMORY_ALLOCATIONS
i32 gNumAlignedAllocs = 0;
i32 gNumAlignedFree = 0;
i32 gTotalBytesAlignedAllocs = 0;  //обнаружение утечек памяти
#endif                             //DRX3D_DEBUG_MEMORY_ALLOCATIONST_DEBUG_ALLOCATIONS

//Дефолтный статический разместитель.
static uk AllocDefault(size_t size)
{
  tuk data = (tuk) malloc(size);
  memset(data,0,size);//keep msan happy
  return data;
}
//Дефолтный статический выместитель.
static void FreeDefault(uk ptr)
{
	free(ptr);
}
//Указатели на дефолтный выместитель и разместитель.
static AllocFunc *sAllocFunc = AllocDefault;
static FreeFunc *sFreeFunc = FreeDefault;

#if defined(DRX3D_HAS_ALIGNED_ALLOCATOR)
#include <malloc.h>
static uk AlignedAllocDefault(size_t size, i32 alignment)
{
	return _aligned_malloc(size, (size_t)alignment);
}

static void AlignedFreeDefault(uk ptr)
{
	_aligned_free(ptr);
}
#elif defined(__CELLOS_LV2__)
#include <stdlib.h>

static inline uk AlignedAllocDefault(size_t size, i32 alignment)
{
	return memalign(alignment, size);
}

static inline void AlignedFreeDefault(uk ptr)
{
	free(ptr);
}
#else

static inline uk AlignedAllocDefault(size_t size, i32 alignment)
{
	uk ret;
	char *real;
	real = (char *)sAllocFunc(size + sizeof(uk ) + (alignment - 1));
	if (real)
	{
		ret = AlignPointer(real + sizeof(uk ), alignment);
		*((uk *)(ret)-1) = (uk )(real);
	}
	else
	{
		ret = (uk )(real);
	}
  //keep msan happy
  memset((tuk) ret, 0, size);
	return (ret);
}

static inline void AlignedFreeDefault(uk ptr)
{
	uk real;

	if (ptr)
	{
		real = *((uk *)(ptr)-1);
		sFreeFunc(real);
	}
}
#endif

static AlignedAllocFunc *sAlignedAllocFunc = AlignedAllocDefault;
static AlignedFreeFunc *sAlignedFreeFunc = AlignedFreeDefault;

void AlignedAllocSetCustomAligned(AlignedAllocFunc *allocFunc, AlignedFreeFunc *freeFunc)
{
	sAlignedAllocFunc = allocFunc ? allocFunc : AlignedAllocDefault;
	sAlignedFreeFunc = freeFunc ? freeFunc : AlignedFreeDefault;
}

void AlignedAllocSetCustom(AllocFunc *allocFunc, FreeFunc *freeFunc)
{
	sAllocFunc = allocFunc ? allocFunc : AllocDefault;
	sFreeFunc = freeFunc ? freeFunc : FreeDefault;
}

#ifdef DRX3D_DEBUG_MEMORY_ALLOCATIONS

static i32 allocations_id[10241024];
static i32 allocations_bytes[10241024];
static i32 mynumallocs = 0;
#include <stdio.h>

i32 DumpMemoryLeaks()
{
	i32 totalLeak = 0;

	for (i32 i = 0; i < mynumallocs; i++)
	{
		printf("Ошибка: утечка памяти аллокации #%d (%d байтов)\n", allocations_id[i], allocations_bytes[i]);
		totalLeak += allocations_bytes[i];
	}
	if (totalLeak)
	{
		printf("Ошибка: утечки памяти: %d аллокаций е было освобождено and leaked together %d bytes\n", mynumallocs, totalLeak);
	}
	return totalLeak;
}
//Этот генерный разместитель предоставляет общее число размещённых байтов.
#include <stdio.h>

struct DebugPtrMagic
{
	union {
		uk *vptrptr;
		uk vptr;
		i32 *iptr;
		char *cptr;
	};
};

uk AlignedAllocInternal(size_t size, i32 alignment, i32 line, tukk filename)
{
	if (size == 0)
	{
		printf("Что-о-о? size==0");
		return 0;
	}
	static i32 allocId = 0;

	uk ret;
	char *real;

	// Чтобы найти отдельную утечку памяти, надо делать что-то типа:
	//	if (allocId==172)
	//	{
	//		printf("впоймай меня!\n");
	//	}
	//	if (size>1024*1024)
	//	{
	//		printf("большой размест!%d\n", size);
	//	}

	gTotalBytesAlignedAllocs += size;
	gNumAlignedAllocs++;

	i32 sz4prt = 4 * sizeof(uk );

	real = (char *)sAllocFunc(size + sz4prt + (alignment - 1));
	if (real)
	{
		ret = (uk )AlignPointer(real + sz4prt, alignment);
		DebugPtrMagic p;
		p.vptr = ret;
		p.cptr -= sizeof(uk );
		*p.vptrptr = (uk )real;
		p.cptr -= sizeof(uk );
		*p.iptr = size;
		p.cptr -= sizeof(uk );
		*p.iptr = allocId;

		allocations_id[mynumallocs] = allocId;
		allocations_bytes[mynumallocs] = size;
		mynumallocs++;
	}
	else
	{
		ret = (uk )(real);  //??
	}

	printf("размещение %d по адресу %x, из %s, строка %d, размер %d (всего размещено = %d)\n", allocId, real, filename, line, size, gTotalBytesAlignedAllocs);
	allocId++;

	i32 *ptr = (i32 *)ret;
	*ptr = 12;
	return (ret);
}

void AlignedFreeInternal(uk ptr, i32 line, tukk filename)
{
	uk real;

	if (ptr)
	{
		gNumAlignedFree++;

		DebugPtrMagic p;
		p.vptr = ptr;
		p.cptr -= sizeof(uk );
		real = *p.vptrptr;
		p.cptr -= sizeof(uk );
		i32 size = *p.iptr;
		p.cptr -= sizeof(uk );
		i32 allocId = *p.iptr;

		bool found = false;

		for (i32 i = 0; i < mynumallocs; i++)
		{
			if (allocations_id[i] == allocId)
			{
				allocations_id[i] = allocations_id[mynumallocs - 1];
				allocations_bytes[i] = allocations_bytes[mynumallocs - 1];
				mynumallocs--;
				found = true;
				break;
			}
		}

		gTotalBytesAlignedAllocs -= size;

		i32 diff = gNumAlignedAllocs - gNumAlignedFree;
		printf("вымещение %d по адресу %x, из %s, строка %d, размер %d (всего осталось = %d в %d неосвобождённых аллокациях)\n", allocId, real, filename, line, size, gTotalBytesAlignedAllocs, diff);

		sFreeFunc(real);
	}
	else
	{
		//printf("deleting a NULL ptr, no effect\n");
	}
}

#else  //DRX3D_DEBUG_MEMORY_ALLOCATIONS

uk AlignedAllocInternal(size_t size, i32 alignment)
{
	uk ptr;
	ptr = sAlignedAllocFunc(size, alignment);
	//	printf("AlignedAllocInternal %d, %x\n",size,ptr);
	return ptr;
}

void AlignedFreeInternal(uk ptr)
{
	if (!ptr)
	{
		return;
	}

	//	printf("AlignedFreeInternal %x\n",ptr);
	sAlignedFreeFunc(ptr);
}

#endif  //DRX3D_DEBUG_MEMORY_ALLOCATIONS
