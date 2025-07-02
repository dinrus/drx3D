// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef DRXMEMREPLAYTRACKEDPTR_H
#define DRXMEMREPLAYTRACKEDPTR_H

#if CAPTURE_REPLAY_LOG

template<class _I> class MemReplayTrackedSmartPtr
{
private:
	_I* p;
public:
	MemReplayTrackedSmartPtr() : p(NULL) {}
	MemReplayTrackedSmartPtr(i32 Null) : p(NULL) {}
	MemReplayTrackedSmartPtr(_I* p_)
	{
		p = p_;
		if (p)
		{
			p->AddRef();
			DrxGetIMemReplay()->AddAllocReference(p, this);
		}
	}

	MemReplayTrackedSmartPtr(const MemReplayTrackedSmartPtr& p_)
	{
		p = p_.p;
		if (p)
		{
			p->AddRef();
			DrxGetIMemReplay()->AddAllocReference(p, this);
		}
	}
	~MemReplayTrackedSmartPtr()
	{
		if (p)
		{
			DrxGetIMemReplay()->RemoveAllocReference(this);
			p->Release();
		}
	}
	operator _I*() const { return p; }
	operator const _I*() const { return p; }
	_I&                       operator*() const      { return *p; }
	_I*                       operator->(void) const { return p; }
	_I*                       get() const            { return p; }
	MemReplayTrackedSmartPtr& operator=(_I* newp)
	{
		if (p)
			DrxGetIMemReplay()->RemoveAllocReference(this);
		if (newp)
			newp->AddRef();
		if (p)
			p->Release();
		if (newp)
			DrxGetIMemReplay()->AddAllocReference(newp, this);
		p = newp;
		return *this;
	}

	void reset()
	{
		MemReplayTrackedSmartPtr<_I>().swap(*this);
	}

	void reset(_I* p)
	{
		MemReplayTrackedSmartPtr<_I>(p).swap(*this);
	}

	MemReplayTrackedSmartPtr& operator=(const MemReplayTrackedSmartPtr& newp)
	{
		if (p)
			DrxGetIMemReplay()->RemoveAllocReference(this);
		if (newp.p)
			newp.p->AddRef();
		if (p)
			p->Release();
		if (newp.p)
			DrxGetIMemReplay()->AddAllocReference(newp.p, this);
		p = newp.p;
		return *this;
	}
	operator bool() const
	{
		return p != NULL;
	};
	bool operator!() const
	{
		return p == NULL;
	};
	bool operator==(const _I* p2) const
	{
		return p == p2;
	};
	bool operator==(_I* p2) const
	{
		return p == p2;
	};
	bool operator==(const MemReplayTrackedSmartPtr<_I>& rhs) const
	{
		return p == rhs.p;
	}
	bool operator!=(const _I* p2) const
	{
		return p != p2;
	};
	bool operator!=(_I* p2) const
	{
		return p != p2;
	};
	bool operator!=(const MemReplayTrackedSmartPtr& p2) const
	{
		return p != p2.p;
	};
	bool operator<(const _I* p2) const
	{
		return p < p2;
	};
	bool operator>(const _I* p2) const
	{
		return p > p2;
	};
	//! Set our contained pointer to null, but don't call Release().
	//! Useful for when we want to do that ourselves, or when we know
	//! that the contained pointer is invalid.
	friend _I* ReleaseOwnership(MemReplayTrackedSmartPtr<_I>& ptr)
	{
		_I* ret = ptr.p;
		if (ret)
			DrxGetIMemReplay()->RemoveAllocReference(&ptr);
		ptr.p = 0;
		return ret;
	}
	void swap(MemReplayTrackedSmartPtr<_I>& other)
	{
		std::swap(*this, other);
	}

	AUTO_STRUCT_INFO;
};

template<typename T>
ILINE void swap(MemReplayTrackedSmartPtr<T>& a, MemReplayTrackedSmartPtr<T>& b)
{
	a.swap(b);
}

template<class _I>
inline bool operator==(const MemReplayTrackedSmartPtr<_I>& p1, i32 null)
{
	assert(!null);
	return !(bool)p1;
}
template<class _I>
inline bool operator!=(const MemReplayTrackedSmartPtr<_I>& p1, i32 null)
{
	assert(!null);
	return (bool)p1;
}
template<class _I>
inline bool operator==(i32 null, const MemReplayTrackedSmartPtr<_I>& p1)
{
	assert(!null);
	return !(bool)p1;
}
template<class _I>
inline bool operator!=(i32 null, const MemReplayTrackedSmartPtr<_I>& p1)
{
	assert(!null);
	return (bool)p1;
}

template<typename T>
class MemReplayTracingPointer
{
public:
	MemReplayTracingPointer() {}

	MemReplayTracingPointer(T* ptr) : m_ptr(ptr)
	{
		DrxGetIMemReplay()->AddAllocReference(ptr, this);
	}

	MemReplayTracingPointer(const MemReplayTracingPointer<T>& other)
		: m_ptr(other.m_ptr)
	{
		DrxGetIMemReplay()->AddAllocReference(other.m_ptr, this);
	}

	MemReplayTracingPointer<T>& operator=(const MemReplayTracingPointer<T>& other)
	{
		DrxGetIMemReplay()->AddAllocReference(m_ptr, this);
		m_ptr = other.m_ptr;
		DrxGetIMemReplay()->AddAllocReference(other.m_ptr, this);
		return *this;
	}

	template<typename O>
	MemReplayTracingPointer(O* ptr) : m_ptr(ptr)
	{
		DrxGetIMemReplay()->AddAllocReference(ptr, this);
	}

	template<typename O>
	MemReplayTracingPointer<T>& operator=(const MemReplayTracingPointer<O>& other)
	{
		DrxGetIMemReplay()->AddAllocReference(m_ptr, this);
		m_ptr = other.m_ptr;
		DrxGetIMemReplay()->AddAllocReference(other.m_ptr, this);
		return *this;
	}

	~MemReplayTracingPointer()
	{
		DrxGetIMemReplay()->AddAllocReference(m_ptr, this);
	}

	T& operator*() const  { return *m_ptr; }
	T* operator->() const { return m_ptr; }

	operator bool() const { return m_ptr != 0; }
	operator T*() const { return m_ptr; }

private:
	T* m_ptr;
};

#else

	#define MemReplayTrackedSmartPtr _smart_ptr

#endif

#endif
