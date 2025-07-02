// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __BRANCHLESS_MASK__
#define __BRANCHLESS_MASK__

///////////////////////////////////////////
// helper functions for branch elimination
//
// msb/lsb - most/less significant byte
//
// mask - 0xFFFFFFFF
// nz   - not zero
// zr   - is zero

ILINE u32k nz2msb(u32k x)
{
	return -(i32)x | x;
}

ILINE u32k msb2mask(u32k x)
{
	return (i32)(x) >> 31;
}

ILINE u32k nz2one(u32k x)
{
	return nz2msb(x) >> 31; // i32((bool)x);
}

ILINE u32k nz2mask(u32k x)
{
	return (i32)msb2mask(nz2msb(x)); // -(i32)(bool)x;
}

//! Select integer with mask (0xFFFFFFFF or 0x0 only!).
ILINE u32k iselmask(u32k mask, u32 x, u32k y)
{
	return (x & mask) | (y & ~mask);
}

//! mask if( x != 0 && y != 0)
ILINE u32k mask_nz_nz(u32k x, u32k y)
{
	return msb2mask(nz2msb(x) & nz2msb(y));
}

//! mask if( x != 0 && y == 0)
ILINE u32k mask_nz_zr(u32k x, u32k y)
{
	return msb2mask(nz2msb(x) & ~nz2msb(y));
}

//! mask if( x == 0 && y == 0)
ILINE u32k mask_zr_zr(u32k x, u32k y)
{
	return ~nz2mask(x | y);
}

#endif//__BRANCHLESS_MASK__
