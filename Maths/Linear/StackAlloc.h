/*
StackAlloc extracted from GJK-EPA collision solver by Nathanael Presson
Nov.2006
*/

#ifndef DRX3D_STACK_ALLOC
#define DRX3D_STACK_ALLOC

#include <drx3D/Maths/Linear/Scalar.h>  //for Assert
#include <drx3D/Maths/Linear/AlignedAllocator.h>

///The Block class is an internal structure for the StackAlloc memory allocator.
struct Block
{
	Block* previous;
	u8* address;
};

///The StackAlloc class provides some fast stack-based memory allocator (LIFO last-in first-out)
class StackAlloc
{
public:
	StackAlloc(u32 size)
	{
		ctor();
		create(size);
	}
	~StackAlloc() { destroy(); }

	inline void create(u32 size)
	{
		destroy();
		data = (u8*)AlignedAlloc(size, 16);
		totalsize = size;
	}
	inline void destroy()
	{
		Assert(usedsize == 0);
		//Raise(L"StackAlloc is still in use");

		if (usedsize == 0)
		{
			if (!ischild && data)
				AlignedFree(data);

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
		Assert(0);
		//&& (L"Not enough memory"));

		return (0);
	}
	SIMD_FORCE_INLINE Block* beginBlock()
	{
		Block* pb = (Block*)allocate(sizeof(Block));
		pb->previous = current;
		pb->address = data + usedsize;
		current = pb;
		return (pb);
	}
	SIMD_FORCE_INLINE void endBlock(Block* block)
	{
		Assert(block == current);
		//Raise(L"Unmatched blocks");
		if (block == current)
		{
			current = block->previous;
			usedsize = (u32)((block->address - data) - sizeof(Block));
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
	Block* current;
	bool ischild;
};

#endif  //DRX3D_STACK_ALLOC
