// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*
   Written by:

   Bruce Dawson, Cygnus Software      Author of Fractal eXtreme for Win32
   http://www.cygnus-software.com/           comments@cygnus-software.com

   Last modified, March 2001


   This source code is supplied on an as-is basis.  You are free
   to use it and modify it, as long as this notice remains intact, and
   as long as Cygnus Software receives credit for any use where it is
   a substantial portion of the information involved.

   This source code has little in common with the high precision
   math code used for Fractal eXtreme, which has been optimized much
   more.


   This source file declares and defines a partially functional
   high precision math class, then uses it to calculate PI to high
   precision.


   For more information on the algorithms see:


   http://www.cygnus-software.com/misc/pidigits.htm


   In a real application, the class declaration, definition,
   and usage would be in three separate files.

   This code was compiled with MSVC5.0.  It should compile on
   MSVC6.0, and many other compilers.

   The platform specific code has been isolated at the top of
   the file, to allow adjusting to different sizes of ints, and
   little endian versus big endian byte ordering.

   This is not a complete math class.  In particular, it does
   not have full support for negative numbers.

   Exercises:

   1) Fully implement this class, to properly handle negative numbers,
   multiplication and division of InfPrec numbers, cos, sin, etc. of
   InfPrec numbers.

   2) Optimize this class.

   3) Template this class on its precision.
 */

#include <drx3D/Sys/StdAfx.h>

#if DRX_PLATFORM_WINDOWS

	#include <drx3D/CoreX/Assert/DrxAssert.h>
	#include <drx3D/CoreX/BaseTypes.h>
	#include <drx3D/Sys/AutoDetectCPUTestSuit.h>

//#define ENABLE_STREAMOUT

	#ifdef ENABLE_STREAMOUT
		#include <math.h>
	#endif

	#if DRX_PLATFORM_WINDOWS
		#define LITTLE_ENDIAN
	#endif

i32k BITSPERLONG(32);

inline u16 Loword(u32 x)
{
	return x & 0xFFFF;
}

inline u16 Hiword(u32 x)
{
	return (x >> 16) & 0xFFFF;
}

inline u32 Makelong(u16 h, u16 l)
{
	return (h << 16) | l;
}

template<i32 PRECISION>
class InfPrec
{
public:
	static i32k NUMLONGS = PRECISION;
	static i32k NUMWORDS = PRECISION * 2;

public:
	InfPrec(i32 Value = 0) { Init(Value); }
	InfPrec(const InfPrec& rhs) { Init(rhs); }
	InfPrec& operator=(const InfPrec& rhs) { Init(rhs); return *this; }

	bool     operator==(i32 rhs) const;
	bool     operator<(i32 rhs) const;
	bool     operator!=(i32 rhs) const { return !(*this == rhs); }
	bool     operator<=(i32 rhs) const { return *this < rhs || *this == rhs; }
	bool     operator>=(i32 rhs) const { return !(*this < rhs); }
	bool     operator>(i32 rhs) const  { return !(*this <= rhs); }

	InfPrec& operator*=(i32 rhs);
	InfPrec  operator*(i32 rhs) const { return InfPrec(*this) *= rhs; }

	InfPrec& operator/=(i32 rhs);
	InfPrec  operator/(i32 rhs) const { return InfPrec(*this) /= rhs; }

	InfPrec& operator+=(const InfPrec& rhs);
	InfPrec  operator+(const InfPrec& rhs) const { return InfPrec(*this) += rhs; }

	InfPrec  operator-() const;
	InfPrec& operator-=(const InfPrec& rhs);
	InfPrec  operator-(const InfPrec& rhs) const { return InfPrec(*this) -= rhs; }

	#ifdef ENABLE_STREAMOUT
	void Print();
	#endif

private:
	void Init(i32 Value);
	void Init(const InfPrec& rhs);

	#ifdef  LITTLE_ENDIAN
	u32&       GetLong(i32 Index)       { return mNumber[NUMLONGS - 1 - Index]; }
	u32k& GetLong(i32 Index) const { return mNumber[NUMLONGS - 1 - Index]; }
	u16&       GetWord(i32 Index)       { return reinterpret_cast<u16*>(mNumber)[NUMWORDS - 1 - Index]; }
	u16k& GetWord(i32 Index) const { return reinterpret_cast<u16k*>(mNumber)[NUMWORDS - 1 - Index]; }
	#else
	u32&       GetLong(i32 Index)       { return mNumber[Index]; }
	u32k& GetLong(i32 Index) const { return mNumber[Index]; }
	u16&       GetWord(i32 Index)       { return reinterpret_cast<u16*>(mNumber)[Index]; }
	u16k& GetWord(i32 Index) const { return reinterpret_cast<u16k*>(mNumber)[Index]; }
	#endif

	u32 mNumber[NUMLONGS];
};

template<i32 PRECISION>
bool InfPrec<PRECISION >::operator==(i32 rhs) const
{
	if ((i32)GetLong(0) != rhs)
		return false;
	for (i32 i = 1; i < NUMLONGS; i++)
		if (GetLong(i) != 0)
			return false;
	return true;
}

template<i32 PRECISION>
bool InfPrec<PRECISION >::operator<(i32 rhs) const
{
	return (i32)GetLong(0) < rhs;
}

template<i32 PRECISION>
InfPrec<PRECISION>& InfPrec<PRECISION >::operator*=(i32 rhs)
{
	assert(rhs >= 0);
	assert(*this >= 0);
	u32 accum = 0;
	for (i32 i = NUMWORDS - 1; i >= 0; i--)
	{
		accum += GetWord(i) * rhs;
		GetWord(i) = Loword(accum);
		accum = Hiword(accum);
	}
	return *this;
}

template<i32 PRECISION>
InfPrec<PRECISION>& InfPrec<PRECISION >::operator/=(i32 rhs)
{
	assert(rhs > 0);
	assert(*this >= 0);
	u16 remainder = 0;
	for (i32 i = 0; i < NUMWORDS; i++)
	{
		u32 divisor = Makelong(remainder, GetWord(i));
		GetWord(i) = (u16)(divisor / rhs);
		remainder = (u16)(divisor % rhs);
	}
	return *this;
}

template<i32 PRECISION>
InfPrec<PRECISION>& InfPrec<PRECISION >::operator+=(const InfPrec& rhs)
{
	u32 accum = 0;
	for (i32 i = NUMWORDS - 1; i >= 0; i--)
	{
		accum += GetWord(i) + rhs.GetWord(i);
		GetWord(i) = Loword(accum);
		accum = Hiword(accum);
	}
	return *this;
}

template<i32 PRECISION>
InfPrec<PRECISION> InfPrec<PRECISION >::operator-() const
{
	InfPrec Result = *this;
	for (i32 i = 0; i < NUMLONGS; i++)
		Result.GetLong(i) = ~Result.GetLong(i);
	bool carry = true;
	for (i32 i = NUMLONGS - 1; i >= 0 && carry; i--)
		if (++Result.GetLong(i) != 0)
			carry = false;
	return Result;
}

template<i32 PRECISION>
InfPrec<PRECISION>& InfPrec<PRECISION >::operator-=(const InfPrec& rhs)
{
	*this += -rhs;
	return *this;
}

template<i32 PRECISION>
void InfPrec<PRECISION >::Init(i32 Value)
{
	GetLong(0) = Value;
	for (i32 i = 1; i < NUMLONGS; i++)
		GetLong(i) = 0;
}

template<i32 PRECISION>
void InfPrec<PRECISION >::Init(const InfPrec& rhs)
{
	for (i32 i = 0; i < NUMLONGS; i++)
		mNumber[i] = rhs.mNumber[i];
}

template<i32 PRECISION>
InfPrec<PRECISION> ataninvint(i32 x)
{
	InfPrec<PRECISION> Result = InfPrec<PRECISION>(1) / x;
	i32 XSquared = x * x;

	i32 Divisor = 1;
	InfPrec<PRECISION> Term = Result;
	InfPrec<PRECISION> TempTerm;
	while (Term != 0)
	{
		Divisor += 2;
		Term /= XSquared;
		Result -= Term / Divisor;

		Divisor += 2;
		Term /= XSquared;
		Result += Term / Divisor;
	}
	return Result;
}

	#ifdef ENABLE_STREAMOUT
template<i32 PRECISION>
void InfPrec<PRECISION >::Print()
{
	InfPrec<PRECISION> Copy = *this; //rhs*
	printf("%d.", Copy.GetLong(0));
	Copy.GetLong(0) = 0;
	//i32 NumDigits = i32((NUMLONGS - 1) * BITSPERLONG * (log(2.0) / log(10.0)));
	i32 NumDigits = (i32) ((NUMLONGS - 1) * BITSPERLONG * (log(2.0) / log(10.0)));
	while (Copy != 0 && NumDigits-- > 0)
	{
		Copy *= 10;
		printf("%d", Copy.GetLong(0));
		Copy.GetLong(0) = 0;
	}
}
	#endif

i32 CCPUTestSuite::RunTest()
{
	i32k PERFPREC(1024);
	InfPrec<PERFPREC> PI((ataninvint<PERFPREC>(5) * 4 - ataninvint<PERFPREC>(239)) * 4);
	//PI.Print();
	return 0;
}

#endif // #if DRX_PLATFORM_WINDOWS
