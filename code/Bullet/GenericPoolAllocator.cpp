#include <drx3D/Physics/Collision/Gimpact/GenericPoolAllocator.h>

/// *************** GenericMemoryPool ******************///////////

size_t GenericMemoryPool::allocate_from_free_nodes(size_t num_elements)
{
	size_t ptr = DRX3D_UINT_MAX;

	if (m_free_nodes_count == 0) return DRX3D_UINT_MAX;
	// find an avaliable free node with the correct size
	size_t revindex = m_free_nodes_count;

	while (revindex-- && ptr == DRX3D_UINT_MAX)
	{
		if (m_allocated_sizes[m_free_nodes[revindex]] >= num_elements)
		{
			ptr = revindex;
		}
	}
	if (ptr == DRX3D_UINT_MAX) return DRX3D_UINT_MAX;  // not found

	revindex = ptr;
	ptr = m_free_nodes[revindex];
	// post: ptr contains the node index, and revindex the index in m_free_nodes

	size_t finalsize = m_allocated_sizes[ptr];
	finalsize -= num_elements;

	m_allocated_sizes[ptr] = num_elements;

	// post: finalsize>=0, m_allocated_sizes[ptr] has the requested size

	if (finalsize > 0)  // preserve free node, there are some free memory
	{
		m_free_nodes[revindex] = ptr + num_elements;
		m_allocated_sizes[ptr + num_elements] = finalsize;
	}
	else  // delete free node
	{
		// swap with end
		m_free_nodes[revindex] = m_free_nodes[m_free_nodes_count - 1];
		m_free_nodes_count--;
	}

	return ptr;
}

size_t GenericMemoryPool::allocate_from_pool(size_t num_elements)
{
	if (m_allocated_count + num_elements > m_max_element_count) return DRX3D_UINT_MAX;

	size_t ptr = m_allocated_count;

	m_allocated_sizes[m_allocated_count] = num_elements;
	m_allocated_count += num_elements;

	return ptr;
}

void GenericMemoryPool::init_pool(size_t element_size, size_t element_count)
{
	m_allocated_count = 0;
	m_free_nodes_count = 0;

	m_element_size = element_size;
	m_max_element_count = element_count;

	m_pool = (u8*)AlignedAlloc(m_element_size * m_max_element_count, 16);
	m_free_nodes = (size_t *)AlignedAlloc(sizeof(size_t) * m_max_element_count, 16);
	m_allocated_sizes = (size_t *)AlignedAlloc(sizeof(size_t) * m_max_element_count, 16);

	for (size_t i = 0; i < m_max_element_count; i++)
	{
		m_allocated_sizes[i] = 0;
	}
}

void GenericMemoryPool::end_pool()
{
	AlignedFree(m_pool);
	AlignedFree(m_free_nodes);
	AlignedFree(m_allocated_sizes);
	m_allocated_count = 0;
	m_free_nodes_count = 0;
}

//! Allocates memory in pool
/*!
\param size_bytes size in bytes of the buffer
*/
uk GenericMemoryPool::allocate(size_t size_bytes)
{
	size_t module = size_bytes % m_element_size;
	size_t element_count = size_bytes / m_element_size;
	if (module > 0) element_count++;

	size_t alloc_pos = allocate_from_free_nodes(element_count);
	// a free node is found
	if (alloc_pos != DRX3D_UINT_MAX)
	{
		return get_element_data(alloc_pos);
	}
	// allocate directly on pool
	alloc_pos = allocate_from_pool(element_count);

	if (alloc_pos == DRX3D_UINT_MAX) return NULL;  // not space
	return get_element_data(alloc_pos);
}

bool GenericMemoryPool::freeMemory(uk pointer)
{
	u8 *pointer_pos = (u8*)pointer;
	u8 *pool_pos = (u8*)m_pool;
	// calc offset
	if (pointer_pos < pool_pos) return false;  //other pool
	size_t offset = size_t(pointer_pos - pool_pos);
	if (offset >= get_pool_capacity()) return false;  // far away

	// find free position
	m_free_nodes[m_free_nodes_count] = offset / m_element_size;
	m_free_nodes_count++;
	return true;
}

/// *******************! GenericPoolAllocator *******************!///

GenericPoolAllocator::~GenericPoolAllocator()
{
	// destroy pools
	size_t i;
	for (i = 0; i < m_pool_count; i++)
	{
		m_pools[i]->end_pool();
		AlignedFree(m_pools[i]);
	}
}

// creates a pool
GenericMemoryPool *GenericPoolAllocator::push_new_pool()
{
	if (m_pool_count >= DRX3D_DEFAULT_MAX_POOLS) return NULL;

	GenericMemoryPool *newptr = (GenericMemoryPool *)AlignedAlloc(sizeof(GenericMemoryPool), 16);

	m_pools[m_pool_count] = newptr;

	m_pools[m_pool_count]->init_pool(m_pool_element_size, m_pool_element_count);

	m_pool_count++;
	return newptr;
}

uk GenericPoolAllocator::failback_alloc(size_t size_bytes)
{
	GenericMemoryPool *pool = NULL;

	if (size_bytes <= get_pool_capacity())
	{
		pool = push_new_pool();
	}

	if (pool == NULL)  // failback
	{
		return AlignedAlloc(size_bytes, 16);
	}

	return pool->allocate(size_bytes);
}

bool GenericPoolAllocator::failback_free(uk pointer)
{
	AlignedFree(pointer);
	return true;
}

//! Allocates memory in pool
/*!
\param size_bytes size in bytes of the buffer
*/
uk GenericPoolAllocator::allocate(size_t size_bytes)
{
	uk ptr = NULL;

	size_t i = 0;
	while (i < m_pool_count && ptr == NULL)
	{
		ptr = m_pools[i]->allocate(size_bytes);
		++i;
	}

	if (ptr) return ptr;

	return failback_alloc(size_bytes);
}

bool GenericPoolAllocator::freeMemory(uk pointer)
{
	bool result = false;

	size_t i = 0;
	while (i < m_pool_count && result == false)
	{
		result = m_pools[i]->freeMemory(pointer);
		++i;
	}

	if (result) return true;

	return failback_free(pointer);
}

/// ************** STANDARD ALLOCATOR ***************************///

#define DRX3D_DEFAULT_POOL_SIZE 32768
#define DRX3D_DEFAULT_POOL_ELEMENT_SIZE 8

// main allocator
class GIM_STANDARD_ALLOCATOR : public GenericPoolAllocator
{
public:
	GIM_STANDARD_ALLOCATOR() : GenericPoolAllocator(DRX3D_DEFAULT_POOL_ELEMENT_SIZE, DRX3D_DEFAULT_POOL_SIZE)
	{
	}
};

// global allocator
GIM_STANDARD_ALLOCATOR g_main_allocator;

uk PoolAlloc(size_t size)
{
	return g_main_allocator.allocate(size);
}

uk PoolRealloc(uk ptr, size_t oldsize, size_t newsize)
{
	uk newptr = PoolAlloc(newsize);
	size_t copysize = oldsize < newsize ? oldsize : newsize;
	memcpy(newptr, ptr, copysize);
	PoolFree(ptr);
	return newptr;
}

void PoolFree(uk ptr)
{
	g_main_allocator.freeMemory(ptr);
}
