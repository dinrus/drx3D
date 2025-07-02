#include <drx3D/Physics/Collision/Dispatch/UnionFind.h>

UnionFind::~UnionFind()
{
	Free();
}

UnionFind::UnionFind()
{
}

void UnionFind::allocate(i32 N)
{
	m_elements.resize(N);
}
void UnionFind::Free()
{
	m_elements.clear();
}

void UnionFind::reset(i32 N)
{
	allocate(N);

	for (i32 i = 0; i < N; i++)
	{
		m_elements[i].m_id = i;
		m_elements[i].m_sz = 1;
	}
}

class UnionFindElementSortPredicate
{
public:
	bool operator()(const Element& lhs, const Element& rhs) const
	{
		return lhs.m_id < rhs.m_id;
	}
};

///this is a special operation, destroying the content of UnionFind.
///it sorts the elements, based on island id, in order to make it easy to iterate over islands
void UnionFind::sortIslands()
{
	//first store the original body index, and islandId
	i32 numElements = m_elements.size();

	for (i32 i = 0; i < numElements; i++)
	{
		m_elements[i].m_id = find(i);
#ifndef STATIC_SIMULATION_ISLAND_OPTIMIZATION
		m_elements[i].m_sz = i;
#endif  //STATIC_SIMULATION_ISLAND_OPTIMIZATION
	}

	// Sort the vector using predicate and std::sort
	//std::sort(m_elements.begin(), m_elements.end(), UnionFindElementSortPredicate);
	m_elements.quickSort(UnionFindElementSortPredicate());
}
