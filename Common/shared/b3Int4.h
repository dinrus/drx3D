#ifndef D3_INT4_H
#define D3_INT4_H

#ifdef __cplusplus

#include <drx3D/Common/b3Scalar.h>

D3_ATTRIBUTE_ALIGNED16(struct)
b3UnsignedInt4
{
	D3_DECLARE_ALIGNED_ALLOCATOR();

	union {
		struct
		{
			u32 x, y, z, w;
		};
		struct
		{
			u32 s[4];
		};
	};
};

D3_ATTRIBUTE_ALIGNED16(struct)
b3Int4
{
	D3_DECLARE_ALIGNED_ALLOCATOR();

	union {
		struct
		{
			i32 x, y, z, w;
		};
		struct
		{
			i32 s[4];
		};
	};
};

D3_FORCE_INLINE b3Int4 b3MakeInt4(i32 x, i32 y, i32 z, i32 w = 0)
{
	b3Int4 v;
	v.s[0] = x;
	v.s[1] = y;
	v.s[2] = z;
	v.s[3] = w;
	return v;
}

D3_FORCE_INLINE b3UnsignedInt4 b3MakeUnsignedInt4(u32 x, u32 y, u32 z, u32 w = 0)
{
	b3UnsignedInt4 v;
	v.s[0] = x;
	v.s[1] = y;
	v.s[2] = z;
	v.s[3] = w;
	return v;
}

#else

#define b3UnsignedInt4 uint4
#define b3Int4 int4
#define b3MakeInt4 (int4)
#define b3MakeUnsignedInt4 (uint4)

#endif  //__cplusplus

#endif  //D3_INT4_H
