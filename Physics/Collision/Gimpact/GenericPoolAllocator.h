#ifndef DRX3D_GENERIC_POOL_ALLOCATOR_H
#define DRX3D_GENERIC_POOL_ALLOCATOR_H

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <drx3D/Maths/Linear/AlignedAllocator.h>

#define DRX3D_UINT_MAX UINT_MAX
#define DRX3D_DEFAULT_MAX_POOLS 16

//! Generic Pool class
class GenericMemoryPool
{
public:
	u8 *m_pool;      //[m_element_size*m_max_element_count];
	size_t *m_free_nodes;       //[m_max_element_count];//! free nodes
	size_t *m_allocated_sizes;  //[m_max_element_count];//! Number of elements allocated per node
	size_t m_allocated_count;
	size_t m_free_nodes_count;

protected:
	size_t m_element_size;
	size_t m_max_element_count;

	size_t allocate_from_free_nodes(size_t num_elements);
	size_t allocate_from_pool(size_t num_elements);

public:
	void init_pool(size_t element_size, size_t element_count);

	void end_pool();

	GenericMemoryPool(size_t element_size, size_t element_count)
	{
		init_pool(element_size, element_count);
	}

	~GenericMemoryPool()
	{
		end_pool();
	}

	inline size_t get_pool_capacity()
	{
		return m_element_size * m_max_element_count;
	}

	inline size_t gem_element_size()
	{
		return m_element_size;
	}

	inline size_t get_max_element_count()
	{
		return m_max_element_count;
	}

	inline size_t get_allocated_count()
	{
		return m_allocated_count;
	}

	inline size_t get_free_positions_count()
	{
		return m_free_nodes_count;
	}

	inline uk get_element_data(size_t element_index)
	{
		return &m_pool[element_index * m_element_size];
	}

	//! Allocates memory in pool
	/*!
	\param size_bytes size in bytes of the buffer
	*/
	uk allocate(size_t size_bytes);

	bool freeMemory(uk pointer);
};

//! Generic Allocator with pools
/*!
General purpose Allocator which can create Memory Pools dynamiacally as needed.
*/
class GenericPoolAllocator
{
protected:
	size_t m_pool_element_size;
	size_t m_pool_element_count;

public:
	GenericMemoryPool *m_pools[DRX3D_DEFAULT_MAX_POOLS];
	size_t m_pool_count;

	inline size_t get_pool_capacity()
	{
		return m_pool_element_size * m_pool_element_count;
	}

protected:
	// creates a pool
	GenericMemoryPool *push_new_pool();

	uk failback_alloc(size_t size_bytes);

	bool failback_free(uk pointer);

public:
	GenericPoolAllocator(size_t pool_element_size, size_t pool_element_count)
	{
		m_pool_count = 0;
		m_pool_element_size = pool_element_size;
		m_pool_element_count = pool_element_count;
	}

	virtual ~GenericPoolAllocator();

	//! Allocates memory in pool
	/*!
	\param size_bytes size in bytes of the buffer
	*/
	uk allocate(size_t size_bytes);

	bool freeMemory(uk pointer);
};

uk PoolAlloc(size_t size);
uk PoolRealloc(uk ptr, size_t oldsize, size_t newsize);
void PoolFree(uk ptr);

#endif
