#ifndef DRX3D_UNION_FIND_H
#define DRX3D_UNION_FIND_H

#include <drx3D/Maths/Linear/AlignedObjectArray.h>

#define USE_PATH_COMPRESSION 1

///see for discussion of static island optimizations by Vroonsh here: http://code.google.com/p/bullet/issues/detail?id=406
#define STATIC_SIMULATION_ISLAND_OPTIMIZATION 1

struct Element
{
	i32 m_id;
	i32 m_sz;
};

///UnionFind calculates connected subsets
// Implements weighted Quick Union with path compression
// optimization: could use short ints instead of ints (halving memory, would limit the number of rigid bodies to 64k, sounds reasonable)
class UnionFind
{
private:
	AlignedObjectArray<Element> m_elements;

public:
	UnionFind();
	~UnionFind();

	//this is a special operation, destroying the content of UnionFind.
	//it sorts the elements, based on island id, in order to make it easy to iterate over islands
	void sortIslands();

	void reset(i32 N);

	SIMD_FORCE_INLINE i32 getNumElements() const
	{
		return i32(m_elements.size());
	}
	SIMD_FORCE_INLINE bool isRoot(i32 x) const
	{
		return (x == m_elements[x].m_id);
	}

	Element& getElement(i32 index)
	{
		return m_elements[index];
	}
	const Element& getElement(i32 index) const
	{
		return m_elements[index];
	}

	void allocate(i32 N);
	void Free();

	i32 find(i32 p, i32 q)
	{
		return (find(p) == find(q));
	}

	void unite(i32 p, i32 q)
	{
		i32 i = find(p), j = find(q);
		if (i == j)
			return;

#ifndef USE_PATH_COMPRESSION
		//weighted quick union, this keeps the 'trees' balanced, and keeps performance of unite O( log(n) )
		if (m_elements[i].m_sz < m_elements[j].m_sz)
		{
			m_elements[i].m_id = j;
			m_elements[j].m_sz += m_elements[i].m_sz;
		}
		else
		{
			m_elements[j].m_id = i;
			m_elements[i].m_sz += m_elements[j].m_sz;
		}
#else
		m_elements[i].m_id = j;
		m_elements[j].m_sz += m_elements[i].m_sz;
#endif  //USE_PATH_COMPRESSION
	}

	i32 find(i32 x)
	{
		//Assert(x < m_N);
		//Assert(x >= 0);

		while (x != m_elements[x].m_id)
		{
			//not really a reason not to use path compression, and it flattens the trees/improves find performance dramatically

#ifdef USE_PATH_COMPRESSION
			const Element* elementPtr = &m_elements[m_elements[x].m_id];
			m_elements[x].m_id = elementPtr->m_id;
			x = elementPtr->m_id;
#else  //
			x = m_elements[x].m_id;
#endif
			//Assert(x < m_N);
			//Assert(x >= 0);
		}
		return x;
	}
};

#endif  //DRX3D_UNION_FIND_H
