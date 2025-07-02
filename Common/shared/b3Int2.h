#ifndef D3_INT2_H
#define D3_INT2_H

#include <drxtypes.h>

#ifdef __cplusplus

struct b3UnsignedInt2
{
	union {
		struct
		{
			u32 x, y;
		};
		struct
		{
			u32 s[2];
		};
	};
};

struct b3Int2
{
	union {
		struct
		{
			i32 x, y;
		};
		struct
		{
			i32 s[2];
		};
	};
};

inline b3Int2 b3MakeInt2(i32 x, i32 y)
{
	b3Int2 v;
	v.s[0] = x;
	v.s[1] = y;
	return v;
}
#else

#define b3UnsignedInt2 uint2
#define b3Int2 int2
#define b3MakeInt2 (int2)

#endif  //__cplusplus
#endif