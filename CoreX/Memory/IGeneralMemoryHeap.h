// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef IGENERALMEMORYHEAP_H
#define IGENERALMEMORYHEAP_H

class IGeneralMemoryHeap
{
public:
	// <interfuscator:shuffle>
	virtual bool  Cleanup() = 0;

	virtual i32   AddRef() = 0;
	virtual i32   Release() = 0;

	virtual bool  IsInAddressRange(uk ptr) const = 0;

	virtual uk Calloc(size_t nmemb, size_t size, tukk sUsage) = 0;
	virtual uk Malloc(size_t sz, tukk sUsage) = 0;

	//! Пытается освободить аллокацию. При успехе возвращает размер аллокации;
	// 0 возвращает, если адрес не принадлежит этой куче.
	virtual size_t Free(uk ptr) = 0;
	virtual uk  Realloc(uk ptr, size_t sz, tukk sUsage) = 0;
	virtual uk  ReallocAlign(uk ptr, size_t size, size_t alignment, tukk sUsage) = 0;
	virtual uk  Memalign(size_t boundary, size_t size, tukk sUsage) = 0;

	//! Получить размер аллокации. Возвращает 0, если ptr не принадлежит этой куче.
	virtual size_t UsableSize(uk ptr) const = 0;
	// </interfuscator:shuffle>
protected:
	virtual ~IGeneralMemoryHeap() {}
};

#endif
