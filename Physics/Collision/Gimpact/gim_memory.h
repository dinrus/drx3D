#ifndef GIM_MEMORY_H_INCLUDED
#define GIM_MEMORY_H_INCLUDED

#include <drx3D/Physics/Collision/Gimpact/gim_math.h>
#include <string.h>

#ifdef PREFETCH
#include <xmmintrin.h>  // for prefetch
#define pfval 64
#define pfval2 128
//! Prefetch 64
#define pf(_x, _i) _mm_prefetch((uk )(_x + _i + pfval), 0)
//! Prefetch 128
#define pf2(_x, _i) _mm_prefetch((uk )(_x + _i + pfval2), 0)
#else
//! Prefetch 64
#define pf(_x, _i)
//! Prefetch 128
#define pf2(_x, _i)
#endif

///Functions for manip packed arrays of numbers
#define GIM_COPY_ARRAYS(dest_array, source_array, element_count) \
	{                                                            \
		for (GUINT _i_ = 0; _i_ < element_count; ++_i_)          \
		{                                                        \
			dest_array[_i_] = source_array[_i_];                 \
		}                                                        \
	}

#define GIM_COPY_ARRAYS_1(dest_array, source_array, element_count, copy_macro) \
	{                                                                          \
		for (GUINT _i_ = 0; _i_ < element_count; ++_i_)                        \
		{                                                                      \
			copy_macro(dest_array[_i_], source_array[_i_]);                    \
		}                                                                      \
	}

#define GIM_ZERO_ARRAY(array, element_count)            \
	{                                                   \
		for (GUINT _i_ = 0; _i_ < element_count; ++_i_) \
		{                                               \
			array[_i_] = 0;                             \
		}                                               \
	}

#define GIM_CONSTANT_ARRAY(array, element_count, constant) \
	{                                                      \
		for (GUINT _i_ = 0; _i_ < element_count; ++_i_)    \
		{                                                  \
			array[_i_] = constant;                         \
		}                                                  \
	}

///Function prototypes to allocate and free memory.
typedef uk gim_alloc_function(size_t size);
typedef uk gim_alloca_function(size_t size);  //Allocs on the heap
typedef uk gim_realloc_function(uk ptr, size_t oldsize, size_t newsize);
typedef void gim_free_function(uk ptr);

///Memory Function Handlers
///set new memory management functions. if fn is 0, the default handlers are used.
void gim_set_alloc_handler(gim_alloc_function *fn);
void gim_set_alloca_handler(gim_alloca_function *fn);
void gim_set_realloc_handler(gim_realloc_function *fn);
void gim_set_free_handler(gim_free_function *fn);

///get current memory management functions.
gim_alloc_function *gim_get_alloc_handler(void);
gim_alloca_function *gim_get_alloca_handler(void);
gim_realloc_function *gim_get_realloc_handler(void);
gim_free_function *gim_get_free_handler(void);

///Standar Memory functions
uk gim_alloc(size_t size);
uk gim_alloca(size_t size);
uk gim_realloc(uk ptr, size_t oldsize, size_t newsize);
void gim_free(uk ptr);

#if defined(_WIN32) && !defined(__MINGW32__) && !defined(__CYGWIN__)
#define GIM_SIMD_MEMORY 1
#endif

//! SIMD POINTER INTEGER
#define SIMD_T GUINT64
//! SIMD INTEGER SIZE
#define SIMD_T_SIZE sizeof(SIMD_T)

inline void gim_simd_memcpy(uk dst, ukk src, size_t copysize)
{
#ifdef GIM_SIMD_MEMORY
	/*
//'z64' is incompatible with visual studio 6...
    //copy words
    SIMD_T * ui_src_ptr = (SIMD_T *)src;
    SIMD_T * ui_dst_ptr = (SIMD_T *)dst;
    while(copysize>=SIMD_T_SIZE)
    {
        *(ui_dst_ptr++) = *(ui_src_ptr++);
        copysize-=SIMD_T_SIZE;
    }
    if(copysize==0) return;
*/

	char *c_src_ptr = (char *)src;
	char *c_dst_ptr = (char *)dst;
	while (copysize > 0)
	{
		*(c_dst_ptr++) = *(c_src_ptr++);
		copysize--;
	}
	return;
#else
	memcpy(dst, src, copysize);
#endif
}

template <class T>
inline void gim_swap_elements(T *_array, size_t _i, size_t _j)
{
	T _e_tmp_ = _array[_i];
	_array[_i] = _array[_j];
	_array[_j] = _e_tmp_;
}

template <class T>
inline void gim_swap_elements_memcpy(T *_array, size_t _i, size_t _j)
{
	char _e_tmp_[sizeof(T)];
	gim_simd_memcpy(_e_tmp_, &_array[_i], sizeof(T));
	gim_simd_memcpy(&_array[_i], &_array[_j], sizeof(T));
	gim_simd_memcpy(&_array[_j], _e_tmp_, sizeof(T));
}

template <i32 SIZE>
inline void gim_swap_elements_ptr(char *_array, size_t _i, size_t _j)
{
	char _e_tmp_[SIZE];
	_i *= SIZE;
	_j *= SIZE;
	gim_simd_memcpy(_e_tmp_, _array + _i, SIZE);
	gim_simd_memcpy(_array + _i, _array + _j, SIZE);
	gim_simd_memcpy(_array + _j, _e_tmp_, SIZE);
}

#endif  // GIM_MEMORY_H_INCLUDED
