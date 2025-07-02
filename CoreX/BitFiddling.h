// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Вычисляет число вводных (leading) нулей.
// Результат находится в диапазоне от 0 to 32/64.
#if (DRX_COMPILER_GCC || DRX_COMPILER_CLANG)

	// builtin doesn't want to work for 0
	#define countLeadingZeros32(x) ((x) ? __builtin_clz  (x) : 32)
	#define countLeadingZeros64(x) ((x) ? __builtin_clzll(x) : 64)

#elif DRX_PLATFORM_SSE4

	#define countLeadingZeros32(x) __lzcnt(x)
	#define countLeadingZeros64(x) __lzcnt64(x)

#else  // Реализация для Windows

ILINE u32 countLeadingZeros32(u32 x)
{
	u64 result = 32 ^ 31;
	if (!_BitScanReverse(&result, x))
		result = 32 ^ 31;
	PREFAST_SUPPRESS_WARNING(6102);
	result ^= 31; // needed because the index is from LSB (whereas all other implementations are from MSB)
	return result;
}

ILINE uint64 countLeadingZeros64(uint64 x)
{
#if DRX_PLATFORM_X64
	u64 result = 64 ^ 63;
	if (!_BitScanReverse64(&result, x))
		result = 64 ^ 63;
	PREFAST_SUPPRESS_WARNING(6102);
	result ^= 63; // needed because the index is from LSB (whereas all other implementations are from MSB)
#else
	u64 result = (x & 0xFFFFFFFF00000000ULL)
		? countLeadingZeros32(u32(x >> 32)) +  0
		: countLeadingZeros32(u32(x >>  0)) + 32;
#endif
	return result;
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Подсчитывает число завершающих (trailing) нулей.
// Результат находится в диапазоне от 0 до 32/64.
#if (DRX_COMPILER_GCC || DRX_COMPILER_CLANG)

	// builtin не срабатывает для 0
	#define countTrailingZeros32(x) ((x) ? __builtin_ctz  (x) : 32)
	#define countTrailingZeros64(x) ((x) ? __builtin_ctzll(x) : 64)

#elif DRX_PLATFORM_BMI1
	#include "ammintrin.h"

	#define countTrailingZeros32(x) _tzcnt_u32(x)
	#define countTrailingZeros64(x) _tzcnt_u64(x)

#else  // Реализация для Windows.

ILINE u32 countTrailingZeros32(u32 x)
{
	u64 result = 32;
	if (!_BitScanForward(&result, x))
		result = 32;
	PREFAST_SUPPRESS_WARNING(6102);
	return result;
}

ILINE uint64 countTrailingZeros64(uint64 x)
{
#if DRX_PLATFORM_X64
	u64 result = 64;
	if (!_BitScanForward64(&result, x))
		result = 64;
	PREFAST_SUPPRESS_WARNING(6102);
#else
	u64 result = (x & 0x00000000FFFFFFFFULL)
		? countTrailingZeros32(u32(x >>  0)) +  0
		: countTrailingZeros32(u32(x >> 32)) + 32;
#endif
	return result;
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
#if DRX_COMPILER_GCC || DRX_COMPILER_CLANG

inline u32 circularShift(u32 shiftBits, u32 value)
{
	return (value << shiftBits) | (value >> (32 - shiftBits));
}

#else  // Реализация для Windows

	#define circularShift(shiftBits, value) _rotl(value, shiftBits)

#endif

#if DRX_PLATFORM_BMI1

	#define isolateLowestBit(x) _blsi_u32(x)
	#define clearLowestBit(x) _blsr_u32(x)

#else  // Реализация для Windows

ILINE u32 isolateLowestBit(u32 x)
{
	PREFAST_SUPPRESS_WARNING(4146);
	return x & (-x);
}

ILINE u32 clearLowestBit(u32 x)
{
	return x & (x - 1);
}
#endif

#if DRX_PLATFORM_TBM

	#define fillFromLowestBit32(x) _blsfill_u32(x)
	#define fillFromLowestBit64(x) _blsfill_u64(x)

#else

ILINE u32 fillFromLowestBit32(u32 x)
{
	return x | (x - 1);
}

ILINE uint64 fillFromLowestBit64(uint64 x)
{
	return x | (x - 1);
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
class CBitIter
{
public:
	ILINE CBitIter(u8  x) : m_val(x) {}
	ILINE CBitIter(u16 x) : m_val(x) {}
	ILINE CBitIter(u32 x) : m_val(x) {}

	template<class T>
	ILINE bool Next(T& rIndex)
	{
		u32 maskLSB = isolateLowestBit(m_val);
		rIndex = countTrailingZeros32(maskLSB);
		m_val ^= maskLSB;
		return maskLSB != 0;
	}

private:
	u32 m_val;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename TInteger>
inline bool IsPowerOfTwo(TInteger x)
{
	return clearLowestBit(x) == 0;
}

//! Версия времени компиляции IsPowerOfTwo (являетсяСтепеньюДва_ли), применяемая для STATIC_CHECK.
template<i32 nValue>
struct IsPowerOfTwoCompileTime
{
	enum { IsPowerOfTwo = ((nValue & (nValue - 1)) == 0) };
};

// Вычисляет логарифм с основанием 2 для заданного числа.
// Передача 0 вернёт -1 при беззначном типе данных.
// Результат находится в диапазоне от -1 до 7/15/31/63.
#if DRX_COMPILER_GCC || DRX_COMPILER_CLANG

static ILINE u8  IntegerLog2(u8  v) { return u8 (31U   - __builtin_clz  (v)); }
static ILINE u16 IntegerLog2(u16 v) { return u16(31U   - __builtin_clz  (v)); }
static ILINE u32 IntegerLog2(u32 v) { return u32(31UL  - __builtin_clz  (v)); }
static ILINE uint64 IntegerLog2(uint64 v) { return uint64(63ULL - __builtin_clzll(v)); }

#else  // Реализация для Windows

static ILINE u8  IntegerLog2(u8  v) { if (!v) return u8 (~0UL ); u64 result; _BitScanReverse  (&result, v); return u8 (result); }
static ILINE u16 IntegerLog2(u16 v) { if (!v) return u16(~0UL ); u64 result; _BitScanReverse  (&result, v); return u16(result); }
static ILINE u32 IntegerLog2(u32 v) { if (!v) return u32(~0UL ); u64 result; _BitScanReverse  (&result, v); return u32(result); }
#if DRX_PLATFORM_X64
static ILINE uint64 IntegerLog2(uint64 v) { if (!v) return uint64(~0ULL); u64 result; _BitScanReverse64(&result, v); return uint64(result); }
#else
static ILINE uint64 IntegerLog2(uint64 v)
{
	if (!v)
		return ~0ULL;
	u64 result;
	if (u32 w = u32(v >> 32))
		_BitScanReverse(&result, u32(w)), result += 32;
	else
		_BitScanReverse(&result, u32(v)), result +=  0;
	return result;
}
#endif

#endif

// Вычисляет число битов, необходимое для представления числа.
// Передача 0 вернёт максимальное число битов.
// Результат находится в диапазоне от 0 до 8/16/32/64.
static ILINE u8  IntegerLog2_RoundUp(u8  v) { return u8 (IntegerLog2(u8 (v - 1U  )) + 1U  ); }
static ILINE u16 IntegerLog2_RoundUp(u16 v) { return u16(IntegerLog2(u16(v - 1U  )) + 1U  ); }
static ILINE u32 IntegerLog2_RoundUp(u32 v) { return u32(IntegerLog2(u32(v - 1UL )) + 1UL ); }
static ILINE uint64 IntegerLog2_RoundUp(uint64 v) { return uint64(IntegerLog2(uint64(v - 1ULL)) + 1ULL); }

// Вычисляет верхний уровень степени 2 заданного числа.
// Passing 0 will return 1 on x86 (shift modulo datatype size is no shift) and 0 on other platforms.
// Результат находится в диапазоне от 1 до всех установленных битов.
inline u32 NextPower2(u32 x)
{
	return 1UL << IntegerLog2_RoundUp(x);
}

inline uint64 NextPower2_64(uint64 x)
{
	return 1ULL << IntegerLog2_RoundUp(x);
}

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
inline u64 IntegerLog2(u64 x)
{
	#if DRX_PLATFORM_64BIT
	return IntegerLog2((uint64)x);
	#else
	return IntegerLog2((u32)x);
	#endif
}

#endif

#if DRX_PLATFORM_ORBIS
inline size_t IntegerLog2(size_t x)
{
	if (sizeof(size_t) == sizeof(u32))
		return IntegerLog2((u32)x);
	else
		return IntegerLog2((uint64)x);
}
#endif

// Find-first-MSB-set
static ILINE u8 BitIndex(u8  v) { return u8(IntegerLog2(v)); }
static ILINE u8 BitIndex(u16 v) { return u8(IntegerLog2(v)); }
static ILINE u8 BitIndex(u32 v) { return u8(IntegerLog2(v)); }
static ILINE u8 BitIndex(uint64 v) { return u8(IntegerLog2(v)); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Вычисляет число установленных битов.
#if DRX_COMPILER_GCC || DRX_COMPILER_CLANG

static ILINE u8 CountBits(u8  v) { return u8(__builtin_popcount  (v)); }
static ILINE u8 CountBits(u16 v) { return u8(__builtin_popcount  (v)); }
static ILINE u8 CountBits(u32 v) { return u8(__builtin_popcount  (v)); }
static ILINE u8 CountBits(uint64 v) { return u8(__builtin_popcountll(v)); }

#elif DRX_PLATFORM_SSE4

static ILINE u8 CountBits(u8  v) { return u8(__popcnt  (v)); }
static ILINE u8 CountBits(u16 v) { return u8(__popcnt  (v)); }
static ILINE u8 CountBits(u32 v) { return u8(__popcnt  (v)); }
static ILINE u8 CountBits(uint64 v) { return u8(__popcnt64(v)); }

#else

static ILINE u8 CountBits(u8 v)
{
	u8 c = v;
	c = ((c >> 1) & 0x55) + (c & 0x55);
	c = ((c >> 2) & 0x33) + (c & 0x33);
	c = ((c >> 4) & 0x0f) + (c & 0x0f);
	return c;
}

static ILINE u8 CountBits(u16 v)
{
	return
		CountBits((u8)(v & 0xff)) +
		CountBits((u8)((v >> 8) & 0xff));
}

static ILINE u8 CountBits(u32 v)
{
	return
		CountBits((u16)(v & 0xffff)) +
		CountBits((u16)((v >> 16) & 0xffff));
}

static ILINE u8 CountBits(uint64 v)
{
	return
		CountBits((u32)(v & 0xffffffff)) +
		CountBits((u32)((v >> 32) & 0xffffffff));
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Branchless version of "return v < 0 ? alt : v;".
ILINE i32 Isel32(i32 v, i32 alt)
{
	return ((static_cast<i32>(v) >> 31) & alt) | ((static_cast<i32>(~v) >> 31) & v);
}

template<u32 ILOG>
struct CompileTimeIntegerLog2
{
	static u32k result = 1 + CompileTimeIntegerLog2<(ILOG >> 1)>::result;
};

template<>
struct CompileTimeIntegerLog2<1>
{
	static u32k result = 0;
};

template<>
struct CompileTimeIntegerLog2<0>; //!< Keep it undefined, we cannot represent "minus infinity" result.

static_assert(CompileTimeIntegerLog2<1>::result == 0, "Wrong calculation result!");
static_assert(CompileTimeIntegerLog2<2>::result == 1, "Wrong calculation result!");
static_assert(CompileTimeIntegerLog2<3>::result == 1, "Wrong calculation result!");
static_assert(CompileTimeIntegerLog2<4>::result == 2, "Wrong calculation result!");
static_assert(CompileTimeIntegerLog2<5>::result == 2, "Wrong calculation result!");
static_assert(CompileTimeIntegerLog2<255>::result == 7, "Wrong calculation result!");
static_assert(CompileTimeIntegerLog2<256>::result == 8, "Wrong calculation result!");
static_assert(CompileTimeIntegerLog2<257>::result == 8, "Wrong calculation result!");

template<u32 ILOG>
struct CompileTimeIntegerLog2_RoundUp
{
	static u32k result = CompileTimeIntegerLog2<ILOG>::result + ((ILOG & (ILOG - 1)) != 0);
};
template<>
struct CompileTimeIntegerLog2_RoundUp<0>; //!< We can return 0, but let's keep it undefined (same as CompileTimeIntegerLog2<0>).

static_assert(CompileTimeIntegerLog2_RoundUp<1>::result == 0, "Wrong calculation result!");
static_assert(CompileTimeIntegerLog2_RoundUp<2>::result == 1, "Wrong calculation result!");
static_assert(CompileTimeIntegerLog2_RoundUp<3>::result == 2, "Wrong calculation result!");
static_assert(CompileTimeIntegerLog2_RoundUp<4>::result == 2, "Wrong calculation result!");
static_assert(CompileTimeIntegerLog2_RoundUp<5>::result == 3, "Wrong calculation result!");
static_assert(CompileTimeIntegerLog2_RoundUp<255>::result == 8, "Wrong calculation result!");
static_assert(CompileTimeIntegerLog2_RoundUp<256>::result == 8, "Wrong calculation result!");
static_assert(CompileTimeIntegerLog2_RoundUp<257>::result == 9, "Wrong calculation result!");

// Character-to-bitfield mapping

inline u32 AlphaBit(char c)
{
	return c >= 'a' && c <= 'z' ? 1 << (c - 'z' + 31) : 0;
}

inline uint64 AlphaBit64(char c)
{
	return (c >= 'a' && c <= 'z' ? 1U << (c - 'z' + 31) : 0) |
	       (c >= 'A' && c <= 'Z' ? 1LL << (c - 'Z' + 63) : 0);
}

inline u32 AlphaBits(u32 wc)
{
	// Handle wide multi-char constants, can be evaluated at compile-time.
	return AlphaBit((char)wc)
	       | AlphaBit((char)(wc >> 8))
	       | AlphaBit((char)(wc >> 16))
	       | AlphaBit((char)(wc >> 24));
}

inline u32 AlphaBits(tukk s)
{
	// Handle string of any length.
	u32 n = 0;
	while (*s)
		n |= AlphaBit(*s++);
	return n;
}

inline uint64 AlphaBits64(tukk s)
{
	// Handle string of any length.
	uint64 n = 0;
	while (*s)
		n |= AlphaBit64(*s++);
	return n;
}

//! \param s should point to a buffer at least 65 chars long
inline void BitsAlpha64(uint64 n, tuk s)
{
	for (i32 i = 0; n != 0; n >>= 1, i++)
		if (n & 1)
			*s++ = i < 32 ? i + 'z' - 31 : i + 'Z' - 63;
	*s++ = '\0';
}

//! If the hardware doesn't support 3Dc we can convert to DXT5 (different channels are used) with almost the same quality but the same memory requirements.
inline void ConvertBlock3DcToDXT5(u8 pDstBlock[16], u8k pSrcBlock[16])
{
	assert(pDstBlock != pSrcBlock);   // does not work in place

	// 4x4 block requires 8 bytes in DXT5 or 3DC

	// DXT5:  8 bit alpha0, 8 bit alpha1, 16*3 bit alpha lerp
	//        16bit col0, 16 bit col1 (R5G6B5 low byte then high byte), 16*2 bit color lerp

	//  3DC:  8 bit x0, 8 bit x1, 16*3 bit x lerp
	//        8 bit y0, 8 bit y1, 16*3 bit y lerp

	for (u32 dwK = 0; dwK < 8; ++dwK)
		pDstBlock[dwK] = pSrcBlock[dwK];
	for (u32 dwK = 8; dwK < 16; ++dwK)
		pDstBlock[dwK] = 0;

	// 6 bit green channel (highest bits)
	// by using all 3 channels with a slight offset we can get more precision but then a dot product would be needed in PS
	// because of bilinear filter we cannot just distribute bits to get perfect result
	u16 colDst0 = (((u16)pSrcBlock[8] + 2) >> 2) << 5;
	u16 colDst1 = (((u16)pSrcBlock[9] + 2) >> 2) << 5;

	bool bFlip = colDst0 <= colDst1;

	if (bFlip)
	{
		u16 help = colDst0;
		colDst0 = colDst1;
		colDst1 = help;
	}

	bool bEqual = colDst0 == colDst1;

	// distribute bytes by hand to not have problems with endianess
	pDstBlock[8 + 0] = (u8)colDst0;
	pDstBlock[8 + 1] = (u8)(colDst0 >> 8);
	pDstBlock[8 + 2] = (u8)colDst1;
	pDstBlock[8 + 3] = (u8)(colDst1 >> 8);

	u16* pSrcBlock16 = (u16*)(pSrcBlock + 10);
	u16* pDstBlock16 = (u16*)(pDstBlock + 12);

	// distribute 16 3 bit values to 16 2 bit values (loosing LSB)
	for (u32 dwK = 0; dwK < 16; ++dwK)
	{
		u32 dwBit0 = dwK * 3 + 0;
		u32 dwBit1 = dwK * 3 + 1;
		u32 dwBit2 = dwK * 3 + 2;

		u8 hexDataIn = (((pSrcBlock16[(dwBit2 >> 4)] >> (dwBit2 & 0xf)) & 1) << 2)     // get HSB
		                  | (((pSrcBlock16[(dwBit1 >> 4)] >> (dwBit1 & 0xf)) & 1) << 1)
		                  | ((pSrcBlock16[(dwBit0 >> 4)] >> (dwBit0 & 0xf)) & 1); // get LSB

		u8 hexDataOut = 0;

		switch (hexDataIn)
		{
		case 0:
			hexDataOut = 0;
			break;                        // color 0
		case 1:
			hexDataOut = 1;
			break;                        // color 1

		case 2:
			hexDataOut = 0;
			break;                        // mostly color 0
		case 3:
			hexDataOut = 2;
			break;
		case 4:
			hexDataOut = 2;
			break;
		case 5:
			hexDataOut = 3;
			break;
		case 6:
			hexDataOut = 3;
			break;
		case 7:
			hexDataOut = 1;
			break;                        // mostly color 1

		default:
			assert(0);
			break;
		}

		if (bFlip)
		{
			if (hexDataOut < 2)
				hexDataOut = 1 - hexDataOut;    // 0<->1
			else
				hexDataOut = 5 - hexDataOut;  // 2<->3
		}

		if (bEqual)
			if (hexDataOut == 3)
				hexDataOut = 1;

		pDstBlock16[(dwK >> 3)] |= (hexDataOut << ((dwK & 0x7) << 1));
	}
}

//! Is a bit on in a new bit field, but off in an old bit field.
static ILINE bool TurnedOnBit(unsigned bit, unsigned oldBits, unsigned newBits)
{
	return (newBits & bit) != 0 && (oldBits & bit) == 0;
}

inline u32 cellUtilCountLeadingZero(u32 x)
{
	u32 y;
	u32 n = 32;

	y = x >> 16;
	if (y != 0) { n = n - 16; x = y; }
	y = x >> 8;
	if (y != 0) { n = n - 8; x = y; }
	y = x >> 4;
	if (y != 0) { n = n - 4; x = y; }
	y = x >> 2;
	if (y != 0) { n = n - 2; x = y; }
	y = x >> 1;
	if (y != 0) { return n - 2; }
	return n - x;
}

inline u32 cellUtilLog2(u32 x)
{
	return 31 - cellUtilCountLeadingZero(x);
}

inline void convertSwizzle(
  u8*& dst, u8k*& src,
  u32k SrcPitch, u32k depth,
  u32k xpos, u32k ypos,
  u32k SciX1, u32k SciY1,
  u32k SciX2, u32k SciY2,
  u32k level)
{
	if (level == 1)
	{
		switch (depth)
		{
		case 16:
			if (xpos >= SciX1 && xpos < SciX2 && ypos >= SciY1 && ypos < SciY2)
			{
				//					*((u32*&)dst)++ = ((u32*)src)[ypos * width + xpos];
				//					*((u32*&)dst)++ = ((u32*)src)[ypos * width + xpos+1];
				//					*((u32*&)dst)++ = ((u32*)src)[ypos * width + xpos+2];
				//					*((u32*&)dst)++ = ((u32*)src)[ypos * width + xpos+3];
				*((u32*&)dst)++ = *((u32*)(src + (ypos * SrcPitch + xpos * 16)));
				*((u32*&)dst)++ = *((u32*)(src + (ypos * SrcPitch + xpos * 16 + 4)));
				*((u32*&)dst)++ = *((u32*)(src + (ypos * SrcPitch + xpos * 16 + 8)));
				*((u32*&)dst)++ = *((u32*)(src + (ypos * SrcPitch + xpos * 16 + 12)));
			}
			else
				((u32*&)dst) += 4;
			break;
		case 8:
			if (xpos >= SciX1 && xpos < SciX2 && ypos >= SciY1 && ypos < SciY2)
			{
				*((u32*&)dst)++ = *((u32*)(src + (ypos * SrcPitch + xpos * 8)));
				*((u32*&)dst)++ = *((u32*)(src + (ypos * SrcPitch + xpos * 8 + 4)));
			}
			else
				((u32*&)dst) += 2;
			break;
		case 4:
			if (xpos >= SciX1 && xpos < SciX2 && ypos >= SciY1 && ypos < SciY2)
				*((u32*&)dst) = *((u32*)(src + (ypos * SrcPitch + xpos * 4)));
			dst += 4;
			break;
		case 3:
			if (xpos >= SciX1 && xpos < SciX2 && ypos >= SciY1 && ypos < SciY2)
			{
				*dst++ = src[ypos * SrcPitch + xpos * depth];
				*dst++ = src[ypos * SrcPitch + xpos * depth + 1];
				*dst++ = src[ypos * SrcPitch + xpos * depth + 2];
			}
			else
				dst += 3;
			break;
		case 1:
			if (xpos >= SciX1 && xpos < SciX2 && ypos >= SciY1 && ypos < SciY2)
				*dst++ = src[ypos * SrcPitch + xpos * depth];
			else
				dst++;
			break;
		default:
			assert(0);
			break;
		}
		return;
	}
	else
	{
		convertSwizzle(dst, src, SrcPitch, depth, xpos, ypos, SciX1, SciY1, SciX2, SciY2, level - 1);
		convertSwizzle(dst, src, SrcPitch, depth, xpos + (1U << (level - 2)), ypos, SciX1, SciY1, SciX2, SciY2, level - 1);
		convertSwizzle(dst, src, SrcPitch, depth, xpos, ypos + (1U << (level - 2)), SciX1, SciY1, SciX2, SciY2, level - 1);
		convertSwizzle(dst, src, SrcPitch, depth, xpos + (1U << (level - 2)), ypos + (1U << (level - 2)), SciX1, SciY1, SciX2, SciY2, level - 1);
	}
}

inline void Linear2Swizzle(
  u8* dst,
  u8k* src,
  u32k SrcPitch,
  u32k width,
  u32k height,
  u32k depth,
  u32k SciX1, u32k SciY1,
  u32k SciX2, u32k SciY2)
{
	src -= SciY1 * SrcPitch + SciX1 * depth;
	if (width == height)
		convertSwizzle(dst, src, SrcPitch, depth, 0, 0, SciX1, SciY1, SciX2, SciY2, cellUtilLog2(width) + 1);
	else if (width > height)
	{
		u32 baseLevel = cellUtilLog2(width) - (cellUtilLog2(width) - cellUtilLog2(height));
		for (u32 i = 0; i < (1UL << (cellUtilLog2(width) - cellUtilLog2(height))); i++)
			convertSwizzle(dst, src, SrcPitch, depth, (1U << baseLevel) * i, 0, SciX1, SciY1, SciX2, SciY2, baseLevel + 1);
	}
	else
	{
		u32 baseLevel = cellUtilLog2(height) - (cellUtilLog2(height) - cellUtilLog2(width));
		for (u32 i = 0; i < (1UL << (cellUtilLog2(height) - cellUtilLog2(width))); i++)
			convertSwizzle(dst, src, SrcPitch, depth, 0, (1U << baseLevel) * i, SciX1, SciY1, SciX2, SciY2, baseLevel + 1);
	}
}
