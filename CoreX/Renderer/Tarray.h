// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/Sys/ILog.h>
#include <drx3D/Sys/IDrxPak.h> //impl of fxopen
#include <drx3D/CoreX/Math/Drx_Math.h>

#ifndef CLAMP
	#define CLAMP(X, mn, mx) ((X) < (mn) ? (mn) : ((X) < (mx) ? (X) : (mx)))
#endif

#ifndef SATURATE
	#define SATURATE(X) clamp_tpl(X, 0.f, 1.f)
#endif

#ifndef SATURATEB
	#define SATURATEB(X) CLAMP(X, 0, 255)
#endif

#ifndef LERP
	#define LERP(A, B, Alpha) ((A) + (Alpha) * ((B)-(A)))
#endif

//! General array class.
//! Can refer to a general (unowned) region of memory (m_nAllocatedCount = 0).
//! Can allocate, grow, and shrink an array.
//! Does not deep copy.
template<class T> class TArray
{
protected:
	T*           m_pElements;
	u32 m_nCount;
	u32 m_nAllocatedCount;

public:
	typedef T value_type;

	//! Empty array.
	TArray()
	{
		MEMSTAT_REGISTER_CONTAINER(this, EMemStatContainerType::MSC_Vector, T);
		ClearArr();
	}

	//! Create a new array, delete it on destruction.
	TArray(i32 Count)
	{
		MEMSTAT_REGISTER_CONTAINER(this, EMemStatContainerType::MSC_Vector, T);
		m_nCount = Count;
		m_nAllocatedCount = Count;
		MEMSTAT_USAGE(begin(), MemSize());
		m_pElements = NULL;
		Realloc(0);
	}
	TArray(i32 Use, i32 Max)
	{
		MEMSTAT_REGISTER_CONTAINER(this, EMemStatContainerType::MSC_Vector, T);
		m_nCount = Use;
		m_nAllocatedCount = Max;
		MEMSTAT_USAGE(begin(), MemSize());
		m_pElements = NULL;
		Realloc(0);
	}

	//! References pre-existing memory. Does not delete it.
	TArray(T* Elems, i32 Count)
	{
		MEMSTAT_REGISTER_CONTAINER(this, EMemStatContainerType::MSC_Vector, T);
		m_pElements = Elems;
		m_nCount = Count;
		m_nAllocatedCount = 0;
		MEMSTAT_BIND_TO_CONTAINER(this, m_pElements);
		MEMSTAT_USAGE(begin(), MemSize());
	}
	~TArray()
	{
		Free();
		MEMSTAT_UNREGISTER_CONTAINER(this);
	}

	void Free()
	{
		m_nCount = 0;
		MEMSTAT_USAGE(begin(), MemSize());
		if (m_nAllocatedCount)
		{
			DrxModuleMemalignFree(m_pElements);
		}
		m_nAllocatedCount = 0;
		m_pElements = NULL;
	}

	void Create(i32 Count)
	{
		MEMSTAT_USAGE(begin(), 0);
		MEMSTAT_UNBIND_FROM_CONTAINER(this, m_pElements);
		m_pElements = NULL;
		m_nCount = Count;
		m_nAllocatedCount = Count;
		Realloc(0);
		Clear();
	}
	void Copy(const TArray<T>& src)
	{
		MEMSTAT_USAGE(begin(), 0);
		MEMSTAT_UNBIND_FROM_CONTAINER(this, m_pElements);
		Free();
		m_pElements = NULL;
		m_nCount = m_nAllocatedCount = src.Num();
		Realloc(0);
		PREFAST_ASSUME(m_pElements); // realloc asserts if it fails - so this is safe
		memcpy(m_pElements, src.m_pElements, src.Num() * sizeof(T));
	}
	void Copy(const T* src, u32 numElems)
	{
		i32 nOffs = m_nCount;
		Grow(numElems);
		memcpy(&m_pElements[nOffs], src, numElems * sizeof(T));
	}
	void Align4Copy(const T* src, u32& numElems)
	{
		i32 nOffs = m_nCount;
		Grow((numElems + 3) & ~3);
		memcpy(&m_pElements[nOffs], src, numElems * sizeof(T));
		if (numElems & 3)
		{
			i32 nSet = 4 - (numElems & 3);
			memset(&m_pElements[nOffs + numElems], 0, nSet);
			numElems += nSet;
		}
	}

	void Realloc(i32 nOldAllocatedCount)
	{
		if (!m_nAllocatedCount)
		{
			MEMSTAT_USAGE(begin(), 0);
			MEMSTAT_UNBIND_FROM_CONTAINER(this, m_pElements);
			m_pElements = NULL;
		}
		else
		{
			PREFAST_SUPPRESS_WARNING(6308) m_pElements = (T*)DrxModuleReallocAlign(m_pElements, m_nAllocatedCount * sizeof(T), alignof(T));
			MEMSTAT_BIND_TO_CONTAINER(this, m_pElements);
			MEMSTAT_USAGE(begin(), MemSize());
			assert(m_pElements);
		}
	}

	void Remove(u32 Index, u32 Count = 1)
	{
		if (Count)
		{
			memmove(m_pElements + Index, m_pElements + (Index + Count), sizeof(T) * (m_nCount - Index - Count));
			m_nCount -= Count;
			MEMSTAT_USAGE(begin(), MemSize());
		}
	}

	void Shrink()
	{
		if (m_nCount == 0 || m_nAllocatedCount == 0)
			return;
		assert(m_nAllocatedCount >= m_nCount);
		if (m_nAllocatedCount != m_nCount)
		{
			i32 nOldAllocatedCount = m_nAllocatedCount;
			m_nAllocatedCount = m_nCount;
			Realloc(nOldAllocatedCount);
		}
	}

	void _Remove(u32 Index, u32 Count)
	{
		assert(Index >= 0);
		assert(Index <= m_nCount);
		assert((Index + Count) <= m_nCount);

		Remove(Index, Count);
	}

	u32 Num(void) const                { return m_nCount; }
	u32 Capacity(void) const           { return m_nAllocatedCount; }
	u32 MemSize(void) const            { return m_nCount * sizeof(T); }
	void         SetNum(u32 n)         { m_nCount = m_nAllocatedCount = n; MEMSTAT_USAGE(begin(), MemSize()); }
	void         SetUse(u32 n)         { m_nCount = n; MEMSTAT_USAGE(begin(), MemSize()); }
	void         Alloc(u32 n)          { i32 nOldAllocatedCount = m_nAllocatedCount; m_nAllocatedCount = n; Realloc(nOldAllocatedCount); }
	void         Reserve(u32 n)        { i32 nOldAllocatedCount = m_nAllocatedCount; SetNum(n); Realloc(nOldAllocatedCount); Clear(); }
	void         ReserveNoClear(u32 n) { i32 nOldAllocatedCount = m_nAllocatedCount; SetNum(n); Realloc(nOldAllocatedCount); }
	void         Expand(u32 n)
	{
		if (n > m_nAllocatedCount)
			ReserveNew(n);
	}
	void ReserveNew(u32 n)
	{
		i32 num = m_nCount;
		if (n > m_nAllocatedCount)
		{
			i32 nOldAllocatedCount = m_nAllocatedCount;
			m_nAllocatedCount = n * 2;
			Realloc(nOldAllocatedCount);
		}
		m_nCount = n;
		MEMSTAT_USAGE(begin(), MemSize());
		memset(&m_pElements[num], 0, sizeof(T) * (m_nCount - num));
	}
	T* Grow(u32 n)
	{
		i32 nStart = m_nCount;
		m_nCount += n;
		MEMSTAT_USAGE(begin(), MemSize());
		if (m_nCount > m_nAllocatedCount)
		{
			i32 nOldAllocatedCount = m_nAllocatedCount;
			m_nAllocatedCount = m_nCount * 2;
			Realloc(nOldAllocatedCount);
		}
		return &m_pElements[nStart];
	}
	T* GrowReset(u32 n)
	{
		i32 num = m_nAllocatedCount;
		T* Obj = AddIndex(n);
		if (num != m_nAllocatedCount)
			memset(&m_pElements[num], 0, sizeof(T) * (m_nAllocatedCount - num));
		return Obj;
	}

	u32* GetNumAddr(void)           { return &m_nCount; }
	T**           GetDataAddr(void)          { return &m_pElements; }

	T*            Data(void) const           { return m_pElements; }
	T&            Get(u32 id) const { return m_pElements[id]; }

	void          Assign(TArray& fa)
	{
		MEMSTAT_USAGE(begin(), 0);
		MEMSTAT_UNBIND_FROM_CONTAINER(this, m_pElements);
		m_pElements = fa.m_pElements;
		m_nCount = fa.m_nCount;
		m_nAllocatedCount = fa.m_nAllocatedCount;
		MEMSTAT_BIND_TO_CONTAINER(this, m_pElements);
		MEMSTAT_USAGE(begin(), MemSize());
	}

	/*const TArray operator=(TArray fa) const
	   {
	   TArray<T> t = TArray(fa.m_nCount,fa.m_nAllocatedCount);
	   for ( i32 i=0; i<fa.Num(); i++ )
	   {
	    t.AddElem(fa[i]);
	   }
	   return t;
	   }*/

	const T&  operator[](u32 i) const { assert(i < m_nCount); return m_pElements[i]; }
	T&        operator[](u32 i)       { assert(i < m_nCount); return m_pElements[i]; }
	T&        operator*()                      { assert(m_nCount > 0); return *m_pElements;   }

	TArray<T> operator()(u32 Start)
	{
		assert(Start < m_nCount);
		return TArray<T>(m_pElements + Start, m_nCount - Start);
	}
	TArray<T> operator()(u32 Start, u32 Count)
	{
		assert(Start < m_nCount);
		assert(Start + Count <= m_nCount);
		return TArray<T>(m_pElements + Start, Count);
	}

	//! For simple types only.
	TArray(const TArray<T>& cTA)
	{
		m_pElements = NULL;
		m_nCount = m_nAllocatedCount = cTA.Num();
		MEMSTAT_USAGE(begin(), MemSize());
		Realloc(0);
		if (m_pElements)
			memcpy(m_pElements, &cTA[0], m_nCount * sizeof(T));
		/*for (u32 i=0; i<cTA.Num(); i++ )
		   {
		   AddElem(cTA[i]);
		   }*/
	}
	inline TArray& operator=(const TArray& cTA)
	{
		Free();
		new(this)TArray(cTA);
		return *this;
	}
	void ClearArr(void)
	{
		m_nCount = 0;
		m_nAllocatedCount = 0;
		MEMSTAT_USAGE(begin(), MemSize());
		MEMSTAT_UNBIND_FROM_CONTAINER(this, m_pElements);
		m_pElements = NULL;
	}

	void Clear(void)
	{
		memset(m_pElements, 0, m_nCount * sizeof(T));
	}

	void AddString(tukk szStr)
	{
		assert(szStr);
		i32 nLen = strlen(szStr) + 1;
		T* pDst = Grow(nLen);
		memcpy(pDst, szStr, nLen);
	}

	void Set(u32 m)
	{
		memset(m_pElements, m, m_nCount * sizeof(T));
	}

	ILINE T* AddIndex(u32 inc)
	{
		u32 nIndex = m_nCount;
		u32 nNewCount = m_nCount + inc;

		if (nNewCount > m_nAllocatedCount)
		{
			i32 nOldAllocatedCount = m_nAllocatedCount;
			m_nAllocatedCount = nNewCount + (nNewCount >> 1) + 10;
			Realloc(nOldAllocatedCount);
		}

		m_nCount = nNewCount;
		MEMSTAT_USAGE(begin(), MemSize());
		return &m_pElements[nIndex];
	}

	T& Insert(u32 nIndex, u32 inc = 1)
	{
		m_nCount += inc;
		MEMSTAT_USAGE(begin(), MemSize());
		if (m_nCount > m_nAllocatedCount)
		{
			i32 nOldAllocatedCount = m_nAllocatedCount;
			m_nAllocatedCount = m_nCount + (m_nCount >> 1) + 32;
			Realloc(nOldAllocatedCount);
		}
		memmove(&m_pElements[nIndex + inc], &m_pElements[nIndex], (m_nCount - inc - nIndex) * sizeof(T));

		return m_pElements[nIndex];
	}

	void AddIndexNoCache(u32 inc)
	{
		m_nCount += inc;
		MEMSTAT_USAGE(begin(), MemSize());
		if (m_nCount > m_nAllocatedCount)
		{
			i32 nOldAllocatedCount = m_nAllocatedCount;
			m_nAllocatedCount = m_nCount;
			Realloc(nOldAllocatedCount);
		}
	}

	void Add(const T& elem) { AddElem(elem); }
	void AddElem(const T& elem)
	{
		u32 m = m_nCount;
		AddIndex(1);
		m_pElements[m] = elem;
	}
	void AddElemNoCache(const T& elem)
	{
		u32 m = m_nCount;
		AddIndexNoCache(1);
		m_pElements[m] = elem;
	}

	i32 Find(const T& p)
	{
		for (u32 i = 0; i < m_nCount; i++)
		{
			if (p == (*this)[i])
				return i;
		}
		return -1;
	}

	void Delete(u32 n) { DelElem(n); }
	void DelElem(u32 n)
	{
		//    memset(&m_pElements[n],0,sizeof(T));
		_Remove(n, 1);
	}

	void Load(tukk file_name)
	{
		Clear();
		FILE* f = fxopen(file_name, "rb");
		if (!f)
			return;

		i32 size = 0;
		fread(&size, 4, 1, f);

		while (!feof(f) && sizeof(T) == size)
		{
			T tmp;
			if (fread(&tmp, 1, sizeof(T), f) == sizeof(T))
				AddElem(tmp);
		}

		fclose(f);
	}

	//! Standard compliance interface.
	//! This is for those who don't want to learn the non standard and thus not very
	//! convenient interface of TArray, but are unlucky enough not to be able to avoid using it.
	void clear()                    { Free(); }
	void resize(u32 nSize) { reserve(nSize); m_nCount = nSize; MEMSTAT_USAGE(begin(), MemSize()); }
	void reserve(u32 nSize)
	{
		if (nSize > m_nAllocatedCount)
			Alloc(nSize);
	}
	unsigned size() const                { return m_nCount; }
	unsigned capacity() const            { return m_nAllocatedCount; }
	bool     empty() const               { return size() == 0; }
	void     push_back(const T& rSample) { Add(rSample); }
	void     pop_back()                  { m_nCount--; MEMSTAT_USAGE(begin(), MemSize()); }
	void     erase(T* pElem)
	{
		i32 n = i32(pElem - m_pElements);
		assert(n >= 0 && n < m_nCount);
		_Remove(n, 1);
	}
	T*       begin()                { return m_pElements; }
	T*       end()                  { return m_pElements + m_nCount; }
	T        last()                 { return m_pElements[m_nCount - 1]; }
	const T* begin() const          { return m_pElements; }
	const T* end() const            { return m_pElements + m_nCount; }

	i32      GetMemoryUsage() const { return (i32)(m_nAllocatedCount * sizeof(T)); }
};

template<class T> inline void Exchange(T& X, T& Y)
{
	const T Tmp = X;
	X = Y;
	Y = Tmp;
}

//! \endcond