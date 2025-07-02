#ifndef D3_STACK_ALLOC
#define D3_STACK_ALLOC

#include <drx3D/Common/b3Scalar.h>  //for drx3DAssert
#include <drx3D/Common/b3AlignedAllocator.h>

///The b3Block class is an internal structure for the b3StackAlloc memory allocator.
struct b3Block
{
	b3Block* previous;
	u8* address;
};

///The StackAlloc class provides some fast stack-based memory allocator (LIFO last-in first-out)
class b3StackAlloc
{
public:
	b3StackAlloc(u32 size)
	{
		ctor();
		create(size);
	}
	~b3StackAlloc() { destroy(); }

	inline void create(u32 size)
	{
		destroy();
		data = (u8*)b3AlignedAlloc(size, 16);
		totalsize = size;
	}
	inline void destroy()
	{
		drx3DAssert(usedsize == 0);
		//Raise(L"StackAlloc is still in use");

		if (usedsize == 0)
		{
			if (!ischild && data)
				b3AlignedFree(data);

			data = 0;
			usedsize = 0;
		}
	}

	i32 getAvailableMemory() const
	{
		return static_cast<i32>(totalsize - usedsize);
	}

	u8* allocate(u32 size)
	{
		u32k nus(usedsize + size);
		if (nus < totalsize)
		{
			usedsize = nus;
			return (data + (usedsize - size));
		}
		drx3DAssert(0);
		//&& (L"Not enough memory"));

		return (0);
	}
	D3_FORCE_INLINE b3Block* beginBlock()
	{
		b3Block* pb = (b3Block*)allocate(sizeof(b3Block));
		pb->previous = current;
		pb->address = data + usedsize;
		current = pb;
		return (pb);
	}
	D3_FORCE_INLINE void endBlock(b3Block* block)
	{
		drx3DAssert(block == current);
		//Raise(L"Unmatched blocks");
		if (block == current)
		{
			current = block->previous;
			usedsize = (u32)((block->address - data) - sizeof(b3Block));
		}
	}

private:
	void ctor()
	{
		data = 0;
		totalsize = 0;
		usedsize = 0;
		current = 0;
		ischild = false;
	}
	u8* data;
	u32 totalsize;
	u32 usedsize;
	b3Block* current;
	bool ischild;
};

#endif  //D3_STACK_ALLOC
