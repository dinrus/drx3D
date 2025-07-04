// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _DRX_PARTICLE_LIST_H_
#define _DRX_PARTICLE_LIST_H_
#pragma once

#include <drx3D/Eng3D/ParticleMemory.h>

class CParticle;

////////////////////////////////////////////////////////////////////////
// Двунаправленный список.

template<class T>
class ParticleList
{

public:

	typedef T value_type;

	// Производный тип с интрузивными ссылками после T, для сохранения раскладки.
	struct Node : T
	{
		Node* pNext;
		Node* pPrev;
	};

	ParticleList()
                             { reset(); }

	~ParticleList()
	                         { clear(); }

	bool   operator!() const
                             { return m_pHead == NULL; }
	bool   empty() const
                             { return m_pHead == NULL; }
	size_t size() const
	                         { return m_nSize; }

	//CONST_VAR_FUNCTION
	inline uk front()  { assert(!empty()); return getNode(m_pHead); }
	
	inline ukk front() const  { assert(!empty()); return getNode(m_pHead); }
	
	//CONST_VAR_FUNCTION
	inline uk back() { assert(!empty()); return getNode(m_pTail); }
	
	inline ukk back() const { assert(!empty());  return getNode(m_pTail); }
	//
	// Обход.
	//
	template<typename CVT, bool bReverse = false>
	struct cv_iterator
	{
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef ptrdiff_t                       difference_type;
		typedef ptrdiff_t                       distance_type;
		typedef T                               value_type;

		cv_iterator(Node* p = NULL)
			: m_pNode(p)     {}

		operator bool() const
		                     { return !!m_pNode; }

		CVT& operator*() const
	                         { assert(m_pNode); return *m_pNode; }
		CVT* operator->() const
		                     { assert(m_pNode); return m_pNode; }
		operator CVT*() const
		                     { assert(m_pNode); return m_pNode; }

		void operator++()
		{
			if (bReverse)
				m_pNode = m_pNode->pPrev;
			else
				m_pNode = m_pNode->pNext;
		}

		bool operator==(cv_iterator o) const
		                     { return m_pNode == o.m_pNode; }
		bool operator!=(cv_iterator o) const
		                     { return m_pNode != o.m_pNode; }

	protected:
		Node* m_pNode;
	   
	};

	// У неконстантного итератора кэшированная инкрементация, позволяющая безопасно "затирать"
	// элементы во время итерации.
	template<bool bReverse = false>
	struct v_iterator : cv_iterator<T, bReverse>
	{
		typedef cv_iterator<T, bReverse> base;
		using base::m_pNode;

		v_iterator(Node* p = NULL)
			: base(p)
		{
			set_next();
		}

		void operator++()
		{
			m_pNode = m_pNext;
			set_next();
		}

	protected:
		Node* m_pNext;

		void set_next()
		{
			if (m_pNode)
			{
				if (bReverse)
					m_pNext = m_pNode->pPrev;
				else
					m_pNext = m_pNode->pNext;
			}
		}
	};

	typedef cv_iterator<const T, false> const_iterator;
	typedef cv_iterator<const T, true>  const_reverse_iterator;

	const_iterator begin() const
	                               { return m_pHead; }
	const_iterator end() const
	                               { return NULL; }

	const_reverse_iterator rbegin() const
	                               { return m_pTail; }
	const_reverse_iterator rend() const
	                               { return NULL; }

	typedef v_iterator<false> iterator;
	typedef v_iterator<true>  reverse_iterator;

	iterator begin()
	                      { return m_pHead; }
	iterator end()
	                      { return NULL; }

	reverse_iterator rbegin()
	                      { return m_pTail; }
	reverse_iterator rend()
	                      { return NULL; }

	template<class IT>
	struct range_t
	{
		range_t(IT b)
			: _begin(b) {}

		IT        begin() const
		{ return _begin; }
		static IT end()
		{ return NULL; }

	protected:
		IT _begin;
	};

	range_t<const_reverse_iterator> reversed() const
	{ return rbegin(); }
	range_t<reverse_iterator>       reversed()
	{ return rbegin(); }

	//
	// Добавить элементы.
	//
	uk push_back_new() DRX_FUNCTION_CONTAINS_UNDEFINED_BEHAVIOR
	{
		uk pNode = allocate();
		if (pNode)
		{
#if DRX_UBSAN
			memset(pNode, 0, sizeof(Node));
#endif
			insert(NULL, static_cast<Node*>(pNode)); // UBSAN: Invalid cast
		}
		return pNode;
	}
	T* push_back()
	{
		T* pNode = ::new(allocate())T();
		if (pNode)
		{
			insert(NULL, get_node(pNode));
		}
		return pNode;
	}
	T* push_back(const T& obj)
	{
		T* pNode = ::new(allocate())T(obj);
		if (pNode)
		{
			insert(NULL, get_node(pNode));
		}
		return pNode;
	}

	uk push_front_new() DRX_FUNCTION_CONTAINS_UNDEFINED_BEHAVIOR
	{
		uk pNode = allocate();
		if (pNode)
		{
#if DRX_UBSAN
			memset(pNode, 0, sizeof(Node));
#endif
			insert(m_pHead, static_cast<Node*>(pNode)); // UBSAN: Invalid cast
		}
		return pNode;
	}
	T* push_front()
	{
		T* pNode = ::new(allocate())T();
		if (pNode)
		{
			insert(m_pHead, get_node(pNode));
		}
		return pNode;
	}
	T* push_front(const T& obj)
	{
		T* pNode = ::new(allocate())T(obj);
		if (pNode)
		{
			insert(m_pHead, get_node(pNode));
		}
		return pNode;
	}

	uk insert_new(const T* pNext) DRX_FUNCTION_CONTAINS_UNDEFINED_BEHAVIOR
	{
		uk pNode = allocate();
		if (pNode)
		{
#if DRX_UBSAN
			memset(pNode, 0, sizeof(Node));
#endif
			insert(get_node(pNext), static_cast<Node*>(pNode)); // UBSAN: Invalid cast
		}
		return pNode;
	}
	T* insert(const T* pNext)
	{
		T* pNode = ::new(allocate())T();
		if (pNode)
		{
			insert(get_node(pNext), get_node(pNode));
		}
		return pNode;
	}
	T* insert(const T* pNext, const T& obj)
	{
		T* pNode = ::new(allocate())T(obj);
		if (pNode)
		{
			insert(get_node(pNext), get_node(pNode));
		}
		return pNode;
	}

	//
	// Удалить элементы.
	//
	void erase(T* p)
	{
		assert(p);
		assert(!empty());

		Node* pNode = get_node(p);
		remove(pNode);
		destroy(pNode);
		validate();
	}

	void pop_back()
	{
		erase(m_pTail);
	}

	void pop_front()
	{
		erase(m_pHead);
	}

	void clear()
	{
		// Destroy all elements, in reverse order
		for (Node* p = m_pTail; p != NULL; )
		{
			Node* pPrev = p->pPrev;
			destroy(p);
			p = pPrev;
		}
		reset();
	}

	//
	// Переместить элемент.
	//
	void move(const T* pDestObj, const T* pSrcObj)
	{
		assert(pDestObj && pSrcObj && pDestObj != pSrcObj);
		Node* pSrcNode = get_node(pSrcObj);

		remove(pSrcNode);
		insert(get_node(pDestObj), pSrcNode);
	}

	template<class TSizer>
	void GetMemoryUsagePlain(TSizer* pSizer) const
	{
		pSizer->AddObject(this, size() * sizeof(Node));
	}

	template<class TSizer>
	void GetMemoryUsage(TSizer* pSizer) const
	{
		for (const auto& e : * this)
		{
			if (pSizer->AddObject(&e, sizeof(Node)))
				e.GetMemoryUsage(pSizer);
		}
	}

protected:

	static Node*  m_pHead;
	static Node*  m_pTail;
	u32 m_nSize;

protected:
	
	uk getNode(Node* nod){return reinterpret_cast<uk> (nod);}
	ukk getNode(Node* nod) const{return reinterpret_cast<ukk> (nod);}

	void reset()
	{
		m_pHead = m_pTail = NULL;
		m_nSize = 0;
	}

	static Node* get_node(const T* p)
	{
		return non_const(alias_cast<const Node*>(p));
	}

	uk allocate()
	{
		uk pNew = ParticleObjectAllocator().Allocate(sizeof(Node));
		if (pNew)
			m_nSize++;
		return pNew;
	}
	void destroy(Node* pNode)
	{
		assert(pNode);
		pNode->~Node();
		ParticleObjectAllocator().Deallocate(pNode, sizeof(Node));
		m_nSize--;
	}

	void insert(Node* pNext, Node* pNode)
	{
		assert(pNode);

		Node*& pPrev = pNext ? pNext->pPrev : m_pTail;
		pNode->pPrev = pPrev;
		pNode->pNext = pNext;

		if (pPrev)
			pPrev->pNext = pNode;
		else
			m_pHead = pNode;

		pPrev = pNode;

		validate();
	}

	void remove(Node* pNode)
	{
		assert(pNode);
		assert(!empty());

		if (!pNode->pPrev)
		{
			assert(pNode == m_pHead);
			m_pHead = pNode->pNext;
		}
		else
			pNode->pPrev->pNext = pNode->pNext;

		if (!pNode->pNext)
		{
			assert(pNode == m_pTail);
			m_pTail = pNode->pPrev;
		}
		else
			pNode->pNext->pPrev = pNode->pPrev;
	}

	void validate() const
	{
#if defined(_DEBUG)
		if (empty())
		{
			assert(!m_nSize && !m_pHead && !m_pTail);
		}
		else
		{
			assert(m_pHead && !m_pHead->pPrev);
			assert(m_pTail && !m_pTail->pNext);
			if (m_nSize == 1)
				assert(m_pHead == m_pTail);
		}

		Node* pPrev = NULL;
		i32 nCount = 0;
		for (Node* p = m_pHead; p; p = p->pNext)
		{
			assert(p->pPrev == pPrev);
			assert(p->pNext || p == m_pTail);
			pPrev = p;
			nCount++;
		}
		assert(nCount == size());
#endif // _DEBUG
	}
};

typedef ParticleList<CParticle>::Node TParticleElem;

#endif // _DRX_PARTICLE_LIST_H_
