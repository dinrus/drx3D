// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef DRX_HALF_INL
#define DRX_HALF_INL

#pragma once

typedef u16 DrxHalf;
class IDrxSizer;

typedef union floatint_union
{
	float  f;
	u32 i;
} floatint_union;

ILINE DrxHalf DrxConvertFloatToHalf(const float Value)
{
	MEMORY_RW_REORDERING_BARRIER;
	unsigned int Result;

	unsigned int IValue = ((unsigned int*)(&Value))[0];
	unsigned int Sign = (IValue & 0x80000000U) >> 16U;
	IValue = IValue & 0x7FFFFFFFU;      // Hack off the sign

	if (IValue > 0x47FFEFFFU)
	{
		// The number is too large to be represented as a half.  Saturate to infinity.
		Result = 0x7FFFU;
	}
	else
	{
		if (IValue < 0x38800000U)
		{
			// The number is too small to be represented as a normalized half.
			// Convert it to a denormalized value.
			unsigned int Shift = 113U - (IValue >> 23U);
#if defined(DRX_UBSAN)
			// UBSAN (technically correctly) detects 113 bits of shift when converting 0.0f, which is undefined behavior.
			// The intended behavior (and probably actual behavior on all CPU) is for those shifts to result in 0.
			if (Shift >= 32)
			{
				IValue = 0;
			}
			else
#endif
			IValue = (0x800000U | (IValue & 0x7FFFFFU)) >> Shift;
		}
		else
		{
			// Rebias the exponent to represent the value as a normalized half.
			IValue += 0xC8000000U;
		}

		Result = ((IValue + 0x0FFFU + ((IValue >> 13U) & 1U)) >> 13U) & 0x7FFFU;
	}
	return (DrxHalf)(Result | Sign);
}

ILINE float DrxConvertHalfToFloat(const DrxHalf Value)
{
	MEMORY_RW_REORDERING_BARRIER;
	unsigned int Mantissa;
	unsigned int Exponent;
	unsigned int Result;

	Mantissa = (unsigned int)(Value & 0x03FF);

	if ((Value & 0x7C00) != 0)  // The value is normalized
	{
		Exponent = (unsigned int)((Value >> 10) & 0x1F);
	}
	else if (Mantissa != 0)     // The value is denormalized
	{
		// Normalize the value in the resulting float
		Exponent = 1;

		do
		{
			Exponent--;
			Mantissa <<= 1;
		}
		while ((Mantissa & 0x0400) == 0);

		Mantissa &= 0x03FF;
	}
	else                        // The value is zero
	{
		Exponent = (unsigned int)-112;
	}

	Result = ((Value & 0x8000) << 16) | // Sign
	         ((Exponent + 112) << 23) | // Exponent
	         (Mantissa << 13);          // Mantissa

	return *(float*)&Result;
}

struct DrxHalf2
{
	DrxHalf x;
	DrxHalf y;

	DrxHalf2()
	{
	}
	DrxHalf2(DrxHalf _x, DrxHalf _y)
		: x(_x)
		, y(_y)
	{
	}
	DrxHalf2(const DrxHalf* const __restrict pArray)
	{
		x = pArray[0];
		y = pArray[1];
	}
	DrxHalf2(float _x, float _y)
	{
		x = DrxConvertFloatToHalf(_x);
		y = DrxConvertFloatToHalf(_y);
	}
	DrxHalf2(const float* const __restrict pArray)
	{
		x = DrxConvertFloatToHalf(pArray[0]);
		y = DrxConvertFloatToHalf(pArray[1]);
	}
	DrxHalf2& operator=(const DrxHalf2& Half2)
	{
		x = Half2.x;
		y = Half2.y;
		return *this;
	}

	bool operator!=(const DrxHalf2& rhs) const
	{
		return x != rhs.x || y != rhs.y;
	}

	bool operator==(const DrxHalf2& rhs) const
	{
		return x == rhs.x && y == rhs.y;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const {}

	AUTO_STRUCT_INFO;
};

struct DrxHalf4
{
	DrxHalf x;
	DrxHalf y;
	DrxHalf z;
	DrxHalf w;

	DrxHalf4()
	{
	}
	DrxHalf4(DrxHalf _x, DrxHalf _y, DrxHalf _z, DrxHalf _w)
		: x(_x)
		, y(_y)
		, z(_z)
		, w(_w)
	{
	}
	DrxHalf4(const DrxHalf* const __restrict pArray)
	{
		x = pArray[0];
		y = pArray[1];
		z = pArray[2];
		w = pArray[3];
	}
	DrxHalf4(float _x, float _y, float _z, float _w)
	{
		x = DrxConvertFloatToHalf(_x);
		y = DrxConvertFloatToHalf(_y);
		z = DrxConvertFloatToHalf(_z);
		w = DrxConvertFloatToHalf(_w);
	}
	DrxHalf4(const float* const __restrict pArray)
	{
		x = DrxConvertFloatToHalf(pArray[0]);
		y = DrxConvertFloatToHalf(pArray[1]);
		z = DrxConvertFloatToHalf(pArray[2]);
		w = DrxConvertFloatToHalf(pArray[3]);
	}
	DrxHalf4& operator=(const DrxHalf4& Half4)
	{
		x = Half4.x;
		y = Half4.y;
		z = Half4.z;
		w = Half4.w;
		return *this;
	}
	bool operator!=(const DrxHalf4& rhs) const
	{
		return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const {}

	AUTO_STRUCT_INFO;
};

#endif // #ifndef DRX_HALF_INL
