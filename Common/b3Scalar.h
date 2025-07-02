#ifndef D3_SCALAR_H
#define D3_SCALAR_H

#include <drxtypes.h>

#ifdef D3_MANAGED_CODE
//Aligned data types not supported in managed code
#pragma unmanaged
#endif

#include <math.h>
#include <stdlib.h>  //size_t for MSVC 6.0
#include <float.h>

//Original repository is at http://github.com/erwincoumans/bullet3
#define D3_DRX3D_VERSION 300

inline i32 b3GetVersion()
{
	return D3_DRX3D_VERSION;
}

#if defined(DEBUG) || defined(_DEBUG)
#define D3_DEBUG
#endif

#include <drx3D/Common/b3Logging.h>  //for drx3DError

#ifdef _WIN32

#if  defined(__GNUC__)	// it should handle both MINGW and CYGWIN
#define D3_FORCE_INLINE             __inline__ __attribute__((always_inline))
#define D3_ATTRIBUTE_ALIGNED16(a)   a __attribute__((aligned(16)))
#define D3_ATTRIBUTE_ALIGNED64(a)   a __attribute__((aligned(64)))
#define D3_ATTRIBUTE_ALIGNED128(a)  a __attribute__((aligned(128)))
#elif ( defined(_MSC_VER) && _MSC_VER < 1300 )
#define D3_FORCE_INLINE inline
#define D3_ATTRIBUTE_ALIGNED16(a) a
#define D3_ATTRIBUTE_ALIGNED64(a) a
#define D3_ATTRIBUTE_ALIGNED128(a) a
#else
//#define D3_HAS_ALIGNED_ALLOCATOR
#pragma warning(disable : 4324)  // disable padding warning
//			#pragma warning(disable:4530) // Disable the exception disable but used in MSCV Stl warning.
#pragma warning(disable : 4996)  //Turn off warnings about deprecated C routines
//			#pragma warning(disable:4786) // Disable the "debug name too long" warning

#define D3_FORCE_INLINE __forceinline
#define D3_ATTRIBUTE_ALIGNED16(a) __declspec(align(16)) a
#define D3_ATTRIBUTE_ALIGNED64(a) __declspec(align(64)) a
#define D3_ATTRIBUTE_ALIGNED128(a) __declspec(align(128)) a
#ifdef _XBOX
#define D3_USE_VMX128

#include <ppcintrinsics.h>
#define D3_HAVE_NATIVE_FSEL
#define b3Fsel(a, b, c) __fsel((a), (b), (c))
#else

#if (defined(_WIN32) && (_MSC_VER) && _MSC_VER >= 1400) && (!defined(D3_USE_DOUBLE_PRECISION))
#if (defined(_M_IX86) || defined(_M_X64))


#ifdef __clang__
//#define D3_NO_SIMD_OPERATOR_OVERLOADS
#define D3_DISABLE_SSE
#endif //__clang__

#ifndef D3_DISABLE_SSE
#define D3_USE_SSE
#endif //D3_DISABLE_SSE

#ifdef D3_USE_SSE
//D3_USE_SSE_IN_API is disabled under Windows by default, because
//it makes it harder to integrate drx3D into your application under Windows
//(structured embedding drx3D structs/classes need to be 16-byte aligned)
//with relatively little performance gain
//If you are not embedded drx3D data in your classes, or make sure that you align those classes on 16-byte boundaries
//you can manually enable this line or set it in the build system for a bit of performance gain (a few percent, dependent on usage)
//#define D3_USE_SSE_IN_API
#endif  //D3_USE_SSE
#include <emmintrin.h>
#endif
#endif

#endif  //_XBOX

#endif  //__MINGW32__

#ifdef D3_DEBUG
#ifdef _MSC_VER
#include <stdio.h>
#define drx3DAssert(x) { if(!(x)){drx3DError("Assert " __FILE__ ":%u (%s)\n", __LINE__, #x);__debugbreak();	}}
#else  //_MSC_VER
#include <assert.h>
#define drx3DAssert assert
#endif  //_MSC_VER
#else
#define drx3DAssert(x)
#endif
//b3FullAssert is optional, slows down a lot
#define b3FullAssert(x)

#define b3Likely(_c) _c
#define b3Unlikely(_c) _c

#else

#if defined(__CELLOS_LV2__)
#define D3_FORCE_INLINE inline __attribute__((always_inline))
#define D3_ATTRIBUTE_ALIGNED16(a) a __attribute__((aligned(16)))
#define D3_ATTRIBUTE_ALIGNED64(a) a __attribute__((aligned(64)))
#define D3_ATTRIBUTE_ALIGNED128(a) a __attribute__((aligned(128)))
#ifndef assert
#include <assert.h>
#endif
#ifdef D3_DEBUG
#ifdef __SPU__
#include <spu_printf.h>
#define printf spu_printf
#define drx3DAssert(x)               \
	{                             \
		if (!(x))                 \
		{                         \
			drx3DError(              \
				"Assert "__FILE__ \
				":%u (" #x ")\n", \
				__LINE__);        \
			spu_hcmpeq(0, 0);     \
		}                         \
	}
#else
#define drx3DAssert assert
#endif

#else
#define drx3DAssert(x)
#endif
//b3FullAssert is optional, slows down a lot
#define b3FullAssert(x)

#define b3Likely(_c) _c
#define b3Unlikely(_c) _c

#else

#ifdef USE_LIBSPE2

#define D3_FORCE_INLINE __inline
#define D3_ATTRIBUTE_ALIGNED16(a) a __attribute__((aligned(16)))
#define D3_ATTRIBUTE_ALIGNED64(a) a __attribute__((aligned(64)))
#define D3_ATTRIBUTE_ALIGNED128(a) a __attribute__((aligned(128)))
#ifndef assert
#include <assert.h>
#endif
#ifdef D3_DEBUG
#define drx3DAssert assert
#else
#define drx3DAssert(x)
#endif
//b3FullAssert is optional, slows down a lot
#define b3FullAssert(x)

#define b3Likely(_c) __builtin_expect((_c), 1)
#define b3Unlikely(_c) __builtin_expect((_c), 0)

#else
//non-windows systems

#if (defined(__APPLE__) && (!defined(D3_USE_DOUBLE_PRECISION)))
#if defined(__i386__) || defined(__x86_64__)
#define D3_USE_SSE
//D3_USE_SSE_IN_API is enabled on Mac OSX by default, because memory is automatically aligned on 16-byte boundaries
//if apps run into issues, we will disable the next line
#define D3_USE_SSE_IN_API
#ifdef D3_USE_SSE
// include appropriate SSE level
#if defined(__SSE4_1__)
#include <smmintrin.h>
#elif defined(__SSSE3__)
#include <tmmintrin.h>
#elif defined(__SSE3__)
#include <pmmintrin.h>
#else
#include <emmintrin.h>
#endif
#endif  //D3_USE_SSE
#elif defined(__armv7__)
#ifdef __clang__
#define D3_USE_NEON 1

#if defined D3_USE_NEON && defined(__clang__)
#include <arm_neon.h>
#endif  //D3_USE_NEON
#endif  //__clang__
#endif  //__arm__

#define D3_FORCE_INLINE inline __attribute__((always_inline))
///@todo: check out alignment methods for other platforms/compilers
#define D3_ATTRIBUTE_ALIGNED16(a) a __attribute__((aligned(16)))
#define D3_ATTRIBUTE_ALIGNED64(a) a __attribute__((aligned(64)))
#define D3_ATTRIBUTE_ALIGNED128(a) a __attribute__((aligned(128)))
#ifndef assert
#include <assert.h>
#endif

#if defined(DEBUG) || defined(_DEBUG)
#if defined(__i386__) || defined(__x86_64__)
#include <stdio.h>
#define drx3DAssert(x)                                                             \
	{                                                                           \
		if (!(x))                                                               \
		{                                                                       \
			drx3DError("Assert %s in line %d, file %s\n", #x, __LINE__, __FILE__); \
			asm ("int3");                                               \
		}                                                                       \
	}
#else  //defined (__i386__) || defined (__x86_64__)
#define drx3DAssert assert
#endif  //defined (__i386__) || defined (__x86_64__)
#else   //defined(DEBUG) || defined (_DEBUG)
#define drx3DAssert(x)
#endif  //defined(DEBUG) || defined (_DEBUG)

//b3FullAssert is optional, slows down a lot
#define b3FullAssert(x)
#define b3Likely(_c) _c
#define b3Unlikely(_c) _c

#else

#define D3_FORCE_INLINE inline
///@todo: check out alignment methods for other platforms/compilers
#define D3_ATTRIBUTE_ALIGNED16(a) a __attribute__((aligned(16)))
#define D3_ATTRIBUTE_ALIGNED64(a) a __attribute__((aligned(64)))
#define D3_ATTRIBUTE_ALIGNED128(a) a __attribute__((aligned(128)))
///#define D3_ATTRIBUTE_ALIGNED16(a) a
///#define D3_ATTRIBUTE_ALIGNED64(a) a
///#define D3_ATTRIBUTE_ALIGNED128(a) a
#ifndef assert
#include <assert.h>
#endif

#if defined(DEBUG) || defined(_DEBUG)
#define drx3DAssert assert
#else
#define drx3DAssert(x)
#endif

//b3FullAssert is optional, slows down a lot
#define b3FullAssert(x)
#define b3Likely(_c) _c
#define b3Unlikely(_c) _c
#endif  //__APPLE__

#endif  // LIBSPE2

#endif  //__CELLOS_LV2__
#endif

///The b3Scalar type abstracts floating point numbers, to easily switch between double and single floating point precision.
#if defined(D3_USE_DOUBLE_PRECISION)
typedef double b3Scalar;
//this number could be bigger in double precision
#define D3_LARGE_FLOAT 1e30
#else
typedef float b3Scalar;
//keep D3_LARGE_FLOAT*D3_LARGE_FLOAT < FLT_MAX
#define D3_LARGE_FLOAT 1e18f
#endif

#ifdef D3_USE_SSE
typedef __m128 b3SimdFloat4;
#endif  //D3_USE_SSE

#if defined D3_USE_SSE_IN_API && defined(D3_USE_SSE)
#ifdef _WIN32

#ifndef D3_NAN
static i32 b3NanMask = 0x7F800001;
#define D3_NAN (*(float *)&b3NanMask)
#endif

#ifndef D3_INFINITY_MASK
static i32 b3InfinityMask = 0x7F800000;
#define D3_INFINITY_MASK (*(float *)&b3InfinityMask)
#endif
#ifndef D3_NO_SIMD_OPERATOR_OVERLOADS
inline __m128 operator+(const __m128 A, const __m128 B)
{
	return _mm_add_ps(A, B);
}

inline __m128 operator-(const __m128 A, const __m128 B)
{
	return _mm_sub_ps(A, B);
}

inline __m128 operator*(const __m128 A, const __m128 B)
{
	return _mm_mul_ps(A, B);
}
#endif //D3_NO_SIMD_OPERATOR_OVERLOADS
#define b3CastfTo128i(a) (_mm_castps_si128(a))
#define b3CastfTo128d(a) (_mm_castps_pd(a))
#define b3CastiTo128f(a) (_mm_castsi128_ps(a))
#define b3CastdTo128f(a) (_mm_castpd_ps(a))
#define b3CastdTo128i(a) (_mm_castpd_si128(a))
#define b3Assign128(r0, r1, r2, r3) _mm_setr_ps(r0, r1, r2, r3)

#else  //_WIN32

#define b3CastfTo128i(a) ((__m128i)(a))
#define b3CastfTo128d(a) ((__m128d)(a))
#define b3CastiTo128f(a) ((__m128)(a))
#define b3CastdTo128f(a) ((__m128)(a))
#define b3CastdTo128i(a) ((__m128i)(a))
#define b3Assign128(r0, r1, r2, r3) \
	(__m128) { r0, r1, r2, r3 }
#endif  //_WIN32
#endif  //D3_USE_SSE_IN_API

#ifdef D3_USE_NEON
#include <arm_neon.h>

typedef float32x4_t b3SimdFloat4;
#define D3_INFINITY INFINITY
#define D3_NAN NAN
#define b3Assign128(r0, r1, r2, r3) \
	(float32x4_t) { r0, r1, r2, r3 }
#endif

#define D3_DECLARE_ALIGNED_ALLOCATOR()                                                                   \
	D3_FORCE_INLINE uk operator new(size_t sizeInBytes) { return b3AlignedAlloc(sizeInBytes, 16); }   \
	D3_FORCE_INLINE void operator delete(uk ptr) { b3AlignedFree(ptr); }                              \
	D3_FORCE_INLINE uk operator new(size_t, uk ptr) { return ptr; }                                \
	D3_FORCE_INLINE void operator delete(uk , uk ) {}                                              \
	D3_FORCE_INLINE uk operator new[](size_t sizeInBytes) { return b3AlignedAlloc(sizeInBytes, 16); } \
	D3_FORCE_INLINE void operator delete[](uk ptr) { b3AlignedFree(ptr); }                            \
	D3_FORCE_INLINE uk operator new[](size_t, uk ptr) { return ptr; }                              \
	D3_FORCE_INLINE void operator delete[](uk , uk ) {}

#if defined(D3_USE_DOUBLE_PRECISION) || defined(D3_FORCE_DOUBLE_FUNCTIONS)

D3_FORCE_INLINE b3Scalar b3Sqrt(b3Scalar x)
{
	return sqrt(x);
}
D3_FORCE_INLINE b3Scalar b3Fabs(b3Scalar x) { return fabs(x); }
D3_FORCE_INLINE b3Scalar b3Cos(b3Scalar x) { return cos(x); }
D3_FORCE_INLINE b3Scalar b3Sin(b3Scalar x) { return sin(x); }
D3_FORCE_INLINE b3Scalar b3Tan(b3Scalar x) { return tan(x); }
D3_FORCE_INLINE b3Scalar b3Acos(b3Scalar x)
{
	if (x < b3Scalar(-1)) x = b3Scalar(-1);
	if (x > b3Scalar(1)) x = b3Scalar(1);
	return acos(x);
}
D3_FORCE_INLINE b3Scalar b3Asin(b3Scalar x)
{
	if (x < b3Scalar(-1)) x = b3Scalar(-1);
	if (x > b3Scalar(1)) x = b3Scalar(1);
	return asin(x);
}
D3_FORCE_INLINE b3Scalar b3Atan(b3Scalar x) { return atan(x); }
D3_FORCE_INLINE b3Scalar b3Atan2(b3Scalar x, b3Scalar y) { return atan2(x, y); }
D3_FORCE_INLINE b3Scalar b3Exp(b3Scalar x) { return exp(x); }
D3_FORCE_INLINE b3Scalar b3Log(b3Scalar x) { return log(x); }
D3_FORCE_INLINE b3Scalar b3Pow(b3Scalar x, b3Scalar y) { return pow(x, y); }
D3_FORCE_INLINE b3Scalar b3Fmod(b3Scalar x, b3Scalar y) { return fmod(x, y); }

#else

D3_FORCE_INLINE b3Scalar b3Sqrt(b3Scalar y)
{
#ifdef USE_APPROXIMATION
	double x, z, tempf;
	u64 *tfptr = ((u64 *)&tempf) + 1;

	tempf = y;
	*tfptr = (0xbfcdd90a - *tfptr) >> 1; /* estimate of 1/sqrt(y) */
	x = tempf;
	z = y * b3Scalar(0.5);
	x = (b3Scalar(1.5) * x) - (x * x) * (x * z); /* iteration formula     */
	x = (b3Scalar(1.5) * x) - (x * x) * (x * z);
	x = (b3Scalar(1.5) * x) - (x * x) * (x * z);
	x = (b3Scalar(1.5) * x) - (x * x) * (x * z);
	x = (b3Scalar(1.5) * x) - (x * x) * (x * z);
	return x * y;
#else
	return sqrtf(y);
#endif
}
D3_FORCE_INLINE b3Scalar b3Fabs(b3Scalar x) { return fabsf(x); }
D3_FORCE_INLINE b3Scalar b3Cos(b3Scalar x) { return cosf(x); }
D3_FORCE_INLINE b3Scalar b3Sin(b3Scalar x) { return sinf(x); }
D3_FORCE_INLINE b3Scalar b3Tan(b3Scalar x) { return tanf(x); }
D3_FORCE_INLINE b3Scalar b3Acos(b3Scalar x)
{
	if (x < b3Scalar(-1))
		x = b3Scalar(-1);
	if (x > b3Scalar(1))
		x = b3Scalar(1);
	return acosf(x);
}
D3_FORCE_INLINE b3Scalar b3Asin(b3Scalar x)
{
	if (x < b3Scalar(-1))
		x = b3Scalar(-1);
	if (x > b3Scalar(1))
		x = b3Scalar(1);
	return asinf(x);
}
D3_FORCE_INLINE b3Scalar b3Atan(b3Scalar x) { return atanf(x); }
D3_FORCE_INLINE b3Scalar b3Atan2(b3Scalar x, b3Scalar y) { return atan2f(x, y); }
D3_FORCE_INLINE b3Scalar b3Exp(b3Scalar x) { return expf(x); }
D3_FORCE_INLINE b3Scalar b3Log(b3Scalar x) { return logf(x); }
D3_FORCE_INLINE b3Scalar b3Pow(b3Scalar x, b3Scalar y) { return powf(x, y); }
D3_FORCE_INLINE b3Scalar b3Fmod(b3Scalar x, b3Scalar y) { return fmodf(x, y); }

#endif

#define D3_2_PI b3Scalar(6.283185307179586232)
#define D3_PI (D3_2_PI * b3Scalar(0.5))
#define D3_HALF_PI (D3_2_PI * b3Scalar(0.25))
#define D3_RADS_PER_DEG (D3_2_PI / b3Scalar(360.0))
#define D3_DEGS_PER_RAD (b3Scalar(360.0) / D3_2_PI)
#define D3_SQRT12 b3Scalar(0.7071067811865475244008443621048490)

#define b3RecipSqrt(x) ((b3Scalar)(b3Scalar(1.0) / b3Sqrt(b3Scalar(x)))) /* reciprocal square root */

#ifdef D3_USE_DOUBLE_PRECISION
#define D3_EPSILON DBL_EPSILON
#define D3_INFINITY DBL_MAX
#else
#define D3_EPSILON FLT_EPSILON
#define D3_INFINITY FLT_MAX
#endif

D3_FORCE_INLINE b3Scalar b3Atan2Fast(b3Scalar y, b3Scalar x)
{
	b3Scalar coeff_1 = D3_PI / 4.0f;
	b3Scalar coeff_2 = 3.0f * coeff_1;
	b3Scalar abs_y = b3Fabs(y);
	b3Scalar angle;
	if (x >= 0.0f)
	{
		b3Scalar r = (x - abs_y) / (x + abs_y);
		angle = coeff_1 - coeff_1 * r;
	}
	else
	{
		b3Scalar r = (x + abs_y) / (abs_y - x);
		angle = coeff_2 - coeff_1 * r;
	}
	return (y < 0.0f) ? -angle : angle;
}

D3_FORCE_INLINE bool b3FuzzyZero(b3Scalar x) { return b3Fabs(x) < D3_EPSILON; }

D3_FORCE_INLINE bool b3Equal(b3Scalar a, b3Scalar eps)
{
	return (((a) <= eps) && !((a) < -eps));
}
D3_FORCE_INLINE bool b3GreaterEqual(b3Scalar a, b3Scalar eps)
{
	return (!((a) <= eps));
}

D3_FORCE_INLINE i32 b3IsNegative(b3Scalar x)
{
	return x < b3Scalar(0.0) ? 1 : 0;
}

D3_FORCE_INLINE b3Scalar b3Radians(b3Scalar x) { return x * D3_RADS_PER_DEG; }
D3_FORCE_INLINE b3Scalar b3Degrees(b3Scalar x) { return x * D3_DEGS_PER_RAD; }

#define D3_DECLARE_HANDLE(name) \
	typedef struct name##__     \
	{                           \
		i32 unused;             \
	} * name

#ifndef b3Fsel
D3_FORCE_INLINE b3Scalar b3Fsel(b3Scalar a, b3Scalar b, b3Scalar c)
{
	return a >= 0 ? b : c;
}
#endif
#define b3Fsels(a, b, c) (b3Scalar) b3Fsel(a, b, c)

D3_FORCE_INLINE bool b3MachineIsLittleEndian()
{
	i32 i = 1;
	tukk p = (tukk)&i;
	if (p[0] == 1)  // Lowest address contains the least significant byte
		return true;
	else
		return false;
}

///b3Select avoids branches, which makes performance much better for consoles like Playstation 3 and XBox 360
///Thanks Phil Knight. See also http://www.cellperformance.com/articles/2006/04/more_techniques_for_eliminatin_1.html
D3_FORCE_INLINE unsigned b3Select(unsigned condition, unsigned valueIfConditionNonZero, unsigned valueIfConditionZero)
{
	// Set testNz to 0xFFFFFFFF if condition is nonzero, 0x00000000 if condition is zero
	// Rely on positive value or'ed with its negative having sign bit on
	// and zero value or'ed with its negative (which is still zero) having sign bit off
	// Use arithmetic shift right, shifting the sign bit through all 32 bits
	unsigned testNz = (unsigned)(((i32)condition | -(i32)condition) >> 31);
	unsigned testEqz = ~testNz;
	return ((valueIfConditionNonZero & testNz) | (valueIfConditionZero & testEqz));
}
D3_FORCE_INLINE i32 b3Select(unsigned condition, i32 valueIfConditionNonZero, i32 valueIfConditionZero)
{
	unsigned testNz = (unsigned)(((i32)condition | -(i32)condition) >> 31);
	unsigned testEqz = ~testNz;
	return static_cast<i32>((valueIfConditionNonZero & testNz) | (valueIfConditionZero & testEqz));
}
D3_FORCE_INLINE float b3Select(unsigned condition, float valueIfConditionNonZero, float valueIfConditionZero)
{
#ifdef D3_HAVE_NATIVE_FSEL
	return (float)b3Fsel((b3Scalar)condition - b3Scalar(1.0f), valueIfConditionNonZero, valueIfConditionZero);
#else
	return (condition != 0) ? valueIfConditionNonZero : valueIfConditionZero;
#endif
}

template <typename T>
D3_FORCE_INLINE void b3Swap(T &a, T &b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

//PCK: endian swapping functions
D3_FORCE_INLINE unsigned b3SwapEndian(unsigned val)
{
	return (((val & 0xff000000) >> 24) | ((val & 0x00ff0000) >> 8) | ((val & 0x0000ff00) << 8) | ((val & 0x000000ff) << 24));
}

D3_FORCE_INLINE unsigned short b3SwapEndian(unsigned short val)
{
	return static_cast<unsigned short>(((val & 0xff00) >> 8) | ((val & 0x00ff) << 8));
}

D3_FORCE_INLINE unsigned b3SwapEndian(i32 val)
{
	return b3SwapEndian((unsigned)val);
}

D3_FORCE_INLINE unsigned short b3SwapEndian(short val)
{
	return b3SwapEndian((unsigned short)val);
}

///b3SwapFloat uses using char pointers to swap the endianness
////b3SwapFloat/b3SwapDouble will NOT return a float, because the machine might 'correct' invalid floating point values
///Not all values of sign/exponent/mantissa are valid floating point numbers according to IEEE 754.
///When a floating point unit is faced with an invalid value, it may actually change the value, or worse, throw an exception.
///In most systems, running user mode code, you wouldn't get an exception, but instead the hardware/os/runtime will 'fix' the number for you.
///so instead of returning a float/double, we return integer/long long integer
D3_FORCE_INLINE u32 b3SwapEndianFloat(float d)
{
	u32 a = 0;
	u8 *dst = (u8*)&a;
	u8 *src = (u8*)&d;

	dst[0] = src[3];
	dst[1] = src[2];
	dst[2] = src[1];
	dst[3] = src[0];
	return a;
}

// unswap using char pointers
D3_FORCE_INLINE float b3UnswapEndianFloat(u32 a)
{
	float d = 0.0f;
	u8 *src = (u8*)&a;
	u8 *dst = (u8*)&d;

	dst[0] = src[3];
	dst[1] = src[2];
	dst[2] = src[1];
	dst[3] = src[0];

	return d;
}

// swap using char pointers
D3_FORCE_INLINE void b3SwapEndianDouble(double d, u8 *dst)
{
	u8 *src = (u8*)&d;

	dst[0] = src[7];
	dst[1] = src[6];
	dst[2] = src[5];
	dst[3] = src[4];
	dst[4] = src[3];
	dst[5] = src[2];
	dst[6] = src[1];
	dst[7] = src[0];
}

// unswap using char pointers
D3_FORCE_INLINE double b3UnswapEndianDouble(u8k *src)
{
	double d = 0.0;
	u8 *dst = (u8*)&d;

	dst[0] = src[7];
	dst[1] = src[6];
	dst[2] = src[5];
	dst[3] = src[4];
	dst[4] = src[3];
	dst[5] = src[2];
	dst[6] = src[1];
	dst[7] = src[0];

	return d;
}

// returns normalized value in range [-D3_PI, D3_PI]
D3_FORCE_INLINE b3Scalar b3NormalizeAngle(b3Scalar angleInRadians)
{
	angleInRadians = b3Fmod(angleInRadians, D3_2_PI);
	if (angleInRadians < -D3_PI)
	{
		return angleInRadians + D3_2_PI;
	}
	else if (angleInRadians > D3_PI)
	{
		return angleInRadians - D3_2_PI;
	}
	else
	{
		return angleInRadians;
	}
}

///rudimentary class to provide type info
struct b3TypedObject
{
	b3TypedObject(i32 objectType)
		: m_objectType(objectType)
	{
	}
	i32 m_objectType;
	inline i32 getObjectType() const
	{
		return m_objectType;
	}
};

///align a pointer to the provided alignment, upwards
template <typename T>
T *b3AlignPointer(T *unalignedPtr, size_t alignment)
{
	struct b3ConvertPointerSizeT
	{
		union {
			T *ptr;
			size_t integer;
		};
	};
	b3ConvertPointerSizeT converter;

	const size_t bit_mask = ~(alignment - 1);
	converter.ptr = unalignedPtr;
	converter.integer += alignment - 1;
	converter.integer &= bit_mask;
	return converter.ptr;
}

#endif  //D3_SCALAR_H
