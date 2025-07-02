// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

//! POD Array.
//! Vector-like class (O(1) random access) without construction/destructor/copy constructor/assignment handling.
//! Over-allocation allows safe prefetching where required without worrying about memory page boundaries.
template<class T, size_t overAllocBytes = 0>
class PodArray
{
	T*  m_pElements;
	i32 m_nCount;
	i32 m_nAllocatedCount;

public:
	typedef T        value_type;
	typedef T*       iterator;
	typedef const T* const_iterator;

	//////////////////////////////////////////////////////////////////////////
	// STL compatible interface
	//////////////////////////////////////////////////////////////////////////
	void resize(size_t numElements)
	{
		i32 nOldCount = m_nCount;
		PreAllocate((i32)numElements, (i32)numElements);
		assert(numElements == size());
		for (i32 nElement = nOldCount; nElement < m_nCount; ++nElement)
			new(&(*this)[nElement])T;
		MEMSTAT_USAGE(begin(), sizeof(T) * size());
	}
	//////////////////////////////////////////////////////////////////////////
	ILINE void     reserve(unsigned numElements) { PreAllocate((i32)numElements); }
	ILINE void     push_back(const T& rElement)  { Add(rElement); }
	ILINE size_t   size() const                  { return (size_t)m_nCount; }
	ILINE size_t   capacity() const              { return (size_t)m_nAllocatedCount; }

	ILINE void     clear()                       { Clear(); }
	ILINE T*       begin()                       { return m_pElements; }
	ILINE T*       end()                         { return m_pElements + m_nCount; }
	ILINE const T* begin() const                 { return m_pElements; }
	ILINE const T* end() const                   { return m_pElements + m_nCount; }
	ILINE bool     empty() const                 { return m_nCount == 0; }

	ILINE const T& front() const                 { assert(m_nCount > 0); return m_pElements[0]; }
	ILINE T&       front()                       { assert(m_nCount > 0); return m_pElements[0]; }

	ILINE const T& back() const                  { assert(m_nCount > 0); return m_pElements[m_nCount - 1]; }
	ILINE T&       back()                        { assert(m_nCount > 0); return m_pElements[m_nCount - 1]; }
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	PodArray() : m_pElements(0),  m_nCount(0), m_nAllocatedCount(0)
	{
		MEMSTAT_REGISTER_CONTAINER(this, EMemStatContainerType::MSC_Vector, T);
	}
	PodArray(i32 elem_count, i32 nNewCount = 0) : m_nCount(0), m_pElements(0), m_nAllocatedCount(0)
	{
		MEMSTAT_REGISTER_CONTAINER(this, EMemStatContainerType::MSC_Vector, T);
		PreAllocate(elem_count, nNewCount);
	}
	PodArray(const PodArray<T>& from) : m_nCount(0), m_pElements(0), m_nAllocatedCount(0)
	{
		MEMSTAT_REGISTER_CONTAINER(this, EMemStatContainerType::MSC_Vector, T);
		AddList(from);
	}
	~PodArray()
	{
		Free();
		MEMSTAT_UNREGISTER_CONTAINER(this);
	}

	void Reset() { Free(); }
	void Free()
	{
		if (m_pElements)
		{
			size_t nOldSizeInBytes = (m_nAllocatedCount * sizeof(T)) + overAllocBytes;
			InternalRealloc(m_pElements, 0, nOldSizeInBytes);
		}
		m_pElements = 0;
		m_nCount = 0;
		m_nAllocatedCount = 0;
	}

	ILINE void Clear()
	{
		m_nCount = 0;
		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
	}

	i32 Find(const T& p)
	{
		for (i32 i = 0; i < m_nCount; i++)
			if (p == (*this)[i])
				return i;

		return -1;
		;
	}

	i32 FindReverse(const T& p)
	{
		for (i32 i = m_nCount - 1; i >= 0; i--)
			if (p == (*this)[i])
				return i;

		return -1;
		;
	}

	inline void AddList(const PodArray<T>& lstAnother)
	{
		if (m_nCount + lstAnother.Count() > m_nAllocatedCount)
			PreAllocate((m_nCount + lstAnother.Count()) * 3 / 2 + 8);

		memcpy(&m_pElements[m_nCount], &lstAnother.m_pElements[0], sizeof(m_pElements[0]) * lstAnother.Count());

		m_nCount += lstAnother.Count();
		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
	}

	inline void AddList(T* pAnotherArray, i32 nAnotherCount)
	{
		if (m_nCount + nAnotherCount > m_nAllocatedCount)
			PreAllocate((m_nCount + nAnotherCount) * 3 / 2 + 8);

		memcpy(&m_pElements[m_nCount], pAnotherArray, sizeof(m_pElements[0]) * nAnotherCount);

		m_nCount += nAnotherCount;
		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
	}

	ILINE void Add(const T& p)
	{
		if (m_nCount >= m_nAllocatedCount)
		{
			assert(&p < m_pElements || &p >= (m_pElements + m_nAllocatedCount));
			size_t nOldSizeInBytes = (m_nAllocatedCount * sizeof(T)) + overAllocBytes;
			m_nAllocatedCount = m_nCount * 2 + 8;

			//keep alignment requirement of T
			m_pElements = InternalRealloc(m_pElements, (m_nAllocatedCount * sizeof(T)) + overAllocBytes, nOldSizeInBytes);
			assert(m_pElements != NULL);

			MEMSTAT_BIND_TO_CONTAINER(this, m_pElements);
		}

		memcpy(&m_pElements[m_nCount], &p, sizeof(m_pElements[m_nCount]));
		m_nCount++;
		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
	}

	ILINE T& AddNew()
	{
		if (m_nCount >= m_nAllocatedCount)
		{
			size_t nOldSizeInBytes = (m_nAllocatedCount * sizeof(T)) + overAllocBytes;
			m_nAllocatedCount = m_nCount * 2 + 8;
			m_pElements = InternalRealloc(m_pElements, (m_nAllocatedCount * sizeof(T)) + overAllocBytes, nOldSizeInBytes);
			assert(m_pElements != NULL);
			MEMSTAT_BIND_TO_CONTAINER(this, m_pElements);
		}

		m_nCount++;

		memset(&Last(), 0, sizeof(T));

		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
		return Last();
	}

	void InsertBefore(const T& p, u32k nBefore)
	{
		assert(nBefore >= 0 && nBefore <= (u32)m_nCount);
		T tmp;
		Add(tmp);        // add empty object to increase memory buffer
		memmove(&(m_pElements[nBefore + 1]), &(m_pElements[nBefore]), sizeof(T) * (m_nCount - nBefore - 1));
		m_pElements[nBefore] = p;
	}

	void CheckAllocated(i32 elem_count)
	{
		i32 nElemNum = (m_nCount > elem_count) ? m_nCount : elem_count;
		PreAllocate(nElemNum, nElemNum);
	}

	void PreAllocate(i32 elem_count, i32 nNewCount = -1)
	{
		if (elem_count > m_nAllocatedCount)
		{
			i32 nPrevAllocCount = m_nAllocatedCount;
			i32 nAddedCount = elem_count - m_nAllocatedCount;
			size_t nOldSizeInBytes = (m_nAllocatedCount * sizeof(T)) + overAllocBytes;

			m_nAllocatedCount = elem_count;

			m_pElements = InternalRealloc(m_pElements, (m_nAllocatedCount * sizeof(T)) + overAllocBytes, nOldSizeInBytes);
			assert(m_pElements);
			memset(m_pElements + nPrevAllocCount, 0, (sizeof(T) * nAddedCount) + overAllocBytes);

			MEMSTAT_BIND_TO_CONTAINER(this, m_pElements);
		}

		if (nNewCount >= 0)
			m_nCount = nNewCount;
		MEMSTAT_USAGE(begin(), sizeof(T) * size());
	}

	inline void Delete(i32k nElemId, i32k nElemCount = 1)
	{
		assert(nElemId >= 0 && nElemId + nElemCount <= m_nCount);
		memmove(&(m_pElements[nElemId]), &(m_pElements[nElemId + nElemCount]), sizeof(T) * (m_nCount - nElemId - nElemCount));
		m_nCount -= nElemCount;
		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
	}

	inline void DeleteFastUnsorted(i32k nElemId, i32k nElemCount = 1)
	{
		assert(nElemId >= 0 && nElemId + nElemCount <= m_nCount);
		memmove(&(m_pElements[nElemId]), &(m_pElements[m_nCount - nElemCount]), sizeof(T) * nElemCount);
		m_nCount -= nElemCount;
		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
	}

	inline bool Delete(const T& del)
	{
		bool bFound = false;
		for (i32 i = 0; i < Count(); i++)
			if (m_pElements[i] == del)
			{
				Delete(i);
				i--;
				bFound = true;
			}
		return bFound;
	}

	ILINE i32          Count() const           { return m_nCount; }
	ILINE u32 Size() const            { return m_nCount; }

	ILINE i32          IsEmpty() const         { return m_nCount == 0; }

	ILINE T&           operator[](i32 i) const { assert(i >= 0 && i < m_nCount); return m_pElements[i]; }
	ILINE T&           GetAt(i32 i) const      { assert(i >= 0 && i < m_nCount); return m_pElements[i]; }
	ILINE T*           Get(i32 i) const        { assert(i >= 0 && i < m_nCount); return &m_pElements[i]; }
	ILINE T*           GetElements()           { return m_pElements; }
	ILINE const T*     GetElements() const     { return m_pElements; }
	ILINE u32 GetDataSize() const     { return m_nCount * sizeof(T); }

	T&                 Last() const            { assert(m_nCount); return m_pElements[m_nCount - 1]; }
	ILINE void         DeleteLast()
	{
		assert(m_nCount);
		m_nCount--;
		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
	}

	PodArray<T>& operator=(const PodArray<T>& source_list)
	{
		Clear();
		AddList(source_list);

		return *this;
	}

	//! \return true if arrays have the same data.
	bool Compare(const PodArray<T>& l) const
	{
		if (Count() != l.Count())
			return 0;

		if (memcmp(m_pElements, l.m_pElements, m_nCount * sizeof(T)) != 0)
			return 0;

		return 1;
	}

	//! For statistics.
	ILINE size_t ComputeSizeInMemory() const
	{
		return (sizeof(*this) + sizeof(T) * m_nAllocatedCount) + overAllocBytes;
	}

private:
	T* InternalRealloc(T* pBuffer, size_t nNewSize, size_t nOldSize)
	{
		PREFAST_SUPPRESS_WARNING(6326)
		if (alignof(T) > _DRX_DEFAULT_MALLOC_ALIGNMENT)
		{
			return (T*)DrxModuleReallocAlign(pBuffer, nNewSize, alignof(T));
		}
		else
		{
			return (T*)DrxModuleRealloc(pBuffer, nNewSize);
		}
	}

};

//! \endcond