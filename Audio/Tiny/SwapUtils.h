#ifndef D3_SWAP_UTILS_H
#define D3_SWAP_UTILS_H

inline void b3Swap16(u8 *ptr)
{
	u8 val;

	// Swap 1st and 2nd bytes
	val = *(ptr);
	*(ptr) = *(ptr + 1);
	*(ptr + 1) = val;
}

inline void b3Swap32(u8 *ptr)
{
	u8 val;

	// Swap 1st and 4th bytes
	val = *(ptr);
	*(ptr) = *(ptr + 3);
	*(ptr + 3) = val;

	//Swap 2nd and 3rd bytes
	ptr += 1;
	val = *(ptr);
	*(ptr) = *(ptr + 1);
	*(ptr + 1) = val;
}

inline void b3Swap64(u8 *ptr)
{
	u8 val;

	// Swap 1st and 8th bytes
	val = *(ptr);
	*(ptr) = *(ptr + 7);
	*(ptr + 7) = val;

	// Swap 2nd and 7th bytes
	ptr += 1;
	val = *(ptr);
	*(ptr) = *(ptr + 5);
	*(ptr + 5) = val;

	// Swap 3rd and 6th bytes
	ptr += 1;
	val = *(ptr);
	*(ptr) = *(ptr + 3);
	*(ptr + 3) = val;

	// Swap 4th and 5th bytes
	ptr += 1;
	val = *(ptr);
	*(ptr) = *(ptr + 1);
	*(ptr + 1) = val;
}

#endif  //D3_SWAP_UTILS_H
