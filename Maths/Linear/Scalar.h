#ifndef DRX3D_SCALAR_H
#define DRX3D_SCALAR_H

#ifdef DRX3D_MANAGED_CODE
//Aligned data types not supported in managed code
#pragma unmanaged
#endif
#include <drxtypes.h>
#include <math.h>
#include <stdlib.h>  //size_t for MSVC 6.0
#include <float.h>

/* SVN $Revision$ on $Date$ from http://bullet.googlecode.com*/
#define DRX3D_DRX3D_VERSION 325

inline i32 GetVersion()
{
    return DRX3D_DRX3D_VERSION;
}

inline i32 IsDoublePrecision()
{
  #ifdef DRX3D_USE_DOUBLE_PRECISION
  return true;
  #else
  return false;
  #endif
}


// The following macro "DRX3D_NOT_EMPTY_FILE" can be put into a file
// in order suppress the MS Visual C++ Linker warning 4221
//
// warning LNK4221: no public symbols found; archive member will be inaccessible
//
// This warning occurs on PC and XBOX when a file compiles out completely
// has no externally visible symbols which may be dependant on configuration
// #defines and options.
//
// see more https://stackoverflow.com/questions/1822887/what-is-the-best-way-to-eliminate-ms-visual-c-linker-warning-warning-lnk422

#if defined(_MSC_VER)
#define DRX3D_NOT_EMPTY_FILE_CAT_II(p, res) res
#define DRX3D_NOT_EMPTY_FILE_CAT_I(a, b) DRX3D_NOT_EMPTY_FILE_CAT_II(~, a##b)
#define DRX3D_NOT_EMPTY_FILE_CAT(a, b) DRX3D_NOT_EMPTY_FILE_CAT_I(a, b)
#define DRX3D_NOT_EMPTY_FILE                                      \
    namespace                                                  \
    {                                                          \
    char DRX3D_NOT_EMPTY_FILE_CAT(NoEmptyFileDummy, __COUNTER__); \
    }
#else
#define DRX3D_NOT_EMPTY_FILE
#endif

// clang and most formatting tools don't support indentation of preprocessor guards, so turn it off
// clang-format off
#if defined(DEBUG) || defined (_DEBUG)
    #define DRX3D_DEBUG
#endif

#ifdef _WIN32
    #if  defined(__GNUC__)  // it should handle both MINGW and CYGWIN
            #define SIMD_FORCE_INLINE        __inline__ __attribute__((always_inline))
            #define ATTRIBUTE_ALIGNED16(a)   a __attribute__((aligned(16)))
            #define ATTRIBUTE_ALIGNED64(a)   a __attribute__((aligned(64)))
            #define ATTRIBUTE_ALIGNED128(a)  a __attribute__((aligned(128)))
        #elif ( defined(_MSC_VER) && _MSC_VER < 1300 )
        #define SIMD_FORCE_INLINE inline
        #define ATTRIBUTE_ALIGNED16(a) a
        #define ATTRIBUTE_ALIGNED64(a) a
        #define ATTRIBUTE_ALIGNED128(a) a
    #elif defined(_M_ARM)
        #define SIMD_FORCE_INLINE __forceinline
        #define ATTRIBUTE_ALIGNED16(a) __declspec() a
        #define ATTRIBUTE_ALIGNED64(a) __declspec() a
        #define ATTRIBUTE_ALIGNED128(a) __declspec () a
    #else//__MINGW32__
        //#define DRX3D_HAS_ALIGNED_ALLOCATOR
        #pragma warning(disable : 4324) // disable padding warning
//          #pragma warning(disable:4530) // Disable the exception disable but used in MSCV Stl warning.
        #pragma warning(disable:4996) //Turn off warnings about deprecated C routines
//          #pragma warning(disable:4786) // Disable the "debug name too long" warning

        #define SIMD_FORCE_INLINE __forceinline
        #define ATTRIBUTE_ALIGNED16(a) __declspec(align(16)) a
        #define ATTRIBUTE_ALIGNED64(a) __declspec(align(64)) a
        #define ATTRIBUTE_ALIGNED128(a) __declspec (align(128)) a
        #ifdef _XBOX
            #define DRX3D_USE_VMX128

            #include <ppcintrinsics.h>
            #define DRX3D_HAVE_NATIVE_FSEL
            #define Fsel(a,b,c) __fsel((a),(b),(c))
        #else

#if defined (_M_ARM) || defined (_M_ARM64)
            //Do not turn SSE on for ARM (may want to turn on DRX3D_USE_NEON however)
#elif (defined (_WIN32) && (_MSC_VER) && _MSC_VER >= 1400) && (!defined (DRX3D_USE_DOUBLE_PRECISION))

#ifdef __clang__
#define __DRX3D_DISABLE_SSE__
#endif
#ifndef __DRX3D_DISABLE_SSE__
            #if _MSC_VER>1400
                #define DRX3D_USE_SIMD_VECTOR3
            #endif
            #define DRX3D_USE_SSE
#endif//__DRX3D_DISABLE_SSE__
            #ifdef DRX3D_USE_SSE

#if (_MSC_FULL_VER >= 170050727)//Visual Studio 2012 can compile SSE4/FMA3 (but SSE4/FMA3 is not enabled by default)
            #define DRX3D_ALLOW_SSE4
#endif //(_MSC_FULL_VER >= 160040219)

            //DRX3D_USE_SSE_IN_API is disabled under Windows by default, because
            //it makes it harder to integrate drx3D into your application under Windows
            //(structured embedding drx3D structs/classes need to be 16-byte aligned)
            //with relatively little performance gain
            //If you are not embedded drx3D data in your classes, or make sure that you align those classes on 16-byte boundaries
            //you can manually enable this line or set it in the build system for a bit of performance gain (a few percent, dependent on usage)
            //#define DRX3D_USE_SSE_IN_API
            #endif //DRX3D_USE_SSE
            #include <emmintrin.h>
#endif

        #endif//_XBOX

    #endif //__MINGW32__

    #ifdef DRX3D_DEBUG
        #ifdef _MSC_VER
            #include <stdio.h>
            #define Assert(x) { if(!(x)){printf("Assert " __FILE__ ":%u (%s)\n", __LINE__, #x);__debugbreak();  }}
        #else//_MSC_VER
            #include <assert.h>
            #define Assert assert
        #endif//_MSC_VER
    #else
        #define Assert(x)
    #endif
        /FullAssert is optional, slows down a lot
        #define FullAssert(x)

        #define btLikely(_c)  _c
        #define Unlikely(_c) _c

#else//_WIN32

    #if defined (__CELLOS_LV2__)
        #define SIMD_FORCE_INLINE inline __attribute__((always_inline))
        #define ATTRIBUTE_ALIGNED16(a) a __attribute__ ((aligned (16)))
        #define ATTRIBUTE_ALIGNED64(a) a __attribute__ ((aligned (64)))
        #define ATTRIBUTE_ALIGNED128(a) a __attribute__ ((aligned (128)))
        #ifndef assert
        #include <assert.h>
        #endif
        #ifdef DRX3D_DEBUG
            #ifdef __SPU__
                #include <spu_printf.h>
                #define printf spu_printf
                #define Assert(x) {if(!(x)){printf("Assert " __FILE__ ":%u ("#x")\n", __LINE__);spu_hcmpeq(0,0);}}
            #else
                #define Assert assert
            #endif

        #else//DRX3D_DEBUG
                #define Assert(x)
        #endif//DRX3D_DEBUG
        /FullAssert is optional, slows down a lot
        #define FullAssert(x)

        #define btLikely(_c)  _c
        #define Unlikely(_c) _c

    #else//defined  (__CELLOS_LV2__)

        #ifdef USE_LIBSPE2

            #define SIMD_FORCE_INLINE __inline
            #define ATTRIBUTE_ALIGNED16(a) a __attribute__ ((aligned (16)))
            #define ATTRIBUTE_ALIGNED64(a) a __attribute__ ((aligned (64)))
            #define ATTRIBUTE_ALIGNED128(a) a __attribute__ ((aligned (128)))
            #ifndef assert
            #include <assert.h>
            #endif
    #ifdef DRX3D_DEBUG
            #define Assert assert
    #else
            #define Assert(x)
    #endif
            /FullAssert is optional, slows down a lot
            #define FullAssert(x)


            #define btLikely(_c)   __builtin_expect((_c), 1)
            #define Unlikely(_c) __builtin_expect((_c), 0)


        #else//USE_LIBSPE2
    //non-windows systems

            #if (defined (__APPLE__) && (!defined (DRX3D_USE_DOUBLE_PRECISION)))
                #if defined (__i386__) || defined (__x86_64__)
                    #define DRX3D_USE_SIMD_VECTOR3
                    #define DRX3D_USE_SSE
                    //DRX3D_USE_SSE_IN_API is enabled on Mac OSX by default, because memory is automatically aligned on 16-byte boundaries
                    //if apps run into issues, we will disable the next line
                    #define DRX3D_USE_SSE_IN_API
                    #ifdef DRX3D_USE_SSE
                        // include appropriate SSE level
                        #if defined (__SSE4_1__)
                            #include <smmintrin.h>
                        #elif defined (__SSSE3__)
                            #include <tmmintrin.h>
                        #elif defined (__SSE3__)
                            #include <pmmintrin.h>
                        #else
                            #include <emmintrin.h>
                        #endif
                    #endif //DRX3D_USE_SSE
                #elif defined( __ARM_NEON__ )
                    #ifdef __clang__
                        #define DRX3D_USE_NEON 1
                        #define DRX3D_USE_SIMD_VECTOR3

                        #if defined DRX3D_USE_NEON && defined (__clang__)
                            #include <arm_neon.h>
                        #endif//DRX3D_USE_NEON
                   #endif //__clang__
                #endif//__arm__

                #define SIMD_FORCE_INLINE inline __attribute__ ((always_inline))
            ///@todo: check out alignment methods for other platforms/compilers
                #define ATTRIBUTE_ALIGNED16(a) a __attribute__ ((aligned (16)))
                #define ATTRIBUTE_ALIGNED64(a) a __attribute__ ((aligned (64)))
                #define ATTRIBUTE_ALIGNED128(a) a __attribute__ ((aligned (128)))
                #ifndef assert
                #include <assert.h>
                #endif

                #if defined(DEBUG) || defined (_DEBUG)
                 #if defined (__i386__) || defined (__x86_64__)
                #include <stdio.h>
                 #define Assert(x)\
                {\
                if(!(x))\
                {\
                    printf("Assert %s in line %d, file %s\n",#x, __LINE__, __FILE__);\
                    asm  ("int3");\
                }\
                }
                #else//defined (__i386__) || defined (__x86_64__)
                    #define Assert assert
                #endif//defined (__i386__) || defined (__x86_64__)
                #else//defined(DEBUG) || defined (_DEBUG)
                    #define Assert(x)
                #endif//defined(DEBUG) || defined (_DEBUG)

                //FullAssert is optional, slows down a lot
                #define FullAssert(x)
                #define btLikely(_c)  _c
                #define Unlikely(_c) _c

            #else//__APPLE__

                #define SIMD_FORCE_INLINE inline
                ///@todo: check out alignment methods for other platforms/compilers
                ///#define ATTRIBUTE_ALIGNED16(a) a __attribute__ ((aligned (16)))
                ///#define ATTRIBUTE_ALIGNED64(a) a __attribute__ ((aligned (64)))
                ///#define ATTRIBUTE_ALIGNED128(a) a __attribute__ ((aligned (128)))
                #define ATTRIBUTE_ALIGNED16(a) a
                #define ATTRIBUTE_ALIGNED64(a) a
                #define ATTRIBUTE_ALIGNED128(a) a
                #ifndef assert
                #include <assert.h>
                #endif

                #if defined(DEBUG) || defined (_DEBUG)
                    #define Assert assert
                #else
                    #define Assert(x)
                #endif

                //FullAssert is optional, slows down a lot
                #define FullAssert(x)
                #define btLikely(_c)  _c
                #define Unlikely(_c) _c
            #endif //__APPLE__
        #endif // LIBSPE2
    #endif  //__CELLOS_LV2__
#endif//_WIN32


///The Scalar type abstracts floating point numbers, to easily switch between double and single floating point precision.
#if defined(DRX3D_USE_DOUBLE_PRECISION)
    typedef double Scalar;
    //this number could be bigger in double precision
    #define DRX3D_LARGE_FLOAT 1e30
#else
    typedef float Scalar;
    //keep DRX3D_LARGE_FLOAT*DRX3D_LARGE_FLOAT < FLT_MAX
    #define DRX3D_LARGE_FLOAT 1e18f
#endif

#ifdef DRX3D_USE_SSE
    typedef __m128 SimdFloat4;
#endif  //DRX3D_USE_SSE

#if defined(DRX3D_USE_SSE)
    //#if defined DRX3D_USE_SSE_IN_API && defined (DRX3D_USE_SSE)
    #ifdef _WIN32

        #ifndef DRX3D_NAN
            static i32 NanMask = 0x7F800001;
            #define DRX3D_NAN (*(float *)&NanMask)
        #endif

        #ifndef DRX3D_INFINITY
            static i32 InfinityMask = 0x7F800000;
            #define DRX3D_INFINITY (*(float *)&InfinityMask)
            inline i32 GetInfinityMask()  //suppress stupid compiler warning
            {
                return InfinityMask;
            }
        #endif



    //use this, in case there are clashes (such as xnamath.h)
    #ifndef DRX3D_NO_SIMD_OPERATOR_OVERLOADS
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
    #endif  //DRX3D_NO_SIMD_OPERATOR_OVERLOADS

    #define CastfTo128i(a) (_mm_castps_si128(a))
    #define CastfTo128d(a) (_mm_castps_pd(a))
    #define CastiTo128f(a) (_mm_castsi128_ps(a))
    #define CastdTo128f(a) (_mm_castpd_ps(a))
    #define CastdTo128i(a) (_mm_castpd_si128(a))
    #define Assign128(r0, r1, r2, r3) _mm_setr_ps(r0, r1, r2, r3)

    #else  //_WIN32

        #define CastfTo128i(a) ((__m128i)(a))
        #define CastfTo128d(a) ((__m128d)(a))
        #define CastiTo128f(a) ((__m128)(a))
        #define CastdTo128f(a) ((__m128)(a))
        #define CastdTo128i(a) ((__m128i)(a))
        #define Assign128(r0, r1, r2, r3) \
            (__m128) { r0, r1, r2, r3 }
        #define DRX3D_INFINITY INFINITY
        #define DRX3D_NAN NAN
    #endif  //_WIN32
#else//DRX3D_USE_SSE

    #ifdef DRX3D_USE_NEON
    #include <arm_neon.h>

    typedef float32x4_t SimdFloat4;
    #define DRX3D_INFINITY INFINITY
    #define DRX3D_NAN NAN
    #define Assign128(r0, r1, r2, r3) \
        (float32x4_t) { r0, r1, r2, r3 }
    #else  //DRX3D_USE_NEON

    #ifndef DRX3D_INFINITY
    struct InfMaskConverter
    {
        union {
            float mask;
            i32 intmask;
        };
        InfMaskConverter(i32 _mask = 0x7F800000)
            : intmask(_mask)
        {
        }
    };
    static InfMaskConverter InfinityMask = 0x7F800000;
    #define DRX3D_INFINITY (InfinityMask.mask)
    inline i32 GetInfinityMask()  //suppress stupid compiler warning
    {
        return InfinityMask.intmask;
    }
    #endif
    #endif  //DRX3D_USE_NEON

#endif  //DRX3D_USE_SSE

#ifdef DRX3D_USE_NEON
    #include <arm_neon.h>

    typedef float32x4_t SimdFloat4;
    #define DRX3D_INFINITY INFINITY
    #define DRX3D_NAN NAN
    #define Assign128(r0, r1, r2, r3) \
        (float32x4_t) { r0, r1, r2, r3 }
#endif//DRX3D_USE_NEON

#define DRX3D_DECLARE_ALIGNED_ALLOCATOR()                                                                     \
    SIMD_FORCE_INLINE uk operator new(size_t sizeInBytes) { return AlignedAlloc(sizeInBytes, 16); }   \
    SIMD_FORCE_INLINE void operator delete(uk ptr) { AlignedFree(ptr); }                              \
    SIMD_FORCE_INLINE uk operator new(size_t, uk ptr) { return ptr; }                                \
    SIMD_FORCE_INLINE void operator delete(uk , uk ) {}                                              \
    SIMD_FORCE_INLINE uk operator new[](size_t sizeInBytes) { return AlignedAlloc(sizeInBytes, 16); } \
    SIMD_FORCE_INLINE void operator delete[](uk ptr) { AlignedFree(ptr); }                            \
    SIMD_FORCE_INLINE uk operator new[](size_t, uk ptr) { return ptr; }                              \
    SIMD_FORCE_INLINE void operator delete[](uk , uk ) {}

#if defined(DRX3D_USE_DOUBLE_PRECISION) || defined(DRX3D_FORCE_DOUBLE_FUNCTIONS)

    SIMD_FORCE_INLINE Scalar Sqrt(Scalar x)
    {
        return sqrt(x);
    }
    SIMD_FORCE_INLINE Scalar Fabs(Scalar x) { return fabs(x); }
    SIMD_FORCE_INLINE Scalar Cos(Scalar x) { return cos(x); }
    SIMD_FORCE_INLINE Scalar Sin(Scalar x) { return sin(x); }
    SIMD_FORCE_INLINE Scalar Tan(Scalar x) { return tan(x); }
    SIMD_FORCE_INLINE Scalar Acos(Scalar x)
    {
        if (x < Scalar(-1)) x = Scalar(-1);
        if (x > Scalar(1)) x = Scalar(1);
        return acos(x);
    }
    SIMD_FORCE_INLINE Scalar Asin(Scalar x)
    {
        if (x < Scalar(-1)) x = Scalar(-1);
        if (x > Scalar(1)) x = Scalar(1);
        return asin(x);
    }
    SIMD_FORCE_INLINE Scalar Atan(Scalar x) { return atan(x); }
    SIMD_FORCE_INLINE Scalar Atan2(Scalar x, Scalar y) { return atan2(x, y); }
    SIMD_FORCE_INLINE Scalar Exp(Scalar x) { return exp(x); }
    SIMD_FORCE_INLINE Scalar btLog(Scalar x) { return log(x); }
    SIMD_FORCE_INLINE Scalar Pow(Scalar x, Scalar y) { return pow(x, y); }
    SIMD_FORCE_INLINE Scalar Fmod(Scalar x, Scalar y) { return fmod(x, y); }

#else//DRX3D_USE_DOUBLE_PRECISION

    SIMD_FORCE_INLINE Scalar Sqrt(Scalar y)
    {
    #ifdef USE_APPROXIMATION
    #ifdef __LP64__
        float xhalf = 0.5f * y;
        i32 i = *(i32 *)&y;
        i = 0x5f375a86 - (i >> 1);
        y = *(float *)&i;
        y = y * (1.5f - xhalf * y * y);
        y = y * (1.5f - xhalf * y * y);
        y = y * (1.5f - xhalf * y * y);
        y = 1 / y;
        return y;
    #else
        double x, z, tempf;
        u64 *tfptr = ((u64 *)&tempf) + 1;
        tempf = y;
        *tfptr = (0xbfcdd90a - *tfptr) >> 1; /* estimate of 1/sqrt(y) */
        x = tempf;
        z = y * Scalar(0.5);
        x = (Scalar(1.5) * x) - (x * x) * (x * z); /* iteration formula     */
        x = (Scalar(1.5) * x) - (x * x) * (x * z);
        x = (Scalar(1.5) * x) - (x * x) * (x * z);
        x = (Scalar(1.5) * x) - (x * x) * (x * z);
        x = (Scalar(1.5) * x) - (x * x) * (x * z);
        return x * y;
    #endif
    #else
        return sqrtf(y);
    #endif
    }
    SIMD_FORCE_INLINE Scalar Fabs(Scalar x) { return fabsf(x); }
    SIMD_FORCE_INLINE Scalar Cos(Scalar x) { return cosf(x); }
    SIMD_FORCE_INLINE Scalar Sin(Scalar x) { return sinf(x); }
    SIMD_FORCE_INLINE Scalar Tan(Scalar x) { return tanf(x); }
    SIMD_FORCE_INLINE Scalar Acos(Scalar x)
    {
        if (x < Scalar(-1))
            x = Scalar(-1);
        if (x > Scalar(1))
            x = Scalar(1);
        return acosf(x);
    }
    SIMD_FORCE_INLINE Scalar Asin(Scalar x)
    {
        if (x < Scalar(-1))
            x = Scalar(-1);
        if (x > Scalar(1))
            x = Scalar(1);
        return asinf(x);
    }
    SIMD_FORCE_INLINE Scalar Atan(Scalar x) { return atanf(x); }
    SIMD_FORCE_INLINE Scalar Atan2(Scalar x, Scalar y) { return atan2f(x, y); }
    SIMD_FORCE_INLINE Scalar Exp(Scalar x) { return expf(x); }
    SIMD_FORCE_INLINE Scalar btLog(Scalar x) { return logf(x); }
    SIMD_FORCE_INLINE Scalar Pow(Scalar x, Scalar y) { return powf(x, y); }
    SIMD_FORCE_INLINE Scalar Fmod(Scalar x, Scalar y) { return fmodf(x, y); }

#endif//DRX3D_USE_DOUBLE_PRECISION

#define SIMD_PI Scalar(3.1415926535897932384626433832795029)
#define SIMD_2_PI (Scalar(2.0) * SIMD_PI)
#define SIMD_HALF_PI (SIMD_PI * Scalar(0.5))
#define SIMD_RADS_PER_DEG (SIMD_2_PI / Scalar(360.0))
#define SIMD_DEGS_PER_RAD (Scalar(360.0) / SIMD_2_PI)
#define SIMDSQRT12 Scalar(0.7071067811865475244008443621048490)
#define RecipSqrt(x) ((Scalar)(Scalar(1.0) / Sqrt(Scalar(x)))) /* reciprocal square root */
#define Recip(x) (Scalar(1.0) / Scalar(x))

#ifdef DRX3D_USE_DOUBLE_PRECISION
    #define SIMD_EPSILON DBL_EPSILON
    #define SIMD_INFINITY DBL_MAX
    #define DRX3D_ONE 1.0
    #define DRX3D_ZERO 0.0
    #define DRX3D_TWO 2.0
    #define DRX3D_HALF 0.5
#else
    #define SIMD_EPSILON FLT_EPSILON
    #define SIMD_INFINITY FLT_MAX
    #define DRX3D_ONE 1.0f
    #define DRX3D_ZERO 0.0f
    #define DRX3D_TWO 2.0f
    #define DRX3D_HALF 0.5f
#endif

// clang-format on

SIMD_FORCE_INLINE Scalar Atan2Fast(Scalar y, Scalar x)
{
    Scalar coeff_1 = SIMD_PI / 4.0f;
    Scalar coeff_2 = 3.0f * coeff_1;
    Scalar abs_y = Fabs(y);
    Scalar angle;
    if (x >= 0.0f)
    {
        Scalar r = (x - abs_y) / (x + abs_y);
        angle = coeff_1 - coeff_1 * r;
    }
    else
    {
        Scalar r = (x + abs_y) / (abs_y - x);
        angle = coeff_2 - coeff_1 * r;
    }
    return (y < 0.0f) ? -angle : angle;
}

SIMD_FORCE_INLINE bool FuzzyZero(Scalar x) { return Fabs(x) < SIMD_EPSILON; }

SIMD_FORCE_INLINE bool Equal(Scalar a, Scalar eps)
{
    return (((a) <= eps) && !((a) < -eps));
}
SIMD_FORCE_INLINE bool GreaterEqual(Scalar a, Scalar eps)
{
    return (!((a) <= eps));
}

SIMD_FORCE_INLINE i32 IsNegative(Scalar x)
{
    return x < Scalar(0.0) ? 1 : 0;
}

SIMD_FORCE_INLINE Scalar Radians(Scalar x) { return x * SIMD_RADS_PER_DEG; }
SIMD_FORCE_INLINE Scalar Degrees(Scalar x) { return x * SIMD_DEGS_PER_RAD; }

#define DRX3D_DECLARE_HANDLE(name) \
    typedef struct name##__     \
    {                           \
        i32 unused;             \
    } * name

#ifndef Fsel
SIMD_FORCE_INLINE Scalar Fsel(Scalar a, Scalar b, Scalar c)
{
    return a >= 0 ? b : c;
}
#endif
#define Fsels(a, b, c) (Scalar) Fsel(a, b, c)

SIMD_FORCE_INLINE bool MachineIsLittleEndian()
{
    i32 i = 1;
    tukk p = (tukk)&i;
    if (p[0] == 1)  // Lowest address contains the least significant byte
        return true;
    else
        return false;
}

//Select avoids branches, which makes performance much better for consoles like Playstation 3 and XBox 360
///Thanks Phil Knight. See also http://www.cellperformance.com/articles/2006/04/more_techniques_for_eliminatin_1.html
SIMD_FORCE_INLINE unsigned Select(unsigned condition, unsigned valueIfConditionNonZero, unsigned valueIfConditionZero)
{
    // Set testNz to 0xFFFFFFFF if condition is nonzero, 0x00000000 if condition is zero
    // Rely on positive value or'ed with its negative having sign bit on
    // and zero value or'ed with its negative (which is still zero) having sign bit off
    // Use arithmetic shift right, shifting the sign bit through all 32 bits
    unsigned testNz = (unsigned)(((i32)condition | -(i32)condition) >> 31);
    unsigned testEqz = ~testNz;
    return ((valueIfConditionNonZero & testNz) | (valueIfConditionZero & testEqz));
}
SIMD_FORCE_INLINE i32 Select(unsigned condition, i32 valueIfConditionNonZero, i32 valueIfConditionZero)
{
    unsigned testNz = (unsigned)(((i32)condition | -(i32)condition) >> 31);
    unsigned testEqz = ~testNz;
    return static_cast<i32>((valueIfConditionNonZero & testNz) | (valueIfConditionZero & testEqz));
}
SIMD_FORCE_INLINE float Select(unsigned condition, float valueIfConditionNonZero, float valueIfConditionZero)
{
#ifdef DRX3D_HAVE_NATIVE_FSEL
    return (float)Fsel((Scalar)condition - Scalar(1.0f), valueIfConditionNonZero, valueIfConditionZero);
#else
    return (condition != 0) ? valueIfConditionNonZero : valueIfConditionZero;
#endif
}

template <typename T>
SIMD_FORCE_INLINE void Swap(T &a, T &b)
{
    T tmp = a;
    a = b;
    b = tmp;
}

//PCK: endian swapping functions
SIMD_FORCE_INLINE unsigned SwapEndian(unsigned val)
{
    return (((val & 0xff000000) >> 24) | ((val & 0x00ff0000) >> 8) | ((val & 0x0000ff00) << 8) | ((val & 0x000000ff) << 24));
}

SIMD_FORCE_INLINE unsigned short SwapEndian(unsigned short val)
{
    return static_cast<unsigned short>(((val & 0xff00) >> 8) | ((val & 0x00ff) << 8));
}

SIMD_FORCE_INLINE unsigned SwapEndian(i32 val)
{
    return SwapEndian((unsigned)val);
}

SIMD_FORCE_INLINE unsigned short SwapEndian(short val)
{
    return SwapEndian((unsigned short)val);
}

//SwapFloat uses using char pointers to swap the endianness
///SwapFloatSwapDouble will NOT return a float, because the machine might 'correct' invalid floating point values
///Not all values of sign/exponent/mantissa are valid floating point numbers according to IEEE 754.
///When a floating point unit is faced with an invalid value, it may actually change the value, or worse, throw an exception.
///In most systems, running user mode code, you wouldn't get an exception, but instead the hardware/os/runtime will 'fix' the number for you.
///so instead of returning a float/double, we return integer/long long integer
SIMD_FORCE_INLINE u32 SwapEndianFloat(float d)
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
SIMD_FORCE_INLINE float UnswapEndianFloat(u32 a)
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
SIMD_FORCE_INLINE void SwapEndianDouble(double d, u8 *dst)
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
SIMD_FORCE_INLINE double UnswapEndianDouble(u8k *src)
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

template <typename T>
SIMD_FORCE_INLINE void SetZero(T *a, i32 n)
{
    T *acurr = a;
    size_t ncurr = n;
    while (ncurr > 0)
    {
        *(acurr++) = 0;
        --ncurr;
    }
}

SIMD_FORCE_INLINE Scalar btLargeDot(const Scalar *a, const Scalar *b, i32 n)
{
    Scalar p0, q0, m0, p1, q1, m1, sum;
    sum = 0;
    n -= 2;
    while (n >= 0)
    {
        p0 = a[0];
        q0 = b[0];
        m0 = p0 * q0;
        p1 = a[1];
        q1 = b[1];
        m1 = p1 * q1;
        sum += m0;
        sum += m1;
        a += 2;
        b += 2;
        n -= 2;
    }
    n += 2;
    while (n > 0)
    {
        sum += (*a) * (*b);
        a++;
        b++;
        n--;
    }
    return sum;
}

// returns normalized value in range [-SIMD_PI, SIMD_PI]
SIMD_FORCE_INLINE Scalar NormalizeAngle(Scalar angleInRadians)
{
    angleInRadians = Fmod(angleInRadians, SIMD_2_PI);
    if (angleInRadians < -SIMD_PI)
    {
        return angleInRadians + SIMD_2_PI;
    }
    else if (angleInRadians > SIMD_PI)
    {
        return angleInRadians - SIMD_2_PI;
    }
    else
    {
        return angleInRadians;
    }
}

///rudimentary class to provide type info
struct TypedObject
{
    TypedObject(i32 objectType)
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
T *AlignPointer(T *unalignedPtr, size_t alignment)
{
    struct ConvertPointerSizeT
    {
        union {
            T *ptr;
            size_t integer;
        };
    };
    ConvertPointerSizeT converter;

    const size_t bit_mask = ~(alignment - 1);
    converter.ptr = unalignedPtr;
    converter.integer += alignment - 1;
    converter.integer &= bit_mask;
    return converter.ptr;
}

#endif  //DRX3D_SCALAR_H
