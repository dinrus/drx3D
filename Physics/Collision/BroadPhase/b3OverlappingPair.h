#ifndef D3_OVERLAPPING_PAIR_H
#define D3_OVERLAPPING_PAIR_H

#include <drx3D/Common/shared/b3Int4.h>

#define D3_NEW_PAIR_MARKER -1
#define D3_REMOVED_PAIR_MARKER -2

typedef b3Int4 b3BroadphasePair;

inline b3Int4 b3MakeBroadphasePair(i32 xx, i32 yy)
{
	b3Int4 pair;

	if (xx < yy)
	{
		pair.x = xx;
		pair.y = yy;
	}
	else
	{
		pair.x = yy;
		pair.y = xx;
	}
	pair.z = D3_NEW_PAIR_MARKER;
	pair.w = D3_NEW_PAIR_MARKER;
	return pair;
}

/*struct b3BroadphasePair : public b3Int4
{
	explicit b3BroadphasePair(){}
	
};
*/

class b3BroadphasePairSortPredicate
{
public:
	bool operator()(const b3BroadphasePair& a, const b3BroadphasePair& b) const
	{
		i32k uidA0 = a.x;
		i32k uidB0 = b.x;
		i32k uidA1 = a.y;
		i32k uidB1 = b.y;
		return uidA0 > uidB0 || (uidA0 == uidB0 && uidA1 > uidB1);
	}
};

D3_FORCE_INLINE bool operator==(const b3BroadphasePair& a, const b3BroadphasePair& b)
{
	return (a.x == b.x) && (a.y == b.y);
}

#endif  //D3_OVERLAPPING_PAIR_H
