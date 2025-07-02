///@file Configuration for Inverse Dynamics Library without external dependencies
#ifndef INVDYNCONFIG_BUILTIN_H_
#define INVDYNCONFIG_BUILTIN_H_
#define drx3d_inverse drx3d_inverseBuiltin
#ifdef DRX3D_USE_DOUBLE_PRECISION
// choose double/single precision version
typedef double idScalar;
#else
typedef float idScalar;
#endif
// use std::vector for arrays
#include <vector>
// this is to make it work with C++2003, otherwise we could do this
// template <typename T>
// using idArray = std::vector<T>;
template <typename T>
struct idArray
{
	typedef std::vector<T> type;
};
typedef std::vector<i32>::size_type idArrayIdx;
// default to standard malloc/free
#include <cstdlib>
#define idMalloc ::malloc
#define idFree ::free
// currently not aligned at all...
#define ID_DECLARE_ALIGNED_ALLOCATOR()                                                     \
	inline uk operator new(std::size_t sizeInBytes) { return idMalloc(sizeInBytes); }   \
	inline void operator delete(uk ptr) { idFree(ptr); }                                \
	inline uk operator new(std::size_t, uk ptr) { return ptr; }                      \
	inline void operator delete(uk , uk ) {}                                           \
	inline uk operator new[](std::size_t sizeInBytes) { return idMalloc(sizeInBytes); } \
	inline void operator delete[](uk ptr) { idFree(ptr); }                              \
	inline uk operator new[](std::size_t, uk ptr) { return ptr; }                    \
	inline void operator delete[](uk , uk ) {}

#include <drx3D/Physics/Dynamics/Inverse/details/IDMatVec.h>
#endif
