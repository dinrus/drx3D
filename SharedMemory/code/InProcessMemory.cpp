#include <drx3D/SharedMemory/InProcessMemory.h>
#include <drx3D/Maths/Linear/HashMap.h>

struct InProcessMemoryInternalData
{
	HashMap<HashInt, uk> m_memoryPointers;
};

InProcessMemory::InProcessMemory()
{
	m_data = new InProcessMemoryInternalData;
}

InProcessMemory::~InProcessMemory()
{
	for (i32 i = 0; i < m_data->m_memoryPointers.size(); i++)
	{
		uk * ptrptr = m_data->m_memoryPointers.getAtIndex(i);
		if (ptrptr)
		{
			uk ptr = *ptrptr;
			free(ptr);
		}
	}
	delete m_data;
}

uk InProcessMemory::allocateSharedMemory(i32 key, i32 size, bool allowCreation)
{
	uk * ptrptr = m_data->m_memoryPointers[key];
	if (ptrptr)
	{
		return *ptrptr;
	}

	uk ptr = malloc(size);
	m_data->m_memoryPointers.insert(key, ptr);
	return ptr;
}

void InProcessMemory::releaseSharedMemory(i32 /*key*/, i32 /*size*/)
{
	//we don't release the memory here, but in the destructor instead,
	//so multiple users could 'share' the memory given some key
}