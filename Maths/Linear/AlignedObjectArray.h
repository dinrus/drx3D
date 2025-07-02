#ifndef DRX3D_OBJECT_ARRAY__
#define DRX3D_OBJECT_ARRAY__

#include <drx3D/Maths/Linear/Scalar.h>  // has definitions like SIMD_FORCE_INLINE
#include <drx3D/Maths/Linear/AlignedAllocator.h>

///If the platform doesn't support placement new, you can disable DRX3D_USE_PLACEMENT_NEW
///then the AlignedObjectArray doesn't support objects with virtual methods, and non-trivial constructors/destructors
///You can enable DRX3D_USE_MEMCPY, then swapping elements in the array will use memcpy instead of operator=
///see discussion here: https://bulletphysics.orgphpBB2/viewtopic.php?t=1231 and
///http://www.continuousphysics.com/drx3D/phpBB2/viewtopic.php?t=1240

#define DRX3D_USE_PLACEMENT_NEW 1
//#define DRX3D_USE_MEMCPY 1 //disable, because it is cumbersome to find out for each platform where memcpy is defined. It can be in <memory.h> or <string.h> or otherwise...
#define DRX3D_ALLOW_ARRAY_COPY_OPERATOR  // enabling this can accidently perform deep copies of data if you are not careful

#ifdef DRX3D_USE_MEMCPY
#include <memory.h>
#include <string.h>
#endif  //DRX3D_USE_MEMCPY

#ifdef DRX3D_USE_PLACEMENT_NEW
#include <new>  //for placement new
#endif          //DRX3D_USE_PLACEMENT_NEW

///The AlignedObjectArray template class uses a subset of the stl::vector interface for its methods
///It is developed to replace stl::vector to avoid portability issues, including STL alignment issues to add SIMD/SSE data
template <typename T>
//template <class T>
class AlignedObjectArray
{
	AlignedAllocator<T, 16> m_allocator;

	i32 m_size;
	i32 m_capacity;
	T* m_data;
	//PCK: added this line
	bool m_ownsMemory;

#ifdef DRX3D_ALLOW_ARRAY_COPY_OPERATOR
public:
	SIMD_FORCE_INLINE AlignedObjectArray<T>& operator=(const AlignedObjectArray<T>& other)
	{
		copyFromArray(other);
		return *this;
	}
#else   //DRX3D_ALLOW_ARRAY_COPY_OPERATOR
private:
	SIMD_FORCE_INLINE AlignedObjectArray<T>& operator=(const AlignedObjectArray<T>& other);
#endif  //DRX3D_ALLOW_ARRAY_COPY_OPERATOR

protected:
	SIMD_FORCE_INLINE i32 allocSize(i32 size)
	{
		return (size ? size * 2 : 1);
	}
	SIMD_FORCE_INLINE void copy(i32 start, i32 end, T* dest) const
	{
		i32 i;
		for (i = start; i < end; ++i)
#ifdef DRX3D_USE_PLACEMENT_NEW
			new (&dest[i]) T(m_data[i]);
#else
			dest[i] = m_data[i];
#endif  //DRX3D_USE_PLACEMENT_NEW
	}

	SIMD_FORCE_INLINE void init()
	{
		//PCK: added this line
		m_ownsMemory = true;
		m_data = 0;
		m_size = 0;
		m_capacity = 0;
	}
	SIMD_FORCE_INLINE void destroy(i32 first, i32 last)
	{
		i32 i;
		for (i = first; i < last; i++)
		{
			m_data[i].~T();
		}
	}

	SIMD_FORCE_INLINE uk allocate(i32 size)
	{
		if (size)
			return m_allocator.allocate(size);
		return 0;
	}

	SIMD_FORCE_INLINE void deallocate()
	{
		if (m_data)
		{
			//PCK: enclosed the deallocation in this block
			if (m_ownsMemory)
			{
				m_allocator.deallocate(m_data);
			}
			m_data = 0;
		}
	}

public:
	AlignedObjectArray()
	{
		init();
	}

	~AlignedObjectArray()
	{
		clear();
	}

	///Generally it is best to avoid using the copy constructor of an AlignedObjectArray, and use a (const) reference to the array instead.
	AlignedObjectArray(const AlignedObjectArray& otherArray)
	{
		init();

		i32 otherSize = otherArray.size();
		resize(otherSize);
		otherArray.copy(0, otherSize, m_data);
	}

	/// return the number of elements in the array
	SIMD_FORCE_INLINE i32 size() const
	{
		return m_size;
	}

	SIMD_FORCE_INLINE const T& at(i32 n) const
	{
		Assert(n >= 0);
		Assert(n < size());
		return m_data[n];
	}

	SIMD_FORCE_INLINE T& at(i32 n)
	{
		Assert(n >= 0);
		Assert(n < size());
		return m_data[n];
	}

	SIMD_FORCE_INLINE const T& operator[](i32 n) const
	{
		Assert(n >= 0);
		Assert(n < size());
		return m_data[n];
	}

	SIMD_FORCE_INLINE T& operator[](i32 n)
	{
		Assert(n >= 0);
		Assert(n < size());
		return m_data[n];
	}

	///clear the array, deallocated memory. Generally it is better to use array.resize(0), to reduce performance overhead of run-time memory (de)allocations.
	SIMD_FORCE_INLINE void clear()
	{
		destroy(0, size());

		deallocate();

		init();
	}

	SIMD_FORCE_INLINE void pop_back()
	{
		Assert(m_size > 0);
		m_size--;
		m_data[m_size].~T();
	}

	///resize changes the number of elements in the array. If the new size is larger, the new elements will be constructed using the optional second argument.
	///when the new number of elements is smaller, the destructor will be called, but memory will not be freed, to reduce performance overhead of run-time memory (de)allocations.
	SIMD_FORCE_INLINE void resizeNoInitialize(i32 newsize)
	{
		if (newsize > size())
		{
			reserve(newsize);
		}
		m_size = newsize;
	}

	SIMD_FORCE_INLINE void resize(i32 newsize, const T& fillData = T())
	{
		i32k curSize = size();

		if (newsize < curSize)
		{
			for (i32 i = newsize; i < curSize; i++)
			{
				m_data[i].~T();
			}
		}
		else
		{
			if (newsize > curSize)
			{
				reserve(newsize);
			}
#ifdef DRX3D_USE_PLACEMENT_NEW
			for (i32 i = curSize; i < newsize; i++)
			{
				new (&m_data[i]) T(fillData);
			}
#endif  //DRX3D_USE_PLACEMENT_NEW
		}

		m_size = newsize;
	}
	SIMD_FORCE_INLINE T& expandNonInitializing()
	{
		i32k sz = size();
		if (sz == capacity())
		{
			reserve(allocSize(size()));
		}
		m_size++;

		return m_data[sz];
	}

	SIMD_FORCE_INLINE T& expand(const T& fillValue = T())
	{
		i32k sz = size();
		if (sz == capacity())
		{
			reserve(allocSize(size()));
		}
		m_size++;
#ifdef DRX3D_USE_PLACEMENT_NEW
		new (&m_data[sz]) T(fillValue);  //use the in-place new (not really allocating heap memory)
#endif

		return m_data[sz];
	}

	SIMD_FORCE_INLINE void push_back(const T& _Val)
	{
		i32k sz = size();
		if (sz == capacity())
		{
			reserve(allocSize(size()));
		}

#ifdef DRX3D_USE_PLACEMENT_NEW
		new (&m_data[m_size]) T(_Val);
#else
		m_data[size()] = _Val;
#endif  //DRX3D_USE_PLACEMENT_NEW

		m_size++;
	}

	/// return the pre-allocated (reserved) elements, this is at least as large as the total number of elements,see size() and reserve()
	SIMD_FORCE_INLINE i32 capacity() const
	{
		return m_capacity;
	}

	SIMD_FORCE_INLINE void reserve(i32 _Count)
	{  // determine new minimum length of allocated storage
		if (capacity() < _Count)
		{  // not enough room, reallocate
			T* s = (T*)allocate(_Count);

			copy(0, size(), s);

			destroy(0, size());

			deallocate();

			//PCK: added this line
			m_ownsMemory = true;

			m_data = s;

			m_capacity = _Count;
		}
	}

	class less
	{
	public:
		bool operator()(const T& a, const T& b) const
		{
			return (a < b);
		}
	};

	template <typename L>
	void quickSortInternal(const L& CompareFunc, i32 lo, i32 hi)
	{
		//  lo is the lower index, hi is the upper index
		//  of the region of array a that is to be sorted
		i32 i = lo, j = hi;
		T x = m_data[(lo + hi) / 2];

		//  partition
		do
		{
			while (CompareFunc(m_data[i], x))
				i++;
			while (CompareFunc(x, m_data[j]))
				j--;
			if (i <= j)
			{
				swap(i, j);
				i++;
				j--;
			}
		} while (i <= j);

		//  recursion
		if (lo < j)
			quickSortInternal(CompareFunc, lo, j);
		if (i < hi)
			quickSortInternal(CompareFunc, i, hi);
	}

	template <typename L>
	void quickSort(const L& CompareFunc)
	{
		//don't sort 0 or 1 elements
		if (size() > 1)
		{
			quickSortInternal(CompareFunc, 0, size() - 1);
		}
	}

	///heap sort from http://www.csse.monash.edu.au/~lloyd/tildeAlgDS/Sort/Heap/
	template <typename L>
	void downHeap(T* pArr, i32 k, i32 n, const L& CompareFunc)
	{
		/*  PRE: a[k+1..N] is a heap */
		/* POST:  a[k..N]  is a heap */

		T temp = pArr[k - 1];
		/* k has child(s) */
		while (k <= n / 2)
		{
			i32 child = 2 * k;

			if ((child < n) && CompareFunc(pArr[child - 1], pArr[child]))
			{
				child++;
			}
			/* pick larger child */
			if (CompareFunc(temp, pArr[child - 1]))
			{
				/* move child up */
				pArr[k - 1] = pArr[child - 1];
				k = child;
			}
			else
			{
				break;
			}
		}
		pArr[k - 1] = temp;
	} /*downHeap*/

	void swap(i32 index0, i32 index1)
	{
#ifdef DRX3D_USE_MEMCPY
		char temp[sizeof(T)];
		memcpy(temp, &m_data[index0], sizeof(T));
		memcpy(&m_data[index0], &m_data[index1], sizeof(T));
		memcpy(&m_data[index1], temp, sizeof(T));
#else
		T temp = m_data[index0];
		m_data[index0] = m_data[index1];
		m_data[index1] = temp;
#endif  //DRX3D_USE_PLACEMENT_NEW
	}

	template <typename L>
	void heapSort(const L& CompareFunc)
	{
		/* sort a[0..N-1],  N.B. 0 to N-1 */
		i32 k;
		i32 n = m_size;
		for (k = n / 2; k > 0; k--)
		{
			downHeap(m_data, k, n, CompareFunc);
		}

		/* a[1..N] is now a heap */
		while (n >= 1)
		{
			swap(0, n - 1); /* largest of a[0..n-1] */

			n = n - 1;
			/* restore a[1..i-1] heap */
			downHeap(m_data, 1, n, CompareFunc);
		}
	}

	///non-recursive binary search, assumes sorted array
	i32 findBinarySearch(const T& key) const
	{
		i32 first = 0;
		i32 last = size() - 1;

		//assume sorted array
		while (first <= last)
		{
			i32 mid = (first + last) / 2;  // compute mid point.
			if (key > m_data[mid])
				first = mid + 1;  // repeat search in top half.
			else if (key < m_data[mid])
				last = mid - 1;  // repeat search in bottom half.
			else
				return mid;  // found it. return position /////
		}
		return size();  // failed to find key
	}

	i32 findLinearSearch(const T& key) const
	{
		i32 index = size();
		i32 i;

		for (i = 0; i < size(); i++)
		{
			if (m_data[i] == key)
			{
				index = i;
				break;
			}
		}
		return index;
	}

	// If the key is not in the array, return -1 instead of 0,
	// since 0 also means the first element in the array.
	i32 findLinearSearch2(const T& key) const
	{
		i32 index = -1;
		i32 i;

		for (i = 0; i < size(); i++)
		{
			if (m_data[i] == key)
			{
				index = i;
				break;
			}
		}
		return index;
	}

	void removeAtIndex(i32 index)
	{
		if (index < size())
		{
			swap(index, size() - 1);
			pop_back();
		}
	}
	void remove(const T& key)
	{
		i32 findIndex = findLinearSearch(key);
		removeAtIndex(findIndex);
	}

	//PCK: whole function
	void initializeFromBuffer(uk buffer, i32 size, i32 capacity)
	{
		clear();
		m_ownsMemory = false;
		m_data = (T*)buffer;
		m_size = size;
		m_capacity = capacity;
	}

	void copyFromArray(const AlignedObjectArray& otherArray)
	{
		i32 otherSize = otherArray.size();
		resize(otherSize);
		otherArray.copy(0, otherSize, m_data);
	}
};

#endif  //DRX3D_OBJECT_ARRAY__
